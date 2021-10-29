///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/listctrltest.cpp
// Purpose:     wxListCtrl unit test
// Author:      Vadim Zeitlin
// Created:     2008-11-26
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
//              (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_LISTCTRL

#include "wx/app.h"

#include "wx/listctrl.h"
#include "wx/artprov.h"
#include "wx/imaglist.h"
#include "listbasetest.h"
#include "testableframe.h"
#include "wx/uiaction.h"


using ListCtrlTest = ListBaseTest<wxListCtrl>;

TEST_CASE_FIXTURE(ListCtrlTest, "List control test")
{
    m_listctrl = std::make_unique<wxListCtrl>(wxTheApp->GetTopWindow());
    m_listctrl->SetWindowStyle(wxLC_REPORT);
    m_listctrl->SetSize(wxSize{400, 200});

    SUBCASE("EditLabel")
    {
        m_listctrl->InsertColumn(0, "Column 0");
        m_listctrl->InsertItem(0, "foo");
        m_listctrl->EditLabel(0);
    }

    SUBCASE("SubitemRect")
    {
        wxBitmap bmp = wxArtProvider::GetBitmap(wxART_ERROR);

        wxImageList* const iml = new wxImageList(bmp.GetWidth(), bmp.GetHeight());
        iml->Add(bmp);
        m_listctrl->AssignImageList(iml, wxIMAGE_LIST_SMALL);

        m_listctrl->InsertColumn(0, "Column 0");
        m_listctrl->InsertColumn(1, "Column 1");
        m_listctrl->InsertColumn(2, "Column 2");
        for ( int i = 0; i < 3; i++ )
        {
            long index = m_listctrl->InsertItem(i, wxString::Format("This is item %d", i), 0);
            m_listctrl->SetItem(index, 1, wxString::Format("Column 1 item %d", i));
            m_listctrl->SetItem(index, 2, wxString::Format("Column 2 item %d", i));
        }

        wxRect rectLabel, rectIcon, rectItem;

        // First check a subitem with an icon: it should have a valid icon
        // rectangle and the label rectangle should be adjacent to it.
        m_listctrl->GetSubItemRect(1, 0, rectItem, wxListRectFlags::Bounds);
        m_listctrl->GetSubItemRect(1, 0, rectIcon, wxListRectFlags::Icon);
        m_listctrl->GetSubItemRect(1, 0, rectLabel, wxListRectFlags::Label);

        CHECK(!rectIcon.IsEmpty());
        // Note that we can't use "==" here, in the native MSW version there is a
        // gap between the item rectangle and the icon one.
        CHECK(rectIcon.GetLeft() >= rectItem.GetLeft());
        CHECK(rectLabel.GetLeft() == rectIcon.GetRight() + 1);
        CHECK(rectLabel.GetRight() == rectItem.GetRight());

        // For a subitem without an icon, label rectangle is the same one as the
        // entire item one and the icon rectangle should be empty.
        m_listctrl->GetSubItemRect(1, 1, rectItem, wxListRectFlags::Bounds);
        m_listctrl->GetSubItemRect(1, 1, rectIcon, wxListRectFlags::Icon);
        m_listctrl->GetSubItemRect(1, 1, rectLabel, wxListRectFlags::Label);

        CHECK(rectIcon.IsEmpty());
        // Here we can't check for exact equality neither as there can be a margin.
        CHECK(rectLabel.GetLeft() >= rectItem.GetLeft());
        CHECK(rectLabel.GetRight() == rectItem.GetRight());
    }

#if wxUSE_UIACTIONSIMULATOR
    SUBCASE("ColumnDrag")
    {
        EventCounter begindrag(m_listctrl.get(), wxEVT_LIST_COL_BEGIN_DRAG);
        EventCounter dragging(m_listctrl.get(), wxEVT_LIST_COL_DRAGGING);
        EventCounter enddrag(m_listctrl.get(), wxEVT_LIST_COL_END_DRAG);

        m_listctrl->InsertColumn(0, "Column 0");
        m_listctrl->InsertColumn(1, "Column 1");
        m_listctrl->InsertColumn(2, "Column 2");
        m_listctrl->Update();
        m_listctrl->SetFocus();

        wxUIActionSimulator sim;

        wxPoint pt = m_listctrl->ClientToScreen(wxPoint(m_listctrl->GetColumnWidth(0), 5));

        sim.MouseMove(pt);
        wxYield();

        sim.MouseDown();
        wxYield();

        sim.MouseMove(pt.x + 50, pt.y);
        wxYield();

        sim.MouseUp();
        wxYield();

        CHECK_EQ(1, begindrag.GetCount());
        CHECK(dragging.GetCount() > 0);
        CHECK_EQ(1, enddrag.GetCount());

        m_listctrl->ClearAll();
    }

    SUBCASE("ColumnClick")
    {
        EventCounter colclick(m_listctrl.get(), wxEVT_LIST_COL_CLICK);
        EventCounter colrclick(m_listctrl.get(), wxEVT_LIST_COL_RIGHT_CLICK);


        m_listctrl->InsertColumn(0, "Column 0", wxListColumnFormat::Left, 60);

        wxUIActionSimulator sim;

        sim.MouseMove(m_listctrl->ClientToScreen(wxPoint(4, 4)));
        wxYield();

        sim.MouseClick();
        sim.MouseClick(wxMOUSE_BTN_RIGHT);
        wxYield();

        CHECK_EQ(1, colclick.GetCount());
        CHECK_EQ(1, colrclick.GetCount());

        m_listctrl->ClearAll();
    }
#endif // wxUSE_UIACTIONSIMULATOR
}

#endif // wxUSE_LISTCTRL
