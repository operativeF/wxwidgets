#ifndef WINPRECOMP_H
#define WINPRECOMP_H

#define OEMRESOURCE

#include "wx/platform.h"

// before including windows.h, define version macros at (currently) maximal
// values because we do all our checks at run-time anyhow
#include "wx/msw/winver.h"

// strict type checking to detect conversion from HFOO to HBAR at compile-time
#ifndef STRICT
    #define STRICT 1
#endif

// When the application wants to use <winsock2.h> (this is required for IPv6
// support, for example), we must include it before winsock.h, and as windows.h
// includes winsock.h, we have to do it before including it.
#if wxUSE_WINSOCK2
    // Avoid warnings about Winsock 1.x functions deprecated in Winsock 2 that
    // we still use (and that will certainly remain available for the
    // foreseeable future anyhow).
    #ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
        #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #endif
    #include <winsock2.h>
#endif

#include <windows.h>

#include <commctrl.h>

#include <ole2.h>
#include <ole2ver.h>
#include <oleidl.h>
#include <oleacc.h>
#include <oleauto.h>

#include <shellapi.h>

#endif // WINPRECOMP_H
