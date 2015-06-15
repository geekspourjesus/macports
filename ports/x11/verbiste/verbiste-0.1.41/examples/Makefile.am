# $Id: Makefile.am,v 1.10 2013/12/08 20:11:54 sarrazip Exp $

libdatadir = $(datadir)/$(PACKAGE)-$(API)

noinst_PROGRAMS = deconjugator conjugator

deconjugator_SOURCES = \
	deconjugator.c 

deconjugator_CFLAGS = \
	-DLIBDATADIR=\"$(libdatadir)\" \
	-I$(top_srcdir)/src

deconjugator_LDADD = \
	$(LIBXML2_LIBS) \
	-lstdc++ \
	../src/verbiste/libverbiste-$(API).la

conjugator_SOURCES = \
	conjugator.c 

conjugator_CFLAGS = \
	-DLIBDATADIR=\"$(libdatadir)\" \
	-I$(top_srcdir)/src

conjugator_LDADD = \
	$(LIBXML2_LIBS) \
	-lstdc++ \
	../src/verbiste/libverbiste-$(API).la

# This makefile is supposed to be encoded in UTF-8 in lines
# where $(LU) is used.
# Setting LC_ALL is necessary on Solaris.
#
LU = LIBDATADIR=$(top_srcdir)/data LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8

# These test lines are in Latin-1, except where noted otherwise:
check: all
	test "`$(LU) ./deconjugator aimerions`" = "aimer, conditional, present, 1, plural"
	test "`$(LU) ./deconjugator était`" = "être, indicative, imperfect, 3, singular"
	test "`$(LU) ./deconjugator x`" = ""
	test "`$(LU) ./conjugator avoir | grep ayant`" = "ayant"
	test "`$(LU) ./conjugator asseoir | grep asseyerai,`" = "assiérai, asseyerai, assoirai"
	test "`$(LU) ./conjugator x`" = "Unknown infinitive."
	test "`echo '- imperative present:' | $(srcdir)/trans-tenses.pl`" = "- impératif présent:"  # this line in UTF-8
	@echo "Success."

EXTRA_DIST = \
	README \
	test-french-deconjugator.pl \
	FrenchDeconjugatorTester.java \
	trans-tenses.pl \
	generate-all-forms.pl

MAINTAINERCLEANFILES = Makefile.in
