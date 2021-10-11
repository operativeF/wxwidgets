///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/checkboxtest.cpp
// Purpose:     wCheckBox unit test
// Author:      Steven Lamerton
// Created:     2010-07-14
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_CHECKBOX


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/checkbox.h"
#endif // WX_PRECOMP

#include "testableframe.h"


// Initialize m_check with a new checkbox with the specified style
//
// This function always returns false just to make it more convenient to
// use inside WX_ASSERT_FAILS_WITH_ASSERT(), its return value doesn't have
// any meaning otherwise

TEST_CASE("Check")
{
    auto m_check = std::make_unique<wxCheckBox>(wxTheApp->GetTopWindow(),
                                                wxID_ANY, "Check box");

    EventCounter clicked(m_check.get(), wxEVT_CHECKBOX);

    //We should be unchecked by default
    CHECK(!m_check->IsChecked());

    m_check->SetValue(true);

    CHECK(m_check->IsChecked());

    m_check->SetValue(false);

    CHECK(!m_check->IsChecked());

    m_check->Set3StateValue(wxCheckBoxState::Checked);

    CHECK(m_check->IsChecked());

    m_check->Set3StateValue(wxCheckBoxState::Unchecked);

    CHECK(!m_check->IsChecked());

    //None of these should send events
    CHECK_EQ(0, clicked.GetCount());
}

#ifdef wxHAS_3STATE_CHECKBOX
TEST_CASE("ThirdState")
{
    auto m_check = std::make_unique<wxCheckBox>(wxTheApp->GetTopWindow(), wxID_ANY, "Check box",
                                            wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);

    CHECK_EQ(wxCheckBoxState::Unchecked, m_check->Get3StateValue());
    CHECK(m_check->Is3State());
    CHECK(!m_check->Is3rdStateAllowedForUser());

    m_check->SetValue(true);

    CHECK_EQ(wxCheckBoxState::Checked, m_check->Get3StateValue());

    m_check->Set3StateValue(wxCheckBoxState::Indeterminate);

    CHECK_EQ(wxCheckBoxState::Indeterminate, m_check->Get3StateValue());
}

TEST_CASE("ThirdStateUser")
{
    auto m_check = std::make_unique<wxCheckBox>(wxTheApp->GetTopWindow(), wxID_ANY, "Check box",
        wxDefaultPosition, wxDefaultSize, wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER);

    CHECK_EQ(wxCheckBoxState::Unchecked, m_check->Get3StateValue());
    CHECK(m_check->Is3State());
    CHECK(m_check->Is3rdStateAllowedForUser());

    m_check->SetValue(true);

    CHECK_EQ(wxCheckBoxState::Checked, m_check->Get3StateValue());

    m_check->Set3StateValue(wxCheckBoxState::Indeterminate);

    CHECK_EQ(wxCheckBoxState::Indeterminate, m_check->Get3StateValue());
}

TEST_CASE("InvalidStyles")
{
    // Check that using incompatible styles doesn't work.
    WX_ASSERT_FAILS_WITH_ASSERT( auto failed_cb = std::make_unique<wxCheckBox>(
        wxTheApp->GetTopWindow(), wxID_ANY, "Check box",
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE | wxCHK_3STATE) );

#if !wxDEBUG_LEVEL
    CHECK( !m_check->Is3State() );
    CHECK( !m_check->Is3rdStateAllowedForUser() );
#endif

    WX_ASSERT_FAILS_WITH_ASSERT(auto failed_cb = std::make_unique<wxCheckBox>(
        wxTheApp->GetTopWindow(), wxID_ANY, "Check box",
        wxDefaultPosition, wxDefaultSize,
        wxCHK_2STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER));

#if !wxDEBUG_LEVEL
    CHECK( !m_check->Is3State() );
    CHECK( !m_check->Is3rdStateAllowedForUser() );
#endif

    // wxCHK_ALLOW_3RD_STATE_FOR_USER without wxCHK_3STATE doesn't work.
    WX_ASSERT_FAILS_WITH_ASSERT(auto failed_cb = std::make_unique<wxCheckBox>(
        wxTheApp->GetTopWindow(), wxID_ANY, "Check box",
        wxDefaultPosition, wxDefaultSize,
        wxCHK_ALLOW_3RD_STATE_FOR_USER));
}

#endif // wxHAS_3STATE_CHECKBOX

#endif // wxUSE_CHECKBOX
