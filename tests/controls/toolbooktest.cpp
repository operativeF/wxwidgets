///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/toolbooktest.cpp
// Purpose:     wxToolbook unit test
// Author:      Steven Lamerton
// Created:     2010-07-02
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_TOOLBOOK

#include "wx/app.h"
#include "wx/toolbar.h"
#include "wx/toolbook.h"

#include "bookctrlbasetest.h"

import WX.Test.Prec;

using wxToolbookTest = BookCtrlBaseT<wxToolbook>;

TEST_CASE_FIXTURE(wxToolbookTest, "List book test")
{
    m_bookctrl = std::make_unique<wxToolbook>(wxTheApp->GetTopWindow(), wxID_ANY,
                                              wxDefaultPosition, wxSize(400, 200));
    AddPanels();

    auto toolbar = static_cast<wxToolBar*>(m_bookctrl->GetToolBar());

    SUBCASE("Toolbar Test")
    {
        CHECK(toolbar);
        CHECK_EQ(3, toolbar->GetToolsCount());
    }

    wxBOOK_CTRL_BASE_TESTS();
}

#endif //wxUSE_TOOLBOOK
