/*
 *  Name:        wx/defs.h
 *  Purpose:     Declarations/definitions common to all wx source files
 *  Author:      Julian Smart and others
 *  Modified by: Ryan Norton (Converted to C)
 *  Created:     01/02/97
 *  Copyright:   (c) Julian Smart
 *  Licence:     wxWindows licence
 */

/* THIS IS A C FILE, DON'T USE C++ FEATURES (IN PARTICULAR COMMENTS) IN IT */


#ifndef _WX_DEFS_H_
#define _WX_DEFS_H_

/*  ---------------------------------------------------------------------------- */
/*  compiler and OS identification */
/*  ---------------------------------------------------------------------------- */

#ifndef wxUSE_BASE
    /*  by default consider that this is a monolithic build */
    #define wxUSE_BASE 1
#endif

#if !wxUSE_GUI && !defined(__WXBASE__)
    #define __WXBASE__
#endif

/*
   g++ gives a warning when a class has private dtor if it has no friends but
   this is a perfectly valid situation for a ref-counted class which destroys
   itself when its ref count drops to 0, so provide a macro to suppress this
   warning
 */
#ifdef __GNUG__
#   define wxSUPPRESS_GCC_PRIVATE_DTOR_WARNING(name) \
        friend class wxDummyFriendFor ## name;
#else /* !g++ */
#   define wxSUPPRESS_GCC_PRIVATE_DTOR_WARNING(name)
#endif

/*
   Clang Support
 */

#ifndef WX_HAS_CLANG_FEATURE
#   ifndef __has_feature
#       define WX_HAS_CLANG_FEATURE(x) 0
#   else
#       define WX_HAS_CLANG_FEATURE(x) __has_feature(x)
#   endif
#endif

/*  ---------------------------------------------------------------------------- */
/*  wxWidgets version and compatibility defines */
/*  ---------------------------------------------------------------------------- */

#include "wx/version.h"
/*  ---------------------------------------------------------------------------- */
/*  compiler defects workarounds */
/*  ---------------------------------------------------------------------------- */

/*
   Digital Unix C++ compiler only defines this symbol for .cxx and .hxx files,
   so define it ourselves (newer versions do it for all files, though, and
   don't allow it to be redefined)
 */
#if defined(__DECCXX) && !defined(__VMS) && !defined(__cplusplus)
#define __cplusplus
#endif /* __DECCXX */

/*  Resolves linking problems under HP-UX when compiling with gcc/g++ */
#if defined(__HPUX__) && defined(__GNUG__)
#define va_list __gnuc_va_list
#endif /*  HP-UX */

/* Prevents conflicts between sys/types.h and winsock.h with Cygwin, */
/* when using Windows sockets. */
#if defined(__CYGWIN__) && defined(WX_WINDOWS)
#define __USE_W32_SOCKETS
#endif

/*  ---------------------------------------------------------------------------- */
/*  check for native bool type and TRUE/FALSE constants */
/*  ---------------------------------------------------------------------------- */

/*  for backwards compatibility, also define TRUE and FALSE */
/*  */
/*  note that these definitions should work both in C++ and C code, so don't */
/*  use true/false below */
#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

/*  ---------------------------------------------------------------------------- */
/*  other feature tests */
/*  ---------------------------------------------------------------------------- */

/* provide replacement for C99 va_copy() if the compiler doesn't have it */

/* could be already defined by configure or the user */
#ifndef wxVaCopy
    /* if va_copy is a macro or configure detected that we have it, use it */
    #if defined(va_copy) || defined(HAVE_VA_COPY)
        #define wxVaCopy va_copy
    #else /* no va_copy, try to provide a replacement */
        /*
           configure tries to determine whether va_list is an array or struct
           type, but it may not be used under Windows, so deal with a few
           special cases.
         */

        #if defined(__PPC__) && (defined(_CALL_SYSV) || defined (_WIN32))
            /*
                PPC using SysV ABI and NT/PPC are special in that they use an
                extra level of indirection.
             */
            #define VA_LIST_IS_POINTER
        #endif /* SysV or Win32 on __PPC__ */

        /*
            note that we use memmove(), not memcpy(), in case anybody tries
            to do wxVaCopy(ap, ap)
         */
        #if defined(VA_LIST_IS_POINTER)
            #define wxVaCopy(d, s)  memmove(*(d), *(s), sizeof(va_list))
        #elif defined(VA_LIST_IS_ARRAY)
            #define wxVaCopy(d, s) memmove((d), (s), sizeof(va_list))
        #else /* we can only hope that va_lists are simple lvalues */
            #define wxVaCopy(d, s) ((d) = (s))
        #endif
    #endif /* va_copy/!va_copy */
#endif /* wxVaCopy */

/*  ---------------------------------------------------------------------------- */
/*  portable calling conventions macros */
/*  ---------------------------------------------------------------------------- */

/*  stdcall is used for all functions called by Windows under Windows */
#if defined(WX_WINDOWS)
    #if defined(__GNUWIN32__)
        #define wxSTDCALL __attribute__((stdcall))
    #else
        #define wxSTDCALL _stdcall
    #endif

#else /*  Win */
    /*  no such stupidness under Unix */
    #define wxSTDCALL
#endif /*  platform */

/*  wxCALLBACK should be used for the functions which are called back by */
/*  Windows (such as compare function for wxListCtrl) */
#if defined(__WIN32__)
    #define wxCALLBACK wxSTDCALL
#else
    /*  no stdcall under Unix nor Win16 */
    #define wxCALLBACK
#endif /*  platform */

/*  generic calling convention for the extern "C" functions */

#if defined(__VISUALC__)
  #define   wxC_CALLING_CONV    _cdecl
#else   /*  !Visual C++ */
  #define   wxC_CALLING_CONV
#endif  /*  compiler */

/*  callling convention for the qsort(3) callback */
#define wxCMPFUNC_CONV wxC_CALLING_CONV

/*  compatibility :-( */
#define CMPFUNC_CONV wxCMPFUNC_CONV

/*  ---------------------------------------------------------------------------- */
/*  Very common macros */
/*  ---------------------------------------------------------------------------- */

/*  Printf-like attribute definitions to obtain warnings with GNU C/C++ */
#define WX_ATTRIBUTE_FORMAT(like, m, n)

#ifndef WX_ATTRIBUTE_PRINTF
#   define WX_ATTRIBUTE_PRINTF(m, n) WX_ATTRIBUTE_FORMAT(__printf__, m, n)

#   define WX_ATTRIBUTE_PRINTF_1 WX_ATTRIBUTE_PRINTF(1, 2)
#   define WX_ATTRIBUTE_PRINTF_2 WX_ATTRIBUTE_PRINTF(2, 3)
#   define WX_ATTRIBUTE_PRINTF_3 WX_ATTRIBUTE_PRINTF(3, 4)
#   define WX_ATTRIBUTE_PRINTF_4 WX_ATTRIBUTE_PRINTF(4, 5)
#   define WX_ATTRIBUTE_PRINTF_5 WX_ATTRIBUTE_PRINTF(5, 6)
#endif /* !defined(WX_ATTRIBUTE_PRINTF) */

#if defined(__GNUC__)
    #define WX_ATTRIBUTE_UNUSED __attribute__ ((unused))
#else
    #define WX_ATTRIBUTE_UNUSED
#endif

/*  size of statically declared array */
#define WXSIZEOF(array)   (sizeof(array)/sizeof(array[0]))

/*  symbolic constant used by all Find()-like functions returning positive */
/*  integer on success as failure indicator */
#define wxNOT_FOUND       (-1)

/* the default value for some length parameters meaning that the string is */
/* NUL-terminated */
#define wxNO_LEN ((size_t)-1)

/*  ---------------------------------------------------------------------------- */
/*  macros dealing with comparison operators */
/*  ---------------------------------------------------------------------------- */

/*
    Expands into m(op, args...) for each op in the set { ==, !=, <, <=, >, >= }.
 */
#define wxFOR_ALL_COMPARISONS(m) \
    m(==) m(!=) m(>=) m(<=) m(>) m(<)

#define wxFOR_ALL_COMPARISONS_1(m, x) \
    m(==,x) m(!=,x) m(>=,x) m(<=,x) m(>,x) m(<,x)

#define wxFOR_ALL_COMPARISONS_2(m, x, y) \
    m(==,x,y) m(!=,x,y) m(>=,x,y) m(<=,x,y) m(>,x,y) m(<,x,y)

#define wxFOR_ALL_COMPARISONS_3(m, x, y, z) \
    m(==,x,y,z) m(!=,x,y,z) m(>=,x,y,z) m(<=,x,y,z) m(>,x,y,z) m(<,x,y,z)

/*
    These are only used with wxDEFINE_COMPARISON_[BY_]REV: they pass both the
    normal and the reversed comparison operators to the macro.
 */
#define wxFOR_ALL_COMPARISONS_2_REV(m, x, y) \
    m(==,x,y,==) m(!=,x,y,!=) m(>=,x,y,<=) \
    m(<=,x,y,>=) m(>,x,y,<) m(<,x,y,>)

#define wxFOR_ALL_COMPARISONS_3_REV(m, x, y, z) \
    m(==,x,y,z,==) m(!=,x,y,z,!=) m(>=,x,y,z,<=) \
    m(<=,x,y,z,>=) m(>,x,y,z,<) m(<,x,y,z,>)


#define wxDEFINE_COMPARISON(op, T1, T2, cmp) \
    inline bool operator op(T1 x, T2 y) { return cmp(x, y, op); }

#define wxDEFINE_COMPARISON_REV(op, T1, T2, cmp, oprev) \
    inline bool operator op(T2 y, T1 x) { return cmp(x, y, oprev); }

#define wxDEFINE_COMPARISON_BY_REV(op, T1, T2, oprev) \
    inline bool operator op(T1 x, T2 y) { return y oprev x; }

/*
    Define all 6 comparison operators (==, !=, <, <=, >, >=) for the given
    types in the specified order. The implementation is provided by the cmp
    macro. Normally wxDEFINE_ALL_COMPARISONS should be used as comparison
    operators are usually symmetric.
 */
#define wxDEFINE_COMPARISONS(T1, T2, cmp) \
    wxFOR_ALL_COMPARISONS_3(wxDEFINE_COMPARISON, T1, T2, cmp)

/*
    Define all 6 comparison operators (==, !=, <, <=, >, >=) for the given
    types in the specified order, implemented in terms of existing operators
    for the reverse order.
 */
#define wxDEFINE_COMPARISONS_BY_REV(T1, T2) \
    wxFOR_ALL_COMPARISONS_2_REV(wxDEFINE_COMPARISON_BY_REV, T1, T2)

/*
    This macro allows to define all 12 comparison operators (6 operators for
    both orders of arguments) for the given types using the provided "cmp"
    macro to implement the actual comparison: the macro is called with the 2
    arguments names, the first of type T1 and the second of type T2, and the
    comparison operator being implemented.
 */
#define wxDEFINE_ALL_COMPARISONS(T1, T2, cmp) \
    wxFOR_ALL_COMPARISONS_3(wxDEFINE_COMPARISON, T1, T2, cmp) \
    wxFOR_ALL_COMPARISONS_3_REV(wxDEFINE_COMPARISON_REV, T1, T2, cmp)

/*  ---------------------------------------------------------------------------- */
/*  macros to avoid compiler warnings */
/*  ---------------------------------------------------------------------------- */

/*  Macro to cut down on compiler warnings. */
#define WXUNUSED(identifier) /* identifier */

/*  some arguments are not used in unicode mode */
#define WXUNUSED_IN_UNICODE(param)  WXUNUSED(param)


/*  unused parameters in non stream builds */
#if wxUSE_STREAMS
    #define WXUNUSED_UNLESS_STREAMS(param)  param
#else
    #define WXUNUSED_UNLESS_STREAMS(param)  WXUNUSED(param)
#endif

/*  some compilers give warning about a possibly unused variable if it is */
/*  initialized in both branches of if/else and shut up if it is initialized */
/*  when declared, but other compilers then give warnings about unused variable */
/*  value -- this should satisfy both of them */
#if defined(__VISUALC__)
    #define wxDUMMY_INITIALIZE(val) = val
#else
    #define wxDUMMY_INITIALIZE(val)
#endif

/*  sometimes the value of a variable is *really* not used, to suppress  the */
/*  resulting warning you may pass it to this function */
#ifdef __cplusplus
    template <class T>
        inline void wxUnusedVar(const T& WXUNUSED(t)) { }
#endif

/*  ---------------------------------------------------------------------------- */
/*  compiler specific settings */
/*  ---------------------------------------------------------------------------- */

/*  where should i put this? we need to make sure of this as it breaks */
/*  the <iostream> code. */
#if defined(__WXDEBUG__)
#    undef wxUSE_DEBUG_NEW_ALWAYS
#    define wxUSE_DEBUG_NEW_ALWAYS 0
#endif

#include "wx/types.h"

#ifdef __cplusplus

// everybody gets the assert and other debug macros
#include "wx/debug.h"

    // delete pointer if it is not NULL and NULL it afterwards
    template <typename T>
    inline void wxDELETE(T*& ptr)
    {
        typedef char TypeIsCompleteCheck[sizeof(T)] WX_ATTRIBUTE_UNUSED;

        if ( ptr != nullptr )
        {
            delete ptr;
            ptr = nullptr;
        }
    }

    // delete an array and NULL it (see comments above)
    template <typename T>
    inline void wxDELETEA(T*& ptr)
    {
        typedef char TypeIsCompleteCheck[sizeof(T)] WX_ATTRIBUTE_UNUSED;

        if ( ptr != nullptr )
        {
            delete [] ptr;
            ptr = nullptr;
        }
    }

/* And also define a couple of simple functions to cast pointer to/from it. */
inline wxUIntPtr wxPtrToUInt(const void *p)
{
    /*
       VC++ 7.1 gives warnings about casts such as below even when they're
       explicit with /Wp64 option, suppress them as we really know what we're
       doing here. Same thing with icc with -Wall.
     */
#ifdef __VISUALC__
    #pragma warning(push)
    /* pointer truncation from '' to '' */
    #pragma warning(disable: 4311)
#elif defined(__INTELC__)
    #pragma warning(push)
    /* conversion from pointer to same-sized integral type */
    #pragma warning(disable: 1684)
#endif

    return reinterpret_cast<wxUIntPtr>(p);

#if defined(__VISUALC__) || defined(__INTELC__)
    #pragma warning(pop)
#endif
}

inline void *wxUIntToPtr(wxUIntPtr p)
{
#ifdef __VISUALC__
    #pragma warning(push)
    /* conversion to type of greater size */
    #pragma warning(disable: 4312)
#elif defined(__INTELC__)
    #pragma warning(push)
    /* invalid type conversion: "wxUIntPtr={unsigned long}" to "void *" */
    #pragma warning(disable: 171)
#endif

    return reinterpret_cast<void *>(p);

#if defined(__VISUALC__) || defined(__INTELC__)
    #pragma warning(pop)
#endif
}
#endif /*__cplusplus*/

/*
    Some (non standard) compilers typedef wchar_t as an existing type instead
    of treating it as a real fundamental type, set wxWCHAR_T_IS_REAL_TYPE to 0
    for them and to 1 for all the others.
 */
#ifndef wxWCHAR_T_IS_REAL_TYPE
    /*
        VC++ typedefs wchar_t as unsigned short by default until VC8, that is
        unless /Za or /Zc:wchar_t option is used in which case _WCHAR_T_DEFINED
        is defined.
     */
#   if defined(__VISUALC__) && !defined(_NATIVE_WCHAR_T_DEFINED)
#       define wxWCHAR_T_IS_REAL_TYPE 0
#   else /* compiler having standard-conforming wchar_t */
#       define wxWCHAR_T_IS_REAL_TYPE 1
#   endif
#endif /* !defined(wxWCHAR_T_IS_REAL_TYPE) */

/* Helper macro for doing something dependent on whether wchar_t is or isn't a
   typedef inside another macro. */
#if wxWCHAR_T_IS_REAL_TYPE
    #define wxIF_WCHAR_T_TYPE(x) x
#else /* !wxWCHAR_T_IS_REAL_TYPE */
    #define wxIF_WCHAR_T_TYPE(x)
#endif /* wxWCHAR_T_IS_REAL_TYPE/!wxWCHAR_T_IS_REAL_TYPE */

/* Define wxChar16 and wxChar32                                              */

#if SIZEOF_WCHAR_T == 2
    #define wxWCHAR_T_IS_WXCHAR16
    typedef wchar_t wxChar16;
#else
    typedef uint16_t wxChar16;
#endif

#if SIZEOF_WCHAR_T == 4
    #define wxWCHAR_T_IS_WXCHAR32
    typedef wchar_t wxChar32;
#else
    typedef uint32_t wxChar32;
#endif


/*
    Helper macro expanding into the given "m" macro invoked with each of the
    integer types as parameter (notice that this does not include char/unsigned
    char and bool but does include wchar_t).
 */
#define wxDO_FOR_INT_TYPES(m) \
    m(short) \
    m(unsigned short) \
    m(int) \
    m(unsigned int) \
    m(long) \
    m(unsigned long) \
    wxIF_LONG_LONG_TYPE( m(wxLongLong_t) ) \
    wxIF_LONG_LONG_TYPE( m(wxULongLong_t) ) \
    wxIF_WCHAR_T_TYPE( m(wchar_t) )

/*
    Same as wxDO_FOR_INT_TYPES() but does include char and unsigned char.

    Notice that we use "char" and "unsigned char" here but not "signed char"
    which would be more correct as "char" could be unsigned by default. But
    wxWidgets code currently supposes that char is signed and we'd need to
    clean up assumptions about it, notably in wx/unichar.h, to be able to use
    "signed char" here.
 */
#define wxDO_FOR_CHAR_INT_TYPES(m) \
    m(char) \
    m(unsigned char) \
    wxDO_FOR_INT_TYPES(m)

/*
    Same as wxDO_FOR_INT_TYPES() above except that m macro takes the
    type as the first argument and some extra argument, passed from this macro
    itself, as the second one.
 */
#define wxDO_FOR_INT_TYPES_1(m, arg) \
    m(short, arg) \
    m(unsigned short, arg) \
    m(int, arg) \
    m(unsigned int, arg) \
    m(long, arg) \
    m(unsigned long, arg) \
    wxIF_LONG_LONG_TYPE( m(wxLongLong_t, arg) ) \
    wxIF_LONG_LONG_TYPE( m(wxULongLong_t, arg) ) \
    wxIF_WCHAR_T_TYPE( m(wchar_t, arg) )

/*
    Combination of wxDO_FOR_CHAR_INT_TYPES() and wxDO_FOR_INT_TYPES_1():
    invokes the given macro with the specified argument as its second parameter
    for all char and int types.
 */
#define wxDO_FOR_CHAR_INT_TYPES_1(m, arg) \
    m(char, arg) \
    m(unsigned char, arg) \
    wxDO_FOR_INT_TYPES_1(m, arg)


/*  ---------------------------------------------------------------------------- */
/*  byte ordering related definition and macros */
/*  ---------------------------------------------------------------------------- */

/*  byte sex */

#define  wxBIG_ENDIAN     4321
#define  wxLITTLE_ENDIAN  1234
#define  wxPDP_ENDIAN     3412

#ifdef WORDS_BIGENDIAN
#define  wxBYTE_ORDER  wxBIG_ENDIAN
#else
#define  wxBYTE_ORDER  wxLITTLE_ENDIAN
#endif

/*  byte swapping */

#define wxUINT16_SWAP_ALWAYS(val) \
   ((uint16_t) ( \
    (((uint16_t) (val) & (uint16_t) 0x00ffU) << 8) | \
    (((uint16_t) (val) & (uint16_t) 0xff00U) >> 8)))

#define wxINT16_SWAP_ALWAYS(val) \
   ((int16_t) ( \
    (((uint16_t) (val) & (uint16_t) 0x00ffU) << 8) | \
    (((uint16_t) (val) & (uint16_t) 0xff00U) >> 8)))

#define wxUINT32_SWAP_ALWAYS(val) \
   ((uint32_t) ( \
    (((uint32_t) (val) & (uint32_t) 0x000000ffU) << 24) | \
    (((uint32_t) (val) & (uint32_t) 0x0000ff00U) <<  8) | \
    (((uint32_t) (val) & (uint32_t) 0x00ff0000U) >>  8) | \
    (((uint32_t) (val) & (uint32_t) 0xff000000U) >> 24)))

#define wxINT32_SWAP_ALWAYS(val) \
   ((int32_t) ( \
    (((uint32_t) (val) & (uint32_t) 0x000000ffU) << 24) | \
    (((uint32_t) (val) & (uint32_t) 0x0000ff00U) <<  8) | \
    (((uint32_t) (val) & (uint32_t) 0x00ff0000U) >>  8) | \
    (((uint32_t) (val) & (uint32_t) 0xff000000U) >> 24)))
/*  machine specific byte swapping */

#define wxUINT64_SWAP_ALWAYS(val) \
    ((uint64_t) ( \
    (((uint64_t) (val) & (uint64_t) 0x00000000000000ff) << 56) | \
    (((uint64_t) (val) & (uint64_t) 0x000000000000ff00) << 40) | \
    (((uint64_t) (val) & (uint64_t) 0x0000000000ff0000) << 24) | \
    (((uint64_t) (val) & (uint64_t) 0x00000000ff000000) <<  8) | \
    (((uint64_t) (val) & (uint64_t) 0x000000ff00000000) >>  8) | \
    (((uint64_t) (val) & (uint64_t) 0x0000ff0000000000) >> 24) | \
    (((uint64_t) (val) & (uint64_t) 0x00ff000000000000) >> 40) | \
    (((uint64_t) (val) & (uint64_t) 0xff00000000000000) >> 56)))

#define wxINT64_SWAP_ALWAYS(val) \
    ((int64_t) ( \
    (((uint64_t) (val) & (uint64_t) 0x00000000000000ff) << 56) | \
    (((uint64_t) (val) & (uint64_t) 0x000000000000ff00) << 40) | \
    (((uint64_t) (val) & (uint64_t) 0x0000000000ff0000) << 24) | \
    (((uint64_t) (val) & (uint64_t) 0x00000000ff000000) <<  8) | \
    (((uint64_t) (val) & (uint64_t) 0x000000ff00000000) >>  8) | \
    (((uint64_t) (val) & (uint64_t) 0x0000ff0000000000) >> 24) | \
    (((uint64_t) (val) & (uint64_t) 0x00ff000000000000) >> 40) | \
    (((uint64_t) (val) & (uint64_t) 0xff00000000000000) >> 56)))

#ifdef WORDS_BIGENDIAN
    #define wxUINT16_SWAP_ON_BE(val)  wxUINT16_SWAP_ALWAYS(val)
    #define wxINT16_SWAP_ON_BE(val)   wxINT16_SWAP_ALWAYS(val)
    #define wxUINT16_SWAP_ON_LE(val)  (val)
    #define wxINT16_SWAP_ON_LE(val)   (val)
    #define wxUINT32_SWAP_ON_BE(val)  wxUINT32_SWAP_ALWAYS(val)
    #define wxINT32_SWAP_ON_BE(val)   wxINT32_SWAP_ALWAYS(val)
    #define wxUINT32_SWAP_ON_LE(val)  (val)
    #define wxINT32_SWAP_ON_LE(val)   (val)
    #if wxHAS_INT64
        #define wxUINT64_SWAP_ON_BE(val)  wxUINT64_SWAP_ALWAYS(val)
        #define wxUINT64_SWAP_ON_LE(val)  (val)
        #define wxINT64_SWAP_ON_BE(val)  wxINT64_SWAP_ALWAYS(val)
        #define wxINT64_SWAP_ON_LE(val)  (val)

        #define wxUINT64_SWAP_ON_BE_IN_PLACE(val)   val = wxUINT64_SWAP_ALWAYS(val)
        #define wxINT64_SWAP_ON_BE_IN_PLACE(val)   val = wxINT64_SWAP_ALWAYS(val)
        #define wxUINT64_SWAP_ON_LE_IN_PLACE(val)
        #define wxINT64_SWAP_ON_LE_IN_PLACE(val)
    #endif

    #define wxUINT16_SWAP_ON_BE_IN_PLACE(val)   val = wxUINT16_SWAP_ALWAYS(val)
    #define wxINT16_SWAP_ON_BE_IN_PLACE(val)   val = wxINT16_SWAP_ALWAYS(val)
    #define wxUINT16_SWAP_ON_LE_IN_PLACE(val)
    #define wxINT16_SWAP_ON_LE_IN_PLACE(val)
    #define wxUINT32_SWAP_ON_BE_IN_PLACE(val)   val = wxUINT32_SWAP_ALWAYS(val)
    #define wxINT32_SWAP_ON_BE_IN_PLACE(val)   val = wxINT32_SWAP_ALWAYS(val)
    #define wxUINT32_SWAP_ON_LE_IN_PLACE(val)
    #define wxINT32_SWAP_ON_LE_IN_PLACE(val)
#else
    #define wxUINT16_SWAP_ON_LE(val)  wxUINT16_SWAP_ALWAYS(val)
    #define wxINT16_SWAP_ON_LE(val)   wxINT16_SWAP_ALWAYS(val)
    #define wxUINT16_SWAP_ON_BE(val)  (val)
    #define wxINT16_SWAP_ON_BE(val)   (val)
    #define wxUINT32_SWAP_ON_LE(val)  wxUINT32_SWAP_ALWAYS(val)
    #define wxINT32_SWAP_ON_LE(val)   wxINT32_SWAP_ALWAYS(val)
    #define wxUINT32_SWAP_ON_BE(val)  (val)
    #define wxINT32_SWAP_ON_BE(val)   (val)
    #if wxHAS_INT64
        #define wxUINT64_SWAP_ON_LE(val)  wxUINT64_SWAP_ALWAYS(val)
        #define wxUINT64_SWAP_ON_BE(val)  (val)
        #define wxINT64_SWAP_ON_LE(val)  wxINT64_SWAP_ALWAYS(val)
        #define wxINT64_SWAP_ON_BE(val)  (val)
        #define wxUINT64_SWAP_ON_BE_IN_PLACE(val)
        #define wxINT64_SWAP_ON_BE_IN_PLACE(val)
        #define wxUINT64_SWAP_ON_LE_IN_PLACE(val)   val = wxUINT64_SWAP_ALWAYS(val)
        #define wxINT64_SWAP_ON_LE_IN_PLACE(val)   val = wxINT64_SWAP_ALWAYS(val)
    #endif

    #define wxUINT16_SWAP_ON_BE_IN_PLACE(val)
    #define wxINT16_SWAP_ON_BE_IN_PLACE(val)
    #define wxUINT16_SWAP_ON_LE_IN_PLACE(val)   val = wxUINT16_SWAP_ALWAYS(val)
    #define wxINT16_SWAP_ON_LE_IN_PLACE(val)   val = wxINT16_SWAP_ALWAYS(val)
    #define wxUINT32_SWAP_ON_BE_IN_PLACE(val)
    #define wxINT32_SWAP_ON_BE_IN_PLACE(val)
    #define wxUINT32_SWAP_ON_LE_IN_PLACE(val)   val = wxUINT32_SWAP_ALWAYS(val)
    #define wxINT32_SWAP_ON_LE_IN_PLACE(val)   val = wxINT32_SWAP_ALWAYS(val)
#endif

/*  ---------------------------------------------------------------------------- */
/*  Geometric flags */
/*  ---------------------------------------------------------------------------- */

/*
    In C++20 operations on the elements of different enums are deprecated and
    many compilers (clang 10+, gcc 11+, MSVS 2019) warn about combining them,
    as a lot of existing code using them does, so we provide explicit operators
    for doing this, that do the same thing as would happen without them, but
    without the warnings.
 */
#if defined(__cplusplus) && (__cplusplus >= 202002L)
    #define wxALLOW_COMBINING_ENUMS_IMPL(en1, en2)                            \
        inline int operator|(en1 v1, en2 v2)                                  \
            { return static_cast<int>(v1) | static_cast<int>(v2); }           \
        inline int operator+(en1 v1, en2 v2)                                  \
            { return static_cast<int>(v1) + static_cast<int>(v2); }

    #define wxALLOW_COMBINING_ENUMS(en1, en2)                                 \
        wxALLOW_COMBINING_ENUMS_IMPL(en1, en2)                                \
        wxALLOW_COMBINING_ENUMS_IMPL(en2, en1)
#else /* !C++ 20 */
    /* Don't bother doing anything in this case. */
    #define wxALLOW_COMBINING_ENUMS(en1, en2)
#endif /* C++ 20 */
/*  ---------------------------------------------------------------------------- */
/*  GDI descriptions */
/*  ---------------------------------------------------------------------------- */

/* Key codes */
enum wxKeyCode
{
    WXK_NONE    =    0,

    WXK_CONTROL_A = 1,
    WXK_CONTROL_B,
    WXK_CONTROL_C,
    WXK_CONTROL_D,
    WXK_CONTROL_E,
    WXK_CONTROL_F,
    WXK_CONTROL_G,
    WXK_CONTROL_H,
    WXK_CONTROL_I,
    WXK_CONTROL_J,
    WXK_CONTROL_K,
    WXK_CONTROL_L,
    WXK_CONTROL_M,
    WXK_CONTROL_N,
    WXK_CONTROL_O,
    WXK_CONTROL_P,
    WXK_CONTROL_Q,
    WXK_CONTROL_R,
    WXK_CONTROL_S,
    WXK_CONTROL_T,
    WXK_CONTROL_U,
    WXK_CONTROL_V,
    WXK_CONTROL_W,
    WXK_CONTROL_X,
    WXK_CONTROL_Y,
    WXK_CONTROL_Z,

    WXK_BACK    =    8, /* backspace */
    WXK_TAB     =    9,
    WXK_RETURN  =    13,
    WXK_ESCAPE  =    27,

    /* values from 33 to 126 are reserved for the standard ASCII characters */

    WXK_SPACE   =    32,
    WXK_DELETE  =    127,

    /* values from 128 to 255 are reserved for ASCII extended characters
       (note that there isn't a single fixed standard for the meaning
       of these values; avoid them in portable apps!) */

    /* These are not compatible with unicode characters.
       If you want to get a unicode character from a key event, use
       wxKeyEvent::GetUnicodeKey                                    */
    WXK_START   = 300,
    WXK_LBUTTON,
    WXK_RBUTTON,
    WXK_CANCEL,
    WXK_MBUTTON,
    WXK_CLEAR,
    WXK_SHIFT,
    WXK_ALT,
    WXK_CONTROL,
    WXK_MENU,
    WXK_PAUSE,
    WXK_CAPITAL,
    WXK_END,
    WXK_HOME,
    WXK_LEFT,
    WXK_UP,
    WXK_RIGHT,
    WXK_DOWN,
    WXK_SELECT,
    WXK_PRINT,
    WXK_EXECUTE,
    WXK_SNAPSHOT,
    WXK_INSERT,
    WXK_HELP,
    WXK_NUMPAD0,
    WXK_NUMPAD1,
    WXK_NUMPAD2,
    WXK_NUMPAD3,
    WXK_NUMPAD4,
    WXK_NUMPAD5,
    WXK_NUMPAD6,
    WXK_NUMPAD7,
    WXK_NUMPAD8,
    WXK_NUMPAD9,
    WXK_MULTIPLY,
    WXK_ADD,
    WXK_SEPARATOR,
    WXK_SUBTRACT,
    WXK_DECIMAL,
    WXK_DIVIDE,
    WXK_F1,
    WXK_F2,
    WXK_F3,
    WXK_F4,
    WXK_F5,
    WXK_F6,
    WXK_F7,
    WXK_F8,
    WXK_F9,
    WXK_F10,
    WXK_F11,
    WXK_F12,
    WXK_F13,
    WXK_F14,
    WXK_F15,
    WXK_F16,
    WXK_F17,
    WXK_F18,
    WXK_F19,
    WXK_F20,
    WXK_F21,
    WXK_F22,
    WXK_F23,
    WXK_F24,
    WXK_NUMLOCK,
    WXK_SCROLL,
    WXK_PAGEUP,
    WXK_PAGEDOWN,
    WXK_NUMPAD_SPACE,
    WXK_NUMPAD_TAB,
    WXK_NUMPAD_ENTER,
    WXK_NUMPAD_F1,
    WXK_NUMPAD_F2,
    WXK_NUMPAD_F3,
    WXK_NUMPAD_F4,
    WXK_NUMPAD_HOME,
    WXK_NUMPAD_LEFT,
    WXK_NUMPAD_UP,
    WXK_NUMPAD_RIGHT,
    WXK_NUMPAD_DOWN,
    WXK_NUMPAD_PAGEUP,
    WXK_NUMPAD_PAGEDOWN,
    WXK_NUMPAD_END,
    WXK_NUMPAD_BEGIN,
    WXK_NUMPAD_INSERT,
    WXK_NUMPAD_DELETE,
    WXK_NUMPAD_EQUAL,
    WXK_NUMPAD_MULTIPLY,
    WXK_NUMPAD_ADD,
    WXK_NUMPAD_SEPARATOR,
    WXK_NUMPAD_SUBTRACT,
    WXK_NUMPAD_DECIMAL,
    WXK_NUMPAD_DIVIDE,

    WXK_WINDOWS_LEFT,
    WXK_WINDOWS_RIGHT,
    WXK_WINDOWS_MENU ,
#ifdef __WXOSX__
    WXK_RAW_CONTROL,
#else
    WXK_RAW_CONTROL = WXK_CONTROL,
#endif
    WXK_COMMAND = WXK_CONTROL,

    /* Hardware-specific buttons */
    WXK_SPECIAL1 = WXK_WINDOWS_MENU + 2, /* Skip WXK_RAW_CONTROL if necessary */
    WXK_SPECIAL2,
    WXK_SPECIAL3,
    WXK_SPECIAL4,
    WXK_SPECIAL5,
    WXK_SPECIAL6,
    WXK_SPECIAL7,
    WXK_SPECIAL8,
    WXK_SPECIAL9,
    WXK_SPECIAL10,
    WXK_SPECIAL11,
    WXK_SPECIAL12,
    WXK_SPECIAL13,
    WXK_SPECIAL14,
    WXK_SPECIAL15,
    WXK_SPECIAL16,
    WXK_SPECIAL17,
    WXK_SPECIAL18,
    WXK_SPECIAL19,
    WXK_SPECIAL20,

    WXK_BROWSER_BACK,
    WXK_BROWSER_FORWARD,
    WXK_BROWSER_REFRESH,
    WXK_BROWSER_STOP,
    WXK_BROWSER_SEARCH,
    WXK_BROWSER_FAVORITES,
    WXK_BROWSER_HOME,
    WXK_VOLUME_MUTE,
    WXK_VOLUME_DOWN,
    WXK_VOLUME_UP,
    WXK_MEDIA_NEXT_TRACK,
    WXK_MEDIA_PREV_TRACK,
    WXK_MEDIA_STOP,
    WXK_MEDIA_PLAY_PAUSE,
    WXK_LAUNCH_MAIL,
    WXK_LAUNCH_APP1,
    WXK_LAUNCH_APP2
};

/* Shortcut for easier dialog-unit-to-pixel conversion */
#define wxDLG_UNIT(parent, pt) parent->ConvertDialogToPixels(pt)

/*  ---------------------------------------------------------------------------- */
/*  miscellaneous */
/*  ---------------------------------------------------------------------------- */

/*  define this macro if font handling is done using the X font names */
#if (defined(__WXGTK__) && !defined(__WXGTK20__)) || defined(__X__)
    #define _WX_X_FONTLIKE
#endif

/*  macro to specify "All Files" on different platforms */
#if defined(__WXMSW__)
#   define wxALL_FILES_PATTERN   wxT("*.*")
#   define wxALL_FILES           gettext_noop("All files (*.*)|*.*")
#else
#   define wxALL_FILES_PATTERN   wxT("*")
#   define wxALL_FILES           gettext_noop("All files (*)|*")
#endif

#if defined(__CYGWIN__) && defined(__WXMSW__)
        /*
        NASTY HACK because the gethostname in sys/unistd.h which the gnu
        stl includes and wx builds with by default clash with each other
        (windows version 2nd param is int, sys/unistd.h version is unsigned
        int).
        */
#        define gethostname gethostnameHACK
#        include <unistd.h>
#        undef gethostname
#endif

/*  --------------------------------------------------------------------------- */
/*  macros that enable wxWidgets apps to be compiled in absence of the */
/*  system headers, although some platform specific types are used in the */
/*  platform specific (implementation) parts of the headers */
/*  --------------------------------------------------------------------------- */

#ifdef __DARWIN__
#define DECLARE_WXOSX_OPAQUE_CFREF( name ) typedef struct __##name* name##Ref;
#define DECLARE_WXOSX_OPAQUE_CONST_CFREF( name ) typedef const struct __##name* name##Ref;

#endif

#ifdef __WXMAC__

#define WX_OPAQUE_TYPE( name ) struct wxOpaque##name

typedef void*       WXHCURSOR;
typedef void*       WXRECTPTR;
typedef void*       WXPOINTPTR;
typedef void*       WXHWND;
typedef void*       WXEVENTREF;
typedef void*       WXEVENTHANDLERREF;
typedef void*       WXEVENTHANDLERCALLREF;
typedef void*       WXAPPLEEVENTREF;

typedef unsigned int    WXUINT;
typedef unsigned long   DWORD;
typedef unsigned short  WXWORD;

typedef WX_OPAQUE_TYPE(PicHandle ) * WXHMETAFILE ;

typedef void*       WXDisplay;

/*
 * core frameworks
 */

#if __has_attribute(objc_bridge) && __has_feature(objc_bridge_id) && __has_feature(objc_bridge_id_on_typedefs)

#ifdef __OBJC__
@class NSArray;
@class NSString;
@class NSData;
@class NSDictionary;
#endif

#define WXOSX_BRIDGED_TYPE(T)		__attribute__((objc_bridge(T)))
#define WXOSX_BRIDGED_MUTABLE_TYPE(T)	__attribute__((objc_bridge_mutable(T)))

#else

#define WXOSX_BRIDGED_TYPE(T)
#define WXOSX_BRIDGED_MUTABLE_TYPE(T)

#endif

#define DECLARE_WXOSX_BRIDGED_TYPE_AND_CFREF( name ) \
    typedef const struct WXOSX_BRIDGED_TYPE(NS##name) __CF##name* CF##name##Ref;
#define DECLARE_WXOSX_BRIDGED_MUTABLE_TYPE_AND_CFREF( name ) \
    typedef struct WXOSX_BRIDGED_MUTABLE_TYPE(NSMutable##name) __CF##name* CFMutable##name##Ref;

typedef const WXOSX_BRIDGED_TYPE(id) void * CFTypeRef;

DECLARE_WXOSX_BRIDGED_TYPE_AND_CFREF( Data )
DECLARE_WXOSX_BRIDGED_MUTABLE_TYPE_AND_CFREF( Data )

DECLARE_WXOSX_BRIDGED_TYPE_AND_CFREF( String )
DECLARE_WXOSX_BRIDGED_MUTABLE_TYPE_AND_CFREF( String )

DECLARE_WXOSX_BRIDGED_TYPE_AND_CFREF( Dictionary )
DECLARE_WXOSX_BRIDGED_MUTABLE_TYPE_AND_CFREF( Dictionary )

DECLARE_WXOSX_BRIDGED_TYPE_AND_CFREF( Array )
DECLARE_WXOSX_BRIDGED_MUTABLE_TYPE_AND_CFREF( Array )

DECLARE_WXOSX_OPAQUE_CFREF( CFRunLoopSource )
DECLARE_WXOSX_OPAQUE_CONST_CFREF( CTFont )
DECLARE_WXOSX_OPAQUE_CONST_CFREF( CTFontDescriptor )

#define DECLARE_WXOSX_OPAQUE_CGREF( name ) typedef struct name* name##Ref;

DECLARE_WXOSX_OPAQUE_CGREF( CGColor )
DECLARE_WXOSX_OPAQUE_CGREF( CGImage )
DECLARE_WXOSX_OPAQUE_CGREF( CGContext )
DECLARE_WXOSX_OPAQUE_CGREF( CGFont )

typedef CGColorRef    WXCOLORREF;
typedef CGImageRef    WXCGIMAGEREF;
typedef CGContextRef  WXHDC;
typedef CGContextRef  WXHBITMAP;

/*
 * carbon
 */

typedef const struct __HIShape * HIShapeRef;
typedef struct __HIShape * HIMutableShapeRef;

#define DECLARE_WXMAC_OPAQUE_REF( name ) typedef struct Opaque##name* name;

DECLARE_WXMAC_OPAQUE_REF( PasteboardRef )
DECLARE_WXMAC_OPAQUE_REF( IconRef )
DECLARE_WXMAC_OPAQUE_REF( MenuRef )

typedef IconRef WXHICON ;
typedef HIShapeRef WXHRGN;

#endif // __WXMAC__

#if defined(__WXMAC__)

/* Definitions of 32-bit/64-bit types
 * These are typedef'd exactly the same way in newer OS X headers so
 * redefinition when real headers are included should not be a problem.  If
 * it is, the types are being defined wrongly here.
 * The purpose of these types is so they can be used from public wx headers.
 * and also because the older (pre-Leopard) headers don't define them.
 */

/* NOTE: We don't pollute namespace with CGFLOAT_MIN/MAX/IS_DOUBLE macros
 * since they are unlikely to be needed in a public header.
 */
#if defined(__LP64__) && __LP64__
    typedef double CGFloat;
#else
    typedef float CGFloat;
#endif

#if (defined(__LP64__) && __LP64__) || (defined(NS_BUILD_32_LIKE_64) && NS_BUILD_32_LIKE_64)
typedef long NSInteger;
typedef unsigned long NSUInteger;
#else
typedef int NSInteger;
typedef unsigned int NSUInteger;
#endif

/* Objective-C type declarations.
 * These are to be used in public headers in lieu of NSSomething* because
 * Objective-C class names are not available in C/C++ code.
 */

/*  NOTE: This ought to work with other compilers too, but I'm being cautious */
#if (defined(__GNUC__) && defined(__APPLE__))
/* It's desirable to have type safety for Objective-C(++) code as it does
at least catch typos of method names among other things.  However, it
is not possible to declare an Objective-C class from plain old C or C++
code.  Furthermore, because of C++ name mangling, the type name must
be the same for both C++ and Objective-C++ code.  Therefore, we define
what should be a pointer to an Objective-C class as a pointer to a plain
old C struct with the same name.  Unfortunately, because the compiler
does not see a struct as an Objective-C class we cannot declare it
as a struct in Objective-C(++) mode.
*/
#if defined(__OBJC__)
#define DECLARE_WXCOCOA_OBJC_CLASS(klass) \
@class klass; \
typedef klass *WX_##klass
#else /*  not defined(__OBJC__) */
#define DECLARE_WXCOCOA_OBJC_CLASS(klass) \
typedef struct klass *WX_##klass
#endif /*  defined(__OBJC__) */

#else /*  not Apple's gcc */
#warning "Objective-C types will not be checked by the compiler."
/*  NOTE: typedef struct objc_object *id; */
/*  IOW, we're declaring these using the id type without using that name, */
/*  since "id" is used extensively not only within wxWidgets itself, but */
/*  also in wxWidgets application code.  The following works fine when */
/*  compiling C(++) code, and works without typesafety for Obj-C(++) code */
#define DECLARE_WXCOCOA_OBJC_CLASS(klass) \
typedef struct objc_object *WX_##klass

#endif /*  (defined(__GNUC__) && defined(__APPLE__)) */

DECLARE_WXCOCOA_OBJC_CLASS(NSArray);
DECLARE_WXCOCOA_OBJC_CLASS(NSData);
DECLARE_WXCOCOA_OBJC_CLASS(NSMutableArray);
DECLARE_WXCOCOA_OBJC_CLASS(NSString);
DECLARE_WXCOCOA_OBJC_CLASS(NSObject);

#if wxOSX_USE_COCOA

DECLARE_WXCOCOA_OBJC_CLASS(NSApplication);
DECLARE_WXCOCOA_OBJC_CLASS(NSBitmapImageRep);
DECLARE_WXCOCOA_OBJC_CLASS(NSBox);
DECLARE_WXCOCOA_OBJC_CLASS(NSButton);
DECLARE_WXCOCOA_OBJC_CLASS(NSColor);
DECLARE_WXCOCOA_OBJC_CLASS(NSColorPanel);
DECLARE_WXCOCOA_OBJC_CLASS(NSControl);
DECLARE_WXCOCOA_OBJC_CLASS(NSCursor);
DECLARE_WXCOCOA_OBJC_CLASS(NSEvent);
DECLARE_WXCOCOA_OBJC_CLASS(NSFont);
DECLARE_WXCOCOA_OBJC_CLASS(NSFontDescriptor);
DECLARE_WXCOCOA_OBJC_CLASS(NSFontPanel);
DECLARE_WXCOCOA_OBJC_CLASS(NSImage);
DECLARE_WXCOCOA_OBJC_CLASS(NSLayoutManager);
DECLARE_WXCOCOA_OBJC_CLASS(NSMenu);
DECLARE_WXCOCOA_OBJC_CLASS(NSMenuExtra);
DECLARE_WXCOCOA_OBJC_CLASS(NSMenuItem);
DECLARE_WXCOCOA_OBJC_CLASS(NSNotification);
DECLARE_WXCOCOA_OBJC_CLASS(NSPanel);
DECLARE_WXCOCOA_OBJC_CLASS(NSResponder);
DECLARE_WXCOCOA_OBJC_CLASS(NSScrollView);
DECLARE_WXCOCOA_OBJC_CLASS(NSSound);
DECLARE_WXCOCOA_OBJC_CLASS(NSStatusItem);
DECLARE_WXCOCOA_OBJC_CLASS(NSTableColumn);
DECLARE_WXCOCOA_OBJC_CLASS(NSTableView);
DECLARE_WXCOCOA_OBJC_CLASS(NSTextContainer);
DECLARE_WXCOCOA_OBJC_CLASS(NSTextField);
DECLARE_WXCOCOA_OBJC_CLASS(NSTextStorage);
DECLARE_WXCOCOA_OBJC_CLASS(NSThread);
DECLARE_WXCOCOA_OBJC_CLASS(NSWindow);
DECLARE_WXCOCOA_OBJC_CLASS(NSView);
DECLARE_WXCOCOA_OBJC_CLASS(NSOpenGLContext);
DECLARE_WXCOCOA_OBJC_CLASS(NSOpenGLPixelFormat);
DECLARE_WXCOCOA_OBJC_CLASS(NSPrintInfo);
DECLARE_WXCOCOA_OBJC_CLASS(NSGestureRecognizer);
DECLARE_WXCOCOA_OBJC_CLASS(NSPanGestureRecognizer);
DECLARE_WXCOCOA_OBJC_CLASS(NSMagnificationGestureRecognizer);
DECLARE_WXCOCOA_OBJC_CLASS(NSRotationGestureRecognizer);
DECLARE_WXCOCOA_OBJC_CLASS(NSPressGestureRecognizer);
DECLARE_WXCOCOA_OBJC_CLASS(NSTouch);
DECLARE_WXCOCOA_OBJC_CLASS(NSPasteboard);
DECLARE_WXCOCOA_OBJC_CLASS(WKWebView);

typedef WX_NSWindow WXWindow;
typedef WX_NSView WXWidget;
typedef WX_NSImage WXImage;
typedef WX_NSMenu WXHMENU;
typedef WX_NSOpenGLPixelFormat WXGLPixelFormat;
typedef WX_NSOpenGLContext WXGLContext;
typedef WX_NSPasteboard OSXPasteboard;
typedef WX_WKWebView OSXWebViewPtr;

#elif wxOSX_USE_IPHONE

DECLARE_WXCOCOA_OBJC_CLASS(UIMenu);
DECLARE_WXCOCOA_OBJC_CLASS(UIMenuItem);
DECLARE_WXCOCOA_OBJC_CLASS(UIWindow);
DECLARE_WXCOCOA_OBJC_CLASS(UImage);
DECLARE_WXCOCOA_OBJC_CLASS(UIView);
DECLARE_WXCOCOA_OBJC_CLASS(UIFont);
DECLARE_WXCOCOA_OBJC_CLASS(UIImage);
DECLARE_WXCOCOA_OBJC_CLASS(UIEvent);
DECLARE_WXCOCOA_OBJC_CLASS(NSSet);
DECLARE_WXCOCOA_OBJC_CLASS(EAGLContext);
DECLARE_WXCOCOA_OBJC_CLASS(UIPasteboard);

typedef WX_UIWindow WXWindow;
typedef WX_UIView WXWidget;
typedef WX_UIImage WXImage;
typedef WX_UIMenu WXHMENU;
typedef WX_EAGLContext WXGLContext;
typedef WX_NSString WXGLPixelFormat;
typedef WX_UIPasteboard WXOSXPasteboard;

#endif



#endif /* __WXMAC__ */

/* ABX: check __WIN32__ instead of __WXMSW__ for the same MSWBase in any Win32 port */
#if defined(__WIN32__)

/*  Stand-ins for Windows types to avoid #including all of windows.h */

#ifndef NO_STRICT
    #define WX_MSW_DECLARE_HANDLE(type) typedef struct type##__ * WX##type
#else
    #define WX_MSW_DECLARE_HANDLE(type) typedef void * WX##type
#endif

WX_MSW_DECLARE_HANDLE(RECTPTR);

#undef WX_MSW_DECLARE_HANDLE

typedef unsigned long   WXCOLORREF;
typedef void *          WXRGNDATA;
typedef void *          WXHCONV;
typedef void *          WXHKEY;
typedef void *          WXHTREEITEM;

#ifndef __WIN64__
typedef wxW64 unsigned int WXWPARAM;
typedef wxW64 long         WXLPARAM;
typedef wxW64 long         WXLRESULT;
#endif

/*
   This is defined for compatibility only, it's not really the same thing as
   FARPROC.
 */
#if defined(__GNUWIN32__)
typedef int             (*WXFARPROC)();
#else
typedef int             (__stdcall *WXFARPROC)();
#endif

#endif /*  __WIN32__ */


#if defined(__WXMOTIF__) || defined(__WXX11__)
/* Stand-ins for X/Xt/Motif types */
typedef void*           WXWindow;
typedef void*           WXWidget;
typedef void*           WXAppContext;
typedef void*           WXColormap;
typedef void*           WXColor;
typedef void            WXDisplay;
typedef void            WXEvent;
typedef void*           WXCursor;
typedef void*           WXPixmap;
typedef void*           WXFontStructPtr;
typedef void*           WXGC;
typedef void*           WXRegion;
typedef void*           WXFont;
typedef void*           WXImage;
typedef void*           WXFontList;
typedef void*           WXFontSet;
typedef void*           WXRendition;
typedef void*           WXRenderTable;
typedef void*           WXFontType; /* either a XmFontList or XmRenderTable */
typedef void*           WXString;

typedef unsigned long   Atom;  /* this might fail on a few architectures */
typedef long            WXPixel; /* safety catch in src/motif/colour.cpp */

#endif /*  Motif */

#ifdef __WXGTK__

/* Stand-ins for GLIB types */
typedef struct _GSList GSList;

/* Stand-ins for GDK types */
typedef struct _GdkColor        GdkColor;
typedef struct _GdkCursor       GdkCursor;
typedef struct _GdkDragContext  GdkDragContext;

#if defined(__WXGTK20__)
    typedef struct _GdkAtom* GdkAtom;
#else
    typedef unsigned long GdkAtom;
#endif

#if !defined(__WXGTK3__)
    typedef struct _GdkColormap GdkColormap;
    typedef struct _GdkFont GdkFont;
    typedef struct _GdkGC GdkGC;
    typedef struct _GdkRegion GdkRegion;
#endif

#if defined(__WXGTK3__)
    typedef struct _GdkWindow GdkWindow;
    typedef struct _GdkEventSequence GdkEventSequence;
#elif defined(__WXGTK20__)
    typedef struct _GdkDrawable GdkWindow;
    typedef struct _GdkDrawable GdkPixmap;
#else
    typedef struct _GdkWindow GdkWindow;
    typedef struct _GdkWindow GdkBitmap;
    typedef struct _GdkWindow GdkPixmap;
#endif

/* Stand-ins for GTK types */
typedef struct _GtkWidget         GtkWidget;
typedef struct _GtkRcStyle        GtkRcStyle;
typedef struct _GtkAdjustment     GtkAdjustment;
typedef struct _GtkToolbar        GtkToolbar;
typedef struct _GtkNotebook       GtkNotebook;
typedef struct _GtkNotebookPage   GtkNotebookPage;
typedef struct _GtkAccelGroup     GtkAccelGroup;
typedef struct _GtkSelectionData  GtkSelectionData;
typedef struct _GtkTextBuffer     GtkTextBuffer;
typedef struct _GtkRange          GtkRange;
typedef struct _GtkCellRenderer   GtkCellRenderer;

typedef GtkWidget *WXWidget;

#ifndef __WXGTK20__
#define GTK_OBJECT_GET_CLASS(object) (GTK_OBJECT(object)->klass)
#define GTK_CLASS_TYPE(klass) ((klass)->type)
#endif

#endif /*  __WXGTK__ */

#if defined(__WXGTK20__) || (defined(__WXX11__))
#define wxUSE_PANGO 1
#else
#define wxUSE_PANGO 0
#endif

#if wxUSE_PANGO
/* Stand-ins for Pango types */
typedef struct _PangoContext         PangoContext;
typedef struct _PangoLayout          PangoLayout;
typedef struct _PangoFontDescription PangoFontDescription;
#endif

#ifdef __WXDFB__
/* DirectFB doesn't have the concept of non-TLW window, so use
   something arbitrary */
typedef const void* WXWidget;
#endif /*  DFB */

#ifdef __WXQT__
#include "wx/qt/defs.h"
#endif

/*  include the feature test macros */
#include "wx/features.h"

/* wxThread and wxProcess priorities */
enum
{
    wxPRIORITY_MIN     = 0u,   /* lowest possible priority */
    wxPRIORITY_DEFAULT = 50u,  /* normal priority */
    wxPRIORITY_MAX     = 100u  /* highest possible priority */
};

#endif
    /*  _WX_DEFS_H_ */
