///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/checklistlistbox.cpp
// Purpose:     wxCheckListBox unit test
// Author:      Steven Lamerton
// Created:     2010-06-30
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "testprec.h"

#if wxUSE_CHECKLISTBOX


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/checklst.h"
#endif // WX_PRECOMP

#include "itemcontainertest.h"
#include "testableframe.h"

class CheckListBoxTestCase : public ItemContainerTestCase, public CppUnit::TestCase
{
public:
    CheckListBoxTestCase() { }

    void setUp() override;
    void tearDown() override;

private:
    wxItemContainer *GetContainer() const override { return m_check; }
    wxWindow *GetContainerWindow() const override { return m_check; }

    CPPUNIT_TEST_SUITE( CheckListBoxTestCase );
        wxITEM_CONTAINER_TESTS();
        CPPUNIT_TEST( Check );
    CPPUNIT_TEST_SUITE_END();

    void Check();

    wxCheckListBox* m_check;

    CheckListBoxTestCase(const CheckListBoxTestCase&) = delete;
	CheckListBoxTestCase& operator=(const CheckListBoxTestCase&) = delete;
};

wxREGISTER_UNIT_TEST_WITH_TAGS(CheckListBoxTestCase,
                               "[CheckListBoxTestCase][item-container]");

void CheckListBoxTestCase::setUp()
{
    m_check = new wxCheckListBox(wxTheApp->GetTopWindow(), wxID_ANY);
}

void CheckListBoxTestCase::tearDown()
{
    wxDELETE(m_check);
}

void CheckListBoxTestCase::Check()
{
    EventCounter toggled(m_check, wxEVT_CHECKLISTBOX);

    std::vector<int> checkedItems;
    std::vector<wxString> testitems;
    testitems.push_back("item 0");
    testitems.push_back("item 1");
    testitems.push_back("item 2");
    testitems.push_back("item 3");

    m_check->Append(testitems);

    m_check->Check(0);
    m_check->Check(1);
    m_check->Check(1, false);

    //We should not get any events when changing this from code
    CPPUNIT_ASSERT_EQUAL(0, toggled.GetCount());
    CPPUNIT_ASSERT_EQUAL(true, m_check->IsChecked(0));
    CPPUNIT_ASSERT_EQUAL(false, m_check->IsChecked(1));

    // FIXME: No conversion from wxArrayInt to std::vector<unsigned int>
    CPPUNIT_ASSERT_EQUAL(1, m_check->GetCheckedItemsCount(checkedItems));
    CPPUNIT_ASSERT_EQUAL(0, checkedItems[0]);

    //Make sure a double check of an items doesn't deselect it
    m_check->Check(0);

    CPPUNIT_ASSERT_EQUAL(true, m_check->IsChecked(0));
}

#endif // wxUSE_CHECKLISTBOX
