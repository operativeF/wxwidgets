/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/statbmp.h
// Purpose:
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __GTKSTATICBITMAPH__
#define __GTKSTATICBITMAPH__

#include "wx/icon.h"

//-----------------------------------------------------------------------------
// wxStaticBitmap
//-----------------------------------------------------------------------------

class wxStaticBitmap : public wxStaticBitmapBase
{
public:
    wxStaticBitmap();
    wxStaticBitmap( wxWindow *parent,
                    wxWindowID id,
                    const wxBitmap& label,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = 0,
                    const wxString& name = wxASCII_STR(wxStaticBitmapNameStr));
    bool Create( wxWindow *parent,
                 wxWindowID id,
                 const wxBitmap& label,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = 0,
                 const wxString& name = wxASCII_STR(wxStaticBitmapNameStr));

    void SetIcon(const wxIcon& icon) override { SetBitmap( icon ); }
    void SetBitmap( const wxBitmap& bitmap ) override;
    wxBitmap GetBitmap() const override { return m_bitmap; }

    // for compatibility with wxMSW
    wxIcon GetIcon() const override
    {
        // don't use wxDynamicCast, icons and bitmaps are really the same thing
        // in wxGTK
        return (const wxIcon &)m_bitmap;
    }

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWINDOW_VARIANT_NORMAL);

private:
    wxBitmap   m_bitmap;

    wxDECLARE_DYNAMIC_CLASS(wxStaticBitmap);
};

#endif // __GTKSTATICBITMAPH__
