/////////////////////////////////////////////////////////////////////////////
// Name:        wx/sysopt.h
// Purpose:     wxSystemOptions
// Author:      Julian Smart
// Modified by:
// Created:     2001-07-10
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SYSOPT_H_
#define _WX_SYSOPT_H_

#include "wx/object.h"

// ----------------------------------------------------------------------------
// Enables an application to influence the wxWidgets implementation
// ----------------------------------------------------------------------------

class
#if wxUSE_SYSTEM_OPTIONS
WXDLLIMPEXP_BASE
#endif
wxSystemOptions
{
public:
    // User-customizable hints to wxWidgets or associated libraries
    // These could also be used to influence GetSystem... calls, indeed
    // to implement SetSystemColour/Font/Metric

#if wxUSE_SYSTEM_OPTIONS
    static void SetOption(const wxString& name, const wxString& value);
    static void SetOption(const wxString& name, int value);
#endif // wxUSE_SYSTEM_OPTIONS
    static wxString GetOption(std::string_view name);
    static int GetOptionInt(std::string_view name);
    static bool HasOption(std::string_view name);

    static bool IsFalse(std::string_view name)
    {
        return HasOption(name) && GetOptionInt(name) == 0;
    }
};

#if !wxUSE_SYSTEM_OPTIONS

// define inline stubs for accessors to make it possible to use wxSystemOptions
// in the library itself without checking for wxUSE_SYSTEM_OPTIONS all the time

/* static */ inline
wxString wxSystemOptions::GetOption(const wxString& WXUNUSED(name))
{
    return {};
}

/* static */ inline
int wxSystemOptions::GetOptionInt(const wxString& WXUNUSED(name))
{
    return 0;
}

/* static */ inline
bool wxSystemOptions::HasOption(const wxString& WXUNUSED(name))
{
    return false;
}

#endif // !wxUSE_SYSTEM_OPTIONS

#endif
    // _WX_SYSOPT_H_

