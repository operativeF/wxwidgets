/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dcgraph.h
// Purpose:     graphics context device bridge header
// Author:      Stefan Csomor
// Modified by:
// Created:
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GRAPHICS_DC_H_
#define _WX_GRAPHICS_DC_H_

#if wxUSE_GRAPHICS_CONTEXT

#include "wx/dc.h"
#include "wx/geometry.h"
#include "wx/geometry/rect.h"
#include "wx/graphics.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class WXDLLIMPEXP_FWD_CORE wxWindowDC;

class WXDLLIMPEXP_CORE wxGCDC: public wxDC
{
public:
    wxGCDC( const wxWindowDC& dc );
    wxGCDC( const wxMemoryDC& dc );
#if wxUSE_PRINTING_ARCHITECTURE
    wxGCDC( const wxPrinterDC& dc );
#endif
#if defined(__WXMSW__) && wxUSE_ENH_METAFILE
    wxGCDC( const wxEnhMetaFileDC& dc );
#endif
    wxGCDC(std::unique_ptr<wxGraphicsContext> context);

    wxGCDC();

    wxGCDC& operator=(wxGCDC&&) = delete;

#ifdef __WXMSW__
    // override wxDC virtual functions to provide access to HDC associated with
    // this Graphics object (implemented in src/msw/graphics.cpp)
    WXHDC AcquireHDC() override;
    void ReleaseHDC(WXHDC hdc) override;
#endif // __WXMSW__

private:
    wxDECLARE_DYNAMIC_CLASS(wxGCDC);
};


class WXDLLIMPEXP_CORE wxGCDCImpl: public wxDCImpl
{
public:
    wxGCDCImpl( wxDC *owner, const wxWindowDC& dc );
    wxGCDCImpl( wxDC *owner, const wxMemoryDC& dc );
#if wxUSE_PRINTING_ARCHITECTURE
    wxGCDCImpl( wxDC *owner, const wxPrinterDC& dc );
#endif
#if defined(__WXMSW__) && wxUSE_ENH_METAFILE
    wxGCDCImpl( wxDC *owner, const wxEnhMetaFileDC& dc );
#endif

    // Ctor using an existing graphics context given to wxGCDC ctor.
    wxGCDCImpl(wxDC *owner, std::unique_ptr<wxGraphicsContext> context);

    wxGCDCImpl( wxDC *owner );

    wxGCDCImpl& operator=(wxGCDCImpl&&) = delete;
    
    void Clear() override;

    bool wxStartDoc( const std::string& message ) override;
    void EndDoc() override;

    void StartPage() override;
    void EndPage() override;

    // flushing the content of this dc immediately onto screen
    void Flush() override;

    void SetFont(const wxFont& font) override;
    void SetPen(const wxPen& pen) override;
    void SetBrush(const wxBrush& brush) override;
    void SetBackground(const wxBrush& brush) override;
    void SetBackgroundMode(wxBrushStyle mode) override;

#if wxUSE_PALETTE
    void SetPalette(const wxPalette& palette) override;
#endif

    void DestroyClippingRegion() override;

    wxCoord GetCharHeight() const override;
    wxCoord wxGetCharWidth() const override;

    bool CanDrawBitmap() const override;
    bool CanGetTextExtent() const override;
    int GetDepth() const override;
    wxSize GetPPI() const override;

    void SetLogicalFunction(wxRasterOperationMode function) override;

    void SetTextForeground(const wxColour& colour) override;
    void SetTextBackground(const wxColour& colour) override;

    void ComputeScaleAndOrigin() override;

    wxGraphicsContext* GetGraphicsContext() const override { return m_graphicContext.get(); }
    void SetGraphicsContext( std::unique_ptr<wxGraphicsContext> ctx ) override;

    void* GetHandle() const override;

#if wxUSE_DC_TRANSFORM_MATRIX
    bool CanUseTransformMatrix() const override;
    bool SetTransformMatrix(const wxAffineMatrix2D& matrix) override;
    wxAffineMatrix2D GetTransformMatrix() const override;
    void ResetTransformMatrix() override;
#endif // wxUSE_DC_TRANSFORM_MATRIX

    // coordinates conversions and transforms
    wxPoint DeviceToLogical(wxCoord x, wxCoord y) const override;
    wxPoint LogicalToDevice(wxCoord x, wxCoord y) const override;
    wxSize DeviceToLogicalRel(int x, int y) const override;
    wxSize LogicalToDeviceRel(int x, int y) const override;

    // the true implementations
    bool DoFloodFill(wxCoord x, wxCoord y, const wxColour& col,
                             wxFloodFillStyle style = wxFloodFillStyle::Surface) override;

    void DoGradientFillLinear(const wxRect& rect,
        const wxColour& initialColour,
        const wxColour& destColour,
        wxDirection nDirection = wxEAST) override;

    void DoGradientFillConcentric(const wxRect& rect,
        const wxColour& initialColour,
        const wxColour& destColour,
        const wxPoint& circleCenter) override;

    bool DoGetPixel(wxCoord x, wxCoord y, wxColour *col) const override;

    void DoDrawPoint(wxCoord x, wxCoord y) override;

#if wxUSE_SPLINES
    void DoDrawSpline(const wxPointList *points) override;
#endif

    void DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2) override;

    void DoDrawArc(wxCoord x1, wxCoord y1,
        wxCoord x2, wxCoord y2,
        wxCoord xc, wxCoord yc) override;

    void DoDrawCheckMark(wxCoord x, wxCoord y,
        wxCoord width, wxCoord height) override;

    void DoDrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
        double sa, double ea) override;

    void DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;
    void DoDrawRoundedRectangle(wxCoord x, wxCoord y,
        wxCoord width, wxCoord height,
        double radius) override;
    void DoDrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;

    void DoCrossHair(wxCoord x, wxCoord y) override;

    void DoDrawIcon(const wxIcon& icon, wxCoord x, wxCoord y) override;
    void DoDrawBitmap(const wxBitmap &bmp, wxCoord x, wxCoord y,
        bool useMask = false) override;

    void DoDrawText(std::string_view text, wxPoint pt) override;
    void DoDrawRotatedText(std::string_view text, wxPoint pt, double angle) override;

    bool DoBlit(wxCoord xdest, wxCoord ydest, wxCoord width, wxCoord height,
                        wxDC *source, 
                        wxPoint src,
                        wxRasterOperationMode rop = wxRasterOperationMode::Copy, bool useMask = false,
                        wxPoint srcMask = wxDefaultPosition) override;

    bool DoStretchBlit(wxCoord xdest, wxCoord ydest,
                               wxCoord dstWidth, wxCoord dstHeight,
                               wxDC *source,
                               wxPoint src,
                               wxCoord srcWidth, wxCoord srcHeight,
                               wxRasterOperationMode = wxRasterOperationMode::Copy, bool useMask = false,
                               wxPoint srcMask = wxDefaultPosition) override;

    wxSize DoGetSize() const override;
    void DoGetSizeMM(int* width, int* height) const override;

    void DoDrawLines(int n, const wxPoint points[],
        wxCoord xoffset, wxCoord yoffset) override;
    void DoDrawPolygon(int n, const wxPoint points[],
                               wxCoord xoffset, wxCoord yoffset,
                               wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven) override;
    void DoDrawPolyPolygon(int n, const int count[], const wxPoint points[],
                                   wxCoord xoffset, wxCoord yoffset,
                                   wxPolygonFillMode fillStyle) override;

    void DoSetDeviceClippingRegion(const wxRegion& region) override;
    void DoSetClippingRegion(wxCoord x, wxCoord y,
        wxCoord width, wxCoord height) override;
    bool DoGetClippingRect(wxRect& rect) const override;

    wxSize DoGetTextExtent(std::string_view string,
        wxCoord *descent = nullptr,
        wxCoord *externalLeading = nullptr,
        const wxFont *theFont = nullptr) const override;

    std::vector<int> DoGetPartialTextExtents(std::string_view text) const override;

#ifdef __WXMSW__
    wxRect MSWApplyGDIPlusTransform(const wxRect& r) const override;
#endif // __WXMSW__

    // update the internal clip box variables
    void UpdateClipBox();

protected:
    // unused int parameter distinguishes this version, which does not create a
    // wxGraphicsContext, in the expectation that the derived class will do it
    wxGCDCImpl(wxDC* owner, int);

#ifdef __WXOSX__
    virtual wxPoint OSXGetOrigin() const { return wxPoint(); }
#endif
private:
    // scaling variables
    wxGraphicsMatrix m_matrixOriginal;
    wxGraphicsMatrix m_matrixCurrent;
    wxGraphicsMatrix m_matrixCurrentInv;
#if wxUSE_DC_TRANSFORM_MATRIX
    wxAffineMatrix2D m_matrixExtTransform;
#endif // wxUSE_DC_TRANSFORM_MATRIX

    std::unique_ptr<wxGraphicsContext> m_graphicContext;

    bool m_isClipBoxValid;

    bool m_logicalFunctionSupported;

    // This method only initializes trivial fields.
    void CommonInit();

    // This method initializes all fields (including those initialized by
    // CommonInit() as it calls it) and the given context, if non-null, which
    // is assumed to be newly created.
    void Init(std::unique_ptr<wxGraphicsContext> ctx);

    // This method initializes m_graphicContext, m_ok and m_matrixOriginal
    // fields, returns true if the context was valid.
    bool DoInitContext(std::unique_ptr<wxGraphicsContext> ctx);

    wxDECLARE_CLASS(wxGCDCImpl);
};

#endif // wxUSE_GRAPHICS_CONTEXT
#endif // _WX_GRAPHICS_DC_H_
