/*  $Id: misc-types.h,v 1.13 2012/11/18 19:56:06 sarrazip Exp $
    misc-types.h - Miscellaneous types used by the dictionary class.

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

#ifndef _H_misc_types
#define _H_misc_types

#include <verbiste/c-api.h>

#include <assert.h>
#include <vector>
#include <string>
#include <map>
#include <set>


/**
    Valid modes.
*/
enum Mode
{
    INVALID_MODE = VERBISTE_INVALID_MODE,
    INFINITIVE_MODE = VERBISTE_INFINITIVE_MODE,
    INDICATIVE_MODE = VERBISTE_INDICATIVE_MODE,
    CONDITIONAL_MODE = VERBISTE_CONDITIONAL_MODE,
    SUBJUNCTIVE_MODE = VERBISTE_SUBJUNCTIVE_MODE,
    IMPERATIVE_MODE = VERBISTE_IMPERATIVE_MODE,
    PARTICIPLE_MODE = VERBISTE_PARTICIPLE_MODE,
    GERUND_MODE = VERBISTE_GERUND_MODE,

    // Greek modes:
    PRESENT_INDICATIVE = VERBISTE_PRESENT_INDICATIVE,
    PRESENT_SUBJUNCTIVE = VERBISTE_PRESENT_SUBJUNCTIVE,
    PRESENT_IMPERATIVE = VERBISTE_PRESENT_IMPERATIVE,
    PRESENT_GERUND = VERBISTE_PRESENT_GERUND,
    PAST_IMPERFECT_INDICATIVE = VERBISTE_PAST_IMPERFECT_INDICATIVE,
    PAST_PERFECT_INDICATIVE = VERBISTE_PAST_PERFECT_INDICATIVE,
    PAST_PERFECT_SUBJUNCTIVE = VERBISTE_PAST_PERFECT_SUBJUNCTIVE,
    PAST_PERFECT_IMPERATIVE = VERBISTE_PAST_PERFECT_IMPERATIVE,
    PAST_PERFECT_INFINITIVE = VERBISTE_PAST_PERFECT_INFINITIVE
};


/**
    Valid tenses.
*/
enum Tense
{
    INVALID_TENSE = VERBISTE_INVALID_TENSE,
    PRESENT_TENSE = VERBISTE_PRESENT_TENSE,
    PAST_TENSE = VERBISTE_PAST_TENSE,
    IMPERFECT_TENSE = VERBISTE_IMPERFECT_TENSE,
    FUTURE_TENSE = VERBISTE_FUTURE_TENSE,

    // Greek tenses:
    ACTIVE_TENSE = VERBISTE_ACTIVE_TENSE,
    PASSIVE_TENSE = VERBISTE_PASSIVE_TENSE,
    IMPERATIVE_ACTIVE_TENSE = VERBISTE_IMPERATIVE_ACTIVE_TENSE,
    IMPERATIVE_PASSIVE_TENSE = VERBISTE_IMPERATIVE_PASSIVE_TENSE,
    PAST_PERFECT = VERBISTE_PAST_PERFECT
};


/**
    Description of a verb inflection.
    Gives the mode, tense and person of a conjugated verb.
    The person is 1, 2 or 3 for je/nous, tu/vous, il/ils, except in
    infinitive and participle mode, where it is always 0
    since it does not apply.
*/
class ModeTensePersonNumber
{
public:

    /** Mode (infinitive, indicative, etc). */
    Mode mode;

    /** Tense (present, past, etc). */
    Tense tense;

    /** Person (1, 2 or 3, or 0 in infinitive mode).
        In participle mode, 1..4 means masc sing, masc plur,
        fem sing, fem plur.
    */
    unsigned char person;

    /** Number (true for plural, false for singular). */
    bool plural;

    /** Indicates if this termination is correct or an error. */
    bool correct;


    ModeTensePersonNumber(Mode m = INVALID_MODE,
                    Tense t = INVALID_TENSE,
                    unsigned char pers = 0,
                    bool plur = false,
                    bool isCorrect = true)
      : mode(m), tense(t), person(pers), plural(plur), correct(isCorrect)
    {
        assert(mode <= PARTICIPLE_MODE);
        assert(tense <= FUTURE_TENSE);
        assert(person <= 3);
    }


    /** Constructs an object from English mode and tense names.
        Calls the set() method.
        @param    modeName      mode name known to method
                                FrenchVerbDictionary::convertModeName()
        @param    tenseName     tense name known to method
                                FrenchVerbDictionary::convertTenseName()
        @param    personNum     person "counter" (1--6, except for imperative
                                mode, where it must be 1--3; ignored for
                                infinitive and participle modes)
        @param    isCorrect     indicates if this termination is correct
                                or an error
    */
    ModeTensePersonNumber(const char *modeName,
                    const char *tenseName,
                    int personNum,
                    bool isCorrect,
                    bool isItalian)
      : mode(INVALID_MODE), tense(INVALID_TENSE), person(0), plural(false), correct(false)
    {
        set(modeName, tenseName, personNum, isCorrect, isItalian);
    }


    /** Initializes an object from English mode and tense names.
        Call the set() method.
        @param    modeName      mode name known to method
                                FrenchVerbDictionary::convertModeName()
        @param    tenseName     tense name known to method
                                FrenchVerbDictionary::convertTenseName()
        @param    personNum     person "counter" (1--6, except for imperative
                                mode, where it must be 1--3; ignored for
                                infinitive mode); for participle mode, must be
                                1..4 (masc sing, masc plur, fem sing, fem plur)
        @param    isCorrect     indicates if this termination is correct
                                or an error
        @param    isItalian     true for Italian, false for French
    */
    void set(const char *modeName, const char *tenseName, int personNum, bool isCorrect, bool isItalian);


    /**
        Dumps the fields of this object into those of the destination object.
        @param    destination   object whose fields (mode, tense, person,
                                number) are assigned with values taken
                                from this object
    */
    void dump(Verbiste_ModeTensePersonNumber &destination) const;
};


struct InflectionSpec
{
    std::string inflection;
    bool isCorrect;

    InflectionSpec(const std::string &inf, bool c) : inflection(inf), isCorrect(c) {}
};


/** List of inflections. */
typedef std::vector<InflectionSpec> PersonSpec;


/** List of persons (1, 3 or 6 persons depending on the mode and tense). */
typedef std::vector<PersonSpec> TenseSpec;


/**
    Mode specification.
    Contains tense specifications indexed by Tense values.
*/
typedef std::map<Tense, TenseSpec> ModeSpec;


/**
    Conjugation template specification.
    Contains mode specifications indexed by Mode values.
*/
typedef std::map<Mode, ModeSpec> TemplateSpec;


/**
    Conjugation system for the known verbs of a language.
    Contains conjugation templates indexed by template names.
*/
typedef std::map<std::string, TemplateSpec> ConjugationSystem;


/**
    Table of template names indexed by verb infinitive.
    If the verb "abaisser" follows the "aim:er" conjugation template,
    then a VerbTable would contain an entry where the key is "abaisser"
    and the value is a set containing "aim:er".
    An infinitive can be associated with more than one template.
*/
typedef std::map< std::string, std::set<std::string> > VerbTable;


/**
    Table that describes the mode, tense and person of a number of inflections.
    For example, in the "aim:er" conjugation template, the inflection
    (termination) "e" is associated with:

    - the 1st person singular of the indicative present;

    - the 3rd person singular of the indicative present;

    - the 1st person singular of the subjunctive present;

    - the 3rd person singular of the subjunctive present;

    - the 2nd person singular of the imperative present.

    Thus, in this table, the key "e" would be be associated with a vector
    of five objects representing these five mode-tense-person combinations.
*/
typedef std::map<std::string, std::vector<ModeTensePersonNumber> >
                                            TemplateInflectionTable;


/**
    Table of template inflection tables, indexed by template names.
    For example, a key "aim:er" would be associated with a template
    inflection table that gives all inflections (terminations) that are
    accepted by the "aim:er" conjugation template (e.g., "e", "es",
    "ons", etc).
*/
typedef std::map<std::string, TemplateInflectionTable> InflectionTable;


/**
    Description of a conjugated verb's inflection.
*/
class InflectionDesc
{
public:

    /** Infinitive form of the conjugated verb (UTF-8). */
    std::string infinitive;

    /** Conjugated template used by the verb (e.g. "aim:er") (UTF-8). */
    std::string templateName;

    /** Mode, tense, person and number of the inflection. */
    ModeTensePersonNumber mtpn;

    /**
        Constructs an inflection description from optional arguments.
    */
    InflectionDesc(const std::string &inf = "",
                const std::string &tname = "",
                ModeTensePersonNumber m = ModeTensePersonNumber())
      : infinitive(inf),
        templateName(tname),
        mtpn(m)
    {
    }
};


#endif  /* _H_misc_types */
