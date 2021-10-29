/////////////////////////////////////////////////////////////////////////////
// Name:        wx/config.h
// Purpose:     wxConfig base header
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CONFIG_H_BASE_
#define _WX_CONFIG_H_BASE_

#include "wx/confbase.h"

#if wxUSE_CONFIG

// ----------------------------------------------------------------------------
// define the native wxConfigBase implementation
// ----------------------------------------------------------------------------

// Prefer using a file based configuration scheme, even under Windows.
// Registry usage is something that should be used as a last resort,
// and should be discouraged as it's not portable, not easily manipulated,
// and things like AppData folders can have data roam between systems.

#include "wx/fileconf.h"
#define wxConfig wxFileConfig

#if defined(WX_WINDOWS) && wxUSE_CONFIG_NATIVE && wxUSE_REGISTRY
    #include "wx/msw/regconf.h"
    #define wxConfig  wxRegConfig
#endif

#endif // wxUSE_CONFIG

#endif // _WX_CONFIG_H_BASE_
