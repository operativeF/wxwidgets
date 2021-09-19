///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/treelistctrltest.cpp
// Purpose:     wxTreeListCtrl unit test.
// Author:      Vadim Zeitlin
// Created:     2011-08-27
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_TREELISTCTRL


#include "wx/treelist.h"

#include "wx/app.h"

struct TreeListCtrlTest
{
    TreeListCtrlTest()
    {
        m_treelist = std::make_unique<wxTreeListCtrl>(wxTheApp->GetTopWindow(),
            wxID_ANY,
            wxDefaultPosition,
            wxSize(400, 200),
            wxTL_MULTIPLE | wxTL_3STATE);

        m_treelist->AppendColumn("Component");
        m_treelist->AppendColumn("# Files");
        m_treelist->AppendColumn("Size");

        // Fill the control with the same data as used in the treelist sample:
        m_code = AddItem("Code");
        AddItem("wxMSW", m_code, "313", "3.94 MiB");
        AddItem("wxGTK", m_code, "180", "1.66 MiB");

        m_code_osx = AddItem("wxOSX", m_code, "265", "2.36 MiB");
        AddItem("Core", m_code_osx, "31", "347 KiB");
        AddItem("Carbon", m_code_osx, "91", "1.34 MiB");
        m_code_osx_cocoa = AddItem("Cocoa", m_code_osx, "46", "512 KiB");

        wxTreeListItem Documentation = AddItem("Documentation");
        AddItem("HTML", Documentation, "many");
        AddItem("CHM", Documentation, "1");

        wxTreeListItem Samples = AddItem("Samples");
        AddItem("minimal", Samples, "1", "7 KiB");
        AddItem("widgets", Samples, "28", "419 KiB");

        m_treelist->Refresh();
        m_treelist->Update();
    }

    wxTreeListItem AddItem(const char* label,
        wxTreeListItem parent = wxTreeListItem(),
        const char* numFiles = "",
        const char* size = "")
    {
        if (!parent.IsOk())
            parent = m_treelist->GetRootItem();

        wxTreeListItem item = m_treelist->AppendItem(parent, label);
        m_treelist->SetItemText(item, 1, numFiles);
        m_treelist->SetItemText(item, 2, size);

        ++m_numItems;

        return item;
    }

    std::unique_ptr<wxTreeListCtrl> m_treelist;
    wxTreeListItem m_code;
    wxTreeListItem m_code_osx;
    wxTreeListItem m_code_osx_cocoa;
    size_t m_numItems{ 0 };
};


TEST_CASE_FIXTURE(TreeListCtrlTest, "Tree list control test")
{
    // Test various tree traversal methods.
    SUBCASE("Traversal")
    {
        // GetParent() tests:
        wxTreeListItem root = m_treelist->GetRootItem();
        CHECK( !m_treelist->GetItemParent(root) );

        CHECK_EQ( root, m_treelist->GetItemParent(m_code) );
        CHECK_EQ( m_code, m_treelist->GetItemParent(m_code_osx) );


        // wxGetFirstChild() and wxGetNextSibling() tests:
        CHECK_EQ( m_code, m_treelist->wxGetFirstChild(root) );
        CHECK_EQ
        (
            m_code_osx,
            m_treelist->wxGetNextSibling
            (
                m_treelist->wxGetNextSibling
                (
                    m_treelist->wxGetFirstChild(m_code)
                )
            )
        );

        // Get{First,Next}Item() test:
        unsigned numItems = 0;
        for ( wxTreeListItem item = m_treelist->GetFirstItem();
              item.IsOk();
              item = m_treelist->GetNextItem(item) )
        {
            numItems++;
        }

        CHECK_EQ( m_numItems, numItems );
    }

    // Test accessing items text.
    SUBCASE("ItemText")
    {
        CHECK_EQ( "Cocoa", m_treelist->GetItemText(m_code_osx_cocoa) );
        CHECK_EQ( "46", m_treelist->GetItemText(m_code_osx_cocoa, 1) );

        m_treelist->SetItemText(m_code_osx_cocoa, "wxCocoa");
        CHECK_EQ( "wxCocoa", m_treelist->GetItemText(m_code_osx_cocoa) );

        m_treelist->SetItemText(m_code_osx_cocoa, 1, "47");
        CHECK_EQ( "47", m_treelist->GetItemText(m_code_osx_cocoa, 1) );
    }

    // Test checking and unchecking items.
    SUBCASE("ItemCheck")
    {
        CHECK_EQ( wxCHK_UNCHECKED,
                              m_treelist->GetCheckedState(m_code) );

        m_treelist->CheckItemRecursively(m_code);
        CHECK_EQ( wxCHK_CHECKED,
                              m_treelist->GetCheckedState(m_code) );
        CHECK_EQ( wxCHK_CHECKED,
                              m_treelist->GetCheckedState(m_code_osx) );
        CHECK_EQ( wxCHK_CHECKED,
                              m_treelist->GetCheckedState(m_code_osx_cocoa) );

        m_treelist->UncheckItem(m_code_osx_cocoa);
        CHECK_EQ( wxCHK_UNCHECKED,
                              m_treelist->GetCheckedState(m_code_osx_cocoa) );

        m_treelist->UpdateItemParentStateRecursively(m_code_osx_cocoa);
        CHECK_EQ( wxCHK_UNDETERMINED,
                              m_treelist->GetCheckedState(m_code_osx) );
        CHECK_EQ( wxCHK_UNDETERMINED,
                              m_treelist->GetCheckedState(m_code) );

        m_treelist->CheckItemRecursively(m_code_osx, wxCHK_UNCHECKED);
        m_treelist->UpdateItemParentStateRecursively(m_code_osx_cocoa);
        CHECK_EQ( wxCHK_UNCHECKED,
                              m_treelist->GetCheckedState(m_code_osx) );
        CHECK_EQ( wxCHK_UNDETERMINED,
                              m_treelist->GetCheckedState(m_code) );
    }
}

#endif // wxUSE_TREELISTCTRL
