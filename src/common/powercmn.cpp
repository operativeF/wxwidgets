///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/powercmn.cpp
// Purpose:     power event types and stubs for power functions
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2006-05-27
// Copyright:   (c) 2006 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/power.h"

// ============================================================================
// implementation
// ============================================================================

#ifdef wxHAS_POWER_EVENTS
    wxDEFINE_EVENT( wxEVT_POWER_SUSPENDING, wxPowerEvent );
    wxDEFINE_EVENT( wxEVT_POWER_SUSPENDED, wxPowerEvent );
    wxDEFINE_EVENT( wxEVT_POWER_SUSPEND_CANCEL, wxPowerEvent );
    wxDEFINE_EVENT( wxEVT_POWER_RESUME, wxPowerEvent );

    wxIMPLEMENT_DYNAMIC_CLASS(wxPowerEvent, wxEvent);
#endif

// Provide stubs for systems without power resource management functions
#if !defined(WX_WINDOWS) && !defined(__APPLE__)

bool
wxPowerResource::Acquire(wxPowerResourceKind WXUNUSED(kind),
                         const wxString& WXUNUSED(reason))
{
    return false;
}

void wxPowerResource::Release(wxPowerResourceKind WXUNUSED(kind))
{
}

#endif // !(WX_WINDOWS || __APPLE__)

// provide stubs for the systems not implementing these functions
#if !defined(WX_WINDOWS)

wxPowerType wxGetPowerType()
{
    return wxPowerType::Unknown;
}

wxBatteryState wxGetBatteryState()
{
    return wxBatteryState::Unknown;
}

#endif // systems without power management functions

