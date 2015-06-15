/*  $Id: Trie.cpp,v 1.16 2012/11/18 19:56:06 sarrazip Exp $
    Trie.cpp - Tree structure for string storage

    verbiste - French conjugation system
    Copyright (C) 2003-2005 Pierre Sarrazin <http://sarrazip.com/>

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

#include "Trie.h"

#include <assert.h>
#include <stdlib.h>
#include <list>
#include <iostream>


namespace verbiste {


///////////////////////////////////////////////////////////////////////////////
//
// Descriptor local class
//
//


template <class T>
Trie<T>::Descriptor::Descriptor(wchar_t u)
  : unichar(u),
    inferiorRow(NULL),
    userData(NULL)
{
}


template <class T>
Trie<T>::Descriptor::~Descriptor()
{
    assert(inferiorRow == NULL);
}


template <class T>
void
Trie<T>::Descriptor::recursiveDelete(bool deleteUserData)
{
    if (deleteUserData)
    {
        delete userData;
        userData = NULL;
    }
    if (inferiorRow != NULL)
    {
        inferiorRow->recursiveDelete(deleteUserData);
        delete inferiorRow;
        inferiorRow = NULL;
    }
}


template <class T>
size_t
Trie<T>::Descriptor::computeMemoryConsumption() const
{
    return sizeof(*this) + (inferiorRow != NULL ? inferiorRow->computeMemoryConsumption() : 0);
}


///////////////////////////////////////////////////////////////////////////////
//
// Row local class
//
//


template <class T>
Trie<T>::Row::~Row()
{
    assert(elements.size() == 0);  // recursiveDelete() should have been called
}


template <class T>
size_t
Trie<T>::Row::computeMemoryConsumption() const
{
    size_t sum = 0;
    for (typename DescVec::const_iterator it = elements.begin(); it != elements.end(); ++it)
        sum += (*it)->computeMemoryConsumption();
    return sizeof(*this) + sum;
}


template <class T>
void
Trie<T>::Row::recursiveDelete(bool deleteUserData)
{
    for (typename DescVec::iterator it = elements.begin(); it != elements.end(); it++)
    {
        Descriptor *d = *it;
        d->recursiveDelete(deleteUserData);
        delete d;
    }

    elements.clear();
}


template <class T>
typename Trie<T>::Descriptor *
Trie<T>::Row::find(wchar_t unichar)
{
    for (typename DescVec::iterator it = elements.begin(); it != elements.end(); it++)
    {
        assert(*it != NULL);
        if ((*it)->unichar == unichar)
            return *it;
    }

    return NULL;
}


template <class T>
typename Trie<T>::Descriptor &
Trie<T>::Row::operator [] (wchar_t unichar)
{
    Descriptor *pd = find(unichar);
    if (pd != NULL)
        return *pd;

    pd = new Descriptor(unichar);
    elements.push_back(pd);
    assert(pd->unichar == unichar);
    return *pd;
}


///////////////////////////////////////////////////////////////////////////////
//
// Trie class
//
//

template <class T>
Trie<T>::Trie(bool _userDataFromNew)
  : emptyKeyUserData(NULL),
    firstRow(new Row()),
    userDataFromNew(_userDataFromNew)
{
}


template <class T>
Trie<T>::~Trie()
{
    if (userDataFromNew)
        delete emptyKeyUserData;

    firstRow->recursiveDelete(userDataFromNew);
    delete firstRow;
}


template <class T>
T *
Trie<T>::get(const std::wstring &key) const
{
    if (emptyKeyUserData != NULL)
        onFoundPrefixWithUserData(key, 0, emptyKeyUserData);

    if (key.empty())
        return emptyKeyUserData;

    Descriptor *d = const_cast<Trie<T> *>(this)->getDesc(firstRow, key, 0, false, true);
    return (d != NULL ? d->userData : NULL);
}


template <class T>
T **
Trie<T>::getUserDataPointer(const std::wstring &key)
{
    if (key.empty())
        return &emptyKeyUserData;

    // Get descriptor associated with 'key' (and create a new entry
    // if the key is not known).
    //
    Descriptor *d = getDesc(firstRow, key, 0, true, false);
    assert(d != NULL);
    return &d->userData;
}


template <class T>
typename Trie<T>::Descriptor *
Trie<T>::getDesc(Row *row,
                const std::wstring &key,
                std::wstring::size_type index,
                bool create,
                bool callFoundPrefixCallback)
{
    assert(row != NULL);
    assert(index < key.length());

    wchar_t unichar = key[index];  // the "expected" character
    assert(unichar != '\0');

    Descriptor *pd = row->find(unichar);

    static bool trieTrace = getenv("TRACE") != NULL;
    if (trieTrace)
        std::wcout << "getDesc(row=" << row
                   << ", key='" << key << "' (len=" << key.length()
                   << "), index=" << index
                   << ", create=" << create
                   << ", call=" << callFoundPrefixCallback
                   << "): unichar=" << unichar << ", pd=" << pd << "\n";

    if (pd == NULL)  // if expected character not found
    {
        if (!create)
            return NULL;

        Descriptor &newDesc = (*row)[unichar];
        assert(row->find(unichar) != NULL);
        assert(row->find(unichar) == &newDesc);

        if (index + 1 == key.length())  // if last char of string
            return &newDesc;

        // Create new descriptor that points to a new inferior row:
        newDesc.inferiorRow = new Row();
        assert(row->find(unichar)->inferiorRow == newDesc.inferiorRow);

        return getDesc(newDesc.inferiorRow,
                        key, index + 1, create, callFoundPrefixCallback);
    }

    if (trieTrace)
        std::wcout << "getDesc: userData=" << pd->userData
                   << ", inferiorRow=" << pd->inferiorRow
                   << "\n";

    if (callFoundPrefixCallback && pd->userData != NULL)
        onFoundPrefixWithUserData(key, index + 1, pd->userData);  // virtual call

    if (index + 1 == key.length())  // if reached end of key
    {
        if (trieTrace)
            std::wcout << "getDesc: reached end of key\n";
        return pd;
    }

    if (pd->inferiorRow == NULL)  // if pd is a leaf:
    {
        if (!create)
            return NULL;  // not found

        pd->inferiorRow = new Row();
    }

    return getDesc(pd->inferiorRow,
                        key, index + 1, create, callFoundPrefixCallback);
}


template <class T>
size_t
Trie<T>::computeMemoryConsumption() const
{
    return sizeof(*this) + (firstRow != NULL ? firstRow->computeMemoryConsumption() : 0);
}


}  // namespace verbiste
