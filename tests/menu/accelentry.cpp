///////////////////////////////////////////////////////////////////////////////
// Name:        tests/menu/accelentry.cpp
// Purpose:     wxAcceleratorEntry unit test
// Author:      Vadim Zeitlin
// Created:     2010-12-03
// Copyright:   (c) 2010 Vadim Zeitlin
///////////////////////////////////////////////////////////////////////////////

#include "wx/accel.h"
#include "wx/defs.h"

import WX.MetaTest;

namespace ut = boost::ut;

/*
 * Test the creation of accelerator keys using the Create function
 */
ut::suite wxAcceleratorEntryCreation = []
{
    using namespace ut;

    "Correct behavior"_test = []
    {
        auto pa = wxAcceleratorEntry::Create("Foo\tCtrl+Z");

        expect( pa.has_value() );
        expect( pa->IsOk() );

        expect( 'Z' == pa->GetKeyCode() );
        expect( wxACCEL_CTRL == pa->GetFlags() );
    };

    "Tab missing"_test = []
    {
        auto pa = wxAcceleratorEntry::Create("Shift-Q");

        expect( !pa.has_value() );
    };

    "No accelerator key specified"_test = []
    {
        auto pa = wxAcceleratorEntry::Create("bloordyblop");

        expect( !pa.has_value() );
    };

    "Display name parsing"_test = []
    {
        auto pa = wxAcceleratorEntry::Create("Test\tBackSpace");

        expect( pa.has_value() );
        expect( pa->IsOk() );

        expect( WXK_BACK == pa->GetKeyCode() );
        expect( wxACCEL_NORMAL == pa->GetFlags() );
    };
};


/*
 * Test the creation of accelerator keys from strings and also the
 * creation of strings from an accelerator key
 */
ut::suite wxAcceleratorEntryStringTests = []
{
    using namespace ut;

    "Create string from key"_test = []
    {
        wxAcceleratorEntry a{ wxACCEL_ALT, 'X', 0, nullptr };

        expect("Alt+X" == a.ToString());
    };

    "Create from valid string"_test = []
    {
        wxAcceleratorEntry a{ wxACCEL_ALT, 'X', 0, nullptr };

        expect( a.FromString("Alt+Shift+F1") );
        
        expect( WXK_F1 == a.GetKeyCode() );
        expect( (wxACCEL_ALT | wxACCEL_SHIFT) == a.GetFlags() );
    };

    "Create from invalid string"_test = []
    {
        wxAcceleratorEntry a{ wxACCEL_ALT, 'X', 0, nullptr };

        expect( !a.FromString("bloordyblop") );
    };
};
