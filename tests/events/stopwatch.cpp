///////////////////////////////////////////////////////////////////////////////
// Name:        tests/events/stopwatch.cpp
// Purpose:     Test wxStopWatch class
// Author:      Francesco Montorsi (extracted from console sample)
// Created:     2010-05-16
// Copyright:   (c) 2010 wxWidgets team
///////////////////////////////////////////////////////////////////////////////


#include "doctest.h"

#include "testprec.h"

#ifndef WX_PRECOMP
#endif // WX_PRECOMP

#include <time.h>

#include "wx/utils.h"

import WX.Cmn.Stopwatch;

namespace
{

const long tolerance = 50;  // in ms
const int sleepTime = 500;

} // anonymous namespace

// --------------------------------------------------------------------------
// test class
// --------------------------------------------------------------------------


TEST_CASE("Misc")
{
    // Buildbot machines are quite slow and sleep doesn't work reliably there,
    // i.e. it can sleep for much longer than requested. This is not really an
    // error, so just don't run this test there -- and if you get failures in
    // this test when running it interactively, this might also be normal if
    // the machine is under heavy load.
    if ( IsAutomaticTest() )
        return;

    wxStopWatch sw;
    long t;
    wxLongLong usec;

    sw.Pause();         // pause it immediately

    // verify that almost no time elapsed
    usec = sw.TimeInMicro();
    CHECK_MESSAGE
    (
        (usec < (tolerance * 1000)),
        ("Elapsed time was %" wxLongLongFmtSpec "dus", usec)
    );

    wxSleep(1);
    t = sw.Time();

    // check that the stop watch doesn't advance while paused
    CHECK_MESSAGE
    (
        ((t >= 0) && (t < tolerance)),
        ("Actual time value is %ld", t)
    );

    sw.Resume();
    wxMilliSleep(sleepTime);
    t = sw.Time();
    // check that it did advance now by ~1.5s
    CHECK_MESSAGE
    (
        ((t > (sleepTime - tolerance)) && (t < (2*sleepTime))),
        ("Actual time value is %ld", t)
    );

    sw.Pause();

    // check that this sleep won't be taken into account below
    wxMilliSleep(sleepTime);
    sw.Resume();

    wxMilliSleep(sleepTime);
    t = sw.Time();

    // and it should advance again
    CHECK_MESSAGE
    (
        (((t > (2*sleepTime) - tolerance) && (t < (3*sleepTime)))),
        ("Actual time value is %ld", t)
    );
}

TEST_CASE("BackwardsClockBug")
{
    wxStopWatch sw;
    wxStopWatch sw2;

    for ( size_t n = 0; n < 10; n++ )
    {
        sw2.Start();

        for ( size_t m = 0; m < 10000; m++ )
        {
            CHECK( sw.Time() >= 0 );
            CHECK( sw2.Time() >= 0 );
        }
    }
}

TEST_CASE("RestartBug")
{
    wxStopWatch sw;
    sw.Pause();

    // Calling Start() should resume the stopwatch if it was paused.
    static constexpr int offset = 5000;
    
    sw.Start(offset);
    wxMilliSleep(sleepTime);

    long t = sw.Time();
    CHECK_MESSAGE
    (
        t >= offset + sleepTime - tolerance,
        ("Actual time value is %ld", t)
    );

    // As above, this is not actually due to the fact of the test being
    // automatic but just because buildbot machines are usually pretty slow, so
    // this test often fails there simply because of the high load on them.
    if ( !IsAutomaticTest() )
    {

        CHECK_MESSAGE
        (
            t < offset + sleepTime + tolerance,
            ("Actual time value is %ld", t)
        );
    }
}
