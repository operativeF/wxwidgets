///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/datecontrols.cpp
// Purpose:     implementation of date controls helper functions
// Author:      Vadim Zeitlin
// Created:     2008-04-04
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_DATEPICKCTRL || wxUSE_CALENDARCTRL

#include "wx/log.h"

#include "wx/msw/wrapcctl.h"

#include "wx/msw/private/datecontrols.h"

#include "wx/dynlib.h"

// ============================================================================
// implementation
// ============================================================================

bool wxMSWDateControls::CheckInitialization()
{
    // although we already call InitCommonControls() in app.cpp which is
    // supposed to initialize all common controls, in comctl32.dll 4.72 (and
    // presumably earlier versions 4.70 and 4.71, date time picker not being
    // supported in < 4.70 anyhow) it does not do it and we have to initialize
    // it explicitly

    // this is initially set to -1 to indicate that we need to perform the
    // initialization and gets set to false or true depending on its result
    static int s_initResult = -1; // MT-ok: used from GUI thread only
    if ( s_initResult == -1 )
    {
        // in any case do nothing the next time, the result won't change and
        // it's enough to give the error only once
        s_initResult = false;

#if wxUSE_DYNLIB_CLASS
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(icex);
        icex.dwICC = ICC_DATE_CLASSES;

        // see comment in wxApp::GetComCtl32Version() explaining the
        // use of wxLoadedDLL
        wxLoadedDLL dllComCtl32("comctl32.dll");
        if ( dllComCtl32.IsLoaded() )
        {
            wxLogNull noLog;

            using ICCEx_t = BOOL (WINAPI*)(INITCOMMONCONTROLSEX *);
            wxDYNLIB_FUNCTION( ICCEx_t, InitCommonControlsEx, dllComCtl32 );

            if ( pfnInitCommonControlsEx )
            {
                s_initResult = (*pfnInitCommonControlsEx)(&icex);
            }
        }
#endif // wxUSE_DYNLIB_CLASS
    }

    return s_initResult != 0;
}

#endif // wxUSE_DATEPICKCTRL || wxUSE_CALENDARCTRL
