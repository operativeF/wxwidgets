///////////////////////////////////////////////////////////////////////////////
// Name:        tests/formatconverter/formatconverter.cpp
// Purpose:     Test wxFormatConverter
// Author:      Mike Wetherell
// Copyright:   (c) 2004 Mike Wetherell
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

//
// Notes:
//
// The conversions wxFormatConverter currently does are as follows:
//
//    %s, %lS   ->  %ls
//    %S, %hS   ->  %s
//    %c, %lC   ->  %lc
//    %C, %hC   ->  %c
//
// %hs and %hc stay the same.
//
// %S and %C aren't actually in the ISO C or C++ standards, but they can be
// avoided when writing portable code.
//
// Nor are %hs or %hc in the standards, which means wxWidgets currently doesn't
// have a specifier for 'char' types that is ok for all builds and platforms.
//
// The effect of using %hs/%hc is undefined, though RTLs are quite likely
// to just ignore the 'h', so maybe it works as required even though it's
// not legal.
//
// I've put in some checks, such as this which will flag up any platforms
// where this is not the case:
//
//  CHECK(wxString::Format(wxT("%hs"), "test") == wxT("test"));
//

#include "doctest.h"

import <iostream>;
import <string>;

using std::string;

///////////////////////////////////////////////////////////////////////////////
// The test case
//
// wxFormatConverter only changes %s, %c, %S and %C, all others are treated
// equally, therefore it is enough to choose just one other for testing, %d
// will do.

static void check(const wxString& input,
    const wxString& expectedScanf,
    const wxString& expectedUtf8,
    const wxString& expectedWcharUnix,
    const wxString& expectedWcharWindows)
{
    // all of them are unused in some build configurations
    wxUnusedVar(expectedScanf);
    wxUnusedVar(expectedUtf8);
    wxUnusedVar(expectedWcharUnix);
    wxUnusedVar(expectedWcharWindows);

    wxString result, msg;

#ifndef WX_WINDOWS
    // on windows, wxScanf() string needs no modifications
    result = wxScanfConvertFormatW(input.wc_str());

    msg = wxT("input: '") + input +
        wxT("', result (scanf): '") + result +
        wxT("', expected: '") + expectedScanf + wxT("'");
    CPPUNIT_ASSERT_MESSAGE(string(msg.mb_str()), result == expectedScanf);
#endif // !WX_WINDOWS

#if wxUSE_UNICODE_UTF8
    result = (const char*)wxFormatString(input);

    msg = wxT("input: '") + input +
        wxT("', result (UTF-8): '") + result +
        wxT("', expected: '") + expectedUtf8 + wxT("'");
    CPPUNIT_ASSERT_MESSAGE(string(msg.mb_str()), result == expectedUtf8);
#endif // wxUSE_UNICODE_UTF8

#if !wxUSE_UTF8_LOCALE_ONLY
    result = (const wchar_t*)wxFormatString(input);

#if defined(WX_WINDOWS) && \
    !defined(__CYGWIN__) && \
    !defined(__MINGW32__)
    wxString expectedWchar(expectedWcharWindows);
#else
    wxString expectedWchar(expectedWcharUnix);
#endif

    msg = wxT("input: '") + input +
        wxT("', result (wchar_t): '") + result +
        wxT("', expected: '") + expectedWchar + wxT("'");
    CHECK_MESSAGE(result == expectedWchar, string(msg.mb_str()));
#endif // !wxUSE_UTF8_LOCALE_ONLY
}

static void doTest(const char* input,
    const char* expectedScanf,
    const char* expectedUtf8,
    const char* expectedWcharUnix,
    const char* expectedWcharWindows)
{
    static const wxChar* flag_width[] =
    { wxT(""), wxT("*"), wxT("10"), wxT("-*"), wxT("-10"), NULL };
    static const wxChar* precision[] =
    { wxT(""), wxT(".*"), wxT(".10"), NULL };
    static const wxChar* empty[] =
    { wxT(""), NULL };

    // no precision for %c or %C
    const wxChar** precs = wxTolower(input[wxStrlen(input) - 1]) == wxT('c') ?
        empty : precision;

    wxString fmt(wxT("%"));

    // try the test for a variety of combinations of flag, width and precision
    for (const wxChar** prec = precs; *prec; prec++)
        for (const wxChar** width = flag_width; *width; width++)
            check(fmt + *width + *prec + input,
                fmt + *width + *prec + expectedScanf,
                fmt + *width + *prec + expectedUtf8,
                fmt + *width + *prec + expectedWcharUnix,
                fmt + *width + *prec + expectedWcharWindows);
}

TEST_CASE("format_d")
{
    doTest("d", "d", "d", "d", "d");
    CHECK(wxString::Format(wxT("%d"), 255) == wxT("255"));
    CHECK(wxString::Format(wxT("%05d"), 255) == wxT("00255"));
    CHECK(wxString::Format(wxT("% 5d"), 255) == wxT("  255"));
    CHECK(wxString::Format(wxT("% 5d"), -255) == wxT(" -255"));
    CHECK(wxString::Format(wxT("%-5d"), -255) == wxT("-255 "));
    CHECK(wxString::Format(wxT("%+5d"), 255) == wxT(" +255"));
    CHECK(wxString::Format(wxT("%*d"), 5, 255) == wxT("  255"));
}

TEST_CASE("format_hd")
{
    doTest("hd", "hd", "hd", "hd", "hd");
    short s = 32767;
    CHECK(wxString::Format(wxT("%hd"), s) == wxT("32767"));
}

TEST_CASE("format_ld")
{
    doTest("ld", "ld", "ld", "ld", "ld");
    long l = 2147483647L;
    CHECK(wxString::Format(wxT("%ld"), l) == wxT("2147483647"));
}

TEST_CASE("format_s")
{
    doTest("s", "ls", "s", "ls", "s");
    CHECK(wxString::Format(wxT("%s!"), wxT("test")) == wxT("test!"));
    CHECK(wxString::Format(wxT("%6s!"), wxT("test")) == wxT("  test!"));
    CHECK(wxString::Format(wxT("%-6s!"), wxT("test")) == wxT("test  !"));
    CHECK(wxString::Format(wxT("%.6s!"), wxT("test")) == wxT("test!"));
    CHECK(wxString::Format(wxT("%6.4s!"), wxT("testing")) == wxT("  test!"));
}

TEST_CASE("format_hs")
{
    doTest("hs", "hs", "s", "ls", "s");
    CHECK(wxString::Format(wxString(wxT("%hs!")), "test") == wxT("test!"));
    CHECK(wxString::Format(wxString(wxT("%6hs!")), "test") == wxT("  test!"));
    CHECK(wxString::Format(wxString(wxT("%-6hs!")), "test") == wxT("test  !"));
    CHECK(wxString::Format(wxString(wxT("%.6hs!")), "test") == wxT("test!"));
    CHECK(wxString::Format(wxString(wxT("%6.4hs!")), "testing") == wxT("  test!"));
}

TEST_CASE("format_ls")
{
    doTest("ls", "ls", "s", "ls", "s");
    CHECK(wxString::Format(wxT("%ls!"), L"test") == wxT("test!"));
    CHECK(wxString::Format(wxT("%6ls!"), L"test") == wxT("  test!"));
    CHECK(wxString::Format(wxT("%-6ls!"), L"test") == wxT("test  !"));
    CHECK(wxString::Format(wxT("%.6ls!"), L"test") == wxT("test!"));
    CHECK(wxString::Format(wxT("%6.4ls!"), L"testing") == wxT("  test!"));
}

TEST_CASE("format_c")
{
    doTest("c", "lc", "lc", "lc", "c");
    CHECK(wxString::Format(wxT("%c"), wxT('x')) == wxT("x"));
    CHECK(wxString::Format(wxT("%2c"), wxT('x')) == wxT(" x"));
    CHECK(wxString::Format(wxT("%-2c"), wxT('x')) == wxT("x "));
}

TEST_CASE("format_hc")
{
    doTest("hc", "hc", "lc", "lc", "c");
    CHECK(wxString::Format(wxString(wxT("%hc")), 'x') == wxT("x"));
    CHECK(wxString::Format(wxString(wxT("%2hc")), 'x') == wxT(" x"));
    CHECK(wxString::Format(wxString(wxT("%-2hc")), 'x') == wxT("x "));
}

TEST_CASE("format_lc")
{
    doTest("lc", "lc", "lc", "lc", "c");
    CHECK(wxString::Format(wxT("%lc"), L'x') == wxT("x"));
    CHECK(wxString::Format(wxT("%2lc"), L'x') == wxT(" x"));
    CHECK(wxString::Format(wxT("%-2lc"), L'x') == wxT("x "));
}


TEST_CASE("format_S")
    { doTest("S",  "s", "s", "ls", "s");  }
TEST_CASE("format_hS")
    { doTest("hS", "s", "s", "ls", "s");  }
TEST_CASE("format_lS")
    { doTest("lS", "ls", "s", "ls", "s"); }

TEST_CASE("format_C")
    { doTest("C",  "c", "lc", "lc", "c");  }
TEST_CASE("format_hC")
    { doTest("hC", "c", "lc", "lc", "c");  }
TEST_CASE("format_lC")
    { doTest("lC", "lc", "lc", "lc", "c"); }

// It's possible that although a format converts correctly alone, it leaves
// the converter in a bad state that will affect subsequent formats, so
// check with a selection of longer patterns.
//
TEST_CASE("Long pattern test")
{
    struct {
        const char *input;
        const char *expectedScanf;
        const char *expectedWcharUnix;
        const char *expectedWcharWindows;
        const char *expectedUtf8;
    } formats[] = {
        { "%d",     "%d",     "%d",     "%d",    "%d"    },
        { "%*hd",   "%*hd",   "%*hd",   "%*hd",  "%*hd"  },
        { "%.4ld",  "%.4ld",  "%.4ld",  "%.4ld", "%.4ld" },
        { "%-.*s",  "%-.*ls", "%-.*ls", "%-.*s", "%-.*s" },
        { "%.*hs",  "%.*hs",  "%.*ls",  "%.*s",  "%.*s"  },
        { "%-.9ls", "%-.9ls", "%-.9ls", "%-.9s", "%-.9s" },
        { "%-*c",   "%-*lc",  "%-*lc",  "%-*c",  "%-*lc" },
        { "%3hc",   "%3hc",   "%3lc",   "%3c",   "%3lc"  },
        { "%-5lc",  "%-5lc",  "%-5lc",  "%-5c",  "%-5lc" }
    };
    size_t i, j;

    // test all possible pairs of the above patterns
    for (i = 0; i < WXSIZEOF(formats); i++) {
        if (formats[i].input) {
            wxString input(formats[i].input);
            wxString expectedScanf(formats[i].expectedScanf);
            wxString expectedUtf8(formats[i].expectedUtf8);
            wxString expectedWcharUnix(formats[i].expectedWcharUnix);
            wxString expectedWcharWindows(formats[i].expectedWcharWindows);

            for (j = 0; j < WXSIZEOF(formats); j++)
                if (formats[j].input)
                    check(input + formats[j].input,
                          expectedScanf + formats[j].expectedScanf,
                          expectedUtf8 + formats[j].expectedUtf8,
                          expectedWcharUnix + formats[j].expectedWcharUnix,
                          expectedWcharWindows + formats[j].expectedWcharWindows);
        }
    }
}
