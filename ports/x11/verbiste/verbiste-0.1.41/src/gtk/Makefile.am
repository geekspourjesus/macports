# $Id: Makefile.am,v 1.12 2013/11/01 02:55:51 sarrazip Exp $

if BUILD_GTK_APP
bin_PROGRAMS = verbiste-gtk
endif

noinst_LTLIBRARIES = libgtkapp.la

localedir = $(datadir)/locale
hicolordir = $(datadir)/icons/hicolor
pixmapdir = $(hicolordir)/scalable/apps
libdatadir = $(datadir)/$(PACKAGE)-$(API)
latexdir = $(datadir)/texmf/tex/latex/$(PACKAGE)

###############################################################################

libgtkapp_la_SOURCES = \
	main-window.cpp \
	main-window.h \
	util.h \
	util.cpp

libgtkapp_la_CXXFLAGS = \
	$(LIBXML2_CFLAGS) \
	-I$(top_srcdir)/src \
	-DPACKAGE_FULL_NAME=\"$(PACKAGE_FULL_NAME)\" \
	-DCOPYRIGHT_YEARS=\"$(COPYRIGHT_YEARS)\" \
	-DGETTEXT_PACKAGE=\"$(PACKAGE)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DPIXMAPDIR=\"$(pixmapdir)\" \
	-DLIBDATADIR=\"$(libdatadir)\" \
	-I../intl -I$(top_srcdir)/intl \
	$(GTK_CFLAGS)

libgtkapp_la_LIBADD = \
	$(LIBXML2_LIBS) \
	../verbiste/libverbiste-$(API).la \
	../gui/libconjugation.la \
	$(GTK_LIBS) \
	$(INTLLIBS) $(LIBS)

###############################################################################

if BUILD_GTK_APP

verbiste_gtk_SOURCES = \
	gtk-app.cpp \
	config.cpp

verbiste_gtk_CXXFLAGS = \
	$(LIBXML2_CFLAGS) \
	-I$(top_srcdir)/src \
	-DPACKAGE_FULL_NAME=\"$(PACKAGE_FULL_NAME)\" \
	-DCOPYRIGHT_YEARS=\"$(COPYRIGHT_YEARS)\" \
	-DGETTEXT_PACKAGE=\"$(PACKAGE)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	$(GTK_CFLAGS)

verbiste_gtk_LDADD = \
	libgtkapp.la

check:
	test "`env DISPLAY= ./$(PACKAGE)-gtk --version`" = "$(PACKAGE) $(VERSION)"
	env DISPLAY= ./$(PACKAGE)-gtk --version 2>&1 > /dev/null
	env DISPLAY= ./$(PACKAGE)-gtk --help 2>&1 > /dev/null

endif BUILD_GTK_APP

###############################################################################

pixmap_DATA = $(PACKAGE).svg

# These icons are installed by the 'install-data-local' target.
# See below in this file.
#
noinst_DATA = \
	$(PACKAGE)-16x16.png \
	$(PACKAGE)-32x32.png \
	$(PACKAGE)-48x48.png \
	$(PACKAGE)-64x64.png \
	$(PACKAGE)-128x128.png \
	$(PACKAGE)-256x256.png \
	$(PACKAGE)-512x512.png

# Generate PNG versions of the SVG application icon and install them each
# in their size-dependent directory.
#
PNG_SIZES = 16 32 48 64 128 256 512
install-data-local:
	for size in $(PNG_SIZES); do \
	    dir="$(DESTDIR)$(hicolordir)/$${size}x$$size/apps"; \
	    echo "Installing $${size}x$$size PNG icon file in $$dir"; \
	    $(MKINSTALLDIRS) -m 755 "$$dir"; \
	    $(INSTALL_DATA) $(srcdir)/$(PACKAGE)-$${size}x$$size.png "$$dir/$(PACKAGE).png"; \
	done

# Undo the actions of install-data-local.
#
uninstall-local:
	for size in $(PNG_SIZES); do \
	    dir="$(DESTDIR)$(hicolordir)/$${size}x$$size/apps"; \
	    echo "Removing $${size}x$$size PNG icon file from $$dir"; \
	    rm -f "$$dir/$(PACKAGE).png"; \
	done

###############################################################################

# LaTeX document class file for LaTeX export feature.
latex_DATA = $(PACKAGE).cls $(PACKAGE).cfg

###############################################################################

EXTRA_DIST = $(pixmap_DATA) $(noinst_DATA) $(latex_DATA)

MAINTAINERCLEANFILES = Makefile.in
