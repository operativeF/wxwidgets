///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/evtloopconsole.cpp
// Purpose:     wxConsoleEventLoop class for Windows
// Author:      Vadim Zeitlin
// Modified by:
// Created:     01.06.01
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"
#include "wx/log.h"
#include "wx/evtloop.h"

import WX.WinDef;

wxMSWEventLoopBase::wxMSWEventLoopBase()
    : m_heventWake(::CreateEventW(nullptr, FALSE, FALSE, nullptr))
{
    // Create initially not signalled auto-reset event object.
    if ( !m_heventWake )
        wxLogLastError("CreateEvent(wake)");
}

wxMSWEventLoopBase::~wxMSWEventLoopBase()
{
    if ( m_heventWake && !::CloseHandle(m_heventWake) )
        wxLogLastError("CloseHandle(wake)");
}

// ----------------------------------------------------------------------------
// wxEventLoop message processing dispatching
// ----------------------------------------------------------------------------

bool wxMSWEventLoopBase::Pending() const
{
    MSG msg;
    return ::PeekMessageW(&msg, nullptr, 0, 0, PM_NOREMOVE) != 0;
}

void wxMSWEventLoopBase::WakeUp()
{
    if ( !::SetEvent(m_heventWake) )
        wxLogLastError("SetEvent(wake)");
}

bool wxMSWEventLoopBase::MSWIsWakeUpRequested()
{
    return ::WaitForSingleObject(m_heventWake, 0) == WAIT_OBJECT_0;
}

#if wxUSE_THREADS

WXDWORD wxMSWEventLoopBase::MSWWaitForThread(WXHANDLE hThread)
{
    // The order is important here, the code using this function assumes that
    // WAIT_OBJECT_0 indicates the thread termination and anything else -- the
    // availability of an input event. So the thread handle must come first.
    HANDLE handles[2] = { hThread, m_heventWake };
    return ::MsgWaitForMultipleObjects
             (
               WXSIZEOF(handles),   // number of objects to wait for
               handles,             // the objects
               false,               // wait for any objects, not all
               INFINITE,            // no timeout
               QS_ALLINPUT |        // return as soon as there are any events
               QS_ALLPOSTMESSAGE
             );
}

#endif // wxUSE_THREADS

bool wxMSWEventLoopBase::GetNextMessage(WXMSG* msg)
{
    return GetNextMessageTimeout(msg, INFINITE) == TRUE;
}

int wxMSWEventLoopBase::GetNextMessageTimeout(WXMSG *msg, unsigned long timeout)
{
    // MsgWaitForMultipleObjects() won't notice any input which was already
    // examined (e.g. using PeekMessage()) but not yet removed from the queue
    // so we need to remove any immediately messages manually
    while ( !::PeekMessageW(msg, nullptr, 0, 0, PM_REMOVE) )
    {
        const WXDWORD rc = ::MsgWaitForMultipleObjects
                     (
                        1, &m_heventWake,
                        FALSE,
                        timeout,
                        QS_ALLINPUT | QS_ALLPOSTMESSAGE
                     );

        switch ( rc )
        {
            default:
                wxLogDebug("unexpected MsgWaitForMultipleObjects() return "
                           "value %lu", rc);
                [[fallthrough]];

            case WAIT_TIMEOUT:
                return -1;

            case WAIT_OBJECT_0:
                // We were woken up by a background thread, which means there
                // is no actual input message available, but we should still
                // return to the event loop, so pretend there was WM_NULL in
                // the queue.
                wxZeroMemory(*msg);
                return TRUE;

            case WAIT_OBJECT_0 + 1:
                // Some message is supposed to be available, but spurious
                // wake ups are also possible, so just return to the loop:
                // either we'll get the message or start waiting again.
                break;
        }
    }

    return msg->message != WM_QUIT;
}

// ============================================================================
// wxConsoleEventLoop implementation
// ============================================================================

#if wxUSE_CONSOLE_EVENTLOOP

void wxConsoleEventLoop::ProcessMessage(WXMSG *msg)
{
    ::DispatchMessageW(msg);
}

bool wxConsoleEventLoop::Dispatch()
{
    MSG msg;
    if ( !GetNextMessage(&msg) )
        return false;

    ProcessMessage(&msg);

    return !m_shouldExit;
}

int wxConsoleEventLoop::DispatchTimeout(unsigned long timeout)
{
    MSG msg;
    const int rc = GetNextMessageTimeout(&msg, timeout);
    if ( rc != 1 )
        return rc;

    ProcessMessage(&msg);

    return !m_shouldExit;
}

void wxConsoleEventLoop::DoYieldFor(long eventsToProcess)
{
    wxEventLoopBase::DoYieldFor(eventsToProcess);
}

#endif // wxUSE_CONSOLE_EVENTLOOP
