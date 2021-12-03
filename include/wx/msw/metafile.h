/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/metafile.h
// Purpose:     wxMetaFile, wxMetaFileDC and wxMetaFileDataObject classes
// Author:      Julian Smart
// Modified by: VZ 07.01.00: implemented wxMetaFileDataObject
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_METAFIILE_H_
#define _WX_METAFIILE_H_

#include "wx/dc.h"
#include "wx/gdiobj.h"

#if wxUSE_DATAOBJ
    #include "wx/dataobj.h"
#endif

import <string>;

class wxMetafile;

struct wxMetafileRefData: public wxGDIRefData
{
    wxMetafileRefData();
    virtual ~wxMetafileRefData();

    bool IsOk() const override { return m_metafile != 0; }

    WXHANDLE m_metafile{nullptr};
    int m_windowsMappingMode{MM_ANISOTROPIC};
    int m_width{0};
    int m_height{0};

    friend class wxMetafile;
};

#define M_METAFILEDATA ((wxMetafileRefData *)m_refData)

class wxMetafile: public wxGDIObject
{
public:
    wxMetafile(const std::string& file = {});

    // After this is called, the metafile cannot be used for anything
    // since it is now owned by the clipboard.
    virtual bool SetClipboard(int width = 0, int height = 0);

    virtual bool Play(wxDC *dc);

    // set/get the size of metafile for clipboard operations
    wxSize GetSize() const { return wxSize(GetWidth(), GetHeight()); }
    int GetWidth() const { return M_METAFILEDATA->m_width; }
    int GetHeight() const { return M_METAFILEDATA->m_height; }

    void SetWidth(int width) { M_METAFILEDATA->m_width = width; }
    void SetHeight(int height) { M_METAFILEDATA->m_height = height; }

    // Implementation
    WXHANDLE GetHMETAFILE() const { return M_METAFILEDATA->m_metafile; }
    void SetHMETAFILE(WXHANDLE mf) ;
    int GetWindowsMappingMode() const { return M_METAFILEDATA->m_windowsMappingMode; }
    void SetWindowsMappingMode(int mm);

protected:
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;
};

class wxMetafileDCImpl: public wxMSWDCImpl
{
public:
    wxMetafileDCImpl(wxDC *owner, const std::string& file = {});
    wxMetafileDCImpl(wxDC *owner, const std::string& file,
                     int xext, int yext, int xorg, int yorg);
    ~wxMetafileDCImpl();

	wxMetafileDCImpl& operator=(wxMetafileDCImpl&&) = delete;

    virtual wxMetafile *Close();
    void SetMapMode(wxMappingMode mode) override;
    virtual void DoGetTextExtent(const wxString& string,
                                 wxCoord *x, wxCoord *y,
                                 wxCoord *descent = NULL,
                                 wxCoord *externalLeading = NULL,
                                 const wxFont *theFont = NULL) const override;

    // Implementation
    wxMetafile *GetMetaFile() const { return m_metaFile; }
    void SetMetaFile(wxMetafile *mf) { m_metaFile = mf; }
    int GetWindowsMappingMode() const { return m_windowsMappingMode; }
    void SetWindowsMappingMode(int mm) { m_windowsMappingMode = mm; }

protected:
    wxSize DoGetSize() const override;

    int           m_windowsMappingMode;
    wxMetafile*   m_metaFile;

private:
    wxDECLARE_CLASS(wxMetafileDCImpl);
};

class wxMetafileDC: public wxDC
{
public:
    // Don't supply origin and extent
    // Supply them to wxMakeMetaFilePlaceable instead.
    wxMetafileDC(const std::string& file)
        : wxDC(std::make_unique<wxMetafileDCImpl>( this, file ))
        { }

    // Supply origin and extent (recommended).
    // Then don't need to supply them to wxMakeMetaFilePlaceable.
    wxMetafileDC(const std::string& file, int xext, int yext, int xorg, int yorg)
        : wxDC(std::make_unique<wxMetafileDCImpl>( this, file, xext, yext, xorg, yorg ))
        { }

   wxMetafileDC& operator=(wxMetafileDC&&) = delete;

    wxMetafile *GetMetafile() const
       { return ((wxMetafileDCImpl*)m_pimpl.get())->GetMetaFile(); }

    wxMetafile *Close()
       { return ((wxMetafileDCImpl*)m_pimpl.get())->Close(); }

private:
    wxDECLARE_CLASS(wxMetafileDC);
};




/*
 * Pass filename of existing non-placeable metafile, and bounding box.
 * Adds a placeable metafile header, sets the mapping mode to anisotropic,
 * and sets the window origin and extent to mimic the wxMappingMode::Text mapping mode.
 *
 */

// No origin or extent
bool wxMakeMetafilePlaceable(const wxString& filename, float scale = 1.0f);

// Optional origin and extent
bool wxMakeMetaFilePlaceable(const wxString& filename, int x1, int y1, int x2, int y2, float scale = 1.0f, bool useOriginAndExtent = true);

// ----------------------------------------------------------------------------
// wxMetafileDataObject is a specialization of wxDataObject for metafile data
// ----------------------------------------------------------------------------

#if wxUSE_DATAOBJ

class wxMetafileDataObject : public wxDataObjectSimple
{
public:
    // ctors
    wxMetafileDataObject() : wxDataObjectSimple(wxDF_METAFILE)
        { }
    wxMetafileDataObject(const wxMetafile& metafile)
        : wxDataObjectSimple(wxDF_METAFILE), m_metafile(metafile) { }

    // functions which you may override if you want to provide data on
    // demand only - otherwise, the trivial default versions will be used
    virtual void SetMetafile(const wxMetafile& metafile)
        { m_metafile = metafile; }
    virtual wxMetafile GetMetafile() const
        { return m_metafile; }

    
    size_t GetDataSize() const override;
    bool GetDataHere(void *buf) const override;
    bool SetData(size_t len, const void *buf) override;

protected:
    wxMetafile m_metafile;
};

#endif // wxUSE_DATAOBJ

#endif
    // _WX_METAFIILE_H_

