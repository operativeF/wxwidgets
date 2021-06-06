/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/dcmemory.cpp
// Purpose:     wxMemoryDC class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#include "wx/dcmemory.h"
#include "wx/msw/dcmemory.h"

#ifndef WX_PRECOMP
    #include "wx/utils.h"
    #include "wx/log.h"
#endif

#include "wx/msw/private.h"

// ----------------------------------------------------------------------------
// wxMemoryDCImpl
// ----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxMemoryDCImpl, wxMSWDCImpl);

wxMemoryDCImpl::wxMemoryDCImpl( wxMemoryDC *owner )
        : wxMSWDCImpl( owner )
{
    CreateCompatible(nullptr);
    Init();
}

wxMemoryDCImpl::wxMemoryDCImpl( wxMemoryDC *owner, wxBitmap& bitmap )
        : wxMSWDCImpl( owner )
{
    CreateCompatible(nullptr);
    Init();
    DoSelect(bitmap);
}

wxMemoryDCImpl::wxMemoryDCImpl( wxMemoryDC *owner, wxDC *dc )
        : wxMSWDCImpl( owner )
{
    wxCHECK_RET( dc, wxT("NULL dc in wxMemoryDC ctor") );

    CreateCompatible(dc);

    Init();
}

void wxMemoryDCImpl::Init()
{
    if ( m_ok )
    {
        SetBrush(*wxWHITE_BRUSH);
        SetPen(*wxBLACK_PEN);
        SetFont(*wxNORMAL_FONT);

        // the background mode is only used for text background and is set in
        // DrawText() to OPAQUE as required, otherwise always TRANSPARENT
        ::SetBkMode( GetHdc(), TRANSPARENT );
    }
}

bool wxMemoryDCImpl::CreateCompatible(wxDC *dc)
{
    wxDCImpl *impl = dc ? dc->GetImpl() : nullptr;
    wxMSWDCImpl *msw_impl = wxDynamicCast( impl, wxMSWDCImpl );
    if ( dc && !msw_impl)
    {
        m_ok = false;
        return false;
    }

    m_hDC = (WXHDC)::CreateCompatibleDC(dc ? GetHdcOf(*msw_impl) : nullptr);

    // as we created the DC, we must delete it in the dtor
    m_bOwnsDC = true;

    m_ok = m_hDC != nullptr;

    return m_ok;
}

void wxMemoryDCImpl::DoSelect( const wxBitmap& bitmap )
{
    // select old bitmap out of the device context
    if ( m_oldBitmap )
    {
        ::SelectObject(GetHdc(), (HBITMAP) m_oldBitmap);
        if ( m_selectedBitmap.IsOk() )
        {
            m_selectedBitmap.SetSelectedInto(nullptr);
            m_selectedBitmap = wxNullBitmap;
        }
    }

    // check for whether the bitmap is already selected into a device context
    wxASSERT_MSG( !bitmap.GetSelectedInto() ||
                  (bitmap.GetSelectedInto() == GetOwner()),
                  wxT("Bitmap is selected in another wxMemoryDC, delete the first wxMemoryDC or use SelectObject(NULL)") );

    m_selectedBitmap = bitmap;
    WXHBITMAP hBmp = m_selectedBitmap.GetHBITMAP();
    if ( !hBmp )
        return;

    m_selectedBitmap.SetSelectedInto(GetOwner());
    hBmp = (WXHBITMAP)::SelectObject(GetHdc(), (HBITMAP)hBmp);

    if ( !hBmp )
    {
        wxLogLastError(wxT("SelectObject(memDC, bitmap)"));

        wxFAIL_MSG(wxT("Couldn't select a bitmap into wxMemoryDC"));
    }
    else if ( !m_oldBitmap )
    {
        m_oldBitmap = hBmp;
    }
}

wxSize wxMemoryDCImpl::DoGetSize() const
{
    if ( m_selectedBitmap.IsOk() )
    {
        return { m_selectedBitmap.GetWidth() , m_selectedBitmap.GetHeight() };
    }
    else
    {
        return { 0, 0 };
    }
}

// the rest of this file deals with drawing rectangles workaround, disabled by
// default

#define wxUSE_MEMORY_DC_DRAW_RECTANGLE 0

#if wxUSE_MEMORY_DC_DRAW_RECTANGLE

// For some reason, drawing a rectangle on a memory DC has problems.
// Use this substitute if we can.
static void wxDrawRectangle(wxDC& dc, wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
    wxBrush brush(dc.GetBrush());
    wxPen pen(dc.GetPen());
    if (brush.IsOk() && brush.GetStyle() != wxBrushStyle::Transparent)
    {
        HBRUSH hBrush = (HBRUSH) brush.GetResourceHandle() ;
        if (hBrush)
        {
            RECT rect;
            rect.left = x; rect.top = y;
            rect.right = x + width - 1;
            rect.bottom = y + height - 1;
            ::FillRect((HDC) dc.GetHDC(), &rect, hBrush);
        }
    }
    width --; height --;
    if (pen.IsOk() && pen.GetStyle() != wxPENSTYLE_TRANSPARENT)
    {
        dc.DrawLine(x, y, x + width, y);
        dc.DrawLine(x, y, x, y + height);
        dc.DrawLine(x, y+height, x+width, y + height);
        dc.DrawLine(x+width, y+height, x+width, y);
    }
}

#endif // wxUSE_MEMORY_DC_DRAW_RECTANGLE

void wxMemoryDCImpl::DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
    // Set this to 1 to work around an apparent video driver bug
    // (visible with e.g. 70x70 rectangle on a memory DC; see Drawing sample)
#if wxUSE_MEMORY_DC_DRAW_RECTANGLE
    if (m_brush.IsOk() && m_pen.IsOk() &&
        (m_brush.GetStyle() == wxBrushStyle::Solid || m_brush.GetStyle() == wxBrushStyle::Transparent) &&
        (m_pen.GetStyle() == wxPENSTYLE_SOLID || m_pen.GetStyle() == wxPENSTYLE_TRANSPARENT) &&
        (GetLogicalFunction() == wxCOPY))
    {
        wxDrawRectangle(* this, x, y, width, height);
    }
    else
#endif // wxUSE_MEMORY_DC_DRAW_RECTANGLE
    {
        wxMSWDCImpl::DoDrawRectangle(x, y, width, height);
    }
}
