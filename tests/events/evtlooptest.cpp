///////////////////////////////////////////////////////////////////////////////
// Name:        tests/events/evtloop.cpp
// Purpose:     Tests for the event loop classes
// Author:      Rob Bresalier
// Created:     2013-05-02
// Copyright:   (c) 2013 Rob Bresalier
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "doctest.h"

#include "testprec.h"


#include "wx/timer.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// Use two arbitrary but different return codes for the two loops.
constexpr int EXIT_CODE_OUTER_LOOP = 99;
constexpr int EXIT_CODE_INNER_LOOP = 55;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------


// Helper class to schedule exit of the given event loop after the specified
// delay.
class ScheduleLoopExitTimer : public wxTimer
{
public:
    ScheduleLoopExitTimer(wxEventLoop& loop, int rc)
        : m_loop(loop),
          m_rc(rc)
    {
    }

    void Notify() override
    {
        m_loop.ScheduleExit(m_rc);
    }

private:
    wxEventLoop& m_loop;
    const int m_rc;
};

// Another helper which runs a nested loop and schedules exiting both the outer
// and the inner loop after the specified delays.
class RunNestedAndExitBothLoopsTimer : public wxTimer
{
public:
    RunNestedAndExitBothLoopsTimer(wxTimer& timerOuter,
                                   int loopOuterDuration,
                                   int loopInnerDuration)
        : m_timerOuter(timerOuter),
          m_loopOuterDuration(loopOuterDuration),
          m_loopInnerDuration(loopInnerDuration)
    {
    }

    void Notify() override
    {
        wxEventLoop loopInner;
        ScheduleLoopExitTimer timerInner(loopInner, EXIT_CODE_INNER_LOOP);

        m_timerOuter.StartOnce(m_loopOuterDuration);
        timerInner.StartOnce(m_loopInnerDuration);

        CHECK_EQ( EXIT_CODE_INNER_LOOP, loopInner.Run() );
    }

private:
    wxTimer& m_timerOuter;
    const int m_loopOuterDuration;
    const int m_loopInnerDuration;
};

TEST_CASE("TestExit")
{
    // Test that simply exiting the loop works.
    wxEventLoop loopOuter;
    ScheduleLoopExitTimer timerExit(loopOuter, EXIT_CODE_OUTER_LOOP);
    timerExit.StartOnce(1);
    CHECK_EQ( EXIT_CODE_OUTER_LOOP, loopOuter.Run() );

    // Test that exiting the outer loop before the inner loop (outer duration
    // parameter less than inner duration in the timer ctor below) works.
    ScheduleLoopExitTimer timerExitOuter(loopOuter, EXIT_CODE_OUTER_LOOP);
    RunNestedAndExitBothLoopsTimer timerRun(timerExitOuter, 5, 10);
    timerRun.StartOnce(1);
    CHECK_EQ( EXIT_CODE_OUTER_LOOP, loopOuter.Run() );

    // Test that exiting the inner loop before the outer one works too.
    ScheduleLoopExitTimer timerExitOuter2(loopOuter, EXIT_CODE_OUTER_LOOP);
    RunNestedAndExitBothLoopsTimer timerRun2(timerExitOuter, 10, 5);
    timerRun2.StartOnce(1);
    CHECK_EQ( EXIT_CODE_OUTER_LOOP, loopOuter.Run() );
}
