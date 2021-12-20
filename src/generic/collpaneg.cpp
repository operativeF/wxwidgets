/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/collpaneg.cpp
// Purpose:     wxGenericCollapsiblePane
// Author:      Francesco Montorsi
// Modified By:
// Created:     8/10/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_COLLPANE && wxUSE_BUTTON && wxUSE_STATLINE

#include "wx/collpane.h"

#include "wx/toplevel.h"
#include "wx/panel.h"

#include "wx/collheaderctrl.h"

import WX.Core.Sizer;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// ============================================================================
// implementation
// ============================================================================

//-----------------------------------------------------------------------------
// wxGenericCollapsiblePane
//-----------------------------------------------------------------------------

wxDEFINE_EVENT( wxEVT_COLLAPSIBLEPANE_CHANGED, wxCollapsiblePaneEvent );

wxBEGIN_EVENT_TABLE(wxGenericCollapsiblePane, wxControl)
    EVT_COLLAPSIBLEHEADER_CHANGED(wxID_ANY, wxGenericCollapsiblePane::OnButton)
    EVT_SIZE(wxGenericCollapsiblePane::OnSize)
wxEND_EVENT_TABLE()



bool wxGenericCollapsiblePane::Create(wxWindow *parent,
                                      wxWindowID id,
                                      std::string_view label,
                                      const wxPoint& pos,
                                      const wxSize& size,
                                      unsigned int style,
                                      const wxValidator& val,
                                      std::string_view name)
{
    if ( !wxControl::Create(parent, id, pos, size, style, val, name) )
        return false;

    // sizer containing the expand button and possibly a static line
    m_sz = new wxBoxSizer(wxVERTICAL);

    // create children and lay them out using a wxBoxSizer
    // (so that we automatically get RTL features)
    m_pButton = new wxCollapsibleHeaderCtrl(this, wxID_ANY, label, wxPoint(0, 0),
                             wxDefaultSize);

    m_sz->Add(m_pButton, wxSizerFlags().Border(wxALL, GetBorder()));

    // FIXME: at least under wxGTK1 the background is black if we don't do
    //        this, no idea why...
#if defined(__WXGTK__)
    SetBackgroundColour(parent->GetBackgroundColour());
#endif

    // do not set sz as our sizers since we handle the pane window without using sizers
    m_pPane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                          wxTAB_TRAVERSAL|wxNO_BORDER, "wxCollapsiblePanePane");

    // start as collapsed:
    m_pPane->Hide();

    return true;
}

wxGenericCollapsiblePane::~wxGenericCollapsiblePane()
{
    if (m_pButton)
        m_pButton->SetContainingSizer(nullptr);

    // our sizer is not deleted automatically since we didn't use SetSizer()!
    wxDELETE(m_sz);
}

wxSize wxGenericCollapsiblePane::DoGetBestClientSize() const
{
    // NB: do not use GetSize() but rather GetMinSize()
    wxSize sz = m_sz->GetMinSize();

    // when expanded, we need more vertical space
    if ( IsExpanded() )
    {
        sz.x = std::max( sz.x, m_pPane->GetBestSize().x );
        sz.y = sz.y + GetBorder() + m_pPane->GetBestSize().y;
    }

    return sz;
}

void wxGenericCollapsiblePane::OnStateChange(const wxSize& sz)
{
    // minimal size has priority over the best size so set here our min size
//    SetMinSize(sz);
    SetSize(sz);

    if (this->HasFlag(wxCP_NO_TLW_RESIZE))
    {
        // the user asked to explicitly handle the resizing itself...
        return;
    }


    wxTopLevelWindow *top =
        wxDynamicCast(wxGetTopLevelParent(this), wxTopLevelWindow);
    if ( !top )
        return;

    wxSizer *sizer = top->GetSizer();
    if ( !sizer )
        return;

    const wxSize newBestSize = sizer->ComputeFittingClientSize(top);
    top->SetMinClientSize(newBestSize);

    // we shouldn't attempt to resize a maximized window, whatever happens
    if ( !top->wxIsMaximized() )
        top->SetClientSize(newBestSize);
}

void wxGenericCollapsiblePane::Collapse(bool collapse)
{
    // optimization
    if ( IsCollapsed() == collapse )
        return;

    InvalidateBestSize();

    // update our state
    m_pPane->Show(!collapse);

    // update button
    // NB: this must be done after updating our "state"
    m_pButton->SetCollapsed(collapse);

    OnStateChange(GetBestSize());
}

void wxGenericCollapsiblePane::SetLabel(std::string_view label)
{
    m_pButton->SetLabel(label);
    m_pButton->SetInitialSize();

    Layout();
}

std::string wxGenericCollapsiblePane::GetLabel() const
{
    return m_pButton->GetLabel();
}

bool wxGenericCollapsiblePane::Layout()
{
    if (!m_pButton || !m_pPane || !m_sz)
        return false;     // we need to complete the creation first!

    wxSize oursz(GetSize());

    // move & resize the button and the static line
    m_sz->SetDimension(0, 0, oursz.x, m_sz->GetMinSize().y);
    m_sz->Layout();

    if ( IsExpanded() )
    {
        // move & resize the container window
        int yoffset = m_sz->GetSize().y + GetBorder();
        m_pPane->SetSize(wxRect{0, yoffset, oursz.x, oursz.y - yoffset});

        // this is very important to make the pane window layout show correctly
        m_pPane->Layout();
    }

    return true;
}

int wxGenericCollapsiblePane::GetBorder() const
{
#if defined( __WXMAC__ )
    return 6;
#elif defined(__WXMSW__)
    wxASSERT(m_pButton);
    return m_pButton->ConvertDialogToPixels(wxSize(2, 0)).x;
#else
    return 5;
#endif
}



//-----------------------------------------------------------------------------
// wxGenericCollapsiblePane - event handlers
//-----------------------------------------------------------------------------

void wxGenericCollapsiblePane::OnButton(wxCommandEvent& event)
{
    if ( event.GetEventObject() != m_pButton )
    {
        event.Skip();
        return;
    }

    Collapse(!IsCollapsed());

    // this change was generated by the user - send the event
    wxCollapsiblePaneEvent ev(this, GetId(), IsCollapsed());
    GetEventHandler()->ProcessEvent(ev);
}

void wxGenericCollapsiblePane::OnSize([[maybe_unused]] wxSizeEvent& event)
{
#if 0       // for debug only
    wxClientDC dc(this);
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(wxPoint(0,0), GetSize());
    dc.SetPen(*wxRED_PEN);
    dc.DrawRectangle(wxPoint(0,0), GetBestSize());
#endif

    Layout();
}

#endif // wxUSE_COLLPANE && wxUSE_BUTTON && wxUSE_STATLINE
