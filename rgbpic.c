#define NW_DRAW
/**************************************************
 * RCSId: $Id: rgbpic.c,v 1.2 2016/03/05 22:08:23 ralblas Exp $
 *
 * Canvas related functions.
 * (e.g. create windows, buttons etc.)
 * Project: SGTK
 * Author: R. Alblas
 *
 * History: 
 * $Log: rgbpic.c,v $
 * Revision 1.2  2016/03/05 22:08:23  ralblas
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

// link to data
/******************************************************
 * Functions for easy RGB picture creation.
 * Resizing is handled.
 * Needed: A function 'draw_pic(GtkWidget *widget)'
 *         with inside the drawing loop:
 *           if (*redraw) break;
 ******************************************************/

/* Remove everything of RGB-pic except window itself */
void Close_RGBPic(GtkWidget *widget)
{
  GtkWidget *window;
  RGBI *rgbi;
  RGBPICINFO *rgbpicinfo;
  GtkWidget *drawing_area=widget; // Find_Widget(window,"GTK_DRAWING_AREA");
  char *p;

  window=gtk_widget_get_toplevel(widget);
  rgbi=Get_RGBI(drawing_area);
  rgbpicinfo=(RGBPICINFO *)gtk_object_get_data((gpointer)drawing_area,RGBPI_DL);

  if (rgbpicinfo)
  {
    if (rgbpicinfo->str8) free(rgbpicinfo->str8);
    if (rgbpicinfo->str16) free(rgbpicinfo->str16);
    free(rgbpicinfo);
  }
  if (rgbi)
  {
    if (rgbi->rgbbuf) free(rgbi->rgbbuf);
    gtk_object_remove_data(GTK_OBJECT(drawing_area),RGBPI_DL);
  }
  if ((p=gtk_object_get_data(GTK_OBJECT(drawing_area),DRAWABLE_INFO)))
  {
    free(p);
  }
  Remove_Window_Info(window);
}


/*************************************
 * Update window.
 * Can be activated multiple times, e.g. during resizing window.
 * Actual redrawing is always done just 1x at a time;
 * this is handled using enable flag and redraw flag.
 *************************************/
static void Update_RGBPic_Wait(RGBPICINFO *rgbpicinfo)
{
  int panic_abort;
  if (!rgbpicinfo) return;
  rgbpicinfo->redraw=TRUE;           /* Signal that a redraw is requested. */
/* Only update if enabled. */
  if (Get_Enable_Update(rgbpicinfo->widget))
  {
    rgbpicinfo->redraw=FALSE;        /* Redraw accepted, remove signal */
    Set_Enable_Update(rgbpicinfo->widget,FALSE); /* Prevent re-drawing while drawing */

    if (rgbpicinfo->draw_func)
    {
      int (*func)()=rgbpicinfo->draw_func;
      panic_abort=func(rgbpicinfo->widget);  // if window destroyed: returns !=0
    }

    if (!panic_abort)
    {
      Set_Enable_Update(rgbpicinfo->widget,TRUE);
    }

/* If redraw was requested (during draw_pic) then honor it now. */
    if (rgbpicinfo->redraw) 
    {
      rgbpicinfo->redraw=FALSE;
      Update_RGBPic(rgbpicinfo);
    }
  }  
}

/*************************************
 * Scheduling redraw.
 *************************************/
static gboolean sched_update(gpointer data)
{
  RGBPICINFO *rgbpicinfo=(RGBPICINFO *)data;
  Update_RGBPic_Wait(rgbpicinfo);
  return FALSE;
}

 
/*************************************
 * Update window (scheduled)
 *************************************/
void Update_RGBPic(RGBPICINFO *rgbpicinfo)
{
  g_idle_add(sched_update,rgbpicinfo);
}


/*************************************
 * Redraw function activated by canvas after resizing
 *************************************/
static void configure_RGBPICevent(GtkWidget         *widget,
                                  GdkEventConfigure *event)
{
  RGBPICINFO *rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)widget,RGBPI_DL);
  Renew_RGBBuf(widget);
  Update_RGBPic(rgbpi);
}


GtkWidget *Create_RGBCanvas1(GtkWidget *widget,
                            int pw,int ph,            /* size picture (or 0) */
                            char *id,
                            int draw_func(),         /* drawing function or NULL */
                            gboolean key_func(),      /* key function or NULL */
                            void mouse_func())        /* mouse function or NULL */
{
  GtkWidget *canvas;
  RGBPICINFO *rgbpicinfo;
#ifdef NW_DRAW
  canvas=Create_Canvas1(widget,id,FALSE,pw,ph,configure_RGBPICevent,key_func,mouse_func,mouse_func);
//  canvas=Create_Canvas1(widget,pw,ph,id,configure_RGBPICevent,key_func,mouse_func,mouse_func);
  rgbpicinfo=calloc(1,sizeof(*rgbpicinfo));
  gtk_object_set_data((gpointer)canvas,RGBPI_DL,(gpointer)rgbpicinfo);
  rgbpicinfo->widget=canvas;
  rgbpicinfo->draw_func=draw_func;
#else
  rgbpicinfo=calloc(1,sizeof(*rgbpicinfo));
  gtk_object_set_data((gpointer)widget,RGBPI_DL,(gpointer)rgbpicinfo);
  rgbpicinfo->widget=widget;
  rgbpicinfo->draw_func=draw_func;

  canvas=Create_Canvas(widget,pw,ph,configure_RGBPICevent,key_func,mouse_func,mouse_func);
#endif
  return canvas;
}

GtkWidget *Create_RGBCanvas(GtkWidget *widget,
                            int pw,int ph,            /* size picture (or 0) */
                            int draw_func(),         /* drawing function or NULL */
                            gboolean key_func(),      /* key function or NULL */
                            void mouse_func())        /* mouse function or NULL */
{
  return Create_RGBCanvas1(widget,pw, ph,NULL, 
                           draw_func,key_func,mouse_func);
}

/*************************************
 *************************************
 * Main RGB creation
 * Create_RGBPic: 
 *  draw_func called by Update_RGBPic_Wait
 *            called by sched_update
 *            scheduled by Update_RGBPic
 *            called by configure_RGBPICevent (and Update_RGBPic_Wait)
 *            gtk_signal_connect in Create_Canvas
 *
 *************************************
 *************************************/

GtkWidget *Create_RGBPic(GtkWidget *widget,char *name,
                         int ww,int wh,            /* size window */
                         int pw,int ph,            /* size picture (or 0) */
                         int draw_func(),         /* drawing function or NULL */
                         gboolean key_func(),      /* key function or NULL */
                         void mouse_func(),        /* mouse function or NULL */
                         void close_func(),        /* close function or NULL (int. close func) */
                         GtkWidget *extra_buts)    /* extra buttons to add */
{

  GtkWidget *wnd,*canvas=NULL,*w[3];
  wnd=Find_Parent_Window(widget);
  if (close_func)
    wnd=Create_Window(wnd,ww,wh,name,close_func);
  else
    wnd=Create_Window(wnd,ww,wh,name,NULL);
  if (!wnd) return NULL;

  canvas=Create_RGBCanvas(wnd,pw, ph,draw_func,key_func,mouse_func);
  w[1]=Create_Button("Dismiss",Close_Window);

  if (extra_buts)
    w[0]=SPack(NULL,"h",extra_buts,"ef1",w[1],"ef1",NULL);
  else
    w[0]=SPack(NULL,"h",w[1],"ef1",NULL);

  w[0]=Pack(NULL,'v',canvas,1,w[0],1,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),w[0]);
  gtk_widget_show_all(wnd);
  Set_Enable_Update(canvas,TRUE);

  return wnd;
}

/*************************************
 * Draw.
 * draw_width/draw_height=size drawable
 * rgbpi->pwidth/pheight=size picture
 * only draw lines y1...y2
 *************************************/
void RGB_Pic_drawableupdate(GtkWidget *drawing_area,int y1,int y2,gboolean cursor_update)
{
  RGBPICINFO *rgbpi=gtk_object_get_data(GTK_OBJECT(drawing_area),RGBPI_DL);
  RGBI *rgbi=Get_RGBI(drawing_area);
  GdkColor clr;
  int draw_width =drawing_area->allocation.width;
  int draw_height=drawing_area->allocation.height;

  int x,y,pos;
  int px,py;
  int pxp,pyp,nx,ny;
  int rgb[3]={0,0,0};
  unsigned int lpos,i;
  if (!rgbpi) return;
  for (i=0; i<NLUMSTAT; i++) rgbpi->lumstat[i]=0;
  pyp=-1;
  if (cursor_update) Create_Cursor(drawing_area,GDK_WATCH);
// tijdeliJK!
//if ((rgbpi->zx<0) || (rgbpi->zx>100)) { /*printf("zx=%f\n",rgbpi->zx);*/ rgbpi->zx=1; return; }

  for (y=y1; y<=y2; y++)                      /* go through all picture rows */
  {
    if (CheckKeyPressed(GDK_Escape)) break;  /* stop if escape pressed */

    while (g_main_iteration(FALSE));         /* Handle GTK actions now and then */
    if (rgbpi->redraw) break;                /* Redraw requested, so abort current drawing */
    if (rgbpi->size_drawable_limit)
      py=(y-rgbpi->oy)*rgbpi->zy*rgbi->height/rgbpi->pheight;
    else
      py=y*rgbi->height/rgbpi->pheight;        /* convert to row in drawable */

    if (py<0) continue;
    if (py>=rgbi->height) continue;
    if (py==pyp) continue;
    if (pyp==-1) pyp=py;

    pxp=-1;
    for (x=0; x<rgbpi->pwidth; x++)          /* go through all picture cols */
    {
if (rgbpi->zx<0) { printf("??? zx=%f\n",rgbpi->zx);rgbpi->zx=1.; }
      if (rgbpi->size_drawable_limit)
        px=(x-rgbpi->ox)*rgbpi->zx*rgbi->width/rgbpi->pwidth;
      else
        px=x*rgbi->width/rgbpi->pwidth;        /* convert to col in drawable */
      if (px<0) continue;
      if (px>=rgbi->width) continue;
      if (px==pxp) continue;
      if (pxp==-1) pxp=px;

      pos=(x+y*rgbpi->pwidth)*3;             /* pointer to pixel in pic */

      if (rgbpi->str8)                       /* if 1-byte per pixel */
      {
        rgb[0]=rgbpi->str8[pos+0];           /* get red pixel from pic */
        rgb[1]=rgbpi->str8[pos+1];           /* id., green */
        rgb[2]=rgbpi->str8[pos+2];           /* id., blue */
      }
      if (rgbpi->str16)                      /* if 2-bytes per pixel */
      {
        rgb[0]=rgbpi->str16[pos+0];          /* get red pixel from pic */
        rgb[1]=rgbpi->str16[pos+1];          /* id., green */
        rgb[2]=rgbpi->str16[pos+2];          /* id., blue */
      }

/* Update lim statistics; convert always to [0:255] range */
      if (rgbpi->anal_lum)
      {
        lpos=rgb[0]*0xff/rgbpi->lummax; if (lpos<NLUMSTAT) rgbpi->lumstat[lpos]++;
        lpos=rgb[1]*0xff/rgbpi->lummax; if (lpos<NLUMSTAT) rgbpi->lumstat[lpos]++;
        lpos=rgb[2]*0xff/rgbpi->lummax; if (lpos<NLUMSTAT) rgbpi->lumstat[lpos]++;

/* Convert lum-in-pic to lum-in-drawable; clip between 0 and 0xff */
        clr.red  =MIN(0xff,0xff*(MAX(rgb[0]-rgbpi->lmin,0))/(rgbpi->lmax-rgbpi->lmin));
        clr.green=MIN(0xff,0xff*(MAX(rgb[1]-rgbpi->lmin,0))/(rgbpi->lmax-rgbpi->lmin));
        clr.blue =MIN(0xff,0xff*(MAX(rgb[2]-rgbpi->lmin,0))/(rgbpi->lmax-rgbpi->lmin));
      }
      else
      {
        clr.red=rgb[0];
        clr.green=rgb[1];
        clr.blue=rgb[2];
      }
/* Draw pixel. If drawable-size > pic-size then copy pixels. */  
      for (ny=pyp+1; ny<=py; ny++)
        for (nx=pxp+1; nx<=px; nx++)
          draw_rgbpoint(rgbi,&clr,nx,ny);

      pxp=px;

    } /* End go through all picture cols */


    clr.red=clr.green=clr.blue=0;
    for (x=rgbi->width*rgbpi->zx; x<rgbi->width; x++)          /* clear out-of-pic area */
    {
      draw_rgbpoint(rgbi,&clr,x,py);
    }
    
    Refresh_Rect(drawing_area,0,pyp+1,rgbi->width,py-pyp);    /* Make new row visible */
    pyp=py;

/* Start again if size window changed during plotting */
    if ((draw_width!=drawing_area->allocation.width) ||
        (draw_height!=drawing_area->allocation.height))
    {
      RGB_Pic_drawableupdate(drawing_area,y1,y2,cursor_update);
      break;
    }

  } /* End go through all picture rows */

  clr.red=clr.green=clr.blue=0;
  pyp=rgbi->height*rgbpi->zy;
  for (y=pyp; y<rgbi->height; y++)      /* clear out-of-pic area */
  {
    for (x=0; x<rgbi->width; x++)                          /* clear out-of-pic area */
    {
      draw_rgbpoint(rgbi,&clr,x,y);
    }
  }
  Refresh_Rect(drawing_area,0,pyp,rgbi->width,rgbi->height);    /* Make new row visible */

  if (rgbpi->xdrawfunc)
  {
    rgbpi->xdrawfunc(rgbi,rgbpi);
    Refresh_Rect(drawing_area,0,0,rgbi->width,rgbi->height);
  }

  if (cursor_update) Create_Cursor(drawing_area,GDK_TOP_LEFT_ARROW);
}

/*
void RGB_Pic_drawupdate(GtkWidget *drawing_area,int y1,int y2,gboolean cursor_update)
{
  RGB_Pic_drawableupdate(drawing_area, y1, y2, cursor_update);
}
*/

/*************************************
 * Draw.
 * draw_width/draw_height=size drawable
 * rgbpi->pwidth/pheight=size picture
 *************************************/
static int RGB_Pic_drawfunc(GtkWidget *widget)
{
  RGBPICINFO *rgbpi=gtk_object_get_data(GTK_OBJECT(widget),RGBPI_DL);
  RGB_Pic_drawableupdate(widget,0,rgbpi->pheight-1,TRUE);
  return 0;
}

#define CNVS_ID "Is_Canvas"
void makeit_square(int sw,int sh,RGBPICINFO *rgbpi)
{
  int ow=rgbpi->pwidth;
  int oh=rgbpi->pheight;
  if (!rgbpi->do_shape) return;
  if (!rgbpi->pixel_shape) return;
  // size to 'square' pixels such that in case of full-zoom full pic is visible.
  if (sw*oh<sh*ow)
  {
    rgbpi->zy=(float)sw/sh*oh/ow*rgbpi->pixel_shape*rgbpi->zx;
  }
  else
  {
    rgbpi->zx=(float)sh/sw*ow/oh/rgbpi->pixel_shape*rgbpi->zy;
  }
}

void Set_Scrollbars(GtkWidget *wnd,RGBPICINFO *rgbpi)
{
  int ox,oy;
  float zx,zy;
  ox=rgbpi->ox;
  oy=rgbpi->oy;
  zx=rgbpi->zx;
  zy=rgbpi->zy;
  Set_Scroll(wnd,LAB_HBAR,rgbpi->pwidth,&ox,&zx,
                 LAB_VBAR,rgbpi->pheight,&oy,&zy);
}

static int mx,my;
/*************************************
 * Key handling
 *************************************/
static gboolean RGB_Pic_keyfunc1(GtkWidget *widget,char *id, GdkEventKey *event,GtkWidget *extra)
{
  GtkWidget *window=Find_Parent_Window(widget);
  GtkWidget *drawing_area=widget;
  GtkWidget *canvas;
  RGBPICINFO *rgbpi;
  gboolean do_draw=FALSE;
  gboolean do_update=FALSE;
// Volgende regel pakt 1e gevonden canvas, moet canvas behorende bij widget vinden.
// -> uit widget=drawing_area: scrolled_window=canvas,
//     gtk_object_set_data(GTK_OBJECT(scrolled_window),CHILD_ID,(gpointer)drawing_area);

  GtkAdjustment *hadj,*vadj;
  int w,h;
  int keyval=event->keyval;
  if (!window) return FALSE;
  if (!drawing_area) return FALSE;
  if (id)
    canvas=Find_Widget(widget,id);
  else
    canvas=Find_Widget(widget,CNVS_ID);
  if (!canvas) return FALSE;
  if (extra) { drawing_area=extra; widget=extra; }
  rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)drawing_area,RGBPI_DL);
  if (!rgbpi) return FALSE;

  if (rgbpi->suppress_key) return FALSE;

  if (!(rgbpi->zx)) rgbpi->zx=1;
  if (!(rgbpi->zy)) rgbpi->zy=1;

  if (GTK_IS_SCROLLED_WINDOW(canvas))
  {
    /* canvas in scrolled window */
    gdk_window_get_size(window->window,&w,&h);
    /* correction because of scrol/button area;
       too much corr. is recorrected by gtk_drawing_area_size
    (Vies; moet anders!)
    */
    w-=25;
    h-=75;
    hadj=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(canvas));
    vadj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(canvas));
// Dit lijkt overbodig! Houdt veranderingen ox/oy vanaf buiten tegen, daarom weggehaald!
// 20-04-2010
//    rgbpi->ox=hadj->value*rgbpi->pwidth/w/rgbpi->zx;
//    rgbpi->oy=vadj->value*rgbpi->pheight/h/rgbpi->zy;
  }
  else
  {
    /* scrollbars added 'manually' */
    w=drawing_area->allocation.width;
    h=drawing_area->allocation.height;
    hadj=gtk_object_get_data(GTK_OBJECT(rgbpi->widget),LAB_HBAR);
    vadj=gtk_object_get_data(GTK_OBJECT(rgbpi->widget),LAB_VBAR);
// Dit lijkt overbodig! Houdt veranderingen ox/oy vanaf buiten tegen, daarom weggehaald!
// 20-04-2010
//    rgbpi->ox=GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(hadj),ADJUST_ID))->value;
//    rgbpi->oy=GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(vadj),ADJUST_ID))->value;
  }
  
#define fix_zoomfactor 2
#define fix_zoomfactorh 1.25
  switch (keyval)
  {
    case 'r': case 'R':
      do_draw=TRUE;
    break;
/* Zoom */
    case 'f': case 'F':
      rgbpi->zx=1;
      rgbpi->zy=1;
      rgbpi->ox=0;
      rgbpi->oy=0;
      do_draw=TRUE;
    break;
    case 'I':
      rgbpi->zx=rgbpi->zx*fix_zoomfactor;
      rgbpi->ox=mx-(int)(rgbpi->pwidth/rgbpi->zx*(fix_zoomfactor-1)/2);
      rgbpi->ox=MAX(rgbpi->ox,0);
      rgbpi->ox=MIN(rgbpi->ox,rgbpi->pwidth);

      rgbpi->zy=rgbpi->zy*fix_zoomfactor;
      rgbpi->oy=my-(int)(rgbpi->pheight/rgbpi->zy*(fix_zoomfactor-1)/2);
      rgbpi->oy=MAX(rgbpi->oy,0);
      rgbpi->oy=MIN(rgbpi->oy,rgbpi->pheight);

/* Move 'virtual cursor pos.' to selected part; 
   OK as long as actual cursor isn't moved!
(werkt nog niet helemaal perfect)
*/
      mx=rgbpi->ox+(int)(rgbpi->pwidth/rgbpi->zx*(fix_zoomfactor-1)/2);
      my=rgbpi->oy+(int)(rgbpi->pheight/rgbpi->zy*(fix_zoomfactor-1)/2);
      do_draw=TRUE;
    break;
    case 'i':
      rgbpi->zx=rgbpi->zx*fix_zoomfactor;
      rgbpi->ox=rgbpi->ox+
                   (int)(rgbpi->pwidth/rgbpi->zx*(fix_zoomfactor-1)/2);
      rgbpi->ox=MAX(rgbpi->ox,0);
      rgbpi->ox=MIN(rgbpi->ox,rgbpi->pwidth);


      rgbpi->zy=rgbpi->zy*fix_zoomfactor;
      rgbpi->oy=rgbpi->oy+
                   (int)(rgbpi->pheight/rgbpi->zy*(fix_zoomfactor-1)/2);
      rgbpi->oy=MAX(rgbpi->oy,0);
      rgbpi->oy=MIN(rgbpi->oy,rgbpi->pheight);

      do_draw=TRUE;
    break;
    case 'j':
      rgbpi->zx=rgbpi->zx*fix_zoomfactorh;
      rgbpi->ox=rgbpi->ox+
                   (int)(rgbpi->pwidth/rgbpi->zx*(fix_zoomfactorh-1)/2);
      rgbpi->ox=MAX(rgbpi->ox,0);
      rgbpi->ox=MIN(rgbpi->ox,rgbpi->pwidth);


      rgbpi->zy=rgbpi->zy*fix_zoomfactorh;
      rgbpi->oy=rgbpi->oy+
                   (int)(rgbpi->pheight/rgbpi->zy*(fix_zoomfactorh-1)/2);
      rgbpi->oy=MAX(rgbpi->oy,0);
      rgbpi->oy=MIN(rgbpi->oy,rgbpi->pheight);

      do_draw=TRUE;
    break;

    case 'p':
      rgbpi->zx=(float)rgbpi->pwidth/(float)w;
      rgbpi->zy=(float)rgbpi->pheight/(float)h;
      do_draw=TRUE;
    break;

    case 'o': case 'O':
      rgbpi->ox=rgbpi->ox-
                   (int)(rgbpi->pwidth/rgbpi->zx*(fix_zoomfactor-1)/2);
      rgbpi->zx=rgbpi->zx/fix_zoomfactor;

      rgbpi->oy=rgbpi->oy-
                   (int)(rgbpi->pheight/rgbpi->zy*(fix_zoomfactor-1)/2);
      rgbpi->zy=rgbpi->zy/fix_zoomfactor;

      do_draw=TRUE;
    break;

/* Shift */
    case GDK_Left: // &0xff:
      rgbpi->ox=rgbpi->ox-(rgbpi->pwidth/rgbpi->zx/2);
      do_update=TRUE;
    break;
    case GDK_Up: // &0xff:
      rgbpi->oy=rgbpi->oy-(rgbpi->pheight/rgbpi->zy/2);
      do_update=TRUE;
    break;
    case GDK_Right: // &0xff:
      rgbpi->ox=rgbpi->ox+(rgbpi->pwidth/rgbpi->zx/2);
      do_update=TRUE;
    break;
    case GDK_Down: // &0xff:
      rgbpi->oy=rgbpi->oy+(rgbpi->pheight/rgbpi->zy/2);
      do_update=TRUE;
    break;

/* Lum */
    case 'a':
    {
      int i,ncut=0;
      for (i=1; i<NLUMSTAT-1; i++)
        ncut=MAX(ncut,rgbpi->lumstat[i]);          /* max. # pics with certain lum */
      ncut/=100;                                   /* for setting lum boundaries */

      for (i=1; i<NLUMSTAT-1; i++)
        if (rgbpi->lumstat[i]>ncut) break;         /* determine lmin */
      rgbpi->lmin=i*rgbpi->lummax/(NLUMSTAT-1);

      for (i=NLUMSTAT-1; i>0; i--)
        if (rgbpi->lumstat[i]>ncut) break;         /* determine lmax */
      rgbpi->lmax=i*rgbpi->lummax/NLUMSTAT;

      if (rgbpi->lmin>rgbpi->lmax)                 /* should never occur */
      {
         i=rgbpi->lmin; rgbpi->lmin=rgbpi->lmax; rgbpi->lmax=i;
      }
      do_draw=TRUE;
    }
    break;
    case 'x':
      rgbpi->lmin=0;
      rgbpi->lmax=rgbpi->lummax;
      do_draw=TRUE;
    break;
  }

//  rgbpi->zx=MAX(rgbpi->zx,1);
//  rgbpi->zy=MAX(rgbpi->zy,1);
  rgbpi->ox=MIN(rgbpi->ox,(1-1./rgbpi->zx)*rgbpi->pwidth);
  rgbpi->oy=MIN(rgbpi->oy,(1-1./rgbpi->zy)*rgbpi->pheight);
  rgbpi->ox=MAX(rgbpi->ox,0);
  rgbpi->oy=MAX(rgbpi->oy,0);
  if ((strchr("fior",keyval)) && (rgbpi->do_shape))
  {
    makeit_square(w,h,rgbpi);
  }

  if ((rgbpi->zx<1.) && (rgbpi->zy<1.))
  {
    if (rgbpi->zx<rgbpi->zy)
    {
      rgbpi->zx*=(1./rgbpi->zy);
      rgbpi->zy=1.;
    }
    else
    {
      rgbpi->zy*=(1./rgbpi->zx);
      rgbpi->zx=1.;
    }
  }
  
  if (rgbpi->keymousefunc)
    rgbpi->keymousefunc(widget,keyval,0,0,0,0,0,0.,0.);

  if ((do_draw) || (do_update))
  {
    if (GTK_IS_SCROLLED_WINDOW(canvas))
    {
      if (do_draw) /* only redraw if no arrow keys pressed */
        gtk_drawing_area_size(GTK_DRAWING_AREA(drawing_area),w*rgbpi->zx,h*rgbpi->zy);
      g_main_iteration(FALSE);
      gtk_adjustment_set_value(hadj,rgbpi->ox*w*rgbpi->zx/rgbpi->pwidth);
      gtk_adjustment_set_value(vadj,rgbpi->oy*h*rgbpi->zy/rgbpi->pheight);
      gtk_adjustment_changed(hadj);
    }
    else
    {
      float zx=rgbpi->zx;  // Set_Scroll makes zx >= 1; ignore this!
      float zy=rgbpi->zy;
//      Set_Scroll(window,LAB_HBAR,rgbpi->pwidth,&rgbpi->ox,&zx,
//                        LAB_VBAR,rgbpi->pheight,&rgbpi->oy,&zy);
      Set_Scrollc((GtkWidget *)hadj,rgbpi->pwidth,&rgbpi->ox,&zx,
                  (GtkWidget *)vadj,rgbpi->pheight,&rgbpi->oy,&zy);

      Update_RGBPic(rgbpi);
    }
    
/* Volgende voor windhoos; forceer adjustment */
    #if __GTK_WIN32__
    if ((do_draw) && (GTK_IS_SCROLLED_WINDOW(canvas)))
    {
      g_main_iteration(FALSE);
      gtk_adjustment_changed(vadj);
      g_main_iteration(FALSE);
      {
        int ww,hh;
        gdk_window_get_size(window->window,&ww,&hh);
        gdk_window_resize(window->window,ww-1,hh-1);
        g_main_iteration(FALSE);
        gdk_window_resize(window->window,ww,hh);
      }
    }
    #endif
  }

  return FALSE;
}

static gboolean RGB_Pic_keyfunc(GtkWidget *widget, GdkEventKey *event,GtkWidget *extra)
{
  return RGB_Pic_keyfunc1(widget,NULL, event,extra);
}

void RGB_Pic_keyfunc_ext(GtkWidget *widget, char sig)
{
  GdkEventKey event;
  event.keyval=sig;
  RGB_Pic_keyfunc(widget,&event,NULL);
}

void keymousefunc(GtkWidget *,int ,int ,int ,int ,int ,int ,float ,float );
/*************************************
 * Mouse handling
 *************************************/
static void RGB_Pic_mousefunc(GtkWidget *widget, GdkEventMotion *event)
{
  RGBI *rgbi;
  RGBPICINFO *rgbpi;
//  GtkWidget *window=Find_Parent_Window(widget);
  GdkModifierType state;
  int x,y;

  rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)widget,RGBPI_DL);
  rgbi=Get_RGBI(widget);  // widget=drawable

  if (event->is_hint)
  {
    gdk_window_get_pointer(event->window,&x,&y,&state);
  }
  else
  {
    x=event->x;
    y=event->y;
    state=event->state;
  }
//  rgbpi->zx=MAX(rgbpi->zx,1);
//  rgbpi->zy=MAX(rgbpi->zy,1);

  {
    float dx,dy;
    int px,py;

    if (rgbpi->size_drawable_limit)
    {
      dx=(float)(rgbpi->pwidth)/(float)rgbi->width/rgbpi->zx;
      dy=(float)(rgbpi->pheight)/(float)rgbi->height/rgbpi->zy;
      px=x*dx+rgbpi->ox;
      py=y*dy+rgbpi->oy;
    }
    else
    {
      dx=(float)(rgbpi->pwidth)/rgbi->width;
      dy=(float)(rgbpi->pheight)/rgbi->height;
      px=x*dx;           /* (px,py)=actual selected pixel in picture */
      py=y*dy;
    }
mx=px; my=py;
  if (rgbpi->keymousefunc)
    rgbpi->keymousefunc(widget,0,state,x,y,px,py,dx,dy);
  }
}

GtkWidget *Create_Canvas_RGBPic_dr(GtkWidget *widget,
                                   int pw,int ph,int pd,
                                   int drawfunc(),
                                   void keymousefunc())
{
  GtkWidget *canvas;
  RGBPICINFO *rgbpicinfo;
  rgbpicinfo=calloc(1,sizeof(*rgbpicinfo));

  rgbpicinfo->widget=widget;
  rgbpicinfo->draw_func=drawfunc;
  rgbpicinfo->pwidth=pw;
  rgbpicinfo->pheight=ph;
  rgbpicinfo->str8=NULL;
  rgbpicinfo->str16=NULL;
  rgbpicinfo->bpp=pd;
  rgbpicinfo->lummax=(1<<rgbpicinfo->bpp)-1;
  rgbpicinfo->lmin=0;
  rgbpicinfo->lmax=rgbpicinfo->lummax;   /* 255 */
  rgbpicinfo->keymousefunc=(void *)keymousefunc;
  rgbpicinfo->pixel_shape=1.;
  gtk_object_set_data((gpointer)widget,RGBPI_DL,(gpointer)rgbpicinfo);
  canvas=Create_Canvas(widget,pw,ph,configure_RGBPICevent,
                       RGB_Pic_keyfunc,RGB_Pic_mousefunc,RGB_Pic_mousefunc);
  gtk_object_set_data(GTK_OBJECT(canvas),WDGT_ID,CNVS_ID);

  return canvas;
}

static void scroll_actions(GtkWidget *adjust,     // is adjustment widget
                           gpointer data)
{
  GtkWidget *canvas,*scrol,*drawable;
  RGBPICINFO *rgbpicinfo;
  char *name=(char *)data;
  float value,lower,upper;

  scrol=gtk_object_get_data((gpointer)adjust,PARENT_ID);
  canvas=gtk_object_get_data((gpointer)scrol,PARENT_ID);
  drawable=gtk_object_get_data((gpointer)canvas,CHILD_ID);

  rgbpicinfo=(RGBPICINFO *)gtk_object_get_data((gpointer)drawable,RGBPI_DL);

  if (!rgbpicinfo) return;
  
  value=GTK_ADJUSTMENT(adjust)->value;
  upper=GTK_ADJUSTMENT(adjust)->upper;
  lower=GTK_ADJUSTMENT(adjust)->lower;
  if (!strcmp(name,LAB_HBAR))
  {
    rgbpicinfo->ox=value-lower;
  }
  if (!strcmp(name,LAB_VBAR))
  {
    rgbpicinfo->oy=value-lower;
  }
  Update_RGBPic(rgbpicinfo);
}


GtkWidget *Create_Canvas1_RGBPic_dr2(GtkWidget *widget,
                                   int pw,int ph,int pd,
                                   char *id,
                                   int drawfunc(),
                                   void keymousefunc())
{
  GtkWidget *canvas,*canvas1;
  RGBPICINFO *rgbpicinfo;
  rgbpicinfo=calloc(1,sizeof(*rgbpicinfo));
  rgbpicinfo->draw_func=drawfunc;
  rgbpicinfo->pwidth=pw;
  rgbpicinfo->pheight=ph;
  rgbpicinfo->str8=NULL;
  rgbpicinfo->str16=NULL;
  rgbpicinfo->bpp=pd;
  rgbpicinfo->lummax=(1<<rgbpicinfo->bpp)-1;
  rgbpicinfo->lmin=0;
  rgbpicinfo->lmax=rgbpicinfo->lummax;   /* 255 */
  rgbpicinfo->keymousefunc=(void *)keymousefunc;
  rgbpicinfo->pixel_shape=1.;
  rgbpicinfo->size_drawable_limit=TRUE;
  canvas1=Create_Canvas(widget,0,0,configure_RGBPICevent,
                       RGB_Pic_keyfunc,RGB_Pic_mousefunc,RGB_Pic_mousefunc);
  rgbpicinfo->widget=canvas1;
  // canvas1=drawable because w/h args in Create_Canvas are 0!
  gtk_object_set_data((gpointer)canvas1,RGBPI_DL,(gpointer)rgbpicinfo);

  canvas=Add_Scrollbars(canvas1,LAB_HBAR,LAB_VBAR,scroll_actions);
  
  rgbpicinfo->zx=rgbpicinfo->zy=1.;
  gtk_object_set_data(GTK_OBJECT(canvas),WDGT_ID,CNVS_ID);

  return canvas;
}

GtkWidget *Create_Canvas_RGBPic_dr2(GtkWidget *widget,
                                   int pw,int ph,int pd,
                                   int drawfunc(),
                                   void keymousefunc())
{
  return Create_Canvas1_RGBPic_dr2(widget,pw, ph, pd,NULL,
                                    drawfunc,keymousefunc);
}

GtkWidget *Create_Canvas_RGBPic_1b(GtkWidget *widget,
                                   int pw,int ph,
                                   char *str,
                                   gboolean fixed_drawable,
                                   gboolean anal_lum,
                                   void xdrawfunc(),
                                   void keymousefunc())
{
  GtkWidget *canvas,*drawable;
  RGBPICINFO *rgbpicinfo;
  if (fixed_drawable)
    canvas=Create_Canvas_RGBPic_dr2(widget,pw,ph,8,RGB_Pic_drawfunc,keymousefunc);
  else
    canvas=Create_Canvas_RGBPic_dr(widget,pw,ph,8,RGB_Pic_drawfunc,keymousefunc);

  drawable=gtk_object_get_data((gpointer)canvas,CHILD_ID);
  rgbpicinfo=(RGBPICINFO *)gtk_object_get_data((gpointer)drawable,RGBPI_DL);

  rgbpicinfo->str8=(guchar *)str;
  rgbpicinfo->xdrawfunc=xdrawfunc;
  rgbpicinfo->anal_lum=anal_lum;

  return canvas;
}


/*************************************
 * Create and draw, 1 byte per pixel
 *************************************/
GtkWidget *Create_RGB_1bPic_b(GtkWidget *widget,
                            char *name,
                            int ww,int wh,
                            int pw,int ph,
                            char *str,
                            gboolean fixed_drawable,
                            gboolean anal_lum,
                            void xdrawfunc(),
                            void keymousefunc(),
                            GtkWidget *extra_buts,
                            void close_func())
{
  GtkWidget *wnd,*canvas=NULL,*w[3];
  wnd=Find_Parent_Window(widget);
  if (close_func)
    wnd=Create_Window(wnd,ww,wh,name,close_func);
  else
    wnd=Create_Window(wnd,ww,wh,name,Close_RGBPic);
  if (!wnd) return NULL;

  canvas=Create_Canvas_RGBPic_1b(wnd,pw,ph,str,fixed_drawable,anal_lum,xdrawfunc,keymousefunc);

  w[1]=Create_Button("Dismiss",Close_Window);
  if (extra_buts) w[1]=Pack(NULL,'h',w[1],1,extra_buts,1,NULL);
  w[0]=SPack(NULL,"v",canvas,"ef1",w[1],"1",NULL);
  gtk_container_add(GTK_CONTAINER(wnd),w[0]);

  gtk_widget_show_all(wnd);
  gtk_drawing_area_size(GTK_DRAWING_AREA(Find_Widget(wnd,"GTK_DRAWING_AREA")),0,0);
  Set_Enable_Update(canvas,TRUE);
  return wnd;
}

GtkWidget *Create_RGB_1bPic(GtkWidget *widget,
                            char *name,
                            int ww,int wh,
                            int pw,int ph,
                            char *str,
                            gboolean fixed_drawable,
                            gboolean anal_lum,
                            void xdrawfunc(),
                            void keymousefunc(),
                            void close_func())
{
  return Create_RGB_1bPic_b(widget,name,ww,wh,pw, ph,str,fixed_drawable,anal_lum,
                                xdrawfunc,keymousefunc,NULL,close_func);
}

#ifdef WEG
GtkWidget *Create_Canvas_RGBPic_2b(GtkWidget *widget,
                                   int pw,int ph,
                                   guint16 *str,
                                   gboolean fixed_drawable,
                                   gboolean anal_lum,
                                   void xdrawfunc(),
                                   void keymousefunc())
{
  GtkWidget *canvas,*drawable;
  RGBPICINFO *rgbpicinfo;
  if (fixed_drawable)
    canvas=Create_Canvas_RGBPic_dr2(widget,pw,ph,10,RGB_Pic_drawfunc,keymousefunc);
  else
    canvas=Create_Canvas_RGBPic_dr(widget,pw,ph,10,RGB_Pic_drawfunc,keymousefunc);
  rgbpicinfo=(RGBPICINFO *)gtk_object_get_data((gpointer)widget,RGBPI_DL);
  rgbpicinfo->str16=str;
  rgbpicinfo->xdrawfunc=xdrawfunc;
  rgbpicinfo->anal_lum=anal_lum;
  drawable=gtk_object_get_data(GTK_OBJECT(canvas),CHILD_ID);
  Set_Enable_Update(canvas,TRUE);      // Enable drawing pic.
  if (!fixed_drawable) Set_Scrollbars(widget,rgbpicinfo);
  

  return canvas;
}

/*************************************
 * Create and draw, 2 bytes per pixel
 *************************************/
GtkWidget *Create_RGB_2bPic(GtkWidget *widget,
                            char *name,
                            int ww,int wh,
                            int pw,int ph,
                            guint16 *str,
                            gboolean anal_lum,
                            void keymousefunc(),
                            void close_func())
{
  GtkWidget *wnd,*canvas=NULL,*w[3];
  wnd=Find_Parent_Window(widget);
  if (close_func)
    wnd=Create_Window(wnd,ww,wh,name,close_func);
  else
    wnd=Create_Window(wnd,ww,wh,name,Close_RGBPic);

  if (!wnd) return NULL;

  canvas=Create_Canvas_RGBPic_2b(wnd,pw,ph,str,FALSE,anal_lum,NULL,keymousefunc);

  w[1]=Create_Button("Dismiss",Close_Window);
  w[0]=Pack(NULL,'v',canvas,1,w[1],1,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),w[0]);
  gtk_widget_show_all(wnd);
  gtk_drawing_area_size(GTK_DRAWING_AREA(Find_Widget(wnd,"GTK_DRAWING_AREA")),0,0);
  Set_Enable_Update(wnd,TRUE);

  return wnd;
}
#endif

