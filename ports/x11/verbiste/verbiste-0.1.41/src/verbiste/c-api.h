/*  $Id: c-api.h,v 1.14 2012/11/18 19:56:06 sarrazip Exp $
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

#ifndef _H_c_api
#define _H_c_api

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    VERBISTE_INVALID_MODE,
    VERBISTE_INFINITIVE_MODE,
    VERBISTE_INDICATIVE_MODE,
    VERBISTE_CONDITIONAL_MODE,
    VERBISTE_SUBJUNCTIVE_MODE,
    VERBISTE_IMPERATIVE_MODE,
    VERBISTE_PARTICIPLE_MODE,
    VERBISTE_GERUND_MODE,

    // Greek modes:
    VERBISTE_PRESENT_INDICATIVE,
    VERBISTE_PRESENT_SUBJUNCTIVE,
    VERBISTE_PRESENT_IMPERATIVE,
    VERBISTE_PRESENT_GERUND,
    VERBISTE_PAST_IMPERFECT_INDICATIVE,
    VERBISTE_PAST_PERFECT_INDICATIVE,
    VERBISTE_PAST_PERFECT_SUBJUNCTIVE,
    VERBISTE_PAST_PERFECT_IMPERATIVE,
    VERBISTE_PAST_PERFECT_INFINITIVE

/** Valid conjugation modes. */
} Verbiste_Mode;

typedef enum
{
    VERBISTE_INVALID_TENSE,
    VERBISTE_PRESENT_TENSE,
    VERBISTE_PAST_TENSE,
    VERBISTE_IMPERFECT_TENSE,
    VERBISTE_FUTURE_TENSE,
    VERBISTE_ACTIVE_TENSE,
    VERBISTE_PASSIVE_TENSE,
    VERBISTE_IMPERATIVE_ACTIVE_TENSE,
    VERBISTE_IMPERATIVE_PASSIVE_TENSE,
    VERBISTE_PAST_PERFECT

/** Valid conjugation tenses. */
} Verbiste_Tense;

/** List of conjugation template names (e.g., "aim:er").
    The last character pointer in an array of this type is NULL.
*/
typedef char **Verbiste_TemplateArray;


typedef struct
{
  char *infinitive_verb;
  Verbiste_Mode   mode;
  Verbiste_Tense  tense;
  int person;  /* 1, 2, 3, or 0 for infinitive and participle modes */
  int plural;  /* boolean indicating plural number instead of singular */
  int correct;  /* boolean indicating if termination is correct or an error */

/** Mode descriptor. */
} Verbiste_ModeTensePersonNumber;


/** List of strings, each string being an inflection (e.g., "assis").
    The last character pointer in an array of this type is NULL.
*/
typedef char **Verbiste_InflectionArray;

/** List of list of strings, each sublist being an inflection array.
    The last Verbiste_InflectionArray pointer in an array of this type is NULL.
*/
typedef Verbiste_InflectionArray *Verbiste_PersonArray;


typedef struct
{
  Verbiste_Mode mode;
  Verbiste_Tense tense;
} Verbiste_ModeTense;


extern const Verbiste_ModeTense verbiste_valid_modes_and_tenses[];


/** Initializes the French verb dictionary object.
    This function must be called before any other function of this library.
    If the construction of the object fails (i.e., -2 is returned),
    call verbist_get_init_error() to obtain a text description of the failure.
    @param  conjugation_filename        filename of the XML document that
                                        defines all the conjugation templates
    @param  verbs_filename              filename of the XML document that
                                        defines all the known verbs and their
                                        corresponding template
    @param  lang_code                   "fr" for French, "it" for Italian
    @returns                            0 on success,
                                        -1 if the object has already been
                                        initialized, or
                                        -2 if the construction of the
                                        object fails.
*/
int verbiste_init(const char *conjugation_filename,
                  const char *verbs_filename,
                  const char *lang_code);


/** Gets a text description of the failure that occurred in verbiste_init().
    This function must only be called if verbiste_init() has returned -2.
    @returns                        the what() value of the logic_error expression
                                thrown by the FrenchVerbDictionary constructor.
*/
const char *verbiste_get_init_error();


/** Frees the resources associated with the French verb dictionary object.
    This function should be called when this library's functionalities
    are not needed anymore.
    After this function has been called successfully, verbiste_init()
    can be called again to reinitialize the dictionary object.
    It must not be called if verbiste_init() failed.
    @returns                        0 on success, or -1 if the object had not
                                been initialized (or had already been destroyed)
*/
int verbiste_close(void);


/** Frees the memory associated with the given string.
    The string to deallocate must have been received from a function
    of this API that specifically requires the deallocation to be
    done with verbiste_free_string().
    @param        str           string to deallocate
*/
void verbiste_free_string(char *str);


/** Returns the ASCII English name of the given mode.
    @param        mode          mode whose name is requested
    @returns                    pointer to an internal string that gives the
                                requested name
*/
const char *verbiste_get_mode_name(Verbiste_Mode mode);


/** Returns the ASCII English name of the given tense.
    @param        tense         tense whose name is requested
    @returns                    pointer to an internal string that gives the
                                requested name
*/
const char *verbiste_get_tense_name(Verbiste_Tense tense);


/** Analyses a conjugated verb and describes how it is conjugated.
    @param        verb          Latin-1 string containing the verb
                                to deconjugate
    @returns                    a dynamically allocated array
                                which must be freed by a call to
                                verbiste_free_mtpn_array();
                                the strings in this array are in Latin-1
*/
Verbiste_ModeTensePersonNumber *verbiste_deconjugate(const char *verb);


/** Frees the memory associated by the given array.
    @param        array         array to be freed;
                                must have been allocated by a function such as
                                verbiste_deconjugate();
                                nothing is done if 'array' is null
*/
void verbiste_free_mtpn_array(Verbiste_ModeTensePersonNumber *array);


/** Returns the list of conjugation templates that apply to the given infinitive.
    @param  infinitive_verb     Latin-1 string containing the infinitive
    @returns                    an array of strings, the last element begin
                                a null pointer; a NULL pointer is returned if
                                the infinitive is unknown; this array must be
                                passed to verbiste_free_verb_template_array()
                                to free the memory
*/
Verbiste_TemplateArray verbiste_get_verb_template_array(const char *infinitive_verb);


/** Frees the memory allocated by verbiste_get_verb_template_array().
    @param  array               an array returned by verbiste_get_verb_template_array();
                                can be NULL: this function does nothing in such a case
*/
void verbiste_free_verb_template_array(Verbiste_TemplateArray array);


/** Conjugates a verb in a certain mode and tense.
    @param  infinitiveVerb      Latin-1 infinitive form of the verb
                                to be conjugated
    @param  template_name       name of the conjugation template to use
                                (e.g., "aim:er")
    @param  mode                selected mode
    @param  tense               selected tense
    @param  include_pronouns    if non-zero, put pronouns before
                                conjugated verbs in the modes where
                                pronouns are used
    @returns                    a dynamically allocated array
                                which must be freed by a call to
                                verbiste_free_person_array();
                                the strings in this array
                                are in Latin-1;
                                returns NULL if an error occurs
*/
Verbiste_PersonArray verbiste_conjugate(const char *infinitive_verb,
                                        const char *template_name,
                                        const Verbiste_Mode mode,
                                        const Verbiste_Tense tense,
                                        int include_pronouns);


/** Frees the memory associated by the given person array.
    @param        array         array to be freed;
                                must have been allocated by a function such as
                                verbiste_conjugate();
                                nothing is done if 'array' is null
*/
void verbiste_free_person_array(Verbiste_PersonArray array);


#ifdef __cplusplus
}
#endif

#endif  /* _H_c_api */
