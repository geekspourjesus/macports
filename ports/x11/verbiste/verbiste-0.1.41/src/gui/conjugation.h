/*  $Id: conjugation.h,v 1.13 2012/11/18 19:56:05 sarrazip Exp $
    conjugation.h - Generic conjugation interface

    verbiste - French conjugation system
    Copyright (C) 2003-2006 Pierre Sarrazin <http://sarrazip.com/>

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

#ifndef _H_conjugation
#define _H_conjugation

#include <verbiste/FrenchVerbDictionary.h>

#include <vector>
#include <string>

typedef std::vector<std::string> VS;
typedef std::vector<VS> VVS;
typedef std::vector<VVS> VVVS;



/** Obtains the conjugation of the given infinitive.
    @param  fvd             verb dictionary from which to obtain
                            the conjugation
    @param  infinitive      UTF-8 string containing the infinitive form
                            of the verb to conjugate (e.g., "manger")
    @param  tname           conjugation template name to use (e.g., "aim:er")
    @param  dest            structure into which the conjugated is written
    @param  includePronouns put pronouns before conjugated verbs in the
                            modes where pronouns are used
*/
void getConjugation(const verbiste::FrenchVerbDictionary &fvd,
                        const std::string &infinitive,
                        const std::string &tname,
                        VVVS &dest,
                        bool includePronouns = false);


/** Get the tense name for a certain cell of the conjugation table.
    The conjugation table is a 4x4 grid and 11 of the 16 cells are
    used by the tenses to be displayed.
    @param      row             row number (0..3)
    @param      col             column number (0..3)
    @param      isItalian       language used (true for Italian, false for French)
    @returns            an internationalized (and possibly abbreviated) name
                        (in UTF-8) for the tense that goes in the cell
                        designated by row and col;
                        the returned string is empty if no tense (in the
                        language used) goes in the designated cell
*/
std::string getTenseNameForTableCell(int row, int col, bool isItalian);


/** Composes the text of the conjugation in a certain tense.
    The words of this conjugation that match a certain user text
    are marked specially.

    @param        tense                 structure that represents a certain
                                        tense of a certain verb
    @param        lowerCaseUTF8UserText conjugated verb as entered by the
                                        user, but converted to lower-case,
                                        and assumed to be in UTF-8
    @param        openMark              ASCII string to be used before verbs
                                        that match lowerCaseUTF8UserText
                                        (e.g., "<b>")
    @param        closeMark             ASCII string to be used after verbs
                                        that match lowerCaseUTF8UserText
                                        (e.g., "</b>")
    @returns                            a UTF-8 string containing several
                                        lines of text, each line being
                                        separated by a newline ('\n')
                                        character; the string does not end
                                        the last line with a newline however;
                                        e.g., "aie\nayons\nayez"
*/
std::string createTableCellText(verbiste::FrenchVerbDictionary &fvd,
                                const VVS &tense,
                                const std::string &lowerCaseUTF8UserText,
                                const std::string &openMark,
                                const std::string &closeMark);


#endif  /* _H_conjugation */
