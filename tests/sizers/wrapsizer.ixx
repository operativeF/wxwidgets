///////////////////////////////////////////////////////////////////////////////
// Name:        tests/sizers/wrapsizer.cpp
// Purpose:     Unit tests for wxWrapSizer
// Author:      Catalin Raceanu
// Created:     2010-10-23
// Copyright:   (c) 2010 wxWidgets development team
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/window.h"
#include "wx/wrapsizer.h"

export module WX.Test.WrapSizer;

import WX.MetaTest;
import WX.Test.Prec;

namespace ut = boost::ut;

export
{

ut::suite WrapSizerCalcMinTest = []
{
    using namespace ut;

    auto win = std::make_unique<wxWindow>(wxTheApp->GetTopWindow(), wxID_ANY);
    win->SetClientSize(180, 240);

    wxSizer *sizer = new wxWrapSizer(wxHORIZONTAL);
    win->SetSizer(sizer);

    wxSize sizeMinExpected;

    // With a single child the min size must be the same as child size.
    const wxSize sizeChild1 = wxSize(80, 60);
    sizeMinExpected = sizeChild1;

    wxWindow * const
        child1 = new wxWindow(win.get(), wxID_ANY, wxDefaultPosition, sizeChild1);
    child1->SetBackgroundColour(*wxRED);
    sizer->Add(child1);
    win->Layout();

    expect( sizeMinExpected == sizer->CalcMin() );

    // If both children can fit in the same row, the minimal size of the sizer
    // is determined by the sum of their minimal horizontal dimensions and
    // the maximum of their minimal vertical dimensions.
    const wxSize sizeChild2 = wxSize(100, 80);
    sizeMinExpected.x += sizeChild2.x;
    sizeMinExpected.y = std::max(sizeChild1.y, sizeChild2.y);

    wxWindow * const
        child2 = new wxWindow(win.get(), wxID_ANY, wxDefaultPosition, sizeChild2);
    child2->SetBackgroundColour(*wxYELLOW);
    sizer->Add(child2);
    win->Layout();

    expect( sizeMinExpected == sizer->CalcMin() );

    // Three children will take at least two rows so the minimal size in
    // vertical direction must increase.
    const wxSize sizeChild3 = wxSize(90, 40);
    sizeMinExpected.y += sizeChild3.y;

    wxWindow * const
        child3 = new wxWindow(win.get(), wxID_ANY, wxDefaultPosition, sizeChild3);
    child3->SetBackgroundColour(*wxGREEN);
    sizer->Add(child3);
    win->Layout();

    expect( sizeMinExpected == sizer->CalcMin() );
};

ut::suite WrapSizerCalcMinFromMinorTest = []
{
    using namespace ut;

    auto win = std::make_unique<wxWindow>(wxTheApp->GetTopWindow(), wxID_ANY);
    win->SetClientSize(180, 240);

    wxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
    win->SetSizer(boxSizer);

    // To test CalcMinFromMinor function the wrap sizer with the
    // horizonral align added to the box sizer with horizontal align.
    wxSizer* wrapSizer = new wxWrapSizer(wxHORIZONTAL);
    boxSizer->Add(wrapSizer);

    // Add three child windows. Sum of the first and the second windows widths should
    // be less than the width of the third window.
    const wxSize sizeChild1 = wxSize(40, 60);
    wxWindow * const
        child1 = new wxWindow(win.get(), wxID_ANY, wxDefaultPosition, sizeChild1);
    child1->SetBackgroundColour(*wxRED);
    wrapSizer->Add(child1);

    const wxSize sizeChild2 = wxSize(50, 80);
    wxWindow * const
        child2 = new wxWindow(win.get(), wxID_ANY, wxDefaultPosition, sizeChild2);
    child2->SetBackgroundColour(*wxGREEN);
    wrapSizer->Add(child2);

    const wxSize sizeChild3 = wxSize(100, 120);
    wxWindow * const
        child3 = new wxWindow(win.get(), wxID_ANY, wxDefaultPosition, sizeChild3);
    child3->SetBackgroundColour(*wxBLUE);
    wrapSizer->Add(child3);

    // First two windows should be in a first row and the third in a second row.
    const wxSize sizeMinExpected = wxSize(sizeChild3.x, sizeChild2.y + sizeChild3.y);
    win->Layout();
    expect(sizeMinExpected == wrapSizer->CalcMin());
};

} // export
