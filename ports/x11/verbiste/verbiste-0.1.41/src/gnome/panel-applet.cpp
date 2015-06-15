/*  $Id: panel-applet.cpp,v 1.20 2012/11/18 19:56:05 sarrazip Exp $
    panel-applet.cpp - GNOME Panel applet main functions

    verbiste - French conjugation system
    Copyright (C) 2003-2005 Pierre Sarrazin <http://sarrazip.com/>

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

#include <panel-applet.h>

#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkhbox.h>

#include <assert.h>
#include <string.h>

#include <string>

using namespace std;


static GtkWidget *appletEntry = NULL;


static
void
onAbout(BonoboUIComponent * /*uic*/, gpointer /*user_data*/, const char * /*verbname*/)
{
    showAbout();
}


static
void
onEmptyTextField(BonoboUIComponent *, gpointer, const char *)
{
    assert(GTK_IS_ENTRY(appletEntry));
    gtk_entry_set_text(GTK_ENTRY(appletEntry), "");
}


static
gboolean
onKeyPressInEntry(GtkWidget *entry, GdkEventKey *event, gpointer)
{
    g_return_val_if_fail(event != NULL, TRUE);

    switch (event->keyval)
    {
        case GDK_Return:
        case GDK_KP_Enter:
            {
                const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
                processText(text);
                gtk_entry_set_text(GTK_ENTRY(entry), "");
            }
            return TRUE;

        default:
            return FALSE;
    }
}


static
gboolean
entry_button_press_event(GtkWidget * /*widget*/,
                        GdkEventButton *event,
                        gpointer *applet)
{
    panel_applet_request_focus((PanelApplet *) applet, event->time);

    /* We just wanted to insert the above function call before the
     * default action...
    */
    return FALSE;
}


static const BonoboUIVerb applet_menu_verbs[] =
{
    /*  Not using BONOBO_UI_VERB and BONOBO_UI_VERB_END because they
        generate compiler warnings because they do not provide an
        initializer for member BonoboUIVerb::dummy.
        See libbonoboui-2.0/bonobo/bonobo-ui-component.h (version 2.24.1).
    */
    { "VerbisteAbout",          onAbout,          NULL, NULL },
    { "VerbisteEmptyTextField", onEmptyTextField, NULL, NULL },
    { NULL, NULL, NULL, NULL }
};


static
gboolean
appletFill(PanelApplet *applet, const gchar *iid, gpointer)
{
    if (strcmp(iid, "OAFIID:GNOME_" PACKAGE) != 0)
        return FALSE;

    /*
        Initialize the panel part of the applet interface.
    */
    GtkWidget *label = gtk_label_new(_("Verb:"));

    appletEntry = gtk_entry_new_with_max_length(255);
    gtk_entry_set_width_chars(GTK_ENTRY(appletEntry), 10);

    g_signal_connect(G_OBJECT(appletEntry), "key-press-event",
                        G_CALLBACK(onKeyPressInEntry), NULL);

    g_signal_connect(G_OBJECT (appletEntry), "button_press_event",
                        G_CALLBACK(entry_button_press_event), applet);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), appletEntry, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(applet), hbox);

    string applet_menu_xml = string() +
        "<popup name=\"button3\">\n"
        "    <menuitem\n"
        "        name=\"Verbiste About Item\"\n"
        "        verb=\"VerbisteAbout\"\n"
        "        _label=\"" + _("_About...") + "\"\n"
        "        pixtype=\"stock\"\n"
        "        pixname=\"gnome-stock-about\"\n"
        "        />\n"
        "    <menuitem\n"
        "        name=\"Verbiste Empty Text Field Item\"\n"
        "        verb=\"VerbisteEmptyTextField\"\n"
        "        _label=\"" + _("_Empty Text Field") + "\"\n"
        "        pixtype=\"stock\"\n"
        "        pixname=\"gnome-stock-erase\"\n"
        "        />\n"
        "</popup>\n"
        ;


    panel_applet_setup_menu(applet,
                        applet_menu_xml.c_str(), applet_menu_verbs, NULL);

    gtk_widget_show_all(GTK_WIDGET(applet));

    string errorMsg = initDictPointers();
    if (!errorMsg.empty())
    {
        gtk_widget_set_sensitive(appletEntry, false);
        showErrorDialog(PACKAGE_FULL_NAME + string(": ") + errorMsg);
    }

    return TRUE;
}


PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_" PACKAGE "_Factory",
                             PANEL_TYPE_APPLET,
                             PACKAGE,
                             VERSION,
                             appletFill,
                             NULL);
