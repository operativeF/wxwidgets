/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/statline.cpp
// Purpose:     MSW version of wxStaticLine class
// Author:      Vadim Zeitlin
// Created:     28.06.99
// Copyright:   (c) 1998 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_STATLINE

#include "wx/statline.h"

#include "wx/msw/private.h"

import WX.WinDef;

// ----------------------------------------------------------------------------
// wxStaticLine
// ----------------------------------------------------------------------------

bool wxStaticLine::Create(wxWindow *parent,
                          wxWindowID id,
                          const wxPoint& pos,
                          const wxSize& sizeOrig,
                          unsigned int style,
                          std::string_view name)
{
    wxSize size = AdjustSize(sizeOrig);

    if ( !CreateControl(parent, id, pos, size, style, wxDefaultValidator, name) )
        return false;

    return MSWCreateControl("STATIC", "", pos, size);
}

WXDWORD wxStaticLine::MSWGetStyle(unsigned int style, WXDWORD *exstyle) const
{
    // we never have border
    style &= ~wxBORDER_MASK;
    style |= wxBORDER_NONE;

    WXDWORD msStyle = wxControl::MSWGetStyle(style, exstyle);

    // add our default styles
    msStyle |= SS_SUNKEN | SS_NOTIFY | WS_CLIPSIBLINGS;
    msStyle |= SS_GRAYRECT ;

    return msStyle;
}

#endif // wxUSE_STATLINE
