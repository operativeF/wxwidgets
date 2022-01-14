///////////////////////////////////////////////////////////////////////////////
// Name:        tests/font/fonttest.cpp
// Purpose:     wxFont unit test
// Author:      Francesco Montorsi
// Created:     16.3.09
// Copyright:   (c) 2009 Francesco Montorsi
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"
#include "wx/font.h"

#include <fmt/core.h>

export module WX.Test.Fonts;

import WX.Test.Prec;
import WX.MetaTest;
import Utils.Strings;

import <array>;
import <string>;

// ----------------------------------------------------------------------------
// local helpers
// ----------------------------------------------------------------------------

namespace ut = boost::ut;

// Returns a point to an array of fonts and fills the output parameter with the
// number of elements in this array.
const auto GetTestFonts(unsigned& numFonts)
{
    static const auto testfonts = std::to_array<wxFont>
    ({
        *wxNORMAL_FONT,
        *wxSMALL_FONT,
        *wxITALIC_FONT,
        *wxSWISS_FONT,
        wxFont(5, wxFontFamily::Teletype, wxFontStyle::Normal, wxFONTWEIGHT_NORMAL)
    });

    numFonts = testfonts.size();

    return testfonts;
}

std::string DumpFont(const wxFont *font)
{
    using namespace ut;
    // dumps the internal properties of a wxFont in the same order they
    // are checked by wxFontBase::operator==()

    expect(font->IsOk());

    // FIXME: Could use source_location to get enum names.
    return fmt::format("{}-{};{}-{}-{}-{}-{}-{}-{}",
             font->GetPointSize(),
             font->GetPixelSize().x,
             font->GetPixelSize().y,
             static_cast<int>(font->GetFamily()),
             static_cast<int>(font->GetStyle()),
             static_cast<int>(font->GetWeight()),
             font->GetUnderlined() ? 1 : 0,
             font->GetFaceName(),
             static_cast<int>(font->GetEncoding()));
}

// ----------------------------------------------------------------------------
// the tests
// ----------------------------------------------------------------------------

ut::suite wxFontConstructTests = []
{
    // The main purpose of this test is to verify that the font ctors below
    // compile because it's easy to introduce ambiguities due to the number of
    // overloaded wxFont ctors.

    using namespace ut;

    expect( wxFont(10,
                   wxFontFamily::Default,
                   wxFontStyle::Normal,
                   wxFONTWEIGHT_NORMAL).IsOk() );
};

ut::suite FontSizeTests = []
{
    using namespace ut;

    const struct Sizes
    {
        int specified;      // Size in points specified in the ctor.
        int expected;       // Expected GetPointSize() return value,
                            // -1 here means "same as wxNORMAL_FONT".
    } sizes[] =
    {
        {  9,  9 },
        { 10, 10 },
        { 11, 11 },
        { -1, -1 },
        { 70, -1 }, // 70 == wxDEFAULT, should be handled specially
        { 90, 90 }, // 90 == wxNORMAL, should not be handled specially
    };

    const int sizeDefault = wxFont(wxFontInfo()).GetPointSize();

    for ( size_t n = 0; n < WXSIZEOF(sizes); n++ )
    {
        const Sizes& size = sizes[n];

        // Note: use the old-style wxFont ctor as wxFontInfo doesn't implement
        // any compatibility hacks.
        const wxFont font(size.specified,
                          wxFontFamily::Default,
                          wxFontStyle::Normal,
                          wxFONTWEIGHT_NORMAL);

        int expected = size.expected;
        if ( expected == -1 )
            expected = sizeDefault;

        // FIXME: Info for new tests?
        // INFO("specified = " << size.specified <<
        //      ", expected = " << size.expected);
        expect( font.GetPointSize() == expected );
    }

    // Note that the compatibility hacks only apply to the old ctors, the newer
    // one, taking wxFontInfo, doesn't support them.
    expect( wxFont(wxFontInfo(70)).GetPointSize() == 70 );
    expect( wxFont(wxFontInfo(90)).GetPointSize() == 90 );

    // Check fractional point sizes as well.
    wxFont font(wxFontInfo(12.25));
    expect( font.GetFractionalPointSize() == 12.25 );
    expect( font.GetPointSize() == 12 );

    font = *wxNORMAL_FONT;
    font.SetFractionalPointSize(9.5);
    expect( font.GetFractionalPointSize() == 9.5 );
    expect( font.GetPointSize() == 10 );
};

ut::suite FontWeightTests = []
{
    using namespace ut;

    wxFont font;
    font.SetNumericWeight(123);

    // WX to QT font weight conversions do not map directly which is why we
    // check if the numeric weight is within a range rather than checking for
    // an exact match.
#ifdef __WXQT__
    expect( ( font.GetNumericWeight() > 113 && font.GetNumericWeight() < 133 ) );
#else
    expect( font.GetNumericWeight() == 123 );
#endif

    expect( font.GetWeight() == wxFONTWEIGHT_THIN );

    font.SetNumericWeight(wxFONTWEIGHT_SEMIBOLD);
    expect( font.GetNumericWeight() == wxFONTWEIGHT_SEMIBOLD );
    expect( font.GetWeight() == wxFONTWEIGHT_SEMIBOLD );
};

ut::suite FontGetSetTests = []
{
    using namespace ut;

    unsigned numFonts;
    auto pf = GetTestFonts(numFonts);
    for (auto& aFont : pf)
    {
        // remember: getters can only be called when wxFont::IsOk() == true
        expect( aFont.IsOk() );


        // test Get/SetFaceName()
#ifndef __WXQT__
        expect( !aFont.SetFaceName("a dummy face name") );
        expect( !aFont.IsOk() );
#endif

        // if the call to SetFaceName() below fails on your system/port,
        // consider adding another branch to this #if
#if defined(__WXMSW__) || defined(__WXOSX__)
        static constexpr char knownGoodFaceName[] = "Arial";
#else
        static constexpr char knownGoodFaceName []= "Monospace";
#endif

        expect(aFont.SetFaceName(knownGoodFaceName)) <<
            fmt::format("failed to set face name \"%s\" for test font\n"
             "(this failure is harmless if this face name is not "
             "available on this system)", knownGoodFaceName);

        expect( aFont.IsOk() );

        // test Get/SetFamily()

        aFont.SetFamily( wxFontFamily::Roman );
        expect( aFont.IsOk() );

        // note that there is always the possibility that GetFamily() returns
        // wxFontFamily::Default (meaning "unknown" in this case) so that we
        // consider it as a valid return value
        const wxFontFamily family = aFont.GetFamily();
        if ( family != wxFontFamily::Default )
            expect( wxFontFamily::Roman == family );


        // test Get/SetEncoding()

        // FIXME: Disabled for now because it prompts user to change fonts.
        //test.SetEncoding( wxFONTENCODING_KOI8 );
        //expect( test.IsOk() );
        //expect( wxFONTENCODING_KOI8 == test.GetEncoding() );

        // test Get/SetPointSize()

        aFont.SetPointSize(30);
        expect( aFont.IsOk() );
        expect( 30 == aFont.GetPointSize() );

        // test Get/SetPixelSize()

        aFont.SetPixelSize(wxSize(0, 30));
        expect( aFont.IsOk() );
        expect( aFont.GetPixelSize().GetHeight() <= 30 );
            // NOTE: the match found by SetPixelSize() may be not 100% precise; it
            //       only grants that a font smaller than the required height will
            //       be selected

        // test Get/SetStyle()

        aFont.SetStyle(wxFontStyle::Slant);
        expect( aFont.IsOk() );
#ifdef __WXMSW__
        // on wxMSW wxFontStyle::Slant==wxFontStyle::Italic, so accept the latter
        // as a valid value too.
        if ( aFont.GetStyle() != wxFontStyle::Italic )
#endif
        expect( wxFontStyle::Slant == aFont.GetStyle() );

        // test Get/SetUnderlined()

        aFont.SetUnderlined(true);
        expect( aFont.IsOk() );
        expect( aFont.GetUnderlined() );

        const wxFont fontBase = aFont.GetBaseFont();
        expect( fontBase.IsOk() );
        expect( !fontBase.GetUnderlined() );
        expect( !fontBase.GetStrikethrough() );
        expect( wxFONTWEIGHT_NORMAL == fontBase.GetWeight() );
        expect( wxFontStyle::Normal == fontBase.GetStyle() );

        // test Get/SetStrikethrough()

        aFont.SetStrikethrough(true);
        expect( aFont.IsOk() );
        expect( aFont.GetStrikethrough() );


        // test Get/SetWeight()

        aFont.SetWeight(wxFONTWEIGHT_BOLD);
        expect( aFont.IsOk() );
        expect( wxFONTWEIGHT_BOLD == aFont.GetWeight() );
    }
};

ut::suite FontNativeFontInfoTests = []
{
    using namespace ut;

    unsigned numFonts;
    auto pf = GetTestFonts(numFonts);

    for ( const auto& aFont : pf )
    {
        std::string nid = aFont.GetNativeFontInfoDesc();
        expect( !nid.empty() );
            // documented to be never empty

        wxFont temp;
        expect( temp.SetNativeFontInfo(nid) );
        expect( temp.IsOk() );
        expect(temp == aFont) << fmt::format("Test failed\ndump of test font: \"%s\"\ndump of temp font: \"%s\"", DumpFont(&aFont), DumpFont(&temp));
    }

    // test that clearly invalid font info strings do not work
    wxFont font;
    expect( !font.SetNativeFontInfo("") );

    // pango_font_description_from_string() used by wxFont in wxGTK and wxX11
    // never returns an error at all so this assertion fails there -- and as it
    // doesn't seem to be possible to do anything about it maybe we should
    // change wxMSW and other ports to also accept any strings?
#if !defined(__WXGTK__) && !defined(__WXX11__) && !defined(__WXQT__)
    expect( !font.SetNativeFontInfo("bloordyblop") );
#endif

    // Pango font description doesn't have 'underlined' and 'strikethrough'
    // attributes, so wxNativeFontInfo implements these itself. Test if these
    // are properly preserved by wxNativeFontInfo or its string description.
    font.SetUnderlined(true);
    font.SetStrikethrough(true);
    expect(font == wxFont(font));
    expect(font == wxFont(*font.GetNativeFontInfo()));
    expect(font == wxFont(font.GetNativeFontInfoDesc()));
    font.SetUnderlined(false);
    expect(font == wxFont(font));
    expect(font == wxFont(*font.GetNativeFontInfo()));
    expect(font == wxFont(font.GetNativeFontInfoDesc()));
    font.SetUnderlined(true);
    font.SetStrikethrough(false);
    expect(font == wxFont(font));
    expect(font == wxFont(*font.GetNativeFontInfo()));
    expect(font == wxFont(font.GetNativeFontInfoDesc()));
    // note: the GetNativeFontInfoUserDesc() doesn't preserve all attributes
    // according to docs, so it is not tested.
};

ut::suite FontNativeFontInfoUserDescTests = []
{
    using namespace ut;

    unsigned numFonts;
    auto pf = GetTestFonts(numFonts);

    for ( const auto& aFont : pf )
    {
        std::string niud = aFont.GetNativeFontInfoUserDesc();
        expect( !niud.empty() );
            // documented to be never empty

        wxFont temp2;
        expect( temp2.SetNativeFontInfoUserDesc(niud) );
        expect( temp2.IsOk() );

#ifdef __WXGTK__
        // Pango saves/restores all font info in the user-friendly string:
        WX_ASSERT_MESSAGE(
            ("Test #%u failed; native info user desc was \"%s\" for test and \"%s\" for temp2", \
             n, niud, temp2.GetNativeFontInfoUserDesc()),
            temp2 == aFont );
#else
        // NOTE: as documented GetNativeFontInfoUserDesc/SetNativeFontInfoUserDesc
        //       are not granted to save/restore all font info.
        //       In fact e.g. the font family is not saved at all; test only those
        //       info which GetNativeFontInfoUserDesc() does indeed save:
        expect( aFont.GetWeight() == temp2.GetWeight() );
        expect( aFont.GetStyle() == temp2.GetStyle() );

        // if the original face name was empty, it means that any face name (in
        // this family) can be used for the new font so we shouldn't be
        // surprised to find that they differ in this case
        const std::string facename = aFont.GetFaceName();
        if ( !facename.empty() )
        {

            expect( wx::utils::ToUpperCopy(facename) == wx::utils::ToUpperCopy(temp2.GetFaceName()) );
        }

        expect( aFont.GetPointSize() == temp2.GetPointSize() );
        expect( aFont.GetEncoding() == temp2.GetEncoding() );
#endif
    }

    // Test for a bug with handling fractional font sizes in description
    // strings (see #18590).
    wxFont font(*wxNORMAL_FONT);

    static constexpr double sizes[] = { 12.0, 10.5, 13.8, 10.123, 11.1 };
    for ( const auto& fontSize : sizes )
    {
        font.SetFractionalPointSize(fontSize);

        // Just setting the font can slightly change it because of rounding
        // errors, so don't expect the actual size to be exactly equal to what
        // we used -- but close enough.
        const double sizeUsed = font.GetFractionalPointSize();
        expect( sizeUsed == doctest::Approx(fontSize).epsilon(0.001) );

        std::string desc = font.GetNativeFontInfoDesc();
        // FIXME: Info in new framework?
        //INFO("Font description: " << desc);
        expect( font.SetNativeFontInfo(desc) );

        // Notice that here we use the exact comparison, there is no reason for
        // a differently rounded size to be used.
        expect( font.GetFractionalPointSize() == sizeUsed );
    }
};
