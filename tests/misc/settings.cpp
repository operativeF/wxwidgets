///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/settings.cpp
// Purpose:     test wxSettings
// Author:      Francesco Montorsi
// Created:     2009-03-24
// Copyright:   (c) 2009 Francesco Montorsi
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "wx/brush.h"
#include "wx/pen.h"
#include "wx/settings.h"
#include "wx/fontenum.h"

import WX.Test.Prec;

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

    std::for_each(ids.cbegin(), ids.cend(),
        [](const auto& id) {
            const wxFont& font = wxSystemSettings::GetFont(id);
            CHECK(font.IsOk());
            CHECK(wxFontEnumerator::IsValidFacename(font.GetFaceName()));
        });
}

TEST_CASE("Get global colours")
{
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

    std::for_each(cols.cbegin(), cols.cend(),
        [](const auto& col) {
            CHECK( col.IsOk() );
        });
}

TEST_CASE("Get global fonts")
{
    static const std::array fonts =
    {
        *wxNORMAL_FONT,
        *wxSMALL_FONT,
        *wxITALIC_FONT,
        *wxSWISS_FONT
    };

    std::for_each(fonts.cbegin(), fonts.cend(),
        [](const auto& font) {
        CHECK( font.IsOk() );

        const wxString facename = font.GetFaceName();
        if ( !facename.empty() )
        {
            CHECK_MESSAGE(
                wxFontEnumerator::IsValidFacename(facename),
                ("font #%s: facename \"%s\" is invalid", font, facename)
            );
        }
    });
}

TEST_CASE("Get global brushes")
{
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

    std::for_each(brushes.cbegin(), brushes.cend(),
        [](const auto& brush) {
            CHECK(brush.IsOk());
        });
}

TEST_CASE("Get global pens")
{
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

    std::for_each(pens.cbegin(), pens.cend(),
        [](const auto& pen) {
            CHECK(pen.IsOk());
        });
}
