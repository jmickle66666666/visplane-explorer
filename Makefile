#----------------------------------------------------------------
#  Visplane Explorer
#----------------------------------------------------------------
#
# GNU Makefile for Unix/Linux version
#

PROGRAM=visplane-explorer

# prefix choices: /usr  /usr/local  /opt
INSTALL_PREFIX=/usr/local

CXX=g++

OBJ_DIR=obj_linux

OPTIMISE=-O2

# operating system choices: UNIX WIN32
OS=UNIX


#--- Internal stuff from here -----------------------------------

CXXFLAGS=$(OPTIMISE) -Wall -D$(OS) -Ivpo_lib \
         -D_THREAD_SAFE -D_REENTRANT

LDFLAGS=-L/usr/X11R6/lib

LIBS=	-lfltk_images -lfltk \
	-lX11 -lXext -lXft -lfontconfig -lXinerama \
	-lz -lm


#----- Common stuff (objects) -------------------------------------

include make_common


#----- Targets ----------------------------------------------------

stripped: $(PROGRAM)
	strip --strip-unneeded $(PROGRAM)

install: stripped
	install -o root -m 755 $(PROGRAM) $(INSTALL_PREFIX)/bin/
	install -o root -m 644 -D misc/visplane-explorer.6 $(INSTALL_PREFIX)/share/man/man6/
	#
	xdg-desktop-menu  install --novendor misc/visplane-explorer.desktop
	xdg-icon-resource install --novendor --size 32 misc/visplane-explorer.xpm

uninstall:
	rm -v  $(INSTALL_PREFIX)/bin/$(PROGRAM)
	rm -vf $(INSTALL_PREFIX)/man/man6/visplane-explorer.6
	#
	xdg-desktop-menu  uninstall --novendor misc/visplane-explorer.desktop
	xdg-icon-resource uninstall --novendor --size 32 visplane-explorer

.PHONY: stripped install uninstall

#--- editor settings ------------
# vi:ts=8:sw=8:noexpandtab
