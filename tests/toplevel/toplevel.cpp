///////////////////////////////////////////////////////////////////////////////
// Name:        tests/toplevel/toplevel.cpp
// Purpose:     Tests for wxTopLevelWindow
// Author:      Kevin Ollivier
// Created:     2008-05-25
// Copyright:   (c) 2009 Kevin Ollivier <kevino@theolliviers.com>
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "doctest.h"

#include "testprec.h"


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/dialog.h"
    #include "wx/frame.h"
    #include "wx/textctrl.h"
    #include "wx/toplevel.h"
#endif // WX_PRECOMP

#include "testableframe.h"

static void TopLevelWindowShowTest(wxTopLevelWindow* tlw)
{
    CHECK_FALSE(tlw->IsShown());

    wxTextCtrl* textCtrl = new wxTextCtrl(tlw, -1, "test");
    textCtrl->SetFocus();

// only run this test on platforms where ShowWithoutActivating is implemented.
#if defined(__WXMSW__) || defined(__WXMAC__)
    wxTheApp->GetTopWindow()->SetFocus();
    tlw->ShowWithoutActivating();
    CHECK(tlw->IsShown());
    CHECK_FALSE(tlw->IsActive());

    tlw->Hide();
    CHECK_FALSE(tlw->IsShown());
    CHECK_FALSE(tlw->IsActive());
#endif

    // Note that at least under MSW, ShowWithoutActivating() still generates
    // wxActivateEvent, so we must only start counting these events after the
    // end of the tests above.
    EventCounter countActivate(tlw, wxEVT_ACTIVATE);

    tlw->Show(true);
    countActivate.WaitEvent();

    // TLWs never become active when running under Xvfb, presumably because
    // there is no WM there.
    if ( !IsRunningUnderXVFB() )
        CHECK(tlw->IsActive());

    CHECK(tlw->IsShown());

    tlw->Hide();
    CHECK_FALSE(tlw->IsShown());

    countActivate.WaitEvent();
    CHECK_FALSE(tlw->IsActive());
}

TEST_CASE("wxTopLevel::Show")
{
    SUBCASE("Dialog")
    {
        auto dialog = std::make_unique<wxDialog>(nullptr, -1, "Dialog Test");

        TopLevelWindowShowTest(dialog.get());
    }

    SUBCASE("Frame")
    {
        auto frame = std::make_unique<wxFrame>(nullptr, -1, "Frame test");

        TopLevelWindowShowTest(frame.get());
    }
}

// Check that we receive the expected event when showing the TLW.
TEST_CASE("wxTopLevel::ShowEvent")
{
    auto frame = std::make_unique<wxFrame>(nullptr, wxID_ANY, "Maximized frame");

    EventCounter countShow(frame.get(), wxEVT_SHOW);

    frame->Maximize();
    frame->Show();

    CHECK( countShow.WaitEvent() );
}
