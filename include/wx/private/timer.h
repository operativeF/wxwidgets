/////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/timer.h
// Purpose:     Base class for wxTimer implementations
// Author:      Lukasz Michalski <lmichalski@sf.net>
// Created:     31.10.2006
// Copyright:   (c) 2006-2007 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TIMERIMPL_H_BASE_
#define _WX_TIMERIMPL_H_BASE_

#include "wx/defs.h"

#include "wx/event.h"
#include "wx/timer.h"

#include <chrono>

// ----------------------------------------------------------------------------
// wxTimerImpl: abstract base class for wxTimer implementations
// ----------------------------------------------------------------------------

using namespace std::chrono_literals;

class WXDLLIMPEXP_BASE wxTimerImpl
{
public:
    // default ctor, SetOwner() must be called after it (wxTimer does it)
    wxTimerImpl(wxTimer *owner);

    wxTimerImpl(const wxTimerImpl&) = delete;
	wxTimerImpl& operator=(const wxTimerImpl&) = delete;

    // this must be called initially but may be also called later
    void SetOwner(wxEvtHandler *owner, int timerid);

    // empty but virtual base class dtor, the caller is responsible for
    // stopping the timer before it's destroyed (it can't be done from here as
    // it's too late)
    virtual ~wxTimerImpl() = default;


    // start the timer. When overriding call base version first.
    virtual bool Start(std::chrono::milliseconds startTime = -1ms, bool oneShot = false);

    // stop the timer, only called if the timer is really running (unlike
    // wxTimer::Stop())
    virtual void Stop() = 0;

    // return true if the timer is running
    virtual bool IsRunning() const = 0;

    // this should be called by the port-specific code when the timer expires
    virtual void Notify() { m_timer->Notify(); }

    // the default implementation of wxTimer::Notify(): generate a wxEVT_TIMER
    void SendEvent();

    wxEvtHandler *GetOwner() const { return m_owner; }
    int GetId() const { return m_idTimer; }
    std::chrono::milliseconds GetInterval() const { return m_interval; }
    bool IsOneShot() const { return m_oneShot; }

protected:
    wxTimer *m_timer;

    wxEvtHandler *m_owner{nullptr};

    int     m_idTimer{wxID_ANY};      // id passed to wxTimerEvent
    std::chrono::milliseconds m_interval{0ms};        // the timer interval
    bool    m_oneShot{false};      // true if one shot
};

#endif // _WX_TIMERIMPL_H_BASE_
