/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/cursor.h
// Purpose:     wxCursor class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CURSOR_H_
#define _WX_CURSOR_H_

class WXDLLIMPEXP_FWD_CORE wxImage;

class WXDLLIMPEXP_CORE wxCursor : public wxCursorBase
{
public:
    wxCursor() = default;
#if wxUSE_IMAGE
    wxCursor(const wxImage& image);
    wxCursor(const char* const* xpmData);
#endif // wxUSE_IMAGE
    wxCursor(const std::string& name,
             wxBitmapType type = wxCURSOR_DEFAULT_TYPE,
             int hotSpotX = 0, int hotSpotY = 0);
    wxCursor(wxStockCursor id) { InitFromStock(id); }

    wxPoint GetHotSpot() const override;

    ~wxCursor() override = default;

    void SetHCURSOR(WXHCURSOR cursor) { SetHandle((WXHANDLE)cursor); }
    WXHCURSOR GetHCURSOR() const { return (WXHCURSOR)GetHandle(); }

protected:
    void InitFromStock(wxStockCursor);

    wxGDIImageRefData *CreateData() const override;

private:
#if wxUSE_IMAGE
    void InitFromImage(const wxImage& image);
#endif // wxUSE_IMAGE

    wxDECLARE_DYNAMIC_CLASS(wxCursor);
};

#endif // _WX_CURSOR_H_
