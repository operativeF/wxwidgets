/////////////////////////////////////////////////////////////////////////////
// Name:        wx/hashmap.h
// Purpose:     wxHashMap class
// Author:      Mattia Barbon
// Modified by:
// Created:     29/01/2002
// Copyright:   (c) Mattia Barbon
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HASHMAP_H_
#define _WX_HASHMAP_H_

#include "wx/string.h"
#include "wx/wxcrt.h"

#include <unordered_map>

#include <gsl/gsl>

#define WX_HASH_MAP_NAMESPACE std

#define _WX_DECLARE_HASH_MAP( KEY_T, VALUE_T, HASH_T, KEY_EQ_T, CLASSNAME, CLASSEXP ) \
    typedef WX_HASH_MAP_NAMESPACE::unordered_map< KEY_T, VALUE_T, HASH_T, KEY_EQ_T > CLASSNAME

// ----------------------------------------------------------------------------
// hashing and comparison functors
// ----------------------------------------------------------------------------

#ifndef wxNEEDS_WX_HASH_MAP

// integer types
struct wxIntegerHash
{
private:
    WX_HASH_MAP_NAMESPACE::hash<long> longHash;
    WX_HASH_MAP_NAMESPACE::hash<unsigned long> ulongHash;
    WX_HASH_MAP_NAMESPACE::hash<int> intHash;
    WX_HASH_MAP_NAMESPACE::hash<unsigned int> uintHash;
    WX_HASH_MAP_NAMESPACE::hash<short> shortHash;
    WX_HASH_MAP_NAMESPACE::hash<unsigned short> ushortHash;

#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
    // hash<wxLongLong_t> ought to work but doesn't on some compilers
    #if (!defined SIZEOF_LONG_LONG && SIZEOF_LONG == 4) \
        || (defined SIZEOF_LONG_LONG && SIZEOF_LONG_LONG == SIZEOF_LONG * 2)
    size_t longlongHash( wxLongLong_t x ) const
    {
        return longHash( gsl::narrow_cast<long>(x) ) ^
               longHash( gsl::narrow_cast<long>(x >> (sizeof(long) * 8)) );
    }
    #elif defined SIZEOF_LONG_LONG && SIZEOF_LONG_LONG == SIZEOF_LONG
    WX_HASH_MAP_NAMESPACE::hash<long> longlongHash;
    #else
    WX_HASH_MAP_NAMESPACE::hash<wxLongLong_t> longlongHash;
    #endif
#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

public:
    size_t operator()( long x ) const noexcept { return longHash( x ); }
    size_t operator()( unsigned long x ) const noexcept { return ulongHash( x ); }
    size_t operator()( int x ) const noexcept { return intHash( x ); }
    size_t operator()( unsigned int x ) const noexcept { return uintHash( x ); }
    size_t operator()( short x ) const noexcept { return shortHash( x ); }
    size_t operator()( unsigned short x ) const noexcept { return ushortHash( x ); }
#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
    size_t operator()( wxLongLong_t x ) const noexcept { return longlongHash(x); }
    size_t operator()( wxULongLong_t x ) const noexcept { return longlongHash(x); }
#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
};

#else // wxNEEDS_WX_HASH_MAP

// integer types
struct wxIntegerHash
{
    unsigned long operator()( long x ) const noexcept { return (unsigned long)x; }
    unsigned long operator()( unsigned long x ) const noexcept { return x; }
    unsigned long operator()( int x ) const noexcept { return (unsigned long)x; }
    unsigned long operator()( unsigned int x ) const noexcept { return x; }
    unsigned long operator()( short x ) const noexcept { return (unsigned long)x; }
    unsigned long operator()( unsigned short x ) const noexcept { return x; }
#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
    wxULongLong_t operator()( wxLongLong_t x ) const noexcept { return static_cast<wxULongLong_t>(x); }
    wxULongLong_t operator()( wxULongLong_t x ) const noexcept { return x; }
#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
};

#endif // !wxNEEDS_WX_HASH_MAP/wxNEEDS_WX_HASH_MAP

struct wxIntegerEqual
{
    bool operator()( long a, long b ) const { return a == b; }
    bool operator()( unsigned long a, unsigned long b ) const { return a == b; }
    bool operator()( int a, int b ) const { return a == b; }
    bool operator()( unsigned int a, unsigned int b ) const { return a == b; }
    bool operator()( short a, short b ) const { return a == b; }
    bool operator()( unsigned short a, unsigned short b ) const { return a == b; }
#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
    bool operator()( wxLongLong_t a, wxLongLong_t b ) const { return a == b; }
    bool operator()( wxULongLong_t a, wxULongLong_t b ) const { return a == b; }
#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
};

// pointers
struct wxPointerHash
{
#ifdef wxNEEDS_WX_HASH_MAP
    wxUIntPtr operator()( const void* k ) const noexcept { return wxPtrToUInt(k); }
#else
    size_t operator()( const void* k ) const noexcept { return (size_t)k; }
#endif
};

struct wxPointerEqual
{
    bool operator()( const void* a, const void* b ) const noexcept { return a == b; }
};

// wxString, char*, wchar_t*
struct wxStringHash
{
    unsigned long operator()( const wxString& x ) const noexcept
        { return stringHash( x.wx_str() ); }
    unsigned long operator()( const wchar_t* x ) const noexcept
        { return stringHash( x ); }
    unsigned long operator()( const char* x ) const noexcept
        { return stringHash( x ); }

    static unsigned long stringHash( const wchar_t* );
    static unsigned long stringHash( const char* );
};

struct wxStringEqual
{
    bool operator()( const wxString& a, const wxString& b ) const noexcept
        { return a == b; }
    bool operator()( const wxChar* a, const wxChar* b ) const noexcept
        { return wxStrcmp( a, b ) == 0; }
    bool operator()( const char* a, const char* b ) const noexcept
        { return strcmp( a, b ) == 0; }
};

#ifdef wxNEEDS_WX_HASH_MAP

#define wxPTROP_NORMAL(pointer) \
    pointer operator ->() const { return &(m_node->m_value); }
#define wxPTROP_NOP(pointer)

#define _WX_DECLARE_HASH_MAP( KEY_T, VALUE_T, HASH_T, KEY_EQ_T, CLASSNAME, CLASSEXP ) \
_WX_DECLARE_PAIR( KEY_T, VALUE_T, CLASSNAME##_wxImplementation_Pair, CLASSEXP ) \
_WX_DECLARE_HASH_MAP_KEY_EX( KEY_T, CLASSNAME##_wxImplementation_Pair, CLASSNAME##_wxImplementation_KeyEx, CLASSEXP ) \
_WX_DECLARE_HASHTABLE( CLASSNAME##_wxImplementation_Pair, KEY_T, HASH_T, \
    CLASSNAME##_wxImplementation_KeyEx, KEY_EQ_T, wxPTROP_NORMAL, \
    CLASSNAME##_wxImplementation_HashTable, CLASSEXP, grow_lf70, never_shrink ) \
CLASSEXP CLASSNAME:public CLASSNAME##_wxImplementation_HashTable \
{ \
public: \
    typedef VALUE_T mapped_type; \
    _WX_DECLARE_PAIR( iterator, bool, Insert_Result, CLASSEXP ) \
 \
    explicit CLASSNAME( size_type hint = 100, hasher hf = hasher(),          \
                        key_equal eq = key_equal() )                         \
        : CLASSNAME##_wxImplementation_HashTable( hint, hf, eq,              \
                                   CLASSNAME##_wxImplementation_KeyEx() ) {} \
 \
    mapped_type& operator[]( const const_key_type& key ) \
    { \
        bool created; \
        return GetOrCreateNode( \
                CLASSNAME##_wxImplementation_Pair( key, mapped_type() ), \
                created)->m_value.second; \
    } \
 \
    const_iterator find( const const_key_type& key ) const \
    { \
        return const_iterator( GetNode( key ), this ); \
    } \
 \
    iterator find( const const_key_type& key ) \
    { \
        return iterator( GetNode( key ), this ); \
    } \
 \
    Insert_Result insert( const value_type& v ) \
    { \
        bool created; \
        Node *node = GetOrCreateNode( \
                CLASSNAME##_wxImplementation_Pair( v.first, v.second ), \
                created); \
        return Insert_Result(iterator(node, this), created); \
    } \
 \
    size_type erase( const key_type& k ) \
        { return CLASSNAME##_wxImplementation_HashTable::erase( k ); } \
    void erase( const iterator& it ) { erase( (*it).first ); } \
 \
    /* count() == 0 | 1 */ \
    size_type count( const const_key_type& key ) \
    { \
        return GetNode( key ) ? 1u : 0u; \
    } \
}

#endif // wxNEEDS_WX_HASH_MAP

// these macros are to be used in the user code
#define WX_DECLARE_HASH_MAP( KEY_T, VALUE_T, HASH_T, KEY_EQ_T, CLASSNAME) \
    _WX_DECLARE_HASH_MAP( KEY_T, VALUE_T, HASH_T, KEY_EQ_T, CLASSNAME, class )

#define WX_DECLARE_STRING_HASH_MAP( VALUE_T, CLASSNAME ) \
    _WX_DECLARE_HASH_MAP( wxString, VALUE_T, wxStringHash, wxStringEqual, \
                          CLASSNAME, class )

#define WX_DECLARE_VOIDPTR_HASH_MAP( VALUE_T, CLASSNAME ) \
    _WX_DECLARE_HASH_MAP( void*, VALUE_T, wxPointerHash, wxPointerEqual, \
                          CLASSNAME, class )

// and these do exactly the same thing but should be used inside the
// library
// note: DECL is not used since the class is inline
#define WX_DECLARE_HASH_MAP_WITH_DECL( KEY_T, VALUE_T, HASH_T, KEY_EQ_T, CLASSNAME, DECL) \
    _WX_DECLARE_HASH_MAP( KEY_T, VALUE_T, HASH_T, KEY_EQ_T, CLASSNAME, class )

#define WX_DECLARE_EXPORTED_HASH_MAP( KEY_T, VALUE_T, HASH_T, KEY_EQ_T, CLASSNAME) \
    WX_DECLARE_HASH_MAP_WITH_DECL( KEY_T, VALUE_T, HASH_T, KEY_EQ_T, \
                                   CLASSNAME, class )

// note: DECL is not used since the class is inline
#define WX_DECLARE_STRING_HASH_MAP_WITH_DECL( VALUE_T, CLASSNAME, DECL ) \
    _WX_DECLARE_HASH_MAP( wxString, VALUE_T, wxStringHash, wxStringEqual, \
                          CLASSNAME, class )

#define WX_DECLARE_EXPORTED_STRING_HASH_MAP( VALUE_T, CLASSNAME ) \
    WX_DECLARE_STRING_HASH_MAP_WITH_DECL( VALUE_T, CLASSNAME, \
                                          class )

// note: DECL is not used since the class is inline
#define WX_DECLARE_VOIDPTR_HASH_MAP_WITH_DECL( VALUE_T, CLASSNAME, DECL ) \
    _WX_DECLARE_HASH_MAP( void*, VALUE_T, wxPointerHash, wxPointerEqual, \
                          CLASSNAME, class )

#define WX_DECLARE_EXPORTED_VOIDPTR_HASH_MAP( VALUE_T, CLASSNAME ) \
    WX_DECLARE_VOIDPTR_HASH_MAP_WITH_DECL( VALUE_T, CLASSNAME, \
                                           class )

// delete all hash elements
//
// NB: the class declaration of the hash elements must be visible from the
//     place where you use this macro, otherwise the proper destructor may not
//     be called (a decent compiler should give a warning about it, but don't
//     count on it)!
#define WX_CLEAR_HASH_MAP(type, hashmap)                                     \
    {                                                                        \
        type::iterator it, en;                                               \
        for( it = (hashmap).begin(), en = (hashmap).end(); it != en; ++it )  \
            delete it->second;                                               \
        (hashmap).clear();                                                   \
    }

//---------------------------------------------------------------------------
// Declarations of common hashmap classes

WX_DECLARE_HASH_MAP_WITH_DECL( long, long, wxIntegerHash, wxIntegerEqual,
                               wxLongToLongHashMap, class );

WX_DECLARE_STRING_HASH_MAP_WITH_DECL( wxString, wxStringToStringHashMap,
                                      class );

WX_DECLARE_STRING_HASH_MAP_WITH_DECL( wxUIntPtr, wxStringToNumHashMap,
                                      class );


#endif // _WX_HASHMAP_H_
