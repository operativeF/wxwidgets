///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/apptrait.h
// Purpose:     class implementing wxAppTraits for MSW
// Author:      Vadim Zeitlin
// Modified by:
// Created:     21.06.2003
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_APPTRAIT_H_
#define _WX_MSW_APPTRAIT_H_

#include "wx/defs.h"

import <string>;

// ----------------------------------------------------------------------------
// wxGUI/ConsoleAppTraits: must derive from wxAppTraits, not wxAppTraitsBase
// ----------------------------------------------------------------------------

struct wxConsoleAppTraits : public wxConsoleAppTraitsBase
{
    wxEventLoopBase *CreateEventLoop() override;
    void *BeforeChildWaitLoop() override;
    void AfterChildWaitLoop(void *data) override;
#if wxUSE_TIMER
    wxTimerImpl *CreateTimerImpl(wxTimer *timer) override;
#endif // wxUSE_TIMER
#if wxUSE_THREADS
    bool DoMessageFromThreadWait() override;
    DWORD WaitForThread(WXHANDLE hThread, wxThreadWait flags) override;
#endif // wxUSE_THREADS
    bool CanUseStderr() override { return true; }
    bool WriteToStderr(const std::string& text) override;
    WXHWND GetMainHWND() const override { return nullptr; }
};

#if wxUSE_GUI

#if defined(__WXMSW__)

struct wxGUIAppTraits : public wxGUIAppTraitsBase
{
    wxEventLoopBase *CreateEventLoop() override;
    void *BeforeChildWaitLoop() override;
    void AfterChildWaitLoop(void *data) override;
#if wxUSE_TIMER
    wxTimerImpl *CreateTimerImpl(wxTimer *timer) override;
#endif // wxUSE_TIMER
#if wxUSE_THREADS
    bool DoMessageFromThreadWait() override;
    DWORD WaitForThread(WXHANDLE hThread, wxThreadWait flags) override;
#endif // wxUSE_THREADS
    wxPortId GetToolkitVersion(int *majVer = nullptr,
                               int *minVer = nullptr,
                               int *microVer = nullptr) const override;

    bool CanUseStderr() override;
    bool WriteToStderr(const std::string& text) override;
    WXHWND GetMainHWND() const override;
};

#elif defined(__WXGTK__)

struct wxGUIAppTraits : public wxGUIAppTraitsBase
{
    virtual wxEventLoopBase *CreateEventLoop();
    virtual void *BeforeChildWaitLoop() { return NULL; }
    virtual void AfterChildWaitLoop(void *WXUNUSED(data)) { }
#if wxUSE_TIMER
    virtual wxTimerImpl *CreateTimerImpl(wxTimer *timer);
#endif

#if wxUSE_THREADS && defined(__WXGTK20__)
    virtual void MutexGuiEnter();
    virtual void MutexGuiLeave();
#endif

#if wxUSE_THREADS
    virtual bool DoMessageFromThreadWait() { return true; }
    virtual DWORD WaitForThread(WXHANDLE hThread, int WXUNUSED(flags))
        { return DoSimpleWaitForThread(hThread); }
#endif // wxUSE_THREADS
    virtual wxPortId GetToolkitVersion(int *majVer = NULL,
                                       int *minVer = NULL,
                                       int *microVer = NULL) const;

    virtual bool CanUseStderr() { return false; }
    virtual bool WriteToStderr(const wxString& WXUNUSED(text)) { return false; }
    WXHWND GetMainHWND() const override { return NULL; }
};

#elif defined(__WXQT__)

struct wxGUIAppTraits : public wxGUIAppTraitsBase
{
    virtual wxEventLoopBase *CreateEventLoop();
    virtual void *BeforeChildWaitLoop() { return NULL; }
    virtual void AfterChildWaitLoop(void*) { }
#if wxUSE_TIMER
    virtual wxTimerImpl *CreateTimerImpl(wxTimer *timer);
#endif

#if wxUSE_THREADS
    virtual bool DoMessageFromThreadWait() { return true; }
    virtual DWORD WaitForThread(WXHANDLE hThread, int WXUNUSED(flags))
        { return DoSimpleWaitForThread(hThread); }
#endif // wxUSE_THREADS
    virtual wxPortId GetToolkitVersion(int *majVer = NULL,
                                       int *minVer = NULL,
                                       int *microVer = NULL) const;

    virtual bool CanUseStderr() { return false; }
    virtual bool WriteToStderr(const wxString&) { return false; }
    WXHWND GetMainHWND() const override { return NULL; }
};

#endif

#endif // wxUSE_GUI

#endif // _WX_MSW_APPTRAIT_H_
