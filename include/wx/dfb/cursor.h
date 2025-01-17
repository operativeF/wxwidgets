/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dfb/cursor.h
// Purpose:     wxCursor declaration
// Author:      Vaclav Slavik
// Created:     2006-08-08
// Copyright:   (c) 2006 REA Elektronik GmbH
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DFB_CURSOR_H_
#define _WX_DFB_CURSOR_H_

class wxBitmap;
class wxImage;

//-----------------------------------------------------------------------------
// wxCursor
//-----------------------------------------------------------------------------

class wxCursor : public wxCursorBase
{
public:
    wxCursor() {}
    wxCursor(wxStockCursor id) { InitFromStock(id); }
#if wxUSE_IMAGE
    wxCursor(const wxImage& image);
    wxCursor(const char* const* xpmData);
#endif // wxUSE_IMAGE
    wxCursor(const wxString& name,
             wxBitmapType type = wxCURSOR_DEFAULT_TYPE,
             int hotSpotX = 0, int hotSpotY = 0);

    // implementation
    wxBitmap GetBitmap() const;

protected:
    void InitFromStock(wxStockCursor);

    // ref counting code
    virtual wxGDIRefData *CreateGDIRefData() const;
    virtual wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const;

    wxDECLARE_DYNAMIC_CLASS(wxCursor);
};

#endif // _WX_DFB_CURSOR_H_
