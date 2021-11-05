///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/power.cpp
// Purpose:     power management functions for MSW
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2006-05-27
// Copyright:   (c) 2006 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"
#include "wx/power.h"

#include <atomic>

// ----------------------------------------------------------------------------
// wxPowerResource
// ----------------------------------------------------------------------------

namespace
{

std::atomic_int g_powerResourceScreenRefCount{};
std::atomic_int g_powerResourceSystemRefCount{};

bool UpdatePowerResourceExecutionState()
{
    EXECUTION_STATE executionState = ES_CONTINUOUS;
    if ( g_powerResourceScreenRefCount > 0 )
        executionState |= ES_DISPLAY_REQUIRED;

    if ( g_powerResourceSystemRefCount > 0 )
        executionState |= ES_SYSTEM_REQUIRED;

    if ( ::SetThreadExecutionState(executionState) == 0 )
    {
        wxLogLastError("SetThreadExecutionState()");
        return false;
    }

    return true;
}

} // anonymous namespace

bool
wxPowerResource::Acquire(wxPowerResourceKind kind,
                         const wxString& WXUNUSED(reason))
{
    switch ( kind )
    {
        case wxPOWER_RESOURCE_SCREEN:
            ++g_powerResourceScreenRefCount;
            break;

        case wxPOWER_RESOURCE_SYSTEM:
            ++g_powerResourceSystemRefCount;
            break;
    }

    return UpdatePowerResourceExecutionState();
}

void wxPowerResource::Release(wxPowerResourceKind kind)
{
    switch ( kind )
    {
        case wxPOWER_RESOURCE_SCREEN:
            if ( g_powerResourceScreenRefCount > 0 )
            {
                --g_powerResourceScreenRefCount;
            }
            else
            {
                wxFAIL_MSG( "Screen power resource was not acquired" );
            }
            break;

        case wxPOWER_RESOURCE_SYSTEM:
            if ( g_powerResourceSystemRefCount > 0 )
            {
                --g_powerResourceSystemRefCount;
            }
            else
            {
                wxFAIL_MSG( "System power resource was not acquired" );
            }
            break;
    }

    UpdatePowerResourceExecutionState();
}


// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

static inline bool wxGetPowerStatus(SYSTEM_POWER_STATUS *sps)
{
    if ( !::GetSystemPowerStatus(sps) )
    {
        wxLogLastError("GetSystemPowerStatus()");
        return false;
    }

    return true;
}

// ============================================================================
// implementation
// ============================================================================

wxPowerType wxGetPowerType()
{
    SYSTEM_POWER_STATUS sps;
    if ( wxGetPowerStatus(&sps) )
    {
        switch ( sps.ACLineStatus )
        {
            case 0:
                return wxPowerType::Battery;

            case 1:
                return wxPowerType::Socket;

            default:
                wxLogDebug("Unknown ACLineStatus=%u", sps.ACLineStatus);
                [[fallthrough]];
            case 255:
                break;
        }
    }

    return wxPowerType::Unknown;
}

wxBatteryState wxGetBatteryState()
{
    SYSTEM_POWER_STATUS sps;
    if ( wxGetPowerStatus(&sps) )
    {
        // there can be other bits set in the flag field ("charging" and "no
        // battery"), extract only those which we need here
        switch ( sps.BatteryFlag & 7 )
        {
            case 1:
                return wxBatteryState::Normal;

            case 2:
                return wxBatteryState::Low;

            case 3:
                return wxBatteryState::Critical;
        }
    }

    return wxBatteryState::Unknown;
}
