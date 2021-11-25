///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/ownerdrawncomboboxtest.cpp
// Purpose:     OwnerDrawnComboBox unit test
// Author:      Jaakko Salli
// Created:     2010-12-17
// Copyright:   (c) 2010 Jaakko Salli
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_ODCOMBOBOX


#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif // WX_PRECOMP

#include "wx/odcombo.h"

#include "textentrytest.h"
#include "itemcontainertest.h"
#include "testableframe.h"

// ----------------------------------------------------------------------------
// tests themselves
// ----------------------------------------------------------------------------

using OwnerDrawnComboBoxTest = ItemContainerTest<wxOwnerDrawnComboBox>;

TEST_CASE_FIXTURE(OwnerDrawnComboBoxTest, "Owner-drawn combo box test")
{
    m_container = std::make_unique<wxOwnerDrawnComboBox>(wxTheApp->GetTopWindow(), wxID_ANY);

    SUBCASE("Size")
    {
        // under MSW changing combobox size is a non-trivial operation because of
        // confusion between the size of the control with and without dropdown, so
        // check that it does work as expected

        const int heightOrig = m_container->GetSize().y;

        // check that the height doesn't change if we don't touch it
        m_container->SetSize(wxSize{100, -1});
        CHECK_EQ( heightOrig, m_container->GetSize().y );

        // check that setting both big and small (but not too small, there is a
        // limit on how small the control can become under MSW) heights works
        m_container->SetSize(wxSize{-1, 50});
        CHECK_EQ( 50, m_container->GetSize().y );

        m_container->SetSize(wxSize{-1, 10});
        CHECK_EQ( 10, m_container->GetSize().y );

        // and also that restoring it works (this used to be broken before 2.9.1)
        m_container->SetSize(wxSize{-1, heightOrig});
        CHECK_EQ( heightOrig, m_container->GetSize().y );
    }

    SUBCASE("PopDismiss")
    {
        EventCounter drop(m_container.get(), wxEVT_COMBOBOX_DROPDOWN);
        EventCounter close(m_container.get(), wxEVT_COMBOBOX_CLOSEUP);

        m_container->Popup();
        m_container->Dismiss();

        CHECK_EQ(1, drop.GetCount());
        CHECK_EQ(1, close.GetCount());
    }

    SUBCASE("Sort")
    {
        m_container = std::make_unique<wxOwnerDrawnComboBox>(wxTheApp->GetTopWindow(),
                                           wxID_ANY, "",
                                           wxDefaultPosition, wxDefaultSize,
                                           0, nullptr,
                                           wxCB_SORT);

        m_container->Append("aaa");
        m_container->Append("Aaa");
        m_container->Append("aba");
        m_container->Append("aaab");
        m_container->Append("aab");
        m_container->Append("AAA");

        CHECK_EQ("AAA", m_container->GetString(0));
        CHECK_EQ("Aaa", m_container->GetString(1));
        CHECK_EQ("aaa", m_container->GetString(2));
        CHECK_EQ("aaab", m_container->GetString(3));
        CHECK_EQ("aab", m_container->GetString(4));
        CHECK_EQ("aba", m_container->GetString(5));

        m_container->Append("a");

        CHECK_EQ("a", m_container->GetString(0));
    }

    SUBCASE("ReadOnly")
    {
        std::vector<std::string> testitems = { "item 1", "item 2" };

        m_container = std::make_unique<wxOwnerDrawnComboBox>(wxTheApp->GetTopWindow(),
                                                             wxID_ANY, "",
                                                             wxDefaultPosition, wxDefaultSize,
                                                             testitems,
                                                             wxCB_READONLY);

        m_container->SetValue("item 1");

        CHECK_EQ("item 1", m_container->GetValue());

        m_container->SetValue("not an item");

        CHECK_EQ("item 1", m_container->GetValue());

        // Since this uses FindString it is case insensitive
        m_container->SetValue("ITEM 2");

        CHECK_EQ("item 2", m_container->GetValue());
    }

    wxITEM_CONTAINER_TESTS();
}

using OwnerDrawnComboBoxTextTest = TextEntryTest<wxOwnerDrawnComboBox>;

TEST_CASE_FIXTURE(OwnerDrawnComboBoxTextTest, "Owner-drawn combo box test")
{
    m_entry = std::make_unique<wxOwnerDrawnComboBox>(wxTheApp->GetTopWindow(), wxID_ANY);

    wxTEXT_ENTRY_TESTS();
}

#endif // wxUSE_ODCOMBOBOX
