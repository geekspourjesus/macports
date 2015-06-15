/*  $Id: FrenchVerbDictionary.cpp,v 1.52 2012/11/18 19:56:05 sarrazip Exp $
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

#include "FrenchVerbDictionary.h"

#include <assert.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace verbiste;


static bool trace = getenv("TRACE") != NULL;


class AutoDoc
{
public:
    AutoDoc(xmlDocPtr d) : doc(d) {}
    ~AutoDoc() { if (doc != NULL) xmlFreeDoc(doc); }
    xmlDocPtr get() const { return doc; }
    bool operator ! () const { return doc == NULL; }
private:
    xmlDocPtr doc;

    // Forbidden operations:
    AutoDoc(const AutoDoc &);
    AutoDoc &operator = (const AutoDoc &);
};


class AutoString
{
public:
    AutoString(xmlChar *s) : str(s) {}
    ~AutoString() { if (str != NULL) xmlFree(str); }
    xmlChar *get() const { return str; }
    bool operator ! () const { return str == NULL; }
    size_t length() const { return str == NULL ? 0 : strlen((char *) str); }
private:
    xmlChar *str;

    // Forbidden operations:
    AutoString(const AutoString &);
    AutoString &operator = (const AutoString &);
};


inline
const xmlChar *
XMLCHAR(const char *s)
{
    return (const xmlChar *) s;
}


inline
int
equal(const xmlChar *a, const char *b)
{
    return xmlStrcmp(a, XMLCHAR(b)) == 0;
}


inline
int
different(const xmlChar *a, const char *b)
{
    return !equal(a, b);
}


inline
xmlChar *
getProp(xmlNodePtr node, const char *propName)
{
    return xmlGetProp(node, XMLCHAR(propName));
}


inline
xmlChar *
getString(xmlDocPtr doc, xmlNodePtr node)
{
    return xmlNodeListGetString(doc, node, 1);
}


inline
string
operator + (const AutoString &a, const string &b)
{
    return (char *) a.get() + b;
}


inline
string
operator + (const string &a, const AutoString &b)
{
    return a + (char *) b.get();
}


inline
Mode
convertModeName(const xmlChar *modeName)
{
    return FrenchVerbDictionary::convertModeName((char *) modeName);
}


inline
Tense
convertTenseName(const xmlChar *tenseName)
{
    return FrenchVerbDictionary::convertTenseName((char *) tenseName);
}


// Latin-1 to ASCII conversion table (codes 0xC0 to 0xFF).
// Some characters have bogus translations, but they are not used in French.
//
static const char *accentRemovalTable =
            "AAAAAA_CEEEEIIII"
            "DNOOOOOxOUUUUYbB"
            "aaaaaa-ceeeeiiii"
            "dnooooo/ouuuuyby";


// Only works on Latin-1 characters.
//
inline wchar_t
removeWideCharAccent(wchar_t c)
{
    if (c >= 0xC0 && c <= 0xFF)
        c = (unsigned char) accentRemovalTable[c - 0xC0];
    return c;
}


string
FrenchVerbDictionary::removeUTF8Accents(const string &utf8String)
{
    wstring result = utf8ToWide(utf8String);
    for (size_t i = 0; i < result.length(); ++i)
        result[i] = removeWideCharAccent(result[i]);
    return wideToUTF8(result);
}


void
FrenchVerbDictionary::formUTF8UnaccentedVariants(const wstring &wideString,
                                                size_t index,
                                                vector<string> &utf8Variants)
{
    for ( ; index < wideString.length(); ++index)
    {
        wchar_t ch = wideString[index];
        wchar_t unacc = removeWideCharAccent(ch);
        if (ch != unacc)
        {
            wstring copy = wideString;
            copy[index] = unacc;
            assert(copy.length() == wideString.length());

            utf8Variants.push_back(wideToUTF8(copy));
            formUTF8UnaccentedVariants(copy, index + 1, utf8Variants);
        }
    }
}


void
FrenchVerbDictionary::formUTF8UnaccentedVariants(const string &utf8String,
                                                size_t index,
                                                vector<string> &utf8Variants)
{
    wstring wideString = utf8ToWide(utf8String);
    formUTF8UnaccentedVariants(wideString, index, utf8Variants);
}


void
FrenchVerbDictionary::getXMLFilenames(string &conjFN, string &verbsFN, Language l)
{
    const char *libdatadir = getenv("LIBDATADIR");
    if (libdatadir == NULL)
        libdatadir = LIBDATADIR;
    string languageCode = getLanguageCode(l);
    conjFN  = libdatadir + string("/") + "conjugation-" + languageCode + ".xml";
    verbsFN = libdatadir + string("/") + "verbs-" + languageCode + ".xml";
}


//static
FrenchVerbDictionary::Language
FrenchVerbDictionary::parseLanguageCode(const std::string &twoLetterCode)
{
    if (twoLetterCode == "fr")
        return FRENCH;
    if (twoLetterCode == "it")
        return ITALIAN;
    if (twoLetterCode == "el")
        return GREEK;
    return NO_LANGUAGE;
}


//static
std::string
FrenchVerbDictionary::getLanguageCode(Language l)
{
    switch (l)
    {
    case NO_LANGUAGE: return "";
    case FRENCH: return "fr";
    case ITALIAN: return "it";
    case GREEK: return "el";
    }
    return "";
}


FrenchVerbDictionary::FrenchVerbDictionary(
                                const string &conjugationFilename,
                                const string &verbsFilename,
                                bool includeWithoutAccents,
                                Language _lang)
                                        throw (logic_error)
  : conjugSys(),
    knownVerbs(),
    aspirateHVerbs(),
    inflectionTable(),
    wideToUTF8Conv((iconv_t) -1),
    utf8ToWideConv((iconv_t) -1),
    verbTrie(*this),
    lang(_lang)
{
    if (lang == NO_LANGUAGE)
        throw logic_error("Invalid language code");
    init(conjugationFilename, verbsFilename, includeWithoutAccents);
}


FrenchVerbDictionary::FrenchVerbDictionary(bool includeWithoutAccents)
                                                throw (std::logic_error)
  : conjugSys(),
    knownVerbs(),
    aspirateHVerbs(),
    inflectionTable(),
    wideToUTF8Conv((iconv_t) -1),
    utf8ToWideConv((iconv_t) -1),
    verbTrie(*this),
    lang(FRENCH)
{
    string conjFN, verbsFN;
    getXMLFilenames(conjFN, verbsFN, lang);

    init(conjFN, verbsFN, includeWithoutAccents);
}


void
FrenchVerbDictionary::init(const string &conjugationFilename,
                            const string &verbsFilename,
                            bool includeWithoutAccents)
                                        throw (logic_error)
{
    wideToUTF8Conv = iconv_open("UTF-8", "WCHAR_T");
    if (wideToUTF8Conv == (iconv_t) -1)
        throw logic_error("conversion from wide characters to UTF-8 not supported");
    utf8ToWideConv = iconv_open("WCHAR_T", "UTF-8");
    if (utf8ToWideConv == (iconv_t) -1)
        throw logic_error("conversion from UTF-8 to wide characters not supported");

    #ifndef NDEBUG  // self-test for the wide character string conversions:
    try
    {
        wstring w = utf8ToWide("ab");
        assert(w.length() == 2);
        assert(w[0] == 'a');
        assert(w[1] == 'b');

        const char u0[] = { '\xc3', '\xa2', 't', '\0' };  // 'a' with circumflex accent
        w = utf8ToWide(u0);
        assert(w.length() == 2);
        assert(w[0] == 0xe2);
        assert(w[1] == 't');

        const char u1[] = { 't', '\xc3', '\xa2', '\0' };  // 'a' with circumflex accent
        w = utf8ToWide(u1);
        assert(w.length() == 2);
        assert(w[0] == 't');
        assert(w[1] == 0xe2);
    }
    catch (int e)
    {
        throw logic_error("self-test of utf8ToWide() failed");
    }

    try
    {
        string u = wideToUTF8(L"ab");
        assert(u.length() == 2);
        assert(u[0] == 'a');
        assert(u[1] == 'b');
    }
    catch (int e)
    {
        throw logic_error("self-test of wideToUTF8() failed");
    }
    #endif  // ndef NDEBUG


    {
        for (int i = 0; i < 0xC0; i++)
            latin1TolowerTable[i] = char(tolower(char(i)));
        for (int i = 0xC0; i < 0xE0; i++)
            latin1TolowerTable[i] = char(i + 0x20);
        for (int i = 0xE0; i < 0x100; i++)
            latin1TolowerTable[i] = char(i);
    }

    loadConjugationDatabase(conjugationFilename.c_str(), includeWithoutAccents);
    loadVerbDatabase(verbsFilename.c_str(), includeWithoutAccents);

    // Load additional verbs from $HOME/.verbiste/verbs-<lang>.xml, if present.
    //
    const char *home = getenv("HOME");
    if (home != NULL)  // do nothing if $HOME not defined
    {
        string otherVerbsFilename = string(home) + "/.verbiste/verbs-" + getLanguageCode(lang) + ".xml";
        struct stat statbuf;
        if (stat(otherVerbsFilename.c_str(), &statbuf) == 0)  // if file exists
        {
            //cout << "otherVerbsFilename=" << otherVerbsFilename << endl;
            loadVerbDatabase(otherVerbsFilename.c_str(), includeWithoutAccents);
        }
    }

    if (trace)
        cout << "FrenchVerbDictionary::init: trie takes "
             << verbTrie.computeMemoryConsumption() << " bytes\n";
}


void
FrenchVerbDictionary::loadConjugationDatabase(
                                const char *conjugationFilename,
                                bool includeWithoutAccents)
                                        throw (logic_error)
{
    if (conjugationFilename == NULL)
        throw invalid_argument("conjugationFilename");

    AutoDoc conjDoc(xmlParseFile(conjugationFilename));
    if (!conjDoc)
        throw logic_error("could not parse " + string(conjugationFilename));

    readConjugation(conjDoc.get(), includeWithoutAccents);
}


void
FrenchVerbDictionary::loadVerbDatabase(
                                const char *verbsFilename,
                                bool includeWithoutAccents)
                                        throw (logic_error)
{
    if (verbsFilename == NULL)
        throw invalid_argument("verbsFilename");

    AutoDoc verbsDoc(xmlParseFile(verbsFilename));
    if (!verbsDoc)
        throw logic_error("could not parse " + string(verbsFilename));

    readVerbs(verbsDoc.get(), includeWithoutAccents);
}


void
FrenchVerbDictionary::readConjugation(xmlDocPtr doc, bool includeWithoutAccents) throw(logic_error)
{
    const bool isItalian = (lang == ITALIAN);

    xmlNodePtr rootNodePtr = xmlDocGetRootElement(doc);

    if (rootNodePtr == NULL)
        throw logic_error("empty conjugation document");

    string langCode = getLanguageCode(lang);
    if (different(rootNodePtr->name, ("conjugation-" + langCode).c_str()))
    {
        string msg = "wrong top node in conjugation document: got "
                     + string((const char *) rootNodePtr->name)
                     + ", expected conjugation-" + langCode;
        throw logic_error(msg);
    }

    for (xmlNodePtr templ = rootNodePtr->xmlChildrenNode;
                        templ != NULL;
                        templ = templ->next)
    {
        if (different(templ->name, "template"))  // ignore junk between tags
            continue;

        string tname = getUTF8XmlProp(templ, "name");
        if (tname.empty())
            throw logic_error("missing template name attribute");

        // The template name is the root and the termination,
        // with a colon in between.  For example, "pla:cer".

        if (tname.find(':') == string::npos)
            throw logic_error("missing colon in template name");

        // The use of the [] operator creates an empty conjugation
        // template spec, to which we keep a reference:

        TemplateSpec &theTemplateSpec = conjugSys[tname];

        // Same idea:

        TemplateInflectionTable &ti = inflectionTable[tname];

        // For each mode (e.g., infinitive, indicative, conditional, etc):
        for (xmlNodePtr mode = templ->xmlChildrenNode;
                            mode != NULL;
                            mode = mode->next)
        {
            if (equal(mode->name, "text") || equal(mode->name, "comment"))  // any text in this node is ignored
                continue;

            if (trace) cout << "readConjugation: mode node: '" << mode->name << "'" << endl;
            Mode theMode = ::convertModeName(mode->name);
            ModeSpec &theModeSpec = theTemplateSpec[theMode];

            // For each tense in the mode:
            for (xmlNodePtr tense = mode->xmlChildrenNode;
                            tense != NULL;
                                tense = tense->next)
            {
                if (equal(tense->name, "text") || equal(tense->name, "comment"))
                    continue;

                Tense theTense = ::convertTenseName(tense->name);
                TenseSpec &theTenseSpec = theModeSpec[theTense];

                // For each person in the tense:
                int personCounter = 0;
                for (xmlNodePtr person = tense->xmlChildrenNode;
                                person != NULL;
                                person = person->next)
                {
                    if (different(person->name, "p"))
                        continue;

                    personCounter++;

                    theTenseSpec.push_back(PersonSpec());
                    PersonSpec &thePersonSpec = theTenseSpec.back();

                    // For each variant for this person:
                    // (Note that most persons of most verbs have only
                    // one variant.)
                    for (xmlNodePtr inf = person->xmlChildrenNode;
                                    inf != NULL;
                                    inf = inf->next)
                    {
                        string variant = getUTF8XmlNodeText(
                                                    doc, inf->xmlChildrenNode);
                        thePersonSpec.push_back(InflectionSpec(variant, true));

                        ModeTensePersonNumber mtpn(
                                reinterpret_cast<const char *>(mode->name),
                                reinterpret_cast<const char *>(tense->name),
                                personCounter,
                                true,
                                isItalian);
                        ti[variant].push_back(mtpn);

                        if (includeWithoutAccents)
                        {
                            // Also include versions where some or all accents are missing.
                            vector<string> unaccentedVariants;
                            formUTF8UnaccentedVariants(variant, 0, unaccentedVariants);
                            for (vector<string>::const_iterator it = unaccentedVariants.begin();
                                                                it != unaccentedVariants.end(); ++it)
                            {
                                thePersonSpec.push_back(InflectionSpec(*it, false));
                                mtpn.correct = false;  // 'false' marks this spelling as incorrect.
                                ti[*it].push_back(mtpn);
                            }
                        }
                    }
                }
            }
        }
    }
}


string
FrenchVerbDictionary::getUTF8XmlNodeText(xmlDocPtr doc, xmlNodePtr node)
                                                                throw(int)
{
    xmlChar *s = getString(doc, node);
    if (s == NULL)
        return string();
    return reinterpret_cast<char *>(s);
}


string
FrenchVerbDictionary::getUTF8XmlProp(xmlNodePtr node, const char *propName)
                                                                throw(int)
{
    xmlChar *s = getProp(node, propName);
    if (s == NULL)
        return string();
    return reinterpret_cast<char *>(s);
}


// Reads the given XML document and adds data to members knownVerbs,
// aspirateHVerbs and verbTrie.
//
void
FrenchVerbDictionary::readVerbs(xmlDocPtr doc,
                                bool includeWithoutAccents)
                                                throw(logic_error)
{
    if (trace)
        cout << "readVerbs: start: includeWithoutAccents=" << includeWithoutAccents << endl;

    xmlNodePtr rootNodePtr = xmlDocGetRootElement(doc);

    if (rootNodePtr == NULL)
        throw logic_error("empty verbs document");

    string langCode = getLanguageCode(lang);
    if (different(rootNodePtr->name, ("verbs-" + langCode).c_str()))
        throw logic_error("wrong top node in verbs document");

    for (xmlNodePtr v = rootNodePtr->xmlChildrenNode; v != NULL; v = v->next)
    {
        if (equal(v->name, "text") || equal(v->name, "comment"))
            continue;

        xmlNodePtr i = v->xmlChildrenNode;
        if (i == NULL || i->xmlChildrenNode == NULL)
            throw logic_error("missing <i> node");

        string utf8Infinitive = getUTF8XmlNodeText(doc, i->xmlChildrenNode);
        wstring wideInfinitive = utf8ToWide(utf8Infinitive);
        if (wideInfinitive.empty())
            throw logic_error("empty <i> node");
        size_t lenInfinitive = wideInfinitive.length();
        if (trace) cout << "utf8Infinitive='" << utf8Infinitive << "'\n";

        if (i->next == NULL)
            throw logic_error("unexpected end after <i> node");

        xmlNodePtr t = i->next->next;
        if (t == NULL)
            throw logic_error("missing <t> node");

        #if 0
        cout << "t=" << t << ", t->xmlChildrenNode=" << t->xmlChildrenNode << "\n";
        if (t->xmlChildrenNode == NULL)
            cout << "  t->next=" << t->next << ", " << (t->next ? getUTF8XmlNodeText(doc, t->next->xmlChildrenNode) : 0) << endl;
        #endif

        // Get template name (e.g., "aim:er") in UTF-8.
        string utf8TName = getUTF8XmlNodeText(doc, t->xmlChildrenNode);
        if (utf8TName.empty())
            throw logic_error("empty <t> node");
        if (trace) cout << "  utf8TName='" << utf8TName << "'\n";

        // Check that this template name (seen in verbs-*.xml) has been
        // seen in conjugation-*.xml.
        //
        if (conjugSys.find(utf8TName) == conjugSys.end())
            throw logic_error("unknown template name: " + utf8TName);

        // Find the offset of the colon in the template name.
        // For example: the offset is 3 in the case of "aim:er".
        // Find this offset in a wide character string, because
        // the offset in a UTF-8 string is in bytes, not characters.
        //
        wstring wideTName = utf8ToWide(utf8TName);
        wstring::size_type posColon = wideTName.find(':');
        if (posColon == wstring::npos)
            throw logic_error("missing colon in <t> node");
        assert(wideTName[posColon] == ':');


        knownVerbs[utf8Infinitive].insert(utf8TName);

        if (includeWithoutAccents)
        {
            // Also include versions where some of all accents are missing.
            vector<string> unaccentedVariants;
            formUTF8UnaccentedVariants(wideInfinitive, 0, unaccentedVariants);
            for (vector<string>::const_iterator it = unaccentedVariants.begin();
                                                it != unaccentedVariants.end(); ++it)
            {
                if (trace) cout << "  unaccvar: '" << *it << "'\n";
                knownVerbs[*it].insert(utf8TName);
            }
        }

        // <aspirate-h>: If this verb starts with an aspirate h, remember it:
        if (t->next != NULL && t->next->next != NULL)
            aspirateHVerbs.insert(utf8Infinitive);

        // Insert the verb in the trie.
        // A list of template names is associated to each verb in this trie.

        size_t lenTermination = wideTName.length() - posColon - 1;
        assert(lenTermination > 0);
        assert(lenInfinitive >= lenTermination);

        wstring wideVerbRadical(wideInfinitive, 0, lenInfinitive - lenTermination);
        string utf8VerbRadical = wideToUTF8(wideVerbRadical);

        insertVerbRadicalInTrie(utf8VerbRadical, utf8TName, utf8VerbRadical);

        if (includeWithoutAccents)
        {
            // Also include versions where some of all accents are missing.
            vector<string> unaccentedVariants;
            formUTF8UnaccentedVariants(wideVerbRadical, 0, unaccentedVariants);
            for (vector<string>::const_iterator it = unaccentedVariants.begin();
                                                it != unaccentedVariants.end(); ++it)
            {
                insertVerbRadicalInTrie(*it, utf8TName, utf8VerbRadical);  // pass correct verb radical as 3rd argument
            }
        }
    }

    if (trace)
        cout << "Number of known verbs (lang " << langCode << "): " << knownVerbs.size() << endl;
}


// String parameters expected to be in UTF-8.
// Adds to 'verbTrie', which contains wide character strings.
//
void
FrenchVerbDictionary::insertVerbRadicalInTrie(
                                    const std::string &verbRadical,
                                    const std::string &tname,
                                    const std::string &correctVerbRadical)
{
    wstring wideVerbRadical = utf8ToWide(verbRadical);
    if (trace)
        cout << "insertVerbRadicalInTrie('"
              << verbRadical << "' (len=" << wideVerbRadical.length()
              << "), '" << tname
              << "', '" << correctVerbRadical
              << "')\n";

    vector<TrieValue> **templateListPtr =
                            verbTrie.getUserDataPointer(wideVerbRadical);
    assert(templateListPtr != NULL);

    // If a new entry was created for 'wideVerbRadical', then the associated
    // user data pointer is null.  Make this pointer point to a new,
    // empty vector of template names.
    //
    if (*templateListPtr == NULL)
        *templateListPtr = new vector<TrieValue>();

    // Associate the given template name to the given verb radical.
    //
    (*templateListPtr)->push_back(TrieValue(tname, correctVerbRadical));
}


FrenchVerbDictionary::~FrenchVerbDictionary()
{
    iconv_close(utf8ToWideConv);
    iconv_close(wideToUTF8Conv);
}


const TemplateSpec *
FrenchVerbDictionary::getTemplate(const string &templateName) const
{
    ConjugationSystem::const_iterator it = conjugSys.find(templateName);
    if (it == conjugSys.end())
        return NULL;
    return &it->second;
}


ConjugationSystem::const_iterator
FrenchVerbDictionary::beginConjugSys() const
{
    return conjugSys.begin();
}


ConjugationSystem::const_iterator
FrenchVerbDictionary::endConjugSys() const
{
    return conjugSys.end();
}


const std::set<std::string> &
FrenchVerbDictionary::getVerbTemplateSet(const char *infinitive) const
{
    static const std::set<std::string> emptySet;
    if (infinitive == NULL)
        return emptySet;
    VerbTable::const_iterator it = knownVerbs.find(infinitive);
    if (it == knownVerbs.end())
        return emptySet;
    return it->second;
}


const std::set<std::string> &
FrenchVerbDictionary::getVerbTemplateSet(const string &infinitive) const
{
    return getVerbTemplateSet(infinitive.c_str());
}


VerbTable::const_iterator
FrenchVerbDictionary::beginKnownVerbs() const
{
    return knownVerbs.begin();
}


VerbTable::const_iterator
FrenchVerbDictionary::endKnownVerbs() const
{
    return knownVerbs.end();
}


const std::vector<ModeTensePersonNumber> *
FrenchVerbDictionary::getMTPNForInflection(
                                const std::string &templateName,
                                const std::string &inflection) const
{
    InflectionTable::const_iterator i = inflectionTable.find(templateName);
    if (i == inflectionTable.end())
        return NULL;
    const TemplateInflectionTable &ti = i->second;
    TemplateInflectionTable::const_iterator j = ti.find(inflection);
    if (j == ti.end())
        return NULL;
    return &j->second;
}


/*static*/
Mode
FrenchVerbDictionary::convertModeName(const char *modeName)
{
    Mode mode = INVALID_MODE;
    if (modeName == NULL)
        ;
    else if (strcmp(modeName, "infinitive") == 0)
        mode = INFINITIVE_MODE;
    else if (strcmp(modeName, "indicative") == 0)
        mode = INDICATIVE_MODE;
    else if (strcmp(modeName, "conditional") == 0)
        mode = CONDITIONAL_MODE;
    else if (strcmp(modeName, "subjunctive") == 0)
        mode = SUBJUNCTIVE_MODE;
    else if (strcmp(modeName, "imperative") == 0)
        mode = IMPERATIVE_MODE;
    else if (strcmp(modeName, "participle") == 0)
        mode = PARTICIPLE_MODE;
    else if (strcmp(modeName, "gerund") == 0)
        mode = GERUND_MODE;
    else if (strcmp(modeName, "present-indicative") == 0)
        mode = PRESENT_INDICATIVE;
    else if (strcmp(modeName, "present-subjunctive") == 0)
        mode = PRESENT_SUBJUNCTIVE;
    else if (strcmp(modeName, "present-imperative") == 0)
        mode = PRESENT_IMPERATIVE;
    else if (strcmp(modeName, "present-gerund") == 0)
        mode = PRESENT_GERUND;
    else if (strcmp(modeName, "past-imperfect-indicative") == 0)
        mode = PAST_IMPERFECT_INDICATIVE;
    else if (strcmp(modeName, "past-perfect-indicative") == 0)
        mode = PAST_PERFECT_INDICATIVE;
    else if (strcmp(modeName, "past-perfect-subjunctive") == 0)
        mode = PAST_PERFECT_SUBJUNCTIVE;
    else if (strcmp(modeName, "past-perfect-imperative") == 0)
        mode = PAST_PERFECT_IMPERATIVE;
    else if (strcmp(modeName, "past-perfect-infinitive") == 0)
        mode = PAST_PERFECT_INFINITIVE;

    if (mode == INVALID_MODE)
    {
        if (trace) cout << "modeName='" << modeName << "'" << endl;
        assert(!"Invalid mode");
    }

    return mode;
}


/*static*/
Tense
FrenchVerbDictionary::convertTenseName(const char *tenseName)
{
    Tense tense = INVALID_TENSE;
    if (tenseName == NULL)
        ;
    else if (strcmp(tenseName, "infinitive-present") == 0)
        tense = PRESENT_TENSE;
    else if (strcmp(tenseName, "present") == 0)
        tense = PRESENT_TENSE;
    else if (strcmp(tenseName, "imperfect") == 0)
        tense = IMPERFECT_TENSE;
    else if (strcmp(tenseName, "future") == 0)
        tense = FUTURE_TENSE;
    else if (strcmp(tenseName, "simple-past") == 0)
        tense = PAST_TENSE;
    else if (strcmp(tenseName, "imperative-present") == 0)
        tense = PRESENT_TENSE;
    else if (strcmp(tenseName, "present-participle") == 0)
        tense = PRESENT_TENSE;
    else if (strcmp(tenseName, "past-participle") == 0)
        tense = PAST_TENSE;
    else if (strcmp(tenseName, "past") == 0)
        tense = PAST_TENSE;
    else if (strcmp(tenseName, "present-gerund") == 0)
        tense = PRESENT_TENSE;
    else if (strcmp(tenseName, "active") == 0)
        tense = ACTIVE_TENSE;
    else if (strcmp(tenseName, "passive") == 0)
        tense = PASSIVE_TENSE;
    else if (strcmp(tenseName, "imp-active") == 0)
        tense = IMPERATIVE_ACTIVE_TENSE;
    else if (strcmp(tenseName, "imp-passive") == 0)
        tense = IMPERATIVE_PASSIVE_TENSE;
    else if (strcmp(tenseName, "past-perfect") == 0)
        tense = PAST_PERFECT;

    if (tense == INVALID_TENSE)
    {
        if (trace) cout << "tenseName='" << tenseName << "'" << endl;
        assert(!"Invalid tense");
    }

    return tense;
}


void
FrenchVerbDictionary::deconjugate(const string &utf8ConjugatedVerb,
                                std::vector<InflectionDesc> &results)
{
    verbTrie.setDestination(&results);

    try
    {
        wstring w = utf8ToWide(utf8ConjugatedVerb);
        (void) verbTrie.get(w);
    }
    catch (int e)  // exception throw by utf8towide()
    {
        // Wrong encoding (possibly Latin-1). Act as with unknown verb.
    }

    verbTrie.setDestination(NULL);
}


/*virtual*/
void
FrenchVerbDictionary::VerbTrie::onFoundPrefixWithUserData(
                        const wstring &conjugatedVerb,
                        wstring::size_type index,
                        const vector<TrieValue> *templateList) const throw()
{
    assert(templateList != NULL);
    if (trace)
        wcout << "VerbTrie::onFoundPrefixWithUserData: start: conjugatedVerb='"
              << conjugatedVerb << "', index=" << index
              << ", templateList: " << templateList->size()
              << ", results=" << results << endl;

    if (results == NULL)
        return;

    const wstring term(conjugatedVerb, index);
    const string utf8Term = fvd.wideToUTF8(term);

    if (trace)
        cout << "  utf8Term='" << utf8Term << "'\n";

    /*
        'templateList' contains the names of conjugated templates that might
        apply to the conjugated verb.  We check each of them to see if there
        is one that accepts the given termination 'term'.
    */
    for (vector<TrieValue>::const_iterator i = templateList->begin();
                                           i != templateList->end(); i++)
    {
        const TrieValue &trieValue = *i;
        const string &tname = trieValue.templateName;
        const TemplateInflectionTable &ti =
                                fvd.inflectionTable.find(tname)->second;
        TemplateInflectionTable::const_iterator j = ti.find(utf8Term);
        if (trace)
            cout << "    tname='" << tname << "'\n";
        if (j == ti.end())
            continue;  // template 'tname' does not accept termination 'term'

        // template 'tname' accepts 'term', so we produce some results.

        string templateTerm(tname, tname.find(':') + 1);
            // termination of the infinitive form
        if (trace)
            cout << "    templateTerm='" << templateTerm << "'\n";

        const vector<ModeTensePersonNumber> &v = j->second;
            // list of mode-tense-person combinations that can correspond
            // to the conjugated verb's termination

        for (vector<ModeTensePersonNumber>::const_iterator k = v.begin();
                                                    k != v.end(); k++)
        {
            const ModeTensePersonNumber &mtpn = *k;

            string infinitive = trieValue.correctVerbRadical + templateTerm;
                // The infinitive of the conjugated verb is formed from its
                // (correct) radical part and from the termination of the template name.
                // Correct means with the proper accents. This allows the user
                // to type "etaler" without the acute accent on the first "e"
                // and obtain the conjugation for the correct verb, which has
                // that accent.

            if (trace)
            {
                const wstring radical(conjugatedVerb, 0, index);
                cout << "VerbTrie::onFoundPrefixWithUserData: radical='"
                    << fvd.wideToUTF8(radical) << "', templateTerm='" << templateTerm
                    << "', tname='" << tname
                    << "', correctVerbRadical='" << trieValue.correctVerbRadical
                    << "', mtpn=("
                    << mtpn.mode << ", "
                    << mtpn.tense << ", "
                    << (unsigned) mtpn.person << ", "
                    << mtpn.plural << ", "
                    << mtpn.correct << ")\n";
            }

            results->push_back(InflectionDesc(infinitive, tname, mtpn));
                // the InflectionDesc object is an analysis of the
                // conjugated verb
        }
    }
}


/*static*/
const char *
FrenchVerbDictionary::getModeName(Mode m)
{
    if (int(m) < int(INFINITIVE_MODE) || int(m) > int(PAST_PERFECT_INFINITIVE))
    {
        assert(!"FrenchVerbDictionary::getModeName() received invalid Mode value");
        return NULL;
    }

    static const char *names[] =
    {
        "infinitive", "indicative", "conditional",
        "subjunctive", "imperative", "participle",
        "gerund",
        "present indicative",
        "present subjunctive",
        "present imperative",
        "present gerund",
        "past imperfect indicative",
        "past perfect indicative",
        "past perfect subjunctive",
        "past perfect imperative",
        "past perfect infinitive",
    };

    size_t index = size_t(m) - 1;
    assert(index < sizeof(names) / sizeof(names[0]));
    return names[index];
}


/*static*/
const char *
FrenchVerbDictionary::getTenseName(Tense t)
{
    if (int(t) < int(PRESENT_TENSE) || int(t) > int(PAST_PERFECT))
    {
        assert(!"FrenchVerbDictionary::getTenseName() received invalid Tense value");
        return NULL;
    }

    static const char *names[] =
    {
        "present", "past", "imperfect", "future",
        "active", "passive", "active", "passive", "past perfect",
    };

    size_t index = size_t(t) - 1;
    assert(index < sizeof(names) / sizeof(names[0]));
    return names[index];
}


wstring
FrenchVerbDictionary::tolowerWide(const wstring &wideString) const
{
    wstring result;
    for (wstring::size_type len = wideString.length(), i = 0; i < len; i++)
    {
        wchar_t c = wideString[i];
        if (c <= 0xFF)
            result += (unsigned char) latin1TolowerTable[(unsigned char) c];
        else
            result += c;
    }
    return result;
}


//static
bool
FrenchVerbDictionary::isWideVowel(wchar_t c)
{
    if (strchr("aeiouyAEIOUY", (unsigned char) c) != NULL)
        return true;
    if (c < 0xc0 || c > 0xff)
        return false;
    return c != 0xc7 && c != 0xd0
        && c != 0xd1 && c != 0xd7 && c != 0xde
        && c != 0xe7
        && c != 0xf0 && c != 0xf1 && c != 0xf7 && c != 0xfe;
}


wstring
FrenchVerbDictionary::utf8ToWide(const string &utf8String) const throw(int)
{
    size_t inbytesleft = utf8String.length() + 1;  // number of *bytes* in UTF-8 string
    size_t outbytesleft = inbytesleft * sizeof(wchar_t);  // oversized for safety
    char *inbuf = strcpy(new char[inbytesleft], utf8String.c_str());
    char *outbuf = new char[outbytesleft];

    ICONV_CONST char *in = inbuf;
    char *out = outbuf;
    size_t initNumOutBytes = outbytesleft;
    if (iconv(utf8ToWideConv, &in, &inbytesleft, &out, &outbytesleft) == (size_t) -1)
    {
        int e = errno;
        delete [] inbuf;
        delete [] outbuf;
        throw e;
    }

    // iconv() has substracted the number of bytes produced
    // from outbytesleft. This allows the computation of the
    // number of wide characters in the result (excluding the
    // terminating null character).
    // See the iconv(3) man page for details.
    //
    const wchar_t *resultPtr = reinterpret_cast<wchar_t *>(outbuf);
    size_t resultLen = (initNumOutBytes - outbytesleft) / sizeof(wchar_t) - 1;
    assert(resultPtr[resultLen] == 0);

    wstring result(resultPtr, resultLen);
    assert(result.length() == resultLen);

    delete [] inbuf;
    delete [] outbuf;
    return result;
}


string
FrenchVerbDictionary::wideToUTF8(const wstring &wideString) const throw(int)
{
    size_t inbytesleft = (wideString.length() + 1) * sizeof(wchar_t);
    size_t outbytesleft = inbytesleft;  // UTF-8 string takes no more room than wstring
    char *inbuf = reinterpret_cast<char *>(memcpy(new char[inbytesleft], wideString.data(), inbytesleft));
    char *outbuf = new char[outbytesleft];

    ICONV_CONST char *in = inbuf;
    char *out = outbuf;
    if (iconv(wideToUTF8Conv, &in, &inbytesleft, &out, &outbytesleft) == (size_t) -1)
    {
        int e = errno;
        delete [] inbuf;
        delete [] outbuf;
        throw e;
    }

    string result = outbuf;
    delete [] inbuf;
    delete [] outbuf;
    return result;
}


/*static*/
string
FrenchVerbDictionary::getRadical(
                        const string &infinitive,
                        const string &templateName) throw(logic_error)
{
    string::size_type posColon = templateName.find(':');
    if (posColon == string::npos)
        throw logic_error("no colon found in template name");

    string::size_type lenSuffix = templateName.length() - posColon - 1;
    string::size_type lenInfPrefix = infinitive.length() - lenSuffix;
    return string(infinitive, 0, lenInfPrefix);
}


bool
FrenchVerbDictionary::generateTense(const string &radical,
                                const TemplateSpec &templ,
                                Mode mode,
                                Tense tense,
                                vector< vector<string> > &dest,
                                bool includePronouns,
                                bool aspirateH,
                                bool isItalian) const throw()
{
    if (templ.find(mode) == templ.end())
        return false;

    const ModeSpec &modeSpec = templ.find(mode)->second;

    if (modeSpec.find(tense) == modeSpec.end())
        return false;

    const TenseSpec &tenseSpec = modeSpec.find(tense)->second;

    if (mode != INDICATIVE_MODE
            && mode != CONDITIONAL_MODE
            && mode != SUBJUNCTIVE_MODE)
        includePronouns = false;

    for (TenseSpec::const_iterator p = tenseSpec.begin();
                                    p != tenseSpec.end(); p++)
    {
        dest.push_back(vector<string>());
        for (PersonSpec::const_iterator i = p->begin(); i != p->end(); i++)
        {
            // Do not return spellings that are marked incorrect.
            // They are in the knowledge base only to allow
            // error-tolerant searches.
            //
            if (!(*i).isCorrect)
                continue;

            string pronoun;  // no pronoun by default

            string v = radical + (*i).inflection;

            if (includePronouns)
            {
                size_t noPers = p - tenseSpec.begin();
                switch (noPers)
                {
                case 0:
                    if (isItalian)
                        pronoun = "io ";
                    else
                    {
                        bool elideJe = false;
                        if (!aspirateH)
                        {
                            wstring wideV = utf8ToWide(v);  // inefficient: converts all chars, only 1st needed
                            wchar_t init = (wideV.empty() ? '\0' : wideV[0]);
                            if (init == 'h' || init == 'H' || isWideVowel(init))
                                elideJe = true;
                        }
                        pronoun = (elideJe ? "j'" : "je ");
                    }
                    break;
                case 1: pronoun = "tu "; break;
                case 2: pronoun = (isItalian ? "egli " : "il "); break;
                case 3: pronoun = (isItalian ? "noi "  : "nous "); break;
                case 4: pronoun = (isItalian ? "voi "  : "vous "); break;
                case 5: pronoun = (isItalian ? "essi " : "ils "); break;
                }

                if (mode == SUBJUNCTIVE_MODE)
                {
                    const char *conj;
                    if (isItalian)
                        conj = "che ";
                    else if (noPers == 2 || noPers == 5)
                        conj = "qu'";
                    else
                        conj = "que ";
                    pronoun = conj + pronoun;
                }
            }

            dest.back().push_back(pronoun + v);
        }
    }

    return true;
}


bool FrenchVerbDictionary::isVerbStartingWithAspirateH(
                                const std::string &infinitive) const throw()
{
    return aspirateHVerbs.find(infinitive) != aspirateHVerbs.end();
}
