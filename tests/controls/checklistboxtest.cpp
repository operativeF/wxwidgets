///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/checklistlistbox.cpp
// Purpose:     wxCheckListBox unit test
// Author:      Steven Lamerton
// Created:     2010-06-30
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_CHECKLISTBOX


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/checklst.h"
#endif // WX_PRECOMP

#include "itemcontainertest.h"
#include "testableframe.h"


TEST_CASE("Checklist box")
{
    auto m_check = new wxCheckListBox(wxTheApp->GetTopWindow(), wxID_ANY);

    EventCounter toggled(m_check, wxEVT_CHECKLISTBOX);

    std::vector<int> checkedItems;
    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");
    testitems.push_back("item 2");
    testitems.push_back("item 3");

    m_check->Append(testitems);

    m_check->Check(0);
    m_check->Check(1);
    m_check->Check(1, false);

    //We should not get any events when changing this from code
    CHECK_EQ(0, toggled.GetCount());
    CHECK_EQ(true, m_check->IsChecked(0));
    CHECK_EQ(false, m_check->IsChecked(1));

    // FIXME: No conversion from wxArrayInt to std::vector<unsigned int>
    CHECK_EQ(1, m_check->GetCheckedItemsCount(checkedItems));
    CHECK_EQ(0, checkedItems[0]);

    //Make sure a double check of an items doesn't deselect it
    m_check->Check(0);

    CHECK_EQ(true, m_check->IsChecked(0));

    wxDELETE(m_check);
}

#endif // wxUSE_CHECKLISTBOX
