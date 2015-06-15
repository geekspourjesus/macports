/*  $Id: misc-types.cpp,v 1.11 2012/11/18 19:56:06 sarrazip Exp $
    misc-types.cpp - Miscellaneous types used by the dictionary class.

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

#include <verbiste/misc-types.h>
#include <verbiste/FrenchVerbDictionary.h>

using namespace std;
using namespace verbiste;


void
ModeTensePersonNumber::set(const char *modeName,
                                const char *tenseName,
                                int personNum,
                                bool isCorrect,
                                bool isItalian)
{
    correct = isCorrect;
    mode = FrenchVerbDictionary::convertModeName(modeName);
    tense = FrenchVerbDictionary::convertTenseName(tenseName);

    if (mode == IMPERATIVE_MODE)
    {
        if (isItalian)
        {
            switch (personNum)
            {
            case 1: person = 2; plural = false; break;
            case 2: person = 3; plural = false; break;
            case 3: person = 1; plural = true; break;
            case 4: person = 2; plural = true; break;
            case 5: person = 3; plural = true; break;
            default: assert(false); person = 0; plural = false;
            }
        }
        else
        {
            if (personNum == 1)
            {
                person = 2;
                plural = false;
            }
            else if (personNum == 2)
            {
                person = 1;
                plural = true;
            }
            else if (personNum == 3)
            {
                person = 2;
                plural = true;
            }
            else
            {
                person = 0;
                plural = false;
            }
        }
    }
    else if (mode == INFINITIVE_MODE
                || mode == INVALID_MODE
                || personNum < 1 || personNum > 6)
    {
        person = 0;
        plural = false;
    }
    else if (mode == PARTICIPLE_MODE)
    {
        assert(personNum >= 1 && personNum <= 4);
        person = static_cast<unsigned char>(personNum <= 2 ? 4 : 5);
                // convention: 4=masculine, 5=feminine
        plural = (personNum == 2 || personNum == 4);
    }
    else if (mode == GERUND_MODE)
    {
        person = 0;
        plural = false;
    }
    else
    {
        person = static_cast<unsigned char>((personNum - 1) % 3 + 1);
        plural = (personNum > 3);
    }
}


void
ModeTensePersonNumber::dump(Verbiste_ModeTensePersonNumber &destination) const
{
    destination.mode = (Verbiste_Mode) mode;
    destination.tense = (Verbiste_Tense) tense;
    destination.person = (int) person;
    destination.plural = (int) plural;
    destination.correct = (int) correct;
}
