/**************************************************
 * RCSId: $Id: windows.c,v 1.4 2016/08/05 20:04:28 ralblas Exp $
 *
 * Window related functions.
 * Project: SGTK
 * Author: R. Alblas
 *
 * History: 
 * $Log: windows.c,v $
 * Revision 1.4  2016/08/05 20:04:28  ralblas
 * _
 *
 * Revision 1.3  2016/01/10 16:26:15  ralblas
 * _
 *
 * Revision 1.2  2015/12/23 23:15:31  ralblas
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

#define CLOSE_TEXT "Close"

/*************************************
 * Add window-widget into "backbone"
 * Return: pointer to backbone
 *************************************/
WINDOW_DATA *Create_Window_Info(GtkWidget *window, /* existing window */
                                GtkWidget *window_nw, /* window to add */
                                char      *title)  /* same title as window_nw */
{
  WINDOW_DATA *wnd_data=NULL;
  WINDOW_DATA *wnd_data_nw;

/* Get backbone of existing window */
  if (window)
  {
    wnd_data=gtk_object_get_data(GTK_OBJECT(window),WNDINFO);
  }

/* Allocate mem for new backbone part */
  if (!(wnd_data_nw=(WINDOW_DATA *)calloc(sizeof(WINDOW_DATA),1))) return NULL;
/* Add backbone part to chain */
  if (wnd_data)
  {
    while (wnd_data->next) wnd_data=wnd_data->next;
    wnd_data->next=wnd_data_nw;
    wnd_data_nw->prev=wnd_data;
  }
  if (title)
    strncpy(wnd_data_nw->title,title,WINNAAMMAXLEN-1);

/* Add connections between backbone and new window */
  gtk_object_set_data(GTK_OBJECT(window_nw),WNDINFO,(gpointer)wnd_data_nw);
  wnd_data_nw->window=window_nw;

  return wnd_data_nw;
}

/*************************************
 * Remove window-widget from "backbone"
 * Return: 0 if succesfull
 *************************************/
void Remove_Window_Info(GtkWidget *window)
{
  WINDOW_DATA *wnd_data=gtk_object_get_data(GTK_OBJECT(window),WNDINFO);

  if (wnd_data)
  {
    if (wnd_data->prev) wnd_data->prev->next=wnd_data->next;
    if (wnd_data->next) wnd_data->next->prev=wnd_data->prev;
    gtk_object_remove_data(GTK_OBJECT(window),WNDINFO);
    free(wnd_data);
  }
}

/**************************************************
   window_get_root_size with correction for win32 
   top-bar isn't included in window_get_root_size!
 **************************************************/
void window_get_root_size_lw(GdkWindow *wnd,int *w,int *h)
{
  window_get_root_size(wnd,w,h);
#if __GTK_WIN32__
  *w+=6;
  *h+=25;
#endif
}

/* gdk_screen_height with correction for win32 */
/**************************************************
   gdk_screen_height with correction for win32 
   bottom command bar seems to be counted twice in gdk_screen_height!
 **************************************************/
int gdk_screen_height_lw()
{
  int h=gdk_screen_height();
#if __GTK_WIN32__
  h+=28;
#endif
  return h;
}

long overlap_wndarea(int x1,int y1,int w1,int h1,GtkWidget *wnd2)
{
  int x2,y2,w2,h2;
  int xa,ya,xb,yb;
  if (!wnd2->window) return 0;
  gdk_window_get_root_origin(wnd2->window,&x2,&y2);
  window_get_root_size_lw(wnd2->window,&w2,&h2);

  xa=MAX(x2,x1);
  xb=MIN(x2+w2,x1+w1);
  if (xb<xa) return 0;
  ya=MAX(y2,y1);
  yb=MIN(y2+h2,y1+h1);
  if (yb<ya) return 0;
  return (xb-xa)*(yb-ya);
}

void freespace(GtkWidget *wnd,int *xe,int *ye)
{
  GtkWidget *wnd_l1,*wnd_l2,*wnd_first;
  int scr_w=gdk_screen_width();
  int scr_h=gdk_screen_height_lw();
  int a,amin=-1,a1;
  int w0,h0;
  int x1,y1,w1,h1;
  int x,y;

  *xe=0;
  *ye=0;
  wnd_first=First_window(wnd);

/* Get size window to be placed */
  window_get_root_size_lw(wnd->window,&w0,&h0);

/* For positions right or below all windows */
  for (wnd_l1=wnd_first; wnd_l1; wnd_l1=Next_window(wnd_l1))
  {
    if (wnd_l1==wnd) continue;
/* position/size of window */
    gdk_window_get_root_origin(wnd_l1->window,&x1,&y1);
    window_get_root_size_lw(wnd_l1->window,&w1,&h1);

    for (x=x1; x<=x1+w1; x+=w1)
    {
      if (x+w0 > scr_w) x=scr_w-w0;
      for (y=y1; y<=y1+h1; y+=h1)
      {
        if (y+h0 > scr_h) y=scr_h-h0;
/* determine position with min. ovelap with any window */
        a=0;
        for (wnd_l2=wnd_first; wnd_l2; wnd_l2=Next_window(wnd_l2))
        {
          if (wnd_l2==wnd) continue;  

          a1=overlap_wndarea(x,y,w0,h0,wnd_l2);
          a=a1+a;
        }

        if ((amin<0) || (a<amin))
        {
          amin=a;
          *xe=x;
          *ye=y;
        }
        if (y==scr_h-h0) break;
      }
      if (x==scr_w-w0) break;
    }
  }
}

/*************************************
 * Place window 
 * wnd  : A gtk_window somewhere in the chain
 * x,y  : coordinates
 * how  : Method:  0  -> at (x,y)
 *                 others: see struct WND_POS; offset (x,y)
 *************************************/
void place_window(GtkWidget *wnd,int x,int y,WND_POS how)
{
  GtkWidget *first=First_window(wnd);
  int scr_w=gdk_screen_width();
  int scr_h=gdk_screen_height_lw();
  int fwnd_x=0,fwnd_y=0,fwnd_w=0,fwnd_h=0;
  int wnd_x,wnd_y,wnd_w,wnd_h;

  if (first)
  {
    gdk_window_get_root_origin(first->window,&fwnd_x,&fwnd_y);
    window_get_root_size_lw(first->window,&fwnd_w,&fwnd_h);
  }

  gdk_window_get_root_origin(wnd->window,&wnd_x,&wnd_y);
  window_get_root_size_lw(wnd->window,&wnd_w,&wnd_h);

  switch(how)
  {
    case deflt_wndpos:     /* let window manager decide */
    break;
    case abs_wndpos:       /* place at (x,y) */
       gdk_window_move(wnd->window,x,y); break;
    break;
    case relfrst_wndpos:   /* place (+x,+y) relative to first window  */
       gdk_window_move(wnd->window,fwnd_x+x,fwnd_y+y);
    break;
    case center_wndpos:    /* place relative to screen centre */
       gdk_window_move(wnd->window,(scr_w-wnd_w)/2+x,(scr_h-wnd_h)/2+y);
    break;
    case smart_wndpos:     /* try to place in free space */
    {
      int xe,ye;
      freespace(wnd,&xe,&ye);
      gdk_window_move(wnd->window,xe,ye);
    }
    break;
    case left_top:         /* place left-top */
       gdk_window_move(wnd->window,x,y);
    break;
    case right_top:        /* place right-top */
       gdk_window_move(wnd->window,(scr_w-wnd_w)+x,0+y);
    break;
    case left_bottom:      /* place left-bottom */
       gdk_window_move(wnd->window,0+x,(scr_h-wnd_h)+y);
    break;
    case right_bottom:     /* place right-bottom */
       gdk_window_move(wnd->window,(scr_w-wnd_w)+x,(scr_h-wnd_h)+y);
    break;

    default:
    break;
  }
}

#ifdef NIETGEBRUIKT
en dit klopt ook niet
gboolean Toggle_full_screen(GtkWidget *wnd)
{
  WINDOW_DATA *wnd_dat=(WINDOW_DATA *)gtk_object_get_data(GTK_OBJECT(wnd),WNDINFO);
  if (!wnd_dat)
  {
    gtk_window_unfullscreen(wnd);
    wnd_dat->state_fullscreen=FALSE;
  }
  else
  {
    if (wnd_dat->state_fullscreen)
    {
      gtk_window_unfullscreen(wnd);
      wnd_dat->state_fullscreen=FALSE;
    }
    else
    {
      gtk_window_fullscreen(wnd);
      wnd_dat->state_fullscreen=TRUE;
    }
  }
  return  wnd_dat->state_fullscreen;
}
#endif

/*************************************
 * Close window 
 *************************************/
void Close_Window(GtkWidget *widget)
{
  GtkWidget *window;
  if (!widget) return;
  window=Find_Parent_Window(widget);
  if (window) gtk_widget_destroy(window);
/* Note: Eventually this should call the Remove_Window_Info function. */
}

/*************************************
 * This is standard destroy function for Create_Window
 *************************************/
void destroy_window(GtkWidget *widget)
{
  GtkWidget *window=gtk_widget_get_toplevel(widget);
  Remove_Window_Info(window);
}

/*************************************
 * Create a new window
 * Return: pointer to new window widget
 * !! If external func then this should call Remove_Window_Info!!
 *************************************/
GtkWidget *Create_Window(GtkWidget *window,  /* Existing window */
                         int width,          /* size (<0 gives default) */
                         int height,
                         char *title,        /* Name of window or NULL */
                         void func())        /* Destroy function or NULL */
{
  GtkWidget *window_nw;
  WINDOW_DATA *wndinfo;

/* Pop-up window if it already exist. */
  if (window)
  {
    if ((window_nw=Find_Window(window,title)) && (window_nw->window))
    {
      gdk_window_show(window_nw->window);
      return NULL;
    }
  }
  window_nw = gtk_window_new(GTK_WINDOW_TOPLEVEL);

/* Define size of new window */
  if (width<0)  width=DEFAULT_WIDTH;
  if (height<0) height=DEFAULT_HEIGHT;

  if ((width>0) && (height>0))
  {
    gtk_window_set_default_size((GtkWindow *)window_nw,width,height);
  }
  else
  {
    width=0;
    height=0;
  }

/* Add title */
  if (title)
  {
    gtk_window_set_title((GtkWindow *)window_nw,title);
  }

/* Add quit-function, to be executed if window disappears.
   Must call Remove_Window_Info!
*/
  if (func)
    gtk_signal_connect(GTK_OBJECT(window_nw), "destroy",
		                      GTK_SIGNAL_FUNC(func), NULL);
  else
    gtk_signal_connect(GTK_OBJECT(window_nw), "destroy",
		                      GTK_SIGNAL_FUNC(destroy_window), NULL);

/* Add window into backbone */
  wndinfo=Create_Window_Info(window,window_nw,title);
  wndinfo->default_width=width;
  wndinfo->default_height=height;

  return window_nw;
}


/* ========== Dialog window ========== */
static int dialog_ret;

/*************************************
 * Close dialog window 
 *************************************/
void Close_Dialog_and_catch(GtkWidget *widget)
{
  GtkWidget *window=gtk_widget_get_toplevel(widget);
  dialog_ret=(int)gtk_object_get_data(GTK_OBJECT(widget),WDGT_ID);
  gtk_grab_remove(window);

  gtk_widget_destroy(window);
  gtk_main_quit();
}

void Close_Dialog(GtkWidget *widget)
{
  GtkWidget *window=gtk_widget_get_toplevel(widget);

  gtk_grab_remove(window);

  gtk_widget_destroy(window);
  gtk_main_quit();
}

void Close_Dialog_cont(GtkWidget *widget)
{
  GtkWidget *window=gtk_widget_get_toplevel(widget);

  gtk_grab_remove(window);

  gtk_widget_destroy(window);
}

#if __GTK_WIN32__
/* Dialog in Windows doesn't work properly. */
/*
void Close_Dialog1(GtkWidget *widget)
{
  GtkWidget *window=gtk_widget_get_toplevel(widget);

  dialog_ret=(int)gtk_object_get_data(GTK_OBJECT(widget),WDGT_ID);
  gtk_grab_remove(window);

  gtk_widget_destroy(window);
  gtk_main_quit();
}
*/

void Create_Message(char *title,        /* title of dialog */
		    char *frmt,...)     /* message */
{
  GtkWidget *message;
  GtkWidget *dialog_window;
  GtkWidget *button;
  GtkWidget *vbox,*action_area;

  char message_str[500];
  va_list arg;
  va_start(arg,frmt);
  vsnprintf(message_str,490,frmt,arg);
  va_end(arg);

/* Open the window */
  dialog_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect(GTK_OBJECT(dialog_window), "destroy",
			GTK_SIGNAL_FUNC(gtk_widget_destroy), &dialog_window);

/* Add title */
  if (title) gtk_window_set_title(GTK_WINDOW(dialog_window),title);

/* Add message */
  message = gtk_label_new(message_str);
  gtk_misc_set_padding(GTK_MISC(message),10,10);

  vbox=gtk_vbox_new(FALSE,0);
  gtk_widget_show(vbox);

  gtk_box_pack_start(GTK_BOX(vbox),message, TRUE, TRUE, 0);

  gtk_widget_show (message);

  action_area=gtk_hbox_new(FALSE,0);


/* Add dismiss button */
  
  button=Create_Button(CLOSE_TEXT,Close_Dialog);

  gtk_box_pack_start(GTK_BOX(action_area),button, TRUE, TRUE, 0);
  gtk_widget_show(action_area);

  gtk_box_pack_start(GTK_BOX(vbox),action_area, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(dialog_window),vbox);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button);

/*
gtk_window_set_modal((GtkWindow *)dialog_window,TRUE);
*/

  gtk_grab_add(dialog_window);

  gtk_widget_show(dialog_window);
  place_window(dialog_window,0,0,center_wndpos);
  gtk_main();
}

/*************************************
 * Create dialog window 
 * Example:
 *   Create_Choice("Name",2,lab1,lab2,"text: %d",n);
 *************************************/
#define NRCHOICEMAX 5
int Create_Choice(char *title,
                  int nrch,          /* amount of funcs */
                  ...)               /* funcs and text */
{
  GtkWidget *message;
  GtkWidget *dialog_window;
  GtkWidget *buttons[NRCHOICEMAX+1]={NULL,NULL,NULL,NULL};
  GtkWidget *vbox,*action_area;
  char *labels[NRCHOICEMAX];
  char *frmt;
  char message_str[1000];
  int i;
  va_list arg;
  va_start(arg,nrch);

/* Get labels */
  for (i=0; i<nrch; i++)
  {
    if (i>=NRCHOICEMAX)
      va_arg(arg,char *);
    else
      labels[i]=va_arg(arg,char *);
  }
  nrch=MIN(nrch,NRCHOICEMAX);

/* Get text */
  frmt=va_arg(arg,char *);
  vsprintf(message_str,frmt,arg);
  va_end(arg);

/* Open the window */
  dialog_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect(GTK_OBJECT(dialog_window), "destroy",
			GTK_SIGNAL_FUNC(gtk_widget_destroy), &dialog_window);

/* Add title */
  if (title) gtk_window_set_title(GTK_WINDOW(dialog_window),title);

/* Add message */
  message = gtk_label_new(message_str);
  gtk_misc_set_padding(GTK_MISC(message),10,10);

  vbox=gtk_vbox_new(FALSE,0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(vbox),message, TRUE, TRUE, 0);
  gtk_widget_show (message);

  action_area=gtk_hbox_new(FALSE,0);

  buttons[0]=gtk_hbox_new(TRUE,2);
  for (i=0; i<nrch; i++)
  {
    buttons[i+1]=Create_Button(labels[i],Close_Dialog_and_catch);
    gtk_object_remove_data(GTK_OBJECT(buttons[i+1]),WDGT_ID);
    gtk_object_set_data(GTK_OBJECT(buttons[i+1]),WDGT_ID,(gpointer)(i+1));
    gtk_box_pack_start(GTK_BOX(buttons[0]), buttons[i+1], TRUE, TRUE, 1);
  }

  gtk_widget_show(buttons[0]);
  if (buttons[0])
  {
    gtk_box_pack_start(GTK_BOX(action_area),buttons[0], TRUE, TRUE, 0);
    gtk_widget_show(action_area);
  }

  gtk_box_pack_start(GTK_BOX(vbox),action_area, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(dialog_window),vbox);

  gtk_grab_add(dialog_window);
  gtk_widget_show(dialog_window);
  place_window(dialog_window,0,0,center_wndpos);
  gtk_main();

/* Return the button nr. clicked */
  i=dialog_ret;
  dialog_ret=0;
  return i;
}

#else
/* dialog for Linux; this part doesn't work with Windows. */
/*************************************
 * Create message window 
 * For short messages.
 * For long ones, see Create_Info().
 *************************************/
static void Create_Message1(char *title,gboolean stop,char *message_str)
{
  GtkWidget *message;
  GtkWidget *dialog_window;
  GtkWidget *button;

/* Open the window */
  dialog_window=gtk_dialog_new();

  if (stop)
    gtk_signal_connect(GTK_OBJECT(dialog_window), "destroy",
			GTK_SIGNAL_FUNC(Close_Dialog), &dialog_window);
  else
    gtk_signal_connect(GTK_OBJECT(dialog_window), "destroy",
			GTK_SIGNAL_FUNC(Close_Dialog_cont), &dialog_window);

/* Add title */
  if (title) gtk_window_set_title(GTK_WINDOW(dialog_window),title);

/* Add message */
  message = gtk_label_new(message_str);

  gtk_misc_set_padding(GTK_MISC(message),10,10);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->vbox),
                                              message, TRUE, TRUE, 0);
  gtk_widget_show (message);

/* Add dismiss button */
  if (stop)
    button=Create_Button(CLOSE_TEXT,Close_Dialog);
  else
    button=Create_Button(CLOSE_TEXT,Close_Dialog_cont);

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->action_area),
                                              button, TRUE, TRUE, 0);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button);

/*
gtk_window_set_modal((GtkWindow *)dialog_window,TRUE);
*/

  gtk_grab_add(dialog_window);

  gtk_widget_show_all(dialog_window);
  place_window(dialog_window,0,0,center_wndpos);
  if (stop)
    gtk_main();
}

void Create_Message(char *title,        /* title of dialog */
                    char *frmt,...)     /* message */
{
  char message_str[1000];
  va_list arg;
  va_start(arg,frmt);
  vsprintf(message_str,frmt,arg);
  va_end(arg);
  Create_Message1(title,TRUE,message_str);     /* message */
}

void Create_Message_cont(char *title,        /* title of dialog */
                         char *frmt,...)     /* message */
{
  char message_str[1000];
  va_list arg;
  va_start(arg,frmt);
  vsprintf(message_str,frmt,arg);
  va_end(arg);
  Create_Message1(title,FALSE,message_str);     /* message */

}

/*************************************
 * Create dialog window 
 * Example:
 *   Create_Choice("Name",2,lab1,lab2,"text: %d",n);
 *************************************/
#define NRCHOICEMAX 5
int Create_Choice(char *title,
                  int nrch,          /* amount of funcs */
                  ...)               /* funcs and text */

{
  GtkWidget *message;
  GtkWidget *dialog_window;
  GtkWidget *buttons[NRCHOICEMAX+1];

  char *labels[NRCHOICEMAX];
  char *frmt;
  char message_str[1000];
  int i;

  va_list arg;
  va_start(arg,nrch);

/* Get labels */
  for (i=0; i<nrch; i++)
  {
    if (i>=NRCHOICEMAX)
      va_arg(arg,char *);
    else
      labels[i]=va_arg(arg,char *);
  }
  nrch=MIN(nrch,NRCHOICEMAX);

/* Get text */
  frmt=va_arg(arg,char *);
  vsprintf(message_str,frmt,arg);
  va_end(arg);

/* Open the window */
  dialog_window=gtk_dialog_new();
  gtk_signal_connect(GTK_OBJECT(dialog_window), "destroy",
			GTK_SIGNAL_FUNC(Close_Dialog), &dialog_window);

/* Add title */
  if (title) gtk_window_set_title(GTK_WINDOW(dialog_window),title);

/* Add message */
  message = gtk_label_new(message_str);
  gtk_misc_set_padding(GTK_MISC(message),10,10);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->vbox),
                                                  message, TRUE, TRUE, 0);
  gtk_widget_show (message);

  buttons[0]=gtk_hbox_new(TRUE,2);
  for (i=0; i<nrch; i++)
  {
    buttons[i+1]=Create_Button(labels[i],Close_Dialog_and_catch);
    gtk_object_remove_data(GTK_OBJECT(buttons[i+1]),WDGT_ID);
    gtk_object_set_data(GTK_OBJECT(buttons[i+1]),WDGT_ID,(gpointer)(i+1));
    gtk_box_pack_start(GTK_BOX(buttons[0]), buttons[i+1], TRUE, TRUE, 1);
  }

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_window)->action_area),
                                                  buttons[0], TRUE, TRUE, 0);

  gtk_widget_show(buttons[0]);
  gtk_widget_show(dialog_window);
  gtk_grab_add(dialog_window);
  place_window(dialog_window,0,0,center_wndpos);
  gtk_main();

/* Return the button nr. clicked */
  i=dialog_ret;
  dialog_ret=0;
  return i;
}
#endif

/*************************************
 * Create text-widget 
 *************************************/
GtkWidget *Create_Text(char *name,gboolean editable,char *font)
{
  GtkWidget *text;
  GtkWidget *scroll_win;

  scroll_win = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);

#ifdef __GTK_20__
  GtkTextBuffer *buffer;
  text=gtk_text_view_new();
  if (font)
  {
    PangoFontDescription *font_desc;
    font_desc = pango_font_description_from_string(font);
    gtk_widget_modify_font (text, font_desc);
    pango_font_description_free(font_desc);
  }
  gtk_container_add(GTK_CONTAINER(scroll_win),text);
  if (name) gtk_object_set_data(GTK_OBJECT(text),WDGT_ID,(gpointer)name);

  buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
  gtk_object_set_data(GTK_OBJECT(scroll_win),"TEXT_ID",(gpointer)buffer);

  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text),TRUE);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text),editable);
  gtk_widget_show(scroll_win);
  gtk_widget_show(text);
  text=text->parent;
#else
  text = gtk_text_new(NULL,NULL);
  gtk_container_add(GTK_CONTAINER(scroll_win),text);

  if (name) gtk_object_set_data(GTK_OBJECT(text),WDGT_ID,(gpointer)name);

  gtk_widget_show(scroll_win);
  gtk_widget_show(text);
  gtk_text_set_word_wrap(GTK_TEXT(text),TRUE);

  gtk_text_set_editable(GTK_TEXT(text), editable);
#endif
  return text;
}



void Close_Info(GtkWidget *widget)
{
  GtkWidget *window=gtk_widget_get_toplevel(widget);
  gtk_widget_destroy(window);
}

/*************************************
 * Create info window 
 * For short messages see Create_Message().
 *************************************/
GtkWidget *Create_Info2(GtkWidget *widget,char *name,
                       int w,int h,
                       void func(),
                       GtkWidget *ebut,char *font)
{
  GtkWidget *wnd,*text,*pck,*but1,*but2=NULL;
  GtkWidget *main_window;
  main_window=Find_Parent_Window(widget);
  wnd=Create_Window(main_window,w,h,name,NULL);
  if (!wnd) return NULL; /* This info already exist; just pop-up */

  text=Create_Text(NULL,FALSE,font);
  but1=Create_Button(CLOSE_TEXT,Close_Info);
  if (func)
  {
    but2=Create_Button("Reload",func);
  }

  pck=SPack(NULL,"h",but1,"ef5",but2,"ef5",ebut,"ef5",NULL);
#ifdef __GTK_20__
  pck=Pack(NULL,'v',text,5,pck,5,NULL);
#else
  pck=Pack(NULL,'v',text->parent,5,pck,5,NULL);
#endif

  gtk_container_add(GTK_CONTAINER(wnd),pck);
  gtk_widget_show(wnd);

  return text;
}
GtkWidget *Create_Info(GtkWidget *widget,char *name,
                       int w,int h,
                       void func(),
                       GtkWidget *ebut)
{
  return Create_Info2(widget,name,w, h,func,ebut,NULL);
}


/*************************************
 * Clear text from gtk_text_new
 *************************************/
void Clear_Text(GtkWidget *text)
{
#ifdef __GTK_20__
  GtkTextBuffer *buffer;
  buffer=(GtkTextBuffer *)gtk_object_get_data(GTK_OBJECT(text),"TEXT_ID");
  gtk_text_buffer_set_text (buffer, "", -1);
#else
  GtkText *itext=(GtkText *)text;
  gtk_text_set_point(itext,0);
  gtk_text_forward_delete(itext,gtk_text_get_length(itext));
#endif
}

/*************************************
 * Add text to gtk_text_new 
 * fontsize= size font
 *************************************/
void Add_Text(GtkWidget *text,int fontsize,char *frmt,...)
{
  static GdkFont *used_font;
  char str[1000];
  va_list arg;
  va_start(arg,frmt);
  vsprintf(str,frmt,arg);
  va_end(arg);
  if (!fontsize) fontsize=120;
  if (!used_font)
  {
    char font[100];
    sprintf(font,SGTK_VARFONT,fontsize);
    used_font = gdk_font_load (font);
  }
#ifdef __GTK_20__
  {
    GtkTextBuffer *buffer;

    buffer=(GtkTextBuffer *)gtk_object_get_data(GTK_OBJECT(text),"TEXT_ID");
    if (buffer)
      gtk_text_buffer_insert_at_cursor(buffer,str,-1);
  }
#else
  gtk_text_insert(GTK_TEXT(text),used_font,&text->style->black,NULL,str, -1);
#endif
}

typedef struct progress_data
{
  gboolean cancel;
  int pct_prev;
} PROGR_DAT;
#define PRGRS_ID "Progress_id"

/*************************************
 * Help functions for Create_Progress 
 *************************************/
/*************************************
 * Cancel progress using button 'Cancel'
 *************************************/
static void Cancel_Progress(GtkWidget *widget)
{
  GtkWidget *window=gtk_widget_get_toplevel(widget);
  PROGR_DAT *prgr_dat=gtk_object_get_data(GTK_OBJECT(window),PRGRS_ID);
  if (prgr_dat) prgr_dat->cancel=TRUE;
}

static gboolean aborted;
/*************************************
 * Cancel progress activated by closing window.
 * If caused by window-manager 'Close': set aborted flag.
 * Note: This will close all running progress bars 
 * because of static 'aborted'!
 *************************************/
static void Abort_Progress(GtkWidget *widget)
{
  PROGR_DAT *prgr_dat=gtk_object_get_data(GTK_OBJECT(widget),PRGRS_ID);

/* set aborted if window-manager close is clicked */
  if (prgr_dat)
  {
    if (!prgr_dat->cancel) aborted=TRUE; 
    if (prgr_dat) free(prgr_dat);
  }
  destroy_window(widget);
}

/*************************************
 * Create window with Progress bar and cancel button
 *************************************/
GtkWidget *Create_Progress(GtkWidget *window,
                           char *title,gboolean may_cancel)
{
  GtkWidget *progress_window=Create_Window(window,0,0,title,Abort_Progress);
  GtkWidget *progressbar=gtk_progress_bar_new();
  GtkWidget *button=NULL;
  GtkWidget *wz;
  PROGR_DAT *prgr_dat;
  if (!progress_window) return NULL;

  if (!(prgr_dat=(PROGR_DAT *)calloc(sizeof(PROGR_DAT),1))) return NULL;
  gtk_object_set_data(GTK_OBJECT(progress_window),PRGRS_ID,(gpointer)prgr_dat);
  if (may_cancel)
  {
    button=Create_Button("Cancel",Cancel_Progress);
  }
  wz=SPack(NULL,"v",progressbar,"ef10",button,"10",NULL);
  gtk_container_add(GTK_CONTAINER(progress_window),wz);
  gtk_widget_show(progressbar);
  gtk_widget_show(progress_window);
/*
  gdk_window_set_decorations(progress_window->window,0);
*/
  place_window(progress_window,0,0,center_wndpos);
  Update_Progress(progressbar,0,0);
  aborted=FALSE;
  return progressbar;
}
#include <time.h>

/*************************************
 * Evaluate cancel button 
 *************************************/
gboolean Get_Progress_state(GtkWidget *progressbar)
{
  GtkWidget *window;
  PROGR_DAT *prgr_dat;
  window=gtk_widget_get_toplevel(progressbar);
  if (!window) return TRUE;
  prgr_dat=gtk_object_get_data(GTK_OBJECT(window),PRGRS_ID);
  if (!prgr_dat) return TRUE;
  if (prgr_dat->cancel) return TRUE;    /* cancel requested */
  return FALSE;
}


/*************************************
 * Update Progress bar 
 *************************************/
gboolean Update_Progress(GtkWidget *progressbar,int pos,int len)
{
  GtkWidget *window;
  PROGR_DAT *prgr_dat;
  gfloat pval;
  int pct;
  if (aborted) return TRUE;             /* aborted using Close button */
  if (!progressbar) return TRUE;
  if (Get_Progress_state(progressbar)) return TRUE;

  window=gtk_widget_get_toplevel(progressbar);
  if (!window) return TRUE;
  prgr_dat=gtk_object_get_data(GTK_OBJECT(window),PRGRS_ID);
  if (!prgr_dat) return TRUE;

  if (len<=0)
  {
    pval=0;
    prgr_dat->pct_prev=-1;
    prgr_dat->cancel=FALSE;
  }
  else
  {
    pval=(float)pos/(float)len;
    pval=MIN(pval,1);
  }

  pct=pval*100;

  if (ABS(pct-prgr_dat->pct_prev) < 1) return prgr_dat->cancel;
  prgr_dat->pct_prev=pct;

  gtk_progress_set_percentage(GTK_PROGRESS(progressbar),pval);

  {
    static time_t st;
    static int dt;
    int rt;

    if (len==0)
    {
      st=time(NULL);
    }
    else if (dt>30)
    {
      char title[100];
      dt=(int)difftime(time(NULL),st);
      rt=dt*(float)(100-pct)/(float)pct;
      sprintf(title,"%d:%02d",rt/60,rt-(rt/60)*60);
      gtk_window_set_title((GtkWindow *)window,title);
    }
  }

  while (g_main_iteration(FALSE));

  return prgr_dat->cancel;
}

void Close_Progress(GtkWidget *widget)
{
  if (!aborted)
  {
    Close_Window(gtk_widget_get_toplevel(widget));
  }
}

GtkWidget *Create_Oneliner(char *label,char *str)
{
  GtkWidget *wnd,*b;

  if (!str) return NULL;

  wnd=Create_Window(NULL,0,0,"Ladende...",NULL);
  b=Create_Entry(label,NULL,"%s",str);
  b=Pack(NULL,'h',b,1,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),b);
  gtk_widget_show(wnd);
  return wnd;
}
