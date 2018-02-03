/**************************************************
 * RCSId: $Id: listtree.c,v 1.2 2015/11/15 23:23:24 ralblas Exp $
 *
 * Lists and trees.
 * Project: SGTK
 * Author: R. Alblas
 *
 * History: 
 * $Log: listtree.c,v $
 * Revision 1.2  2015/11/15 23:23:24  ralblas
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
// OOPS, has mem leak! See row=(char **)calloc
/*************************************
 * Create a clist 
 * funcsel   : function executed if item selected 
 * funcunsel : function executed if item deselected 
 * funccolsel : function executed if col header selected 
 * ncol      : amount columns
 * ...       : def. of 1 or more rows (last=0):
 *             "title_col1",width_col1,"title_col2",width_col2,...
 *             "row1_col1","row1_col2",.....
 *             "row2_col1","row2_col2",.....
 *             ...........
 *             0
 * Example:
 * w=Create_Clist("Example",func1,func2,func3,3,
 *                 "title col1",10,"title col2",10,"title col3",20,
 *                 "row1_col1", "row1_col2", "row1_col3",
 *                  0);
 *************************************/
GtkWidget *Create_Clist(char *name,        /* name of clist */
                        void funcsel(),    /* function to be executed if sel*/
                        void funcunsel(),  /* function to be executed if desel*/
                        void funccolsel(), /* function to be executed if colsel*/
                        int ncol, ...)     /* # columns */
{
  GtkWidget *clist;
  GtkWidget *scroll_win;
  int col;
  int w;
  char **row;
  char *str;
  gboolean stop=FALSE;
  va_list ap;
  va_start(ap,ncol);

/* Create clist */
  scroll_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
  clist = gtk_clist_new(ncol);
  gtk_object_set_data(GTK_OBJECT(clist),WDGT_ID,(gpointer)name);
  gtk_container_add (GTK_CONTAINER (scroll_win), clist);

/* Add function to clist */
  if (funcsel)
  {

    gtk_signal_connect(GTK_OBJECT(clist), "select_row",
		       GTK_SIGNAL_FUNC(funcsel),
		       NULL);
  }
  if (funcunsel)
  {

    gtk_signal_connect(GTK_OBJECT(clist), "unselect_row",
		       GTK_SIGNAL_FUNC(funcunsel),
		       NULL);

  }

  if (funccolsel)
  {
    gtk_signal_connect(GTK_OBJECT(clist), "click_column",
  		       GTK_SIGNAL_FUNC(funccolsel),
		       NULL);
  }

/* Add title and define column width */
  for (col=0; col<ncol; col++)
  {
    str=(char *)va_arg(ap,char *);
    w=(int)va_arg(ap,int);
    gtk_clist_set_column_width (GTK_CLIST(clist), col, w*8);
    if (str)
    {
      gtk_clist_set_column_title(GTK_CLIST(clist),col,str);
      gtk_clist_column_titles_show(GTK_CLIST(clist));
      gtk_clist_column_titles_passive(GTK_CLIST(clist));
    }
  }

/* Add rows */
  row=(char **)calloc(sizeof(char *),ncol);
  do
  {
    for (col=0; col<ncol; col++)
    {
      str=(char *)va_arg(ap,char *);
      if (str==0) { stop=TRUE; break; }
      *(row+col)=str;
    }
    if (!stop) gtk_clist_append(GTK_CLIST(clist), row);

  } while (!stop);

  va_end(ap);
  free(row);

  gtk_clist_set_shadow_type (GTK_CLIST(clist), GTK_SHADOW_OUT);
/* ??? Gives core dump! (Just GTK_SELECTION_BROWSE...) at clist_append */
/*
  gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_BROWSE);
*/
  gtk_widget_show(scroll_win);
  gtk_widget_show(clist);
  if (funccolsel)
    gtk_clist_column_titles_active(GTK_CLIST(clist));
  return clist;
}

/*==============================================================*
 * get first selected item
 * Example: 
 * for (gsl=get_selected_first(clist); gsl; gsl=gsl->next)
 *==============================================================*/
GList *get_selected_first(GtkCList *clist)
{
  if (!clist) return NULL;
  return clist->selection;
}

/*==============================================================*
 * Double-list related functions.
 * The double-list has a 'Add' and 'Remove' function to move
 * items from one list to another.
 * (To be done: Buttons could be changed into 'drag&drop'.)
 *==============================================================*/
#define Dlist_Maxcol 10
#define Addbut "Add ==>"
#define Rembut "<== Remove"
#define DLIST_ID "Dlist_ID"
typedef struct
{
  gchar *Dlist_text[Dlist_Maxcol];
  int Dlist_row;
  int Dlist_ncol;
} Dlist_data;


void Dlist_Selleft(GtkWidget      *clist,
                   gint            row,
                   gint            column,
                   GdkEventButton *event,
                   gpointer        data)
{
  Dlist_data *dd=gtk_object_get_data(GTK_OBJECT(clist),DLIST_ID);
  int col;
  if (!dd) return;

  for (col=0; col<dd->Dlist_ncol; col++)
    gtk_clist_get_text(GTK_CLIST(clist), row, col, &dd->Dlist_text[col]);
  dd->Dlist_row=row;
}

void Dlist_Selright(GtkWidget      *clist,
                    gint            row,
                    gint            column,
		    GdkEventButton *event,
                    gpointer        data)
{
  Dlist_data *dd=gtk_object_get_data(GTK_OBJECT(clist),DLIST_ID);
  int col;
  if (!dd) return;

  for (col=0; col<dd->Dlist_ncol; col++)
    gtk_clist_get_text(GTK_CLIST(clist), row, col, &dd->Dlist_text[col]);
  dd->Dlist_row=row;
}

void Dlist_Unselleft(GtkWidget      *clist,
                     gint            row,
                     gint            column,
                     GdkEventButton *event,
                     gpointer        data)
{
  Dlist_data *dd=gtk_object_get_data(GTK_OBJECT(clist),DLIST_ID);
  int col;
  if (!dd) return;

  for (col=0; col<dd->Dlist_ncol; col++)
    dd->Dlist_text[col]=NULL;
  dd->Dlist_row=row;
}

void Dlist_Unselright(GtkWidget      *clist,
                      gint            row,
                      gint            column,
                      GdkEventButton *event,
                      gpointer        data)
{
  Dlist_data *dd=gtk_object_get_data(GTK_OBJECT(clist),DLIST_ID);
  int col;
  if (!dd) return;

  for (col=0; col<dd->Dlist_ncol; col++)
    dd->Dlist_text[col]=NULL;
  dd->Dlist_row=row;
}



void Dlist_Commands(GtkWidget *widget, gpointer data)
{
  GtkWidget *wl=Find_Widget(widget,Dlist_LeftID);
  GtkWidget *wr=Find_Widget(widget,Dlist_RightID);
  Dlist_data *ddl,*ddr;
  char *name=(char *)data;
  if (!wl) return;

  ddl=gtk_object_get_data(GTK_OBJECT(wl),DLIST_ID);
  ddr=gtk_object_get_data(GTK_OBJECT(wr),DLIST_ID);
  if (!ddl) return;
  if (!ddr) return;

  if (!strcmp(name,Addbut))
  {
    if (!ddl->Dlist_text[0]) return;
    gtk_clist_append(GTK_CLIST(wr), ddl->Dlist_text);
    gtk_clist_remove(GTK_CLIST(wl), ddl->Dlist_row);
    ddl->Dlist_text[0]=NULL;
  }
  if (!strcmp(name,Rembut))
  {
    if (!ddr->Dlist_text[0]) return;
    gtk_clist_append(GTK_CLIST(wl), ddr->Dlist_text);
    gtk_clist_remove(GTK_CLIST(wr), ddr->Dlist_row);
    ddr->Dlist_text[0]=NULL;
  }
}


GtkWidget *Create_Dlist(char *name,void funccolsel(),...)        /* name of clist */
{
  GtkWidget *wl,*wr,*wa,*wb,*wc,*wz;
  Dlist_data *dlist_datal;
  Dlist_data *dlist_datar;
  va_list ap;
  char *title[Dlist_Maxcol];
  int width[Dlist_Maxcol];
  int ncol=0;
  va_start(ap,funccolsel);
  while (ncol<Dlist_Maxcol)
  {
    title[ncol]=(char *)va_arg(ap,char *);
    if (!title[ncol]) break;
    width[ncol]=(int)va_arg(ap,int);
    ncol++;
  }
  va_end(ap);
  dlist_datal=(Dlist_data *)calloc(sizeof(Dlist_data),1);
  dlist_datar=(Dlist_data *)calloc(sizeof(Dlist_data),1);

  dlist_datal->Dlist_ncol=ncol;
  dlist_datar->Dlist_ncol=ncol;
  wl=Create_Clist(Dlist_LeftID,  Dlist_Selleft,Dlist_Unselleft,funccolsel,ncol,
                                 title[0],width[0],
                                 title[1],width[1],
                                 title[2],width[2],
                                 title[3],width[3],
                                 title[4],width[4],NULL);

  wr=Create_Clist(Dlist_RightID, Dlist_Selright,Dlist_Unselright,funccolsel,ncol,
                                 title[0],width[0],
                                 title[1],width[1],
                                 title[2],width[2],
                                 title[3],width[3],
                                 title[4],width[4],NULL);

/*
  wc=Create_ButtonArray("",Dlist_Commands,1,LABEL,"",
                                            BUTTON,Addbut,
                                            BUTTON,Rembut,NULL);
*/
  wa=Create_Button(Addbut,Dlist_Commands);
  wb=Create_Button(Rembut,Dlist_Commands);
  wc=Pack("",'V',wa,2,wb,2,NULL);
  wz=Pack(NULL,'h',wl,2,
                   wc,2,
                   wr,2,NULL);

  gtk_object_set_data(GTK_OBJECT(wl),DLIST_ID,(gpointer)dlist_datal);
  gtk_object_set_data(GTK_OBJECT(wr),DLIST_ID,(gpointer)dlist_datar);

  return wz;
}

#ifdef KANWEG
/*==============================================================*
 * Tree-related functions.
 *==============================================================*/

/*************************************
 * Create a tree
 * The tree is attached to a window in a 'scrollable' way
 *************************************/
GtkWidget *Create_Tree(GtkWidget *wnd)
{
  GtkWidget *tree,*scrolled_win;

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
                                 GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
  tree=gtk_tree_new();
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_win),tree); 
  gtk_container_add(GTK_CONTAINER(wnd),scrolled_win);
  gtk_widget_show(tree);

  return tree;
}

/*************************************
 * Add a 'leaf' to a (sub)tree
 *************************************/
GtkWidget *Add_Leaf(GtkWidget *tree,char *name,
                     void func_sel(),void func_exp(),void func_col())
{
  GtkWidget *leaf;
  if (name)
    leaf=gtk_tree_item_new_with_label(name);
  else
    leaf = gtk_tree_item_new();

  gtk_tree_append(GTK_TREE(tree),leaf);
  gtk_widget_show(leaf);
  if (func_sel)
    gtk_signal_connect(GTK_OBJECT(leaf),"select",GTK_SIGNAL_FUNC(func_sel),name);
  if (func_exp)
    gtk_signal_connect(GTK_OBJECT(leaf),"expand",GTK_SIGNAL_FUNC(func_exp),name);
  if (func_col)
    gtk_signal_connect(GTK_OBJECT(leaf),"collapse",GTK_SIGNAL_FUNC(func_col),name);

  return leaf;
}

/*************************************
 * Add a 'twig' (subtree) to a tree
 *************************************/
GtkWidget *Add_Twig(GtkWidget *tree,char *name,
                     void func_sel(),void func_exp(),void func_col())
{
  GtkWidget *subtree,*leaf;

  leaf=Add_Leaf(tree,name,func_sel,func_exp,func_col);
  subtree=leaf2twig(leaf);
  return subtree;
}

/*************************************
 * Change a leaf into a twig
 *************************************/
GtkWidget *leaf2twig(GtkWidget *leaf)
{
  GtkWidget *subtree;
  subtree=gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(leaf),subtree);
  gtk_object_set_data(GTK_OBJECT(leaf),SUBTREEFLAG,(gpointer)subtree);
  return subtree;
}
#endif
/* Get node and double click info */
GtkCTreeNode *ctree_get_buttonpress_info(GtkWidget *widget,GdkEventButton *ev,
                                         gboolean *double_click)
{
  GtkCTree *ctree=GTK_CTREE(widget);
  GtkCTreeNode *node=NULL;
  int row,column;
  if (double_click) *double_click=FALSE;
  if (ev->button == 1)
  {
    if (!gtk_clist_get_selection_info(GTK_CLIST(widget),
                                        (gint)ev->x, (gint)ev->y,
                                        &row, &column))
    {
      return NULL;
    }
    node=gtk_ctree_node_nth(ctree,row);

    if (ev->type == GDK_2BUTTON_PRESS)
    {
      if (double_click) *double_click=TRUE;
    }
  }
  return node;
}


/*************************************
 * Create a Ctree
 * The ctree is attached to a window in a 'scrollable' way
 * if sf="hv". "h": hor. scrollbar; "v": vert. scrollbar
 * NULL: no scrollbars
 * colhdr=pointer to header texts for each column
 * *sf: add scrollbars or not; "h", "v", "hv", "" or NULL
 *************************************/
GtkCTree *Create_CTree_s(char *name,
                         void func_sel(),
                         void func_unsel(),
                         void func_colsel(),
                         void func_clicked(),
                         void func_expand(),
                         void func_collapse(),
                         char *sf,
                         int nrcol,
                         char **colhdr)

{
  GtkCTree *ctree;
  GtkWidget *scroll_win;
  int i;

  scroll_win = gtk_scrolled_window_new (NULL, NULL);
  if (!sf)            // don't add scroll bars
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
                                   GTK_POLICY_NEVER,GTK_POLICY_NEVER);
  else
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
                                   (strchr(sf,'h')? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER),
                                   (strchr(sf,'v')? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER));

  ctree=GTK_CTREE(gtk_ctree_new_with_titles(nrcol,0,colhdr));
  gtk_object_set_data(GTK_OBJECT(ctree),WDGT_ID,(gpointer)name);
  for (i=0; i<nrcol; i++)
  {
    gtk_clist_set_column_auto_resize(GTK_CLIST(ctree), i, TRUE);
//    gtk_clist_set_column_resizeable(GTK_CLIST(ctree), i, TRUE); // turns off auto_resize!
  }

  gtk_container_add(GTK_CONTAINER(scroll_win), GTK_WIDGET(ctree));
  gtk_widget_show(scroll_win);
  gtk_widget_show(GTK_WIDGET(ctree));

  if (func_sel)
  {
    gtk_signal_connect(GTK_OBJECT(ctree), "tree_select_row",
		       GTK_SIGNAL_FUNC(func_sel),
		       NULL);
  }

  if (func_unsel)
  {

    gtk_signal_connect(GTK_OBJECT(ctree), "tree_unselect_row",
		       GTK_SIGNAL_FUNC(func_unsel),
		       NULL);
  }

  if (func_colsel)
  {
    gtk_signal_connect(GTK_OBJECT(ctree), "click_column",
  		       GTK_SIGNAL_FUNC(func_colsel),
		       NULL);
  }


  if (func_clicked)
  {
    gtk_signal_connect(GTK_OBJECT(ctree), "button_press_event",
		       GTK_SIGNAL_FUNC(func_clicked),
		       NULL);
  }

  if (func_expand)
  {
    gtk_signal_connect(GTK_OBJECT(ctree), "tree_expand",
		       GTK_SIGNAL_FUNC(func_expand),
		       NULL);
  }

  if (func_collapse)
  {
    gtk_signal_connect(GTK_OBJECT(ctree), "tree_collapse",
		       GTK_SIGNAL_FUNC(func_collapse),
		       NULL);
  }
  return ctree;
}

GtkCTree *Create_CTree(char *name,
                       void func_sel(),
                       void func_unsel(),
                       void func_colsel(),
                       void func_expand(),
                       void func_collapse(),
                       int nrcol,
                       char **colhdr)

{
  GtkCTree *ctree;
  ctree=Create_CTree_s(name,
                      func_sel,
                      func_unsel,
                      func_colsel,
                      NULL,
                      func_expand,
                      func_collapse,
                      "hv",
                      nrcol,
                      colhdr);
  return ctree;
}


/*************************************
 * Add a 'leaf' to a ctree
 *************************************/
GtkCTreeNode *Add_CLeaf(GtkCTree *ctree,         
                        GtkCTreeNode *parent,  /* if NULL: new subtree */
                        char *text[])          /* pointer to text of columns */
{
  GtkCTreeNode *leaf;
  leaf=gtk_ctree_insert_node(ctree,parent,NULL,text,1,
                             NULL,NULL,NULL,NULL,FALSE,FALSE);

  return leaf;
}

int find_clistitem(GtkCList *clist,char *str)
{
  int row;
  gchar *text;

  for (row=0; ; row++)
  {
    if (!gtk_clist_get_text(GTK_CLIST(clist), row, 0, &text))
      return 0;
    if (!strcmp(text,str))
      return row+1;
  }
  return 0;
}
