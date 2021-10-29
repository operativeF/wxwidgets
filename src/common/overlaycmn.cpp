/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/overlaycmn.cpp
// Purpose:     common wxOverlay code
// Author:      Stefan Csomor
// Modified by:
// Created:     2006-10-20
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/dc.h"
#include "wx/overlay.h"
#include "wx/private/overlay.h"
#include "wx/dcmemory.h"

#include <memory>

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxOverlay
// ----------------------------------------------------------------------------

wxOverlay::wxOverlay()
    : m_impl(std::make_unique<wxOverlayImpl>())
{
}

bool wxOverlay::IsOk()
{
    return m_impl->IsOk();
}

void wxOverlay::Init( wxDC* dc, wxRect boundary )
{
    m_impl->Init(dc, boundary);
}

void wxOverlay::BeginDrawing( wxDC* dc)
{
    m_impl->BeginDrawing(dc);
    m_inDrawing = true ;
}

void wxOverlay::EndDrawing( wxDC* dc)
{
    m_impl->EndDrawing(dc);
    m_inDrawing = false ;
}

void wxOverlay::Clear( wxDC* dc)
{
    m_impl->Clear(dc);
}

void wxOverlay::Reset()
{
    wxASSERT_MSG(m_inDrawing == false, wxT("cannot reset overlay during drawing"));
    m_impl->Reset();
}


// ----------------------------------------------------------------------------
// wxDCOverlay
// ----------------------------------------------------------------------------

wxDCOverlay::wxDCOverlay(wxOverlay &overlay, wxDC *dc, wxRect boundary) :
    m_overlay(overlay)
{
    Init(dc, boundary);
}

wxDCOverlay::wxDCOverlay(wxOverlay &overlay, wxDC *dc) :
    m_overlay(overlay)
{
    const wxSize size(dc->GetSize());

    const wxCoord logicalLeft   = dc->DeviceToLogicalX(0);
    const wxCoord logicalTop    = dc->DeviceToLogicalY(0);
    const wxCoord logicalRight  = dc->DeviceToLogicalX(size.x);
    const wxCoord logicalBottom = dc->DeviceToLogicalY(size.y);

    Init(dc,
         wxRect{logicalLeft,
                logicalTop,
                logicalRight - logicalLeft,
                logicalBottom - logicalTop});
}

wxDCOverlay::~wxDCOverlay()
{
    m_overlay.EndDrawing(m_dc);
}

void wxDCOverlay::Init(wxDC *dc, wxRect boundary )
{
    m_dc = dc ;
    if ( !m_overlay.IsOk() )
    {
        m_overlay.Init(dc, boundary);
    }
    m_overlay.BeginDrawing(dc);
}

void wxDCOverlay::Clear()
{
    m_overlay.Clear(m_dc);
}

// ----------------------------------------------------------------------------
// generic implementation of wxOverlayImpl
// ----------------------------------------------------------------------------

#ifndef wxHAS_NATIVE_OVERLAY

bool wxOverlayImpl::IsOk()
{
    return m_bmpSaved.IsOk() ;
}

void wxOverlayImpl::Init( wxDC* dc, wxRect boundary )
{
    m_window = dc->GetWindow();
    wxMemoryDC dcMem ;
    m_bmpSaved.Create( boundary.GetSize() );
    dcMem.SelectObject( m_bmpSaved );
    m_x = boundary.x;
    m_y = boundary.y;
    m_width = boundary.width;
    m_height = boundary.height;
    dcMem.Blit(wxPoint{0, 0}, wxSize{m_width, m_height},
        dc, boundary.GetPosition());
    dcMem.SelectObject( wxNullBitmap );
}

void wxOverlayImpl::Clear(wxDC* dc)
{
    wxMemoryDC dcMem ;
    dcMem.SelectObject( m_bmpSaved );
    dc->Blit( wxPoint{m_x, m_y}, wxSize{m_width, m_height}, &dcMem , wxPoint{0, 0} );
    dcMem.SelectObject( wxNullBitmap );
}

void wxOverlayImpl::Reset()
{
    m_bmpSaved = wxBitmap();
}

void wxOverlayImpl::BeginDrawing(wxDC*  WXUNUSED(dc))
{
}

void wxOverlayImpl::EndDrawing(wxDC* WXUNUSED(dc))
{
}

#endif // !wxHAS_NATIVE_OVERLAY


