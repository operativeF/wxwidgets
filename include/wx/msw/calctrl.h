/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/calctrl.h
// Purpose:     wxCalendarCtrl control implementation for MSW
// Author:      Vadim Zeitlin
// Copyright:   (C) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_CALCTRL_H_
#define _WX_MSW_CALCTRL_H_

import WX.WinDef;

import <cstdint>;
import <string>;

class wxCalendarCtrl : public wxCalendarCtrlBase
{
public:
    wxCalendarCtrl() = default;
    wxCalendarCtrl(wxWindow *parent,
                   wxWindowID id,
                   const wxDateTime& date = wxDefaultDateTime,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   unsigned int style = wxCAL_SHOW_HOLIDAYS,
                   const std::string& name = wxCalendarNameStr)
    {
        Create(parent, id, date, pos, size, style, name);
    }

    wxCalendarCtrl& operator=(wxCalendarCtrl&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxDateTime& date = wxDefaultDateTime,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxCAL_SHOW_HOLIDAYS,
                const std::string& name = wxCalendarNameStr);

    bool SetDate(const wxDateTime& date) override;
    wxDateTime GetDate() const override;

    bool SetDateRange(const wxDateTime& lowerdate = wxDefaultDateTime,
                              const wxDateTime& upperdate = wxDefaultDateTime) override;
    bool GetDateRange(wxDateTime *lowerdate, wxDateTime *upperdate) const override;

    bool EnableMonthChange(bool enable = true) override;

    void Mark(size_t day, bool mark) override;
    void SetHoliday(size_t day) override;

    wxCalendarHitTestResult HitTest(const wxPoint& pos,
                                            wxDateTime *date = nullptr,
                                            wxDateTime::WeekDay *wd = nullptr) override;

    void SetWindowStyleFlag(unsigned int style) override;

protected:
    wxSize DoGetBestSize() const override;

    WXDWORD MSWGetStyle(unsigned int style, WXDWORD *exstyle) const override;

    bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) override;

    void MSWOnClick(wxMouseEvent& event);
    void MSWOnDoubleClick(wxMouseEvent& event);

private:
    // bring the control in sync with m_marks
    void UpdateMarks();

    // set first day of week in the control to correspond to our
    // wxCAL_MONDAY_FIRST flag
    void UpdateFirstDayOfWeek();

    // reset holiday information
    void ResetHolidayAttrs() override { m_holidays = 0; }

    // redisplay holidays
    void RefreshHolidays() override { UpdateMarks(); }


    // current date, we need to store it instead of simply retrieving it from
    // the control as needed in order to be able to generate the correct events
    // from MSWOnNotify()
    wxDateTime m_date;

    // bit field containing the state (marked or not) of all days in the month
    std::uint32_t m_marks {0};

    // the same but indicating whether a day is a holiday or not
    std::uint32_t m_holidays {0};


    wxDECLARE_DYNAMIC_CLASS(wxCalendarCtrl);
};

#endif // _WX_MSW_CALCTRL_H_
