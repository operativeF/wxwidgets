///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/checklistlistbox.cpp
// Purpose:     wxCheckListBox unit test
// Author:      Steven Lamerton
// Created:     2010-06-30
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_CHECKLISTBOX

#include "wx/app.h"
#include "wx/checklst.h"

#include "itemcontainertest.h"
#include "testableframe.h"

import WX.Test.Prec;

using wxChecklistBoxTest = ItemContainerTest<wxCheckListBox>;

TEST_CASE_FIXTURE(wxChecklistBoxTest, "Checklist box test")
{
    m_container = std::make_unique<wxCheckListBox>(wxTheApp->GetTopWindow(), wxID_ANY);

    SUBCASE("Check")
    {
        EventCounter toggled(m_container.get(), wxEVT_CHECKLISTBOX);

        const std::vector<std::string> testitems = {
            "item 0",
            "item 1",
            "item 2",
            "item 3"
        };

        m_container->Append(testitems);

        m_container->Check(0);
        m_container->Check(1);
        m_container->Check(1, false);

        //We should not get any events when changing this from code
        CHECK_EQ(0, toggled.GetCount());
        CHECK_EQ(true, m_container->IsChecked(0));
        CHECK_EQ(false, m_container->IsChecked(1));

        std::vector<int> checkedItems;

        CHECK_EQ(1, m_container->GetCheckedItemsCount(checkedItems));
        CHECK_EQ(0, checkedItems[0]);

        //Make sure a double check of an items doesn't deselect it
        m_container->Check(0);

        CHECK_EQ(true, m_container->IsChecked(0));
    }

    wxITEM_CONTAINER_TESTS();
}

#endif // wxUSE_CHECKLISTBOX
