/////////////////////////////////////////////////////////////////////////////
// Name:        wx/x11/cursor.h
// Purpose:     wxCursor class
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CURSOR_H_
#define _WX_CURSOR_H_

#include "wx/colour.h"

class WXDLLIMPEXP_FWD_CORE wxImage;

//-----------------------------------------------------------------------------
// wxCursor
//-----------------------------------------------------------------------------

class wxCursor : public wxCursorBase
{
public:
    wxCursor();
    wxCursor(wxStockCursor id) { InitFromStock(id); }
#if wxUSE_IMAGE
    wxCursor( const wxImage & image );
    wxCursor(const char* const* xpmData);
#endif

    wxCursor(const wxString& name,
             wxBitmapType type = wxCURSOR_DEFAULT_TYPE,
             int hotSpotX = 0, int hotSpotY = 0);
    virtual ~wxCursor();

    // implementation

    WXCursor GetCursor() const;

protected:
    void InitFromStock(wxStockCursor);

    virtual wxGDIRefData *CreateGDIRefData() const;
    virtual wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const;

private:
    wxDECLARE_DYNAMIC_CLASS(wxCursor);
};

#endif // _WX_CURSOR_H_
