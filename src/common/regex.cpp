///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/regex.cpp
// Purpose:     regular expression matching
// Author:      Karsten Ballueder and Vadim Zeitlin
// Modified by:
// Created:     13.07.01
// Copyright:   (c) 2000 Karsten Ballueder <ballueder@gmx.net>
//                  2001 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_REGEX

#include "wx/regex.h"

#include "wx/log.h"
#include "wx/intl.h"
#include "wx/crt.h"

// At least FreeBSD requires this.
#if defined(__UNIX__)
#   include <sys/types.h>
#endif

#include <regex.h>

import WX.Utils.Cast;

// WXREGEX_USING_BUILTIN    defined when using the built-in regex lib
// WXREGEX_USING_RE_SEARCH  defined when using re_search in the GNU regex lib
// WXREGEX_IF_NEED_LEN()    wrap the len parameter only used with the built-in
//                          or GNU regex
// WXREGEX_CONVERT_TO_MB    defined when the regex lib is using chars and
//                          wxChar is wide, so conversion must be done
// WXREGEX_CHAR(x)          Convert wxChar to wxRegChar
//
#ifdef __REG_NOFRONT
#   define WXREGEX_USING_BUILTIN
#   define WXREGEX_IF_NEED_LEN(x) ,x
#   define WXREGEX_CHAR(x) (x).wc_str()
#else
#   ifdef HAVE_RE_SEARCH
#       define WXREGEX_IF_NEED_LEN(x) ,x
#       define WXREGEX_USING_RE_SEARCH
#   else
#       define WXREGEX_IF_NEED_LEN(x)
#   endif
#   define WXREGEX_CONVERT_TO_MB
#   define WXREGEX_CHAR(x) (x).mb_str()
#   define wx_regfree regfree
#   define wx_regerror regerror
#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

#ifndef WXREGEX_USING_RE_SEARCH

// the array of offsets for the matches, the usual POSIX regmatch_t array.
class wxRegExMatches
{
public:
    using match_type = regmatch_t *;

    explicit wxRegExMatches(size_t n) : m_matches(new regmatch_t[n]) {}
    ~wxRegExMatches()               { delete [] m_matches; }

    // we just use casts here because the fields of regmatch_t struct may be 64
    // bit but we're limited to size_t in our public API and are not going to
    // change it because operating on strings longer than 4GB using it is
    // absolutely impractical anyhow
    size_t Start(size_t n) const
    {
        return wx::narrow_cast<size_t>(m_matches[n].rm_so);
    }

    size_t End(size_t n) const
    {
        return wx::narrow_cast<size_t>(m_matches[n].rm_eo);
    }

    regmatch_t *get() const         { return m_matches; }

private:
    regmatch_t *m_matches;
};

#else // WXREGEX_USING_RE_SEARCH

// the array of offsets for the matches, the struct used by the GNU lib
class wxRegExMatches
{
public:
    typedef re_registers *match_type;

    wxRegExMatches(size_t n)
    {
        m_matches.num_regs = n;
        m_matches.start = new regoff_t[n];
        m_matches.end = new regoff_t[n];
    }

    ~wxRegExMatches()
    {
        delete [] m_matches.start;
        delete [] m_matches.end;
    }

    size_t Start(size_t n) const    { return m_matches.start[n]; }
    size_t End(size_t n) const      { return m_matches.end[n]; }

    re_registers *get()             { return &m_matches; }

private:
    re_registers m_matches;
};

#endif // WXREGEX_USING_RE_SEARCH

// the character type used by the regular expression engine
#ifndef WXREGEX_CONVERT_TO_MB
using wxRegChar = wxChar;
#else
typedef char wxRegChar;
#endif

// the real implementation of wxRegEx
class wxRegExImpl
{
public:
    // ctor and dtor
    wxRegExImpl() = default;
    ~wxRegExImpl();

    // return true if Compile() had been called successfully
    bool IsValid() const { return m_isCompiled; }

    // RE operations
    bool Compile(const wxString& expr, int flags = 0);
    bool Matches(const wxRegChar *str, int flags
                 WXREGEX_IF_NEED_LEN(size_t len)) const;
    bool GetMatch(size_t *start, size_t *len, size_t index = 0) const;
    size_t GetMatchCount() const;
    int Replace(wxString *pattern, const wxString& replacement,
                size_t maxMatches = 0) const;

private:
    // return the string containing the error message for the given err code
    wxString GetErrorMsg(int errorcode, bool badconv) const;

    // init the members
    

    // free the RE if compiled
    void Free()
    {
        if ( IsValid() )
        {
            wx_regfree(&m_RegEx);
        }

        delete m_Matches;
    }

    // free the RE if any and reinit the members
    void Reinit()
    {
        Free();
        
        m_isCompiled = false;
        m_Matches = nullptr;
        m_nMatches = 0;
    
    }

    // compiled RE
    regex_t         m_RegEx;

    // the subexpressions data
    wxRegExMatches *m_Matches{nullptr};
    size_t          m_nMatches{0};

    // true if m_RegEx is valid
    bool            m_isCompiled{false};
};


// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxRegExImpl
// ----------------------------------------------------------------------------

wxRegExImpl::~wxRegExImpl()
{
    Free();
}

wxString wxRegExImpl::GetErrorMsg(int errorcode, bool badconv) const
{
#ifdef WXREGEX_CONVERT_TO_MB
    // currently only needed when using system library in Unicode mode
    if ( badconv )
    {
        return _("conversion to 8-bit encoding failed");
    }
#else
    // 'use' badconv to avoid a compiler warning
    std::ignore = badconv;
#endif

    wxString szError;

    // first get the string length needed
    int len = wx_regerror(errorcode, &m_RegEx, nullptr, 0);
    if ( len > 0 )
    {
        char* szcmbError = new char[++len];

        std::ignore = wx_regerror(errorcode, &m_RegEx, szcmbError, len);

        szError = wxConvLibc.cMB2WX(szcmbError);
        delete [] szcmbError;
    }
    else // regerror() returned 0
    {
        szError = _("unknown error");
    }

    return szError;
}

bool wxRegExImpl::Compile(const wxString& expr, int flags)
{
    Reinit();

#ifdef WX_NO_REGEX_ADVANCED
#   define FLAVORS wxRE_BASIC
#else
#   define FLAVORS (wxRE_ADVANCED | wxRE_BASIC)
    wxASSERT_MSG( (flags & FLAVORS) != FLAVORS,
                  "incompatible flags in wxRegEx::Compile" );
#endif
    wxASSERT_MSG( !(flags & ~(FLAVORS | wxRE_ICASE | wxRE_NOSUB | wxRE_NEWLINE)),
                  "unrecognized flags in wxRegEx::Compile" );

    // translate our flags to regcomp() ones
    int flagsRE = 0;
    if ( !(flags & wxRE_BASIC) )
    {
#ifndef WX_NO_REGEX_ADVANCED
        if (flags & wxRE_ADVANCED)
            flagsRE |= REG_ADVANCED;
        else
#endif
            flagsRE |= REG_EXTENDED;
    }
    if ( flags & wxRE_ICASE )
        flagsRE |= REG_ICASE;
    if ( flags & wxRE_NOSUB )
        flagsRE |= REG_NOSUB;
    if ( flags & wxRE_NEWLINE )
        flagsRE |= REG_NEWLINE;

    // compile it
#ifdef WXREGEX_USING_BUILTIN
    bool conv = true;
    // FIXME-UTF8: use wc_str() after removing ANSI build
    int errorcode = wx_re_comp(&m_RegEx, expr.c_str(), expr.length(), flagsRE);
#else
    // FIXME-UTF8: this is potentially broken, we shouldn't even try it
    //             and should always use builtin regex library (or PCRE?)
    const wxWX2MBbuf conv = expr.mbc_str();
    int errorcode = conv ? regcomp(&m_RegEx, conv, flagsRE) : REG_BADPAT;
#endif

    if ( errorcode )
    {
        wxLogError(_("Invalid regular expression '%s': %s"),
                   expr.c_str(), GetErrorMsg(errorcode, !conv).c_str());

        m_isCompiled = false;
    }
    else // ok
    {
        // don't allocate the matches array now, but do it later if necessary
        if ( flags & wxRE_NOSUB )
        {
            // we don't need it at all
            m_nMatches = 0;
        }
        else
        {
            // we will alloc the array later (only if really needed) but count
            // the number of sub-expressions in the regex right now

            // there is always one for the whole expression
            m_nMatches = 1;

            // and some more for bracketed subexperessions
            for ( const wxChar *cptr = expr.c_str(); *cptr; cptr++ )
            {
                if ( *cptr == wxT('\\') )
                {
                    // in basic RE syntax groups are inside \(...\)
                    if ( *++cptr == wxT('(') && (flags & wxRE_BASIC) )
                    {
                        m_nMatches++;
                    }
                }
                else if ( *cptr == wxT('(') && !(flags & wxRE_BASIC) )
                {
                    // we know that the previous character is not an unquoted
                    // backslash because it would have been eaten above, so we
                    // have a bare '(' and this indicates a group start for the
                    // extended syntax. '(?' is used for extensions by perl-
                    // like REs (e.g. advanced), and is not valid for POSIX
                    // extended, so ignore them always.
                    if ( cptr[1] != wxT('?') )
                        m_nMatches++;
                }
            }
        }

        m_isCompiled = true;
    }

    return IsValid();
}

#ifdef WXREGEX_USING_RE_SEARCH

// On GNU, regexec is implemented as a wrapper around re_search. re_search
// requires a length parameter which the POSIX regexec does not have,
// therefore regexec must do a strlen on the search text each time it is
// called. This can drastically affect performance when matching is done in
// a loop along a string, such as during a search and replace. Therefore if
// re_search is detected by configure, it is used directly.
//
static int ReSearch(const regex_t *preg,
                    const char *text,
                    size_t len,
                    re_registers *matches,
                    int eflags)
{
    regex_t *pattern = const_cast<regex_t*>(preg);

    pattern->not_bol = (eflags & REG_NOTBOL) != 0;
    pattern->not_eol = (eflags & REG_NOTEOL) != 0;
    pattern->regs_allocated = REGS_FIXED;

    int ret = re_search(pattern, text, len, 0, len, matches);
    return ret >= 0 ? 0 : REG_NOMATCH;
}

#endif // WXREGEX_USING_RE_SEARCH

bool wxRegExImpl::Matches(const wxRegChar *str,
                          int flags
                          WXREGEX_IF_NEED_LEN(size_t len)) const
{
    wxCHECK_MSG( IsValid(), false, "must successfully Compile() first" );

    // translate our flags to regexec() ones
    wxASSERT_MSG( !(flags & ~(wxRE_NOTBOL | wxRE_NOTEOL)),
                  "unrecognized flags in wxRegEx::Matches" );

    int flagsRE = 0;
    if ( flags & wxRE_NOTBOL )
        flagsRE |= REG_NOTBOL;
    if ( flags & wxRE_NOTEOL )
        flagsRE |= REG_NOTEOL;

    // allocate matches array if needed
    wxRegExImpl *self = const_cast<wxRegExImpl *>(this);
    if ( !m_Matches && m_nMatches )
    {
        self->m_Matches = new wxRegExMatches(m_nMatches);
    }

    wxRegExMatches::match_type matches = m_Matches ? m_Matches->get() : nullptr;

    // do match it
#if defined WXREGEX_USING_BUILTIN
    const int rc = wx_re_exec(&self->m_RegEx, str, len, nullptr, m_nMatches, matches, flagsRE);
#elif defined WXREGEX_USING_RE_SEARCH
    const int rc = str ? ReSearch(&self->m_RegEx, str, len, matches, flagsRE) : REG_BADPAT;
#else
    const int rc = str ? regexec(&self->m_RegEx, str, m_nMatches, matches, flagsRE) : REG_BADPAT;
#endif

    switch ( rc )
    {
        case 0:
            // matched successfully
            return true;

        default:
            // an error occurred
            wxLogError(_("Failed to find match for regular expression: %s"),
                       GetErrorMsg(rc, !str).c_str());
            [[fallthrough]];

        case REG_NOMATCH:
            // no match
            return false;
    }
}

bool wxRegExImpl::GetMatch(size_t *start, size_t *len, size_t index) const
{
    wxCHECK_MSG( IsValid(), false, "must successfully Compile() first" );
    wxCHECK_MSG( m_nMatches, false, "can't use with wxRE_NOSUB" );
    wxCHECK_MSG( m_Matches, false, "must call Matches() first" );
    wxCHECK_MSG( index < m_nMatches, false, "invalid match index" );

    if ( start )
        *start = m_Matches->Start(index);
    if ( len )
        *len = m_Matches->End(index) - m_Matches->Start(index);

    return true;
}

size_t wxRegExImpl::GetMatchCount() const
{
    wxCHECK_MSG( IsValid(), 0, "must successfully Compile() first" );
    wxCHECK_MSG( m_nMatches, 0, "can't use with wxRE_NOSUB" );

    return m_nMatches;
}

int wxRegExImpl::Replace(wxString *text,
                         const wxString& replacement,
                         size_t maxMatches) const
{
    wxCHECK_MSG( text, wxNOT_FOUND, "NULL text in wxRegEx::Replace" );
    wxCHECK_MSG( IsValid(), wxNOT_FOUND, "must successfully Compile() first" );

    // the input string
#ifndef WXREGEX_CONVERT_TO_MB
    const wxChar *textstr = text->c_str();
    const size_t textlen = text->length();
#else
    const wxWX2MBbuf textstr = WXREGEX_CHAR(*text);
    if (!textstr)
    {
        wxLogError(_("Failed to find match for regular expression: %s"),
                   GetErrorMsg(0, true).c_str());
        return 0;
    }
    size_t textlen = strlen(textstr);
    text->clear();
#endif

    // the replacement text
    wxString textNew;

    // the result, allow 25% extra
    wxString result;
    result.reserve(5 * textlen / 4);

    // attempt at optimization: don't iterate over the string if it doesn't
    // contain back references at all
    bool mayHaveBackrefs =
        replacement.find_first_of("\\&") != wxString::npos;

    if ( !mayHaveBackrefs )
    {
        textNew = replacement;
    }

    // the position where we start looking for the match
    size_t matchStart = 0;

    // number of replacement made: we won't make more than maxMatches of them
    // (unless maxMatches is 0 which doesn't limit the number of replacements)
    size_t countRepl = 0;

    // note that "^" shouldn't match after the first call to Matches() so we
    // use wxRE_NOTBOL to prevent it from happening
    while ( (!maxMatches || countRepl < maxMatches) &&
             Matches(
#ifndef WXREGEX_CONVERT_TO_MB
                    textstr + matchStart,
#else
                    textstr.data() + matchStart,
#endif
                    countRepl ? wxRE_NOTBOL : 0
                    WXREGEX_IF_NEED_LEN(textlen - matchStart)) )
    {
        // the string possibly contains back references: we need to calculate
        // the replacement text anew after each match
        if ( mayHaveBackrefs )
        {
            mayHaveBackrefs = false;
            textNew.clear();
            textNew.reserve(replacement.length());

            for ( const wxChar *p = replacement.c_str(); *p; p++ )
            {
                size_t index = (size_t)-1;

                if ( *p == wxT('\\') )
                {
                    if ( wxIsdigit(*++p) )
                    {
                        // back reference
                        wxChar *end;
                        index = (size_t)wxStrtoul(p, &end, 10);
                        p = end - 1; // -1 to compensate for p++ in the loop
                    }
                    //else: backslash used as escape character
                }
                else if ( *p == wxT('&') )
                {
                    // treat this as "\0" for compatbility with ed and such
                    index = 0;
                }

                // do we have a back reference?
                if ( index != (size_t)-1 )
                {
                    // yes, get its text
                    size_t start, len;
                    if ( !GetMatch(&start, &len, index) )
                    {
                        wxFAIL_MSG( "invalid back reference" );

                        // just eat it...
                    }
                    else
                    {
                        textNew += wxString(
#ifndef WXREGEX_CONVERT_TO_MB
                                textstr
#else
                                textstr.data()
#endif
                                + matchStart + start,
                                *wxConvCurrent, len);

                        mayHaveBackrefs = true;
                    }
                }
                else // ordinary character
                {
                    textNew += *p;
                }
            }
        }

        size_t start, len;
        if ( !GetMatch(&start, &len) )
        {
            // we did have match as Matches() returned true above!
            wxFAIL_MSG( "internal logic error in wxRegEx::Replace" );

            return wxNOT_FOUND;
        }

        // an insurance against implementations that don't grow exponentially
        // to ensure building the result takes linear time
        if (result.capacity() < result.length() + start + textNew.length())
            result.reserve(2 * result.length());

#ifndef WXREGEX_CONVERT_TO_MB
        result.append(*text, matchStart, start);
#else
        result.append(wxString(textstr.data() + matchStart, *wxConvCurrent, start));
#endif
        matchStart += start;
        result.append(textNew);

        countRepl++;

        matchStart += len;
    }

#ifndef WXREGEX_CONVERT_TO_MB
    result.append(*text, matchStart, wxString::npos);
#else
    result.append(wxString(textstr.data() + matchStart, *wxConvCurrent));
#endif
    *text = result;

    return countRepl;
}

// ----------------------------------------------------------------------------
// wxRegEx: all methods are mostly forwarded to wxRegExImpl
// ----------------------------------------------------------------------------



wxRegEx::~wxRegEx()
{
    delete m_impl;
}

bool wxRegEx::Compile(const wxString& expr, int flags)
{
    if ( !m_impl )
    {
        m_impl = new wxRegExImpl;
    }

    if ( !m_impl->Compile(expr, flags) )
    {
        // error message already given in wxRegExImpl::Compile
        wxDELETE(m_impl);

        return false;
    }

    return true;
}

bool wxRegEx::Matches(const wxString& str, int flags) const
{
    wxCHECK_MSG( IsValid(), false, "must successfully Compile() first" );

    return m_impl->Matches(WXREGEX_CHAR(str), flags
                            WXREGEX_IF_NEED_LEN(str.length()));
}

bool wxRegEx::GetMatch(size_t *start, size_t *len, size_t index) const
{
    wxCHECK_MSG( IsValid(), false, "must successfully Compile() first" );

    return m_impl->GetMatch(start, len, index);
}

wxString wxRegEx::GetMatch(const wxString& text, size_t index) const
{
    size_t start, len;
    if ( !GetMatch(&start, &len, index) )
        return {};

    return text.Mid(start, len);
}

size_t wxRegEx::GetMatchCount() const
{
    wxCHECK_MSG( IsValid(), 0, "must successfully Compile() first" );

    return m_impl->GetMatchCount();
}

int wxRegEx::Replace(wxString *pattern,
                     const wxString& replacement,
                     size_t maxMatches) const
{
    wxCHECK_MSG( IsValid(), wxNOT_FOUND, "must successfully Compile() first" );

    return m_impl->Replace(pattern, replacement, maxMatches);
}

wxString wxRegEx::QuoteMeta(const wxString& str)
{
    static const wxString s_strMetaChars = "\\^$.|?*+()[]{}";

    wxString strEscaped;

    // This is the maximal possible length of the resulting string, if every
    // character were escaped.
    strEscaped.reserve(str.length() * 2);

    for ( wxString::const_iterator it = str.begin(); it != str.end(); ++it )
    {
        if ( s_strMetaChars.find(*it) != wxString::npos )
        {
            strEscaped += wxS('\\');
        }

        strEscaped += *it;
    }

    strEscaped.shrink_to_fit();

    return strEscaped;
}

#endif // wxUSE_REGEX
