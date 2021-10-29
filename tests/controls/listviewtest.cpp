///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/listviewtest.cpp
// Purpose:     wxListView unit test
// Author:      Steven Lamerton
// Created:     2010-07-10
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_LISTCTRL

#include "wx/app.h"

#include "wx/uiaction.h"
#include "wx/listctrl.h"
#include "listbasetest.h"
#include "testableframe.h"

using ListViewTest = ListBaseTest<wxListView>;

TEST_CASE_FIXTURE(ListViewTest, "List view test")
{
    m_listctrl = std::make_unique<wxListView>(wxTheApp->GetTopWindow());
    m_listctrl->SetWindowStyle(wxLC_REPORT);
    m_listctrl->SetSize(wxSize{400, 200});

    SUBCASE("Selection")
    {
        m_listctrl->InsertColumn(0, "Column 0");

        m_listctrl->InsertItem(0, "Item 0");
        m_listctrl->InsertItem(1, "Item 1");
        m_listctrl->InsertItem(2, "Item 2");
        m_listctrl->InsertItem(3, "Item 3");

        m_listctrl->Select(0);
        m_listctrl->Select(2);
        m_listctrl->Select(3);

        CHECK(m_listctrl->IsSelected(0));
        CHECK(!m_listctrl->IsSelected(1));

        long sel = m_listctrl->GetFirstSelected();

        CHECK_EQ(0, sel);

        sel = m_listctrl->GetNextSelected(sel);

        CHECK_EQ(2, sel);

        sel = m_listctrl->GetNextSelected(sel);

        CHECK_EQ(3, sel);

        sel = m_listctrl->GetNextSelected(sel);

        CHECK_EQ(-1, sel);

        m_listctrl->Select(0, false);

        CHECK(!m_listctrl->IsSelected(0));
        CHECK_EQ(2, m_listctrl->GetFirstSelected());
    }

    SUBCASE("Focus")
    {
        EventCounter focused(m_listctrl.get(), wxEVT_LIST_ITEM_FOCUSED);

        m_listctrl->InsertColumn(0, "Column 0");

        m_listctrl->InsertItem(0, "Item 0");
        m_listctrl->InsertItem(1, "Item 1");
        m_listctrl->InsertItem(2, "Item 2");
        m_listctrl->InsertItem(3, "Item 3");

        CHECK_EQ(0, focused.GetCount());
        CHECK_EQ(-1, m_listctrl->GetFocusedItem());

        m_listctrl->Focus(0);

        CHECK_EQ(1, focused.GetCount());
        CHECK_EQ(0, m_listctrl->GetFocusedItem());
    }

    wxLIST_BASE_TESTS();
}


#endif
