///////////////////////////////////////////////////////////////////////////////
// Name:        tests/mbconv/convauto.cpp
// Purpose:     wxConvAuto unit test
// Author:      Vadim Zeitlin
// Created:     2006-04-04
// Copyright:   (c) 2006 Vadim Zeitlin
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/convauto.h"

import WX.Cmn.MemStream;
import WX.Cmn.TextStream;

// ----------------------------------------------------------------------------
// tests
// ----------------------------------------------------------------------------

    // expected converter state, UTF-8 without BOM by default
struct ConvState
{
    ConvState(wxBOM bom = wxBOM::None,
        wxFontEncoding enc = wxFONTENCODING_UTF8,
        bool fallback = false)
        : m_bom(bom), m_enc(enc), m_fallback(fallback) {}

    void Check(const wxConvAuto& conv) const
    {
        CHECK(conv.GetBOM() == m_bom);
        CHECK(conv.GetEncoding() == m_enc);
        CHECK(conv.IsUsingFallbackEncoding() == m_fallback);
        CHECK(conv.IsUTF8() == (m_enc == wxFONTENCODING_UTF8));
    }

    wxBOM m_bom;
    wxFontEncoding m_enc;
    bool m_fallback;
};

static void TestFirstChar(const char *src, wchar_t wch, size_t len = wxNO_LEN,
                          ConvState st = ConvState(), wxFontEncoding fe = wxFONTENCODING_DEFAULT)
{
    wxConvAuto conv(fe);
    wxWCharBuffer wbuf = conv.cMB2WC(src, len, NULL);
    CHECK( wbuf );
    CHECK_EQ( wch, *wbuf );
    st.Check(conv);
}

// real test function: check that converting the src multibyte string to
// wide char using wxConvAuto yields wch as the first result
//
// the length of the string may need to be passed explicitly if it has
// embedded NULs, otherwise it's not necessary
static void TestTextStream(const char* src,
    size_t srclength,
    const wxString& line1,
    const wxString& line2,
    wxFontEncoding fe = wxFONTENCODING_DEFAULT)
{
    wxMemoryInputStream instream(src, srclength);
    wxTextInputStream text(instream, wxT(" \t"), wxConvAuto(fe));

    CHECK_EQ(line1, text.ReadLine());
    CHECK_EQ(line2, text.ReadLine());
}

TEST_CASE("Init")
{
    ConvState(wxBOM::Unknown, wxFONTENCODING_MAX).Check(wxConvAuto());
}

TEST_CASE("Empty")
{
    wxConvAuto conv;
    CHECK( !conv.cMB2WC("") );
    ConvState(wxBOM::Unknown, wxFONTENCODING_MAX).Check(conv);
}

TEST_CASE("Encode")
{
    wxConvAuto conv;
    wxString str = wxString::FromUTF8("\xd0\x9f\xe3\x81\x82");
    wxCharBuffer buf = conv.cWC2MB(str.wc_str());
    CHECK( buf );
    CHECK_EQ( str, wxString::FromUTF8(buf) );
    ConvState(wxBOM::Unknown, wxFONTENCODING_UTF8).Check(conv);
}

TEST_CASE("Short")
{
    TestFirstChar("1", wxT('1'));
}

TEST_CASE("None")
{
    TestFirstChar("Hello world", wxT('H'));
}

TEST_CASE("UTF32LE")
{
    TestFirstChar("\xff\xfe\0\0A\0\0\0", wxT('A'), 8, ConvState(wxBOM::UTF32LE, wxFONTENCODING_UTF32LE));
}

TEST_CASE("UTF32BE")
{
    TestFirstChar("\0\0\xfe\xff\0\0\0B", wxT('B'), 8, ConvState(wxBOM::UTF32BE, wxFONTENCODING_UTF32BE));
}

TEST_CASE("UTF16LE")
{
    TestFirstChar("\xff\xfeZ\0", wxT('Z'), 4, ConvState(wxBOM::UTF16LE, wxFONTENCODING_UTF16LE));
}

TEST_CASE("UTF16BE")
{
    TestFirstChar("\xfe\xff\0Y", wxT('Y'), 4, ConvState(wxBOM::UTF16BE, wxFONTENCODING_UTF16BE));
}

TEST_CASE("UTF8")
{
    TestFirstChar("\xef\xbb\xbf\xd0\x9f", L'\u041f', wxNO_LEN, ConvState(wxBOM::UTF8, wxFONTENCODING_UTF8));
}

TEST_CASE("UTF8NoBom")
{
    TestFirstChar("\xd0\x9f\xe3\x81\x82", L'\u041f', wxNO_LEN, ConvState(wxBOM::None, wxFONTENCODING_UTF8));
}

TEST_CASE("Fallback")
{
    TestFirstChar("\xbf", L'\u041f', wxNO_LEN,
                  ConvState(wxBOM::None, wxFONTENCODING_ISO8859_5, true),
                  wxFONTENCODING_ISO8859_5);
}

TEST_CASE("FallbackMultibyte")
{
    TestFirstChar("\x84\x50", L'\u041f', wxNO_LEN,
                  ConvState(wxBOM::None, wxFONTENCODING_CP932, true),
                  wxFONTENCODING_CP932);
}

TEST_CASE("FallbackShort")
{
    TestFirstChar("\x61\xc4", 'a', 2,
                  ConvState(wxBOM::None, wxFONTENCODING_ISO8859_5, true),
                  wxFONTENCODING_ISO8859_5);
}

// the first line of the teststring used in the following functions is an
// 'a' followed by a Japanese hiragana A (u+3042).
// The second line is a single Greek beta (u+03B2). There is no blank line
// at the end.


static const wxString line1 = wxString::FromUTF8("a\xe3\x81\x82");
static const wxString line2 = wxString::FromUTF8("\xce\xb2");


TEST_CASE("StreamUTF8NoBOM")
{
    TestTextStream("\x61\xE3\x81\x82\x0A\xCE\xB2",
                   7, line1, line2);
}

TEST_CASE("StreamUTF8")
{
    TestTextStream("\xEF\xBB\xBF\x61\xE3\x81\x82\x0A\xCE\xB2",
                   10, line1, line2);
}

TEST_CASE("StreamUTF16LE")
{
    TestTextStream("\xFF\xFE\x61\x00\x42\x30\x0A\x00\xB2\x03",
                   10, line1, line2);
}

TEST_CASE("StreamUTF16BE")
{
    TestTextStream("\xFE\xFF\x00\x61\x30\x42\x00\x0A\x03\xB2",
                   10, line1, line2);
}

TEST_CASE("StreamUTF32LE")
{
    TestTextStream("\xFF\xFE\0\0\x61\x00\0\0\x42\x30\0\0\x0A"
                   "\x00\0\0\xB2\x03\0\0",
                   20, line1, line2);
}

TEST_CASE("StreamUTF32BE")
{
    TestTextStream("\0\0\xFE\xFF\0\0\x00\x61\0\0\x30\x42\0\0\x00\x0A"
                   "\0\0\x03\xB2",
                   20, line1, line2);
}

TEST_CASE("StreamFallback")
{
    TestTextStream("\x61\xbf\x0A\xe0",
                   4, wxString::FromUTF8("a\xd0\x9f"), wxString::FromUTF8("\xd1\x80"),
                   wxFONTENCODING_ISO8859_5);
}

TEST_CASE("StreamFallbackMultibyte")
{
    TestTextStream("\x61\x82\xa0\x0A\x83\xc0",
                   6, line1, line2, wxFONTENCODING_CP932);
}

