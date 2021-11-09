/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/timercmn.cpp
// Purpose:     wxTimerBase implementation
// Author:      Julian Smart, Guillermo Rodriguez, Vadim Zeitlin
// Modified by: VZ: extracted all non-wxTimer stuff in stopwatch.cpp (20.06.03)
// Created:     04/01/98
// Copyright:   (c) Julian Smart
//              (c) 1999 Guillermo Rodriguez <guille@iies.es>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_TIMER

#include "wx/app.h"
#include "wx/timer.h"
#include "wx/apptrait.h"
#include "wx/private/timer.h"

import <chrono>;

// ----------------------------------------------------------------------------
// wxWin macros
// ----------------------------------------------------------------------------

// This class is not really abstract, but this macro has to be used because it
// doesn't have a default ctor.
wxIMPLEMENT_ABSTRACT_CLASS(wxTimerEvent, wxEvent);

wxDEFINE_EVENT(wxEVT_TIMER, wxTimerEvent);

// ============================================================================
// wxTimerBase implementation
// ============================================================================

wxTimer::~wxTimer()
{
    Stop();

    delete m_impl;
}

void wxTimer::Init()
{
    wxAppTraits * const traits = wxApp::GetTraitsIfExists();
    m_impl = traits ? traits->CreateTimerImpl(this) : nullptr;
    if ( !m_impl )
    {
        wxFAIL_MSG( "No timer implementation for this platform" );

    }
}

// ============================================================================
// rest of wxTimer implementation forwarded to wxTimerImpl
// ============================================================================

void wxTimer::SetOwner(wxEvtHandler *owner, int timerid)
{
    wxCHECK_RET( m_impl, "uninitialized timer" );

    m_impl->SetOwner(owner, timerid);
}

wxEvtHandler *wxTimer::GetOwner() const
{
    wxCHECK_MSG( m_impl, nullptr, "uninitialized timer" );

    return m_impl->GetOwner();
}

bool wxTimer::Start(std::chrono::milliseconds startTime, bool oneShot)
{
    wxCHECK_MSG( m_impl, false, "uninitialized timer" );

    return m_impl->Start(startTime, oneShot);
}

void wxTimer::Stop()
{
    wxCHECK_RET( m_impl, "uninitialized timer" );

    if ( m_impl->IsRunning() )
        m_impl->Stop();
}

void wxTimer::Notify()
{
    // the base class version generates an event if it has owner - which it
    // should because otherwise nobody can process timer events
    wxCHECK_RET( GetOwner(), "wxTimer::Notify() should be overridden." );

    m_impl->SendEvent();
}

bool wxTimer::IsRunning() const
{
    wxCHECK_MSG( m_impl, false, "uninitialized timer" );

    return m_impl->IsRunning();
}

int wxTimer::GetId() const
{
    wxCHECK_MSG( m_impl, wxID_ANY, "uninitialized timer" );

    return m_impl->GetId();
}

std::chrono::milliseconds wxTimer::GetInterval() const
{
    wxCHECK_MSG( m_impl, -1ms, "uninitialized timer" );

    return m_impl->GetInterval();
}

bool wxTimer::IsOneShot() const
{
    wxCHECK_MSG( m_impl, false, "uninitialized timer" );

    return m_impl->IsOneShot();
}

#endif // wxUSE_TIMER

