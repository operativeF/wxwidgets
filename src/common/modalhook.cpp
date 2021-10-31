///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/modalhook.cpp
// Purpose:     wxModalDialogHook implementation
// Author:      Vadim Zeitlin
// Created:     2013-05-19
// Copyright:   (c) 2013 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/defs.h"
#include "wx/modalhook.h"

#include <algorithm>


wxModalDialogHook::Hooks wxModalDialogHook::ms_hooks;

// ============================================================================
// wxModalDialogHook implementation
// ============================================================================

// ----------------------------------------------------------------------------
// Hooks management
// ----------------------------------------------------------------------------

void wxModalDialogHook::Register()
{
#if wxDEBUG_LEVEL
    for ( Hooks::const_iterator it = ms_hooks.begin();
          it != ms_hooks.end();
          ++it)
    {
        if ( *it == this )
        {
            wxFAIL_MSG( wxS("Registering already registered hook?") );
            return;
        }
    }
#endif // wxDEBUG_LEVEL

    ms_hooks.insert(ms_hooks.begin(), this);
}

void wxModalDialogHook::Unregister()
{
    if ( !DoUnregister() )
    {
        wxFAIL_MSG( wxS("Unregistering not registered hook?") );
    }
}

bool wxModalDialogHook::DoUnregister()
{
    const auto unregCount = std::erase_if(ms_hooks,
        [this](const auto& aHook){ return aHook == this; });

    return unregCount == 1;
}

// ----------------------------------------------------------------------------
// Invoking hooks methods
// ----------------------------------------------------------------------------

/* static */
int wxModalDialogHook::CallEnter(wxDialog* dialog)
{
    // Make a copy of the hooks list to avoid problems if it's modified while
    // we're iterating over it: this is unlikely to happen in our case, but
    // quite possible in CallExit() as the hooks may remove themselves after
    // the call to their Exit(), so do it here for symmetry as well.
    const Hooks hooks = ms_hooks;

    for ( const auto& hook : hooks )
    {
        const int rc = hook->Enter(dialog);
        if ( rc != wxID_NONE )
        {
            // Skip calling all the rest of the hooks if one of them preempts
            // showing the dialog completely.
            return rc;
        }
    }

    return wxID_NONE;
}

/* static */
void wxModalDialogHook::CallExit(wxDialog* dialog)
{
    // See comment in CallEnter() for the reasons for making a copy here.
    const Hooks hooks = ms_hooks;

    for ( const auto& hook : ms_hooks )
    {
        hook->Exit(dialog);
    }
}
