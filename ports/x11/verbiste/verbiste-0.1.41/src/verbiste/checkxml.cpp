/*  $Id: checkxml.cpp,v 1.4 2012/11/18 19:56:06 sarrazip Exp $
    checkxml.cpp - Checks on the integrity of the XML files

    verbiste - French conjugation system
    Copyright (C) 2003-2006 Pierre Sarrazin <http://sarrazip.com/>

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

#ifndef VERBSFRXML
#error VERBSFRXML expected to be a macro designating the verbs-fr.xml file
#endif

#include <iostream>
#include <set>
#include <stdlib.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

using namespace std;


static const string testName = "checkxml";

int main()
{
    xmlDocPtr doc = xmlParseFile(VERBSFRXML);
    if (doc == NULL)
    {
        cout << testName << ": could not open " << VERBSFRXML << endl;
        return EXIT_FAILURE;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == NULL)
    {
        cout << testName << ": could not get XML root element" << endl;
        return EXIT_FAILURE;
    }
    if (xmlStrcmp(root->name, reinterpret_cast<const xmlChar *>("verbs-fr")) != 0)
    {
        cout << testName << ": root element is not <verbs-fr>" << endl;
        return EXIT_FAILURE;
    }

    size_t numErrors = 0;
    size_t vCounter = 0;
    set<string> infinitiveSet;
    for (xmlNodePtr v = root->xmlChildrenNode; v != NULL; v = v->next)
    {
        //cout << "v->name=\"" << v->name << "\"\n";
        if (xmlStrcmp(v->name, reinterpret_cast<const xmlChar *>("text")) == 0
                || xmlStrcmp(v->name, reinterpret_cast<const xmlChar *>("comment")) == 0)
            continue;

        ++vCounter;
        xmlNodePtr i = v->xmlChildrenNode;
        if (i == NULL || i->xmlChildrenNode == NULL)
        {
            cout << testName << ": missing <i> node at <v> #" << vCounter << endl;
            ++numErrors;
            continue;
        }

        string inf = reinterpret_cast<char *>(
                            xmlNodeListGetString(doc, i->xmlChildrenNode, 1));
        if (infinitiveSet.find(inf) != infinitiveSet.end())
        {
            cout << testName << ": infinitive \"" << inf
                    << "\" found more than once at <v> #"
                    << vCounter << endl;
            ++numErrors;
            continue;
        }

        infinitiveSet.insert(inf);
    }

    cout << numErrors << " error(s) found.\n";
    return numErrors == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
