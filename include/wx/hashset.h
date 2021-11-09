/////////////////////////////////////////////////////////////////////////////
// Name:        wx/hashset.h
// Purpose:     wxHashSet class
// Author:      Mattia Barbon
// Modified by:
// Created:     11/08/2003
// Copyright:   (c) Mattia Barbon
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HASHSET_H_
#define _WX_HASHSET_H_

#include "wx/hashmap.h"

// see comment in wx/hashmap.h which also applies to different standard hash
// set classes

import <unordered_set>;
#define WX_HASH_SET_BASE_TEMPLATE std::unordered_set

// we need to define the class declared by _WX_DECLARE_HASH_SET as a class and
// not a typedef to allow forward declaring it
#define _WX_DECLARE_HASH_SET_IMPL( KEY_T, HASH_T, KEY_EQ_T, PTROP, CLASSNAME, CLASSEXP )  \
CLASSEXP CLASSNAME                                                            \
    : public WX_HASH_SET_BASE_TEMPLATE< KEY_T, HASH_T, KEY_EQ_T >             \
{                                                                             \
public:                                                                       \
    explicit CLASSNAME(size_type n = 3,                                       \
                       const hasher& h = hasher(),                            \
                       const key_equal& ke = key_equal(),                     \
                       const allocator_type& a = allocator_type())            \
        : WX_HASH_SET_BASE_TEMPLATE< KEY_T, HASH_T, KEY_EQ_T >(n, h, ke, a)   \
    {}                                                                        \
    template <class InputIterator>                                            \
    CLASSNAME(InputIterator f, InputIterator l,                               \
              const hasher& h = hasher(),                                     \
              const key_equal& ke = key_equal(),                              \
              const allocator_type& a = allocator_type())                     \
        : WX_HASH_SET_BASE_TEMPLATE< KEY_T, HASH_T, KEY_EQ_T >(f, l, h, ke, a)\
    {}                                                                        \
    CLASSNAME(const WX_HASH_SET_BASE_TEMPLATE< KEY_T, HASH_T, KEY_EQ_T >& s)  \
        : WX_HASH_SET_BASE_TEMPLATE< KEY_T, HASH_T, KEY_EQ_T >(s)             \
    {}                                                                        \
}

// In some standard library implementations (in particular, the libstdc++ that
// ships with g++ 4.7), std::unordered_set inherits privately from its hasher
// and comparator template arguments for purposes of empty base optimization.
// As a result, in the declaration of a class deriving from std::unordered_set
// the names of the hasher and comparator classes are interpreted as naming
// the base class which is inaccessible.
// The workaround is to prefix the class names with 'struct'; however, don't
// do this unconditionally, as with other compilers (both MSVC and clang)
// doing it causes a warning if the class was declared as a 'class' rather than
// a 'struct'.
#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 7)
#define WX_MAYBE_PREFIX_WITH_STRUCT(STRUCTNAME) struct STRUCTNAME
#else
#define WX_MAYBE_PREFIX_WITH_STRUCT(STRUCTNAME) STRUCTNAME
#endif

#define _WX_DECLARE_HASH_SET( KEY_T, HASH_T, KEY_EQ_T, PTROP, CLASSNAME, CLASSEXP )   \
    _WX_DECLARE_HASH_SET_IMPL(                                                \
        KEY_T,                                                                \
        WX_MAYBE_PREFIX_WITH_STRUCT(HASH_T),                                  \
        WX_MAYBE_PREFIX_WITH_STRUCT(KEY_EQ_T),                                \
        PTROP,                                                                \
        CLASSNAME,                                                            \
        CLASSEXP)

// these macros are to be used in the user code
#define WX_DECLARE_HASH_SET( KEY_T, HASH_T, KEY_EQ_T, CLASSNAME) \
    _WX_DECLARE_HASH_SET( KEY_T, HASH_T, KEY_EQ_T, wxPTROP_NORMAL, CLASSNAME, class )

// and these do exactly the same thing but should be used inside the
// library
// note: DECL is not used since the class is inline
#define WX_DECLARE_HASH_SET_WITH_DECL( KEY_T, HASH_T, KEY_EQ_T, CLASSNAME, DECL) \
    _WX_DECLARE_HASH_SET( KEY_T, HASH_T, KEY_EQ_T, wxPTROP_NORMAL, CLASSNAME, class )

#define WX_DECLARE_EXPORTED_HASH_SET( KEY_T, HASH_T, KEY_EQ_T, CLASSNAME) \
    WX_DECLARE_HASH_SET_WITH_DECL( KEY_T, HASH_T, KEY_EQ_T, \
                                   CLASSNAME, class )

// Finally these versions allow to define hash sets of non-objects (including
// pointers, hence the confusing but wxArray-compatible name) without
// operator->() which can't be used for them. This is mostly used inside the
// library itself to avoid warnings when using such hash sets with some less
// common compilers (notably Sun CC).
#define WX_DECLARE_HASH_SET_PTR( KEY_T, HASH_T, KEY_EQ_T, CLASSNAME) \
    _WX_DECLARE_HASH_SET( KEY_T, HASH_T, KEY_EQ_T, wxPTROP_NOP, CLASSNAME, class )
// note: DECL is not used since the class is inline
#define WX_DECLARE_HASH_SET_WITH_DECL_PTR( KEY_T, HASH_T, KEY_EQ_T, CLASSNAME, DECL) \
    _WX_DECLARE_HASH_SET( KEY_T, HASH_T, KEY_EQ_T, wxPTROP_NOP, CLASSNAME, class )

// delete all hash elements
//
// NB: the class declaration of the hash elements must be visible from the
//     place where you use this macro, otherwise the proper destructor may not
//     be called (a decent compiler should give a warning about it, but don't
//     count on it)!
#define WX_CLEAR_HASH_SET(type, hashset)                                     \
    {                                                                        \
        type::iterator it, en;                                               \
        for( it = (hashset).begin(), en = (hashset).end(); it != en; ++it )  \
            delete *it;                                                      \
        (hashset).clear();                                                   \
    }

#endif // _WX_HASHSET_H_
