# $Id: Makefile.am,v 1.11 2010/04/18 20:10:35 sarrazip Exp $

lib_LTLIBRARIES = libverbiste-0.1.la

libdatadir = $(datadir)/$(PACKAGE)-$(API)

libverbiste_0_1_la_SOURCES = \
	FrenchVerbDictionary.cpp \
	FrenchVerbDictionary.h \
	misc-types.cpp \
	misc-types.h \
	c-api.cpp \
	c-api.h \
	Trie.h

libverbiste_0_1_la_CXXFLAGS = \
	-I$(top_srcdir)/src \
	-DLIBDATADIR=\"$(libdatadir)\" \
	$(LIBXML2_CFLAGS)

libverbiste_0_1_la_LIBADD = \
	$(LIBXML2_LIBS)

pkgincludedir = $(includedir)/$(PACKAGE)-$(API)/$(PACKAGE)
pkginclude_HEADERS = \
	misc-types.h \
	c-api.h \
	FrenchVerbDictionary.h \
	Trie.cpp \
	Trie.h

TESTS = checkxml

check_PROGRAMS = checkxml

checkxml_SOURCES = checkxml.cpp

checkxml_CXXFLAGS = \
	-DVERBSFRXML=\"$(top_srcdir)/data/verbs-fr.xml\" \
	-DCONJUGATIONFRXML=\"$(top_srcdir)/data/conjugation-fr.xml\" \
	$(LIBXML2_CFLAGS)

checkxml_LDADD = \
	$(LIBXML2_LIBS)

doc:
	doxygen $(PACKAGE).dox
	@echo "HTML documentation should now be in 'html' subdirectory."

EXTRA_DIST = $(pkginclude_HEADERS) $(PACKAGE).dox

MAINTAINERCLEANFILES = Makefile.in
