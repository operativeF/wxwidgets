///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/auitest.cpp
// Purpose:     wxAui control tests
// Author:      Sebastian Walderich
// Created:     2018-12-19
// Copyright:   (c) 2018 Sebastian Walderich
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_AUI

#include "wx/app.h"
#include "wx/panel.h"

#include "asserthelper.h"

import WX.AUI.Book;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

TEST_CASE( "wxAuiNotebook::DoGetBestSize")
{
    wxWindow *frame = wxTheApp->GetTopWindow();
    REQUIRE( frame );
    wxAuiNotebook *nb = new wxAuiNotebook(frame);
    std::unique_ptr<wxAuiNotebook> cleanUp(nb);

    wxPanel *p = new wxPanel(nb);
    p->SetMinSize(wxSize(100, 100));
    REQUIRE( nb->AddPage(p, "Center Pane") );

    const int tabHeight = nb->GetTabCtrlHeight();

    SUBCASE( "Single pane with multiple tabs" )
    {
        p = new wxPanel(nb);
        p->SetMinSize(wxSize(300, 100));
        nb->AddPage(p, "Center Tab 2");

        p = new wxPanel(nb);
        p->SetMinSize(wxSize(100, 200));
        nb->AddPage(p, "Center Tab 3");

        CHECK( nb->GetBestSize() == wxSize(300, 200 + tabHeight) );
    }

    SUBCASE( "Horizontal split" )
    {
        p = new wxPanel(nb);
        p->SetMinSize(wxSize(25, 0));
        nb->AddPage(p, "Left Pane");
        nb->Split(nb->GetPageCount()-1, wxLEFT);

        CHECK( nb->GetBestSize() == wxSize(125, 100 + tabHeight) );

        p = new wxPanel(nb);
        p->SetMinSize(wxSize(50, 0));
        nb->AddPage(p, "Right Pane 1");
        nb->Split(nb->GetPageCount()-1, wxRIGHT);

        CHECK( nb->GetBestSize() == wxSize(175, 100 + tabHeight) );

        p = new wxPanel(nb);
        p->SetMinSize(wxSize(100, 0));
        nb->AddPage(p, "Right Pane 2");
        nb->Split(nb->GetPageCount()-1, wxRIGHT);

        CHECK( nb->GetBestSize() == wxSize(275, 100 + tabHeight) );
    }

    SUBCASE( "Vertical split" )
    {
        p = new wxPanel(nb);
        p->SetMinSize(wxSize(0, 100));
        nb->AddPage(p, "Top Pane 1");
        nb->Split(nb->GetPageCount()-1, wxTOP);

        p = new wxPanel(nb);
        p->SetMinSize(wxSize(0, 50));
        nb->AddPage(p, "Top Pane 2");
        nb->Split(nb->GetPageCount()-1, wxTOP);

        CHECK( nb->GetBestSize() == wxSize(100, 250 + 3*tabHeight) );

        p = new wxPanel(nb);
        p->SetMinSize(wxSize(0, 25));
        nb->AddPage(p, "Bottom Pane");
        nb->Split(nb->GetPageCount()-1, wxBOTTOM);

        CHECK( nb->GetBestSize() == wxSize(100, 275 + 4*tabHeight) );
    }

    SUBCASE( "Surrounding panes" )
    {
        p = new wxPanel(nb);
        p->SetMinSize(wxSize(50, 25));
        nb->AddPage(p, "Bottom Pane");
        nb->Split(nb->GetPageCount()-1, wxBOTTOM);

        p = new wxPanel(nb);
        p->SetMinSize(wxSize(50, 120));
        nb->AddPage(p, "Right Pane");
        nb->Split(nb->GetPageCount()-1, wxRIGHT);

        p = new wxPanel(nb);
        p->SetMinSize(wxSize(225, 50));
        nb->AddPage(p, "Top Pane");
        nb->Split(nb->GetPageCount()-1, wxTOP);

        p = new wxPanel(nb);
        p->SetMinSize(wxSize(25, 105));
        nb->AddPage(p, "Left Pane");
        nb->Split(nb->GetPageCount()-1, wxLEFT);

        CHECK( nb->GetBestSize() == wxSize(250, 175 + 3*tabHeight) );
    }
}

#endif
