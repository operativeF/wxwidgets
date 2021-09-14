///////////////////////////////////////////////////////////////////////////////
// Name:        tests/font/fonttest.cpp
// Purpose:     wxFont unit test
// Author:      Francesco Montorsi
// Created:     16.3.09
// Copyright:   (c) 2009 Francesco Montorsi
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif // WX_PRECOMP

#include "wx/font.h"

#include "asserthelper.h"

// ----------------------------------------------------------------------------
// local helpers
// ----------------------------------------------------------------------------

// Returns a point to an array of fonts and fills the output parameter with the
// number of elements in this array.
static const wxFont *GetTestFonts(unsigned& numFonts)
{
    static const wxFont testfonts[] =
    {
        *wxNORMAL_FONT,
        *wxSMALL_FONT,
        *wxITALIC_FONT,
        *wxSWISS_FONT,
        wxFont(5, wxFontFamily::Teletype, wxFontStyle::Normal, wxFONTWEIGHT_NORMAL)
    };

    numFonts = WXSIZEOF(testfonts);

    return testfonts;
}

wxString DumpFont(const wxFont *font)
{
    // dumps the internal properties of a wxFont in the same order they
    // are checked by wxFontBase::operator==()

    wxASSERT(font->IsOk());

    wxString s;
    s.Printf(wxS("%d-%d;%d-%d-%d-%d-%d-%s-%d"),
             font->GetPointSize(),
             font->GetPixelSize().x,
             font->GetPixelSize().y,
             font->GetFamily(),
             font->GetStyle(),
             font->GetWeight(),
             font->GetUnderlined() ? 1 : 0,
             font->GetFaceName(),
             font->GetEncoding());

    return s;
}

// ----------------------------------------------------------------------------
// the tests
// ----------------------------------------------------------------------------

TEST_CASE("wxFont::Construct")
{
    // The main purpose of this test is to verify that the font ctors below
    // compile because it's easy to introduce ambiguities due to the number of
    // overloaded wxFont ctors.

    CHECK( wxFont(10, wxFontFamily::Default,
                      wxFontStyle::Normal,
                      wxFONTWEIGHT_NORMAL).IsOk() );
}

TEST_CASE("wxFont::Size")
{
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

        INFO("specified = " << size.specified <<
             ", expected = " << size.expected);
        CHECK( font.GetPointSize() == expected );
    }

    // Note that the compatibility hacks only apply to the old ctors, the newer
    // one, taking wxFontInfo, doesn't support them.
    CHECK( wxFont(wxFontInfo(70)).GetPointSize() == 70 );
    CHECK( wxFont(wxFontInfo(90)).GetPointSize() == 90 );

    // Check fractional point sizes as well.
    wxFont font(wxFontInfo(12.25));
    CHECK( font.GetFractionalPointSize() == 12.25 );
    CHECK( font.GetPointSize() == 12 );

    font = *wxNORMAL_FONT;
    font.SetFractionalPointSize(9.5);
    CHECK( font.GetFractionalPointSize() == 9.5 );
    CHECK( font.GetPointSize() == 10 );
}

TEST_CASE("wxFont::Weight")
{
    wxFont font;
    font.SetNumericWeight(123);

    // WX to QT font weight conversions do not map directly which is why we
    // check if the numeric weight is within a range rather than checking for
    // an exact match.
#ifdef __WXQT__
    CHECK( ( font.GetNumericWeight() > 113 && font.GetNumericWeight() < 133 ) );
#else
    CHECK( font.GetNumericWeight() == 123 );
#endif

    CHECK( font.GetWeight() == wxFONTWEIGHT_THIN );

    font.SetNumericWeight(wxFONTWEIGHT_SEMIBOLD);
    CHECK( font.GetNumericWeight() == wxFONTWEIGHT_SEMIBOLD );
    CHECK( font.GetWeight() == wxFONTWEIGHT_SEMIBOLD );
}

TEST_CASE("wxFont::GetSet")
{
    unsigned numFonts;
    const wxFont *pf = GetTestFonts(numFonts);
    for ( unsigned n = 0; n < numFonts; n++ )
    {
        wxFont test(*pf++);

        // remember: getters can only be called when wxFont::IsOk() == true
        CHECK( test.IsOk() );


        // test Get/SetFaceName()
#ifndef __WXQT__
        CHECK( !test.SetFaceName("a dummy face name") );
        CHECK( !test.IsOk() );
#endif

        // if the call to SetFaceName() below fails on your system/port,
        // consider adding another branch to this #if
#if defined(__WXMSW__) || defined(__WXOSX__)
        static const char *knownGoodFaceName = "Arial";
#else
        static const char *knownGoodFaceName = "Monospace";
#endif

        CHECK_MESSAGE
        (
            test.SetFaceName(knownGoodFaceName),
            ("failed to set face name \"%s\" for test font #%u\n"
             "(this failure is harmless if this face name is not "
             "available on this system)", knownGoodFaceName, n)
        );
        CHECK( test.IsOk() );


        // test Get/SetFamily()

        test.SetFamily( wxFontFamily::Roman );
        CHECK( test.IsOk() );

        // note that there is always the possibility that GetFamily() returns
        // wxFontFamily::Default (meaning "unknown" in this case) so that we
        // consider it as a valid return value
        const wxFontFamily family = test.GetFamily();
        if ( family != wxFontFamily::Default )
            CHECK( wxFontFamily::Roman == family );


        // test Get/SetEncoding()

        // FIXME: Disabled for now because it prompts user to change fonts.
        //test.SetEncoding( wxFONTENCODING_KOI8 );
        //CHECK( test.IsOk() );
        //CHECK( wxFONTENCODING_KOI8 == test.GetEncoding() );


        // test Get/SetPointSize()

        test.SetPointSize(30);
        CHECK( test.IsOk() );
        CHECK( 30 == test.GetPointSize() );


        // test Get/SetPixelSize()

        test.SetPixelSize(wxSize(0,30));
        CHECK( test.IsOk() );
        CHECK( test.GetPixelSize().GetHeight() <= 30 );
            // NOTE: the match found by SetPixelSize() may be not 100% precise; it
            //       only grants that a font smaller than the required height will
            //       be selected


        // test Get/SetStyle()

        test.SetStyle(wxFontStyle::Slant);
        CHECK( test.IsOk() );
#ifdef __WXMSW__
        // on wxMSW wxFontStyle::Slant==wxFontStyle::Italic, so accept the latter
        // as a valid value too.
        if ( test.GetStyle() != wxFontStyle::Italic )
#endif
        CHECK( wxFontStyle::Slant == test.GetStyle() );

        // test Get/SetUnderlined()

        test.SetUnderlined(true);
        CHECK( test.IsOk() );
        CHECK( test.GetUnderlined() );

        const wxFont fontBase = test.GetBaseFont();
        CHECK( fontBase.IsOk() );
        CHECK( !fontBase.GetUnderlined() );
        CHECK( !fontBase.GetStrikethrough() );
        CHECK( wxFONTWEIGHT_NORMAL == fontBase.GetWeight() );
        CHECK( wxFontStyle::Normal == fontBase.GetStyle() );

        // test Get/SetStrikethrough()

        test.SetStrikethrough(true);
        CHECK( test.IsOk() );
        CHECK( test.GetStrikethrough() );


        // test Get/SetWeight()

        test.SetWeight(wxFONTWEIGHT_BOLD);
        CHECK( test.IsOk() );
        CHECK( wxFONTWEIGHT_BOLD == test.GetWeight() );
    }
}

TEST_CASE("wxFont::NativeFontInfo")
{
    unsigned numFonts;
    const wxFont *pf = GetTestFonts(numFonts);
    for ( unsigned n = 0; n < numFonts; n++ )
    {
        wxFont test(*pf++);

        const wxString& nid = test.GetNativeFontInfoDesc();
        CHECK( !nid.empty() );
            // documented to be never empty

        wxFont temp;
        CHECK( temp.SetNativeFontInfo(nid) );
        CHECK( temp.IsOk() );
        CHECK_MESSAGE(
            temp == test,
            ("Test #%u failed\ndump of test font: \"%s\"\ndump of temp font: \"%s\"", \
             n, DumpFont(&test), DumpFont(&temp)));
    }

    // test that clearly invalid font info strings do not work
    wxFont font;
    CHECK( !font.SetNativeFontInfo("") );

    // pango_font_description_from_string() used by wxFont in wxGTK and wxX11
    // never returns an error at all so this assertion fails there -- and as it
    // doesn't seem to be possible to do anything about it maybe we should
    // change wxMSW and other ports to also accept any strings?
#if !defined(__WXGTK__) && !defined(__WXX11__) && !defined(__WXQT__)
    CHECK( !font.SetNativeFontInfo("bloordyblop") );
#endif

    // Pango font description doesn't have 'underlined' and 'strikethrough'
    // attributes, so wxNativeFontInfo implements these itself. Test if these
    // are properly preserved by wxNativeFontInfo or its string description.
    font.SetUnderlined(true);
    font.SetStrikethrough(true);
    CHECK(font == wxFont(font));
    CHECK(font == wxFont(*font.GetNativeFontInfo()));
    CHECK(font == wxFont(font.GetNativeFontInfoDesc()));
    font.SetUnderlined(false);
    CHECK(font == wxFont(font));
    CHECK(font == wxFont(*font.GetNativeFontInfo()));
    CHECK(font == wxFont(font.GetNativeFontInfoDesc()));
    font.SetUnderlined(true);
    font.SetStrikethrough(false);
    CHECK(font == wxFont(font));
    CHECK(font == wxFont(*font.GetNativeFontInfo()));
    CHECK(font == wxFont(font.GetNativeFontInfoDesc()));
    // note: the GetNativeFontInfoUserDesc() doesn't preserve all attributes
    // according to docs, so it is not tested.
}

TEST_CASE("wxFont::NativeFontInfoUserDesc")
{
    unsigned numFonts;
    const wxFont *pf = GetTestFonts(numFonts);
    for ( unsigned n = 0; n < numFonts; n++ )
    {
        wxFont test(*pf++);

        const wxString& niud = test.GetNativeFontInfoUserDesc();
        CHECK( !niud.empty() );
            // documented to be never empty

        wxFont temp2;
        CHECK( temp2.SetNativeFontInfoUserDesc(niud) );
        CHECK( temp2.IsOk() );

#ifdef __WXGTK__
        // Pango saves/restores all font info in the user-friendly string:
        WX_ASSERT_MESSAGE(
            ("Test #%u failed; native info user desc was \"%s\" for test and \"%s\" for temp2", \
             n, niud, temp2.GetNativeFontInfoUserDesc()),
            temp2 == test );
#else
        // NOTE: as documented GetNativeFontInfoUserDesc/SetNativeFontInfoUserDesc
        //       are not granted to save/restore all font info.
        //       In fact e.g. the font family is not saved at all; test only those
        //       info which GetNativeFontInfoUserDesc() does indeed save:
        CHECK( test.GetWeight() == temp2.GetWeight() );
        CHECK( test.GetStyle() == temp2.GetStyle() );

        // if the original face name was empty, it means that any face name (in
        // this family) can be used for the new font so we shouldn't be
        // surprised to find that they differ in this case
        const wxString facename = test.GetFaceName();
        if ( !facename.empty() )
        {
            CHECK( facename.Upper() == temp2.GetFaceName().Upper() );
        }

        CHECK( test.GetPointSize() == temp2.GetPointSize() );
        CHECK( test.GetEncoding() == temp2.GetEncoding() );
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
        CHECK( sizeUsed == doctest::Approx(fontSize).epsilon(0.001) );

        const wxString& desc = font.GetNativeFontInfoDesc();
        INFO("Font description: " << desc);
        CHECK( font.SetNativeFontInfo(desc) );

        // Notice that here we use the exact comparison, there is no reason for
        // a differently rounded size to be used.
        CHECK( font.GetFractionalPointSize() == sizeUsed );
    }
}
