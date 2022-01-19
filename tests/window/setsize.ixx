///////////////////////////////////////////////////////////////////////////////
// Name:        tests/window/setsize.cpp
// Purpose:     Tests for SetSize() and related wxWindow methods
// Author:      Vadim Zeitlin
// Created:     2008-05-25
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/frame.h"
#include "wx/window.h"

#include "waitforpaint.h"

export module WX.Test.SetSize;

import WX.Test.Prec;
import WX.MetaTest;

namespace ut = boost::ut;

ut::suite SetSizeTests = []
{
    using namespace ut;
    
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

    "SetSize"_test = [&]
    {
        auto w = std::make_unique<MyWindow>(wxTheApp->GetTopWindow());

        "Simple"_test = [&]
        {
            const wxSize size(127, 35);
            w->SetSize(size);
            expect( size == w->GetSize() );
        };

        "With min size"_test = [&]
        {
            w->SetMinSize(wxSize(100, 100));

            const wxSize size(200, 50);
            w->SetSize(size);
            expect( size == w->GetSize() );
        };
    };

    "GetBestSize"_test = [&]
    {
        auto w = std::make_unique<MyWindow>(wxTheApp->GetTopWindow());

        expect( wxSize(50, 250) == w->GetBestSize() );

        w->SetMinSize(wxSize(100, 100));
        expect( wxSize(100, 250) == w->GetBestSize() );

        w->SetMaxSize(wxSize(200, 200));
        expect( wxSize(100, 200) == w->GetBestSize() );
    };

    "MovePreservesSize"_test = [&]
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
        expect( w->GetSize() == rectOrig.GetSize() );
    };
};
