/*  $Id: main-window.cpp,v 1.46 2014/04/06 23:30:40 sarrazip Exp $
    main-window.cpp - Input and conjugation window

    verbiste - French conjugation system
    Copyright (C) 2003-2014 Pierre Sarrazin <http://sarrazip.com/>

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
#include "util.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#include <libintl.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>  /* for setw() */
#include <algorithm>

#define _(x) gettext(x)

using namespace std;
using namespace verbiste;


/*****************************************************************************/

gboolean hideOnDelete = TRUE;


FrenchVerbDictionary *frenchDict = NULL;
FrenchVerbDictionary *italianDict = NULL;

static GtkWidget *resultWin = NULL;
static GtkWidget *verbEntry = NULL;
static GtkWidget *conjButton = NULL;
static GtkWidget *saveAsButton = NULL;
static GtkWidget *showPronounsCB = NULL;
static GtkWidget *useFrenchDictCB = NULL;
static GtkWidget *useItalianDictCB = NULL;
static GtkWidget *resultNotebook = NULL;
static GtkWidget *diceScale = NULL;

static const double DICE_SCALE_MIN = 0.80;
static const double DICE_SCALE_MAX = 1.00;

static const gint SP = 2;  // default spacing for the GTK+ boxes
static bool trace = getenv("TRACE") != NULL;


/*  External functions to be provided by the application.
    A GNOME app would have these functions call gnome_config_get_string(), etc.
*/
char *get_config_string(const char *path);
void set_config_string(const char *path, const char *value);
void sync_config();


/*****************************************************************************/


// Encapsulates a gchar pointer to memory that needs to be freed with g_free().
class Catena
{
public:
    Catena(gchar *p) : ptr(p) {}  // p allowed to be NULL
    ~Catena() { g_free(ptr); }
    gchar *get() const { return ptr; }  // returned pointer may be NULL
    gint atoi() const { return ptr == NULL ? INT_MIN : static_cast<gint>(::atoi(ptr)); }
    bool startsWithDigit() const { return ptr != NULL && ::isdigit(ptr[0]); }
private:
    gchar *ptr;

    // Forbidden operations:
    Catena(const Catena &);
    Catena &operator = (const Catena &);
};


/*****************************************************************************/


static GtkWidget *
newLabel(const string &markup, gboolean selectable, gfloat yalign = 0.0)
{
    GtkWidget *label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label), markup.c_str());
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, yalign);
    gtk_label_set_selectable(GTK_LABEL(label), selectable);
    return label;
}


static string
removeFirstOccurrenceOfChar(const string &original, char toRemove)
{
    string copy = original;
    size_t pos = copy.find(toRemove);
    if (pos != string::npos)  // if found
        copy.erase(pos, 1);
    return copy;
}


/*****************************************************************************/


class ResultPage
{
public:

    GtkWidget *notebookPage;
    GtkWidget *table;

    ResultPage();

    // No destructor because this object does not own the two GtkWidgets.

    void showTemplateVerb(const string &templateVerb);

    void enableLinkButton(const string &utf8Infinitive, bool isItalian);

};


ResultPage::ResultPage()
  : notebookPage(gtk_vbox_new(FALSE, SP)),
    table(gtk_table_new(4, 4, FALSE))
{
    GtkWidget *scrolledWin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(
                        GTK_SCROLLED_WINDOW(scrolledWin),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(
                        GTK_SCROLLED_WINDOW(scrolledWin), table);

    g_object_set_data(G_OBJECT(notebookPage), "ResultPage", this);
        // make the vbox widget point to the corresponding ResultPage object

    gtk_box_pack_start(GTK_BOX(notebookPage), scrolledWin, TRUE, TRUE, 0);
}


// Converts the given UTF-8 string to an ASCII-only string where any
// non-ASCII character is represented by a %HH sequence where HH is
// the 2-digit hexadecimal Unicode number of the non-ASCII character.
// Example: "bébé" becomes "b%e9b%e9".
//
static string
convertUTF8ToPercentHex(const char *utf8)
{
    stringstream result;
    result << hex;
    for (const char *p = utf8; *p != '\0'; p = g_utf8_next_char(p))
    {
        gunichar c = g_utf8_get_char(p);
        if (c <= 127)  // if ASCII
            result << char(c);
        else
            result << '%' << setw(2) << c;
    }
    return result.str();
}


// Shows the verb that serves as the template for the displayed conjugation.
//
void
ResultPage::showTemplateVerb(const string &templateVerb)
{
    GtkWidget *cell = newLabel(_("Model:") + (" " + templateVerb), TRUE);
    gtk_table_attach(GTK_TABLE(table), cell,
                            1, 2, 0, 1,
                            GTK_FILL, GTK_SHRINK,
                            8, 8);
}


// Adds a link to the table that points to a web page that defines the verb.
//
void
ResultPage::enableLinkButton(const string &utf8Infinitive, bool isItalian)
{
    string uri = "http://" + string(isItalian ? "it" : "fr")
                    + ".wiktionary.org/wiki/"
                    + convertUTF8ToPercentHex(utf8Infinitive.c_str());
    GtkWidget *linkButton = gtk_link_button_new_with_label(
                                uri.c_str(),
                                isItalian
                                    ?  _("Look up on Wikizionario")
                                    :  _("Look up on Wiktionnaire")
                                );
    gtk_table_attach(GTK_TABLE(table), linkButton,
                        2, 3, 0, 1,
                        GTK_SHRINK, GTK_SHRINK,
                        8, 8);
}


/*****************************************************************************/


// All code that wants to make the program quit must use this function
// instead of calling gtk_main_quit() directly.
//
static
void
quit()
{
    // Save the current size of the results window.
    gint resultWinWidth, resultWinHeight;
    gtk_window_get_size(GTK_WINDOW(resultWin), &resultWinWidth, &resultWinHeight);

    stringstream ss;
    ss << resultWinWidth;
    set_config_string("Preferences/ResultWindowWidth", ss.str().c_str());
    ss.str("");
    ss << resultWinHeight;
    set_config_string("Preferences/ResultWindowHeight", ss.str().c_str());

    // Save the state of the checkboxes, etc.
    bool active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(showPronounsCB));
    set_config_string("Preferences/ShowPronouns", active ? "1" : "0");
    active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(useFrenchDictCB));
    set_config_string("Preferences/UseFrenchDict", active ? "1" : "0");
    active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(useItalianDictCB));
    set_config_string("Preferences/UseItalianDict", active ? "1" : "0");

    assert(diceScale != NULL);
    gdouble minDiceCoefficient = gtk_range_get_value(GTK_RANGE(diceScale));
    if (minDiceCoefficient < DICE_SCALE_MIN)
        minDiceCoefficient = DICE_SCALE_MIN;
    else if (minDiceCoefficient > DICE_SCALE_MAX)
        minDiceCoefficient = DICE_SCALE_MAX;
    ss.str("");
    ss << int(floor(minDiceCoefficient * 1000));  // save as int to avoid locale problems
    set_config_string("Preferences/MinDiceCoefficient", ss.str().c_str());

    sync_config();  // commit changes to file

    gtk_main_quit();
}


static
void
hideOrQuit(GtkWidget *, gpointer)
{
    if (hideOnDelete)
        gtk_widget_hide(resultWin);
    else
        quit();
}


static
gboolean
onKeyPressInResultWin(GtkWidget *, GdkEventKey *event, gpointer)
{
    g_return_val_if_fail(event != NULL, TRUE);

    if (event->keyval == GDK_w && (event->state & GDK_CONTROL_MASK) != 0)
    {
        hideOrQuit(NULL, NULL);
        return TRUE;
    }

    return FALSE;
}


static
gboolean
onKeyPressInAbout(GtkWidget *about, GdkEventKey *event, gpointer)
{
    g_return_val_if_fail(event != NULL, TRUE);

    switch (event->keyval)
    {
        case GDK_Escape:
            gtk_dialog_response(GTK_DIALOG(about), GTK_RESPONSE_OK);
            return TRUE;

        default:
            return FALSE;
    }
}


void
showAbout()
{
    const gchar *authors[] =
    {
        "Pierre Sarrazin <http://sarrazip.com/>",
        NULL
    };

    string logoFilename = string(PIXMAPDIR) + "/" + PACKAGE ".svg";

    GdkPixbuf *logo = gdk_pixbuf_new_from_file_at_size(logoFilename.c_str(), 48, 48, NULL);

    string copyright =
        string("Copyright (C) " COPYRIGHT_YEARS " Pierre Sarrazin <http://sarrazip.com/>\n")
        + _("Distributed under the GNU General Public License");

    GtkWidget *about = gtk_about_dialog_new();
    gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(about), PACKAGE_FULL_NAME);
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), VERSION);
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about), copyright.c_str());
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about),
                                            _("A French conjugation system"));
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about), authors);
    if (logo != NULL)
        gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about), logo);

    {
        ifstream licenseFile((LIBDATADIR + string("/COPYING")).c_str());
        if (licenseFile.good())
        {
            stringstream ss;
            ss << licenseFile.rdbuf();
            gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about),
                                                    ss.str().c_str());
        }
    }

    if (logo != NULL)
        g_object_unref(logo);

    g_signal_connect(G_OBJECT(about), "key-press-event",
                        G_CALLBACK(onKeyPressInAbout), NULL);

    gtk_dialog_run(GTK_DIALOG(about));
    gtk_widget_hide(about);
    gtk_widget_destroy(about);
}


static
gboolean
onKeyPressInEntry(GtkWidget *entry, GdkEventKey *event, gpointer /*data*/)
{
    g_return_val_if_fail(event != NULL, TRUE);

    switch (event->keyval)
    {
        case GDK_Return:
        case GDK_KP_Enter:
            {
                const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
                processText(text);
            }
            return TRUE;

        default:
            return FALSE;
    }
}


static
void
onChangeInEntry(GtkEditable *, gpointer)
{
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(verbEntry));
    gtk_widget_set_sensitive(GTK_WIDGET(conjButton), text[0] != '\0');
}


static
void
onConjugateButton(GtkWidget *, gpointer)
{
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(verbEntry));
    processText(text);
}


// Escape for HTML, including converting newline into <br/>.
static
string
esc_html(const string &s)
{
    string out;
    string::size_type len = s.length();
    for (string::size_type i = 0; i < len; i++)
    {
        switch (s[i])
        {
            case '\n': out += "<br/>"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '&': out += "&amp;"; break;
            default:  out += s[i];
        }
    }
    return out;
}

// Escape for LaTeX, for now just converting newline into "\\"
// (is more needed here?)
static
string
esc_latex(const string &s)
{
    string out;
    string::size_type len = s.length();
    for (string::size_type i = 0; i < len; i++)
    {
        switch (s[i])
        {
            case '\n': out += "\\\\\n"; break;
            default:  out += s[i];
        }
    }
    return out;
}


void writeHTMLConjugation(ostream &html)
{
    html << "<?xml version='1.0'?>\n";
    html << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""
            << " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
    html << "<html>\n";
    html << "<head>\n";
    html << "<title>" << esc_html(gtk_entry_get_text(GTK_ENTRY(verbEntry)))
            << "</title>\n";
    html << "<meta http-equiv='Content-Type'"
            << " content='text/html; charset=UTF-8' />\n";
    html << "</head>\n";
    html << "<body>\n";


    assert(resultNotebook != NULL);
    GtkWidget *w;
    for (int i = 0; (w = gtk_notebook_get_nth_page(
                        GTK_NOTEBOOK(resultNotebook), i)) != NULL;
                    i++)
    {
        ResultPage *rp = (ResultPage *) g_object_get_data(
                                                G_OBJECT(w), "ResultPage");
        g_return_if_fail(rp != NULL);
        g_return_if_fail(GTK_IS_TABLE(rp->table));

        GtkWidget *label = gtk_notebook_get_tab_label(
                                            GTK_NOTEBOOK(resultNotebook),
                                            rp->notebookPage);
        g_return_if_fail(GTK_IS_LABEL(label));

        const gchar *infinitive = gtk_label_get_text(GTK_LABEL(label));
        html << "<h1>" << esc_html(infinitive) << "</h1>\n";
        html << "<table border='1' cellspacing='0' cellpadding='4'>\n";
        string tr = "<tr valign='top'>\n";
        html << tr;

        GList *children = gtk_container_get_children(GTK_CONTAINER(rp->table));
        children = g_list_reverse(children);
        int childCounter = 0;
        for (GList *p = children; p != NULL; p = p->next, childCounter++)
        {
            GtkWidget *vbox = (GtkWidget *) p->data;
            g_return_if_fail(vbox != NULL);
            g_return_if_fail(GTK_IS_VBOX(vbox));

            GList *vboxChildren = gtk_container_get_children(
                                                GTK_CONTAINER(vbox));
            g_return_if_fail(vboxChildren != NULL);
            g_return_if_fail(vboxChildren->next != NULL);
            g_return_if_fail(vboxChildren->next->next == NULL);

            GtkWidget *nameLabel = (GtkWidget *) vboxChildren->data;
            GtkWidget *personsLabel = (GtkWidget *) vboxChildren->next->data;

            g_return_if_fail(nameLabel != NULL);
            g_return_if_fail(GTK_IS_LABEL(nameLabel));
            g_return_if_fail(personsLabel != NULL);
            g_return_if_fail(GTK_IS_LABEL(personsLabel));

            const gchar *name = gtk_label_get_text(GTK_LABEL(nameLabel));
            const gchar *persons = gtk_label_get_text(GTK_LABEL(personsLabel));

            g_return_if_fail(name != NULL);
            g_return_if_fail(persons != NULL);

            html << "<td>\n";
            html << "<strong>" << esc_html(name) << "</strong>\n<br/>\n";
            html << esc_html(persons) << "\n";
            html << "</td>\n";

            if (childCounter == 0)
            {
                html << "<td>&nbsp;</td>\n";
                html << "<td>&nbsp;</td>\n";
                html << "<td>&nbsp;</td>\n";
                html << "</tr>\n";
                html << tr;
            }
            else if (childCounter == 4)
            {
                html << "</tr>\n";
                html << tr;
            }
            else if (childCounter == 7 || childCounter == 10)
            {
                html << "<td>&nbsp;</td>\n";
                html << "</tr>\n";
                if (childCounter == 7)
                    html << tr;
            }

            g_list_free(vboxChildren);
        }
        g_list_free(children);

        html << "</table>\n";
    }

    html << "</body>\n";
    html << "</html>\n";
}


static
void
writeLaTeXConjugation(ostream &tex)
{
    tex << "% verbiste output - do not edit\n";
    tex << "% " << esc_latex(gtk_entry_get_text(GTK_ENTRY(verbEntry))) << "\n";
    tex << "\\documentclass{verbiste}\n";
    tex << "\\title{" << esc_latex(gtk_entry_get_text(GTK_ENTRY(verbEntry))) << "}\n";
    tex << "\\begin{document}\n";
    tex << "\\thispagestyle{empty}\n";

    assert(resultNotebook != NULL);
    GtkWidget *w;
    for (int i = 0; (w = gtk_notebook_get_nth_page(
                        GTK_NOTEBOOK(resultNotebook), i)) != NULL;
                    i++)
    {
        ResultPage *rp = (ResultPage *) g_object_get_data(
                                                G_OBJECT(w), "ResultPage");
        g_return_if_fail(rp != NULL);
        g_return_if_fail(GTK_IS_TABLE(rp->table));

        GtkWidget *label = gtk_notebook_get_tab_label(
                                            GTK_NOTEBOOK(resultNotebook),
                                            rp->notebookPage);
        g_return_if_fail(GTK_IS_LABEL(label));

        string btab = "\\begin{tense}\n";
        string etab = "\\end{tense}\n";
        string eol  = "\\\\\n";
        string eor  = "\\\\ \\hline\n";
        string sep  = "&\n";

        const gchar *infinitive = gtk_label_get_text(GTK_LABEL(label));
        tex << "\\section*{" << esc_latex(infinitive) << "}\n";
        tex << "\\noindent\n";
        tex << "\\begin{tensetable}\n";

        GList *children = gtk_container_get_children(GTK_CONTAINER(rp->table));
        children = g_list_reverse(children);
        int childCounter = 0;
        for (GList *p = children; p != NULL; p = p->next, childCounter++)
        {
            GtkWidget *vbox = (GtkWidget *) p->data;
            g_return_if_fail(vbox != NULL);
            g_return_if_fail(GTK_IS_VBOX(vbox));

            GList *vboxChildren = gtk_container_get_children(
                                                GTK_CONTAINER(vbox));
            g_return_if_fail(vboxChildren != NULL);
            g_return_if_fail(vboxChildren->next != NULL);
            g_return_if_fail(vboxChildren->next->next == NULL);

            GtkWidget *nameLabel = (GtkWidget *) vboxChildren->data;
            GtkWidget *personsLabel = (GtkWidget *) vboxChildren->next->data;

            g_return_if_fail(nameLabel != NULL);
            g_return_if_fail(GTK_IS_LABEL(nameLabel));
            g_return_if_fail(personsLabel != NULL);
            g_return_if_fail(GTK_IS_LABEL(personsLabel));

            const gchar *name = gtk_label_get_text(GTK_LABEL(nameLabel));
            const gchar *persons = gtk_label_get_text(GTK_LABEL(personsLabel));

            g_return_if_fail(name != NULL);
            g_return_if_fail(persons != NULL);

            tex << btab;
            tex << "\\tensename{" << esc_latex(name) << "}" << eol;
            tex << esc_latex(persons) << "\n";
            tex << etab;

            switch (childCounter)
              {
              case 0:
                tex << sep << sep << sep << eor;
                break;
              case 4:
                tex << eor;
                break;
              case 7:
              case 10:
                tex << sep << eor;
                break;
              default:
                tex << sep;
              }

            g_list_free(vboxChildren);
        }
        g_list_free(children);

        tex << "\\end{tensetable}\n";
    }

    tex << "\\end{document}\n";
}


enum SaveFileFormat { HTML, LATEX, NUMFORMATS };


const char *getSaveFileFormatExtension(SaveFileFormat fmt)
{
    switch (fmt)
    {
        case HTML: return ".html";
        case LATEX: return ".tex";
        case NUMFORMATS: return NULL;
    }
    return NULL;
}


static
void
onSaveFormatComboBoxChanged(GtkComboBox *comboBox, gpointer chooserWidget)
{
    // Get the file extension that corresponds to the currently selected format.
    gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(comboBox));
    const char *newExt = getSaveFileFormatExtension(SaveFileFormat(index));
    if (newExt == NULL)  // if invalid choice, bail out
        return;

    // Get the currently selected filename (full path).
    GtkWidget *chooser = reinterpret_cast<GtkWidget *>(chooserWidget);
    Catena f(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser)));

    // Find the last slash and period in that filename.
    // If the last period comes before the last slash, then the basename
    // has no extension.
    string newFilename = f.get();
    string::size_type posLastPeriod = newFilename.rfind('.');
    if (posLastPeriod == string::npos)
        return;  // no extension found
    string::size_type posLastSlash  = newFilename.rfind('/');
    if (posLastSlash != string::npos && posLastPeriod < posLastSlash)
        return;  // extension is not in basename

    // Replace the current extension with the selected one.
    newFilename.replace(posLastPeriod, newFilename.length() - posLastPeriod, newExt);
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(chooser), newFilename.c_str());
}


static
void
onSaveAsButton(GtkWidget *, gpointer)
{
    // Show a Save dialog that points to the last save dir, if available.
    GtkWidget *chooser = gtk_file_chooser_dialog_new(
                                _("Save Conjugation to File"),
                                GTK_WINDOW(resultWin),
                                GTK_FILE_CHOOSER_ACTION_SAVE,
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(
                                GTK_FILE_CHOOSER(chooser),
                                true);


    // Add a pull-down menu that offers to choose the file format.

    GtkWidget *comboBox = gtk_combo_box_new_text();
    // These appends must be in same order as enum SaveFileFormat:
    gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), _("HTML"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(comboBox), _("LaTeX"));

    // Get the default format from the saved configuration.
    Catena saveFormat(get_config_string("Files/SaveFormat"));
    gint formatIndex = saveFormat.atoi();
    SaveFileFormat defaultFormat = SaveFileFormat(formatIndex < 0 || formatIndex >= NUMFORMATS ? 0 : formatIndex);

    gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), gint(defaultFormat));

    // Connect the "changed" signal of the combo box.
    g_signal_connect(G_OBJECT(comboBox), "changed",
                G_CALLBACK(onSaveFormatComboBoxChanged), chooser);

    GtkWidget *prompt = gtk_label_new_with_mnemonic(_("_Format of the saved file:"));
    gtk_label_set_mnemonic_widget(GTK_LABEL(prompt), comboBox);

    // Add the combo box and its prompt to the file chooser dialog.
    GtkWidget *formatBox = gtk_hbox_new(FALSE, SP);
    gtk_box_pack_start(GTK_BOX(formatBox), prompt, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(formatBox), comboBox, TRUE, TRUE, 0);

    gtk_widget_show_all(GTK_WIDGET(formatBox));

    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(chooser), formatBox);


    // Set the default directory from the saved configuration.
    Catena saveDir(get_config_string("Files/SaveDirectory"));
    if (saveDir.get() != NULL)
        gtk_file_chooser_set_current_folder(
                                    GTK_FILE_CHOOSER(chooser),
                                    saveDir.get());


    // Base default filename on entered verb:
    string defaultFilename = gtk_entry_get_text(GTK_ENTRY(verbEntry))
                                + string(getSaveFileFormatExtension(defaultFormat));
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(chooser), defaultFilename.c_str());


    bool cancel = (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_CANCEL);
    string filename;
    SaveFileFormat chosenFormat = NUMFORMATS;  // invalid value for now
    if (!cancel)
    {
        Catena f(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser)));
        filename = f.get();

        gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(comboBox));
        chosenFormat = SaveFileFormat(index < 0 || index >= NUMFORMATS ? 0 : index);

        // Remember the chosen directory and format:
        Catena dir(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(chooser)));
        set_config_string("Files/SaveDirectory", dir.get());
        stringstream ss;
        ss << int(chosenFormat);
        set_config_string("Files/SaveFormat", ss.str().c_str());

        // We do not call sync_config() because it will be done by quit()
        // when the program quits.
    }
    gtk_widget_destroy(chooser);
    if (cancel)
        return;

    // Try to create the file:

    ofstream file(filename.c_str(), ios::out);
    if (!file)
    {
        showErrorDialog(_("Could not create file:") + ("\n" + filename));
        return;
    }

    switch (chosenFormat)
    {
        case HTML:
            writeHTMLConjugation(file);
            break;
        case LATEX:
            writeLaTeXConjugation(file);
            break;
        case NUMFORMATS:
            assert(!"invalid chosen format");
            break;
    }

    file.close();
    if (!file)
    {
        showErrorDialog(_("Could not close file:") + ("\n" + filename));
        return;
    }
}


static
ResultPage *
appendResultPage(const string &utf8LabelText)
{
    ResultPage *rp = new ResultPage();
    GtkWidget *label = newLabel("<b>" + utf8LabelText + "</b>" , FALSE);

    assert(resultNotebook != NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(resultNotebook),
                                GTK_WIDGET(rp->notebookPage),
                                GTK_WIDGET(label));
    return rp;
}


static
void
onAboutButton(GtkWidget *, gpointer)
{
    showAbout();
}


static
void
onShowPronounsToggled(GtkToggleButton *, gpointer)
{
    onConjugateButton(NULL, NULL);
}


static
void
onDiceScaleValueChanged(GtkRange *, gpointer)
{
    onConjugateButton(NULL, NULL);
}


static
void
onUseFrenchDictToggled(GtkToggleButton *, gpointer)
{
    onConjugateButton(NULL, NULL);
}


static
void
onUseItalianDictToggled(GtkToggleButton *, gpointer)
{
    onConjugateButton(NULL, NULL);
}


static
bool
getResultWindowSize(gint &width, gint &height)
{
    Catena w(get_config_string("Preferences/ResultWindowWidth"));
    Catena h(get_config_string("Preferences/ResultWindowHeight"));
    if (!w.startsWithDigit() || !h.startsWithDigit())
        return false;

    enum { MIN = 0, MAX = 65535 };
    width = w.atoi();
    if (width < MIN || width > MAX)
        return false;
    height = h.atoi();
    if (height < MIN || height > MAX)
        return false;

    return true;
}


static
void
onAccentedCharClicked(GtkWidget *, gpointer a)
{
    // Insert UTF-8 string designated by 'a' in Verb text field,
    // at current cursor position.
    //
    const char *utf8AccentedChar = reinterpret_cast<char *>(a);
    gint currentPos = gtk_editable_get_position(GTK_EDITABLE(verbEntry));
    gtk_editable_insert_text(GTK_EDITABLE(verbEntry), utf8AccentedChar, -1, &currentPos);

    // Put focus on text field, so user can immediately
    // go back to using keyboard.
    // This must be done before setting cursor position,
    // because setting focus selects entire text field contents.
    // Setting position will unselect.
    //
    gtk_widget_grab_focus(GTK_WIDGET(verbEntry));

    // currentPos is now after insertion. Put cursor there.
    //
    gtk_editable_set_position(GTK_EDITABLE(verbEntry), currentPos);
}


static
void
addAccentedCharButton(GtkWidget *box, const char *utf8AccentedChar)
{
    GtkWidget *button = gtk_button_new_with_mnemonic(utf8AccentedChar);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(button), "clicked",
                G_CALLBACK(onAccentedCharClicked),
                reinterpret_cast<gpointer>(const_cast<char *>(utf8AccentedChar)));
}


static
void
showResultWin()
{
    if (resultWin == NULL)
    {
        resultWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(resultWin), PACKAGE_FULL_NAME);

        // Get the size of the results window from the saved configuration, if available.
        gint resultWinWidth = 0, resultWinHeight = 0;
        if (!getResultWindowSize(resultWinWidth, resultWinHeight))  // if failed
            resultWinWidth = 610, resultWinHeight = 530;  // use default values
        gtk_window_set_default_size(GTK_WINDOW(resultWin), resultWinWidth, resultWinHeight);

        gtk_container_set_border_width(GTK_CONTAINER(resultWin), 4);

        if (hideOnDelete)
        {
            /*
                When user clicks on title bar's close button, the window must
                only be hidden, not be destroyed.
            */
            g_signal_connect(G_OBJECT(resultWin), "delete_event",
                        G_CALLBACK(gtk_widget_hide_on_delete), NULL);
        }
        else
            g_signal_connect(G_OBJECT(resultWin), "delete_event",
                        G_CALLBACK(quit), NULL);

        /*
            Capture the key presses in order to hide or close the window
            on Ctrl-W.
        */
        g_signal_connect(G_OBJECT(resultWin), "key-press-event",
                        G_CALLBACK(onKeyPressInResultWin), NULL);


        /*
            Create a text field where the user can enter requests:
        */
        verbEntry = gtk_entry_new_with_max_length(255);
        g_signal_connect(G_OBJECT(verbEntry), "key-press-event",
                        G_CALLBACK(onKeyPressInEntry), NULL);
        g_signal_connect(G_OBJECT(verbEntry), "changed",
                        G_CALLBACK(onChangeInEntry), NULL);
        GtkWidget *prompt = gtk_label_new_with_mnemonic(_("_Verb:"));
        gtk_label_set_mnemonic_widget(GTK_LABEL(prompt), verbEntry);

        GtkWidget *promptBox = gtk_hbox_new(FALSE, SP);
        gtk_box_pack_start(GTK_BOX(promptBox), prompt, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(promptBox), verbEntry, TRUE, TRUE, 0);

        conjButton = gtk_button_new_with_mnemonic(_("Conjuga_te"));
        gtk_widget_set_sensitive(GTK_WIDGET(conjButton), false);
        gtk_box_pack_start(GTK_BOX(promptBox), conjButton, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(conjButton), "clicked",
                                    G_CALLBACK(onConjugateButton), NULL);

        saveAsButton = gtk_button_new_from_stock(GTK_STOCK_SAVE_AS);
        gtk_widget_set_sensitive(GTK_WIDGET(saveAsButton), false);
        gtk_box_pack_start(GTK_BOX(promptBox), saveAsButton, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(saveAsButton), "clicked",
                                    G_CALLBACK(onSaveAsButton), NULL);


        /*
            Create an options box.
        */
        GtkWidget *optionsBox = gtk_hbox_new(FALSE, SP);
        showPronounsCB = gtk_check_button_new_with_mnemonic(
                                                        _("Show _Pronouns"));
        gtk_box_pack_start(GTK_BOX(optionsBox), showPronounsCB,
                                                        FALSE, FALSE, 0);
        Catena showPronouns(get_config_string("Preferences/ShowPronouns"));

        // Initialize the check box *before* registering its callback, to avoid
        // having this callback invoked prematurely.
        //
        if (showPronouns.get() != NULL)
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(showPronounsCB),
                                        strcmp(showPronouns.get(), "1") == 0);
        g_signal_connect(G_OBJECT(showPronounsCB), "toggled",
                                    G_CALLBACK(onShowPronounsToggled), NULL);

        gtk_box_pack_start(GTK_BOX(optionsBox), gtk_vseparator_new(), FALSE, FALSE, 0);

        // Provide buttons that add accented characters to the Verb text field.
        // The \x codes are in UTF-8.
        //
        addAccentedCharButton(optionsBox, "\xc3\xa0"); // in HTML: &agrave;
        addAccentedCharButton(optionsBox, "\xc3\xa2"); // in HTML: &acirc;
        addAccentedCharButton(optionsBox, "\xc3\xa7"); // in HTML: &ccedil;
        addAccentedCharButton(optionsBox, "\xc3\xa9"); // in HTML: &eacute;
        addAccentedCharButton(optionsBox, "\xc3\xa8"); // in HTML: &egrave;
        addAccentedCharButton(optionsBox, "\xc3\xaa"); // in HTML: &ecirc;
        addAccentedCharButton(optionsBox, "\xc3\xab"); // in HTML: &euml;
        addAccentedCharButton(optionsBox, "\xc3\xae"); // in HTML: &icirc;
        addAccentedCharButton(optionsBox, "\xc3\xaf"); // in HTML: &iuml;
        addAccentedCharButton(optionsBox, "\xc3\xac"); // in HTML: &igrave;
        addAccentedCharButton(optionsBox, "\xc3\xb4"); // in HTML: &ocirc;
        addAccentedCharButton(optionsBox, "\xc3\xb2"); // in HTML: &ograve;
        addAccentedCharButton(optionsBox, "\xc3\xb9"); // in HTML: &ugrave;
        addAccentedCharButton(optionsBox, "\xc3\xbb"); // in HTML: &ucirc;

        gtk_box_pack_start(GTK_BOX(optionsBox), gtk_vseparator_new(), FALSE, FALSE, 0);

        useFrenchDictCB = gtk_check_button_new_with_mnemonic(_("Search Fr_ench"));
                                        // 'F' is already shortcut for "Fermer" button in French interface
        gtk_box_pack_start(GTK_BOX(optionsBox), useFrenchDictCB, FALSE, FALSE, 0);
        Catena useFrenchDict(get_config_string("Preferences/UseFrenchDict"));
        const char *value = (useFrenchDict.get() != NULL ? useFrenchDict.get() : "1");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(useFrenchDictCB), strcmp(value, "1") == 0);
        g_signal_connect(G_OBJECT(useFrenchDictCB), "toggled", G_CALLBACK(onUseFrenchDictToggled), NULL);

        useItalianDictCB = gtk_check_button_new_with_mnemonic(_("Search _Italian"));
        gtk_box_pack_start(GTK_BOX(optionsBox), useItalianDictCB, FALSE, FALSE, 0);
        Catena useItalianDict(get_config_string("Preferences/UseItalianDict"));
        value = (useItalianDict.get() != NULL ? useItalianDict.get() : "0");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(useItalianDictCB), strcmp(value, "1") == 0);
        g_signal_connect(G_OBJECT(useItalianDictCB), "toggled", G_CALLBACK(onUseItalianDictToggled), NULL);


        /*
            Create a notebook that receives the conjugations.
        */
        resultNotebook = gtk_notebook_new();
        assert(resultNotebook != NULL);
        gtk_notebook_set_scrollable(GTK_NOTEBOOK(resultNotebook), TRUE);


        /*
            Create a button box at the bottom.
        */
        GtkWidget *bottomBox = gtk_hbox_new(FALSE, SP);

        GtkWidget *diceBeforeLabel = gtk_label_new_with_mnemonic(_("_Spelling: tolerant"));

        diceScale = gtk_hscale_new_with_range(DICE_SCALE_MIN, DICE_SCALE_MAX, 0.01);
        gtk_range_set_update_policy(GTK_RANGE(diceScale), GTK_UPDATE_DISCONTINUOUS);
        gtk_scale_set_draw_value(GTK_SCALE(diceScale), FALSE);  // don't show number
        gtk_scale_set_digits(GTK_SCALE(diceScale), 2);

        Catena coef(get_config_string("Preferences/MinDiceCoefficient"));
        if (trace)
            cout << "coef: " << (coef.get() ? coef.get() : "<null>") << " (" << (void *) coef.get() << ")" << endl;
        errno = 0;
        long d = (coef.get() != NULL ? strtol(coef.get(), NULL, 10) : 1000);
        if (trace)
        {
            int e = errno;
            cout << "d=" << d << ", errno=" << e << endl;
            errno = e;
        }
        double minDiceCoefficient = d / 1000.0;  // saved as int to avoid locale problems
        if (errno == ERANGE || minDiceCoefficient < DICE_SCALE_MIN || minDiceCoefficient > DICE_SCALE_MAX)
            minDiceCoefficient = 1.0;
        gtk_range_set_value(GTK_RANGE(diceScale), minDiceCoefficient);

        g_signal_connect(G_OBJECT(diceScale), "value-changed",
                                    G_CALLBACK(onDiceScaleValueChanged), NULL);

        GtkWidget *diceAfterLabel = gtk_label_new_with_mnemonic(_("strict"));

        gtk_label_set_mnemonic_widget(GTK_LABEL(diceBeforeLabel), diceScale);

        gtk_box_pack_start(GTK_BOX(bottomBox), diceBeforeLabel, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(bottomBox), diceScale, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(bottomBox), diceAfterLabel, FALSE, FALSE, 0);

        gtk_box_pack_start(GTK_BOX(bottomBox), gtk_vseparator_new(), FALSE, FALSE, 0);

        GtkWidget *closeButton = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
        gtk_box_pack_end(GTK_BOX(bottomBox), closeButton, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(closeButton), "clicked",
                                    G_CALLBACK(hideOrQuit), NULL);

        GtkWidget *aboutButton = gtk_button_new_from_stock(GTK_STOCK_ABOUT);
        g_signal_connect(G_OBJECT(aboutButton), "clicked",
                                    G_CALLBACK(onAboutButton), NULL);
        gtk_box_pack_end(GTK_BOX(bottomBox), aboutButton, FALSE, FALSE, 0);



        /*
            Finish the window setup:
        */
        GtkWidget *vbox = gtk_vbox_new(FALSE, SP);
        gtk_box_pack_start(GTK_BOX(vbox), promptBox, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), optionsBox, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), resultNotebook, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), bottomBox, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(resultWin), vbox);
        set_window_icon_to_default(resultWin);
        gtk_widget_show_all(GTK_WIDGET(resultWin));
    }

    gtk_widget_grab_focus(GTK_WIDGET(verbEntry));

    gtk_window_present(GTK_WINDOW(resultWin));
}


static
void
clearResultNotebook()
{
    if (resultNotebook != NULL)
    {
        GtkWidget *w;
        while ((w = gtk_notebook_get_nth_page(
                            GTK_NOTEBOOK(resultNotebook), 0)) != NULL)
        {
            ResultPage *rp = (ResultPage *) g_object_get_data(
                                                    G_OBJECT(w), "ResultPage");
            if (rp == NULL)
                g_warning("clearResultNotebook: null ResultPage pointer");
            gtk_notebook_remove_page(GTK_NOTEBOOK(resultNotebook), 0);
            delete rp;
        }
    }

    gtk_widget_set_sensitive(GTK_WIDGET(saveAsButton), false);
}


static
string
tolowerUTF8(const string &s)
{
    Catena down(g_utf8_strdown(s.data(), s.length()));
    return down.get();
}


static
GtkWidget *
createTableCell(const VVS &utf8Tense,
                const string &utf8TenseName,
                const string &utf8UserText,
                FrenchVerbDictionary *fvd)
{
    GtkWidget *vbox = gtk_vbox_new(FALSE, SP);
    GtkWidget *nameLabel = newLabel("<b><u>" + utf8TenseName + "</u></b>", TRUE);

    string utf8Persons = createTableCellText(
                                *fvd,
                                utf8Tense,
                                tolowerUTF8(utf8UserText),
                                "<span foreground=\"red\">",
                                "</span>");

    GtkWidget *personsLabel = newLabel(utf8Persons, TRUE);

    gtk_box_pack_start(GTK_BOX(vbox), nameLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), personsLabel, FALSE, FALSE, 0);

    return vbox;
}


class WaitCursor
{
public:
    WaitCursor()
    {
        if (resultWin == NULL || resultWin->window == NULL)
            return;

        GdkCursor *cursor = gdk_cursor_new(GDK_WATCH);
        gdk_window_set_cursor(resultWin->window, cursor);
        gdk_cursor_unref(cursor);
    }

    ~WaitCursor()
    {
        if (resultWin == NULL || resultWin->window == NULL)
            return;

        gdk_window_set_cursor(resultWin->window, NULL);
    }
};


static size_t deconjugate(FrenchVerbDictionary &fvd, const string &utf8UserText,
                          const string &lowerCaseUTF8UserText, bool includePronouns);
static size_t deconjugateFuzzyMatches(FrenchVerbDictionary &fvd, const string &utf8UserText,
                                      const string &lowerCaseUTF8UserText, bool includePronouns);
static void finishProcessingText(const string &utf8UserText, size_t numPages);


// 'utf8UserText' must be a UTF-8 string to be deconjugated.
// It must not contain a newline character ('\n').
//
void
processText(const string &utf8UserText)
{
    showResultWin();  // initializes resultWin if not already done

    WaitCursor wc;  // must be used after resultWin initialized; destructor removes wait cursor

    gtk_entry_set_text(GTK_ENTRY(verbEntry), utf8UserText.c_str());
    gtk_editable_select_region(GTK_EDITABLE(verbEntry), 0, -1);

    clearResultNotebook();

    size_t numFrenchPages = 0, numItalianPages = 0;

    if (!utf8UserText.empty())
    {
        string lowerCaseUTF8UserText = tolowerUTF8(utf8UserText);

        assert(showPronounsCB != NULL);
        assert(useFrenchDictCB != NULL);
        assert(useItalianDictCB != NULL);

        bool includePronouns = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(showPronounsCB));
        bool useFrenchDict   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(useFrenchDictCB));
        bool useItalianDict  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(useItalianDictCB));
        if (useFrenchDict)
        {
            numFrenchPages += deconjugate(*frenchDict, utf8UserText, lowerCaseUTF8UserText, includePronouns);
            if (numFrenchPages == 0)
                numFrenchPages += deconjugateFuzzyMatches(*frenchDict, utf8UserText, lowerCaseUTF8UserText, includePronouns);
        }
        if (useItalianDict)
        {
            numItalianPages += deconjugate(*italianDict, utf8UserText, lowerCaseUTF8UserText, includePronouns);
            if (numItalianPages == 0)
                numItalianPages += deconjugateFuzzyMatches(*italianDict, utf8UserText, lowerCaseUTF8UserText, includePronouns);
        }
    }

    finishProcessingText(utf8UserText, numFrenchPages + numItalianPages);
}


// Asks the FrenchVerbDictionary to deconjugate some user text,
// then creates a notebook page for each resulting conjugation.
// Returns the number of pages created in the notebook.
//
size_t
deconjugate(FrenchVerbDictionary &fvd, const string &utf8UserText,
            const string &lowerCaseUTF8UserText, bool includePronouns)
{
    if (trace)
        cout << "deconjugate(fvd=@" << &fvd << " (" << fvd.getLanguage()
             << "), utf8UserText='" << utf8UserText
             << "', lowerCaseUTF8UserText='" << lowerCaseUTF8UserText
             << "', includePronouns=" << includePronouns
             << ")" << endl;

    bool isItalian = (fvd.getLanguage() == FrenchVerbDictionary::ITALIAN);

    /*
        For each possible deconjugation, take the infinitive form and
        obtain its complete conjugation.
    */
    vector<InflectionDesc> v;
    fvd.deconjugate(lowerCaseUTF8UserText, v);

    size_t numPages = 0;  // counts number of pages added to notebook
    string prevUTF8Infinitive, prevTemplateName;

    for (vector<InflectionDesc>::const_iterator it = v.begin();
                                            it != v.end(); it++)
    {
        const InflectionDesc &d = *it;

        VVVS conjug;
        getConjugation(fvd, d.infinitive, d.templateName, conjug, includePronouns);

        if (conjug.size() == 0           // if no tenses
            || conjug[0].size() == 0     // if no infinitive tense
            || conjug[0][0].size() == 0  // if no person in inf. tense
            || conjug[0][0][0].empty())  // if infinitive string empty
        {
            continue;
        }

        string utf8Infinitive = conjug[0][0][0];

        if (trace)
            cout << "deconjugate: d.templateName='" << d.templateName
                 << "', utf8Infinitive='" << utf8Infinitive
                 << "', prevUTF8Infinitive='" << prevUTF8Infinitive
                 << "'\n";
        if (utf8Infinitive == prevUTF8Infinitive && d.templateName == prevTemplateName)
            continue;

        string label = utf8Infinitive + " (" + FrenchVerbDictionary::getLanguageCode(fvd.getLanguage()) + ")";
        ResultPage *rp = appendResultPage(label);

        rp->showTemplateVerb(removeFirstOccurrenceOfChar(d.templateName, ':'));

        rp->enableLinkButton(utf8Infinitive, isItalian);

        numPages++;

        int i = 0;
        for (VVVS::const_iterator t = conjug.begin();
                                t != conjug.end(); t++, i++)
        {
            if (i == 1)
                i = 4;
            else if (i == 11)
                i = 12;
            assert(i >= 0 && i < 16);

            int row = i / 4;
            int col = i % 4;

            string utf8TenseName = getTenseNameForTableCell(row, col, isItalian);
            if (utf8TenseName.empty())
                continue;

            GtkWidget *cell = createTableCell(
                                *t, utf8TenseName, utf8UserText, &fvd);
            gtk_table_attach(GTK_TABLE(rp->table), cell,
                                col, col + 1, row, row + 1,
                                GTK_FILL, GTK_FILL,
                                8, 8);
        }

        gtk_widget_show_all(GTK_WIDGET(rp->notebookPage));
                /* must be done here to show the elements added in the for() */

        prevUTF8Infinitive = utf8Infinitive;
        prevTemplateName = d.templateName;
    }

    return numPages;
}


// See http://en.wikipedia.org/wiki/Dice_coefficient
//
static double
diceCoefficient(const wstring &a, const wstring &b)
{
    size_t lenA = a.length();
    size_t lenB = b.length();
    if (lenA == 0 && lenB == 0)  // special case: two empty strings
        return 1.0;

    wstring bCopy = b;
    size_t numCommonChars = 0;

    for (size_t i = 0; i < lenA; ++i)
    {
        wchar_t aChar = a[i];
        for (size_t j = 0; j < lenB; ++j)
        {
            if (aChar == bCopy[j])
            {
                ++numCommonChars;
                bCopy[j] = '\0';
                break;
            }
        }
    }

    return (2 * numCommonChars) / double(lenA + lenB);
}


// Only works for Latin-1 characters.
//
inline bool
isWideCharConsonant(wchar_t c)
{
    if (c > 0xFF)
        return false;
    static const char *vowelsInLatin1 = "aeiouy\xe1\xe2\xe0\xe9\xea\xe8\xeb\xed\xee\xef\xf3\xf4\xfa\xfb\xf9\xfc";
    return strchr(vowelsInLatin1, static_cast<unsigned char>(c)) == NULL;
}


struct Score
{
    double coefficient;  // 0..1 (1 = same)
    string verbName;

    Score(double c, const string &v) : coefficient(c), verbName(v) {}

    static bool decreasingCoefficientOrder(const Score &a, const Score &b)
    {
        if (a.coefficient != b.coefficient)
            return a.coefficient > b.coefficient;
        return a.verbName < b.verbName;  // use dictionary order
    }
};


static void
fuzzyMatch(FrenchVerbDictionary &fvd,
           const string &utf8String,
           double minDiceCoefficient,
           vector<string> &utf8Alternatives)
{
    if (trace)
        cout << "fuzzyMatch('" << utf8String << "', " << minDiceCoefficient << "):\n";

    if (utf8String.empty())
        return;

    wstring wideString = fvd.utf8ToWide(utf8String);
    wchar_t initial = wideString[0];
    bool isInitConsonant = isWideCharConsonant(initial);
    vector<Score> scores;

    for (VerbTable::const_iterator it = fvd.beginKnownVerbs();
                                   it != fvd.endKnownVerbs(); ++it)
    {
        const string &verbName = it->first;
        assert(!verbName.empty());  // empty verb names unexpected in dictionary

        const wstring wideVerbName = fvd.utf8ToWide(verbName);

        if (isInitConsonant && wideVerbName[0] != initial)
            continue;

        double coef = diceCoefficient(wideVerbName, wideString);

        if (coef >= minDiceCoefficient)  // if similar enough
        {
            if (trace)
                wcout << "  MATCH\t" << coef << "\t" << wideVerbName << "\n";
            scores.push_back(Score(coef, fvd.wideToUTF8(wideVerbName)));
        }
    }

    sort(scores.begin(), scores.end(), Score::decreasingCoefficientOrder);

    for (vector<Score>::const_iterator it = scores.begin();
                                       it != scores.end(); ++it)
    {
        if (trace)
            cout << "  RET\t" << it->coefficient << "\t" << it->verbName << "\n";
        utf8Alternatives.push_back(it->verbName);
    }
}


size_t
deconjugateFuzzyMatches(FrenchVerbDictionary &fvd, const string &utf8UserText,
                        const string &lowerCaseUTF8UserText, bool includePronouns)
{
    // Get minimum Dice coefficient from GUI's spelling tolerance slider.
    //
    assert(diceScale != NULL);
    gdouble minDiceCoefficient = gtk_range_get_value(GTK_RANGE(diceScale));
    assert(minDiceCoefficient >= 0.0 && minDiceCoefficient <= 1.0);
    if (minDiceCoefficient == 1.0)  // if not using fuzzy matching
        return 0;

    // Get a list of verbs that are similar to lowerCaseUTF8UserText.
    //
    vector<string> utf8Alternatives;
    fuzzyMatch(fvd, lowerCaseUTF8UserText, minDiceCoefficient, utf8Alternatives);

    // Show these verbs.
    //
    size_t numPages = 0;
    for (vector<string>::const_iterator it = utf8Alternatives.begin();
                                        it != utf8Alternatives.end(); ++it)
        numPages += deconjugate(fvd, utf8UserText, *it, includePronouns);

    return numPages;
}


void
finishProcessingText(const string &utf8UserText, size_t numPages)
{
    if (numPages == 0 && !utf8UserText.empty())
    {
        ResultPage *rp = appendResultPage("<i>" + string(_("error")) + "</i>");
        GtkWidget *cell = newLabel(_("Unknown verb."), FALSE);
        gtk_table_attach(GTK_TABLE(rp->table), cell,
                                0, 1, 0, 1,
                                GTK_FILL, GTK_FILL,
                                8, 8);
        gtk_widget_show_all(GTK_WIDGET(rp->notebookPage));
    }


    gtk_widget_set_sensitive(GTK_WIDGET(saveAsButton), numPages > 0);


    if (resultNotebook != NULL)
    {
        /*  PATCH: without this hack, a 2-page notebook has its first tab
            active but showing the contents of the second page.  This can
            be tested by entering "parais", which corresponds to the two
            infinitives "paraître" and "parer".
        */
        if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(resultNotebook)) > 1)
            gtk_notebook_set_current_page(GTK_NOTEBOOK(resultNotebook), 1);



        gtk_notebook_set_current_page(GTK_NOTEBOOK(resultNotebook), 0);
    }
}


void
showErrorDialog(const string &msg)
{
    GtkWidget *dlg = gtk_message_dialog_new(NULL,
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_CLOSE,
                                        "%s", msg.c_str());
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}
