///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/listbasetest.cpp
// Purpose:     Base class for wxListCtrl and wxListView tests
// Author:      Steven Lamerton
// Created:     2010-07-20
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>,
//              (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TESTS_CONTROLS_LISTBASETEST_H_
#define _WX_TESTS_CONTROLS_LISTBASETEST_H_

#include "wx/app.h"
#include "wx/uiaction.h"
#include "wx/artprov.h"
#include "wx/listctrl.h"
#include "wx/imaglist.h"

#include "listbasetest.h"
#include "testableframe.h"

#include <fmt/core.h>

import WX.Test.Prec;

template<typename ListCtrlT>
struct ListBaseTest
{
    wxListCtrl* GetList() { return m_listctrl.get(); }

    void ColumnsOrder()
    {
#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
        wxListCtrl* const list = GetList();

        int n;
        wxListItem li;
        li.SetMask(ListMaskFlags{ListMasks::Text});

        // first set up some columns
        static const int NUM_COLS = 3;

        list->InsertColumn(0, "Column 0");
        list->InsertColumn(1, "Column 1");
        list->InsertColumn(2, "Column 2");

        // and a couple of test items too
        list->InsertItem(0, "Item 0");
        list->SetItem(0, 1, "first in first");

        list->InsertItem(1, "Item 1");
        list->SetItem(1, 2, "second in second");


        // check that the order is natural in the beginning
        const auto orderOrig = list->GetColumnsOrder();
        for (n = 0; n < NUM_COLS; n++)
            CHECK_EQ(n, orderOrig[n]);

        // then rearrange them: using { 2, 0, 1 } order means that column 2 is
        // shown first, then column 0 and finally column 1
        std::vector<int> order{2, 0, 1};

        list->SetColumnsOrder(order);

        // check that we get back the same order as we set
        const auto orderNew = list->GetColumnsOrder();
        for (n = 0; n < NUM_COLS; n++)
            CHECK_EQ(order[n], orderNew[n]);

        // and the order -> index mappings for individual columns
        for (n = 0; n < NUM_COLS; n++)
            CHECK_EQ(order[n], list->GetColumnIndexFromOrder(n));

        // and also the reverse mapping
        CHECK_EQ(1, list->GetColumnOrder(0));
        CHECK_EQ(2, list->GetColumnOrder(1));
        CHECK_EQ(0, list->GetColumnOrder(2));


        // finally check that accessors still use indices, not order
        CHECK(list->GetColumn(0, li));
        CHECK_EQ("Column 0", li.GetText());

        li.SetId(0);
        li.SetColumn(1);
        CHECK(list->GetItem(li));
        CHECK_EQ("first in first", li.GetText());

        li.SetId(1);
        li.SetColumn(2);
        CHECK(list->GetItem(li));
        CHECK_EQ("second in second", li.GetText());
#endif // wxHAS_LISTCTRL_COLUMN_ORDER
    }

    void ItemRect()
    {
        wxListCtrl* const list = GetList();

        // set up for the test
        list->InsertColumn(0, "Column 0", wxListColumnFormat::Left, 60);
        list->InsertColumn(1, "Column 1", wxListColumnFormat::Left, 50);
        list->InsertColumn(2, "Column 2", wxListColumnFormat::Left, 40);

        list->InsertItem(0, "Item 0");
        list->SetItem(0, 1, "first column");
        list->SetItem(0, 1, "second column");

        // do test
        wxRect r;
        CHECK_THROWS(list->GetItemRect(1, r));
        CHECK(list->GetItemRect(0, r));
        CHECK_EQ(150, r.GetWidth());

        CHECK(list->GetSubItemRect(0, 0, r));
        CHECK_EQ(60, r.GetWidth());

        CHECK(list->GetSubItemRect(0, 1, r));
        CHECK_EQ(50, r.GetWidth());

        CHECK(list->GetSubItemRect(0, 2, r));
        CHECK_EQ(40, r.GetWidth());

        CHECK_THROWS(list->GetSubItemRect(0, 3, r));


        // As we have a header, the top item shouldn't be at (0, 0), but somewhere
        // below the header.
        //
        // Notice that we consider that the header can't be less than 10 pixels
        // because we don't know its exact height.
        CHECK(list->GetItemRect(0, r));
        CHECK(r.y >= 10);

        // However if we remove the header now, the item should be at (0, 0).
        list->SetWindowStyle(wxLC_REPORT | wxLC_NO_HEADER);
        CHECK(list->GetItemRect(0, r));
        CHECK_EQ(0, r.y);
    }

    void ItemText()
    {
        wxListCtrl* const list = GetList();

        list->InsertColumn(0, "First");
        list->InsertColumn(1, "Second");

        list->InsertItem(0, "0,0");
        CHECK_EQ("0,0", list->GetItemText(0));
        CHECK_EQ("", list->GetItemText(0, 1));

        list->SetItem(0, 1, "0,1");
        CHECK_EQ("0,1", list->GetItemText(0, 1));
    }

    void ChangeMode()
    {
        wxListCtrl* const list = GetList();

        list->InsertColumn(0, "Header");
        list->InsertItem(0, "First");
        list->InsertItem(1, "Second");
        CHECK_EQ(2, list->GetItemCount());

        // check that switching the mode preserves the items
        list->SetWindowStyle(wxLC_ICON);
        CHECK_EQ(2, list->GetItemCount());
        CHECK_EQ("First", list->GetItemText(0));

        // and so does switching back
        list->SetWindowStyle(wxLC_REPORT);
        CHECK_EQ(2, list->GetItemCount());
        CHECK_EQ("First", list->GetItemText(0));
    }

    void MultiSelect()
    {
#if wxUSE_UIACTIONSIMULATOR

#ifndef __WXMSW__
        // FIXME: This test fails on Travis CI although works fine on
        //        development machine, no idea why though!
        if (IsAutomaticTest())
            return;
#endif // !__WXMSW__

        wxListCtrl* const list = GetList();

        EventCounter focused(list, wxEVT_LIST_ITEM_FOCUSED);
        EventCounter selected(list, wxEVT_LIST_ITEM_SELECTED);
        EventCounter deselected(list, wxEVT_LIST_ITEM_DESELECTED);

        list->InsertColumn(0, "Header");

        for (int i = 0; i < 10; ++i)
            list->InsertItem(i, fmt::format("Item {:d}", i));

        wxUIActionSimulator sim;

        wxRect pos;
        list->GetItemRect(2, pos); // Choose the third item as anchor

        // We move in slightly so we are not on the edge
        wxPoint point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10);

        sim.MouseMove(point);
        wxYield();

        sim.MouseClick(); // select the anchor
        wxYield();

        list->GetItemRect(5, pos);
        point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10);

        sim.MouseMove(point);
        wxYield();

        sim.KeyDown(WXK_SHIFT);
        sim.MouseClick();
        sim.KeyUp(WXK_SHIFT);
        wxYield();

        // when the first item was selected the focus changes to it, but not
        // on subsequent clicks
        CHECK_EQ(4, list->GetSelectedItemCount()); // item 2 to 5 (inclusive) are selected
        CHECK_EQ(2, focused.GetCount()); // count the focus which was on the anchor
        CHECK_EQ(4, selected.GetCount());
        CHECK_EQ(0, deselected.GetCount());

        focused.Clear();
        selected.Clear();
        deselected.Clear();

        sim.Char(WXK_END, wxMOD_SHIFT); // extend the selection to the last item
        wxYield();

        CHECK_EQ(8, list->GetSelectedItemCount()); // item 2 to 9 (inclusive) are selected
        CHECK_EQ(1, focused.GetCount()); // focus is on the last item
        CHECK_EQ(4, selected.GetCount()); // only newly selected items got the event
        CHECK_EQ(0, deselected.GetCount());

        focused.Clear();
        selected.Clear();
        deselected.Clear();

        sim.Char(WXK_HOME, wxMOD_SHIFT); // select from anchor to the first item
        wxYield();

        CHECK_EQ(3, list->GetSelectedItemCount()); // item 0 to 2 (inclusive) are selected
        CHECK_EQ(1, focused.GetCount()); // focus is on item 0
        CHECK_EQ(2, selected.GetCount()); // events are only generated for item 0 and 1
        CHECK_EQ(7, deselected.GetCount()); // item 2 (exclusive) to 9 are deselected

        focused.Clear();
        selected.Clear();
        deselected.Clear();

        list->EnsureVisible(0);
        wxYield();

        list->GetItemRect(2, pos);
        point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10);

        sim.MouseMove(point);
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, list->GetSelectedItemCount()); // anchor is the only selected item
        CHECK_EQ(1, focused.GetCount()); // because the focus changed from item 0 to anchor
        CHECK_EQ(0, selected.GetCount()); // anchor is already in selection state
        CHECK_EQ(2, deselected.GetCount()); // items 0 and 1 are deselected
#endif // wxUSE_UIACTIONSIMULATOR
    }

    void ItemClick()
    {
#if wxUSE_UIACTIONSIMULATOR

#ifdef __WXMSW__
        // FIXME: This test fails on MSW buildbot slaves although works fine on
        //        development machine, no idea why. It seems to be a problem with
        //        wxUIActionSimulator rather the wxListCtrl control itself however.
        if (IsAutomaticTest())
            return;
#endif // __WXMSW__

        wxListCtrl* const list = GetList();

        list->InsertColumn(0, "Column 0", wxListColumnFormat::Left, 60);
        list->InsertColumn(1, "Column 1", wxListColumnFormat::Left, 50);
        list->InsertColumn(2, "Column 2", wxListColumnFormat::Left, 40);

        list->InsertItem(0, "Item 0");
        list->SetItem(0, 1, "first column");
        list->SetItem(0, 2, "second column");

        EventCounter selected(list, wxEVT_LIST_ITEM_SELECTED);
        EventCounter focused(list, wxEVT_LIST_ITEM_FOCUSED);
        EventCounter activated(list, wxEVT_LIST_ITEM_ACTIVATED);
        EventCounter rclick(list, wxEVT_LIST_ITEM_RIGHT_CLICK);
        EventCounter deselected(list, wxEVT_LIST_ITEM_DESELECTED);

        wxUIActionSimulator sim;

        wxRect pos;
        list->GetItemRect(0, pos);

        //We move in slightly so we are not on the edge
        wxPoint point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10);

        sim.MouseMove(point);
        wxYield();

        sim.MouseClick();
        wxYield();

        sim.MouseDblClick();
        wxYield();

        sim.MouseClick(wxMOUSE_BTN_RIGHT);
        wxYield();

        // We want a point within the listctrl but below any items
        point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 50);

        sim.MouseMove(point);
        wxYield();

        sim.MouseClick();
        wxYield();

        // when the first item was selected the focus changes to it, but not
        // on subsequent clicks
        CHECK_EQ(1, focused.GetCount());
        CHECK_EQ(1, selected.GetCount());
        CHECK_EQ(1, deselected.GetCount());
        CHECK_EQ(1, activated.GetCount());
        CHECK_EQ(1, rclick.GetCount());
#endif // wxUSE_UIACTIONSIMULATOR
    }

    void KeyDown()
    {
#if wxUSE_UIACTIONSIMULATOR
        wxListCtrl* const list = GetList();

        EventCounter keydown(list, wxEVT_LIST_KEY_DOWN);

        wxUIActionSimulator sim;

        list->SetFocus();
        wxYield();
        sim.Text("aAbB"); // 4 letters + 2 shift mods.
        wxYield();

        CHECK_EQ(6, keydown.GetCount());
#endif
    }

    void DeleteItems()
    {
#ifndef __WXOSX__
        wxListCtrl* const list = GetList();

        EventCounter deleteitem(list, wxEVT_LIST_DELETE_ITEM);
        EventCounter deleteall(list, wxEVT_LIST_DELETE_ALL_ITEMS);


        list->InsertColumn(0, "Column 0", wxListColumnFormat::Left, 60);
        list->InsertColumn(1, "Column 1", wxListColumnFormat::Left, 50);
        list->InsertColumn(2, "Column 2", wxListColumnFormat::Left, 40);

        list->InsertItem(0, "Item 0");
        list->InsertItem(1, "Item 1");
        list->InsertItem(2, "Item 1");

        list->DeleteItem(0);
        list->DeleteItem(0);
        list->DeleteAllItems();

        //Add some new items to tests ClearAll with
        list->InsertColumn(0, "Column 0");
        list->InsertItem(0, "Item 0");
        list->InsertItem(1, "Item 1");

        //Check that ClearAll actually sends a DELETE_ALL_ITEMS event
        list->ClearAll();

        //ClearAll and DeleteAllItems shouldn't send an event if there was nothing
        //to clear
        list->ClearAll();
        list->DeleteAllItems();

        CHECK_EQ(2, deleteitem.GetCount());
        CHECK_EQ(2, deleteall.GetCount());
#endif
    }

    void InsertItem()
    {
        wxListCtrl* const list = GetList();

        EventCounter insert(list, wxEVT_LIST_INSERT_ITEM);

        list->InsertColumn(0, "Column 0", wxListColumnFormat::Left, 60);

        wxListItem item;
        item.SetId(0);
        item.SetText("some text");

        list->InsertItem(item);
        list->InsertItem(1, "more text");

        CHECK_EQ(2, insert.GetCount());
    }

    void Find()
    {
        wxListCtrl* const list = GetList();

        // set up for the test
        list->InsertColumn(0, "Column 0");
        list->InsertColumn(1, "Column 1");

        list->InsertItem(0, "Item 0");
        list->SetItem(0, 1, "first column");

        list->InsertItem(1, "Item 1");
        list->SetItem(1, 1, "first column");

        list->InsertItem(2, "Item 40");
        list->SetItem(2, 1, "first column");

        list->InsertItem(3, "ITEM 01");
        list->SetItem(3, 1, "first column");

        CHECK_EQ(1, list->FindItem(-1, "Item 1"));
        CHECK_EQ(2, list->FindItem(-1, "Item 4", true));
        CHECK_EQ(2, list->FindItem(1, "Item 40"));
        CHECK_EQ(3, list->FindItem(2, "Item 0", true));
    }

    void Visible()
    {
        wxListCtrl* const list = GetList();

        list->InsertColumn(0, "Column 0");

        int count = list->GetCountPerPage();

        for (int i = 0; i < count + 10; i++)
        {
            list->InsertItem(i, fmt::format("string {:d}", i));
        }

        CHECK_EQ(count + 10, list->GetItemCount());
        CHECK_EQ(0, list->GetTopItem());

        list->EnsureVisible(count + 9);

        CHECK(list->GetTopItem() != 0);
    }

    void ItemFormatting()
    {
        wxListCtrl* const list = GetList();

        list->InsertColumn(0, "Column 0");

        list->InsertItem(0, "Item 0");
        list->InsertItem(1, "Item 1");
        list->InsertItem(2, "Item 2");

        list->SetTextColour(*wxYELLOW);
        list->SetBackgroundColour(*wxGREEN);
        list->SetItemTextColour(0, *wxRED);
        list->SetItemBackgroundColour(1, *wxBLUE);

        CHECK_EQ(*wxGREEN, list->GetBackgroundColour());
        CHECK_EQ(*wxBLUE, list->GetItemBackgroundColour(1));

        CHECK_EQ(*wxYELLOW, list->GetTextColour());
        CHECK_EQ(*wxRED, list->GetItemTextColour(0));
    }

    void EditLabel()
    {
#if wxUSE_UIACTIONSIMULATOR
        wxListCtrl* const list = GetList();

        list->SetWindowStyleFlag(wxLC_REPORT | wxLC_EDIT_LABELS);

        list->InsertColumn(0, "Column 0");

        list->InsertItem(0, "Item 0");
        list->InsertItem(1, "Item 1");

        EventCounter beginedit(list, wxEVT_LIST_BEGIN_LABEL_EDIT);
        EventCounter endedit(list, wxEVT_LIST_END_LABEL_EDIT);

        wxUIActionSimulator sim;

        list->EditLabel(0);
        wxYield();

        sim.Text("sometext");
        wxYield();

        sim.Char(WXK_RETURN);

        wxYield();

        CHECK_EQ(1, beginedit.GetCount());
        CHECK_EQ(1, endedit.GetCount());
#endif
    }

    void ImageList()
    {
        wxListCtrl* const list = GetList();

        wxSize size(32, 32);

        wxImageList* imglist = new wxImageList(size.x, size.y);
        imglist->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, size));
        imglist->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, size));
        imglist->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, size));

        list->AssignImageList(imglist, wxIMAGE_LIST_NORMAL);

        CHECK_EQ(imglist, list->GetImageList(wxIMAGE_LIST_NORMAL));
    }

    void HitTest()
    {
#ifdef __WXMSW__ // ..until proven to work with other platforms
        wxListCtrl* const list = GetList();
        list->SetWindowStyle(wxLC_REPORT);

        // set small image list
        wxSize size(16, 16);
        wxImageList* m_imglistSmall = new wxImageList(size.x, size.y);
        m_imglistSmall->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_LIST, size));
        list->AssignImageList(m_imglistSmall, wxIMAGE_LIST_SMALL);

        // insert 2 columns
        list->InsertColumn(0, "Column 0");
        list->InsertColumn(1, "Column 1");

        // and a couple of test items too
        list->InsertItem(0, "Item 0", 0);
        list->SetItem(0, 1, "0, 1");

        list->InsertItem(1, "Item 1", 0);

        // enable checkboxes to test state icon
        list->EnableCheckBoxes();

        // get coordinates
        wxRect rectSubItem0, rectIcon;
        list->GetSubItemRect(0, 0, rectSubItem0); // column 0
        list->GetItemRect(0, rectIcon, wxListRectFlags::Icon); // icon
        int y = rectSubItem0.GetTop() + (rectSubItem0.GetBottom() -
            rectSubItem0.GetTop()) / 2;

        ListHitTestFlags flags{};

        // state icon (checkbox)
        int xCheckBox = rectSubItem0.GetLeft() + (rectIcon.GetLeft() -
            rectSubItem0.GetLeft()) / 2;
        list->HitTest(wxPoint(xCheckBox, y), flags);
        CHECK_MESSAGE((ListHitTest::OnItemStateIcon & flags),
                      "Expected ListHitTest::OnItemStateIcon");

        // icon
        int xIcon = rectIcon.GetLeft() + (rectIcon.GetRight() - rectIcon.GetLeft()) / 2;
        list->HitTest(wxPoint(xIcon, y), flags);
        CHECK_MESSAGE((ListHitTest::OnItemIcon & flags),
                      "Expected ListHitTest::OnItemIcon");

        // label, beyond column 0
        wxRect rectItem;
        list->GetItemRect(0, rectItem); // entire item
        int xHit = rectSubItem0.GetRight() + (rectItem.GetRight() - rectSubItem0.GetRight()) / 2;
        list->HitTest(wxPoint(xHit, y), flags);
        CHECK_MESSAGE((ListHitTest::OnItemLabel & flags),
            "Expected ListHitTest::OnItemLabel");
#endif // __WXMSW__
    }

    //From the sample but fixed so it actually inverts
    static int wxCALLBACK
        MyCompareFunction(wxIntPtr item1, wxIntPtr item2, [[maybe_unused]] wxIntPtr sortData)
    {
        // inverse the order
        if (item1 < item2)
            return 1;
        if (item1 > item2)
            return -1;

        return 0;
    }


    void Sort()
    {
        wxListCtrl* const list = GetList();

        list->InsertColumn(0, "Column 0");

        list->InsertItem(0, "Item 0");
        list->SetItemData(0, 0);
        list->InsertItem(1, "Item 1");
        list->SetItemData(1, 1);

        list->SortItems(&MyCompareFunction, 0);

        CHECK_EQ("Item 1", list->GetItemText(0));
        CHECK_EQ("Item 0", list->GetItemText(1));
    }

    #define wxLIST_BASE_TESTS() \
            SUBCASE( "ColumnsOrder" ) { ColumnsOrder(); } \
            SUBCASE( "ItemRect" ) { ItemRect(); } \
            SUBCASE( "ItemText" ) { ItemText(); } \
            SUBCASE( "ChangeMode" ) { ChangeMode(); } \
            SUBCASE( "ItemClick" ) { ItemClick(); } \
            SUBCASE( "KeyDown" ) { KeyDown(); } \
            SUBCASE( "MultiSelect" ) { MultiSelect(); } \
            SUBCASE( "DeleteItems" ) { DeleteItems(); } \
            SUBCASE( "InsertItem" ) { InsertItem(); } \
            SUBCASE( "Find" ) { Find(); } \
            SUBCASE( "Visible" ) { Visible(); } \
            SUBCASE( "ItemFormatting" ) { ItemFormatting(); } \
            SUBCASE( "EditLabel" ) { EditLabel(); } \
            SUBCASE( "ImageList" ) { ImageList(); } \
            SUBCASE( "HitTest" ) { HitTest(); } \
            SUBCASE( "Sort" ) { Sort(); }

    std::unique_ptr<ListCtrlT> m_listctrl;
};

#endif
