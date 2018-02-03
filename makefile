#######################################################
# Makefile for sgtk, to create object files or lib/dll
#   A set of gtk high level functions to create in an easy way a gui based on GTK2.0
#   You must have installed the developpers gtk source files.
#
# Usage:
#   Linux:
#     make all                               compile to objects
#     make dlib                              also compile to lib file
#
#   Windows: (wingnu)
#     export OS=\"windows\"; make all        compile to objects
#     export OS=\"windows\"; make dlib       compile to objects
#
# Instead of export, you can change default OS wo windows.
#######################################################
#Choose linux or windows
OS="linux"

#Choose gtk version: 2.0 or 1.2
GTK_REL="2.0"

DBGFLAG=

ifneq ($(OS),"linux")
  ifneq ($(OS),"windows")
    ifneq ($(OS),"solaris")
all:
	echo "Wrong choice $(OS)."

    endif
  endif
endif

ifeq ($(OS),"solaris")
  OS="linux"
endif

#Define c-flags for compiler.
#  Linux: use pkg-config
#  Windows: no pkg-config?? 
#    location of gtk header files: here points to: c:/bin/wingnu/gtk/include
#    ==> CHANGE TO YOUR INSTAL. LOC!
ifeq ($(OS),"linux")
  GTKCONFIG_CFLAG=`pkg-config --cflags gtk+-2.0` -D__GTK_20__
else
  GTKCONFIG_CFLAG=-Ic:/bin/wingnu/gtk/include/gppm3 -Ic:/bin/wingnu/gtk/include/gtkp -Ic:/bin/wingnu/gtk/include/gtkp/gdk
  CFLAGS_EXTRA=-D__GTK_WIN32__=1
endif

CC=gcc -Wall $(INCL_OPT) $(GTKCONFIG_CFLAG) $(DBGFLAG)

LOC_SGTK=.
#GTK sgtk files
SRC_GUI=$(LOC_SGTK)/sgtk_misc.c $(LOC_SGTK)/windows.c $(LOC_SGTK)/filemngr.c \
        $(LOC_SGTK)/canvas.c $(LOC_SGTK)/rgbpic.c \
        $(LOC_SGTK)/buttons.c $(LOC_SGTK)/listtree.c \
        $(LOC_SGTK)/adjust.c $(LOC_SGTK)/menus.c $(LOC_SGTK)/packing.c \
        $(LOC_SGTK)/rgbdraw.c $(LOC_SGTK)/gtk_led.c
INC_GUI=$(LOC_SGTK)/sgtk.h $(LOC_SGTK)/gtk_led.h $(LOC_SGTK)/sgtk_functions.h

ALL_SRC=$(SRC_GUI) 

ALL_INC=$(INC_GUI)
ALL_OBJ=$(ALL_SRC:.c=.o)

all: $(ALL_OBJ) 

%.o: %.c $(ALL_INC)
	$(CC) -c $(CCFLAGS) $(CFLAGS_EXTRA) -o $@ $<

ifeq ($(OS),"linux")
dlib: libsgtk.so

libsgtk.so: $(ALL_OBJ)
	$(CC) -o libsgtk.so -shared $(ALL_OBJ)
else
dlib: sgtk.dll
sgtk.dll: $(ALL_OBJ)
	dlltool --export-all --output-def sgtk.def $(ALL_OBJ)
	dlltool --dllname sgtk.dll --def sgtk.def --output-lib libjsgtk.a
	dllwrap -o sgtk.dll --def sgtk.def $(ALL_OBJ)
endif

arch:
	zip sgtk_src.zip $(SRC_GUI) $(INC_GUI) makefile
