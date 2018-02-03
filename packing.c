/**************************************************
 * RCSId: $Id: packing.c,v 1.2 2017/04/11 20:18:59 ralblas Exp $
 *
 * Packing functions.
 * Project: SGTK
 * Author: R. Alblas
 *
 * History: 
 * $Log: packing.c,v $
 * Revision 1.2  2017/04/11 20:18:59  ralblas
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "sgtk.h"

/*************************************
 * Pack widgets, specific.
 * label: Add frame with label (NULL: no frame)
 * hv   : has 'h'-> pack horizontal
 *            'v'-> pack vertical
 *            'H'-> pack homogeneous
 *            's'-> add scrollbars if needed
 *            'sTR'-> add scrollbars left-bottom
 *            'sBL'-> add scrollbars right-top
 *            'sBR'-> add scrollbars left-top
 *            'sTL'-> add scrollbars right-bottom (def.)
 * ...  : one or more widget/flag_space pairs
 *        Last should be NULL
 *        widget/flag_space: A string containing
 *          "E"    pack at end
 *          "e"    expand
 *          "f"    fill
 *          "a"    automatic expand/fill flag
 *          "5"    (or any number): padding number
 *************************************/
GtkWidget *SPack(char *label,char *hv,...)
{
  GtkWidget *widgeto;
  GtkWidget *widgeti;
  char *option,*p;
  gboolean fill=FALSE,expand=FALSE;
  gboolean homogeneous=FALSE;
  int paned=0;
  int padding1=0;
  int padding2=0;
  va_list ap;
  va_start(ap,hv);
  for (p=hv; *p && (!isdigit((int)*p)); p++);
  if (p) padding1=atoi(p);
  if (strchr(hv,'H')) homogeneous=TRUE;

  if (strchr(hv,'p'))
  {
    paned=1;
    if (strchr(hv,'h'))      widgeto = gtk_hpaned_new();
    else if (strchr(hv,'v')) widgeto = gtk_vpaned_new();
    else                     widgeto = gtk_hpaned_new();
  }
  else
  {
    if (strchr(hv,'h'))      widgeto = gtk_hbox_new(homogeneous,padding1);
    else if (strchr(hv,'v')) widgeto = gtk_vbox_new(homogeneous,padding1);
    else                     widgeto = gtk_hbox_new(homogeneous,padding1);
  }

  if (label)
    gtk_object_set_data(GTK_OBJECT(widgeto),WDGT_ID,(gpointer)label);

  while (TRUE)
  {
    widgeti=(GtkWidget *)va_arg(ap,GtkWidget *);
    if (!widgeti) break;

/* To add special widgets easily into packing box...*/
    if (GTK_IS_CLIST(widgeti)) widgeti=widgeti->parent;
#ifndef __GTK_20__
    if (GTK_IS_TEXT(widgeti)) widgeti=widgeti->parent;
#endif
    option=(char *)va_arg(ap,char *);

    if (strchr(option,'f')) fill  =TRUE; else fill  =FALSE;
    if (strchr(option,'e')) expand=TRUE; else expand=FALSE;
    for (p=option; *p && (!isdigit((int)*p)); p++);
    if (p) padding2=atoi(p);

/* Make drawable and scrollable items fill the window */
    if (strchr(option,'a'))
    {
      if ((GTK_IS_DRAWING_AREA(widgeti)) || (GTK_IS_SCROLLED_WINDOW(widgeti)))
      {
        fill=TRUE;
      }
      else
      {
        fill=FALSE;
      }
    }
    if (strchr(option,'a'))
    {
      if ((GTK_IS_DRAWING_AREA(widgeti)) || (GTK_IS_SCROLLED_WINDOW(widgeti)))
      {
        expand=TRUE;
      }
      else
      {
        expand=FALSE;
      }
    }

    if (paned==1)
    {
      gtk_paned_pack1 (GTK_PANED (widgeto), widgeti, TRUE, FALSE);
      paned++;
    }
    else if (paned==2)
    {
      gtk_paned_pack2 (GTK_PANED (widgeto), widgeti, TRUE, FALSE);
    }
    else
    {
      if (strchr(option,'E'))
        gtk_box_pack_end(GTK_BOX(widgeto), widgeti, expand, fill, padding2);
      else
        gtk_box_pack_start(GTK_BOX(widgeto), widgeti, expand, fill, padding2);
    }
  }
  va_end(ap);
  gtk_widget_show(widgeto);


  if ((label) && (*label!='!'))
  {
    GtkWidget *frame;
    if (*label)
      frame=gtk_frame_new(label);
    else
      frame=gtk_frame_new(NULL);
    gtk_widget_show(frame);
    gtk_container_add(GTK_CONTAINER(frame),widgeto);
    widgeto=frame;
  }

  if (strchr(hv,'s'))
  {
    GtkScrolledWindow *scroll_win;
    scroll_win=(GtkScrolledWindow *)gtk_scrolled_window_new(NULL, NULL);
    if (strstr(hv,"TR"))
      gtk_scrolled_window_set_placement(scroll_win,GTK_CORNER_TOP_RIGHT); 
    else if (strstr(hv,"BL"))
      gtk_scrolled_window_set_placement(scroll_win,GTK_CORNER_BOTTOM_LEFT); 
    else if (strstr(hv,"BR"))
      gtk_scrolled_window_set_placement(scroll_win,GTK_CORNER_BOTTOM_RIGHT); 
    else
      gtk_scrolled_window_set_placement(scroll_win,GTK_CORNER_TOP_LEFT); 
    if (strchr(hv,'h'))
      gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
                                                       GTK_POLICY_AUTOMATIC,
                                                       GTK_POLICY_NEVER);
    else
      gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
                                                       GTK_POLICY_NEVER,
                                                       GTK_POLICY_AUTOMATIC);

    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll_win), 
                                                                    widgeto);

    widgeto=(GtkWidget *)scroll_win;
  }
  if (label)
    gtk_object_set_data(GTK_OBJECT(widgeto),TWDGT_ID,(gpointer)label);

  return widgeto;
}

/*************************************
 * Pack widgets.
 * label: Add frame with label (NULL: no frame)
 * hv   : ='h'-> pack horizontal
 *         'v'-> pack vertical
 *         'H'-> pack horizontal homogeneous
 *         'V'-> pack vertical homogeneous
 * ...  : one or more widget/space pairs
 *        Last should be NULL
 *        If space<0 then this widget should fill the window.
 *        (drawable and scrollable widgets are always packed as 'fill')
 *************************************/
GtkWidget *Pack(char *label,char hv,...)
{
  GtkWidget *widgeto;
  GtkWidget *widgeti;
  int space;
  va_list ap;
  va_start(ap,hv);
  switch(hv)
  {
    case 'h': widgeto = gtk_hbox_new(FALSE, 0); break; /* non-homo */
    case 'H': widgeto = gtk_hbox_new(TRUE, 0);  break; /* homo */
    case 'v': widgeto = gtk_vbox_new(FALSE, 0); break; /* non-homo */
    case 'V': widgeto = gtk_vbox_new(TRUE, 0);  break; /* homo */
    default : widgeto = gtk_hbox_new(FALSE, 0); break; 
  }

  if (label)
    gtk_object_set_data(GTK_OBJECT(widgeto),WDGT_ID,(gpointer)label);

  while (TRUE)
  {
    widgeti=(GtkWidget *)va_arg(ap,GtkWidget *);
    if (!widgeti) break;
    if (GTK_IS_CLIST(widgeti)) widgeti=widgeti->parent;
#ifndef __GTK_20__
    if (GTK_IS_TEXT(widgeti)) widgeti=widgeti->parent;
#endif
    space=(int)va_arg(ap,int);

/* Make drawable and scrollable items fill the window */
    if ((GTK_IS_DRAWING_AREA(widgeti)) || (GTK_IS_SCROLLED_WINDOW(widgeti)))
    {
      gtk_box_pack_start(GTK_BOX (widgeto), widgeti, TRUE, TRUE, ABS(space));
    }
    else if (space<0) /* This widget should fill the window. */
    {
      gtk_box_pack_start(GTK_BOX (widgeto), widgeti, TRUE, TRUE, ABS(space));
    }
    else
    {
      gtk_box_pack_start(GTK_BOX (widgeto), widgeti, FALSE, FALSE, space);
    }
  }
  va_end(ap);
  gtk_widget_show(widgeto);


  if ((label) && (*label!='!'))
  {
    GtkWidget *frame;
    if (*label)
      frame=gtk_frame_new(label);
    else
      frame=gtk_frame_new(NULL);
    gtk_widget_show(frame);
    gtk_container_add(GTK_CONTAINER(frame),widgeto);
    widgeto=frame;
  }

  if (label)
    gtk_object_set_data(GTK_OBJECT(widgeto),TWDGT_ID,(gpointer)label);

  return widgeto;
}

