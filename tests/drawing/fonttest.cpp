///////////////////////////////////////////////////////////////////////////////
// Name:        tests/drawing/basictest.cpp
// Purpose:     Basic tests for wxGraphicsContext
// Author:      Armel Asselin
// Created:     2014-02-28
// Copyright:   (c) 2014 Ellié Computing <opensource@elliecomputing.com>
///////////////////////////////////////////////////////////////////////////////

#include "testprec.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/font.h"
#endif // WX_PRECOMP

#include "drawing.h"

#ifdef __WXGTK__
#include "glib-object.h"
#endif

#if wxUSE_TEST_GC_DRAWING

const GraphicsContextDrawingTestCase::DrawingTestCase
GraphicsContextDrawingTestCase::ms_drawingFontTc = {
    2, &GraphicsContextDrawingTestCase::DoFontDrawings, 800, 600, 72., false
};

void GraphicsContextDrawingTestCase::DoFontDrawings (wxGraphicsContext *gc)
{
#ifdef __WXGTK__
    g_type_init();
#endif

    // This test is expected to treat about fonts/texts. Fonts are a bit special
    // because we cannot expect the same rendering by several engines, and the
    // dimensions of the same text in same font will vary.

    wxGraphicsBrush gbBackground =
        gc->CreateBrush( wxBrush ( wxColour ( 240, 240, 240 ) ) );

    gc->SetBrush( gbBackground );
    gc->DrawRectangle(0, 0, 800, 600);

    wxGraphicsBrush gbTextBackground =
        gc->CreateBrush( wxBrush ( wxColour ( 192, 192, 192 ) ) );

    // set underlined font for testing
    gc->SetFont( wxFont(wxFontInfo(12).Family(wxFontFamily::Modern).Underlined()), *wxBLACK );
    gc->wxDrawText( wxT("This is text"), 110, 10, gbTextBackground );
    gc->wxDrawText( wxT("That is text"), 20, 10, wxDegToRad(-45), gbTextBackground );

    // use wxSWISS_FONT and not wxNORMAL_FONT as the latter can't be rotated
    // (it is not TrueType)
    gc->SetFont( wxFont(wxFontInfo(12).Family(wxFontFamily::Swiss)), *wxBLACK );

    wxString text;

    for ( int n = -180; n < 180; n += 30 )
    {
        text.Printf(wxT("     %d rotated text"), n);
        gc->wxDrawText(text , 400, 400, wxDegToRad(n) );
    }

    wxFont swissDcFont( wxFontInfo(18).Family(wxFontFamily::Swiss) );
    wxGraphicsFont swissFont = gc->wxCreateFont( swissDcFont, *wxBLACK );
    gc->SetFont( swissFont );

    gc->wxDrawText( wxT("This is Swiss 18pt text."), 110, 40 );

    double descent;
    auto [length, height] = gc->GetTextExtent( wxT("This is Swiss 18pt text."), &descent );
    text.Printf( wxT("Dimensions are length %f, height %f, descent %f"), length, height, descent );
    gc->wxDrawText( text, 110, 80 );

    // (did not find equivalent to CharHeight())

    gc->SetBrush( *wxWHITE_BRUSH );

    gc->DrawRectangle( 100, 40, 4, height );

    // test the logical function effect
    wxCoord y = 150;
    // text drawing should ignore logical function
    gc->wxDrawText( wxT("There should be a text below"), 110, 150 );
    gc->DrawRectangle( 110, y, 100, height );

    y += height;
    gc->wxDrawText( wxT("Visible text"), 110, y );
    gc->DrawRectangle( 110, y, 100, height );
    gc->wxDrawText( wxT("Visible text"), 110, y );
    gc->DrawRectangle( 110, y, 100, height );

    y += height;
    gc->DrawRectangle( 110, y, 100, height );
    gc->wxDrawText( wxT("Another visible text"), 110, y );

    y += height;
    gc->wxDrawText("And\nmore\ntext on\nmultiple\nlines", 110, y);
    y += 5*height;

    gc->SetFont( swissDcFont, *wxBLUE );
    gc->wxDrawText( "Rotated text\ncan have\nmultiple lines\nas well", 110, y, wxDegToRad(15) );
}

#endif // wxUSE_TEST_GC_DRAWING
