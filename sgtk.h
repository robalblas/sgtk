/*****************************************************
 * RCSId: $Id: sgtk.h,v 1.3 2016/01/10 16:25:59 ralblas Exp $
 *
 * Header file for Simple GTK functions
 * Author: R. Alblas
 *
 * History: 
 * $Log: sgtk.h,v $
 * Revision 1.3  2016/01/10 16:25:59  ralblas
 * _
 *
 * Revision 1.2  2016/01/09 20:45:02  ralblas
 * _
 *
 * Revision 1.1  2015/11/15 22:31:49  ralblas
 * Initial revision
 *
 *****************************************************/
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
#ifdef TEST_MEM
  #ifndef malloc
    #define malloc tmalloc
  #endif
  #ifndef realloc
    #define realloc trealloc
  #endif
  #ifndef calloc
    #define calloc tcalloc
  #endif
  #ifndef free
    #define free tfree
  #endif
#endif

#ifndef GUI_RTS_HDR
#define GUI_RTS_HDR

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

#define UNIX_DIRSEP '/'
#define DOS_DIRSEP '\\'
#define UNIX_PATHSEP ':'
#define UNIX_PATHSEP_STR ":"
#define DOS_PATHSEP ';'
#define DOS_PATHSEP_STR ";"

#ifdef __GTK_WIN32__
  #if __GTK_WIN32__ == 0
    #define DIR_SEPARATOR UNIX_DIRSEP
    #define PATH_SEPARATOR UNIX_PATHSEP
    #define PATH_SEPARATOR_STR UNIX_PATHSEP_STR
  #else
    #define DIR_SEPARATOR DOS_DIRSEP
    #define PATH_SEPARATOR DOS_PATHSEP
    #define PATH_SEPARATOR_STR DOS_PATHSEP_STR
  #endif
#else
  #define DIR_SEPARATOR UNIX_DIRSEP
  #define PATH_SEPARATOR UNIX_PATHSEP
  #define PATH_SEPARATOR_STR UNIX_PATHSEP_STR
#endif

#define DEFAULT_WIDTH 400
#define DEFAULT_HEIGHT 300

/* ID's of object_data */
#define WNDINFO "wndinfo"            /* window params; links to other windows */
#define WDGT_ID "sgtk_id"            /* widgets containing state */
#define TWDGT_ID "sgtk_top_id"       /* top-widget of e.g. label + entry pair */
#define ADJUST_ID "ADJ"
#define PARENT_ID "PARENT_IS"
#define CHILD_ID "CHILD_IS"
#define LAB_HBAR "hbar"
#define LAB_VBAR "vbar"

/* Links between widgets */
#define DRAWABLE_INFO "drawable_info"          /* link to window */
#define RGBPI_DL "SGTK_RGBPI_DL"

/* All data needed for RGB drawing */
typedef struct rgbi
{
  guchar *rgbbuf;
  long bufsize;              /* allocated size of rgbbuf */
  GdkRgbCmap *rgb_cmap;
  int width,height;
  int size;                  /* size of the 'gdk_rgb_cmap' colormap */
  int enable_update;
} RGBI;

/*************************************
 * Needed vars and function for RGB creation
 *************************************/
#define NLUMSTAT 16384
typedef struct rgbpicinfo
{
  gboolean size_drawable_limit;
  GtkWidget *widget;
  int (*draw_func)();
  int pwidth,pheight;
  float pixel_shape;   /* 0: ignore; 1: 'square', > 1 -> width > height */
  gboolean do_shape;
  float xscale,yscale;
  guchar *str8; 
  guint16 *str16; 
  float zx,zy;
  int ox,oy;
  int lmin,lmax;
  int bpp,lummax;
  guint32 lumstat[NLUMSTAT];
  int lminrgb[3],lmaxrgb[3];
  guint32 lumstatrgb[3][NLUMSTAT];
  void (*xdrawfunc)(RGBI *,struct rgbpicinfo *);
  void (*keymousefunc)(GtkWidget *,int ,int ,int ,int ,int ,int ,float ,float );
  gboolean redraw;
  gboolean anal_lum;
  gboolean suppress_key;
} RGBPICINFO;

/* Backbone to connect windows to each other and
   to add window specific info 
*/
#define WINNAAMMAXLEN 100
typedef struct Wnd_data
{
  struct Wnd_data *prev,*next;

  GtkWidget *window;
  int default_width,default_height;
  GdkPixmap *pixmap;
  RGBI rgbi;
  GdkGC **gc;
  int gc_size;
  char title[WINNAAMMAXLEN];
  gboolean maximized;
  gboolean enable_update;
  gboolean state_fullscreen;
} WINDOW_DATA;

/* Window position methodology */
typedef enum {deflt_wndpos=0,abs_wndpos,center_wndpos,
              smart_wndpos,relfrst_wndpos,
              left_top,right_top,left_bottom,right_bottom
             } WND_POS;

/* Types of buttons */
typedef enum { UNDEF_BUTTON,LABEL,
               TOGGLE,NTOGGLE,BUTTON,NBUTTON,CHECK,CHECKI,RADIO,LED,
               RADIOs,RADIOn,
               ENTRY,ENTRY_NOFUNC,NENTRY,HSCALE,VSCALE,NSCALE,SPIN,NSPIN} TYPE_BUT;

/* Used fonts for text */
#define SGTK_VARFONT "-adobe-helvetica-medium-r-normal-*-*-%d-*-*-p-*-iso8859-1"
#define SGTK_FIXFONT "-adobe-courier-medium-r-normal-*-*-%d-*-*-m-*-iso8859-1"

/*
oud; kan weg?
#define SGTK_FIXFONT "-misc-fixed-medium-r-*-*-*-%d-*-*-*-*-iso8859-1"
*/

/* Next are ID's for double clist. */
#define Dlist_LeftID "Dlist_LeftID"
#define Dlist_RightID "Dlist_RightID"

#define SUBTREEFLAG "Subtree"

/* A char buffer with size */
typedef struct
{
  char *buffer;
  int size;
} BUFFER;

/***************************************
 * Record for saving gui state
 ***************************************/
 typedef struct guistate
{
  char *window;
  char *button;
  char *state;
  struct guistate *prev,*next;
} GUISTATE;

#include "sgtk_functions.h"
#endif
