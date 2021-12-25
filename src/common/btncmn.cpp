///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/btncmn.cpp
// Purpose:     implementation of wxButtonBase
// Author:      Vadim Zeitlin
// Created:     2007-04-08
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_BUTTON

#include "wx/button.h"
#include "wx/toplevel.h"

import WX.Cfg.Flags;

// ============================================================================
// implementation
// ============================================================================

wxWindow *wxButtonBase::SetDefault()
{
    wxTopLevelWindow * const
        tlw = dynamic_cast<wxTopLevelWindow*>(wxGetTopLevelParent(this));

    wxCHECK_MSG( tlw, nullptr, "button without top level window?");

    return tlw->SetDefaultItem(this);
}

// FIXME: wxDirection should be enum class.
void wxAnyButtonBase::SetBitmapPosition(wxDirection dir)
{
    wxASSERT_MSG( !(dir & ~wxDIRECTION_MASK), "non-direction flag used" );
    wxASSERT_MSG( !!(dir & wxDirection::wxLEFT) +
                    !!(dir & wxDirection::wxRIGHT) +
                      !!(dir & wxDirection::wxTOP) +
                       !!(dir & wxDirection::wxBOTTOM) == 1,
                   "exactly one direction flag must be set" );

    DoSetBitmapPosition(dir);

}
#endif // wxUSE_BUTTON
