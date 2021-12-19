/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/dcpsg.h
// Purpose:     wxPostScriptDC class
// Author:      Julian Smart and others
// Modified by:
// Copyright:   (c) Julian Smart and Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCPSG_H_
#define _WX_DCPSG_H_

#if wxUSE_PRINTING_ARCHITECTURE && wxUSE_POSTSCRIPT

#include "wx/dc.h"
#include "wx/dcprint.h"
#include "wx/dialog.h"
#include "wx/module.h"
#include "wx/strvararg.h"

import WX.Core.Cmndata;

//-----------------------------------------------------------------------------
// wxPostScriptDC
//-----------------------------------------------------------------------------


class wxPostScriptDC : public wxDC
{
public:
    wxPostScriptDC();

    // Recommended constructor
    wxPostScriptDC(const wxPrintData& printData);
};

class wxPostScriptDCImpl : public wxDCImpl
{
public:
    wxPostScriptDCImpl( wxPrinterDC *owner );
    wxPostScriptDCImpl( wxPrinterDC *owner, const wxPrintData& data );
    wxPostScriptDCImpl( wxPostScriptDC *owner );
    wxPostScriptDCImpl( wxPostScriptDC *owner, const wxPrintData& data );

    void Init();

    ~wxPostScriptDCImpl();

    bool IsOk() const override;

    bool CanDrawBitmap() const override { return true; }

    void Clear() override;
    void SetFont( const wxFont& font ) override;
    void SetPen( const wxPen& pen ) override;
    void SetBrush( const wxBrush& brush ) override;
    void SetLogicalFunction( wxRasterOperationMode function ) override;
    void SetBackground( const wxBrush& brush ) override;

    void DestroyClippingRegion() override;

    bool wxStartDoc(const std::string& message) override;
    void EndDoc() override;
    void StartPage() override;
    void EndPage() override;

    wxCoord GetCharHeight() const override;
    wxCoord wxGetCharWidth() const override;
    bool CanGetTextExtent() const override { return true; }

    // Resolution in pixels per logical inch
    wxSize GetPPI() const override;

    void ComputeScaleAndOrigin() override;

    void SetBackgroundMode([[maybe_unused]] wxBrushStyle mode) override { }
#if wxUSE_PALETTE
    void SetPalette([[maybe_unused]] const wxPalette& palette) override { }
#endif

    void SetPrintData(const wxPrintData& data);
    wxPrintData& GetPrintData() { return m_printData; }

    int GetDepth() const override { return 24; }

    void PsPrint( const std::string& psdata );

    // Overridden for wxPrinterDC Impl

    int GetResolution() const override;
    wxRect GetPaperRect() const override;

    void* GetHandle() const override { return nullptr; }

protected:
    bool DoFloodFill(wxCoord x1, wxCoord y1, const wxColour &col,
                     wxFloodFillStyle style = wxFloodFillStyle::Surface) override;
    bool DoGetPixel(wxCoord x1, wxCoord y1, wxColour *col) const override;
    void DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2) override;
    void DoCrossHair(wxCoord x, wxCoord y) override ;
    void DoDrawArc(wxCoord x1,wxCoord y1,wxCoord x2,wxCoord y2,wxCoord xc,wxCoord yc) override;
    void DoDrawEllipticArc(wxCoord x,wxCoord y,wxCoord w,wxCoord h,double sa,double ea) override;
    void DoDrawPoint(wxCoord x, wxCoord y) override;
    void DoDrawLines(int n, const wxPoint points[], wxCoord xoffset = 0, wxCoord yoffset = 0) override;
    void DoDrawPolygon(int n, const wxPoint points[],
                       wxCoord xoffset = 0, wxCoord yoffset = 0,
                       wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven) override;
    void DoDrawPolyPolygon(int n, const int count[], const wxPoint points[],
                           wxCoord xoffset = 0, wxCoord yoffset = 0,
                           wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven) override;
    void DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;
    void DoDrawRoundedRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height, double radius = 20) override;
    void DoDrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;
#if wxUSE_SPLINES
    void DoDrawSpline(const wxPointList *points) override;
#endif
    bool DoBlit(wxCoord xdest, wxCoord ydest, wxCoord width, wxCoord height,
                wxDC *source, wxCoord xsrc, wxCoord ysrc,
                wxRasterOperationMode rop = wxRasterOperationMode::Copy, bool useMask = false,
                wxCoord xsrcMask = wxDefaultCoord, wxCoord ysrcMask = wxDefaultCoord) override;
    void DoDrawIcon(const wxIcon& icon, wxCoord x, wxCoord y) override;
    void DoDrawBitmap(const wxBitmap& bitmap, wxCoord x, wxCoord y, bool useMask = false) override;
    void DoDrawText(const std::string& text, const wxPoint& pt) override;
    void DoDrawRotatedText(const std::string& text, const wxPoint& pt, double angle) override;
    void DoSetClippingRegion(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;
    void DoSetDeviceClippingRegion( [[maybe_unused]] const wxRegion &clip) override
    {
        wxFAIL_MSG( "not implemented" );
    }
    void DoGetTextExtent(const std::string& string, wxCoord *x, wxCoord *y,
                         wxCoord *descent = nullptr,
                         wxCoord *externalLeading = nullptr,
                         const wxFont *theFont = nullptr) const override;
    wxSize DoGetSize() const override;
    void DoGetSizeMM(int *width, int *height) const override;

    // Common part of DoDrawText() and DoDrawRotatedText()
    void DrawAnyText(const wxWX2MBbuf& textbuf, wxCoord testDescent, double lineHeight);
    // Actually set PostScript font
    void SetPSFont();
    // Set PostScript color
    void SetPSColour(const wxColour& col);

    std::vector<std::string>     m_definedPSFonts;

    wxPrintData       m_printData;

    FILE*             m_pstream;    // PostScript output stream

    double            m_pageHeight{842 * PS2DEV};
    mutable double    m_underlinePosition;
    mutable double    m_underlineThickness;

    unsigned char     m_currentRed;
    unsigned char     m_currentGreen;
    unsigned char     m_currentBlue;
    int               m_pageNumber;

    bool              m_isFontChanged;
    bool              m_clipping;
};

#endif
    // wxUSE_POSTSCRIPT && wxUSE_PRINTING_ARCHITECTURE

#endif
        // _WX_DCPSG_H_
