///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/searchctrltest.cpp
// Purpose:     wxSearchCtrl unit test
// Author:      Vadim Zeitlin
// Created:     2013-01-20
// Copyright:   (c) 2013 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_SEARCHCTRL

#include "wx/app.h"
#include "wx/srchctrl.h"

#include "testwindow.h"

import WX.Test.Prec;

class SearchCtrlTestCase
{
public:
    SearchCtrlTestCase()
        : m_search(std::make_unique<wxSearchCtrl>(wxTheApp->GetTopWindow(),
                                                  wxID_ANY))
    {
    }

protected:
    std::unique_ptr<wxSearchCtrl> m_search;
};

#define SEARCH_CTRL_TEST_CASE(name) \
    TEST_CASE_FIXTURE(SearchCtrlTestCase, name)

// TODO OS X test only passes when run solo ...
#ifndef __WXOSX__
SEARCH_CTRL_TEST_CASE("wxSearchCtrl::Focus")
{
    m_search->SetFocus();
    CHECK_FOCUS_IS( m_search );
}
#endif // !__WXOSX__

SEARCH_CTRL_TEST_CASE("wxSearchCtrl::ChangeValue")
{
    CHECK( m_search->GetValue().empty() );

    m_search->ChangeValue("foo");
    CHECK( m_search->GetValue() == "foo" );
}

#endif // wxUSE_SEARCHCTRL
