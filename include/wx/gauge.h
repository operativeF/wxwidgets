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

#include "wx/defs.h"

#if wxUSE_GAUGE

#include "wx/control.h"

#include <string>

// ----------------------------------------------------------------------------
// wxGauge style flags
// ----------------------------------------------------------------------------

inline constexpr int wxGA_HORIZONTAL      = wxHORIZONTAL;
inline constexpr int wxGA_VERTICAL        = wxVERTICAL;

// Available since Windows 7 only. With this style, the value of gauge will
// reflect on the taskbar button.
inline constexpr int wxGA_PROGRESS = 0x0010;
// Win32 only, is default (and only) on some other platforms
inline constexpr int wxGA_SMOOTH = 0x0020;
// QT only, display current completed percentage (text default format "%p%")
inline constexpr int wxGA_TEXT = 0x0040;

// GTK and Mac always have native implementation of the indeterminate mode
// wxMSW has native implementation only if comctl32.dll >= 6.00
#if !defined(__WXGTK20__) && !defined(__WXMAC__)
    #define wxGAUGE_EMULATE_INDETERMINATE_MODE 1
#else
    #define wxGAUGE_EMULATE_INDETERMINATE_MODE 0
#endif

inline constexpr char wxGaugeNameStr[] = "gauge";

class WXDLLIMPEXP_FWD_CORE wxAppProgressIndicator;

// ----------------------------------------------------------------------------
// wxGauge: a progress bar
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxGaugeBase : public wxControl
{
public:
    wxGaugeBase() : m_rangeMax(0), m_gaugePos(0),
#if wxGAUGE_EMULATE_INDETERMINATE_MODE
        m_nDirection(wxRIGHT),
#endif
        m_appProgressIndicator(nullptr) { }

    ~wxGaugeBase();

    wxGaugeBase(const wxGaugeBase&) = delete;
    wxGaugeBase& operator=(const wxGaugeBase&) = delete;
    wxGaugeBase(wxGaugeBase&&) = default;
    wxGaugeBase& operator=(wxGaugeBase&&) = default;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                int range,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxGA_HORIZONTAL,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxGaugeNameStr);

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
    int m_rangeMax;

    // the current position
    int m_gaugePos;

#if wxGAUGE_EMULATE_INDETERMINATE_MODE
    // in case we need to emulate indeterminate mode...
    int m_nDirection{wxRIGHT};       // can be wxRIGHT or wxLEFT
#endif

    wxAppProgressIndicator *m_appProgressIndicator;
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
