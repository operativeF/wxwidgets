///////////////////////////////////////////////////////////////////////////////
// Name:        wx/activityindicator.h
// Purpose:     wxActivityIndicator declaration.
// Author:      Vadim Zeitlin
// Created:     2015-03-05
// Copyright:   (c) 2015 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ACTIVITYINDICATOR_H_
#define _WX_ACTIVITYINDICATOR_H_

#if wxUSE_ACTIVITYINDICATOR

#include "wx/defs.h"

#include "wx/control.h"

import WX.Cfg.Flags;

inline constexpr std::string_view wxActivityIndicatorNameStr = "activityindicator";

// ----------------------------------------------------------------------------
// wxActivityIndicator: small animated indicator of some application activity.
// ----------------------------------------------------------------------------

class wxActivityIndicatorBase : public wxControl
{
public:
    // Start or stop the activity animation (it is stopped initially).
    virtual void Start() = 0;
    virtual void Stop() = 0;

    // Return true if the control is currently showing activity.
    virtual bool IsRunning() const = 0;

    // Override some base class virtual methods.
    bool AcceptsFocus() const override { return false; }
    bool HasTransparentBackground() override { return true; }

protected:
    // choose the default border for this window
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }
};

#ifndef __WXUNIVERSAL__
#if defined(__WXGTK220__)
    #define wxHAS_NATIVE_ACTIVITYINDICATOR
    #include "wx/gtk/activityindicator.h"
#elif defined(__WXOSX_COCOA__)
    #define wxHAS_NATIVE_ACTIVITYINDICATOR
    #include "wx/osx/activityindicator.h"
#endif
#endif // !__WXUNIVERSAL__

#ifndef wxHAS_NATIVE_ACTIVITYINDICATOR
    #include "wx/generic/activityindicator.h"
#endif

#endif // wxUSE_ACTIVITYINDICATOR

#endif // _WX_ACTIVITYINDICATOR_H_
