///////////////////////////////////////////////////////////////////////////////
// Name:        wx/gauge.h
// Purpose:     wxGauge interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.02.01
// Copyright:   (c) 1996-2001 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GAUGE_H_BASE_
#define _WX_GAUGE_H_BASE_

#if wxUSE_GAUGE

#include "wx/appprogress.h"
#include "wx/control.h"

import WX.Cfg.Flags;

#include <memory>
import <string>;

// ----------------------------------------------------------------------------
// wxGauge style flags
// ----------------------------------------------------------------------------

inline constexpr unsigned int wxGA_HORIZONTAL      = wxHORIZONTAL;
inline constexpr unsigned int wxGA_VERTICAL        = wxVERTICAL;

// Available since Windows 7 only. With this style, the value of gauge will
// reflect on the taskbar button.
inline constexpr unsigned int wxGA_PROGRESS = 0x0010;
// Win32 only, is default (and only) on some other platforms
inline constexpr unsigned int wxGA_SMOOTH = 0x0020;
// QT only, display current completed percentage (text default format "%p%")
inline constexpr unsigned int wxGA_TEXT = 0x0040;

// GTK and Mac always have native implementation of the indeterminate mode
// wxMSW has native implementation only if comctl32.dll >= 6.00
#if !defined(__WXGTK20__) && !defined(__WXMAC__)
    #define wxGAUGE_EMULATE_INDETERMINATE_MODE 1
#else
    #define wxGAUGE_EMULATE_INDETERMINATE_MODE 0
#endif

inline constexpr std::string_view wxGaugeNameStr = "gauge";

// ----------------------------------------------------------------------------
// wxGauge: a progress bar
// ----------------------------------------------------------------------------

class wxGaugeBase : public wxControl
{
public:
    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                int range,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxGA_HORIZONTAL,
                const wxValidator& validator = wxDefaultValidator,
                std::string_view name = wxGaugeNameStr);

    // determinate mode API

    // set/get the control range
    virtual void SetRange(int range);
    virtual int GetRange() const;

    virtual void SetValue(int pos);
    virtual int GetValue() const;

    // indeterminate mode API
    virtual void Pulse();

    // simple accessors
    bool IsVertical() const { return HasFlag(wxGA_VERTICAL); }

    // overridden base class virtuals
    bool AcceptsFocus() const override { return false; }

protected:
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    // Initialize m_appProgressIndicator if necessary, i.e. if this object has
    // wxGA_PROGRESS style. This method is supposed to be called from the
    // derived class Create() if it doesn't call the base class Create(), which
    // already does it, after initializing the window style and range.
    void InitProgressIndicatorIfNeeded();

    // the max position
    int m_rangeMax{0};

    // the current position
    int m_gaugePos{0};

#if wxGAUGE_EMULATE_INDETERMINATE_MODE
    // in case we need to emulate indeterminate mode...
    int m_nDirection{wxRIGHT};       // can be wxRIGHT or wxLEFT
#endif

    std::unique_ptr<wxAppProgressIndicator> m_appProgressIndicator;
};

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/gauge.h"
#elif defined(__WXMSW__)
    #include "wx/msw/gauge.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/gauge.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/gauge.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/gauge.h"
#elif defined(__WXMAC__)
    #include "wx/osx/gauge.h"
#elif defined(__WXQT__)
    #include "wx/qt/gauge.h"
#endif

#endif // wxUSE_GAUGE

#endif
    // _WX_GAUGE_H_BASE_
