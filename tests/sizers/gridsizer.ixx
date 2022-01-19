///////////////////////////////////////////////////////////////////////////////
// Name:        tests/sizers/gridsizer.cpp
// Purpose:     Unit tests for wxGridSizer and wxFlexGridSizer.
// Author:      Vadim Zeitlin
// Created:     2015-04-03
// Copyright:   (c) 2015 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/window.h"

export module WX.Test.GridSizer;

import WX.MetaTest;
import Utils.Geometry;
import WX.Core.Sizer;

import WX.Test.Prec;

namespace ut = boost::ut;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

struct GridSizerTestCase
{
    GridSizerTestCase();

    // Clear the current sizer contents and add the specified windows to it,
    // using the same flags for all of them.
    void SetChildren(const std::vector<wxWindow*>& children,
                     const wxSizerFlags& flags);

    std::unique_ptr<wxWindow> m_win;
    wxFlexGridSizer* m_sizer;
};

// ----------------------------------------------------------------------------
// test initialization
// ----------------------------------------------------------------------------

GridSizerTestCase::GridSizerTestCase()
{
    m_win = std::make_unique<wxWindow>(wxTheApp->GetTopWindow(), wxID_ANY);
    m_win->SetClientSize(127, 35);

    m_sizer = new wxFlexGridSizer(2);
    m_win->SetSizer(m_sizer);
}

// ----------------------------------------------------------------------------
// helpers
// ----------------------------------------------------------------------------

void GridSizerTestCase::SetChildren(const std::vector<wxWindow*>& children,
                                    const wxSizerFlags& flags)
{
    m_sizer->Clear();
    for (const auto& child : children)
    {
        m_sizer->Add(child, flags);
    }

    m_win->Layout();
}

// ----------------------------------------------------------------------------
// tests themselves
// ----------------------------------------------------------------------------

export ut::suite GridSizerLayoutTest = []
{
    using namespace ut;

    GridSizerTestCase gridtest{};

    const wxSize sizeTotal = gridtest.m_win->GetClientSize();
    const wxSize sizeChild(sizeTotal.x / 4, sizeTotal.y / 4);
    const wxSize sizeRest(sizeTotal.x - sizeTotal.x / 4,
                          sizeTotal.y - sizeTotal.y / 4);

    std::vector<wxWindow*> children;

    for ( int n = 0; n < 4; n++ )
    {
        children.push_back(new wxWindow(gridtest.m_win.get(), wxID_ANY, wxDefaultPosition,
                                        sizeChild));
    }

    gridtest.m_sizer->AddGrowableRow(1);
    gridtest.m_sizer->AddGrowableCol(1);

    // Without Expand() windows have their initial size.
    "No flags"_test = [&gridtest, children, sizeChild]
    {
        gridtest.SetChildren(children, wxSizerFlags());
        expect( children[0]->GetSize() == sizeChild );
        expect( children[1]->GetSize() == sizeChild );
        expect( children[2]->GetSize() == sizeChild );
        expect( children[3]->GetSize() == sizeChild );
    };

    // With just expand, they expand to fill the entire column and the row
    // containing them (which may or not expand on its own).
    "Expand"_test = [&gridtest, children, sizeChild, sizeRest]
    {
        gridtest.SetChildren(children, wxSizerFlags().Expand());
        expect( children[0]->GetSize() == sizeChild );
        expect( children[1]->GetSize() == wxSize(sizeRest.x, sizeChild.y) );
        expect( children[2]->GetSize() == wxSize(sizeChild.x, sizeRest.y) );
        expect( children[3]->GetSize() == sizeRest );
    };

    // With expand and another alignment flag, they should expand only in the
    // direction in which the alignment is not given.
    "Expand and center vertically"_test = [&gridtest, children, sizeChild, sizeRest]
    {
        gridtest.SetChildren(children, wxSizerFlags().Expand().CentreVertical());
        expect( children[0]->GetSize() == sizeChild );
        expect( children[1]->GetSize() == wxSize(sizeRest.x, sizeChild.y) );
        expect( children[2]->GetSize() == sizeChild );
        expect( children[3]->GetSize() == wxSize(sizeRest.x, sizeChild.y) );
    };

    // Same as above but mirrored.
    "Expand and center horizontally"_test = [&gridtest, children, sizeChild, sizeRest]
    {
        gridtest.SetChildren(children, wxSizerFlags().Expand().CentreHorizontal());
        expect( children[0]->GetSize() == sizeChild );
        expect( children[1]->GetSize() == sizeChild );
        expect( children[2]->GetSize() == wxSize(sizeChild.x, sizeRest.y) );
        expect( children[3]->GetSize() == wxSize(sizeChild.x, sizeRest.y) );
    };

    // Test alignment flag too.
    "Right align"_test = [&gridtest, children, sizeChild, sizeRest]
    {
        gridtest.SetChildren(children, wxSizerFlags().Right());
        expect( children[0]->GetPosition() == wxPoint(         0,           0) );
        expect( children[1]->GetPosition() == wxPoint(sizeRest.x,           0) );
        expect( children[2]->GetPosition() == wxPoint(         0, sizeChild.y) );
        expect( children[3]->GetPosition() == wxPoint(sizeRest.x, sizeChild.y) );
    };

    // Also test combining centering in one direction and aligning in another.
    "Right align and center vertically"_test = [&gridtest, children, sizeChild, sizeRest]
    {
        gridtest.SetChildren(children, wxSizerFlags().Right().CentreVertical());

        const int yMid = sizeChild.y + (sizeRest.y - sizeChild.y) / 2;

        expect( children[0]->GetPosition() == wxPoint(         0,    0) );
        expect( children[1]->GetPosition() == wxPoint(sizeRest.x,    0) );
        expect( children[2]->GetPosition() == wxPoint(         0, yMid) );
        expect( children[3]->GetPosition() == wxPoint(sizeRest.x, yMid) );
    };

    "IncompatibleFlags"_test = [&]
    {
        expect(throws([&]{ gridtest.m_sizer->Add(10, 10, wxSizerFlags().Expand().Centre()); })) <<
            "Combining wxEXPAND and wxCENTRE should throw.";
    };
};
