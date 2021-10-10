/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/dc.h
// Purpose:     wxDC class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_DC_H_
#define _WX_MSW_DC_H_

#include "wx/defs.h"
#include "wx/dc.h"
#include "wx/geometry/rect.h"

#include "wx/msw/wrap/utils.h"

#include <string>
#include <string_view>

#if wxUSE_DC_CACHEING
/*
 * Cached blitting, maintaining a cache
 * of bitmaps required for transparent blitting
 * instead of constant creation/deletion
 */

using msw::utils::unique_bitmap;
using msw::utils::unique_dc;

struct wxDCCacheEntry: public wxObject
{
    wxDCCacheEntry(WXHBITMAP hBitmap, int w, int h, int depth);
    wxDCCacheEntry(WXHDC hDC, int depth);

    unique_bitmap   m_bitmap;
    unique_dc       m_dc;
    int             m_width{0};
    int             m_height{0};
    int             m_depth{0};
};
#endif

// this is an ABC: use one of the derived classes to create a DC associated
// with a window, screen, printer and so on
class WXDLLIMPEXP_CORE wxMSWDCImpl: public wxDCImpl
{
public:
    wxMSWDCImpl(wxDC *owner, WXHDC hDC);
    ~wxMSWDCImpl();

    wxMSWDCImpl(const wxMSWDCImpl&) = delete;
    wxMSWDCImpl& operator=(const wxMSWDCImpl&) = delete;
    wxMSWDCImpl(wxMSWDCImpl&&) = default;
    wxMSWDCImpl& operator=(wxMSWDCImpl&&) = default;

    void Clear() override;

    bool wxStartDoc(const std::string& message) override;
    void EndDoc() override;

    void StartPage() override;
    void EndPage() override;

    void SetFont(const wxFont& font) override;
    void SetPen(const wxPen& pen) override;
    void SetBrush(const wxBrush& brush) override;
    void SetBackground(const wxBrush& brush) override;
    void SetBackgroundMode(wxBrushStyle mode) override;
#if wxUSE_PALETTE
    void SetPalette(const wxPalette& palette) override;
#endif // wxUSE_PALETTE

    void DestroyClippingRegion() override;

    wxCoord GetCharHeight() const override;
    wxCoord wxGetCharWidth() const override;

    bool CanDrawBitmap() const override;
    bool CanGetTextExtent() const override;
    int GetDepth() const override;
    wxSize GetPPI() const override;


    void SetMapMode(wxMappingMode mode) override;
    void SetUserScale(wxScale userScale) override;
    void SetLogicalScale(wxScale logicalScale) override;
    void SetLogicalOrigin(wxPoint logicalOrigin) override;
    void SetDeviceOrigin(wxPoint deviceOrigin) override;
    void SetAxisOrientation(bool xLeftRight, bool yBottomUp) override;

    wxPoint DeviceToLogical(wxCoord x, wxCoord y) const override;
    wxPoint LogicalToDevice(wxCoord x, wxCoord y) const override;
    wxSize DeviceToLogicalRel(int x, int y) const override;
    wxSize LogicalToDeviceRel(int x, int y) const override;

#if wxUSE_DC_TRANSFORM_MATRIX
    bool CanUseTransformMatrix() const override;
    bool SetTransformMatrix(const wxAffineMatrix2D& matrix) override;
    wxAffineMatrix2D GetTransformMatrix() const override;
    void ResetTransformMatrix() override;
#endif // wxUSE_DC_TRANSFORM_MATRIX

    void SetLogicalFunction(wxRasterOperationMode function) override;

    virtual void SetRop(WXHDC cdc);
    virtual void SelectOldObjects(WXHDC dc);

    void SetWindow(wxWindow *win)
    {
        m_window = win;

#if wxUSE_PALETTE
        // if we have palettes use the correct one for this window
        InitializePalette();
#endif // wxUSE_PALETTE
    }

    WXHDC GetHDC() const { return m_hDC; }
    void SetHDC(WXHDC dc, bool bOwnsDC = false)
    {
        m_hDC = dc;
        m_bOwnsDC = bOwnsDC;

        // we might have a pre existing clipping region, make sure that we
        // return it if asked -- but avoid calling ::GetClipBox() right now as
        // it could be unnecessary wasteful
        m_clipping = true;
        m_isClipBoxValid = false;
    }

    void* GetHandle() const override { return (void*)GetHDC(); }

    const wxBitmap& GetSelectedBitmap() const override { return m_selectedBitmap; }
    wxBitmap& GetSelectedBitmap() override { return m_selectedBitmap; }

    // update the internal clip box variables
    void UpdateClipBox();

#if wxUSE_DC_CACHEING
    static wxDCCacheEntry* FindBitmapInCache(WXHDC hDC, int w, int h);
    static wxDCCacheEntry* FindDCInCache(wxDCCacheEntry* notThis, WXHDC hDC);

    static void AddToBitmapCache(wxDCCacheEntry* entry);
    static void AddToDCCache(wxDCCacheEntry* entry);
    static void ClearCache();
#endif

    // RTL related functions
    // ---------------------

    // get or change the layout direction (LTR or RTL) for this dc,
    // wxLayoutDirection::Default is returned if layout direction is not supported
    wxLayoutDirection GetLayoutDirection() const override;
    void SetLayoutDirection(wxLayoutDirection dir) override;

protected:

    // create an uninitialized DC: this should be only used by the derived
    // classes
    wxMSWDCImpl( wxDC *owner ) : wxDCImpl( owner ) {}

    void RealizeScaleAndOrigin();

public:
    void DoGetFontMetrics(int *height,
                                  int *ascent,
                                  int *descent,
                                  int *internalLeading,
                                  int *externalLeading,
                                  int *averageWidth) const override;
    wxSize DoGetTextExtent(std::string_view string,
                                 wxCoord *descent = nullptr,
                                 wxCoord *externalLeading = nullptr,
                                 const wxFont *theFont = nullptr) const override;
    std::vector<int> DoGetPartialTextExtents(std::string_view text) const override;

    bool DoFloodFill(wxCoord x, wxCoord y, const wxColour& col,
                             wxFloodFillStyle style = wxFloodFillStyle::Surface) override;

    void DoGradientFillLinear(const wxRect& rect,
                                      const wxColour& initialColour,
                                      const wxColour& destColour,
                                      wxDirection nDirection = wxEAST) override;

    bool DoGetPixel(wxCoord x, wxCoord y, wxColour *col) const override;

    void DoDrawPoint(wxCoord x, wxCoord y) override;
    void DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2) override;

    void DoDrawArc(wxCoord x1, wxCoord y1,
                           wxCoord x2, wxCoord y2,
                           wxCoord xc, wxCoord yc) override;
    void DoDrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
                                   double sa, double ea) override;

    void DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;
    void DoDrawRoundedRectangle(wxCoord x, wxCoord y,
                                        wxCoord width, wxCoord height,
                                        double radius) override;
    void DoDrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;

#if wxUSE_SPLINES
    void DoDrawSpline(const wxPointList *points) override;
#endif

    void DoCrossHair(wxCoord x, wxCoord y) override;

    void DoDrawIcon(const wxIcon& icon, wxCoord x, wxCoord y) override;
    void DoDrawBitmap(const wxBitmap &bmp, wxCoord x, wxCoord y,
                              bool useMask = false) override;

    void DoDrawText(std::string_view text, wxPoint pt) override;
    void DoDrawRotatedText(std::string_view text, wxPoint pt, double angle) override;

    bool DoBlit(wxCoord xdest, wxCoord ydest, wxCoord width, wxCoord height,
                        wxDC *source, wxCoord xsrc, wxCoord ysrc,
                        wxRasterOperationMode rop = wxRasterOperationMode::Copy, bool useMask = false,
                        wxCoord xsrcMask = wxDefaultCoord, wxCoord ysrcMask = wxDefaultCoord) override;

    bool DoStretchBlit(wxCoord xdest, wxCoord ydest,
                               wxCoord dstWidth, wxCoord dstHeight,
                               wxDC *source,
                               wxCoord xsrc, wxCoord ysrc,
                               wxCoord srcWidth, wxCoord srcHeight,
                               wxRasterOperationMode rop = wxRasterOperationMode::Copy, bool useMask = false,
                               wxCoord xsrcMask = wxDefaultCoord, wxCoord ysrcMask = wxDefaultCoord) override;

    void DoSetClippingRegion(wxCoord x, wxCoord y,
                                     wxCoord width, wxCoord height) override;
    void DoSetDeviceClippingRegion(const wxRegion& region) override;
    bool DoGetClippingRect(wxRect& rect) const override;

    void DoGetSizeMM(int* width, int* height) const override;

    void DoDrawLines(int n, const wxPoint points[],
                             wxCoord xoffset, wxCoord yoffset) override;
    void DoDrawPolygon(int n, const wxPoint points[],
                               wxCoord xoffset, wxCoord yoffset,
                               wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven) override;
    void DoDrawPolyPolygon(int n, const int count[], const wxPoint points[],
                                   wxCoord xoffset, wxCoord yoffset,
                                   wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven) override;
    wxBitmap DoGetAsBitmap(const wxRect *subrect) const override
    {
        return subrect == nullptr ? GetSelectedBitmap()
                               : GetSelectedBitmap().GetSubBitmap(*subrect);
    }


#if wxUSE_PALETTE
    // MSW specific, select a logical palette into the HDC
    // (tell windows to translate pixel from other palettes to our custom one
    // and vice versa)
    // Realize tells it to also reset the system palette to this one.
    void DoSelectPalette(bool realize = false);

    // Find out what palette our parent window has, then select it into the dc
    void InitializePalette();
#endif // wxUSE_PALETTE

protected:
    // common part of DoDrawText() and DoDrawRotatedText()
    void DrawAnyText(std::string_view text, wxPoint pt);

    // common part of DoSetClippingRegion() and DoSetDeviceClippingRegion()
    void SetClippingHrgn(WXHRGN hrgn);

    // implementation of DoGetSize() for wxScreen/PrinterDC: this simply
    // returns the size of the entire device this DC is associated with
    //
    // notice that we intentionally put it in a separate function instead of
    // DoGetSize() itself because we want it to remain pure virtual both
    // because each derived class should take care to define it as needed (this
    // implementation is not at all always appropriate) and because we want
    // wxDC to be an ABC to prevent it from being created directly
    wxSize GetDeviceSize() const;


    // MSW-specific member variables
    // -----------------------------

    wxBitmap          m_selectedBitmap;

    // our HDC
    WXHDC             m_hDC{nullptr};

    // Store all old GDI objects when do a SelectObject, so we can select them
    // back in (this unselecting user's objects) so we can safely delete the
    // DC.
    WXHBITMAP         m_oldBitmap{nullptr};
    WXHPEN            m_oldPen{nullptr};
    WXHBRUSH          m_oldBrush{nullptr};
    WXHFONT           m_oldFont{nullptr};

#if wxUSE_PALETTE
    WXHPALETTE        m_oldPalette{nullptr};
#endif // wxUSE_PALETTE

#if wxUSE_DC_CACHEING
    inline static wxObjectList     sm_bitmapCache;
    inline static wxObjectList     sm_dcCache;
#endif

    bool m_isClipBoxValid{false};
    
    // TRUE => DeleteDC() in dtor, FALSE => only ReleaseDC() it
    bool              m_bOwnsDC{false};


    wxDECLARE_CLASS(wxMSWDCImpl);
};

// ----------------------------------------------------------------------------
// wxDCTemp: a wxDC which doesn't free the given HDC (used by wxWidgets
// only/mainly)
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxDCTempImpl : public wxMSWDCImpl
{
public:
    // construct a temporary DC with the specified HDC and size (it should be
    // specified whenever we know it for this HDC)
    wxDCTempImpl(wxDC *owner, WXHDC hdc, const wxSize& size )
        : wxMSWDCImpl( owner, hdc ),
          m_size(size)
    {
    }

    ~wxDCTempImpl()
    {
        // prevent base class dtor from freeing it
        SetHDC((WXHDC)nullptr);
    }

    wxDCTempImpl(const wxDCTempImpl&) = delete;
	wxDCTempImpl& operator=(const wxDCTempImpl&) = delete;
    wxDCTempImpl(wxDCTempImpl&&) = default;
	wxDCTempImpl& operator=(wxDCTempImpl&&) = default;

    wxSize DoGetSize() const override
    {
        wxASSERT_MSG( m_size.IsFullySpecified(),
                      wxT("size of this DC hadn't been set and is unknown") );

        return {m_size.x, m_size.y};
    }

private:
    // size of this DC must be explicitly set by SetSize() as we have no way to
    // find it ourselves
    const wxSize m_size;
};

struct WXDLLIMPEXP_CORE wxDCTemp : public wxDC
{
    wxDCTemp(WXHDC hdc, const wxSize& size = wxDefaultSize)
        : wxDC(std::make_unique<wxDCTempImpl>(this, hdc, size))
    {
    }
};

#endif // _WX_MSW_DC_H_

