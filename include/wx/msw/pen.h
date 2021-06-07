/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/pen.h
// Purpose:     wxPen class
// Author:      Julian Smart
// Modified by: Vadim Zeitlin: fixed operator=(), ==(), !=()
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PEN_H_
#define _WX_PEN_H_

#include "wx/gdiobj.h"
#include "wx/gdicmn.h"

// ----------------------------------------------------------------------------
// Pen
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPen : public wxPenBase
{
public:
    wxPen() = default;
    wxPen(const wxColour& col, int width = 1, wxPenStyle style = wxPenStyle::Solid);

    wxPen(const wxBitmap& stipple, int width);

    wxPen(const wxPenInfo& info);

    ~wxPen() override = default;

    bool operator==(const wxPen& pen) const;
    bool operator!=(const wxPen& pen) const { return !(*this == pen); }

    // Override in order to recreate the pen
    void SetColour(const wxColour& col) override;
    void SetColour(unsigned char r, unsigned char g, unsigned char b) override;

    void SetWidth(int width) override;
    void SetStyle(wxPenStyle style) override;
    void SetStipple(const wxBitmap& stipple) override;
    void SetDashes(int nb_dashes, const wxDash *dash) override;
    void SetJoin(wxPenJoin join) override;
    void SetCap(wxPenCap cap) override;
    void SetQuality(wxPenQuality quality) override;

    wxColour GetColour() const override;
    int GetWidth() const override;
    wxPenStyle GetStyle() const override;
    wxPenJoin GetJoin() const override;
    wxPenCap GetCap() const override;
    wxPenQuality GetQuality() const override;
    int GetDashes(wxDash** ptr) const override;
    wxDash* GetDash() const;
    int GetDashCount() const;
    wxBitmap* GetStipple() const override;

    // internal: wxGDIObject methods
    bool RealizeResource() override;
    bool FreeResource(bool force = false) override;
    WXHANDLE GetResourceHandle() const override;
    bool IsFree() const override;

protected:
    wxGDIRefData* CreateGDIRefData() const override;
    wxGDIRefData* CloneGDIRefData(const wxGDIRefData* data) const override;

    // same as FreeResource() + RealizeResource()
    bool Recreate();

    wxDECLARE_DYNAMIC_CLASS(wxPen);
};

#endif // _WX_PEN_H_
