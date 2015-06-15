/*  $Id: gnome-app.cpp,v 1.16 2013/05/20 21:01:30 sarrazip Exp $
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

#include "gtk/main-window.h"
#include "gui/conjugation.h"
#include "gtk/util.h"

#include <libintl.h>
#include <locale.h>
#include <libgnomeui/libgnomeui.h>
#include <stdlib.h>
#include <iostream>

using namespace std;


static
struct poptOption options[] =
{
    {
        NULL,
        '\0',
        0,
        NULL,
        0,
        NULL,
        NULL
    }
};


static
poptContext
get_poptContext(GnomeProgram *program)
{
    GValue value;
    ::memset(&value, '\0', sizeof(value));
    g_value_init(&value, G_TYPE_POINTER);
    g_object_get_property(G_OBJECT(program),
                            GNOME_PARAM_POPT_CONTEXT, &value);
    return (poptContext) g_value_get_pointer(&value);
}


static bool trace = getenv("TRACE") != NULL;


int
main(int argc, char *argv[])
{
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    // Special mode to test GNU gettext translations.
    //
    if (getenv("VERBISTE_TEST_GETTEXT") != NULL)
    {
        setlocale(LC_ALL, getenv("LANG"));  // normally done by gnome_program_init()

        // Print one of the translated UI strings as a representative.
        cout << gettext("_Spelling: tolerant") << endl;
        return 0;
    }

    GnomeProgram *program = gnome_program_init(
                        PACKAGE, VERSION, LIBGNOMEUI_MODULE, argc, argv,
                        GNOME_PARAM_POPT_TABLE, options,
                        GNOME_PARAM_APP_DATADIR, DATADIR,
                        NULL);
    gnome_window_icon_set_default_from_file(GNOMEICONDIR "/" PACKAGE ".png");

    poptContext pctx = get_poptContext(program);
    /*const char **args =*/ poptGetArgs(pctx);
    poptFreeContext(pctx);

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
