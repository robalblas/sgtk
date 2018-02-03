/**************************************************
 * RCSId: $Id: buttons.c,v 1.3 2016/02/21 23:21:32 ralblas Exp $
 *
 * Button related functions.
 * Project: SGTK
 * Author: R. Alblas
 *
 * History: 
 * $Log: buttons.c,v $
 * Revision 1.3  2016/02/21 23:21:32  ralblas
 * _
 *
 * Revision 1.2  2016/01/09 20:44:55  ralblas
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
#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>
#include "sgtk.h"

/*************************************
 * Remove the widget-ID 
 * if button is destroyed
 *************************************/
void Remove_WidgetID(GtkWidget *w,gpointer data)
{

  free((char *)data);
}

/*************************************
 * Add the widget-ID 
 * May be done in 2 steps:
 *   widget=NULL -> allocate mem for ID and copy name; 
 *                  return pointer to allocated mem
 *   name=NULL   -> connect ID to widget
 *************************************/
char *Add_WidgetID(GtkWidget *widget,char *name)
{
  static char *iname;
  if (name)
  {
    iname=malloc(strlen(name)+2);
    strcpy(iname,name);
  }
  if (widget)
  {
    gtk_object_set_data(GTK_OBJECT(widget),WDGT_ID,(gpointer)iname);
    gtk_signal_connect(GTK_OBJECT(widget), "destroy",
		                      GTK_SIGNAL_FUNC(Remove_WidgetID), iname);
  }
  return iname;
}

/*************************************
 * Create a label
 * name: name of label
 *************************************/
GtkWidget *Create_Label(char *frmt,...)
{
  GtkWidget *button;
  char name[200],*name2=NULL;
  va_list arg;
  if (frmt)
  {
    va_start(arg,frmt);
    vsnprintf(name,199,frmt,arg);
    va_end(arg);
    name2=Add_WidgetID(NULL,name);
    button = gtk_label_new(name2);
    Add_WidgetID(button,NULL);
  }
  else
  {
    button = gtk_label_new(NULL);
  }
  gtk_widget_show(button);
  return button;
}

/*************************************
 * Create a press button
 *************************************/
GtkWidget *Create_Button(char *name,        /* name of the button */
                         void func())       /* function to be executed */
{
  GtkWidget *button;

  if ((*name!='!'))
  {
    name=Add_WidgetID(NULL,name);
    button = gtk_button_new_with_label(name);
    Add_WidgetID(button,NULL);
  }
  else
  {
    button = gtk_button_new();
    name=Add_WidgetID(button,name);
  }
  if (func) gtk_signal_connect (GTK_OBJECT (button), "clicked",
			GTK_SIGNAL_FUNC(func), name);

  gtk_widget_show(button);

  return button;
}

/*************************************
 * Create a check button
 *************************************/
GtkWidget *Create_Check(char    *name,      /* name of the button */
                        void     func(),    /* function to be executed */
                        gboolean clicked)   /* init state of button */
{
  GtkWidget *button;
  GtkWidget *tbut=NULL;

  if ((*name!='!') && (*name!='^'))
  {
    name=Add_WidgetID(NULL,name);
    button = gtk_check_button_new_with_label(name);
    Add_WidgetID(button,NULL);
  }
  else 
  {
    button = gtk_check_button_new();
    Add_WidgetID(button,name);
  }

  if (*name=='^')
  {
    tbut=add_label(name);
    gtk_box_pack_start(GTK_BOX(tbut), button, FALSE, FALSE, 0);
  }

  if (clicked) gtk_button_clicked(GTK_BUTTON(button));
  if (func) gtk_signal_connect(GTK_OBJECT (button), "released",/*"pressed",*/
		                            GTK_SIGNAL_FUNC(func),name);
  if (tbut)
  {
    gtk_widget_show_all(tbut);
    return tbut;
  }
  gtk_widget_show(button);
  return button;
}

#define TOGGLAB "TOGGLAB"
/*************************************
 * Create a toggle button
 *************************************/
GtkWidget *Create_Toggle(char     *name,     /* name of the button */    
                         void     func(),    /* function to be executed */
                         gboolean clicked)   /* init state of button */
{
  GtkWidget *button;
  char *p,*iname=malloc(strlen(name)+5);

  strcpy(iname,name);
  if ((p=strchr(iname,'/')) && (*(p+1) != '/'))
  {
    *p=0;
  }
  button = gtk_toggle_button_new_with_label(iname);
  free(iname);

  name=Add_WidgetID(button,name);
  if ((p=strchr(name,'/')) && (*(p+1) != '/'))
  {
    gtk_object_set_data(GTK_OBJECT(button),TOGGLAB,(gpointer)name);
  }

  if (clicked) gtk_button_clicked(GTK_BUTTON(button));

/* 'toggled': if button set using Set_Button it gives a double toggle!
   Consequence: using 'released': Function is NOT activated if
   toggle button is set/reset using Set_Button!
   This is fixed by adding gtk_button_released() function in Set_Button.
*/

  if (func) gtk_signal_connect(GTK_OBJECT(button), "released",/*"toggled",*/
			GTK_SIGNAL_FUNC(func), name);
  gtk_widget_show(button);
  return button;
}

gboolean Update_Togglelabel(GtkWidget *button)
{
  GtkLabel *label;
  char *dname;
  char *name1,*name2;
  gboolean ret=FALSE;

  if (GTK_BIN(button)->child) label=GTK_LABEL(GTK_BIN(button)->child);
  dname=gtk_object_get_data(GTK_OBJECT(button),TOGGLAB);

  if (!dname) return FALSE;

  name1=malloc(strlen(dname)+5);
  strcpy(name1,dname);
  if ((name2=strchr(name1,'/')))
  {
    *name2=0; name2++;
  }
  
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
  {
    if (name2) gtk_label_set_text(label,name2);
    ret=TRUE;
  }
  else
  {
    if (name1) gtk_label_set_text(label,name1);
    ret=FALSE;
  }
  free(name1);
  return ret;
}

/*************************************
 * Create a radio button
 *************************************/
GtkWidget *Create_Radio(char     *name,       /* name of the button */
                        gboolean reset,       /* start of new group */
                        void     func())      /* function to be executed */
{
  static GtkWidget *button;
  GSList *group;

  if ((reset) || (button==NULL))
    group=NULL;
  else
    group=gtk_radio_button_group(GTK_RADIO_BUTTON(button));

  if (*name!='!')
    button = gtk_radio_button_new_with_label(group,name);
  else
    button = gtk_radio_button_new(group);

  name=Add_WidgetID(button,name);

  if (func) gtk_signal_connect(GTK_OBJECT(button), "clicked",
			                    GTK_SIGNAL_FUNC(func), name);
  gtk_widget_show(button);

  return button;
}


/*************************************
 * Create a entry button
 * name: name of button=label string
 *   if starts with '^' => place above (default: left)
 *   if starts with '!' => don't show
 * func: function to execute (to handle button actions)
 *       If NULL ==> Set on non-editable 
 * frmt: format string to define text
 *************************************/
GtkWidget *Create_Entry(char *name,       /* name of the button */
                        void func(),      /* function to be executed */
                        char *frmt,...)   /* init string */
{
  GtkWidget *entry;
  GtkWidget *tbut;
  char str[200],*p;
  int nr_chars;

/* Get arguments, and make string */
  va_list arg;
  va_start(arg,frmt);
  vsprintf(str,frmt,arg);
  va_end(arg);

/* Determine # characters of button, BEFORE removing trailing spaces */
  nr_chars=strlen(str);

/* Remove trailing spaces */
  for (p=str+strlen(str)-1; *p==' '; p--) *p=0;

/* Only add label if name doesn't start with '!'. */
  tbut=add_label(name);

  entry=gtk_entry_new();



/* add some extra length */
  gtk_entry_set_text(GTK_ENTRY(entry),str);
#ifdef __GTK_20__
  gtk_entry_set_width_chars(GTK_ENTRY(entry),nr_chars);
#else
  {
    int size_button;
    int size_M;
  /* Determine needed size entry field */
    size_button=gdk_string_width(gtk_style_get_font(entry->style),str);
  /* Add extra width (equal to biggest char) for spaces in original string */
    size_M=gdk_char_width(gtk_style_get_font(entry->style),'M');
    if (nr_chars!=strlen(str))
      size_button=size_button+
           (size_M*(nr_chars-strlen(str)));

    size_button+=size_M;
    gtk_widget_set_usize (entry,size_button, -1);
  }
#endif
  if (tbut) gtk_box_pack_start(GTK_BOX(tbut), entry, FALSE, FALSE, 0);
  if (func)
    gtk_signal_connect(GTK_OBJECT (entry),"changed",
			GTK_SIGNAL_FUNC(func),name);
  else
    gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);

  Add_WidgetID(entry,name);

  gtk_widget_show(entry);
  if (tbut)
  {
    gtk_widget_show(tbut);
    return tbut;
  }
  else
  {
    return entry;
  }
}

GtkWidget *Create_Entry_compl(char *name,       /* name of the button */
                              int  func(),      /* function to be executed */
                              char *lijst[],int len,
                              char *frmt,...)   /* init string */
{
  GtkWidget *w,*entry;
  GtkEntryCompletion *completion;
  GtkListStore *model;
  GtkTreeIter iter;
  char str[300];
  int n;
  va_list arg;
  va_start(arg,frmt);
  vsnprintf(str,290,frmt,arg);
  va_end(arg);
  w=Create_Entry(name,NULL,str);
  if (*name=='!') entry=w;
  else            entry=Find_Widget(w,name);

  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);

  completion = gtk_entry_completion_new();
  gtk_entry_completion_set_text_column(completion, 0);
  gtk_entry_set_completion(GTK_ENTRY(entry), completion);
  g_signal_connect(G_OBJECT(completion), "match-selected",G_CALLBACK(func), NULL);
  model = gtk_list_store_new(1, G_TYPE_STRING);
  gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(model));

  for (n=0; n<len; n++)
  {
    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, 0, lijst[n],-1);
  }
  return w;
}

void Add_To_Compllist(GtkWidget *w,char *name,char *str)
{
  GtkWidget *entry;
  GtkEntryCompletion *completion;
  GtkListStore *model;
  GtkTreeIter iter;
  entry=Find_Widget(w,name);
  completion=gtk_entry_get_completion(GTK_ENTRY(entry));
  model=(GtkListStore *)gtk_entry_completion_get_model(completion); 
  gtk_list_store_append(model, &iter);
  gtk_list_store_set(model, &iter, 0, str,-1);
}



/*************************************
 * Adjust va_list, after doing a vsprintf.
 * Used in Create_ButtonArray.
 * *s: format string used within previous vsprintf.
 *************************************/
static void adjust_valist(va_list *ap,char *s)
{
  int n;
  while (*s)
  {
    if (*s=='%')
    {
      s++;
      while (*s)
      {
        if (*s=='f') { n=va_arg(*ap,double); break; }
        if (*s=='d') { n=va_arg(*ap,int); break; }
        if (*s=='x') { n=va_arg(*ap,int); break; }
        if (*s=='c') { n=va_arg(*ap,int); break; }
        if (*s=='s') { n=va_arg(*ap,int); break; }
        if (*s=='%') { break; }

        s++;
      }
    }
    s++;
  }
}


/*************************************
 * Create a array of buttons
 * frame_name: name of the array (if NULL then not packed)
 * func      : function to execute (to handle button actions)
 * nh        : amount of buttons in a row, so build row by row
 *             if <0: abs value = amount of buttons in a column, so build column by column
 * ...       : def. of 1 or more buttons (last=0)
 * Example:
 * w=Create_ButtonArray("Example",function,2,
 *                      TOGGLE,"but1",FALSE,
 *                      ENTRY,"first","%d",n,
 *                      ENTRY,"second","%f",f,
 *                      RADIO,"third","aa",
 *                      RADIO,"fourth","bb",
 *                      0);
 * Radio buttons are in 1 group.
 *************************************/
GtkWidget *Create_ButtonArray(char *frame_name,    /* name of array */
                              void func(),         /* function to be executed */
                              int nh, ...)         /* row length */
{
  TYPE_BUT type;
  GtkWidget *table,*tbut,*w;
  int h,v=0;
  int n;
  gboolean build_v=FALSE;
  gboolean stop=FALSE;
  gboolean reset_radio=TRUE;
  char *name;
  char frmt[100],*p;
  char entry_data[100];
  float f_val[4];
  int i_val[4];
  char tmp;
  va_list ap;
  va_start(ap,nh);
  if (!nh) nh=100;                    /* Everything in 1 row */
  if (nh<0)
  {
    nh*=-1;
    build_v=TRUE;
  }
  table=gtk_table_new(nh,1,FALSE);
  while (!stop)
  {
    for (h=0; h<nh; h++)
    {
      type=(int)va_arg(ap,int);
      tbut=NULL;

      if (!type) { stop=TRUE; break; }
      name=(char *)va_arg(ap,char *);

      if (type!=RADIO) reset_radio=TRUE;
      switch(type)
      {
        case LABEL:
          tbut=Create_Label(name);
        break;
        case LED:
          tbut=Create_Led(name,va_arg(ap,int));
        break;
        case BUTTON:
          tbut=Create_Button(name,func);
        break;
        case TOGGLE: 
          tbut=Create_Toggle(name,func,va_arg(ap,int));
        break;
        case NTOGGLE:
          n=va_arg(ap,int);   /* n= to prevent warnings */
        break;
        case CHECK:
          tbut=Create_Check(name,func,FALSE);
        break;
        case CHECKI:
          tbut=Create_Check(name,func,va_arg(ap,int));
        break;
        case RADIO:
          tbut=Create_Radio(name,reset_radio,func);
          reset_radio=FALSE;
        break;
        case RADIOs:
          tbut=Create_Radio(name,TRUE,func);
        break;
        case RADIOn:
          tbut=Create_Radio(name,FALSE,func);
        break;
        case ENTRY:
        {
          va_list apt;
          strcpy(frmt,(char *)va_arg(ap,char *));
          va_copy(apt,ap);
          vsprintf(entry_data,frmt,apt);
          adjust_valist(&ap,frmt);
          tbut=Create_Entry(name,func,entry_data);
          va_end(apt);
        }
        break;
        case ENTRY_NOFUNC:
        {
          va_list apt;
          strcpy(frmt,(char *)va_arg(ap,char *));
          va_copy(apt,ap);
          vsprintf(entry_data,frmt,apt);
          adjust_valist(&ap,frmt);
          tbut=Create_Entry(name,NULL,entry_data);
          va_end(apt);
        }
        break;
        case NENTRY:
          strcpy(frmt,(char *)va_arg(ap,char *));
          adjust_valist(&ap,frmt);
        break;
        case VSCALE: case HSCALE: case NSCALE:
          tmp=0;
          if (type==HSCALE) tmp='h';
          if (type==VSCALE) tmp='v';
          n=0;
          strcpy(frmt,(char *)va_arg(ap,char *));
          for (p=frmt; *p; p++)
          {
            if (*p=='d')
            {
              i_val[n]=va_arg(ap,int);
              f_val[n]=i_val[n];
              n++;
            }
            if (*p=='f')
            {
              f_val[n++]=va_arg(ap,double);
            }
          }
          if (strstr(frmt,"f"))
            tbut=Create_Scale(name,tmp,func,frmt,f_val[0],f_val[1],f_val[2]);
          else
            tbut=Create_Scale(name,tmp,func,frmt,i_val[0],i_val[1],i_val[2]);
        break;
        case SPIN: case NSPIN:
          tmp=0;
          n=0;
          strcpy(frmt,(char *)va_arg(ap,char *));
          for (p=frmt; *p; p++)
          {
            if ((*p=='d') || (*p=='x'))
            {
              i_val[n]=va_arg(ap,int);
              f_val[n]=i_val[n];
              n++;
            }
            if (*p=='f')
            {
              f_val[n++]=va_arg(ap,double);
            }
          }
          if (type==NSPIN) break;
          if (strstr(frmt,"f"))
            tbut=Create_Spin(name,func,frmt,f_val[0],f_val[1],f_val[2],f_val[3]);
          else
            tbut=Create_Spin(name,func,frmt,i_val[0],i_val[1],i_val[2],i_val[3]);
        break;
        default:
        break;
      }
      if ((!stop) && (tbut))
      {
        if (build_v)
          gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,0,0);
        else
          gtk_table_attach(GTK_TABLE(table),tbut,h,h+1,v,v+1,
                           GTK_FILL,GTK_FILL,0,0);
      }
    }
    v++;
  }
  va_end(ap);
  gtk_widget_show(table);
  if (frame_name)
    w=Pack(frame_name,'h',table,10,NULL);
  else
    w=table;

  return w;
}


/*************************************
 * Set or reset button in a window or menu 
 * Set option menu to a certain state. 
 * return: 0 if button not found, else 1
 * Note: With toggle button func connected 
 *       to it is NOT activated! (To fix.. Done; gtk_button_released())
 *************************************/
int Set_Button(GtkWidget    *button,        /* window containing button */
               char         *id,            /* ID of button to set/reset */
               int          state)          /* State (TRUE/FALSE or number) */
{
  if (id)
    button=Find_Widget(button,id);
  if (!button) return 0;

  if (GTK_IS_MENU_ITEM(button))
  {
/* Next line causes window to redraw. WHY?? I don't want this! */
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(button),state);
  }

  if (GTK_IS_OPTION_MENU(button))
  {
    gtk_option_menu_set_history(GTK_OPTION_MENU(button),state);
    gtk_menu_item_activate(GTK_MENU_ITEM(GTK_OPTION_MENU(button)->menu_item));
  }
  else if (GTK_IS_TOGGLE_BUTTON(button))
  {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),state); 
    gtk_button_released((GtkButton *)button);
#ifdef __GTK_20__
/* Check waarom zo; GTK_CHECK_MENU_ITEM(button))->active werkt niet in gtk20?? */
 //   gtk_widget_draw_default(button);
#else
    gtk_widget_draw_default(button);
#endif
    Update_Togglelabel(button);
  }
  else if (GTK_IS_BUTTON(button))
  {
    gtk_button_clicked(GTK_BUTTON(button)); 
  }
  return 1;
}


void Set_All_Buttons(GtkWidget *iwdgt,int state)
{
  GList *List_Childs;
  GtkWidget *wdgt;
  GList *node;
  List_Childs = gtk_container_children((GtkContainer *)iwdgt);
  for (node = List_Childs; node != NULL; node = node->next)
  {

    wdgt = (GtkWidget *)node->data;
    if (GTK_IS_TOGGLE_BUTTON(wdgt))
    {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wdgt),state); 
      gtk_button_released((GtkButton *)wdgt);
#ifndef __GTK_20__
      gtk_widget_draw_default(wdgt);
#endif
      Update_Togglelabel(wdgt);
    }
    
/* === CHECK CONTAINER === */
/* If requested widget is a container then search through all its children  */
    if (GTK_IS_CONTAINER(wdgt))
    {
      Set_All_Buttons(wdgt,state);
    }
  }
  if (List_Childs) g_list_free(List_Childs);
}

/*************************************
 * Get button status 
 * wdgt : window widget
 * id   : name of widget to get state
 * return: state
 *************************************/
gboolean Get_Button(GtkWidget    *button,
                    char         *id)
{
  if (id) button=Find_Widget(button,id);
  if (!button) return FALSE;

  if (GTK_IS_MENU_ITEM(button))
  {
    return (GTK_CHECK_MENU_ITEM(button))->active;
  }

  if (GTK_IS_BUTTON(button))
  {

/* NOTE Use 'released' signal for toggle button! */
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)); 
  }
  printf("buttons.c:  gtk_is_button verwacht... id=>%s<\n",id);
  return FALSE;
}

/*************************************
 * Get local button status 
 * wdgt :  container widget
 * id   : name of widget to get state
 * return: state
 *************************************/
gboolean Get_Local_Button(GtkWidget    *button,
                          char         *id)
{
  button=Find_Local_Widget(button,id);
  if (!button) return FALSE;
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)); 
}

/*************************************
 * Set local button status 
 * wdgt :  widget containing  the button to set
 * id   : name of widget to set
 * state: The state
 *************************************/
void Set_Local_Button(GtkWidget    *wdgt,
                      char         *id,
                      gboolean     state)
{
  GtkWidget *button;
  button=Find_Local_Widget(wdgt,id);
  if (!button) return;

  if (GTK_IS_MENU_ITEM(button))
  {
/* Next line causes window to redraw. WHY?? I don't want this! */
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(button),state);
  }
  else if (GTK_IS_TOGGLE_BUTTON(button))
  {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),state); 
  }
  else if (GTK_IS_BUTTON(button))
  {
    gtk_button_clicked(GTK_BUTTON(button)); 
  } 
}

/*************************************
 * Make button sensitive or insensitive 
 * wdgt : window widget
 * id   : name of widget to make sensitive/insensitive
 * state: TRUE (sensitive) or FALSE (insensitive)
 * return: 0 if button not found, else 1
 *************************************/
int Sense_Button(GtkWidget    *widget,
                 char         *id,
                 gboolean     state)
{
  GtkWidget    *button;
  button=Find_Top_Widget(widget,id);
  if (!button) button=Find_Top_Widget(Find_Parent_Window(widget),id);
  if (!button) button=Find_Widget(widget,id);  // menuitems
  if (!button) return 0;

  gtk_widget_set_sensitive(button,state);    
  
  if ((GTK_IS_TOGGLE_BUTTON(button)) && (!state))
  {
    gboolean b=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    if (b) Set_Button(button,id,b);
  }

  return 1;
}

/*************************************
 * Make button visible or invisible 
 * wdgt : window widget
 * id   : name of widget to make sensitive/insensitive
 * state: TRUE (sensitive) or FALSE (insensitive)
 * return: 0 if button not found, else 1
 *************************************/
int Show_Button(GtkWidget    *widget,
                char         *id,
                gboolean     state)
{
  GtkWidget    *button;
  button=Find_Top_Widget(widget,id);
  if (!button) button=Find_Top_Widget(Find_Parent_Window(widget),id);
  if (!button) button=Find_Widget(widget,id);  // menuitems
  if (!button) return 0;

  if (state)
    gtk_widget_show(button);
  else
    gtk_widget_hide(button);
  return 1;
}

/*************************************
 * TRUE if option-set 'id' is selected
 *************************************/
gboolean Get_Options(GtkWidget    *widget,
                    char         *name,
                    char         *id)
{
  char *opt=soption_menu_get_history(Find_Widget(widget,id));
  if ((opt) && (!strcmp(name,opt)))         /* suppress deselect */
    return TRUE;
  return FALSE;
}


/*************************************
 * TRUE if option 'req_name' of option-set 'id' is active
 *************************************/
gboolean Get_Option(GtkWidget    *widget,
                    char         *name,
                    char         *id,
                    char         *req_name)
{
  char *opt=soption_menu_get_history(Find_Widget(widget,id));
  if ((opt) && (!strcmp(name,opt)))         /* suppress deselect */
    if (!strcmp(name,req_name))
      return TRUE;
  return FALSE;
}

/*************************************
 * Set entry button text content 
 * wdgt: window widget
 * id: name of widget to find
 * frmt,...: new text 
 * return: 0 if button not found, else 1
 *************************************/
int Set_Entry(GtkWidget    *wdgt,
              char         *id,
              char         *frmt,...)
{
  GtkWidget *button;
  char str[300];
  va_list arg;
  va_start(arg,frmt);
  vsnprintf(str,290,frmt,arg);
  va_end(arg);

  button=Find_Widget(wdgt,id);
  if (!button) return 0;
  gtk_entry_set_text(GTK_ENTRY(button),str);
  return 1;
}

int Set_Local_Entry(GtkWidget    *wdgt,
              char         *id,
              char         *frmt,...)
{
  GtkWidget *button;
  char str[300];
  va_list arg;
  va_start(arg,frmt);
  vsnprintf(str,290,frmt,arg);
  va_end(arg);

  button=Find_Local_Widget(wdgt,id);
  if (!button) return 0;
  gtk_entry_set_text(GTK_ENTRY(button),str);
  return 1;
}

/*************************************
 * Set entry button text content 
 * Identical to SetEntry, but accepts plain string instead of format.
 *  (This is to pass special characters like '%'.)
 * wdgt: window widget
 * id: name of widget to find
 * str: new text 
 * return: 0 if button not found, else 1
 *************************************/
int Set_Entry1(GtkWidget    *wdgt,
              char          *id,
              char          *str)
{
  GtkWidget *button;

  button=Find_Widget(wdgt,id);
  if (!button) return 0;
  gtk_entry_set_text(GTK_ENTRY(button),str);
  return 1;
}

char *Get_Entry(GtkWidget *wdgt,char *id)
{
  GtkWidget *button;
  button=Find_Widget(wdgt,id);
  if (!button) return 0;
  return (char *)gtk_entry_get_text(GTK_ENTRY(button));
}

/* Tijdelijk weggehaald; moet anders voor gtk2.0?? */
#include "gtk_led.h"
/*************************************
 * Create a LED; init color=rgb
 * r,g,b: 0 ... 0xff
 *************************************/
GtkWidget *Create_Led(char *name,int rgb)
{
  GtkWidget *led,*led1,*iled,*tled=NULL;
  if ((name) && (*name!='!') && (*name!='^'))
  {
    led1=gtk_led_new_with_label(name);
    led=led1->parent;
  }
  else if ((name) && (*name=='^'))
  {
    led=gtk_led_new();
    led1=led;
    tled=add_label(name);
    if (tled) gtk_box_pack_start(GTK_BOX(tled), led, FALSE, FALSE, 0);
  }
  else
  {
    led=gtk_led_new();
    led1=led;
  }
  iled=gtk_object_get_user_data(GTK_OBJECT(led1));
  if (name) Add_WidgetID(iled,name);
  gtk_widget_show_all(led);

  Set_Led(iled,NULL,rgb);
  if (tled) return tled;
  return led;
}

/*************************************
 * Change LED color into color=rgb
 * r,g,b: 0 ... 0xff
 *************************************/
void Set_Led(GtkWidget *widget,char *name,int rgb)
{
  GdkColor clr;
  int r=((rgb)&0xf00)>>8;
  int g=((rgb)&0x0f0)>>4;
  int b=((rgb)&0x00f);
  if (!widget) return;

  clr.red  =(r<<12)+(r<<8)+(r<<4)+r;
  clr.green=(g<<12)+(g<<8)+(g<<4)+g;
  clr.blue =(b<<12)+(b<<8)+(b<<4)+b;
  if (name) widget=Find_Widget(widget,name);
  if (widget) 
    Set_Color(widget,GTK_RC_FG,&clr,&clr,&clr,&clr,&clr);
}
