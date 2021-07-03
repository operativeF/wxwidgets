///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/treectrltest.cpp
// Purpose:     wxTreeCtrl unit test
// Author:      Vadim Zeitlin
// Created:     2008-11-26
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
//              (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "doctest.h"

#include "testprec.h"

#if wxUSE_TREECTRL


#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif // WX_PRECOMP

#include "wx/artprov.h"
#include "wx/imaglist.h"
#include "wx/treectrl.h"
#include "wx/uiaction.h"
#include "testableframe.h"

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

/*
        CPPUNIT_TEST( HasChildren );
        CPPUNIT_TEST( SelectItemSingle );
        CPPUNIT_TEST( PseudoTest_MultiSelect );
        CPPUNIT_TEST( SelectItemMulti );
        CPPUNIT_TEST( PseudoTest_SetHiddenRoot );
        CPPUNIT_TEST( HasChildren );
        CPPUNIT_TEST( Getcount );
*/

TEST_CASE("Tree control test")
{
    auto m_tree = std::make_unique<wxTreeCtrl>(wxTheApp->GetTopWindow(),
                                          wxID_ANY,
                                          wxDefaultPosition,
                                          wxSize(400, 200),
                                          wxTR_DEFAULT_STYLE | wxTR_EDIT_LABELS);

    wxTreeItemId m_root = m_tree->AddRoot("root");
    wxTreeItemId m_child1 = m_tree->AppendItem(m_root, "child1");
    wxTreeItemId m_child2 = m_tree->AppendItem(m_root, "child2");
    wxTreeItemId m_grandchild = m_tree->AppendItem(m_child1, "grandchild");

    m_tree->SetSize(400, 200);
    m_tree->ExpandAll();
    m_tree->Refresh();
    m_tree->Update();

    SUBCASE("MultiSelectHiddenRootHasChildren")
    {
        m_tree->ToggleWindowStyle(wxTR_HIDE_ROOT | wxTR_MULTIPLE);

        CHECK( m_tree->HasChildren(m_root) );
        CHECK( m_tree->HasChildren(m_child1) );
        CHECK( !m_tree->HasChildren(m_child2) );
        CHECK( !m_tree->HasChildren(m_grandchild));

        m_tree->ToggleWindowStyle(wxTR_HIDE_ROOT | wxTR_MULTIPLE);
    }

    SUBCASE("HasChildren")
    {
        m_tree->ToggleWindowStyle(wxTR_HIDE_ROOT);

        CHECK(m_tree->HasChildren(m_root));
        CHECK(m_tree->HasChildren(m_child1));
        CHECK(!m_tree->HasChildren(m_child2));
        CHECK(!m_tree->HasChildren(m_grandchild));

        m_tree->ToggleWindowStyle(wxTR_HIDE_ROOT);
    }

    SUBCASE("Get count including root")
    {
        CHECK_EQ(4, m_tree->GetCount());
    }

    SUBCASE("Select a single item")
    {
        // this test should be only ran in single-selection control
        REQUIRE( !m_tree->HasFlag(wxTR_MULTIPLE) );

        // initially nothing is selected
        CHECK( !m_tree->IsSelected(m_child1) );

        // selecting an item should make it selected
        m_tree->SelectItem(m_child1);
        CHECK( m_tree->IsSelected(m_child1) );

        // selecting it again shouldn't change anything
        m_tree->SelectItem(m_child1);
        CHECK( m_tree->IsSelected(m_child1) );

        // selecting another item should switch the selection to it
        m_tree->SelectItem(m_child2);
        CHECK( !m_tree->IsSelected(m_child1) );
        CHECK( m_tree->IsSelected(m_child2) );

        // selecting it again still shouldn't change anything
        m_tree->SelectItem(m_child2);
        CHECK( !m_tree->IsSelected(m_child1) );
        CHECK( m_tree->IsSelected(m_child2) );

        // deselecting an item should remove the selection entirely
        m_tree->UnselectItem(m_child2);
        CHECK( !m_tree->IsSelected(m_child1) );
        CHECK( !m_tree->IsSelected(m_child2) );
    }

    SUBCASE("SelectItemMulti")
    {
        m_tree->ToggleWindowStyle(wxTR_MULTIPLE);

        // this test should be only ran in multi-selection control
        REQUIRE(m_tree->HasFlag(wxTR_MULTIPLE));

        // initially nothing is selected
        CHECK( !m_tree->IsSelected(m_child1) );

        // selecting an item should make it selected
        m_tree->SelectItem(m_child1);
        CHECK( m_tree->IsSelected(m_child1) );

        // selecting it again shouldn't change anything
        m_tree->SelectItem(m_child1);
        CHECK( m_tree->IsSelected(m_child1) );

        // selecting another item shouldn't deselect the previously selected one
        m_tree->SelectItem(m_child2);
        CHECK( m_tree->IsSelected(m_child1) );
        CHECK( m_tree->IsSelected(m_child2) );

        // selecting it again still shouldn't change anything
        m_tree->SelectItem(m_child2);
        CHECK( m_tree->IsSelected(m_child1) );
        CHECK( m_tree->IsSelected(m_child2) );

        // deselecting one of the items should leave the others selected
        m_tree->UnselectItem(m_child1);
        CHECK( !m_tree->IsSelected(m_child1) );
        CHECK( m_tree->IsSelected(m_child2) );

        // collapsing a branch with selected items should still leave them selected
        m_tree->Expand(m_child1);
        m_tree->SelectItem(m_grandchild);
        CHECK( m_tree->IsSelected(m_grandchild) );
        m_tree->Collapse(m_child1);
        CHECK( m_tree->IsSelected(m_grandchild) );
        m_tree->Expand(m_child1);
        CHECK( m_tree->IsSelected(m_grandchild) );

        m_tree->ToggleWindowStyle(wxTR_MULTIPLE);
    }

    SUBCASE("ItemClick")
    {
    #if wxUSE_UIACTIONSIMULATOR
        EventCounter activated(m_tree.get(), wxEVT_TREE_ITEM_ACTIVATED);
        EventCounter rclick(m_tree.get(), wxEVT_TREE_ITEM_RIGHT_CLICK);

        wxUIActionSimulator sim;

        wxRect pos;
        m_tree->GetBoundingRect(m_child1, pos, true);

        // We move in slightly so we are not on the edge
        wxPoint point = m_tree->ClientToScreen(pos.GetPosition()) + wxPoint(4, 4);

        sim.MouseMove(point);
        wxYield();

        sim.MouseDblClick();
        wxYield();

        sim.MouseClick(wxMOUSE_BTN_RIGHT);
        wxYield();

        CHECK_EQ(1, activated.GetCount());
        CHECK_EQ(1, rclick.GetCount());
    #endif // wxUSE_UIACTIONSIMULATOR
    }

    SUBCASE("DeleteItem")
    {
        EventCounter deleteitem(m_tree.get(), wxEVT_TREE_DELETE_ITEM);

        wxTreeItemId todelete = m_tree->AppendItem(m_root, "deleteme");
        m_tree->AppendItem(todelete, "deleteme2");
        m_tree->Delete(todelete);

        CHECK_EQ(2, deleteitem.GetCount());
    }

    SUBCASE("DeleteChildren")
    {
        EventCounter deletechildren(m_tree.get(), wxEVT_TREE_DELETE_ITEM);

        m_tree->AppendItem(m_child1, "another grandchild");
        m_tree->DeleteChildren(m_child1);

        CHECK( deletechildren.GetCount() == 2 );
    }

    SUBCASE("DeleteAllItems")
    {
        EventCounter deleteall(m_tree.get(), wxEVT_TREE_DELETE_ITEM);

        m_tree->DeleteAllItems();

        CHECK( deleteall.GetCount() == 4 );
    }

#if wxUSE_UIACTIONSIMULATOR

    SUBCASE("LabelEdit")
    {
        EventCounter beginedit(m_tree.get(), wxEVT_TREE_BEGIN_LABEL_EDIT);
        EventCounter endedit(m_tree.get(), wxEVT_TREE_END_LABEL_EDIT);

        wxUIActionSimulator sim;

        m_tree->SetFocusedItem(m_tree->GetRootItem());
        m_tree->EditLabel(m_tree->GetRootItem());

        sim.Text("newroottext");
        wxYield();

        CHECK_EQ(1, beginedit.GetCount());

        sim.Char(WXK_RETURN);
        wxYield();

        CHECK_EQ(1, endedit.GetCount());
    }

    SUBCASE("KeyDown")
    {
        EventCounter keydown(m_tree.get(), wxEVT_TREE_KEY_DOWN);

        wxUIActionSimulator sim;

        m_tree->SetFocus();
        sim.Text("aAbB");
        wxYield();

        CHECK_EQ(6, keydown.GetCount());
    }

    SUBCASE("CollapseExpandEvents")
    {
    #ifdef __WXGTK20__
        // Works locally, but not when run on Travis CI.
        if ( IsAutomaticTest() )
            return;
    #endif

        m_tree->CollapseAll();

        EventCounter collapsed(m_tree.get(), wxEVT_TREE_ITEM_COLLAPSED);
        EventCounter collapsing(m_tree.get(), wxEVT_TREE_ITEM_COLLAPSING);
        EventCounter expanded(m_tree.get(), wxEVT_TREE_ITEM_EXPANDED);
        EventCounter expanding(m_tree.get(), wxEVT_TREE_ITEM_EXPANDING);

        wxUIActionSimulator sim;

        wxRect pos;
        m_tree->GetBoundingRect(m_root, pos, true);

        // We move in slightly so we are not on the edge
        wxPoint point = m_tree->ClientToScreen(pos.GetPosition()) + wxPoint(4, 4);

        sim.MouseMove(point);
        wxYield();

        sim.MouseDblClick();
        wxYield();

        CHECK_EQ(1, expanding.GetCount());
        CHECK_EQ(1, expanded.GetCount());

    #ifdef __WXGTK__
        // Don't even know the reason why, but GTK has to sleep
        // no less than 1200 for the test case to succeed.
        wxMilliSleep(1200);
    #endif

        sim.MouseDblClick();
        wxYield();

        CHECK_EQ(1, collapsing.GetCount());
        CHECK_EQ(1, collapsed.GetCount());
    }

    SUBCASE("SelectionChange")
    {
        m_tree->ExpandAll();

        // This is currently needed to work around a problem under wxMSW: clicking
        // on an item in an unfocused control generates two selection change events
        // because of the SetFocus() call in TVN_SELCHANGED handler in wxMSW code.
        // This is, of course, wrong on its own, but fixing it without breaking
        // anything else is non-obvious, so for now at least work around this
        // problem in the test.
        m_tree->SetFocus();

        EventCounter changed(m_tree.get(), wxEVT_TREE_SEL_CHANGED);
        EventCounter changing(m_tree.get(), wxEVT_TREE_SEL_CHANGING);

        wxUIActionSimulator sim;

        wxRect poschild1, poschild2;
        m_tree->GetBoundingRect(m_child1, poschild1, true);
        m_tree->GetBoundingRect(m_child2, poschild2, true);

        // We move in slightly so we are not on the edge
        wxPoint point1 = m_tree->ClientToScreen(poschild1.GetPosition()) + wxPoint(4, 4);
        wxPoint point2 = m_tree->ClientToScreen(poschild2.GetPosition()) + wxPoint(4, 4);

        sim.MouseMove(point1);
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, changed.GetCount());
        CHECK_EQ(1, changing.GetCount());

        sim.MouseMove(point2);
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(2, changed.GetCount());
        CHECK_EQ(2, changing.GetCount());
    }

    SUBCASE("Menu")
    {
        EventCounter menu(m_tree.get(), wxEVT_TREE_ITEM_MENU);
        wxUIActionSimulator sim;

        wxRect pos;
        m_tree->GetBoundingRect(m_child1, pos, true);

        // We move in slightly so we are not on the edge
        wxPoint point = m_tree->ClientToScreen(pos.GetPosition()) + wxPoint(4, 4);

        sim.MouseMove(point);
        wxYield();

        sim.MouseClick(wxMOUSE_BTN_RIGHT);
        wxYield();

        CHECK_EQ(1, menu.GetCount());
    }

#endif // wxUSE_UIACTIONSIMULATOR

    SUBCASE("ItemData")
    {
        wxTreeItemData* child1data = new wxTreeItemData();
        wxTreeItemData* appenddata = new wxTreeItemData();
        wxTreeItemData* insertdata = new wxTreeItemData();

        m_tree->SetItemData(m_child1, child1data);

        CHECK_EQ(child1data, m_tree->GetItemData(m_child1));
        CHECK_EQ(m_child1, child1data->GetId());

        wxTreeItemId append = m_tree->AppendItem(m_root, "new", -1, -1, appenddata);

        CHECK_EQ(appenddata, m_tree->GetItemData(append));
        CHECK_EQ(append, appenddata->GetId());

        wxTreeItemId insert = m_tree->InsertItem(m_root, m_child1, "new", -1, -1,
                                                 insertdata);

        CHECK_EQ(insertdata, m_tree->GetItemData(insert));
        CHECK_EQ(insert, insertdata->GetId());
    }

    SUBCASE("Iteration")
    {
        // Get first / next / last child
        wxTreeItemIdValue cookie;
        CHECK_EQ(m_tree->GetFirstChild(m_root, cookie), m_child1);
        CHECK_EQ(m_tree->GetNextChild(m_root, cookie),
                             m_tree->GetLastChild(m_root));
        CHECK_EQ(m_child2, m_tree->GetLastChild(m_root));

        // Get next / previous sibling
        CHECK_EQ(m_child2, m_tree->GetNextSibling(m_child1));
        CHECK_EQ(m_child1, m_tree->GetPrevSibling(m_child2));
    }

    SUBCASE("Parent")
    {
        CHECK_EQ(m_root, m_tree->GetRootItem());
        CHECK_EQ(m_root, m_tree->GetItemParent(m_child1));
        CHECK_EQ(m_root, m_tree->GetItemParent(m_child2));
        CHECK_EQ(m_child1, m_tree->GetItemParent(m_grandchild));
    }

    SUBCASE("CollapseExpand")
    {
        m_tree->ExpandAll();

        CHECK(m_tree->IsExpanded(m_root));
        CHECK(m_tree->IsExpanded(m_child1));

        m_tree->CollapseAll();

        CHECK(!m_tree->IsExpanded(m_root));
        CHECK(!m_tree->IsExpanded(m_child1));

        m_tree->ExpandAllChildren(m_root);

        CHECK(m_tree->IsExpanded(m_root));
        CHECK(m_tree->IsExpanded(m_child1));

        m_tree->CollapseAllChildren(m_child1);

        CHECK(!m_tree->IsExpanded(m_child1));

        m_tree->Expand(m_child1);

        CHECK(m_tree->IsExpanded(m_child1));

        m_tree->Collapse(m_root);

        CHECK(!m_tree->IsExpanded(m_root));
        CHECK(m_tree->IsExpanded(m_child1));

        m_tree->CollapseAndReset(m_root);

        CHECK(!m_tree->IsExpanded(m_root));
    }

    SUBCASE("AssignImageList")
    {
        wxSize size(16, 16);

        wxImageList *imagelist = new wxImageList(size.x, size.y);
        imagelist->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, size));

        wxImageList *statelist = new wxImageList(size.x, size.y);
        statelist->Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, size));

        m_tree->AssignImageList(imagelist);
        m_tree->AssignStateImageList(statelist);

        CHECK_EQ(imagelist, m_tree->GetImageList());
        CHECK_EQ(statelist, m_tree->GetStateImageList());
    }

    SUBCASE("Focus")
    {
        m_tree->SetFocusedItem(m_child1);

        CHECK_EQ(m_child1, m_tree->GetFocusedItem());

        m_tree->ClearFocusedItem();

        CHECK(!m_tree->GetFocusedItem());
    }

    SUBCASE("Bold")
    {
        CHECK(!m_tree->IsBold(m_child1));

        m_tree->SetItemBold(m_child1);

        CHECK(m_tree->IsBold(m_child1));

        m_tree->SetItemBold(m_child1, false);

        CHECK(!m_tree->IsBold(m_child1));
    }

    SUBCASE("Visible")
    {
        m_tree->CollapseAll();

        CHECK(m_tree->IsVisible(m_root));
        CHECK(!m_tree->IsVisible(m_child1));

        m_tree->EnsureVisible(m_grandchild);

        CHECK(m_tree->IsVisible(m_grandchild));

        m_tree->ExpandAll();

        CHECK_EQ(m_root, m_tree->GetFirstVisibleItem());
        CHECK_EQ(m_child1, m_tree->GetNextVisible(m_root));
        CHECK_EQ(m_grandchild, m_tree->GetNextVisible(m_child1));
        CHECK_EQ(m_child2, m_tree->GetNextVisible(m_grandchild));

        CHECK(!m_tree->GetNextVisible(m_child2));
        CHECK(!m_tree->GetPrevVisible(m_root));
    }

    SUBCASE("Sort")
    {
        wxTreeItemId zitem = m_tree->AppendItem(m_root, "zzzz");
        wxTreeItemId aitem = m_tree->AppendItem(m_root, "aaaa");

        m_tree->SortChildren(m_root);

        wxTreeItemIdValue cookie;

        CHECK_EQ(aitem, m_tree->GetFirstChild(m_root, cookie));
        CHECK_EQ(m_child1, m_tree->GetNextChild(m_root, cookie));
        CHECK_EQ(m_child2, m_tree->GetNextChild(m_root, cookie));
        CHECK_EQ(zitem, m_tree->GetNextChild(m_root, cookie));
    }

    SUBCASE("KeyNavigation")
    {
    #if wxUSE_UIACTIONSIMULATOR
        wxUIActionSimulator sim;

        m_tree->CollapseAll();

        m_tree->SelectItem(m_root);
        wxYield();

        m_tree->SetFocus();
        sim.Char(WXK_RIGHT);
        wxYield();

        CHECK(m_tree->IsExpanded(m_root));

    #ifdef wxHAS_GENERIC_TREECTRL
        sim.Char('-');
    #else
        sim.Char(WXK_LEFT);
    #endif

        wxYield();

        CHECK(!m_tree->IsExpanded(m_root));

        wxYield();

        sim.Char(WXK_RIGHT);
        sim.Char(WXK_DOWN);
        wxYield();

        CHECK_EQ(m_child1, m_tree->GetSelection());

        sim.Char(WXK_DOWN);
        wxYield();

        CHECK_EQ(m_child2, m_tree->GetSelection());
    #endif
    }

}

#endif //wxUSE_TREECTRL
