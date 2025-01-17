/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/icon.h
// Purpose:     wxIcon implementation for ports where it's same as wxBitmap
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_ICON_H_
#define _WX_GENERIC_ICON_H_

#include "wx/bitmap.h"

//-----------------------------------------------------------------------------
// wxIcon
//-----------------------------------------------------------------------------

class wxIcon: public wxBitmap
{
public:
    wxIcon();

    wxIcon(const char* const* bits);

    // For compatibility with wxMSW where desired size is sometimes required to
    // distinguish between multiple icons in a resource.
    wxIcon( const std::string& filename,
            wxBitmapType type = wxICON_DEFAULT_TYPE,
            [[maybe_unused]] int desiredWidth=-1, [[maybe_unused]] int desiredHeight=-1 ) :
        wxBitmap(filename, type)
    {
    }

    wxIcon(const wxIconLocation& loc)
        : wxBitmap(loc.GetFileName(), wxBitmapType::Any)
    {
    }

    bool LoadFile(const std::string& name, wxBitmapType flags,
                  [[maybe_unused]] int desiredWidth, [[maybe_unused]] int desiredHeight)
        { return wxBitmap::LoadFile(name, flags); }

    // unhide the base class version
    virtual bool LoadFile(const std::string& name,
                          wxBitmapType flags = wxICON_DEFAULT_TYPE) override
        { return wxBitmap::LoadFile(name, flags); }

    // create from bitmap (which should have a mask unless it's monochrome):
    // there shouldn't be any implicit bitmap -> icon conversion (i.e. no
    // ctors, assignment operators...), but it's ok to have such function
    void CopyFromBitmap(const wxBitmap& bmp);
};

#endif // _WX_GENERIC_ICON_H_
