# $Id: Makefile.am,v 1.2 2014/04/06 23:14:00 sarrazip Exp $

if BUILD_MATE_APPLET
libexec_PROGRAMS = verbiste-mate-applet
endif

localedir = $(datadir)/locale
pixmapdir = $(datadir)/pixmaps
libdatadir = $(datadir)/$(PACKAGE)-$(API)

###############################################################################

if BUILD_MATE_APPLET

verbiste_mate_applet_SOURCES = \
	panel-applet.cpp \
	config.cpp

verbiste_mate_applet_CXXFLAGS = \
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
	$(LIBMATEPANELAPPLET_CFLAGS)

verbiste_mate_applet_LDADD = \
	$(LIBXML2_LIBS) \
        ../verbiste/libverbiste-$(API).la \
        ../gtk/libgtkapp.la \
	../gui/libconjugation.la \
	$(LIBMATEPANELAPPLET_LIBS) \
	$(INTLLIBS) $(LIBS)

mateappletsdir = $(datadir)/mate-panel/applets
mateapplets_DATA = org.mate.applets.VerbisteApplet.mate-panel-applet

org.mate.applets.VerbisteApplet.mate-panel-applet: org.mate.applets.VerbisteApplet.mate-panel-applet.pre
	sed -e 's|%{libexecdir}|$(libexecdir)|g' \
		-e 's|%{PACKAGE}|$(PACKAGE)|g' \
		-e 's|%{PACKAGE_FULL_NAME}|$(PACKAGE_FULL_NAME)|g' \
		-e 's|%{APPLET_FULL_NAME}|$(APPLET_FULL_NAME)|g' \
		-e 's|%{APPLET_FULL_NAME_FR}|$(APPLET_FULL_NAME_FR)|g' \
		-e 's|%{APPLET_DESCRIPTION}|$(APPLET_DESCRIPTION)|g' \
		-e 's|%{APPLET_DESCRIPTION_FR}|$(APPLET_DESCRIPTION_FR)|g' \
		-e 's|%{APPLET_CATEGORY}|$(APPLET_CATEGORY)|g' \
		-e 's|%{APPLET_CATEGORY_FR}|$(APPLET_CATEGORY_FR)|g' \
			< $< | grep -v '^#' > $@.tmp
	mv -f $@.tmp $@

dbusdir = $(datadir)/dbus-1/services
dbus_DATA = org.mate.panel.applet.VerbisteAppletFactory.service

org.mate.panel.applet.VerbisteAppletFactory.service: org.mate.panel.applet.VerbisteAppletFactory.service.pre
	sed -e 's|%{libexecdir}|$(libexecdir)|g' \
		-e 's|%{PACKAGE}|$(PACKAGE)|g' \
		-e 's|%{PACKAGE_FULL_NAME}|$(PACKAGE_FULL_NAME)|g' \
		-e 's|%{APPLET_FULL_NAME}|$(APPLET_FULL_NAME)|g' \
		-e 's|%{APPLET_FULL_NAME_FR}|$(APPLET_FULL_NAME_FR)|g' \
		-e 's|%{APPLET_DESCRIPTION}|$(APPLET_DESCRIPTION)|g' \
		-e 's|%{APPLET_DESCRIPTION_FR}|$(APPLET_DESCRIPTION_FR)|g' \
		-e 's|%{APPLET_CATEGORY}|$(APPLET_CATEGORY)|g' \
		-e 's|%{APPLET_CATEGORY_FR}|$(APPLET_CATEGORY_FR)|g' \
			< $< | grep -v '^#' > $@.tmp
	mv -f $@.tmp $@

uninstall-local:
	rm -f $(DESTDIR)$(mateappletsdir)/org.mate.applets.VerbisteApplet.mate-panel-applet \
	rm -f $(DESTDIR)$(dbusdir)/org.mate.panel.applet.VerbisteAppletFactory.service

endif BUILD_MATE_APPLET

###############################################################################

EXTRA_DIST = \
	org.mate.applets.VerbisteApplet.mate-panel-applet.pre \
	org.mate.panel.applet.VerbisteAppletFactory.service.pre

CLEANFILES = $(PACKAGE).server $(PACKAGE).server.tmp

MAINTAINERCLEANFILES = Makefile.in
