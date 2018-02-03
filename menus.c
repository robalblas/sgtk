/**************************************************
 * RCSId: $Id: menus.c,v 1.3 2017/02/10 09:17:30 ralblas Exp $
 *
 * Menu related functions.
 * Project: SGTK
 * Author: R. Alblas
 *
 * History: 
 * $Log: menus.c,v $
 * Revision 1.3  2017/02/10 09:17:30  ralblas
 * _
 *
 * Revision 1.2  2015/12/29 22:12:05  ralblas
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

#include "sgtk.h"
#include "string.h"
#include <malloc.h>
#define WDGT_OL_ID "options_list"

/* ========== Button create functions ========== */
/*************************************
 * Create a press button menu 
 * name: name of button
 *   part after last '/' = text
 *   no  '/' => text=name
 * func: function to execute (to handle button actions)
 *************************************/
GtkWidget *CreateMenuButton(char *name,       /* name of the button */
                            void func())      /* function to be executed */
{
  GtkWidget *button;
  char *text;

  if ((text=strrchr(name,'/')))
    text++;
  else
    text=name;

  button=gtk_menu_item_new_with_label(text);
  gtk_object_set_data(GTK_OBJECT(button),WDGT_ID,(gpointer)name);
  if (func) gtk_signal_connect(GTK_OBJECT (button), "activate",
			                  GTK_SIGNAL_FUNC(func), name);
  gtk_widget_show(button);
  return button;
}

/*************************************
 * Create a check menu 
 * name: name of button
 *   part after last '/' = text
 *   no  '/' => text=name
 * func: function to execute (to handle button actions)
 *************************************/
GtkWidget *CreateMenuCheck(char *name,
                           void func())
{
  GtkWidget *button;
  char *text;

  if ((text=strrchr(name,'/')))
    text++;
  else
    text=name;

  button=gtk_check_menu_item_new_with_label(text);
  gtk_object_set_data(GTK_OBJECT(button),WDGT_ID,(gpointer)name);
  if (func) gtk_signal_connect(GTK_OBJECT(button),"toggled",
                                           GTK_SIGNAL_FUNC(func),name);
  gtk_widget_show(button);
  return button;
}


/*************************************
 * Create a radio button menu 
 * name: name of button
 *   part after last '/' = text
 *   no  '/' => text=name
 * reset: If true: Start a new group
 * func: function to execute (to handle button actions)
 *************************************/
GtkWidget *CreateMenuRadio(char     *name,
                           gboolean reset,
                           void     func())
{
  static GtkWidget *button;
  GSList *group=NULL;
  char *text;

  if ((text=strrchr(name,'/')))
    text++;
  else
    text=name;

  if ((reset) || (button==NULL))
    group=NULL;
  else
    group=gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(button));

  button=gtk_radio_menu_item_new_with_label(group,text);
  gtk_object_set_data(GTK_OBJECT(button),WDGT_ID,(gpointer)name);

  if (func) gtk_signal_connect (GTK_OBJECT(button),"activate",
                                           GTK_SIGNAL_FUNC(func),name);
  gtk_widget_show(button);
  return button;
}

/* ========== Main menu item functions ========== */
/*************************************
 * Create a menu item  
 * menu: main menu widget 
 * name: name of button
 *   part after last '/' = text
 *   no  '/' => text=name
 * func: function to execute (to handle button actions)
 *************************************/
GtkWidget *CreateMenuItem(GtkWidget *menubar,
                          TYPE_BUT  type,
                          char      *name,
                          void      func())
{
  GtkWidget *menuitem;
  static gboolean radio_reset=TRUE;

/* Create the button */
  switch(type)
  {
    case CHECK:
      menuitem=CreateMenuCheck(name,func);
      radio_reset=TRUE;
    break;
    case RADIO:
      menuitem=CreateMenuRadio(name,radio_reset,func);
      radio_reset=FALSE;
    break;
    default:
      menuitem=CreateMenuButton(name,func);
      radio_reset=TRUE;
    break;
  }

/* Add button to menu bar */
  gtk_menu_bar_append(GTK_MENU_BAR(menubar),menuitem);
  return menuitem;
}


/* ========== Sub menu item functions ========== */

/*************************************
 * Create a submenu item  
 * menu: submenu widget in which button is added
 * TYPE_BUT: type of button (BUTTON, CHECK, RADIO)
 * name: name of button
 *   part after last '/' = text
 *   no  '/' => text=name
 * func: function to execute (to handle button actions)
 * If menu==NUL: Only set radio_reset to TRUE.
 *************************************/
GtkWidget *CreateSubMenuItem(GtkWidget *menu,
                             TYPE_BUT  type,
                             char      *name,
                             void      func())
{
  GtkWidget *menuitem;
  static gboolean radio_reset=TRUE;
  if (!menu)
  {
    radio_reset=TRUE;
    return NULL;
  }

  if ((name) && (*name) && (*(name+strlen(name)-1)!='/'))
  {
/* submenu item */
    if (!strcmp(name,"tearoff"))
    {
/* Add tearoff */
      menuitem=gtk_tearoff_menu_item_new();
      gtk_widget_show(menuitem);
      radio_reset=TRUE;
    }
    else
    {
/* Add a submenu item */
      switch(type)
      {
        case CHECK:
          menuitem=CreateMenuCheck(name,func);
          radio_reset=TRUE;
        break;
        case TOGGLE:
/* No toggle menu */
        break;
        case RADIO:
          menuitem=CreateMenuRadio(name,radio_reset,func);
          radio_reset=FALSE;
        break;
        default:
          menuitem=CreateMenuButton(name,func);
          radio_reset=TRUE;
        break;
      }
    }
  }
  else
  {
/* Empty menu: Add just some (hor.) space in the menu bar */
    menuitem=gtk_menu_item_new();
    gtk_widget_show(menuitem);
    radio_reset=1;
  }

/* Add new item to menu menu */
  gtk_menu_append(GTK_MENU(menu),menuitem);
  return menuitem;
}

/*************************************
 * Create a submenu entry  
 * menu: main menu widget 
 * name: name of button
 *   part after last '/' = text
 *   no  '/' => text=name
 * func: function to execute (to handle button actions)
 *************************************/
GtkWidget *CreateBarSubMenu(GtkWidget *menu,
                            char      *name,
                            void      func())
{

  GtkWidget *menuitem;
  GtkWidget *submenu;

/* Create button and connect to menu */
  menuitem=CreateMenuButton(name,NULL);

  if (GTK_IS_MENU_BAR(menu))
    gtk_menu_bar_append(GTK_MENU_BAR(menu),menuitem);
  else
    gtk_menu_append(GTK_MENU(menu),menuitem);

/* Create new menu and connect to the menu button */
  submenu=gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),submenu);

/* Add tearoff */
  CreateSubMenuItem(submenu,0,"tearoff",NULL);

  return submenu;
}

#ifdef TOETEVOEGEN
char *ssub_menu_get_history(GtkWidget *widget,char *name)
{
  GtkWidget *submenu;
  submenu=GTK_MENU_ITEM(Find_Widget(widget,name))->submenu;
  if (submenu)
  {
    return gtk_object_get_data(GTK_OBJECT(submenu),WDGT_OL_ID);

  }
  return NULL;
}

static void funcx(GtkWidget *widget,gpointer data)
{
  char *str_selected=gtk_object_get_data(GTK_OBJECT(widget->parent),WDGT_OL_ID);
  strcpy(str_selected,(char *)data);
}


GtkWidget *Add_Submenuss(GtkWidget *widget,char *name,char *set,SLIST *sl)
{
  GtkWidget *submenu,*menu,*ssm;
  submenu=NULL;
  char *str_selected;
  submenu=GTK_MENU_ITEM(Find_Widget(widget,name))->submenu;
  if (str_selected=gtk_object_get_data(GTK_OBJECT(submenu),WDGT_OL_ID))
  {
    GList *node;
    GList *List_Childs;
    List_Childs = gtk_container_children((GtkContainer *)submenu);
    for (node = List_Childs; node != NULL; node = node->next)
    {
      gtk_widget_destroy((GtkWidget *)node->data);
    }
    if (List_Childs) g_list_free(List_Childs);

    free(str_selected);
    ssm=CreateSubMenuItem(submenu,0,"tearoff",NULL);
  }
  CreateSubMenuItem(NULL,0,NULL,NULL);  // reset radio
  
  str_selected=calloc(10,sizeof(char));
  gtk_object_set_data(GTK_OBJECT(submenu),WDGT_OL_ID,(gpointer)str_selected);
  strcpy(str_selected,sl->str);
  
  for (; sl; sl=sl->next)
  {
    ssm=CreateSubMenuItem(submenu,RADIO,sl->str,funcx);
  }

  if (set)
  {
    strcpy(str_selected,set);
    Set_Local_Button(submenu,set,TRUE);
  }
  return NULL;
}

#endif

/* ========== Main menu creation functions ========== */
int menu_hrchy_count(char *str)
{
  int n=0;
  if (!str) return 0;
  while (*str) { if (*str=='/') n++; str++; }
  return n;
}


#define MAX_MENU_HIER 5   /* max number of menu levels */
/*************************************
 * Create a menu   
 * window: window in which to add the menu 
 * ...: define menu buttons:
 * <name>, <function>, <type>
 *
 * Radio buttons are grouped automatically; one or more non-radio items
 * (button, separator etc.) behind a group of radio buttons will
 * put the next radio buttons in a new group.
 * Each submenu has a tearoff.
 *
 * Example:
 * Create_Menu(window,"/File"        ,NULL    ,0,        | submenu entry
 *                    "File/Open"    ,func1   ,BUTTON,   | submenu item
 *                    "File/"        ,NULL    ,0,        | submenu separator
 *                    "File/Quit"    ,func2   ,BUTTON,   | submenu item
 *                    ""             ,NULL    ,0,        | menu separator
 *                    "   Aap"       ,func    ,CHECK,    | menu item
 *                    "/Noot"        ,NULL    ,0,        | submenu entry
 *                    "/Noot/Mies"   ,NULL    ,0,        | sub-submenu entry
 *                    "Noot/Mies/Wim",func    ,RADIO,    | sub-submenu item
 *                    "Noot/Mies/Zus",func    ,RADIO,    | sub-submenu item
 *                     0);                               | end of arguments
 *
 * Each button can be catched by the Find_Widget function.
 * The name string is used for this.
 * E.g., to get the widget of button 'Quit':
 * widget=Find_Widget(window_widget,"File/Quit"); 
 *
 * Unfortunately, check and radio buttons immediately in the menu bar, 
 * need extra spaces in the name to prevent the knob to be on the name. 
 * This has to be done by the user (see above example, "   Aap")
 *
 *************************************/
GtkWidget *Create_Menu(GtkWidget *window,...)
{
  GtkWidget *menubar;
  GtkWidget *submenu[MAX_MENU_HIER];
  char *name;
  void *func;
  va_list ap;
  va_start(ap,window);

/* create new menu bar */
  menubar=gtk_menu_bar_new();
  gtk_widget_show(menubar);

  submenu[0]=menubar;
/* Loop through all arguments: Add all items */
  while (TRUE)
  {
    TYPE_BUT type;
    int hiercnt;

/* Hierarchical name of item */
    name=(char *)va_arg(ap,char *);
    if (!(int)name) break;

/* Function to activate if pressed */
    func=(int *)va_arg(ap,int *);

/* type of button */
    type=(int)va_arg(ap,int);

    hiercnt=menu_hrchy_count(name);
    if (hiercnt >= MAX_MENU_HIER)
    {
      fprintf(stderr,"ERROR in CreateMenu: Too deep: %s!\n",name);
      return NULL;
    }

    if (*name=='/')
    {
/* Create entry of submenu (or sub-sub menu) */
      if (hiercnt < 1)
      {
        fprintf(stderr,"ERROR in CreateMenu: Missing '/': %s!\n",name);
        return NULL;
      }
      submenu[hiercnt]=CreateBarSubMenu(submenu[hiercnt-1],name+1,func);
    }
    else if (strrchr(name,'/'))
    {
/* Create submenu item (or sub-submenu item) */
      CreateSubMenuItem(submenu[hiercnt],type,name,func);
    }
    else
    {
/* Create menu item */
      CreateMenuItem(menubar,type,name,func);
    }
  }
  va_end(ap);
  return menubar;
}


#ifdef __GTK_20__
/*************************************
 * Changes for GTK2.0.
 * Opschonen; dit is vies gedaan!
 * Parallelle optielijst  omdat gesel. optie niet 
 * uit combo gehaald kan worden.
 * Dit kan wel vanaf versie 2.6, is nog niet geinst.
 * Zie soption_menu_get_history()!
 *************************************/
#define MAXSIZE_OPTLIST 50
typedef struct option_list
{
  char s[MAXSIZE_OPTLIST][MAXSIZE_OPTLIST];
  int n;
} OPT_LIST;

void Remove_WidgetID(GtkWidget *w,gpointer data);

/*************************************
 * Create a options menu 
 * name               : name of menu 
 * func               : function to execute 
 * set                : init. set of options menu 
 * ...                : list of item strings, last = 0 
 *************************************/
GtkWidget *Create_Optionmenu(char *name,        /* name of clist */
                             void func(),       /* function to be executed */
                             int set,           /* init. state */
                             ...)               /*  */
{
  GtkWidget *omenu;
  GtkWidget *tbut;
  GSList *group;
  char *str;
  va_list ap;
  OPT_LIST *opt_list;

  va_start(ap,set);
  group=NULL;
  name=Add_WidgetID(NULL,name);

/* Only add label if name doesn't start with '!'. */
  tbut=add_label(name);
  omenu=gtk_combo_box_new_text();
  opt_list=calloc(1,sizeof(OPT_LIST));
  gtk_object_set_data(GTK_OBJECT(omenu),WDGT_OL_ID,(gpointer)opt_list);
  gtk_signal_connect(GTK_OBJECT(omenu), "destroy",
		                    GTK_SIGNAL_FUNC(Remove_WidgetID), opt_list);
  if (tbut) gtk_box_pack_start(GTK_BOX(tbut), omenu, FALSE, FALSE, 0);
  Add_WidgetID(omenu,NULL);
//  gtk_object_set_data(GTK_OBJECT(omenu),WDGT_ID,(gpointer)name);

  do
  {
    str=(char *)va_arg(ap,char *);
    if (str==0)  break;
    {
      OPT_LIST *ol=gtk_object_get_data(GTK_OBJECT(omenu),WDGT_OL_ID);
      if (ol->n<MAXSIZE_OPTLIST)
      {
        strncpy(ol->s[ol->n],str,MAXSIZE_OPTLIST);
        gtk_combo_box_append_text(GTK_COMBO_BOX(omenu),str);
      }
      else
      {
        gtk_combo_box_append_text(GTK_COMBO_BOX(omenu),"(too much items)");
        fprintf(stderr,"ERROR: TOO MUCH Option items! Ignored %s\n",str);
      }
      ol->n++;
    }

    if (func)
    {
      gtk_signal_connect(GTK_OBJECT(omenu),
                         "changed",
                         GTK_SIGNAL_FUNC(func),	
    	                 (gpointer)str);
    }
  } while (str);
  va_end(ap);

  if (GTK_IS_COMBO_BOX(omenu))
    gtk_combo_box_set_active(GTK_COMBO_BOX(omenu),set);


  if (tbut)
  {
    gtk_widget_show(tbut);
    return tbut;
  }
  else
    return omenu;
}

/*************************************
 * Add/change options of options menu 
 * name               : name of menu 
 * func               : function to execute 
 * set                : init. set of options menu 
 * lab                : 2D string [nrs][lens]
 * nrs                : # items
 * lens               : length string 
 *************************************/
GtkWidget *Add_Options(GtkWidget *widget,char *name,void func(),
                       int set,char **lab,int nrs,int lens)
{
  GtkWidget *omenu=Find_Widget(widget,name);
  char *str;
  int i;

  if (!omenu) omenu=widget;      /* widget=omenu -> Find_Widget fails... */

  for (i=0; i<nrs; i++)
  {
    if (lens)
      str=(char *)lab+i*lens;
    else
      str=lab[i];

    {
      OPT_LIST *ol=gtk_object_get_data(GTK_OBJECT(omenu),WDGT_OL_ID);
      if (ol->n<MAXSIZE_OPTLIST)
      {
        strncpy(ol->s[ol->n],str,MAXSIZE_OPTLIST);
        gtk_combo_box_append_text(GTK_COMBO_BOX(omenu),str);
      }
      else
      {
        gtk_combo_box_append_text(GTK_COMBO_BOX(omenu),"(too much items)");
        fprintf(stderr,"ERROR: TOO MUCH Option items! Ignored %s\n",str);
      }

      if (func)
      {
        gtk_signal_connect(GTK_OBJECT(omenu),
                           "changed",
                           GTK_SIGNAL_FUNC(func),	
    	                   (gpointer)str);
      }

      ol->n++;
    }
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(omenu),set);
  return NULL;
}

gboolean Has_Option(GtkWidget *widget,char *m_name,char *o_name)
{
  int n;
  GtkWidget *omenu=Find_Widget(widget,m_name);
  OPT_LIST *ol;
  if (!omenu) return FALSE;
  ol=gtk_object_get_data(GTK_OBJECT(omenu),WDGT_OL_ID);
  if (!ol) return FALSE;
  for (n=0; n<ol->n; n++) if (!strcmp(ol->s[n],o_name)) return TRUE;
  return FALSE;
}

void Remove_Options(GtkWidget *omenu,int start)
{
  int i;
  OPT_LIST *ol=gtk_object_get_data(GTK_OBJECT(omenu),WDGT_OL_ID);
  if (!ol) return;
  for (i=start; i<ol->n; i++)
  {
    //gtk_combo_box_remove_text(,....0); geeft problemen bij wissen/opnieuw opbouwen!
    gtk_combo_box_remove_text(GTK_COMBO_BOX(omenu),start); // first item gives p
    if (i<MAXSIZE_OPTLIST) *ol->s[i]=0;
  }
  ol->n=start;
}

#ifdef XXX
GtkWidget *Add_Optionss(GtkWidget *widget,char *name,void func(),
                        int set,SLIST *sl)
{
  GtkWidget *omenu=Find_Widget(widget,name);
  char *str;
  
  Remove_Options(omenu,0);
  for (; sl; sl=sl->next)
  {
    str=sl->str;
    {
      OPT_LIST *ol=gtk_object_get_data(GTK_OBJECT(omenu),WDGT_OL_ID);
      if (ol->n<MAXSIZE_OPTLIST)
      {
        strncpy(ol->s[ol->n],str,MAXSIZE_OPTLIST);
        gtk_combo_box_append_text(GTK_COMBO_BOX(omenu),str);
      }
      else
      {
        fprintf(stderr,"ERROR: TOO MUCH Option items! Ignored %s\n",str);
        gtk_combo_box_append_text(GTK_COMBO_BOX(omenu),"(too much items)");
      }
      ol->n++;
    }
    gtk_widget_show(omenu);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(omenu),set);
  return NULL;
}

#endif

/* Get string of selected option */
char *soption_menu_get_history(GtkWidget *omenu)
{
  if (omenu)
  {
    int n=gtk_combo_box_get_active(GTK_COMBO_BOX(omenu));
    OPT_LIST *ol=gtk_object_get_data(GTK_OBJECT(omenu),WDGT_OL_ID);
    return ol->s[n];
//    return gtk_combo_box_get_active_text(omenu); // in 2.6; str wordt gealloc.!
  }
  return NULL;
}

void Set_Optionsmenu(GtkWidget *widget,char *label,int nr)
{
#ifdef WEGVOORGTK20
  static gboolean run_flag;
#endif
  if ((widget=Find_Widget(widget,label)))
  {
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget),nr);
    
/* Set a flag, in case gtk_menu_item_activate activates Set_Optionsmenu again */
#ifdef WEGVOORGTK20
    if (run_flag) return; /* gtk_menu_item_activate already busy; skip! */
    run_flag=TRUE;   /* set flag */
    gtk_menu_item_activate(GTK_MENU_ITEM(GTK_OPTION_MENU(widget)->menu_item));
    run_flag=FALSE;  /* Done; reset flag */
#endif
  }
}

int Get_Optionsmenu(GtkWidget *widget,char *label)
{
  if (!widget) return 0;
  widget=Find_Widget(widget,label);
  if (!GTK_IS_COMBO_BOX(widget)) return 0;

  return gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
}

#else /* gtk-1.2 */

/*************************************
 * Create a options menu 
 * name               : name of menu 
 * func               : function to execute 
 * set                : init. set of options menu 
 * ...                : list of item strings, last = 0 
 *************************************/
GtkWidget *Create_Optionmenu(char *name,        /* name of clist */
                             void func(),       /* function to be executed */
                             int set,           /* init. state */
                             ...)               /*  */
{
  GtkWidget *omenu,*menu,*menuitem=NULL,*tbut;
  GSList *group;
  char *str;
  va_list ap;
  va_start(ap,set);
  group=NULL;
  name=Add_WidgetID(NULL,name);

/* Only add label if name doesn't start with '!'. */
  tbut=add_label(name);

  omenu=gtk_option_menu_new();
  menu=gtk_menu_new();
  if (tbut) gtk_box_pack_start(GTK_BOX(tbut), omenu, FALSE, FALSE, 0);
//  gtk_object_set_data(GTK_OBJECT(omenu),WDGT_ID,(gpointer)name);
  Add_WidgetID(omenu,NULL);

  do
  {
    str=(char *)va_arg(ap,char *);
    if (str==0)  break;
    menuitem=gtk_radio_menu_item_new_with_label(group,str);
    gtk_object_set_data(GTK_OBJECT(menuitem),WDGT_ID,(gpointer)str);

    group=gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(menuitem));
    gtk_menu_append(GTK_MENU(menu), menuitem);
    gtk_widget_show(menuitem);

    if (func)
    {
      gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                         GTK_SIGNAL_FUNC(func),
		         (gpointer)str);
    }
  } while (str);
  va_end(ap);

  gtk_option_menu_set_menu(GTK_OPTION_MENU(omenu),menu);
  gtk_widget_show(omenu);
  gtk_option_menu_set_history(GTK_OPTION_MENU(omenu),set);

/* Needed for setting default value.
  !!! NOTE!!! Will activate 'func' at a moment that not everything is ready!
*/
  if (menuitem)
    gtk_menu_item_activate(GTK_MENU_ITEM(GTK_OPTION_MENU(omenu)->menu_item));

  if (tbut)
  {
    gtk_widget_show(tbut);
    return tbut;
  }
  else
    return omenu;
}

/*************************************
 * Add/change options of options menu 
 * name               : name of menu 
 * func               : function to execute 
 * set                : init. set of options menu 
 * lab                : 2D string [nrs][lens]
 * nrs                : # items
 * lens               : length string 
 * If lens=0 -> lab=array of pointers to strings
 *************************************/
GtkWidget *Add_Options(GtkWidget *widget,char *name,void func(),
                       int set,char **lab,int nrs,int lens)
{
  GtkWidget *omenu=Find_Widget(widget,name);
  GtkWidget *menuitem,*menu;
  GSList *group=NULL;
  char *str;
  int i;

  if (!omenu) omenu=widget;      /* widget=omenu -> Find_Widget fails... */
  menu=gtk_menu_new();

  for (i=0; i<nrs; i++)
  {
    if (lens)
      str=(char *)lab+i*lens;
    else
      str=lab[i];
    str=Add_WidgetID(NULL,str);
    menuitem=gtk_radio_menu_item_new_with_label(group,str);
    Add_WidgetID(menuitem,NULL);

    group=gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(menuitem));
    gtk_menu_append(GTK_MENU(menu), menuitem);
    gtk_widget_show(menuitem);

    if (func)
    {
      gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                         GTK_SIGNAL_FUNC(func),
		         (gpointer)str);
    }
  }
  gtk_option_menu_remove_menu(GTK_OPTION_MENU(omenu));
  gtk_option_menu_set_menu(GTK_OPTION_MENU(omenu),menu);
  gtk_widget_show(omenu);
  gtk_option_menu_set_history(GTK_OPTION_MENU(omenu),set);
  gtk_menu_item_activate(GTK_MENU_ITEM(GTK_OPTION_MENU(omenu)->menu_item));
  return menu;
}

#ifdef XXX
// bijgevoegd na aanpassen gtk20, mar SLIST niet gedef...
GtkWidget *Add_Optionss(GtkWidget *widget,char *name,void func(),
                        int set,SLIST *sl)
{
  GtkWidget *omenu=Find_Widget(widget,name);
  GtkWidget *menuitem,*menu;
  GSList *group=NULL;
  char *str;
  gtk_option_menu_remove_menu(GTK_OPTION_MENU(omenu));
  if (!sl)
  {
    if (omenu) gtk_widget_hide(omenu);
    return menu;
  }
  else
  {
    if (omenu) gtk_widget_show(omenu);
  }
  
  if (!omenu) omenu=widget;      /* widget=omenu -> Find_Widget fails... */
  menu=gtk_menu_new();

  for (; sl; sl=sl->next)
  {
    str=sl->str;
    str=Add_WidgetID(NULL,str);
    menuitem=gtk_radio_menu_item_new_with_label(group,str);
    Add_WidgetID(menuitem,NULL);

    group=gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(menuitem));
    gtk_menu_append(GTK_MENU(menu), menuitem);
    gtk_widget_show(menuitem);

    if (func)
    {
      gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                         GTK_SIGNAL_FUNC(func),
		         (gpointer)str);
    }
  }

  gtk_option_menu_set_menu(GTK_OPTION_MENU(omenu),menu);
  gtk_widget_show(omenu);
  gtk_option_menu_set_history(GTK_OPTION_MENU(omenu),set);
  gtk_menu_item_activate(GTK_MENU_ITEM(GTK_OPTION_MENU(omenu)->menu_item));
  return menu;
}

#endif

/* Get string of selected option */
char *soption_menu_get_history(GtkWidget *omenu)
{
  if (omenu)
  {
    GtkWidget *menu, *menu_item;
    menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(omenu));
    menu_item = GTK_OPTION_MENU(omenu)->menu_item;
    if (menu)
    {
      return gtk_object_get_data(GTK_OBJECT(menu_item),WDGT_ID);
    }
  }
  return NULL;
}



void Set_Optionsmenu(GtkWidget *widget,char *label,int nr)
{
  static gboolean run_flag;
  if ((widget=Find_Widget(widget,label)))
  {
    gtk_option_menu_set_history(GTK_OPTION_MENU(widget),nr);

/* Set a flag, in case gtk_menu_item_activate activates Set_Optionsmenu again */
    if (run_flag) return; /* gtk_menu_item_activate already busy; skip! */
    run_flag=TRUE;   /* set flag */
    gtk_menu_item_activate(GTK_MENU_ITEM(GTK_OPTION_MENU(widget)->menu_item));
    run_flag=FALSE;  /* Done; reset flag */
  }
}


/* get # selected, Counterpart of gtk_option_menu_set_history */
int option_menu_get_history(GtkWidget *omenu)
{
  GtkWidget *menu, *menu_item;
  GList *children, *child;

  if (omenu)
  {
    menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(omenu));
    menu_item = GTK_OPTION_MENU(omenu)->menu_item;
    if (menu)
    {
      children = GTK_MENU_SHELL(menu)->children;
      child = g_list_find(children, menu_item);
      if (child)
      {
        return g_list_position(children, child);
      }
    }
  }
  return -1;
}



int Get_Optionsmenu(GtkWidget *widget,char *label)
{
  if (!widget) return 0;
  return option_menu_get_history(Find_Widget(widget,label));
}

#endif    /* gtk-1.2 */

/*************************************
 * Create a toolbar  
 * func               : function to execute 
 * ...                : list of item strings, last = 0 
 *************************************/
GtkWidget *Create_Toolbar(void func(),    /* function to be executed */
                          ...)            /*  */
{
  GtkWidget *toolbar,*toolbar_item;
  char *str1,*str2;
  va_list ap;

#ifdef __GTK_20__
  toolbar=gtk_toolbar_new();
#else
  toolbar=gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL,GTK_TOOLBAR_TEXT);
#endif
  va_start(ap,func);
  do
  {
    str1=(char *)va_arg(ap,char *);
    if (str1==0)  break;
    str2=(char *)va_arg(ap,char *);
    toolbar_item=gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                         str1,str2,NULL,NULL,func,str1);
  } while (str2);
  va_end(ap);
  gtk_widget_show(toolbar);
  return toolbar;
}

/*************************************
 * Create a notebook
 * tab_pos: position of tabs (GTK_POS_TOP,GTK_POS_BOTTOM,
                              GTK_POS_LEFT,GTK_POS_RIGHT)
 * ...       : def. of 1 or more items (last=0)
 * func(GtkWidget *widget,gpointer data): data =pointer frame1/2 etc.
 * Example:
 * w=Create_Notebook(func,GTK_POS_TOP,
 *                   "tab_1",frame1,
 *                   "tab_2",frame2,
 *                   NULL);
 *************************************/
GtkWidget *Create_Notebook(void func(),int tab_pos,...)
{
  GtkWidget *nb;
  GtkWidget *label;
  GtkWidget *cnt;
  char *name;
  va_list ap;
  va_start(ap,tab_pos);

  nb=gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(nb),tab_pos);

  while (TRUE)
  {
    name=(char *)va_arg(ap,char *);
    if (name==NULL) break;
    cnt=(GtkWidget *)va_arg(ap,char *);
    if (cnt==NULL) break;
 
/* To add special widgets easily into notebook...*/
    if (GTK_IS_CLIST(cnt)) cnt=cnt->parent;
#ifndef __GTK_20__
    if (GTK_IS_TEXT(cnt)) cnt=cnt->parent;
#endif

    label=gtk_label_new(name);
    gtk_notebook_append_page((GtkNotebook *)nb,cnt,label);
  }
  va_end(ap);

  if (func) gtk_signal_connect(GTK_OBJECT (nb), "switch-page",
			                  GTK_SIGNAL_FUNC(func), name);

  gtk_widget_show(nb);
  return nb;
}
