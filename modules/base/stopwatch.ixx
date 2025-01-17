/////////////////////////////////////////////////////////////////////////////
// Name:        wx/stopwatch.h
// Purpose:     wxStopWatch and global time-related functions
// Author:      Julian Smart (wxTimer), Sylvain Bougnoux (wxStopWatch),
//              Vadim Zeitlin (time functions, current wxStopWatch)
// Created:     26.06.03 (extracted from wx/timer.h)
// Copyright:   (c) 1998-2003 Julian Smart, Sylvain Bougnoux
//              (c) 2011 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/debug.h"

export module WX.Cmn.Stopwatch;

// Time-related functions are also available via this header for compatibility
// but you should include wx/time.h directly if you need only them and not
// wxStopWatch itself.
import WX.Cmn.Time;

import <cstdint>;

// ----------------------------------------------------------------------------
// wxStopWatch: measure time intervals with up to 1ms resolution
// ----------------------------------------------------------------------------

#if wxUSE_STOPWATCH

export
{

class wxStopWatch
{
public:
    // ctor starts the stop watch
    wxStopWatch() { Start(); }

    // Start the stop watch at the moment t0 expressed in milliseconds (i.e.
    // calling Time() immediately afterwards returns t0). This can be used to
    // restart an existing stopwatch.
    void Start(std::int32_t t0 = 0);

    // pause the stop watch
    void Pause()
    {
        if ( m_pauseCount++ == 0 )
            m_elapsedBeforePause = GetCurrentClockValue() - m_t0;
    }

    // resume it
    void Resume()
    {
        wxASSERT_MSG( m_pauseCount > 0,
                      "Resuming stop watch which is not paused" );

        if ( --m_pauseCount == 0 )
        {
            DoStart();
            m_t0 -= m_elapsedBeforePause;
        }
    }

    // Get elapsed time since the last Start() in microseconds.
    std::int64_t TimeInMicro() const;

    // get elapsed time since the last Start() in milliseconds
    std::int32_t Time() const { return TimeInMicro() / 1000; }

private:
    // Really starts the stop watch. The initial time is set to current clock
    // value.
    void DoStart();

    // Returns the current clock value in its native units.
    std::int64_t GetCurrentClockValue() const;

    // Return the frequency of the clock used in its ticks per second.
    std::int64_t GetClockFreq() const;


    // The clock value when the stop watch was last started. Its units vary
    // depending on the platform.
    std::int64_t m_t0;

    // The elapsed time as of last Pause() call (only valid if m_pauseCount >
    // 0) in the same units as m_t0.
    std::int64_t m_elapsedBeforePause;

    // if > 0, the stop watch is paused, otherwise it is running
    int m_pauseCount{0};
};

} // export

#endif // wxUSE_STOPWATCH
