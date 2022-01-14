///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/checkboxtest.cpp
// Purpose:     wCheckBox unit test
// Author:      Steven Lamerton
// Created:     2010-07-14
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/checkbox.h"

#include "testableframe.h"

#include <memory>

export module WX.Test.Checkbox;

import WX.Test.Prec;
import WX.MetaTest;

import <tuple>;

#if wxUSE_CHECKBOX

// Initialize m_check with a new checkbox with the specified style
//
// This function always returns false just to make it more convenient to
// use inside CHECK_THROWS(), its return value doesn't have
// any meaning otherwise

namespace ut = boost::ut;

ut::suite StandardCheckBoxTest = []
{
    using namespace ut;

    auto m_check = std::make_unique<wxCheckBox>(wxTheApp->GetTopWindow(),
                                                wxID_ANY, "Check box");

    EventCounter clicked(m_check.get(), wxEVT_CHECKBOX);

    //We should be unchecked by default
    expect(!m_check->IsChecked());

    m_check->SetValue(true);

    expect(m_check->IsChecked());

    m_check->SetValue(false);

    expect(!m_check->IsChecked());

    m_check->Set3StateValue(wxCheckBoxState::Checked);

    expect(m_check->IsChecked());

    m_check->Set3StateValue(wxCheckBoxState::Unchecked);

    expect(!m_check->IsChecked());

    //None of these should send events
    expect(0 == clicked.GetCount());
};

#ifdef wxHAS_3STATE_CHECKBOX
ut::suite ThirdStateCBTest = []
{
    using namespace ut;

    auto m_check = std::make_unique<wxCheckBox>(wxTheApp->GetTopWindow(), wxID_ANY, "Check box",
                                            wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);

    expect(wxCheckBoxState::Unchecked == m_check->Get3StateValue());
    expect(m_check->Is3State());
    expect(!m_check->Is3rdStateAllowedForUser());

    m_check->SetValue(true);

    expect(wxCheckBoxState::Checked == m_check->Get3StateValue());

    m_check->Set3StateValue(wxCheckBoxState::Indeterminate);

    expect(wxCheckBoxState::Indeterminate == m_check->Get3StateValue());
};

ut::suite ThirdStateCBUserTest = []
{
    using namespace ut;

    auto m_check = std::make_unique<wxCheckBox>(wxTheApp->GetTopWindow(), wxID_ANY, "Check box",
        wxDefaultPosition, wxDefaultSize, wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER);

    expect(wxCheckBoxState::Unchecked == m_check->Get3StateValue());
    expect(m_check->Is3State());
    expect(m_check->Is3rdStateAllowedForUser());

    m_check->SetValue(true);

    expect(wxCheckBoxState::Checked == m_check->Get3StateValue());

    m_check->Set3StateValue(wxCheckBoxState::Indeterminate);

    expect(wxCheckBoxState::Indeterminate == m_check->Get3StateValue());
};

ut::suite InvalidStylesCBTest = []
{
    using namespace ut;
    // Check that using incompatible styles doesn't work.
    expect(throws([] { std::ignore = std::make_unique<wxCheckBox>(
        wxTheApp->GetTopWindow(), wxID_ANY, "Check box",
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE | wxCHK_3STATE); }));

    // FIXME: out of scope wxCheckbox instance
#if !wxDEBUG_LEVEL
    //CHECK( !m_check->Is3State() );
    //CHECK( !m_check->Is3rdStateAllowedForUser() );
#endif

    expect(throws([] { std::ignore = std::make_unique<wxCheckBox>(
        wxTheApp->GetTopWindow(), wxID_ANY, "Check box",
        wxDefaultPosition, wxDefaultSize,
        wxCHK_2STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER); }));

    // FIXME: out of scope wxCheckbox instance
#if !wxDEBUG_LEVEL
    //CHECK( !m_check->Is3State() );
    //CHECK( !m_check->Is3rdStateAllowedForUser() );
#endif

    // wxCHK_ALLOW_3RD_STATE_FOR_USER without wxCHK_3STATE doesn't work.
    expect(throws([] { std::ignore = std::make_unique<wxCheckBox>(
        wxTheApp->GetTopWindow(), wxID_ANY, "Check box",
        wxDefaultPosition, wxDefaultSize,
        wxCHK_ALLOW_3RD_STATE_FOR_USER); }));
};

#endif // wxHAS_3STATE_CHECKBOX

#endif // wxUSE_CHECKBOX
