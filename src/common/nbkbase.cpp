///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/nbkbase.cpp
// Purpose:     common wxNotebook methods
// Author:      Vadim Zeitlin
// Modified by:
// Created:     02.07.01
// Copyright:   (c) 2001 Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_NOTEBOOK

#include "wx/notebook.h"

// ============================================================================
// implementation
// ============================================================================

wxDEFINE_EVENT( wxEVT_NOTEBOOK_PAGE_CHANGED, wxBookCtrlEvent );
wxDEFINE_EVENT( wxEVT_NOTEBOOK_PAGE_CHANGING, wxBookCtrlEvent );

// ----------------------------------------------------------------------------
// XTI
// ----------------------------------------------------------------------------

#include "wx/listimpl.cpp"
wxDEFINE_FLAGS( wxNotebookStyle )
wxBEGIN_FLAGS( wxNotebookStyle )
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

wxFLAGS_MEMBER(wxNB_FIXEDWIDTH)
wxFLAGS_MEMBER(wxBK_DEFAULT)
wxFLAGS_MEMBER(wxBK_TOP)
wxFLAGS_MEMBER(wxBK_LEFT)
wxFLAGS_MEMBER(wxBK_RIGHT)
wxFLAGS_MEMBER(wxBK_BOTTOM)
wxFLAGS_MEMBER(wxNB_NOPAGETHEME)
wxEND_FLAGS( wxNotebookStyle )

// ----------------------------------------------------------------------------
// geometry
// ----------------------------------------------------------------------------

wxSize wxNotebookBase::CalcSizeFromPage(const wxSize& sizePage) const
{
    // this is, of course, totally bogus -- but we must do something by
    // default because not all ports implement this
    wxSize sizeTotal = sizePage;

    if ( HasFlag(wxBK_LEFT) || HasFlag(wxBK_RIGHT) )
    {
        sizeTotal.x += 90;
        sizeTotal.y += 10;
    }
    else // tabs on top/bottom side
    {
        sizeTotal.x += 10;
        sizeTotal.y += 40;
    }

    return sizeTotal;
}

// ----------------------------------------------------------------------------
// events
// ----------------------------------------------------------------------------

bool wxNotebookBase::SendPageChangingEvent(int nPage)
{
    wxBookCtrlEvent event(wxEVT_NOTEBOOK_PAGE_CHANGING, GetId());
    event.SetSelection(nPage);
    event.SetOldSelection(GetSelection());
    event.SetEventObject(this);
    return !GetEventHandler()->ProcessEvent(event) || event.IsAllowed();
}

void wxNotebookBase::SendPageChangedEvent(int nPageOld, int nPageNew)
{
    wxBookCtrlEvent event(wxEVT_NOTEBOOK_PAGE_CHANGED, GetId());
    event.SetSelection(nPageNew == -1 ? GetSelection() : nPageNew);
    event.SetOldSelection(nPageOld);
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event);
}

#endif // wxUSE_NOTEBOOK
