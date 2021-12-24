///////////////////////////////////////////////////////////////////////////////
// Name:        wx/time.h
// Purpose:     Miscellaneous time-related functions.
// Author:      Vadim Zeitlin
// Created:     2011-11-26
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/longlong.h"

export module WX.Cmn.Time;

import WX.Utils.Cast;

export
{

// Returns the difference between UTC and local time in seconds.
int wxGetTimeZone();

// Get number of seconds since local time 00:00:00 Jan 1st 1970.
extern std::int32_t wxGetLocalTime();

// Get number of seconds since GMT 00:00:00, Jan 1st 1970.
extern std::int32_t wxGetUTCTime();

#if wxUSE_LONGLONG
    using wxMilliClock_t = std::int64_t;
    inline std::int32_t wxMilliClockToLong(std::int64_t ll) { return wx::narrow_cast<std::int32_t>(ll); }
#else
    using wxMilliClock_t = double;
    inline long wxMilliClockToLong(double d) { return wx::narrow_cast<long>(d); }
#endif // wxUSE_LONGLONG

// Get number of milliseconds since local time 00:00:00 Jan 1st 1970
extern wxMilliClock_t wxGetLocalTimeMillis();

#if wxUSE_LONGLONG

// Get the number of milliseconds or microseconds since the Epoch.
std::int64_t wxGetUTCTimeMillis();
std::int64_t wxGetUTCTimeUSec();

#endif // wxUSE_LONGLONG

/* Two wrapper functions for thread safety */
#ifdef HAVE_LOCALTIME_R
using wxLocaltime_r = localtime_r;
#else
struct tm *wxLocaltime_r(const time_t*, struct tm*);
#if wxUSE_THREADS && !defined(WX_WINDOWS)
     // On Windows, localtime _is_ threadsafe!
#warning using pseudo thread-safe wrapper for localtime to emulate localtime_r
#endif
#endif

#ifdef HAVE_GMTIME_R
using wxGmtime_r = gmtime_r;
#else
struct tm *wxGmtime_r(const time_t*, struct tm*);
#if wxUSE_THREADS && !defined(WX_WINDOWS)
     // On Windows, gmtime _is_ threadsafe!
#warning using pseudo thread-safe wrapper for gmtime to emulate gmtime_r
#endif
#endif

} // export
