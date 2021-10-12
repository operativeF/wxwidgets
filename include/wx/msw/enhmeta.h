///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/enhmeta.h
// Purpose:     wxEnhMetaFile class for Win32
// Author:      Vadim Zeitlin
// Modified by:
// Created:     13.01.00
// Copyright:   (c) 2000 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_ENHMETA_H_
#define _WX_MSW_ENHMETA_H_

#include "wx/defs.h"

#if wxUSE_ENH_METAFILE

#include "wx/dc.h"
#include "wx/gdiobj.h"
#include "wx/geometry/rect.h"

#if wxUSE_DATAOBJ
    #include "wx/dataobj.h"
#endif

#include <string>


// ----------------------------------------------------------------------------
// wxEnhMetaFile: encapsulation of Win32 HENHMETAFILE
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxEnhMetaFile : public wxGDIObject
{
public:
    wxEnhMetaFile(const std::string& file = {}) : m_filename(file)
        { Init(); }
    wxEnhMetaFile(const wxEnhMetaFile& metafile) : wxGDIObject()
        { Init(); Assign(metafile); }
    wxEnhMetaFile& operator=(const wxEnhMetaFile& metafile)
        { Free(); Assign(metafile); return *this; }

    ~wxEnhMetaFile()
        { Free(); }

    // display the picture stored in the metafile on the given DC
    bool Play(wxDC *dc, wxRect *rectBound = nullptr);

    bool IsOk() const override { return m_hMF != nullptr; }

    wxSize GetSize() const;
    int GetWidth() const { return GetSize().x; }
    int GetHeight() const { return GetSize().y; }

    const std::string& GetFileName() const { return m_filename; }

    // copy the metafile to the clipboard: the width and height parameters are
    // for backwards compatibility (with wxMetaFile) only, they are ignored by
    // this method
    bool SetClipboard(int width = 0, int height = 0);

    // Detach the HENHMETAFILE from this object, i.e. don't delete the handle
    // in the dtor -- the caller is now responsible for doing this, e.g. using
    // Free() method below.
    WXHANDLE Detach() { WXHANDLE h = m_hMF; m_hMF = nullptr; return h; }

    // Destroy the given HENHMETAFILE object.
    static void Free(WXHANDLE handle);

    // implementation
    WXHANDLE GetHENHMETAFILE() const { return m_hMF; }
    void SetHENHMETAFILE(WXHANDLE hMF) { Free(); m_hMF = hMF; }

protected:
    // FIXME: Protected Init
    void Init();
    void Free() { Free(m_hMF); }
    void Assign(const wxEnhMetaFile& mf);

    // we don't use these functions (but probably should) but have to implement
    // them as they're pure virtual in the base class
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;

private:
    std::string m_filename;
    WXHANDLE m_hMF;

    wxDECLARE_DYNAMIC_CLASS(wxEnhMetaFile);
};

// ----------------------------------------------------------------------------
// wxEnhMetaFileDC: allows to create a wxEnhMetaFile
// ----------------------------------------------------------------------------

struct WXDLLIMPEXP_CORE wxEnhMetaFileDC : public wxDC
{
    // the ctor parameters specify the filename (empty for memory metafiles),
    // the metafile picture size and the optional description/comment
    wxEnhMetaFileDC(const std::string& filename = {},
                    int width = 0, int height = 0,
                    const std::string& description = {});

    // as above, but takes reference DC as first argument to take resolution,
    // size, font metrics etc. from
    explicit
    wxEnhMetaFileDC(const wxDC& referenceDC,
                    const std::string& filename = {},
                    int width = 0, int height = 0,
                    const std::string& description = {});

    wxEnhMetaFileDC(const wxEnhMetaFileDC&) = delete;
    wxEnhMetaFileDC& operator=(const wxEnhMetaFileDC&) = delete;
    wxEnhMetaFileDC(wxEnhMetaFileDC&&) = default;
    wxEnhMetaFileDC& operator=(wxEnhMetaFileDC&&) = default;

    // obtain a pointer to the new metafile (caller should delete it)
    wxEnhMetaFile *Close();

	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#if wxUSE_DATAOBJ

// ----------------------------------------------------------------------------
// wxEnhMetaFileDataObject is a specialization of wxDataObject for enh metafile
// ----------------------------------------------------------------------------

// notice that we want to support both CF_METAFILEPICT and CF_ENHMETAFILE and
// so we derive from wxDataObject and not from wxDataObjectSimple
class WXDLLIMPEXP_CORE wxEnhMetaFileDataObject : public wxDataObject
{
public:
    // ctors
    wxEnhMetaFileDataObject() = default;
    wxEnhMetaFileDataObject(const wxEnhMetaFile& metafile)
        : m_metafile(metafile) { }

    wxEnhMetaFileDataObject(const wxEnhMetaFileDataObject&) = delete;
    wxEnhMetaFileDataObject& operator=(const wxEnhMetaFileDataObject&) = delete;

    // functions which you may override if you want to provide data on
    // demand only - otherwise, the trivial default versions will be used
    virtual void SetMetafile(const wxEnhMetaFile& metafile)
        { m_metafile = metafile; }
    virtual wxEnhMetaFile GetMetafile() const
        { return m_metafile; }

    
    wxDataFormat GetPreferredFormat(Direction dir) const override;
    size_t GetFormatCount(Direction dir) const override;
    void GetAllFormats(wxDataFormat *formats, Direction dir) const override;
    size_t GetDataSize(const wxDataFormat& format) const override;
    bool GetDataHere(const wxDataFormat& format, void *buf) const override;
    bool SetData(const wxDataFormat& format, size_t len,
                         const void *buf) override;

protected:
    wxEnhMetaFile m_metafile;
};


// ----------------------------------------------------------------------------
// wxEnhMetaFileSimpleDataObject does derive from wxDataObjectSimple which
// makes it more convenient to use (it can be used with wxDataObjectComposite)
// at the price of not supoprting any more CF_METAFILEPICT but only
// CF_ENHMETAFILE
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxEnhMetaFileSimpleDataObject : public wxDataObjectSimple
{
public:
    // ctors
    wxEnhMetaFileSimpleDataObject() : wxDataObjectSimple(wxDF_ENHMETAFILE) { }
    wxEnhMetaFileSimpleDataObject(const wxEnhMetaFile& metafile)
        : wxDataObjectSimple(wxDF_ENHMETAFILE), m_metafile(metafile) { }
    
    wxEnhMetaFileSimpleDataObject(const wxEnhMetaFileSimpleDataObject&) = delete;
    wxEnhMetaFileSimpleDataObject& operator=(const wxEnhMetaFileSimpleDataObject&) = delete;

    // functions which you may override if you want to provide data on
    // demand only - otherwise, the trivial default versions will be used
    virtual void SetEnhMetafile(const wxEnhMetaFile& metafile)
        { m_metafile = metafile; }
    virtual wxEnhMetaFile GetEnhMetafile() const
        { return m_metafile; }

    
    size_t GetDataSize() const override;
    bool GetDataHere(void *buf) const override;
    bool SetData(size_t len, const void *buf) override;

    size_t GetDataSize(const wxDataFormat& WXUNUSED(format)) const override
        { return GetDataSize(); }
    bool GetDataHere(const wxDataFormat& WXUNUSED(format),
                             void *buf) const override
        { return GetDataHere(buf); }
    bool SetData(const wxDataFormat& WXUNUSED(format),
                         size_t len, const void *buf) override
        { return SetData(len, buf); }

protected:
    wxEnhMetaFile m_metafile;
};

#endif // wxUSE_DATAOBJ

#endif // wxUSE_ENH_METAFILE

#endif // _WX_MSW_ENHMETA_H_
