/////////////////////////////////////////////////////////////////////////////
// Name:        wx/list.h
// Purpose:     wxList, wxStringList classes
// Author:      Julian Smart
// Modified by: VZ at 16/11/98: WX_DECLARE_LIST() and typesafe lists added
// Created:     29/01/98
// Copyright:   (c) 1998 Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

/*
  All this is quite ugly but serves two purposes:
    1. Be almost 100% compatible with old, untyped, wxList class
    2. Ensure compile-time type checking for the linked lists

  The idea is to have one base class (wxListBase) working with "void *" data,
  but to hide these untyped functions - i.e. make them protected, so they
  can only be used from derived classes which have inline member functions
  working with right types. This achieves the 2nd goal. As for the first one,
  we provide a special derivation of wxListBase called wxList which looks just
  like the old class.
*/

#ifndef _WX_LIST_H_
#define _WX_LIST_H_

// -----------------------------------------------------------------------------
// headers
// -----------------------------------------------------------------------------

#include "wx/defs.h"
#include "wx/object.h"
#include "wx/string.h"
#include <vector>

#include <algorithm>
#include <iterator>
#include <list>

/* ---------------------------------------------------------------------------- */
/* wxList types */
/* ---------------------------------------------------------------------------- */

/* wxList iterator function */
typedef int (* wxListIterateFunction)(void *current);

/* type of compare function for list sort operation (as in 'qsort'): it should
   return a negative value, 0 or positive value if the first element is less
   than, equal or greater than the second */

typedef int (* wxSortCompareFunction)(const void *elem1, const void *elem2);


// ----------------------------------------------------------------------------
// types
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_FWD_BASE wxObjectListNode;
using wxNode = wxObjectListNode;

#define wxLIST_COMPATIBILITY

#define WX_DECLARE_LIST_3(elT, dummy1, liT, dummy2, decl) \
    WX_DECLARE_LIST_WITH_DECL(elT, liT, decl)
#define WX_DECLARE_LIST_PTR_3(elT, dummy1, liT, dummy2, decl) \
    WX_DECLARE_LIST_3(elT, dummy1, liT, dummy2, decl)

#define WX_DECLARE_LIST_2(elT, liT, dummy, decl) \
    WX_DECLARE_LIST_WITH_DECL(elT, liT, decl)
#define WX_DECLARE_LIST_PTR_2(elT, liT, dummy, decl) \
    WX_DECLARE_LIST_2(elT, liT, dummy, decl) \

#define WX_DECLARE_LIST_WITH_DECL(elT, liT, decl) \
    WX_DECLARE_LIST_XO(elT*, liT, decl)

template<class T>
class wxList_SortFunction
{
public:
    wxList_SortFunction(wxSortCompareFunction f) : m_f(f) { }
    bool operator()(const T& i1, const T& i2)
      { return m_f(&i1, &i2) < 0; }
private:
    wxSortCompareFunction m_f;
};

/*
    Note 1: the outer helper class _WX_LIST_HELPER_##liT below is a workaround
    for mingw 3.2.3 compiler bug that prevents a static function of liT class
    from being exported into dll. A minimal code snippet reproducing the bug:

         struct Foo
         {
            static void Bar();
            struct SomeInnerClass
            {
              friend class Foo; // comment this out to make it link
            };
            ~Foo()
            {
                Bar();
            }
         };

    The program does not link under mingw_gcc 3.2.3 producing undefined
    reference to Foo::Bar() function


    Note 2: the EmptyList is needed to allow having a NULL pointer-like
    invalid iterator. We used to use just an uninitialized iterator object
    instead but this fails with some debug/checked versions of STL, notably the
    glibc version activated with _GLIBCXX_DEBUG, so we need to have a separate
    invalid iterator.
 */

// the real wxList-class declaration
#define WX_DECLARE_LIST_XO(elT, liT, decl)                                    \
    decl _WX_LIST_HELPER_##liT                                                \
    {                                                                         \
        typedef elT _WX_LIST_ITEM_TYPE_##liT;                                 \
        typedef std::list<elT> BaseListType;                                  \
    public:                                                                   \
        static BaseListType EmptyList;                                        \
        static void DeleteFunction( _WX_LIST_ITEM_TYPE_##liT X );             \
    };                                                                        \
                                                                              \
    class liT : public std::list<elT>                                          \
    {                                                                         \
    private:                                                                  \
        typedef std::list<elT> BaseListType;                                  \
                                                                              \
        bool m_destroy;                                                       \
                                                                              \
    public:                                                                   \
        class compatibility_iterator                                           \
        {                                                                     \
        private:                                                              \
            friend class liT;                                                 \
                                                                              \
            iterator m_iter;                                                  \
            liT * m_list;                                                     \
                                                                              \
        public:                                                               \
            compatibility_iterator()                                          \
                : m_iter(_WX_LIST_HELPER_##liT::EmptyList.end()), m_list( NULL ) {}                  \
            compatibility_iterator( liT* li, iterator i )                     \
                : m_iter( i ), m_list( li ) {}                                \
            compatibility_iterator( const liT* li, iterator i )               \
                : m_iter( i ), m_list( const_cast< liT* >( li ) ) {}          \
                                                                              \
            compatibility_iterator* operator->() { return this; }             \
            const compatibility_iterator* operator->() const { return this; } \
                                                                              \
            bool operator==(const compatibility_iterator& i) const            \
            {                                                                 \
                wxASSERT_MSG( m_list && i.m_list,                             \
                              wxT("comparing invalid iterators is illegal") ); \
                return (m_list == i.m_list) && (m_iter == i.m_iter);          \
            }                                                                 \
            bool operator!=(const compatibility_iterator& i) const            \
                { return !( operator==( i ) ); }                              \
            operator bool() const                                             \
                { return m_list ? m_iter != m_list->end() : false; }          \
            bool operator !() const                                           \
                { return !( operator bool() ); }                              \
                                                                              \
            elT GetData() const                                               \
                { return *m_iter; }                                           \
            void SetData( elT e )                                             \
                { *m_iter = e; }                                              \
                                                                              \
            compatibility_iterator GetNext() const                            \
            {                                                                 \
                iterator i = m_iter;                                          \
                return compatibility_iterator( m_list, ++i );                 \
            }                                                                 \
            compatibility_iterator GetPrevious() const                        \
            {                                                                 \
                if ( m_iter == m_list->begin() )                              \
                    return compatibility_iterator();                          \
                                                                              \
                iterator i = m_iter;                                          \
                return compatibility_iterator( m_list, --i );                 \
            }                                                                 \
            int IndexOf() const                                               \
            {                                                                 \
                return *this ? (int)std::distance( m_list->begin(), m_iter )  \
                             : wxNOT_FOUND;                                   \
            }                                                                 \
        };                                                                    \
    public:                                                                   \
        liT() : m_destroy( false ) {}                                         \
                                                                              \
        compatibility_iterator Find( const elT e ) const                      \
        {                                                                     \
          liT* _this = const_cast< liT* >( this );                            \
          return compatibility_iterator( _this,                               \
                     std::find( _this->begin(), _this->end(), e ) );          \
        }                                                                     \
                                                                              \
        bool IsEmpty() const                                                  \
            { return empty(); }                                               \
        size_t GetCount() const                                               \
            { return size(); }                                                \
        int Number() const                                                    \
            { return static_cast< int >( GetCount() ); }                      \
                                                                              \
        compatibility_iterator Item( size_t idx ) const                       \
        {                                                                     \
            iterator i = const_cast< liT* >(this)->begin();                   \
            std::advance( i, idx );                                           \
            return compatibility_iterator( this, i );                         \
        }                                                                     \
        elT operator[](size_t idx) const                                      \
        {                                                                     \
            return Item(idx).GetData();                                       \
        }                                                                     \
                                                                              \
        compatibility_iterator GetFirst() const                               \
        {                                                                     \
            return compatibility_iterator( this,                              \
                const_cast< liT* >(this)->begin() );                          \
        }                                                                     \
        compatibility_iterator GetLast() const                                \
        {                                                                     \
            iterator i = const_cast< liT* >(this)->end();                     \
            return compatibility_iterator( this, !empty() ? --i : i );        \
        }                                                                     \
        bool Member( elT e ) const                                            \
            { return Find( e ); }                                             \
        compatibility_iterator Nth( int n ) const                             \
            { return Item( n ); }                                             \
        int IndexOf( elT e ) const                                            \
            { return Find( e ).IndexOf(); }                                   \
                                                                              \
        compatibility_iterator Append( elT e )                                \
        {                                                                     \
            push_back( e );                                                   \
            return GetLast();                                                 \
        }                                                                     \
        compatibility_iterator Insert( elT e )                                \
        {                                                                     \
            push_front( e );                                                  \
            return compatibility_iterator( this, begin() );                   \
        }                                                                     \
        compatibility_iterator Insert(const compatibility_iterator & i, elT e)\
        {                                                                     \
            return compatibility_iterator( this, insert( i.m_iter, e ) );     \
        }                                                                     \
        compatibility_iterator Insert( size_t idx, elT e )                    \
        {                                                                     \
            return compatibility_iterator( this,                              \
                                           insert( Item( idx ).m_iter, e ) ); \
        }                                                                     \
                                                                              \
        void DeleteContents( bool destroy )                                   \
            { m_destroy = destroy; }                                          \
        bool GetDeleteContents() const                                        \
            { return m_destroy; }                                             \
        void Erase( const compatibility_iterator& i )                         \
        {                                                                     \
            if ( m_destroy )                                                  \
                _WX_LIST_HELPER_##liT::DeleteFunction( i->GetData() );        \
            erase( i.m_iter );                                                \
        }                                                                     \
        bool DeleteNode( const compatibility_iterator& i )                    \
        {                                                                     \
            if( i )                                                           \
            {                                                                 \
                Erase( i );                                                   \
                return true;                                                  \
            }                                                                 \
            return false;                                                     \
        }                                                                     \
        bool DeleteObject( elT e )                                            \
        {                                                                     \
            return DeleteNode( Find( e ) );                                   \
        }                                                                     \
        void Clear()                                                          \
        {                                                                     \
            if ( m_destroy )                                                  \
                std::for_each( begin(), end(),                                \
                               _WX_LIST_HELPER_##liT::DeleteFunction );       \
            clear();                                                          \
        }                                                                     \
        /* Workaround for broken VC6 std::list::sort() see above */           \
        void Sort( wxSortCompareFunction compfunc )                           \
            { sort( wxList_SortFunction<elT>(compfunc ) ); }                  \
        ~liT() { Clear(); }                                                   \
                                                                              \
        /* It needs access to our EmptyList */                                \
        friend class compatibility_iterator;                                  \
    }

#define WX_DECLARE_LIST(elementtype, listname)                              \
    WX_DECLARE_LIST_WITH_DECL(elementtype, listname, class)
#define WX_DECLARE_LIST_PTR(elementtype, listname)                          \
    WX_DECLARE_LIST(elementtype, listname)

#define WX_DECLARE_EXPORTED_LIST(elementtype, listname)                     \
    WX_DECLARE_LIST_WITH_DECL(elementtype, listname, class WXDLLIMPEXP_CORE)
#define WX_DECLARE_EXPORTED_LIST_PTR(elementtype, listname)                 \
    WX_DECLARE_EXPORTED_LIST(elementtype, listname)

#define WX_DECLARE_USER_EXPORTED_LIST(elementtype, listname, usergoo)       \
    WX_DECLARE_LIST_WITH_DECL(elementtype, listname, class usergoo)
#define WX_DECLARE_USER_EXPORTED_LIST_PTR(elementtype, listname, usergoo)   \
    WX_DECLARE_USER_EXPORTED_LIST(elementtype, listname, usergoo)

// this macro must be inserted in your program after
//      #include "wx/listimpl.cpp"
#define WX_DEFINE_LIST(name)    "don't forget to include listimpl.cpp!"

#define WX_DEFINE_EXPORTED_LIST(name)      WX_DEFINE_LIST(name)
#define WX_DEFINE_USER_EXPORTED_LIST(name) WX_DEFINE_LIST(name)

// ============================================================================
// now we can define classes 100% compatible with the old ones
// ============================================================================

// ----------------------------------------------------------------------------
// commonly used list classes
// ----------------------------------------------------------------------------

#if defined(wxLIST_COMPATIBILITY)

// define this to make a lot of noise about use of the old wxList classes.
//#define wxWARN_COMPAT_LIST_USE

// ----------------------------------------------------------------------------
// wxList compatibility class: in fact, it's a list of wxObjects
// ----------------------------------------------------------------------------

WX_DECLARE_LIST_2(wxObject, wxObjectList, wxObjectListNode,
                        class WXDLLIMPEXP_BASE);

class WXDLLIMPEXP_BASE wxList : public wxObjectList
{
public:
    template<typename T>
    std::vector<T> AsVector() const
    {
        std::vector<T> vector(size());
        size_t i = 0;

        for ( const_iterator it = begin(); it != end(); ++it )
        {
            vector[i++] = static_cast<T>(*it);
        }

        return vector;
    }

};

WX_DECLARE_LIST_XO(wxString, wxStringListBase, class WXDLLIMPEXP_BASE);

class WXDLLIMPEXP_BASE wxStringList : public wxStringListBase
{
public:
    compatibility_iterator Append(wxChar* s)
        { wxString tmp = s; delete[] s; return wxStringListBase::Append(tmp); }
    compatibility_iterator Insert(wxChar* s)
        { wxString tmp = s; delete[] s; return wxStringListBase::Insert(tmp); }
    compatibility_iterator Insert(size_t pos, wxChar* s)
    {
        wxString tmp = s;
        delete[] s;
        return wxStringListBase::Insert(pos, tmp);
    }
    compatibility_iterator Add(const wxChar* s)
        { push_back(s); return GetLast(); }
    compatibility_iterator Prepend(const wxChar* s)
        { push_front(s); return GetFirst(); }
};

#endif // wxLIST_COMPATIBILITY

// delete all list elements
//
// NB: the class declaration of the list elements must be visible from the
//     place where you use this macro, otherwise the proper destructor may not
//     be called (a decent compiler should give a warning about it, but don't
//     count on it)!
#define WX_CLEAR_LIST(type, list)                                            \
    {                                                                        \
        type::iterator it, en;                                               \
        for( it = (list).begin(), en = (list).end(); it != en; ++it )        \
            delete *it;                                                      \
        (list).clear();                                                      \
    }

// append all element of one list to another one
#define WX_APPEND_LIST(list, other)                                           \
    {                                                                         \
        wxList::compatibility_iterator node = other->GetFirst();              \
        while ( node )                                                        \
        {                                                                     \
            (list)->push_back(node->GetData());                               \
            node = node->GetNext();                                           \
        }                                                                     \
    }

#endif // _WX_LISTH__
