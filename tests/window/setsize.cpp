///////////////////////////////////////////////////////////////////////////////
// Name:        tests/window/setsize.cpp
// Purpose:     Tests for SetSize() and related wxWindow methods
// Author:      Vadim Zeitlin
// Created:     2008-05-25
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "wx/app.h"
#include "wx/frame.h"
#include "wx/window.h"

#include "waitforpaint.h"

import WX.Test.Prec;

// ----------------------------------------------------------------------------
// tests helpers
// ----------------------------------------------------------------------------

namespace
{

// Helper class overriding DoGetBestSize() for testing purposes.
class MyWindow : public wxWindow
{
public:
    MyWindow(wxWindow* parent)
        : wxWindow(parent, wxID_ANY)
    {
    }

protected:
    wxSize DoGetBestSize() const override { return wxSize(50, 250); }
};

} // anonymous namespace

// ----------------------------------------------------------------------------
// tests themselves
// ----------------------------------------------------------------------------

TEST_CASE("wxWindow::SetSize")
{
    auto w = std::make_unique<MyWindow>(wxTheApp->GetTopWindow());

    SUBCASE("Simple")
    {
        const wxSize size(127, 35);
        w->SetSize(size);
        CHECK( size == w->GetSize() );
    }

    SUBCASE("With min size")
    {
        w->SetMinSize(wxSize(100, 100));

        const wxSize size(200, 50);
        w->SetSize(size);
        CHECK( size == w->GetSize() );
    }
}

TEST_CASE("wxWindow::GetBestSize")
{
    auto w = std::make_unique<MyWindow>(wxTheApp->GetTopWindow());

    CHECK( wxSize(50, 250) == w->GetBestSize() );

    w->SetMinSize(wxSize(100, 100));
    CHECK( wxSize(100, 250) == w->GetBestSize() );

    w->SetMaxSize(wxSize(200, 200));
    CHECK( wxSize(100, 200) == w->GetBestSize() );
}

TEST_CASE("wxWindow::MovePreservesSize")
{
    auto w = std::make_unique<wxFrame>(wxTheApp->GetTopWindow(), wxID_ANY,
                                       "Test child frame");

    // Unfortunately showing the window is asynchronous, at least when using
    // X11, so we have to wait for some time before retrieving its true
    // geometry. And it's not clear how long should we wait, so we do it until
    // we get the first paint event -- by then the window really should have
    // its final size.
    WaitForPaint waitForPaint(w.get());

    w->Show();

    waitForPaint.YieldUntilPainted();

    const wxRect rectOrig = w->GetRect();

    // Check that moving the window doesn't change its size.
    w->Move(rectOrig.GetPosition() + wxPoint(100, 100));
    CHECK( w->GetSize() == rectOrig.GetSize() );
}
