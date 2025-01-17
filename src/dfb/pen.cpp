/////////////////////////////////////////////////////////////////////////////
// Name:        src/dfb/pen.cpp
// Purpose:     wxPen class implementation
// Author:      Vaclav Slavik
// Created:     2006-08-04
// Copyright:   (c) 2006 REA Elektronik GmbH
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#include "wx/pen.h"

#ifndef WX_PRECOMP
    #include "wx/bitmap.h"
    #include "wx/colour.h"
#endif

//-----------------------------------------------------------------------------
// wxPen
//-----------------------------------------------------------------------------

class wxPenRefData : public wxGDIRefData
{
public:
    wxPenRefData(const wxColour& clr = wxNullColour, wxPenStyle style = wxPenStyle::Solid)
    {
        m_colour = clr;
        SetStyle(style);
    }

    wxPenRefData(const wxPenRefData& data)
        : m_style(data.m_style), m_colour(data.m_colour) {}

    virtual bool IsOk() const { return m_colour.IsOk(); }

    void SetStyle(wxPenStyle style)
    {
        if ( style != wxPenStyle::Solid && style != wxPenStyle::Transparent )
        {
            wxFAIL_MSG( "only wxPenStyle::Solid and wxPenStyle::Transparent styles are supported" );
            style = wxPenStyle::Solid;
        }

        m_style = style;
    }

    wxPenStyle     m_style;
    wxColour       m_colour;
};

//-----------------------------------------------------------------------------

#define M_PENDATA ((wxPenRefData *)m_refData)

wxIMPLEMENT_DYNAMIC_CLASS(wxPen, wxGDIObject);

wxPen::wxPen(const wxColour &colour, int width, wxPenStyle style)
{
    wxASSERT_MSG( width <= 1, "only width=0,1 are supported" );

    m_refData = new wxPenRefData(colour, style);
}

wxPen::wxPen([[maybe_unused]] const wxBitmap& stipple, [[maybe_unused]] int width)
{
    wxFAIL_MSG( "stipple pens not supported" );

    m_refData = new wxPenRefData();
}

wxPen::wxPen(const wxPenInfo& info)
{
    m_refData = new wxPenRefData(info.GetColour(), info.GetStyle());
}

bool wxPen::operator==(const wxPen& pen) const
{
#warning "this is incorrect"
    return m_refData == pen.m_refData;
}

void wxPen::SetColour(const wxColour &colour)
{
    AllocExclusive();
    M_PENDATA->m_colour = colour;
}

void wxPen::SetDashes([[maybe_unused]] int number_of_dashes, [[maybe_unused]] const wxDash *dash)
{
    wxFAIL_MSG( "SetDashes not implemented" );
}

void wxPen::SetColour(unsigned char red, unsigned char green, unsigned char blue)
{
    AllocExclusive();
    M_PENDATA->m_colour.Set(red, green, blue);
}

void wxPen::SetCap([[maybe_unused]] wxPenCap capStyle)
{
    wxFAIL_MSG( "SetCap not implemented" );
}

void wxPen::SetJoin([[maybe_unused]] wxPenJoin joinStyle)
{
    wxFAIL_MSG( "SetJoin not implemented" );
}

void wxPen::SetStyle(wxPenStyle style)
{
    AllocExclusive();
    M_PENDATA->SetStyle(style);
}

void wxPen::SetStipple([[maybe_unused]] const wxBitmap& stipple)
{
    wxFAIL_MSG( "SetStipple not implemented" );
}

void wxPen::SetWidth(int width)
{
    wxASSERT_MSG( width <= 1, "only width=0,1 are implemented" );
}

int wxPen::GetDashes(wxDash **ptr) const
{
    wxFAIL_MSG( "GetDashes not implemented" );

    *ptr = NULL;
    return 0;
}

int wxPen::GetDashCount() const
{
    wxFAIL_MSG( "GetDashCount not implemented" );

    return 0;
}

wxDash* wxPen::GetDash() const
{
    wxFAIL_MSG( "GetDash not implemented" );

    return NULL;
}

wxPenCap wxPen::GetCap() const
{
    wxCHECK_MSG( IsOk(), wxCAP_INVALID, "invalid pen" );

    wxFAIL_MSG( "GetCap not implemented" );
    return wxCAP_INVALID;
}

wxPenJoin wxPen::GetJoin() const
{
    wxCHECK_MSG( IsOk(), wxJOIN_INVALID, "invalid pen" );

    wxFAIL_MSG( "GetJoin not implemented" );
    return wxJOIN_INVALID;
}

wxPenStyle wxPen::GetStyle() const
{
    wxCHECK_MSG( IsOk(), wxPenStyle::Invalid, "invalid pen" );

    return M_PENDATA->m_style;
}

int wxPen::GetWidth() const
{
    wxCHECK_MSG( IsOk(), -1, "invalid pen" );

    return 1;
}

wxColour wxPen::GetColour() const
{
    wxCHECK_MSG( IsOk(), wxNullColour, "invalid pen" );

    return M_PENDATA->m_colour;
}

wxBitmap *wxPen::GetStipple() const
{
    wxCHECK_MSG( IsOk(), NULL, "invalid pen" );

    wxFAIL_MSG( "GetStipple not implemented" );
    return NULL;
}

wxGDIRefData *wxPen::CreateGDIRefData() const
{
    return new wxPenRefData;
}

wxGDIRefData *wxPen::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxPenRefData(*(wxPenRefData *)data);
}
