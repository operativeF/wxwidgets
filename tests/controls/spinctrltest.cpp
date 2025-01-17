///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/spinctrltest.cpp
// Purpose:     wxSpinCtrl unit test
// Author:      Steven Lamerton
// Created:     2010-07-21
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_SPINCTRL

#include "wx/app.h"
#include "wx/uiaction.h"
#include "wx/spinctrl.h"
#include "wx/textctrl.h"

#include "testableframe.h"

#include <memory>

import WX.Test.Prec;

class SpinCtrlTestCase1
{
public:
    SpinCtrlTestCase1()
        : m_spin(std::make_unique<wxSpinCtrl>())
    {
    }

protected:
    std::unique_ptr<wxSpinCtrl> m_spin;
};

class SpinCtrlTestCase2
{
public:
    SpinCtrlTestCase2()
        : m_spin(std::make_unique<wxSpinCtrl>(wxTheApp->GetTopWindow()))
    {
    }

protected:
    std::unique_ptr<wxSpinCtrl> m_spin;
};

class SpinCtrlTestCase3
{
public:
    SpinCtrlTestCase3()
        : m_spin(std::make_unique<wxSpinCtrl>(wxTheApp->GetTopWindow()))
    {
        m_spin->Bind(wxEVT_SPINCTRL, &SpinCtrlTestCase3::OnSpinSetValue, this);
    }

private:
    void OnSpinSetValue(wxSpinEvent &e)
    {
        // Constrain the value to be in the 1..16 range or 32.
        int newVal = e.GetValue();

        if ( newVal == 31 )
            m_spin->SetValue(16);
        else if ( newVal > 16 )
            m_spin->SetValue(32);
    }

protected:
    std::unique_ptr<wxSpinCtrl> m_spin;
};


TEST_CASE_FIXTURE(SpinCtrlTestCase2, "SpinCtrl::Init")
{
    // Initial value is defined by "initial" argument which is 0 by default.
    CHECK(m_spin->GetValue() == 0);
}

TEST_CASE_FIXTURE(SpinCtrlTestCase1, "SpinCtrl::Init2")
{
    m_spin->Create(wxTheApp->GetTopWindow(), wxID_ANY, "",
                   wxDefaultPosition, wxDefaultSize, 0,
                   0, 100, 17);

    // Recreate the control with another "initial" to check this.
    CHECK(m_spin->GetValue() == 17);
}

TEST_CASE_FIXTURE(SpinCtrlTestCase1, "SpinCtrl::Init3")
{
    m_spin->Create(wxTheApp->GetTopWindow(), wxID_ANY, "",
                   wxDefaultPosition, wxDefaultSize, 0,
                   0, 200, 150);

    // Recreate the control with another "initial" outside of standard spin
    // ctrl range.
    CHECK(m_spin->GetValue() == 150);
}

TEST_CASE_FIXTURE(SpinCtrlTestCase1, "SpinCtrl::Init4")
{
    m_spin->Create(wxTheApp->GetTopWindow(), wxID_ANY, "99",
                   wxDefaultPosition, wxDefaultSize, 0,
                   0, 100, 17);

    // Recreate the control with another "initial" outside of standard spin
    // ctrl range.
    // But if the text string is specified, it takes precedence.
    CHECK(m_spin->GetValue() == 99);
}

TEST_CASE_FIXTURE(SpinCtrlTestCase1, "SpinCtrl::NoEventsInCtor")
{
    // Verify that creating the control does not generate any events. This is
    // unexpected and shouldn't happen.
    EventCounter updatedSpin(m_spin.get(), wxEVT_SPINCTRL);
    EventCounter updatedText(m_spin.get(), wxEVT_TEXT);

    m_spin->Create(wxTheApp->GetTopWindow(), wxID_ANY, "",
                   wxDefaultPosition, wxDefaultSize, 0,
                   0, 100, 17);

    CHECK(updatedSpin.GetCount() == 0);
    CHECK(updatedText.GetCount() == 0);
}

TEST_CASE_FIXTURE(SpinCtrlTestCase2, "SpinCtrl::Arrows")
{
#if wxUSE_UIACTIONSIMULATOR
    EventCounter updated(m_spin.get(), wxEVT_SPINCTRL);

    wxUIActionSimulator sim;

    m_spin->SetFocus();
    wxYield();

    sim.Char(WXK_UP);

    wxYield();

    CHECK(updated.GetCount() == 1);
    CHECK(m_spin->GetValue() == 1);
    updated.Clear();

    sim.Char(WXK_DOWN);

    wxYield();

    CHECK(updated.GetCount() == 1);
    CHECK(m_spin->GetValue() == 0);
#endif
}

TEST_CASE_FIXTURE(SpinCtrlTestCase1, "SpinCtrl::Wrap")
{
#if wxUSE_UIACTIONSIMULATOR
    m_spin->Create(wxTheApp->GetTopWindow(), wxID_ANY, "",
                   wxDefaultPosition, wxDefaultSize,
                   wxSP_ARROW_KEYS | wxSP_WRAP);

    wxUIActionSimulator sim;

    m_spin->SetFocus();
    wxYield();

    sim.Char(WXK_DOWN);

    wxYield();

    CHECK(m_spin->GetValue() == 100);

    sim.Char(WXK_UP);

    wxYield();

    CHECK(m_spin->GetValue() == 0);
#endif
}

TEST_CASE_FIXTURE(SpinCtrlTestCase2, "SpinCtrl::Range")
{
    CHECK(m_spin->GetMin() == 0);
    CHECK(m_spin->GetMax() == 100);
    CHECK(m_spin->GetBase() == 10);

    // Test that the value is adjusted to be inside the new valid range but
    // that this doesn't result in any events (as this is not something done by
    // the user).
    SUBCASE("Test value adjustment for no new events.")
    {
        EventCounter updatedSpin(m_spin.get(), wxEVT_SPINCTRL);
        EventCounter updatedText(m_spin.get(), wxEVT_TEXT);

        m_spin->SetRange(1, 10);
        CHECK(m_spin->GetValue() == 1);

        CHECK(updatedSpin.GetCount() == 0);
        CHECK(updatedText.GetCount() == 0);
    }

    // Test negative ranges
    m_spin->SetRange(-10, 10);

    CHECK(m_spin->GetMin() == -10);
    CHECK(m_spin->GetMax() == 10);

    // With base 16 only ranges including values >= 0 are allowed
    m_spin->SetRange(0, 10);
    int oldMinVal = m_spin->GetMin();
    int oldMaxVal = m_spin->GetMax();
    CHECK(oldMinVal == 0);
    CHECK(oldMaxVal == 10);

    CHECK(m_spin->SetBase(16) == true);
    CHECK(m_spin->GetBase() == 16);

    // New range should be silently ignored
    m_spin->SetRange(-20, 20);
    CHECK(m_spin->GetMin() == oldMinVal);
    CHECK(m_spin->GetMax() == oldMaxVal);

    // This range should be accepted
    m_spin->SetRange(2, 8);
    CHECK(m_spin->GetMin() == 2);
    CHECK(m_spin->GetMax() == 8);

    CHECK(m_spin->SetBase(10) == true);

    CHECK(m_spin->GetBase() == 10);

    //Test backwards ranges
    m_spin->SetRange(75, 50);

    CHECK(m_spin->GetMin() == 75);
    CHECK(m_spin->GetMax() == 50);
}

TEST_CASE_FIXTURE(SpinCtrlTestCase2, "SpinCtrl::Value")
{
    EventCounter updatedSpin(m_spin.get(), wxEVT_SPINCTRL);
    EventCounter updatedText(m_spin.get(), wxEVT_TEXT);

    CHECK(m_spin->GetValue() == 0);

    m_spin->SetValue(50);
    CHECK(m_spin->GetValue() == 50);

    m_spin->SetValue(-10);
    CHECK(m_spin->GetValue() == 0);

    m_spin->SetValue(110);
    CHECK(m_spin->GetValue() == 100);

    // Calling SetValue() shouldn't have generated any events.
    CHECK(updatedSpin.GetCount() == 0);
    CHECK(updatedText.GetCount() == 0);

    // Also test that setting the text value works.
    CHECK( m_spin->GetTextValue() == "100" );

    m_spin->SetValue("57");
    CHECK( m_spin->GetTextValue() == "57" );
    CHECK( m_spin->GetValue() == 57 );

    CHECK(updatedSpin.GetCount() == 0);
    CHECK(updatedText.GetCount() == 0);

    m_spin->SetValue("");
    CHECK( m_spin->GetTextValue().empty() );
    CHECK( m_spin->GetValue() == 0 );

    CHECK(updatedSpin.GetCount() == 0);
    CHECK(updatedText.GetCount() == 0);
}

TEST_CASE_FIXTURE(SpinCtrlTestCase2, "SpinCtrl::Base")
{
    CHECK(m_spin->GetMin() == 0);
    CHECK(m_spin->GetMax() == 100);
    CHECK(m_spin->GetBase() == 10);

    // Only 10 and 16 bases are allowed
    CHECK(m_spin->SetBase(10) == true);
    CHECK(m_spin->GetBase() == 10);

    CHECK_FALSE(m_spin->SetBase(8));
    CHECK(m_spin->GetBase() == 10);

    CHECK_FALSE(m_spin->SetBase(2));
    CHECK(m_spin->GetBase() == 10);

    CHECK(m_spin->SetBase(16) == true);
    CHECK(m_spin->GetBase() == 16);

    CHECK(m_spin->SetBase(10) == true);
    CHECK(m_spin->GetBase() == 10);

    // When range contains negative values only base 10 is allowed
    m_spin->SetRange(-10, 10);
    CHECK(m_spin->GetMin() == -10);
    CHECK(m_spin->GetMax() == 10);

    CHECK_FALSE(m_spin->SetBase(8));
    CHECK(m_spin->GetBase() == 10);

    CHECK_FALSE(m_spin->SetBase(2));
    CHECK(m_spin->GetBase() == 10);

    CHECK_FALSE(m_spin->SetBase(16));
    CHECK(m_spin->GetBase() == 10);

    CHECK(m_spin->SetBase(10) == true);
    CHECK(m_spin->GetBase() == 10);
}

#if wxUSE_UIACTIONSIMULATOR

TEST_CASE_FIXTURE(SpinCtrlTestCase3, "SpinCtrl::SetValueInsideEventHandler")
{
    // A dummy control with which we change the focus.
    auto text = std::make_unique<wxTextCtrl>(wxTheApp->GetTopWindow(), wxID_ANY);
    text->Move(wxPoint{m_spin->GetSize().x, m_spin->GetSize().y * 3});

    wxUIActionSimulator sim;

    // run multiple times to make sure there are no issues with keeping old value
    for ( size_t i = 0; i < 2; i++ )
    {
        m_spin->SetFocus();
        wxYield();

        sim.Char(WXK_DELETE);
        sim.Char(WXK_DELETE);
        sim.Text("20");
        wxYield();

        text->SetFocus();
        wxYield();

        CHECK(m_spin->GetValue() == 32);
    }
}

#endif // wxUSE_UIACTIONSIMULATOR

#endif
