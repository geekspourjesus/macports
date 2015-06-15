/*  $Id: Command.cpp,v 1.22 2012/11/18 19:56:04 sarrazip Exp $
    Command.cpp - Abstract command-line driver

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

#include <stdlib.h>
#include <iostream>

using namespace std;
using namespace verbiste;


namespace verbiste {


Command::Command(const string &conjugationFilename,
                 const string &verbsFilename,
                 FrenchVerbDictionary::Language _lang) throw (logic_error)
  : fvd(new FrenchVerbDictionary(conjugationFilename, verbsFilename, false, _lang)),
                // command-line tools do not tolerate missing accents
    lang(_lang)
{
}


Command::~Command()
{
    delete fvd;
}


int
Command::run(int argc, char *argv[]) throw()
{
    int optind = 0;
    bool useArgs = (optind < argc);

    try
    {
        if (fvd == NULL)
            throw logic_error("fvd is NULL");

        for (;;)
        {
            string inputWord;

            if (useArgs)
            {
                if (optind == argc)
                    break;

                inputWord = argv[optind];
                optind++;
            }
            else
            {
                if (!getline(cin, inputWord))
                    break;
            }


            processInputWord(inputWord);
        }
    }
    catch (const exception &e)
    {
        cerr << "Exception: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


const FrenchVerbDictionary &
Command::getFrenchVerbDictionary() const throw (logic_error)
{
    if (fvd == NULL)
        throw logic_error("fvd is NULL");
    return *fvd;
}


FrenchVerbDictionary &
Command::getFrenchVerbDictionary() throw (logic_error)
{
    if (fvd == NULL)
        throw logic_error("fvd is NULL");
    return *fvd;
}


int Command::listAllInfinitives(std::ostream &out) const
                                                throw (std::logic_error)
{
    (void) getFrenchVerbDictionary();  // to check that fvd is not null

    for (VerbTable::const_iterator it = fvd->beginKnownVerbs();
                                  it != fvd->endKnownVerbs(); ++it)
        out << it->first << '\n';

    return EXIT_SUCCESS;
}


//static
std::string
Command::getEnv(const char *name, const char *defaultValue /*= string()*/)
{
    const char *value = ::getenv(name);
    if (value == NULL)
        return defaultValue;
    return value;
}


}  // namespace verbiste
