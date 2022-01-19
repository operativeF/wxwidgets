///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/clientsize.cpp
// Purpose:     Client vs. window size handling unit test
// Author:      Vaclav Slavik
// Created:     2008-02-12
// Copyright:   (c) 2008 Vaclav Slavik <vslavik@fastmail.fm>
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/window.h"

#include <fmt/core.h>

export module WX.Test.ClientSize;

import WX.Test.Prec;
import WX.MetaTest;

// ----------------------------------------------------------------------------
// tests themselves
// ----------------------------------------------------------------------------

namespace ut = boost::ut;

ut::suite ClientSizeTests = []
{
    using namespace ut;

    "SizeRoundTrip"_test = [&]
    {
        wxWindow* const w = wxTheApp->GetTopWindow();
        expect( w );

        const wxSize sizeWindow = w->GetSize();
        const wxSize sizeClient = w->GetClientSize();

        expect( sizeWindow == w->ClientToWindowSize(sizeClient) ) << fmt::format("client size: ({}, {})", sizeClient.x, sizeClient.y);

        expect( sizeClient == w->WindowToClientSize(sizeWindow) ) << fmt::format("window size: ({}, {})", sizeWindow.x, sizeWindow.y);
    };

    "MinClientSize"_test = [&]
    {
        auto w = std::make_unique<wxWindow>(wxTheApp->GetTopWindow(),
                                            wxID_ANY,
                                            wxDefaultPosition,
                                            wxDefaultSize,
                                            wxBORDER_THEME);
        w->SetSize(wxSize(1,1));
        const wxSize szw = w->GetClientSize();

        expect(szw.GetWidth() >= 0);
        expect(szw.GetHeight() >= 0);
    };
};
