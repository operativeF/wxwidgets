/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/hyperlnkcmn.cpp
// Purpose:     Hyperlink control
// Author:      David Norris <danorris@gmail.com>, Otto Wyss
// Modified by: Ryan Norton, Francesco Montorsi
// Created:     04/02/2005
// Copyright:   (c) 2005 David Norris
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_HYPERLINKCTRL

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

#include "wx/hyperlink.h"
#include "wx/log.h"
#include "wx/utils.h"

// ============================================================================
// implementation
// ============================================================================

wxDEFINE_FLAGS( wxHyperlinkStyle )
wxBEGIN_FLAGS( wxHyperlinkStyle )
// new style border flags, we put them first to
// use them for streaming out
wxFLAGS_MEMBER(wxBORDER_SIMPLE)
wxFLAGS_MEMBER(wxBORDER_SUNKEN)
wxFLAGS_MEMBER(wxBORDER_DOUBLE)
wxFLAGS_MEMBER(wxBORDER_RAISED)
wxFLAGS_MEMBER(wxBORDER_STATIC)
wxFLAGS_MEMBER(wxBORDER_NONE)

// old style border flags
wxFLAGS_MEMBER(wxSIMPLE_BORDER)
wxFLAGS_MEMBER(wxSUNKEN_BORDER)
wxFLAGS_MEMBER(wxDOUBLE_BORDER)
wxFLAGS_MEMBER(wxRAISED_BORDER)
wxFLAGS_MEMBER(wxSTATIC_BORDER)
wxFLAGS_MEMBER(wxBORDER)

// standard window styles
wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
wxFLAGS_MEMBER(wxCLIP_CHILDREN)
wxFLAGS_MEMBER(wxTRANSPARENT_WINDOW)
wxFLAGS_MEMBER(wxWANTS_CHARS)
wxFLAGS_MEMBER(wxFULL_REPAINT_ON_RESIZE)
wxFLAGS_MEMBER(wxALWAYS_SHOW_SB )
wxFLAGS_MEMBER(wxVSCROLL)
wxFLAGS_MEMBER(wxHSCROLL)

wxFLAGS_MEMBER(wxHL_CONTEXTMENU)
wxFLAGS_MEMBER(wxHL_ALIGN_LEFT)
wxFLAGS_MEMBER(wxHL_ALIGN_RIGHT)
wxFLAGS_MEMBER(wxHL_ALIGN_CENTRE)
wxEND_FLAGS( wxHyperlinkStyle )

wxDEFINE_EVENT( wxEVT_HYPERLINK, wxHyperlinkEvent );

// ----------------------------------------------------------------------------
// wxHyperlinkCtrlBase
// ----------------------------------------------------------------------------

void
wxHyperlinkCtrlBase::CheckParams(const std::string& label,
                                 const std::string& url,
                                 unsigned int style)
{
#if wxDEBUG_LEVEL
    wxASSERT_MSG(!url.empty() || !label.empty(),
                 "Both URL and label are empty ?");

    int alignment = (int)((style & wxHL_ALIGN_LEFT) != 0) +
                    (int)((style & wxHL_ALIGN_CENTRE) != 0) +
                    (int)((style & wxHL_ALIGN_RIGHT) != 0);
    wxASSERT_MSG(alignment == 1,
        "Specify exactly one align flag!");
#else // !wxDEBUG_LEVEL
    wxUnusedVar(label);
    wxUnusedVar(url);
    wxUnusedVar(style);
#endif // wxDEBUG_LEVEL/!wxDEBUG_LEVEL
}

void wxHyperlinkCtrlBase::SendEvent()
{
    const std::string& url = GetURL();
    wxHyperlinkEvent linkEvent(this, GetId(), url);
    if (!GetEventHandler()->ProcessEvent(linkEvent))     // was the event skipped ?
    {
        if (!wxLaunchDefaultBrowser(url))
        {
            wxLogWarning(_("Failed to open URL \"%s\" in the default browser"), url);
        }
    }
}

#endif // wxUSE_HYPERLINKCTRL
