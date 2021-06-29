///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/rearrangelisttest.cpp
// Purpose:     wxRearrangeList unit test
// Author:      Steven Lamerton
// Created:     2010-07-05
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#ifndef __WXOSX_IPHONE__


#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif // WX_PRECOMP

#include "wx/rearrangectrl.h"
#include "itemcontainertest.h"
#include "testableframe.h"

class RearrangeListTestCase : public ItemContainerTestCase, public CppUnit::TestCase
{
public:
    RearrangeListTestCase() { }

    void setUp() override;
    void tearDown() override;

private:
    wxItemContainer *GetContainer() const override { return m_rearrange; }
    wxWindow *GetContainerWindow() const override { return m_rearrange; }

    CPPUNIT_TEST_SUITE( RearrangeListTestCase );
        wxITEM_CONTAINER_TESTS();
        CPPUNIT_TEST( Move );
        CPPUNIT_TEST( MoveClientData );
    CPPUNIT_TEST_SUITE_END();

    void Move();
    void MoveClientData();

    wxRearrangeList* m_rearrange;

    RearrangeListTestCase(const RearrangeListTestCase&) = delete;
	RearrangeListTestCase& operator=(const RearrangeListTestCase&) = delete;
};

wxREGISTER_UNIT_TEST_WITH_TAGS(RearrangeListTestCase,
                               "[RearrangeListTestCase][item-container]");

void RearrangeListTestCase::setUp()
{
    //We do not add items here as the wxITEM_CONTAINER_TESTS add their own
    std::vector<int> order;
    std::vector<wxString> items;

    m_rearrange = new wxRearrangeList(wxTheApp->GetTopWindow(), wxID_ANY,
                                      wxDefaultPosition, wxDefaultSize, order,
                                      items);
}

void RearrangeListTestCase::tearDown()
{
    wxDELETE(m_rearrange);
}

void RearrangeListTestCase::Move()
{
    std::vector<int> order;
    order.push_back(1);
    order.push_back(~2);
    order.push_back(0);

    std::vector<wxString> items;
    items.push_back("first");
    items.push_back("second");
    items.push_back("third");

    wxDELETE(m_rearrange);

    m_rearrange = new wxRearrangeList(wxTheApp->GetTopWindow(), wxID_ANY,
                                      wxDefaultPosition, wxDefaultSize, order,
                                      items);

    //Confusingly setselection sets the physical item rather than the
    //item specified in the constructor
    m_rearrange->SetSelection(0);

    CHECK(!m_rearrange->CanMoveCurrentUp());
    CHECK(m_rearrange->CanMoveCurrentDown());

    m_rearrange->SetSelection(1);

    CHECK(m_rearrange->CanMoveCurrentUp());
    CHECK(m_rearrange->CanMoveCurrentDown());

    m_rearrange->SetSelection(2);

    CHECK(m_rearrange->CanMoveCurrentUp());
    CHECK(!m_rearrange->CanMoveCurrentDown());

    m_rearrange->MoveCurrentUp();
    m_rearrange->SetSelection(0);
    m_rearrange->MoveCurrentDown();

    auto neworder = m_rearrange->GetCurrentOrder();

    CHECK_EQ(neworder[0], 0);
    CHECK_EQ(neworder[1], 1);
    CHECK_EQ(neworder[2], ~2);

    CHECK_EQ("first", m_rearrange->GetString(0));
    CHECK_EQ("second", m_rearrange->GetString(1));
    CHECK_EQ("third", m_rearrange->GetString(2));
}

void RearrangeListTestCase::MoveClientData()
{
    std::vector<int> order;
    order.push_back(0);
    order.push_back(1);
    order.push_back(2);

    std::vector<wxString> items;
    items.push_back("first");
    items.push_back("second");
    items.push_back("third");

    wxClientData* item0data = new wxStringClientData("item0data");
    wxClientData* item1data = new wxStringClientData("item1data");
    wxClientData* item2data = new wxStringClientData("item2data");

    wxDELETE(m_rearrange);

    m_rearrange = new wxRearrangeList(wxTheApp->GetTopWindow(), wxID_ANY,
                                      wxDefaultPosition, wxDefaultSize, order,
                                      items);

    m_rearrange->SetClientObject(0, item0data);
    m_rearrange->SetClientObject(1, item1data);
    m_rearrange->SetClientObject(2, item2data);

    m_rearrange->SetSelection(0);
    m_rearrange->MoveCurrentDown();

    m_rearrange->SetSelection(2);
    m_rearrange->MoveCurrentUp();

    CHECK_EQ(item1data, m_rearrange->GetClientObject(0));
    CHECK_EQ(item2data, m_rearrange->GetClientObject(1));
    CHECK_EQ(item0data, m_rearrange->GetClientObject(2));

    CHECK_EQ("second", m_rearrange->GetString(0));
    CHECK_EQ("third", m_rearrange->GetString(1));
    CHECK_EQ("first", m_rearrange->GetString(2));
}

#endif
