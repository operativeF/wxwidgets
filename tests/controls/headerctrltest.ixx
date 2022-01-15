///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/headerctrltest.cpp
// Purpose:     wxHeaderCtrl unit test
// Author:      Vadim Zeitlin
// Created:     2008-11-26
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/headerctrl.h"

export module WX.Test.HeaderCtrl;

import WX.Test.Prec;
import WX.MetaTest;

import <algorithm>;
import <array>;
import <ranges>;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

namespace ut = boost::ut;

ut::suite HeaderCtrlTests = []
{
    using namespace ut;

    auto m_header = std::make_unique<wxHeaderCtrlSimple>(wxTheApp->GetTopWindow());

    "AddDelete"_test = [&]
    {
        expect( 0 == m_header->GetColumnCount() );

        m_header->AppendColumn(wxHeaderColumnSimple("Column 1"));
        expect( 1 == m_header->GetColumnCount() );

        m_header->AppendColumn(wxHeaderColumnSimple("Column 2"));
        expect( 2 == m_header->GetColumnCount() );

        m_header->InsertColumn(wxHeaderColumnSimple("Column 0"), 0);
        expect( 3 == m_header->GetColumnCount() );

        m_header->DeleteColumn(2);
        expect( 2 == m_header->GetColumnCount() );

        m_header->DeleteAllColumns();
    };

    "BestSize"_test = [&]
    {
        const wxSize sizeEmpty = m_header->GetBestSize();
        // this fails under wxGTK where wxControl::GetBestSize() is 0 in horizontal
        // direction
        //expect( sizeEmpty.x > 0 );
        expect( sizeEmpty.y > 0 );

        m_header->AppendColumn(wxHeaderColumnSimple("Foo"));
        m_header->AppendColumn(wxHeaderColumnSimple("Bar"));
        const wxSize size = m_header->GetBestSize();
        expect( sizeEmpty.y == size.y );

        m_header->DeleteAllColumns();
    };

    "Reorder"_test = [&]
    {
        for ( int n = 0; n < 4; n++ )
            m_header->AppendColumn(wxHeaderColumnSimple(fmt::format("{:d}", n)));

        auto ordering = m_header->GetColumnsOrder();

        // "Check initial order [0, 1, 2, 3]")

        constexpr std::array<int, 4> initOrder{ 0, 1, 2, 3 };

        expect(std::ranges::equal(ordering, initOrder));

        // "Check reorder [1, 2, 0, 3]"

        constexpr std::array<int, 4> reorderA{ 1, 2, 0, 3 };

        wxHeaderCtrl::MoveColumnInOrderArray(ordering, 0, 2);
        m_header->SetColumnsOrder(ordering);

        ordering = m_header->GetColumnsOrder();

        expect(std::ranges::equal(ordering, reorderA));

        // "Check reorder [1, 2, 3, 0]"

        ordering[2] = 3;
        ordering[3] = 0;

        m_header->SetColumnsOrder(ordering);
        ordering = m_header->GetColumnsOrder();

        constexpr std::array<int, 4> reorderB{ 1, 2, 3, 0 };

        expect(std::ranges::equal(ordering, reorderB));

        // "Check reorder [2, 3, 0, 1]"

        wxHeaderCtrl::MoveColumnInOrderArray(ordering, 1, 3);
        m_header->SetColumnsOrder(ordering);
        ordering = m_header->GetColumnsOrder();

        constexpr std::array<int, 4> reorderC{ 2, 3, 0, 1 };

        expect(std::ranges::equal(ordering, reorderC));
    };
};
