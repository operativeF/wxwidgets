/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/hyperlink.cpp
// Purpose:     Hyperlink control
// Author:      Rickard Westerlund
// Created:     2010-08-03
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_HYPERLINKCTRL

#include "wx/hyperlink.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h" // include <commctrl.h> "properly"
    #include "wx/msw/private.h"
    
    #include <fmt/core.h>
#endif

#include "wx/app.h"

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
// Helper functions
// ----------------------------------------------------------------------------

namespace
{
    bool HasNativeHyperlinkCtrl()
    {
        // Notice that we really must test comctl32.dll version and not the OS
        // version here as even under Vista/7 we could be not using the v6 e.g.
        // if the program doesn't have the correct manifest for some reason.
        return wxApp::GetComCtl32Version() >= 600;
    }

    std::string GetLabelForSysLink(const std::string& text, const std::string& url)
    {
        // Any "&"s in the text should appear on the screen and not be (mis)
        // interpreted as mnemonics.
        return fmt::format("<A HREF=\"{:s}\">{:s}</A>",
                                url,
                                wxControl::EscapeMnemonics(text));
    }
}

// ----------------------------------------------------------------------------
// wxHyperlinkCtrl
// ----------------------------------------------------------------------------

bool wxHyperlinkCtrl::Create(wxWindow *parent,
                             wxWindowID id,
                             const std::string& label,
                             const std::string& url,
                             const wxPoint& pos,
                             const wxSize& size,
                             unsigned int style,
                             const std::string& name)
{
    if ( !HasNativeHyperlinkCtrl() )
    {
        return wxGenericHyperlinkCtrl::Create( parent, id, label, url, pos,
                                               size, style, name );
    }

    if ( !CreateControl(parent, id, pos, size, style,
                        wxDefaultValidator, name) )
    {
        return false;
    }

    SetURL( url );
    SetVisited( false );

    DWORD exstyle;
    DWORD msStyle = MSWGetStyle(style, &exstyle);

    // "SysLink" would be WC_LINK but it's a wide-string
    if ( !MSWCreateControl("SysLink", msStyle, pos, size,
                           GetLabelForSysLink( label, url ), exstyle) )
    {
        return false;
    }

    // Make sure both the label and URL are non-empty strings.
    SetURL(url.empty() ? label : url);
    SetLabel(label.empty() ? url : label);

    ConnectMenuHandlers();

    return true;
}

DWORD wxHyperlinkCtrl::MSWGetStyle(unsigned int style, DWORD *exstyle) const
{
    DWORD msStyle = wxControl::MSWGetStyle( style, exstyle );

    if ( style & wxHL_ALIGN_RIGHT )
        msStyle |= LWS_RIGHT;

    return msStyle;
}

void wxHyperlinkCtrl::SetURL(const std::string& url)
{
    if ( !HasNativeHyperlinkCtrl() )
    {
        wxGenericHyperlinkCtrl::SetURL( url );
        return;
    }

    if ( GetURL() != url )
        SetVisited( false );
    wxGenericHyperlinkCtrl::SetURL( url );
    wxWindow::SetLabel( GetLabelForSysLink(m_labelOrig, url) );
}

void wxHyperlinkCtrl::SetLabel(const std::string& label)
{
    if ( !HasNativeHyperlinkCtrl() )
    {
        wxGenericHyperlinkCtrl::SetLabel( label );
        return;
    }

    m_labelOrig = label;
    wxWindow::SetLabel( GetLabelForSysLink(label, GetURL()) );
    InvalidateBestSize();
}

wxSize wxHyperlinkCtrl::DoGetBestClientSize() const
{
    // LM_GETIDEALSIZE only exists under Vista so use the generic version even
    // when using the native control under XP
    if ( !HasNativeHyperlinkCtrl() || (wxGetWinVersion() < wxWinVersion_6) )
        return wxGenericHyperlinkCtrl::DoGetBestClientSize();

    SIZE idealSize;
    ::SendMessageW(m_hWnd, LM_GETIDEALSIZE, 0, (LPARAM)&idealSize);

    return {idealSize.cx, idealSize.cy};
}

bool wxHyperlinkCtrl::MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result)
{
    if ( HasNativeHyperlinkCtrl() )
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
    }

   return wxGenericHyperlinkCtrl::MSWOnNotify(idCtrl, lParam, result);
}

#endif // wxUSE_HYPERLINKCTRL
