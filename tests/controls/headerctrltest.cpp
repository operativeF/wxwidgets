///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/headerctrltest.cpp
// Purpose:     wxHeaderCtrl unit test
// Author:      Vadim Zeitlin
// Created:     2008-11-26
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "wx/app.h"
#include "wx/headerctrl.h"

import WX.Test.Prec;

import <algorithm>;
import <array>;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

TEST_CASE("Header control test")
{
    auto m_header = std::make_unique<wxHeaderCtrlSimple>(wxTheApp->GetTopWindow());

    SUBCASE("AddDelete")
    {
        CHECK_EQ( 0, m_header->GetColumnCount() );

        m_header->AppendColumn(wxHeaderColumnSimple("Column 1"));
        CHECK_EQ( 1, m_header->GetColumnCount() );

        m_header->AppendColumn(wxHeaderColumnSimple("Column 2"));
        CHECK_EQ( 2, m_header->GetColumnCount() );

        m_header->InsertColumn(wxHeaderColumnSimple("Column 0"), 0);
        CHECK_EQ( 3, m_header->GetColumnCount() );

        m_header->DeleteColumn(2);
        CHECK_EQ( 2, m_header->GetColumnCount() );
    }

    SUBCASE("BestSize")
    {
        const wxSize sizeEmpty = m_header->GetBestSize();
        // this fails under wxGTK where wxControl::GetBestSize() is 0 in horizontal
        // direction
        //CHECK( sizeEmpty.x > 0 );
        CHECK( sizeEmpty.y > 0 );

        m_header->AppendColumn(wxHeaderColumnSimple("Foo"));
        m_header->AppendColumn(wxHeaderColumnSimple("Bar"));
        const wxSize size = m_header->GetBestSize();
        CHECK_EQ( sizeEmpty.y, size.y );
    }

    SUBCASE("Reorder")
    {
        for ( int n = 0; n < 4; n++ )
            m_header->AppendColumn(wxHeaderColumnSimple(fmt::format("{:d}", n)));

        auto ordering = m_header->GetColumnsOrder();

        // "Check initial order [0, 1, 2, 3]")

        constexpr std::array<int, 4> initOrder{ 0, 1, 2, 3 };

        CHECK(std::equal(ordering.cbegin(), ordering.cend(),
                         initOrder.cbegin(), initOrder.cend()));

        // "Check reorder [1, 2, 0, 3]"

        constexpr std::array<int, 4> reorderA{ 1, 2, 0, 3 };

        wxHeaderCtrl::MoveColumnInOrderArray(ordering, 0, 2);
        m_header->SetColumnsOrder(ordering);

        ordering = m_header->GetColumnsOrder();

        CHECK(std::equal(ordering.cbegin(), ordering.cend(),
                         reorderA.cbegin(), reorderA.cend()));

        // "Check reorder [1, 2, 3, 0]"

        ordering[2] = 3;
        ordering[3] = 0;

        m_header->SetColumnsOrder(ordering);
        ordering = m_header->GetColumnsOrder();

        constexpr std::array<int, 4> reorderB{ 1, 2, 3, 0 };

        CHECK(std::equal(ordering.cbegin(), ordering.cend(),
                         reorderB.cbegin(), reorderB.cend()));

        // "Check reorder [2, 3, 0, 1]"

        wxHeaderCtrl::MoveColumnInOrderArray(ordering, 1, 3);
        m_header->SetColumnsOrder(ordering);
        ordering = m_header->GetColumnsOrder();

        constexpr std::array<int, 4> reorderC{ 2, 3, 0, 1 };

        CHECK(std::equal(ordering.cbegin(), ordering.cend(),
                         reorderC.cbegin(), reorderC.cend()));
    }
}
