///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/ownerdrawncomboboxtest.cpp
// Purpose:     OwnerDrawnComboBox unit test
// Author:      Jaakko Salli
// Created:     2010-12-17
// Copyright:   (c) 2010 Jaakko Salli
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

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
// test class
// ----------------------------------------------------------------------------

class OwnerDrawnComboBoxTestCase : public TextEntryTestCase,
                                   public ItemContainerTestCase,
                                   public CppUnit::TestCase
{
public:
    OwnerDrawnComboBoxTestCase() { }

    void setUp() override;
    void tearDown() override;

private:
    wxTextEntry *GetTestEntry() const override { return m_combo; }
    wxWindow *GetTestWindow() const override { return m_combo; }

    wxItemContainer *GetContainer() const override { return m_combo; }
    wxWindow *GetContainerWindow() const override { return m_combo; }

    void CheckStringSelection(const char * WXUNUSED(sel)) override
    {
        // do nothing here, as explained in TextEntryTestCase comment, our
        // GetStringSelection() is the wxChoice, not wxTextEntry, one and there
        // is no way to return the selection contents directly
    }

    CPPUNIT_TEST_SUITE( OwnerDrawnComboBoxTestCase );
        wxTEXT_ENTRY_TESTS();
        wxITEM_CONTAINER_TESTS();
        CPPUNIT_TEST( Size );
        CPPUNIT_TEST( PopDismiss );
        CPPUNIT_TEST( Sort );
        CPPUNIT_TEST( ReadOnly );
    CPPUNIT_TEST_SUITE_END();

    void Size();
    void PopDismiss();
    void Sort();
    void ReadOnly();

    wxOwnerDrawnComboBox *m_combo;

    OwnerDrawnComboBoxTestCase(const OwnerDrawnComboBoxTestCase&) = delete;
	OwnerDrawnComboBoxTestCase& operator=(const OwnerDrawnComboBoxTestCase&) = delete;
};

wxREGISTER_UNIT_TEST_WITH_TAGS(OwnerDrawnComboBoxTestCase,
                               "[OwnerDrawnComboBoxTestCase][item-container]");

// ----------------------------------------------------------------------------
// test initialization
// ----------------------------------------------------------------------------

void OwnerDrawnComboBoxTestCase::setUp()
{
    m_combo = new wxOwnerDrawnComboBox(wxTheApp->GetTopWindow(), wxID_ANY);
}

void OwnerDrawnComboBoxTestCase::tearDown()
{
    delete m_combo;
    m_combo = NULL;
}

// ----------------------------------------------------------------------------
// tests themselves
// ----------------------------------------------------------------------------

void OwnerDrawnComboBoxTestCase::Size()
{
    // under MSW changing combobox size is a non-trivial operation because of
    // confusion between the size of the control with and without dropdown, so
    // check that it does work as expected

    const int heightOrig = m_combo->GetSize().y;

    // check that the height doesn't change if we don't touch it
    m_combo->SetSize(100, -1);
    CHECK_EQ( heightOrig, m_combo->GetSize().y );

    // check that setting both big and small (but not too small, there is a
    // limit on how small the control can become under MSW) heights works
    m_combo->SetSize(-1, 50);
    CHECK_EQ( 50, m_combo->GetSize().y );

    m_combo->SetSize(-1, 10);
    CHECK_EQ( 10, m_combo->GetSize().y );

    // and also that restoring it works (this used to be broken before 2.9.1)
    m_combo->SetSize(-1, heightOrig);
    CHECK_EQ( heightOrig, m_combo->GetSize().y );
}

void OwnerDrawnComboBoxTestCase::PopDismiss()
{
    EventCounter drop(m_combo, wxEVT_COMBOBOX_DROPDOWN);
    EventCounter close(m_combo, wxEVT_COMBOBOX_CLOSEUP);

    m_combo->Popup();
    m_combo->Dismiss();

    CHECK_EQ(1, drop.GetCount());
    CHECK_EQ(1, close.GetCount());
}

void OwnerDrawnComboBoxTestCase::Sort()
{
    delete m_combo;
    m_combo = new wxOwnerDrawnComboBox(wxTheApp->GetTopWindow(),
                                       wxID_ANY, "",
                                       wxDefaultPosition, wxDefaultSize,
                                       0, NULL,
                                       wxCB_SORT);

    m_combo->Append("aaa");
    m_combo->Append("Aaa");
    m_combo->Append("aba");
    m_combo->Append("aaab");
    m_combo->Append("aab");
    m_combo->Append("AAA");

    CHECK_EQ("AAA", m_combo->GetString(0));
    CHECK_EQ("Aaa", m_combo->GetString(1));
    CHECK_EQ("aaa", m_combo->GetString(2));
    CHECK_EQ("aaab", m_combo->GetString(3));
    CHECK_EQ("aab", m_combo->GetString(4));
    CHECK_EQ("aba", m_combo->GetString(5));

    m_combo->Append("a");

    CHECK_EQ("a", m_combo->GetString(0));
}

void OwnerDrawnComboBoxTestCase::ReadOnly()
{
    std::vector<wxString> testitems;
    testitems.push_back("item 1");
    testitems.push_back("item 2");

    delete m_combo;
    m_combo = new wxOwnerDrawnComboBox(wxTheApp->GetTopWindow(), wxID_ANY, "",
                                       wxDefaultPosition, wxDefaultSize,
                                       testitems,
                                       wxCB_READONLY);

    m_combo->SetValue("item 1");

    CHECK_EQ("item 1", m_combo->GetValue());

    m_combo->SetValue("not an item");

    CHECK_EQ("item 1", m_combo->GetValue());

    // Since this uses FindString it is case insensitive
    m_combo->SetValue("ITEM 2");

    CHECK_EQ("item 2", m_combo->GetValue());
}

#endif // wxUSE_ODCOMBOBOX
