///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/simplebooktest.cpp
// Purpose:     wxSimplebook unit test
// Author:      Vadim Zeitlin
// Created:     2013-06-23
// Copyright:   (c) 2013 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_BOOKCTRL

#include "wx/app.h"
#include "wx/simplebook.h"

#include "bookctrlbasetest.h"

import WX.Test.Prec;

using wxSimplebookTest = BookCtrlBaseT<wxSimplebook>;

TEST_CASE_FIXTURE(wxSimplebookTest, "Treebook Test")
{
    m_bookctrl = std::make_unique<wxSimplebook>(wxTheApp->GetTopWindow(), wxID_ANY);
    AddPanels();

    wxBOOK_CTRL_BASE_TESTS();
}

#endif // wxUSE_BOOKCTRL

