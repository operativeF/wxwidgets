/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dcsvg.h
// Purpose:     wxSVGFileDC
// Author:      Chris Elliott
// Modified by:
// Created:
// Copyright:   (c) Chris Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCSVG_H_
#define _WX_DCSVG_H_

#if wxUSE_SVG

#include "wx/string.h"
#include "wx/filename.h"
#include "wx/dc.h"

#include <memory>

inline constexpr wxChar wxSVGVersion[] = wxT("v0101");

enum wxSVGShapeRenderingMode
{
    wxSVG_SHAPE_RENDERING_AUTO = 0,
    wxSVG_SHAPE_RENDERING_OPTIMIZE_SPEED,
    wxSVG_SHAPE_RENDERING_CRISP_EDGES,
    wxSVG_SHAPE_RENDERING_GEOMETRIC_PRECISION,

    wxSVG_SHAPE_RENDERING_OPTIMISE_SPEED = wxSVG_SHAPE_RENDERING_OPTIMIZE_SPEED
};

class WXDLLIMPEXP_FWD_BASE wxFileOutputStream;
class WXDLLIMPEXP_FWD_CORE wxSVGFileDC;

// Base class for bitmap handlers used by wxSVGFileDC, used by the standard
// "embed" and "link" handlers below but can also be used to create a custom
// handler.
class WXDLLIMPEXP_CORE wxSVGBitmapHandler
{
public:
    // Write the representation of the given bitmap, appearing at the specified
    // position, to the provided stream.
    virtual bool ProcessBitmap(const wxBitmap& bitmap,
                               wxCoord x, wxCoord y,
                               wxOutputStream& stream) const = 0;

    virtual ~wxSVGBitmapHandler() = default;
};

// Predefined standard bitmap handler: creates a file, stores the bitmap in
// this file and uses the file name in the generated SVG.
class WXDLLIMPEXP_CORE wxSVGBitmapFileHandler : public wxSVGBitmapHandler
{
public:
    wxSVGBitmapFileHandler() = default;

    explicit wxSVGBitmapFileHandler(const wxFileName& path)
        : m_path(path)
    {
    }

    bool ProcessBitmap(const wxBitmap& bitmap,
                               wxCoord x, wxCoord y,
                               wxOutputStream& stream) const override;

private:
    wxFileName m_path; // When set, name will be appended with _image#.png
};

// Predefined handler which embeds the bitmap (base64-encoding it) inside the
// generated SVG file.
class WXDLLIMPEXP_CORE wxSVGBitmapEmbedHandler : public wxSVGBitmapHandler
{
public:
    bool ProcessBitmap(const wxBitmap& bitmap,
                               wxCoord x, wxCoord y,
                               wxOutputStream& stream) const override;
};

class WXDLLIMPEXP_CORE wxSVGFileDCImpl : public wxDCImpl
{
public:
    wxSVGFileDCImpl(wxSVGFileDC* owner,
                    const std::string& filename,
                    int width = 320,
                    int height = 240,
                    double dpi = 72.0,
                    const std::string& title = {});

    ~wxSVGFileDCImpl();

    wxSVGFileDCImpl(const wxSVGFileDCImpl&) = delete;
    wxSVGFileDCImpl& operator=(const wxSVGFileDCImpl&) = delete;
    wxSVGFileDCImpl(wxSVGFileDCImpl&&) = default;
    wxSVGFileDCImpl& operator=(wxSVGFileDCImpl&&) = default;

    bool IsOk() const override { return m_OK; }

    bool CanDrawBitmap() const override { return true; }
    bool CanGetTextExtent() const override { return true; }

    int GetDepth() const override
    {
        wxFAIL_MSG(wxT("wxSVGFILEDC::GetDepth Call not implemented"));
        return -1;
    }

    void Clear() override;

    void DestroyClippingRegion() override;

    wxCoord GetCharHeight() const override;
    wxCoord GetCharWidth() const override;

#if wxUSE_PALETTE
    void SetPalette(const wxPalette& WXUNUSED(palette)) override
    {
        wxFAIL_MSG(wxT("wxSVGFILEDC::SetPalette not implemented"));
    }
#endif

    void SetLogicalFunction(wxRasterOperationMode WXUNUSED(function)) override
    {
        wxFAIL_MSG(wxT("wxSVGFILEDC::SetLogicalFunction Call not implemented"));
    }

    wxRasterOperationMode GetLogicalFunction() const override
    {
        wxFAIL_MSG(wxT("wxSVGFILEDC::GetLogicalFunction() not implemented"));
        return wxRasterOperationMode::Copy;
    }

    void SetLogicalOrigin(wxPoint logicalOrigin) override
    {
        wxDCImpl::SetLogicalOrigin(logicalOrigin);
        m_graphics_changed = true;
    }

    void SetDeviceOrigin(wxPoint deviceOrigin) override
    {
        wxDCImpl::SetDeviceOrigin(deviceOrigin);
        m_graphics_changed = true;
    }

    void SetAxisOrientation(bool xLeftRight, bool yBottomUp) override
    {
        wxDCImpl::SetAxisOrientation(xLeftRight, yBottomUp);
        m_graphics_changed = true;
    }

    void SetBackground(const wxBrush& brush) override;
    void SetBackgroundMode(wxBrushStyle mode) override;
    void SetBrush(const wxBrush& brush) override;
    void SetFont(const wxFont& font) override;
    void SetPen(const wxPen& pen) override;

    void* GetHandle() const override { return nullptr; }

    void SetBitmapHandler(wxSVGBitmapHandler* handler);

    void SetShapeRenderingMode(wxSVGShapeRenderingMode renderingMode);

private:
    bool DoGetPixel(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                            wxColour* WXUNUSED(col)) const override
    {
        wxFAIL_MSG(wxT("wxSVGFILEDC::DoGetPixel Call not implemented"));
        return true;
    }

    bool DoBlit(wxCoord xdest, wxCoord ydest,
                        wxCoord width, wxCoord height,
                        wxDC* source,
                        wxCoord xsrc, wxCoord ysrc,
                        wxRasterOperationMode rop,
                        bool useMask = false,
                        wxCoord xsrcMask = wxDefaultCoord,
                        wxCoord ysrcMask = wxDefaultCoord) override;

    void DoCrossHair(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y)) override
    {
        wxFAIL_MSG(wxT("wxSVGFILEDC::CrossHair Call not implemented"));
    }

    void DoDrawArc(wxCoord x1, wxCoord y1,
                           wxCoord x2, wxCoord y2,
                           wxCoord xc, wxCoord yc) override;

    void DoDrawBitmap(const wxBitmap& bmp, wxCoord x, wxCoord y,
                              bool useMask = false) override;

    void DoDrawEllipse(wxCoord x, wxCoord y,
                               wxCoord width, wxCoord height) override;

    void DoDrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
                                   double sa, double ea) override;

    void DoDrawIcon(const wxIcon& icon, wxCoord x, wxCoord y) override;

    void DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2) override;

    void DoDrawLines(int n, const wxPoint points[],
                             wxCoord xoffset, wxCoord yoffset) override;

    void DoDrawPoint(wxCoord x, wxCoord y) override;

    void DoDrawPolygon(int n, const wxPoint points[],
                               wxCoord xoffset, wxCoord yoffset,
                               wxPolygonFillMode fillStyle = wxODDEVEN_RULE) override;

    void DoDrawPolyPolygon(int n, const int count[], const wxPoint points[],
                                   wxCoord xoffset, wxCoord yoffset,
                                   wxPolygonFillMode fillStyle) override;

    void DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;

    void DoDrawRotatedText(std::string_view text, wxCoord x, wxCoord y,
                                   double angle) override;

    void DoDrawRoundedRectangle(wxCoord x, wxCoord y,
                                        wxCoord width, wxCoord height,
                                        double radius) override;

    void DoDrawText(std::string_view text, wxCoord x, wxCoord y) override;

    bool DoFloodFill(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                             const wxColour& WXUNUSED(col),
                             wxFloodFillStyle WXUNUSED(style)) override
    {
        wxFAIL_MSG(wxT("wxSVGFILEDC::DoFloodFill Call not implemented"));
        return false;
    }

    void DoGradientFillLinear(const wxRect& rect,
                                      const wxColour& initialColour,
                                      const wxColour& destColour,
                                      wxDirection nDirection) override;

    void DoGradientFillConcentric(const wxRect& rect,
                                          const wxColour& initialColour,
                                          const wxColour& destColour,
                                          const wxPoint& circleCenter) override;

    wxSize DoGetSize() const override
    {
        return {m_width, m_height};
    }

    wxSize DoGetTextExtent(std::string_view string,
                                 wxCoord* descent = nullptr,
                                 wxCoord* externalLeading = nullptr,
                                 const wxFont* theFont = nullptr) const override;

    void DoSetDeviceClippingRegion(const wxRegion& region) override
    {
        DoSetClippingRegion(region.GetBox().x, region.GetBox().y,
                            region.GetBox().width, region.GetBox().height);
    }

    void DoSetClippingRegion(wxCoord x, wxCoord y,
                                     wxCoord w, wxCoord h) override;

    void DoGetSizeMM(int* width, int* height) const override;

    wxSize GetPPI() const override;

    void Init(const std::string& filename, int width, int height,
              double dpi, const std::string& title);

    void write(const wxString& s);

private:
    // If m_graphics_changed is true, close the current <g> element and start a
    // new one for the last pen/brush change.
    void NewGraphicsIfNeeded();

    // Open a new graphics group setting up all the attributes according to
    // their current values in wxDC.
    void DoStartNewGraphics();

    std::string         m_filename;
    bool                m_OK;
    bool                m_graphics_changed;  // set by Set{Brush,Pen}()
    int                 m_width;
    int                 m_height;
    double              m_dpi;
    std::unique_ptr<wxFileOutputStream> m_outfile;
    std::unique_ptr<wxSVGBitmapHandler> m_bmp_handler; // class to handle bitmaps
    wxSVGShapeRenderingMode m_renderingMode;

    // The clipping nesting level is incremented by every call to
    // SetClippingRegion() and reset when DestroyClippingRegion() is called.
    size_t m_clipNestingLevel;

    // Unique ID for every clipping graphics group: this is simply always
    // incremented in each SetClippingRegion() call.
    size_t m_clipUniqueId;

    // Unique ID for every gradient.
    size_t m_gradientUniqueId;

    wxDECLARE_ABSTRACT_CLASS(wxSVGFileDCImpl);
};


class WXDLLIMPEXP_CORE wxSVGFileDC : public wxDC
{
public:
    wxSVGFileDC(const std::string& filename,
                int width = 320,
                int height = 240,
                double dpi = 72.0,
                const std::string& title = {})
        : wxDC(new wxSVGFileDCImpl(this, filename, width, height, dpi, title))
    {
    }

    // wxSVGFileDC-specific methods:

    // Use a custom bitmap handler: takes ownership of the handler.
    void SetBitmapHandler(wxSVGBitmapHandler* handler);

    void SetShapeRenderingMode(wxSVGShapeRenderingMode renderingMode);

private:
    wxDECLARE_ABSTRACT_CLASS(wxSVGFileDC);
};

#endif // wxUSE_SVG

#endif // _WX_DCSVG_H_
