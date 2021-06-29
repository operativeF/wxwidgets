///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/itemcontainertest.cpp
// Purpose:     wxItemContainer unit test
// Author:      Steven Lamerton
// Created:     2010-06-29
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/ctrlsub.h"
#endif // WX_PRECOMP

#include "wx/scopeguard.h"
#include "wx/uiaction.h"

#include "itemcontainertest.h"

void ItemContainerTestCase::Append()
{
    wxItemContainer * const container = GetContainer();

    container->Append("item 0");

    CHECK_EQ("item 0", container->GetString(0));

    std::vector<wxString> testitems;
    testitems.push_back("item 1");
    testitems.push_back("item 2");

    container->Append(testitems);

    CHECK_EQ("item 1", container->GetString(1));
    CHECK_EQ("item 2", container->GetString(2));

    //wxString arritems[] = { "item 3", "item 4" };

    //container->Append(2, arritems);

    //CHECK_EQ("item 3", container->GetString(3));
    //CHECK_EQ("item 4", container->GetString(4));
}

void ItemContainerTestCase::Insert()
{
    wxItemContainer * const container = GetContainer();

    CHECK_EQ( 0, container->Insert("item 0", 0) );
    CHECK_EQ("item 0", container->GetString(0));

    std::vector<wxString> testitems;
    testitems.push_back("item 1");
    testitems.push_back("item 2");

    CHECK_EQ( 1, container->Insert(testitems, 0) );

    CHECK_EQ("item 1", container->GetString(0));
    CHECK_EQ("item 2", container->GetString(1));

    //wxString arritems[] = { "item 3", "item 4" };

    //CHECK_EQ( 2, container->Insert(2, arritems, 1) );
    //CHECK_EQ("item 3", container->GetString(1));
    //CHECK_EQ("item 4", container->GetString(2));
}

void ItemContainerTestCase::Count()
{
    wxItemContainer * const container = GetContainer();

    CHECK(container->IsEmpty());
    WX_ASSERT_FAILS_WITH_ASSERT( container->GetString(0) );

    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");
    testitems.push_back("item 2");
    testitems.push_back("item 3");

    container->Append(testitems);

    CHECK(!container->IsEmpty());
    CHECK_EQ(4, container->GetCount());

    container->Delete(0);

    CHECK_EQ(3, container->GetCount());

    container->Delete(0);
    container->Delete(0);

    CHECK_EQ(1, container->GetCount());

    container->Insert(testitems, 1);

    CHECK_EQ(5, container->GetCount());
    WX_ASSERT_FAILS_WITH_ASSERT( container->GetString(10) );
}

void ItemContainerTestCase::ItemSelection()
{
    wxItemContainer * const container = GetContainer();

    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");
    testitems.push_back("item 2");
    testitems.push_back("ITEM 2"); // The same as the last one except for case.

    container->Append(testitems);

    container->SetSelection(wxNOT_FOUND);
    CHECK_EQ(wxNOT_FOUND, container->GetSelection());
    CHECK_EQ("", container->GetStringSelection());

    container->SetSelection(1);
    CHECK_EQ(1, container->GetSelection());
    CHECK_EQ("item 1", container->GetStringSelection());

    CHECK( container->SetStringSelection("item 2") );
    CHECK_EQ(2, container->GetSelection());
    CHECK_EQ("item 2", container->GetStringSelection());

    // Check that selecting a non-existent item fails.
    CHECK( !container->SetStringSelection("bloordyblop") );

    // Check that SetStringSelection() is case-insensitive.
    CHECK( container->SetStringSelection("ITEM 2") );
    CHECK_EQ(2, container->GetSelection());
    CHECK_EQ("item 2", container->GetStringSelection());
}

void ItemContainerTestCase::FindString()
{
   wxItemContainer * const container = GetContainer();

    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");
    testitems.push_back("item 2");
    testitems.push_back("item 3");

    container->Append(testitems);

    CHECK_EQ(1, container->FindString("item 1"));
    CHECK_EQ(1, container->FindString("ITEM 1"));
    CHECK_EQ(wxNOT_FOUND, container->FindString("ITEM 1", true));
}

void ItemContainerTestCase::ClientData()
{
    wxItemContainer * const container = GetContainer();

    wxStringClientData* item0data = new wxStringClientData("item0data");
    wxStringClientData* item1data = new wxStringClientData("item1data");
    wxStringClientData* item2data = new wxStringClientData("item2data");

    container->Append("item 0", item0data);

    CHECK_EQ(static_cast<wxClientData*>(item0data),
                         container->GetClientObject(0));

    container->Append("item 1");
    container->SetClientObject(1, item1data);

    CHECK_EQ(static_cast<wxClientData*>(item1data),
                         container->GetClientObject(1));

    container->Insert("item 2", 2, item2data);

    CHECK_EQ(static_cast<wxClientData*>(item2data),
                         container->GetClientObject(2));

    WX_ASSERT_FAILS_WITH_ASSERT( container->SetClientObject((unsigned)-1, item0data) );
    WX_ASSERT_FAILS_WITH_ASSERT( container->SetClientObject(12345, item0data) );
}

void ItemContainerTestCase::VoidData()
{
    wxItemContainer * const container = GetContainer();

    wxString item0data("item0data"), item1data("item0data"),
             item2data("item0data");

    void* item0 = &item0data;
    void* item1 = &item1data;
    void* item2 = &item2data;

    container->Append("item 0", item0);

    CHECK_EQ(item0, container->GetClientData(0));

    container->Append("item 1");
    container->SetClientData(1, item1);

    CHECK_EQ(item1, container->GetClientData(1));

    container->Insert("item 2", 2, item2);

    CHECK_EQ(item2, container->GetClientData(2));

    WX_ASSERT_FAILS_WITH_ASSERT( container->SetClientData((unsigned)-1, NULL) );
    WX_ASSERT_FAILS_WITH_ASSERT( container->SetClientData(12345, NULL) );

    // wxMSW used to hace problems retrieving the client data of -1 from a few
    // standard controls, especially if the last error was set before doing it,
    // so test for this specially.
    const wxUIntPtr minus1 = static_cast<wxUIntPtr>(-1);
    container->Append("item -1", wxUIntToPtr(minus1));

#ifdef __WINDOWS__
    ::SetLastError(ERROR_INVALID_DATA);
#endif

    CHECK_EQ( minus1, wxPtrToUInt(container->GetClientData(3)) );
}

void ItemContainerTestCase::Set()
{
    wxItemContainer * const container = GetContainer();

    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");

    container->Append(testitems);

    std::vector<wxString> newtestitems;
    newtestitems.push_back("new item 0");
    newtestitems.push_back("new item 1");
    newtestitems.push_back("new item 2");
    newtestitems.push_back("new item 3");

    container->Set(newtestitems);

    CHECK_EQ(4, container->GetCount());
    CHECK_EQ("new item 1", container->GetString(1));

    //wxString arrnewitems[] = { "even newer 0", "event newer 1" };

    //container->Set(2, arrnewitems);

    //CHECK_EQ(2, container->GetCount());
    //CHECK_EQ("even newer 0", container->GetString(0));
}

void ItemContainerTestCase::SetString()
{
   wxItemContainer * const container = GetContainer();

    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");
    testitems.push_back("item 2");
    testitems.push_back("item 3");

    container->Append(testitems);

    container->SetSelection(0);
    container->SetString(0, "new item 0");
    CHECK_EQ("new item 0", container->GetString(0));

    // Modifying the item shouldn't deselect it.
    CHECK_EQ(0, container->GetSelection());

    // wxOSX doesn't support having empty items in some containers.
#ifndef __WXOSX__
    container->SetString(2, "");
    CHECK_EQ("", container->GetString(2));
#endif
}

void ItemContainerTestCase::SelectionAfterDelete()
{
    wxItemContainer * const container = GetContainer();

    container->Append("item 0");
    container->Append("item 1");
    container->Append("item 2");
    container->Append("item 3");

    container->SetSelection(1);
    CHECK( container->GetSelection() == 1 );

    container->Delete(3);
    CHECK( container->GetSelection() == 1 );

    container->Delete(1);
    CHECK( container->GetSelection() == wxNOT_FOUND );

    container->SetSelection(1);
    container->Delete(1);
    CHECK( container->GetSelection() == wxNOT_FOUND );

    container->SetSelection(0);
    container->Delete(0);
    CHECK( container->GetSelection() == wxNOT_FOUND );
}

void ItemContainerTestCase::SetSelection()
{
    wxItemContainer * const container = GetContainer();

    container->Append("first");
    container->Append("second");

    // This class is used to check that SetSelection() doesn't generate any
    // events, as documented.
    class CommandEventHandler : public wxEvtHandler
    {
    public:
        bool ProcessEvent(wxEvent& event) override
        {
            CPPUNIT_ASSERT_MESSAGE
            (
                "unexpected command event from SetSelection",
                !event.IsCommandEvent()
            );

            return wxEvtHandler::ProcessEvent(event);
        }
    } h;

    wxWindow * const win = GetContainerWindow();
    win->PushEventHandler(&h);
    wxON_BLOCK_EXIT_OBJ1( *win, wxWindow::PopEventHandler, false );

    container->SetSelection(0);
    CHECK_EQ( 0, container->GetSelection() );

    container->SetSelection(1);
    CHECK_EQ( 1, container->GetSelection() );
}

#if wxUSE_UIACTIONSIMULATOR

void ItemContainerTestCase::SimSelect()
{
    wxItemContainer * const container = GetContainer();

    container->Append("first");
    container->Append("second");
    container->Append("third");

    GetContainerWindow()->SetFocus();

    wxUIActionSimulator sim;
    CHECK( sim.Select("third") );
    CHECK_EQ( 2, container->GetSelection() );

    CHECK( sim.Select("first") );
    CHECK_EQ( 0, container->GetSelection() );

    CHECK( !sim.Select("tenth") );
}

#endif // wxUSE_UIACTIONSIMULATOR
