/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/metafile.cpp
// Purpose:     wxMetafileDC etc.
// Author:      Julian Smart
// Modified by: VZ 07.01.00: implemented wxMetaFileDataObject
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"

#include "wx/utils.h"

#include "wx/metafile.h"

import WX.File.Filename;
import WX.WinDef;

import <cstdio>;
import <cstring>;

#if wxUSE_METAFILE && !defined(wxMETAFILE_IS_ENH)

#include "wx/clipbrd.h"


// ----------------------------------------------------------------------------
// wxWin macros
// ----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxMetafileDC, wxDC);

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxMetafileRefData
// ----------------------------------------------------------------------------

/*
 * Metafiles
 * Currently, the only purpose for making a metafile is to put
 * it on the clipboard.
 */

wxMetafileRefData::~wxMetafileRefData()
{
    if (m_metafile)
    {
        DeleteMetaFile((HMETAFILE) m_metafile);
        m_metafile = 0;
    }
}

// ----------------------------------------------------------------------------
// wxMetafile
// ----------------------------------------------------------------------------

wxMetafile::wxMetafile(const std::string& file)
{
    m_refData = new wxMetafileRefData;

    M_METAFILEDATA->m_windowsMappingMode = MM_ANISOTROPIC;
    M_METAFILEDATA->m_metafile = 0;
    if (!file.empty())
        M_METAFILEDATA->m_metafile = (WXHANDLE) GetMetaFile(file);
}

wxGDIRefData *wxMetafile::CreateGDIRefData() const
{
    return new wxMetafileRefData;
}

wxGDIRefData *wxMetafile::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxMetafileRefData(*static_cast<const wxMetafileRefData *>(data));
}

bool wxMetafile::SetClipboard(int width, int height)
{
#if !wxUSE_CLIPBOARD
    return false;
#else
    if (!m_refData)
        return false;

    bool alreadyOpen = wxClipboardOpen();
    if (!alreadyOpen)
    {
        wxOpenClipboard();
        if (!wxEmptyClipboard())
            return false;
    }
    bool success = wxSetClipboardData(wxDF_METAFILE, this, width,height);
    if (!alreadyOpen)
        wxCloseClipboard();

    return success;
#endif
}

bool wxMetafile::Play(wxDC *dc)
{
    if (!m_refData)
        return false;

    if (dc->GetHDC() && M_METAFILEDATA->m_metafile)
    {
        if ( !::PlayMetaFile(GetHdcOf(*dc), (HMETAFILE)
                             M_METAFILEDATA->m_metafile) )
        {
            wxLogLastError("PlayMetaFile");
        }
    }

    return true;
}

void wxMetafile::SetHMETAFILE(WXHANDLE mf)
{
    if (!m_refData)
        m_refData = new wxMetafileRefData;

    M_METAFILEDATA->m_metafile = mf;
}

void wxMetafile::SetWindowsMappingMode(int mm)
{
    if (!m_refData)
        m_refData = new wxMetafileRefData;

    M_METAFILEDATA->m_windowsMappingMode = mm;
}

// ----------------------------------------------------------------------------
// Metafile device context
// ----------------------------------------------------------------------------

// Original constructor that does not takes origin and extent. If you use this,
// *DO* give origin/extent arguments to wxMakeMetafilePlaceable.
wxMetafileDCImpl::wxMetafileDCImpl(wxDC *owner, const std::string& file)
    : wxMSWDCImpl(owner)
{
    m_metaFile = NULL;
    m_minX = 10000;
    m_minY = 10000;
    m_maxX = -10000;
    m_maxY = -10000;
    //  m_title = NULL;

    if ( wxFileExists(file) )
        wxRemoveFile(file);

    if ( file.empty() )
        m_hDC = (WXHDC) CreateMetaFile(NULL);
    else
        m_hDC = (WXHDC) CreateMetaFile(file);

    m_ok = (m_hDC != (WXHDC) 0) ;

    // Actual Windows mapping mode, for future reference.
    m_windowsMappingMode = wxMappingMode::Text;

    SetMapMode(wxMappingMode::Text); // NOTE: does not set WXHDC mapmode (this is correct)
}

// New constructor that takes origin and extent. If you use this, don't
// give origin/extent arguments to wxMakeMetafilePlaceable.
wxMetafileDCImpl::wxMetafileDCImpl(wxDC *owner, const std::string& file,
                                   int xext, int yext, int xorg, int yorg)
    : wxMSWDCImpl(owner)
{
    m_minX = 10000;
    m_minY = 10000;
    m_maxX = -10000;
    m_maxY = -10000;
    if ( !file.empty() && wxFileExists(file) )
        wxRemoveFile(file);
    m_hDC = (WXHDC) CreateMetaFile(file.empty() ? NULL : wxMSW_CONV_LPCTSTR(file));

    m_ok = true;

    ::SetWindowOrgEx((WXHDC) m_hDC,xorg,yorg, NULL);
    ::SetWindowExtEx((WXHDC) m_hDC,xext,yext, NULL);

    // Actual Windows mapping mode, for future reference.
    m_windowsMappingMode = MM_ANISOTROPIC;

    SetMapMode(wxMappingMode::Text); // NOTE: does not set WXHDC mapmode (this is correct)
}

wxMetafileDCImpl::~wxMetafileDCImpl()
{
    m_hDC = 0;
}

void wxMetafileDCImpl::DoGetTextExtent(const wxString& string,
                                       wxCoord *x, wxCoord *y,
                                       wxCoord *descent, wxCoord *externalLeading,
                                       const wxFont *theFont) const
{
    const wxFont *fontToUse = theFont;
    if (!fontToUse)
        fontToUse = &m_font;

    ScreenHDC dc;
    SelectInHDC selFont(dc, GetHfontOf(*fontToUse));

    SIZE sizeRect;
    TEXTMETRICW tm;
    ::GetTextExtentPoint32(dc, string.c_str(), string.length(), &sizeRect);
    ::GetTextMetrics(dc, &tm);

    if ( x )
        *x = sizeRect.cx;
    if ( y )
        *y = sizeRect.cy;
    if ( descent )
        *descent = tm.tmDescent;
    if ( externalLeading )
        *externalLeading = tm.tmExternalLeading;
}

void wxMetafileDCImpl::DoGetSize(int *width, int *height) const
{
    wxCHECK_RET( m_refData, "invalid wxMetafileDC" );

    if ( width )
        *width = M_METAFILEDATA->m_width;
    if ( height )
        *height = M_METAFILEDATA->m_height;
}

wxMetafile *wxMetafileDCImpl::Close()
{
    SelectOldObjects(m_hDC);
    HANDLE mf = CloseMetaFile((WXHDC) m_hDC);
    m_hDC = 0;
    if (mf)
    {
        wxMetafile *wx_mf = new wxMetafile;
        wx_mf->SetHMETAFILE((WXHANDLE) mf);
        wx_mf->SetWindowsMappingMode(m_windowsMappingMode);
        return wx_mf;
    }
    return NULL;
}

void wxMetafileDCImpl::SetMapMode(wxMappingMode mode)
{
    m_mappingMode = mode;

    //  int pixel_width = 0;
    //  int pixel_height = 0;
    //  int mm_width = 0;
    //  int mm_height = 0;

    const double mm2pixelsX = 10;
    const double mm2pixelsY = 10;

    switch (mode)
    {
        case wxMappingMode::Twips:
            {
                m_logicalScale = {twips2mm * mm2pixelsX, twips2mm * mm2pixelsY};
                break;
            }
        case wxMappingMode::Points:
            {
                m_logicalScale = {pt2mm * mm2pixelsX, pt2mm * mm2pixelsY};
                break;
            }
        case wxMappingMode::Metric:
            {
                m_logicalScale = {mm2pixelsX, mm2pixelsY};
                break;
            }
        case wxMappingMode::LoMetric:
            {
                m_logicalScale = {mm2pixelsX / 10, mm2pixelsY / 10};
                break;
            }
        default:
        case wxMappingMode::Text:
            {
                m_logicalScale = {1.0, 1.0};
                break;
            }
    }
}

// ----------------------------------------------------------------------------
// wxMakeMetafilePlaceable
// ----------------------------------------------------------------------------

struct RECT32
{
  short left;
  short top;
  short right;
  short bottom;
};

struct mfPLACEABLEHEADER {
    WXDWORD    key;
    short    hmf;
    RECT32    bbox;
    WXWORD    inch;
    WXDWORD    reserved;
    WXWORD    checksum;
};

/*
 * Pass filename of existing non-placeable metafile, and bounding box.
 * Adds a placeable metafile header, sets the mapping mode to anisotropic,
 * and sets the window origin and extent to mimic the wxMappingMode::Text mapping mode.
 *
 */

bool wxMakeMetafilePlaceable(const wxString& filename, float scale)
{
    return wxMakeMetafilePlaceable(filename, 0, 0, 0, 0, scale, false);
}

bool wxMakeMetafilePlaceable(const wxString& filename, int x1, int y1, int x2, int y2, float scale, bool useOriginAndExtent)
{
    // I'm not sure if this is the correct way of suggesting a scale
    // to the client application, but it's the only way I can find.
    int unitsPerInch = (int)(576/scale);

    mfPLACEABLEHEADER header;
    header.key = 0x9AC6CDD7L;
    header.hmf = 0;
    header.bbox.left = (int)(x1);
    header.bbox.top = (int)(y1);
    header.bbox.right = (int)(x2);
    header.bbox.bottom = (int)(y2);
    header.inch = unitsPerInch;
    header.reserved = 0;

    // Calculate checksum
    WXWORD *p;
    mfPLACEABLEHEADER *pMFHead = &header;
    for (p =(WXWORD *)pMFHead,pMFHead -> checksum = 0;
            p < (WXWORD *)&pMFHead ->checksum; ++p)
        pMFHead ->checksum ^= *p;

    FILE *fd = wxFopen(filename.fn_str(), "rb");
    if (!fd) return false;

    wxString tempFileBuf = wxFileName::CreateTempFileName("mf");
    if (tempFileBuf.empty())
        return false;

    FILE *fHandle = wxFopen(tempFileBuf.fn_str(), "wb");
    if (!fHandle)
        return false;
    fwrite((void *)&header, 1, sizeof(mfPLACEABLEHEADER), fHandle);

    // Calculate origin and extent
    int originX = x1;
    int originY = y1;
    int extentX = x2 - x1;
    int extentY = (y2 - y1);

    // Read metafile header and write
    METAHEADER metaHeader;
    fread((void *)&metaHeader, 1, sizeof(metaHeader), fd);

    if (useOriginAndExtent)
        metaHeader.mtSize += 15;
    else
        metaHeader.mtSize += 5;

    fwrite((void *)&metaHeader, 1, sizeof(metaHeader), fHandle);

    // Write SetMapMode, SetWindowOrigin and SetWindowExt records
    char modeBuffer[8];
    char originBuffer[10];
    char extentBuffer[10];
    METARECORD *modeRecord = (METARECORD *)&modeBuffer;

    METARECORD *originRecord = (METARECORD *)&originBuffer;
    METARECORD *extentRecord = (METARECORD *)&extentBuffer;

    modeRecord->rdSize = 4;
    modeRecord->rdFunction = META_SETMAPMODE;
    modeRecord->rdParm[0] = MM_ANISOTROPIC;

    originRecord->rdSize = 5;
    originRecord->rdFunction = META_SETWINDOWORG;
    originRecord->rdParm[0] = originY;
    originRecord->rdParm[1] = originX;

    extentRecord->rdSize = 5;
    extentRecord->rdFunction = META_SETWINDOWEXT;
    extentRecord->rdParm[0] = extentY;
    extentRecord->rdParm[1] = extentX;

    fwrite((void *)modeBuffer, 1, 8, fHandle);

    if (useOriginAndExtent)
    {
        fwrite((void *)originBuffer, 1, 10, fHandle);
        fwrite((void *)extentBuffer, 1, 10, fHandle);
    }

    int ch = -2;
    while (ch != EOF)
    {
        ch = getc(fd);
        if (ch != EOF)
        {
            putc(ch, fHandle);
        }
    }
    fclose(fHandle);
    fclose(fd);
    wxRemoveFile(filename);
    wxCopyFile(tempFileBuf, filename);
    wxRemoveFile(tempFileBuf);
    return true;
}


#if wxUSE_DATAOBJ

// ----------------------------------------------------------------------------
// wxMetafileDataObject
// ----------------------------------------------------------------------------

size_t wxMetafileDataObject::GetDataSize() const
{
    return sizeof(METAFILEPICT);
}

bool wxMetafileDataObject::GetDataHere(void *buf) const
{
    METAFILEPICT *mfpict = (METAFILEPICT *)buf;
    const wxMetafile& mf = GetMetafile();

    wxCHECK_MSG( mf.GetHMETAFILE(), false, "copying invalid metafile" );

    // doesn't seem to work with any other mapping mode...
    mfpict->mm   = MM_ANISOTROPIC; //mf.GetWindowsMappingMode();
    mfpict->xExt = mf.GetWidth();
    mfpict->yExt = mf.GetHeight();

    // transform the picture size to HIMETRIC units (0.01mm) - as we don't know
    // what DC the picture will be rendered to, use the default display one
    PixelToHIMETRIC(&mfpict->xExt, &mfpict->yExt);

    mfpict->hMF  = CopyMetaFile((HMETAFILE)mf.GetHMETAFILE(), NULL);

    return true;
}

bool wxMetafileDataObject::SetData([[maybe_unused]] size_t len, const void *buf)
{
    const METAFILEPICT *mfpict = (const METAFILEPICT *)buf;

    wxMetafile mf;
    mf.SetWindowsMappingMode(mfpict->mm);

    LONG w = mfpict->xExt,
         h = mfpict->yExt;
    if ( mfpict->mm == MM_ANISOTROPIC )
    {
        // in this case xExt and yExt contain suggested size in HIMETRIC units
        // (0.01 mm) - transform this to something more reasonable (pixels)
        HIMETRICToPixel(&w, &h);
    }

    mf.SetWidth(w);
    mf.SetHeight(h);
    mf.SetHMETAFILE((WXHANDLE)mfpict->hMF);

    wxCHECK_MSG( mfpict->hMF, false, "pasting invalid metafile" );

    SetMetafile(mf);

    return true;
}

#endif // wxUSE_DATAOBJ

#endif // wxUSE_METAFILE
