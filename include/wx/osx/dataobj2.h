///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/dataobj2.h
// Purpose:     declaration of standard wxDataObjectSimple-derived classes
// Author:      David Webster (adapted from Robert Roebling's gtk port
// Modified by:
// Created:     10/21/99
// Copyright:   (c) 1998, 1999 Vadim Zeitlin, Robert Roebling
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MAC_DATAOBJ2_H_
#define _WX_MAC_DATAOBJ2_H_

// ----------------------------------------------------------------------------
// wxBitmapDataObject is a specialization of wxDataObject for bitmaps
// ----------------------------------------------------------------------------

class wxBitmapDataObject : public wxBitmapDataObjectBase
{
public:
    // ctors
    wxBitmapDataObject();
    wxBitmapDataObject(const wxBitmap& bitmap);

    // destr
    virtual ~wxBitmapDataObject();

    // override base class virtual to update PNG data too
    void SetBitmap(const wxBitmap& bitmap) override;

    
    // ----------------------------------

    size_t GetDataSize() const override;
    bool GetDataHere(void *buf) const override;
    bool SetData(size_t len, const void *buf) override;
    // Must provide overloads to avoid hiding them (and warnings about it)
    size_t GetDataSize(const wxDataFormat&) const override
    {
        return GetDataSize();
    }
    bool GetDataHere(const wxDataFormat&, void *buf) const override
    {
        return GetDataHere(buf);
    }
    bool SetData(const wxDataFormat&, size_t len, const void *buf) override
    {
        return SetData(len, buf);
    }

protected :
    void Init() ;
    void Clear() ;

    CFDataRef m_pictData ;
};

// ----------------------------------------------------------------------------
// wxFileDataObject is a specialization of wxDataObject for file names
// ----------------------------------------------------------------------------

class wxFileDataObject : public wxFileDataObjectBase
{
public:
    
    // ----------------------------------

    void AddFile( const wxString &filename );

    size_t GetDataSize() const override;
    bool GetDataHere(void *buf) const override;
    bool SetData(size_t len, const void *buf) override;
    // Must provide overloads to avoid hiding them (and warnings about it)
    size_t GetDataSize(const wxDataFormat&) const override
    {
        return GetDataSize();
    }
    bool GetDataHere(const wxDataFormat&, void *buf) const override
    {
        return GetDataHere(buf);
    }
    bool SetData(const wxDataFormat&, size_t len, const void *buf) override
    {
        return SetData(len, buf);
    }
protected:
    // translates the filenames stored into a utf8 encoded char stream
    void GetFileNames(wxCharBuffer &buf) const ;
};

#endif // _WX_MAC_DATAOBJ2_H_

