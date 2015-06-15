/*  $Id: gtk-app.cpp,v 1.9 2013/05/20 21:01:30 sarrazip Exp $
    gtk-app.cpp - GTK+ application main function

    verbiste - French conjugation system
    Copyright (C) 2003-2013 Pierre Sarrazin <http://sarrazip.com/>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "main-window.h"
#include "gui/conjugation.h"
#include "gtk/util.h"

#include <stdlib.h>
#include <locale.h>
#include <libintl.h>
#define _(x) gettext(x)

#include <gtk/gtk.h>

#ifdef HAVE_GETOPT_LONG
#include <unistd.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>
#endif

#include <iostream>

using namespace std;


#ifdef HAVE_GETOPT_LONG

static struct option knownOptions[] =
{
    { "help",           no_argument,            NULL, 'h' },
    { "version",        no_argument,            NULL, 'v' },

    { NULL, 0, NULL, 0 }  // marks the end
};


static
void
displayVersionNo()
{
    cout << PACKAGE << ' ' << VERSION << '\n';
}


static
void
displayHelp()
{
    cout << '\n';

    displayVersionNo();

    cout << "Part of " << PACKAGE << " " << VERSION << "\n";

    cout <<
"\n"
"Copyright (C) 2003-2006 Pierre Sarrazin <http://sarrazip.com/>\n"
"This program is free software; you may redistribute it under the terms of\n"
"the GNU General Public License.  This program has absolutely no warranty.\n"
    ;

    cout <<
"\n"
"Known options:\n"
"--help             Display this help page and exit\n"
"--version          Display this program's version number and exit\n"
"\n"
    ;
}

#endif  /* HAVE_GETOPT_LONG */


int
main(int argc, char *argv[])
{
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);


    #ifdef HAVE_GETOPT_LONG

    /*  Interpret the command-line options:
    */
    int c;
    do
    {
        c = getopt_long(argc, argv, "hv", knownOptions, NULL);

        switch (c)
        {
            case EOF:
                break;  // nothing to do

            case 'v':
                displayVersionNo();
                return EXIT_SUCCESS;

            case 'h':
                displayHelp();
                return EXIT_SUCCESS;

            default:
                displayHelp();
                return EXIT_FAILURE;
        }
    } while (c != EOF && c != '?');

    #endif  /* ndef HAVE_GETOPT_LONG */


    gtk_init(&argc, &argv);

    hideOnDelete = FALSE;

    string errorMsg = initDictPointers();
    if (!errorMsg.empty())
    {
        showErrorDialog(PACKAGE_FULL_NAME + string(": ") + errorMsg);
        return EXIT_FAILURE;
    }

    // Look for the first non-option argument and use it as the initial
    // text value to process. Calling processText() here also forces
    // the result window to be initially displayed.
    //
    const char *initText = "";
    for (int i = 1; i < argc; ++i)
        if (argv[i][0] != '-')
        {
            initText = argv[i];
            break;
        }
    processText(initText);

    gtk_main();
    return EXIT_SUCCESS;
}
