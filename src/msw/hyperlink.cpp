/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/hyperlink.cpp
// Purpose:     Hyperlink control
// Author:      Rickard Westerlund
// Created:     2010-08-03
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_HYPERLINKCTRL

#include "wx/hyperlink.h"
#include "wx/app.h"

#include "wx/msw/wrapcctl.h" // include <commctrl.h> "properly"
#include "wx/msw/private.h"

#include <fmt/core.h>

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#ifndef LM_GETIDEALSIZE
    #define LM_GETIDEALSIZE (WM_USER + 0x301)
#endif

#ifndef LWS_RIGHT
    #define LWS_RIGHT 0x0020
#endif

#ifndef WC_LINK
    #define WC_LINK L"SysLink"
#endif

// ----------------------------------------------------------------------------
// wxHyperlinkCtrl
// ----------------------------------------------------------------------------

DWORD wxHyperlinkCtrl::MSWGetStyle(unsigned int style, DWORD *exstyle) const
{
    DWORD msStyle = wxControl::MSWGetStyle( style, exstyle );

    if ( style & wxHL_ALIGN_RIGHT )
        msStyle |= LWS_RIGHT;

    return msStyle;
}

void wxHyperlinkCtrl::SetURL(const std::string& url)
{
    if ( GetURL() != url )
        SetVisited( false );
    wxGenericHyperlinkCtrl::SetURL( url );
    wxWindow::SetLabel( GetLabelForSysLink(m_labelOrig, url) );
}

void wxHyperlinkCtrl::SetLabel(std::string_view label)
{
    m_labelOrig = {label.begin(), label.end()};
    wxWindow::SetLabel( GetLabelForSysLink(label, GetURL()) );
    InvalidateBestSize();
}

wxSize wxHyperlinkCtrl::DoGetBestClientSize() const
{
    SIZE idealSize;
    ::SendMessageW(m_hWnd, LM_GETIDEALSIZE, 0, (WXLPARAM)&idealSize);

    return {idealSize.cx, idealSize.cy};
}

bool wxHyperlinkCtrl::MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result)
{

    switch ( ((LPNMHDR) lParam)->code )
    {
        case NM_CLICK:
        case NM_RETURN:
            SetVisited();
            SendEvent();

            // SendEvent() launches the browser by default, so we consider
            // that the event was processed in any case, either by user
            // code or by wx itself, hence we always return true to
            // indicate that the default processing shouldn't take place.
            return true;
    }

   return wxGenericHyperlinkCtrl::MSWOnNotify(idCtrl, lParam, result);
}

#endif // wxUSE_HYPERLINKCTRL
