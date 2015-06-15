#include "util.h"

#include <gtk/gtkwindow.h>

using namespace std;


string
initDictPointers()
{
    extern verbiste::FrenchVerbDictionary *frenchDict, *italianDict;

    try
    {
        using namespace verbiste;

        // Initialize global dictionary pointers.
        string conjFN, verbsFN;
        FrenchVerbDictionary::getXMLFilenames(conjFN, verbsFN, FrenchVerbDictionary::FRENCH);
        //cout << "fr: '" << conjFN << "', '" << verbsFN << "'\n";
        frenchDict = new FrenchVerbDictionary(conjFN, verbsFN, true, FrenchVerbDictionary::FRENCH);  // may throw

        FrenchVerbDictionary::getXMLFilenames(conjFN, verbsFN, FrenchVerbDictionary::ITALIAN);
        //cout << "it: '" << conjFN << "', '" << verbsFN << "'\n";
        italianDict = new FrenchVerbDictionary(conjFN, verbsFN, true, FrenchVerbDictionary::ITALIAN);  // may throw
        return string();  // success
    }
    catch(logic_error &e)
    {
        delete italianDict;
        italianDict = NULL;
        delete frenchDict;
        frenchDict = NULL;
        return e.what();
    }
}


void
set_window_icon_to_default(GtkWidget *window)
{
    GdkPixbuf *icon = gdk_pixbuf_new_from_file(PIXMAPDIR "/" PACKAGE ".svg", NULL);
    if (icon == NULL)
        g_warning("gdk_pixbuf_new_from_file() failed on " PIXMAPDIR "/" PACKAGE ".svg");
    else
        gtk_window_set_icon(GTK_WINDOW(window), icon);
}
