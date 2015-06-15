/*  $Id: FrenchVerbDictionary.h,v 1.45 2012/11/18 19:56:06 sarrazip Exp $
    FrenchVerbDictionary.h - Dictionary of verbs and conjugation templates

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

#ifndef _H_FrenchVerbDictionary
#define _H_FrenchVerbDictionary

#include <verbiste/c-api.h>
#include <verbiste/misc-types.h>
#include <verbiste/Trie.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <iconv.h>

#include <assert.h>
#include <stdexcept>
#include <vector>
#include <set>
#include <string>


/** C++ namespace in which all of this library's names are defined. */
namespace verbiste {


/** French verbs and conjugation knowledge base.
    The text processing done by this class is case-sensitive.
*/
class FrenchVerbDictionary
{
public:

    enum Language { NO_LANGUAGE, FRENCH, ITALIAN, GREEK };

    /** Returns the language identifier recognized in the given string.
        @param  twoLetterCode           string containing a language code
        @returns                        a member of the 'Language' enum,
                                        or NO_LANGUAGE if the string was
                                        not recognized
    */
    static Language parseLanguageCode(const std::string &twoLetterCode);

    /** Returns the two-letter code that names the given language identifier.
        @param  l               valid language identifier
        @returns                empty string if 'l' is NO_LANGUAGE or invalid,
                                or a twi-letter string otherwise
    */
    static std::string getLanguageCode(Language l);

    /** Returns the full path names of the conjugation and verb XML files
        for the given language code.
        @param  conjFN          receives the full path of the conjugation template XML file
        @param  verbsFN         receives the full path of the verb list XML file
        @param  languageCode    "fr" for French or "it" for Italian
    */
    static void getXMLFilenames(std::string &conjFN, std::string &verbsFN,
                                Language l);

    /** Load a conjugation database.
        @param    conjugationFilename   filename of the XML document that
                                        defines all the conjugation templates
        @param    verbsFilename         filename of the XML document that
                                        defines all the known verbs and their
                                        corresponding template
        @param    includeWithoutAccents fill knowledge base with variants of
                                        verbs where some or all accents are missing
        @param    lang                  language of the dictionary
        @throws   logic_error           for invalid arguments,
                                        unparseable or unexpected XML documents
    */
    FrenchVerbDictionary(const std::string &conjugationFilename,
                        const std::string &verbsFilename,
                        bool includeWithoutAccents,
                        Language lang)
                                        throw (std::logic_error);

    /** Load the French conjugation database.
        Uses the default (hard-coded) location for the French dictionary's
        data filenames.
        @param    includeWithoutAccents include in the knowledge base variants
                                        verbs where some or all accents are missing
        @throws   logic_error           for invalid filename arguments,
                                        unparseable or unexpected XML documents
                                        (if verbs or template names are
                                        mentioned, they are in Latin-1)
    */
    FrenchVerbDictionary(bool includeWithoutAccents) throw (std::logic_error);

    /** Frees the memory used by this dictionary.
    */
    ~FrenchVerbDictionary();

    /** Returns a conjugation template specification from its name.
        @param    templateName  name of the desired template (e.g. "aim:er")
        @returns                a pointer to the TemplateSpec object if found,
                                or NULL otherwise
    */
    const TemplateSpec *getTemplate(const std::string &templateName) const;

    /** Returns an iterator for the list of conjugation templates. */
    ConjugationSystem::const_iterator beginConjugSys() const;

    /** Returns an iterator for the end of the list of conjugation templates. */
    ConjugationSystem::const_iterator endConjugSys() const;

    /** Returns the set of templates used by a verb.
        @param    infinitive    infinitive form of the verb in Latin-1
                                (e.g., "manger", not "mange")
        @returns                a set of template names of the form radical:termination
                                in Latin-1 (e.g., "aim:er"),
                                or an empty set if the verb is unknown
                                or if 'infinitive' is NULL
    */
    const std::set<std::string> &getVerbTemplateSet(const char *infinitive) const;

    /** Returns the template used by a verb.
        @param    infinitive    infinitive form of the verb in Latin-1
                                (e.g., "manger", not "mange")
        @returns                a set of template names of the form radical:termination
                                in Latin-1 (e.g., "aim:er"),
                                or an empty set if the verb is unknown
                                or NULL if the verb is unknown
    */
    const std::set<std::string> &getVerbTemplateSet(const std::string &infinitive) const;

    /** Returns an iterator for the list of known verbs. */
    VerbTable::const_iterator beginKnownVerbs() const;

    /** Returns an iterator for the end of the list of known verbs. */
    VerbTable::const_iterator endKnownVerbs() const;


    /** Describes an inflection according to a given conjugation template.
        If the given inflection is known to the given conjugation template,
        the list of possible modes, tenses and persons is returned.
        For example, the inflection "es" in the "aim:er" template
        can be the 2nd person singular of the indicative present
        ("tu aimes")
        or the 2nd person singular of the subjunctive present.
        ("que tu aimes").
        Here, two ModeTensePersonNumber objects would be in the returned vector.

        @param    templateName  name of the conjugation template to use
                                (e.g., "aim:er")
        @param    inflection    inflection to be described
                                (e.g., "erions")
        @returns                a pointer to a vector of ModeTensePersonNumber
                                objects (which must not be modified nor
                                destroyed), or NULL if the inflection was not
                                known to the template
    */
    const std::vector<ModeTensePersonNumber> *getMTPNForInflection(
                                        const std::string &templateName,
                                        const std::string &inflection) const;

    /** Converts an English mode name into the corresponding enumerated type.
        @param        modeName        English mode name (infinitive, indicative, etc)
        @returns                a member of the Mode enumeration
                                (INVALID_MODE if 'modeName' is not known)
    */
    static Mode convertModeName(const char *modeName);

    /** Converts an English tense name into the corresponding enumerated type.
        @param        tenseName        English tense name (present, past, etc)
        @returns                a member of the Tense enumeration
                                (INVALID_MODE if 'modeName' is not known)
    */
    static Tense convertTenseName(const char *tenseName);

    /** Analyzes a conjugated verb and finds all known possible cases.
        @param   utf8ConjugatedVerb     conjugated French verb in UTF-8
                                        (e.g., "aimerions")
        @param   results        vector in which to store the inflection
                                descriptions (this vector is not emptied
                                before elements are stored in it);
                                no elements are stored in this vector
                                if the given conjugated verb is unknown
    */
    void deconjugate(const std::string &utf8ConjugatedVerb,
                            std::vector<InflectionDesc> &results);

    /** Returns the English name (in ASCII) of the given mode.
    */
    static const char *getModeName(Mode m);

    /** Returns the English name (in ASCII) of the given tense.
    */
    static const char *getTenseName(Tense t);

    /** Converts the Latin-1 characters of a wide character string to lower-case.
        @param    wideString    Unicode character string to be converted
        @returns                lower-case version of the character string
    */
    std::wstring tolowerWide(const std::wstring &wideString) const;

    /** Determines if a Unicode character is lower-case.
        Only works on Latin-1 characters.
        @param      c           Unicode character code
        @returns                true iff 'c' is a Latin-1 vowel.
    */
    static bool isWideVowel(wchar_t c);

    /** Converts a UTF-8 string to a wide character string.
        @param      utf8String  UTF-8 string to be converted
        @returns                Unicode string
    */
    std::wstring utf8ToWide(const std::string &utf8String) const throw(int);

    /** Converts a wide character string to a UTF-8 string.
        @param      wideString  Unicode string to be converted
        @returns                UTF-8 string
    */
    std::string wideToUTF8(const std::wstring &wideString) const throw(int);

    /** Removes accents from accented letters in the given string.
        @param   utf8String     UTF-8 string with accented characters
        @returns                a UTF-8 string with the accents removed
    */
    std::string removeUTF8Accents(const std::string &utf8String);

    /** Returns all unaccented variants of a wide character string.
        If N letters are accented in 'utf8String', then 2^N variants
        are returned, where each accented letter either keeps or loses
        its accents.
        For example, the word "été" has 3 unaccented variants:
        "eté", "ete" and "éte".
        @param    wideString    wide character string with accented characters
        @param    index         pass zero (recursive calls to this function
                                pass non-zero indices)
        @param    utf8Variants  vector to which UTF-8 strings are added
                                with push_back (this function does not
                                clear the vector beforehand)
    */
    void formUTF8UnaccentedVariants(const std::wstring &wideString,
                                        size_t index,
                                        std::vector<std::string> &utf8Variants);

    /** Returns all unaccented variants of a UTF-8 string.
        If N letters are accented in 'utf8String', then 2^N variants
        are returned, where each accented letter either keeps or loses
        its accents.
        For example, the word "été" has 3 unaccented variants:
        "eté", "ete" and "éte".
        @param    utf8String    UTF-8 string with accented characters
        @param    index         pass zero (recursive calls to this function
                                pass non-zero indices)
        @param    utf8Variants  vector to which UTF-8 strings are added
                                with push_back (this function does not
                                clear the vector beforehand)
    */
    void formUTF8UnaccentedVariants(const std::string &utf8String,
                                        size_t index,
                                        std::vector<std::string> &utf8Variants);


    /** Returns the content of an XML node in UTF-8.
        @param    doc           the XML document
        @param    node          the node of the XML document whose contents
                                are to be extracted
        @returns                a Latin-1 string representing the contents
                                of the node; this string is empty the
                                requested node does not exist
        @throws    int          errno value set by iconv(3), in the case of a
                                UTF-8 to Latin-1 conversion error
    */
    std::string getUTF8XmlNodeText(
                        xmlDocPtr doc, xmlNodePtr node) throw(int);

    /** Returns the content of an XML property in UTF-8.
        For example, if 'node' represents <foo type='xyz'/>,
        then passing "type" for 'propName' will return "xyz".
        @param    node          the node of the XML document
        @param    propName      the name of the property to extract
        @returns                a Latin-1 string representing the contents
                                of the property; this string is empty the
                                requested property does not exist
        @throws   int           errno value set by iconv(3), in the case of a
                                UTF-8 to Latin-1 conversion error
    */
    std::string getUTF8XmlProp(
                        xmlNodePtr node, const char *propName) throw(int);


    /** Gets the radical part of an infinitive, according to a template name.
        @param    infinitive    infinitive whose radical is requested
        @param    templateName  name of the conjugation template that applies
        @returns                a prefix of 'infinitive'
        @throws   logic_error   the template name is invalid (no ':' found)
    */
    static std::string getRadical(
                    const std::string &infinitive,
                    const std::string &templateName) throw(std::logic_error);

    /** Generates the conjugation of a verb for a given mode and tense.
        The generated words are complete, they are not just inflections.
        @param    radical       radical part of the verb to conjugate
        @param    templ         conjugation template to apply
        @param    mode          mode to use
        @param    tense         tense to use
        @param    dest          vector of vectors of strings into which to
                                store the results; the result is a list of
                                "persons", and a person is a list of
                                "inflections"
        @param    includePronouns put pronouns before conjugated verbs in the
                                modes where pronouns are used
        @param    aspirateH     notifies this function that the verb starts
                                with an aspirate h (e.g., "hacher", which
                                gives "je hache") instead of a silent h
                                (e.g., "habiter", which gives "j'habite")
        @param    isItalian     language used (true for Italian, false for French)
        @returns                true for success, or false if the mode or
                                tense is unknown.
    */
    bool generateTense(const std::string &radical,
                        const TemplateSpec &templ,
                        Mode mode,
                        Tense tense,
                        std::vector< std::vector<std::string> > &dest,
                        bool includePronouns,
                        bool aspirateH,
                        bool isItalian) const throw();

    /** Indicates if the given verb starts with an aspirate h.
        An aspirate h means that one cannot make a contraction or liaison
        in front of the word.  For example, "hacher" has an aspirate h
        and this means that one says "je hache" and not "j'hache".
        The verb "habiter" however does not have an aspirate h, so one
        says "j'habite" and not "je habite".
    */
    bool isVerbStartingWithAspirateH(
                                const std::string &infinitive) const throw();

    /** Returns the code representing this dictionary's language.
    */
    Language getLanguage() const { return lang; }

private:

    // User data employed in the Verb Trie.
    // Remembers the correct spelling of the verb, in case the user
    // reached a trie entry through tolerance of missing accents.
    // This way, if the user enters "etaler", the displayed conjugation
    // will show the missing acute accent on the first "e".
    //
    class TrieValue
    {
    public:
        TrieValue(const std::string &t, const std::string &r)
        :   templateName(t), correctVerbRadical(r) {}

        std::string templateName;
        std::string correctVerbRadical;
    };

    /** Trie that contains all known verb radicals.
        The associated information is a list of template names
        that can apply to the radical.
        The verb radicals and the template names are stored in Latin-1.
    */
    class VerbTrie : public Trie< std::vector<TrieValue> >
    {
    public:
        const FrenchVerbDictionary &fvd;
        std::vector<InflectionDesc> *results;

        /** Constructs a trie that keeps a reference to the dictionary.
            @param        d        reference to the verb dictionary
        */
        VerbTrie(const FrenchVerbDictionary &d)
          : Trie< std::vector<TrieValue> >(true),
            fvd(d),
            results(NULL)
        {
        }

        /** Callback invoked by the Trie<>::get() method.
            Inherited from Trie<>.
            This callback will be called for each prefix of the searched
            string that corresponds to the radical of a known verb.
            Stores data in the vector<InflectionDesc> designated by
            the last call to setDestination().
            @param        conjugatedVerb    the searched string
            @param        index             length of the prefix
            @param        templateList      list of conjugation templates that
                                            might apply to the conjugated verb
        */
        virtual void onFoundPrefixWithUserData(
                        const std::wstring &conjugatedVerb,
                        std::wstring::size_type index,
                        const std::vector<TrieValue> *templateList) const
                                                                throw();

        /** Sets the destination vector in which callback() stores results.
            When the Trie<>::get() method is called on this object,
            it may invoke the callback() virtual method.
            callback() will store any results in the vector designated here.
            After calling get(), iterate through the vector to obtain
            the possible inflections of the conjugated verb.
            @param        d        destination vector designated as the
                                   repository for results (may be NULL)
        */
        void setDestination(std::vector<InflectionDesc> *d)
        {
            results = d;
        }

    private:
        // Forbidden operations:
        VerbTrie(const VerbTrie &);
        VerbTrie &operator = (const VerbTrie &);
    };

    friend class VerbTrie;

private:

    ConjugationSystem conjugSys;
    VerbTable knownVerbs;
    std::set<std::string> aspirateHVerbs;
    InflectionTable inflectionTable;
    iconv_t wideToUTF8Conv;
    iconv_t utf8ToWideConv;
    char latin1TolowerTable[256];
    VerbTrie verbTrie;
    Language lang;

private:

    void init(const std::string &conjugationFilename,
                        const std::string &verbsFilename,
                        bool includeWithoutAccents)
                                        throw (std::logic_error);
    void loadConjugationDatabase(const char *conjugationFilename,
                                bool includeWithoutAccents)
                                        throw (std::logic_error);
    void loadVerbDatabase(const char *verbsFilename,
                        bool includeWithoutAccents)
                                        throw (std::logic_error);
    void readConjugation(xmlDocPtr doc,
                        bool includeWithoutAccents) throw(std::logic_error);
    static void generateOtherPastParticiple(const char *mascSing,
                                        std::vector<std::string> &dest);
    void readVerbs(xmlDocPtr doc,
                   bool includeWithoutAccents)
                                throw(std::logic_error);
    void insertVerbRadicalInTrie(const std::string &verbRadical,
                                    const std::string &tname,
                                    const std::string &correctVerbRadical);

    // Forbidden operations:
    FrenchVerbDictionary(const FrenchVerbDictionary &x);
    FrenchVerbDictionary &operator = (const FrenchVerbDictionary &x);
};


}  // namespace verbiste


#endif  /* _H_FrenchVerbDictionary */
