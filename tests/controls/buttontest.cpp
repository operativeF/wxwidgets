///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/buttontest.cpp
// Purpose:     wxButton unit test
// Author:      Steven Lamerton
// Created:     2010-06-21
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_BUTTON

#include "wx/app.h"
#include "wx/button.h"

#include "testableframe.h"
#include "wx/uiaction.h"
#include "wx/artprov.h"
#include "wx/utils.h"

import WX.Test.Prec;

TEST_CASE("Button test")
{
    auto m_button = std::make_unique<wxButton>(wxTheApp->GetTopWindow(), wxID_ANY, "wxButton");

#if wxUSE_UIACTIONSIMULATOR

    SUBCASE("Click")
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

        CHECK_EQ( 1, clicked.GetCount() );
    }

    SUBCASE("Disabled")
    {
        wxUIActionSimulator sim;

        // In this test we disable the button and check events are not sent and we
        // do it once by disabling the previously enabled button and once by
        // creating the button in the disabled state.
        SUBCASE("Disable after creation")
        {
            m_button->Disable();
        }

        SUBCASE("Create disabled")
        {
            m_button = std::make_unique<wxButton>();
            m_button->Disable();
            m_button->Create(wxTheApp->GetTopWindow(), wxID_ANY, "wxButton");
        }

        EventCounter clicked(m_button.get(), wxEVT_BUTTON);

        sim.MouseMove(m_button->GetScreenPosition() + wxPoint(10, 10));
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ( 0, clicked.GetCount() );
    }

#endif // wxUSE_UIACTIONSIMULATOR

    SUBCASE("Auth")
    {
        //Some functions only work on specific operating system versions, for
        //this we need a runtime check
        int major = 0;

        if(wxGetOsVersion(&major) != wxOS_WINDOWS_NT || major < 6)
            return;

        //We are running Windows Vista or newer
        CHECK(!m_button->GetAuthNeeded());

        m_button->SetAuthNeeded();

        CHECK(m_button->GetAuthNeeded());

        //We test both states
        m_button->SetAuthNeeded(false);

        CHECK(!m_button->GetAuthNeeded());
    }

    SUBCASE("BitmapMargins")
    {
        //Some functions only work on specific platforms in which case we can use
        //a preprocessor check
    #ifdef __WXMSW__
        //We must set a bitmap before we can set its margins, when writing unit
        //tests it is easiest to use an image from wxArtProvider
        m_button->SetBitmap(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER,
                                                   wxSize(32, 32)));

        m_button->SetBitmapMargins(15, 15);

        CHECK_EQ(wxSize(15, 15), m_button->GetBitmapMargins());

        m_button->SetBitmapMargins(wxSize(20, 20));

        CHECK_EQ(wxSize(20, 20), m_button->GetBitmapMargins());
    #endif
    }

    SUBCASE("Bitmap")
    {
        //We start with no bitmaps
        CHECK(!m_button->GetBitmap().IsOk());

        // Some bitmap, doesn't really matter which.
        const wxBitmap bmp = wxArtProvider::GetBitmap(wxART_INFORMATION);

        m_button->SetBitmap(bmp);

        CHECK(m_button->GetBitmap().IsOk());

        // Check that resetting the button label doesn't result in problems when
        // updating the bitmap later, as it used to be the case in wxGTK (#18898).
        m_button->SetLabel("");
        CHECK_NOTHROW( m_button->Disable() );
    }
}

#endif //wxUSE_BUTTON
