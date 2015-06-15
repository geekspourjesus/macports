/*  $Id: conjugation.cpp,v 1.15 2013/06/28 18:12:03 sarrazip Exp $
    conjugation.cpp - Generic conjugation interface

    verbiste - French conjugation system
    Copyright (C) 2003-2013 Pierre Sarrazin <http://sarrazip.com/>

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

#include "conjugation.h"

#include <libintl.h>
#define _(x) gettext(x)
#define N_(x) (x)

#include <iostream>
#include <string.h>

using namespace std;
using namespace verbiste;


void
getConjugation(const FrenchVerbDictionary &fvd,
                        const string &infinitive,
                        const string &tname,
                        VVVS &dest,
                        bool includePronouns)
{
    const TemplateSpec *templ = fvd.getTemplate(tname);
    if (templ == NULL)
        return;

    try
    {
        static const struct { Mode m; Tense t; } table[] =
        {
            { INFINITIVE_MODE, PRESENT_TENSE },
            { INDICATIVE_MODE, PRESENT_TENSE },
            { INDICATIVE_MODE, IMPERFECT_TENSE },
            { INDICATIVE_MODE, FUTURE_TENSE },
            { INDICATIVE_MODE, PAST_TENSE },
            { CONDITIONAL_MODE, PRESENT_TENSE },
            { SUBJUNCTIVE_MODE, PRESENT_TENSE },
            { SUBJUNCTIVE_MODE, IMPERFECT_TENSE },
            { IMPERATIVE_MODE, PRESENT_TENSE },
            { PARTICIPLE_MODE, PRESENT_TENSE },
            { PARTICIPLE_MODE, PAST_TENSE },
            { GERUND_MODE, PRESENT_TENSE },  // italian only
            { INVALID_MODE, INVALID_TENSE }  // marks the end
        };


        string radical = FrenchVerbDictionary::getRadical(infinitive, tname);

        bool isItalian = (fvd.getLanguage() == FrenchVerbDictionary::ITALIAN);

        for (int j = 0; table[j].m != INVALID_MODE; j++)
        {
            if (table[j].m == GERUND_MODE && !isItalian)
                continue;

            dest.push_back(VVS());
            VVS &tenseDest = dest.back();

            VVS conjug;
            fvd.generateTense(radical, *templ, table[j].m, table[j].t, conjug,
                                includePronouns,
                                fvd.isVerbStartingWithAspirateH(infinitive),
                                isItalian);

            for (VVS::const_iterator p = conjug.begin(); p != conjug.end(); p++)
            {
                tenseDest.push_back(VS());
                for (VS::const_iterator i = p->begin(); i != p->end(); i++)
                {
                    string s = *i;
                    tenseDest.back().push_back(s);
                }
            }
        }
    }
    catch (logic_error &e)
    {
        dest.clear();
        return;
    }
}


static const char *tn[16] =
{
    N_("inf. pres."),
    "",
    "",
    "",
    N_("ind. pres."),
    N_("ind. imperf."),
    N_("ind. fut."),
    N_("ind. past"),
    N_("cond. pres."),
    N_("subj. pres."),
    N_("subj. imperf."),
    "",
    N_("imp. pres."),
    N_("part. pres."),
    N_("part. past"),
    N_("gerund pres."),  // italian only
};


string
getTenseNameForTableCell(int row, int col, bool isItalian)
{
    if (row < 0 || row > 3 || col < 0 || col > 3)
    {
        assert(false);
        return "";
    }

    if (row == 3 && col == 3 && !isItalian)
        return string();

    return _(tn[row * 4 + col]);
}


static
string
removePronoun(const string &s)
{
    string::size_type start = 0;
    if (s.find("que ") == 0)
        start = 4;
    else if (s.find("qu'") == 0)
        start = 3;
    else if (s.find("che ") == 0)  // Italian
        start = 4;

    static const char *pronouns[] =
    {
        "je ", "j'", "tu ", "il ", "nous ", "vous ", "ils ",
        "io ", "tu ", "egli ", "noi ", "voi ", "essi ",  // Italian
        NULL
    };

    for (size_t i = 0; pronouns[i] != NULL; i++)
    {
        if (s.find(pronouns[i], start) == start)
        {
            start += strlen(pronouns[i]);
            break;
        }
    }

    return string(s, start, string::npos);
}


string
createTableCellText(verbiste::FrenchVerbDictionary &fvd,
                    const VVS &tense,
                    const string &lowerCaseUTF8UserText,
                    const string &openMark,
                    const string &closeMark)
{
    string userTextWOAccents = fvd.removeUTF8Accents(lowerCaseUTF8UserText);

    string persons;
    for (VVS::const_iterator it = tense.begin(); it != tense.end(); it++)
    {
        const VS &person = *it;

        if (it != tense.begin())
            persons += "\n";

        for (VS::const_iterator i = person.begin(); i != person.end(); i++)
        {
            if (i != person.begin())
                persons += ", ";

            string inflection = fvd.removeUTF8Accents(removePronoun(*i));
            if (inflection == userTextWOAccents)
                persons += openMark + *i + closeMark;
            else
                persons += *i;
        }
    }
    return persons;
}
