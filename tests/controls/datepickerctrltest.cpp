///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/datepickerctrltest.cpp
// Purpose:     wxDatePickerCtrl unit test
// Author:      Vadim Zeitlin
// Created:     2011-06-18
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_DATEPICKCTRL

#include "wx/app.h"
#include "wx/button.h"

#include "wx/datectrl.h"
#include "wx/uiaction.h"

#include "testableframe.h"
#include "testdate.h"

#if wxUSE_UIACTIONSIMULATOR

static wxPoint GetRectCenter(const wxRect& r)
{
    return (r.GetTopRight() + r.GetBottomLeft()) / 2;
}

#endif // wxUSE_UIACTIONSIMULATOR

TEST_CASE("Date picker control test")
{
    auto m_datepicker = std::make_unique<wxDatePickerCtrl>(wxTheApp->GetTopWindow(),
                                                           wxID_ANY);
    SUBCASE("Value")
    {
        const wxDateTime dt(18, wxDateTime::Jul, 2011);
        m_datepicker->SetValue(dt);

        CHECK_EQ( dt, m_datepicker->GetValue() );

        // We don't use wxDP_ALLOWNONE currently, hence a value is required.
        WX_ASSERT_FAILS_WITH_ASSERT( m_datepicker->SetValue(wxDateTime()) );
    }

    SUBCASE("Range")
    {
        // Initially we have no valid range but MSW version still has (built in)
        // minimum as it doesn't support dates before 1601-01-01, hence don't rely
        // on GetRange() returning false.
        wxDateTime dtRangeStart, dtRangeEnd;

        // Default end date for QT is 31/12/7999 which is considered valid,
        // therefore we should omit this assertion for QT
    #ifndef __WXQT__
        m_datepicker->GetRange(&dtRangeStart, &dtRangeEnd);
        CHECK( !dtRangeEnd.IsValid() );
    #endif

        // After we set it we should be able to get it back.
        const wxDateTime
            dtStart(15, wxDateTime::Feb, 1923),
            dtEnd(18, wxDateTime::Jun, 2011);

        m_datepicker->SetRange(dtStart, dtEnd);
        CHECK( m_datepicker->GetRange(&dtRangeStart, &dtRangeEnd) );
        CHECK_EQ( dtStart, dtRangeStart );
        CHECK_EQ( dtEnd, dtRangeEnd );

        // Setting dates inside the range should work, including the range end
        // points.
        m_datepicker->SetValue(dtStart);
        CHECK_EQ( dtStart, m_datepicker->GetValue() );

        m_datepicker->SetValue(dtEnd);
        CHECK_EQ( dtEnd, m_datepicker->GetValue() );


        // Setting dates outside the range should not work.
        m_datepicker->SetValue(dtEnd + wxTimeSpan::Day());
        CHECK_EQ( dtEnd, m_datepicker->GetValue() );

        m_datepicker->SetValue(dtStart - wxTimeSpan::Day());
        CHECK_EQ( dtEnd, m_datepicker->GetValue() );


        // Changing the range should clamp the current value to it if necessary.
        const wxDateTime
            dtBeforeEnd = dtEnd - wxDateSpan::Day();
        m_datepicker->SetRange(dtStart, dtBeforeEnd);
        CHECK_EQ( dtBeforeEnd, m_datepicker->GetValue() );
    }

#if wxUSE_UIACTIONSIMULATOR

    SUBCASE("Focus")
    {
        // Create another control just to give focus to it initially.
        auto m_button = std::make_unique<wxButton>(wxTheApp->GetTopWindow(), wxID_OK);
        m_button->Move(wxPoint{0, m_datepicker->GetSize().y * 3});
        m_button->SetFocus();
        wxYield();

        CHECK( !m_datepicker->HasFocus() );

        EventCounter setFocus(m_datepicker.get(), wxEVT_SET_FOCUS);
        EventCounter killFocus(m_datepicker.get(), wxEVT_KILL_FOCUS);

        wxUIActionSimulator sim;

        sim.MouseMove(GetRectCenter(m_datepicker->GetScreenRect()));
        sim.MouseClick();
        wxYield();

        REQUIRE( m_datepicker->HasFocus() );
        CHECK( setFocus.GetCount() == 1 );
        CHECK( killFocus.GetCount() == 0 );

        sim.MouseMove(GetRectCenter(m_button->GetScreenRect()));
        sim.MouseClick();
        wxYield();

        CHECK( !m_datepicker->HasFocus() );
        CHECK( setFocus.GetCount() == 1 );
        CHECK( killFocus.GetCount() == 1 );
    }
}

#endif // wxUSE_UIACTIONSIMULATOR

#endif // wxUSE_DATEPICKCTRL
