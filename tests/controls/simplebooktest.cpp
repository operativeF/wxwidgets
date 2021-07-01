///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/simplebooktest.cpp
// Purpose:     wxSimplebook unit test
// Author:      Vadim Zeitlin
// Created:     2013-06-23
// Copyright:   (c) 2013 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_BOOKCTRL


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/panel.h"
#endif // WX_PRECOMP

#include "wx/simplebook.h"
#include "bookctrlbasetest.h"

using wxSimplebookTest = BookCtrlBaseT<wxSimplebook>;

TEST_CASE_FIXTURE(wxSimplebookTest, "Treebook Test")
{
    m_bookctrl = std::make_unique<wxSimplebook>(wxTheApp->GetTopWindow(), wxID_ANY);
    AddPanels();

    wxBOOK_CTRL_BASE_TESTS();
}

#endif // wxUSE_BOOKCTRL

