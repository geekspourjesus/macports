# $Id: Makefile.am,v 1.2 2006/07/08 03:27:41 sarrazip Exp $

localedir = $(datadir)/locale
libdatadir = $(datadir)/$(PACKAGE)-$(API)

noinst_LTLIBRARIES = libconjugation.la

libconjugation_la_SOURCES = \
	conjugation.cpp \
	conjugation.h

libconjugation_la_CXXFLAGS = \
	$(LIBXML2_CFLAGS) \
        -I$(top_srcdir)/src \
	-DPACKAGE_FULL_NAME=\"$(PACKAGE_FULL_NAME)\" \
	-DGETTEXT_PACKAGE=\"$(PACKAGE)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDATADIR=\"$(libdatadir)\" \
	-I../intl -I$(top_srcdir)/intl

libconjugation_la_LIBADD = \
	$(LIBXML2_LIBS) \
        ../verbiste/libverbiste-$(API).la \
	$(INTLLIBS) $(LIBS)

MAINTAINERCLEANFILES = Makefile.in
