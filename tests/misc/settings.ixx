///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/settings.cpp
// Purpose:     test wxSettings
// Author:      Francesco Montorsi
// Created:     2009-03-24
// Copyright:   (c) 2009 Francesco Montorsi
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "wx/brush.h"
#include "wx/pen.h"
#include "wx/fontenum.h"

#include <fmt/core.h>

export module WX.Test.Settings;

import WX.Test.Prec;

import WX.Utils.Settings;
import WX.MetaTest;

import <algorithm>;
import <ranges>;

namespace ut = boost::ut;

// NOTE / FIXME: Cannot convert to new test framework.
// C:\dev\wxWidgets\tests\misc\settings.ixx(156,1): fatal error C1001: Internal compiler error.
// (compiler file 'msc1.cpp', line 1691)
// Most likely cannot reach static member functions.
TEST_CASE("Get colour")
{
    for (unsigned int i=wxSYS_COLOUR_SCROLLBAR; i < wxSYS_COLOUR_MAX; i++)
        CHECK( wxSystemSettings::GetColour((wxSystemColour)i).IsOk() );
}

TEST_CASE("Get font")
{
    static constexpr std::array ids =
    {
        wxSYS_OEM_FIXED_FONT,
        wxSYS_ANSI_FIXED_FONT,
        wxSYS_ANSI_VAR_FONT,
        wxSYS_SYSTEM_FONT,
        wxSYS_DEVICE_DEFAULT_FONT,
        wxSYS_SYSTEM_FIXED_FONT,
        wxSYS_DEFAULT_GUI_FONT
    };

    std::ranges::for_each(ids,
        [](const auto& id) {
            const wxFont& font = wxSystemSettings::GetFont(id);
            CHECK(font.IsOk());
            CHECK(wxFontEnumerator::IsValidFacename(font.GetFaceName()));
        });
}

export
{

ut::suite GlobalColorTest = []
{
    using namespace ut;

    static const std::array cols =
    {
        *wxBLACK,
        *wxBLUE,
        *wxCYAN,
        *wxGREEN,
        *wxLIGHT_GREY,
        *wxRED,
        *wxWHITE
    };

    std::ranges::for_each(cols,
        [](const auto& col) {
            expect( col.IsOk() );
        });
};

ut::suite GlobalFontTest = []
{
    using namespace ut;

    static const std::array fonts =
    {
        *wxNORMAL_FONT,
        *wxSMALL_FONT,
        *wxITALIC_FONT,
        *wxSWISS_FONT
    };

    std::ranges::for_each(fonts,
        [](const auto& font)
    {
        expect( font.IsOk() );

        const auto facename = font.GetFaceName();

        if ( !facename.empty() )
        {
            expect(wxFontEnumerator::IsValidFacename(facename)) <<
                fmt::format("facename \"%s\" is invalid", facename);
        }
    });
};

ut::suite GlobalBrushTest = []
{
    using namespace ut;

    static const std::array brushes =
    {
        *wxBLACK_BRUSH,
        *wxBLUE_BRUSH,
        *wxCYAN_BRUSH,
        *wxGREEN_BRUSH,
        *wxGREY_BRUSH,
        *wxLIGHT_GREY_BRUSH,
        *wxMEDIUM_GREY_BRUSH,
        *wxRED_BRUSH,
        *wxTRANSPARENT_BRUSH,
        *wxWHITE_BRUSH
    };

    std::ranges::for_each(brushes,
        [](const auto& brush) {
            expect(brush.IsOk());
        });
};

ut::suite GlobalPensTest = []
{
    using namespace ut;

    static const std::array pens =
    {
        *wxBLACK_DASHED_PEN,
        *wxBLACK_PEN,
        *wxBLUE_PEN,
        *wxCYAN_PEN,
        *wxGREEN_PEN,
        *wxGREY_PEN,
        *wxLIGHT_GREY_PEN,
        *wxMEDIUM_GREY_PEN,
        *wxRED_PEN,
        *wxTRANSPARENT_PEN,
        *wxWHITE_PEN
    };

    std::ranges::for_each(pens,
        [](const auto& pen) {
            expect(pen.IsOk());
        });
};

} // export
