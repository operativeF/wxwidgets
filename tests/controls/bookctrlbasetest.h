///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/bookctrlbasetest.cpp
// Purpose:     wxBookCtrlBase unit test
// Author:      Steven Lamerton
// Created:     2010-07-02
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TESTS_CONTROLS_BOOKCTRLBASETEST_H_
#define _WX_TESTS_CONTROLS_BOOKCTRLBASETEST_H_

#ifndef WX_PRECOMP
#include "wx/app.h"
#include "wx/panel.h"
#endif // WX_PRECOMP

#include "wx/artprov.h"
#include "wx/imaglist.h"
#include "wx/bookctrl.h"
#include "wx/treebook.h"
#include "wx/toolbook.h"
#include "wx/simplebook.h"
#include "wx/listbook.h"

#include "bookctrlbasetest.h"
#include "testableframe.h"

class wxChoicebook;
class wxNotebook;
class wxSimplebook;
class wxToolbook;
class wxTreebook;
class wxListbook;

template<typename BookCtrlT>
class BookCtrlBaseT
{
public:
    wxBookCtrlBase* GetBase() { return m_bookctrl.get(); }

    wxEventType GetChangedEvent() const
    {
        if constexpr(std::is_same_v<BookCtrlT, wxChoicebook>)
        {
            return wxEVT_CHOICEBOOK_PAGE_CHANGED;
        }
        else if constexpr (std::is_same_v<BookCtrlT, wxListbook>)
        {
            return wxEVT_LISTBOOK_PAGE_CHANGED;
        }
        else if constexpr (std::is_same_v<BookCtrlT, wxNotebook>)
        {
            return wxEVT_NOTEBOOK_PAGE_CHANGED;
        }
        else if constexpr(std::is_same_v<BookCtrlT, wxSimplebook>)
        {
            return wxEVT_BOOKCTRL_PAGE_CHANGED;
        }
        else if constexpr (std::is_same_v<BookCtrlT, wxToolbook>)
        {
            return wxEVT_TOOLBOOK_PAGE_CHANGED;
        }
        else if constexpr (std::is_same_v<BookCtrlT, wxTreebook>)
        {
            return wxEVT_TREEBOOK_PAGE_CHANGED;
        }
    }

    wxEventType GetChangingEvent() const
    {
        if constexpr(std::is_same_v<BookCtrlT, wxChoicebook>)
        {
            return wxEVT_CHOICEBOOK_PAGE_CHANGING;
        }
        else if constexpr (std::is_same_v<BookCtrlT, wxListbook>)
        {
            return wxEVT_LISTBOOK_PAGE_CHANGING;
        }
        else if constexpr (std::is_same_v<BookCtrlT, wxNotebook>)
        {
            return wxEVT_NOTEBOOK_PAGE_CHANGING;
        }
        else if constexpr (std::is_same_v<BookCtrlT, wxSimplebook>)
        {
            return wxEVT_BOOKCTRL_PAGE_CHANGING;
        }
        else if constexpr (std::is_same_v<BookCtrlT, wxToolbook>)
        {
            return wxEVT_TOOLBOOK_PAGE_CHANGING;
        }
        else if constexpr (std::is_same_v<BookCtrlT, wxTreebook>)
        {
            return wxEVT_TREEBOOK_PAGE_CHANGING;
        }
    }

    // this should be inserted in the derived class test case
    // to run all wxBookCtrlBase tests as part of it
    #define wxBOOK_CTRL_BASE_TESTS() \
            SUBCASE( "Selection" ) { Selection(); } \
            SUBCASE( "Text" ) { Text(); } \
            SUBCASE( "Page Management" ) { PageManagement(); } \
            SUBCASE( "Change Events" ) { ChangeEvents(); }

    void AddPanels()
    {
        wxBookCtrlBase* const base = GetBase();

        wxSize size(32, 32);

        m_list = new wxImageList(size.x, size.y);
        m_list->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, size));
        m_list->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, size));
        m_list->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, size));

        base->AssignImageList(m_list);

        if constexpr(std::is_same_v<BookCtrlT, wxToolbook>)
        {
            m_bookctrl->GetToolBar()->Realize();
        }

        m_panel1 = new wxPanel(base);
        m_panel2 = new wxPanel(base);
        m_panel3 = new wxPanel(base);

        base->AddPage(m_panel1, "Panel 1", false, 0);
        base->AddPage(m_panel2, "Panel 2", false, 1);
        base->AddPage(m_panel3, "Panel 3", false, 2);
    }

    void Selection()
    {
        wxBookCtrlBase* const base = GetBase();

        base->SetSelection(0);

        CHECK_EQ(0, base->GetSelection());
        CHECK_EQ(wxStaticCast(m_panel1, wxWindow), base->GetCurrentPage());

        base->AdvanceSelection(false);

        CHECK_EQ(2, base->GetSelection());
        CHECK_EQ(wxStaticCast(m_panel3, wxWindow), base->GetCurrentPage());

        base->AdvanceSelection();

        CHECK_EQ(0, base->GetSelection());
        CHECK_EQ(wxStaticCast(m_panel1, wxWindow), base->GetCurrentPage());

        base->ChangeSelection(1);

        CHECK_EQ(1, base->GetSelection());
        CHECK_EQ(wxStaticCast(m_panel2, wxWindow), base->GetCurrentPage());
    }

    void Text()
    {
        wxBookCtrlBase* const base = GetBase();

        CHECK_EQ("Panel 1", base->GetPageText(0));

        base->SetPageText(1, "Some other string");

        CHECK_EQ("Some other string", base->GetPageText(1));

        base->SetPageText(2, "string with /nline break");

        CHECK_EQ("string with /nline break", base->GetPageText(2));
    }

    void PageManagement()
    {
        wxBookCtrlBase* const base = GetBase();

        base->InsertPage(0, new wxPanel(base), "New Panel", true, 0);

        if constexpr (std::is_same_v<BookCtrlT, wxToolbook>)
        {
            m_bookctrl->GetToolBar()->Realize();
        }

        CHECK_EQ(0, base->GetSelection());
        CHECK_EQ(4, base->GetPageCount());

        // Change the selection to verify that deleting a page before the currently
        // selected one correctly updates the selection.
        base->SetSelection(2);
        CHECK_EQ(2, base->GetSelection());

        base->DeletePage(1);

        CHECK_EQ(3, base->GetPageCount());
        CHECK_EQ(1, base->GetSelection());

        base->RemovePage(0);

        CHECK_EQ(2, base->GetPageCount());
        CHECK_EQ(0, base->GetSelection());

        base->DeleteAllPages();

        CHECK_EQ(0, base->GetPageCount());
        CHECK_EQ(-1, base->GetSelection());
    }

    void ChangeEvents()
    {
        wxBookCtrlBase* const base = GetBase();

        base->SetSelection(0);

        EventCounter changing(base, GetChangingEvent());
        EventCounter changed(base, GetChangedEvent());

        base->SetSelection(1);

        CHECK_EQ(1, changing.GetCount());
        CHECK_EQ(1, changed.GetCount());

        changed.Clear();
        changing.Clear();
        base->ChangeSelection(2);

        CHECK_EQ(0, changing.GetCount());
        CHECK_EQ(0, changed.GetCount());

        base->AdvanceSelection();

        CHECK_EQ(1, changing.GetCount());
        CHECK_EQ(1, changed.GetCount());

        changed.Clear();
        changing.Clear();
        base->AdvanceSelection(false);

        CHECK_EQ(1, changing.GetCount());
        CHECK_EQ(1, changed.GetCount());
    }

    void Image()
    {
        wxBookCtrlBase* const base = GetBase();

        //Check AddPanels() set things correctly
        CHECK_EQ(m_list, base->GetImageList());
        CHECK_EQ(0, base->GetPageImage(0));
        CHECK_EQ(1, base->GetPageImage(1));
        CHECK_EQ(2, base->GetPageImage(2));

        base->SetPageImage(0, 2);

        CHECK_EQ(2, base->GetPageImage(2));
    }

    std::unique_ptr<BookCtrlT> m_bookctrl;

private:
    wxPanel* m_panel1;
    wxPanel* m_panel2;
    wxPanel* m_panel3;

    wxImageList* m_list;

};

#endif // _WX_TESTS_CONTROLS_BOOKCTRLBASETEST_H_
