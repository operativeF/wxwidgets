///////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/evtloop.h
// Purpose:     wxGTK event loop implementation
// Author:      Vadim Zeitlin
// Created:     2008-12-27
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_EVTLOOP_H_
#define _WX_GTK_EVTLOOP_H_

// ----------------------------------------------------------------------------
// wxGUIEventLoop for wxGTK
// ----------------------------------------------------------------------------

typedef union  _GdkEvent        GdkEvent;

class wxGUIEventLoop : public wxEventLoopBase
{
public:
    wxGUIEventLoop();

    void ScheduleExit(int rc = 0) override;
    bool Pending() const override;
    bool Dispatch() override;
    int DispatchTimeout(unsigned long timeout) override;
    void WakeUp() override;

    void StoreGdkEventForLaterProcessing(GdkEvent* ev)
        { m_arrGdkEvents.push_back(ev); }

protected:
    int DoRun() override;
    void DoYieldFor(long eventsToProcess) override;

private:
    // the exit code of this event loop
    int m_exitcode;

    // used to temporarily store events in DoYield()
    std::vector<void*> m_arrGdkEvents;

    wxGUIEventLoop(const wxGUIEventLoop&) = delete;
	wxGUIEventLoop& operator=(const wxGUIEventLoop&) = delete;
};

#endif // _WX_GTK_EVTLOOP_H_
