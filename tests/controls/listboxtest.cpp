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


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/listbox.h"
#endif // WX_PRECOMP

#include "itemcontainertest.h"
#include "testableframe.h"
#include "wx/uiaction.h"

class ListBoxTestCase : public ItemContainerTestCase, public CppUnit::TestCase
{
public:
    ListBoxTestCase() { }

    void setUp() override;
    void tearDown() override;

private:
    wxItemContainer *GetContainer() const override { return m_list; }
    wxWindow *GetContainerWindow() const override { return m_list; }

    CPPUNIT_TEST_SUITE( ListBoxTestCase );
        wxITEM_CONTAINER_TESTS();
        CPPUNIT_TEST( Sort );
        CPPUNIT_TEST( MultipleSelect );
        WXUISIM_TEST( ClickEvents );
        WXUISIM_TEST( ClickNotOnItem );
        CPPUNIT_TEST( HitTest );
        //We also run all tests as an ownerdrawn list box.  We do not need to
        //run the wxITEM_CONTAINER_TESTS as they are tested with wxCheckListBox
#ifdef __WXMSW__
        CPPUNIT_TEST( PseudoTest_OwnerDrawn );
        CPPUNIT_TEST( Sort );
        CPPUNIT_TEST( MultipleSelect );
        WXUISIM_TEST( ClickEvents );
        WXUISIM_TEST( ClickNotOnItem );
        CPPUNIT_TEST( HitTest );
#endif
    CPPUNIT_TEST_SUITE_END();

    void Sort();
    void MultipleSelect();
    void ClickEvents();
    void ClickNotOnItem();
    void HitTest();
    void PseudoTest_OwnerDrawn() { ms_ownerdrawn = true; }

    static bool ms_ownerdrawn;

    wxListBox* m_list;

    ListBoxTestCase(const ListBoxTestCase&) = delete;
	ListBoxTestCase& operator=(const ListBoxTestCase&) = delete;
};

wxREGISTER_UNIT_TEST_WITH_TAGS(ListBoxTestCase,
                               "[ListBoxTestCase][item-container]");

//initialise the static variable
bool ListBoxTestCase::ms_ownerdrawn = false;

void ListBoxTestCase::setUp()
{
    if( ms_ownerdrawn )
    {
        m_list = new wxListBox(wxTheApp->GetTopWindow(), wxID_ANY,
                               wxDefaultPosition, wxSize(300, 200), {},
                               wxLB_OWNERDRAW);
    }
    else
    {
        m_list = new wxListBox(wxTheApp->GetTopWindow(), wxID_ANY,
                               wxDefaultPosition, wxSize(300, 200));
    }
}

void ListBoxTestCase::tearDown()
{
    wxDELETE(m_list);
}

void ListBoxTestCase::Sort()
{
#ifndef __WXOSX__
    wxDELETE(m_list);
    m_list = new wxListBox(wxTheApp->GetTopWindow(), wxID_ANY,
                            wxDefaultPosition, wxDefaultSize, {},
                            wxLB_SORT);

    std::vector<wxString> testitems;
    testitems.push_back("aaa");
    testitems.push_back("Aaa");
    testitems.push_back("aba");
    testitems.push_back("aaab");
    testitems.push_back("aab");
    testitems.push_back("AAA");

    m_list->Append(testitems);

    CHECK_EQ("AAA", m_list->GetString(0));
    CHECK_EQ("Aaa", m_list->GetString(1));
    CHECK_EQ("aaa", m_list->GetString(2));
    CHECK_EQ("aaab", m_list->GetString(3));
    CHECK_EQ("aab", m_list->GetString(4));
    CHECK_EQ("aba", m_list->GetString(5));

    m_list->Append("a", wxUIntToPtr(1));

    CHECK_EQ("a", m_list->GetString(0));
    CHECK_EQ(wxUIntToPtr(1), m_list->GetClientData(0));
#endif
}

void ListBoxTestCase::MultipleSelect()
{
    wxDELETE(m_list);
    m_list = new wxListBox(wxTheApp->GetTopWindow(), wxID_ANY,
                            wxDefaultPosition, wxDefaultSize, {},
                            wxLB_MULTIPLE);

    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");
    testitems.push_back("item 2");
    testitems.push_back("item 3");

    m_list->Append(testitems);

    m_list->SetSelection(0);

    wxArrayInt selected;
    m_list->GetSelections(selected);

    CHECK_EQ(1, selected.Count());
    CHECK_EQ(0, selected.Item(0));

    m_list->SetSelection(2);

    m_list->GetSelections(selected);

    CHECK_EQ(2, selected.Count());
    CHECK_EQ(2, selected.Item(1));

    m_list->Deselect(0);

    m_list->GetSelections(selected);

    CHECK_EQ(1, selected.Count());
    CHECK_EQ(2, selected.Item(0));

    CHECK(!m_list->IsSelected(0));
    CHECK(!m_list->IsSelected(1));
    CHECK(m_list->IsSelected(2));
    CHECK(!m_list->IsSelected(3));

    m_list->SetSelection(0);
    m_list->SetSelection(wxNOT_FOUND);

    m_list->GetSelections(selected);
    CHECK_EQ(0, selected.Count());
}

void ListBoxTestCase::ClickEvents()
{
#if wxUSE_UIACTIONSIMULATOR
    wxTestableFrame* frame = wxStaticCast(wxTheApp->GetTopWindow(),
                                              wxTestableFrame);

    EventCounter selected(frame, wxEVT_LISTBOX);
    EventCounter dclicked(frame, wxEVT_LISTBOX_DCLICK);

    wxUIActionSimulator sim;

    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");
    testitems.push_back("item 2");

    m_list->Append(testitems);

    m_list->Update();
    m_list->Refresh();

    sim.MouseMove(m_list->ClientToScreen(wxPoint(10, 10)));
    wxYield();

    sim.MouseClick();
    wxYield();

    CHECK_EQ(1, selected.GetCount());

    sim.MouseDblClick();
    wxYield();

    CHECK_EQ(1, dclicked.GetCount());
#endif
}

void ListBoxTestCase::ClickNotOnItem()
{
#if wxUSE_UIACTIONSIMULATOR
    wxTestableFrame* frame = wxStaticCast(wxTheApp->GetTopWindow(),
                                              wxTestableFrame);

    EventCounter selected(frame, wxEVT_LISTBOX);
    EventCounter dclicked(frame, wxEVT_LISTBOX_DCLICK);

    wxUIActionSimulator sim;

    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");
    testitems.push_back("item 2");

    m_list->Append(testitems);

    // It is important to set a valid selection: if the control doesn't have
    // any, clicking anywhere in it, even outside of any item, selects the
    // first item in the control under GTK resulting in a selection changed
    // event. This is not a wx bug, just the native platform behaviour so
    // simply avoid it by starting with a valid selection.
    m_list->SetSelection(0);

    m_list->Update();
    m_list->Refresh();

    sim.MouseMove(m_list->ClientToScreen(wxPoint(m_list->GetSize().x - 10, m_list->GetSize().y - 10)));
    wxYield();

    sim.MouseClick();
    wxYield();

    sim.MouseDblClick();
    wxYield();

    //If we are not clicking on an item we shouldn't have any events
    CHECK_EQ(0, selected.GetCount());
    CHECK_EQ(0, dclicked.GetCount());
#endif
}

void ListBoxTestCase::HitTest()
{
    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");
    testitems.push_back("item 2");

    m_list->Append(testitems);

#ifdef __WXGTK__
    // The control needs to be realized for HitTest() to work.
    wxYield();
#endif

    CHECK_EQ( 0, m_list->HitTest(5, 5) );

    CHECK_EQ( wxNOT_FOUND, m_list->HitTest(290, 190) );
}

#endif //wxUSE_LISTBOX
