///////////////////////////////////////////////////////////////////////////////
// Name:        tests/strings/unicode.cpp
// Purpose:     Unicode unit test
// Author:      Vadim Zeitlin, Wlodzimierz ABX Skiba
// Created:     2004-04-28
// Copyright:   (c) 2004 Vadim Zeitlin, Wlodzimierz Skiba
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

import WX.Cmn.EncConv;

// ----------------------------------------------------------------------------
// helper class holding the matching MB and WC strings
// ----------------------------------------------------------------------------

struct StringConversionData
{
    // either str or wcs (but not both) may be nullptr, this means that the conversion
    // to it should fail
    StringConversionData(const char *str_, const wchar_t *wcs_, int flags_ = 0)
        : str(str_), wcs(wcs_), flags(flags_)
    {
    }

    const char * const str;
    const wchar_t * const wcs;

    enum
    {
        TEST_BOTH  = 0, // test both str -> wcs and wcs -> str
        ONLY_MB2WC = 1  // only test str -> wcs conversion
    };

    const int flags;

    // test that the conversion between str and wcs (subject to flags) succeeds
    //
    // the first argument is the index in the test array and is used solely for
    // diagnostics
    void Test(size_t n, wxMBConv& conv) const
    {
        if ( str )
        {
            wxWCharBuffer wbuf = conv.cMB2WC(str);

            if ( wcs )
            {
                CHECK_MESSAGE
                (
                    wbuf.data(),
                    Message(n, "MB2WC failed")
                );

                CHECK_MESSAGE
                (
                    wxStrcmp(wbuf, wcs) == 0,
                    Message(n, "MB2WC", wbuf, wcs)
                );
            }
            else // conversion is supposed to fail
            {
                CHECK_MESSAGE
                (
                    !wbuf.data(),
                    Message(n, "MB2WC succeeded")
                );
            }
        }

        if ( wcs && !(flags & ONLY_MB2WC) )
        {
            wxCharBuffer buf = conv.cWC2MB(wcs);

            if ( str )
            {
                CHECK_MESSAGE
                (
                    buf.data(),
                    Message(n, "WC2MB failed")
                );

                CHECK_MESSAGE
                (
                    strcmp(buf, str) == 0,
                    Message(n, "WC2MB", buf, str)
                );
            }
            else
            {
                CHECK_MESSAGE
                (
                    !buf.data(),
                    Message(n, "WC2MB succeeded")
                );
            }
        }
    }

private:
    static std::string
    Message(size_t n, const wxString& msg)
    {
        return wxString::Format("#%lu: %s", (unsigned long)n, msg).ToStdString();
    }

    template <typename T>
    static std::string
    Message(size_t n,
            const char *func,
            const wxCharTypeBuffer<T>& actual,
            const T *expected)
    {
        return Message(n,
                       wxString::Format("%s returned \"%s\", expected \"%s\"",
                                        func, actual.data(), expected));
    }
};


TEST_CASE("ToFromAscii")
{

#define TEST_TO_FROM_ASCII(txt)                              \
    {                                                        \
        static const char *msg = txt;                        \
        wxString s = wxString::FromAscii(msg);               \
        CHECK( strcmp( s.ToAscii() , msg ) == 0 );  \
    }

    TEST_TO_FROM_ASCII( "Hello, world!" );
    TEST_TO_FROM_ASCII( "additional \" special \t test \\ component \n :-)" );
}

TEST_CASE("ConstructorsWithConversion")
{
    // the string "Déjà" in UTF-8 and wchar_t:
    const unsigned char utf8Buf[] = {0x44,0xC3,0xA9,0x6A,0xC3,0xA0,0};
    const unsigned char utf8subBuf[] = {0x44,0xC3,0xA9,0x6A,0}; // just "Déj"
    const char* utf8 = reinterpret_cast<const char*>(utf8Buf);
    const char* utf8sub = reinterpret_cast<const char*>(utf8subBuf);

    wxString s1(utf8, wxConvUTF8);

    const wchar_t wchar[] = {0x44,0xE9,0x6A,0xE0,0};
    CHECK_EQ( wchar, s1 );

    wxString s2(wchar);
    CHECK_EQ( wchar, s2 );
    CHECK_EQ( wxString::FromUTF8(utf8), s2 );

    wxString sub(utf8sub, wxConvUTF8); // "Dej" substring
    wxString s3(utf8, wxConvUTF8, 4);
    CHECK_EQ( sub, s3 );

    wxString s4(wchar, wxConvUTF8, 3);
    CHECK_EQ( sub, s4 );

    // conversion should stop with failure at pos 35
    wxString s("\t[pl]open.format.Sformatuj dyskietk\xea=gfloppy %f", wxConvUTF8);
    CHECK( s.empty() );

    // test using Unicode strings together with char* strings (this must work
    // in ANSI mode as well, of course):
    wxString s5("ascii");
    CHECK_EQ( "ascii", s5 );

    s5 += " value";

    CHECK( strcmp(s5.mb_str(), "ascii value") == 0 );
    CHECK_EQ( "ascii value", s5 );
    CHECK( s5 != "SomethingElse" );
}

TEST_CASE("ConversionFixed")
{
    size_t len;

    wxConvLibc.cWC2MB(L"", 0, &len);
    CHECK_EQ( 0, len );

    // check that when we convert a fixed number of characters we obtain the
    // expected return value
    CHECK_EQ( 0, wxConvLibc.ToWChar(nullptr, 0, "", 0) );
    CHECK_EQ( 1, wxConvLibc.ToWChar(nullptr, 0, "x", 1) );
    CHECK_EQ( 2, wxConvLibc.ToWChar(nullptr, 0, "x", 2) );
    CHECK_EQ( 2, wxConvLibc.ToWChar(nullptr, 0, "xy", 2) );
}

TEST_CASE("ConversionWithNULs")
{
    static constexpr size_t lenNulString = 10;

    wxString szTheString(L"The\0String", wxConvLibc, lenNulString);
    wxCharBuffer theBuffer = szTheString.mb_str(wxConvLibc);

    CHECK( memcmp(theBuffer.data(), "The\0String",
                    lenNulString + 1) == 0 );

    wxString szTheString2("The\0String", wxConvLocal, lenNulString);
    CHECK_EQ( lenNulString, szTheString2.length() );
    CHECK( wxTmemcmp(szTheString2.c_str(), L"The\0String",
                    lenNulString) == 0 );

    const char *null4buff = "\0\0\0\0";
    wxString null4str(null4buff, 4);
    CHECK_EQ( 4, null4str.length() );
}

TEST_CASE("ConversionUTF7")
{
    static const StringConversionData utf7data[] =
    {
        // normal fragments
        StringConversionData("+AKM-", L"\xa3"),
        StringConversionData("+AOk-t+AOk-", L"\xe9t\xe9"),

        // this one is an alternative valid encoding of the same string
        StringConversionData("+AOk-t+AOk", L"\xe9t\xe9",
                             StringConversionData::ONLY_MB2WC),

        // some special cases
        StringConversionData("+-", L"+"),
        StringConversionData("+--", L"+-"),

        // the following are invalid UTF-7 sequences
        StringConversionData("\xa3", nullptr),
        StringConversionData("+", nullptr),
        StringConversionData("+~", nullptr),
        StringConversionData("a+", nullptr),
    };

    for ( size_t n = 0; n < WXSIZEOF(utf7data); n++ )
    {
        const StringConversionData& d = utf7data[n];

        // converting to/from UTF-7 using iconv() currently doesn't work
        // because of several problems:
        //  - GetMBNulLen() doesn't return correct result (iconv converts L'\0'
        //    to an incomplete and anyhow nonsensical "+AA" string)
        //  - iconv refuses to convert "+-" (although it converts "+-\n" just
        //    fine, go figure)
        //
        // I have no idea how to fix this so just disable the test for now
#ifdef WX_WINDOWS
        wxCSConv conv("utf-7");
        d.Test(n, conv);
#endif
        d.Test(n, wxConvUTF7);
    }
}

TEST_CASE("ConversionUTF8")
{
    static const StringConversionData utf8data[] =
    {
        StringConversionData("\xc2\xa3", L"\u00a3"),
        StringConversionData("\xc2", nullptr),
    };

    wxCSConv conv(wxT("utf-8"));
    for ( size_t n = 0; n < WXSIZEOF(utf8data); n++ )
    {
        const StringConversionData& d = utf8data[n];
        d.Test(n, conv);
        d.Test(n, wxConvUTF8);
    }

    static constexpr char u25a6[] = "\xe2\x96\xa6";
    wxMBConvUTF8 c(wxMBConvUTF8::MAP_INVALID_UTF8_TO_OCTAL);
    CHECK_EQ( 2, c.ToWChar(nullptr, 0, u25a6, wxNO_LEN) );
    CHECK_EQ( 0, c.ToWChar(nullptr, 0, u25a6, 0) );
    CHECK_EQ( 1, c.ToWChar(nullptr, 0, u25a6, 3) );
    CHECK_EQ( 2, c.ToWChar(nullptr, 0, u25a6, 4) );

    // Verify that converting a string with embedded NULs works.
    CHECK_EQ( 5, wxString::FromUTF8("abc\0\x32", 5).length() );

    // Verify that converting a string containing invalid UTF-8 does not work,
    // even if it happens after an embedded NUL.
    CHECK( wxString::FromUTF8("abc\xff").empty() );
    CHECK( wxString::FromUTF8("abc\0\xff", 5).empty() );
}

TEST_CASE("ConversionUTF16")
{
    static const StringConversionData utf16data[] =
    {
        StringConversionData(
            "\x04\x1f\x04\x40\x04\x38\x04\x32\x04\x35\x04\x42\0\0",
            L"\u041f\u0440\u0438\u0432\u0435\u0442"),
        StringConversionData(
            "\x01\0\0b\x01\0\0a\x01\0\0r\0\0",
            L"\u0100b\u0100a\u0100r"),
        StringConversionData("\0f\0o\0o\0\0", L"foo"),
    };

    wxCSConv conv(wxFONTENCODING_UTF16BE);
    for ( size_t n = 0; n < WXSIZEOF(utf16data); n++ )
    {
        const StringConversionData& d = utf16data[n];
        d.Test(n, conv);
    }

    // special case: this string has consecutive NULs inside it which don't
    // terminate the string, this exposed a bug in our conversion code which
    // got confused in this case
    size_t len;
    conv.cMB2WC("\x01\0\0B\0C" /* A macron BC */, 6, &len);
    CHECK_EQ( 3, len );

    // When using UTF-16 internally (i.e. MSW), we don't have any surrogate
    // support, so the length of the string below is 2, not 1.
#if SIZEOF_WCHAR_T == 4
    // Another one: verify that the length of the resulting string is computed
    // correctly when there is a surrogate in the input.
    wxMBConvUTF16BE().cMB2WC("\xd8\x03\xdc\x01\0" /* OLD TURKIC LETTER YENISEI A */, wxNO_LEN, &len);
    CHECK_EQ( 1, len );
#endif // UTF-32 internal representation

#if SIZEOF_WCHAR_T == 2
    // Verify that the length of UTF-32 string is correct even when converting
    // to it from a longer UTF-16 string with surrogates.

    // Construct CAT FACE U+1F431 without using \U which is not supported by
    // ancient compilers and without using \u with surrogates which is
    // (correctly) flagged as an error by the newer ones.
    wchar_t ws[2];
    ws[0] = 0xd83d;
    ws[1] = 0xdc31;
    CHECK_EQ( 4, wxMBConvUTF32BE().FromWChar(nullptr, 0, ws, 2) );
#endif // UTF-16 internal representation
}

TEST_CASE("ConversionUTF32")
{
    static const StringConversionData utf32data[] =
    {
        StringConversionData(
            "\0\0\x04\x1f\0\0\x04\x40\0\0\x04\x38\0\0\x04\x32\0\0\x04\x35\0\0\x04\x42\0\0\0\0",
          L"\u041f\u0440\u0438\u0432\u0435\u0442"),
        StringConversionData("\0\0\0f\0\0\0o\0\0\0o\0\0\0\0", L"foo"),
    };

    wxCSConv conv(wxFONTENCODING_UTF32BE);
    for ( size_t n = 0; n < WXSIZEOF(utf32data); n++ )
    {
        const StringConversionData& d = utf32data[n];
        d.Test(n, conv);
    }

    size_t len;
    conv.cMB2WC("\0\0\x01\0\0\0\0B\0\0\0C" /* A macron BC */, 12, &len);
    CHECK_EQ( 3, len );
}

TEST_CASE("IsConvOk")
{
    CHECK( wxCSConv(wxFONTENCODING_SYSTEM).IsOk() );
    CHECK( wxCSConv("US-ASCII").IsOk() );
    CHECK( wxCSConv("UTF-8").IsOk() );
    CHECK( !wxCSConv("NoSuchConversion").IsOk() );

#ifdef WX_WINDOWS
    CHECK( wxCSConv("WINDOWS-437").IsOk() );
#endif
}

TEST_CASE("Iteration")
{
    // "czech" in Czech ("cestina"):
    static constexpr char textUTF8[] = "\304\215e\305\241tina";
    static constexpr wchar_t textUTF16[] = {0x10D, 0x65, 0x161, 0x74, 0x69, 0x6E, 0x61, 0};

    wxString text(wxString::FromUTF8(textUTF8));
    CHECK( wxStrcmp(text.wc_str(), textUTF16) == 0 );

    // verify the string was decoded correctly:
    {
        size_t idx = 0;
        for ( wxString::const_iterator i = text.begin(); i != text.end(); ++i, ++idx )
        {
            CHECK( *i == textUTF16[idx] );
        }
    }

    // overwrite the string with something that is shorter in UTF-8:
    {
        for ( wxString::iterator i = text.begin(); i != text.end(); ++i )
            *i = 'x';
    }

    // restore the original text now:
    {
        wxString::iterator end1 = text.end();
        wxString::const_iterator end2 = text.end();

        size_t idx = 0;
        for ( wxString::iterator i = text.begin(); i != text.end(); ++i, ++idx )
        {
            *i = textUTF16[idx];

            CHECK( end1 == text.end() );
            CHECK( end2 == text.end() );
        }

        CHECK( end1 == text.end() );
        CHECK( end2 == text.end() );
    }

    // and verify it again:
    {
        size_t idx = 0;
        for ( wxString::const_iterator i = text.begin(); i != text.end(); ++i, ++idx )
        {
            CHECK( *i == textUTF16[idx] );
        }
    }
}

