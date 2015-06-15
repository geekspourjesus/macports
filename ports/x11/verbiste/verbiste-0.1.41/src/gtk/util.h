#ifndef _H_util
#define _H_util

#include <gtk/gtkwidget.h>
#include "gui/conjugation.h"
#include <string>


// Inits global variables frenchDict and italianDict.
// Returns empty string on success, or error message otherwise.
std::string initDictPointers();

void set_window_icon_to_default(GtkWidget *window);


#endif  /* _H_util */
