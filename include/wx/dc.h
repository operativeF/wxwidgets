/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dc.h
// Purpose:     wxDC class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     05/25/99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DC_H_BASE_
#define _WX_DC_H_BASE_

// ----------------------------------------------------------------------------
// headers which we must include here
// ----------------------------------------------------------------------------

#include "wx/object.h"          // the base class

#include "wx/intl.h"            // for wxLayoutDirection
#include "wx/colour.h"          // we have member variables of these classes
#include "wx/font.h"            // so we can't do without them
#include "wx/bitmap.h"          // for wxNullBitmap
#include "wx/brush.h"
#include "wx/pen.h"
#include "wx/palette.h"

import WX.Cfg.Flags;
import WX.GDI.Flags;

import Utils.Geometry;

#include <memory>
import <string>;
import <string_view>;
import <vector>;

class wxDC;
class wxClientDC;
class wxPaintDC;
class wxWindowDC;
class wxScreenDC;
class wxMemoryDC;
class wxPrinterDC;
class wxPrintData;
class wxWindow;

#if wxUSE_GRAPHICS_CONTEXT
#include "wx/graphics.h"
#endif

//  Logical ops
enum class wxRasterOperationMode
{
    Clear,       // 0
    Xor,         // src XOR dst
    Invert,      // NOT dst
    OrReverse,  // src OR (NOT dst)
    AndReverse, // src AND (NOT dst)
    Copy,        // src
    And,         // src AND dst
    AndInvert,  // (NOT src) AND dst
    NoOp,       // dst
    Nor,         // (NOT src) AND (NOT dst)
    Equiv,       // (NOT src) XOR dst
    SrcInvert,  // (NOT src)
    OrInvert,   // (NOT src) OR dst
    Nand,        // (NOT src) OR (NOT dst)
    Or,          // src OR dst
    Set          // 1
};

//  Flood styles
enum class wxFloodFillStyle
{
    Surface,
    Border
};

//  Mapping modes
enum class wxMappingMode
{
    Text,
    Metric,
    LoMetric,
    Twips,
    Points
};

WX_DECLARE_LIST_WITH_DECL(wxPoint, wxPointList, class);

// Description of text characteristics.
struct wxFontMetrics
{
    int height{0};             // Total character height.
    int ascent{0};             // Part of the height above the baseline.
    int descent{0};            // Part of the height below the baseline.
    int internalLeading{0};    // Intra-line spacing.
    int externalLeading{0};    // Inter-line spacing.
    int averageWidth{0};       // Average font width, a.k.a. "x-width".
};

//-----------------------------------------------------------------------------
// wxDCFactory
//-----------------------------------------------------------------------------

class wxDCImpl;

class wxDCFactory
{
public:
    virtual ~wxDCFactory() = default;

    virtual std::unique_ptr<wxDCImpl> CreateWindowDC( wxWindowDC *owner, wxWindow *window ) = 0;
    virtual std::unique_ptr<wxDCImpl> CreateClientDC( wxClientDC *owner, wxWindow *window ) = 0;
    virtual std::unique_ptr<wxDCImpl> CreatePaintDC( wxPaintDC *owner, wxWindow *window ) = 0;
    virtual std::unique_ptr<wxDCImpl> CreateMemoryDC( wxMemoryDC *owner ) = 0;
    virtual std::unique_ptr<wxDCImpl> CreateMemoryDC( wxMemoryDC *owner, wxBitmap &bitmap ) = 0;
    virtual std::unique_ptr<wxDCImpl> CreateMemoryDC( wxMemoryDC *owner, wxDC *dc ) = 0;
    virtual std::unique_ptr<wxDCImpl> CreateScreenDC( wxScreenDC *owner ) = 0;
#if wxUSE_PRINTING_ARCHITECTURE
    virtual std::unique_ptr<wxDCImpl> CreatePrinterDC( wxPrinterDC *owner, const wxPrintData &data  ) = 0;
#endif

    static void Set(wxDCFactory *factory);
    static wxDCFactory *Get();

private:
    inline static wxDCFactory *m_factory{nullptr};
};

//-----------------------------------------------------------------------------
// wxNativeDCFactory
//-----------------------------------------------------------------------------

class wxNativeDCFactory: public wxDCFactory
{
public:
    std::unique_ptr<wxDCImpl> CreateWindowDC( wxWindowDC *owner, wxWindow *window ) override;
    std::unique_ptr<wxDCImpl> CreateClientDC( wxClientDC *owner, wxWindow *window ) override;
    std::unique_ptr<wxDCImpl> CreatePaintDC( wxPaintDC *owner, wxWindow *window ) override;
    std::unique_ptr<wxDCImpl> CreateMemoryDC( wxMemoryDC *owner ) override;
    std::unique_ptr<wxDCImpl> CreateMemoryDC( wxMemoryDC *owner, wxBitmap &bitmap ) override;
    std::unique_ptr<wxDCImpl> CreateMemoryDC( wxMemoryDC *owner, wxDC *dc ) override;
    std::unique_ptr<wxDCImpl> CreateScreenDC( wxScreenDC *owner ) override;
#if wxUSE_PRINTING_ARCHITECTURE
    std::unique_ptr<wxDCImpl> CreatePrinterDC( wxPrinterDC *owner, const wxPrintData &data  ) override;
#endif
};

//-----------------------------------------------------------------------------
// wxDCImpl
//-----------------------------------------------------------------------------

class wxDCImpl: public wxObject
{
public:
    wxDCImpl( wxDC *owner );

    wxDC *GetOwner() const { return m_owner; }

    wxWindow* GetWindow() const { return m_window; }

    void SetWindow(wxWindow* w) { m_window = w; }

    virtual bool IsOk() const { return m_ok; }

    // query capabilities

    virtual bool CanDrawBitmap() const = 0;
    virtual bool CanGetTextExtent() const = 0;

    // get Cairo context
    virtual void* GetCairoContext() const
    {
        return nullptr;
    }

    virtual void* GetHandle() const { return nullptr; }

    // query dimension, colour deps, resolution

    virtual wxSize DoGetSize() const = 0;

    wxSize GetSize() const
    {
        return DoGetSize();
    }

    virtual void DoGetSizeMM(int* width, int* height) const = 0;

    virtual int GetDepth() const = 0;
    virtual wxSize GetPPI() const = 0;

    // Right-To-Left (RTL) modes

    virtual void SetLayoutDirection([[maybe_unused]] wxLayoutDirection dir) { }
    virtual wxLayoutDirection GetLayoutDirection() const  { return wxLayoutDirection::Default; }

    // page and document

    virtual bool wxStartDoc([[maybe_unused]] const std::string& message) { return true; }
    virtual void EndDoc() { }

    virtual void StartPage() { }
    virtual void EndPage() { }

    // flushing the content of this dc immediately eg onto screen
    virtual void Flush() { }

    // coordinates conversions and transforms
    virtual wxPoint DeviceToLogical(wxCoord x, wxCoord y) const;
    virtual wxPoint LogicalToDevice(wxCoord x, wxCoord y) const;
    virtual wxSize DeviceToLogicalRel(int x, int y) const;
    virtual wxSize LogicalToDeviceRel(int x, int y) const;

    // bounding box

    virtual void CalcBoundingBox(wxCoord x, wxCoord y)
    {
      // Bounding box is internally stored in device units.
      const wxPoint ptDev = LogicalToDevice(x, y);
      x = ptDev.x;
      y = ptDev.y;

      if ( m_isBBoxValid )
      {
         if ( x < m_minX ) m_minX = x;
         if ( y < m_minY ) m_minY = y;
         if ( x > m_maxX ) m_maxX = x;
         if ( y > m_maxY ) m_maxY = y;
      }
      else
      {
         m_isBBoxValid = true;

         m_minX = x;
         m_minY = y;
         m_maxX = x;
         m_maxY = y;
      }
    }
    
    void ResetBoundingBox()
    {
        m_isBBoxValid = false;

        m_minX = 0;
        m_maxX = 0;
        m_minY = 0;
        m_maxY = 0;
    }

    // Get bounding box in logical units.
    wxCoord MinX() const { return m_isBBoxValid ? DeviceToLogical(m_minX, m_minY).x : 0; }
    wxCoord MaxX() const { return m_isBBoxValid ? DeviceToLogical(m_maxX, m_maxY).x : 0; }
    wxCoord MinY() const { return m_isBBoxValid ? DeviceToLogical(m_minX, m_minY).y : 0; }
    wxCoord MaxY() const { return m_isBBoxValid ? DeviceToLogical(m_maxX, m_maxY).y : 0; }

    // setters and getters

    virtual void SetFont(const wxFont& font) = 0;
    virtual const wxFont& GetFont() const { return m_font; }

    virtual void SetPen(const wxPen& pen) = 0;
    virtual const wxPen& GetPen() const { return m_pen; }

    virtual void SetBrush(const wxBrush& brush) = 0;
    virtual const wxBrush& GetBrush() const { return m_brush; }

    virtual void SetBackground(const wxBrush& brush) = 0;
    virtual const wxBrush& GetBackground() const { return m_backgroundBrush; }

    virtual void SetBackgroundMode(wxBrushStyle mode) = 0;
    virtual wxBrushStyle GetBackgroundMode() const { return m_backgroundMode; }

    virtual void SetTextForeground(const wxColour& colour)
        { m_textForegroundColour = colour; }
    virtual const wxColour& GetTextForeground() const
        { return m_textForegroundColour; }

    virtual void SetTextBackground(const wxColour& colour)
        { m_textBackgroundColour = colour; }
    virtual const wxColour& GetTextBackground() const
        { return m_textBackgroundColour; }

#if wxUSE_PALETTE
    virtual void SetPalette(const wxPalette& palette) = 0;
#endif // wxUSE_PALETTE

    // inherit the DC attributes (font and colours) from the given window
    //
    // this is called automatically when a window, client or paint DC is
    // created
    virtual void InheritAttributes(wxWindow *win);


    // logical functions

    virtual void SetLogicalFunction(wxRasterOperationMode function) = 0;
    virtual wxRasterOperationMode GetLogicalFunction() const
                                      { return m_logicalFunction; }

    // text measurement

    virtual wxCoord GetCharHeight() const = 0;
    virtual wxCoord wxGetCharWidth() const = 0;

    // The derived classes should really override DoGetFontMetrics() to return
    // the correct values in the future but for now provide a default
    // implementation in terms of DoGetTextExtent() to avoid breaking the
    // compilation of all other ports as wxMSW is the only one to implement it.
    virtual void DoGetFontMetrics(int *height,
                                  int *ascent,
                                  int *descent,
                                  int *internalLeading,
                                  int *externalLeading,
                                  int *averageWidth) const;

    virtual wxSize DoGetTextExtent(std::string_view string,
                                 wxCoord *descent = nullptr,
                                 wxCoord *externalLeading = nullptr,
                                 const wxFont *theFont = nullptr) const = 0;
    virtual void GetMultiLineTextExtent(std::string_view string,
                                        wxCoord *width,
                                        wxCoord *height,
                                        wxCoord *heightLine = nullptr,
                                        const wxFont *font = nullptr) const;
    virtual std::vector<int> DoGetPartialTextExtents(std::string_view text) const;

    // clearing

    virtual void Clear() = 0;

    // clipping

    // Note that this pure virtual method has an implementation that updates
    // the values returned by DoGetClippingBox() and so can be called from the
    // derived class overridden version if it makes sense (i.e. if the clipping
    // box coordinates are not already updated in some other way).
    virtual void DoSetClippingRegion(wxCoord x, wxCoord y,
                                     wxCoord w, wxCoord h) = 0;

    // NB: this function works with device coordinates, not the logical ones!
    virtual void DoSetDeviceClippingRegion(const wxRegion& region) = 0;

    // Method used to implement wxDC::GetClippingBox().
    //
    // Default implementation returns values stored in m_clip[XY][12] member
    // variables, so this method doesn't need to be overridden if they're kept
    // up to date.
    virtual bool DoGetClippingRect(wxRect& rect) const;

    virtual void DestroyClippingRegion()
    {
        m_clipping = false;

        m_clipX1 = 0;
        m_clipX2 = 0;
        m_clipY1 = 0;
        m_clipY2 = 0;
    }

    // coordinates conversions and transforms

    virtual wxCoord DeviceToLogicalX(wxCoord x) const;
    virtual wxCoord DeviceToLogicalY(wxCoord y) const;
    virtual wxCoord DeviceToLogicalXRel(wxCoord x) const;
    virtual wxCoord DeviceToLogicalYRel(wxCoord y) const;
    virtual wxCoord LogicalToDeviceX(wxCoord x) const;
    virtual wxCoord LogicalToDeviceY(wxCoord y) const;
    virtual wxCoord LogicalToDeviceXRel(wxCoord x) const;
    virtual wxCoord LogicalToDeviceYRel(wxCoord y) const;

    virtual void SetMapMode(wxMappingMode mode);
    virtual wxMappingMode GetMapMode() const { return m_mappingMode; }

    virtual void SetUserScale(double userScale);
    virtual void SetUserScale(wxScale userScale);
    virtual wxScale GetUserScale() const
    {
        return m_userScale;
    }

    virtual void SetLogicalScale(double logicalScale);
    virtual void SetLogicalScale(wxScale logicalScale);
    virtual wxScale GetLogicalScale() const
    {
        return m_logicalScale;
    }

    virtual void SetLogicalOrigin(wxPoint logicalOrigin);
    virtual wxPoint DoGetLogicalOrigin() const
    {
        return m_logicalOrigin;
    }

    virtual void SetDeviceOrigin(wxPoint deviceOrigin);

    virtual wxPoint DoGetDeviceOrigin() const
    {
        return m_deviceLocalOrigin;
    }

#if wxUSE_DC_TRANSFORM_MATRIX
    // Transform matrix support is not available in most ports right now
    // (currently only wxMSW provides it) so do nothing in these methods by
    // default.
    virtual bool CanUseTransformMatrix() const
        { return false; }
    virtual bool SetTransformMatrix([[maybe_unused]] const wxAffineMatrix2D& matrix)
        { return false; }
    virtual wxAffineMatrix2D GetTransformMatrix() const
        { return wxAffineMatrix2D(); }
    virtual void ResetTransformMatrix()
        { }
#endif // wxUSE_DC_TRANSFORM_MATRIX

    virtual void SetDeviceLocalOrigin( wxPoint deviceLocalOrigin );

    virtual void ComputeScaleAndOrigin();

    // this needs to overridden if the axis is inverted
    virtual void SetAxisOrientation(bool xLeftRight, bool yBottomUp);

    virtual double GetContentScaleFactor() const { return m_contentScaleFactor; }

#ifdef __WXMSW__
    // Native Windows functions using the underlying WXHDC don't honour GDI+
    // transformations which may be applied to it. Using this function we can
    // transform the coordinates manually before passing them to such functions
    // (as in e.g. wxRendererMSW code). It doesn't do anything if this is not a
    // wxGCDC.
    virtual wxRect MSWApplyGDIPlusTransform(const wxRect& r) const
    {
        return r;
    }
#endif // __WXMSW__


    // ---------------------------------------------------------
    // the actual drawing API

    virtual bool DoFloodFill(wxCoord x, wxCoord y, const wxColour& col,
                             wxFloodFillStyle style = wxFloodFillStyle::Surface) = 0;

    virtual void DoGradientFillLinear(const wxRect& rect,
                                      const wxColour& initialColour,
                                      const wxColour& destColour,
                                      wxDirection nDirection = wxEAST);

    virtual void DoGradientFillConcentric(const wxRect& rect,
                                        const wxColour& initialColour,
                                        const wxColour& destColour,
                                        const wxPoint& circleCenter);

    virtual bool DoGetPixel(wxCoord x, wxCoord y, wxColour *col) const = 0;

    virtual void DoDrawPoint(wxCoord x, wxCoord y) = 0;
    virtual void DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2) = 0;

    virtual void DoDrawArc(wxCoord x1, wxCoord y1,
                           wxCoord x2, wxCoord y2,
                           wxCoord xc, wxCoord yc) = 0;
    virtual void DoDrawCheckMark(wxCoord x, wxCoord y,
                                 wxCoord width, wxCoord height);
    virtual void DoDrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
                                   double sa, double ea) = 0;

    virtual void DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height) = 0;
    virtual void DoDrawRoundedRectangle(wxCoord x, wxCoord y,
                                        wxCoord width, wxCoord height,
                                        double radius) = 0;
    virtual void DoDrawEllipse(wxCoord x, wxCoord y,
                               wxCoord width, wxCoord height) = 0;

    virtual void DoCrossHair(wxCoord x, wxCoord y) = 0;

    virtual void DoDrawIcon(const wxIcon& icon, wxCoord x, wxCoord y) = 0;
    virtual void DoDrawBitmap(const wxBitmap &bmp, wxCoord x, wxCoord y,
                              bool useMask = false) = 0;

    virtual void DoDrawText(std::string_view text, wxPoint pt) = 0;
    virtual void DoDrawRotatedText(std::string_view text,
                                   wxPoint pt, double angle) = 0;

    virtual bool DoBlit(wxCoord xdest, wxCoord ydest,
                        wxCoord width, wxCoord height,
                        wxDC *source,
                        wxPoint src,
                        wxRasterOperationMode rop = wxRasterOperationMode::Copy,
                        bool useMask = false,
                        wxPoint srcMask = wxDefaultPosition) = 0;

    virtual bool DoStretchBlit(wxCoord xdest, wxCoord ydest,
                               wxCoord dstWidth, wxCoord dstHeight,
                               wxDC *source,
                               wxPoint src,
                               wxCoord srcWidth, wxCoord srcHeight,
                               wxRasterOperationMode rop = wxRasterOperationMode::Copy,
                               bool useMask = false,
                               wxPoint srcMask = wxDefaultPosition);

    virtual wxBitmap DoGetAsBitmap([[maybe_unused]] const wxRect *subrect) const
        { return wxNullBitmap; }


    virtual void DoDrawLines(int n, const wxPoint points[],
                             wxCoord xoffset, wxCoord yoffset ) = 0;
    virtual void DrawLines(const wxPointList *list,
                           wxCoord xoffset, wxCoord yoffset );

    virtual void DoDrawPolygon(int n, const wxPoint points[],
                           wxCoord xoffset, wxCoord yoffset,
                           wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven) = 0;
    virtual void DoDrawPolyPolygon(int n, const int count[], const wxPoint points[],
                               wxCoord xoffset, wxCoord yoffset,
                               wxPolygonFillMode fillStyle);
    void DrawPolygon(const wxPointList *list,
                     wxCoord xoffset, wxCoord yoffset,
                     wxPolygonFillMode fillStyle );


#if wxUSE_SPLINES
    void DrawSpline(wxPoint pt1,
                    wxPoint pt2,
                    wxPoint pt3);
    void DrawSpline(int n, const wxPoint points[]);
    void DrawSpline(const wxPointList *points) { DoDrawSpline(points); }

    virtual void DoDrawSpline(const wxPointList *points);
#endif

    // ---------------------------------------------------------
    // wxMemoryDC Impl API

    virtual void DoSelect([[maybe_unused]] const wxBitmap& bmp)
       { }

    virtual const wxBitmap& GetSelectedBitmap() const
        { return wxNullBitmap; }
    virtual wxBitmap& GetSelectedBitmap()
        { return wxNullBitmap; }

    // ---------------------------------------------------------
    // wxPrinterDC Impl API

    virtual wxRect GetPaperRect() const
        { const wxSize sz = DoGetSize(); return wxRect(0,0, sz.x, sz.y); }

    virtual int GetResolution() const
        { return -1; }

#if wxUSE_GRAPHICS_CONTEXT
    virtual wxGraphicsContext* GetGraphicsContext() const
        { return nullptr; }
    virtual void SetGraphicsContext( [[maybe_unused]] std::unique_ptr<wxGraphicsContext> ctx )
        {}
#endif

protected:
    // returns adjustment factor for converting wxFont "point size"; in wx
    // it is point size on screen and needs to be multiplied by this value
    // for rendering on higher-resolution DCs such as printer ones
    static float GetFontPointSizeAdjustment(float dpi);

    // Return the number of pixels per mm in the horizontal and vertical
    // directions, respectively.
    //
    // If the physical size of the DC is not known, or doesn't make sense, as
    // for a SVG DC, for example, a fixed value corresponding to the standard
    // DPI is used.
    double GetMMToPXx() const;
    double GetMMToPXy() const;

    wxColour          m_textForegroundColour;
    wxColour          m_textBackgroundColour;

    wxPen             m_pen;
    wxBrush           m_brush;
    wxBrush           m_backgroundBrush;
    wxFont            m_font;
#if wxUSE_PALETTE
    wxPalette         m_palette;
#endif

    wxScale m_logicalScale{1.0, 1.0};
    wxScale m_userScale{1.0, 1.0};
    wxScale m_scale{1.0, 1.0};

        // coordinate system variables

    wxPoint m_logicalOrigin{0, 0};
    wxPoint m_deviceOrigin{0, 0};
    wxPoint m_deviceLocalOrigin{0, 0};   // non-zero if native top-left corner
                                   // is not at 0,0. This was the case under
                                   // Mac's GrafPorts (coordinate system
                                   // used toplevel window's origin) and
                                   // e.g. for Postscript, where the native
                                   // origin in the bottom left corner.

    // window on which the DC draws or NULL
    wxWindow   *m_window{nullptr};

// FIXME: Probably should make all variables private.
private:
    wxDC       *m_owner;

protected:

    double m_contentScaleFactor{1.0}; // used by high resolution displays (retina)

    // Pixel per mm in horizontal and vertical directions.
    //
    // These variables are computed on demand by GetMMToPX[xy]() functions,
    // don't access them directly other than for assigning to them.
    mutable double m_mm_to_pix_x{0.0};
    mutable double m_mm_to_pix_y{0.0};

    int m_signX{1};
    int m_signY{1};  // Used by SetAxisOrientation() to invert the axes
    
    // bounding and clipping boxes
    wxCoord m_minX{0};
    wxCoord m_minY{0};
    wxCoord m_maxX{0};
    wxCoord m_maxY{0}; // Bounding box is stored in device units.

    wxCoord m_clipX1{0};
    wxCoord m_clipY1{0};
    wxCoord m_clipX2{0};
    wxCoord m_clipY2{0};  // Clipping box is stored in logical units.

    wxRasterOperationMode m_logicalFunction{wxRasterOperationMode::Copy};
    wxBrushStyle m_backgroundMode{wxBrushStyle::Transparent};
    wxMappingMode m_mappingMode{wxMappingMode::Text};

    // flags
#if wxUSE_PALETTE
    bool              m_hasCustomPalette{false};
#endif // wxUSE_PALETTE

    bool m_colour{true};
    bool m_ok{true};
    bool m_clipping{false};
    bool m_isInteractive{false};
    bool m_isBBoxValid{false};

private:
    // Return the full DC area in logical coordinates.
    wxRect GetLogicalArea() const;

    wxDECLARE_ABSTRACT_CLASS(wxDCImpl);
};


class wxDC : public wxObject
{
public:
    // copy attributes (font, colours and writing direction) from another DC
    void CopyAttributes(const wxDC& dc);

    wxDCImpl *GetImpl()
        { return m_pimpl.get(); }
    const wxDCImpl *GetImpl() const
        { return m_pimpl.get(); }

    wxWindow *GetWindow() const
        { return m_pimpl->GetWindow(); }

    void *GetHandle() const
        { return m_pimpl->GetHandle(); }

    bool IsOk() const
        { return m_pimpl && m_pimpl->IsOk(); }

    // query capabilities

    bool CanDrawBitmap() const
        { return m_pimpl->CanDrawBitmap(); }
    bool CanGetTextExtent() const
        { return m_pimpl->CanGetTextExtent(); }

    // query dimension, colour deps, resolution

    wxSize GetSize() const
        { return m_pimpl->DoGetSize(); }

    void GetSizeMM(int* width, int* height) const
        { m_pimpl->DoGetSizeMM(width, height); }
    wxSize GetSizeMM() const
    {
        int w, h;
        m_pimpl->DoGetSizeMM(&w, &h);
        return wxSize(w, h);
    }

    int GetDepth() const
        { return m_pimpl->GetDepth(); }
    wxSize GetPPI() const
        { return m_pimpl->GetPPI(); }

    virtual int GetResolution() const
        { return m_pimpl->GetResolution(); }

    double GetContentScaleFactor() const
        { return m_pimpl->GetContentScaleFactor(); }

    // Right-To-Left (RTL) modes

    void SetLayoutDirection(wxLayoutDirection dir)
        { m_pimpl->SetLayoutDirection( dir ); }
    wxLayoutDirection GetLayoutDirection() const
        { return m_pimpl->GetLayoutDirection(); }

    // page and document

    bool wxStartDoc(const std::string& message)
        { return m_pimpl->wxStartDoc(message); }
    void EndDoc()
        { m_pimpl->EndDoc(); }

    void StartPage()
        { m_pimpl->StartPage(); }
    void EndPage()
        { m_pimpl->EndPage(); }

    // bounding box

    void CalcBoundingBox(wxCoord x, wxCoord y)
        { m_pimpl->CalcBoundingBox(x,y); }
    void ResetBoundingBox()
        { m_pimpl->ResetBoundingBox(); }

    wxCoord MinX() const
        { return m_pimpl->MinX(); }
    wxCoord MaxX() const
        { return m_pimpl->MaxX(); }
    wxCoord MinY() const
        { return m_pimpl->MinY(); }
    wxCoord MaxY() const
        { return m_pimpl->MaxY(); }

    // setters and getters

    void SetFont(const wxFont& font)
        { m_pimpl->SetFont( font ); }
    const wxFont&   GetFont() const
        { return m_pimpl->GetFont(); }

    void SetPen(const wxPen& pen)
        { m_pimpl->SetPen( pen ); }
    const wxPen&    GetPen() const
        { return m_pimpl->GetPen(); }

    void SetBrush(const wxBrush& brush)
        { m_pimpl->SetBrush( brush ); }
    const wxBrush&  GetBrush() const
        { return m_pimpl->GetBrush(); }

    void SetBackground(const wxBrush& brush)
        { m_pimpl->SetBackground( brush ); }
    const wxBrush&  GetBackground() const
        { return m_pimpl->GetBackground(); }

    void SetBackgroundMode(wxBrushStyle mode)
        { m_pimpl->SetBackgroundMode( mode ); }
    wxBrushStyle GetBackgroundMode() const
        { return m_pimpl->GetBackgroundMode(); }

    void SetTextForeground(const wxColour& colour)
        { m_pimpl->SetTextForeground(colour); }
    const wxColour& GetTextForeground() const
        { return m_pimpl->GetTextForeground(); }

    void SetTextBackground(const wxColour& colour)
        { m_pimpl->SetTextBackground(colour); }
    const wxColour& GetTextBackground() const
        { return m_pimpl->GetTextBackground(); }

#if wxUSE_PALETTE
    void SetPalette(const wxPalette& palette)
        { m_pimpl->SetPalette(palette); }
#endif // wxUSE_PALETTE

    // logical functions

    void SetLogicalFunction(wxRasterOperationMode function)
        { m_pimpl->SetLogicalFunction(function); }
    wxRasterOperationMode GetLogicalFunction() const
        { return m_pimpl->GetLogicalFunction(); }

    // text measurement

    wxCoord GetCharHeight() const
        { return m_pimpl->GetCharHeight(); }
    wxCoord wxGetCharWidth() const
        { return m_pimpl->wxGetCharWidth(); }

    wxFontMetrics GetFontMetrics() const
    {
        wxFontMetrics fm;
        m_pimpl->DoGetFontMetrics(&fm.height, &fm.ascent, &fm.descent,
                                  &fm.internalLeading, &fm.externalLeading,
                                  &fm.averageWidth);
        return fm;
    }

    wxSize GetTextExtent(std::string_view string,
                       wxCoord *descent = nullptr,
                       wxCoord *externalLeading = nullptr,
                       const wxFont *theFont = nullptr) const
        { return m_pimpl->DoGetTextExtent(string, descent, externalLeading, theFont); }

    void GetMultiLineTextExtent(std::string_view string,
                                        wxCoord *width,
                                        wxCoord *height,
                                        wxCoord *heightLine = nullptr,
                                        const wxFont *font = nullptr) const
        { m_pimpl->GetMultiLineTextExtent( string, width, height, heightLine, font ); }

    wxSize GetMultiLineTextExtent(std::string_view string) const
    {
        wxCoord w, h;
        m_pimpl->GetMultiLineTextExtent(string, &w, &h);
        return wxSize(w, h);
    }

    std::vector<int> GetPartialTextExtents(std::string_view text) const
        { return m_pimpl->DoGetPartialTextExtents(text); }

    // clearing

    void Clear()
        { m_pimpl->Clear(); }

    // clipping

    void SetClippingRegion(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
        { m_pimpl->DoSetClippingRegion(x, y, width, height); }
    void SetClippingRegion(const wxPoint& pt, const wxSize& sz)
        { m_pimpl->DoSetClippingRegion(pt.x, pt.y, sz.x, sz.y); }
    void SetClippingRegion(const wxRect& rect)
        { m_pimpl->DoSetClippingRegion(rect.x, rect.y, rect.width, rect.height); }

    // unlike the functions above, the coordinates of the region used in this
    // one are in device coordinates, not the logical ones
    void SetDeviceClippingRegion(const wxRegion& region)
        { m_pimpl->DoSetDeviceClippingRegion(region); }

    void DestroyClippingRegion()
        { m_pimpl->DestroyClippingRegion(); }

    bool GetClippingBox(wxCoord *x, wxCoord *y, wxCoord *w, wxCoord *h) const
    {
        wxRect r;
        const bool clipping = m_pimpl->DoGetClippingRect(r);
        if ( x )
            *x = r.x;
        if ( y )
            *y = r.y;
        if ( w )
            *w = r.width;
        if ( h )
            *h = r.height;
        return clipping;
    }
    bool GetClippingBox(wxRect& rect) const
        { return m_pimpl->DoGetClippingRect(rect); }

    // coordinates conversions and transforms

    wxCoord DeviceToLogicalX(wxCoord x) const
        { return m_pimpl->DeviceToLogicalX(x); }
    wxCoord DeviceToLogicalY(wxCoord y) const
        { return m_pimpl->DeviceToLogicalY(y); }
    wxCoord DeviceToLogicalXRel(wxCoord x) const
        { return m_pimpl->DeviceToLogicalXRel(x); }
    wxCoord DeviceToLogicalYRel(wxCoord y) const
        { return m_pimpl->DeviceToLogicalYRel(y); }
    wxPoint DeviceToLogical(const wxPoint& pt) const
        { return m_pimpl->DeviceToLogical(pt.x, pt.y); }
    wxPoint DeviceToLogical(wxCoord x, wxCoord y) const
        { return m_pimpl->DeviceToLogical(x, y); }
    wxSize DeviceToLogicalRel(const wxSize& dim) const
        { return m_pimpl->DeviceToLogicalRel(dim.x, dim.y); }
    wxSize DeviceToLogicalRel(int x, int y) const
        { return m_pimpl->DeviceToLogicalRel(x, y); }
    wxCoord LogicalToDeviceX(wxCoord x) const
        { return m_pimpl->LogicalToDeviceX(x); }
    wxCoord LogicalToDeviceY(wxCoord y) const
        { return m_pimpl->LogicalToDeviceY(y); }
    wxCoord LogicalToDeviceXRel(wxCoord x) const
        { return m_pimpl->LogicalToDeviceXRel(x); }
    wxCoord LogicalToDeviceYRel(wxCoord y) const
        { return m_pimpl->LogicalToDeviceYRel(y); }
    wxPoint LogicalToDevice(const wxPoint& pt) const
        { return m_pimpl->LogicalToDevice(pt.x, pt.y); }
    wxPoint LogicalToDevice(wxCoord x, wxCoord y) const
        { return m_pimpl->LogicalToDevice(x, y); }
    wxSize LogicalToDeviceRel(const wxSize& dim) const
        { return m_pimpl->LogicalToDeviceRel(dim.x, dim.y); }
    wxSize LogicalToDeviceRel(int x, int y) const
        { return m_pimpl->LogicalToDeviceRel(x, y); }

    void SetMapMode(wxMappingMode mode)
        { m_pimpl->SetMapMode(mode); }
    wxMappingMode GetMapMode() const
        { return m_pimpl->GetMapMode(); }

    void SetUserScale(double userScale)
        { SetUserScale({ userScale, userScale }); }
    void SetUserScale(wxScale userScale)
        { m_pimpl->SetUserScale(userScale); }
    wxScale GetUserScale() const
        { return m_pimpl->GetUserScale(); }

    void SetLogicalScale(double logicalScale)
        { SetLogicalScale({ logicalScale, logicalScale }); }
    void SetLogicalScale(wxScale logicalScale)
        { m_pimpl->SetLogicalScale(logicalScale); }
    wxScale GetLogicalScale() const
        { return m_pimpl->GetLogicalScale(); }

    void SetLogicalOrigin(wxPoint logicalOrigin)
        { m_pimpl->SetLogicalOrigin(logicalOrigin); }
    wxPoint GetLogicalOrigin() const
        { return m_pimpl->DoGetLogicalOrigin(); }

    void SetDeviceOrigin( wxPoint deviceOrigin )
        { m_pimpl->SetDeviceOrigin(deviceOrigin); }
    wxPoint GetDeviceOrigin() const
        { return m_pimpl->DoGetDeviceOrigin(); }

    void SetAxisOrientation(bool xLeftRight, bool yBottomUp)
        { m_pimpl->SetAxisOrientation(xLeftRight, yBottomUp); }

#if wxUSE_DC_TRANSFORM_MATRIX
    bool CanUseTransformMatrix() const
        { return m_pimpl->CanUseTransformMatrix(); }

    bool SetTransformMatrix(const wxAffineMatrix2D &matrix)
        { return m_pimpl->SetTransformMatrix(matrix); }

    wxAffineMatrix2D GetTransformMatrix() const
        { return m_pimpl->GetTransformMatrix(); }

    void ResetTransformMatrix()
        { m_pimpl->ResetTransformMatrix(); }
#endif // wxUSE_DC_TRANSFORM_MATRIX

    // mostly internal
    void SetDeviceLocalOrigin( wxPoint deviceLocalOrigin )
        { m_pimpl->SetDeviceLocalOrigin( deviceLocalOrigin ); }


    // -----------------------------------------------
    // the actual drawing API

    bool FloodFill(wxCoord x, wxCoord y, const wxColour& col,
                   wxFloodFillStyle style = wxFloodFillStyle::Surface)
        { return m_pimpl->DoFloodFill(x, y, col, style); }
    bool FloodFill(const wxPoint& pt, const wxColour& col,
                   wxFloodFillStyle style = wxFloodFillStyle::Surface)
        { return m_pimpl->DoFloodFill(pt.x, pt.y, col, style); }

    // fill the area specified by rect with a radial gradient, starting from
    // initialColour in the centre of the cercle and fading to destColour.
    void GradientFillConcentric(const wxRect& rect,
                                const wxColour& initialColour,
                                const wxColour& destColour)
        { m_pimpl->DoGradientFillConcentric( rect, initialColour, destColour,
                                             wxPoint(rect.GetWidth() / 2,
                                                     rect.GetHeight() / 2)); }

    void GradientFillConcentric(const wxRect& rect,
                                const wxColour& initialColour,
                                const wxColour& destColour,
                                const wxPoint& circleCenter)
        { m_pimpl->DoGradientFillConcentric(rect, initialColour, destColour, circleCenter); }

    // fill the area specified by rect with a linear gradient
    void GradientFillLinear(const wxRect& rect,
                            const wxColour& initialColour,
                            const wxColour& destColour,
                            wxDirection nDirection = wxEAST)
        { m_pimpl->DoGradientFillLinear(rect, initialColour, destColour, nDirection); }

    bool GetPixel(wxCoord x, wxCoord y, wxColour *col) const
        { return m_pimpl->DoGetPixel(x, y, col); }
    bool GetPixel(const wxPoint& pt, wxColour *col) const
        { return m_pimpl->DoGetPixel(pt.x, pt.y, col); }

    void DrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2)
        { m_pimpl->DoDrawLine(x1, y1, x2, y2); }
    void DrawLine(const wxPoint& pt1, const wxPoint& pt2)
        { m_pimpl->DoDrawLine(pt1.x, pt1.y, pt2.x, pt2.y); }

    void CrossHair(wxCoord x, wxCoord y)
        { m_pimpl->DoCrossHair(x, y); }
    void CrossHair(const wxPoint& pt)
        { m_pimpl->DoCrossHair(pt.x, pt.y); }

    void DrawArc(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2,
                 wxCoord xc, wxCoord yc)
        { m_pimpl->DoDrawArc(x1, y1, x2, y2, xc, yc); }
    void DrawArc(const wxPoint& pt1, const wxPoint& pt2, const wxPoint& centre)
        { m_pimpl->DoDrawArc(pt1.x, pt1.y, pt2.x, pt2.y, centre.x, centre.y); }

    void DrawCheckMark(wxCoord x, wxCoord y,
                       wxCoord width, wxCoord height)
        { m_pimpl->DoDrawCheckMark(x, y, width, height); }
    void DrawCheckMark(const wxRect& rect)
        { m_pimpl->DoDrawCheckMark(rect.x, rect.y, rect.width, rect.height); }

    void DrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
                         double sa, double ea)
        { m_pimpl->DoDrawEllipticArc(x, y, w, h, sa, ea); }
    void DrawEllipticArc(const wxPoint& pt, const wxSize& sz,
                         double sa, double ea)
        { m_pimpl->DoDrawEllipticArc(pt.x, pt.y, sz.x, sz.y, sa, ea); }

    void DrawPoint(wxCoord x, wxCoord y)
        { m_pimpl->DoDrawPoint(x, y); }
    void DrawPoint(const wxPoint& pt)
        { m_pimpl->DoDrawPoint(pt.x, pt.y); }

    void DrawLines(int n, const wxPoint points[],
                   wxCoord xoffset = 0, wxCoord yoffset = 0)
        { m_pimpl->DoDrawLines(n, points, xoffset, yoffset); }
    void DrawLines(const wxPointList *list,
                   wxCoord xoffset = 0, wxCoord yoffset = 0)
        { m_pimpl->DrawLines( list, xoffset, yoffset ); }
    void DrawPolygon(int n, const wxPoint points[],
                     wxCoord xoffset = 0, wxCoord yoffset = 0,
                     wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven)
        { m_pimpl->DoDrawPolygon(n, points, xoffset, yoffset, fillStyle); }
    void DrawPolygon(const wxPointList *list,
                     wxCoord xoffset = 0, wxCoord yoffset = 0,
                     wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven)
        { m_pimpl->DrawPolygon( list, xoffset, yoffset, fillStyle ); }
    void DrawPolyPolygon(int n, const int count[], const wxPoint points[],
                         wxCoord xoffset = 0, wxCoord yoffset = 0,
                         wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven)
        { m_pimpl->DoDrawPolyPolygon(n, count, points, xoffset, yoffset, fillStyle); }
    void DrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
        { m_pimpl->DoDrawRectangle(x, y, width, height); }
    void DrawRectangle(const wxPoint& pt, const wxSize& sz)
        { m_pimpl->DoDrawRectangle(pt.x, pt.y, sz.x, sz.y); }
    void DrawRectangle(const wxRect& rect)
        { m_pimpl->DoDrawRectangle(rect.x, rect.y, rect.width, rect.height); }

    void DrawRoundedRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height,
                              double radius)
        { m_pimpl->DoDrawRoundedRectangle(x, y, width, height, radius); }
    void DrawRoundedRectangle(const wxPoint& pt, const wxSize& sz,
                             double radius)
        { m_pimpl->DoDrawRoundedRectangle(pt.x, pt.y, sz.x, sz.y, radius); }
    void DrawRoundedRectangle(const wxRect& r, double radius)
        { m_pimpl->DoDrawRoundedRectangle(r.x, r.y, r.width, r.height, radius); }

    void DrawCircle(wxCoord x, wxCoord y, wxCoord radius)
        { m_pimpl->DoDrawEllipse(x - radius, y - radius, 2*radius, 2*radius); }
    void DrawCircle(const wxPoint& pt, wxCoord radius)
        { m_pimpl->DoDrawEllipse(pt.x - radius, pt.y - radius, 2*radius, 2*radius); }

    void DrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
        { m_pimpl->DoDrawEllipse(x, y, width, height); }
    void DrawEllipse(const wxPoint& pt, const wxSize& sz)
        { m_pimpl->DoDrawEllipse(pt.x, pt.y, sz.x, sz.y); }
    void DrawEllipse(const wxRect& rect)
        { m_pimpl->DoDrawEllipse(rect.x, rect.y, rect.width, rect.height); }

    void DrawIcon(const wxIcon& icon, wxCoord x, wxCoord y)
        { m_pimpl->DoDrawIcon(icon, x, y); }
    void DrawIcon(const wxIcon& icon, const wxPoint& pt)
        { m_pimpl->DoDrawIcon(icon, pt.x, pt.y); }

    void DrawBitmap(const wxBitmap &bmp, wxCoord x, wxCoord y,
                    bool useMask = false)
        { m_pimpl->DoDrawBitmap(bmp, x, y, useMask); }
    void DrawBitmap(const wxBitmap &bmp, const wxPoint& pt,
                    bool useMask = false)
        { m_pimpl->DoDrawBitmap(bmp, pt.x, pt.y, useMask); }

    void wxDrawText(std::string_view text, const wxPoint& pt)
        { m_pimpl->DoDrawText(text, pt); }

    void DrawRotatedText(std::string_view text, const wxPoint& pt, double angle)
        { m_pimpl->DoDrawRotatedText(text, pt, angle); }

    // this version puts both optional bitmap and the text into the given
    // rectangle and aligns is as specified by alignment parameter; it also
    // will emphasize the character with the given index if it is != -1 and
    // return the bounding rectangle if required
    void DrawLabel(std::string_view text,
                           const wxBitmap& image,
                           const wxRect& rect,
                           unsigned int alignment = wxALIGN_LEFT | wxALIGN_TOP,
                           int indexAccel = -1,
                           wxRect *rectBounding = nullptr);

    void DrawLabel(std::string_view text, const wxRect& rect,
                   unsigned int alignment = wxALIGN_LEFT | wxALIGN_TOP,
                   int indexAccel = -1)
        { DrawLabel(text, wxNullBitmap, rect, alignment, indexAccel); }

    bool Blit(const wxPoint& destPt, const wxSize& sz,
              wxDC *source, const wxPoint& srcPt,
              wxRasterOperationMode rop = wxRasterOperationMode::Copy, bool useMask = false,
              const wxPoint& srcPtMask = wxDefaultPosition)
    {
        return m_pimpl->DoBlit(destPt.x, destPt.y, sz.x, sz.y,
                      source, srcPt, rop, useMask, srcPtMask);
    }

    bool StretchBlit(const wxPoint& dstPt, const wxSize& dstSize,
                     wxDC *source, const wxPoint& srcPt, const wxSize& srcSize,
                     wxRasterOperationMode rop = wxRasterOperationMode::Copy, bool useMask = false,
                     const wxPoint& srcMaskPt = wxDefaultPosition)
    {
        return m_pimpl->DoStretchBlit(dstPt.x, dstPt.y, dstSize.x, dstSize.y,
                      source, srcPt, srcSize.x, srcSize.y, rop, useMask, srcMaskPt);
    }

    wxBitmap GetAsBitmap(const wxRect *subrect = (const wxRect *) nullptr) const
    {
        return m_pimpl->DoGetAsBitmap(subrect);
    }

#if wxUSE_SPLINES
    void DrawSpline(wxPoint pt1,
                    wxPoint pt2,
                    wxPoint pt3)
        { m_pimpl->DrawSpline(pt1, pt2, pt3); }
    void DrawSpline(int n, const wxPoint points[])
        { m_pimpl->DrawSpline(n,points); }
    void DrawSpline(const wxPointList *points)
        { m_pimpl->DrawSpline(points); }
#endif // wxUSE_SPLINES

#ifdef WX_WINDOWS
    // GetHDC() is the simplest way to retrieve an WXHDC From a wxDC but only
    // works if this wxDC is GDI-based and fails for GDI+ contexts (and
    // anything else without WXHDC, e.g. wxPostScriptDC)
    WXHDC GetHDC() const;

    // don't use these methods manually, use GetTempHDC() instead
    virtual WXHDC AcquireHDC() { return GetHDC(); }
    virtual void ReleaseHDC([[maybe_unused]] WXHDC hdc) { }

    // helper class holding the result of GetTempHDC() with std::auto_ptr<>-like
    // semantics, i.e. it is moved when copied
    class TempHDC
    {
    public:
        TempHDC(wxDC& dc)
            : m_dc(dc),
              m_hdc(dc.AcquireHDC())
        {
        }

        TempHDC(const TempHDC& thdc)
            : m_dc(thdc.m_dc),
              m_hdc(thdc.m_hdc)
        {
            const_cast<TempHDC&>(thdc).m_hdc = nullptr;
        }

        ~TempHDC()
        {
            if ( m_hdc )
                m_dc.ReleaseHDC(m_hdc);
        }

        WXHDC GetHDC() const { return m_hdc; }

    private:
        wxDC& m_dc;
        WXHDC m_hdc;
    };

    // GetTempHDC() also works for wxGCDC (but still not for wxPostScriptDC &c)
    TempHDC GetTempHDC() { return TempHDC(*this); }
#endif // __WXMSW__

#if wxUSE_GRAPHICS_CONTEXT
    virtual wxGraphicsContext* GetGraphicsContext() const
    {
        return m_pimpl->GetGraphicsContext();
    }
    virtual void SetGraphicsContext( std::unique_ptr<wxGraphicsContext> ctx )
    {
        m_pimpl->SetGraphicsContext(std::move(ctx));
    }
#endif

protected:
    wxDC(std::unique_ptr<wxDCImpl> pimpl) : m_pimpl(std::move(pimpl)) { }

    std::unique_ptr<wxDCImpl> m_pimpl;

    void SetWindow(wxWindow* w)
        { return m_pimpl->SetWindow(w); }

private:
    wxDECLARE_ABSTRACT_CLASS(wxDC);
};

// ----------------------------------------------------------------------------
// helper class: you can use it to temporarily change the DC text colour and
// restore it automatically when the object goes out of scope
// ----------------------------------------------------------------------------

class wxDCTextColourChanger
{
public:
    wxDCTextColourChanger(wxDC& dc) : m_dc(dc) { }

    wxDCTextColourChanger(wxDC& dc, const wxColour& col) : m_dc(dc)
    {
        Set(col);
    }

    ~wxDCTextColourChanger()
    {
        if ( m_colFgOld.IsOk() )
            m_dc.SetTextForeground(m_colFgOld);
    }

    void Set(const wxColour& col)
    {
        if ( !m_colFgOld.IsOk() )
            m_colFgOld = m_dc.GetTextForeground();
        m_dc.SetTextForeground(col);
    }

private:
    wxDC& m_dc;

    wxColour m_colFgOld;
};

// ----------------------------------------------------------------------------
// helper class: you can use it to temporarily change the DC text background colour and
// restore it automatically when the object goes out of scope
// ----------------------------------------------------------------------------

class wxDCTextBgColourChanger
{
public:
    wxDCTextBgColourChanger(wxDC& dc) : m_dc(dc) { }

    wxDCTextBgColourChanger(wxDC& dc, const wxColour& col) : m_dc(dc)
    {
        Set(col);
    }

    ~wxDCTextBgColourChanger()
    {
        if ( m_colBgOld.IsOk() )
            m_dc.SetTextBackground(m_colBgOld);
    }

    void Set(const wxColour& col)
    {
        if ( !m_colBgOld.IsOk() )
            m_colBgOld = m_dc.GetTextBackground();
        m_dc.SetTextBackground(col);
    }

private:
    wxDC& m_dc;

    wxColour m_colBgOld;
};

// ----------------------------------------------------------------------------
// helper class: you can use it to temporarily change the DC text background mode and
// restore it automatically when the object goes out of scope
// ----------------------------------------------------------------------------

class wxDCTextBgModeChanger
{
public:
    wxDCTextBgModeChanger(wxDC& dc) : m_dc(dc), m_modeOld(wxBrushStyle::Invalid) { }

    wxDCTextBgModeChanger(wxDC& dc, wxBrushStyle mode) : m_dc(dc)
    {
        Set(mode);
    }

    ~wxDCTextBgModeChanger()
    {
        if ( m_modeOld != wxBrushStyle::Invalid )
            m_dc.SetBackgroundMode(m_modeOld);
    }

    void Set(wxBrushStyle mode)
    {
        if ( m_modeOld == wxBrushStyle::Invalid )
            m_modeOld = m_dc.GetBackgroundMode();
        m_dc.SetBackgroundMode(mode);
    }

private:
    wxDC& m_dc;

    wxBrushStyle m_modeOld;
};

// ----------------------------------------------------------------------------
// helper class: you can use it to temporarily change the DC pen and
// restore it automatically when the object goes out of scope
// ----------------------------------------------------------------------------

class wxDCPenChanger
{
public:
    wxDCPenChanger(wxDC& dc, const wxPen& pen) : m_dc(dc), m_penOld(dc.GetPen())
    {
        m_dc.SetPen(pen);
    }

    ~wxDCPenChanger()
    {
        if ( m_penOld.IsOk() )
            m_dc.SetPen(m_penOld);
    }

private:
    wxDC& m_dc;

    wxPen m_penOld;
};

// ----------------------------------------------------------------------------
// helper class: you can use it to temporarily change the DC brush and
// restore it automatically when the object goes out of scope
// ----------------------------------------------------------------------------

class wxDCBrushChanger
{
public:
    wxDCBrushChanger(wxDC& dc, const wxBrush& brush) : m_dc(dc), m_brushOld(dc.GetBrush())
    {
        m_dc.SetBrush(brush);
    }

    ~wxDCBrushChanger()
    {
        if ( m_brushOld.IsOk() )
            m_dc.SetBrush(m_brushOld);
    }

private:
    wxDC& m_dc;

    wxBrush m_brushOld;
};

// ----------------------------------------------------------------------------
// another small helper class: sets the clipping region in its ctor and
// destroys it in the dtor
// ----------------------------------------------------------------------------

class wxDCClipper
{
public:
    wxDCClipper(wxDC& dc, const wxRect& r)
        : m_dc(dc)
    {
        m_restoreOld = m_dc.GetClippingBox(m_oldClipRect);
        m_dc.SetClippingRegion(r);
    }
    wxDCClipper(wxDC& dc, const wxRegion& r)
        : wxDCClipper{dc, r.GetBox()}
    {
    }
    wxDCClipper(wxDC& dc, wxCoord x, wxCoord y, wxCoord w, wxCoord h)
        : wxDCClipper{dc, wxRect{x, y, w, h}}
    {
    }

    ~wxDCClipper()
    {
        m_dc.DestroyClippingRegion();
        if ( m_restoreOld )
            m_dc.SetClippingRegion(m_oldClipRect);
    }

private:
    wxDC& m_dc;
    wxRect m_oldClipRect;
    bool m_restoreOld;
};

// ----------------------------------------------------------------------------
// helper class: you can use it to temporarily change the DC font and
// restore it automatically when the object goes out of scope
// ----------------------------------------------------------------------------

class wxDCFontChanger
{
public:
    wxDCFontChanger(wxDC& dc)
        : m_dc(dc) 
    {
    }

    wxDCFontChanger(wxDC& dc, const wxFont& font)
        : m_dc(dc), m_fontOld(dc.GetFont())
    {
        m_dc.SetFont(font);
    }

    void Set(const wxFont& font)
    {
        if ( !m_fontOld.IsOk() )
            m_fontOld = m_dc.GetFont();
        m_dc.SetFont(font);
    }

    ~wxDCFontChanger()
    {
        if ( m_fontOld.IsOk() )
            m_dc.SetFont(m_fontOld);
    }

private:
    wxDC& m_dc;

    wxFont m_fontOld;
};


#endif // _WX_DC_H_BASE_
