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

using RearrangeListTest = ItemContainerTest<wxRearrangeList>;

TEST_CASE_FIXTURE(RearrangeListTest, "Rearrange list control test")
{
    auto m_rearrange = std::make_unique<wxRearrangeList>(wxTheApp->GetTopWindow(),
                                                         wxID_ANY,
                                                         wxDefaultPosition,
                                                         wxDefaultSize,
                                                         std::vector<int>{},
                                                         std::vector<wxString>{});
    SUBCASE("Move")
    {
        std::vector<int> order;
        order.push_back(1);
        order.push_back(~2);
        order.push_back(0);

        std::vector<wxString> items;
        items.push_back("first");
        items.push_back("second");
        items.push_back("third");

        m_rearrange = std::make_unique<wxRearrangeList>(wxTheApp->GetTopWindow(),
                                                        wxID_ANY, wxDefaultPosition,
                                                        wxDefaultSize, order,
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

    SUBCASE("MoveClientData")
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

        m_rearrange = std::make_unique<wxRearrangeList>(wxTheApp->GetTopWindow(),
                                                        wxID_ANY, wxDefaultPosition,
                                                        wxDefaultSize, order,
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

    m_container = std::make_unique<wxRearrangeList>(wxTheApp->GetTopWindow(),
                                                    wxID_ANY,
                                                    wxDefaultPosition,
                                                    wxDefaultSize,
                                                    std::vector<int>{},
                                                    std::vector<wxString>{});

    wxITEM_CONTAINER_TESTS();
}

#endif
