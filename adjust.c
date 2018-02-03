/**************************************************
 * RCSId: $Id: adjust.c,v 1.2 2017/03/20 21:22:58 ralblas Exp $
 *
 * Button related functions.
 * Project: SGTK
 * Author: R. Alblas
 *
 * History: 
 * $Log: adjust.c,v $
 * Revision 1.2  2017/03/20 21:22:58  ralblas
 * _
 *
 * Revision 1.1  2015/11/15 22:32:25  ralblas
 * Initial revision
 *
 **************************************************/
/*******************************************************************
 * Copyright (C) 2000 R. Alblas 
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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "sgtk.h"

/*************************************
 * Parse format; determine # digits.
 * Supported: %d, %f and e.g. %2d, %5.3f
 * p  format string
 * i  number   after '%' if present (e.g. *p="%5.3f": i=5)
 * d  fraction after '%' if present (e.g. *p="%5.3f": d=3)
 * %? means: skip this argument
 *************************************/
char *get_frmt(char *p,int *i,int *d)
{
  if (!p) return NULL;
  while (*p)
  {
    if (*p=='%')
    {
      if (isdigit((int)*(p+1)))
      {
        *i=atoi(p+1); 
      }

      while ((*p) && (!strchr("df?",*p)))
      {
        if (*p=='.') *d=atoi(p+1);
        p++;
      }
      if (*p=='d') { *d=0; break; }
      if (*p=='f') {       break; }
      if (*p=='?') {       break; }
    }
    p++;
  }
  return p;
}

/*************************************
 * Create a scale widget
 * 1 value  behind frmt:  max_val.
 * 2 values behind frmt:  min_val/max_val.
 * 3 values behind frmt:  init_val/min_val/max_val.
 * 4 values behind frmt:  step_val/init_val/min_val/max_val.
 * Not specified values are 0.
 * Increment value depends on frmt, e.g. %3.2f: 0.01.
 * Examples:
 *   Create_Scale("test",'h',NULL,"%d%d%d",3,0,10);
 *   Create_Scale("test",'v',NULL,"%3.1f",9.);
 *************************************/
GtkWidget *Create_Scale(char *name,     /* name of the button */
                        char hv,        /* horizontal ('h') or vetical ('v') */
                        void func(),    /* function to be executed */
                        char *frmt,...) /* format string, then max. 3 values */
{
  static GtkWidget *tbut;
  GtkWidget *label;
  GtkWidget *scale;
  GtkObject *adjustment;
  char *p;
  int n=5,d=2;

  float cur_val=0.;
  float min_val=0.;
  float max_val=0.;
  float step_incr=1.,page_incr=10.;

  va_list arg;
  va_start(arg,frmt);

/* Get values specified behind 'frmt'. Max. 3. */
  p=frmt;
  while (*(p=get_frmt(p,&n,&d)))
  {   
    if (*p=='f')
    {
      cur_val=min_val;
      min_val=max_val;
      max_val=va_arg(arg,double);
      step_incr=(d? 1./pow(10.,(float)d) : 0.1);
      page_incr=1.0;
    }
    if (*p=='d')
    {
      step_incr=cur_val;
      cur_val=min_val;
      min_val=max_val;
      max_val=va_arg(arg,int);
      if (step_incr<1) step_incr=1.;
      page_incr=10.;
    }
  }
  va_end(arg);
  page_incr=step_incr;

/* Determine horizontal/vertical */
  if (hv=='h')
    tbut = gtk_hbox_new(FALSE, 0);
  else if (hv=='v')
    tbut = gtk_vbox_new(FALSE, 0);
  else
    return NULL;

/* Name and place Label */
  label = gtk_label_new(name);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

  gtk_box_pack_start(GTK_BOX(tbut), label, FALSE, FALSE, 10);
  gtk_widget_show(label);

/* The ranger */
  adjustment = gtk_adjustment_new(cur_val,min_val,max_val,
                                               step_incr,page_incr, 0.0);
  if (hv=='h')
    scale = gtk_hscale_new(GTK_ADJUSTMENT(adjustment));
  else
    scale = gtk_vscale_new(GTK_ADJUSTMENT(adjustment));
  gtk_range_set_update_policy(GTK_RANGE(scale), GTK_UPDATE_DELAYED);

#ifdef __GTK_20__
  gtk_widget_set_size_request(scale, 70, 0);
#endif

/* Determine # of decimals to display: 1 for floats, 0 for integers */
  if (strstr(frmt,"f"))
    gtk_scale_set_digits(GTK_SCALE(scale), d);
  else
    gtk_scale_set_digits(GTK_SCALE(scale), 0);
  gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);

/* Make widget trace-able with FindWidget function */
  gtk_object_set_data(GTK_OBJECT(scale),WDGT_ID,(gpointer)name);
  gtk_object_set_data(GTK_OBJECT(tbut),TWDGT_ID,(gpointer)name);

/* To remove.; don't know how to get adjustment from scale and vv. */
  gtk_object_set_data(GTK_OBJECT(scale),ADJUST_ID,(gpointer)adjustment);
  gtk_object_set_data(GTK_OBJECT(adjustment),PARENT_ID,(gpointer)scale);

// volgende moet weg -> gedaan 05-01-2007
  gtk_object_set_data(GTK_OBJECT(adjustment),ADJUST_ID,(gpointer)scale);
// 'TRUE, TRUE' makes it possible to size ('fill') scale button.
  gtk_box_pack_start(GTK_BOX(tbut), scale, TRUE, TRUE, 0);

  gtk_widget_show (scale);
  gtk_widget_show (tbut);

/* Add function if specified */
  if (func) gtk_signal_connect(GTK_OBJECT(adjustment), "value-changed",
			GTK_SIGNAL_FUNC(func), name);

  return tbut;
}

/*************************************
 * Create a spin widget
 * 1 value  behind frmt:  max_val.
 * 2 values behind frmt:  min_val/max_val.
 * 3 values behind frmt:  init_val/min_val/max_val.
 * 4 values behind frmt:  step_val/init_val/min_val/max_val.
 * Not specified values are 0.
 * Increment value depends on frmt, e.g. %3.2f: 0.01.
 * name: name label.
 *       If starts with '^': label top (default: left) 
 *       If starts with '!': label invisible (only used for widget-search) 
 * Examples:
 *   Create_Spin("test",NULL,"%d%d%d",3,0,10);
 *   Create_Spin("test",NULL,"%f",9.);
 *************************************/
GtkWidget *Create_Spin(char *name,     /* name of the button */
                       void func(),    /* function to be executed */
                       char *frmt,...) /* format string, then max. 4 values */
{
  GtkWidget *tbut;
  GtkWidget *spin;
  GtkObject *adjustment;
  char *p;
  int nrnum=5,nrfrac=2;

  float cur_val=0.;
  float min_val=0.;
  float max_val=0.;
  float step_incr=1.,page_incr=10.;

  va_list arg;
  va_start(arg,frmt);

/* Get values specified behind 'frmt'. Max. 3. */
  p=frmt;
  while (*(p=get_frmt(p,&nrnum,&nrfrac)))
  {   
    if (*p=='f')
    {
      step_incr=cur_val;
      cur_val=min_val;
      min_val=max_val;
      max_val=va_arg(arg,double);
      if (!step_incr)
        step_incr=(nrfrac? 1./pow(10.,(float)nrfrac) : 0.1);
      page_incr=step_incr*100;
    }
    if (*p=='d')
    {
      step_incr=cur_val;
      cur_val=min_val;
      min_val=max_val;
      max_val=va_arg(arg,int);
      if (step_incr<1) step_incr=1.;
      page_incr=10.;
    }
  }
  va_end(arg);

//  tbut = gtk_vbox_new(FALSE, 0);

/* Determine where to place label. (start with '<' -> left) */
/* Only add label if name doesn't start with '!'. */
  tbut=add_label(name);

/* The spin */
  adjustment = gtk_adjustment_new(cur_val,min_val,max_val,
                                               step_incr,page_incr, 0.0);

  spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),1.0,1);

  gtk_spin_button_configure((GtkSpinButton *)spin,
                               GTK_ADJUSTMENT(adjustment),1.0,nrfrac);

  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spin), TRUE);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spin), FALSE);

  {
  // gtk2.0: size auto set, no resizing needed.
#ifndef __GTK_20__
    int size_button=gdk_char_width(gtk_style_get_font(spin->style),'M')*(nrnum+2);
    gtk_widget_set_usize(spin, size_button, 0);
#endif
  }


/* Make widget trace-able with FindWidget function */
  Add_WidgetID(spin,name);
/* volgende moet zoiets zijn als Add_WidgetID maar dan met TWDGT_ID!*/
  if (tbut) gtk_object_set_data(GTK_OBJECT(tbut),TWDGT_ID,(gpointer)name);

/* To remove.; don't know how to get adjustment from spin and vv. */
  gtk_object_set_data(GTK_OBJECT(spin),ADJUST_ID,(gpointer)adjustment);
  gtk_object_set_data(GTK_OBJECT(adjustment),PARENT_ID,(gpointer)spin);

// volgende moet weg
  gtk_object_set_data(GTK_OBJECT(adjustment),ADJUST_ID,(gpointer)spin);

/* Add function if specified */
  if (func) gtk_signal_connect(GTK_OBJECT(adjustment), "value-changed",
			GTK_SIGNAL_FUNC(func), name);

  gtk_widget_show(spin);
  if (tbut)
  {
    gtk_box_pack_start(GTK_BOX(tbut), spin, FALSE, FALSE, 0);
    gtk_widget_show(tbut);
    return tbut;
  }
  else
    return spin;
}

GtkWidget *Create_Scrollbar(char *name,     /* name of the button */
                            char hv,        /* hor/ver bar */
                            float min,float max,float page_size,
                            void func())    /* function to be executed */
{
  GtkWidget *scroll;
  GtkObject *adjust;

  adjust = gtk_adjustment_new(min,min,max,page_size/10.,page_size, page_size);
  if (hv=='v')
    scroll=gtk_vscrollbar_new((GtkAdjustment *)adjust);
  else
    scroll=gtk_hscrollbar_new((GtkAdjustment *)adjust);

  if (func) gtk_signal_connect(GTK_OBJECT(adjust), "value_changed",
			GTK_SIGNAL_FUNC(func), name);

  gtk_object_set_data(GTK_OBJECT(scroll),WDGT_ID,(gpointer)name);

/* To remove.; don't know how to get adjustment from scale and vv. */
  gtk_object_set_data(GTK_OBJECT(scroll),ADJUST_ID,(gpointer)adjust);
  gtk_object_set_data(GTK_OBJECT(adjust),PARENT_ID,(gpointer)scroll);

// volgende moet weg
  gtk_object_set_data(GTK_OBJECT(adjust),ADJUST_ID,(gpointer)scroll);

/*
  gtk_range_set_update_policy (GTK_RANGE (scroll), 
                                 GTK_UPDATE_CONTINUOUS);
  gtk_widget_show (scroll);
*/
  return scroll;
}

/*************************************
 * Add scrollbar(s);
 * hname!=NULL      -> add hor. scrollbar
 * vname!=NULL      -> add ver. scrollbar
 * hname=vname=NULL -> Add scrolled_window
 *************************************/
GtkWidget *Add_Scrollbars(GtkWidget *wi,char *hname,char *vname,void func())
{
  GtkWidget *wo,*wh=NULL,*wv=NULL;
  if ((!hname) && (!vname))
  {
    wo = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(wo),wi);
  }
  else
  {
    if (hname) 
    {
      wh=Create_Scrollbar(hname,'h', 0, 10, 10,func);
    }
    if (vname) 
    {
      wv=Create_Scrollbar(vname,'v', 0, 10, 10,func);
    }
    if (wv) wo=SPack(NULL,"h",wi,"ef1",wv,"1",NULL);
    if ((wh) && (wv))
    {
/* Try to make width hor. scrollbar same as widget.
   Quite ugly; is there a better way???
*/
      GtkWidget *whp,*spacing=Create_Label("    ");
      whp=SPack(NULL,"h",wh,"ef1",spacing,"1",NULL);
      wo=SPack(NULL,"v",wo,"ef1",whp,"1",NULL);
    }
    else
    {
      if (wh) wo=SPack(NULL,"h",wi,"ef1",wh,"1",NULL);
    }
    if (wh) gtk_object_set_data(GTK_OBJECT(wh),PARENT_ID,(gpointer)wo);
    if (wv) gtk_object_set_data(GTK_OBJECT(wv),PARENT_ID,(gpointer)wo);
    if (wh) gtk_object_set_data(GTK_OBJECT(wi),LAB_HBAR,(gpointer)wh);
    if (wv) gtk_object_set_data(GTK_OBJECT(wi),LAB_VBAR,(gpointer)wv);

  }
  gtk_object_set_data(GTK_OBJECT(wo),CHILD_ID,(gpointer)wi);
  return wo;
}

float Get_Adjust(GtkWidget *wdgt,char *id)
{
  GtkWidget *button;
  button=Find_Widget(wdgt,id);
  if (!button) return 0;
/* To change.; don't know how to get adjustment from scale. */
  button=gtk_object_get_data(GTK_OBJECT(button),ADJUST_ID);
  if (!button) return 0;
  return GTK_ADJUSTMENT(button)->value;
}

float Get_Adjustc(GtkWidget *wdgt,char *id)
{
  GtkWidget *button;
//  button=Find_Widget(wdgt,id);
  button=wdgt;
  if (!button) return 0;
/* To change.; don't know how to get adjustment from scale. */
  button=gtk_object_get_data(GTK_OBJECT(button),ADJUST_ID);
  if (!button) return 0;
  return GTK_ADJUSTMENT(button)->value;
}

/*************************************
 * Set adjust boundaries.
 * arg 1: lower
 * arg 2: upper
 * arg 3: step increment
 * arg 4: page size
 * arg 5: page increment
 *************************************/
int Set_Adjustbnd(GtkWidget    *wdgt,
                   char         *id,
                   char         *frmt,...)
{
  GtkWidget *button;
  GtkAdjustment *adj;
  int narg=0;
  va_list arg;
  char *p;
  int n=5,d=2;

  if (!(button=Find_Widget(wdgt,id))) return 0;

/* To change.; don't know how to get adjustment from scale. */
  if (!(adj=gtk_object_get_data(GTK_OBJECT(button),ADJUST_ID))) return 0;

  va_start(arg,frmt);
  p=frmt;

  while (*(p=get_frmt(p,&n,&d)))
  {
    narg++;
    if (*p=='f')
    {
      if (narg==1) adj->lower=va_arg(arg,double);
      if (narg==2) adj->upper=va_arg(arg,double);
      if (narg==3) adj->step_increment=va_arg(arg,double);
      if (narg==4) adj->page_size=va_arg(arg,double);
      if (narg==5) adj->page_increment=va_arg(arg,double);
    }
    if (*p=='d')
    {
      if (narg==1) adj->lower=va_arg(arg,int);
      if (narg==2) adj->upper=va_arg(arg,int);
      if (narg==3) adj->step_increment=va_arg(arg,int);
      if (narg==4) adj->page_size=va_arg(arg,int);
      if (narg==5) adj->page_increment=va_arg(arg,int);
    }
  }
  va_end(arg);
  gtk_adjustment_changed(adj);

  return 1;
}

int Set_Adjustbndc(GtkWidget    *wdgt,
                   char         *frmt,...)
{
  GtkWidget *button;
  GtkAdjustment *adj;
  int narg=0;
  va_list arg;
  char *p;
  int n=5,d=2;

//  if (!(button=Find_Local_Widget(wdgt,id))) return 0;
if (!(button=wdgt)) return 0;

/* To change.; don't know how to get adjustment from scale. */
  if (!(adj=gtk_object_get_data(GTK_OBJECT(button),ADJUST_ID))) return 0;

  va_start(arg,frmt);
  p=frmt;

  while (*(p=get_frmt(p,&n,&d)))
  {
    narg++;
    if (*p=='f')
    {
      if (narg==1) adj->lower=va_arg(arg,double);
      if (narg==2) adj->upper=va_arg(arg,double);
      if (narg==3) adj->step_increment=va_arg(arg,double);
      if (narg==4) adj->page_size=va_arg(arg,double);
      if (narg==5) adj->page_increment=va_arg(arg,double);
    }
    if (*p=='d')
    {
      if (narg==1) adj->lower=va_arg(arg,int);
      if (narg==2) adj->upper=va_arg(arg,int);
      if (narg==3) adj->step_increment=va_arg(arg,int);
      if (narg==4) adj->page_size=va_arg(arg,int);
      if (narg==5) adj->page_increment=va_arg(arg,int);
    }
  }
  va_end(arg);

  gtk_adjustment_changed(adj);

  return 1;
}

int Set_Adjust(GtkWidget    *wdgt,
               char         *id,
               char         *frmt,...)
{
  GtkWidget *button;
  GtkAdjustment *adj;
  float val;
  va_list arg;
  va_start(arg,frmt);

  if (strstr(frmt,"%f"))
    val=va_arg(arg,double);
  else
    val=va_arg(arg,int);
  va_end(arg);

  button=Find_Widget(wdgt,id);
  if (!button) return 0;

/* To change.; don't know how to get adjustment from scale. */
  adj=gtk_object_get_data(GTK_OBJECT(button),ADJUST_ID);
  if (!adj) return 0;
  gtk_adjustment_set_value(adj,val);
  return 1;
}

int Set_Adjustc(GtkWidget    *wdgt,
               char         *frmt,...)
{
  GtkWidget *button;
  GtkAdjustment *adj;
  float val;
  va_list arg;
  va_start(arg,frmt);
  if (strstr(frmt,"%f"))
    val=va_arg(arg,double);
  else
    val=va_arg(arg,int);
  va_end(arg);

//  button=Find_Local_Widget(wdgt,id);
button=wdgt;
  if (!button) return 0;
/* To change.; don't know how to get adjustment from scale. */
  adj=gtk_object_get_data(GTK_OBJECT(button),ADJUST_ID);
  if (!adj) return 0;
  gtk_adjustment_set_value(adj,val);
  return 1;
}

/*************************************
 * Set scroll and correct zoom variables.
 * width/height are size of picture, not of drawable!
 *************************************/
void Set_Scroll(GtkWidget *wnd,char *idh,int width ,int *ox,float *zx,
                               char *idv,int height,int *oy,float *zy)
{
  int pagex,pagey,tmp;
  float zx1,zy1;

/* Correct out-of-bounds */
// NIET naar buiten sturen! Veranderd 20-2-2011, zx of zy mag <1 zijn.
  zx1=MAX(*zx,1.);
  zy1=MAX(*zy,1.);

/* To prevent unnecessary redraw page MUST be at least 1 smaller than width!? */
  pagex=width/(zx1)-1; if (!pagex) pagex=1;
  pagey=height/(zy1)-1; if (!pagey) pagey=1;

  *ox=MAX(*ox,0);
  *ox=MIN(*ox,(width-pagex));
  *oy=MAX(*oy,0);
  *oy=MIN(*oy,(height-pagey));

/* For some reason first zoom around mouse pointer ('I') doesn't work
   correctly; ox/oy changes in Set_Adjustbnd (via func allback).
   So first save and then restore offsets.
*/
  tmp=*ox; // Save ox; changes via Set_Adjustbnd etc. in 'some way'
  Set_Adjustbnd(wnd,idh,"%d%d%d%d%d",0,width ,pagex/10,pagex,pagex);

  *ox=tmp; // R
  tmp=*oy;
  Set_Adjustbnd(wnd,idv,"%d%d%d%d%d",0,height,pagey/10,pagey,pagey);
  *oy=tmp;

  Set_Adjust(wnd,"hbar","%d",*ox);
  Set_Adjust(wnd,"vbar","%d",*oy);
}

/*************************************
 * Set scroll and correct zoom variables.
 * width/height are size of picture, not of drawable!
 *************************************/
void Set_Scrollc(GtkWidget *hadj,int width ,int *ox,float *zx,
                 GtkWidget *vadj,int height,int *oy,float *zy)
{
  int pagex,pagey,tmp;
/* Correct out-of-bounds */
  *zx=MAX(*zx,1.);
  *zy=MAX(*zy,1.);

/* To prevent unnecessary redraw page MUST be at least 1 smaller than width!? */
  pagex=width/(*zx)-1; if (!pagex) pagex=1;
  pagey=height/(*zy)-1; if (!pagey) pagey=1;
  *ox=MAX(*ox,0);
  *ox=MIN(*ox,(width-pagex));
  *oy=MAX(*oy,0);
  *oy=MIN(*oy,(height-pagey));

/* For some reason first zoom around mouse pointer ('I') doesn't work
   correctly; ox/oy changes in Set_Adjustbnd (via func allback).
   So first save and then restore offsets.
*/
  tmp=*ox; // Save ox; changes via Set_Adjustbnd etc. in 'some way'
  Set_Adjustbndc(hadj,"%d%d%d%d%d",0,width ,pagex/10,pagex,pagex);
  *ox=tmp; // R
  tmp=*oy;
  Set_Adjustbndc(vadj,"%d%d%d%d%d",0,height,pagey/10,pagey,pagey);
  *oy=tmp;
  Set_Adjustc(hadj,"%d",*ox);
  Set_Adjustc(vadj,"%d",*oy);
}

static GdkColor gdkclr_8216(GdkColor ci)
{
  GdkColor co;
  co.red  =(ci.red<<8)  +ci.red;
  co.green=(ci.green<<8)+ci.green;
  co.blue =(ci.blue<<8) +ci.blue;
  return co;
}

//Color selector.
//func:
//void func(GtkColorButton *widget, gpointer data)
// to get color (16 bits per color!):
//   use gtk_color_button_get_color(widget,&clr);
GtkWidget *Create_Colorsel(char *name,        /* name of the button */
                           GdkColor *color,
                           void func())       /* function to be executed */
{
  GtkWidget *button,*tbut;
  char *name1=(*name=='!'? name+1 : name);
  tbut=add_label(name);
  if (color)
  {
    GdkColor clr2= gdkclr_8216(*color);
    button = gtk_color_button_new_with_color(&clr2);
  }
  else
    button = gtk_color_button_new();

  if (tbut) gtk_box_pack_start(GTK_BOX(tbut), button, FALSE, FALSE, 0);

  gtk_color_button_set_title((GtkColorButton *)button,name1);
  if (func) gtk_signal_connect (GTK_OBJECT (button), "color-set",
			GTK_SIGNAL_FUNC(func), name);

  Add_WidgetID(button,name);
  gtk_widget_show(button);
  if (tbut)
  {
    gtk_widget_show(tbut);
    return tbut;
  }
  else
  {
    return button;
  }

  return button;
}

