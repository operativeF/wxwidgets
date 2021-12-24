///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/stopwatch.cpp
// Purpose:     wxStopWatch and other non-GUI stuff from wx/timer.h
// Author:
//    Original version by Julian Smart
//    Vadim Zeitlin got rid of all ifdefs (11.12.99)
//    Sylvain Bougnoux added wxStopWatch class
//    Guillermo Rodriguez <guille@iies.es> rewrote from scratch (Dic/99)
// Modified by:
// Created:     20.06.2003 (extracted from common/timercmn.cpp)
// Copyright:   (c) 1998-2003 wxWidgets Team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/thread.h"

module WX.Cmn.Stopwatch;

import WX.Utils.Cast;

import <cstdint>;

#if wxUSE_STOPWATCH

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxStopWatch
// ----------------------------------------------------------------------------

namespace
{

#ifdef WX_WINDOWS

struct PerfCounter
{
    wxCRIT_SECT_DECLARE_MEMBER(cs);
    LARGE_INTEGER freq;
    bool init{false};
};

// Return the global perf counter state.
//
// This is wrapped in a function to avoid initialization order problems,
// otherwise simply creating a global wxStopWatch variable could crash because
// it would be using a (possibly) still uninitialized critical section.
PerfCounter& GetPerfCounterState()
{
    static PerfCounter s_perfCounter;

    return s_perfCounter;
}

#endif // WX_WINDOWS

} // anonymous namespace

constexpr int MILLISECONDS_PER_SECOND = 1000;
constexpr int MICROSECONDS_PER_SECOND = 1000 * 1000;

void wxStopWatch::DoStart()
{
#ifdef WX_WINDOWS
    PerfCounter& perfCounter = GetPerfCounterState();
    if ( !perfCounter.init )
    {
        wxCRIT_SECT_LOCKER(lock, perfCounter.cs);
        ::QueryPerformanceFrequency(&perfCounter.freq);

        perfCounter.init = true;
    }
#endif // WX_WINDOWS

    m_t0 = GetCurrentClockValue();
}

std::int64_t wxStopWatch::GetClockFreq() const
{
#ifdef WX_WINDOWS
    // Under MSW we use the high resolution performance counter timer which has
    // its own frequency (usually related to the CPU clock speed).
    return GetPerfCounterState().freq.QuadPart;
#elif defined(HAVE_GETTIMEOFDAY)
    // With gettimeofday() we can have nominally microsecond precision and
    // while this is not the case in practice, it's still better than
    // millisecond.
    return MICROSECONDS_PER_SECOND;
#else // !HAVE_GETTIMEOFDAY
    // Currently milliseconds are used everywhere else.
    return MILLISECONDS_PER_SECOND;
#endif // WX_WINDOWS/HAVE_GETTIMEOFDAY/else
}

void wxStopWatch::Start(std::int32_t t0)
{
    // Calling Start() makes the stop watch run however many times it was
    // paused before.
    m_pauseCount = 0;

    DoStart();

    m_t0 -= (std::int64_t{t0} * GetClockFreq() ) / MILLISECONDS_PER_SECOND;
}

std::int64_t wxStopWatch::GetCurrentClockValue() const
{
#ifdef WX_WINDOWS
    LARGE_INTEGER counter;
    ::QueryPerformanceCounter(&counter);
    return counter.QuadPart;
#elif defined(HAVE_GETTIMEOFDAY)
    return wxGetUTCTimeUSec();
#else // !HAVE_GETTIMEOFDAY
    return wxGetUTCTimeMillis();
#endif // WX_WINDOWS/HAVE_GETTIMEOFDAY/else
}

std::int64_t wxStopWatch::TimeInMicro() const
{
    const std::int64_t elapsed(m_pauseCount ? m_elapsedBeforePause
                                          : GetCurrentClockValue() - m_t0);

    return (elapsed * MICROSECONDS_PER_SECOND) / GetClockFreq();
}

#endif // wxUSE_STOPWATCH

// ----------------------------------------------------------------------------
// old timer functions superceded by wxStopWatch
// ----------------------------------------------------------------------------

#if wxUSE_LONGLONG

static std::int64_t wxStartTime = 0;

// starts the global timer
void wxStartTimer()
{
    wxStartTime = wxGetUTCTimeMillis();
}

// Returns elapsed time in milliseconds
std::int32_t wxGetElapsedTime(bool resetTimer)
{
    std::int64_t oldTime = wxStartTime;
    std::int64_t newTime = wxGetUTCTimeMillis();

    if ( resetTimer )
        wxStartTime = newTime;

    // NOTE: Truncating by design.
    return wx::narrow_cast<std::int32_t>(newTime - oldTime);
}

#endif // wxUSE_LONGLONG
