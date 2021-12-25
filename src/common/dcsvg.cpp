/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/svg.cpp
// Purpose:     SVG sample
// Author:      Chris Elliott
// Modified by:
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_SVG

#include "wx/dcmemory.h"
#include "wx/dcscreen.h"
#include "wx/icon.h"
#include "wx/dcsvg.h"
#include "wx/scopedarray.h"

#if wxUSE_MARKUP
    #include "wx/private/markupparser.h"
#endif

import WX.Cmn.Base64;

import WX.Image;
import WX.Cfg.Flags;
import WX.Cmn.WFStream;
import WX.Cmn.MemStream;
import WX.File.Filename;

import Utils.Strings;

import <cmath>;
import <numbers>;

// ----------------------------------------------------------
// Global utilities
// ----------------------------------------------------------

namespace
{

// This function returns a string representation of a floating point number in
// C locale (i.e. always using "." for the decimal separator) and with the
// fixed precision (which is 2 for some unknown reason but this is what it was
// in this code originally).
inline wxString NumStr(double f)
{
    // Handle this case specially to avoid generating "-0.00" in the output.
    if ( f == 0 )
    {
        return "0.00";
    }

    return wxString::FromCDouble(f, 2);
}

inline wxString NumStr(float f)
{
    return NumStr(double(f));
}

// Return the colour representation as HTML-like "#rrggbb" string and also
// returns its alpha as opacity number in 0..1 range.
wxString Col2SVG(wxColour c, float* opacity = nullptr)
{
    if ( c.Alpha() != wxALPHA_OPAQUE )
    {
        if ( opacity )
            *opacity = c.Alpha() / 255.0f;

        // Remove the alpha before using GetAsString(wxC2S_HTML_SYNTAX) as it
        // doesn't support colours with alpha channel.
        c = wxColour(c.GetRGB());
    }
    else // No alpha.
    {
        if ( opacity )
            *opacity = 1.0f;
    }

    return c.GetAsString(wxC2S_HTML_SYNTAX);
}

wxString GetPenStroke(const wxColour& c, wxPenStyle style = wxPenStyle::Solid)
{
    float opacity;
    wxString s = "stroke:" + Col2SVG(c, &opacity) + ";";

    switch ( style )
    {
        case wxPenStyle::Solid:
        case wxPenStyle::Dot:
        case wxPenStyle::ShortDash:
        case wxPenStyle::LongDash:
        case wxPenStyle::DotDash:
        case wxPenStyle::UserDash:
            s += wxString::Format(" stroke-opacity:%s;", NumStr(opacity));
            break;
        case wxPenStyle::Transparent:
            s += " stroke-opacity:0.0;";
            break;
        default:
            wxASSERT_MSG(false, "wxSVGFileDC::Requested Pen Style not available");
            break;
    }

    return s;
}

wxString GetBrushFill(const wxColour& c, wxBrushStyle style = wxBrushStyle::Solid)
{
    float opacity;
    wxString s = "fill:" + Col2SVG(c, &opacity) + ";";

    switch ( style )
    {
        case wxBrushStyle::Solid:
        case wxBrushStyle::BDiagonalHatch:
        case wxBrushStyle::FDiagonalHatch:
        case wxBrushStyle::CrossDiagHatch:
        case wxBrushStyle::CrossHatch:
        case wxBrushStyle::VerticalHatch:
        case wxBrushStyle::HorizontalHatch:
            s += wxString::Format(" fill-opacity:%s;", NumStr(opacity));
            break;
        case wxBrushStyle::Transparent:
            s += " fill-opacity:0.0;";
            break;
        default:
            wxASSERT_MSG(false, "wxSVGFileDC::Requested Brush Style not available");
            break;
    }

    return s;
}

wxString GetPenPattern(const wxPen& pen)
{
    wxString s;

    // The length of the dashes and gaps have a constant factor.
    // Dots have a width of 2, short dashes 10, long dashes 15 and gaps 8 (5 for dots).
    // When the pen width increases, lines become thicker and unrecognizable.
    // Multiplying with 1/3th of the width creates line styles matching the appearance of wxDC.
    // The pen width is not used to modify user provided dash styles.
    double w = pen.GetWidth();
    if (pen.GetWidth() == 0)
        w = 1;
    w = w / 3;

    switch (pen.GetStyle())
    {
        case wxPenStyle::Dot:
            s = wxString::Format("stroke-dasharray=\"%f,%f\"", w * 2, w * 5);
            break;
        case wxPenStyle::ShortDash:
            s = wxString::Format("stroke-dasharray=\"%f,%f\"", w * 10, w * 8);
            break;
        case wxPenStyle::LongDash:
            s = wxString::Format("stroke-dasharray=\"%f,%f\"", w * 15, w * 8);
            break;
        case wxPenStyle::DotDash:
            s = wxString::Format("stroke-dasharray=\"%f,%f,%f,%f\"", w * 8, w * 8, w * 2, w * 8);
            break;
        case wxPenStyle::UserDash:
        {
            s = "stroke-dasharray=\"";
            // TODO: Return from GetDashes
            wxDash* dashes{nullptr};
            int count = pen.GetDashes(&dashes);
            if ((dashes != nullptr) && (count > 0))
            {
                for (int i = 0; i < count; ++i)
                {
                    s << dashes[i];
                    if (i < count - 1)
                        s << ",";
                }
            }
            s += "\"";
            break;
        }
        case wxPenStyle::StippleMaskOpaque:
        case wxPenStyle::StippleMask:
        case wxPenStyle::Stipple:
        case wxPenStyle::BDiagonalHatch:
        case wxPenStyle::CrossDiagHatch:
        case wxPenStyle::FDiagonalHatch:
        case wxPenStyle::CrossHatch:
        case wxPenStyle::HorizontalHatch:
        case wxPenStyle::VerticalHatch:
            wxASSERT_MSG(false, "wxSVGFileDC::Requested Pen Pattern not available");
            break;
        case wxPenStyle::Solid:
        case wxPenStyle::Transparent:
        case wxPenStyle::Invalid:
            // these penstyles do not need a pattern.
            break;
    }
    return s;
}

wxString GetPenStyle(const wxPen& pen)
{
    wxString penStyle;

    penStyle += wxString::Format("stroke-width:%d;", pen.GetWidth());

    switch (pen.GetCap())
    {
        case wxCAP_PROJECTING:
            penStyle += " stroke-linecap:square;";
            break;
        case wxCAP_BUTT:
            penStyle += " stroke-linecap:butt;";
            break;
        case wxCAP_ROUND:
        default:
            penStyle += " stroke-linecap:round;";
            break;
    }

    switch (pen.GetJoin())
    {
        case wxJOIN_BEVEL:
            penStyle += " stroke-linejoin:bevel;";
            break;
        case wxJOIN_MITER:
            penStyle += " stroke-linejoin:miter;";
            break;
        case wxJOIN_ROUND:
        default:
            penStyle += " stroke-linejoin:round;";
            break;
    }

    return penStyle;
}

wxString GetBrushStyleName(const wxBrush& brush)
{
    wxString brushStyle;

    switch (brush.GetStyle())
    {
        case wxBrushStyle::BDiagonalHatch:
            brushStyle = "BdiagonalHatch";
            break;
        case wxBrushStyle::FDiagonalHatch:
            brushStyle = "FdiagonalHatch";
            break;
        case wxBrushStyle::CrossDiagHatch:
            brushStyle = "CrossDiagHatch";
            break;
        case wxBrushStyle::CrossHatch:
            brushStyle = "CrossHatch";
            break;
        case wxBrushStyle::VerticalHatch:
            brushStyle = "VerticalHatch";
            break;
        case wxBrushStyle::HorizontalHatch:
            brushStyle = "HorizontalHatch";
            break;
        case wxBrushStyle::StippleMaskOpaque:
        case wxBrushStyle::StippleMask:
        case wxBrushStyle::Stipple:
            wxASSERT_MSG(false, "wxSVGFileDC::Requested Brush Fill not available");
            break;
        case wxBrushStyle::Solid:
        case wxBrushStyle::Transparent:
        case wxBrushStyle::Invalid:
            // these brushstyles do not need a fill.
            break;
    }

    if (!brushStyle.empty())
        brushStyle += wxString::Format("%s%02X", Col2SVG(brush.GetColour()).substr(1), brush.GetColour().Alpha());

    return brushStyle;
}

wxString GetBrushPattern(const wxBrush& brush)
{
    wxString s;
    wxString brushStyle = GetBrushStyleName(brush);

    if (!brushStyle.empty())
        s = wxString::Format("fill=\"url(#%s)\"", brushStyle);

    return s;
}

wxString GetRenderMode(const wxSVGShapeRenderingMode style)
{
    wxString mode;
    switch (style)
    {
        case wxSVGShapeRenderingMode::OptimizeSpeed:
            mode = "optimizeSpeed";
            break;
        case wxSVGShapeRenderingMode::CrispEdges:
            mode = "crispEdges";
            break;
        case wxSVGShapeRenderingMode::GeometricPrecision:
            mode = "geometricPrecision";
            break;
        case wxSVGShapeRenderingMode::Auto:
            mode = "auto";
            break;
    }

    wxString s = wxString::Format("shape-rendering=\"%s\"", mode);
    return s;
}

wxString CreateBrushFill(const wxBrush& brush, wxSVGShapeRenderingMode mode)
{
    wxString s;
    wxString patternName = GetBrushStyleName(brush);

    if (!patternName.empty())
    {
        wxString pattern;
        switch (brush.GetStyle())
        {
            case wxBrushStyle::BDiagonalHatch:
                pattern = "d=\"M-1,1 l2,-2 M0,8 l8,-8 M7,9 l2,-2\"";
                break;
            case wxBrushStyle::FDiagonalHatch:
                pattern = "d=\"M7,-1 l2,2 M0,0 l8,8 M-1,7 l2,2\"";
                break;
            case wxBrushStyle::CrossDiagHatch:
                pattern = "d=\"M7,-1 l2,2 M0,0 l8,8 M-1,7 l2,2 M-1,1 l2,-2 M0,8 l8,-8 M7,9 l2,-2\"";
                break;
            case wxBrushStyle::CrossHatch:
                pattern = "d=\"M4,0 l0,8 M0,4 l8,0\"";
                break;
            case wxBrushStyle::VerticalHatch:
                pattern = "d=\"M4,0 l0,8\"";
                break;
            case wxBrushStyle::HorizontalHatch:
                pattern = "d=\"M0,4 l8,0\"";
                break;
            default:
                break;
        }

        float opacity;
        wxString brushColourStr = Col2SVG(brush.GetColour(), &opacity);
        wxString brushStrokeStr = "stroke-width:1; stroke-linecap:round; stroke-linejoin:round;";

        s += wxString::Format("  <pattern id=\"%s\" patternUnits=\"userSpaceOnUse\" width=\"8\" height=\"8\">\n",
            patternName);
        s += wxString::Format("    <path style=\"stroke:%s; stroke-opacity:%s; %s\" %s %s/>\n",
            brushColourStr, NumStr(opacity), brushStrokeStr, pattern, GetRenderMode(mode));
        s += "  </pattern>\n";
    }

    return s;
}

void SetScaledScreenDCFont(wxScreenDC& sDC, const wxFont& font)
{
    // When using DPI-independent pixels, the results of GetTextExtent() and
    // similar don't depend on DPI anyhow.
#ifndef wxHAVE_DPI_INDEPENDENT_PIXELS
    static constexpr int SVG_DPI = 96;

    const double screenDPI = sDC.GetPPI().y;
    const double scale = screenDPI / SVG_DPI;
    if ( scale > 1 )
    {
        // wxScreenDC uses the DPI of the main screen to determine the text
        // extent and character width/height. Because the SVG should be
        // DPI-independent we want the text extent of the default (96) DPI.
        //
        // We can't just divide the returned sizes by the scale factor, because
        // text does not scale linear (at least on Windows). Therefore, we scale
        // the font size instead.
        wxFont scaledFont = font;
        scaledFont.SetFractionalPointSize(scaledFont.GetFractionalPointSize() / scale);
        sDC.SetFont(scaledFont);
    }
    else
#endif // !wxHAVE_DPI_INDEPENDENT_PIXELS
    {
        sDC.SetFont(font);
    }
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// wxSVGBitmapEmbedHandler
// ----------------------------------------------------------------------------

bool
wxSVGBitmapEmbedHandler::ProcessBitmap(const wxBitmap& bmp,
                                       wxCoord x, wxCoord y,
                                       wxOutputStream& stream) const
{
    static int sub_images = 0;

    if ( wxImage::FindHandler(wxBitmapType::PNG) == nullptr )
        wxImage::AddHandler(new wxPNGHandler);

    // write the bitmap as a PNG to a memory stream and Base64 encode
    wxMemoryOutputStream mem;
    bmp.ConvertToImage().SaveFile(mem, wxBitmapType::PNG);
    wxString data = wxBase64Encode(mem.GetOutputStreamBuffer()->GetBufferStart(),
                                   mem.GetSize());

    // write image meta information
    wxString s;
    s += wxString::Format(R"(  <image x="%d" y="%d" width="%dpx" height="%dpx")",
                          x, y, bmp.GetWidth(), bmp.GetHeight());
    s += wxString::Format(" id=\"image%d\" "
                          "xlink:href=\"data:image/png;base64,\n",
                          sub_images++);

    // Wrap Base64 encoded data on 76 columns boundary (same as Inkscape).
    static constexpr unsigned WRAP = 76;
    for ( size_t i = 0; i < data.size(); i += WRAP )
    {
        if (i < data.size() - WRAP)
            s += data.Mid(i, WRAP) + "\n";
        else
            s += data.Mid(i, s.size() - i) + "\"\n  />\n"; // last line
    }

    // write to the SVG file
    const wxCharBuffer buf = s.utf8_str();
    stream.Write(buf, strlen((const char*)buf));

    return stream.IsOk();
}

// ----------------------------------------------------------
// wxSVGBitmapFileHandler
// ----------------------------------------------------------

bool
wxSVGBitmapFileHandler::ProcessBitmap(const wxBitmap& bmp,
                                      wxCoord x, wxCoord y,
                                      wxOutputStream& stream) const
{
    static int sub_images = 0;

    if ( wxImage::FindHandler(wxBitmapType::PNG) == nullptr )
        wxImage::AddHandler(new wxPNGHandler);

    // find a suitable file name
    wxFileName sPNG = m_path;
    do
    {
        sPNG.SetFullName(wxString::Format("%s%simage%d.png",
                         sPNG.GetName(),
                         sPNG.GetName().empty() ? "" : "_",
                         sub_images++));
    }
    while ( sPNG.FileExists() );

    if ( !bmp.SaveFile(sPNG.GetFullPath(), wxBitmapType::PNG) )
        return false;

    // reference the bitmap from the SVG doc
    wxString s;
    s += wxString::Format(R"(  <image x="%d" y="%d" width="%dpx" height="%dpx")",
                          x, y, bmp.GetWidth(), bmp.GetHeight());
    s += wxString::Format(" xlink:href=\"%s\"/>\n", sPNG.GetFullName());

    // write to the SVG file
    const wxCharBuffer buf = s.utf8_str();
    stream.Write(buf, strlen((const char*)buf));

    return stream.IsOk();
}

// ----------------------------------------------------------
// wxSVGFileDC (specialisations)
// ----------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxSVGFileDC, wxDC);

void wxSVGFileDC::SetBitmapHandler(std::unique_ptr<wxSVGBitmapHandler> handler)
{
    ((wxSVGFileDCImpl*)GetImpl())->SetBitmapHandler(std::move(handler));
}

void wxSVGFileDC::SetShapeRenderingMode(wxSVGShapeRenderingMode renderingMode)
{
    ((wxSVGFileDCImpl*)GetImpl())->SetShapeRenderingMode(renderingMode);
}

// ----------------------------------------------------------
// wxSVGFileDCImpl
// ----------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxSVGFileDCImpl, wxDCImpl);

wxSVGFileDCImpl::wxSVGFileDCImpl(wxSVGFileDC* owner,
                const std::string& filename,
                wxSize dimen,
                double dpi,
                const std::string& title)
    : wxDCImpl{owner},
      m_width{dimen.x},
      m_height{dimen.y},
      m_dpi{dpi}
{
    m_mm_to_pix_x = dpi / 25.4;
    m_mm_to_pix_y = dpi / 25.4;

    m_backgroundBrush = *wxTRANSPARENT_BRUSH;
    m_textForegroundColour = *wxBLACK;
    m_textBackgroundColour = *wxWHITE;

    m_pen = *wxBLACK_PEN;
    m_font = *wxNORMAL_FONT;
    m_brush = *wxWHITE_BRUSH;

    m_filename = filename;
    // FIXME: Empty filename?
    m_outfile = std::make_unique<wxFileOutputStream>(m_filename);

    wxString s;
    s += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    s += "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n\n";
    s += "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"";
    s += wxString::Format(" width=\"%scm\" height=\"%scm\" viewBox=\"0 0 %d %d\">\n", NumStr(m_width / dpi * 2.54), NumStr(m_height / dpi * 2.54), m_width, m_height);
    s += wxString::Format("<title>%s</title>\n", title);
    s += wxString("<desc>Picture generated by wxSVG ") + wxSVGVersion + "</desc>\n\n";
    s += "<g style=\"fill:black; stroke:black; stroke-width:1\">\n";
    write(s);
}

wxSVGFileDCImpl::~wxSVGFileDCImpl()
{
    wxString s;

    // Close remaining clipping group elements
    for (size_t i = 0; i < m_clipUniqueId; i++)
        s += "</g>\n";

    s += "</g>\n</svg>\n";
    write(s);
}

void wxSVGFileDCImpl::DoGetSizeMM(int* width, int* height) const
{
    if (width)
        *width = std::lround( (double)m_width / GetMMToPXx() );

    if (height)
        *height = std::lround( (double)m_height / GetMMToPXy() );
}

wxSize wxSVGFileDCImpl::GetPPI() const
{
    return {std::lround(m_dpi), std::lround(m_dpi)};
}

void wxSVGFileDCImpl::Clear()
{
    {
        wxDCBrushChanger setBackground(*GetOwner(), m_backgroundBrush);
        wxDCPenChanger setTransp(*GetOwner(), *wxTRANSPARENT_PEN);
        DoDrawRectangle(0, 0, m_width, m_height);
    }

    NewGraphicsIfNeeded();
}

void wxSVGFileDCImpl::DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2)
{
    NewGraphicsIfNeeded();

    wxString s;
    s = wxString::Format("  <path d=\"M%d %d L%d %d\" %s %s/>\n",
        x1, y1, x2, y2, GetRenderMode(m_renderingMode), GetPenPattern(m_pen));

    write(s);

    CalcBoundingBox(x1, y1);
    CalcBoundingBox(x2, y2);
}

void wxSVGFileDCImpl::DoDrawLines(int n, const wxPoint points[], wxCoord xoffset, wxCoord yoffset)
{
    if (n > 1)
    {
        NewGraphicsIfNeeded();
        wxString s;

        s = wxString::Format("  <path d=\"M%d %d",
            (points[0].x + xoffset), (points[0].y + yoffset));

        CalcBoundingBox(points[0].x + xoffset, points[0].y + yoffset);

        for (int i = 1; i < n; ++i)
        {
            s += wxString::Format(" L%d %d", (points[i].x + xoffset), (points[i].y + yoffset));
            CalcBoundingBox(points[i].x + xoffset, points[i].y + yoffset);
        }

        s += wxString::Format("\" style=\"fill:none\" %s %s/>\n",
            GetRenderMode(m_renderingMode), GetPenPattern(m_pen));

        write(s);
    }
}

void wxSVGFileDCImpl::DoDrawPoint(wxCoord x, wxCoord y)
{
    NewGraphicsIfNeeded();

    wxString s;

    s = "  <g style=\"stroke-width:1; stroke-linecap:round;\">\n  ";
    write(s);

    DoDrawLine(x, y, x, y);

    s = "  </g>\n";
    write(s);
}

void wxSVGFileDCImpl::DoDrawText(std::string_view text, wxPoint pt)
{
    DoDrawRotatedText(text, pt, 0.0);
}

void wxSVGFileDCImpl::DoDrawRotatedText(std::string_view sText, wxPoint pt, double angle)
{
    //known bug; if the font is drawn in a scaled DC, it will not behave exactly as wxMSW
    NewGraphicsIfNeeded();
    wxString s;

    // Get extent of whole text.
    wxCoord w, h, heightLine;
    GetOwner()->GetMultiLineTextExtent(sText, &w, &h, &heightLine);

    // Compute the shift for the origin of the next line.
    const double rad = wxDegToRad(angle);

    // Update bounding box: upper left, upper right, bottom left, bottom right
    CalcBoundingBox(pt.x, pt.y);
    CalcBoundingBox((wxCoord)(pt.x + w * std::cos(rad)), (wxCoord)(pt.y - h * std::sin(rad)));
    CalcBoundingBox((wxCoord)(pt.x + h * std::sin(rad)), (wxCoord)(pt.y + h * std::cos(rad)));
    CalcBoundingBox((wxCoord)(pt.x + h * std::sin(rad) + w * std::cos(rad)), (wxCoord)(pt.y + h * std::cos(rad) - w * std::sin(rad)));

    // Create text style string
    wxString fontstyle;
    switch (m_font.GetStyle())
    {
        case wxFontStyle::Max:
            wxFAIL_MSG("invalid font style value");
            [[fallthrough]];
        case wxFontStyle::Normal:
            fontstyle = "normal";
            break;
        case wxFontStyle::Italic:
            fontstyle = "italic";
            break;
        case wxFontStyle::Slant:
            fontstyle = "oblique";
            break;
    }

    wxString textDecoration;
    if (m_font.GetUnderlined())
        textDecoration += " underline";
    if (m_font.GetStrikethrough())
        textDecoration += " line-through";
    if (textDecoration.empty())
        textDecoration = " none";

    wxString style = "style=\"";
    style += wxString::Format("font-family:%s; ", m_font.GetFaceName());
    style += wxString::Format("font-weight:%d; ", m_font.GetWeight());
    style += wxString::Format("font-style:%s; ", fontstyle);
    style += wxString::Format("font-size:%spt; ", NumStr(m_font.GetFractionalPointSize()));
    style += wxString::Format("text-decoration:%s; ", textDecoration);
    style += wxString::Format("%s %s stroke-width:0; ",
                              GetBrushFill(m_textForegroundColour),
                              GetPenStroke(m_textForegroundColour));
    style += "white-space: pre;";
    style += "\"";

    // this is deprecated in favour of "white-space: pre", keep it for now to
    // support SVG viewers that do not support the new tag
    style += " xml:space=\"preserve\"";

    // Draw all text line by line
    const std::vector<std::string_view> lines = wx::unsafe::StrViewSplit(sText, '\n');

    const double dx = heightLine * std::sin(rad);
    const double dy = heightLine * std::cos(rad);

    size_t lineNum{0};
    
    for (auto line : lines)
    {
        const double xRect = pt.x + lineNum * dx;
        const double yRect = pt.y + lineNum * dy;

        // convert x,y to SVG text x,y (the coordinates of the text baseline)
        wxCoord desc;
        const auto textExtents = DoGetTextExtent(line, &desc);
        const double xText = xRect + (textExtents.y - desc) * std::sin(rad);
        const double yText = yRect + (textExtents.y - desc) * std::cos(rad);

        if (m_backgroundMode == wxBrushStyle::Solid)
        {
            // draw text background
            const wxString rectStyle = wxString::Format(
                "style=\"%s %s stroke-width:1;\"",
                GetBrushFill(m_textBackgroundColour),
                GetPenStroke(m_textBackgroundColour));

            const wxString rectTransform = wxString::Format(
                "transform=\"rotate(%s %s %s)\"",
                NumStr(-angle), NumStr(xRect), NumStr(yRect));

            s = wxString::Format(
                "  <rect x=\"%s\" y=\"%s\" width=\"%d\" height=\"%d\" %s %s %s/>\n",
                NumStr(xRect), NumStr(yRect), textExtents.x, textExtents.y,
                GetRenderMode(m_renderingMode), rectStyle, rectTransform);

            write(s);
        }

        const wxString transform = wxString::Format(
            "transform=\"rotate(%s %s %s)\"",
            NumStr(-angle), NumStr(xText), NumStr(yText));

        s = wxString::Format(
            "  <text x=\"%s\" y=\"%s\" textLength=\"%d\" %s %s>%s</text>\n",
            NumStr(xText), NumStr(yText), textExtents.x, style, transform,
#if wxUSE_MARKUP
            wxMarkupParser::Quote(std::string{line})
#else
            std::string{line}
#endif
        );

        write(s);

        ++lineNum;
    }
}

void wxSVGFileDCImpl::DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
    DoDrawRoundedRectangle(x, y, width, height, 0.0);
}

void wxSVGFileDCImpl::DoDrawRoundedRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height, double radius)
{
    NewGraphicsIfNeeded();
    wxString s;

    s = wxString::Format("  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" rx=\"%s\" %s %s %s/>\n",
        x, y, width, height, NumStr(radius),
        GetRenderMode(m_renderingMode), GetPenPattern(m_pen), GetBrushPattern(m_brush));

    write(s);

    CalcBoundingBox(x, y);
    CalcBoundingBox(x + width, y + height);
}

void wxSVGFileDCImpl::DoDrawPolygon(int n, const wxPoint points[],
                                    wxCoord xoffset, wxCoord yoffset,
                                    wxPolygonFillMode fillStyle)
{
    NewGraphicsIfNeeded();

    wxString s;

    s = "  <polygon points=\"";

    for (int i = 0; i < n; i++)
    {
        s += wxString::Format("%d %d ", points[i].x + xoffset, points[i].y + yoffset);
        CalcBoundingBox(points[i].x + xoffset, points[i].y + yoffset);
    }

    s += wxString::Format("\" %s %s %s style=\"fill-rule:%s;\"/>\n",
        GetRenderMode(m_renderingMode), GetPenPattern(m_pen), GetBrushPattern(m_brush),
        fillStyle == wxPolygonFillMode::OddEven ? "evenodd" : "nonzero");

    write(s);
}

void wxSVGFileDCImpl::DoDrawPolyPolygon(int n, const int count[], const wxPoint points[],
                                        wxCoord xoffset, wxCoord yoffset,
                                        wxPolygonFillMode fillStyle)
{
    if (n == 1)
    {
        DoDrawPolygon(count[0], points, xoffset, yoffset, fillStyle);
        return;
    }

    int i, j;
    int totalPts = 0;
    for (j = 0; j < n; ++j)
        totalPts += count[j];

    std::vector<wxPoint> pts(totalPts + n);

    int polyCounter = 0, polyIndex = 0;
    for (i = j = 0; i < totalPts; ++i)
    {
        pts[j++] = points[i];
        ++polyCounter;
        if (polyCounter == count[polyIndex])
        {
            pts[j++] = points[i - count[polyIndex] + 1];
            ++polyIndex;
            polyCounter = 0;
        }
    }

    {
        wxDCPenChanger setTransp(*GetOwner(), *wxTRANSPARENT_PEN);
        DoDrawPolygon(j, pts.data(), xoffset, yoffset, fillStyle);
    }

    for (i = j = 0; i < n; i++)
    {
        DoDrawLines(count[i] + 1, pts.data() + j, xoffset, yoffset);
        j += count[i] + 1;
    }
}

void wxSVGFileDCImpl::DoDrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
    NewGraphicsIfNeeded();

    const double rh = height / 2.0;
    const double rw = width / 2.0;

    wxString s;
    s = wxString::Format("  <ellipse cx=\"%s\" cy=\"%s\" rx=\"%s\" ry=\"%s\" %s %s",
        NumStr(x + rw), NumStr(y + rh), NumStr(rw), NumStr(rh),
        GetRenderMode(m_renderingMode), GetPenPattern(m_pen));
    s += "/>\n";

    write(s);

    CalcBoundingBox(x, y);
    CalcBoundingBox(x + width, y + height);
}

void wxSVGFileDCImpl::DoDrawArc(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2, wxCoord xc, wxCoord yc)
{
    /* Draws an arc of a circle, centred on (xc, yc), with starting point
    (x1, y1) and ending at (x2, y2). The current pen is used for the outline
    and the current brush for filling the shape.

    The arc is drawn in an anticlockwise direction from the start point to
    the end point.

    Might be better described as Pie drawing */

    NewGraphicsIfNeeded();
    wxString s;

    // we need the radius of the circle which has two estimates
    const double r1 = std::hypot(x1 - xc, y1 - yc);
    const double r2 = std::hypot(x2 - xc, y2 - yc);

    wxASSERT_MSG((std::fabs( r2 - r1 ) <= 3), "wxSVGFileDC::DoDrawArc Error in getting radii of circle");
    if ( std::fabs( r2 - r1 ) > 3 )    //pixels
    {
        s = "<!--- wxSVGFileDC::DoDrawArc Error in getting radii of circle -->\n";
        write(s);
    }

    double theta1 = std::atan2((double)(yc - y1), (double)(x1 - xc));
    if (theta1 < 0)
        theta1 = theta1 + std::numbers::pi * 2.0;

    double theta2 = std::atan2((double)(yc - y2), (double)(x2 - xc));
    if (theta2 < 0)
        theta2 = theta2 + std::numbers::pi * 2.0;
    if (theta2 < theta1) theta2 = theta2 + std::numbers::pi * 2;

    int fArc;                  // flag for large or small arc 0 means less than 180 degrees
    if (std::fabs(theta2 - theta1) > std::numbers::pi)
        fArc = 1; else fArc = 0;

    int fSweep = 0;             // flag for sweep always 0

    if (x1 == x2 && y1 == y2)
    {
        // drawing full circle fails with default arc. Draw two half arcs instead.
        s = wxString::Format("  <path d=\"M%d %d a%s %s 0 %d %d %s 0 a%s %s 0 %d %d %s 0",
            x1, y1,
            NumStr(r1), NumStr(r2), fArc, fSweep, NumStr( r1 * 2),
            NumStr(r1), NumStr(r2), fArc, fSweep, NumStr(-r1 * 2));
    }
    else
    {
        // comply to wxDC specs by drawing closing line if brush is not transparent
        wxString line;
        if (GetBrush().GetStyle() != wxBrushStyle::Transparent)
            line = wxString::Format("L%d %d z", xc, yc);

        s = wxString::Format("  <path d=\"M%d %d A%s %s 0 %d %d %d %d %s",
            x1, y1, NumStr(r1), NumStr(r2), fArc, fSweep, x2, y2, line);
    }

    s += wxString::Format("\" %s %s/>\n",
        GetRenderMode(m_renderingMode), GetPenPattern(m_pen));

    write(s);
}

void wxSVGFileDCImpl::DoDrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h, double sa, double ea)
{
    /*
    Draws an arc of an ellipse. The current pen is used for drawing the arc
    and the current brush is used for drawing the pie.

    x and y specify the x and y coordinates of the upper-left corner of the
    rectangle that contains the ellipse.

    width and height specify the width and height of the rectangle that
    contains the ellipse.

    start and end specify the start and end of the arc relative to the
    three-o'clock position from the center of the rectangle. Angles are
    specified in degrees (360 is a complete circle). Positive values mean
    counter-clockwise motion. If start is equal to end, a complete ellipse
    will be drawn. */

    //radius
    const double rx = w / 2.0;
    const double ry = h / 2.0;
    // center
    const double xc = x + rx;
    const double yc = y + ry;

    // start and end coords
    const double xs = xc + rx * std::cos(wxDegToRad(sa));
    const double xe = xc + rx * std::cos(wxDegToRad(ea));
    const double ys = yc - ry * std::sin(wxDegToRad(sa));
    const double ye = yc - ry * std::sin(wxDegToRad(ea));

    // svg arcs have 0 degrees at 12-o'clock instead of 3-o'clock
    double start = (sa - 90);
    if (start < 0)
        start += 360;
    while (std::fabs(start) > 360)
        start -= (start / std::fabs(start)) * 360;

    double end = (ea - 90);
    if (end < 0)
        end += 360;
    while (std::fabs(end) > 360)
        end -= (end / std::fabs(end)) * 360;

    // svg arcs are in clockwise direction, reverse angle
    double angle = end - start;
    if (angle <= 0)
        angle += 360;

    int fArc = angle > 180 ? 1 : 0;  // flag for large or small arc
    static constexpr int fSweep = 0; // flag for sweep always 0

    wxString arcPath;
    if (angle == 360)
    {
        // Drawing full circle fails with default arc. Draw two half arcs instead.
        fArc = 1;
        arcPath = wxString::Format("  <path d=\"M%d %s a%s %s 0 %d %d %s 0 a%s %s 0 %d %d %s 0",
            x, NumStr(y + ry),
            NumStr(rx), NumStr(ry), fArc, fSweep, NumStr( rx * 2),
            NumStr(rx), NumStr(ry), fArc, fSweep, NumStr(-rx * 2));
    }
    else
    {
        arcPath = wxString::Format("  <path d=\"M%s %s A%s %s 0 %d %d %s %s",
            NumStr(xs), NumStr(ys),
            NumStr(rx), NumStr(ry), fArc, fSweep, NumStr(xe), NumStr(ye));
    }

    // Workaround so SVG does not draw an extra line from the centre of the drawn arc
    // to the start point of the arc.
    // First draw the arc with the current brush, without a border,
    // then draw the border without filling the arc.
    if (GetBrush().GetStyle() != wxBrushStyle::Transparent)
    {
        wxDCPenChanger setTransp(*GetOwner(), *wxTRANSPARENT_PEN);
        NewGraphicsIfNeeded();

        wxString arcFill = arcPath;
        arcFill += wxString::Format(" L%s %s z\" %s %s/>\n",
            NumStr(xc), NumStr(yc),
            GetRenderMode(m_renderingMode), GetPenPattern(m_pen));
        write(arcFill);
    }

    wxDCBrushChanger setTransp(*GetOwner(), *wxTRANSPARENT_BRUSH);
    NewGraphicsIfNeeded();

    wxString arcLine = wxString::Format("%s\" %s %s/>\n",
        arcPath, GetRenderMode(m_renderingMode), GetPenPattern(m_pen));
    write(arcLine);
}

void wxSVGFileDCImpl::DoGradientFillLinear(const wxRect& rect,
                                           const wxColour& initialColour,
                                           const wxColour& destColour,
                                           wxDirection nDirection)
{
    NewGraphicsIfNeeded();

    float initOpacity;
    float destOpacity;
    wxString initCol = Col2SVG(initialColour, &initOpacity);
    wxString destCol = Col2SVG(destColour, &destOpacity);

    const int x1 = ((nDirection & wxDirection::wxLEFT) > 0) ? 100 : 0;
    const int y1 = ((nDirection & wxDirection::wxUP) > 0) ? 100 : 0;
    const int x2 = ((nDirection & wxDirection::wxRIGHT) > 0) ? 100 : 0;
    const int y2 = ((nDirection & wxDirection::wxDOWN) > 0) ? 100 : 0;

    wxString s;
    s += "  <defs>\n";
    s += wxString::Format("    <linearGradient id=\"gradient%zu\" x1=\"%d%%\" y1=\"%d%%\" x2=\"%d%%\" y2=\"%d%%\">\n",
        m_gradientUniqueId, x1, y1, x2, y2);
    s += wxString::Format("      <stop offset=\"0%%\" style=\"stop-color:%s; stop-opacity:%s\"/>\n",
        initCol, NumStr(initOpacity));
    s += wxString::Format("      <stop offset=\"100%%\" style=\"stop-color:%s; stop-opacity:%s\"/>\n",
        destCol, NumStr(destOpacity));
    s += "    </linearGradient>\n";
    s += "  </defs>\n";

    s += wxString::Format("  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"url(#gradient%zu)\" %s %s %s/>\n",
        rect.x, rect.y, rect.width, rect.height, m_gradientUniqueId,
        GetRenderMode(m_renderingMode), GetPenPattern(m_pen), GetBrushPattern(m_brush));

    m_gradientUniqueId++;

    write(s);

    CalcBoundingBox(rect.x, rect.y);
    CalcBoundingBox(rect.x + rect.width, rect.y + rect.height);
}

void wxSVGFileDCImpl::DoGradientFillConcentric(const wxRect& rect,
                                               const wxColour& initialColour,
                                               const wxColour& destColour,
                                               const wxPoint& circleCenter)
{
    NewGraphicsIfNeeded();

    // TODO: Return color with opacity
    float initOpacity;
    float destOpacity;
    wxString initCol = Col2SVG(initialColour, &initOpacity);
    wxString destCol = Col2SVG(destColour, &destOpacity);

    const double cx = circleCenter.x * 100.0 / rect.GetWidth();
    const double cy = circleCenter.y * 100.0 / rect.GetHeight();
    const double fx = cx;
    const double fd = cy;

    wxString s;
    s += "  <defs>\n";
    s += wxString::Format("    <radialGradient id=\"gradient%zu\" cx=\"%s%%\" cy=\"%s%%\" fx=\"%s%%\" fy=\"%s%%\">\n",
        m_gradientUniqueId, NumStr(cx), NumStr(cy), NumStr(fx), NumStr(fd));
    s += wxString::Format("      <stop offset=\"0%%\" style=\"stop-color:%s; stop-opacity:%s\" />\n",
        initCol, NumStr(initOpacity));
    s += wxString::Format("      <stop offset=\"100%%\" style=\"stop-color:%s; stop-opacity:%s\" />\n",
        destCol, NumStr(destOpacity));
    s += "    </radialGradient>\n";
    s += "  </defs>\n";

    s += wxString::Format("  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"url(#gradient%zu)\" %s %s %s/>\n",
        rect.x, rect.y, rect.width, rect.height, m_gradientUniqueId,
        GetRenderMode(m_renderingMode), GetPenPattern(m_pen), GetBrushPattern(m_brush));

    m_gradientUniqueId++;

    write(s);

    CalcBoundingBox(rect.x, rect.y);
    CalcBoundingBox(rect.x + rect.width, rect.y + rect.height);
}

void wxSVGFileDCImpl::DoSetClippingRegion(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
    // End current graphics group to ensure proper xml nesting (e.g. so that
    // graphics can be subsequently changed inside the clipping region)
    wxString svg =
fmt::format(R"svg_data(</g>
<defs>
  <clipPath id="clip{}">
    <rect id="cliprect{}"
    x="{}"
    y="{}"
    width="{}"
    height="{}"
    style="stroke: gray; fill: none;"
  </clipPath>
</defs>
<g style="clip-path: url(#clip{});">

)svg_data");

 //   wxString svg = "</g>\n"
 //          "<defs>\n"
 //          "  <clipPath id=\"clip" << m_clipNestingLevel << "\">\n"
 //          "    <rect id=\"cliprect" << m_clipNestingLevel << "\" "
 //               "x=\"" << x << "\" "
 //               "y=\"" << y << "\" "
 //               "width=\"" << width << "\" "
 //               "height=\"" << height << "\" "
 //               "style=\"stroke: gray; fill: none;\"/>\n"
 //          "  </clipPath>\n"
 //          "</defs>\n"
 //          "<g style=\"clip-path: url(#clip" << m_clipNestingLevel << ");\">\n";

    write(svg);

    // Re-apply current graphics to ensure proper xml nesting
    DoStartNewGraphics();

    m_clipUniqueId++;
    m_clipNestingLevel++;

    // Update the base class m_clip[XY][12] fields too.
    wxDCImpl::DoSetClippingRegion(x, y, width, height);
}

void wxSVGFileDCImpl::DestroyClippingRegion()
{
    wxString svg;

    // End current graphics element to ensure proper xml nesting (e.g. graphics
    // might have been changed inside the clipping region)
    svg << "</g>\n";

    // Close clipping group elements
    for (size_t i = 0; i < m_clipUniqueId; i++)
    {
        svg << "</g>\n";
    }

    write(svg);

    // Re-apply current graphics (e.g. brush may have been changed inside one
    // of the clipped regions - that change will have been lost after xml
    // elements for the clipped region have been closed).
    DoStartNewGraphics();

    m_clipUniqueId = 0;

    // Also update the base class clipping region information.
    wxDCImpl::DestroyClippingRegion();
}

wxSize wxSVGFileDCImpl::DoGetTextExtent(std::string_view string,
                                      wxCoord* descent,
                                      wxCoord* externalLeading,
                                      const wxFont* theFont) const
{
    wxScreenDC sDC;
    SetScaledScreenDCFont(sDC, theFont ? *theFont : m_font);

    return sDC.GetTextExtent(string, descent, externalLeading);
}

wxCoord wxSVGFileDCImpl::GetCharHeight() const
{
    wxScreenDC sDC;
    SetScaledScreenDCFont(sDC, m_font);

    return sDC.GetCharHeight();

}

wxCoord wxSVGFileDCImpl::wxGetCharWidth() const
{
    wxScreenDC sDC;
    SetScaledScreenDCFont(sDC, m_font);

    return sDC.wxGetCharWidth();
}


// ----------------------------------------------------------
// wxSVGFileDCImpl - set functions
// ----------------------------------------------------------

void wxSVGFileDCImpl::SetBackground(const wxBrush& brush)
{
    m_backgroundBrush = brush;
}

void wxSVGFileDCImpl::SetBackgroundMode(wxBrushStyle mode)
{
    m_backgroundMode = mode;
}

void wxSVGFileDCImpl::SetBitmapHandler(std::unique_ptr<wxSVGBitmapHandler> handler)
{
    m_bmp_handler = std::move(handler);
}

void wxSVGFileDCImpl::SetShapeRenderingMode(wxSVGShapeRenderingMode renderingMode)
{
    m_renderingMode = renderingMode;
}

void wxSVGFileDCImpl::SetBrush(const wxBrush& brush)
{
    m_brush = brush;

    m_graphics_changed = true;

    wxString pattern = CreateBrushFill(m_brush, m_renderingMode);
    if ( !pattern.empty() )
    {
        NewGraphicsIfNeeded();

        write(pattern);
    }
}

void wxSVGFileDCImpl::SetPen(const wxPen& pen)
{
    m_pen = pen;

    m_graphics_changed = true;
}

void wxSVGFileDCImpl::NewGraphicsIfNeeded()
{
    if ( !m_graphics_changed )
        return;

    m_graphics_changed = false;

    write("</g>\n");

    DoStartNewGraphics();
}

void wxSVGFileDCImpl::DoStartNewGraphics()
{
    wxString s = wxString::Format("<g style=\"%s %s %s\" transform=\"translate(%d %d) scale(%s %s)\">\n",
        GetPenStyle(m_pen),
        GetBrushFill(m_brush.GetColour(), m_brush.GetStyle()),
        GetPenStroke(m_pen.GetColour(), m_pen.GetStyle()),
        (m_deviceOrigin.x - m_logicalOrigin.x) * m_signX,
        (m_deviceOrigin.y - m_logicalOrigin.y) * m_signY,
        NumStr(m_scale.x * m_signX),
        NumStr(m_scale.y * m_signY));

    write(s);
}

void wxSVGFileDCImpl::SetFont(const wxFont& font)
{
    m_font = font;
}

bool wxSVGFileDCImpl::DoBlit(wxCoord xdest, wxCoord ydest,
                             wxCoord width, wxCoord height,
                             wxDC* source,
                             wxPoint src,
                             wxRasterOperationMode rop,
                             bool useMask,
                             [[maybe_unused]] wxPoint srcMask)
{
    if (rop != wxRasterOperationMode::Copy)
    {
        wxASSERT_MSG(false, "wxSVGFileDC::DoBlit Call requested nonCopy mode; this is not possible");
        return false;
    }
    if (useMask)
    {
        wxASSERT_MSG(false, "wxSVGFileDC::DoBlit Call requested mask; this is not possible");
        return false;
    }
    wxBitmap myBitmap(wxSize{width, height});
    wxMemoryDC memDC;
    memDC.SelectObject(myBitmap);
    memDC.Blit(wxPoint{0, 0}, wxSize{width, height}, source, src);
    memDC.SelectObject(wxNullBitmap);
    DoDrawBitmap(myBitmap, xdest, ydest);
    return false;
}

void wxSVGFileDCImpl::DoDrawIcon(const wxIcon& icon, wxCoord x, wxCoord y)
{
    wxBitmap myBitmap(icon.GetSize());
    wxMemoryDC memDC;
    memDC.SelectObject(myBitmap);
    memDC.DrawIcon(icon, 0, 0);
    memDC.SelectObject(wxNullBitmap);
    DoDrawBitmap(myBitmap, x, y);
}

void wxSVGFileDCImpl::DoDrawBitmap(const wxBitmap& bmp, wxCoord x, wxCoord y,
                                   [[maybe_unused]] bool useMask)
{
    NewGraphicsIfNeeded();

    // If we don't have any bitmap handler yet, use the default one.
    if ( !m_bmp_handler )
        m_bmp_handler = std::make_unique<wxSVGBitmapFileHandler>(wxFileName{m_filename});

    m_OK = m_outfile && m_outfile->IsOk();
    if (!m_OK)
        return;

    m_bmp_handler->ProcessBitmap(bmp, x, y, *m_outfile);
    m_OK = m_outfile->IsOk();
}

void wxSVGFileDCImpl::write(const std::string& s)
{
    m_OK = m_outfile && m_outfile->IsOk();
    if (!m_OK)
        return;

    const wxCharBuffer buf = s.c_str();
    m_outfile->Write(buf, strlen((const char*)buf));
    m_OK = m_outfile->IsOk();
}

#endif // wxUSE_SVG
