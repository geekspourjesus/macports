# $Id: Makefile.am,v 1.13 2014/04/06 22:44:23 sarrazip Exp $

SUBDIRS = verbiste
if BUILD_GUI
SUBDIRS += gui
endif
if BUILD_GTK
SUBDIRS += gtk
endif
if BUILD_GNOME
SUBDIRS += gnome
endif
if BUILD_MATE_APPLET
SUBDIRS += mate
endif

# Commands are added last, because console-app, if built, needs 'gui'.
SUBDIRS += commands

MAINTAINERCLEANFILES = Makefile.in
