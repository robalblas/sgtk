/**************************************************
 * RCSId: $Id: sgtk_misc.c,v 1.2 2017/01/29 12:58:13 ralblas Exp $
 *
 * Some help functions.
 * Project: SGTK
 * Author: R. Alblas
 *
 * History: 
 * $Log: sgtk_misc.c,v $
 * Revision 1.2  2017/01/29 12:58:13  ralblas
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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "sgtk.h"

/*************************************
 *************************************/

#define rewind_s(str) while ((str) && (str->prev)) str=str->prev

/*************************************
 * Find widget in a window or menu with certain user_data
 * E.g. buttons and certain containers can be found.
 * For widget pairs (e.g. label + entry) then:
 *  top=FALSE ==> Return widget containing state (e.g. the entry widget)
 *  top=TRUE  ==> Return top-widget (e.g. containing both label and entry)
 *************************************/
GtkWidget *Find_Widget1(GtkWidget *wdgt,     /* window widget to search in */
                        char      *id,       /* ID of widget to find */
                        gboolean  top)       /* true if top of group */
{
  GtkWidget *cur_wdgt,*cur_wdgt2;
  char *cur_wdgt_id;
  GList *node;
  GList *List_Childs;

  if (!wdgt) return NULL;
/* Create a list of widget in current container */
  List_Childs = gtk_container_children((GtkContainer *)wdgt);

/* Loop through all these widgets */
  for (node = List_Childs; node != NULL; node = node->next)
  {
/* Get current widget and its ID */
    cur_wdgt = (GtkWidget *)node->data;
    cur_wdgt_id=NULL;
    if (top)
      cur_wdgt_id = (char *)gtk_object_get_data(GTK_OBJECT(cur_wdgt),TWDGT_ID);
    if (!cur_wdgt_id)
      cur_wdgt_id = (char *)gtk_object_get_data(GTK_OBJECT(cur_wdgt),WDGT_ID);

/* === CHECK BUTTONS === */
/* If we have a id, compare it with requested id */
    if (cur_wdgt_id)
    {
      if (!id)            /* for debug only: list all buttons */
        printf("ID=0! Debug-mode! name=%s (%x)\n",cur_wdgt_id,(int)cur_wdgt);
      else
      {
/* Return if found! */
        if (!strcmp(id, cur_wdgt_id))
          break;
      }
    }

/* === CHECK DRAWING_AREA === */
/* If requested widget is a drawing_area return if found */
    if ((GTK_IS_DRAWING_AREA(cur_wdgt)) && (id) &&
              (!strcmp(id, "GTK_DRAWING_AREA")))
      break;

/* === CHECK VBOX === */
/* If requested widget is a v_box return if found */
    if ((GTK_IS_VBOX(cur_wdgt)) && (id) &&
              (!strcmp(id, "GTK_VBOX")))
      break;

/* === CHECK HBOX === */
/* If requested widget is a h_box return if found */
    if ((GTK_IS_HBOX(cur_wdgt)) && (id) &&
              (!strcmp(id, "GTK_HBOX")))
      break;

/* === CHECK ANY BOX === */
/* If requested widget is a h_box or v_box return if found */
    if (((GTK_IS_HBOX(cur_wdgt)) || (GTK_IS_VBOX(cur_wdgt))) && (id) &&
              (!strcmp(id, "GTK_BOX")))
      break;

/* === CHECK CONTAINER === */
/* If requested widget is a container then search through all its children  */
    if (GTK_IS_CONTAINER(cur_wdgt))
    {
      if ((cur_wdgt2=Find_Widget1(cur_wdgt,id,top)))
      {
        cur_wdgt=cur_wdgt2;
        break;
      }
    }

/* === CHECK MENU ITEMS === */
    if ((cur_wdgt) && (cur_wdgt_id))
    {
      if (GTK_IS_MENU_ITEM(cur_wdgt))
      {
        if ((cur_wdgt2=GTK_MENU_ITEM(cur_wdgt)->submenu))
        {
          if (GTK_IS_CONTAINER(cur_wdgt2))
          {
            if ((cur_wdgt2=Find_Widget1(cur_wdgt2,id,top)))
            {
              cur_wdgt=cur_wdgt2;
              break;
            }
          }
        }
      }

/* === CHECK OPTION MENU ITEMS === */
      if ((id) && (GTK_IS_COMBO_BOX(cur_wdgt)) && (!strcmp(id, cur_wdgt_id)))
        break;
    }
  }

  g_list_free(List_Childs);
  if (node==NULL)
    return NULL;
  else
    return cur_wdgt;
}


/*************************************
 * Find local widget in a container with certain user_data
 * E.g. buttons and certain containers can be found
 *************************************/
GtkWidget *Find_Local_Widget(GtkWidget    *wdgt, /* container to search in */
                             char         *id)   /* ID of widget to find */
{
  return Find_Widget1(wdgt,id,FALSE);
}

/*************************************
 * Find widget in a window or menu with certain user_data
 * E.g. buttons and certain containers can be found
 *************************************/
GtkWidget *Find_Widget(GtkWidget    *wdgt,      /* window widget to search in */
                       char         *id)        /* ID of widget to find */
{
  if ((id) && (*id=='/'))        /* find widget from this level on */
    return Find_Widget1(wdgt,id+1,FALSE);
  else                 /* find widget from top level (=in this window) */
    return Find_Widget1(Find_Parent_Window(wdgt),id,FALSE);
}

/*************************************
 * Find widget in a window or menu
 * If it is a pair (e.g. label/entry) return the top-widget
 *************************************/
GtkWidget *Find_Top_Widget(GtkWidget *wdgt,  /* window widget to search in */
                           char      *id)    /* ID of widget to find */
{
  return Find_Widget1(wdgt,id,TRUE);
}



/*************************************
 * Find window in which the widget exist.
 * Finds also window of menu/submenu etc. widgets.
 *************************************/
GtkWidget *Find_Parent_Window(GtkWidget *widget)
{
  GtkWidget *wnd;
  if (!widget) return NULL;

/* if w = adjust-widget then first get widget connected to it. 
   (Adjustment widget can not be used to get window directly.) 
*/
  if (GTK_IS_ADJUSTMENT(widget))
    widget = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(widget),ADJUST_ID);

  while (GTK_IS_MENU_ITEM(widget))
  {
    if (GTK_IS_MENU(widget->parent))
      widget=gtk_menu_get_attach_widget((GtkMenu *)widget->parent);
    else
      break;
  }
  if (GTK_IS_WIDGET(widget))
    wnd=gtk_widget_get_toplevel(widget);
  else
    wnd=NULL;
  return wnd;
}



int Remove_Widget(GtkWidget *w,char *name)
{
  if ((w=Find_Top_Widget(w,name)))
  {
    gtk_widget_destroy(w);
    return 1;
  }
  return 0;
}

int Add_Widget(GtkWidget *w,GtkWidget *w_add,char *name,char *option)
{
  GtkWidget *box;
  gboolean fill=FALSE,expand=FALSE;
  int padding=0;
  char *p;
  if (option)
  {
    if (strchr(option,'f')) fill  =TRUE; else fill  =FALSE;
    if (strchr(option,'e')) expand=TRUE; else expand=FALSE;
    for (p=option; *p & (!isdigit((int)*p)); p++);
    if (p) padding=atoi(p);
  }

  box=w;
  if ((!name) || (box=Find_Widget(w,name)))
  {
    gtk_box_pack_start(GTK_BOX(box), w_add, fill,expand, padding);
    return 1;
  }
  return 0;
}

/*************************************
 * Deselect all items. 
 * If wid=NULL: window=window in which clist is.
 *************************************/
void Deselect_Clist(GtkWidget *window,char *wid,char *lid)
{
  GtkWidget *clist;
  if (wid)
  {
    window=Find_Window(window,wid);
    clist=Find_Widget(window,lid);
  }
  else
  {
    clist=Find_Widget(window,lid);
  }
  if (!clist) return;
  gtk_clist_unselect_all(GTK_CLIST(clist));
}

/*************************************
 * Add label to e.g. entry or spin button. 
 * If name starts with '!' -> No label gnerated.
 * If name starts with '^' -> label placed above button
 * Default: label placed left of button
 *************************************/
GtkWidget *add_label(char *name)
{
  GtkWidget *tbut=NULL;
  GtkWidget *label;
  if (*name != '!')
  {
/* Name and place Label */
    if (*name=='^')
    {
      tbut = gtk_vbox_new(FALSE, 0);
      label = gtk_label_new(name+1);
      gtk_misc_set_alignment (GTK_MISC (label), 0., 0.);
      gtk_box_pack_start(GTK_BOX(tbut), label, FALSE, FALSE, 1);
      gtk_widget_show(label);
    }
    else
    {
      tbut = gtk_hbox_new(FALSE, 0);
      label = gtk_label_new(name);
      gtk_misc_set_alignment (GTK_MISC (label), 1., 0.5);
      gtk_box_pack_start(GTK_BOX(tbut), label, FALSE, FALSE, 10);
      gtk_widget_show(label);
    }
  }
  return tbut;
}

char *get_wndtitle(GtkWidget *window)
{
  WINDOW_DATA *wnd_data;
  wnd_data=gtk_object_get_data(GTK_OBJECT(window),WNDINFO);
  if (!wnd_data) return NULL;
  return wnd_data->title;
}

/*************************************
 * Find window 
 * window  : A gtk_window somewhere in the chain
 * title: Title of window to find
 *************************************/
GtkWidget *Find_Window(GtkWidget *window,
                       char      *title)
{
  WINDOW_DATA *wnd_data;

/* In case window is actually a widget: Get its window */
  if (!(window=Find_Parent_Window(window))) return NULL;

  wnd_data=gtk_object_get_data(GTK_OBJECT(window),WNDINFO);
  if (!wnd_data) return NULL;

  while (wnd_data->prev) wnd_data=wnd_data->prev;

  while ((wnd_data) && ((!title) || (strcmp(wnd_data->title,title))))
  {
    if (!title) puts(wnd_data->title);
    wnd_data=wnd_data->next;
  }
  if (!wnd_data) return NULL;

  return wnd_data->window;
}

/*************************************
 * Find first window 
 * wnd  : A gtk_window somewhere in the chain
 *************************************/
GtkWidget *First_window(GtkWidget *wnd)
{
  WINDOW_DATA *wnd_data;
  if (!wnd) return NULL;
  wnd_data=gtk_object_get_data(GTK_OBJECT(wnd),WNDINFO);
  if (!wnd_data) return NULL;
  while (wnd_data->prev) wnd_data=wnd_data->prev;
  return wnd_data->window;
}

/*************************************
 * Find next window 
 * wnd  : A gtk_window somewhere in the chain
 *************************************/
GtkWidget *Next_window(GtkWidget *wnd)
{
  WINDOW_DATA *wnd_data;
  if (!wnd) return NULL;
  wnd_data=gtk_object_get_data(GTK_OBJECT(wnd),WNDINFO);
  if (!wnd_data) return NULL;
  if (!wnd_data->next) return NULL;
  
  return wnd_data->next->window;
}

/*************************************
 * Find widget in current window 
 *************************************/
GtkWidget *Find_cw_Widget(GtkWidget    *wdgt,      /* window widget to search in */
                          char         *id)        /* ID of widget to find */
{
  return Find_Widget(wdgt,id);
}

/*************************************
 * Find widget in any window, starting with current
 *************************************/
GtkWidget *Find_aw_Widget(GtkWidget    *iwdgt,     /* window widget to search in */
                          char         *id)        /* ID of widget to find */
{
  GtkWidget *wdgt,*wnd;
  if ((wdgt=Find_Widget(iwdgt,id))) return wdgt;
  wnd=Find_Parent_Window(iwdgt);
  wnd=First_window(wnd);
  while (wnd)
  {
    if ((wdgt=Find_Widget(wnd,id))) return wdgt;
    wnd=Next_window(wnd);
  }
  return NULL;    
}

/*************************************
 * Find widget in certain window, starting with current
 *************************************/
GtkWidget *Find_iw_Widget(GtkWidget    *iwdgt,     /* window widget to search in */
                          char         *idw,       /* ID of window to find */
                          char         *idb)       /* ID of button to find */
{
  return Find_cw_Widget(Find_Window(Find_Parent_Window(iwdgt),idw),idb);  
}

/*************************************
 * Toggle between maximum and original window size
 *************************************/
void window_toggle_size(GtkWidget *widget)
{
  GdkWindow *window=widget->window;
  WINDOW_DATA *wnd_data=gtk_object_get_data(GTK_OBJECT(widget),WNDINFO);
  static int x,y,w,h;
  static gboolean first_time=TRUE;
  static gboolean use_deskrelative_origin=FALSE;

/* Test which get_position routine to use 
   (differs from one desktop to another) 
*/
  if (first_time)
  {
    gint x1, y1, x2, y2;
    gdk_window_get_root_origin(window,&x1,&y1);
    gdk_window_get_root_origin(window,&x1,&y1);
    gdk_window_move(window,x1,y1);
    gdk_window_get_root_origin(window,&x2,&y2);
    gdk_window_get_root_origin(window,&x2,&y2);
    if ((x1 != x2) || (y1 != x2))
    {
      use_deskrelative_origin=TRUE;
      gdk_window_move(window,x1+x1-x2,y1+y1-y2);
    }

    first_time=FALSE;
  }
  if (wnd_data->maximized)
  {
    gdk_window_move_resize(window,x,y,w,h);
    /* set_decoration doesn't work in win32 */
    #if __GTK_WIN32__ == 0
      gdk_window_set_decorations(window,GDK_DECOR_MAXIMIZE);
    #endif
    wnd_data->maximized=FALSE;
  }
  else
  {
    int bw,bh;

    if (use_deskrelative_origin)
    {
      gdk_window_get_deskrelative_origin(window,&x,&y);
      gdk_window_get_size(window,&w,&h);
      window_get_border_size(window,&bw,&bh);
      bw+=10;
      bh+=25;
      gdk_window_move_resize(window,5,20,gdk_screen_width()-bw,
                                         gdk_screen_height()-bh);
    }
    else
    {
      gdk_window_get_root_origin(window,&x,&y);
      gdk_window_get_size(window,&w,&h);
      window_get_border_size(window,&bw,&bh);
      gdk_window_move_resize(window,0,0,gdk_screen_width()-bw,
                                        gdk_screen_height()-bh);
      gdk_window_set_decorations(window, 0);
    }
    wnd_data->maximized=TRUE;
  }
}

/*************************************
 * Get total window size, inclusive decorations.
 * (No GTK function for this?)
 *************************************/
void window_get_border_size(GdkWindow *window,int *w,int *h)
{
  int x1,x2,y1,y2;

  gdk_window_get_root_origin(window,&x1,&y1);
/* For some reasson sometimes 2x gdk_window_get_root_origin is needed
   to get the real root_origin. (gtk-bug??)
*/
  gdk_window_get_root_origin(window,&x1,&y1); 
  gdk_window_get_origin(window,&x2,&y2);
  *w=x2-x1;
  *h=y2-y1;

/* Add right and bottom border.
  (Not very clean; assumed: right border = bottom border = left border
*/
  *h+=*w;
  (*w)*=2;
}

/*************************************
 * Get total window size, inclusive decorations.
 * (No GTK function for this?)
 *************************************/
void window_get_root_size(GdkWindow *window,int *w,int *h)
{
  int bw,bh;

  window_get_border_size(window,&bw,&bh); 
  gdk_window_get_size(window,w,h);
  *w=*w+bw;
  *h=*h+bh;
}


/*************************************
 * Get pixmap 
 * window  : A gtk_window somewhere in the chain
 *************************************/
GdkPixmap *Get_Pixmap(GtkWidget *window)
{
  WINDOW_DATA *wnd;
  if (window==NULL) return NULL;

  wnd=gtk_object_get_data(GTK_OBJECT(window),WNDINFO);
  if (wnd==NULL) return NULL;

  return wnd->pixmap;
}

/*************************************
 * Put pixmap 
 * window  : A gtk_window somewhere in the chain
 *************************************/
void Put_Pixmap(GtkWidget *window,       /* Add pixmap to this window */
                GdkPixmap *pixmap)       /* pixmap to add */
{
  WINDOW_DATA *wnd=gtk_object_get_data(GTK_OBJECT(window),WNDINFO);
  wnd->pixmap=pixmap;
}

/*************************************
 * Get RGB info from drawable
 *************************************/
RGBI *Get_RGBI(GtkWidget *drawable)
{
  RGBI *rgbi;
  if (!drawable) return NULL;

  rgbi=(RGBI *)gtk_object_get_data(GTK_OBJECT(drawable),DRAWABLE_INFO);
  return rgbi;
}

RGBPICINFO *Get_RGBPI(GtkWidget *drawable)
{
  RGBPICINFO *rgbpi;
  if (!drawable) return NULL;

  rgbpi=(RGBPICINFO *)gtk_object_get_data((gpointer)drawable,RGBPI_DL);
  return rgbpi;
}


/*************************************
 * Get pointer to GC  
 * window  : A gtk_window
 *************************************/
GdkGC **Get_Gc(GtkWidget *window)
{
  WINDOW_DATA *wnd;
  if (!window) return NULL;
  wnd=gtk_object_get_data(GTK_OBJECT(window),WNDINFO);
  return wnd->gc;
}

/*************************************
 * Set flag to enable update  to 'true' or 'false'
 *************************************/
void Set_Enable_Update(GtkWidget *widget,gboolean state)
{
  GtkWidget *drawable;
  RGBI *rgbi;
  if (!(drawable=gtk_object_get_data((gpointer)widget,CHILD_ID)))
    drawable=widget;

  if ((rgbi=Get_RGBI(drawable))) rgbi->enable_update=state;
}

/*************************************
 * Get state of 'enable update' flag
 *************************************/
gboolean Get_Enable_Update(GtkWidget *widget)
{
  gboolean state=FALSE;
  GtkWidget *drawable;
  RGBI *rgbi;
  if (!(drawable=gtk_object_get_data((gpointer)widget,CHILD_ID)))
    drawable=widget;

  if ((rgbi=Get_RGBI(drawable))) state=rgbi->enable_update;

  return state;
}

/*************************************
 * Create/change a cursor.
 * Doesn't work fine under Win-XP, so removed for Windows!
 * (messages gdk_cursor_destroy)
 * To be fixed...
 *************************************/
GdkCursor *Create_Cursor(GtkWidget *window,int type)
{
  static GdkCursor *cursor;
//#if __GTK_WIN32__ != 1
  if (!window) return NULL;
  if (!window->window) return NULL;
  if (cursor!=NULL) gdk_cursor_destroy(cursor);
  cursor=gdk_cursor_new(type);
  gdk_window_set_cursor(window->window,cursor);
//#endif
  return cursor;
}

/************widget coloring using rc (doesn't work with 'newer' gtk2) */
/*************************************
 * Create a rcstyle for widgets
 * Return: The rcstyle.
 *************************************/
static GtkRcStyle *Create_RcStyle(GtkRcFlags flag,    /*GTK_RC_xx, xx=FG,BG,TEXT,BASE*/
                           GdkColor *normal,     /* normal state color or NULL*/
                           GdkColor *prelight,   /* prelight color or NULL */
                           GdkColor *active,     /* active color or NULL */
                           GdkColor *selected,   /* selected color or NULL */
                           GdkColor *insensitive)/* insensitive color or NULL */
{
  GtkRcStyle *rc_style;
  rc_style=gtk_rc_style_new();
  if (normal)
  {
    switch(flag)
    {
      case GTK_RC_BG:
        rc_style->bg[GTK_STATE_NORMAL]=*normal;
      break;
      case GTK_RC_FG:
        rc_style->fg[GTK_STATE_NORMAL]=*normal;
      break;
      case GTK_RC_TEXT:
        rc_style->text[GTK_STATE_NORMAL]=*normal;
      break;
      case GTK_RC_BASE:
        rc_style->base[GTK_STATE_NORMAL]=*normal;
      break;
    }
    rc_style->color_flags[GTK_STATE_NORMAL] |= flag;

  }
  if (prelight)
  {
    switch(flag)
    {
      case GTK_RC_BG:
        rc_style->bg[GTK_STATE_PRELIGHT]=*prelight;
      break;
      case GTK_RC_FG:
        rc_style->fg[GTK_STATE_PRELIGHT]=*prelight;
      break;
      case GTK_RC_TEXT:
        rc_style->text[GTK_STATE_PRELIGHT]=*prelight;
      break;
      case GTK_RC_BASE:
        rc_style->base[GTK_STATE_PRELIGHT]=*prelight;
      break;
    }
    rc_style->color_flags[GTK_STATE_PRELIGHT] |= flag;

  }
  if (active)
  {
    switch(flag)
    {
      case GTK_RC_BG:
        rc_style->bg[GTK_STATE_ACTIVE]=*active;
      break;
      case GTK_RC_FG:
        rc_style->fg[GTK_STATE_ACTIVE]=*active;
      break;
      case GTK_RC_TEXT:
        rc_style->text[GTK_STATE_ACTIVE]=*active;
      break;
      case GTK_RC_BASE:
        rc_style->base[GTK_STATE_ACTIVE]=*active;
      break;
    }
    rc_style->color_flags[GTK_STATE_ACTIVE] |= flag;

  }
  if (selected)
  {
    switch(flag)
    {
      case GTK_RC_BG:
        rc_style->bg[GTK_STATE_SELECTED]=*selected;
      break;
      case GTK_RC_FG:
        rc_style->fg[GTK_STATE_SELECTED]=*selected;
      break;
      case GTK_RC_TEXT:
        rc_style->text[GTK_STATE_SELECTED]=*selected;
      break;
      case GTK_RC_BASE:
        rc_style->base[GTK_STATE_SELECTED]=*selected;
      break;
    }
    rc_style->color_flags[GTK_STATE_SELECTED] |= flag;

  }
  if (insensitive)
  {
    switch(flag)
    {
      case GTK_RC_BG:
        rc_style->bg[GTK_STATE_INSENSITIVE]=*insensitive;
      break;
      case GTK_RC_FG:
        rc_style->fg[GTK_STATE_INSENSITIVE]=*insensitive;
      break;
      case GTK_RC_TEXT:
        rc_style->text[GTK_STATE_INSENSITIVE]=*insensitive;
      break;
      case GTK_RC_BASE:
        rc_style->base[GTK_STATE_INSENSITIVE]=*insensitive;
      break;
    }
    rc_style->color_flags[GTK_STATE_INSENSITIVE] |= flag;

  }
  return rc_style;
}


void Set_Widgetcolor1(GtkWidget *widget,char fb,int r,int g,int b)
{
  GdkColor clr;
  clr.red=(r<<8)+r;
  clr.green=(g<<8)+g;
  clr.blue=(b<<8)+b;
  switch(fb)
  {
    case 'b':
      Set_Color(widget,GTK_RC_BG,&clr,&clr,&clr,&clr,&clr);
    break;
    case 'B':
      Set_Color(widget,GTK_RC_BASE,&clr,&clr,&clr,&clr,&clr);
    break;
    case 't':
      Set_Color(widget,GTK_RC_TEXT,&clr,&clr,&clr,&clr,&clr);
    break;
    default:
      Set_Color(widget,GTK_RC_FG,&clr,&clr,&clr,&clr,&clr);
    break;
  }
}

// backfground buttons, also take into account 'active'/'selected'
void Set_Buttoncolor1(GtkWidget *widget,int rgb)
{
  GdkColor clr[2];
  int r,g,b;
  r=(rgb>>8)&0xf;
  g=(rgb>>4)&0xf;
  b=(rgb>>0)&0xf;
  clr[0].red=(r<<12)+(r<<8)+(r<<4)+r;
  clr[0].green=(g<<12)+(g<<8)+(g<<4)+g;
  clr[0].blue=(b<<12)+(b<<8)+(b<<4)+b;

  r-=0x2; if (r<0) r=0;
  g-=0x2; if (g<0) g=0;
  b-=0x2; if (b<0) b=0;
  clr[1].red=(r<<12)+(r<<8)+(r<<4)+r;
  clr[1].green=(g<<12)+(g<<8)+(g<<4)+g;
  clr[1].blue=(b<<12)+(b<<8)+(b<<4)+b;
  Set_Color(widget,GTK_RC_BG,&clr[0],&clr[0],&clr[1],&clr[1],&clr[0]);
}
/*************************************
 * Add rcstyle to widget.
 *************************************/
void Set_Color(GtkWidget *widget,GtkRcFlags flag,
               GdkColor *normal,
               GdkColor *prelight,
               GdkColor *active,
               GdkColor *selected, 
               GdkColor *insensitive)
{
  GtkRcStyle *rc_style;
  if (!widget) return;
  rc_style=Create_RcStyle(flag,normal,prelight,active,
                                           selected,insensitive);
  gtk_widget_modify_style(widget,rc_style);

  gtk_rc_style_unref(rc_style);
}

/************widget coloring using style (also works with 'newer' gtk2) */
static void set_clr_style(GtkStyle *style,int fb,int type,GdkColor *clr)
{
  if (!clr) return;
if (fb==GTK_RC_BG) fb='b';
if (fb==GTK_RC_FG) fb='f';
if (fb==GTK_RC_BASE) fb='B';
if (fb==GTK_RC_TEXT) fb='t';
  switch(fb)
  {
    case 'b':
      style->bg[type]=*clr;
    break;
    case 'f':
      style->fg[type]=*clr;
    break;
    case 't':
      style->text[type]=*clr;
    break;
    case 'B':
      style->base[type]=*clr;
    break;
  }
}

static GtkStyle *Create_Style(int fb,    /* 'b','f','t','B' */
                           GdkColor *normal,     /* normal state color or NULL*/
                           GdkColor *prelight,   /* prelight color or NULL */
                           GdkColor *active,     /* active color or NULL */
                           GdkColor *selected,   /* selected color or NULL */
                           GdkColor *insensitive)/* insensitive color or NULL */
{
  GtkStyle *style;
  style = gtk_style_new();
  set_clr_style(style,fb,GTK_STATE_NORMAL,normal);
  set_clr_style(style,fb,GTK_STATE_PRELIGHT,prelight);
  set_clr_style(style,fb,GTK_STATE_ACTIVE,active);
  set_clr_style(style,fb,GTK_STATE_SELECTED,selected);
  set_clr_style(style,fb,GTK_STATE_INSENSITIVE,insensitive);
  return style;
}

/*************************************
 * Add rcstyle to widget.
 *************************************/
void Set_Color2(GtkWidget *widget,int fb,
               GdkColor *normal,
               GdkColor *prelight,
               GdkColor *active,
               GdkColor *selected, 
               GdkColor *insensitive)
{
  GtkStyle *style=Create_Style(fb,normal,prelight,active,
                                           selected,insensitive);
  if (!widget) return;
  if (!style) return;
  gtk_widget_set_style(widget,style);
  g_object_unref (style);
}

void Set_Widgetcolor(GtkWidget *widget,char fb,int r,int g,int b)
{
  GdkColor clr;
  clr.red=(r<<8)+r;
  clr.green=(g<<8)+g;
  clr.blue=(b<<8)+b;
  Set_Color2(widget,fb,&clr,&clr,&clr,&clr,&clr);
}

// backfground buttons, also take into account 'active'/'selected'
void Set_Buttoncolor(GtkWidget *widget,int rgb)
{
  GdkColor clr[2];
  int r,g,b;
  r=(rgb>>8)&0xf;
  g=(rgb>>4)&0xf;
  b=(rgb>>0)&0xf;
  clr[0].red=(r<<12)+(r<<8)+(r<<4)+r;
  clr[0].green=(g<<12)+(g<<8)+(g<<4)+g;
  clr[0].blue=(b<<12)+(b<<8)+(b<<4)+b;

  r-=0x2; if (r<0) r=0;
  g-=0x2; if (g<0) g=0;
  b-=0x2; if (b<0) b=0;
  clr[1].red=(r<<12)+(r<<8)+(r<<4)+r;
  clr[1].green=(g<<12)+(g<<8)+(g<<4)+g;
  clr[1].blue=(b<<12)+(b<<8)+(b<<4)+b;
  Set_Color2(widget,'b',&clr[0],&clr[0],&clr[1],&clr[1],&clr[0]);
}


/*************************************
 * Change font size
 * >0: size
 * =0: don't change
 * <0: set to default (which is 120?)
 *************************************/
int set_fontsize(int size)
{
  GtkStyle *style;
  GdkFont *font;
  char tmp[1000];
  if (size==0) return 120;
  if (size<0) size=120;
  sprintf(tmp,SGTK_VARFONT,size);
  style=gtk_style_copy(gtk_widget_get_default_style());

  if (!(font=gdk_font_load(tmp))) return 0;
  gtk_style_set_font(style,font);
  return size;
}


GdkFont *get_font(int size)
{
  char tmp[1000];
  sprintf(tmp,SGTK_VARFONT,size);
  return gdk_font_load(tmp);
}

int CheckKeyPressed(int val)
{
  if (gdk_events_pending())
  {
    GdkEvent *event=gdk_event_get();
    if (event)
    {
      if (event->type == GDK_KEY_PRESS)
      {
        if (event->key.keyval==val)
        {
          gdk_event_free(event);
          return 1;
        }
      }
      gdk_event_put(event);   /* If key-press then put event back. */
      gdk_event_free(event);
    }
  }
  return 0;
}

/*************************************
 * Get location of program.
 * Takes care of soft-links (Unix).
 *************************************/
#define BUFFLEN 1024
#define DEFPATH "."
char *get_path(char *iprogname)
{
  char *p,
       *pp,
       *path,
       *prog,
       *totname,
        buffer[BUFFLEN];

  FILE  *fp;
  int status;
  int len_totname;
  
  prog=(char *)g_basename(iprogname);

  if (strrchr(iprogname,DIR_SEPARATOR)) 
  {                               /* path name in progname */
    path=g_dirname(iprogname); 
  }
  else
  {                               /* get path from env. variable */
    if (!(p=(char *)g_getenv("PATH"))) 
    {
      p = DEFPATH;
    }
    path = malloc(strlen(p) + 10);
    if (p[0] == PATH_SEPARATOR)
    {
      sprintf(path, ".%s", p);
    }
    else if ((pp = strstr(p, "::")))
    {
      *pp = 0;
      sprintf(path, "%s%c.%s", p, PATH_SEPARATOR,pp + 1);
    }
    else if (p[strlen(p) - 1] == PATH_SEPARATOR)
    {
      sprintf(path, "%s.", p);
    }
    else
    {
      strcpy(path, p);
    }
    #if __GTK_WIN32__ == 1
      strcat(path,";"); strcat(path,".");
    #endif
  }
  len_totname=strlen(path) + strlen(prog) + 15;
  totname = malloc(len_totname);

  p = strtok(path, PATH_SEPARATOR_STR);
  do
  {
    #if __GTK_WIN32__ == 1
      if ((strlen(prog) > 4) && (!strcmp(prog+strlen(prog)-4,".exe")))
        sprintf(totname, "%s%c%s", p, DIR_SEPARATOR, prog);
      else
        sprintf(totname, "%s%c%s.exe", p, DIR_SEPARATOR, prog);
    #else
      sprintf(totname, "%s%c%s", p, DIR_SEPARATOR, prog);
    #endif
    if ((fp = fopen(totname, "r")))
    {
      fclose(fp);
      break;
    }
  } while ((p = strtok(NULL, PATH_SEPARATOR_STR)));

  if (p)
  {
    #if __GTK_WIN32__ == 0
      status = readlink(totname, buffer, BUFFLEN);
      while (status >= 0)
      {
        buffer[status] = '\0';
        if (strlen(buffer) > len_totname)
        {
          len_totname=strlen(buffer)+10;
          totname=realloc(totname,len_totname);
        }
        strcpy(totname, buffer);
        status = readlink(totname, buffer, BUFFLEN);
      }
    #endif
    free(path);
    path=g_dirname(totname); 
  }
  else
  {
    free(path);
    path = NULL;
  }
  free(totname);
  return path;
}



GUISTATE *Create_Guistate(GUISTATE *g,char *w,char *b,char *s,...)
{
  GUISTATE *gnw;

  gnw=calloc(1,sizeof(GUISTATE));
  if (w)
  {
    gnw->window=malloc(strlen(w)+10);
    strcpy(gnw->window,w);
  }
  if (b)
  {
    gnw->button=malloc(strlen(b)+10);
    strcpy(gnw->button,b);
  }
  if (s)
  {
    va_list arg;
    char str[1000];
    va_start(arg,s);
    vsprintf(str,s,arg);
    va_end(arg);

    gnw->state=malloc(strlen(str)+10);
    strcpy(gnw->state,str);
  }

  if (g)
  {
    while (g->next) g=g->next;
    g->next=gnw;
    gnw->prev=g;
  }

  return gnw;
}

GUISTATE *Remove_Guistate1(GUISTATE *g)
{
  GUISTATE *gp=g->prev;
  if (g->next) g->next->prev=g->prev;
  if (g->prev) g->prev->next=g->next;
  if (!gp)
    gp=g->next;
  else 
    while (gp->prev) gp=gp->prev;

  if (g->window) free(g->window);
  if (g->button) free(g->button);
  if (g->state) free(g->state);
  free(g);
  return gp;
}

void Remove_Guistate(GUISTATE *g)
{
  GUISTATE *gnext;
  for (; g; g=gnext)
  {
    gnext=g->next;
    if (g->window) free(g->window);
    if (g->button) free(g->button);
    if (g->state) free(g->state);
    free(g);
  }
}

/***************************************
 * read gui state from file; store in GUISTATE record
 ***************************************/
GUISTATE *read_guistate(char *file)
{
  char l[1000],*w1,*w2,*w3,*p;
  FILE *fp;
  GUISTATE *gs=NULL;
  if (!(fp=fopen(file,"r"))) return NULL;
  while (fgets(l,1000,fp))
  {
    if ((w1=strchr(l,'#'))) *w1=0;
    w1=strtok(l,"\"");
    w2=strtok(NULL,"\"\n");
    w2=strtok(NULL,"\"\n");
    if ((w2) && (p=strstr(w2,"\\n")))  { *p='\n'; strcpy(p+1,p+2); }
    w3=strtok(NULL,"\"\n");
    w3=strtok(NULL,"\"\n");
    if (!w3) continue;
    gs=Create_Guistate(gs,w1,w2,"%s",w3);
  }
  fclose(fp);
  rewind_s(gs);
  return gs;
}

/***************************************
 * restore gui state from file
 ***************************************/
void restore_guistate_fromfile(GtkWidget *wnd,char *file)
{
  GUISTATE *gs;
  gs=read_guistate(file);
  restore_guistate(wnd,gs);
  Remove_Guistate(gs);
}

void restore_guistate(GtkWidget *wnd,GUISTATE *gs)
{
  GtkWidget *parentwnd=Find_Parent_Window(wnd);
  WINDOW_DATA *wnd_data=gtk_object_get_data(GTK_OBJECT(parentwnd),WNDINFO);
  char *wndtitle;
  GtkWidget *wdgt,*wdgt2;
  char *w1,*w2,*w3;
  if (wnd_data) wndtitle=wnd_data->title; else wndtitle="";
  for (; gs; gs=gs->next)
  {
    w1=gs->window;
    w2=gs->button;
    w3=gs->state;
    if ((w3) && (w1) && (!strcmp(w1,wndtitle)))
    {
      if ((wdgt=Find_Widget(wnd,w2)))
      {
        if (GTK_IS_TOGGLE_BUTTON(wdgt))
        {
	  int pst;
          pst=Get_Button(wnd,w2);
          /* to prevent change other buttons influenced by this one: */
          if (pst!=atoi(w3))       /* only restore if state != state to restore! */
          {
            Set_Button(wnd,w2,atoi(w3));
          }
        }
        else if ((wdgt2=gtk_object_get_data(GTK_OBJECT(wdgt),ADJUST_ID)))
        {
          while ((*w3) && (*w3==' ')) w3++;
          Set_Adjust(wnd,w2,"%f",atof(w3));
        } 
        else if (GTK_IS_ENTRY(wdgt))
        {
//          if (gtk_editable_get_editable(GTK_ENTRY(wdgt)));
          Set_Entry(wnd,w2,"%s",w3);
        }
        else if (GTK_IS_COMBO_BOX(wdgt))
        {
          Set_Optionsmenu(wdgt,w2,atoi(w3));
        }
        else if (GTK_IS_CHECK_MENU_ITEM(wdgt))
        {
          Set_Button(wnd,w2,atoi(w3));
        }
      }
    }
  }
}

/***************************************
 * store gui state into GUISTATE record
 ***************************************/
GUISTATE *store_guistate(GtkWidget *wnd,GtkWidget *wdgt,GUISTATE *gs)
{
  GtkWidget *parentwnd;
  WINDOW_DATA *wnd_data;
  char *wndtitle;
  GtkWidget *cur_wdgt,*cur_wdgt2;
  char *cur_wdgt_id;
  GList *node;
  GList *List_Childs;

// Find_Parent_Window of menu-item doesn't work? So added extra wnd arg.
  if (wnd) parentwnd=wnd; else parentwnd=Find_Parent_Window(wdgt);

  if (!wdgt) return NULL;
  if (!parentwnd) return NULL;
  
  List_Childs = gtk_container_children((GtkContainer *)wdgt);
  wnd_data=gtk_object_get_data(GTK_OBJECT(parentwnd),WNDINFO);
  if (wnd_data)
  {
    wndtitle=wnd_data->title;
  }
  else
  {
    wndtitle="";
  }

  for (node = List_Childs; node != NULL; node = node->next)
  {
    cur_wdgt = (GtkWidget *)node->data;
    if (!cur_wdgt) continue;

    if (GTK_IS_CONTAINER(cur_wdgt))
    {
      gs=store_guistate(wnd,cur_wdgt,gs);
    }

    cur_wdgt_id = (char *)gtk_object_get_data(GTK_OBJECT(cur_wdgt),WDGT_ID);
    if (cur_wdgt_id)
    {
      if (GTK_IS_TOGGLE_BUTTON(cur_wdgt))
        gs=Create_Guistate(gs,wndtitle,cur_wdgt_id,"%d",
               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cur_wdgt)));

      else if ((cur_wdgt2=gtk_object_get_data(GTK_OBJECT(cur_wdgt),ADJUST_ID)))
        gs=Create_Guistate(gs,wndtitle,cur_wdgt_id,"%f",
                  GTK_ADJUSTMENT(cur_wdgt2)->value);

      else if (GTK_IS_ENTRY(cur_wdgt))
        gs=Create_Guistate(gs,wndtitle,cur_wdgt_id,"%s",
                  gtk_entry_get_text(GTK_ENTRY(cur_wdgt)));

      else if (GTK_IS_COMBO_BOX(cur_wdgt))
        gs=Create_Guistate(gs,wndtitle,cur_wdgt_id,"%d",
                  gtk_combo_box_get_active(GTK_COMBO_BOX(cur_wdgt)));

      else if (GTK_IS_CHECK_MENU_ITEM(cur_wdgt))
      {
        gs=Create_Guistate(gs,wndtitle,cur_wdgt_id,"%d",
               (GTK_CHECK_MENU_ITEM(cur_wdgt))->active);
      }
    }
  }
  g_list_free(List_Childs);
  rewind_s(gs);
  return gs;
}

void write_states(GUISTATE *gsi,FILE *fp)
{
  GUISTATE *gs;
  char *p;
  for (gs=gsi; gs; gs=gs->next) 
  {
    fprintf(fp,"\"%s\" ",gs->window);
    fprintf(fp,"\"");
    for (p=gs->button; *p; p++)
      if (*p=='\n') fprintf(fp,"\\n"); else fprintf(fp,"%c",*p);
    fprintf(fp,"\" ");
      
    fprintf(fp,"\"%s\"\n",gs->state);
  }
}

/***************************************
 * Save gui state from GUISTATE record into file
 ***************************************/
int save_guistate(GtkWidget *wnd,GtkWidget *wdgt,char *file,gboolean append)
{
  GUISTATE *gsi;
  FILE *fp;
  char *oflag=(append? "a" : "w");
  gsi=store_guistate(wnd,wdgt,NULL);  // store state of current window

  if (!(fp=fopen(file,oflag))) return 0;
  if (!append)
  {
    fprintf(fp,"###########################################################\n");
    fprintf(fp,"# xrit2pic gui settings\n");
    fprintf(fp,"# Format:\n");
    fprintf(fp,"# \"<name_of_window>\" \"<name_of_button>\" \"<state>\"\n");
    fprintf(fp,"# Everything after a '#' is ignored.\n");
    fprintf(fp,"###########################################################\n");
  }
  
  write_states(gsi,fp);
  fclose(fp);
  Remove_Guistate(gsi);
  return 1;
}


void add_guistate(GUISTATE *gsp,char *file)
{
  FILE *fp;
  GUISTATE *gs,*gs1t,*gs1,*gs2=NULL;
  if (!gsp) return;
  gs1t=read_guistate(file);
  for (gs=gsp; gs; gs=gs->next)
  {
    if (!gs->window) continue;
    for (gs1=gs1t; gs1; gs1=gs1->next)
    {
      if (!gs1->window) continue;
      if (!strcmp(gs1->window,gs->window))
      {
        break;
      }
    }
    if (!gs1)
    {
      gs2=Create_Guistate(gs2,gs->window,gs->button,gs->state);
    }
  }
  Remove_Guistate(gs1t);
  if (!gs2) return;
  while (gs2->prev) gs2=gs2->prev;
  
  if (!(fp=fopen(file,"a"))) return;
  write_states(gs2,fp);
  Remove_Guistate(gs2);
  fclose(fp);
}


union fl_lng
{
  float f;
  guint32 l;
};


float float32_from_le(float f)
{
  union fl_lng conv;
  conv.f=f;
  conv.l=GUINT32_FROM_LE(conv.l);
  return conv.f;
}

float float32_to_le(float f)
{
  union fl_lng conv;
  conv.f=f;
  conv.l=GUINT32_TO_LE(conv.l);
  return conv.f;
}


#if ENABLE_DEBUG == 1
#define DEBUG_FILE "sgtk.dbg"
/*************************************
 * print for debug
 * print info in file if global __debug__ is true 
 *************************************/
int __sgtkdebug__;
#include <time.h>
void pri_dbg(char *frmt,...)
{
  FILE *fp;
  char str[100];
  time_t t1;
  char *t=NULL;
  va_list arg;

  if (!__sgtkdebug__) return;

  t1=time(NULL);
/*
  t=ctime(&t1);
*/

  va_start(arg,frmt);
  vsprintf(str,frmt,arg);
  va_end(arg);

  fp=fopen(DEBUG_FILE,"a");
  fprintf(fp,"%s           %s",str,(t? t : "\n"));
  fclose(fp);
}

pri_wndinfo(WINDOW_DATA *wnd_data)
{
  WINDOW_DATA *wd=NULL;
  for (wd=wnd_data; wd->prev; wd=wd->prev);
  for (; wd; wd=wd->next)
    pri_dbg("WINDOW_DATA: %x  %s  %x",wd,wd->title,wd->window);
}
#endif
