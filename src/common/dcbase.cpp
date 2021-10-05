/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/dcbase.cpp
// Purpose:     generic methods of the wxDC Class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     05/25/99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/dcprint.h"
#include "wx/prntbase.h"
#include "wx/scopeguard.h"
#include "wx/dc.h"
#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/dcscreen.h"
#include "wx/math.h"
#include "wx/module.h"
#include "wx/window.h"

#ifndef WX_PRECOMP
    #include <array>
    #include <cmath>
#endif

#include "wx/private/textmeasure.h"

#ifdef __WXMSW__
    #include "wx/msw/dcclient.h"
    #include "wx/msw/dcmemory.h"
    #include "wx/msw/dcscreen.h"
#endif

#ifdef __WXGTK3__
    #include "wx/gtk/dc.h"
#elif defined __WXGTK20__
    #include "wx/gtk/dcclient.h"
    #include "wx/gtk/dcmemory.h"
    #include "wx/gtk/dcscreen.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/dcclient.h"
    #include "wx/gtk1/dcmemory.h"
    #include "wx/gtk1/dcscreen.h"
#endif

#ifdef __WXMAC__
    #include "wx/osx/dcclient.h"
    #include "wx/osx/dcmemory.h"
    #include "wx/osx/dcscreen.h"
#endif

#ifdef __WXMOTIF__
    #include "wx/motif/dcclient.h"
    #include "wx/motif/dcmemory.h"
    #include "wx/motif/dcscreen.h"
#endif

#ifdef __WXX11__
    #include "wx/x11/dcclient.h"
    #include "wx/x11/dcmemory.h"
    #include "wx/x11/dcscreen.h"
#endif

#ifdef __WXDFB__
    #include "wx/dfb/dcclient.h"
    #include "wx/dfb/dcmemory.h"
    #include "wx/dfb/dcscreen.h"
#endif

#ifdef __WXQT__
    #include "wx/qt/dcclient.h"
    #include "wx/qt/dcmemory.h"
    #include "wx/qt/dcscreen.h"
#endif

//----------------------------------------------------------------------------
// wxDCFactory
//----------------------------------------------------------------------------

void wxDCFactory::Set(wxDCFactory *factory)
{
    delete m_factory;

    m_factory = factory;
}

wxDCFactory *wxDCFactory::Get()
{
    if ( !m_factory )
        m_factory = new wxNativeDCFactory;

    return m_factory;
}

class wxDCFactoryCleanupModule : public wxModule
{
public:
    bool OnInit() override { return true; }
    void OnExit() override { wxDCFactory::Set(nullptr); }

private:
    wxDECLARE_DYNAMIC_CLASS(wxDCFactoryCleanupModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxDCFactoryCleanupModule, wxModule);

//-----------------------------------------------------------------------------
// wxNativeDCFactory
//-----------------------------------------------------------------------------

std::unique_ptr<wxDCImpl> wxNativeDCFactory::CreateWindowDC( wxWindowDC *owner, wxWindow *window )
{
    auto impl = std::make_unique<wxWindowDCImpl>( owner, window );
    impl->InheritAttributes(window);
    return impl;
}

std::unique_ptr<wxDCImpl> wxNativeDCFactory::CreateClientDC( wxClientDC *owner, wxWindow *window )
{
    auto impl = std::make_unique<wxClientDCImpl>( owner, window );
    impl->InheritAttributes(window);
    return impl;
}

std::unique_ptr<wxDCImpl> wxNativeDCFactory::CreatePaintDC( wxPaintDC *owner, wxWindow *window )
{
    auto impl = std::make_unique<wxPaintDCImpl>( owner, window );
    impl->InheritAttributes(window);
    return impl;
}

std::unique_ptr<wxDCImpl> wxNativeDCFactory::CreateMemoryDC( wxMemoryDC *owner )
{
    return std::make_unique<wxMemoryDCImpl>( owner );
}

std::unique_ptr<wxDCImpl> wxNativeDCFactory::CreateMemoryDC(wxMemoryDC *owner, wxBitmap& bitmap)
{
    // the bitmap may be modified when it's selected into a memory DC so make
    // sure changing this bitmap doesn't affect any other shallow copies of it
    // (see wxMemoryDC::SelectObject())
    //
    // notice that we don't provide any ctor equivalent to SelectObjectAsSource
    // method because this should be rarely needed and easy to work around by
    // using the default ctor and calling SelectObjectAsSource itself
    if ( bitmap.IsOk() )
        bitmap.UnShare();

    return std::make_unique<wxMemoryDCImpl>(owner, bitmap);
}

std::unique_ptr<wxDCImpl> wxNativeDCFactory::CreateMemoryDC( wxMemoryDC *owner, wxDC *dc )
{
    return std::make_unique<wxMemoryDCImpl>( owner, dc );
}

std::unique_ptr<wxDCImpl> wxNativeDCFactory::CreateScreenDC( wxScreenDC *owner )
{
    return std::make_unique<wxScreenDCImpl>( owner );
}

#if wxUSE_PRINTING_ARCHITECTURE
std::unique_ptr<wxDCImpl> wxNativeDCFactory::CreatePrinterDC( wxPrinterDC *owner, const wxPrintData &data )
{
    wxPrintFactory *factory = wxPrintFactory::GetFactory();
    return factory->CreatePrinterDCImpl( owner, data );
}
#endif

//-----------------------------------------------------------------------------
// wxWindowDC
//-----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxWindowDC, wxDC);

wxWindowDC::wxWindowDC(wxWindow *win)
          : wxDC(wxDCFactory::Get()->CreateWindowDC(this, win))
{
}

//-----------------------------------------------------------------------------
// wxClientDC
//-----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxClientDC, wxWindowDC);

wxClientDC::wxClientDC(wxWindow *win)
          : wxWindowDC(wxDCFactory::Get()->CreateClientDC(this, win))
{
}

//-----------------------------------------------------------------------------
// wxMemoryDC
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxMemoryDC, wxDC);

wxMemoryDC::wxMemoryDC()
          : wxDC(wxDCFactory::Get()->CreateMemoryDC(this))
{
}

wxMemoryDC::wxMemoryDC(wxBitmap& bitmap)
          : wxDC(wxDCFactory::Get()->CreateMemoryDC(this, bitmap))
{
}

wxMemoryDC::wxMemoryDC(wxDC *dc)
          : wxDC(wxDCFactory::Get()->CreateMemoryDC(this, dc))
{
}

void wxMemoryDC::SelectObject(wxBitmap& bmp)
{
    if ( bmp.IsSameAs(GetSelectedBitmap()) )
    {
        // Nothing to do, this bitmap is already selected.
        return;
    }

    // make sure that the given wxBitmap is not sharing its data with other
    // wxBitmap instances as its contents will be modified by any drawing
    // operation done on this DC
    if (bmp.IsOk())
        bmp.UnShare();

    GetImpl()->DoSelect(bmp);
}

void wxMemoryDC::SelectObjectAsSource(const wxBitmap& bmp)
{
    GetImpl()->DoSelect(bmp);
}

const wxBitmap& wxMemoryDC::GetSelectedBitmap() const
{
    return GetImpl()->GetSelectedBitmap();
}

wxBitmap& wxMemoryDC::GetSelectedBitmap()
{
    return GetImpl()->GetSelectedBitmap();
}


//-----------------------------------------------------------------------------
// wxPaintDC
//-----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxPaintDC, wxClientDC);

wxPaintDC::wxPaintDC(wxWindow *win)
         : wxClientDC(wxDCFactory::Get()->CreatePaintDC(this, win))
{
}

//-----------------------------------------------------------------------------
// wxScreenDC
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxScreenDC, wxWindowDC);

wxScreenDC::wxScreenDC()
          : wxDC(wxDCFactory::Get()->CreateScreenDC(this))
{
}

//-----------------------------------------------------------------------------
// wxPrinterDC
//-----------------------------------------------------------------------------

#if wxUSE_PRINTING_ARCHITECTURE

wxIMPLEMENT_DYNAMIC_CLASS(wxPrinterDC, wxDC);

wxPrinterDC::wxPrinterDC()
           : wxDC(wxDCFactory::Get()->CreatePrinterDC(this, wxPrintData()))
{
}

wxPrinterDC::wxPrinterDC(const wxPrintData& data)
           : wxDC(wxDCFactory::Get()->CreatePrinterDC(this, data))
{
}

wxRect wxPrinterDC::GetPaperRect() const
{
    return GetImpl()->GetPaperRect();
}

int wxPrinterDC::GetResolution() const
{
    return GetImpl()->GetResolution();
}

#endif // wxUSE_PRINTING_ARCHITECTURE

//-----------------------------------------------------------------------------
// wxDCImpl
//-----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxDCImpl, wxObject);

wxDCImpl::wxDCImpl( wxDC *owner )
        : m_textForegroundColour(*wxBLACK)
        , m_textBackgroundColour(*wxWHITE)
        , m_owner(owner)
{
}

// ----------------------------------------------------------------------------
// clipping
// ----------------------------------------------------------------------------

void wxDCImpl::DoSetClippingRegion(wxCoord x, wxCoord y, wxCoord w, wxCoord h)
{
    wxASSERT_MSG( w >= 0 && h >= 0,
                  wxS("Clipping box size values cannot be negative") );

    wxRect clipRegion(x, y, w, h);

    if ( m_clipping )
    {
        // New clipping box is an intersection
        // of required clipping box and the current one.
        wxRect curRegion(m_clipX1, m_clipY1, m_clipX2 - m_clipX1, m_clipY2 - m_clipY1);
        clipRegion.Intersect(curRegion);
    }
    else
    {
        // Effective clipping box is an intersection
        // of required clipping box and DC surface.
        wxSize dcSize = DoGetSize();
        wxRect dcRect(DeviceToLogicalX(0), DeviceToLogicalY(0),
                      DeviceToLogicalXRel(dcSize.x), DeviceToLogicalYRel(dcSize.y));
        clipRegion.Intersect(dcRect);

        m_clipping = true;
    }

    if ( clipRegion.IsEmpty() )
    {
        m_clipX1 = 0;
        m_clipY1 = 0;
        m_clipX2 = 0;
        m_clipY2 = 0;
    }
    else
    {
        m_clipX1 = clipRegion.GetLeft();
        m_clipY1 = clipRegion.GetTop();
        m_clipX2 = clipRegion.GetRight() + 1;
        m_clipY2 = clipRegion.GetBottom() + 1;
    }
}

wxRect wxDCImpl::GetLogicalArea() const
{
    const wxSize size = GetSize();
    return {DeviceToLogicalX(0),
            DeviceToLogicalY(0),
            DeviceToLogicalXRel(size.x),
            DeviceToLogicalYRel(size.y)};
}

bool wxDCImpl::DoGetClippingRect(wxRect& rect) const
{
    if ( m_clipping )
    {
        rect = wxRect(m_clipX1,
                      m_clipY1,
                      m_clipX2 - m_clipX1,
                      m_clipY2 - m_clipY1);

        return true;
    }
    else // No active clipping region.
    {
        rect = GetLogicalArea();

        return false;
    }
}

// ----------------------------------------------------------------------------
// coordinate conversions and transforms
// ----------------------------------------------------------------------------

wxCoord wxDCImpl::DeviceToLogicalX(wxCoord x) const
{
    return std::lround( (double)((x - m_deviceOrigin.x - m_deviceLocalOrigin.x) * m_signX) / m_scale.x ) + m_logicalOrigin.x ;
}

wxCoord wxDCImpl::DeviceToLogicalY(wxCoord y) const
{
    return std::lround( (double)((y - m_deviceOrigin.y - m_deviceLocalOrigin.y) * m_signY) / m_scale.y ) + m_logicalOrigin.y ;
}

wxCoord wxDCImpl::DeviceToLogicalXRel(wxCoord x) const
{
    return std::lround((double)(x) / m_scale.x);
}

wxCoord wxDCImpl::DeviceToLogicalYRel(wxCoord y) const
{
    return std::lround((double)(y) / m_scale.y);
}

wxCoord wxDCImpl::LogicalToDeviceX(wxCoord x) const
{
    return std::lround( (double)((x - m_logicalOrigin.x) * m_signX) * m_scale.x) + m_deviceOrigin.x + m_deviceLocalOrigin.x;
}

wxCoord wxDCImpl::LogicalToDeviceY(wxCoord y) const
{
    return std::lround( (double)((y - m_logicalOrigin.y) * m_signY) * m_scale.y) + m_deviceOrigin.y + m_deviceLocalOrigin.y;
}

wxCoord wxDCImpl::LogicalToDeviceXRel(wxCoord x) const
{
    return std::lround((double)(x) * m_scale.x);
}

wxCoord wxDCImpl::LogicalToDeviceYRel(wxCoord y) const
{
    return std::lround((double)(y) * m_scale.y);
}

wxPoint wxDCImpl::DeviceToLogical(wxCoord x, wxCoord y) const
{
    return {DeviceToLogicalX(x), DeviceToLogicalY(y)};
}

wxPoint wxDCImpl::LogicalToDevice(wxCoord x, wxCoord y) const
{
    return {LogicalToDeviceX(x), LogicalToDeviceY(y)};
}

wxSize wxDCImpl::DeviceToLogicalRel(int x, int y) const
{
    return {DeviceToLogicalXRel(x), DeviceToLogicalYRel(y)};
}

wxSize wxDCImpl::LogicalToDeviceRel(int x, int y) const
{
    return {LogicalToDeviceXRel(x), LogicalToDeviceYRel(y)};
}

void wxDCImpl::ComputeScaleAndOrigin()
{
    m_scale.x = m_logicalScale.x * m_userScale.x;
    m_scale.y = m_logicalScale.y * m_userScale.y;
}

void wxDCImpl::SetMapMode( wxMappingMode mode )
{
    switch (mode)
    {
        case wxMappingMode::Twips:
          SetLogicalScale( {twips2mm*GetMMToPXx(), twips2mm*GetMMToPXy()} );
          break;
        case wxMappingMode::Points:
          SetLogicalScale( {pt2mm*GetMMToPXx(), pt2mm*GetMMToPXy()} );
          break;
        case wxMappingMode::Metric:
          SetLogicalScale( {GetMMToPXx(), GetMMToPXy()} );
          break;
        case wxMappingMode::LoMetric:
          SetLogicalScale( {GetMMToPXx()/10.0, GetMMToPXy()/10.0} );
          break;
        default:
        case wxMappingMode::Text:
          SetLogicalScale(1.0);
          break;
    }
    m_mappingMode = mode;
}

void wxDCImpl::SetUserScale(double userScale)
{
    SetUserScale({ userScale, userScale });
}

void wxDCImpl::SetUserScale(wxScale userScale)
{
    // allow negative ? -> no
    m_userScale = userScale;
    ComputeScaleAndOrigin();
}

void wxDCImpl::SetLogicalScale(double logicalScale)
{
    SetLogicalScale(wxScale{ logicalScale, logicalScale });
}

void wxDCImpl::SetLogicalScale(wxScale logicalScale)
{
    // allow negative ?
    m_logicalScale = logicalScale;
    ComputeScaleAndOrigin();
}

void wxDCImpl::SetLogicalOrigin( wxPoint logicalOrigin )
{
    m_logicalOrigin = {logicalOrigin.x * m_signX, logicalOrigin.y * m_signY};
    ComputeScaleAndOrigin();
}

void wxDCImpl::SetDeviceOrigin( wxPoint deviceOrigin )
{
    m_deviceOrigin = deviceOrigin;
    ComputeScaleAndOrigin();
}

void wxDCImpl::SetDeviceLocalOrigin( wxPoint deviceLocalOrigin )
{
    m_deviceLocalOrigin = deviceLocalOrigin;
    ComputeScaleAndOrigin();
}

void wxDCImpl::SetAxisOrientation( bool xLeftRight, bool yBottomUp )
{
    // only wxPostScripDC has m_signX = -1, we override SetAxisOrientation there
    // wxWidgets 2.9: no longer override it
    m_signX = (xLeftRight ?  1 : -1);
    m_signY = (yBottomUp  ? -1 :  1);
    ComputeScaleAndOrigin();
}

std::vector<int> wxDCImpl::DoGetPartialTextExtents(std::string_view text) const
{
    wxTextMeasure tm(GetOwner(), &m_font);
    return tm.GetPartialTextExtents(text, m_scale.x);
}

void wxDCImpl::GetMultiLineTextExtent(std::string_view text,
                                      wxCoord *x,
                                      wxCoord *y,
                                      wxCoord *h,
                                      const wxFont *font) const
{
    wxTextMeasure tm(GetOwner(), font && font->IsOk() ? font : &m_font);
    tm.GetMultiLineTextExtent(text, x, y, h);
}

void wxDCImpl::DoDrawCheckMark(wxCoord x1, wxCoord y1,
                               wxCoord width, wxCoord height)
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    const wxCoord x2 = x1 + width;
    const wxCoord y2 = y1 + height;

    // the pen width is calibrated to give 3 for width == height == 10
    wxDCPenChanger pen( *m_owner, wxPen(GetTextForeground(), (width + height + 1)/7));

    // we're drawing a scaled version of wx/generic/tick.xpm here
    const wxCoord x3 = x1 + (4 * width) / 10;   // x of the tick bottom
    const wxCoord y3 = y1 + height / 2;       // y of the left tick branch
    DoDrawLine(x1, y3, x3, y2);
    DoDrawLine(x3, y2, x2, y1);

    CalcBoundingBox(x1, y1);
    CalcBoundingBox(x2, y2);
}

bool
wxDCImpl::DoStretchBlit(wxCoord xdest, wxCoord ydest,
                        wxCoord dstWidth, wxCoord dstHeight,
                        wxDC *source,
                        wxCoord xsrc, wxCoord ysrc,
                        wxCoord srcWidth, wxCoord srcHeight,
                        wxRasterOperationMode rop,
                        bool useMask,
                        wxCoord xsrcMask,
                        wxCoord ysrcMask)
{
    wxCHECK_MSG( srcWidth && srcHeight && dstWidth && dstHeight, false,
                 wxT("invalid blit size") );

    // emulate the stretching by modifying the DC scale
    const double xscale = (double)srcWidth / dstWidth;
    const double yscale = (double)srcHeight/dstHeight;

    // Shift origin to avoid imprecision of integer destination coordinates
    const int deviceOriginX = m_deviceOrigin.x;
    const int deviceOriginY = m_deviceOrigin.y;
    const int deviceLocalOriginX = m_deviceLocalOrigin.x;
    const int deviceLocalOriginY = m_deviceLocalOrigin.y;
    const int logicalOriginX = m_logicalOrigin.x;
    const int logicalOriginY = m_logicalOrigin.y;
    m_deviceOrigin.x = LogicalToDeviceX(xdest);
    m_deviceOrigin.y = LogicalToDeviceY(ydest);
    m_deviceLocalOrigin.x = 0;
    m_deviceLocalOrigin.y = 0;
    m_logicalOrigin.x = 0;
    m_logicalOrigin.y = 0;

    wxScale scaleOld = GetUserScale();
    SetUserScale({scaleOld.x / xscale, scaleOld.y / yscale});

    const bool rc = DoBlit(0, 0, srcWidth, srcHeight,
                     source,
                     xsrc, ysrc, rop, useMask, xsrcMask, ysrcMask);

    m_deviceOrigin.x = deviceOriginX;
    m_deviceOrigin.y = deviceOriginY;
    m_deviceLocalOrigin.x = deviceLocalOriginX;
    m_deviceLocalOrigin.y = deviceLocalOriginY;
    m_logicalOrigin.x = logicalOriginX;
    m_logicalOrigin.y = logicalOriginY;
    SetUserScale(scaleOld);

    return rc;
}

void wxDCImpl::DrawLines(const wxPointList *list, wxCoord xoffset, wxCoord yoffset)
{
    int n = list->GetCount();

    std::vector<wxPoint> points(n);

    int i = 0;
    for ( wxPointList::compatibility_iterator node = list->GetFirst(); node; node = node->GetNext(), i++ )
    {
        wxPoint *point = node->GetData();
        points[i].x = point->x;
        points[i].y = point->y;
    }

    DoDrawLines(n, points.data(), xoffset, yoffset);
}

void wxDCImpl::DrawPolygon(const wxPointList *list,
                           wxCoord xoffset, wxCoord yoffset,
                           wxPolygonFillMode fillStyle)
{
    const int n = list->GetCount();
    std::vector<wxPoint> points(n);

    int i = 0;
    for ( wxPointList::compatibility_iterator node = list->GetFirst(); node; node = node->GetNext(), i++ )
    {
        wxPoint *point = node->GetData();
        points[i].x = point->x;
        points[i].y = point->y;
    }

    DoDrawPolygon(n, points.data(), xoffset, yoffset, fillStyle);
}

void
wxDCImpl::DoDrawPolyPolygon(int n,
                            const int count[],
                            const wxPoint points[],
                            wxCoord xoffset, wxCoord yoffset,
                            wxPolygonFillMode fillStyle)
{
    if ( n == 1 )
    {
        DoDrawPolygon(count[0], points, xoffset, yoffset, fillStyle);
        return;
    }

    int      i, j, lastOfs;

    for (i = j = lastOfs = 0; i < n; i++)
    {
        lastOfs = j;
        j      += count[i];
    }

    std::vector<wxPoint> pts(j + n - 1);
    
    for (i = 0; i < j; i++)
        pts[i] = points[i];
    for (i = 2; i <= n; i++)
    {
        lastOfs -= count[n-i];
        pts[j++] = pts[lastOfs];
    }

    {
        wxDCPenChanger setTransp(*m_owner, *wxTRANSPARENT_PEN);
        DoDrawPolygon(j, pts.data(), xoffset, yoffset, fillStyle);
    }

    for (i = j = 0; i < n; i++)
    {
        // FIXME: Pointer arithmetic 
        DoDrawLines(count[i], pts.data() + j, xoffset, yoffset);
        j += count[i];
    }
}

#if wxUSE_SPLINES

// TODO: Use wxPoint.
void wxDCImpl::DrawSpline(wxPoint pt1,
                          wxPoint pt2,
                          wxPoint pt3)
{
    std::array<wxPoint, 3> points = { pt1, pt2, pt3 };
    // TODO: Use span
    DrawSpline(points.size(), points.data());
}

void wxDCImpl::DrawSpline(int n, const wxPoint points[])
{
    wxPointList list;
    for ( int i = 0; i < n; i++ )
        list.Append(const_cast<wxPoint*>(&points[i]));

    DrawSpline(&list);
}

// ----------------------------------- spline code ----------------------------------------

static
void wx_quadratic_spline(double a1, double b1, double a2, double b2,
                         double a3, double b3, double a4, double b4);
static
void wx_clear_stack();
static
int wx_spline_pop(double *x1, double *y1, double *x2, double *y2, double *x3,
        double *y3, double *x4, double *y4);
static
void wx_spline_push(double x1, double y1, double x2, double y2, double x3, double y3,
          double x4, double y4);
static bool wx_spline_add_point(double x, double y);
static void wx_spline_draw_point_array(wxDC *dc);

static wxPointList wx_spline_point_list;

#define                half(z1, z2)        ((z1+z2)/2.0)
constexpr int THRESHOLD = 5;

/* iterative version */

void wx_quadratic_spline(double a1, double b1, double a2, double b2, double a3, double b3, double a4,
                 double b4)
{
    double x1, y1, x2, y2, x3, y3, x4, y4;

    wx_clear_stack();
    wx_spline_push(a1, b1, a2, b2, a3, b3, a4, b4);

    while (wx_spline_pop(&x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4)) {
        double xmid, ymid;
        xmid = (double)half(x2, x3);
        ymid = (double)half(y2, y3);
        if (std::fabs(x1 - xmid) < THRESHOLD && std::fabs(y1 - ymid) < THRESHOLD &&
            std::fabs(xmid - x4) < THRESHOLD && std::fabs(ymid - y4) < THRESHOLD) {
            wx_spline_add_point( x1, y1 );
            wx_spline_add_point( xmid, ymid );
        } else {
            wx_spline_push(xmid, ymid, (double)half(xmid, x3), (double)half(ymid, y3),
                 (double)half(x3, x4), (double)half(y3, y4), x4, y4);
            wx_spline_push(x1, y1, (double)half(x1, x2), (double)half(y1, y2),
                 (double)half(x2, xmid), (double)half(y2, ymid), xmid, ymid);
        }
    }
}

/* utilities used by spline drawing routines */

typedef struct wx_spline_stack_struct {
    double           x1, y1, x2, y2, x3, y3, x4, y4;
} Stack;

constexpr int SPLINE_STACK_DEPTH = 20;
static Stack    wx_spline_stack[SPLINE_STACK_DEPTH];
static Stack   *wx_stack_top;
static int      wx_stack_count;

void wx_clear_stack()
{
    wx_stack_top = wx_spline_stack;
    wx_stack_count = 0;
}

void wx_spline_push(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
    wx_stack_top->x1 = x1;
    wx_stack_top->y1 = y1;
    wx_stack_top->x2 = x2;
    wx_stack_top->y2 = y2;
    wx_stack_top->x3 = x3;
    wx_stack_top->y3 = y3;
    wx_stack_top->x4 = x4;
    wx_stack_top->y4 = y4;
    wx_stack_top++;
    wx_stack_count++;
}

int wx_spline_pop(double *x1, double *y1, double *x2, double *y2,
                  double *x3, double *y3, double *x4, double *y4)
{
    if (wx_stack_count == 0)
        return (0);
    wx_stack_top--;
    wx_stack_count--;
    *x1 = wx_stack_top->x1;
    *y1 = wx_stack_top->y1;
    *x2 = wx_stack_top->x2;
    *y2 = wx_stack_top->y2;
    *x3 = wx_stack_top->x3;
    *y3 = wx_stack_top->y3;
    *x4 = wx_stack_top->x4;
    *y4 = wx_stack_top->y4;
    return (1);
}

static bool wx_spline_add_point(double x, double y)
{
    wxPoint *point = new wxPoint( std::lround(x), std::lround(y) );
    wx_spline_point_list.Append(point );
    return true;
}

static void wx_spline_draw_point_array(wxDC *dc)
{
    dc->DrawLines(&wx_spline_point_list, 0, 0 );
    wxPointList::compatibility_iterator node = wx_spline_point_list.GetFirst();
    while (node)
    {
        wxPoint *point = node->GetData();
        delete point;
        wx_spline_point_list.Erase(node);
        node = wx_spline_point_list.GetFirst();
    }
}

void wxDCImpl::DoDrawSpline( const wxPointList *points )
{
    wxCHECK_RET( IsOk(), wxT("invalid window dc") );

    wxPointList::compatibility_iterator node = points->GetFirst();
    if (!node)
        // empty list
        return;

    const wxPoint* p = node->GetData();

    double x1 = static_cast<double>(p->x);
    double y1 = static_cast<double>(p->y);

    node = node->GetNext();
    p = node->GetData();

    double x2 = static_cast<double>(p->x);
    double y2 = static_cast<double>(p->y);
    double cx1 = (x1 + x2) / 2.0;
    double cy1 = (y1 + y2) / 2.0;
    double cx2 = (cx1 + x2) / 2.0;
    double cy2 = (cy1 + y2) / 2.0;

    wx_spline_add_point(x1, y1);

    while ((node = node->GetNext()))
    {
        p = node->GetData();
        x1 = x2;
        y1 = y2;
        x2 = p->x;
        y2 = p->y;
        double cx4 = (x1 + x2) / 2.0;
        double cy4 = (y1 + y2) / 2.0;
        double cx3 = (x1 + cx4) / 2.0;
        double cy3 = (y1 + cy4) / 2.0;

        wx_quadratic_spline(cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4);

        cx1 = cx4;
        cy1 = cy4;
        cx2 = (double)(cx1 + x2) / 2;
        cy2 = (double)(cy1 + y2) / 2;
    }

    wx_spline_add_point( cx1, cy1 );
    wx_spline_add_point( x2, y2 );

    wx_spline_draw_point_array( m_owner );
}

#endif // wxUSE_SPLINES



void wxDCImpl::DoGradientFillLinear(const wxRect& rect,
                                    const wxColour& initialColour,
                                    const wxColour& destColour,
                                    wxDirection nDirection)
{
    // save old pen
    wxPen oldPen = m_pen;
    wxBrush oldBrush = m_brush;

    std::uint8_t nR1 = initialColour.Red();
    std::uint8_t nG1 = initialColour.Green();
    std::uint8_t nB1 = initialColour.Blue();
    std::uint8_t nR2 = destColour.Red();
    std::uint8_t nG2 = destColour.Green();
    std::uint8_t nB2 = destColour.Blue();
    std::uint8_t nR, nG, nB;

    if ( nDirection == wxEAST || nDirection == wxWEST )
    {
        std::int32_t x = rect.GetWidth();
        std::int32_t w = x;              // width of area to shade
        std::int32_t xDelta = w/256;     // height of one shade bend
        if (xDelta < 1)
            xDelta = 1;

        while (x >= xDelta)
        {
            x -= xDelta;
            if (nR1 > nR2)
                nR = nR1 - (nR1-nR2)*(w-x)/w;
            else
                nR = nR1 + (nR2-nR1)*(w-x)/w;

            if (nG1 > nG2)
                nG = nG1 - (nG1-nG2)*(w-x)/w;
            else
                nG = nG1 + (nG2-nG1)*(w-x)/w;

            if (nB1 > nB2)
                nB = nB1 - (nB1-nB2)*(w-x)/w;
            else
                nB = nB1 + (nB2-nB1)*(w-x)/w;

            wxColour colour(nR,nG,nB);
            SetPen(wxPen(colour, 1, wxPenStyle::Solid));
            SetBrush(wxBrush(colour));
            if(nDirection == wxEAST)
                DoDrawRectangle(rect.GetRight()-x-xDelta+1, rect.GetTop(),
                        xDelta, rect.GetHeight());
            else //nDirection == wxWEST
                DoDrawRectangle(rect.GetLeft()+x, rect.GetTop(),
                        xDelta, rect.GetHeight());
        }
    }
    else  // nDirection == wxNORTH || nDirection == wxSOUTH
    {
        std::int32_t y = rect.GetHeight();
        std::int32_t w = y;              // height of area to shade
        std::int32_t yDelta = w/255;     // height of one shade bend
        if (yDelta < 1)
            yDelta = 1;

        while (y > 0)
        {
            y -= yDelta;
            if (nR1 > nR2)
                nR = nR1 - (nR1-nR2)*(w-y)/w;
            else
                nR = nR1 + (nR2-nR1)*(w-y)/w;

            if (nG1 > nG2)
                nG = nG1 - (nG1-nG2)*(w-y)/w;
            else
                nG = nG1 + (nG2-nG1)*(w-y)/w;

            if (nB1 > nB2)
                nB = nB1 - (nB1-nB2)*(w-y)/w;
            else
                nB = nB1 + (nB2-nB1)*(w-y)/w;

            wxColour colour(nR,nG,nB);
            SetPen(wxPen(colour, 1, wxPenStyle::Solid));
            SetBrush(wxBrush(colour));
            if(nDirection == wxNORTH)
                DoDrawRectangle(rect.GetLeft(), rect.GetTop()+y,
                        rect.GetWidth(), yDelta);
            else //nDirection == wxSOUTH
                DoDrawRectangle(rect.GetLeft(), rect.GetBottom()-y-yDelta+1,
                        rect.GetWidth(), yDelta);
        }
    }

    SetPen(oldPen);
    SetBrush(oldBrush);
}

void wxDCImpl::DoGradientFillConcentric(const wxRect& rect,
                                      const wxColour& initialColour,
                                      const wxColour& destColour,
                                      const wxPoint& circleCenter)
{
    // save the old pen and ensure it is restored on exit
    const wxPen penOrig = m_pen;
    wxON_BLOCK_EXIT_SET(m_pen, penOrig);

    const std::uint8_t nR1 = destColour.Red();
    const std::uint8_t nG1 = destColour.Green();
    const std::uint8_t nB1 = destColour.Blue();
    const std::uint8_t nR2 = initialColour.Red();
    const std::uint8_t nG2 = initialColour.Green();
    const std::uint8_t nB2 = initialColour.Blue();

    //Radius
    double cx = rect.GetWidth() / 2;
    double cy = rect.GetHeight() / 2;

    double dRadius;
    if (cx < cy)
        dRadius = cx;
    else
        dRadius = cy;

    //Offset of circle
    const double ptX = circleCenter.x;
    const double ptY = circleCenter.y;
    const double nCircleOffX = ptX - cx;
    const double nCircleOffY = ptY - cy;

    for ( std::int32_t x = 0; x < rect.GetWidth(); x++ )
    {
        for ( std::int32_t y = 0; y < rect.GetHeight(); y++ )
        {
            //get color difference
            const double dx = x;
            const double dy = y;

            double dGradient = ((dRadius - std::hypot({dx - cx - nCircleOffX}, {dy - cy - nCircleOffY})) * 100) / dRadius;

            //normalize Gradient
            if (dGradient < 0)
                dGradient = 0.0;

            //get dest colors
            const std::uint8_t nR = (std::uint8_t)(nR1 + ((nR2 - nR1) * dGradient / 100));
            const std::uint8_t nG = (std::uint8_t)(nG1 + ((nG2 - nG1) * dGradient / 100));
            const std::uint8_t nB = (std::uint8_t)(nB1 + ((nB2 - nB1) * dGradient / 100));

            //set the pixel
            SetPen(wxColour(nR,nG,nB));
            DoDrawPoint(x + rect.GetLeft(), y + rect.GetTop());
        }
    }
}

void wxDCImpl::InheritAttributes(wxWindow *win)
{
    wxCHECK_RET( win, "window can't be NULL" );

    SetFont(win->GetFont());
    SetTextForeground(win->GetForegroundColour());
    SetTextBackground(win->GetBackgroundColour());
    SetBackground(win->GetBackgroundColour());
    SetLayoutDirection(win->GetLayoutDirection());
}

void wxDCImpl::DoGetFontMetrics(int *height,
                                int *ascent,
                                int *descent,
                                int *internalLeading,
                                int *externalLeading,
                                int *averageWidth) const
{
    // Average width is typically the same as width of 'x'.
    wxCoord d;
    auto textExtents = DoGetTextExtent("x", &d, externalLeading);

    if(averageWidth)
        *averageWidth = textExtents.x;

    if ( height )
        *height = textExtents.y;
    if ( ascent )
        *ascent = textExtents.y - d;
    if ( descent )
        *descent = d;
    if ( internalLeading )
        *internalLeading = 0;
}

//-----------------------------------------------------------------------------
// wxDC
//-----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxDC, wxObject);

void wxDC::CopyAttributes(const wxDC& dc)
{
    SetFont(dc.GetFont());
    SetTextForeground(dc.GetTextForeground());
    SetTextBackground(dc.GetTextBackground());
    SetBackground(dc.GetBackground());
    SetLayoutDirection(dc.GetLayoutDirection());
}

void wxDC::DrawLabel(std::string_view text,
                         const wxBitmap& bitmap,
                         const wxRect& rect,
                         unsigned int alignment,
                         int indexAccel,
                         wxRect *rectBounding)
{
    // find the text position
    wxCoord widthText, heightText, heightLine;
    GetMultiLineTextExtent(text, &widthText, &heightText, &heightLine);

    wxCoord width, height;
    if ( bitmap.IsOk() )
    {
        width = widthText + bitmap.GetWidth();
        height = bitmap.GetHeight();
    }
    else // no bitmap
    {
        width = widthText;
        height = heightText;
    }

    wxCoord x, y;
    if ( alignment & wxALIGN_RIGHT )
    {
        x = rect.GetRight() - width;
    }
    else if ( alignment & wxALIGN_CENTRE_HORIZONTAL )
    {
        x = (rect.GetLeft() + rect.GetRight() + 1 - width) / 2;
    }
    else // alignment & wxALIGN_LEFT
    {
        x = rect.GetLeft();
    }

    if ( alignment & wxALIGN_BOTTOM )
    {
        y = rect.GetBottom() - height;
    }
    else if ( alignment & wxALIGN_CENTRE_VERTICAL )
    {
        y = (rect.GetTop() + rect.GetBottom() + 1 - height) / 2;
    }
    else // alignment & wxALIGN_TOP
    {
        y = rect.GetTop();
    }

    // draw the bitmap first
    const wxCoord x0 = x;
    const wxCoord y0 = y;
    const wxCoord width0 = width;

    if ( bitmap.IsOk() )
    {
        DrawBitmap(bitmap, x, y, true /* use mask */);

        const wxCoord offset = bitmap.GetWidth() + 4;
        x += offset;
        width -= offset;

        y += (height - heightText) / 2;
    }

    // we will draw the underscore under the accel char later
    wxCoord startUnderscore = 0,
            endUnderscore = 0,
            yUnderscore = 0;

    // split the string into lines and draw each of them separately
    //
    // NB: while wxDC::wxDrawText() on some platforms supports drawing multi-line
    //     strings natively, this is not the case for all of them, notably not
    //     wxMSW which uses this function for multi-line texts, so we may only
    //     call wxDrawText() for single-line strings from here to avoid infinite
    //     recursion.
    std::string curLine;
    for ( std::string_view::const_iterator pc = text.begin(); ; ++pc )
    {
        if ( pc == text.end() || *pc == '\n' )
        {
            int xRealStart = x; // init it here to avoid compielr warnings

            if ( !curLine.empty() )
            {
                // NB: can't test for !(alignment & wxALIGN_LEFT) because
                //     wxALIGN_LEFT is 0
                if ( alignment & (wxALIGN_RIGHT | wxALIGN_CENTRE_HORIZONTAL) )
                {
                    wxCoord widthLine = GetTextExtent(curLine).x;

                    if ( alignment & wxALIGN_RIGHT )
                    {
                        xRealStart += width - widthLine;
                    }
                    else // if ( alignment & wxALIGN_CENTRE_HORIZONTAL )
                    {
                        xRealStart += (width - widthLine) / 2;
                    }
                }
                //else: left aligned, nothing to do

                wxDrawText(curLine, wxPoint{xRealStart, y});
            }

            y += heightLine;

            // do we have underscore in this line? we can check yUnderscore
            // because it is set below to just y + heightLine if we do
            if ( y == yUnderscore )
            {
                // adjust the horz positions to account for the shift
                startUnderscore += xRealStart;
                endUnderscore += xRealStart;
            }

            if ( pc == text.end() )
                break;

            curLine.clear();
        }
        else // not end of line
        {
            if ( pc - text.begin() == indexAccel )
            {
                // remember to draw underscore here
                startUnderscore = GetTextExtent(curLine).x;
                curLine += *pc;
                endUnderscore = GetTextExtent(curLine).x;

                yUnderscore = y + heightLine;
            }
            else
            {
                curLine += *pc;
            }
        }
    }

    // draw the underscore if found
    if ( startUnderscore != endUnderscore )
    {
        // it should be of the same colour as text
        SetPen(wxPen(GetTextForeground(), 0, wxPenStyle::Solid));

        // This adjustment is relatively arbitrary: we need to draw the
        // underline slightly higher to avoid overflowing the character cell
        // but whether we should do it 1, 2 or 3 pixels higher is not clear.
        //
        // The currently used value seems to be compatible with native MSW
        // behaviour, i.e. it results in the same appearance of the owner-drawn
        // and normal labels.
        yUnderscore -= 2;

        DrawLine(startUnderscore, yUnderscore, endUnderscore, yUnderscore);
    }

    // return bounding rect if requested
    if ( rectBounding )
    {
        *rectBounding = wxRect(x, y - heightText, widthText, heightText);
    }

    CalcBoundingBox(x0, y0);
    CalcBoundingBox(x0 + width0, y0 + height);
}

/*
Notes for wxWidgets DrawEllipticArcRot(...)

wxDCBase::DrawEllipticArcRot(...) draws a rotated elliptic arc or an ellipse.
It uses wxDCBase::CalculateEllipticPoints(...) and wxDCBase::Rotate(...),
which are also new.

All methods are generic, so they can be implemented in wxDCBase.

CalculateEllipticPoints(...) fills a given list of wxPoints with some points
of an elliptic arc. The algorithm is pixel-based: In every row (in flat
parts) or every column (in steep parts) only one pixel is calculated.
Trigonometric calculation (sin, cos, tan, atan) is only done if the
starting angle is not equal to the ending angle. The calculation of the
pixels is done using simple arithmetic only and should perform not too
bad even on devices without floating point processor. I didn't test this yet.

Rotate(...) rotates a list of point pixel-based, you will see rounding errors.
For instance: an ellipse rotated 180 degrees is drawn
slightly different from the original.

The points are then moved to an array and used to draw a polyline and/or polygon
(with center added, the pie).
The result looks quite similar to the native ellipse, only e few pixels differ.

The performance on a desktop system (Athlon 1800, WinXP) is about 7 times
slower as DrawEllipse(...), which calls the native API.
An rotated ellipse outside the clipping region takes nearly the same time,
while an native ellipse outside takes nearly no time to draw.

If you draw an arc with this new method, you will see the starting and ending angles
are calculated properly.
If you use DrawEllipticArc(...), you will see they are only correct for circles
and not properly calculated for ellipses.

Peter Lenhard
p.lenhard@t-online.de
*/

float wxDCImpl::GetFontPointSizeAdjustment(float dpi)
{
    // wxMSW has long-standing bug where wxFont point size is interpreted as
    // "pixel size corresponding to given point size *on screen*". In other
    // words, on a typical 600dpi printer and a typical 96dpi screen, fonts
    // are ~6 times smaller when printing. Unfortunately, this bug is so severe
    // that *all* printing code has to account for it and consequently, other
    // ports need to emulate this bug too:
    const wxSize screenPPI = wxGetDisplayPPI();
    return float(screenPPI.y) / dpi;
}

double wxDCImpl::GetMMToPXx() const
{
    // FIXME: Double equality
    if ( m_mm_to_pix_x == 0.0 )
    {
        m_mm_to_pix_x = (double)wxGetDisplaySize().x /
                        (double)wxGetDisplaySizeMM().x;
    }

    return m_mm_to_pix_x;
}

double wxDCImpl::GetMMToPXy() const
{
    // FIXME: Double equality
    if ( m_mm_to_pix_y == 0.0 )
    {
        m_mm_to_pix_y = (double)wxGetDisplaySize().y /
                        (double)wxGetDisplaySizeMM().y;
    }

    return m_mm_to_pix_y;
}
