///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/radiobuttontest.cpp
// Purpose:     wxRadioButton unit test
// Author:      Steven Lamerton
// Created:     2010-07-30
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_RADIOBTN

#include "wx/app.h"
#include "wx/button.h"
#include "wx/panel.h"
#include "wx/radiobut.h"
#include "wx/stattext.h"
#include "wx/uiaction.h"

#include "testableframe.h"
#include "testwindow.h"

import WX.Core.Sizer;

import WX.Test.Prec;

class RadioButtonTestCase
{
public:
    RadioButtonTestCase();

protected:
    std::unique_ptr<wxRadioButton> m_radio;
};

RadioButtonTestCase::RadioButtonTestCase()
{
    m_radio = std::make_unique<wxRadioButton>(wxTheApp->GetTopWindow(), wxID_ANY,
                                              "wxRadioButton");
    m_radio->Update();
    m_radio->Refresh();
}

TEST_CASE_FIXTURE(RadioButtonTestCase, "RadioButton::Click")
{
    // OS X doesn't support selecting a single radio button
#if wxUSE_UIACTIONSIMULATOR && !defined(__WXOSX__)
    EventCounter selected(m_radio.get(), wxEVT_RADIOBUTTON);

    wxUIActionSimulator sim;
    wxYield();

    sim.MouseMove(m_radio->GetScreenPosition() + wxPoint(10, 10));
    sim.MouseClick();

    wxYield();

    CHECK(selected.GetCount() == 1);
#endif
}

TEST_CASE_FIXTURE(RadioButtonTestCase, "RadioButton::Value")
{
#ifndef __WXGTK__
    EventCounter selected(m_radio.get(), wxEVT_RADIOBUTTON);

    m_radio->SetValue(true);

    CHECK(m_radio->GetValue());

    m_radio->SetValue(false);

    CHECK(!m_radio->GetValue());

    CHECK(selected.GetCount() == 0);
#endif
}

TEST_CASE("RadioButton::Group")
{
    wxWindow* const parent = wxTheApp->GetTopWindow();

    // Create two different radio groups.
    auto g1radio0 = std::make_unique<wxRadioButton>(parent, wxID_ANY, "radio 1.0",
                                                wxDefaultPosition, wxDefaultSize,
                                                wxRB_GROUP);

    auto g1radio1 = std::make_unique<wxRadioButton>(parent, wxID_ANY, "radio 1.1");

    auto g2radio0 = std::make_unique<wxRadioButton>(parent, wxID_ANY, "radio 2.0",
                                                wxDefaultPosition, wxDefaultSize,
                                                wxRB_GROUP);

    auto g2radio1 = std::make_unique<wxRadioButton>(parent, wxID_ANY, "radio 2.1");

    // Check that having another control between radio buttons doesn't break
    // grouping.
    auto text = std::make_unique<wxStaticText>(parent, wxID_ANY, "Label");
    auto g2radio2 = std::make_unique<wxRadioButton>(parent, wxID_ANY, "radio 2.2");

    g1radio0->SetValue(true);
    g2radio0->SetValue(true);

    CHECK(g1radio0->GetValue());
    CHECK(!g1radio1->GetValue());
    CHECK(g2radio0->GetValue());
    CHECK(!g2radio1->GetValue());

    g1radio1->SetValue(true);
    g2radio1->SetValue(true);

    CHECK(!g1radio0->GetValue());
    CHECK(g1radio1->GetValue());
    CHECK(!g2radio0->GetValue());
    CHECK(g2radio1->GetValue());

    g2radio2->SetValue(true);
    CHECK(!g2radio0->GetValue());
    CHECK(!g2radio1->GetValue());
    CHECK(g2radio2->GetValue());

    g1radio0->SetValue(true);
    g2radio0->SetValue(true);

    CHECK(g1radio0->GetValue());
    CHECK(!g1radio1->GetValue());
    CHECK(g2radio0->GetValue());
    CHECK(!g2radio1->GetValue());


    // Check that group navigation functions behave as expected.

    // GetFirstInGroup()
    CHECK_SAME_WINDOW(g1radio0->GetFirstInGroup(), g1radio0);
    CHECK_SAME_WINDOW(g1radio1->GetFirstInGroup(), g1radio0);

    CHECK_SAME_WINDOW(g2radio0->GetFirstInGroup(), g2radio0);
    CHECK_SAME_WINDOW(g2radio1->GetFirstInGroup(), g2radio0);
    CHECK_SAME_WINDOW(g2radio2->GetFirstInGroup(), g2radio0);

    // GetLastInGroup()
    CHECK_SAME_WINDOW(g1radio0->GetLastInGroup(), g1radio1);
    CHECK_SAME_WINDOW(g1radio1->GetLastInGroup(), g1radio1);

    CHECK_SAME_WINDOW(g2radio0->GetLastInGroup(), g2radio2);
    CHECK_SAME_WINDOW(g2radio1->GetLastInGroup(), g2radio2);
    CHECK_SAME_WINDOW(g2radio2->GetLastInGroup(), g2radio2);

    // GetNextInGroup()
    CHECK_SAME_WINDOW(g1radio0->GetNextInGroup(), g1radio1);
    CHECK_SAME_WINDOW(g1radio1->GetNextInGroup(), nullptr);

    CHECK_SAME_WINDOW(g2radio0->GetNextInGroup(), g2radio1);
    CHECK_SAME_WINDOW(g2radio1->GetNextInGroup(), g2radio2);
    CHECK_SAME_WINDOW(g2radio2->GetNextInGroup(), nullptr);

    // GetPreviousInGroup()
    CHECK_SAME_WINDOW(g1radio0->GetPreviousInGroup(), nullptr);
    CHECK_SAME_WINDOW(g1radio1->GetPreviousInGroup(), g1radio0);

    CHECK_SAME_WINDOW(g2radio0->GetPreviousInGroup(), nullptr);
    CHECK_SAME_WINDOW(g2radio1->GetPreviousInGroup(), g2radio0);
    CHECK_SAME_WINDOW(g2radio2->GetPreviousInGroup(), g2radio1);
}

TEST_CASE("RadioButton::Single")
{
    //Create a group of 2 buttons, having second button selected
    auto gradio0 = std::make_unique<wxRadioButton>(wxTheApp->GetTopWindow(),
        wxID_ANY, "wxRadioButton",
        wxDefaultPosition,
        wxDefaultSize, wxRB_GROUP);

    auto gradio1 = std::make_unique<wxRadioButton>(wxTheApp->GetTopWindow(),
        wxID_ANY, "wxRadioButton");

    gradio1->SetValue(true);

    //Create a "single" button (by default it will not be selected)
    auto sradio = std::make_unique<wxRadioButton>(wxTheApp->GetTopWindow(),
        wxID_ANY, "wxRadioButton",
        wxDefaultPosition,
        wxDefaultSize, wxRB_SINGLE);

    //Create a non-grouped button and select it
    auto ngradio = std::make_unique<wxRadioButton>(wxTheApp->GetTopWindow(),
        wxID_ANY, "wxRadioButton");

    ngradio->SetValue(true);

    //Select the "single" button
    sradio->SetValue(true);

    CHECK(gradio1->GetValue());
    CHECK(ngradio->GetValue());

    // Also check that navigation works as expected with "single" buttons.
    CHECK_SAME_WINDOW(sradio->GetFirstInGroup(), sradio);
    CHECK_SAME_WINDOW(sradio->GetLastInGroup(), sradio);
    CHECK_SAME_WINDOW(sradio->GetPreviousInGroup(), nullptr);
    CHECK_SAME_WINDOW(sradio->GetNextInGroup(), nullptr);
}

TEST_CASE("RadioButton::Focus")
{
    // Create a container panel just to be able to destroy all the windows
    // created here at once by simply destroying it.
    wxWindow* const tlw = wxTheApp->GetTopWindow();
    auto parentPanel = std::make_unique<wxPanel>(tlw);

    // Create a panel containing 2 radio buttons and another control outside
    // this panel, so that we could give focus to something different and then
    // return it back to the panel.
    wxPanel* const radioPanel = new wxPanel(parentPanel.get());
    wxRadioButton* const radio1 = new wxRadioButton(radioPanel, wxID_ANY, "1");
    wxRadioButton* const radio2 = new wxRadioButton(radioPanel, wxID_ANY, "2");
    wxSizer* const radioSizer = new wxBoxSizer(wxHORIZONTAL);
    radioSizer->Add(radio1);
    radioSizer->Add(radio2);
    radioPanel->SetSizer(radioSizer);

    wxButton* const dummyButton = new wxButton(parentPanel.get(), wxID_OK);

    wxSizer* const sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(radioPanel, wxSizerFlags(1).Expand());
    sizer->Add(dummyButton, wxSizerFlags().Expand());
    parentPanel->SetSizer(sizer);

    parentPanel->SetSize(tlw->GetClientSize());
    parentPanel->Layout();

    // Initially the first radio button should be checked.
    radio1->SetFocus();
    CHECK(radio1->GetValue());
    CHECK_FOCUS_IS(radio1);

    // Switching focus from it shouldn't change this.
    dummyButton->SetFocus();
    CHECK(radio1->GetValue());

    // Checking another radio button should make it checked and uncheck the
    // first one.
    radio2->SetValue(true);
    CHECK(!radio1->GetValue());
    CHECK(radio2->GetValue());

    // While not changing focus.
    CHECK_FOCUS_IS(dummyButton);

    // And giving the focus to the panel shouldn't change radio button
    // selection.
    radioPanel->SetFocus();

    // Under MSW, focus is always on the selected button, but in the other
    // ports this is not necessarily the case, i.e. under wxGTK this check
    // would fail because focus gets set to the first button -- even though the
    // second one remains checked.
#ifdef __WXMSW__
    CHECK_FOCUS_IS(radio2);
#endif

    CHECK(!radio1->GetValue());
    CHECK(radio2->GetValue());
}

#endif //wxUSE_RADIOBTN
