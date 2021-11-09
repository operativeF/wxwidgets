///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/gdiimage.h
// Purpose:     wxGDIImage class: base class for wxBitmap, wxIcon, wxCursor
//              under MSW
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.11.99
// Copyright:   (c) 1999 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// NB: this is a private header, it is not intended to be directly included by
//     user code (but may be included from other, public, wxWin headers

#ifndef _WX_MSW_GDIIMAGE_H_
#define _WX_MSW_GDIIMAGE_H_

#include "wx/gdiobj.h"          // base class
#include "wx/gdicmn.h"          // wxBitmapType::Invalid
#include "wx/list.h"
#include "wx/geometry/size.h"

import <string>;

struct wxGDIImageRefData;
class wxGDIImageHandler;
class wxGDIImage;

WX_DECLARE_EXPORTED_LIST(wxGDIImageHandler, wxGDIImageHandlerList);

// ----------------------------------------------------------------------------
// wxGDIImageRefData: common data fields for all derived classes
// ----------------------------------------------------------------------------

struct wxGDIImageRefData : public wxGDIRefData
{
    wxGDIImageRefData() = default;

    wxGDIImageRefData(const wxGDIImageRefData& data)  
    {
        m_size = data.m_size;
        m_depth = data.m_depth;

        // can't copy handles like this, derived class copy ctor must do it!
        m_handle = nullptr;
    }

    bool IsOk() const override { return m_handle != nullptr; }

    void SetSize(wxSize sz) { m_size = sz; }

    virtual void Free() = 0;

    // for compatibility, the member fields are public

    // the handle to it
    union
    {
        WXHANDLE  m_handle;     // for untyped access
        WXHBITMAP m_hBitmap;
        WXHICON   m_hIcon;
        WXHCURSOR m_hCursor;
    };
    
    // the size of the image
    wxSize m_size{0, 0};

    // the depth of the image
    int m_depth{0};
};

// ----------------------------------------------------------------------------
// wxGDIImage: this class supports GDI image handlers which may be registered
// dynamically and will be used for loading/saving the images in the specified
// format. It also falls back to wxImage if no appropriate image is found.
// ----------------------------------------------------------------------------

class wxGDIImage : public wxGDIObject
{
public:
    // handlers list interface
    static wxGDIImageHandlerList& GetHandlers() { return ms_handlers; }

    static void AddHandler(wxGDIImageHandler *handler);
    static void InsertHandler(wxGDIImageHandler *handler);
    static bool RemoveHandler(const std::string& name);

    static wxGDIImageHandler *FindHandler(const std::string& name);
    static wxGDIImageHandler *FindHandler(const std::string& extension, long type);
    static wxGDIImageHandler *FindHandler(long type);

    static void InitStandardHandlers();
    static void CleanUpHandlers();

    // access to the ref data casted to the right type
    wxGDIImageRefData *GetGDIImageData() const
        { return (wxGDIImageRefData *)m_refData; }

    WXHANDLE GetHandle() const
        { return IsNull() ? nullptr : GetGDIImageData()->m_handle; }
    void SetHandle(WXHANDLE handle)
        { AllocExclusive(); GetGDIImageData()->m_handle = handle; }

    int GetWidth() const { return IsNull() ? 0 : GetGDIImageData()->m_size.x; }
    int GetHeight() const { return IsNull() ? 0 : GetGDIImageData()->m_size.y; }
    int GetDepth() const { return IsNull() ? 0 : GetGDIImageData()->m_depth; }

    wxSize GetSize() const
    {
        return IsNull() ? wxSize{0, 0} : GetGDIImageData()->m_size;
    }

    // forward some of base class virtuals to wxGDIImageRefData
    bool FreeResource(bool force = false) override;
    WXHANDLE GetResourceHandle() const override;

protected:
    // create the data for the derived class here
    virtual wxGDIImageRefData *CreateData() const = 0;

    // implement the wxGDIObject method in terms of our, more specific, one
    wxGDIRefData *CreateGDIRefData() const override { return CreateData(); }

    // we can't [efficiently] clone objects of this class
    wxGDIRefData *
    CloneGDIRefData(const wxGDIRefData *WXUNUSED(data)) const override
    {
        wxFAIL_MSG( "must be implemented if used" );

        return nullptr;
    }

    static wxGDIImageHandlerList ms_handlers;
};

// ----------------------------------------------------------------------------
// wxGDIImageHandler: a class which knows how to load/save wxGDIImages.
// ----------------------------------------------------------------------------

class wxGDIImageHandler
{
public:
    wxGDIImageHandler() = default;
    wxGDIImageHandler(const std::string& name,
                      const std::string& ext,
                      wxBitmapType type)
        : m_name(name), m_extension(ext), m_type(type) { }

    virtual ~wxGDIImageHandler() = default;
    
    void SetName(const std::string& name) { m_name = name; }
    void SetExtension(const std::string& ext) { m_extension = ext; }
    void SetType(wxBitmapType type) { m_type = type; }

    const std::string& GetName() const { return m_name; }
    const std::string& GetExtension() const { return m_extension; }
    wxBitmapType GetType() const { return m_type; }

    // real handler operations: to implement in derived classes
    [[maybe_unused]] virtual bool Create(wxGDIImage *image,
                        const void* data,
                        wxBitmapType flags,
                        wxSize sz, int depth = 1) = 0;
    virtual bool Load(wxGDIImage *image,
                      const std::string& name,
                      wxBitmapType flags,
                      wxSize desiredSz) = 0;
    virtual bool Save(const wxGDIImage *image,
                      const std::string& name,
                      wxBitmapType type) const = 0;

private:
    std::string  m_name;
    std::string  m_extension;

    wxBitmapType m_type{wxBitmapType::Invalid};
};

#endif // _WX_MSW_GDIIMAGE_H_
