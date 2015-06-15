/*  $Id: french-deconjugator.cpp,v 1.25 2012/11/18 19:56:05 sarrazip Exp $
    french-deconjugator.cpp - Analyzer of conjugated French verbs

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


#include "Command.h"

#ifdef ENABLE_NLS
#ifdef HAVE_GETOPT_LONG
#include <unistd.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>
#endif
#endif  /* ENABLE_NLS */

#include <langinfo.h>

#include <iostream>
#include <vector>
#include <string.h>

using namespace std;
using namespace verbiste;


static const char *commandName = "french-deconjugator";


#if defined(ENABLE_NLS) && defined(HAVE_GETOPT_LONG)
static struct option knownOptions[] =
{
    { "help",            no_argument,       NULL, 'h' },
    { "version",         no_argument,       NULL, 'v' },
    { "lang",            required_argument, NULL, 'l' },
    { "all-infinitives", no_argument,       NULL, 'i' },

    { NULL, 0, NULL, 0 }  // marks the end
};
#endif


class DeconjugatorCommand : public Command
{
public:

    DeconjugatorCommand(const string &conjugationFilename,
                        const string &verbsFilename,
                        FrenchVerbDictionary::Language lang) throw(logic_error)
      : Command(conjugationFilename, verbsFilename, lang)
    {
    }

    virtual ~DeconjugatorCommand()
    {
    }

protected:

    virtual void processInputWord(const std::string &inputWord);

};


/*virtual*/
void
DeconjugatorCommand::processInputWord(const string &inputWord)
{
    // Analyze the word and get the results in a vector:
    vector<InflectionDesc> v;
    fvd->deconjugate(inputWord, v);


    for (vector<InflectionDesc>::const_iterator it = v.begin();
                                            it != v.end(); it++)
    {
        const InflectionDesc &d = *it;
        cout
            << d.infinitive
            << ", " << FrenchVerbDictionary::getModeName(d.mtpn.mode)
            << ", " << FrenchVerbDictionary::getTenseName(d.mtpn.tense)
            << ", " << int(d.mtpn.person)
            << ", " << (d.mtpn.plural ? "plural" : "singular")
            << "\n";
    }
    cout << endl;
}


static
void
displayVersionNo()
{
    cout << commandName << ' ' << VERSION << '\n';
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
"Copyright (C) " COPYRIGHT_YEARS " Pierre Sarrazin <http://sarrazip.com/>\n"
"This program is free software; you may redistribute it under the terms of\n"
"the GNU General Public License.  This program has absolutely no warranty.\n"
    ;

    cout <<
"\n"
"Options:\n"
"--help             Display this help page and exit\n"
"--version          Display this program's version number and exit\n"
"--lang=L           Select language L (fr for French, it for Italian)\n"
"                   Default is French.\n"
"--all-infinitives  Print the names of all known verbs, one per line (unsorted)\n"
"\n"
"See the " << commandName << "(1) manual page for details.\n"
"\n"
    ;
}


int
main(int argc, char *argv[])
{
    setlocale(LC_CTYPE, "");
    bool listAllInfinitives = false;
    string langCode = "fr";

    #if defined(ENABLE_NLS) && defined(HAVE_GETOPT_LONG)

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

            case 'l':
                langCode = optarg;
                if (FrenchVerbDictionary::parseLanguageCode(langCode) == FrenchVerbDictionary::NO_LANGUAGE)
                {
                    cerr << commandName << ": invalid language code " << optarg << "\n";
                    return EXIT_FAILURE;
                }
                break;

            case 'v':
                displayVersionNo();
                return EXIT_SUCCESS;

            case 'h':
                displayHelp();
                return EXIT_SUCCESS;

            case 'i':
                listAllInfinitives = true;
                break;

            default:
                displayHelp();
                return EXIT_FAILURE;
        }
    } while (c != EOF && c != '?');

    #else

    int optind = 1;

    #endif

    try
    {
        FrenchVerbDictionary::Language lang = FrenchVerbDictionary::parseLanguageCode(langCode);
        if (lang == FrenchVerbDictionary::NO_LANGUAGE)
            lang = FrenchVerbDictionary::FRENCH;

        string conjFN, verbsFN;
        FrenchVerbDictionary::getXMLFilenames(conjFN, verbsFN, lang);

        DeconjugatorCommand cmd(conjFN, verbsFN, lang);

        if (listAllInfinitives)
            return cmd.listAllInfinitives(cout);

        return cmd.run(argc - optind, argv + optind);
    }
    catch (const exception &e)
    {
        cerr << commandName << ": exception: " << e.what() << endl;
        return EXIT_FAILURE;
    }
}
