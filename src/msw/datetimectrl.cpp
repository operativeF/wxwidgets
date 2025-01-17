///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/datetimectrl.cpp
// Purpose:     Implementation of wxDateTimePickerCtrl for MSW.
// Author:      Vadim Zeitlin
// Created:     2011-09-22 (extracted from src/msw/datectrl.cpp)
// Copyright:   (c) 2005-2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/datetimectrl.h"

// FIXME: Change this so order doesn't matter.

#ifdef wxNEEDS_DATETIMEPICKCTRL

#include "wx/msw/wrapcctl.h" // include <commctrl.h> "properly"
#include "wx/msw/private.h"

#include "wx/dcclient.h"

#include "wx/msw/private/datecontrols.h"

import WX.Utils.Settings;

// apparently some versions of mingw define these macros erroneously
#ifndef DateTime_GetSystemtime
    #define DateTime_GetSystemtime DateTime_GetSystemTime
#endif

#ifndef DateTime_SetSystemtime
    #define DateTime_SetSystemtime DateTime_SetSystemTime
#endif

#ifndef DTM_GETIDEALSIZE
    #define DTM_GETIDEALSIZE 0x100f
#endif

import <algorithm>;
import <string>;
import <string_view>;

// ============================================================================
// wxDateTimePickerCtrl implementation
// ============================================================================

bool
wxDateTimePickerCtrl::MSWCreateDateTimePicker(wxWindow *parent,
                                              wxWindowID id,
                                              const wxDateTime& dt,
                                              const wxPoint& pos,
                                              const wxSize& size,
                                              unsigned int style,
                                              const wxValidator& validator,
                                              std::string_view name)
{
    if ( !wxMSWDateControls::CheckInitialization() )
        return false;

    // initialize the base class
    if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

    // create the native control
    if ( !MSWCreateControl(DATETIMEPICK_CLASSA, "", pos, size) )
        return false;

    if ( dt.IsValid() || MSWAllowsNone() )
        SetValue(dt);
    else
        SetValue(wxDateTime::Now());

    return true;
}

void wxDateTimePickerCtrl::SetValue(const wxDateTime& dt)
{
    wxCHECK_RET( dt.IsValid() || MSWAllowsNone(),
                    "this control requires a valid date" );

    SYSTEMTIME st;
    if ( dt.IsValid() )
        dt.GetAsMSWSysTime(&st);

    if ( !DateTime_SetSystemtime(GetHwnd(),
                                 dt.IsValid() ? GDT_VALID : GDT_NONE,
                                 &st) )
    {
        // The only expected failure is when the date is out of range but we
        // already checked for this above.
        wxFAIL_MSG( "Setting the calendar date unexpectedly failed." );

        // In any case, skip updating m_date below.
        return;
    }

    MSWUpdateFormatIfNeeded(dt.IsValid());

    m_date = dt;
}

wxDateTime wxDateTimePickerCtrl::GetValue() const
{
    return m_date;
}

void wxDateTimePickerCtrl::MSWUpdateFormatIfNeeded(bool valid)
{
    if ( MSWAllowsNone() && !m_nullText.empty() && valid != m_date.IsValid() )
        MSWUpdateFormat(valid);
}

void wxDateTimePickerCtrl::MSWUpdateFormat(bool valid)
{
    // We just use NULL to reset to the default format when the date is valid,
    // as the control seems to remember whichever format was used when it was
    // created, i.e. this works both with and without wxDP_SHOWCENTURY.

    // Note: due to a bug in MinGW headers, with missing parentheses around the
    // macro argument (corrected in or before 8.2, but still existing in 5.3),
    // we have to use a temporary variable here.
    if(valid)
    {
        DateTime_SetFormat(GetHwnd(), nullptr);
    }
    else
    {
        boost::nowide::wstackstring stackNullText{m_nullText.c_str()};
        DateTime_SetFormat(GetHwnd(), stackNullText.get());
    }
}

void wxDateTimePickerCtrl::SetNullText(const std::string& text)
{
    m_nullText = text;
    if ( m_nullText.empty() )
    {
        // Using empty format doesn't work with the native control, it just
        // uses the default short date format in this case, so set the format
        // to the single space which is more or less guaranteed to work as it's
        // the semi-official way to clear the control contents when it doesn't
        // have any valid value, according to Microsoft's old KB document
        // Q238077, which can still be found online by searching for its
        // number, even if it's not available on Microsoft web site any more.
        //
        // Coincidentally, it's also convenient for us, as we can just check if
        // null text is empty to see if we need to use it elsewhere in the code.
        m_nullText = " ";
    }
    else
    {
        // We need to quote the text, as otherwise format specifiers (e.g.
        // "d", "m" etc) would be interpreted specially by the control. To make
        // things simple, we just quote it entirely and always.
        wx::utils::ReplaceAll(m_nullText, "'", "''");
        m_nullText.insert(0, "'");
        m_nullText.push_back('\'');
    }

    // Apply it immediately if we don't have any value right now.
    if ( !m_date.IsValid() )
        MSWUpdateFormat(false);
}

wxSize wxDateTimePickerCtrl::DoGetBestSize() const
{
    wxSize size;

    // Use DTM_GETIDEALSIZE to ask the control itself to compute its ideal size.
    SIZE idealSize = { 0, 0 };
    if ( wxGetWinVersion() >= wxWinVersion_Vista )
    {
        // We can't use DTM_GETIDEALSIZE with DTS_SHOWNONE because handling of
        // this flag is completely broken (up to at least Window 10 20H2): it's
        // not just ignored, but we get completely wrong results when this flag
        // is on, e.g. the returned width is less than the width without it or
        // much greater than the real value after a DPI change (and growing
        // with every new change, even when repeatedly switching between the
        // same DPI values, e.g. dragging a window between 2 monitors with
        // different scaling). Moreover, note that even without DTS_SHOWNONE,
        // DTM_GETIDEALSIZE still returns wrong results for the height after a
        // DPI change, so we never use the vertical component of the value
        // returned by it.
        //
        // Unfortunately, resetting this style doesn't work neither, so we have
        // to create a whole new window just for this, which is pretty wasteful
        // but seems unavoidable.
        WXHWND hwnd;
        if ( MSWAllowsNone() )
        {
            hwnd = ::CreateWindowW
                     (
                        DATETIMEPICK_CLASSW,
                        L"",
                        ::GetWindowLongPtrW(GetHwnd(), GWL_STYLE) & ~DTS_SHOWNONE,
                        0, 0, 1, 1,
                        GetHwndOf(m_parent),
                        nullptr,
                        wxGetInstance(),
                        nullptr
                     );

            wxCHECK_MSG( hwnd, wxSize(),
                         "SysDateTimePick32 creation unexpected failed" );

            wxSetWindowFont(hwnd, GetFont());
        }
        else
        {
            hwnd = GetHwnd();
        }

        // Also work around https://bugs.winehq.org/show_bug.cgi?id=44680 by
        // checking for the return value: even if all "real" MSW systems do support
        // this message, Wine does not, even when it's configured to return Vista
        // or later version to the application, and returns FALSE for it.
        if ( ::SendMessageW(hwnd, DTM_GETIDEALSIZE, 0, (WXLPARAM)&idealSize) )
        {
            size.x = idealSize.cx;
            size.y = GetCharHeight();
        }

        if ( hwnd != GetHwnd() )
        {
            ::DestroyWindow(hwnd);
        }
    }

    if ( !idealSize.cx ) // Compute the size ourselves.
    {
        // Use the same native format as the underlying native control.
#if wxUSE_INTL
        std::string s = wxDateTime::Now().Format(wxLocale::GetOSInfo(MSWGetFormat()));
#else // !wxUSE_INTL
        std::string s("XXX-YYY-ZZZZ");
#endif // wxUSE_INTL/!wxUSE_INTL

        // the best size for the control is bigger than just the string
        // representation of the current value because the control must accommodate
        // any date and while the widths of all digits are usually about the same,
        // the width of the month string varies a lot, so try to account for it
        s += 'W';

        size = GetTextExtent(s);

        // Account for the drop-down arrow or spin arrows.
        size.x += wxSystemSettings::GetMetric(wxSYS_HSCROLL_ARROW_X, m_parent);
    }

    // Account for the checkbox.
    if ( MSWAllowsNone() )
    {
        // The extra 8px here was determined heuristically as the value which
        // results in the same layout with and without DTS_SHOWNONE under
        // Windows 7 and Windows 10 with 100%, 150% and 200% scaling.
        size.x += wxGetSystemMetrics(SM_CXMENUCHECK, m_parent) + 8;
    }

    int scrollY = wxSystemSettings::GetMetric(wxSYS_HSCROLL_ARROW_Y, m_parent);
    size.y = std::max(size.y, scrollY);

    // In any case, adjust the height to be the same as for the text controls.
    size.y = EDIT_HEIGHT_FROM_CHAR_HEIGHT(size.y);

    return size;
}

bool
wxDateTimePickerCtrl::MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result)
{
    NMHDR* hdr = (NMHDR *)lParam;
    switch ( hdr->code )
    {
        case DTN_DATETIMECHANGE:
            const NMDATETIMECHANGE& dtch = *(NMDATETIMECHANGE*)(hdr);

            // Update the format before showing the new date if necessary.
            MSWUpdateFormatIfNeeded(dtch.dwFlags == GDT_VALID);

            if ( MSWOnDateTimeChange(dtch) )
            {
                *result = 0;
                return true;
            }
            break;
    }

    return wxDateTimePickerCtrlBase::MSWOnNotify(idCtrl, lParam, result);
}

#endif // wxNEEDS_DATETIMEPICKCTRL
