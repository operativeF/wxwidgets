/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/pen.cpp
// Purpose:     wxPen
// Author:      Julian Smart
// Modified by: Vadim Zeitlin: refactored wxPen code to wxPenRefData
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"

#include "wx/pen.h"

#include "wx/bitmap.h"
#include "wx/utils.h"

import WX.WinDef;
import WX.Win.UniqueHnd;

#define M_PENDATA ((wxPenRefData*)m_refData)

// ----------------------------------------------------------------------------
// wxPenRefData: contains information about an WXHPEN and its handle
// ----------------------------------------------------------------------------

using namespace msw::utils;

class wxPenRefData : public wxGDIRefData
{
public:
    // ctors and dtor
    // --------------

    wxPenRefData() = default;
    wxPenRefData(const wxPenRefData& data);
    explicit wxPenRefData(const wxPenInfo& info);

    wxPenRefData& operator=(const wxPenRefData&) = delete;

    bool operator==(const wxPenRefData& data) const
    {
        // we intentionally don't compare m_hPen fields here
        return m_style == data.m_style &&
               m_width == data.m_width &&
               m_join == data.m_join &&
               m_cap == data.m_cap &&
               m_quality == data.m_quality &&
               m_colour == data.m_colour &&
               (m_style != wxPenStyle::Stipple || m_stipple.IsSameAs(data.m_stipple)) &&
               (m_style != wxPenStyle::UserDash ||
                (m_nbDash == data.m_nbDash &&
                    memcmp(m_dash, data.m_dash, m_nbDash*sizeof(wxDash)) == 0));
    }

    wxColour& GetColour() const { return const_cast<wxColour&>(m_colour); }
    int GetWidth() const { return m_width; }
    wxPenStyle GetStyle() const { return m_style; }
    wxPenJoin GetJoin() const { return m_join; }
    wxPenCap GetCap() const { return m_cap; }
    wxPenQuality GetQuality() const { return m_quality; }
    wxDash* GetDash() const { return m_dash; }
    int GetDashCount() const { return m_nbDash; }
    wxBitmap* GetStipple() const { return const_cast<wxBitmap *>(&m_stipple); }

    void SetColour(const wxColour& col) { m_hPen.reset(); m_colour = col; }
    void SetWidth(int width) { m_hPen.reset(); m_width = width; }
    void SetStyle(wxPenStyle style) { m_hPen.reset(); m_style = style; }
    void SetStipple(const wxBitmap& stipple)
    {
        m_hPen.reset();

        m_style = wxPenStyle::Stipple;
        m_stipple = stipple;
    }

    void SetDashes(int nb_dashes, const wxDash *dash)
    {
        m_hPen.reset();

        m_nbDash = nb_dashes;
        m_dash = const_cast<wxDash *>(dash);
    }

    void SetJoin(wxPenJoin join) { m_hPen.reset(); m_join = join; }
    void SetCap(wxPenCap cap) { m_hPen.reset(); m_cap = cap; }
    void SetQuality(wxPenQuality quality) { m_hPen.reset(); m_quality = quality; }


    // WXHPEN management
    // ---------------

    // create the WXHPEN if we don't have it yet
    bool Alloc();

    // get the WXHPEN creating it on demand
    WXHPEN GetHPEN() const;

    // return true if we have a valid WXHPEN
    bool HasHPEN() const { return m_hPen != nullptr; }

    void Free();

private:
    // initialize the fields which have reasonable default values
    //
    // doesn't initialize m_width and m_style which must be initialize in ctor
    wxColour      m_colour;

    wxBitmap      m_stipple;

    wxDash* m_dash{nullptr}; // FIXME: Use array
    unique_pen    m_hPen;

    int           m_width{1};
    int           m_nbDash{0};

    wxPenStyle    m_style{wxPenStyle::Solid};
    wxPenJoin     m_join{wxJOIN_ROUND};
    wxPenCap      m_cap{wxCAP_ROUND};
    wxPenQuality  m_quality{wxPenQuality::Default};
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxPenRefData ctors/dtor
// ----------------------------------------------------------------------------

wxPenRefData::wxPenRefData(const wxPenRefData& data)
             :
     m_colour(data.m_colour)
{
    m_style = data.m_style;
    m_width = data.m_width;
    m_join = data.m_join;
    m_cap = data.m_cap;
    m_quality = data.m_quality;
    m_nbDash = data.m_nbDash;
    m_dash = data.m_dash;
}

wxPenRefData::wxPenRefData(const wxPenInfo& info)
    : m_stipple(info.GetStipple())
    , m_colour(info.GetColour())
{
    m_style = info.GetStyle();
    m_width = info.GetWidth();
    m_join = info.GetJoin();
    m_cap = info.GetCap();
    m_quality = info.GetQuality();
    m_nbDash = info.GetDashes(&m_dash);
}

// ----------------------------------------------------------------------------
// wxPenRefData WXHPEN management
// ----------------------------------------------------------------------------

static int ConvertPenStyle(wxPenStyle style)
{
    switch ( style )
    {
        case wxPenStyle::ShortDash:
        case wxPenStyle::LongDash:
            return PS_DASH;

        case wxPenStyle::Transparent:
            return PS_NULL;

        default:
            wxFAIL_MSG( "unknown pen style" );
            [[fallthrough]];

        case wxPenStyle::Dot:
            return PS_DOT;

        case wxPenStyle::DotDash:
            return PS_DASHDOT;

        case wxPenStyle::UserDash:
            return PS_USERSTYLE;

        case wxPenStyle::Stipple:
        case wxPenStyle::BDiagonalHatch:
        case wxPenStyle::CrossDiagHatch:
        case wxPenStyle::FDiagonalHatch:
        case wxPenStyle::CrossHatch:
        case wxPenStyle::HorizontalHatch:
        case wxPenStyle::VerticalHatch:
        case wxPenStyle::Solid:

            return PS_SOLID;
    }
}

static int ConvertJoinStyle(wxPenJoin join)
{
    switch( join )
    {
        case wxJOIN_BEVEL:
            return PS_JOIN_BEVEL;

        case wxJOIN_MITER:
            return PS_JOIN_MITER;

        default:
            wxFAIL_MSG( "unknown pen join style" );
            [[fallthrough]];

        case wxJOIN_ROUND:
            return PS_JOIN_ROUND;
    }
}

static int ConvertCapStyle(wxPenCap cap)
{
    switch ( cap )
    {
        case wxCAP_PROJECTING:
            return PS_ENDCAP_SQUARE;

        case wxCAP_BUTT:
            return PS_ENDCAP_FLAT;

        default:
            wxFAIL_MSG( "unknown pen cap style" );
            [[fallthrough]];

        case wxCAP_ROUND:
            return PS_ENDCAP_ROUND;
    }
}

bool wxPenRefData::Alloc()
{
   if ( m_hPen )
       return false;

   if ( m_style == wxPenStyle::Transparent )
   {
       m_hPen = unique_pen(static_cast<WXHPEN>(::GetStockObject(NULL_PEN)));
       return true;
   }

   const COLORREF col = m_colour.GetPixel();

   // check if it's a standard kind of pen which can be created with just
   // CreatePen(), which always creates cosmetic pens that don't support all
   // wxPen features and are less precise (e.g. draw dotted lines as dashes
   // rather than real dots), but much, much faster than geometric pens created
   // by ExtCreatePen(), see #18875, so we still prefer to use them if possible
   // unless it's explicitly disabled by setting the quality to "high"
   bool useCreatePen = m_quality != wxPenQuality::High;

   if ( useCreatePen )
   {
       switch ( m_style )
       {
           case wxPenStyle::Solid:
               // No problem with using cosmetic pens for solid lines.
               break;

           case wxPenStyle::Dot:
           case wxPenStyle::LongDash:
           case wxPenStyle::ShortDash:
           case wxPenStyle::DotDash:
               if ( m_width > 1 )
               {
                   // Cosmetic pens with these styles would result in solid
                   // lines for pens wider than a single pixel, so never use
                   // them in this case.
                   useCreatePen = false;
               }
               else
               {
                   // For the single pixel pens we can use cosmetic pens, but
                   // they look ugly, so we prefer to not do it by default,
                   // however this can be explicitly requested if speed is more
                   // important than the exact appearance.
                   useCreatePen = m_quality == wxPenQuality::Low;
               }
               break;

           default:
               // Other styles are not supported by cosmetic pens at all.
               useCreatePen = false;
               break;
       }
   }

   // Join and cap styles are also not supported for cosmetic pens.
   if ( m_join != wxJOIN_ROUND || m_cap != wxCAP_ROUND )
       useCreatePen = false;

   if ( useCreatePen )
   {
       m_hPen = unique_pen(::CreatePen(ConvertPenStyle(m_style), m_width, col));
   }
   else // need to use ExtCreatePen()
   {
       DWORD styleMSW = PS_GEOMETRIC |
                        ConvertPenStyle(m_style) |
                        ConvertJoinStyle(m_join) |
                        ConvertCapStyle(m_cap);

       LOGBRUSH lb;
       switch( m_style )
       {
           case wxPenStyle::Stipple:
               lb.lbStyle = BS_PATTERN;
               lb.lbHatch = wxPtrToUInt(m_stipple.GetHBITMAP());
               break;

           case wxPenStyle::BDiagonalHatch:
               lb.lbStyle = BS_HATCHED;
               lb.lbHatch = HS_BDIAGONAL;
               break;

           case wxPenStyle::CrossDiagHatch:
               lb.lbStyle = BS_HATCHED;
               lb.lbHatch = HS_DIAGCROSS;
               break;

           case wxPenStyle::FDiagonalHatch:
               lb.lbStyle = BS_HATCHED;
               lb.lbHatch = HS_FDIAGONAL;
               break;

           case wxPenStyle::CrossHatch:
               lb.lbStyle = BS_HATCHED;
               lb.lbHatch = HS_CROSS;
               break;

           case wxPenStyle::HorizontalHatch:
               lb.lbStyle = BS_HATCHED;
               lb.lbHatch = HS_HORIZONTAL;
               break;

           case wxPenStyle::VerticalHatch:
               lb.lbStyle = BS_HATCHED;
               lb.lbHatch = HS_VERTICAL;
               break;

           default:
               lb.lbStyle = BS_SOLID;
               // this should be unnecessary (it's unused) but suppresses the
               // Purify messages about uninitialized memory read
               lb.lbHatch = 0;
               break;
       }

       lb.lbColor = col;

       std::unique_ptr<DWORD[]> dash;
       
       if ( m_style == wxPenStyle::UserDash && m_nbDash && m_dash )
       {
           dash.reset(new DWORD[m_nbDash]);
           
           int rw = m_width > 1 ? m_width : 1;

           for ( int i = 0; i < m_nbDash; i++ )
               dash[i] = m_dash[i] * rw;
       }

       // Note that width can't be 0 for ExtCreatePen(), unlike for CreatePen().
       int width = m_width == 0 ? 1 : m_width;

       m_hPen = unique_pen(::ExtCreatePen(styleMSW, width, &lb, m_nbDash, (LPDWORD)dash.get()));
   }

   return m_hPen != nullptr;
}

void wxPenRefData::Free()
{
    m_hPen.reset();
}

WXHPEN wxPenRefData::GetHPEN() const
{
    if ( !m_hPen )
        const_cast<wxPenRefData *>(this)->Alloc();

    return m_hPen.get();
}

// ----------------------------------------------------------------------------
// wxPen
// ----------------------------------------------------------------------------

wxPen::wxPen(const wxColour& col, int width, wxPenStyle style)
{
    m_refData = new wxPenRefData(wxPenInfo(col, width).Style(style));
}

wxPen::wxPen(const wxBitmap& stipple, int width)
{
    m_refData = new wxPenRefData(wxPenInfo().Stipple(stipple).Width(width));
}

wxPen::wxPen(const wxPenInfo& info)
{
    m_refData = new wxPenRefData(info);
}

bool wxPen::operator==(const wxPen& pen) const
{
    const wxPenRefData *
        penData = dynamic_cast<const wxPenRefData *>(pen.m_refData);

    // an invalid pen is only equal to another invalid pen
    return m_refData ? penData && *M_PENDATA == *penData : !penData;
}

bool wxPen::RealizeResource()
{
    return M_PENDATA && M_PENDATA->Alloc();
}

WXHANDLE wxPen::GetResourceHandle() const
{
    return M_PENDATA ? M_PENDATA->GetHPEN() : nullptr;
}

bool wxPen::FreeResource([[maybe_unused]] bool force)
{
    if(M_PENDATA)
    {
        M_PENDATA->Free();
        return true;
    }

    return false;
}

bool wxPen::IsFree() const
{
    return M_PENDATA && !M_PENDATA->HasHPEN();
}

wxGDIRefData* wxPen::CreateGDIRefData() const
{
    return new wxPenRefData;
}

wxGDIRefData* wxPen::CloneGDIRefData(const wxGDIRefData* data) const
{
    return new wxPenRefData(*dynamic_cast<const wxPenRefData*>(data));
}

void wxPen::SetColour(const wxColour& col)
{
    AllocExclusive();

    M_PENDATA->SetColour(col);
}

void wxPen::SetColour(unsigned char r, unsigned char g, unsigned char b)
{
    SetColour(wxColour(r, g, b));
}

void wxPen::SetWidth(int width)
{
    AllocExclusive();

    M_PENDATA->SetWidth(width);
}

void wxPen::SetStyle(wxPenStyle style)
{
    AllocExclusive();

    M_PENDATA->SetStyle(style);
}

void wxPen::SetStipple(const wxBitmap& stipple)
{
    AllocExclusive();

    M_PENDATA->SetStipple(stipple);
}

void wxPen::SetDashes(int nb_dashes, const wxDash *dash)
{
    AllocExclusive();

    M_PENDATA->SetDashes(nb_dashes, dash);
}

void wxPen::SetJoin(wxPenJoin join)
{
    AllocExclusive();

    M_PENDATA->SetJoin(join);
}

void wxPen::SetCap(wxPenCap cap)
{
    AllocExclusive();

    M_PENDATA->SetCap(cap);
}

void wxPen::SetQuality(wxPenQuality quality)
{
    AllocExclusive();

    M_PENDATA->SetQuality(quality);
}

wxColour wxPen::GetColour() const
{
    wxCHECK_MSG( IsOk(), wxNullColour, "invalid pen" );

    return M_PENDATA->GetColour();
}

int wxPen::GetWidth() const
{
    wxCHECK_MSG( IsOk(), -1, "invalid pen" );

    return M_PENDATA->GetWidth();
}

wxPenStyle wxPen::GetStyle() const
{
    wxCHECK_MSG( IsOk(), wxPenStyle::Invalid, "invalid pen" );

    return M_PENDATA->GetStyle();
}

wxPenJoin wxPen::GetJoin() const
{
    wxCHECK_MSG( IsOk(), wxJOIN_INVALID, "invalid pen" );

    return M_PENDATA->GetJoin();
}

wxPenCap wxPen::GetCap() const
{
    wxCHECK_MSG( IsOk(), wxCAP_INVALID, "invalid pen" );

    return M_PENDATA->GetCap();
}

wxPenQuality wxPen::GetQuality() const
{
    wxCHECK_MSG( IsOk(), wxPenQuality::Default, "invalid pen" );

    return M_PENDATA->GetQuality();
}

int wxPen::GetDashes(wxDash** ptr) const
{
    wxCHECK_MSG( IsOk(), -1, "invalid pen" );

    *ptr = M_PENDATA->GetDash();
    return M_PENDATA->GetDashCount();
}

wxDash* wxPen::GetDash() const
{
    wxCHECK_MSG( IsOk(), nullptr, "invalid pen" );

    return m_refData ? M_PENDATA->GetDash() : nullptr;
}

int wxPen::GetDashCount() const
{
    wxCHECK_MSG( IsOk(), -1, "invalid pen" );

    return m_refData ? M_PENDATA->GetDashCount() : 0;
}

wxBitmap* wxPen::GetStipple() const
{
    wxCHECK_MSG( IsOk(), nullptr, "invalid pen" );

    return m_refData ? M_PENDATA->GetStipple() : nullptr;
}
