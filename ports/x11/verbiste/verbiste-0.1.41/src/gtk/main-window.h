/*  $Id: main-window.h,v 1.4 2012/11/18 19:56:05 sarrazip Exp $
    main-window.h - Input and conjugation window

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

#ifndef _H_main_window
#define _H_main_window

#include <gtk/gtkentry.h>
#include <gdk/gdkkeysyms.h>

#include <string>
#include <stdexcept>


extern gboolean hideOnDelete;


void showAbout();


/**
    Shows a GNOME error dialog and waits for the user to close it.
    @param        msg                UTF-8 error message to display
*/
void showErrorDialog(const std::string &msg);


/**
    Show a window with the full conjugation of the given verb.
    @param      utf8UserText    UTF-8 string of the conjugated verb
                                (upper-case is allowed)
*/
void processText(const std::string &utf8UserText);


#endif  /* _H_main_window */
