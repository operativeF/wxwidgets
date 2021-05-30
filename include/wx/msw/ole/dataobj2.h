///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/ole/dataobj2.h
// Purpose:     second part of platform specific wxDataObject header -
//              declarations of predefined wxDataObjectSimple-derived classes
// Author:      Vadim Zeitlin
// Modified by:
// Created:     21.10.99
// Copyright:   (c) 1999 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_OLE_DATAOBJ2_H
#define _WX_MSW_OLE_DATAOBJ2_H

// ----------------------------------------------------------------------------
// wxBitmapDataObject is a specialization of wxDataObject for bitmap data
//
// NB: in fact, under MSW we support CF_DIB (and not CF_BITMAP) clipboard
//     format and we also provide wxBitmapDataObject2 for CF_BITMAP (which is
//     rarely used). This is ugly, but I haven't found a solution for it yet.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxBitmapDataObject : public wxBitmapDataObjectBase
{
public:
    // ctors
    wxBitmapDataObject(const wxBitmap& bitmap = wxNullBitmap)
        : wxBitmapDataObjectBase(bitmap)
        {
            SetFormat(wxDF_DIB);

            m_data = nullptr;
        }

    // implement base class pure virtuals
    size_t GetDataSize() const override;
    bool GetDataHere(void *buf) const override;
    bool SetData(size_t len, const void *buf) override;

    size_t GetDataSize(const wxDataFormat& WXUNUSED(format)) const override
        { return GetDataSize(); }
    virtual bool GetDataHere(const wxDataFormat& WXUNUSED(format),
                             void *buf) const override
        { return GetDataHere(buf); }
    virtual bool SetData(const wxDataFormat& WXUNUSED(format),
                         size_t len, const void *buf) override
        { return SetData(len, buf); }

private:
    // the DIB data
    void /* BITMAPINFO */ *m_data;

    wxBitmapDataObject(const wxBitmapDataObject&) = delete;
	wxBitmapDataObject& operator=(const wxBitmapDataObject&) = delete;
};

// ----------------------------------------------------------------------------
// wxBitmapDataObject2 - a data object for CF_BITMAP
//
// FIXME did I already mention it was ugly?
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxBitmapDataObject2 : public wxBitmapDataObjectBase
{
public:
    // ctors
    wxBitmapDataObject2(const wxBitmap& bitmap = wxNullBitmap)
        : wxBitmapDataObjectBase(bitmap)
        {
        }

    // implement base class pure virtuals
    size_t GetDataSize() const override;
    bool GetDataHere(void *buf) const override;
    bool SetData(size_t len, const void *buf) override;

    size_t GetDataSize(const wxDataFormat& WXUNUSED(format)) const override
        { return GetDataSize(); }
    virtual bool GetDataHere(const wxDataFormat& WXUNUSED(format),
                             void *buf) const override
        { return GetDataHere(buf); }
    virtual bool SetData(const wxDataFormat& WXUNUSED(format),
                         size_t len, const void *buf) override
        { return SetData(len, buf); }

private:
    wxBitmapDataObject2(const wxBitmapDataObject2&) = delete;
	wxBitmapDataObject2& operator=(const wxBitmapDataObject2&) = delete;
};

// ----------------------------------------------------------------------------
// wxFileDataObject - data object for CF_HDROP
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxFileDataObject : public wxFileDataObjectBase
{
public:
    wxFileDataObject() = default;

    // implement base class pure virtuals
    bool SetData(size_t len, const void *buf) override;
    size_t GetDataSize() const override;
    bool GetDataHere(void *pData) const override;
    virtual void AddFile(const wxString& file);

    size_t GetDataSize(const wxDataFormat& WXUNUSED(format)) const override
        { return GetDataSize(); }
    virtual bool GetDataHere(const wxDataFormat& WXUNUSED(format),
                             void *buf) const override
        { return GetDataHere(buf); }
    virtual bool SetData(const wxDataFormat& WXUNUSED(format),
                         size_t len, const void *buf) override
        { return SetData(len, buf); }

private:
    wxFileDataObject(const wxFileDataObject&) = delete;
	wxFileDataObject& operator=(const wxFileDataObject&) = delete;
};

// ----------------------------------------------------------------------------
// wxURLDataObject: data object for URLs
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxURLDataObject : public wxDataObjectComposite
{
public:
    // initialize with URL in ctor or use SetURL later
    wxURLDataObject(const wxString& url = wxEmptyString);

    // return the URL as string
    wxString GetURL() const;

    // Set a string as the URL in the data object
    void SetURL(const wxString& url);

    // override to set m_textFormat
    virtual bool SetData(const wxDataFormat& format,
                         size_t len,
                         const void *buf) override;

private:
    // last data object we got data in
    wxDataObjectSimple *m_dataObjectLast;

    wxURLDataObject(const wxURLDataObject&) = delete;
	wxURLDataObject& operator=(const wxURLDataObject&) = delete;
};

#endif // _WX_MSW_OLE_DATAOBJ2_H
