///////////////////////////////////////////////////////////////////////////////
// Name:        tests/graphics/ellipsization.cpp
// Purpose:     wxControlBase::*Ellipsize* unit test
// Author:      Francesco Montorsi
// Created:     2010-03-10
// Copyright:   (c) 2010 Francesco Montorsi
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#include "wx/control.h"
#include "wx/dcmemory.h"
#include "wx/stringutils.h"

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

TEST_CASE("Ellipsization::NormalCase")
{
    wxMemoryDC dc;

    static const char *stringsToTest[] =
    {
        "N",
        ".",
        "x",
        "foobar",
        "\xCE\xB1", // U03B1 (GREEK SMALL LETTER ALPHA)
        "Another test",
        "a very very very very very very very long string",
        // alpha+beta+gamma+delta+epsilon+zeta+eta+theta+iota
        "\xCE\xB1\xCE\xB2\xCE\xB3\xCE\xB4\xCE\xB5\xCE\xB6\xCE\xB7\xCE\xB8\xCE\xB9",
        "\t", "\t\t\t\t\t", "a\tstring\twith\ttabs",
        "\n", "\n\n\n\n\n", "a\nstring\nwith\nnewlines",
        "&", "&&&&&&&", "a&string&with&newlines",
        "\t\n&", "a\t\n&string\t\n&with\t\n&many\t\n&chars"
    };

    static constexpr int flagsToTest[] =
    {
        0,
        wxELLIPSIZE_FLAGS_PROCESS_MNEMONICS,
        wxELLIPSIZE_FLAGS_EXPAND_TABS,
        wxELLIPSIZE_FLAGS_PROCESS_MNEMONICS | wxELLIPSIZE_FLAGS_EXPAND_TABS
    };

    static constexpr wxEllipsizeMode modesToTest[] =
    {
        wxEllipsizeMode::Start,
        wxEllipsizeMode::Middle,
        wxEllipsizeMode::End
    };

    const int charWidth = dc.wxGetCharWidth();
    int widthsToTest[] = { 6*charWidth, 10*charWidth, 15*charWidth };

    for ( unsigned int s = 0; s < WXSIZEOF(stringsToTest); s++ )
    {
        const std::string str = std::string(stringsToTest[s]);

        for ( unsigned int  f = 0; f < WXSIZEOF(flagsToTest); f++ )
        {
            for ( unsigned int m = 0; m < WXSIZEOF(modesToTest); m++ )
            {
                for ( unsigned int w = 0; w < WXSIZEOF(widthsToTest); w++ )
                {
                    std::string ret = wxControl::Ellipsize
                                   (
                                    str,
                                    dc,
                                    modesToTest[m],
                                    widthsToTest[w],
                                    flagsToTest[f]
                                   );

                    // Note that we must measure the width of the text that
                    // will be rendered, and when mnemonics are used, this
                    // means we have to remove them first.
                    const std::string
                        displayed = flagsToTest[f] & wxELLIPSIZE_FLAGS_PROCESS_MNEMONICS
                                        ? wxControl::RemoveMnemonics(ret)
                                        : ret;
                    const int
                        width = dc.GetMultiLineTextExtent(displayed).GetWidth();

                    CHECK_MESSAGE
                    (
                        width <= widthsToTest[w],
                     (
                        "Test #(%u,%u.%u): %s\n\"%s\" -> \"%s\"; width=%dpx > %dpx",
                        s, f, m,
                        dc.GetFont().GetNativeFontInfoUserDesc(),
                        str,
                        ret,
                        width,
                        widthsToTest[w]
                     )
                    );
                }
            }
        }
    }
}


TEST_CASE("Ellipsization::EnoughSpace")
{
    // No ellipsization should occur if there's plenty of space.

    wxMemoryDC dc;

    std::string testString{"some label"};
    const int width = dc.GetTextExtent(testString).GetWidth() + 50;

    CHECK( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Start, width) == testString );
    CHECK( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Middle, width) == testString );
    CHECK( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::End, width) == testString );
}


TEST_CASE("Ellipsization::VeryLittleSpace")
{
    // If there's not enough space, the shortened label should still contain "..." and one character

    wxMemoryDC dc;

    const int width = dc.GetTextExtent("s...").GetWidth();

    CHECK_EQ( wxControl::Ellipsize("some label", dc, wxEllipsizeMode::Start, width), "...l" );
    CHECK_EQ( wxControl::Ellipsize("some label", dc, wxEllipsizeMode::Middle, width), "s..." );
    CHECK_EQ( wxControl::Ellipsize("some label1", dc, wxEllipsizeMode::Middle, width), "s..." );
    CHECK_EQ( wxControl::Ellipsize("some label", dc, wxEllipsizeMode::End, width), "s..." );
}


TEST_CASE("Ellipsization::HasThreeDots")
{
    wxMemoryDC dc;

    std::string testString("some longer text");
    const int width = dc.GetTextExtent(testString).GetWidth() - 5;

    CHECK( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Start, width).starts_with("..."));
    CHECK_FALSE( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Start, width).ends_with("..."));

    CHECK( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::End, width).ends_with("...") );

    CHECK( wx::utils::Contains(wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Middle, width), "...") );
    CHECK_FALSE( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Middle, width).starts_with("...") );
    CHECK_FALSE( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Middle, width).ends_with("...") );
}
