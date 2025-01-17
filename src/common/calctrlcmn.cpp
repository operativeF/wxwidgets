///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/calctrlcmn.cpp
// Author:      Marcin Wojdyr
// Created:     2008-03-26
// Copyright:   (C) Marcin Wojdyr
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_CALENDARCTRL || wxUSE_DATEPICKCTRL || wxUSE_TIMEPICKCTRL

#include "wx/dateevt.h"
wxDEFINE_EVENT(wxEVT_DATE_CHANGED, wxDateEvent);
wxDEFINE_EVENT(wxEVT_TIME_CHANGED, wxDateEvent);

#endif // wxUSE_CALENDARCTRL || wxUSE_DATEPICKCTRL || wxUSE_TIMEPICKCTRL


#if wxUSE_CALENDARCTRL

#include "wx/calctrl.h"

// ----------------------------------------------------------------------------
// events
// ----------------------------------------------------------------------------

wxDEFINE_EVENT( wxEVT_CALENDAR_SEL_CHANGED, wxCalendarEvent );
wxDEFINE_EVENT( wxEVT_CALENDAR_PAGE_CHANGED, wxCalendarEvent );
wxDEFINE_EVENT( wxEVT_CALENDAR_DOUBLECLICKED, wxCalendarEvent );
wxDEFINE_EVENT( wxEVT_CALENDAR_WEEKDAY_CLICKED, wxCalendarEvent );
wxDEFINE_EVENT( wxEVT_CALENDAR_WEEK_CLICKED, wxCalendarEvent );

// deprecated events
wxDEFINE_EVENT( wxEVT_CALENDAR_DAY_CHANGED, wxCalendarEvent );
wxDEFINE_EVENT( wxEVT_CALENDAR_MONTH_CHANGED, wxCalendarEvent );
wxDEFINE_EVENT( wxEVT_CALENDAR_YEAR_CHANGED, wxCalendarEvent );


wxCalendarDateAttr wxCalendarDateAttr::m_mark(wxCalendarDateBorder::Square);

bool wxCalendarCtrlBase::EnableMonthChange(bool enable)
{
    const unsigned int styleOrig = wxGetWindowStyle();
    unsigned int style = enable ? styleOrig & ~wxCAL_NO_MONTH_CHANGE
                        : styleOrig | wxCAL_NO_MONTH_CHANGE;
    if ( style == styleOrig )
        return false;

    SetWindowStyle(style);

    return true;
}

bool wxCalendarCtrlBase::GenerateAllChangeEvents(const wxDateTime& dateOld)
{
    const wxDateTime::Tm tm1 = dateOld.GetTm(),
                         tm2 = GetDate().GetTm();

    bool pageChanged = false;

    GenerateEvent(wxEVT_CALENDAR_SEL_CHANGED);
    if ( tm1.year != tm2.year || tm1.mon != tm2.mon )
    {
        GenerateEvent(wxEVT_CALENDAR_PAGE_CHANGED);

        pageChanged = true;
    }

    // send also one of the deprecated events
    if ( tm1.year != tm2.year )
        GenerateEvent(wxEVT_CALENDAR_YEAR_CHANGED);
    else if ( tm1.mon != tm2.mon )
        GenerateEvent(wxEVT_CALENDAR_MONTH_CHANGED);
    else
        GenerateEvent(wxEVT_CALENDAR_DAY_CHANGED);

    return pageChanged;
}

void wxCalendarCtrlBase::EnableHolidayDisplay(bool display)
{
    unsigned int style = wxGetWindowStyle();
    if ( display )
        style |= wxCAL_SHOW_HOLIDAYS;
    else
        style &= ~wxCAL_SHOW_HOLIDAYS;

    if ( style == wxGetWindowStyle() )
        return;

    SetWindowStyle(style);

    if ( display )
        SetHolidayAttrs();
    else
        ResetHolidayAttrs();

    RefreshHolidays();
}

bool wxCalendarCtrlBase::SetHolidayAttrs()
{
    if ( !HasFlag(wxCAL_SHOW_HOLIDAYS) )
        return false;

    ResetHolidayAttrs();

    wxDateTime::Tm tm = GetDate().GetTm();
    wxDateTime dtStart(1, tm.mon, tm.year),
               dtEnd = dtStart.GetLastMonthDay();

    std::vector<wxDateTime> holidays = wxDateTimeHolidayAuthority::GetHolidaysInRange(dtStart, dtEnd);

    for ( const auto& holiday : holidays )
    {
        SetHoliday(holiday.GetDay());
    }

    return true;
}

bool wxCalendarCtrlBase::WeekStartsOnMonday() const
{
    if ( HasFlag(wxCAL_MONDAY_FIRST) )
    {
        return true;
    }
    else if ( HasFlag(wxCAL_SUNDAY_FIRST) )
    {
        return false;
    }
    else
    {
        // Neither flag was explicitly given, let's make a best guess
        // based on locale and/or OS settings.

        return wxDateTime::GetFirstWeekDay() == wxDateTime::Mon;
    }
}

#endif // wxUSE_CALENDARCTRL

