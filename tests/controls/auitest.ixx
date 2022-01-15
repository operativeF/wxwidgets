///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/auitest.cpp
// Purpose:     wxAui control tests
// Author:      Sebastian Walderich
// Created:     2018-12-19
// Copyright:   (c) 2018 Sebastian Walderich
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "wx/app.h"
#include "wx/panel.h"

export module WX.Test.AUI;

import WX.Test.Prec;
import WX.MetaTest;

import WX.AUI;

#if wxUSE_AUI

namespace ut = boost::ut;

ut::suite wxAuiNotebookDoGetBestSizeTest = []
{
    using namespace ut;

    wxWindow *frame = wxTheApp->GetTopWindow();
    expect( frame );
    auto nb = std::make_unique<wxAuiNotebook>(frame);

    wxPanel *p = new wxPanel(nb.get());
    p->SetMinSize(wxSize(100, 100));
    expect( nb->AddPage(p, "Center Pane") );

    const int tabHeight = nb->GetTabCtrlHeight();

    "Single pane with multiple tabs"_test = [&]
    {
        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(300, 100));
        nb->AddPage(p, "Center Tab 2");

        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(100, 200));
        nb->AddPage(p, "Center Tab 3");

        expect( nb->GetBestSize().x == 300 );
        expect( nb->GetBestSize().y == (200 + tabHeight));
    };

    "Horizontal split"_test = [&]
    {
        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(25, 0));
        nb->AddPage(p, "Left Pane");
        nb->Split(nb->GetPageCount()-1, wxDirection::wxLEFT);

        expect( nb->GetBestSize().x == 325 ) << fmt::format("{}", nb->GetBestSize().x);
        expect( nb->GetBestSize().y == (200 + tabHeight)) << fmt::format("{}", nb->GetBestSize().y);

        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(50, 0));
        nb->AddPage(p, "Right Pane 1");
        nb->Split(nb->GetPageCount()-1, wxDirection::wxRIGHT);

        expect( nb->GetBestSize().x == 375 ) << fmt::format("{}", nb->GetBestSize().x);
        expect( nb->GetBestSize().y == (200 + tabHeight)) << fmt::format("{}", nb->GetBestSize().y);

        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(100, 0));
        nb->AddPage(p, "Right Pane 2");
        nb->Split(nb->GetPageCount()-1, wxDirection::wxRIGHT);

        expect( nb->GetBestSize().x == 475 ) << fmt::format("{}", nb->GetBestSize().x);
        expect( nb->GetBestSize().y == (200 + tabHeight)) << fmt::format("{}", nb->GetBestSize().y);
    };

    "Vertical split"_test = [&]
    {
        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(0, 100));
        nb->AddPage(p, "Top Pane 1");
        nb->Split(nb->GetPageCount()-1, wxDirection::wxTOP);

        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(0, 50));
        nb->AddPage(p, "Top Pane 2");
        nb->Split(nb->GetPageCount()-1, wxDirection::wxTOP);

        expect( nb->GetBestSize().x == 475 ) << fmt::format("{}", nb->GetBestSize().x);
        expect( nb->GetBestSize().y == (350 + 3 * tabHeight)) << fmt::format("{}", nb->GetBestSize().y);

        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(0, 25));
        nb->AddPage(p, "Bottom Pane");
        nb->Split(nb->GetPageCount()-1, wxDirection::wxBOTTOM);

        expect( nb->GetBestSize().y == (375 + 4 * tabHeight)) << fmt::format("{}", nb->GetBestSize().y);
    };

    "Surrounding panes"_test = [&]
    {
        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(50, 25));
        nb->AddPage(p, "Bottom Pane");
        nb->Split(nb->GetPageCount()-1, wxDirection::wxBOTTOM);

        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(50, 120));
        nb->AddPage(p, "Right Pane");
        nb->Split(nb->GetPageCount()-1, wxDirection::wxRIGHT);

        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(225, 50));
        nb->AddPage(p, "Top Pane");
        nb->Split(nb->GetPageCount()-1, wxDirection::wxTOP);

        p = new wxPanel(nb.get());
        p->SetMinSize(wxSize(25, 105));
        nb->AddPage(p, "Left Pane");
        nb->Split(nb->GetPageCount()-1, wxDirection::wxLEFT);

        expect( nb->GetBestSize().x == 550 ) << fmt::format("{}", nb->GetBestSize().x);
        expect( nb->GetBestSize().y == (522 + 3*tabHeight)) << fmt::format("{} {}", nb->GetBestSize().y, tabHeight);
    };
};

#endif
