///////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/dataobj2.h
// Purpose:     declaration of standard wxDataObjectSimple-derived classes
// Author:      Robert Roebling
// Created:     19.10.99 (extracted from gtk/dataobj.h)
// Copyright:   (c) 1998, 1999 Vadim Zeitlin, Robert Roebling
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_DATAOBJ2_H_
#define _WX_GTK_DATAOBJ2_H_

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

    size_t GetDataSize() const override { return m_pngSize; }
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
    void Clear() { free(m_pngData); }
    void ClearAll() { Clear(); Init(); }

    size_t      m_pngSize;
    void       *m_pngData;

    void DoConvertToPng();

private:
    void Init() { m_pngData = NULL; m_pngSize = 0; }
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
};

// ----------------------------------------------------------------------------
// wxURLDataObject is a specialization of wxDataObject for URLs
// ----------------------------------------------------------------------------

class wxURLDataObject : public wxDataObjectComposite
{
public:
    wxURLDataObject(const wxString& url = wxEmptyString);

    wxString GetURL() const;
    void SetURL(const wxString& url);

private:
    class wxTextURIListDataObject* const m_dobjURIList;
    wxTextDataObject* const m_dobjText;

    wxURLDataObject(const wxURLDataObject&) = delete;
	wxURLDataObject& operator=(const wxURLDataObject&) = delete;
};


#endif // _WX_GTK_DATAOBJ2_H_

