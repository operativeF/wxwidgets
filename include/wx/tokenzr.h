/////////////////////////////////////////////////////////////////////////////
// Name:        wx/tokenzr.h
// Purpose:     String tokenizer - a C++ replacement for strtok(3)
// Author:      Guilhem Lavaux
// Modified by: (or rather rewritten by) Vadim Zeitlin
// Created:     04/22/98
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TOKENZRH
#define _WX_TOKENZRH

#include "wx/string.h"

import <vector>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// default: delimiters are usual white space characters
inline constexpr std::string_view wxDEFAULT_DELIMITERS = " \t\r\n";

// wxStringTokenizer mode flags which determine its behaviour
enum class wxStringTokenizerMode
{
    Invalid,   // set by def ctor until SetString() is called
    Default,        // strtok() for whitespace delims, RET_EMPTY else
    RetEmpty,      // return empty token in the middle of the string
    RetEmptyAll,  // return trailing empty tokens too
    RetDelims,     // return the delim with token (implies RET_EMPTY)
    StrTok          // behave exactly like strtok(3)
};

// ----------------------------------------------------------------------------
// wxStringTokenizer: replaces infamous strtok() and has some other features
// ----------------------------------------------------------------------------

class wxStringTokenizer
{
public:
    // ctors and initializers
        // default ctor, call SetString() later
    wxStringTokenizer() = default;
        // ctor which gives us the string
    wxStringTokenizer(std::string_view str,
                      std::string_view delims = wxDEFAULT_DELIMITERS,
                      wxStringTokenizerMode mode = wxStringTokenizerMode::Default);
        // copy ctor and assignment operator
    wxStringTokenizer(const wxStringTokenizer& src);
    wxStringTokenizer& operator=(const wxStringTokenizer& src);

        // args are same as for the non default ctor above
    void SetString(std::string_view str,
                   std::string_view delims = wxDEFAULT_DELIMITERS,
                   wxStringTokenizerMode mode = wxStringTokenizerMode::Default);

        // reinitialize the tokenizer with the same delimiters/mode
    void Reinit(std::string_view str);

    // tokens access
        // return the number of remaining tokens
    size_t CountTokens() const;
        // did we reach the end of the string?
    bool HasMoreTokens() const;
        // get the next token, will return empty string if !HasMoreTokens()
    std::string GetNextToken();
        // get the delimiter which terminated the token last retrieved by
        // GetNextToken() or NUL if there had been no tokens yet or the last
        // one wasn't terminated (but ran to the end of the string)
    wxChar GetLastDelimiter() const { return m_lastDelim; }

    // get current tokenizer state
        // returns the part of the string which remains to tokenize (*not* the
        // initial string)
    std::string GetString() const { return std::string(m_pos, m_string.end()); }

        // returns the current position (i.e. one index after the last
        // returned token or 0 if GetNextToken() has never been called) in the
        // original string
    size_t GetPosition() const { return m_pos - m_string.begin(); }

    // misc
        // get the current mode - can be different from the one passed to the
        // ctor if it was wxStringTokenizerMode::Default
    wxStringTokenizerMode GetMode() const { return m_mode; }
        // do we return empty tokens?
    bool AllowEmpty() const { return m_mode != wxStringTokenizerMode::StrTok; }

protected:
    bool IsOk() const { return m_mode != wxStringTokenizerMode::Invalid; }

    bool DoHasMoreTokens() const;

    void DoCopyFrom(const wxStringTokenizer& src);

    enum class MoreTokensState
    {
        Unknown,
        Yes,
        No
    };

    std::string m_string;              // the string we tokenize
    
    std::string::const_iterator m_stringEnd;
    // FIXME-UTF8: use wxWcharBuffer
    std::string m_delims;        // all possible delimiters

    std::string::const_iterator m_pos; // the current position in m_string

    size_t m_delimsLen;

    MoreTokensState m_hasMoreTokens;

    wxStringTokenizerMode m_mode{wxStringTokenizerMode::Invalid}; // see wxTOKEN_XXX values

    char   m_lastDelim;           // delimiter after last token or '\0'
};

// ----------------------------------------------------------------------------
// convenience function which returns all tokens at once
// ----------------------------------------------------------------------------

// the function takes the same parameters as wxStringTokenizer ctor and returns
// the array containing all tokens
std::vector<std::string>
wxStringTokenize(std::string_view str,
                 std::string_view delims = wxDEFAULT_DELIMITERS,
                 wxStringTokenizerMode mode = wxStringTokenizerMode::Default);

#endif // _WX_TOKENZRH
