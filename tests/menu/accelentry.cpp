///////////////////////////////////////////////////////////////////////////////
// Name:        tests/menu/accelentry.cpp
// Purpose:     wxAcceleratorEntry unit test
// Author:      Vadim Zeitlin
// Created:     2010-12-03
// Copyright:   (c) 2010 Vadim Zeitlin
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "doctest.h"

#include "testprec.h"


#ifndef WX_PRECOMP
#endif // WX_PRECOMP

#include "wx/accel.h"


namespace
{

void CheckAccelEntry(const wxAcceleratorEntry& accel, int keycode, int flags)
{
    CHECK( keycode == accel.GetKeyCode() );
    CHECK( flags == accel.GetFlags() );
}

} // anonymous namespace


/*
 * Test the creation of accelerator keys using the Create function
 */
TEST_CASE( "wxAcceleratorEntry::Create")
{
    std::unique_ptr<wxAcceleratorEntry> pa;

    SUBCASE( "Correct behavior" )
    {
        pa.reset( wxAcceleratorEntry::Create("Foo\tCtrl+Z") );

        CHECK( pa );
        CHECK( pa->IsOk() );
        CheckAccelEntry(*pa, 'Z', wxACCEL_CTRL);
    }

    SUBCASE( "Tab missing" )
    {
        pa.reset( wxAcceleratorEntry::Create("Shift-Q") );

        CHECK( !pa );
    }

    SUBCASE( "No accelerator key specified" )
    {
        pa.reset( wxAcceleratorEntry::Create("bloordyblop") );

        CHECK( !pa );
    }

    SUBCASE( "Display name parsing" )
    {
        pa.reset( wxAcceleratorEntry::Create("Test\tBackSpace") );

        CHECK( pa );
        CHECK( pa->IsOk() );
        CheckAccelEntry(*pa, WXK_BACK, wxACCEL_NORMAL);
    }
}


/*
 * Test the creation of accelerator keys from strings and also the
 * creation of strings from an accelerator key
 */
TEST_CASE( "wxAcceleratorEntry::StringTests")
{
    wxAcceleratorEntry a(wxACCEL_ALT, 'X');

    SUBCASE( "Create string from key" )
    {
        CHECK( "Alt+X" == a.ToString() );
    }

    SUBCASE( "Create from valid string" )
    {
        CHECK( a.FromString("Alt+Shift+F1") );
        CheckAccelEntry(a, WXK_F1, wxACCEL_ALT | wxACCEL_SHIFT);
    }

    SUBCASE( "Create from invalid string" )
    {
        CHECK( !a.FromString("bloordyblop") );
    }
}
