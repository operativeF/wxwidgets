/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/bmpbase.cpp
// Purpose:     wxBitmapBase
// Author:      VaclavSlavik
// Created:     2001/04/11
// Copyright:   (c) 2001, Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/bitmap.h"
#include "wx/colour.h"
#include "wx/icon.h"

#if wxUSE_IMAGE && wxUSE_LIBPNG && wxUSE_STREAMS
    #define wxHAS_PNG_LOAD
#endif

import WX.Image;
import WX.Cmn.MemStream;

// ----------------------------------------------------------------------------
// wxVariant support
// ----------------------------------------------------------------------------

#if wxUSE_VARIANT
IMPLEMENT_VARIANT_SHALLOWCMP(wxBitmap)
IMPLEMENT_VARIANT_SHALLOWCMP(wxIcon)
#endif

// ----------------------------------------------------------------------------
// wxBitmapHelpers
// ----------------------------------------------------------------------------

// wxOSX has a native version and doesn't use this one.

#ifndef __WXOSX__

/* static */
wxBitmap wxBitmapHelpers::NewFromPNGData(const void* data, size_t size)
{
    wxBitmap bitmap;

#ifdef wxHAS_PNG_LOAD
    wxMemoryInputStream is(data, size);
    wxImage image(is, wxBitmapType::PNG);
    if ( image.IsOk() )
        bitmap = wxBitmap(image);
#endif // wxHAS_PNG_LOAD

    return bitmap;
}

#endif // !__WXOSX__

// ----------------------------------------------------------------------------
// wxBitmapBase
// ----------------------------------------------------------------------------

#if wxUSE_BITMAP_BASE

#include "wx/log.h"
#include "wx/utils.h"
#include "wx/palette.h"
#include "wx/module.h"


wxIMPLEMENT_ABSTRACT_CLASS(wxBitmapBase, wxGDIObject);
wxIMPLEMENT_ABSTRACT_CLASS(wxBitmapHandler, wxObject);

void wxBitmapBase::AddHandler(wxBitmapHandler *handler)
{
    sm_handlers.Append(handler);
}

void wxBitmapBase::InsertHandler(wxBitmapHandler *handler)
{
    sm_handlers.Insert(handler);
}

bool wxBitmapBase::RemoveHandler(const std::string& name)
{
    wxBitmapHandler *handler = FindHandler(name);
    if ( handler )
    {
        sm_handlers.DeleteObject(handler);
        return true;
    }
    else
        return false;
}

wxBitmapHandler *wxBitmapBase::FindHandler(std::string_view name)
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while ( node )
    {
        wxBitmapHandler *handler = (wxBitmapHandler *)node->GetData();
        if ( handler->GetName() == name )
            return handler;
        node = node->GetNext();
    }
    return NULL;
}

wxBitmapHandler *wxBitmapBase::FindHandler(std::string_view extension, wxBitmapType bitmapType)
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while ( node )
    {
        wxBitmapHandler *handler = (wxBitmapHandler *)node->GetData();
        if ( handler->GetExtension() == extension &&
                    (bitmapType == wxBitmapType::Any || handler->GetType() == bitmapType) )
            return handler;
        node = node->GetNext();
    }
    return NULL;
}

wxBitmapHandler *wxBitmapBase::FindHandler(wxBitmapType bitmapType)
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while ( node )
    {
        wxBitmapHandler *handler = (wxBitmapHandler *)node->GetData();
        if (handler->GetType() == bitmapType)
            return handler;
        node = node->GetNext();
    }
    return NULL;
}

void wxBitmapBase::CleanUpHandlers()
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while ( node )
    {
        wxBitmapHandler *handler = (wxBitmapHandler *)node->GetData();
        wxList::compatibility_iterator next = node->GetNext();
        delete handler;
        sm_handlers.Erase(node);
        node = next;
    }
}

class wxBitmapBaseModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxBitmapBaseModule);
public:
    bool OnInit() override { wxBitmap::InitStandardHandlers(); return true; }
    void OnExit() override { wxBitmap::CleanUpHandlers(); }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxBitmapBaseModule, wxModule);

#endif // wxUSE_BITMAP_BASE

// ----------------------------------------------------------------------------
// wxBitmap common
// ----------------------------------------------------------------------------

#if !(defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXX11__) || defined(__WXQT__))

wxBitmap::wxBitmap(const char* const* bits)
{
    wxCHECK2_MSG(bits != nullptr, return, "invalid bitmap data");

#if wxUSE_IMAGE && wxUSE_XPM
    wxImage image(bits);
    wxCHECK2_MSG(image.IsOk(), return, "invalid bitmap data");

    *this = wxBitmap(image);
#else
    wxFAIL_MSG("creating bitmaps from XPMs not supported");
#endif // wxUSE_IMAGE && wxUSE_XPM
}
#endif // !(defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXX11__))

// ----------------------------------------------------------------------------
// wxMaskBase
// ----------------------------------------------------------------------------

bool wxMaskBase::Create(const wxBitmap& bitmap, const wxColour& colour)
{
    FreeData();

    return InitFromColour(bitmap, colour);
}

#if wxUSE_PALETTE

bool wxMaskBase::Create(const wxBitmap& bitmap, int paletteIndex)
{
    wxPalette *pal = bitmap.GetPalette();

    wxCHECK_MSG( pal, false,
                 "Cannot create mask from palette index of a bitmap without palette" );

    unsigned char r{};
    unsigned char g{};
    unsigned char b{};
    pal->GetRGB(paletteIndex, &r, &g, &b);

    return Create(bitmap, wxColour(r, g, b));
}

#endif // wxUSE_PALETTE

bool wxMaskBase::Create(const wxBitmap& bitmap)
{
    FreeData();

    return InitFromMonoBitmap(bitmap);
}
