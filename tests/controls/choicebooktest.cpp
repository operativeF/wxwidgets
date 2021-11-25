///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/choicebooktest.cpp
// Purpose:     wxChoicebook unit test
// Author:      Steven Lamerton
// Created:     2010-07-02
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_CHOICEBOOK

#include "wx/app.h"
#include "wx/choicebk.h"

#include "bookctrlbasetest.h"

using wxChoicebookTest = BookCtrlBaseT<wxChoicebook>;

TEST_CASE_FIXTURE(wxChoicebookTest, "wxChoicebook Test")
{
    m_bookctrl = std::make_unique<wxChoicebook>(wxTheApp->GetTopWindow(), wxID_ANY);
    AddPanels();

    wxBOOK_CTRL_BASE_TESTS();

    SUBCASE("Choice")
    {
        wxChoice* choice = m_bookctrl->GetChoiceCtrl();

        CHECK(choice);
        CHECK_EQ(3, choice->GetCount());
        CHECK_EQ("Panel 1", choice->GetString(0));
    }
}

#endif //wxUSE_CHOICEBOOK
