///////////////////////////////////////////////////////////////////////////////
// Name:        tests/strings/strings.cpp
// Purpose:     wxStringTokenizer unit test
// Author:      Vadim Zeitlin
// Created:     2005-12-20 (extacted from strings.cpp)
// Copyright:   (c) 2004-2005 Vadim Zeitlin
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif // WX_PRECOMP

#include "wx/tokenzr.h"

static constexpr struct TokenizerTestData
{
    // the string to tokenize
    const wxChar *str;

    // the delimiters to use
    const wxChar *delims;

    // the tokenizer mode
    wxStringTokenizerMode mode;

    // expected number of tokens
    size_t count;
}
gs_testData[] =
{
    { wxT(""),                   wxT(" "),              wxTOKEN_DEFAULT,       0 },
    { wxT(""),                   wxT(" "),              wxTOKEN_RET_EMPTY,     0 },
    { wxT(""),                   wxT(" "),              wxTOKEN_RET_EMPTY_ALL, 0 },
    { wxT(""),                   wxT(" "),              wxTOKEN_RET_DELIMS,    0 },
    { wxT(":"),                  wxT(":"),              wxTOKEN_RET_EMPTY,     1 },
    { wxT(":"),                  wxT(":"),              wxTOKEN_RET_DELIMS,    1 },
    { wxT(":"),                  wxT(":"),              wxTOKEN_RET_EMPTY_ALL, 2 },
    { wxT("::"),                 wxT(":"),              wxTOKEN_RET_EMPTY,     1 },
    { wxT("::"),                 wxT(":"),              wxTOKEN_RET_DELIMS,    1 },
    { wxT("::"),                 wxT(":"),              wxTOKEN_RET_EMPTY_ALL, 3 },

    { wxT("Hello, world"),       wxT(" "),              wxTOKEN_DEFAULT,       2 },
    { wxT("Hello,   world  "),   wxT(" "),              wxTOKEN_DEFAULT,       2 },
    { wxT("Hello, world"),       wxT(","),              wxTOKEN_DEFAULT,       2 },
    { wxT("Hello, world!"),      wxT(",!"),             wxTOKEN_DEFAULT,       2 },
    { wxT("Hello,, world!"),     wxT(",!"),             wxTOKEN_DEFAULT,       3 },
    { wxT("Hello,, world!"),     wxT(",!"),             wxTOKEN_STRTOK,        2 },
    { wxT("Hello, world!"),      wxT(",!"),             wxTOKEN_RET_EMPTY_ALL, 3 },

    { wxT("username:password:uid:gid:gecos:home:shell"),
                                wxT(":"),              wxTOKEN_DEFAULT,       7 },

    { wxT("1:2::3:"),            wxT(":"),              wxTOKEN_DEFAULT,       4 },
    { wxT("1:2::3:"),            wxT(":"),              wxTOKEN_RET_EMPTY,     4 },
    { wxT("1:2::3:"),            wxT(":"),              wxTOKEN_RET_EMPTY_ALL, 5 },
    { wxT("1:2::3:"),            wxT(":"),              wxTOKEN_RET_DELIMS,    4 },
    { wxT("1:2::3:"),            wxT(":"),              wxTOKEN_STRTOK,        3 },

    { wxT("1:2::3::"),           wxT(":"),              wxTOKEN_DEFAULT,       4 },
    { wxT("1:2::3::"),           wxT(":"),              wxTOKEN_RET_EMPTY,     4 },
    { wxT("1:2::3::"),           wxT(":"),              wxTOKEN_RET_EMPTY_ALL, 6 },
    { wxT("1:2::3::"),           wxT(":"),              wxTOKEN_RET_DELIMS,    4 },
    { wxT("1:2::3::"),           wxT(":"),              wxTOKEN_STRTOK,        3 },

    { wxT("1 \t3\t4  6   "),     wxDEFAULT_DELIMITERS, wxTOKEN_DEFAULT,       4 },
    { wxT("1 \t3\t4  6   "),     wxDEFAULT_DELIMITERS, wxTOKEN_STRTOK,        4 },
    { wxT("1 \t3\t4  6   "),     wxDEFAULT_DELIMITERS, wxTOKEN_RET_EMPTY,     6 },
    { wxT("1 \t3\t4  6   "),     wxDEFAULT_DELIMITERS, wxTOKEN_RET_EMPTY_ALL, 9 },

    { wxT("01/02/99"),           wxT("/-"),             wxTOKEN_DEFAULT,       3 },
    { wxT("01-02/99"),           wxT("/-"),             wxTOKEN_RET_DELIMS,    3 },
};

// helper function returning the string showing the index for which the test
// fails in the diagnostic message
static std::string Nth(size_t n)
{
    return std::string(wxString::Format(wxT("for loop index %lu"),
                                        (unsigned long)n).mb_str());
}

// ----------------------------------------------------------------------------
// the tests
// ----------------------------------------------------------------------------

TEST_CASE("GetCount")
{
    for ( size_t n = 0; n < WXSIZEOF(gs_testData); n++ )
    {
        const TokenizerTestData& ttd = gs_testData[n];

        wxStringTokenizer tkz(ttd.str, ttd.delims, ttd.mode);
        CHECK_MESSAGE(ttd.count == tkz.CountTokens(), Nth(n));

        size_t count = 0;
        while ( tkz.HasMoreTokens() )
        {
            tkz.GetNextToken();
            count++;
        }

        CHECK_MESSAGE(ttd.count == count, Nth(n));
    }
}

// call this with the string to tokenize, delimeters to use and the expected
// positions (i.e. results of GetPosition()) after each GetNextToken() call,
// terminate positions with 0
static void
DoTestGetPosition(const wxChar *s, const wxChar *delims, int pos, ...)
{
    wxStringTokenizer tkz(s, delims);

    CHECK_EQ( (size_t)0, tkz.GetPosition() );

    va_list ap;
    va_start(ap, pos);

    for ( ;; )
    {
        if ( !pos )
        {
            CHECK( !tkz.HasMoreTokens() );
            break;
        }

        tkz.GetNextToken();

        CHECK_EQ( (size_t)pos, tkz.GetPosition() );

        pos = va_arg(ap, int);
    }

    va_end(ap);
}

TEST_CASE("GetPosition")
{
    DoTestGetPosition(wxT("foo"), wxT("_"), 3, 0);
    DoTestGetPosition(wxT("foo_bar"), wxT("_"), 4, 7, 0);
    DoTestGetPosition(wxT("foo_bar_"), wxT("_"), 4, 8, 0);
}

// helper for GetString(): the parameters are the same as for DoTestGetPosition
// but it checks GetString() return value instead of GetPosition()
static void
DoTestGetString(const wxChar *s, const wxChar *delims, int pos, ...)
{
    wxStringTokenizer tkz(s, delims);

    CHECK_EQ( wxString(s), tkz.GetString() );

    va_list ap;
    va_start(ap, pos);

    for ( ;; )
    {
        if ( !pos )
        {
            CHECK( tkz.GetString().empty() ) ;
            break;
        }

        tkz.GetNextToken();

        CHECK_EQ( wxString(s + pos), tkz.GetString() );

        pos = va_arg(ap, int);
    }

    va_end(ap);
}

TEST_CASE("GetString")
{
    DoTestGetString(wxT("foo"), wxT("_"), 3, 0);
    DoTestGetString(wxT("foo_bar"), wxT("_"), 4, 7, 0);
    DoTestGetString(wxT("foo_bar_"), wxT("_"), 4, 8, 0);
}

TEST_CASE("LastDelimiter")
{
    wxStringTokenizer tkz(wxT("a+-b=c"), wxT("+-="));

    tkz.GetNextToken();
    CHECK_EQ( wxT('+'), tkz.GetLastDelimiter() );

    tkz.GetNextToken();
    CHECK_EQ( wxT('-'), tkz.GetLastDelimiter() );

    tkz.GetNextToken();
    CHECK_EQ( wxT('='), tkz.GetLastDelimiter() );

    tkz.GetNextToken();
    CHECK_EQ( wxT('\0'), tkz.GetLastDelimiter() );
}

TEST_CASE("StrtokCompat")
{
    for ( size_t n = 0; n < WXSIZEOF(gs_testData); n++ )
    {
        const TokenizerTestData& ttd = gs_testData[n];
        if ( ttd.mode != wxTOKEN_STRTOK )
            continue;

        wxWCharBuffer buf(ttd.str);
        wxChar *last;
        wxChar *s = wxStrtok(buf.data(), ttd.delims, &last);

        wxStringTokenizer tkz(ttd.str, ttd.delims, ttd.mode);
        while ( tkz.HasMoreTokens() )
        {
            CHECK_EQ( wxString(s), tkz.GetNextToken() );
            s = wxStrtok(NULL, ttd.delims, &last);
        }
    }
}

TEST_CASE("CopyObj")
{
    // Test copy ctor
    wxStringTokenizer tkzSrc(wxT("first:second:third:fourth"), wxT(":"));
    while ( tkzSrc.HasMoreTokens() )
    {
        wxString tokenSrc = tkzSrc.GetNextToken();
        wxStringTokenizer tkz = tkzSrc;

        CHECK_EQ( tkzSrc.GetPosition(), tkz.GetPosition() );
        CHECK_EQ( tkzSrc.GetString(), tkz.GetString() );

        // Change the state of both objects and compare again...
        tokenSrc = tkzSrc.GetNextToken();
        wxString token = tkz.GetNextToken();

        CHECK_EQ( tkzSrc.GetPosition(), tkz.GetPosition() );
        CHECK_EQ( tkzSrc.GetString(), tkz.GetString() );
    }
}

TEST_CASE("AssignObj")
{
    // Test assignment
    wxStringTokenizer tkzSrc(wxT("first:second:third:fourth"), wxT(":"));
    wxStringTokenizer tkz;
    while ( tkzSrc.HasMoreTokens() )
    {
        wxString tokenSrc = tkzSrc.GetNextToken();
        tkz = tkzSrc;

        CHECK_EQ( tkzSrc.GetPosition(), tkz.GetPosition() );
        CHECK_EQ( tkzSrc.GetString(), tkz.GetString() );

        // Change the state of both objects and compare again...
        tokenSrc = tkzSrc.GetNextToken();
        wxString token = tkz.GetNextToken();

        CHECK_EQ( tkzSrc.GetPosition(), tkz.GetPosition() );
        CHECK_EQ( tkzSrc.GetString(), tkz.GetString() );
    }
}
