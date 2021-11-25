///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/listbooktest.cpp
// Purpose:     wxListbook unit test
// Author:      Steven Lamerton
// Created:     2010-07-02
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_LISTBOOK

#include "wx/app.h"
#include "wx/panel.h"
#include "wx/listbook.h"
#include "wx/listctrl.h"

#include "bookctrlbasetest.h"

using wxListbookTest = BookCtrlBaseT<wxListbook>;

TEST_CASE_FIXTURE(wxListbookTest, "List book test")
{
    m_bookctrl = std::make_unique<wxListbook>(wxTheApp->GetTopWindow(), wxID_ANY,
                                              wxDefaultPosition, wxSize(400, 300));
    AddPanels();

    SUBCASE("List view")
    {
        wxListView* listview = m_bookctrl->GetListView();

        CHECK(listview);
        CHECK_EQ(3, listview->GetItemCount());
        CHECK_EQ("Panel 1", listview->GetItemText(0));
    }

    wxBOOK_CTRL_BASE_TESTS();
}

#endif //wxUSE_LISTBOOK
