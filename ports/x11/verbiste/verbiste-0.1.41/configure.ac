# $Id: configure.ac,v 1.109 2014/04/06 23:30:40 sarrazip Exp $

AC_PREREQ(2.54)

AC_INIT(verbiste, 0.1.41)
AC_CONFIG_SRCDIR(src/Makefile.am)
AM_INIT_AUTOMAKE

API=`echo $VERSION | cut -d . -f -2`; AC_SUBST(API)
RELEASE=`echo $VERSION | cut -d . -f 3`; AC_SUBST(RELEASE)

COPYRIGHT_YEARS="2003-2014"; AC_SUBST(COPYRIGHT_YEARS)

MANUAL_DATE_EN="April 6th, 2014"; AC_SUBST(MANUAL_DATE_EN)
MANUAL_DATE_FR="6 avril 2014"; AC_SUBST(MANUAL_DATE_FR)  # UTF-8

LIBXML2_MINVER=2.4.0; AC_SUBST(LIBXML2_MINVER)
GTK_MINVER=2.6.0; AC_SUBST(GTK_MINVER)
LIBGNOMEUI_MINVER=2.0.0; AC_SUBST(LIBGNOMEUI_MINVER)
LIBPANELAPPLET_MINVER=2.0.0; AC_SUBST(LIBPANELAPPLET_MINVER)
LIBMATEPANELAPPLET_MINVER=1.6.0; AC_SUBST(LIBMATEPANELAPPLET_MINVER)

PACKAGE_FULL_NAME="Verbiste"
PACKAGE_SUMMARY_EN="French conjugation system"
PACKAGE_SUMMARY_FR="Système de conjugaison française"  # UTF-8
AC_SUBST(PACKAGE_FULL_NAME)
AC_SUBST(PACKAGE_SUMMARY_EN)
AC_SUBST(PACKAGE_SUMMARY_FR)


AC_DISABLE_STATIC
AC_PROG_LIBTOOL		dnl This must come after AC_DISABLE_STATIC.

AM_ICONV

PKG_CHECK_MODULES(LIBXML2, libxml-2.0 >= $LIBXML2_MINVER)
AC_SUBST(LIBXML2_CFLAGS)
AC_SUBST(LIBXML2_LIBS)

dnl  The xmllint program comes with libxml2.
AC_PATH_PROG(XMLLINT, xmllint)


AC_PROG_CXX
AC_LANG_CPLUSPLUS
AC_COMPILE_WARNINGS

# Automake 1.10 says: compiling `conjugator.c' with per-target
# flags requires `AM_PROG_CC_C_O' in `configure.ac'
AM_PROG_CC_C_O

GETOPT_LONG


AM_GNU_GETTEXT

dnl  nl_langinfo and CODESET are required by the command-line tools
dnl  to determine if the terminal and command line use UTF-8 instead
dnl  of Latin-1.  AM_GNU_GETTEXT makes the check, but here we abort
dnl  if the required functionality is not available.
dnl  See the "UTF-8 and Unicode FAQ for Unix/Linux" by Markus Kuhn:
dnl  http://www.cl.cam.ac.uk/~mgk25/unicode.html#activate
if test $am_cv_langinfo_codeset != yes; then
	AC_MSG_ERROR([nl_langinfo and CODESET are required])
fi

AC_PROG_PERL_MODULES(IPC::Open2 XML::Parser,
	,
	AC_MSG_ERROR([missing some Perl modules required for testing]))


###############################################################################

build_gtk_app=no
build_gnome_app=no
build_gnome_applet=no
build_mate_applet=no

AC_ARG_WITH([gtk-app],
[  --with-gtk-app       build GTK+ app (default: yes)],
[
	if test "$withval" = no; then
		build_gtk_app=no
	else
		build_gtk_app=yes
	fi
],
[
	build_gtk_app=yes
])

AC_ARG_WITH([gnome-app],
[  --with-gnome-app     build GNOME app (default: yes)],
[
	if test "$withval" = no; then
		build_gnome_app=no
	else
		build_gnome_app=yes
	fi
],
[
	build_gnome_app=yes
])

AC_ARG_WITH([gnome-applet],
[  --with-gnome-applet  build GNOME Panel applet (default: no)],
[
	if test "$withval" = no; then
		build_gnome_applet=no
	else
		build_gnome_applet=yes
	fi
],
[
	build_gnome_applet=no
])

AC_ARG_WITH([mate-applet],
[  --with-mate-applet   build MATE Panel applet (default: no)],
[
	if test "$withval" = no; then
		build_mate_applet=no
	else
		build_mate_applet=yes
	fi
],
[
	build_mate_applet=no
])

AC_ARG_WITH([console-app],
[  --with-console-app   build GUI-equivalent console app for testing (default: no)],
[
	if test "$withval" = no; then
		build_console_app=no
	else
		build_console_app=yes
	fi
],
[
	build_console_app=no
])


AC_MSG_RESULT([building GTK+ application: $build_gtk_app])
AC_MSG_RESULT([building GNOME application: $build_gnome_app])
AC_MSG_RESULT([building GNOME Panel applet: $build_gnome_applet])
AC_MSG_RESULT([building MATE Panel applet: $build_mate_applet])
AC_MSG_RESULT([building console application: $build_console_app])

AM_CONDITIONAL(BUILD_GNOME_APP, [test "$build_gnome_app" = yes])
AM_CONDITIONAL(BUILD_GNOME_APPLET, [test "$build_gnome_applet" = yes])
AM_CONDITIONAL(BUILD_MATE_APPLET, [test "$build_mate_applet" = yes])
AM_CONDITIONAL(BUILD_GTK_APP, [test "$build_gtk_app" = yes])
AM_CONDITIONAL(BUILD_GTK, [test "$build_gtk_app" = yes -o "$build_gnome_app" = yes -o "$build_gnome_applet" = yes -o "$build_mate_applet" = yes])
AM_CONDITIONAL(BUILD_CONSOLE_APP, [test "$build_console_app" = yes])
AM_CONDITIONAL(BUILD_GUI, [test "$build_gnome_app" = yes -o "$build_gnome_applet" = yes -o "$build_mate_applet" = yes -o "$build_gtk_app" = yes -o "$build_console_app" = yes])
AM_CONDITIONAL(BUILD_GNOME, [test "$build_gnome_app" = yes -o "$build_gnome_applet" = yes])


# Check necessary GTK+/GNOME libraries.

if test "$build_gnome_app" = yes -o "$build_gnome_applet" = yes -o "$build_mate_applet" = yes; then
	PKG_CHECK_MODULES(LIBGNOMEUI, libgnomeui-2.0 >= $LIBGNOMEUI_MINVER)
	AC_SUBST(LIBGNOMEUI_CFLAGS)
	AC_SUBST(LIBGNOMEUI_LIBS)
fi

if test "$build_gnome_applet" = yes; then
	APPLET_FULL_NAME="Verbiste Applet";
	APPLET_FULL_NAME_FR="Verbiste";
	AC_SUBST(APPLET_FULL_NAME)
	AC_SUBST(APPLET_FULL_NAME_FR)
	APPLET_DESCRIPTION="French conjugation system"
	APPLET_DESCRIPTION_FR="Système de conjugaison française"
	AC_SUBST(APPLET_DESCRIPTION)
	AC_SUBST(APPLET_DESCRIPTION_FR)
	APPLET_CATEGORY="Accessories"
	APPLET_CATEGORY_FR="Accessoires"
	AC_SUBST(APPLET_CATEGORY)
	AC_SUBST(APPLET_CATEGORY_FR)

	PKG_CHECK_MODULES(LIBPANELAPPLET,
				libpanelapplet-2.0 >= $LIBPANELAPPLET_MINVER)
	AC_SUBST(LIBPANELAPPLET_CFLAGS)
	AC_SUBST(LIBPANELAPPLET_LIBS)
fi

if test "$build_mate_applet" = yes; then
	APPLET_FULL_NAME="Verbiste Applet";
	APPLET_FULL_NAME_FR="Verbiste";
	AC_SUBST(APPLET_FULL_NAME)
	AC_SUBST(APPLET_FULL_NAME_FR)
	APPLET_DESCRIPTION="French conjugation system"
	APPLET_DESCRIPTION_FR="Système de conjugaison française"
	AC_SUBST(APPLET_DESCRIPTION)
	AC_SUBST(APPLET_DESCRIPTION_FR)
	APPLET_CATEGORY="Accessories"
	APPLET_CATEGORY_FR="Accessoires"
	AC_SUBST(APPLET_CATEGORY)
	AC_SUBST(APPLET_CATEGORY_FR)

	PKG_CHECK_MODULES(LIBMATEPANELAPPLET,
				libmatepanelapplet-4.0 >= $LIBMATEPANELAPPLET_MINVER
				libbonobo-2.0
				bonobo-activation-2.0
				libbonoboui-2.0
				ORBit-2.0)
	AC_SUBST(LIBMATEPANELAPPLET_CFLAGS)
	AC_SUBST(LIBMATEPANELAPPLET_LIBS)
fi

if test "$build_gtk_app" = yes \
		-o "$build_gnome_app" = yes \
		-o "$build_gnome_applet" = yes \
		-o "$build_mate_applet" = yes; then
	PKG_CHECK_MODULES(GTK, gtk+-2.0 >= $GTK_MINVER)
	AC_SUBST(GTK_CFLAGS)
	AC_SUBST(GTK_LIBS)
fi

# Disable NLS if only the command-line tools are used.
AC_MSG_CHECKING([if NLS support is needed])
if test "$build_gtk_app" = yes \
		-o "$build_gnome_app" = yes \
		-o "$build_gnome_applet" = yes; then
	AC_MSG_RESULT([yes])
else
	USE_NLS=no
	AC_MSG_RESULT([no])
fi


###############################################################################


AC_ARG_WITH([examples],
[  --with-examples      build examples (default: yes)],
[
	if test "$withval" = no; then
		build_examples=no
	else
		build_examples=yes
	fi
],
[
	build_examples=yes
])

AC_MSG_RESULT([building examples: $build_examples])

AM_CONDITIONAL(BUILD_EXAMPLES, [test "$build_examples" = yes])


###############################################################################

AC_CONFIG_FILES([
	intl/Makefile
	po/Makefile.in
	macros/Makefile
	Makefile
	verbiste.spec
	verbiste-0.1.pc
	src/Makefile
	src/verbiste/Makefile
	src/commands/Makefile
	src/gui/Makefile
	src/gtk/Makefile
	src/gnome/verbiste.desktop
	src/gnome/Makefile
	src/mate/Makefile
	doc/fr/verbiste.3
	doc/fr/french-conjugator.1
	doc/fr/french-deconjugator.1
	doc/verbiste.3
	doc/french-conjugator.1
	doc/french-deconjugator.1
	examples/Makefile
])
AC_OUTPUT
