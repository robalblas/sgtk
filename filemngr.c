/**************************************************
 * RCSId: $Id: filemngr.c,v 1.3 2017/06/11 16:53:43 ralblas Exp $
 *
 * File managers.
 * Project: SGTK
 * Author: R. Alblas
 *
 * History: 
 * $Log: filemngr.c,v $
 * Revision 1.3  2017/06/11 16:53:43  ralblas
 * _
 *
 * Revision 1.2  2017/04/11 19:59:49  ralblas
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
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "sgtk.h"

#define RCHAR(l) l[strlen(l)-1]

static BUFFER file_selected;
static int limit_nfiles;


int make_dir(char *dir)
{
  #if __GTK_WIN32__ == 1
    return mkdir(dir);
  #else
    return mkdir(dir,0xfff);
  #endif
}

int make_dir_hier(char *dir)
{
  char *idir,*p;
  int err=0;
  if (!(idir=malloc(strlen(dir)+2))) return -1;
  strcpy(idir,dir);
  p=idir;
  do
  {
    if ((p=strchr(p,DIR_SEPARATOR))) { *p=0; }
    err=make_dir(idir);
    if (p) { *p=DIR_SEPARATOR; p++; }
    if ((p) && (!*p)) break;
  } while (p);
  free(idir);
  return err;
}

/*************************************
 * Construct directory path and place into BUFFER struct.
 * Relative path is translated into absolute path.
 *************************************/
#define EXTRA_MEMALLOC 5
BUFFER construct_dirpath(char *ipath)
{
  BUFFER path;
  path.size=0;

  if ((ipath) && ((*ipath=='/') || (*ipath=='\\') || (*(ipath+1)==':')))
  {                                            /* absolute path */
/* Copy absolute path.
   Extra chars allocated so there is space to add dir-separator if needed.
*/
    path.size=strlen(ipath)+EXTRA_MEMALLOC;
    path.buffer=(char *)malloc(path.size);
    strcpy(path.buffer,ipath);
  }
  else
  {
/* If relative path then construct absolute path. */
    path.buffer=(char *)getcwd(NULL,0);
/* Some OS don't support size=0; try with size!=0 */
    if (!path.buffer) path.buffer=(char *)getcwd(NULL,200);
    if (!path.buffer) return path;              /* Error! */

    path.size=strlen(path.buffer);

    if ((ipath) && (*ipath))
    {
      path.size+=(strlen(ipath)+EXTRA_MEMALLOC);
      path.buffer=(char *)realloc(path.buffer,path.size);
      sprintf(path.buffer,"%s%c%s",path.buffer,DIR_SEPARATOR,ipath);
    }
    else
    {
      path.size+=EXTRA_MEMALLOC;
      path.buffer=(char *)realloc(path.buffer,path.size);
      sprintf(path.buffer,"%s%c",path.buffer,DIR_SEPARATOR);
    }
  }
/* Add closing dir-separator if not present */
  if (RCHAR(path.buffer) != DIR_SEPARATOR)
    sprintf(path.buffer,"%s%c",path.buffer,DIR_SEPARATOR);

  return path;
}

/*************************************
 * String compare; case insensitive
 *************************************/
int strcmp_ci(char *a,char *b)
{
  int i;

  for (i=0; ;i++)
  {
    if ((!a[i]) && (!b[i])) break;
    if (!a[i]) return -1;
    if (!b[i]) return 1;
    if (tolower(a[i]) < tolower(b[i])) return -1;
    if (tolower(a[i]) > tolower(b[i])) return 1;
    if (a[i]<b[i]) return -1;
    if (a[i]>b[i]) return 1;
  }
  return 0;
}

/*************************************
 * Sort function for clist: case insensitive
 *************************************/
int sort_ci(GtkCList *clist,gconstpointer *ptr1,gconstpointer *ptr2)
{
  const GtkCListRow *row1 = (const GtkCListRow *) ptr1;
  const GtkCListRow *row2 = (const GtkCListRow *) ptr2;
  char *text1,*text2;

  text1 = GTK_CELL_TEXT(row1->cell[clist->sort_column])->text;
  text2 = GTK_CELL_TEXT(row2->cell[clist->sort_column])->text;

  return strcmp_ci(text1,text2);
}

/*************************************
 * Wild char comparator
 *************************************/
static int istrcmpwild(char *s,char *v)
{
  while ((*v) && (*s))
  {
    if (*v=='*')
    {
      v++;
      if (!*v) return 0;                   /* "abc"=="abc*" */
      while ((*s) && (istrcmpwild(s,v))) s++;
    }
    if ((*v!='?') && (*v!=*s)) return 1;   /* "abc"=="adb" */
    v++; s++;
  }
  if (*s) return 2;                        /* "abc"=="ab" */
  while (*v=='*') v++;
  if (*v) return 3;                        /* "ab"=="abc" */
  return 0;                                /* "abc"=="a*c" enz. */
}      

/*************************************
 * remove content of dir.
 * return: amount of items deleted
 *************************************/
int remove_dircontent(char *dir,char *filter,gboolean doit)
{
  DIR *directory;
  int n=0;
  char s[1000];
  struct dirent *dirEntry;
/* opendir(): For Windows path MUST end with '.' */
  strcpy(s,dir);
  if (RCHAR(s)!=DIR_SEPARATOR) sprintf(s+strlen(s),"%c",DIR_SEPARATOR);
  if (RCHAR(s)!='.') strcat(s,".");
  directory = opendir(s);
  if (directory) while ((dirEntry = readdir(directory)))
  {
    if (RCHAR(dir)!=DIR_SEPARATOR)
      sprintf(s,"%s%c%s",dir,DIR_SEPARATOR,dirEntry->d_name);
    else
      sprintf(s,"%s%s",dir,dirEntry->d_name);
    if ((!filter) || (!istrcmpwild(s,filter)))
    {
      if (doit)
      {
        if (*dirEntry->d_name != '.')
        {
          if (is_a_dir(s))
          {
            rmdir(s);
          }
          else
          {
            if (!(remove(s))) n++;
          }
        }
      }
      else n++;
    }
  }
  if (directory) closedir(directory);
  return n;
}

/*************************************
 * move content of dir
 *************************************/
int move_dircontent(char *dir,char *dirdest)
{
  DIR *directory;
  int ret=-1;
  char s[1000];
  char d[1000];
  struct dirent *dirEntry;
/* opendir(): For Windows path MUST end with '.' */
  strcpy(s,dir);
  if (RCHAR(s)!='.') strcat(s,".");
  directory = opendir(s);
  if (directory) while ((dirEntry = readdir(directory)))
  {
    sprintf(s,"%s%s",dir,dirEntry->d_name);
    sprintf(d,"%s%s",dirdest,dirEntry->d_name);
    
    if (*dirEntry->d_name != '.')
    {
      remove(d);              // needed for windhoos...
      ret&=rename(s,d);
    }
  }
  if (directory) closedir(directory);
  return ret;
}

/* Allocate and copy. 2 extra bytes allocated: for closing 0 and maybe 1 extra char to add. */
static int strcpyd_fmngr(char **sp,char *s)
{
  if (!(*sp=(char *)malloc(strlen(s)+2))) return 1;
  strcpy(*sp,s);
  return 0;
}

/* Allocate and concat. 2 extra bytes allocated: for closing 0 and maybe 1 extra char to add. */
static int strcatd_fmngr(char **sp,char *s)
{
  if (!*sp)
  {
    strcpyd_fmngr(sp,s);
    return 0;
  }
  
  if (!(*sp=(char *)realloc(*sp,strlen(*sp)+strlen(s)+2))) return 1;
  strcat(*sp,s);
  return 0;
}

/*************************************
 * directory help functions
 *************************************/
gboolean is_a_dir_t(char *path,time_t *time)
{
  struct stat buf;
  gboolean ret=FALSE;
  #if __GTK_WIN32__ == 1
    char *ip;
    strcpyd_fmngr(&ip,path);
    if (ip)
    {
      if (RCHAR(ip)==DIR_SEPARATOR) RCHAR(ip)=0;
      ret=((stat(ip, &buf) >= 0) && (S_ISDIR(buf.st_mode)));
      free(ip);
    }
  #else
    ret=((stat(path, &buf) >= 0) && (S_ISDIR(buf.st_mode)));

  #endif
  if (time) *time=buf.st_mtime;
  return ret;
}

gboolean is_a_dir(char *path)
{
  return is_a_dir_t(path,NULL);
}

void construct_new_path(BUFFER *new_path,BUFFER *path,char *d_name)
{
/* Allocate extra space in path buffer if needed */
  if (new_path->size < strlen(path->buffer)+strlen(d_name)+4)
  {
    new_path->size=strlen(path->buffer)+strlen(d_name)+4;
    new_path->buffer=realloc(new_path->buffer,new_path->size);
  }

/* Construct new path */
  sprintf(new_path->buffer, "%s%c%s", 
                     path->buffer,DIR_SEPARATOR,d_name); 
}

/*************************************
 * Create directory content
 *************************************/
#define MAXFILELEN 100
static void Create_Dircont(GtkWidget *wl,GtkWidget *wr,
                           char *ipath,int nmaxfile,
                           char *filterd,char *filterf)
{
  int nrf=0;
  DIR *directory;
  BUFFER path,new_path;
  struct dirent *dirEntry;
  char *tmp[2];
  char txt[1][MAXFILELEN+2];
  time_t time;
  path.buffer=NULL;
  path.size=0;
  new_path.buffer=NULL;
  new_path.size=0;

/* Clear the lists and then fill them with new dir. content */
  gtk_clist_clear(GTK_CLIST(wl));
  gtk_clist_clear(GTK_CLIST(wr));


  path=construct_dirpath(ipath);

/* For Windows path MUST end with '.' */
  if (RCHAR(path.buffer) != '.') strcat(path.buffer,".");

  directory = opendir(path.buffer);
  if (directory) while ((dirEntry = readdir(directory)))
  {
    construct_new_path(&new_path,&path,dirEntry->d_name);
    strncpy(txt[0],dirEntry->d_name,MAXFILELEN); txt[0][MAXFILELEN-1]=0;
    
    tmp[0]=txt[0];
    tmp[1]="";

/* Determine directory/file */
    if (is_a_dir_t(new_path.buffer,&time))
    {
      sprintf(txt[0],"%s%c",txt[0],DIR_SEPARATOR);
/* Add dirname to left list if it matches with filter */
      if (filterd)
        if ((!*filterd) || (!istrcmpwild(dirEntry->d_name,filterd)))
          gtk_clist_append(GTK_CLIST(wl),tmp);

    }
    else
    {
      char t[20];
      struct tm *tm=gmtime(&time);
      strftime(t,sizeof(t)-1,"%Y-%m-%d_%H:%M",tm);
      tmp[1]=t;
/* Add filename to right list if it matches with filter */
      if ((!nmaxfile) || (nrf<nmaxfile))
      {
        if (filterf)
          if ((!*filterf) || (!istrcmpwild(dirEntry->d_name,filterf)))
            gtk_clist_append(GTK_CLIST(wr),tmp);
      }
      nrf++;
    }
  }
  if (directory) closedir(directory);

  gtk_clist_sort(GTK_CLIST(wl));
  gtk_clist_sort(GTK_CLIST(wr));

  if ((nmaxfile) && (nrf>=nmaxfile))
  {
    sprintf(txt[0],"(%d more files.)",nrf-nmaxfile);
    gtk_clist_append(GTK_CLIST(wr),tmp);
  }
/* free allocated buffers */

  if (path.size) free(path.buffer);
  if (new_path.size) free(new_path.buffer);
}

#define LAB_DIR "^Directory"
#define LAB_DISK "^Disk"

#define LAB_FILD "Filter D"
#define LAB_FILF "Filter F"
#define LAB_FSEL "File selected"
#define LAB_FDEL "Delete file"
#define LAB_FREN "Rename file"

#define LAB_OK   "OK"
#define LAB_CLOSE "Close"

#define LAB_DIRLIST "Dirlist"
#define LAB_FILELIST "Filelist"
static GtkWidget *wd,*wfl,*wfr,*wl,*wr;
static char *file_name; /* Pointer to filename (arg. Create_Fileselectf) */
static char *dir_name;
static char id_filter[100],if_filter[100];

void Close_Fileselectf(GtkWidget *window)
{
  if (file_selected.size)
  {
    free(file_selected.buffer);
  }
  file_selected.size=0;
  file_selected.buffer=NULL;
  Close_Window(window);
}

/* Copy filename and exit */
static void int_okfuncf(GtkWidget *widget,gpointer data)
{
  GtkWidget *window=Find_Parent_Window(widget);
  char *cur_dir=Get_Entry(wd,LAB_DIR);
  char *cur_file=Get_Entry(wd,LAB_FSEL);
  if (dir_name) strcpy(dir_name,cur_dir);
  if (RCHAR(cur_dir)==DIR_SEPARATOR) RCHAR(cur_dir)=0;

  if ((file_name) && ((cur_file)&&(*cur_file)))
    sprintf(file_name,"%s%c%s",cur_dir,DIR_SEPARATOR,cur_file);
  if (window) Close_Fileselectf(window);
}

/* Just exit */
static void int_canfuncf(GtkWidget *widget)
{
  GtkWidget *window=Find_Parent_Window(widget);
  Close_Fileselectf(window);
}

/*************************************
 * directory clist callbacks for Create_Fileselectf
 *************************************/
static void selected_dir( GtkWidget      *clist,
                          gint            row,
                          gint            col,
	       	          GdkEventButton *event,
                          gpointer        data)
{
  char *sel_dir,*old_dir,*new_dir,*p;

/* Get the selected text (= dir) */
  gtk_clist_get_text(GTK_CLIST(clist), row, col, &sel_dir);
  switch(event->type)
  {
/* Activate if double-clicked */
    case GDK_2BUTTON_PRESS:
/* Get the current dir. */
      old_dir=Get_Entry(wd,LAB_DIR);

/* One dir. up */
      if (!strncmp(sel_dir,"..",2))
      {
        new_dir=malloc(strlen(old_dir)+10);
	strcpy(new_dir,old_dir);

/* Strip last part from dir */
        if ((p=strrchr(new_dir,DIR_SEPARATOR)))
        {
          if (*(p+1))
          {                  /* old_dir didn't end with dirsep */
            *(p+1)=0;
          }
          else
          {                  /* old_dir did end with dirsep, so strip 2x */
            *p=0;
            if ((p=strrchr(new_dir,DIR_SEPARATOR)))
              *(p+1)=0;
          }
        }
        Set_Entry(wd,LAB_DIR,new_dir);
        free(new_dir);
      }
/* No change */
      else if (!strncmp(sel_dir,".",1))
      {
        new_dir=old_dir;
        Set_Entry(wd,LAB_DIR,new_dir);
      }
/* Add dir to path */
      else
      {
        new_dir=malloc(strlen(old_dir)+strlen(sel_dir)+10);
        if (RCHAR(old_dir)==DIR_SEPARATOR) RCHAR(old_dir)=0;
        sprintf(new_dir,"%s%c%s",old_dir,DIR_SEPARATOR,sel_dir);
        Set_Entry(wd,LAB_DIR,new_dir);
        free(new_dir);
      }
    break;
    default:
    break;
  }
  {
    char *cur_dir=Get_Entry(wd,LAB_DIR);
    if (dir_name) strcpy(dir_name,cur_dir);
  }
}


/*************************************
 * File clist callbacks for Create_Fileselectf
 *************************************/
static void selected_file(GtkWidget      *clist,
                          gint            row,
                          gint            col,
	       	          GdkEventButton *event,
                          gpointer        data)
{
  char *fn;

/* Get the selected text (= filename) */
  gtk_clist_get_text(GTK_CLIST(clist), row, 0, &fn);
  switch(event->type)
  {
    case GDK_BUTTON_RELEASE:
      Set_Entry1(wd,LAB_FSEL,fn);
    break;
/* if double click: read file and exit */
    case GDK_2BUTTON_PRESS:
      Set_Entry(wd,LAB_FSEL,fn);
      Set_Button(clist,LAB_OK,TRUE);     /* causes destroy Create_Fileselectf */
      return;                            /* so stop now! */
    break;
    default:
    break;
  }
  {
    char *cur_dir=Get_Entry(wd,LAB_DIR);
    char *cur_file=Get_Entry(wd,LAB_FSEL);
    if (dir_name) strcpy(dir_name,cur_dir);

    if (file_selected.size<strlen(cur_dir)+strlen(cur_file)+10)
    {
      file_selected.size=strlen(cur_dir)+strlen(cur_file)+10;
      file_selected.buffer=realloc(file_selected.buffer,file_selected.size);
    }
    if (RCHAR(cur_dir)==DIR_SEPARATOR)
      sprintf(file_selected.buffer,"%s%s",cur_dir,cur_file);
    else
      sprintf(file_selected.buffer,"%s%c%s",cur_dir,DIR_SEPARATOR,cur_file);
  }
}



static char *filterd,*filterf;
/*************************************
 * Buttons for Create_Fileselectf
 *************************************/
static void but_func(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  char *new_dir=Get_Entry(wd,LAB_DIR);
/* evaluate dir-cont only if last char in dirname is 'dirsep' */
  if (RCHAR(new_dir)==DIR_SEPARATOR)
  {
/* Get the filters */
    filterd=Get_Entry(wd,LAB_FILD);
    filterf=Get_Entry(wd,LAB_FILF);
/* Create new dir-lists */
    Create_Dircont(wl,wr,new_dir,limit_nfiles,filterd,filterf);
    if (dir_name) strcpy(dir_name,new_dir);
  }
  if (!strcmp(name,LAB_FILD))
    strcpy(id_filter,Get_Entry(wd,LAB_FILD));
  if (!strcmp(name,LAB_FILF))
    strcpy(if_filter,Get_Entry(wd,LAB_FILF));

/* File delete */
  if (!strcmp(name,LAB_FDEL))
  {
    char *file_selected=Get_Entry(widget,LAB_FSEL);
    if ((file_selected) && (*file_selected))
    {
      int ret;
      ret=Create_Choice("Delete",2,"Delete","Cancel","Really delete file\n%s?",
                                                                file_selected);
      if (ret==1)
      {
        char *absname=NULL;
        if (new_dir)
          strcpyd_fmngr(&absname,new_dir);
        strcatd_fmngr(&absname,file_selected);
        if ((remove(absname)))
        {
          Create_Message("Error","Failed to delete %s.\nIn use?",file_selected);
        }
        else
        {
          filterd=Get_Entry(wd,LAB_FILD);
          filterf=Get_Entry(wd,LAB_FILF);
          Create_Dircont(wl,wr,new_dir,limit_nfiles,filterd,filterf);
        }
        free(absname); absname=NULL;
      }
    }
  }
  
/*
  if (!strcmp(name,LAB_FREN))
  {
    if ((file_selected) && (*file_selected))
    {
rename(file_selected,nieuw);
    }
  }
*/
}

#if __GTK_WIN32__ == 1
static discmem_ready;
static void disk_func(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  if (discmem_ready)
  {
    #ifdef __GTK_20__
    name=soption_menu_get_history(widget);
    #endif
    Set_Entry(widget,LAB_DIR,name);
  }
}
#endif


char *file_selection_get_filename()
{
  return file_selected.buffer;
}

char *file_selection_get_dirname()
{
  return dir_name;
}

#define PIJLO "[^]"
#define PIJLN "[v]"
static gint sortfunc(GtkCList *clist,gconstpointer *ptr1, gconstpointer *ptr2)
{
  const GtkCListRow *row1 = (const GtkCListRow *) ptr1;
  const GtkCListRow *row2 = (const GtkCListRow *) ptr2;
  char *s1,*s2;

  s1 = GTK_CELL_TEXT(row1->cell[clist->sort_column])->text;
  s2 = GTK_CELL_TEXT(row2->cell[clist->sort_column])->text;
  if ((!s1) || (!s2)) return 0;
  return strcmp(s1,s2);
}
static int colwidth[]={20,10};

static void sort_col(GtkWidget      *widget,
                     gint            column,
                     gint            nrcol,
                     gboolean        as_desn,
                     gpointer        data)
{
  GtkCList *w_cl=(GtkCList *)widget;
  char str[50],*p;
  int c;
  gtk_clist_set_sort_column(w_cl,column);
  gtk_clist_set_compare_func(w_cl,(GtkCListCompareFunc)sortfunc);
  if (as_desn)
    gtk_clist_set_sort_type(w_cl,GTK_SORT_DESCENDING);
  else
    gtk_clist_set_sort_type(w_cl,GTK_SORT_ASCENDING);
  gtk_clist_sort(w_cl);

  for (c=0; c<nrcol; c++)
  {
    strcpy(str,gtk_clist_get_column_title(w_cl,c));
    while (strlen(str)<colwidth[c]) strcat(str," ");
    if ((p=strstr(str,PIJLO))) *p=0;
    if ((p=strstr(str,PIJLN))) *p=0;
    if (c==column)
    {
      if (as_desn)
        strcat(str,PIJLO);
      else
        strcat(str,PIJLN);
    }
    gtk_clist_set_column_title(w_cl,c,str);
  }
}
static void sort_col1(GtkWidget      *widget,
                      gint            icol,
                      gpointer        data)
{
  static gboolean lastcol_sorted;
  static gboolean as_desn=FALSE;
  int column;
  if (icol<0)
  {
    column=lastcol_sorted;
  }
  else
  {
    column=icol;
    as_desn=!as_desn;
  }
  sort_col(widget,column,2,as_desn,data);
  if (icol>=0)
  {
    lastcol_sorted=column;
  }
}



/*************************************
 * Create file select window with filters
 *
 * window       : A window widget
 * title        : window title
 * ext_canfunc(): external cancel function. 
 *                If NULL: internal cancel function used.
 * ext_okfunc() : external OK function. 
 *                         Create_Fileselectf is NOT modal.
 *                If NULL: internal OK function used.
 *                         Create_Fileselectf is modal, exit if
 *                         OK/cancel button is pressed.
 * startdir     : directory where to start. 
 *                file_selection_get_dirname may be used to catch
 *                last visited dir. 
 *                !!! Must be big enough to hold biggest expected path!!!
 *                If NULL: No dir. catch possible.
 * d_filter     : directory filter; if NULL: "*" and no restore after exit
 * f_filter     : file filter; if NULL: "*" and no restore after exit
 * file         : Gets selected filename if internal OK function is used *************************************/
GtkWidget *Create_Fileselectf(GtkWidget *window,  /* Existing window */
                              char *title,        /* Name of window or NULL */
                              void ext_canfuncf(),/* cancel function or NULL */
                              void ext_okfuncf(), /* OK function or NULL */
                              char *startdir,     /* start dir */
                              char *d_filter,     /* dir filter */
                              char *f_filter,     /* file filter */
                              int nmaxfile,       /* if !=0 then limit # files to load */
                              char *file)         /* selected file or NULL */
{
  GtkWidget *file_window;
  GtkWidget *wx,*wy,*wy1,*wy2,*wz,*wlx,*wrx;
  GtkWidget *wfsel,*wfdel,*wfren;
  GtkWidget *wok,*wcan;
  BUFFER path;

  file_selected.size=0;
  file_selected.buffer=NULL;
  if (!*if_filter)
  {
    strcpy(if_filter,"*");
    if (f_filter) strcpy(if_filter,f_filter);
  }
  if (!*id_filter)
  {
    strcpy(id_filter,"*");
    if (d_filter) strcpy(id_filter,d_filter);
  }
  file_name=file;
  dir_name=startdir;
  limit_nfiles=nmaxfile;

/* Determine current path */
  path=construct_dirpath(startdir);
  file_window=Create_Window(window,350,400,title,NULL);
  if (!file_window) return NULL;          /* window did already exist! */



  wd=Create_Entry(LAB_DIR,but_func,"%s",path.buffer);
  if (path.size) free(path.buffer);

  #if __GTK_WIN32__ == 1
  {
    GtkWidget *dsel;
    dsel=Create_Optionmenu(LAB_DISK,disk_func,1,NULL);
    wd=SPack(NULL,"h",dsel,"1",wd,"ef1",NULL);
  }
  #endif

/* Dir-part */
  wfl=Create_Entry(LAB_FILD,but_func,"%-20s",id_filter);
  wl=Create_Clist(LAB_DIRLIST, selected_dir ,NULL,NULL,1,"Directories",40,NULL);
  wlx=SPack("Directory","v",wfl,"2",wl->parent,"ef2",NULL);

/* File-part */
  wfr=Create_Entry(LAB_FILF,but_func,"%-20s",if_filter);
  wr=Create_Clist(LAB_FILELIST,selected_file,NULL,sort_col1,2,"Files",40,"time",16,NULL);
  wrx=SPack("File","v",wfr,"2",wr->parent,"ef2",NULL);
  gtk_clist_set_column_auto_resize((GtkCList *)wr,0,TRUE);

/* File */
  wfsel=Create_Entry(LAB_FSEL,NULL,"%-25s","");
  wfdel=Create_Button(LAB_FDEL,but_func);
/*
  wfren=Create_Button(LAB_FREN,but_func);
*/
wfren=NULL;

/* OK/Close part */
  if (ext_okfuncf)
    wok=Create_Button(LAB_OK,ext_okfuncf);
  else
    wok=Create_Button(LAB_OK,int_okfuncf);
  if (ext_canfuncf)
    wcan=Create_Button(LAB_CLOSE,ext_canfuncf);
  else
    wcan=Create_Button(LAB_CLOSE,int_canfuncf);

/* Pack all together */
  wx=SPack(NULL,"h",wlx,"ef2",wrx,"ef2",NULL);
  wy1=SPack(NULL,"h",wfsel,"2",wfdel,"E2",wfren,"2ef",NULL);
  wy2=SPack(NULL,"H",wok,"2ef",wcan,"2ef",NULL);

  wy=SPack("","v",wy1,"2",wy2,"2",NULL);
  wz=SPack(NULL,"v",wd,"2",wx,"ef2",wy,"2",NULL);
  gtk_container_add(GTK_CONTAINER(file_window),wz);
  gtk_widget_show_all(file_window);

/* Define sort function */
  gtk_clist_set_compare_func(GTK_CLIST(wl),(GtkCListCompareFunc)sort_ci);
  gtk_clist_set_compare_func(GTK_CLIST(wr),(GtkCListCompareFunc)sort_ci);

/* Create initial dir. content */
  Create_Dircont(wl,wr,startdir,nmaxfile,id_filter,if_filter);

  #if __GTK_WIN32__ == 1
  {
    char disc[26][10];
    int ndisc=0;
    char disc1;
    int cd=0;
    DIR *directory;
    strcpy(disc[ndisc++],"a:\\");
    strcpy(disc[ndisc++],"b:\\");
    for (disc1='c'; disc1<='z'; disc1++)
    {
      strcpy(disc[ndisc],"c:\\");
      disc[ndisc][0]=disc1;
      if ((directory=opendir(disc[ndisc])))
      {
        closedir(directory);
        ndisc++;
      }
    }
    if (startdir)
    {
      for (disc1=0; disc1<ndisc; disc1++)
      {
        if (!strncmp(startdir,disc[disc1],2)) cd=disc1;
      }
    }
    if (!cd) cd=2;
    discmem_ready=FALSE;
    Add_Options(file_window,LAB_DISK,disk_func,cd,disc,ndisc,10);
    discmem_ready=TRUE;

  }
  #endif

  if (!ext_okfuncf) gtk_main();
  return file_window;
}

/*************************************
 *************************************
  Next are functions for standard gtk fileselector 
 *************************************
 *************************************/
/*************************************
 * Close file select window 
 *************************************/
void Close_Fileselect(GtkWidget *window)
{
  if (!window) return;
  Remove_Window_Info(window);
  gtk_widget_destroy(window);
}

/*************************************
 * Help functions for Create_Fileselect 
 *************************************/
static char *file_name;
static void int_okfunc(GtkWidget *widget,GtkFileSelection *fs)
{
  if (file_name)
    strcpy(file_name,gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)));
  gtk_main_quit();
}

static void int_canfunc1(GtkWidget *widget)
{
  GtkWidget *window=Find_Parent_Window(widget);
  Close_Fileselect(window);
  gtk_main_quit();
}

static void int_canfunc(GtkWidget *widget)
{
  GtkWidget *window=Find_Parent_Window(widget);
  Close_Fileselect(window);
}

/*************************************
 * Create file select window 
 *
 * window       : A window widget
 * title        : window titke
 * ext_canfunc(): external cancel function. 
 *                If NULL: internal cancel function used.
 * ext_okfunc() : external OK function. 
 *                         Create_Fileselect is NOT modal.
 *                If NULL: internal OK function used.
 *                         Create_Fileselect is modal, exit if
 *                         OK/cancel button is pressed.
 * file         : Gets selected filename if internal OK function is used *************************************/
GtkWidget *Create_Fileselect(GtkWidget *window,  /* Existing window */
                             char *title,        /* Name of window or NULL */
                             void ext_canfunc(), /* cancel function or NULL */
                             void ext_okfunc(),  /* OK function or NULL */
                             char *file)         /* selected file or NULL */
{
  GtkWidget *file_window;
  file_name=file;
  file_window=gtk_file_selection_new(title);

/* Connect the cancel_button to destroy the widget */
  if (ext_canfunc)
  {
    gtk_signal_connect_object(
      GTK_OBJECT(GTK_FILE_SELECTION(file_window)->cancel_button),
      "clicked", (GtkSignalFunc)ext_canfunc,GTK_OBJECT(file_window));
  }
  else if (!ext_okfunc)
  {
    gtk_signal_connect_object(
      GTK_OBJECT(GTK_FILE_SELECTION(file_window)->cancel_button),
      "clicked", (GtkSignalFunc)int_canfunc1,GTK_OBJECT(file_window));
  }
  else
  {
    gtk_signal_connect_object(
      GTK_OBJECT(GTK_FILE_SELECTION(file_window)->cancel_button),
      "clicked", (GtkSignalFunc)int_canfunc,GTK_OBJECT(file_window));
  }

/* Connect the ok_button to file_ok_sel function */
  if (ext_okfunc)
  {
    gtk_signal_connect(
      GTK_OBJECT(GTK_FILE_SELECTION(file_window)->ok_button),
      "clicked", (GtkSignalFunc)ext_okfunc,GTK_OBJECT(file_window));
  }
  else
  {
    gtk_signal_connect(
      GTK_OBJECT(GTK_FILE_SELECTION(file_window)->ok_button),
      "clicked", (GtkSignalFunc)int_okfunc,GTK_OBJECT(file_window));
  }

  gtk_widget_show(file_window);

  if (window) Create_Window_Info(window,file_window,"");

  if (!ext_okfunc) gtk_main();

  return file_window;
}

char *Create_Save(char *fn,char *dir)
{
  char *dirfn=NULL;
  GtkWidget *dialog;
  GtkWindow *parent_window=NULL;
  dialog = gtk_file_chooser_dialog_new ("Save File",
                                   parent_window,
                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                   GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                   NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), dir);
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), fn);
     
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    dirfn = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
  }

  gtk_widget_destroy (dialog);
  return dirfn;
}
