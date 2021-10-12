/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/arrstr.cpp
// Purpose:     wxArrayString class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     29/01/98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ===========================================================================
// headers, declarations, constants
// ===========================================================================

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#include "wx/arrstr.h"
#include "wx/scopedarray.h"
#include "wx/wxcrt.h"

#ifndef WX_PRECOMP
    #include <algorithm>
    #include <functional>
#endif

#if defined( __WINDOWS__ )
    #include <shlwapi.h>

    // In some distributions of MinGW32, this function is exported in the library,
    // but not declared in shlwapi.h. Therefore we declare it here.
    #if defined( __MINGW32_TOOLCHAIN__ )
        extern "C" __declspec(dllimport) int WINAPI StrCmpLogicalW(LPCWSTR psz1, LPCWSTR psz2);
    #endif

    // For MSVC we can also link the library containing StrCmpLogicalW()
    // directly from here, for the other compilers this needs to be done at
    // makefiles level.
    #ifdef __VISUALC__
        #pragma comment(lib, "shlwapi")
    #endif
#endif

// ============================================================================
// ArrayString
// ============================================================================

wxArrayString::wxArrayString(size_t sz, const char** a)
{
    assign(a, a + sz);
}

wxArrayString::wxArrayString(size_t sz, const wchar_t** a)
{
    assign(a, a + sz);
}

wxArrayString::wxArrayString(size_t sz, const wxString* a)
{
    assign(a, a + sz);
}

#include "wx/arrstr.h"

int wxArrayString::Index(const wxString& str, bool bCase, bool WXUNUSED(bFromEnd)) const
{
    int n = 0;
    for ( const auto& s: *this )
    {
        if ( s.IsSameAs(str, bCase) )
            return n;

        ++n;
    }

    return wxNOT_FOUND;
}

void wxArrayString::Sort(CompareFunction function)
{
    std::sort(begin(), end(),
              [function](const wxString& s1, const wxString& s2)
              {
                  return function(s1, s2) < 0;
              }
             );
}

void wxArrayString::Sort(bool reverseOrder)
{
    if (reverseOrder)
    {
        std::sort(begin(), end(), std::greater<wxString>());
    }
    else
    {
        std::sort(begin(), end());
    }
}

int wxSortedArrayString::Index(const wxString& str,
                               bool WXUNUSED_UNLESS_DEBUG(bCase),
                               bool WXUNUSED_UNLESS_DEBUG(bFromEnd)) const
{
    wxASSERT_MSG( bCase && !bFromEnd,
                  "search parameters ignored for sorted array" );

    SCMPFUNC function = GetCompareFunction();
    wxSortedArrayString::const_iterator
        it = std::lower_bound(begin(), end(), str,
                              [function](const wxString& s1, const wxString& s2)
                              {
                                  return function(s1, s2) < 0;
                              }
                              );

    if ( it == end() || str.Cmp(*it) != 0 )
        return wxNOT_FOUND;

    return it - begin();
}

// ===========================================================================
// wxJoin and wxSplit
// ===========================================================================

#include "wx/tokenzr.h"

wxString wxJoin(const std::vector<wxString>& arr, const wxChar sep, const wxChar escape)
{
    wxString str;

    size_t count = arr.size();
    if ( count == 0 )
        return str;

    // pre-allocate memory using the estimation of the average length of the
    // strings in the given array: this is very imprecise, of course, but
    // better than nothing
    str.reserve(count*(arr[0].length() + arr[count-1].length()) / 2);

    if ( escape == wxT('\0') )
    {
        // escaping is disabled:
        for ( size_t i = 0; i < count; i++ )
        {
            if ( i )
                str += sep;
            str += arr[i];
        }
    }
    else // use escape character
    {
        for ( size_t n = 0; n < count; n++ )
        {
            if ( n )
            {
                // We don't escape the escape characters in the middle of the
                // string because this is not needed, strictly speaking, but we
                // must do it if they occur at the end because otherwise we
                // wouldn't split the string back correctly as the separator
                // would appear to be escaped.
                if ( !str.empty() && *str.rbegin() == escape )
                    str += escape;

                str += sep;
            }

            for ( wxString::const_iterator i = arr[n].begin(),
                                         end = arr[n].end();
                  i != end;
                  ++i )
            {
                const wxChar ch = *i;
                if ( ch == sep )
                    str += escape;      // escape this separator
                str += ch;
            }
        }
    }

    str.Shrink(); // release extra memory if we allocated too much
    return str;
}

std::vector<wxString> wxSplit(const wxString& str, const wxChar sep, const wxChar escape)
{
    if ( escape == wxT('\0') )
    {
        // simple case: we don't need to honour the escape character
        return wxStringTokenize(str, sep, wxStringTokenizerMode::RetEmptyAll);
    }

    std::vector<wxString> ret;
    wxString curr;

    for ( wxString::const_iterator i = str.begin(),
                                 end = str.end();
          i != end;
          ++i )
    {
        const wxChar ch = *i;

        // Order of tests matters here in the uncommon, but possible, case when
        // the separator is the same as the escape character: it has to be
        // recognized as a separator in this case (escaping doesn't work at all
        // in this case).
        if ( ch == sep )
        {
            ret.push_back(curr);
            curr.clear();
        }
        else if ( ch == escape )
        {
            ++i;
            if ( i == end )
            {
                // Escape at the end of the string is not handled specially.
                curr += ch;
                break;
            }

            // Separator or the escape character itself may be escaped,
            // cancelling their special meaning, but escape character followed
            // by anything else is not handled specially.
            if ( *i != sep && *i != escape )
                curr += ch;

            curr += *i;
        }
        else // normal character
        {
            curr += ch;
        }
    }

    // add the last token, which we always have unless the string is empty
    if ( !str.empty() )
        ret.push_back(curr);

    return ret;
}

namespace // helpers needed by wxCmpNaturalGeneric()
{
// Used for comparison of string parts
struct wxStringFragment
{
    // Fragment types are generally sorted like this:
    // Empty < SpaceOrPunct < Digit < LetterOrSymbol
    // Fragments of the same type are compared as follows:
    // SpaceOrPunct - collated, Digit - as numbers using value
    // LetterOrSymbol - lower-cased and then collated
    enum Type
    {
        Empty,
        SpaceOrPunct,  // whitespace or punctuation
        Digit,         // a sequence of decimal digits
        LetterOrSymbol // letters and symbols, i.e., anything not covered by the above types
    };

    wxString text;

    std::uint64_t value{0}; // used only for Digit type

    Type     type{ Empty };
};


wxStringFragment GetFragment(wxString& text)
{
    if ( text.empty() )
        return {};

    // the maximum length of a sequence of digits that
    // can fit into std::uint64_t when converted to a number
    static constexpr ptrdiff_t maxDigitSequenceLength = 19;

    wxStringFragment         fragment;
    wxString::const_iterator it;

    for ( it = text.cbegin(); it != text.cend(); ++it )
    {
        const wxUniChar&       ch = *it;
        wxStringFragment::Type chType = wxStringFragment::Empty;

        if ( wxIsspace(ch) || wxIspunct(ch) )
            chType = wxStringFragment::SpaceOrPunct;
        else if ( wxIsdigit(ch) )
            chType = wxStringFragment::Digit;
        else
            chType = wxStringFragment::LetterOrSymbol;

        // check if evaluating the first character
        if ( fragment.type == wxStringFragment::Empty )
        {
            fragment.type = chType;
            continue;
        }

        // stop processing when the current character has a different
        // string fragment type than the previously processed characters had
        // or a sequence of digits is too long
        if ( fragment.type != chType
             || (fragment.type == wxStringFragment::Digit
                 && it - text.cbegin() > maxDigitSequenceLength) )
        {
            break;
        }
    }

    fragment.text.assign(text.cbegin(), it);
    if ( fragment.type == wxStringFragment::Digit )
        fragment.text.ToULongLong(&fragment.value);

    text.erase(0, it - text.cbegin());

    return fragment;
}

int CompareFragmentNatural(const wxStringFragment& lhs, const wxStringFragment& rhs)
{
    switch ( lhs.type )
    {
        case wxStringFragment::Empty:
            switch ( rhs.type )
            {
                case wxStringFragment::Empty:
                    return 0;
                case wxStringFragment::SpaceOrPunct:
                case wxStringFragment::Digit:
                case wxStringFragment::LetterOrSymbol:
                    return -1;
            }
            break;

        case wxStringFragment::SpaceOrPunct:
            switch ( rhs.type )
            {
                case wxStringFragment::Empty:
                    return 1;
                case wxStringFragment::SpaceOrPunct:
                    return wxStrcoll_String(lhs.text, rhs.text);
                case wxStringFragment::Digit:
                case wxStringFragment::LetterOrSymbol:
                    return -1;
            }
            break;

        case wxStringFragment::Digit:
            switch ( rhs.type )
            {
                case wxStringFragment::Empty:
                case wxStringFragment::SpaceOrPunct:
                    return 1;
                case wxStringFragment::Digit:
                    if ( lhs.value >  rhs.value )
                        return 1;
                    else if ( lhs.value <  rhs.value )
                        return -1;
                    else
                        return 0;
                case wxStringFragment::LetterOrSymbol:
                    return -1;
            }
            break;

        case wxStringFragment::LetterOrSymbol:
            switch ( rhs.type )
            {
                case wxStringFragment::Empty:
                case wxStringFragment::SpaceOrPunct:
                case wxStringFragment::Digit:
                    return 1;
                case wxStringFragment::LetterOrSymbol:
                    return wxStrcoll_String(lhs.text.Lower(), rhs.text.Lower());
            }
            break;
    }

    // all possible cases should be covered by the switch above
    // but return also from here to prevent the compiler warning
    return 1;
}

} // unnamed namespace


// ----------------------------------------------------------------------------
// wxCmpNaturalGeneric
// ----------------------------------------------------------------------------
//
int wxCMPFUNC_CONV wxCmpNaturalGeneric(const wxString& s1, const wxString& s2)
{
    wxString lhs(s1);
    wxString rhs(s2);

    int comparison = 0;

    while ( (comparison == 0) && (!lhs.empty() || !rhs.empty()) )
    {
        const wxStringFragment fragmentLHS = GetFragment(lhs);
        const wxStringFragment fragmentRHS = GetFragment(rhs);

        comparison = CompareFragmentNatural(fragmentLHS, fragmentRHS);
    }

    return comparison;
}

// ----------------------------------------------------------------------------
// wxCmpNatural
// ----------------------------------------------------------------------------
//
// If a native version of Natural sort is available, then use that, otherwise
// use the generic version.
int wxCMPFUNC_CONV wxCmpNatural(const wxString& s1, const wxString& s2)
{
#if defined( __WINDOWS__ )
    return StrCmpLogicalW(s1.wc_str(), s2.wc_str());
#else
    return wxCmpNaturalGeneric(s1, s2);
#endif // #if defined( __WINDOWS__ )
}

