///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/itemcontainertest.h
// Purpose:     wxItemContainer unit test
// Author:      Steven Lamerton
// Created:     2010-06-29
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TESTS_CONTROLS_ITEMCONTAINERTEST_H_
#define _WX_TESTS_CONTROLS_ITEMCONTAINERTEST_H_

#include "doctest.h"

#include "testprec.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/ctrlsub.h"
#endif // WX_PRECOMP

#include "wx/ctrlsub.h"
#include "wx/scopeguard.h"
#include "wx/uiaction.h"

#include <memory>

template<typename ItemContT>
class ItemContainerTest
{
public:
    std::unique_ptr<ItemContT> m_container;

    // this function must be overridden by the derived classes to return the
    // text entry object we're testing, typically this is done by creating a
    // control implementing wxItemContainer interface in setUp() virtual method and
    // just returning it from here
    wxItemContainer* GetContainer() { return m_container.get(); }

    // and this one must be overridden to return the window which implements
    // wxItemContainer interface -- usually it will return the same pointer as
    // GetContainer(), just as a different type
    wxWindow* GetContainerWindow() const { return m_container.get(); }

protected:
    void AppendTest()
    {
        m_container->Append("item 0");

        CHECK_EQ("item 0", m_container->GetString(0));

        std::vector<wxString> testitems;
        testitems.push_back("item 1");
        testitems.push_back("item 2");

        m_container->Append(testitems);

        CHECK_EQ("item 1", m_container->GetString(1));
        CHECK_EQ("item 2", m_container->GetString(2));

        //wxString arritems[] = { "item 3", "item 4" };

        //m_container->Append(2, arritems);

        //CHECK_EQ("item 3", m_container->GetString(3));
        //CHECK_EQ("item 4", m_container->GetString(4));
    }

    void InsertTest()
    {
        auto pos_0 = m_container->Insert("item 0", 0);
        CHECK_EQ( 0, pos_0 );
        CHECK_EQ("item 0", m_container->GetString(0));

        std::vector<wxString> testitems;
        testitems.push_back("item 1");
        testitems.push_back("item 2");

        auto pos_1 = m_container->Insert(testitems, 0);
        CHECK_EQ( 1, pos_1 );

        CHECK_EQ("item 1", m_container->GetString(0));
        CHECK_EQ("item 2", m_container->GetString(1));

        //wxString arritems[] = { "item 3", "item 4" };

        //CHECK_EQ( 2, m_container->Insert(2, arritems, 1) );
        //CHECK_EQ("item 3", m_container->GetString(1));
        //CHECK_EQ("item 4", m_container->GetString(2));
    }

    void CountTest()
    {
        CHECK(m_container->wxItemContainerImmutable::IsEmpty());
        WX_ASSERT_FAILS_WITH_ASSERT( m_container->GetString(0) );

        std::vector<wxString> testitems;
        testitems.push_back("item 0");
        testitems.push_back("item 1");
        testitems.push_back("item 2");
        testitems.push_back("item 3");

        m_container->Append(testitems);

        CHECK(!m_container->wxItemContainerImmutable::IsEmpty());
        CHECK_EQ(4, m_container->GetCount());

        m_container->Delete(0);

        CHECK_EQ(3, m_container->GetCount());

        m_container->Delete(0);
        m_container->Delete(0);

        CHECK_EQ(1, m_container->GetCount());

        m_container->Insert(testitems, 1);

        CHECK_EQ(5, m_container->GetCount());
        WX_ASSERT_FAILS_WITH_ASSERT( m_container->GetString(10) );
    }

    void ItemSelectionTest()
    {
        std::vector<wxString> testitems;
        testitems.push_back("item 0");
        testitems.push_back("item 1");
        testitems.push_back("item 2");
        testitems.push_back("ITEM 2"); // The same as the last one except for case.

        m_container->Append(testitems);

        m_container->SetSelection(wxNOT_FOUND);
        CHECK_EQ(wxNOT_FOUND, m_container->GetSelection());
        CHECK_EQ("", m_container->wxItemContainer::GetStringSelection());

        m_container->SetSelection(1);
        CHECK_EQ(1, m_container->GetSelection());
        CHECK_EQ("item 1", m_container->wxItemContainer::GetStringSelection());

        CHECK( m_container->SetStringSelection("item 2") );
        CHECK_EQ(2, m_container->GetSelection());
        CHECK_EQ("item 2", m_container->wxItemContainer::GetStringSelection());

        // Check that selecting a non-existent item fails.
        CHECK( !m_container->SetStringSelection("bloordyblop") );

        // Check that SetStringSelection() is case-insensitive.
        CHECK( m_container->SetStringSelection("ITEM 2") );
        CHECK_EQ(2, m_container->GetSelection());
        CHECK_EQ("item 2", m_container->wxItemContainer::GetStringSelection());
    }

    void FindStringTest()
    {
        std::vector<wxString> testitems;
        testitems.push_back("item 0");
        testitems.push_back("item 1");
        testitems.push_back("item 2");
        testitems.push_back("item 3");

        m_container->Append(testitems);

        CHECK_EQ(1, m_container->FindString("item 1"));
        CHECK_EQ(1, m_container->FindString("ITEM 1"));
        CHECK_EQ(wxNOT_FOUND, m_container->FindString("ITEM 1", true));
    }

    void ClientDataTest()
    {
        wxStringClientData* item0data = new wxStringClientData("item0data");
        wxStringClientData* item1data = new wxStringClientData("item1data");
        wxStringClientData* item2data = new wxStringClientData("item2data");

        m_container->Append("item 0", item0data);

        CHECK_EQ(static_cast<wxClientData*>(item0data),
                            m_container->GetClientObject(0));

        m_container->Append("item 1");
        m_container->SetClientObject(1, item1data);

        CHECK_EQ(static_cast<wxClientData*>(item1data),
                            m_container->GetClientObject(1));

        m_container->Insert("item 2", 2, item2data);

        CHECK_EQ(static_cast<wxClientData*>(item2data),
                            m_container->GetClientObject(2));

        WX_ASSERT_FAILS_WITH_ASSERT( m_container->SetClientObject((unsigned)-1, item0data) );
        WX_ASSERT_FAILS_WITH_ASSERT( m_container->SetClientObject(12345, item0data) );
    }

    void VoidDataTest()
    {
        wxString item0data("item0data"), item1data("item0data"),
                item2data("item0data");

        void* item0 = &item0data;
        void* item1 = &item1data;
        void* item2 = &item2data;

        m_container->Append("item 0", item0);

        CHECK_EQ(item0, m_container->GetClientData(0));

        m_container->Append("item 1");
        m_container->SetClientData(1, item1);

        CHECK_EQ(item1, m_container->GetClientData(1));

        m_container->Insert("item 2", 2, item2);

        CHECK_EQ(item2, m_container->GetClientData(2));

        WX_ASSERT_FAILS_WITH_ASSERT( m_container->SetClientData((unsigned)-1, NULL) );
        WX_ASSERT_FAILS_WITH_ASSERT( m_container->SetClientData(12345, NULL) );

        // wxMSW used to hace problems retrieving the client data of -1 from a few
        // standard controls, especially if the last error was set before doing it,
        // so test for this specially.
        const wxUIntPtr minus1 = static_cast<wxUIntPtr>(-1);
        m_container->Append("item -1", wxUIntToPtr(minus1));

    #ifdef __WINDOWS__
        ::SetLastError(ERROR_INVALID_DATA);
    #endif

        CHECK_EQ( minus1, wxPtrToUInt(m_container->GetClientData(3)) );
    }

    void SetTest()
    {
        std::vector<wxString> testitems = {
            "item 0",
            "item 1"
        };

        m_container->Append(testitems);

        std::vector<wxString> newtestitems;
        newtestitems.push_back("new item 0");
        newtestitems.push_back("new item 1");
        newtestitems.push_back("new item 2");
        newtestitems.push_back("new item 3");

        m_container->Set(newtestitems);

        CHECK_EQ(4, m_container->GetCount());
        CHECK_EQ("new item 1", m_container->GetString(1));

        //wxString arrnewitems[] = { "even newer 0", "event newer 1" };

        //m_container->Set(2, arrnewitems);

        //CHECK_EQ(2, m_container->GetCount());
        //CHECK_EQ("even newer 0", m_container->GetString(0));
    }

    void SetStringTest()
    {
        std::vector<wxString> testitems;
        testitems.push_back("item 0");
        testitems.push_back("item 1");
        testitems.push_back("item 2");
        testitems.push_back("item 3");

        m_container->Append(testitems);

        m_container->SetSelection(0);
        m_container->SetString(0, "new item 0");
        CHECK_EQ("new item 0", m_container->GetString(0));

        // Modifying the item shouldn't deselect it.
        CHECK_EQ(0, m_container->GetSelection());

        // wxOSX doesn't support having empty items in some containers.
    #ifndef __WXOSX__
        m_container->SetString(2, "");
        CHECK_EQ("", m_container->GetString(2));
    #endif
    }

    void SelectionAfterDeleteTest()
    {
        m_container->Append("item 0");
        m_container->Append("item 1");
        m_container->Append("item 2");
        m_container->Append("item 3");

        m_container->SetSelection(1);
        CHECK( m_container->GetSelection() == 1 );

        m_container->Delete(3);
        CHECK( m_container->GetSelection() == 1 );

        m_container->Delete(1);
        CHECK( m_container->GetSelection() == wxNOT_FOUND );

        m_container->SetSelection(1);
        m_container->Delete(1);
        CHECK( m_container->GetSelection() == wxNOT_FOUND );

        m_container->SetSelection(0);
        m_container->Delete(0);
        CHECK( m_container->GetSelection() == wxNOT_FOUND );
    }

    void SetSelectionTest()
    {
        m_container->Append("first");
        m_container->Append("second");

        // This class is used to check that SetSelection() doesn't generate any
        // events, as documented.
        class CommandEventHandler : public wxEvtHandler
        {
        public:
            bool ProcessEvent(wxEvent& event) override
            {
                CHECK_MESSAGE
                (
                    !event.IsCommandEvent(),
                    "unexpected command event from SetSelection"
                );

                return wxEvtHandler::ProcessEvent(event);
            }
        } h;

        wxWindow * const win = GetContainerWindow();
        win->PushEventHandler(&h);
        wxON_BLOCK_EXIT_OBJ1( *win, wxWindow::PopEventHandler, false );

        m_container->SetSelection(0);
        CHECK_EQ( 0, m_container->GetSelection() );

        m_container->SetSelection(1);
        CHECK_EQ( 1, m_container->GetSelection() );
    }

    #if wxUSE_UIACTIONSIMULATOR

    void SimSelectTest()
    {
        m_container->Append("first");
        m_container->Append("second");
        m_container->Append("third");

        GetContainerWindow()->SetFocus();

        wxUIActionSimulator sim;
        CHECK( sim.Select("third") );
        CHECK_EQ( 2, m_container->GetSelection() );

        CHECK( sim.Select("first") );
        CHECK_EQ( 0, m_container->GetSelection() );

        CHECK( !sim.Select("tenth") );
    }

    #endif // wxUSE_UIACTIONSIMULATOR

    // this should be inserted in the derived class CPPUNIT_TEST_SUITE
    // definition to run all wxItemContainer tests as part of it
    #define wxITEM_CONTAINER_TESTS() \
            SUBCASE( "AppendTest" ) { AppendTest(); } \
            SUBCASE( "InsertTest" ) { InsertTest(); } \
            SUBCASE( "CountTest" ) { CountTest(); } \
            SUBCASE( "ItemSelectionTest" ) { ItemSelectionTest(); } \
            SUBCASE( "FindStringTest" ) { FindStringTest(); } \
            SUBCASE( "ClientDataTest" ) { ClientDataTest(); } \
            SUBCASE( "VoidDataTest" ) { VoidDataTest(); } \
            SUBCASE( "SetTest" ) { SetTest(); } \
            SUBCASE( "SetSelectionTest" ) { SetSelectionTest(); } \
            SUBCASE( "SetStringTest" ) { SetStringTest(); } \
            SUBCASE( "SelectionAfterDeleteTest" ) { SelectionAfterDeleteTest(); } \
            SUBCASE( "SimSelectTest" ) { SimSelectTest(); }
};

#endif // _WX_TESTS_CONTROLS_ITEMCONTAINERTEST_H_
