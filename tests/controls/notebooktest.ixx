///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/notebooktest.cpp
// Purpose:     wxNotebook unit test
// Author:      Steven Lamerton
// Created:     2010-07-02
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "testprec.h"

#include "wx/app.h"
#include "wx/notebook.h"
#include "wx/panel.h"

#include "bookctrlbasetest.h"
#include "testableframe.h"

export module WX.Test.Notebook;

#if wxUSE_NOTEBOOK

//void OnPageChanged(wxBookCtrlEvent&) { }

using wxNotebookTest = BookCtrlBaseT<wxNotebook>;

TEST_CASE_FIXTURE(wxNotebookTest, "wxChoicebook Test")
{
    m_bookctrl = std::make_unique<wxNotebook>(wxTheApp->GetTopWindow(), wxID_ANY,
                                              wxDefaultPosition, wxSize(400, 200));
    AddPanels();

    int m_numPageChanges{0};

    SUBCASE("RowCount")
    {
        CHECK_EQ(1, m_bookctrl->GetRowCount());

    #ifdef __WXMSW__
        m_bookctrl = std::make_unique<wxNotebook>(wxTheApp->GetTopWindow(), wxID_ANY,
                                    wxDefaultPosition, wxSize(400, 200),
                                    wxNB_MULTILINE);

        for( unsigned int i = 0; i < 10; i++ )
        {
            m_bookctrl->AddPage(new wxPanel(m_bookctrl.get()), "Panel", false, 0);
        }

        CHECK( m_bookctrl->GetRowCount() != 1 );
    #endif
    }

    // FIXME: Need to bind to R-value instead of whatever it does poorly.
    /*
    SUBCASE("NoEventsOnDestruction")
    {
        // We can't use EventCounter helper here as it doesn't deal with the window
        // it's connected to being destroyed during its life-time, so do it
        // manually.
        m_bookctrl->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED,
                         &OnPageChanged, this);

        // Normally deleting a page before the selected one results in page
        // selection changing and the corresponding event.
        m_bookctrl->DeletePage(static_cast<size_t>(0));
        CHECK( m_numPageChanges == 1 );

        // But deleting the entire control shouldn't generate any events, yet it
        // used to do under GTK+ 3 when a page different from the first one was
        // selected.
        m_bookctrl->ChangeSelection(1);
        m_bookctrl->Destroy();
        m_bookctrl.reset();
        CHECK( m_numPageChanges == 1 );
    }
    */
}

TEST_CASE("wxNotebook::AddPageEvents")
{
    wxNotebook* const
        notebook = new wxNotebook(wxTheApp->GetTopWindow(), wxID_ANY,
                                  wxDefaultPosition, wxSize(400, 200));
    std::unique_ptr<wxNotebook> cleanup(notebook);

    CHECK( notebook->GetSelection() == wxNOT_FOUND );

    EventCounter countPageChanging(notebook, wxEVT_NOTEBOOK_PAGE_CHANGING);
    EventCounter countPageChanged(notebook, wxEVT_NOTEBOOK_PAGE_CHANGED);

    // Add the first page, it is special.
    notebook->AddPage(new wxPanel(notebook), "Initial page");

    // The selection should have been changed.
    CHECK( notebook->GetSelection() == 0 );

    // But no events should have been generated.
    CHECK( countPageChanging.GetCount() == 0 );
    CHECK( countPageChanged.GetCount() == 0 );


    // Add another page without selecting it.
    notebook->AddPage(new wxPanel(notebook), "Unselected page");

    // Selection shouldn't have changed.
    CHECK( notebook->GetSelection() == 0 );

    // And no events should have been generated, of course.
    CHECK( countPageChanging.GetCount() == 0 );
    CHECK( countPageChanged.GetCount() == 0 );


    // Finally add another page and do select it.
    notebook->AddPage(new wxPanel(notebook), "Selected page", true);

    // It should have become selected.
    CHECK( notebook->GetSelection() == 2 );

    // And events for the selection change should have been generated.
    CHECK( countPageChanging.GetCount() == 1 );
    CHECK( countPageChanged.GetCount() == 1 );
}

#endif //wxUSE_NOTEBOOK
