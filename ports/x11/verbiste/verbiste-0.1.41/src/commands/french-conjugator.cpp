/*  $Id: french-conjugator.cpp,v 1.31 2012/11/18 19:56:04 sarrazip Exp $
    french-conjugator.cpp - Conjugation of French verbs

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

#include "gui/conjugation.h"

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


static const char *commandName = "french-conjugator";


#if defined(ENABLE_NLS) && defined(HAVE_GETOPT_LONG)
static struct option knownOptions[] =
{
    { "help",           no_argument,            NULL, 'h' },
    { "version",        no_argument,            NULL, 'v' },
    { "lang",           required_argument,      NULL, 'l' },
    { "mode",           required_argument,      NULL, 'm' },
    { "tense",          required_argument,      NULL, 't' },
    { "template",       required_argument,      NULL, 'e' },
    { "pronouns",       no_argument,            NULL, 'p' },
    { "all-infinitives",no_argument,            NULL, 'i' },

    { NULL, 0, NULL, 0 }  // marks the end
};
#endif


class ConjugatorCommand : public Command
{
public:

    ConjugatorCommand(const string &conjugationFilename,
                      const string &verbsFilename,
                      FrenchVerbDictionary::Language lang) throw(logic_error)
      : Command(conjugationFilename, verbsFilename, lang),
        reqMode(INVALID_MODE),
        reqTense(INVALID_TENSE),
        reqTemplate(),
        includePronouns(false),
        aspirateH(false)
    {
    }

    virtual ~ConjugatorCommand()
    {
    }

    // Constraints:
    Mode reqMode;
    Tense reqTense;
    string reqTemplate;
    bool includePronouns;

protected:

    virtual void processInputWord(const std::string &inputWord);

private:

    bool aspirateH;

    void displayTense(FrenchVerbDictionary &fvd,
                        const string &radical,
                        const TemplateSpec &templ,
                        Mode mode,
                        Tense tense);

    void displayConjugation(FrenchVerbDictionary &fvd,
                        const string &infinitive,
                        const string &tname,
                        const TemplateSpec &templ);
};


void
ConjugatorCommand::displayTense(FrenchVerbDictionary &fvd,
                                const string &radical,
                                const TemplateSpec &templ,
                                Mode mode,
                                Tense tense)
{
    if (reqMode != INVALID_MODE && mode != reqMode)
        return;
    if (reqTense != INVALID_TENSE && tense != reqTense)
        return;

    cout << "- " << FrenchVerbDictionary::getModeName(mode)
                << " " << FrenchVerbDictionary::getTenseName(tense) << ":\n";


    typedef vector<string> VS;
    typedef vector<VS> VVS;

    VVS conjug;
    fvd.generateTense(radical, templ, mode, tense, conjug,
                      includePronouns, aspirateH, lang == FrenchVerbDictionary::ITALIAN);

    for (VVS::const_iterator p = conjug.begin(); p != conjug.end(); p++)
    {
        for (VS::const_iterator i = p->begin(); i != p->end(); i++)
        {
            if (i != p->begin())
                cout << ", ";
            string s = *i;

            cout << s;
        }
        cout << "\n";
    }
}


void
ConjugatorCommand::displayConjugation(FrenchVerbDictionary &fvd,
                                        const string &infinitive,
                                        const string &tname,
                                        const TemplateSpec &templ)
{
    try
    {
        aspirateH = fvd.isVerbStartingWithAspirateH(infinitive);

        string radical = FrenchVerbDictionary::getRadical(infinitive, tname);

        if (lang == FrenchVerbDictionary::FRENCH || lang == FrenchVerbDictionary::ITALIAN)
        {
            displayTense(fvd, radical, templ, INFINITIVE_MODE, PRESENT_TENSE);

            displayTense(fvd, radical, templ, INDICATIVE_MODE, PRESENT_TENSE);
            displayTense(fvd, radical, templ, INDICATIVE_MODE, IMPERFECT_TENSE);
            displayTense(fvd, radical, templ, INDICATIVE_MODE, FUTURE_TENSE);
            displayTense(fvd, radical, templ, INDICATIVE_MODE, PAST_TENSE);

            displayTense(fvd, radical, templ, CONDITIONAL_MODE, PRESENT_TENSE);

            displayTense(fvd, radical, templ, SUBJUNCTIVE_MODE, PRESENT_TENSE);
            displayTense(fvd, radical, templ, SUBJUNCTIVE_MODE, IMPERFECT_TENSE);

            displayTense(fvd, radical, templ, IMPERATIVE_MODE, PRESENT_TENSE);

            displayTense(fvd, radical, templ, PARTICIPLE_MODE, PRESENT_TENSE);
            displayTense(fvd, radical, templ, PARTICIPLE_MODE, PAST_TENSE);
        }
        if (lang == FrenchVerbDictionary::ITALIAN)
            displayTense(fvd, radical, templ, GERUND_MODE, PRESENT_TENSE);

        if (lang == FrenchVerbDictionary::GREEK)
        {
            displayTense(fvd, radical, templ, PRESENT_INDICATIVE, ACTIVE_TENSE);
            displayTense(fvd, radical, templ, PRESENT_INDICATIVE, PASSIVE_TENSE);
            displayTense(fvd, radical, templ, PRESENT_SUBJUNCTIVE, ACTIVE_TENSE);
            displayTense(fvd, radical, templ, PRESENT_SUBJUNCTIVE, PASSIVE_TENSE);
            displayTense(fvd, radical, templ, PRESENT_IMPERATIVE, IMPERATIVE_ACTIVE_TENSE);
            displayTense(fvd, radical, templ, PRESENT_IMPERATIVE, IMPERATIVE_PASSIVE_TENSE);
            displayTense(fvd, radical, templ, PRESENT_GERUND, PRESENT_TENSE);
            displayTense(fvd, radical, templ, PAST_IMPERFECT_INDICATIVE, ACTIVE_TENSE);
            displayTense(fvd, radical, templ, PAST_IMPERFECT_INDICATIVE, PASSIVE_TENSE);
            displayTense(fvd, radical, templ, PAST_PERFECT_INDICATIVE, ACTIVE_TENSE);
            displayTense(fvd, radical, templ, PAST_PERFECT_INDICATIVE, PASSIVE_TENSE);
            displayTense(fvd, radical, templ, PAST_PERFECT_SUBJUNCTIVE, ACTIVE_TENSE);
            displayTense(fvd, radical, templ, PAST_PERFECT_SUBJUNCTIVE, PASSIVE_TENSE);
            displayTense(fvd, radical, templ, PAST_PERFECT_IMPERATIVE, IMPERATIVE_ACTIVE_TENSE);
            displayTense(fvd, radical, templ, PAST_PERFECT_IMPERATIVE, IMPERATIVE_PASSIVE_TENSE);
            displayTense(fvd, radical, templ, PAST_PERFECT_INFINITIVE, PAST_PERFECT);
        }
    }
    catch (logic_error &e)
    {
        return;
    }
}


/*virtual*/
void
ConjugatorCommand::processInputWord(const string &inputWord)
{
    const char *tname = (reqTemplate.empty() ? NULL : reqTemplate.c_str());
    const std::set<std::string> *templateSet = NULL;
    std::set<std::string> singleton;

    if (tname == NULL)  // if no specific template requested
    {
        // Use the template associated with the verb, if known:
        templateSet = &fvd->getVerbTemplateSet(inputWord);
    }
    else
    {
        singleton.insert(tname);
        templateSet = &singleton;
    }

    for (std::set<std::string>::const_iterator it = templateSet->begin();
                                               it != templateSet->end(); ++it)
    {
        if (it != templateSet->begin())
            cout << "-\n";  // separate conjugations

        const string &tname = *it;
        const TemplateSpec *templ = fvd->getTemplate(tname);

        if (templ != NULL)
            displayConjugation(*fvd, inputWord, tname, *templ);
    }

    cout << "-" << endl;  // marks the end of the answer, and flushes it
}


#if defined(ENABLE_NLS) && defined(HAVE_GETOPT_LONG)

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
"--mode=M           Only display mode M (infinitive, indicative, etc)\n"
"--tense=T          Only display tense T (present, past, etc)\n"
"--template=T       Use template T to conjugate the verbs\n"
"--pronouns         Include pronouns in the displayed conjugation\n"
"--all-infinitives  Print the names of all known verbs, one per line (unsorted)\n"
"\n"
"See the " << commandName << "(1) manual page for details.\n"
"\n"
    ;
}

#endif


int
main(int argc, char *argv[])
{
    setlocale(LC_CTYPE, "");

    Mode reqMode = INVALID_MODE;
    Tense reqTense = INVALID_TENSE;
    string reqTemplate;
    bool includePronouns = false;
    bool listAllInfinitives = false;
    string langCode = "fr";

    #if defined(ENABLE_NLS) && defined(HAVE_GETOPT_LONG)

    /*  Interpret the command-line options:
    */
    int c;
    try
    {
        do
        {
            c = getopt_long(argc, argv, "hvm:t:l:p", knownOptions, NULL);

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

                case 'm':
                    reqMode = FrenchVerbDictionary::convertModeName(optarg);
                    if (reqMode == INVALID_MODE)
                    {
                        cerr << commandName << ": invalid mode " << optarg << "\n";
                        return EXIT_FAILURE;
                    }
                    break;

                case 't':
                    reqTense = FrenchVerbDictionary::convertTenseName(optarg);
                    if (reqTense == INVALID_TENSE)
                    {
                        cerr << commandName << ": invalid tense " << optarg << "\n";
                        return EXIT_FAILURE;
                    }
                    break;

                case 'e':
                    reqTemplate = optarg;
                    break;

                case 'p':
                    includePronouns = true;
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
    }
    catch (const exception &e)
    {
        cerr << commandName << ": exception: " << e.what() << endl;
        return EXIT_FAILURE;
    }

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

        ConjugatorCommand cmd(conjFN, verbsFN, lang);

        if (listAllInfinitives)
            return cmd.listAllInfinitives(cout);

        cmd.reqMode = reqMode;
        cmd.reqTense = reqTense;
        cmd.reqTemplate = reqTemplate;
        cmd.includePronouns = includePronouns;

        if (!reqTemplate.empty() &&
                    cmd.getFrenchVerbDictionary().getTemplate(reqTemplate) == NULL)
        {
            cerr << commandName << ": invalid conjugation template "
                                                << optarg << "\n";
            return EXIT_FAILURE;
        }

        return cmd.run(argc - optind, argv + optind);
    }
    catch (const exception &e)
    {
        cerr << commandName << ": exception: " << e.what() << endl;
        return EXIT_FAILURE;
    }
}
