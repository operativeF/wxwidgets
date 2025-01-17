///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/metrics.h
// Purpose:     various helper functions to retrieve system metrics
// Author:      Vadim Zeitlin
// Created:     2008-09-05
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PRIVATE_METRICS_H_
#define _WX_MSW_PRIVATE_METRICS_H_

#include "wx/msw/private.h"

namespace wxMSWImpl
{

// return NONCLIENTMETRICSW as retrieved by SystemParametersInfo()
//
// currently this is not cached as the values may change when system settings
// do and we don't react to this to invalidate the cache but it could be done
// in the future
//
// MT-safety: this function is only meant to be called from the main thread
inline const NONCLIENTMETRICSW GetNonClientMetrics(const wxWindow* win)
{
    WinStruct<NONCLIENTMETRICSW> nm;
    if ( !wxSystemParametersInfo(SPI_GETNONCLIENTMETRICS,
                                 sizeof(NONCLIENTMETRICSW),
                                 &nm,
                                 0,
                                 win) )
    {
#if WINVER >= 0x0600
        // a new field has been added to NONCLIENTMETRICSW under Vista, so
        // the call to SystemParametersInfo() fails if we use the struct
        // size incorporating this new value on an older system -- retry
        // without it
        nm.cbSize -= sizeof(int);
        if ( !wxSystemParametersInfo(SPI_GETNONCLIENTMETRICS,
                                     sizeof(NONCLIENTMETRICSW),
                                     &nm,
                                     0,
                                     win) )
#endif // WINVER >= 0x0600
        {
            // maybe we should initialize the struct with some defaults?
            wxLogLastError("SystemParametersInfo(SPI_GETNONCLIENTMETRICS)");
        }
    }

    return nm;
}

} // namespace wxMSWImpl

#endif // _WX_MSW_PRIVATE_METRICS_H_

