/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/choiccmn.cpp
// Purpose:     common (to all ports) wxChoice functions
// Author:      Vadim Zeitlin
// Modified by:
// Created:     26.07.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_CHOICE

#include "wx/choice.h"
#include "wx/private/textmeasure.h"

// ============================================================================
// implementation
// ============================================================================

wxSize wxChoiceBase::DoGetBestSize() const
{
    // a reasonable width for an empty choice list
    wxSize best(FromDIP(80), -1);

    const unsigned int nItems = GetCount();
    if ( nItems > 0 )
    {
        wxTextMeasure txm(this);
        best.x = txm.GetLargestStringExtent(GetStrings()).x;
    }

    return best;
}

// ----------------------------------------------------------------------------
// misc
// ----------------------------------------------------------------------------

void wxChoiceBase::Command(wxCommandEvent& event)
{
    SetSelection(event.GetInt());
    GetEventHandler()->ProcessEvent(event);
}

#endif // wxUSE_CHOICE
