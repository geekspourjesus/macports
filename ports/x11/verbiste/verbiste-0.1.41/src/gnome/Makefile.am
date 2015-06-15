# $Id: Makefile.am,v 1.25 2010/12/19 18:00:13 sarrazip Exp $

if BUILD_GNOME_APP
bin_PROGRAMS = verbiste
endif

if BUILD_GNOME_APPLET
libexec_PROGRAMS = verbiste-applet
endif

localedir = $(datadir)/locale
pixmapdir = $(datadir)/pixmaps
libdatadir = $(datadir)/$(PACKAGE)-$(API)

###############################################################################

if BUILD_GNOME_APP

verbiste_SOURCES = \
	gnome-app.cpp \
	config.cpp

verbiste_CXXFLAGS = \
	$(LIBXML2_CFLAGS) \
        -I$(top_srcdir)/src \
	-DPACKAGE_FULL_NAME=\"$(PACKAGE_FULL_NAME)\" \
	-DGETTEXT_PACKAGE=\"$(PACKAGE)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DGNOMEICONDIR=\"$(pixmapdir)\" \
	-DPIXMAPDIR=\"$(pixmapdir)\" \
	-DLIBDATADIR=\"$(libdatadir)\" \
	-I../intl -I$(top_srcdir)/intl \
	$(LIBGNOMEUI_CFLAGS)

verbiste_LDADD = \
	$(LIBXML2_LIBS) \
        ../verbiste/libverbiste-$(API).la \
        ../gtk/libgtkapp.la \
	../gui/libconjugation.la \
	$(LIBGNOMEUI_LIBS) \
	$(INTLLIBS) $(LIBS)

desktopdir = $(datadir)/applications
desktop_DATA = $(PACKAGE).desktop

check:
	@echo "Testing that --help exits with status zero:"
	env DISPLAY= ./$(PACKAGE) --help >/dev/null
	@echo "Testing that --version exits with status zero:"
	env DISPLAY= ./$(PACKAGE) --version >/dev/null
	@echo "Testing that --version returns right version number:"
	test "`env DISPLAY= ./$(PACKAGE) --version | sed 's/gnome/GNOME/i'`" = "GNOME $(PACKAGE) $(VERSION)"

# Must not be run before a successful install, because GNU gettext
# needs to load one of the .mo files, which are created at installation.
#
check-gettext:
	@echo "Testing GNU gettext user interface translations:"
	test "`env DISPLAY= VERBISTE_TEST_GETTEXT=1 LANG=en_US ./$(PACKAGE)`" = "_Spelling: tolerant"
	test "`env DISPLAY= VERBISTE_TEST_GETTEXT=1 LANG=fr_CA ./$(PACKAGE)`" = "_Orthographe: tolérante"  # UTF-8 line
	test "`env DISPLAY= VERBISTE_TEST_GETTEXT=1 LANG=de_DE ./$(PACKAGE)`" = "_Orthographie: Tolerant"
	test "`env DISPLAY= VERBISTE_TEST_GETTEXT=1 LANG=el_GR ./$(PACKAGE)`" = "_Ορθογραφία: χαλαρή"  # UTF-8 line
	test "`env DISPLAY= VERBISTE_TEST_GETTEXT=1 LANG=da_DK ./$(PACKAGE)`" = "_Stavekontrol: Tolerant"

endif BUILD_GNOME_APP

###############################################################################

if BUILD_GNOME_APPLET

verbiste_applet_SOURCES = \
	panel-applet.cpp \
	config.cpp

verbiste_applet_CXXFLAGS = \
	$(LIBXML2_CFLAGS) \
        -I$(top_srcdir)/src \
	-DPACKAGE_FULL_NAME=\"$(PACKAGE_FULL_NAME)\" \
	-DGETTEXT_PACKAGE=\"$(PACKAGE)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DGNOMELOCALEDIR=\"$(localedir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DGNOMEICONDIR=\"$(pixmapdir)\" \
	-DPIXMAPDIR=\"$(pixmapdir)\" \
	-DLIBDATADIR=\"$(libdatadir)\" \
	-I../intl -I$(top_srcdir)/intl \
	$(LIBPANELAPPLET_CFLAGS)

verbiste_applet_LDADD = \
	$(LIBXML2_LIBS) \
        ../verbiste/libverbiste-$(API).la \
        ../gtk/libgtkapp.la \
	../gui/libconjugation.la \
	$(LIBPANELAPPLET_LIBS) \
	$(INTLLIBS) $(LIBS)

bonoboserverdir = $(libdir)/bonobo/servers
bonoboserver_DATA = $(PACKAGE).server

$(PACKAGE).server: $(PACKAGE).server.pre
	sed -e 's|%{libexecdir}|$(libexecdir)|g' \
		-e 's|%{PACKAGE}|$(PACKAGE)|g' \
		-e 's|%{APPLET_FULL_NAME}|$(APPLET_FULL_NAME)|g' \
		-e 's|%{APPLET_FULL_NAME_FR}|$(APPLET_FULL_NAME_FR)|g' \
		-e 's|%{APPLET_DESCRIPTION}|$(APPLET_DESCRIPTION)|g' \
		-e 's|%{APPLET_DESCRIPTION_FR}|$(APPLET_DESCRIPTION_FR)|g' \
		-e 's|%{APPLET_CATEGORY}|$(APPLET_CATEGORY)|g' \
		-e 's|%{APPLET_CATEGORY_FR}|$(APPLET_CATEGORY_FR)|g' \
			< $< | grep -v '^#' > $@.tmp
	mv -f $@.tmp $@

uninstall-local:
	rm -f $(DESTDIR)$(bonoboserverdir)/$(PACKAGE).server

endif BUILD_GNOME_APPLET

###############################################################################

EXTRA_DIST = $(PACKAGE).server.pre

CLEANFILES = $(PACKAGE).server $(PACKAGE).server.tmp

MAINTAINERCLEANFILES = Makefile.in
