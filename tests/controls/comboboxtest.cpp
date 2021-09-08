///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/comboboxtest.cpp
// Purpose:     wxComboBox unit test
// Author:      Vadim Zeitlin
// Created:     2007-09-25
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_COMBOBOX


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/combobox.h"

    #include <array>
#endif // WX_PRECOMP

#include "textentrytest.h"
#include "itemcontainertest.h"
#include "testableframe.h"


// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

using wxComboBoxTest = ItemContainerTest<wxComboBox>;

TEST_CASE_FIXTURE(wxComboBoxTest, "Combo box test")
{
    m_container = std::make_unique<wxComboBox>(wxTheApp->GetTopWindow(), wxID_ANY);

    SUBCASE("Size")
    {
        // under MSW changing combobox size is a non-trivial operation because of
        // confusion between the size of the control with and without dropdown, so
        // check that it does work as expected

        const int heightOrig = m_container->GetSize().y;

        // check that the height doesn't change if we don't touch it
        m_container->SetSize(100, -1);
        CHECK_EQ( heightOrig, m_container->GetSize().y );

        // check that setting both big and small (but not too small, there is a
        // limit on how small the control can become under MSW) heights works
        m_container->SetSize(-1, 50);
        CHECK_EQ( 50, m_container->GetSize().y );

        m_container->SetSize(-1, 10);
        CHECK_EQ( 10, m_container->GetSize().y );

        // and also that restoring it works (this used to be broken before 2.9.1)
        m_container->SetSize(-1, heightOrig);
        CHECK_EQ( heightOrig, m_container->GetSize().y );
    }

    SUBCASE("PopDismiss")
    {
    #if defined(__WXMSW__) || defined(__WXGTK210__)
        EventCounter drop(m_container.get(), wxEVT_COMBOBOX_DROPDOWN);
        EventCounter close(m_container.get(), wxEVT_COMBOBOX_CLOSEUP);

        m_container->Popup();
        CHECK_EQ(1, drop.GetCount());

        m_container->Dismiss();

    #if defined(__WXGTK__) && !defined(__WXGTK3__)
        // Under wxGTK2, the event is sent only during idle time and not
        // immediately, so we need this yield to get it.
        wxYield();
    #endif // wxGTK2

        CHECK_EQ(1, close.GetCount());
    #endif
    }

    SUBCASE("Sort")
    {
    #if !defined(__WXOSX__)
        m_container = std::make_unique<wxComboBox>(wxTheApp->GetTopWindow(),
                                                   wxID_ANY, "",
                                                   wxDefaultPosition, wxDefaultSize,
                                                   std::vector<std::string>{},
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
    #endif
    }

    SUBCASE("ReadOnly")
    {
        const std::vector<std::string> testitems = {
            "item 1",
            "item 2"
        };

        m_container = std::make_unique<wxComboBox>(wxTheApp->GetTopWindow(),
                                                   wxID_ANY, "",
                                                   wxDefaultPosition, wxDefaultSize,
                                                   testitems, wxCB_READONLY);

        m_container->SetValue("item 1");

        CHECK_EQ("item 1", m_container->GetValue());

        m_container->SetValue("not an item");

        CHECK_EQ("item 1", m_container->GetValue());

        // Since this uses FindString it is case insensitive
        m_container->SetValue("ITEM 2");

        CHECK_EQ("item 2", m_container->GetValue());
    }

    SUBCASE("IsEmpty")
    {
        CHECK( m_container->IsListEmpty() );
        CHECK( m_container->IsTextEmpty() );

        m_container->Append("foo");
        CHECK( !m_container->IsListEmpty() );
        CHECK( m_container->IsTextEmpty() );

        m_container->SetValue("bar");
        CHECK( !m_container->IsListEmpty() );
        CHECK( !m_container->IsTextEmpty() );

        m_container->Clear();
        CHECK( m_container->IsListEmpty() );
        CHECK( m_container->IsTextEmpty() );

    #ifdef TEST_INVALID_COMBOBOX_ISEMPTY
        // Compiling this should fail, see failtest target definition in test.bkl.
        m_container->IsEmpty();
    #endif
    }

    wxITEM_CONTAINER_TESTS();
}

using wxComboBoxTextTest = TextEntryTest<wxComboBox>;

TEST_CASE_FIXTURE(wxComboBoxTextTest, "Combo box text test")
{
    m_entry = std::make_unique<wxComboBox>(wxTheApp->GetTopWindow(), wxID_ANY);

    wxTEXT_ENTRY_TESTS();
}

TEST_CASE("wxComboBox::ProcessEnter")
{

    class ComboBoxCreator : public TextLikeControlCreator
    {
    public:
        wxControl* Create(wxWindow* parent, int style) const override
        {
            const std::vector<std::string> choices = { "foo", "bar", "baz" };

            return new wxComboBox(parent, wxID_ANY, wxString(),
                                  wxDefaultPosition, wxDefaultSize,
                                  choices,
                                  style);
        }
    };

    TestProcessEnter(ComboBoxCreator());
}

#endif //wxUSE_COMBOBOX
