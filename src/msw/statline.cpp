/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/statline.cpp
// Purpose:     MSW version of wxStaticLine class
// Author:      Vadim Zeitlin
// Created:     28.06.99
// Copyright:   (c) 1998 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_STATLINE

#include "wx/statline.h"

#ifndef WX_PRECOMP
    #include "wx/msw/private.h"
#endif

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxStaticLine
// ----------------------------------------------------------------------------

bool wxStaticLine::Create(wxWindow *parent,
                          wxWindowID id,
                          const wxPoint& pos,
                          const wxSize& sizeOrig,
                          unsigned int style,
                          const std::string& name)
{
    wxSize size = AdjustSize(sizeOrig);

    if ( !CreateControl(parent, id, pos, size, style, wxDefaultValidator, name) )
        return false;

    return MSWCreateControl("STATIC", "", pos, size);
}

DWORD wxStaticLine::MSWGetStyle(unsigned int style, DWORD *exstyle) const
{
    // we never have border
    style &= ~wxBORDER_MASK;
    style |= wxBORDER_NONE;

    DWORD msStyle = wxControl::MSWGetStyle(style, exstyle);

    // add our default styles
    msStyle |= SS_SUNKEN | SS_NOTIFY | WS_CLIPSIBLINGS;
    msStyle |= SS_GRAYRECT ;

    return msStyle;
}

#endif // wxUSE_STATLINE
