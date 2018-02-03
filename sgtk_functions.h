/*****************************************************
 * RCSId: $Id: sgtk_functions.h,v 1.9 2017/06/11 16:54:21 ralblas Exp $
 *
 * Header file containing sgtk function definitions
 * Author: R. Alblas
 *
 * History: 
 * $Log: sgtk_functions.h,v $
 * Revision 1.9  2017/06/11 16:54:21  ralblas
 * _
 *
 * Revision 1.8  2017/03/19 10:22:23  ralblas
 * _
 *
 * Revision 1.7  2016/05/25 18:13:04  ralblas
 * _
 *
 * Revision 1.6  2016/03/05 22:08:19  ralblas
 * _
 *
 * Revision 1.5  2016/01/10 16:26:06  ralblas
 * _
 *
 * Revision 1.4  2015/12/29 22:12:35  ralblas
 * _
 *
 * Revision 1.3  2015/12/23 23:17:22  ralblas
 * _
 *
 * Revision 1.2  2015/11/15 23:22:15  ralblas
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

#ifndef SGTK_FUNCS
#define SGTK_FUNCS

#ifndef __GTK_20__
void  gtk_style_set_font(GtkStyle *style,GdkFont *font);
GdkFont *gtk_style_get_font(GtkStyle *style);
#endif

GtkWidget *Create_Window(GtkWidget *,int,int,char *,void func());
void Close_Window(GtkWidget *);
void destroy_window(GtkWidget *);

GtkWidget *Create_Drawable(int ,int,void f0(), void f1(),void f2());
GtkWidget *Create_Canvas(GtkWidget *,int,int,void func1(),gboolean func2(),void func3(),void func4());
GtkWidget *Create_Canvas1(GtkWidget *,char *,gboolean,int,int,void func1(),gboolean func2(),void func3(),void func4());
GtkWidget *Create_Canvas_RGBPic_dr(GtkWidget *,int ,int ,int ,int drawfunc(),void keymousefunc());
GtkWidget *Create_Canvas_RGBPic_1b(GtkWidget *,int ,int,char *, gboolean,gboolean,void xd(),void kf());
GtkWidget *Create_Canvas_RGBPic_2b(GtkWidget *,int ,int ,guint16 *,gboolean,gboolean,void xd(),void kf());
GtkWidget *Create_Canvas_RGBPic_dr2(GtkWidget *,int ,int ,int ,int drawfunc(),void keymousefunc());
void RGB_Pic_drawupdate(GtkWidget *,int ,int ,gboolean );
void RGB_Pic_keyfunc_ext(GtkWidget *widget, char sig);
void makeit_square(int ,int , RGBPICINFO *);

char *Add_WidgetID(GtkWidget *,char *);
GtkWidget *Pack(char *,char ,...);
GtkWidget *SPack(char *,char *,...);
GtkWidget *Create_Label(char *,...);
GtkWidget *Create_Button(char *,void func());
GtkWidget *Create_Check(char *,void func(),gboolean );
GtkWidget *Create_Toggle(char *,void func(),gboolean );
gboolean Update_Togglelabel(GtkWidget *);
GtkWidget *Create_Radio(char *,gboolean ,void func());
GtkWidget *Create_Entry(char *,void func(),char *,...);
GtkWidget *Create_Entry_compl(char *,int func(),char *lijst[],int,char *,...);
void Add_To_Compllist(GtkWidget *,char *,char *);
//void adjust_valist(va_list *ap,char *s);
GtkWidget *Create_ButtonArray(char *,void func(),int , ...);
GtkWidget *Create_Notebook(void func(),int,...);
GtkWidget *Create_Clist(char *,void func1(),void func2(),void func3(),int,...);
GList *get_selected_first(GtkCList *clist);
GtkWidget *Create_Dlist(char *name,void func1(),...);

GtkWidget *Create_Scale(char *,char,void func(),char *,...);
GtkWidget *Create_Spin(char *,void func(),char *,...);
GtkWidget *Create_Scrollbar(char *,char, float,float,float,void func());
GtkWidget *Add_Scrollbars(GtkWidget *,char *,char *,void func());
GtkWidget *Create_Colorsel(char *, GdkColor *,void func());

WINDOW_DATA *Create_Window_Info(GtkWidget *, GtkWidget *,char *); 
void Remove_Window_Info(GtkWidget *);

GtkWidget *Create_Optionmenu(char *,void func(),int,...);
void Set_Optionsmenu(GtkWidget *,char *,int );
int Get_Optionsmenu(GtkWidget *widget,char *label);
int option_menu_get_history(GtkWidget *);
GtkWidget *Add_Options(GtkWidget *,char *,void func(),int ,char **lab,int ,int );
gboolean Has_Option(GtkWidget *widget,char *m_name,char *o_name);
void Remove_Options(GtkWidget *omenu,int);
char *soption_menu_get_history(GtkWidget *omenu);

GtkWidget *Create_Menu(GtkWidget *,...);
GtkWidget *Create_Toolbar(void func(),...);

void Create_MenuFac();

/* Search functions */
GtkWidget *Find_Top_Widget();
GtkWidget *Find_Widget1();
GtkWidget *Find_Widget();
GtkWidget *Find_cw_Widget();
GtkWidget *Find_aw_Widget();
GtkWidget *Find_iw_Widget();
GtkWidget *Find_Window();
GtkWidget *Find_Parent_Window();
GtkWidget *First_window();
GtkWidget *Next_window();
void window_get_total_size();
void window_toggle_size();
void window_get_border_size();
void window_get_root_size();
void place_window();
gboolean Toggle_full_screen(GtkWidget *wnd);

/* Draw related functions */
void Create_GCPalette();
void Create_RGBPalette();
void Create_Palette();

GdkGC **Get_Gc();

GdkPixmap *Get_Pixmap();
void Put_Pixmap();
void Renew_Pixmap();
gboolean Renew_RGBBuf();

RGBI *Get_RGBI(GtkWidget *);
RGBPICINFO *Get_RGBPI(GtkWidget *);

void draw_rgbmap();

void Set_Enable_Update();
gboolean Get_Enable_Update();

gint expose_event();

int Set_Button();
void Set_Buttoncolor(GtkWidget *widget,int rgb);
void Set_All_Buttons(GtkWidget *,int );
gboolean Get_Button();

int Set_Entry(GtkWidget *,char *,char *,...);
int Set_Entry1(GtkWidget *,char *,char *);
char *Get_Entry(GtkWidget *,char *);

GtkWidget *Create_Led(char *,int);
void Set_Led(GtkWidget *,char *,int);

int Set_Adjust(GtkWidget *,char *,char *,...);
int Set_Adjustbnd(GtkWidget *,char *,char *,...);
void Set_Scroll(GtkWidget *,char *,int,int *,float *,char *,int,int *,float *);
void Set_Scrollbars(GtkWidget *wnd,RGBPICINFO *rgbpi);
void Set_Scrollc(GtkWidget *hadj,int width ,int *ox,float *zx,
                 GtkWidget *vadj,int height,int *oy,float *zy);
float Get_Adjust(GtkWidget *,char *);
void Deselect_Clist(GtkWidget *,char *,char *);
GtkWidget *add_label(char *);
char *get_wndtitle(GtkWidget *window);

int Sense_Button(GtkWidget *,char *,gboolean);
int Show_Button(GtkWidget *,char *,gboolean);
gboolean Get_Options(GtkWidget *widget,char *name,char *id);
gboolean Get_Option(GtkWidget *widget,char *name,char *id,char *req_name);

int Create_Choice(char *,int ,...)   ;   

void Create_Message(char *,char *,...);
void Create_Message_cont(char *,char *,...);

GtkWidget *Create_Fileselect();
void Close_Fileselect();
char *Create_Save(char *fn,char *dir);

GtkWidget *Create_Fileselectf(GtkWidget *,char *,void (), void (), char *,char *,char *,int,char *);
void Close_Fileselectf();

char *file_selection_get_filename();
char *file_selection_get_dirname();

GtkWidget *Create_Text(char *,gboolean,char *);
GtkWidget *Create_Info(GtkWidget *,char *,int ,int,void func(),GtkWidget *);
GtkWidget *Create_Info2(GtkWidget *,char *,int,int ,void func(),GtkWidget *,char *);
void Add_Text(GtkWidget *,int,char *,...);
void Clear_Text(GtkWidget *);
void Close_Info(GtkWidget *);

GtkWidget *Create_Progress();
gboolean Get_Progress_state(GtkWidget *);
int Update_Progress();
void Close_Progress(GtkWidget *);
GtkWidget *Create_Oneliner(char *,char *);

void Add_Mouse_events();
void Add_Key_events();
void Add_Func();

void Refresh_Rect();

void draw_rgbpoint(RGBI *,GdkColor *,int,int);
void draw_rgbline(RGBI *,GdkColor *,int ,int ,int ,int);
void draw_rgbrect(RGBI *,GdkColor *,int ,int ,int ,int );
void draw_rgbstring(RGBI *,GdkColor *,int ,int ,char *);
void get_strline(char *,int ,char *,int );

int get_rgbpoint(RGBI *,GdkColor *,int,int);

GtkWidget *Find_Local_Widget(GtkWidget *, char *);
gboolean Get_Local_Button(GtkWidget *,char *);
void Set_Local_Button(GtkWidget *,char *,gboolean);
int Add_Widget(GtkWidget *,GtkWidget *,char *,char *);

GtkWidget *Create_Tree(GtkWidget *);
GtkWidget *leaf2twig(GtkWidget *);
GtkWidget *Add_Twig(GtkWidget *,char *,void func1(),void func2(),void func3());
GtkWidget *Add_Leaf(GtkWidget *,char *,void func1(),void func2(),void func3());
GtkCTreeNode *ctree_get_buttonpress_info(GtkWidget *,GdkEventButton *,gboolean *);

GtkCTree *Create_CTree(char *,void (),void (),void (),void (),void (),int ,char **);
GtkCTree *Create_CTree_s(char *,void (),void (),void (),void (),void (),void (),char *,int ,char **);
GtkCTreeNode *Add_CLeaf(GtkCTree *,GtkCTreeNode *,char **);
int find_clistitem(GtkCList *,char *);

//GtkRcStyle *Create_RcStyle(GtkRcFlags,GdkColor *,GdkColor *,
//                                      GdkColor *,GdkColor *,GdkColor *);

void Set_Widgetcolor(GtkWidget *,char ,int ,int ,int );

void Set_Color(GtkWidget *,GtkRcFlags,GdkColor *,GdkColor *,
                                      GdkColor *,GdkColor *,GdkColor *);
void Set_Color2(GtkWidget *,int ,GdkColor *,GdkColor *,GdkColor *,GdkColor *,GdkColor *);
void Set_Widgetcolor2(GtkWidget *widget,char fb,int r,int g,int b);
void Set_Buttoncolor2(GtkWidget *widget,int rgb);

int set_fontsize(int );

char *get_path(char *);
int strcmpwild(char *,char *);
void restore_guistate_fromfile(GtkWidget *,char *);
void restore_guistate(GtkWidget *,GUISTATE *);
int save_guistate(GtkWidget *,GtkWidget *,char *,gboolean );
GUISTATE *store_guistate(GtkWidget *,GtkWidget *,GUISTATE *);
void Remove_Guistate(GUISTATE *);
GUISTATE *read_guistate(char *file);
void add_guistate(GUISTATE *gs2t,char *file);

int CheckKeyPressed(int);

void pri_dbg(char *frmt,...);

#include <dirent.h>
BUFFER construct_dirpath(char *);
void construct_new_path(BUFFER *,BUFFER *,char *);
gboolean is_a_dir_t(char *,time_t *t);
gboolean is_a_dir(char *);
int remove_dircontent(char *,char *,gboolean);
int move_dircontent(char *,char *);

GdkCursor *Create_Cursor(GtkWidget *window,int type);
GtkWidget *Create_RGBPic(GtkWidget *,char *,int,int,int,int,int (),gboolean(),void (),void (),GtkWidget *);
void Close_RGBPic(GtkWidget *widget);
GtkWidget *Create_RGB_1bPic_b(GtkWidget *,char *,int ,int ,int ,int ,char *,gboolean,gboolean,void func1(),void func2(),GtkWidget *,void func());
GtkWidget *Create_RGB_1bPic(GtkWidget *,char *,int ,int ,int ,int ,char *,gboolean,gboolean,void func1(),void func2(),void func());
GtkWidget *Create_RGB_2bPic(GtkWidget *,char *,int ,int ,int ,int ,guint16 *,gboolean,void func1(),void func());
void Update_RGBPic(RGBPICINFO *rgbpicinfo);
void RGB_Pic_drawableupdate(GtkWidget *drawing_area,int y1,int y2,gboolean cursor_update);

float float32_from_le(float f);
float float32_to_le(float f);

#endif
