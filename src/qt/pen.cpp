/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/pen.cpp
// Author:      Peter Most, Javier Torres
// Copyright:   (c) Peter Most, Javier Torres
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////



#include "wx/pen.h"
#include "wx/colour.h"
#include "wx/qt/private/utils.h"
#include <QtGui/QPen>

wxIMPLEMENT_DYNAMIC_CLASS(wxPen,wxGDIObject);

static Qt::PenStyle ConvertPenStyle(wxPenStyle style)
{
    switch(style)
    {
        case wxPenStyle::Solid:
            return Qt::SolidLine;

        case wxPenStyle::Transparent:
            return Qt::NoPen;

        case wxPenStyle::Dot:
            return Qt::DotLine;

        case wxPenStyle::LongDash:
        case wxPenStyle::ShortDash:
            return Qt::DashLine;

        case wxPenStyle::DotDash:
            return Qt::DotLine;

        case wxPenStyle::UserDash:
            return Qt::CustomDashLine;

        case wxPenStyle::Stipple:
            wxMISSING_IMPLEMENTATION( "wxPenStyle::Stipple" );
            break;

        case wxPenStyle::BDiagonalHatch:
            wxMISSING_IMPLEMENTATION( "wxPenStyle::BDiagonalHatch" );
            break;

        case wxPenStyle::CrossDiagHatch:
            wxMISSING_IMPLEMENTATION( "wxPenStyle::CrossDiagHatch" );
            break;

        case wxPenStyle::FDiagonalHatch:
            wxMISSING_IMPLEMENTATION( "wxPenStyle::FDiagonalHatch" );
            break;

        case wxPenStyle::CrossHatch:
            wxMISSING_IMPLEMENTATION( "wxPenStyle::CrossHatch" );
            break;

        case wxPenStyle::HorizontalHatch:
            wxMISSING_IMPLEMENTATION( "wxPenStyle::HorizontalHatch" );
            break;

        case wxPenStyle::VerticalHatch:
            wxMISSING_IMPLEMENTATION( "wxPenStyle::VerticalHatch" );
            break;

        case wxPenStyle::StippleMask:
            wxMISSING_IMPLEMENTATION( "wxPenStyle::StippleMask" );
            break;

        case wxPenStyle::StippleMaskOpaque:
            wxMISSING_IMPLEMENTATION( "wxPenStyle::StippleMaskOpaque" );
            break;

        case wxPenStyle::Invalid:
            wxFAIL_MSG( "Invalid pen style value" );
            break;
    }
    return Qt::SolidLine;
}

static wxPenStyle ConvertPenStyle(Qt::PenStyle style)
{
    switch (style)
    {
        case Qt::SolidLine:
            return wxPenStyle::Solid;

        case Qt::NoPen:
            return wxPenStyle::Transparent;

        case Qt::DotLine:
            return wxPenStyle::Dot;

        case Qt::DashLine:
            return wxPenStyle::ShortDash;

        case Qt::DashDotLine:
            return wxPenStyle::DotDash;

        case Qt::DashDotDotLine:
            wxMISSING_IMPLEMENTATION( "Qt::DashDotDotLine" );
            return wxPenStyle::DotDash;

        case Qt::CustomDashLine:
            return wxPenStyle::UserDash;

        case Qt::MPenStyle:
            wxMISSING_IMPLEMENTATION( "Qt::MPenStyle" );
            break;
    }
    return wxPenStyle::Solid;
}

static Qt::PenCapStyle ConvertPenCapStyle(wxPenCap style)
{
    switch (style)
    {
        case wxCAP_BUTT:
            return Qt::FlatCap;

        case wxCAP_PROJECTING:
            return Qt::SquareCap;

        case wxCAP_ROUND:
            return Qt::RoundCap;

        case wxCAP_INVALID:
            wxFAIL_MSG( "Invalid pen cap value" );
            break;
    }
    return Qt::SquareCap;
}

static wxPenCap ConvertPenCapStyle(Qt::PenCapStyle style)
{
    switch (style)
    {
        case Qt::FlatCap:
            return wxCAP_BUTT;

        case Qt::SquareCap:
            return wxCAP_PROJECTING;

        case Qt::RoundCap:
            return wxCAP_ROUND;

        case Qt::MPenCapStyle:
            wxMISSING_IMPLEMENTATION( "Qt::MPenCapStyle" );
            break;
    }
    return wxCAP_PROJECTING;
}

static Qt::PenJoinStyle ConvertPenJoinStyle(wxPenJoin style)
{
    switch (style)
    {
        case wxJOIN_BEVEL:
            return Qt::BevelJoin;

        case wxJOIN_MITER:
            return Qt::MiterJoin;

        case wxJOIN_ROUND:
            return Qt::RoundJoin;

        case wxJOIN_INVALID:
            wxFAIL_MSG( "Invalid pen join value" );
            break;
    }
    return Qt::BevelJoin;
}

static wxPenJoin ConvertPenJoinStyle(Qt::PenJoinStyle style)
{
    switch (style)
    {
        case Qt::BevelJoin:
            return wxJOIN_BEVEL;

        case Qt::MiterJoin:
            return wxJOIN_MITER;

        case Qt::RoundJoin:
            return wxJOIN_ROUND;

        case Qt::SvgMiterJoin:
            wxMISSING_IMPLEMENTATION( "Qt::SvgMiterJoin" );
            return wxJOIN_MITER;

        case Qt::MPenJoinStyle:
            wxMISSING_IMPLEMENTATION( "Qt::MPenJoinStyle" );
            break;
    }
    return wxJOIN_BEVEL;
}

//-----------------------------------------------------------------------------
// wxPen
//-----------------------------------------------------------------------------

class wxPenRefData: public wxGDIRefData
{
    public:
        void defaultPen()
        {
            m_qtPen.setCapStyle(Qt::RoundCap);
            m_qtPen.setJoinStyle(Qt::RoundJoin);
            m_dashes = NULL;
            m_dashesSize = 0;
        }

        wxPenRefData()
        {
            defaultPen();
        }

        wxPenRefData( const wxPenRefData& data )
        : wxGDIRefData()
            , m_qtPen(data.m_qtPen)
        {
            defaultPen();
        }

        bool operator == (const wxPenRefData& data) const
        {
             return m_qtPen == data.m_qtPen;
        }

        QPen m_qtPen;
        const wxDash *m_dashes;
        int m_dashesSize;
};

//-----------------------------------------------------------------------------

#define M_PENDATA ((wxPenRefData *)m_refData)->m_qtPen

wxPen::wxPen()
{
    m_refData = new wxPenRefData();
}

wxPen::wxPen( const wxColour &colour, int width, wxPenStyle style)
{
    m_refData = new wxPenRefData();
    M_PENDATA.setWidth(width);
    M_PENDATA.setStyle(ConvertPenStyle(style));
    M_PENDATA.setColor(colour.GetQColor());
}

bool wxPen::operator==(const wxPen& pen) const
{
    if (m_refData == pen.m_refData) return true;

    if (!m_refData || !pen.m_refData) return false;

    return ( *(wxPenRefData*)m_refData == *(wxPenRefData*)pen.m_refData );
}

bool wxPen::operator!=(const wxPen& pen) const
{
    return !(*this == pen);
}

void wxPen::SetColour(const wxColour& col)
{
    AllocExclusive();
    M_PENDATA.setColor(col.GetQColor());
}

void wxPen::SetColour(unsigned char r, unsigned char g, unsigned char b)
{
    AllocExclusive();
    M_PENDATA.setColor(QColor(r, g, b));
}

void wxPen::SetWidth(int width)
{
    AllocExclusive();
    M_PENDATA.setWidth(width);
}

void wxPen::SetStyle(wxPenStyle style)
{
    AllocExclusive();
    M_PENDATA.setStyle(ConvertPenStyle(style));
}

void wxPen::SetStipple(const wxBitmap& WXUNUSED(stipple))
{
    wxFAIL_MSG( "stippled pens not supported" );
}

void wxPen::SetDashes(int nb_dashes, const wxDash *dash)
{
    AllocExclusive();
    ((wxPenRefData *)m_refData)->m_dashes = dash;
    ((wxPenRefData *)m_refData)->m_dashesSize = nb_dashes;

    QVector<qreal> dashes;
    if (dash)
    {
        for (int i = 0; i < nb_dashes; i++)
            dashes << dash[i];
    }

    M_PENDATA.setDashPattern(dashes);
}

void wxPen::SetJoin(wxPenJoin join)
{
    AllocExclusive();
    M_PENDATA.setJoinStyle(ConvertPenJoinStyle(join));
}

void wxPen::SetCap(wxPenCap cap)
{
    AllocExclusive();
    M_PENDATA.setCapStyle(ConvertPenCapStyle(cap));
}

wxColour wxPen::GetColour() const
{
    wxColour c(M_PENDATA.color());
    return c;
}

wxBitmap *wxPen::GetStipple() const
{
    return NULL;
}

wxPenStyle wxPen::GetStyle() const
{
    return ConvertPenStyle(M_PENDATA.style());
}

wxPenJoin wxPen::GetJoin() const
{
    return ConvertPenJoinStyle(M_PENDATA.joinStyle());
}

wxPenCap wxPen::GetCap() const
{
    return ConvertPenCapStyle(M_PENDATA.capStyle());
}

int wxPen::GetWidth() const
{
    return M_PENDATA.width();
}

int wxPen::GetDashes(wxDash **ptr) const
{
    *ptr = (wxDash *)((wxPenRefData *)m_refData)->m_dashes;
    return ((wxPenRefData *)m_refData)->m_dashesSize;
}

QPen wxPen::GetHandle() const
{
    return M_PENDATA;
}

wxGDIRefData *wxPen::CreateGDIRefData() const
{
    return new wxPenRefData;
}

wxGDIRefData *wxPen::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxPenRefData(*(wxPenRefData *)data);
}
