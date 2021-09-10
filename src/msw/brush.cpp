/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/brush.cpp
// Purpose:     wxBrush
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/msw/private.h"
    #include "wx/msw/wrap/utils.h"
#endif

#include "wx/brush.h"
#include "wx/utils.h"
#include "wx/app.h"
#include "wx/bitmap.h"


// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

using namespace msw::utils;

class WXDLLEXPORT wxBrushRefData: public wxGDIRefData
{
public:
    explicit wxBrushRefData(const wxColour& colour = wxNullColour, wxBrushStyle style = wxBrushStyle::Solid);
    explicit wxBrushRefData(const wxBitmap& stipple);
    wxBrushRefData(const wxBrushRefData& data);
    ~wxBrushRefData() = default;

    // no assignment operator, the objects of this class are shared and never
    // assigned after being created once
    wxBrushRefData& operator=(const wxBrushRefData&) = delete;

    bool operator==(const wxBrushRefData& data) const;

    HBRUSH GetHBRUSH();

    const wxColour& GetColour() const { return m_colour; }
    wxBrushStyle GetStyle() const { return m_style; }
    wxBitmap *GetStipple() { return &m_stipple; }

    void SetColour(const wxColour& colour) { m_hBrush.reset(); m_colour = colour; }
    void SetStyle(wxBrushStyle style) { m_hBrush.reset(); m_style = style; }
    void SetStipple(const wxBitmap& stipple) { m_hBrush.reset(); DoSetStipple(stipple); }

private:
    void DoSetStipple(const wxBitmap& stipple);

    wxBrushStyle  m_style;
    wxBitmap      m_stipple;
    wxColour      m_colour;
    unique_brush  m_hBrush;
};

#define M_BRUSHDATA ((wxBrushRefData *)m_refData)

// ============================================================================
// wxBrushRefData implementation
// ============================================================================

wxIMPLEMENT_DYNAMIC_CLASS(wxBrush, wxGDIObject);

// ----------------------------------------------------------------------------
// wxBrushRefData ctors/dtor
// ----------------------------------------------------------------------------

wxBrushRefData::wxBrushRefData(const wxColour& colour, wxBrushStyle style)
              : m_colour(colour), m_style(style)
{
}

wxBrushRefData::wxBrushRefData(const wxBitmap& stipple)
{
    DoSetStipple(stipple);
}

wxBrushRefData::wxBrushRefData(const wxBrushRefData& data)
              : 
                m_stipple(data.m_stipple),
                m_colour(data.m_colour)
{
    m_style = data.m_style;

    // we can't share HBRUSH, we'd need to create another one ourselves
    m_hBrush.reset();
}

// ----------------------------------------------------------------------------
// wxBrushRefData accesors
// ----------------------------------------------------------------------------

bool wxBrushRefData::operator==(const wxBrushRefData& data) const
{
    // don't compare HBRUSHes
    return m_style == data.m_style &&
           m_colour == data.m_colour &&
           m_stipple.IsSameAs(data.m_stipple);
}

void wxBrushRefData::DoSetStipple(const wxBitmap& stipple)
{
    m_stipple = stipple;
    m_style = stipple.GetMask() ? wxBrushStyle::StippleMaskOpaque
                                : wxBrushStyle::Stipple;
}

// ----------------------------------------------------------------------------
// wxBrushRefData resource handling
// ----------------------------------------------------------------------------

static int TranslateHatchStyle(wxBrushStyle style)
{
    switch ( style )
    {
        case wxBrushStyle::BDiagonalHatch: return HS_BDIAGONAL;
        case wxBrushStyle::CrossDiagHatch: return HS_DIAGCROSS;
        case wxBrushStyle::FDiagonalHatch: return HS_FDIAGONAL;
        case wxBrushStyle::CrossHatch:     return HS_CROSS;
        case wxBrushStyle::HorizontalHatch:return HS_HORIZONTAL;
        case wxBrushStyle::VerticalHatch:  return HS_VERTICAL;
        default:                return -1;
    }
}

HBRUSH wxBrushRefData::GetHBRUSH()
{
    if ( !m_hBrush )
    {
        int hatchStyle = TranslateHatchStyle(m_style);
        if ( hatchStyle == -1 )
        {
            switch ( m_style )
            {
                case wxBrushStyle::Transparent:
                    m_hBrush = unique_brush(static_cast<HBRUSH>(::GetStockObject(NULL_BRUSH)));
                    break;

                case wxBrushStyle::Stipple:
                    m_hBrush = unique_brush(::CreatePatternBrush(GetHbitmapOf(m_stipple)));
                    break;

                case wxBrushStyle::StippleMaskOpaque:
                    m_hBrush = unique_brush(::CreatePatternBrush((HBITMAP)m_stipple.GetMask()
                                                        ->GetMaskBitmap()));
                    break;

                default:
                    wxFAIL_MSG( wxT("unknown brush style") );
                    [[fallthrough]];

                case wxBrushStyle::Solid:
                    m_hBrush = unique_brush(::CreateSolidBrush(m_colour.GetPixel()));
                    break;
            }
        }
        else // create a hatched brush
        {
            m_hBrush = unique_brush(::CreateHatchBrush(hatchStyle, m_colour.GetPixel()));
        }

        if ( !m_hBrush )
        {
            wxLogLastError(wxT("CreateXXXBrush()"));
        }
    }

    return m_hBrush.get();
}

// ============================================================================
// wxBrush implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxBrush ctors/dtor
// ----------------------------------------------------------------------------

wxBrush::wxBrush(const wxColour& col, wxBrushStyle style)
{
    m_refData = new wxBrushRefData(col, style);
}

wxBrush::wxBrush(const wxBitmap& stipple)
{
    m_refData = new wxBrushRefData(stipple);
}

// ----------------------------------------------------------------------------
// wxBrush house keeping stuff
// ----------------------------------------------------------------------------

bool wxBrush::operator==(const wxBrush& brush) const
{
    const wxBrushRefData *brushData = (wxBrushRefData *)brush.m_refData;

    // an invalid brush is considered to be only equal to another invalid brush
    return m_refData ? (brushData && *M_BRUSHDATA == *brushData) : !brushData;
}

wxGDIRefData *wxBrush::CreateGDIRefData() const
{
    return new wxBrushRefData;
}

wxGDIRefData *wxBrush::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxBrushRefData(*(const wxBrushRefData *)data);
}

// ----------------------------------------------------------------------------
// wxBrush accessors
// ----------------------------------------------------------------------------

wxColour wxBrush::GetColour() const
{
    wxCHECK_MSG( IsOk(), wxNullColour, wxT("invalid brush") );

    return M_BRUSHDATA->GetColour();
}

wxBrushStyle wxBrush::GetStyle() const
{
    wxCHECK_MSG( IsOk(), wxBrushStyle::Invalid, wxT("invalid brush") );

    return M_BRUSHDATA->GetStyle();
}

wxBitmap *wxBrush::GetStipple() const
{
    wxCHECK_MSG( IsOk(), nullptr, wxT("invalid brush") );

    return M_BRUSHDATA->GetStipple();
}

WXHANDLE wxBrush::GetResourceHandle() const
{
    wxCHECK_MSG( IsOk(), FALSE, wxT("invalid brush") );

    return (WXHANDLE)M_BRUSHDATA->GetHBRUSH();
}

// ----------------------------------------------------------------------------
// wxBrush setters
// ----------------------------------------------------------------------------

void wxBrush::SetColour(const wxColour& col)
{
    AllocExclusive();

    M_BRUSHDATA->SetColour(col);
}

void wxBrush::SetColour(unsigned char r, unsigned char g, unsigned char b)
{
    AllocExclusive();

    M_BRUSHDATA->SetColour(wxColour(r, g, b));
}

void wxBrush::SetStyle(wxBrushStyle style)
{
    AllocExclusive();

    M_BRUSHDATA->SetStyle(style);
}

void wxBrush::SetStipple(const wxBitmap& stipple)
{
    AllocExclusive();

    M_BRUSHDATA->SetStipple(stipple);
}
