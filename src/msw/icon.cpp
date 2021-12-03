/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/icon.cpp
// Purpose:     wxIcon class
// Author:      Julian Smart
// Modified by: 20.11.99 (VZ): don't derive from wxBitmap any more
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/utils.h"
#include "wx/icon.h"
#include "wx/bitmap.h"
#include "wx/log.h"

#include "wx/msw/private.h"

#include <fmt/core.h>

import WX.WinDef;

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxIconRefData
// ----------------------------------------------------------------------------

void wxIconRefData::Free()
{
    if ( m_hIcon )
    {
        ::DestroyIcon((WXHICON) m_hIcon);

        m_hIcon = nullptr;
    }
}

// ----------------------------------------------------------------------------
// wxIcon
// ----------------------------------------------------------------------------

wxIcon::wxIcon(const char bits[], wxSize sz)
{
    wxBitmap bmp(bits, sz);
    CopyFromBitmap(bmp);
}

wxIcon::wxIcon(const std::string& iconfile,
               wxBitmapType type,
               wxSize desiredSz)

{
    LoadFile(iconfile, type, desiredSz);
}

wxIcon::wxIcon(const wxIconLocation& loc)
{
    // wxICOFileHandler accepts names in the format "filename;index"
    std::string fullname = loc.GetFileName();
    if ( loc.GetIndex() != 0 )
    {
        fullname += fmt::format(";{}", loc.GetIndex());
    }
    //else: 0 is default

    LoadFile(fullname, wxBitmapType::ICO);
}

wxObjectRefData *wxIcon::CloneRefData(const wxObjectRefData *dataOrig) const
{
    const auto* data = dynamic_cast<const wxIconRefData *>(dataOrig);
    if ( !data )
        return nullptr;

    // we don't have to copy m_hIcon because we're only called from SetHICON()
    // which overwrites m_hIcon anyhow currently
    //
    // and if we're called from SetWidth/Height/Depth(), it doesn't make sense
    // to copy it neither as the handle would be inconsistent with the new size
    return new wxIconRefData(*data);
}

void wxIcon::CopyFromBitmap(const wxBitmap& bmp)
{
    WXHICON hicon = wxBitmapToHICON(bmp);
    if ( !hicon )
    {
        wxLogLastError("CreateIconIndirect");
    }
    else
    {
        InitFromHICON((WXHICON)hicon, bmp.GetSize());
    }
}

void wxIcon::CreateIconFromXpm(const char* const* data)
{
    wxBitmap bmp(data);
    CopyFromBitmap(bmp);
}

bool wxIcon::LoadFile(const std::string& filename,
                      wxBitmapType type,
                      wxSize desiredSz)
{
    UnRef();

    // FIXME: Stupid solution.
    wxGDIImageHandler *handler = FindHandler(static_cast<int>(type));

    if ( !handler )
    {
        // load via wxBitmap which, in turn, uses wxImage allowing us to
        // support more formats
        wxBitmap bmp;
        if ( !bmp.LoadFile(filename, type) )
            return false;

        CopyFromBitmap(bmp);
        return true;
    }

    return handler->Load(this, filename, type, desiredSz);
}

bool wxIcon::CreateFromHICON(WXHICON icon)
{
    return InitFromHICON(icon, wxGetHiconSize(icon));
}

bool wxIcon::InitFromHICON(WXHICON icon, wxSize sz)
{
#if wxDEBUG_LEVEL >= 2
    if ( icon != NULL )
    {
        wxSize size = wxGetHiconSize(icon);
        wxASSERT_MSG(size.GetWidth() == width && size.GetHeight() == height,
                     "Inconsistent icon parameters");
    }
#endif // wxDEBUG_LEVEL >= 2

    AllocExclusive();

    GetGDIImageData()->m_handle = (WXHANDLE)icon;
    GetGDIImageData()->m_size   = sz;

    return IsOk();
}
