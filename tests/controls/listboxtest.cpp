///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/listbox.cpp
// Purpose:     wxListBox unit test
// Author:      Steven Lamerton
// Created:     2010-06-29
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_LISTBOX


#include "wx/app.h"
#include "wx/listbox.h"

#include "itemcontainertest.h"
#include "testableframe.h"
#include "wx/uiaction.h"

using ListBoxTest = ItemContainerTest<wxListBox>;

TEST_CASE_FIXTURE(ListBoxTest, "List box control test")
{
    m_container = std::make_unique<wxListBox>(wxTheApp->GetTopWindow(), wxID_ANY,
        wxDefaultPosition, wxSize(300, 200));

#ifndef __WXOSX__
    SUBCASE("Sort")
    {
        m_container = std::make_unique<wxListBox>(wxTheApp->GetTopWindow(),
                                wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                std::vector<std::string>{}, wxLB_SORT);

        const std::vector<std::string> testitems = {
            "aaa",
            "Aaa",
            "aba",
            "aaab",
            "aab",
            "AAA"
        };

        m_container->Append(testitems);

        CHECK_EQ("AAA", m_container->GetString(0));
        CHECK_EQ("Aaa", m_container->GetString(1));
        CHECK_EQ("aaa", m_container->GetString(2));
        CHECK_EQ("aaab", m_container->GetString(3));
        CHECK_EQ("aab", m_container->GetString(4));
        CHECK_EQ("aba", m_container->GetString(5));

        m_container->Append("a", wxUIntToPtr(1));

        CHECK_EQ("a", m_container->GetString(0));
        CHECK_EQ(wxUIntToPtr(1), m_container->GetClientData(0));
    }
#endif

    SUBCASE("MultipleSelect")
    {
        m_container = std::make_unique<wxListBox>(wxTheApp->GetTopWindow(),
                                wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                std::vector<std::string>{}, wxLB_MULTIPLE);

        const std::vector<std::string> testitems = {
            "item 0",
            "item 1",
            "item 2",
            "item 3"
        };

        m_container->Append(testitems);

        m_container->SetSelection(0);

        std::vector<int> selected;
        m_container->GetSelections(selected);

        CHECK_EQ(1, selected.size());
        CHECK_EQ(0, selected[0]);

        m_container->SetSelection(2);

        m_container->GetSelections(selected);

        CHECK_EQ(2, selected.size());
        CHECK_EQ(2, selected[1]);

        m_container->Deselect(0);

        m_container->GetSelections(selected);

        CHECK_EQ(1, selected.size());
        CHECK_EQ(2, selected[0]);

        CHECK(!m_container->IsSelected(0));
        CHECK(!m_container->IsSelected(1));
        CHECK(m_container->IsSelected(2));
        CHECK(!m_container->IsSelected(3));

        m_container->SetSelection(0);
        m_container->SetSelection(wxNOT_FOUND);

        m_container->GetSelections(selected);
        CHECK_EQ(0, selected.size());
    }

#if wxUSE_UIACTIONSIMULATOR
    SUBCASE("ClickEvents")
    {
        wxTestableFrame* frame = wxStaticCast(wxTheApp->GetTopWindow(),
                                                  wxTestableFrame);

        EventCounter selected(frame, wxEVT_LISTBOX);
        EventCounter dclicked(frame, wxEVT_LISTBOX_DCLICK);

        wxUIActionSimulator sim;

        const std::vector<std::string> testitems = {
            "item 0",
            "item 1",
            "item 2"
        };

        m_container->Append(testitems);

        m_container->Update();
        m_container->Refresh();

        sim.MouseMove(m_container->ClientToScreen(wxPoint(10, 10)));
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, selected.GetCount());

        sim.MouseDblClick();
        wxYield();

        CHECK_EQ(1, dclicked.GetCount());
    }

    SUBCASE("ClickNotOnItem")
    {
        wxTestableFrame* frame = wxStaticCast(wxTheApp->GetTopWindow(),
                                                  wxTestableFrame);

        EventCounter selected(frame, wxEVT_LISTBOX);
        EventCounter dclicked(frame, wxEVT_LISTBOX_DCLICK);

        wxUIActionSimulator sim;

        const std::vector<std::string> testitems = {
            "item 0",
            "item 1",
            "item 2"
        };

        m_container->Append(testitems);

        // It is important to set a valid selection: if the control doesn't have
        // any, clicking anywhere in it, even outside of any item, selects the
        // first item in the control under GTK resulting in a selection changed
        // event. This is not a wx bug, just the native platform behaviour so
        // simply avoid it by starting with a valid selection.
        m_container->SetSelection(0);

        m_container->Update();
        m_container->Refresh();

        sim.MouseMove(m_container->ClientToScreen(wxPoint(m_container->GetSize().x - 10, m_container->GetSize().y - 10)));
        wxYield();

        sim.MouseClick();
        wxYield();

        sim.MouseDblClick();
        wxYield();

        //If we are not clicking on an item we shouldn't have any events
        CHECK_EQ(0, selected.GetCount());
        CHECK_EQ(0, dclicked.GetCount());
    }
#endif

    SUBCASE("HitTest")
    {
        const std::vector<std::string> testitems = {
            "item 0",
            "item 1",
            "item 2"
        };

        m_container->Append(testitems);

    #ifdef __WXGTK__
        // The control needs to be realized for HitTest() to work.
        wxYield();
    #endif

        CHECK_EQ( 0, m_container->HitTest(5, 5) );

        CHECK_EQ( wxNOT_FOUND, m_container->HitTest(290, 190) );
    }

    wxITEM_CONTAINER_TESTS();
}

#ifdef __WXMSW__

TEST_CASE_FIXTURE(ListBoxTest, "Owner-drawn list box test ")
{
    m_container = std::make_unique<wxListBox>(wxTheApp->GetTopWindow(), wxID_ANY,
                                              wxDefaultPosition, wxSize(300, 200),
                                              std::vector<std::string>{}, wxLB_OWNERDRAW);
    SUBCASE("Sort")
    {
        m_container = std::make_unique<wxListBox>(wxTheApp->GetTopWindow(), wxID_ANY,
            wxDefaultPosition, wxDefaultSize, std::vector<std::string>{},
            wxLB_SORT | wxLB_OWNERDRAW);

        const std::vector<std::string> testitems = {
            "aaa",
            "Aaa",
            "aba",
            "aaab",
            "aab",
            "AAA"
        };

        m_container->Append(testitems);

        CHECK_EQ("AAA", m_container->GetString(0));
        CHECK_EQ("Aaa", m_container->GetString(1));
        CHECK_EQ("aaa", m_container->GetString(2));
        CHECK_EQ("aaab", m_container->GetString(3));
        CHECK_EQ("aab", m_container->GetString(4));
        CHECK_EQ("aba", m_container->GetString(5));

        m_container->Append("a", wxUIntToPtr(1));

        CHECK_EQ("a", m_container->GetString(0));
        CHECK_EQ(wxUIntToPtr(1), m_container->GetClientData(0));
    }

    SUBCASE("MultipleSelect")
    {
        m_container = std::make_unique<wxListBox>(wxTheApp->GetTopWindow(), wxID_ANY,
            wxDefaultPosition, wxDefaultSize, std::vector<std::string>{},
            wxLB_MULTIPLE | wxLB_OWNERDRAW);

        const std::vector<std::string> testitems = {
            "item 0",
            "item 1",
            "item 2",
            "item 3"
        };

        m_container->Append(testitems);

        m_container->SetSelection(0);

        std::vector<int> selected;
        m_container->GetSelections(selected);

        CHECK_EQ(1, selected.size());
        CHECK_EQ(0, selected[0]);

        m_container->SetSelection(2);

        m_container->GetSelections(selected);

        CHECK_EQ(2, selected.size());
        CHECK_EQ(2, selected[1]);

        m_container->Deselect(0);

        m_container->GetSelections(selected);

        CHECK_EQ(1, selected.size());
        CHECK_EQ(2, selected[0]);

        CHECK(!m_container->IsSelected(0));
        CHECK(!m_container->IsSelected(1));
        CHECK(m_container->IsSelected(2));
        CHECK(!m_container->IsSelected(3));

        m_container->SetSelection(0);
        m_container->SetSelection(wxNOT_FOUND);

        m_container->GetSelections(selected);
        CHECK_EQ(0, selected.size());
    }

    SUBCASE("ClickEvents")
    {
        wxTestableFrame* frame = wxStaticCast(wxTheApp->GetTopWindow(),
            wxTestableFrame);

        EventCounter selected(frame, wxEVT_LISTBOX);
        EventCounter dclicked(frame, wxEVT_LISTBOX_DCLICK);

        wxUIActionSimulator sim;

        std::vector<std::string> testitems = {
            "item 0",
            "item 1",
            "item 2"
        };

        m_container->Append(testitems);

        m_container->Update();
        m_container->Refresh();

        sim.MouseMove(m_container->ClientToScreen(wxPoint(10, 10)));
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, selected.GetCount());

        sim.MouseDblClick();
        wxYield();

        CHECK_EQ(1, dclicked.GetCount());
    }

    SUBCASE("ClickNotOnItem")
    {
        wxTestableFrame* frame = wxStaticCast(wxTheApp->GetTopWindow(),
            wxTestableFrame);

        EventCounter selected(frame, wxEVT_LISTBOX);
        EventCounter dclicked(frame, wxEVT_LISTBOX_DCLICK);

        wxUIActionSimulator sim;

        const std::vector<std::string> testitems = {
            "item 0",
            "item 1",
            "item 2"
        };

        m_container->Append(testitems);

        // It is important to set a valid selection: if the control doesn't have
        // any, clicking anywhere in it, even outside of any item, selects the
        // first item in the control under GTK resulting in a selection changed
        // event. This is not a wx bug, just the native platform behaviour so
        // simply avoid it by starting with a valid selection.
        m_container->SetSelection(0);

        m_container->Update();
        m_container->Refresh();

        sim.MouseMove(m_container->ClientToScreen(wxPoint(m_container->GetSize().x - 10, m_container->GetSize().y - 10)));
        wxYield();

        sim.MouseClick();
        wxYield();

        sim.MouseDblClick();
        wxYield();

        //If we are not clicking on an item we shouldn't have any events
        CHECK_EQ(0, selected.GetCount());
        CHECK_EQ(0, dclicked.GetCount());
    }

    SUBCASE("HitTest")
    {
        const std::vector<std::string> testitems = {
            "item 0",
            "item 1",
            "item 2"
        };

        m_container->Append(testitems);

        CHECK_EQ(0, m_container->HitTest(5, 5));

        CHECK_EQ(wxNOT_FOUND, m_container->HitTest(290, 190));
    }

    wxITEM_CONTAINER_TESTS();
}

#endif

#endif //wxUSE_LISTBOX
