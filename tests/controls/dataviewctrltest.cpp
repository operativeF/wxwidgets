///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/dataviewctrltest.cpp
// Purpose:     wxDataViewCtrl unit test
// Author:      Vaclav Slavik
// Created:     2011-08-08
// Copyright:   (c) 2011 Vaclav Slavik <vslavik@gmail.com>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_DATAVIEWCTRL

#include "wx/app.h"

#include "wx/dataview.h"
#ifdef __WXGTK__
import WX.Cmn.Stopwatch;
#endif // __WXGTK__

#include "testableframe.h"

import WX.Test.Prec;


// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

class DataViewCtrlTestCase
{
public:
    explicit DataViewCtrlTestCase(unsigned int style);

    DataViewCtrlTestCase(const DataViewCtrlTestCase&) = delete;
    DataViewCtrlTestCase& operator=(const DataViewCtrlTestCase&) = delete;

protected:
    void TestSelectionFor0and1();

    // the dataview control itself
    std::unique_ptr<wxDataViewTreeCtrl> m_dvc;

    // and some of its items
    wxDataViewItem m_root;
    wxDataViewItem m_child1;
    wxDataViewItem m_child2;
    wxDataViewItem m_grandchild;
};

class SingleSelectDataViewCtrlTestCase : public DataViewCtrlTestCase
{
public:
    SingleSelectDataViewCtrlTestCase()
        : DataViewCtrlTestCase(wxDV_SINGLE)
    {
    }
};

class MultiSelectDataViewCtrlTestCase : public DataViewCtrlTestCase
{
public:
    MultiSelectDataViewCtrlTestCase()
        : DataViewCtrlTestCase(wxDV_MULTIPLE)
    {
    }
};

class MultiColumnsDataViewCtrlTestCase
{
public:
    MultiColumnsDataViewCtrlTestCase();

protected:
    // the dataview control itself
    std::unique_ptr<wxDataViewListCtrl> m_dvc;

    // constants
    const wxSize m_size;
    const int m_firstColumnWidth;

    // and the columns
    wxDataViewColumn* m_firstColumn;
    wxDataViewColumn* m_lastColumn;
};

// ----------------------------------------------------------------------------
// test initialization
// ----------------------------------------------------------------------------

DataViewCtrlTestCase::DataViewCtrlTestCase(unsigned int style)
{
    m_dvc = std::make_unique<wxDataViewTreeCtrl>(wxTheApp->GetTopWindow(),
                                   wxID_ANY,
                                   wxDefaultPosition,
                                   wxSize(400, 200),
                                   style);

    m_root = m_dvc->AppendContainer(wxDataViewItem(), "The root");
      m_child1 = m_dvc->AppendContainer(m_root, "child1");
        m_grandchild = m_dvc->AppendItem(m_child1, "grandchild");
      m_child2 = m_dvc->AppendItem(m_root, "child2");

    m_dvc->Layout();
    m_dvc->Expand(m_root);
    m_dvc->Refresh();
    m_dvc->Update();
}

MultiColumnsDataViewCtrlTestCase::MultiColumnsDataViewCtrlTestCase()
    : m_size(200, 100),
      m_firstColumnWidth(50)
{
    m_dvc = std::make_unique<wxDataViewListCtrl>(wxTheApp->GetTopWindow(), wxID_ANY);

    m_firstColumn =
        m_dvc->AppendTextColumn(std::string{}, wxDataViewCellMode::Inert, m_firstColumnWidth);
    m_lastColumn =
        m_dvc->AppendTextColumn(std::string{}, wxDataViewCellMode::Inert);

    // Set size after columns appending to extend size of the last column.
    m_dvc->SetSize(m_size);
    m_dvc->Layout();
    m_dvc->Refresh();
    m_dvc->Update();
}

// ----------------------------------------------------------------------------
// the tests themselves
// ----------------------------------------------------------------------------

TEST_CASE_FIXTURE(MultiSelectDataViewCtrlTestCase,
                 "wxDVC::Selection")
{
    // Check selection round-trip.
    wxDataViewItemArray sel;
    sel.push_back(m_child1);
    sel.push_back(m_grandchild);
    REQUIRE_NOTHROW( m_dvc->SetSelections(sel) );

    wxDataViewItemArray sel2;
    CHECK( m_dvc->GetSelections(sel2) == static_cast<int>(sel.size()) );

    CHECK( sel2 == sel );

    // Invalid items in GetSelections() input are supposed to be just skipped.
    sel.clear();
    sel.push_back(wxDataViewItem());
    REQUIRE_NOTHROW( m_dvc->SetSelections(sel) );

    CHECK( m_dvc->GetSelections(sel2) == 0 );
    CHECK( sel2.empty() );
}

TEST_CASE_FIXTURE(MultiSelectDataViewCtrlTestCase,
                 "wxDVC::DeleteSelected")
{
#ifdef __WXGTK__
    std::string useASAN;
    if ( wxGetEnv("wxUSE_ASAN", &useASAN) && useASAN == "1" )
    {
        WARN("Skipping test resulting in a memory leak report with wxGTK");
        return;
    }
#endif // __WXGTK__

    wxDataViewItemArray sel;
    sel.push_back(m_child1);
    sel.push_back(m_grandchild);
    sel.push_back(m_child2);
    m_dvc->SetSelections(sel);

    // delete a selected item
    m_dvc->DeleteItem(m_child1);

    m_dvc->GetSelections(sel);

    // m_child1 and its children should be removed from the selection now
    REQUIRE( sel.size() == 1 );
    CHECK( sel[0] == m_child2 );
}

TEST_CASE_FIXTURE(MultiSelectDataViewCtrlTestCase,
                 "wxDVC::DeleteNotSelected")
{
    // TODO not working on OS X as expected
#ifdef __WXOSX__
    WARN("Disabled under MacOS because this test currently fails");
#else
    wxDataViewItemArray sel;
    sel.push_back(m_child1);
    sel.push_back(m_grandchild);
    m_dvc->SetSelections(sel);

    // delete unselected item
    m_dvc->DeleteItem(m_child2);

    m_dvc->GetSelections(sel);

    // m_child1 and its children should be unaffected
    REQUIRE( sel.size() == 2 );
    CHECK( sel[0] == m_child1 );
    CHECK( sel[1] == m_grandchild );
#endif
}

void DataViewCtrlTestCase::TestSelectionFor0and1()
{
    wxDataViewItemArray selections;

    // Initially there is no selection.
    CHECK( m_dvc->GetSelectedItemsCount() == 0 );
    CHECK( !m_dvc->HasSelection() );
    CHECK( !m_dvc->GetSelection().IsOk() );

    CHECK( !m_dvc->GetSelections(selections) );
    CHECK( selections.empty() );

    // Select one item.
    m_dvc->Select(m_child1);
    CHECK( m_dvc->GetSelectedItemsCount() == 1 );
    CHECK( m_dvc->HasSelection() );
    CHECK( m_dvc->GetSelection().IsOk() );
    REQUIRE( m_dvc->GetSelections(selections) == 1 );
    CHECK( selections[0] == m_child1 );
}

TEST_CASE_FIXTURE(MultiSelectDataViewCtrlTestCase,
                 "wxDVC::GetSelectionForMulti")
{
    wxDataViewItemArray selections;

    TestSelectionFor0and1();

    m_dvc->Select(m_child2);

    CHECK( m_dvc->GetSelectedItemsCount() == 2 );
    CHECK( m_dvc->HasSelection() );
    CHECK( !m_dvc->GetSelection().IsOk() );
    REQUIRE( m_dvc->GetSelections(selections) == 2 );
    CHECK( selections[1] == m_child2 );
}

TEST_CASE_FIXTURE(SingleSelectDataViewCtrlTestCase,
                 "wxDVC::SingleSelection")
{
    TestSelectionFor0and1();
}

TEST_CASE_FIXTURE(SingleSelectDataViewCtrlTestCase,
                 "wxDVC::IsExpanded")
{
    CHECK( m_dvc->IsExpanded(m_root) );
    CHECK( !m_dvc->IsExpanded(m_child1) );
    // No idea why, but the native NSOutlineView isItemExpanded: method returns
    // true for this item for some reason.
#ifdef __WXOSX__
    WARN("Disabled under MacOS: IsExpanded() returns true for grand child");
#else
    CHECK( !m_dvc->IsExpanded(m_grandchild) );
#endif
    CHECK( !m_dvc->IsExpanded(m_child2) );

    m_dvc->Collapse(m_root);
    CHECK( !m_dvc->IsExpanded(m_root) );

    m_dvc->ExpandChildren(m_root);
    CHECK( m_dvc->IsExpanded(m_root) );
    CHECK( m_dvc->IsExpanded(m_child1) );

    // Expanding an already expanded node must still expand all its children.
    m_dvc->Collapse(m_child1);
    CHECK( !m_dvc->IsExpanded(m_child1) );
    m_dvc->ExpandChildren(m_root);
    CHECK( m_dvc->IsExpanded(m_child1) );
}

TEST_CASE_FIXTURE(SingleSelectDataViewCtrlTestCase,
                 "wxDVC::GetItemRect")
{
#ifdef __WXGTK__
    // We need to let the native control have some events to lay itself out.
    wxYield();
#endif // __WXGTK__

    const wxRect rect1 = m_dvc->GetItemRect(m_child1);
    const wxRect rect2 = m_dvc->GetItemRect(m_child2);

    CHECK( rect1 != wxRect() );
    CHECK( rect2 != wxRect() );

    CHECK( rect1.x == rect2.x );
    CHECK( rect1.width == rect2.width );
    CHECK( rect1.height == rect2.height );

    {
        INFO("First child: " << rect1 << ", second one: " << rect2);
        CHECK( rect1.y < rect2.y );
    }

    // This forces generic implementation to add m_grandchild to the tree, as
    // it does it only on demand. We want the item to really be there to check
    // that GetItemRect() returns an empty rectangle for collapsed items.
    m_dvc->Expand(m_child1);
    m_dvc->Collapse(m_child1);

    const wxRect rectNotShown = m_dvc->GetItemRect(m_grandchild);
    CHECK( rectNotShown == wxRect() );

    // Append enough items to make the window scrollable.
    for ( int i = 3; i < 100; ++i )
        m_dvc->AppendItem(m_root, fmt::format("child%d", i));

    const wxDataViewItem last = m_dvc->AppendItem(m_root, "last");

    // This should scroll the window to bring this item into view.
    m_dvc->EnsureVisible(last);

#ifdef __WXGTK__
    // Wait for the list control to be relaid out.
    wxStopWatch sw;
    while ( m_dvc->GetTopItem() == m_root )
    {
        if ( sw.Time() > 500 )
        {
            WARN("Timed out waiting for wxDataViewCtrl layout");
            break;
        }
        wxYield();
    }
#endif // __WXGTK__

    // Check that this was indeed the case.
    const wxDataViewItem top = m_dvc->GetTopItem();
    CHECK( top != m_root );

    // Verify that the coordinates are returned in physical coordinates of the
    // window and not the logical coordinates affected by scrolling.
    const wxRect rectScrolled = m_dvc->GetItemRect(top);
    CHECK( rectScrolled.GetBottom() > 0 );
    CHECK( rectScrolled.GetTop() <= m_dvc->GetClientSize().y );

    // Also check that the root item is not currently visible (because it's
    // scrolled off).
    const wxRect rectRoot = m_dvc->GetItemRect(m_root);
    CHECK( rectRoot == wxRect() );
}

TEST_CASE_FIXTURE(SingleSelectDataViewCtrlTestCase,
                 "wxDVC::DeleteAllItems")
{
    // The invalid item corresponds to the root of tree store model, so it
    // should have a single item (our m_root) initially.
    CHECK( m_dvc->GetChildCount(wxDataViewItem()) == 1 );

    m_dvc->DeleteAllItems();

    // And none at all after deleting all the items.
    CHECK( m_dvc->GetChildCount(wxDataViewItem()) == 0 );
}

TEST_CASE_FIXTURE(MultiColumnsDataViewCtrlTestCase,
                 "wxDVC::AppendTextColumn")
{
#ifdef __WXGTK__
    // Wait for the list control to be realized.
    wxStopWatch sw;
    while ( m_firstColumn->GetWidth() == 0 )
    {
        if ( sw.Time() > 500 )
        {
            WARN("Timed out waiting for wxDataViewListCtrl to be realized");
            break;
        }
        wxYield();
    }
#endif

    // Check the width of the first column.
    CHECK( m_firstColumn->GetWidth() == m_firstColumnWidth );

    // Check that the last column was extended to fit client area.
    const int lastColumnMaxWidth =
        m_dvc->GetClientSize().GetWidth() - m_firstColumnWidth;
    // In GTK and under Mac the width of the last column is less then
    // a remaining client area.
    const int lastColumnMinWidth = lastColumnMaxWidth - 10;
    CHECK( m_lastColumn->GetWidth() <= lastColumnMaxWidth );
    CHECK( m_lastColumn->GetWidth() >= lastColumnMinWidth );
}

#endif //wxUSE_DATAVIEWCTRL
