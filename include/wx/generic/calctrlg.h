///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/calctrlg.h
// Purpose:     generic implementation of date-picker control
// Author:      Vadim Zeitlin
// Modified by:
// Created:     29.12.99
// Copyright:   (c) 1999 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_CALCTRLG_H
#define _WX_GENERIC_CALCTRLG_H

#include "wx/control.h"         // the base class
#include "wx/dcclient.h"        // for wxPaintDC

import Utils.Geometry;

import <string>;

class wxComboBox;
class wxStaticText;
class wxSpinCtrl;
class wxSpinEvent;

// ----------------------------------------------------------------------------
// wxGenericCalendarCtrl
// ----------------------------------------------------------------------------

class wxGenericCalendarCtrl : public wxCalendarCtrlBase
{
public:
    // construction
    // FIXME: This is just the other constructor without creation. Necessary?
    wxGenericCalendarCtrl()
    { 
        wxDateTime::WeekDay wd;
        for ( wd = wxDateTime::Sun; wd < wxDateTime::Inv_WeekDay; wxNextWDay(wd) )
        {
            m_weekdays[wd] = wxDateTime::GetWeekDayName(wd, wxDateTime::Name_Abbr);
        }

        // FIXME: unique_ptr
        for ( auto* attr : m_attrs )
        {
            attr = nullptr;
        }

        InitColours();
    }

    wxGenericCalendarCtrl(wxWindow *parent,
                          wxWindowID id,
                          const wxDateTime& date = wxDefaultDateTime,
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxDefaultSize,
                          unsigned int style = wxCAL_SHOW_HOLIDAYS,
                          const std::string& name = wxCalendarNameStr);

	wxGenericCalendarCtrl& operator=(wxGenericCalendarCtrl&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxDateTime& date = wxDefaultDateTime,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxCAL_SHOW_HOLIDAYS,
                const std::string& name = wxCalendarNameStr);

    ~wxGenericCalendarCtrl();

    bool Destroy() override;

    // set/get the current date
    // ------------------------

    bool SetDate(const wxDateTime& date) override;
    wxDateTime GetDate() const override { return m_date; }


    // set/get the range in which selection can occur
    // ---------------------------------------------

    bool SetDateRange(const wxDateTime& lowerdate = wxDefaultDateTime,
                              const wxDateTime& upperdate = wxDefaultDateTime) override;

    bool GetDateRange(wxDateTime *lowerdate, wxDateTime *upperdate) const override;

    // these functions are for generic version only, don't use them but use the
    // Set/GetDateRange() above instead
    bool SetLowerDateLimit(const wxDateTime& date = wxDefaultDateTime);
    const wxDateTime& GetLowerDateLimit() const { return m_lowdate; }
    bool SetUpperDateLimit(const wxDateTime& date = wxDefaultDateTime);
    const wxDateTime& GetUpperDateLimit() const { return m_highdate; }


    // calendar mode
    // -------------

    // some calendar styles can't be changed after the control creation by
    // just using SetWindowStyle() and Refresh() and the functions below
    // should be used instead for them

    // corresponds to wxCAL_NO_MONTH_CHANGE bit
    bool EnableMonthChange(bool enable = true) override;

    // corresponds to wxCAL_NO_YEAR_CHANGE bit, deprecated, generic only
    void EnableYearChange(bool enable = true);


    // customization
    // -------------

    void Mark(size_t day, bool mark) override;

    // all other functions in this section are for generic version only

    // header colours are used for painting the weekdays at the top
    void SetHeaderColours(const wxColour& colFg, const wxColour& colBg) override
    {
        m_colHeaderFg = colFg;
        m_colHeaderBg = colBg;
    }

    const wxColour& GetHeaderColourFg() const override { return m_colHeaderFg; }
    const wxColour& GetHeaderColourBg() const override { return m_colHeaderBg; }

    // highlight colour is used for the currently selected date
    void SetHighlightColours(const wxColour& colFg, const wxColour& colBg) override
    {
        m_colHighlightFg = colFg;
        m_colHighlightBg = colBg;
    }

    const wxColour& GetHighlightColourFg() const override { return m_colHighlightFg; }
    const wxColour& GetHighlightColourBg() const override { return m_colHighlightBg; }

    // holiday colour is used for the holidays (if style & wxCAL_SHOW_HOLIDAYS)
    void SetHolidayColours(const wxColour& colFg, const wxColour& colBg) override
    {
        m_colHolidayFg = colFg;
        m_colHolidayBg = colBg;
    }

    const wxColour& GetHolidayColourFg() const override { return m_colHolidayFg; }
    const wxColour& GetHolidayColourBg() const override { return m_colHolidayBg; }

    wxCalendarDateAttr *GetAttr(size_t day) const override
    {
        wxCHECK_MSG( day > 0 && day < 32, nullptr, "invalid day" );

        return m_attrs[day - 1];
    }

    void SetAttr(size_t day, wxCalendarDateAttr *attr) override
    {
        wxCHECK_RET( day > 0 && day < 32, "invalid day" );

        delete m_attrs[day - 1];
        m_attrs[day - 1] = attr;
    }

    void ResetAttr(size_t day) override { SetAttr(day, nullptr); }

    void SetHoliday(size_t day) override;

    wxCalendarHitTestResult HitTest(const wxPoint& pos,
                                            wxDateTime *date = nullptr,
                                            wxDateTime::WeekDay *wd = nullptr) override;

    // implementation only from now on
    // -------------------------------

    // forward these functions to all subcontrols
    bool Enable(bool enable = true) override;
    bool Show(bool show = true) override;

    void SetWindowStyleFlag(unsigned int style) override;

    wxVisualAttributes GetDefaultAttributes() const override
        { return GetClassDefaultAttributes(GetWindowVariant()); }

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWindowVariant::Normal);

    void OnSysColourChanged(wxSysColourChangedEvent& event);

protected:
    // override some base class virtuals
    wxSize DoGetBestSize() const override;
    void DoMoveWindow(wxRect boundary) override;
    wxSize DoGetSize() const override;

private:
    // common part of all ctors
    

    // startup colours and reinitialization after colour changes in system
    void InitColours();

    // event handlers
    void OnPaint(wxPaintEvent& event);
    void OnClick(wxMouseEvent& event);
    void OnDClick(wxMouseEvent& event);
    void OnWheel(wxMouseEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMonthChange(wxCommandEvent& event);

    void HandleYearChange(wxCommandEvent& event);
    void OnYearChange(wxSpinEvent& event);
    void OnYearTextChange(wxCommandEvent& event);

    // (re)calc m_widthCol and m_heightRow
    void RecalcGeometry();

    // set the date and send the notification
    void SetDateAndNotify(const wxDateTime& date);

    // get the week (row, in range 1..6) for the given date
    size_t GetWeek(const wxDateTime& date) const;

    // get the date from which we start drawing days
    wxDateTime GetStartDate() const;

    // get the first/last days of the week corresponding to the current style
    wxDateTime::WeekDay GetWeekStart() const
    {
        return WeekStartsOnMonday() ? wxDateTime::Mon
                                           : wxDateTime::Sun;
    }

    wxDateTime::WeekDay GetWeekEnd() const
    {
        return WeekStartsOnMonday() ? wxDateTime::Sun
                                           : wxDateTime::Sat;
    }


    // is this date shown?
    bool IsDateShown(const wxDateTime& date) const;

    // is this date in the currently allowed range?
    bool IsDateInRange(const wxDateTime& date) const;

    // adjust the date to the currently allowed range, return true if it was
    // changed
    bool AdjustDateToRange(wxDateTime *date) const;

    // redraw the given date
    void RefreshDate(const wxDateTime& date);

    // change the date inside the same month/year
    void ChangeDay(const wxDateTime& date);

    // deprecated
    bool AllowYearChange() const
    {
        return !(wxGetWindowStyle() & wxCAL_NO_YEAR_CHANGE);
    }

    // show the correct controls
    void ShowCurrentControls();

    // create the month combo and year spin controls
    void CreateMonthComboBox();
    void CreateYearSpinCtrl();

public:
    // get the currently shown control for month/year
    wxControl *GetMonthControl() const;
    wxControl *GetYearControl() const;

private:
    void ResetHolidayAttrs() override;
    void RefreshHolidays() override { Refresh(); }

    // OnPaint helper-methods

    // Highlight the [fromdate : todate] range using pen and brush
    void HighlightRange(wxPaintDC* dc, const wxDateTime& fromdate, const wxDateTime& todate, const wxPen* pen, const wxBrush* brush);

    // Get the "coordinates" for the date relative to the month currently displayed.
    // using (day, week): upper left coord is (1, 1), lower right coord is (7, 6)
    // if the date isn't visible (-1, -1) is put in (day, week) and false is returned
    bool GetDateCoord(const wxDateTime& date, int *day, int *week) const;

    // Set the flag for SetDate(): otherwise it would overwrite the year
    // typed in by the user
    void SetUserChangedYear() { m_userChangedYear = true; }


    // the subcontrols
    wxStaticText *m_staticMonth{nullptr};
    wxComboBox *m_comboMonth{nullptr};

    wxStaticText *m_staticYear{nullptr};
    wxSpinCtrl *m_spinYear{nullptr};

    // the current selection
    wxDateTime m_date;

    // the date-range
    wxDateTime m_lowdate;
    wxDateTime m_highdate;

    // default attributes
    wxColour m_colHighlightFg;
    wxColour m_colHighlightBg;
    wxColour m_colHolidayFg;
    wxColour m_colHolidayBg;
    wxColour m_colHeaderFg;
    wxColour m_colHeaderBg;
    wxColour m_colBackground;
    wxColour m_colSurrounding;

    // the attributes for each of the month days
    wxCalendarDateAttr *m_attrs[31];

    // the width and height of one column/row in the calendar
    wxCoord m_widthCol{0};
    wxCoord m_heightRow{0};
    wxCoord m_calendarWeekWidth{0};
    wxCoord m_rowOffset;

    wxRect m_leftArrowRect;
    wxRect m_rightArrowRect;

    // the week day names
    std::string m_weekdays[7];

    // true if SetDate() is being called as the result of changing the year in
    // the year control
    bool m_userChangedYear{false};

    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_GENERIC_CALCTRLG_H
