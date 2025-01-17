///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/evtloopconsole.h
// Purpose:     wxConsoleEventLoop class for Windows
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2004-07-31
// Copyright:   (c) 2003-2004 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_EVTLOOPCONSOLE_H_
#define _WX_MSW_EVTLOOPCONSOLE_H_

import WX.WinDef;

class wxMSWEventLoopBase : public wxEventLoopManual
{
public:
    wxMSWEventLoopBase();
    ~wxMSWEventLoopBase();

    bool Pending() const override;
    void WakeUp() override;

#if wxUSE_THREADS
    // MSW-specific method to wait for the termination of the specified (by its
    // native handle) thread or any input message arriving (in GUI case).
    //
    // Return value is WAIT_OBJECT_0 if the thread terminated, WAIT_OBJECT_0+1
    // if a message arrived with anything else indicating an error.
    WXDWORD MSWWaitForThread(WXHANDLE hThread);
#endif // wxUSE_THREADS

    // Return true if wake up was requested and not handled yet, i.e. if
    // m_heventWake is signaled.
    bool MSWIsWakeUpRequested();

protected:
    // get the next message from queue and return true or return false if we
    // got WM_QUIT or an error occurred
    bool GetNextMessage(WXMSG *msg);

    // same as above but with a timeout and return value can be -1 meaning that
    // time out expired in addition to true/false
    int GetNextMessageTimeout(WXMSG *msg, unsigned long timeout);

private:
    // An auto-reset Win32 event which is signalled when we need to wake up the
    // main thread waiting in GetNextMessage[Timeout]().
    WXHANDLE m_heventWake;
};

#if wxUSE_CONSOLE_EVENTLOOP

class wxConsoleEventLoop : public wxMSWEventLoopBase
{
public:
    wxConsoleEventLoop() = default;

    // override/implement base class virtuals
    bool Dispatch() override;
    int DispatchTimeout(unsigned long timeout) override;

    // Windows-specific function to process a single message
    virtual void ProcessMessage(WXMSG *msg);

protected:
    void DoYieldFor(long eventsToProcess) override;
};

#endif // wxUSE_CONSOLE_EVENTLOOP

#endif // _WX_MSW_EVTLOOPCONSOLE_H_
