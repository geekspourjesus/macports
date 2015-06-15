/*  $Id: c-api.cpp,v 1.15 2012/11/18 19:56:06 sarrazip Exp $
    FrenchVerbDictionary.cpp - Dictionary of verbs and conjugation templates

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

#include <verbiste/FrenchVerbDictionary.h>

#include <verbiste/misc-types.h>

#include <iostream>
#include <errno.h>
#include <string.h>

using namespace std;
using namespace verbiste;

#define malloc @FORBIDDEN@
#define free @FORBIDDEN@


typedef vector<string> VS;
typedef vector<VS> VVS;


const Verbiste_ModeTense verbiste_valid_modes_and_tenses[] =
{
    { VERBISTE_INFINITIVE_MODE,   VERBISTE_PRESENT_TENSE },

    { VERBISTE_INDICATIVE_MODE,   VERBISTE_PRESENT_TENSE },
    { VERBISTE_INDICATIVE_MODE,   VERBISTE_IMPERFECT_TENSE },
    { VERBISTE_INDICATIVE_MODE,   VERBISTE_FUTURE_TENSE },
    { VERBISTE_INDICATIVE_MODE,   VERBISTE_PAST_TENSE },

    { VERBISTE_CONDITIONAL_MODE,  VERBISTE_PRESENT_TENSE },

    { VERBISTE_SUBJUNCTIVE_MODE,  VERBISTE_PRESENT_TENSE },
    { VERBISTE_SUBJUNCTIVE_MODE,  VERBISTE_IMPERFECT_TENSE },

    { VERBISTE_IMPERATIVE_MODE,   VERBISTE_PRESENT_TENSE },

    { VERBISTE_PARTICIPLE_MODE,   VERBISTE_PRESENT_TENSE },
    { VERBISTE_PARTICIPLE_MODE,   VERBISTE_PAST_TENSE },

    { VERBISTE_GERUND_MODE,       VERBISTE_PRESENT_TENSE },  // Italian only

    { VERBISTE_INVALID_MODE,      VERBISTE_INVALID_TENSE }  // marks the end
};



static FrenchVerbDictionary *fvd = NULL;
static string constructionLogicError;


/*  Returns a dynamically allocated copy of the given C string.
    Example: char *p0 = strnew("foo"); delete [] p0;
    Example: char *p1 = strnew(""); delete [] p1;
    Example: char *p2 = strnew(NULL); delete [] p2;
    @param        c_str         '\0'-terminated string; allowed to be NULL
    @returns                    a pointer to an array of characters that was
                                allocated by new[] and which must be destroyed
                                by the caller with delete[]; if 'c_str' was
                                NULL, the returned pointer is NULL.
*/
inline
char *
strnew(const char *c_str)
{
    if (c_str == NULL)
        return NULL;
    return strcpy(new char[strlen(c_str) + 1], c_str);
}


/*  Returns a dynamically allocated copy of the given string.
    @param        s                string that contains no '\0' characters
    @returns                        a pointer as with strnew(const char *)
*/
inline
char *
strnew(const string &s)
{
    return strnew(s.c_str());
}


int
verbiste_init(const char *conjugation_filename, const char *verbs_filename, const char *lang_code)
{
    if (fvd != NULL)
        return -1;
    if (lang_code == NULL)
        lang_code = "";

    try
    {
        FrenchVerbDictionary::Language lang = FrenchVerbDictionary::parseLanguageCode(lang_code);
        fvd = new FrenchVerbDictionary(conjugation_filename, verbs_filename, false, lang);
    }
    catch (logic_error &e)
    {
        constructionLogicError = e.what();
        return -2;
    }

    return 0;
}


const char *
verbiste_get_init_error()
{
    return constructionLogicError.c_str();
}


int
verbiste_close(void)
{
    if (fvd == NULL)
        return -1;

    delete fvd;
    fvd = NULL;
    return 0;
}


void
verbiste_free_string(char *str)
{
    delete [] str;
}


const char *
verbiste_get_mode_name(Verbiste_Mode mode)
{
    return FrenchVerbDictionary::getModeName((Mode) mode);
}


const char *
verbiste_get_tense_name(Verbiste_Tense tense)
{
    return FrenchVerbDictionary::getTenseName((Tense) tense);
}


static
Verbiste_ModeTensePersonNumber *
createModeTensePersonNumberArray(const vector<InflectionDesc> &vec)
{
    size_t vecSize = vec.size();

    Verbiste_ModeTensePersonNumber *array =
                        new Verbiste_ModeTensePersonNumber[vecSize + 1];
    if (array == NULL)
        return NULL;

    for (size_t i = 0; i < vecSize; i++)
    {
        array[i].infinitive_verb = strnew(vec[i].infinitive.c_str());
        vec[i].mtpn.dump(array[i]);
    }

    array[vecSize].infinitive_verb = NULL;
    array[vecSize].mode = VERBISTE_INVALID_MODE;
    array[vecSize].tense = VERBISTE_INVALID_TENSE;
    array[vecSize].person = 0;
    array[vecSize].plural = false;

    return array;
}


Verbiste_ModeTensePersonNumber *
verbiste_deconjugate(const char *verb)
{
    vector<InflectionDesc> vec;
    fvd->deconjugate(verb, vec);
    return createModeTensePersonNumberArray(vec);
}


void
verbiste_free_mtpn_array(Verbiste_ModeTensePersonNumber *array)
{
    if (array == NULL)
        return;

    for (size_t i = 0; array[i].infinitive_verb != NULL; i++)
        delete [] array[i].infinitive_verb;

    delete [] array;
}


static
int
generateTense(VVS &conjug,
                const char *infinitive,
                const char *templateName,
                Verbiste_Mode mode,
                Verbiste_Tense tense,
                bool include_pronouns)
{
    const TemplateSpec *templ = fvd->getTemplate(templateName);
    if (templ == NULL)
        return -2;
    string radical = FrenchVerbDictionary::getRadical(infinitive, templateName);

    fvd->generateTense(radical, *templ, (Mode) mode, (Tense) tense, conjug,
                        include_pronouns,
                        fvd->isVerbStartingWithAspirateH(infinitive),
                        false);
    return 0;
}


Verbiste_TemplateArray
verbiste_get_verb_template_array(const char *infinitive_verb)
{
    if (infinitive_verb == NULL)
        return NULL;
    const std::set<std::string> &templateSet = fvd->getVerbTemplateSet(infinitive_verb);
    if (templateSet.empty())
        return NULL;

    Verbiste_TemplateArray a = new char *[templateSet.size() + 1];
    size_t i = 0;
    for (std::set<std::string>::const_iterator it = templateSet.begin();
                                               it != templateSet.end(); ++it, ++i)
        a[i] = strnew(it->c_str());
    a[i] = NULL;
    return a;
}


static void
free_string_array(char *array[])
{
    if (array == NULL)
        return;
    for (size_t i = 0; array[i] != NULL; ++i)
        delete [] array[i];
    delete [] array;
}


void
verbiste_free_verb_template_array(Verbiste_TemplateArray array)
{
    free_string_array(array);
}


Verbiste_PersonArray
verbiste_conjugate(const char *infinitive_verb,
                   const char *template_name,
                   const Verbiste_Mode mode,
                   const Verbiste_Tense tense,
                   int include_pronouns)
{
    VVS tenseConjug;
    if (::generateTense(tenseConjug, infinitive_verb, template_name, mode, tense,
                                                include_pronouns != 0) != 0)
        return NULL;

    size_t numPersons = tenseConjug.size();
    Verbiste_PersonArray personArray =
                                new Verbiste_InflectionArray[numPersons + 1];

    for (size_t i = 0; i < numPersons; i++)
    {
        const VS &inflections = tenseConjug[i];
        size_t numInf = inflections.size();
        Verbiste_InflectionArray infArray = new char *[numInf + 1];
        for (size_t j = 0; j < numInf; j++)
            infArray[j] = strnew(inflections[j]);
        infArray[numInf] = NULL;
        personArray[i] = infArray;
    }
    personArray[numPersons] = NULL;
    return personArray;
}


void
verbiste_free_person_array(Verbiste_PersonArray array)
{
    if (array == NULL)
        return;

    for (size_t i = 0; array[i] != NULL; i++)
        free_string_array(array[i]);
    delete [] array;
}
