# $Id: Makefile.am,v 1.26 2013/06/28 17:38:59 sarrazip Exp $

SUBDIRS = intl src po macros
if BUILD_EXAMPLES
SUBDIRS += examples
endif

libdatadir = $(datadir)/$(PACKAGE)-$(API)
libdata_DATA = \
	data/conjugation-fr.xml \
	data/verbs-fr.xml \
	data/conjugation-it.xml \
	data/verbs-it.xml \
	data/conjugation-el.xml \
	data/verbs-el.xml \
	COPYING
# COPYING contains the license; main-window.cpp expects it there.

docdir = $(datadir)/doc/$(PACKAGE)-$(VERSION)
doc_DATA = \
	AUTHORS \
	COPYING \
	NEWS \
	README \
	LISEZMOI \
	HACKING \
	THANKS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(PACKAGE)-$(API).pc

man_MANS = \
	doc/$(PACKAGE).3 \
	doc/french-conjugator.1 \
	doc/french-deconjugator.1

frman1dir = $(mandir)/fr/man1
frman1_DATA = \
	doc/fr/french-conjugator.1 \
	doc/fr/french-deconjugator.1

frman3dir = $(mandir)/fr/man3
frman3_DATA = \
	doc/fr/$(PACKAGE).3

RPMBUILD = rpmbuild
rpm: dist
	$(RPMBUILD) -ta $(distdir).tar.gz

VALIDATEXML = $(XMLLINT) --noout --valid

.PHONY: check-data
check-data:
	$(VALIDATEXML) $(srcdir)/data/verbs-el.xml
	$(VALIDATEXML) $(srcdir)/data/conjugation-el.xml
	$(VALIDATEXML) $(srcdir)/data/verbs-it.xml
	$(VALIDATEXML) $(srcdir)/data/conjugation-it.xml
	$(VALIDATEXML) $(srcdir)/data/verbs-fr.xml
	$(VALIDATEXML) $(srcdir)/data/conjugation-fr.xml
	$(PERL) -cw $(srcdir)/data/check-data.pl
	$(PERL) $(srcdir)/data/check-data.pl $(srcdir)/data

check: check-data

DISTCHECK_CONFIGURE_FLAGS = --without-included-gettext

EXTRA_DIST = \
	bootstrap \
	autogen.sh \
	config.rpath \
	mkinstalldirs \
	LISEZMOI \
	HACKING \
	$(libdata_DATA) \
	$(PACKAGE).spec \
	doc/fr/verbiste.3.in \
	doc/fr/french-conjugator.1.in \
	doc/fr/french-deconjugator.1.in \
	doc/verbiste.3.in \
	doc/french-conjugator.1.in \
	doc/french-deconjugator.1.in \
	data/check-data.pl

ACLOCAL_AMFLAGS = -I macros

MAINTAINERCLEANFILES = \
	Makefile.in \
	aclocal.m4 \
	configure \
	config.guess \
	config.sub \
	depcomp \
	install-sh \
	ltmain.sh \
	missing \
	compile
