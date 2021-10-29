/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/brush.cpp
// Author:      Peter Most, Javier Torres
// Copyright:   (c) Peter Most, Javier Torres
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////



#include "wx/brush.h"
#include "wx/colour.h"
#include "wx/qt/private/utils.h"
#include "wx/bitmap.h"

#include <QtGui/QBrush>

wxIMPLEMENT_DYNAMIC_CLASS(wxBrush,wxGDIObject);

static Qt::BrushStyle ConvertBrushStyle(wxBrushStyle style)
{
    switch (style)
    {
        case wxBrushStyle::Invalid:
        case wxBrushStyle::Solid:
            return Qt::SolidPattern;

        case wxBrushStyle::Transparent:
            return Qt::NoBrush;

        case wxBrushStyle::BDiagonalHatch:
            return Qt::BDiagPattern;

        case wxBrushStyle::CrossDiagHatch:
            return Qt::DiagCrossPattern;

        case wxBrushStyle::FDiagonalHatch:
            return Qt::FDiagPattern;

        case wxBrushStyle::CrossHatch:
            return Qt::CrossPattern;

        case wxBrushStyle::HorizontalHatch:
            return Qt::HorPattern;

        case wxBrushStyle::VerticalHatch:
            return Qt::VerPattern;

        case wxBrushStyle::Stipple:
        case wxBrushStyle::StippleMaskOpaque:
        case wxBrushStyle::StippleMask:
            return Qt::TexturePattern;
            break;
    }
    return Qt::SolidPattern;
}

//-----------------------------------------------------------------------------
// wxBrush
//-----------------------------------------------------------------------------

class wxBrushRefData: public wxGDIRefData
{
    public:
        wxBrushRefData() :
            m_style(wxBrushStyle::Invalid)
        {
        }

        wxBrushRefData( const wxBrushRefData& data )
            : m_qtBrush(data.m_qtBrush)
        {
            m_style = data.m_style;
        }

        bool operator == (const wxBrushRefData& data) const
        {
            return m_qtBrush == data.m_qtBrush;
        }

        QBrush m_qtBrush;

        // To keep if mask is stippled
        wxBrushStyle m_style;
};

//-----------------------------------------------------------------------------

#define M_BRUSHDATA ((wxBrushRefData *)m_refData)->m_qtBrush
#define M_STYLEDATA ((wxBrushRefData *)m_refData)->m_style

wxBrush::wxBrush()
{
    m_refData = new wxBrushRefData();
}

wxBrush::wxBrush(const wxColour& col, wxBrushStyle style )
{
    m_refData = new wxBrushRefData();
    M_BRUSHDATA.setColor(col.GetQColor());
    M_BRUSHDATA.setStyle(ConvertBrushStyle(style));
    M_STYLEDATA = style;
}

wxBrush::wxBrush(const wxBitmap& stipple)
{
    m_refData = new wxBrushRefData();
    M_BRUSHDATA.setTexture(*stipple.GetHandle());
    if (stipple.GetMask() != NULL)
        M_STYLEDATA = wxBrushStyle::StippleMaskOpaque;
    else
        M_STYLEDATA = wxBrushStyle::Stipple;
}


void wxBrush::SetColour(const wxColour& col)
{
    AllocExclusive();
    M_BRUSHDATA.setColor(col.GetQColor());
}

void wxBrush::SetColour(unsigned char r, unsigned char g, unsigned char b)
{
    AllocExclusive();
    M_BRUSHDATA.setColor(QColor(r, g, b));
}

void wxBrush::SetStyle(wxBrushStyle style)
{
    AllocExclusive();
    M_BRUSHDATA.setStyle(ConvertBrushStyle((wxBrushStyle)style));
    M_STYLEDATA = style;
}

void wxBrush::SetStipple(const wxBitmap& stipple)
{
    AllocExclusive();
    M_BRUSHDATA.setTexture(*stipple.GetHandle());

    if (stipple.GetMask() != NULL)
        M_STYLEDATA = wxBrushStyle::StippleMaskOpaque;
    else
        M_STYLEDATA = wxBrushStyle::Stipple;
}


bool wxBrush::operator==(const wxBrush& brush) const
{
    if (m_refData == brush.m_refData) return true;

    if (!m_refData || !brush.m_refData) return false;

    return ( *(wxBrushRefData*)m_refData == *(wxBrushRefData*)brush.m_refData );
}


wxColour wxBrush::GetColour() const
{
    return wxColour(M_BRUSHDATA.color());
}

wxBrushStyle wxBrush::GetStyle() const
{
    return M_STYLEDATA;
}

wxBitmap *wxBrush::GetStipple() const
{
    QPixmap p = M_BRUSHDATA.texture();

    if (p.isNull())
        return new wxBitmap();
    else
        return new wxBitmap(p);
}

QBrush wxBrush::GetHandle() const
{
    return M_BRUSHDATA;
}

wxGDIRefData *wxBrush::CreateGDIRefData() const
{
    return new wxBrushRefData;
}

wxGDIRefData *wxBrush::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxBrushRefData(*(wxBrushRefData *)data);
}
