/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/icon.h
// Purpose:     wxIcon class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ICON_H_
#define _WX_ICON_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/msw/gdiimage.h"

import WX.WinDef;

import <string>;

// ---------------------------------------------------------------------------
// icon data
// ---------------------------------------------------------------------------

// notice that although wxIconRefData inherits from wxBitmapRefData, it is not
// a valid wxBitmapRefData
struct wxIconRefData : public wxGDIImageRefData
{
    wxIconRefData() = default;
    ~wxIconRefData() { Free(); }

    void Free() override;
};

// ---------------------------------------------------------------------------
// Icon
// ---------------------------------------------------------------------------

class wxIcon : public wxGDIImage
{
public:
    wxIcon() = default;

        // from raw data
    explicit wxIcon(const char bits[], wxSize sz);

        // from XPM data
    explicit wxIcon(const char* const* data) { CreateIconFromXpm(data); }
        // from resource/file
    wxIcon(const std::string& name,
           wxBitmapType type = wxICON_DEFAULT_TYPE,
           wxSize desiredSz = {-1, -1});

    explicit wxIcon(const wxIconLocation& loc);

    virtual bool LoadFile(const std::string& name,
                          wxBitmapType type = wxICON_DEFAULT_TYPE,
                          wxSize desiredSz = {-1, -1});

    bool CreateFromHICON(WXHICON icon);

    // implementation only from now on
    wxIconRefData *GetIconData() const { return (wxIconRefData *)m_refData; }

    WXHICON GetHICON() const { return (WXHICON)GetHandle(); }
    bool InitFromHICON(WXHICON icon, wxSize sz);

    // create from bitmap (which should have a mask unless it's monochrome):
    // there shouldn't be any implicit bitmap -> icon conversion (i.e. no
    // ctors, assignment operators...), but it's ok to have such function
    void CopyFromBitmap(const wxBitmap& bmp);

protected:
    wxGDIImageRefData *CreateData() const override
    {
        return new wxIconRefData;
    }

    wxObjectRefData *CloneRefData(const wxObjectRefData *data) const override;

    // create from XPM data
    void CreateIconFromXpm(const char* const* data);
};

inline const wxIcon wxNullIcon;

#endif
    // _WX_ICON_H_
