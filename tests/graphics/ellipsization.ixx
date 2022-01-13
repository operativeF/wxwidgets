///////////////////////////////////////////////////////////////////////////////
// Name:        tests/graphics/ellipsization.cpp
// Purpose:     wxControlBase::*Ellipsize* unit test
// Author:      Francesco Montorsi
// Created:     2010-03-10
// Copyright:   (c) 2010 Francesco Montorsi
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/control.h"
#include "wx/dcmemory.h"

export module WX.Test.Ellipsization;

import WX.MetaTest;
import WX.Test.Prec;

import Utils.Strings;

import <array>;
import <string_view>;

namespace ut = boost::ut;

ut::suite EllipsizationTest = []
{
    using namespace ut;

    "Ellipsization::NormalCase"_test = []
    {
        wxMemoryDC dc;

        static constexpr std::array<std::string_view, 19> stringsToTest =
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
            "\t",
            "\t\t\t\t\t",
            "a\tstring\twith\ttabs",
            "\n",
            "\n\n\n\n\n",
            "a\nstring\nwith\nnewlines",
            "&",
            "&&&&&&&",
            "a&string&with&newlines",
            "\t\n&",
            "a\t\n&string\t\n&with\t\n&many\t\n&chars"
        };

        static constexpr std::array<EllipsizeFlags, 4> flagsToTest =
        {
            EllipsizeFlags{},
            EllipsizeFlags{wxEllipsizeFlags::ProcessMnemonics},
            EllipsizeFlags{wxEllipsizeFlags::ExpandTabs},
            EllipsizeFlags{wxEllipsizeFlags::Default}
        };

        static constexpr std::array<wxEllipsizeMode, 3> modesToTest =
        {
            wxEllipsizeMode::Start,
            wxEllipsizeMode::Middle,
            wxEllipsizeMode::End
        };

        const int charWidth = dc.wxGetCharWidth();
        int widthsToTest[] = { 6*charWidth, 10*charWidth, 15*charWidth };

        for ( auto&& str : stringsToTest )
        {
            for ( auto&& flagTT : flagsToTest )
            {
                for ( auto&& modeTT : modesToTest )
                {
                    for ( auto&& widthTT : widthsToTest )
                    {
                        std::string ret = wxControl::Ellipsize
                                    (
                                        str,
                                        dc,
                                        modeTT,
                                        widthTT,
                                        flagTT
                                    );

                        // Note that we must measure the width of the text that
                        // will be rendered, and when mnemonics are used, this
                        // means we have to remove them first.
                        const std::string
                            displayed = flagTT & wxEllipsizeFlags::ProcessMnemonics
                                            ? wxControl::RemoveMnemonics(ret)
                                            : ret;
                        const int
                            width = dc.GetMultiLineTextExtent(displayed).GetWidth();

                        expect(width <= widthTT) <<
                            fmt::format("Test failed with: %s\n\"%s\" -> \"%s\"; width=%dpx > %dpx",
                                dc.GetFont().GetNativeFontInfoUserDesc(),
                                str,
                                ret,
                                width,
                                widthTT);
                    }
                }
            }
        }
    };

    "Ellipsization::EnoughSpace"_test = []
    {
        // No ellipsization should occur if there's plenty of space.

        wxMemoryDC dc;

        std::string_view testString{"some label"};
        const int width = dc.GetTextExtent(testString).GetWidth() + 50;

        expect( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Start, width) == testString );
        expect( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Middle, width) == testString );
        expect( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::End, width) == testString );
    };

    "Ellipsization::VeryLittleSpace"_test = []
    {
        // If there's not enough space, the shortened label should still contain "..." and one character

        wxMemoryDC dc;

        const int width = dc.GetTextExtent("s...").GetWidth();

        expect( wxControl::Ellipsize("some label", dc, wxEllipsizeMode::Start, width) == "...l" );
        expect( wxControl::Ellipsize("some label", dc, wxEllipsizeMode::Middle, width) == "s..." );
        expect( wxControl::Ellipsize("some label1", dc, wxEllipsizeMode::Middle, width) == "s..." );
        expect( wxControl::Ellipsize("some label", dc, wxEllipsizeMode::End, width) == "s..." );
    };

    "Ellipsization::HasThreeDots"_test = []
    {
        wxMemoryDC dc;

        std::string_view testString("some longer text");
        const int width = dc.GetTextExtent(testString).GetWidth() - 5;

        expect( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Start, width).starts_with("..."));
        expect( !wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Start, width).ends_with("..."));

        expect( wxControl::Ellipsize(testString, dc, wxEllipsizeMode::End, width).ends_with("...") );

        expect( wx::utils::Contains(wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Middle, width), "...") );
        expect( !wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Middle, width).starts_with("...") );
        expect( !wxControl::Ellipsize(testString, dc, wxEllipsizeMode::Middle, width).ends_with("...") );
    };
};