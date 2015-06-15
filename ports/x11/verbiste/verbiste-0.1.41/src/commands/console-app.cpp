/*  $Id: console-app.cpp,v 1.13 2013/12/08 20:11:54 sarrazip Exp $
    console-app.cpp - Console application main function

    verbiste - French conjugation system
    Copyright (C) 2003-2010 Pierre Sarrazin <http://sarrazip.com/>

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

#include "gui/conjugation.h"
#include "verbiste/FrenchVerbDictionary.h"

#include <libintl.h>
#include <locale.h>
#include <stdlib.h>
#include <iostream>

using namespace std;


class ConsoleApp
{
public:

    ConsoleApp(verbiste::FrenchVerbDictionary &_fvd)
    :   fvd(_fvd),
        includePronouns(false)
    {
    }

    void processCommand(const string &utf8Command)
    {
        string lowerCaseUTF8Command = fvd.wideToUTF8(
                                        fvd.tolowerWide(
                                          fvd.utf8ToWide(utf8Command)));

        if (lowerCaseUTF8Command == "/showpronouns" || lowerCaseUTF8Command == "/sp")
        {
            includePronouns = true;
            return;
        }
        if (lowerCaseUTF8Command == "/hidepronouns" || lowerCaseUTF8Command == "/hp")
        {
            includePronouns = false;
            return;
        }

        bool isItalian = (fvd.getLanguage() == verbiste::FrenchVerbDictionary::ITALIAN);

        /*
            For each possible deconjugation, take the infinitive form and
            obtain its complete conjugation.
        */
        vector<InflectionDesc> v;
        fvd.deconjugate(lowerCaseUTF8Command, v);

        string prevUTF8Infinitive;
        size_t numPages = 0;

        cout << "<result input='" << utf8Command << "'>\n";

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

            if (utf8Infinitive == prevUTF8Infinitive)
                continue;

            cout << "<conjugation verb='" << utf8Infinitive << "'>\n";
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

                string utf8Persons = createTableCellText(
                                                fvd,
                                                *t,
                                                lowerCaseUTF8Command,
                                                "*",
                                                "");

                cout << "<tense name='" << utf8TenseName << "'>\n";
                cout << utf8Persons << "\n";
                cout << "</tense>\n";
            }

            cout << "</conjugation>\n";

            prevUTF8Infinitive = utf8Infinitive;

        }   // for

        cout << "</result>\n";
    }

private:

    verbiste::FrenchVerbDictionary &fvd;
    bool includePronouns;

};


int
main(int /*argc*/, char * /*argv*/[])
{
    setlocale(LC_CTYPE, "");  // necessary on Solaris
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    using namespace verbiste;

    FrenchVerbDictionary *fvd;
    try
    {
        const char *langCode = getenv("VERBISTE_LANG");
        if (langCode == NULL || langCode[0] == '\0')
            langCode = "fr";
        string conjFN, verbsFN;

        FrenchVerbDictionary::Language lang = FrenchVerbDictionary::parseLanguageCode(langCode);
        if (lang == FrenchVerbDictionary::NO_LANGUAGE)
            lang = FrenchVerbDictionary::FRENCH;

        FrenchVerbDictionary::getXMLFilenames(conjFN, verbsFN, lang);
        fvd = new FrenchVerbDictionary(conjFN, verbsFN, true, lang);  // may throw
    }
    catch(logic_error &e)
    {
        cerr << PACKAGE_FULL_NAME << ": " << e.what() << endl;
        return EXIT_FAILURE;
    }

    ConsoleApp app(*fvd);

    string utf8Command;
    while (getline(cin, utf8Command))
        app.processCommand(utf8Command);

    return EXIT_SUCCESS;
}
