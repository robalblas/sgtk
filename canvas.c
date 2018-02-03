/**************************************************
 * RCSId: $Id: canvas.c,v 1.2 2016/05/25 19:13:15 ralblas Exp $
 *
 * Canvas related functions.
 * (e.g. create windows, buttons etc.)
 * Project: SGTK
 * Author: R. Alblas
 *
 * History: 
 * $Log: canvas.c,v $
 * Revision 1.2  2016/05/25 19:13:15  ralblas
 * _
 *
 * Revision 1.1  2015/11/15 22:32:25  ralblas
 * Initial revision
 *
 **************************************************/
/*******************************************************************
 * Copyright (C) 2000 R. Alblas. 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 * 02111-1307, USA.
 ********************************************************************/
#include <malloc.h>
#include "sgtk.h"
#include "gdk/gdkkeysyms.h"

/*************************************
 * Refresh defined area
 *************************************/
void Refresh_Rect(GtkWidget *drawing_area,
                  int x,int y,int w,int h)
{
  GdkRectangle update_rect;
  if (!drawing_area) return;
  if (w<0) return;
  if (h<0) return;
  update_rect.x=x;
  update_rect.width=w;
  update_rect.y=y;
  update_rect.height=h;
  gtk_widget_draw(drawing_area,&update_rect);
}

void draw_rgbmap(GtkWidget *widget,
                 guchar    *rgbbuf,
                 int x,int y,int w,int h)
{
  if (rgbbuf)
  {
    guchar *p=rgbbuf+(x+y*widget->allocation.width)*3;

    gdk_draw_rgb_image(widget->window,
           widget->style->fg_gc[GTK_STATE_NORMAL],
           x,y,w,h,
           GDK_RGB_DITHER_NONE,p,
           widget->allocation.width*3);
  }
}

/*************************************
 * Refresh (part of) canvas 
 *************************************/
gboolean expose_event(GtkWidget      *widget,
                      GdkEventExpose *event)
{
  GtkWidget *window;
  GtkWidget *dr_a;
  RGBI *rgbi;
  GdkPixmap *pixmap;
  int x,y,w,h;

  if (!widget) return FALSE;

  pixmap=Get_Pixmap(gtk_widget_get_toplevel(widget));

  if ((rgbi=Get_RGBI(widget)))
  {
    window=widget;
    dr_a=widget;
  }

  else // als alle rgbi's in drawing_area dan kan dit weg!
  {
    window=gtk_widget_get_toplevel(widget);
    dr_a=Find_Widget(window,"GTK_DRAWING_AREA");
    if (!dr_a) return FALSE;
    rgbi=Get_RGBI(window);
  }



/* Determine part of drawable to refresh */
  if (event)
  {
/* Refresh-part is in GdkEventExpose */ 
    x=event->area.x;
    y=event->area.y;
    w=event->area.width;
    h=event->area.height;
  }
  else
  {
/* No GdkEventExpose avaliable ==> refresh full drawable */
    x=0;
    y=0;
    w=dr_a->allocation.width;
    h=dr_a->allocation.height;
  }

  if (x>=dr_a->allocation.width) return FALSE;
  if (y>=dr_a->allocation.height) return FALSE;
  if (x<0) return FALSE;
  if (y<0) return FALSE;
/* Copy pixmap to drawable */
  if (pixmap)
  {
    gdk_draw_pixmap(dr_a->window,
                    dr_a->style->fg_gc[GTK_WIDGET_STATE(dr_a)],
                    pixmap,
                    x,y,x,y,w,h);

  }
  if ((rgbi) && (rgbi->rgbbuf))
  {
    guchar *p;
    if (y+h>=dr_a->allocation.height)   // anders: fout, kan optreden bij resize??? 22-2-2009
    {
      h=dr_a->allocation.height-y-1;
    }
    p=rgbi->rgbbuf+(y*dr_a->allocation.width+x)*3;
    gdk_draw_rgb_image(dr_a->window,
                       dr_a->style->fg_gc[GTK_STATE_NORMAL],
                       x,y,w,h,
                       GDK_RGB_DITHER_NONE,p,
                       dr_a->allocation.width*3);

  }

  return FALSE;
}


/*************************************
 * Create a new drawable
 *************************************/
GtkWidget *Create_Drawable1(char *id,int width,int height,
                           void      config_event(),        /* executed after resize */
                           void      button_press_event(),  /* mouse press handling */
                           void      motion_notify_event()) /* mouse move handling */
{
  GtkWidget *drawing_area,*drawing_area_org;
  RGBI *rgbi;
  drawing_area=gtk_drawing_area_new();
  if (id) Add_WidgetID(drawing_area,id);


  if ((width) || (height)) 
    gtk_drawing_area_size(GTK_DRAWING_AREA(drawing_area),width,height);


  gtk_widget_set_events(drawing_area, GDK_BUTTON_PRESS_MASK |
                                      GDK_POINTER_MOTION_MASK |
                                      GDK_POINTER_MOTION_HINT_MASK);
//                                      GDK_KEY_PRESS_MASK|
//                                      GDK_ENTER_NOTIFY_MASK|
//                                      GDK_KEY_RELEASE_MASK);
                                      
// werkt niet!?? GDK_KEY_PRESS_MASK|GDK_ENTER_NOTIFY_MASK|GDK_KEY_RELEASE_MASK);

/* Add window update routines, to reconstruct picture */
  gtk_signal_connect(GTK_OBJECT(drawing_area),"expose_event",
                (GtkSignalFunc)expose_event,NULL);



  if (config_event)
    gtk_signal_connect_after(GTK_OBJECT(drawing_area),"configure_event",
                (GtkSignalFunc)config_event,NULL);



  if (button_press_event)
    gtk_signal_connect(GTK_OBJECT(drawing_area),"button_press_event",
                (GtkSignalFunc)button_press_event,NULL);

  if (motion_notify_event)
    gtk_signal_connect(GTK_OBJECT(drawing_area),"motion_notify_event",
                (GtkSignalFunc)motion_notify_event,NULL);


  if (!(rgbi=(RGBI *)calloc(sizeof(RGBI),1))) return NULL;
  gtk_object_set_data(GTK_OBJECT(drawing_area),DRAWABLE_INFO,(gpointer)rgbi);

  gtk_widget_show(drawing_area);
  drawing_area_org=drawing_area;

//  if (height) drawing_area=Pack(NULL,'v',drawing_area,1,NULL);
//  if (width)  drawing_area=Pack(NULL,'h',drawing_area,1,NULL);
  if (drawing_area!=drawing_area_org) 
    gtk_object_set_data(GTK_OBJECT(drawing_area),CHILD_ID,(gpointer)drawing_area_org);

  return drawing_area;
}

GtkWidget *Create_Drawable(int width,int height,
                           void      config_event(),        /* executed after resize */
                           void      button_press_event(),  /* mouse press handling */
                           void      motion_notify_event()) /* mouse move handling */
{
  return Create_Drawable1(NULL, width, height,
                           config_event,        /* executed after resize */
                           button_press_event,  /* mouse press handling */
                           motion_notify_event); /* mouse move handling */
}


/*************************************
 * Create a new canvas; 1 per window possible
 *************************************/
GtkWidget *Create_Canvas1(
             GtkWidget *window,               /* this window gets canvas */
             char      *id,
             gboolean  dis_scroll,
             int       width,                 /* size; if non-zero: */
             int       height,                /*       Add scrollbars */
             void      config_event(),        /* executed after resize */
             gboolean  key_press_event(),     /* key-press handling */
             void      button_press_event(),  /* mouse press handling */
             void      motion_notify_event()) /* mouse move handling */
{
  GtkWidget *drawing_area;
  GtkWidget *scrolled_window=NULL;
  GtkWidget *canvas;
  WINDOW_DATA *wnd_data=gtk_object_get_data(GTK_OBJECT(window),WNDINFO);
  drawing_area=Create_Drawable1(id,width,height,config_event,button_press_event,motion_notify_event);

  if (key_press_event)
    gtk_signal_connect(GTK_OBJECT(window),"key_press_event",
                        (GtkSignalFunc)key_press_event,drawing_area);
  gtk_widget_show(window);

/* if window size and canvas size != 0 define scrolled window */
  if ((!dis_scroll) && ((width>0) || (height>0)) &&
      ((wnd_data->default_width>0) || (wnd_data->default_height>0)))
  {
    scrolled_window = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window),drawing_area); 

//    gtk_drawing_area_size(GTK_DRAWING_AREA(drawing_area),width,height);
    canvas=scrolled_window;
    gtk_object_set_data(GTK_OBJECT(canvas),CHILD_ID,(gpointer)drawing_area);
  }
  else
  {
    canvas=drawing_area;
  }

  gtk_widget_show(canvas);
  return canvas;
}

GtkWidget *Create_Canvas(
             GtkWidget *window,               /* this window gets canvas */
             int       width,                 /* size; if non-zero: */
             int       height,                /*       Add scrollbars */
             void      config_event(),        /* executed after resize */
             gboolean  key_press_event(),     /* key-press handling */
             void      button_press_event(),  /* mouse press handling */
             void      motion_notify_event()) /* mouse move handling */
{
  return Create_Canvas1
         (
             window,
             NULL,
             FALSE,
             width,
             height,
             config_event,        /* executed after resize */
             key_press_event,     /* key-press handling */
             button_press_event,  /* mouse press handling */
             motion_notify_event
         ); /* mouse move handling */
}

/*************************************
 * Connect mouse button and movement events to functions
 * mousebutton_func: executed if mouse button is pressed
 * mousemove_func: executed if mouse is moved
 *************************************/
void Add_Mouse_events(GtkWidget *window,
                      void      mousebutton_func(),
                      void      mousemove_func())
{
  GtkWidget *drawing_area=Find_Widget(window,"GTK_DRAWING_AREA");
  if (mousebutton_func) 
    gtk_signal_connect(GTK_OBJECT(drawing_area),"button_press_event",
                                  (GtkSignalFunc)mousebutton_func,NULL);
  if (mousemove_func) 
    gtk_signal_connect(GTK_OBJECT(drawing_area),"motion_notify_event",
                                  (GtkSignalFunc)mousemove_func,NULL);
}


/*************************************
 * Add a color RGB palette to a window
 * This clrmap is suitable to be used with gdk_draw_indexed_image().
 *************************************/
void Create_RGBPalette(GtkWidget   *drawable,      /* window struct */
                       GdkColormap *clrmap)      /* Contains colors to load */
{
  RGBI *rgbi=Get_RGBI(drawable);
  guint32 *colors=calloc(sizeof(guint32),clrmap->size);
  int i,clrcmpr;
  if (!rgbi) return;
  if (rgbi->rgb_cmap) gdk_rgb_cmap_free(rgbi->rgb_cmap);
  rgbi->size=MIN(clrmap->size,256);
  clrcmpr=clrmap->size/rgbi->size;
  for (i=0; i<rgbi->size; i++)
    colors[i]=((clrmap->colors[i*clrcmpr].red   & 0xff00)<<8) + 
               (clrmap->colors[i*clrcmpr].green & 0xff00)     + 
              ((clrmap->colors[i*clrcmpr].blue  & 0xff00)>>8);

  rgbi->rgb_cmap=gdk_rgb_cmap_new(colors,rgbi->size);
  free(colors);
}

/*************************************
 * Add a color palette to a window (based on gc)
 *************************************/
void Create_GCPalette(GtkWidget   *window,        /* window struct */
                      GdkColormap *clrmap)        /* Contains colors to load */
{
  WINDOW_DATA *wnd_data=gtk_object_get_data(GTK_OBJECT(window),WNDINFO);
  GdkGC **gc;
  int i;
  GtkWidget *drawing_area=Find_Widget(window,"GTK_DRAWING_AREA");
  GdkColormap *colormap=gtk_widget_get_colormap(drawing_area);

  if (wnd_data->gc) free(wnd_data->gc);

  gc=(GdkGC **)calloc(sizeof(GdkGC),clrmap->size);

  for (i=0; i<clrmap->size; i++)
  {
    if (!(gdk_colormap_alloc_color(colormap,&clrmap->colors[i],FALSE,TRUE)))
      return;

    gc[i]=gdk_gc_new(drawing_area->window);
    gdk_gc_set_foreground(gc[i],&clrmap->colors[i]);
  }

  wnd_data->gc=gc;
  wnd_data->gc_size=clrmap->size;
}

/*************************************
 * redefine the pixmap (needed after resize)
 *************************************/
void Renew_Pixmap(GtkWidget *window,gboolean clear)
{
  GdkPixmap *pixmap=Get_Pixmap(window);
  GtkWidget *drwbl=Find_Widget(window,"GTK_DRAWING_AREA");
  if (!window) return;
  if (!drwbl) return;

  if (pixmap) gdk_pixmap_unref(pixmap);
  pixmap=gdk_pixmap_new(window->window,
                        drwbl->allocation.width,
                        drwbl->allocation.height,-1);
/* Clear pixmap */
  if (clear)
    gdk_draw_rectangle(pixmap,window->style->black_gc,TRUE,0,0,
                       drwbl->allocation.width,drwbl->allocation.height);
  Put_Pixmap(window,pixmap);
}


/*************************************
 * redefine the rgb buffer (needed after resize)
 *************************************/
gboolean Renew_RGBBuf(GtkWidget *widget)
{
  RGBI *rgbi=Get_RGBI(widget);   // widget=drawable
  GtkWidget *drawing_area;
  int new_size;
  if (!rgbi) return FALSE;
  
  if (GTK_IS_DRAWING_AREA(widget))
    drawing_area=widget;
  else                                 // weg als 
  {
    drawing_area=Find_Widget(widget,"GTK_DRAWING_AREA");
  }
  if (!drawing_area) return FALSE;

  new_size=drawing_area->allocation.width*
                drawing_area->allocation.height*3;

  if (new_size!=rgbi->bufsize) 
  {
    if (rgbi->rgbbuf) free(rgbi->rgbbuf);

    rgbi->bufsize=new_size;
    rgbi->rgbbuf=calloc(rgbi->bufsize,1);
    rgbi->width =drawing_area->allocation.width;
    rgbi->height=drawing_area->allocation.height;
    return TRUE;
  }
  else
  {
    memset(rgbi->rgbbuf,0,rgbi->bufsize);
    return FALSE;
  }
}

