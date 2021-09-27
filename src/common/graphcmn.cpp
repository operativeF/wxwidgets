/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/graphcmn.cpp
// Purpose:     graphics context methods common to all platforms
// Author:      Stefan Csomor
// Modified by:
// Created:
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_GRAPHICS_CONTEXT

#include "wx/graphics.h"

#include "wx/bitmap.h"
#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/dcprint.h"
#include "wx/math.h"
#include "wx/pen.h"
#include "wx/log.h"
#include "wx/window.h"

#ifdef __WXMSW__
    #include "wx/msw/enhmeta.h"
#endif

#include "wx/private/graphics.h"

#include <memory>

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// wxGraphicsObject
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxGraphicsObject, wxObject);

wxGraphicsObjectRefData::wxGraphicsObjectRefData( wxGraphicsRenderer* renderer )
    : m_renderer(renderer)
{
}

wxGraphicsObjectRefData::wxGraphicsObjectRefData( const wxGraphicsObjectRefData* data )
    : m_renderer(data->m_renderer)
{
}

wxGraphicsRenderer* wxGraphicsObjectRefData::GetRenderer() const
{
    return m_renderer;
}

wxGraphicsObjectRefData* wxGraphicsObjectRefData::Clone() const
{
    return new wxGraphicsObjectRefData(this);
}

wxGraphicsObject::wxGraphicsObject( wxGraphicsRenderer* renderer )
{
    SetRefData( new wxGraphicsObjectRefData(renderer));
}

bool wxGraphicsObject::IsNull() const
{
    return m_refData == nullptr;
}

wxGraphicsRenderer* wxGraphicsObject::GetRenderer() const
{
    return ( IsNull() ? nullptr : GetGraphicsData()->GetRenderer() );
}

wxGraphicsObjectRefData* wxGraphicsObject::GetGraphicsData() const
{
    return (wxGraphicsObjectRefData*) m_refData;
}

wxObjectRefData* wxGraphicsObject::CreateRefData() const
{
    wxLogDebug(wxT("A Null Object cannot be changed"));
    return nullptr;
}

wxObjectRefData* wxGraphicsObject::CloneRefData(const wxObjectRefData* data) const
{
    const wxGraphicsObjectRefData* ptr = (const wxGraphicsObjectRefData*) data;
    return ptr->Clone();
}

//-----------------------------------------------------------------------------
// pens etc.
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxGraphicsPen, wxGraphicsObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxGraphicsBrush, wxGraphicsObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxGraphicsFont, wxGraphicsObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxGraphicsBitmap, wxGraphicsObject);

WXDLLIMPEXP_DATA_CORE(wxGraphicsPen) wxNullGraphicsPen;
WXDLLIMPEXP_DATA_CORE(wxGraphicsBrush) wxNullGraphicsBrush;
WXDLLIMPEXP_DATA_CORE(wxGraphicsFont) wxNullGraphicsFont;
WXDLLIMPEXP_DATA_CORE(wxGraphicsBitmap) wxNullGraphicsBitmap;

//-----------------------------------------------------------------------------
// matrix
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxGraphicsMatrix, wxGraphicsObject);
WXDLLIMPEXP_DATA_CORE(wxGraphicsMatrix) wxNullGraphicsMatrix;

// concatenates the matrix
void wxGraphicsMatrix::Concat( const wxGraphicsMatrix *t )
{
    AllocExclusive();
    GetMatrixData()->Concat(t->GetMatrixData());
}

// sets the matrix to the respective values
void wxGraphicsMatrix::Set(double a, double b, double c, double d,
                           double tx, double ty)
{
    AllocExclusive();
    GetMatrixData()->Set(a,b,c,d,tx,ty);
}

// gets the component valuess of the matrix
void wxGraphicsMatrix::Get(double* a, double* b,  double* c,
                           double* d, double* tx, double* ty) const
{
    GetMatrixData()->Get(a, b, c, d, tx, ty);
}

// makes this the inverse matrix
void wxGraphicsMatrix::Invert()
{
    AllocExclusive();
    GetMatrixData()->Invert();
}

// returns true if the elements of the transformation matrix are equal ?
bool wxGraphicsMatrix::IsEqual( const wxGraphicsMatrix* t) const
{
    return GetMatrixData()->IsEqual(t->GetMatrixData());
}

// return true if this is the identity matrix
bool wxGraphicsMatrix::IsIdentity() const
{
    return GetMatrixData()->IsIdentity();
}

// add the translation to this matrix
void wxGraphicsMatrix::Translate( double dx , double dy )
{
    AllocExclusive();
    GetMatrixData()->Translate(dx,dy);
}

// add the scale to this matrix
void wxGraphicsMatrix::Scale( double xScale , double yScale )
{
    AllocExclusive();
    GetMatrixData()->Scale(xScale,yScale);
}

// add the rotation to this matrix (radians)
void wxGraphicsMatrix::Rotate( double angle )
{
    AllocExclusive();
    GetMatrixData()->Rotate(angle);
}

//
// apply the transforms
//

// applies that matrix to the point
void wxGraphicsMatrix::TransformPoint( double *x, double *y ) const
{
    GetMatrixData()->TransformPoint(x,y);
}

// applies the matrix except for translations
void wxGraphicsMatrix::TransformDistance( double *dx, double *dy ) const
{
    GetMatrixData()->TransformDistance(dx,dy);
}

// returns the native representation
void * wxGraphicsMatrix::GetNativeMatrix() const
{
    return GetMatrixData()->GetNativeMatrix();
}

//-----------------------------------------------------------------------------
// path
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxGraphicsPath, wxGraphicsObject);
WXDLLIMPEXP_DATA_CORE(wxGraphicsPath) wxNullGraphicsPath;

// convenience functions, for using wxPoint2DDouble etc

wxPoint2DDouble wxGraphicsPath::GetCurrentPoint() const
{
    double x,y;
    GetCurrentPoint(&x,&y);
    return {x, y};
}

void wxGraphicsPath::MoveToPoint( const wxPoint2DDouble& p)
{
    MoveToPoint( p.m_x , p.m_y);
}

void wxGraphicsPath::AddLineToPoint( const wxPoint2DDouble& p)
{
    AddLineToPoint( p.m_x , p.m_y);
}

void wxGraphicsPath::AddCurveToPoint( const wxPoint2DDouble& c1, const wxPoint2DDouble& c2, const wxPoint2DDouble& e)
{
    AddCurveToPoint(c1.m_x, c1.m_y, c2.m_x, c2.m_y, e.m_x, e.m_y);
}

void wxGraphicsPath::AddArc( const wxPoint2DDouble& c, double r, double startAngle, double endAngle, bool clockwise)
{
    AddArc(c.m_x, c.m_y, r, startAngle, endAngle, clockwise);
}

wxRect2DDouble wxGraphicsPath::GetBox() const
{
    double x,y,w,h;
    GetBox(&x,&y,&w,&h);
    return { x, y, w, h };
}

bool wxGraphicsPath::Contains( const wxPoint2DDouble& c, wxPolygonFillMode fillStyle ) const
{
    return Contains( c.m_x, c.m_y, fillStyle);
}

// true redirections

// begins a new subpath at (x,y)
void wxGraphicsPath::MoveToPoint( double x, double y )
{
    AllocExclusive();
    GetPathData()->MoveToPoint(x,y);
}

// adds a straight line from the current point to (x,y)
void wxGraphicsPath::AddLineToPoint( double x, double y )
{
    AllocExclusive();
    GetPathData()->AddLineToPoint(x,y);
}

// adds a cubic Bezier curve from the current point, using two control points and an end point
void wxGraphicsPath::AddCurveToPoint( double cx1, double cy1, double cx2, double cy2, double x, double y )
{
    AllocExclusive();
    GetPathData()->AddCurveToPoint(cx1,cy1,cx2,cy2,x,y);
}

// adds another path
void wxGraphicsPath::AddPath( const wxGraphicsPath& path )
{
    AllocExclusive();
    GetPathData()->AddPath(path.GetPathData());
}

// closes the current sub-path
void wxGraphicsPath::CloseSubpath()
{
    AllocExclusive();
    GetPathData()->CloseSubpath();
}

// gets the last point of the current path, (0,0) if not yet set
void wxGraphicsPath::GetCurrentPoint( double* x, double* y) const
{
    GetPathData()->GetCurrentPoint(x,y);
}

// adds an arc of a circle centering at (x,y) with radius (r) from startAngle to endAngle
void wxGraphicsPath::AddArc( double x, double y, double r, double startAngle, double endAngle, bool clockwise )
{
    AllocExclusive();
    GetPathData()->AddArc(x,y,r,startAngle,endAngle,clockwise);
}

//
// These are convenience functions which - if not available natively will be assembled
// using the primitives from above
//

// adds a quadratic Bezier curve from the current point, using a control point and an end point
void wxGraphicsPath::AddQuadCurveToPoint( double cx, double cy, double x, double y )
{
    AllocExclusive();
    GetPathData()->AddQuadCurveToPoint(cx,cy,x,y);
}

// appends a rectangle as a new closed subpath
void wxGraphicsPath::AddRectangle( double x, double y, double w, double h )
{
    AllocExclusive();
    GetPathData()->AddRectangle(x,y,w,h);
}

// appends an ellipsis as a new closed subpath fitting the passed rectangle
void wxGraphicsPath::AddCircle( double x, double y, double r )
{
    AllocExclusive();
    GetPathData()->AddCircle(x,y,r);
}

// appends a an arc to two tangents connecting (current) to (x1,y1) and (x1,y1) to (x2,y2), also a straight line from (current) to (x1,y1)
void wxGraphicsPath::AddArcToPoint( double x1, double y1 , double x2, double y2, double r )
{
    GetPathData()->AddArcToPoint(x1,y1,x2,y2,r);
}

// appends an ellipse
void wxGraphicsPath::AddEllipse( double x, double y, double w, double h)
{
    AllocExclusive();
    GetPathData()->AddEllipse(x,y,w,h);
}

// appends a rounded rectangle
void wxGraphicsPath::AddRoundedRectangle( double x, double y, double w, double h, double radius)
{
    AllocExclusive();
    GetPathData()->AddRoundedRectangle(x,y,w,h,radius);
}

// returns the native path
void * wxGraphicsPath::GetNativePath() const
{
    return GetPathData()->GetNativePath();
}

// give the native path returned by GetNativePath() back (there might be some deallocations necessary)
void wxGraphicsPath::UnGetNativePath(void *p)const
{
    GetPathData()->UnGetNativePath(p);
}

// transforms each point of this path by the matrix
void wxGraphicsPath::Transform( const wxGraphicsMatrix& matrix )
{
    AllocExclusive();
    GetPathData()->Transform(matrix.GetMatrixData());
}

// gets the bounding box enclosing all points (possibly including control points)
void wxGraphicsPath::GetBox(double *x, double *y, double *w, double *h) const
{
    GetPathData()->GetBox(x,y,w,h);
}

bool wxGraphicsPath::Contains( double x, double y, wxPolygonFillMode fillStyle ) const
{
    return GetPathData()->Contains(x,y,fillStyle);
}

//
// Emulations, these mus be implemented in the ...Data classes in order to allow for proper overrides
//

void wxGraphicsPathData::AddQuadCurveToPoint( double cx, double cy, double x, double y )
{
    // calculate using degree elevation to a cubic bezier
    wxPoint2DDouble c1;
    wxPoint2DDouble c2;

    wxPoint2DDouble start;
    GetCurrentPoint(&start.m_x,&start.m_y);
    const wxPoint2DDouble end(x,y);
    const wxPoint2DDouble c(cx,cy);
    c1 = double(1/3.0) * start + double(2/3.0) * c;
    c2 = double(2/3.0) * c + double(1/3.0) * end;
    AddCurveToPoint(c1.m_x,c1.m_y,c2.m_x,c2.m_y,x,y);
}

void wxGraphicsPathData::AddRectangle( double x, double y, double w, double h )
{
    MoveToPoint(x,y);
    AddLineToPoint(x,y+h);
    AddLineToPoint(x+w,y+h);
    AddLineToPoint(x+w,y);
    CloseSubpath();
}

void wxGraphicsPathData::AddCircle( double x, double y, double r )
{
    MoveToPoint(x+r,y);
    AddArc( x,y,r,0, 2.0 * std::numbers::pi, false);
    CloseSubpath();
}

void wxGraphicsPathData::AddEllipse( double x, double y, double w, double h)
{
    if (w <= 0. || h <= 0.)
      return;

    const double rw = w/2;
    const double rh = h/2;
    const double xc = x + rw;
    const double yc = y + rh;
    wxGraphicsMatrix m = GetRenderer()->CreateMatrix();
    m.Translate(xc,yc);
    m.Scale(rw/rh,1.0);
    wxGraphicsPath p = GetRenderer()->CreatePath();
    p.AddCircle(0,0,rh);
    p.Transform(m);
    AddPath(p.GetPathData());
}

void wxGraphicsPathData::AddRoundedRectangle( double x, double y, double w, double h, double radius)
{
    if ( radius == 0 )
        AddRectangle(x,y,w,h);
    else
    {
        MoveToPoint(x+w, y+h/2);
        AddArc(x+w-radius, y+h-radius, radius, 0.0, std::numbers::pi / 2.0, true);
        AddArc(x+radius, y+h-radius, radius, std::numbers::pi / 2.0, std::numbers::pi, true);
        AddArc(x+radius, y+radius, radius, std::numbers::pi, 3.0 * std::numbers::pi /2.0, true);
        AddArc(x+w-radius, y+radius, radius, 3.0 * std::numbers::pi /2.0, 2.0 * std::numbers::pi, true);
        CloseSubpath();
    }
}

// draws a an arc to two tangents connecting (current) to (x1,y1) and (x1,y1) to (x2,y2), also a straight line from (current) to (x1,y1)
void wxGraphicsPathData::AddArcToPoint( double x1, double y1 , double x2, double y2, double r )
{
    wxPoint2DDouble current;
    GetCurrentPoint(&current.m_x, &current.m_y);
    if ( current == wxPoint2DDouble{0, 0} )
    {
        // (0, 0) is returned by GetCurrentPoint() also when the last point is not yet actually set,
        // so we should reposition it to (0, 0) to be sure that a last point is initially set.
        MoveToPoint(0, 0);
    }

    const wxPoint2DDouble p1(x1, y1);
    const wxPoint2DDouble p2(x2, y2);

    wxPoint2DDouble v1 = current - p1;
    const double v1Length = GetVectorLength(v1);
    wxPoint2DDouble v2 = p2 - p1;
    const double v2Length = GetVectorLength(v2);

    double alpha = GetVectorAngle(v1) - GetVectorAngle(v2);
    // Reduce angle value to the range [0..180] degrees.
    if ( alpha < 0 )
        alpha = 360 + alpha;
    if ( alpha > 180 )
        alpha = 360 - alpha;

    // Degenerated cases: there is no need
    // to draw an arc connecting points when:
    // - There are no 3 different points provided,
    // - Points (and lines) are colinear,
    // - Radius equals zero.
    if ( v1Length == 0 || v2Length == 0 ||
         alpha == 0 || alpha == 180 || r == 0 )
    {
        AddLineToPoint(p1.m_x, p1.m_y);
        return;
    }

    // Determine spatial relation between the vectors.
    bool drawClockwiseArc = VectorCrossProduct(v1, v2) < 0;

    alpha = wxDegToRad(alpha);
    const double distT = r / std::sin(alpha) * (1.0 + std::cos(alpha)); // = r / tan(a/2) =  r / sin(a/2) * cos(a/2)
    const double distC = r / std::sin(alpha / 2.0);
    // Calculate tangential points
    Normalize(v1);
    Normalize(v2);
    const wxPoint2DDouble t1 = distT*v1 + p1;
    const wxPoint2DDouble t2 = distT*v2 + p1;
    // Calculate the angle bisector vector
    // (because central point is located on the bisector).
    wxPoint2DDouble v = v1 + v2;
    if ( GetVectorLength(v) > 0 )
        Normalize(v);
    // Calculate center of the arc
    const wxPoint2DDouble c = distC*v + p1;
    // Calculate normal vectors at tangential points
    // (with inverted directions to make angle calculations easier).
    const wxPoint2DDouble nv1 = t1 - c;
    const wxPoint2DDouble nv2 = t2 - c;
    // Calculate start and end angle of the arc.
    const double a1 = GetVectorAngle(nv1);
    const double a2 = GetVectorAngle(nv2);

    AddLineToPoint(t1.m_x, t1.m_y);
    AddArc(c.m_x, c.m_y, r, wxDegToRad(a1), wxDegToRad(a2), drawClockwiseArc);
}

//-----------------------------------------------------------------------------
// wxGraphicsGradientStops
//-----------------------------------------------------------------------------

void wxGraphicsGradientStops::Add(const wxGraphicsGradientStop& stop)
{
    // FIXME: Can't we just avoid this loop altogether?
    for ( std::vector<wxGraphicsGradientStop>::iterator it = m_stops.begin();
          it != m_stops.end();
          ++it )
    {
        if ( stop.GetPosition() < it->GetPosition() )
        {
            if ( it != m_stops.begin() )
            {
                m_stops.insert(it, stop);
            }
            else // we shouldn't be inserting it at the beginning
            {
                wxFAIL_MSG( "invalid gradient stop position < 0" );
            }

            return;
        }
    }

    if ( stop.GetPosition() == 1 )
    {
        m_stops.insert(m_stops.end() - 1, stop);
    }
    else
    {
        wxFAIL_MSG( "invalid gradient stop position > 1" );
    }
}

void * wxGraphicsBitmap::GetNativeBitmap() const
{
    return GetBitmapData()->GetNativeBitmap();
}

//-----------------------------------------------------------------------------
// wxGraphicsContext Convenience Methods
//-----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxGraphicsContext, wxObject);


wxGraphicsContext::wxGraphicsContext(wxGraphicsRenderer* renderer,
                                     wxWindow* window)
    : wxGraphicsObject(renderer),
      m_antialias(wxAntialiasMode::Default),
      m_composition(wxCOMPOSITION_OVER),
      m_interpolation(wxInterpolationQuality::Default),
      m_enableOffset(false),
      m_window(window),
      m_contentScaleFactor(window ? window->GetContentScaleFactor() : 1.0)
{
}

bool wxGraphicsContext::wxStartDoc(const std::string& WXUNUSED(message))
{
    return true;
}

void wxGraphicsContext::EndDoc()
{
}

void wxGraphicsContext::StartPage(double WXUNUSED(width),
                                  double WXUNUSED(height))
{
}

void wxGraphicsContext::EndPage()
{
}

void wxGraphicsContext::Flush()
{
}

void wxGraphicsContext::EnableOffset(bool enable)
{
    m_enableOffset = enable;
}

void wxGraphicsContext::SetContentScaleFactor(double contentScaleFactor)
{
    m_enableOffset = true;
    m_contentScaleFactor = contentScaleFactor;
}

#if 0
void wxGraphicsContext::SetAlpha( double WXUNUSED(alpha) )
{
}

double wxGraphicsContext::GetAlpha() const
{
    return 1.0;
}
#endif

void wxGraphicsContext::GetDPI( double* dpiX, double* dpiY) const
{
    if ( m_window )
    {
        const wxSize ppi = m_window->GetDPI();
        *dpiX = ppi.x;
        *dpiY = ppi.y;
    }
    else
    {
        // Use some standard DPI value, it doesn't make much sense for the
        // contexts not using any pixels anyhow.
        *dpiX = 72.0;
        *dpiY = 72.0;
    }
}

// sets the pen
void wxGraphicsContext::SetPen( const wxGraphicsPen& pen )
{
    m_pen = pen;
}

void wxGraphicsContext::SetPen( const wxPen& pen )
{
    if ( !pen.IsOk() || pen.GetStyle() == wxPenStyle::Transparent )
        SetPen( wxNullGraphicsPen );
    else
        SetPen( CreatePen( pen ) );
}

// sets the brush for filling
void wxGraphicsContext::SetBrush( const wxGraphicsBrush& brush )
{
    m_brush = brush;
}

void wxGraphicsContext::SetBrush( const wxBrush& brush )
{
    if ( !brush.IsOk() || brush.GetStyle() == wxBrushStyle::Transparent )
        SetBrush( wxNullGraphicsBrush );
    else
        SetBrush( CreateBrush( brush ) );
}

// sets the brush for filling
void wxGraphicsContext::SetFont( const wxGraphicsFont& font )
{
    m_font = font;
}

void wxGraphicsContext::SetFont(const wxFont& font, const wxColour& colour)
{
    if ( font.IsOk() )
    {
        // Change current font only if new graphics font is successfully created.
        wxGraphicsFont grFont = wxCreateFont(font, colour);
        if ( !grFont.IsSameAs(wxNullGraphicsFont) )
        {
            SetFont(grFont);
        }
    }
    else
    {
        SetFont( wxNullGraphicsFont );
    }
}

void wxGraphicsContext::DrawPath( const wxGraphicsPath& path, wxPolygonFillMode fillStyle )
{
    FillPath( path , fillStyle );
    StrokePath( path );
}

void
wxGraphicsContext::DoDrawRotatedText(std::string_view str,
                                     double x,
                                     double y,
                                     double angle)
{
    Translate(x,y);
    Rotate( -angle );
    wxDrawText( str , 0, 0 );
    Rotate( angle );
    Translate(-x,-y);
}

void
wxGraphicsContext::DoDrawFilledText(std::string_view str,
                                    double x,
                                    double y,
                                    const wxGraphicsBrush& backgroundBrush)
{
    wxGraphicsBrush formerBrush = m_brush;
    wxGraphicsPen formerPen = m_pen;

    double descent;
    double externalLeading;
    auto [width, height] = GetTextExtent( str, &descent, &externalLeading );
    SetBrush( backgroundBrush );
    // to make sure our 'OffsetToPixelBoundaries' doesn't move the fill shape
    SetPen( wxNullGraphicsPen );

    DrawRectangle(x , y, width, height);

    wxDrawText( str, x ,y);
    SetBrush( formerBrush );
    SetPen( formerPen );
}

void
wxGraphicsContext::DoDrawRotatedFilledText(std::string_view str,
                                           double x, double y,
                                           double angle,
                                           const wxGraphicsBrush& backgroundBrush)
{
    wxGraphicsBrush formerBrush = m_brush;
    wxGraphicsPen formerPen = m_pen;

    double descent;
    double externalLeading;

    auto [width, height] = GetTextExtent( str, &descent, &externalLeading );
    SetBrush( backgroundBrush );
    // to make sure our 'OffsetToPixelBoundaries' doesn't move the fill shape
    SetPen( wxNullGraphicsPen );

    wxGraphicsPath path = CreatePath();
    path.MoveToPoint( x , y );
    path.AddLineToPoint( (int) (x + std::sin(angle) * height) , (int) (y + std::cos(angle) * height) );
    path.AddLineToPoint(
        (int) (x + std::sin(angle) * height + std::cos(angle) * width) ,
        (int) (y + std::cos(angle) * height - std::sin(angle) * width));
    path.AddLineToPoint((int) (x + std::cos(angle) * width) , (int) (y - std::sin(angle) * width) );
    FillPath( path );
    wxDrawText( str, x ,y, angle);
    SetBrush( formerBrush );
    SetPen( formerPen );
}

void wxGraphicsContext::StrokeLine( double x1, double y1, double x2, double y2)
{
    wxGraphicsPath path = CreatePath();
    path.MoveToPoint(x1, y1);
    path.AddLineToPoint( x2, y2 );
    StrokePath( path );
}

void wxGraphicsContext::DrawRectangle( double x, double y, double w, double h)
{
    wxGraphicsPath path = CreatePath();
    path.AddRectangle( x , y , w , h );
    DrawPath( path );
}

void wxGraphicsContext::ClearRectangle( double WXUNUSED(x), double WXUNUSED(y), double WXUNUSED(w), double WXUNUSED(h))
{

}

void wxGraphicsContext::DrawEllipse( double x, double y, double w, double h)
{
    wxGraphicsPath path = CreatePath();
    path.AddEllipse(x,y,w,h);
    DrawPath(path);
}

void wxGraphicsContext::DrawRoundedRectangle( double x, double y, double w, double h, double radius)
{
    wxGraphicsPath path = CreatePath();
    path.AddRoundedRectangle(x,y,w,h,radius);
    DrawPath(path);
}

void wxGraphicsContext::StrokeLines( size_t n, const wxPoint2DDouble *points)
{
    wxASSERT(n > 1);
    wxGraphicsPath path = CreatePath();
    path.MoveToPoint(points[0].m_x, points[0].m_y);
    for ( size_t i = 1; i < n; ++i)
        path.AddLineToPoint( points[i].m_x, points[i].m_y );
    StrokePath( path );
}

void wxGraphicsContext::DrawLines( size_t n, const wxPoint2DDouble *points, wxPolygonFillMode fillStyle)
{
    wxASSERT(n > 1);
    wxGraphicsPath path = CreatePath();
    path.MoveToPoint(points[0].m_x, points[0].m_y);
    for ( size_t i = 1; i < n; ++i)
        path.AddLineToPoint( points[i].m_x, points[i].m_y );
    DrawPath( path , fillStyle);
}

void wxGraphicsContext::StrokeLines( size_t n, const wxPoint2DDouble *beginPoints, const wxPoint2DDouble *endPoints)
{
    wxASSERT(n > 0);
    wxGraphicsPath path = CreatePath();
    for ( size_t i = 0; i < n; ++i)
    {
        path.MoveToPoint(beginPoints[i].m_x, beginPoints[i].m_y);
        path.AddLineToPoint( endPoints[i].m_x, endPoints[i].m_y );
    }
    StrokePath( path );
}

// create a 'native' matrix corresponding to these values
wxGraphicsMatrix wxGraphicsContext::CreateMatrix( double a, double b, double c, double d,
    double tx, double ty) const
{
    return GetRenderer()->CreateMatrix(a,b,c,d,tx,ty);
}

wxGraphicsPath wxGraphicsContext::CreatePath() const
{
    return GetRenderer()->CreatePath();
}

wxGraphicsPen wxGraphicsContext::CreatePen(const wxPen& pen) const
{
    if ( !pen.IsOk() )
        return {};

    wxGraphicsPenInfo info = wxGraphicsPenInfo()
                                .Colour(pen.GetColour())
                                .Width(pen.GetWidth())
                                .Style(pen.GetStyle())
                                .Join(pen.GetJoin())
                                .Cap(pen.GetCap())
                                ;

    if ( info.GetStyle() == wxPenStyle::UserDash )
    {
        wxDash *dashes;
        if ( const int nb_dashes = pen.GetDashes(&dashes) )
            info.Dashes(nb_dashes, dashes);
    }

    if ( info.GetStyle() == wxPenStyle::Stipple )
    {
        if ( wxBitmap* const stipple = pen.GetStipple() )
            info.Stipple(*stipple);
    }

    return DoCreatePen(info);
}

wxGraphicsPen wxGraphicsContext::DoCreatePen(const wxGraphicsPenInfo& info) const
{
    return GetRenderer()->CreatePen(info);
}

wxGraphicsBrush wxGraphicsContext::CreateBrush(const wxBrush& brush ) const
{
    return GetRenderer()->CreateBrush(brush);
}

wxGraphicsBrush
wxGraphicsContext::CreateLinearGradientBrush(
    double x1, double y1,
    double x2, double y2,
    const wxColour& c1, const wxColour& c2,
    const wxGraphicsMatrix& matrix) const
{
    return GetRenderer()->CreateLinearGradientBrush
                          (
                            x1, y1,
                            x2, y2,
                            wxGraphicsGradientStops(c1,c2),
                            matrix
                          );
}

wxGraphicsBrush
wxGraphicsContext::CreateLinearGradientBrush(
    double x1, double y1,
    double x2, double y2,
    const wxGraphicsGradientStops& gradientStops,
    const wxGraphicsMatrix& matrix) const
{
    return GetRenderer()->CreateLinearGradientBrush
                          (
                            x1, y1,
                            x2, y2, 
                            gradientStops, 
                            matrix
                          );
}

wxGraphicsBrush
wxGraphicsContext::CreateRadialGradientBrush(
        double startX, double startY,
        double endX, double endY, double radius,
        const wxColour &oColor, const wxColour &cColor,
        const wxGraphicsMatrix& matrix) const
{
    return GetRenderer()->CreateRadialGradientBrush
                          (
                            startX, startY,
                            endX, endY, radius,
                            wxGraphicsGradientStops(oColor, cColor),
                            matrix
                          );
}

wxGraphicsBrush
wxGraphicsContext::CreateRadialGradientBrush(
        double startX, double startY,
        double endX, double endY, double radius,
        const wxGraphicsGradientStops& gradientStops,
        const wxGraphicsMatrix& matrix) const
{
    return GetRenderer()->CreateRadialGradientBrush
                          (
                            startX, startY,
                            endX, endY, radius,
                            gradientStops,
                            matrix
                          );
}

wxGraphicsFont wxGraphicsContext::wxCreateFont( const wxFont &font , const wxColour &col ) const
{
    wxRealPoint dpi;
    GetDPI(&dpi.x, &dpi.y);
    return GetRenderer()->CreateFontAtDPI(font, dpi, col);
}

wxGraphicsFont
wxGraphicsContext::wxCreateFont(double sizeInPixels,
                              const std::string& facename,
                              int flags,
                              const wxColour& col) const
{
    return GetRenderer()->wxCreateFont(sizeInPixels, facename, flags, col);
}

wxGraphicsBitmap wxGraphicsContext::CreateBitmap( const wxBitmap& bmp ) const
{
    return GetRenderer()->CreateBitmap(bmp);
}

#if wxUSE_IMAGE
wxGraphicsBitmap wxGraphicsContext::CreateBitmapFromImage(const wxImage& image) const
{
    return GetRenderer()->CreateBitmapFromImage(image);
}
#endif // wxUSE_IMAGE

wxGraphicsBitmap wxGraphicsContext::CreateSubBitmap( const wxGraphicsBitmap &bmp, double x, double y, double w, double h   ) const
{
    return GetRenderer()->CreateSubBitmap(bmp,x,y,w,h);
}

/* static */ std::unique_ptr<wxGraphicsContext> wxGraphicsContext::Create( const wxWindowDC& dc)
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(dc);
}

/* static */ std::unique_ptr<wxGraphicsContext> wxGraphicsContext::Create( const wxMemoryDC& dc)
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(dc);
}

#if wxUSE_PRINTING_ARCHITECTURE
/* static */ std::unique_ptr<wxGraphicsContext> wxGraphicsContext::Create( const wxPrinterDC& dc)
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(dc);
}
#endif

#ifdef __WXMSW__
#if wxUSE_ENH_METAFILE
/* static */ std::unique_ptr<wxGraphicsContext> wxGraphicsContext::Create( const wxEnhMetaFileDC& dc)
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(dc);
}
#endif
#endif

std::unique_ptr<wxGraphicsContext> wxGraphicsContext::CreateFromUnknownDC(const wxDC& dc)
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateContextFromUnknownDC(dc);
}

std::unique_ptr<wxGraphicsContext> wxGraphicsContext::CreateFromNative( void * context )
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateContextFromNativeContext(context);
}

std::unique_ptr<wxGraphicsContext> wxGraphicsContext::CreateFromNativeWindow( void * window )
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateContextFromNativeWindow(window);
}

#ifdef __WXMSW__
std::unique_ptr<wxGraphicsContext> wxGraphicsContext::CreateFromNativeHDC(WXHDC dc)
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateContextFromNativeHDC(dc);
}
#endif

std::unique_ptr<wxGraphicsContext> wxGraphicsContext::Create( wxWindow* window )
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(window);
}

#if wxUSE_IMAGE
/* static */ std::unique_ptr<wxGraphicsContext> wxGraphicsContext::Create(wxImage& image)
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateContextFromImage(image);
}
#endif // wxUSE_IMAGE

std::unique_ptr<wxGraphicsContext> wxGraphicsContext::Create()
{
    return wxGraphicsRenderer::GetDefaultRenderer()->CreateMeasuringContext();
}

//-----------------------------------------------------------------------------
// wxGraphicsRenderer
//-----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxGraphicsRenderer, wxObject);

std::unique_ptr<wxGraphicsContext> wxGraphicsRenderer::CreateContextFromUnknownDC(const wxDC& dc)
{
    if ( const wxWindowDC *windc = wxDynamicCast(&dc, wxWindowDC) )
        return CreateContext(*windc);

    if ( const wxMemoryDC *memdc = wxDynamicCast(&dc, wxMemoryDC) )
        return CreateContext(*memdc);

#if wxUSE_PRINTING_ARCHITECTURE
    if ( const wxPrinterDC *printdc = wxDynamicCast(&dc, wxPrinterDC) )
        return CreateContext(*printdc);
#endif

#ifdef __WXMSW__
#if wxUSE_ENH_METAFILE
    if ( const wxEnhMetaFileDC *mfdc = wxDynamicCast(&dc, wxEnhMetaFileDC) )
        return CreateContext(*mfdc);
#endif
#endif

    return nullptr;
}

#endif // wxUSE_GRAPHICS_CONTEXT
