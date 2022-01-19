///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/buttontest.cpp
// Purpose:     wxButton unit test
// Author:      Steven Lamerton
// Created:     2010-06-21
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/button.h"

#include "testableframe.h"
#include "wx/uiaction.h"
#include "wx/artprov.h"
#include "wx/utils.h"

export module WX.Test.Button;

import WX.Test.Prec;
import WX.MetaTest;

#if wxUSE_BUTTON

namespace ut = boost::ut;

ut::suite ButtonTests = []
{
    using namespace ut;

    auto m_button = std::make_unique<wxButton>(wxTheApp->GetTopWindow(), wxID_ANY, "wxButton");

    "Auth"_test = [&]
    {
        //Some functions only work on specific operating system versions, for
        //this we need a runtime check
        int major = 0;

        if(wxGetOsVersion(&major) != wxOS_WINDOWS_NT || major < 6)
            return;

        //We are running Windows Vista or newer
        expect(!m_button->GetAuthNeeded());

        m_button->SetAuthNeeded();

        expect(m_button->GetAuthNeeded());

        //We test both states
        m_button->SetAuthNeeded(false);

        expect(!m_button->GetAuthNeeded());
    };

    //Some functions only work on specific platforms in which case we can use
    //a preprocessor check
#ifdef __WXMSW__
    "BitmapMargins"_test = [&]
    {
        //We must set a bitmap before we can set its margins, when writing unit
        //tests it is easiest to use an image from wxArtProvider
        m_button->SetBitmap(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER,
                                                   wxSize(32, 32)));

        m_button->SetBitmapMargins(15, 15);

        expect(wxSize(15, 15) == m_button->GetBitmapMargins());

        m_button->SetBitmapMargins(wxSize(20, 20));

        expect(wxSize(20, 20) == m_button->GetBitmapMargins());
    };
#endif

    "Bitmap"_test = [&]
    {
        auto bmp_button = std::make_unique<wxButton>(wxTheApp->GetTopWindow(), wxID_ANY, "wxButton");
        //We start with no bitmaps
        expect(!bmp_button->GetBitmap().IsOk());

        // Some bitmap, doesn't really matter which.
        const wxBitmap bmp = wxArtProvider::GetBitmap(wxART_INFORMATION);

        bmp_button->SetBitmap(bmp);

        expect(bmp_button->GetBitmap().IsOk());

        // Check that resetting the button label doesn't result in problems when
        // updating the bitmap later, as it used to be the case in wxGTK (#18898).
        m_button->SetLabel("");
        expect(nothrow([&]{ bmp_button->Disable(); }));
    };
    
    // FIXME: Fails; currently unknown as to why, but some actions are indeed
    // registered.
    "Click"_test = [&]
    {
        //We use the internal class EventCounter which handles connecting and
        //disconnecting the control to the wxTestableFrame
        EventCounter clicked(m_button.get(), wxEVT_BUTTON);

        wxUIActionSimulator sim;

        //We move in slightly to account for window decorations, we need to yield
        //after every wxUIActionSimulator action to keep everything working in GTK
        sim.MouseMove(m_button->GetScreenPosition() + wxPoint(10, 10));
        wxYield();

        sim.MouseClick();
        wxYield();

        expect( 1 == clicked.GetCount() );
    };

    // // FIXME: Test will succeed whether or not actual functions work properly!
    // // Needs to add an enabled state to verify actual functionality.
    "Disabled"_test = [&]
    {
        wxUIActionSimulator sim;

        // In this test we disable the button and check events are not sent and we
        // do it once by disabling the previously enabled button and once by
        // creating the button in the disabled state.
        m_button->Disable();

        m_button = std::make_unique<wxButton>();
        m_button->Disable();
        m_button->Create(wxTheApp->GetTopWindow(), wxID_ANY, "wxButton");

        EventCounter clicked(m_button.get(), wxEVT_BUTTON);

        sim.MouseMove(m_button->GetScreenPosition() + wxPoint(10, 10));
        wxYield();

        sim.MouseClick();
        wxYield();

        expect( 0 == clicked.GetCount() );
    };
};

#endif //wxUSE_BUTTON
