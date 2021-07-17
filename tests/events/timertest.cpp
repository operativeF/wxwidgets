///////////////////////////////////////////////////////////////////////////////
// Name:        tests/events/timertest.cpp
// Purpose:     Test wxTimer events
// Author:      Vadim Zeitlin
// Created:     2008-10-22
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#ifndef WX_PRECOMP
#endif // WX_PRECOMP

#include <time.h>

#include "wx/evtloop.h"
#include "wx/timer.h"

// --------------------------------------------------------------------------
// helper class counting the number of timer events
// --------------------------------------------------------------------------

class TimerCounterHandler : public wxEvtHandler
{
public:
    TimerCounterHandler()
    {
        m_events = 0;

        Connect(wxEVT_TIMER, wxTimerEventHandler(TimerCounterHandler::OnTimer));
    }

    int GetNumEvents() const { return m_events; }

private:
    void OnTimer(wxTimerEvent& WXUNUSED(event))
    {
        m_events++;

        Tick();
    }

    virtual void Tick() { /* nothing to do in the base class */ }

    int m_events;

    TimerCounterHandler(const TimerCounterHandler&) = delete;
	TimerCounterHandler& operator=(const TimerCounterHandler&) = delete;
};

TEST_CASE("OneShot")
{
    class ExitOnTimerHandler : public TimerCounterHandler
    {
    public:
        ExitOnTimerHandler(wxEventLoopBase& loop)
            : TimerCounterHandler(),
              m_loop(loop)
        {
        }

    private:
        void Tick() override { m_loop.Exit(); }

        wxEventLoopBase& m_loop;

	    ExitOnTimerHandler& operator=(const ExitOnTimerHandler&) = delete;
    };

    wxEventLoop loop;

    ExitOnTimerHandler handler(loop);
    wxTimer timer(&handler);
    timer.Start(200, true);

    loop.Run();

    CHECK_EQ( 1, handler.GetNumEvents() );
};

TEST_CASE("Multiple")
{
    // FIXME: This test crashes on wxGTK ANSI build slave for unknown reason,
    //        disable it here to let the rest of the test suite run until this
    //        can be fixed.
#if !defined(__WXGTK__)
    wxEventLoop loop;

    TimerCounterHandler handler;
    wxTimer timer(&handler);
    timer.Start(100);

    // run the loop for 2 seconds
    time_t t;
    time(&t);
    const time_t tEnd = t + 2;
    while ( time(&t) < tEnd )
    {
        loop.Dispatch();
    }

    // we can't count on getting exactly 20 ticks but we shouldn't get more
    // than this
    const int numTicks = handler.GetNumEvents();
    CHECK( numTicks <= 20 );

    // and we should get a decent number of them but if the system is very
    // loaded (as happens with build bot slaves running a couple of builds in
    // parallel actually) it may be much less than 20 so just check that we get
    // more than one
    CHECK( numTicks > 1 );
#endif // !(wxGTK)
}
