///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/treebooktest.cpp
// Purpose:     wxtreebook unit test
// Author:      Steven Lamerton
// Created:     2010-07-02
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_TREEBOOK

#include "wx/app.h"
#include "wx/panel.h"
#include "wx/treebook.h"

#include "bookctrlbasetest.h"

using wxTreebookTest = BookCtrlBaseT<wxTreebook>;

TEST_CASE_FIXTURE(wxTreebookTest, "Treebook Test")
{
    m_bookctrl = std::make_unique<wxTreebook>(wxTheApp->GetTopWindow(), wxID_ANY);
    AddPanels();

    SUBCASE("SubPages")
    {
        wxPanel* subpanel1 = new wxPanel(m_bookctrl.get());
        wxPanel* subpanel2 = new wxPanel(m_bookctrl.get());
        wxPanel* subpanel3 = new wxPanel(m_bookctrl.get());

        m_bookctrl->AddSubPage(subpanel1, "Subpanel 1", false, 0);

        CHECK_EQ(2, m_bookctrl->GetPageParent(3));

        m_bookctrl->InsertSubPage(1, subpanel2, "Subpanel 2", false, 1);

        CHECK_EQ(1, m_bookctrl->GetPageParent(2));

        m_bookctrl->AddSubPage(subpanel3, "Subpanel 3", false, 2);

        CHECK_EQ(3, m_bookctrl->GetPageParent(5));
    }

    SUBCASE("ContainerPage")
    {
        // Get rid of the pages added in setUp().
        m_bookctrl->DeleteAllPages();
        CHECK( m_bookctrl->GetPageCount() == 0 );

        // Adding a page without the associated window should be allowed.
        REQUIRE_NOTHROW( m_bookctrl->AddPage(nullptr, "Container page") );
        CHECK( m_bookctrl->GetPageParent(0) == -1 );

        m_bookctrl->AddSubPage(new wxPanel(m_bookctrl.get()), "Child page");
        CHECK( m_bookctrl->GetPageParent(1) == 0 );
    }

    SUBCASE("Expand")
    {
        wxPanel* subpanel1 = new wxPanel(m_bookctrl.get());
        wxPanel* subpanel2 = new wxPanel(m_bookctrl.get());
        wxPanel* subpanel3 = new wxPanel(m_bookctrl.get());

        m_bookctrl->AddSubPage(subpanel1, "Subpanel 1", false, 0);
        m_bookctrl->InsertSubPage(1, subpanel2, "Subpanel 2", false, 1);
        m_bookctrl->AddSubPage(subpanel3, "Subpanel 3", false, 2);

        CHECK(!m_bookctrl->IsNodeExpanded(1));
        CHECK(!m_bookctrl->IsNodeExpanded(3));

        m_bookctrl->CollapseNode(1);

        CHECK(!m_bookctrl->IsNodeExpanded(1));

        m_bookctrl->ExpandNode(3, false);

        CHECK(!m_bookctrl->IsNodeExpanded(3));

        m_bookctrl->ExpandNode(1);

        CHECK(m_bookctrl->IsNodeExpanded(1));
    }

    SUBCASE("Delete")
    {
        wxPanel* subpanel1 = new wxPanel(m_bookctrl.get());
        wxPanel* subpanel2 = new wxPanel(m_bookctrl.get());
        wxPanel* subpanel3 = new wxPanel(m_bookctrl.get());

        m_bookctrl->AddSubPage(subpanel1, "Subpanel 1", false, 0);
        m_bookctrl->InsertSubPage(1, subpanel2, "Subpanel 2", false, 1);
        m_bookctrl->AddSubPage(subpanel3, "Subpanel 3", false, 2);

        CHECK_EQ(6, m_bookctrl->GetPageCount());

        m_bookctrl->DeletePage(3);

        CHECK_EQ(3, m_bookctrl->GetPageCount());

        m_bookctrl->DeletePage(1);

        CHECK_EQ(1, m_bookctrl->GetPageCount());

        m_bookctrl->DeletePage(0);

        CHECK_EQ(0, m_bookctrl->GetPageCount());
    }

    wxBOOK_CTRL_BASE_TESTS();
}

#endif // wxUSE_TREEBOOK
