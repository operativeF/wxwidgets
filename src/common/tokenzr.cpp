/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/tokenzr.cpp
// Purpose:     String tokenizer
// Author:      Guilhem Lavaux
// Modified by: Vadim Zeitlin (almost full rewrite)
// Created:     04/22/98
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/tokenzr.h"
#include "wx/crt.h"

import <vector>;

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// helpers
// ----------------------------------------------------------------------------

static wxString::const_iterator
find_first_of(const wxChar *delims, size_t len,
              const wxString::const_iterator& from,
              const wxString::const_iterator& end)
{
    wxASSERT_MSG( from <= end,  "invalid index" );

    for ( wxString::const_iterator i = from; i != end; ++i )
    {
        if ( wxTmemchr(delims, *i, len) )
            return i;
    }

    return end;
}

static wxString::const_iterator
find_first_not_of(const wxChar *delims, size_t len,
                  const wxString::const_iterator& from,
                  const wxString::const_iterator& end)
{
    wxASSERT_MSG( from <= end,  "invalid index" );

    for ( wxString::const_iterator i = from; i != end; ++i )
    {
        if ( !wxTmemchr(delims, *i, len) )
            return i;
    }

    return end;
}

// ----------------------------------------------------------------------------
// wxStringTokenizer construction
// ----------------------------------------------------------------------------

wxStringTokenizer::wxStringTokenizer(const wxString& str,
                                     const wxString& delims,
                                     wxStringTokenizerMode mode)
{
    SetString(str, delims, mode);
}

wxStringTokenizer::wxStringTokenizer(const wxStringTokenizer& src)
                  
{
    DoCopyFrom(src);
}

wxStringTokenizer& wxStringTokenizer::operator=(const wxStringTokenizer& src)
{
    if (this != &src)
    {
        DoCopyFrom(src);
    }

    return *this;
}

void wxStringTokenizer::SetString(const wxString& str,
                                  const wxString& delims,
                                  wxStringTokenizerMode mode)
{
    if ( mode == wxStringTokenizerMode::Default )
    {
        // by default, we behave like strtok() if the delimiters are only
        // whitespace characters and as wxStringTokenizerMode::RetEmpty otherwise (for
        // whitespace delimiters, strtok() behaviour is better because we want
        // to count consecutive spaces as one delimiter)
        wxString::const_iterator p;
        for ( p = delims.begin(); p != delims.end(); ++p )
        {
            if ( !wxIsspace(*p) )
                break;
        }

        if ( p != delims.end() )
        {
            // not whitespace char in delims
            mode = wxStringTokenizerMode::RetEmpty;
        }
        else
        {
            // only whitespaces
            mode = wxStringTokenizerMode::StrTok;
        }
    }

    // FIXME-UTF8: only wc_str()
    m_delims = delims.wc_str();
    m_delimsLen = delims.length();

    m_mode = mode;

    Reinit(str);
}

void wxStringTokenizer::Reinit(const wxString& str)
{
    wxASSERT_MSG( IsOk(), "you should call SetString() first" );

    m_string = str;
    m_stringEnd = m_string.end();
    m_pos = m_string.begin();
    m_lastDelim = wxT('\0');
    m_hasMoreTokens = MoreTokensState::Unknown;
}

void wxStringTokenizer::DoCopyFrom(const wxStringTokenizer& src)
{
    m_string = src.m_string;
    m_stringEnd = m_string.end();
    m_pos = m_string.begin() + (src.m_pos - src.m_string.begin());
    m_delims = src.m_delims;
    m_delimsLen = src.m_delimsLen;
    m_mode = src.m_mode;
    m_lastDelim = src.m_lastDelim;
    m_hasMoreTokens = src.m_hasMoreTokens;
}

// ----------------------------------------------------------------------------
// access to the tokens
// ----------------------------------------------------------------------------

// do we have more of them?
bool wxStringTokenizer::HasMoreTokens() const
{
    // GetNextToken() calls HasMoreTokens() and so HasMoreTokens() is called
    // twice in every interation in the following common usage patten:
    //     while ( HasMoreTokens() )
    //        GetNextToken();
    // We optimize this case by caching HasMoreTokens() return value here:
    if ( m_hasMoreTokens == MoreTokensState::Unknown )
    {
        const bool r = DoHasMoreTokens();
        const_cast<wxStringTokenizer *>(this)->m_hasMoreTokens =
            r ? MoreTokensState::Yes : MoreTokensState::No;
        return r;
    }
    else
        return m_hasMoreTokens == MoreTokensState::Yes;
}

bool wxStringTokenizer::DoHasMoreTokens() const
{
    wxCHECK_MSG( IsOk(), false, "you should call SetString() first" );

    if ( find_first_not_of(m_delims, m_delimsLen, m_pos, m_stringEnd)
         != m_stringEnd )
    {
        // there are non delimiter characters left, so we do have more tokens
        return true;
    }

    switch ( m_mode )
    {
        case wxStringTokenizerMode::RetEmpty:
        case wxStringTokenizerMode::RetDelims:
            // special hack for wxStringTokenizerMode::RetEmpty: we should return the initial
            // empty token even if there are only delimiters after it
            return !m_string.empty() && m_pos == m_string.begin();

        case wxStringTokenizerMode::RetEmptyAll:
            // special hack for wxStringTokenizerMode::RetEmptyAll: we can know if we had
            // already returned the trailing empty token after the last
            // delimiter by examining m_lastDelim: it is set to NUL if we run
            // up to the end of the string in GetNextToken(), but if it is not
            // NUL yet we still have this last token to return even if m_pos is
            // already at m_string.length()
            return m_pos < m_stringEnd || m_lastDelim != wxT('\0');

        case wxStringTokenizerMode::Invalid:
        case wxStringTokenizerMode::Default:
            wxFAIL_MSG( "unexpected tokenizer mode" );
            [[fallthrough]];

        case wxStringTokenizerMode::StrTok:
            // never return empty delimiters
            break;
    }

    return false;
}

// count the number of (remaining) tokens in the string
size_t wxStringTokenizer::CountTokens() const
{
    wxCHECK_MSG( IsOk(), 0, "you should call SetString() first" );

    // VZ: this function is IMHO not very useful, so it's probably not very
    //     important if its implementation here is not as efficient as it
    //     could be -- but OTOH like this we're sure to get the correct answer
    //     in all modes
    wxStringTokenizer tkz(wxString(m_pos, m_stringEnd), m_delims, m_mode);

    size_t count = 0;
    while ( tkz.HasMoreTokens() )
    {
        count++;

        (void)tkz.GetNextToken();
    }

    return count;
}

// ----------------------------------------------------------------------------
// token extraction
// ----------------------------------------------------------------------------

wxString wxStringTokenizer::GetNextToken()
{
    wxString token;
    do
    {
        if ( !HasMoreTokens() )
        {
            break;
        }

        m_hasMoreTokens = MoreTokensState::Unknown;

        // find the end of this token
        wxString::const_iterator pos =
            find_first_of(m_delims, m_delimsLen, m_pos, m_stringEnd);

        // and the start of the next one
        if ( pos == m_stringEnd )
        {
            // no more delimiters, the token is everything till the end of
            // string
            token.assign(m_pos, m_stringEnd);

            // skip the token
            m_pos = m_stringEnd;

            // it wasn't terminated
            m_lastDelim = wxT('\0');
        }
        else // we found a delimiter at pos
        {
            // in wxStringTokenizerMode::RetDelims mode we return the delimiter character
            // with token, otherwise leave it out
            wxString::const_iterator tokenEnd(pos);
            if ( m_mode == wxStringTokenizerMode::RetDelims )
                ++tokenEnd;

            token.assign(m_pos, tokenEnd);

            // skip the token and the trailing delimiter
            m_pos = pos + 1;

            m_lastDelim = (pos == m_stringEnd) ? wxT('\0') : (wxChar)*pos;
        }
    }
    while ( !AllowEmpty() && token.empty() );

    return token;
}

// ----------------------------------------------------------------------------
// public functions
// ----------------------------------------------------------------------------

std::vector<wxString> wxStringTokenize(const wxString& str,
                               const wxString& delims,
                               wxStringTokenizerMode mode)
{
    std::vector<wxString> tokens;
    wxStringTokenizer tk(str, delims, mode);
    while ( tk.HasMoreTokens() )
    {
        tokens.push_back(tk.GetNextToken());
    }

    return tokens;
}
