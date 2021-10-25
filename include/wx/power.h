///////////////////////////////////////////////////////////////////////////////
// Name:        wx/power.h
// Purpose:     functions and classes for system power management
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2006-05-27
// Copyright:   (c) 2006 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_POWER_H_
#define _WX_POWER_H_

#include "wx/event.h"

// ----------------------------------------------------------------------------
// power management constants
// ----------------------------------------------------------------------------

enum class wxPowerType
{
    Socket,
    Battery,
    Unknown
};

enum class wxBatteryState
{
    Normal,    // system is fully usable
    Low,       // start to worry
    Critical,  // save quickly
    Shutdown,  // too late
    Unknown
};

// ----------------------------------------------------------------------------
// wxPowerEvent is generated when the system online status changes
// ----------------------------------------------------------------------------

// currently the power events are only available under Windows, to avoid
// compiling in the code for handling them which is never going to be invoked
// under the other platforms, we define wxHAS_POWER_EVENTS symbol if this event
// is available, it should be used to guard all code using wxPowerEvent
#ifdef __WINDOWS__

#define wxHAS_POWER_EVENTS

class WXDLLIMPEXP_BASE wxPowerEvent : public wxEvent
{
public:
    wxPowerEvent() = default;

    wxPowerEvent(wxEventType evtType) : wxEvent(wxID_NONE, evtType)
    {
    }

	wxPowerEvent& operator=(const wxPowerEvent&) = delete;

    // Veto the operation (only makes sense with EVT_POWER_SUSPENDING)
    void Veto() { m_veto = true; }

    bool IsVetoed() const { return m_veto; }


    // default copy ctor, assignment operator and dtor are ok

    wxEvent *Clone() const override { return new wxPowerEvent(*this); }

private:
    bool m_veto{false};

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_BASE, wxEVT_POWER_SUSPENDING, wxPowerEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_BASE, wxEVT_POWER_SUSPENDED, wxPowerEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_BASE, wxEVT_POWER_SUSPEND_CANCEL, wxPowerEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_BASE, wxEVT_POWER_RESUME, wxPowerEvent );

typedef void (wxEvtHandler::*wxPowerEventFunction)(wxPowerEvent&);

#define wxPowerEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxPowerEventFunction, func)

#define EVT_POWER_SUSPENDING(func) \
    wx__DECLARE_EVT0(wxEVT_POWER_SUSPENDING, wxPowerEventHandler(func))
#define EVT_POWER_SUSPENDED(func) \
    wx__DECLARE_EVT0(wxEVT_POWER_SUSPENDED, wxPowerEventHandler(func))
#define EVT_POWER_SUSPEND_CANCEL(func) \
    wx__DECLARE_EVT0(wxEVT_POWER_SUSPEND_CANCEL, wxPowerEventHandler(func))
#define EVT_POWER_RESUME(func) \
    wx__DECLARE_EVT0(wxEVT_POWER_RESUME, wxPowerEventHandler(func))

#else // no support for power events
    #undef wxHAS_POWER_EVENTS
#endif // support for power events/no support

// ----------------------------------------------------------------------------
// wxPowerResourceBlocker
// ----------------------------------------------------------------------------

enum wxPowerResourceKind
{
    wxPOWER_RESOURCE_SCREEN,
    wxPOWER_RESOURCE_SYSTEM
};

class WXDLLIMPEXP_BASE wxPowerResource
{
public:
    static bool Acquire(wxPowerResourceKind kind,
                        const wxString& reason = {});
    static void Release(wxPowerResourceKind kind);
};

class wxPowerResourceBlocker
{
public:
    explicit wxPowerResourceBlocker(wxPowerResourceKind kind,
                                    const wxString& reason = {})
        : m_kind(kind),
          m_acquired(wxPowerResource::Acquire(kind, reason))
    {
    }

    bool IsInEffect() const { return m_acquired; }

    ~wxPowerResourceBlocker()
    {
        if ( m_acquired )
            wxPowerResource::Release(m_kind);
    }

private:
    const wxPowerResourceKind m_kind;
    const bool m_acquired;
};

// ----------------------------------------------------------------------------
// power management functions
// ----------------------------------------------------------------------------

// return the current system power state: online or offline
WXDLLIMPEXP_BASE wxPowerType wxGetPowerType();

// return approximate battery state
WXDLLIMPEXP_BASE wxBatteryState wxGetBatteryState();

#endif // _WX_POWER_H_
