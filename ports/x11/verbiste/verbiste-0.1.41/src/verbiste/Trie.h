/*  $Id: Trie.h,v 1.20 2012/11/18 19:56:06 sarrazip Exp $
    Trie.h - Tree structure for string storage

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

#ifndef _H_Trie
#define _H_Trie

#include <string>
#include <vector>


namespace verbiste {


/** Tree structure for (wide character) string storage.
    @param        T     type of the user data attached to the stored strings;
                        pointers to objects of type T will be stored in the
                        trie, but no T object will be created, copied,
                        assigned or destroyed by the trie.
*/
template <class T>
class Trie
{
public:

    /** Constructs an empty trie.
        @param        userDataFromNew   determines if the destructor
                                        must assume that all "user data"
                                        pointers come from new and must
                                        thus be destroyed with delete
    */
    Trie(bool userDataFromNew);


    /** Destroys the trie and its contents.
    */
    virtual ~Trie();


    /** Searches the trie with the given (wide character) key.
        Invokes the virtual function onFoundPrefixWithUserData()
        for each find.
        @param  key         wide character string to search for
        @returns            a pointer to the user data pointer
                            associated with 'key', or NULL if
                            nothing was found
    */
    T *get(const std::wstring &key) const;


    /** Obtains the address of the user data associated with 'key'
        and adds an entry if necessary.
        @returns        a non-null pointer to the user data pointer
                        associated with 'key';
                        if a new entry was created, the T * is null.
    */
    T **getUserDataPointer(const std::wstring &key);


    /** Callback invoked by the Trie<>::get() method.
        This callback will be called for each prefix of the searched
        string for which the trie has some user data.
        This method does nothing if it is not overridden in a derived class.
        @param  key         the searched string
        @param  index       length of the prefix
        @param  userData    user data that is associated with the prefix
    */
    virtual void onFoundPrefixWithUserData(const std::wstring &/*key*/,
                                        std::wstring::size_type /*index*/,
                                        const T * /*userData*/) const
                                                        throw()
    {
    }

    /** Computes and returns the number of memory bytes consumed by
        this object, excluding the size of the user data instances.
        @returns                        number of bytes
    */
    size_t computeMemoryConsumption() const;

private:

    class Row;

    /** Associates user data with a character, and points to the row
        below that character in the trie.
    */
    class Descriptor
    {
    public:
        Descriptor(wchar_t u);

        /** Does nothing: recursiveDelete() must have been called before.
        */
        ~Descriptor();

        /** Calls recursiveDelete() on *inferiorRow if inferiorRow is not NULL.
            Then, calls operator delete that row and sets inferiorRow to NULL.
            @param        deleteUserData        if true, operator delete is called
                                            on userData (which may be NULL)
        */
        void recursiveDelete(bool deleteUserData);

        /** Computes and returns the number of memory bytes consumed by
            this object, excluding the size of the user data.
            @returns                        number of bytes
        */
        size_t computeMemoryConsumption() const;

    public:
        wchar_t unichar;  // Unicode character code
        Row *inferiorRow;
        T *userData;

    private:
        // Forbidden operations:
        Descriptor(const Descriptor &d);
        Descriptor &operator = (const Descriptor &d);
    };


    typedef std::vector<Descriptor *> DescVec;


    /** Sequence of Descriptor objects. A trie is a tree of Row objects.
    */
    class Row
    {
    public:
        Row()
          : elements()
        {
        }

        /** Does nothing: recursiveDelete() should have been called before.
        */
        ~Row();

        /** Calls recursiveDelete() on each Descriptor in this row.
            Then empties this row.
            @param        deleteUserData        if true, operator delete is called
                                            on the userData field of the
                                        Descriptor objects
        */
        void recursiveDelete(bool deleteUserData);


        /** Finds an element of this row whose (wide) character field is
            equal to 'unichar'.
            Returns NULL if no such element exists.
        */
        Descriptor *find(wchar_t unichar);

        /** Finds or creates an element of this row whose char. field is 'unichar'.
            If no such element exists, one is created using the
            default constructor of the Descriptor class.
        */
        Descriptor &operator [] (wchar_t unichar);

        /** Computes and returns the number of memory bytes consumed by
            this object, excluding the size of the Descriptors' user data.
            @returns                        number of bytes
        */
        size_t computeMemoryConsumption() const;

    private:
        DescVec elements;  // members allocated with operator new
    };


    Descriptor *getDesc(Row *row,
                        const std::wstring &key,
                        std::wstring::size_type index,
                        bool create,
                        bool callFoundPrefixCallback);


    T *emptyKeyUserData;  // user data associated with the empty string key
    Row *firstRow;  // must be created by operator new
    bool userDataFromNew;


    // Forbidden operations:
    Trie(const Trie &);
    Trie &operator = (const Trie &);

};


}  // namespace verbiste


#include "Trie.cpp"


#endif  /* _H_Trie */
