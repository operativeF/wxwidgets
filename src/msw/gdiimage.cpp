///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/gdiimage.cpp
// Purpose:     wxGDIImage implementation
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.11.99
// Copyright:   (c) 1999 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/log.h"
#include "wx/app.h"
#include "wx/bitmap.h"
#include "wx/icon.h"

#if wxUSE_PNG_RESOURCE_HANDLER
    #include "wx/image.h"
    #include "wx/utils.h"       // For wxLoadUserResource()
#endif

#include "wx/msw/gdiimage.h"

#if wxUSE_WXDIB
#include "wx/msw/dib.h"
#endif

#include "wx/msw/private.h"

#include <boost/nowide/convert.hpp>

import WX.WinDef;

import Utils.Strings;

import <string>;

// By default, use PNG resource handler if we can, i.e. have support for
// loading PNG images in the library. This symbol could be predefined as 0 to
// avoid doing this if anybody ever needs to do it for some reason.
#if !defined(wxUSE_PNG_RESOURCE_HANDLER)
#define wxUSE_PNG_RESOURCE_HANDLER wxUSE_LIBPNG && wxUSE_IMAGE
#endif

#include "wx/listimpl.cpp"
WX_DEFINE_LIST(wxGDIImageHandlerList)


// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// all image handlers are declared/defined in this file because the outside
// world doesn't have to know about them (but only about wxBITMAP_TYPE_XXX ids)

class wxBMPFileHandler : public wxBitmapHandler
{
public:
    wxBMPFileHandler() : wxBitmapHandler("Windows bitmap file",
                                         "bmp",
                                         wxBitmapType::BMP)
    {
    }

    bool LoadFile(wxBitmap *bitmap,
                          const std::string& name, wxBitmapType flags,
                          wxSize desiredSz) override;
    bool SaveFile(const wxBitmap *bitmap,
                          const std::string& name, wxBitmapType type,
                          const wxPalette *palette = nullptr) const override;
};

class wxBMPResourceHandler: public wxBitmapHandler
{
public:
    wxBMPResourceHandler() : wxBitmapHandler("Windows bitmap resource",
                                             "",
                                             wxBitmapType::BMP_Resource)
    {
    }

    bool LoadFile(wxBitmap *bitmap,
                          const std::string& name, wxBitmapType flags,
                          wxSize desiredSz) override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxBMPResourceHandler);
};

class wxIconHandler : public wxGDIImageHandler
{
public:
    wxIconHandler(const std::string& name, const std::string& ext, wxBitmapType type)
        : wxGDIImageHandler(name, ext, type)
    {
    }

    // creating and saving icons is not supported
    bool Create([[maybe_unused]] wxGDIImage *image,
                        [[maybe_unused]] const void* data,
                        [[maybe_unused]] wxBitmapType flags,
                        [[maybe_unused]] wxSize sz,
                        [[maybe_unused]] int depth = 1) override
    {
        return false;
    }

    bool Save([[maybe_unused]] const wxGDIImage *image,
                      [[maybe_unused]] const std::string& name,
                      [[maybe_unused]] wxBitmapType type) const override
    {
        return false;
    }

    bool Load(wxGDIImage *image,
                      const std::string& name,
                      wxBitmapType flags,
                      wxSize desiredSz) override
    {
        wxIcon *icon = wxDynamicCast(image, wxIcon);
        wxCHECK_MSG( icon, false, "wxIconHandler only works with icons" );

        return wxLoadIcon(icon, name, flags, desiredSz);
    }

protected:
    virtual bool wxLoadIcon(wxIcon *icon,
                          const std::string& name, wxBitmapType flags,
                          wxSize desiredSz = {-1, -1}) = 0;
};

class wxICOFileHandler : public wxIconHandler
{
public:
    wxICOFileHandler() : wxIconHandler("ICO icon file",
                                       "ico",
                                       wxBitmapType::ICO)
    {
    }

protected:
    bool wxLoadIcon(wxIcon *icon,
                          const std::string& name, wxBitmapType flags,
                          wxSize desiredSz = {-1, -1}) override;
};

class wxICOResourceHandler: public wxIconHandler
{
public:
    wxICOResourceHandler() : wxIconHandler("ICO resource",
                                           "ico",
                                           wxBitmapType::ICO_Resource)
    {
    }

protected:
    bool wxLoadIcon(wxIcon *icon,
                          const std::string& name, wxBitmapType flags,
                          wxSize desiredSz = {-1, -1}) override;
};

#if wxUSE_PNG_RESOURCE_HANDLER

class wxPNGResourceHandler : public wxBitmapHandler
{
public:
    wxPNGResourceHandler() : wxBitmapHandler("Windows PNG resource",
                                             "",
                                             wxBitmapType::PNG_Resource)
    {
    }

    bool LoadFile(wxBitmap *bitmap,
                          const std::string& name, wxBitmapType flags,
                          wxSize desiredSz) override;
};

#endif // wxUSE_PNG_RESOURCE_HANDLER

// ============================================================================
// implementation
// ============================================================================

wxGDIImageHandlerList wxGDIImage::ms_handlers;

// ----------------------------------------------------------------------------
// wxGDIImage functions forwarded to wxGDIImageRefData
// ----------------------------------------------------------------------------

bool wxGDIImage::FreeResource([[maybe_unused]] bool force)
{
    if ( !IsNull() )
    {
        GetGDIImageData()->Free();
        GetGDIImageData()->m_handle = nullptr;
    }

    return true;
}

WXHANDLE wxGDIImage::GetResourceHandle() const
{
    return GetHandle();
}

// ----------------------------------------------------------------------------
// wxGDIImage handler stuff
// ----------------------------------------------------------------------------

void wxGDIImage::AddHandler(wxGDIImageHandler *handler)
{
    ms_handlers.Append(handler);
}

void wxGDIImage::InsertHandler(wxGDIImageHandler *handler)
{
    ms_handlers.Insert(handler);
}

bool wxGDIImage::RemoveHandler(const std::string& name)
{
    wxGDIImageHandler *handler = FindHandler(name);
    if ( handler )
    {
        ms_handlers.DeleteObject(handler);
        return true;
    }
    else
        return false;
}

wxGDIImageHandler *wxGDIImage::FindHandler(const std::string& name)
{
    wxGDIImageHandlerList::compatibility_iterator node = ms_handlers.GetFirst();
    while ( node )
    {
        wxGDIImageHandler *handler = node->GetData();
        if ( handler->GetName() == name )
            return handler;
        node = node->GetNext();
    }

    return nullptr;
}

wxGDIImageHandler *wxGDIImage::FindHandler(const std::string& extension,
                                           long type)
{
    wxGDIImageHandlerList::compatibility_iterator node = ms_handlers.GetFirst();
    while ( node )
    {
        // FIXME: Stupid solution.
        wxGDIImageHandler *handler = node->GetData();
        if ( (handler->GetExtension() == extension) &&
             (type == -1 || static_cast<long>(handler->GetType()) == type) )
        {
            return handler;
        }

        node = node->GetNext();
    }
    return nullptr;
}

wxGDIImageHandler *wxGDIImage::FindHandler(long type)
{
    wxGDIImageHandlerList::compatibility_iterator node = ms_handlers.GetFirst();
    while ( node )
    {
        // FIXME: Stupid solution.
        wxGDIImageHandler *handler = node->GetData();
        if ( static_cast<long>(handler->GetType()) == type )
            return handler;

        node = node->GetNext();
    }

    return nullptr;
}

void wxGDIImage::CleanUpHandlers()
{
    wxGDIImageHandlerList::compatibility_iterator node = ms_handlers.GetFirst();
    while ( node )
    {
        wxGDIImageHandler *handler = node->GetData();
        wxGDIImageHandlerList::compatibility_iterator next = node->GetNext();
        delete handler;
        ms_handlers.Erase( node );
        node = next;
    }
}

void wxGDIImage::InitStandardHandlers()
{
    AddHandler(new wxBMPResourceHandler);
    AddHandler(new wxBMPFileHandler);
    AddHandler(new wxICOFileHandler);
    AddHandler(new wxICOResourceHandler);
#if wxUSE_PNG_RESOURCE_HANDLER
    AddHandler(new wxPNGResourceHandler);
#endif // wxUSE_PNG_RESOURCE_HANDLER
}

// ----------------------------------------------------------------------------
// wxBitmap handlers
// ----------------------------------------------------------------------------

bool wxBMPResourceHandler::LoadFile(wxBitmap *bitmap,
                                    const std::string& name, [[maybe_unused]] wxBitmapType flags,
                                    [[maybe_unused]] wxSize desiredSz)
{
    // TODO: load colourmap.
    WXHBITMAP hbmp = ::LoadBitmapW(wxGetInstance(), boost::nowide::widen(name).c_str());
    if ( hbmp == nullptr )
    {
        // it's probably not found
        wxLogError("Can't load bitmap '%s' from resources! Check .rc file.",
            name.c_str());

        return false;
    }

    int w, h, d;
    BITMAP bm;
    if (::GetObjectW(hbmp, sizeof(BITMAP), &bm))
    {
        w = bm.bmWidth;
        h = bm.bmHeight;
        d = bm.bmBitsPixel;
    }
    else
    {
        wxLogLastError("GetObject(WXHBITMAP)");
        w = h = d = 0;
    }

    bitmap->InitFromHBITMAP((WXHBITMAP)hbmp, wxSize{w, h}, d);

    // use 0xc0c0c0 as transparent colour by default
    bitmap->SetMask(new wxMask(*bitmap, *wxLIGHT_GREY));

    return true;
}

bool wxBMPFileHandler::LoadFile(wxBitmap *bitmap,
                                const std::string& name,
                                [[maybe_unused]] wxBitmapType flags,
                                [[maybe_unused]] wxSize desiredSz)
{
    wxCHECK_MSG( bitmap, false, "NULL bitmap in LoadFile" );

#if wxUSE_WXDIB
    // Try loading using native Windows LoadImage() first.
    wxDIB dib(name);
    if ( dib.IsOk() )
        return bitmap->CopyFromDIB(dib);
#endif // wxUSE_WXDIB

    // Some valid bitmap files are not supported by LoadImage(), e.g. those
    // with negative height. Try to use our own bitmap loading code which does
    // support them.
#if wxUSE_IMAGE
    wxImage img(name, wxBitmapType::BMP);
    if ( img.IsOk() )
    {
        *bitmap = wxBitmap(img);
        return true;
    }
#endif // wxUSE_IMAGE

    return false;
}

bool wxBMPFileHandler::SaveFile(const wxBitmap *bitmap,
                                const std::string& name,
                                [[maybe_unused]] wxBitmapType type,
                                const [[maybe_unused]] wxPalette * pal) const
{
#if wxUSE_WXDIB
    wxCHECK_MSG( bitmap, false, "NULL bitmap in SaveFile" );

    wxDIB dib(*bitmap);

    return dib.Save(name);
#else
    return false;
#endif
}

// ----------------------------------------------------------------------------
// wxIcon handlers
// ----------------------------------------------------------------------------

bool wxICOFileHandler::wxLoadIcon(wxIcon *icon,
                                const std::string& name,
                                [[maybe_unused]] wxBitmapType flags,
                                wxSize desiredSz)
{
    icon->UnRef();

    WXHICON hicon = nullptr;

    // Parse the filename: it may be of the form "filename;n" in order to
    // specify the nth icon in the file.
    //
    // For the moment, ignore the issue of possible semicolons in the
    // filename.
    int iconIndex = 0;
    std::string nameReal(name);
    std::string strIconIndex = wx::utils::AfterLast(name, ';');
    if (strIconIndex != name)
    {
        iconIndex = wxAtoi(strIconIndex);
        nameReal = wx::utils::BeforeLast(name, ';');
    }

#if 0
    // If we don't know what size icon we're looking for,
    // try to find out what's there.
    // Unfortunately this doesn't work, because ExtractIconEx
    // will scale the icon to the 'desired' size, even if that
    // size of icon isn't explicitly stored. So we would have
    // to parse the icon file outselves.
    if ( desiredWidth == -1 &&
         desiredHeight == -1)
    {
        // Try loading a large icon first
        if ( ::ExtractIconEx(nameReal, iconIndex, &hicon, NULL, 1) == 1)
        {
        }
        // Then try loading a small icon
        else if ( ::ExtractIconEx(nameReal, iconIndex, NULL, &hicon, 1) == 1)
        {
        }
    }
    else
#endif
    // were we asked for a large icon?
    const wxWindow* win = wxApp::GetMainTopWindow();
    if ( desiredSz == wxSize{wxGetSystemMetrics(SM_CXICON, win), wxGetSystemMetrics(SM_CYICON, win)} )
    {
        // get the specified large icon from file
        if ( !::ExtractIconExW(boost::nowide::widen(nameReal).c_str(), iconIndex, &hicon, nullptr, 1) )
        {
            // it is not an error, but it might still be useful to be informed
            // about it optionally
            wxLogTrace("iconload",
                       "No large icons found in the file '%s'.",
                       name.c_str());
        }
    }
    else if ( desiredSz == wxSize{wxGetSystemMetrics(SM_CXSMICON, win), wxGetSystemMetrics(SM_CYSMICON, win)} )
    {
        // get the specified small icon from file
        if ( !::ExtractIconExW(boost::nowide::widen(nameReal).c_str(), iconIndex, nullptr, &hicon, 1) )
        {
            wxLogTrace("iconload",
                       "No small icons found in the file '%s'.",
                       name.c_str());
        }
    }
    //else: not standard size, load below

    if ( !hicon )
    {
        // take any size icon from the file by index
        hicon = ::ExtractIconW(wxGetInstance(), boost::nowide::widen(nameReal).c_str(), iconIndex);
    }

    if ( !hicon )
    {
        wxLogSysError("Failed to load icon from the file '%s'",
                      name.c_str());

        return false;
    }

    if ( !icon->CreateFromHICON(hicon) )
        return false;

    // FIXME: Maybe some of these should be std::optional.
    if ( (desiredSz.x != -1 && desiredSz.x != icon->GetWidth()) ||
         (desiredSz.y != -1 && desiredSz.y != icon->GetHeight()) )
    {
        wxLogTrace("iconload",
                   "Returning false from wxICOFileHandler::Load because of the size mismatch: actual (%d, %d), requested (%d, %d)",
                   icon->GetWidth(), icon->GetHeight(),
                   desiredSz.x, desiredSz.y);

        icon->UnRef();

        return false;
    }

    return true;
}

bool wxICOResourceHandler::wxLoadIcon(wxIcon *icon,
                                    const std::string& name,
                                    [[maybe_unused]] wxBitmapType flags,
                                    wxSize desiredSz)
{
    // do we need the icon of the specific size or would any icon do?
    bool hasSize = desiredSz == wxSize{-1, -1};

    // TODO: Assertion here is questionable.
    //wxASSERT_MSG( !hasSize || (desiredWidth != -1 && desiredHeight != -1),
    //              "width and height should be either both -1 or not" );

    // try to load the icon from this program first to allow overriding the
    // standard icons (although why one would want to do it considering that
    // we already have wxApp::GetStdIcon() is unclear)

    // note that we can't just always call LoadImage() because it seems to do
    // some icon rescaling internally which results in very ugly 16x16 icons
    WXHICON hicon = [=]() {
        if ( hasSize )
        {
            return (WXHICON)::LoadImageW(wxGetInstance(), boost::nowide::widen(name).c_str(),
                                       IMAGE_ICON,
                                       desiredSz.x,
                                       desiredSz.y,
                                       LR_DEFAULTCOLOR);
        }
        else
        {
            return ::LoadIconW(wxGetInstance(), boost::nowide::widen(name).c_str());
        }
    }();

    // next check if it's not a standard icon
    if ( !hicon && !hasSize )
    {
        static const struct
        {
            std::string name;
            LPWSTR id;
        } stdIcons[] =
        {
            { "wxICON_QUESTION",          IDI_QUESTION    },
            { "wxICON_WARNING",           IDI_EXCLAMATION },
            { "wxICON_ERROR",             IDI_HAND        },
            { "wxICON_INFORMATION",       IDI_ASTERISK    },
        };

        for ( size_t nIcon = 0; !hicon && nIcon < WXSIZEOF(stdIcons); nIcon++ )
        {
            if ( name == stdIcons[nIcon].name )
            {
                hicon = ::LoadIconW((WXHINSTANCE)nullptr, stdIcons[nIcon].id);
                break;
            }
        }
    }

    return icon->CreateFromHICON((WXHICON)hicon);
}

#if wxUSE_PNG_RESOURCE_HANDLER

// ----------------------------------------------------------------------------
// PNG handler
// ----------------------------------------------------------------------------

bool wxPNGResourceHandler::LoadFile(wxBitmap *bitmap,
                                    const std::string& name,
                                    [[maybe_unused]] wxBitmapType flags,
                                    [[maybe_unused]] wxSize desiredSz)
{
    const void* pngData = nullptr;
    size_t pngSize = 0;

    // Currently we hardcode RCDATA resource type as this is what is usually
    // used for the embedded images. We could allow specifying the type as part
    // of the name in the future (e.g. "type:name" or something like this) if
    // really needed.
    if ( !wxLoadUserResource(&pngData,
                             &pngSize,
                             name,
                             RT_RCDATA,
                             wxGetInstance()) )
    {
        // Notice that this message is not translated because only the
        // programmer (and not the end user) can make any use of it.
        wxLogError("Bitmap in PNG format \"%s\" not found, check "
                   "that the resource file contains \"RCDATA\" "
                   "resource with this name.",
                   name);

        return false;
    }

    *bitmap = wxBitmap::NewFromPNGData(pngData, pngSize);
    if ( !bitmap->IsOk() )
    {
        wxLogError("Couldn't load resource bitmap \"%s\" as a PNG. "
                   "Have you registered PNG image handler?",
                   name);

        return false;
    }

    return true;
}

#endif // wxUSE_PNG_RESOURCE_HANDLER

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

wxSize wxGetHiconSize(WXHICON hicon)
{
    wxSize size;

    if ( hicon )
    {
        AutoIconInfo info;
        if ( info.GetFrom(hicon) )
        {
            WXHBITMAP hbmp = info.hbmMask;
            if ( hbmp )
            {
                BITMAP bm;
                if ( ::GetObjectW(hbmp, sizeof(BITMAP), (LPSTR) &bm) )
                {
                    size = wxSize(bm.bmWidth, bm.bmHeight);
                }
            }
            // For monochrome icon reported height is doubled
            // because it contains both AND and XOR masks.
            if ( info.hbmColor == nullptr )
            {
                size.y /= 2;
            }
        }
    }

    if ( !size.x )
    {
        // use default icon size on this hardware
        const wxWindow* win = wxApp::GetMainTopWindow();
        size.x = wxGetSystemMetrics(SM_CXICON, win);
        size.y = wxGetSystemMetrics(SM_CYICON, win);
    }

    return size;
}
