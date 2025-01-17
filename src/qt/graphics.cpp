/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/graphics.cpp
// Purpose:     Graphics context methods for the Qt platform
// Author:      Jay Nabonne
// Created:     2018-12-13
// Copyright:   (c) Jay Nabonne
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_GRAPHICS_CONTEXT

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPainterPath>
#include <QPicture>

#ifndef WX_PRECOMP
    #include "wx/bitmap.h"
    #include "wx/icon.h"
    #include "wx/dcclient.h"
    #include "wx/dcmemory.h"
    #include "wx/dcprint.h"
    #include "wx/window.h"
#endif

#include "wx/graphics.h"

#include "wx/tokenzr.h"

#include "wx/private/graphics.h"

namespace
{

// Ensure that the given painter is active by calling begin() if it isn't. If
// it already is, don't do anything.
class EnsurePainterIsActive
{
public:
    explicit EnsurePainterIsActive(QPainter* painter)
        : m_painter(painter),
          m_wasActive(painter->isActive())
    {
        if ( !m_wasActive )
            m_painter->begin(&m_picture);
    }

    ~EnsurePainterIsActive()
    {
        if ( !m_wasActive )
            m_painter->end();
    }

private:
    QPainter* m_painter;
    QPicture m_picture;
    bool m_wasActive;

    EnsurePainterIsActive(const EnsurePainterIsActive&) = delete;
	EnsurePainterIsActive& operator=(const EnsurePainterIsActive&) = delete;
};

} // anonymous namespace

class wxQtBrushData : public wxGraphicsObjectRefData
{
public:
    wxQtBrushData(wxGraphicsRenderer* renderer)
        : wxGraphicsObjectRefData(renderer)
    {
    }

    wxQtBrushData(wxGraphicsRenderer* renderer, const wxBrush& wxbrush)
        : wxGraphicsObjectRefData(renderer),
          m_brush(wxbrush.GetHandle())
    {
    }

    void CreateLinearGradientBrush(double x1, double y1,
                                   double x2, double y2,
                                   const wxGraphicsGradientStops& stops)
    {
        QLinearGradient gradient(x1, y1, x2, y2);
        SetStops(gradient, stops);
        m_brush = QBrush(gradient);
    }

    void CreateRadialGradientBrush(double startX, double startY,
                                   double endX, double endY,
                                   double radius,
                                   const wxGraphicsGradientStops& stops)
    {
        QRadialGradient gradient(QPointF(endX, endY), radius, QPointF(startX, startY));
        SetStops(gradient, stops);
        m_brush = QBrush(gradient);
    }

    const QBrush& getBrush() const
    {
        return m_brush;
    }

private:
    static void
    SetStops(QGradient& gradient, const wxGraphicsGradientStops& stops)
    {
        QGradientStops qstops;
        for ( size_t i = 0; i < stops.GetCount(); ++i )
        {
            const wxGraphicsGradientStop stop = stops.Item(i);
            qstops.append(QGradientStop(stop.GetPosition(),
                                        stop.GetColour().GetQColor()));
        }

        gradient.setStops(qstops);
    }

    QBrush m_brush;

    wxQtBrushData(const wxQtBrushData&) = delete;
	wxQtBrushData& operator=(const wxQtBrushData&) = delete;
};

class wxQtPenData : public wxGraphicsObjectRefData
{
public:
    wxQtPenData(wxGraphicsRenderer* renderer, const wxGraphicsPenInfo& info)
        : wxGraphicsObjectRefData(renderer),
          m_pen(CreatePenFromInfo(info))
    {
    }

    const QPen& GetPen() const
    {
        return m_pen;
    }

private:
    static QPen CreatePenFromInfo(const wxGraphicsPenInfo& info)
    {
        wxPen wxpen(info.GetColour(), info.GetWidth(), info.GetStyle());
        wxpen.SetDashes(info.GetDashCount(), info.GetDash());
        wxpen.SetJoin(info.GetJoin());
        wxpen.SetCap(info.GetCap());
        switch ( info.GetStyle() )
        {
            case wxPenStyle::Stipple:
            case wxPenStyle::StippleMask:
            case wxPenStyle::StippleMaskOpaque:
            {
                wxpen.SetStipple(info.GetStipple());
                break;
            }
            default:
                break;
        }

        return wxpen.GetHandle();
    }
    QPen m_pen;

    wxQtPenData(const wxQtPenData&) = delete;
	wxQtPenData& operator=(const wxQtPenData&) = delete;
};

class wxQtBitmapData : public wxGraphicsBitmapData
{
public:
    wxQtBitmapData(wxGraphicsRenderer* renderer, QPixmap* pixmap)
        : wxGraphicsBitmapData(renderer),
          m_pixmap(pixmap)
    {
    }

    wxQtBitmapData(wxGraphicsRenderer* renderer, const wxBitmap& bmp)
        : wxGraphicsBitmapData(renderer),
          m_pixmap(bmp.GetHandle())
    {
    }

    QPixmap* GetPixmap() const
    {
        return m_pixmap;
    }

    void* GetNativeBitmap() const override
    {
        return m_pixmap;
    }

    static const QPixmap* GetPixmapFromBitmap(const wxGraphicsBitmap& bitmap)
    {
        const wxQtBitmapData* const
            data = static_cast<const wxQtBitmapData*>(bitmap.GetBitmapData());
        return data->GetPixmap();
    }


#if wxUSE_IMAGE
    wxImage DoConvertToImage() const
    {
        wxBitmap bitmap(*m_pixmap);
        return bitmap.ConvertToImage();
    }
#endif // wxUSE_IMAGE

private:
    QPixmap* m_pixmap;

    wxQtBitmapData(const wxQtBitmapData&) = delete;
	wxQtBitmapData& operator=(const wxQtBitmapData&) = delete;
};


class wxQtMatrixData : public wxGraphicsMatrixData
{
public:
    explicit wxQtMatrixData(wxGraphicsRenderer* renderer)
        : wxGraphicsMatrixData(renderer),
          m_transform(new QTransform)
    {
    }

    wxQtMatrixData(wxGraphicsRenderer* renderer, const QTransform& transform)
        : wxGraphicsMatrixData(renderer),
          m_transform(new QTransform(transform))
    {
    }

    virtual ~wxQtMatrixData()
    {
        delete m_transform;
    }

    wxGraphicsObjectRefData* Clone() const override
    {
        wxQtMatrixData* newMatrix = new wxQtMatrixData(m_renderer);
        *newMatrix->m_transform = *m_transform;
        return newMatrix;
    }

    // concatenates the matrix
    void Concat(const wxGraphicsMatrixData *t) override
    {
        const wxQtMatrixData* rhs = static_cast<const wxQtMatrixData*>(t);
        *m_transform = *rhs->m_transform* (*m_transform);
    }

    // sets the matrix to the respective values
    virtual void Set(double a = 1.0, double b = 0.0,
                     double c = 0.0, double d = 1.0,
                     double tx = 0.0, double ty = 0.0) override
    {
        m_transform->setMatrix(a, b, 0.0,
                               c, d, 0.0,
                               tx, ty, 1.0);
    }

    // gets the component values of the matrix
    virtual void Get(double* a, double* b,
                     double* c, double* d,
                     double* tx, double* ty) const override
    {
        if ( a )
            *a = m_transform->m11();
        if ( b )
            *b = m_transform->m12();
        if ( c )
            *c = m_transform->m21();
        if ( d )
            *d = m_transform->m22();
        if ( tx )
            *tx = m_transform->m31();
        if ( ty )
            *ty = m_transform->m32();
    }

    // makes this the inverse matrix
    void Invert() override
    {
        bool invertible = false;
        const QTransform invTransform = m_transform->inverted(&invertible);
        if ( invertible )
        {
            *m_transform = invTransform;
        }
    }

    // returns true if the elements of the transformation matrix are equal ?
    bool IsEqual(const wxGraphicsMatrixData* t) const override
    {
        const wxQtMatrixData* rhs = static_cast<const wxQtMatrixData*>(t);
        return *m_transform == *rhs->m_transform;
    }

    // return true if this is the identity matrix
    bool IsIdentity() const override
    {
        return m_transform->isIdentity();
    }

    //
    // transformation
    //

    // add the translation to this matrix
    void Translate(double dx, double dy) override
    {
        m_transform->translate(dx, dy);
    }

    // add the scale to this matrix
    void Scale(double xScale, double yScale) override
    {
        m_transform->scale(xScale, yScale);
    }

    // add the rotation to this matrix (radians)
    void Rotate(double angle) override
    {
        m_transform->rotateRadians(angle);
    }

    //
    // apply the transforms
    //

    // applies that matrix to the point
    void TransformPoint(double *x, double *y) const override
    {
        qreal transformed_x, transformed_y;
        m_transform->map(static_cast<qreal>(*x), static_cast<qreal>(*y),
                         &transformed_x, &transformed_y);
        *x = transformed_x;
        *y = transformed_y;
    }

    // applies the matrix except for translations
    void TransformDistance(double *dx, double *dy) const override
    {
        const QTransform untransTransform(
            m_transform->m11(),
            m_transform->m12(),
            m_transform->m13(),
            m_transform->m21(),
            m_transform->m22(),
            m_transform->m23(),
            0.0,
            0.0,
            1.0);

        qreal transformed_x, transformed_y;
        untransTransform.map(static_cast<qreal>(*dx), static_cast<qreal>(*dy),
                             &transformed_x, &transformed_y);

        *dx = transformed_x;
        *dy = transformed_y;
    }

    // returns the native representation
    void* GetNativeMatrix() const override
    {
        return static_cast<void*>(m_transform);
    }

    const QTransform& GetQTransform() const
    {
        return *m_transform;
    }
private:
    QTransform* m_transform;

    wxQtMatrixData(const wxQtMatrixData&) = delete;
	wxQtMatrixData& operator=(const wxQtMatrixData&) = delete;
};

class wxQtFontData : public wxGraphicsObjectRefData
{
public:
    wxQtFontData(wxGraphicsRenderer* renderer,
                 const wxFont& font,
                 const wxColour& col)
        : wxGraphicsObjectRefData(renderer),
          m_font(font.GetHandle()),
          m_color(col.GetQColor())
    {
    }

    wxQtFontData(
        wxGraphicsRenderer* renderer,
        double sizeInPixels,
        const wxString& facename,
        int flags,
        const wxColour& col)
        : wxGraphicsObjectRefData(renderer),
          m_color(col.GetQColor())
    {
        m_font.setFamily(QString(facename));
        m_font.setPixelSize(static_cast<int>(sizeInPixels));
        if ( flags & wxFONTFLAG_LIGHT )
            m_font.setWeight(QFont::Light);
        if ( flags & wxFONTFLAG_BOLD )
            m_font.setWeight(QFont::Bold);
        if ( flags & (wxFONTFLAG_ITALIC | wxFONTFLAG_SLANT) )
            m_font.setItalic(true);
        if ( flags & wxFONTFLAG_ANTIALIASED )
            m_font.setStyleStrategy(QFont::PreferAntialias);
        if ( flags & wxFONTFLAG_NOT_ANTIALIASED )
            m_font.setStyleStrategy(QFont::NoAntialias);
        if ( flags & wxFONTFLAG_UNDERLINED )
            m_font.setUnderline(true);
        if ( flags & wxFONTFLAG_STRIKETHROUGH )
            m_font.setStrikeOut(true);
    }

    const QFont& GetFont() const
    {
        return m_font;
    }

    const QColor& GetColor() const
    {
        return m_color;
    }

private:
    QFont m_font;
    QColor m_color;

    wxQtFontData(const wxQtFontData&) = delete;
	wxQtFontData& operator=(const wxQtFontData&) = delete;
};

//-----------------------------------------------------------------------------
// wxQtGraphicsPathData
//-----------------------------------------------------------------------------

class wxQtGraphicsPathData : public wxGraphicsPathData
{
public:
    wxQtGraphicsPathData(wxGraphicsRenderer* renderer)
        : wxGraphicsPathData(renderer),
          m_path(new QPainterPath()),
          m_current_subpath_start(-1)
    {
    }

    ~wxQtGraphicsPathData()
    {
        delete m_path;
    }

    wxGraphicsObjectRefData *Clone() const override
    {
        return new wxQtGraphicsPathData(*this);
    }

    //
    // These are the path primitives from which everything else can be
    // constructed.
    //

    // begins a new subpath at (x,y)
    void MoveToPoint(double x, double y) override
    {
        m_path->moveTo(x, y);
        m_current_subpath_start = m_path->elementCount() - 1;
    }

    // adds a straight line from the current point to (x,y)
    // if there is no current path, it is equivalent to a moveTo.
    void AddLineToPoint(double x, double y) override
    {
        if ( !HasCurrentSubpath() )
            MoveToPoint(x, y);
        else
            m_path->lineTo(x, y);
    }

    // adds a cubic Bezier curve from the current point, using two control
    // points and an end point
    virtual void
    AddCurveToPoint(double cx1, double cy1,
                    double cx2, double cy2,
                    double x, double y) override
    {
        if ( !HasCurrentSubpath() )
            MoveToPoint(cx1, cy1);
        m_path->cubicTo(QPointF(cx1, cy1), QPointF(cx2, cy2), QPointF(x, y));
    }

    // adds an arc of a circle centering at (x,y) with radius (r) from
    // startAngle to endAngle
    virtual void
    AddArc(double x, double y, double r,
           double startAngle, double endAngle,
           bool clockwise) override
    {
        const bool fixupFirst = !HasCurrentSubpath();
        if ( fixupFirst )
            MoveToPoint(x, y);

        qreal arcLength;
        if ( clockwise )
        {
            if ( endAngle < startAngle )
                endAngle += 2* M_PI;
            arcLength = -wxRadToDeg(endAngle - startAngle);
        }
        else
        {
            if ( endAngle > startAngle )
                endAngle -= 2* M_PI;
            arcLength = -wxRadToDeg(endAngle - startAngle);
        }
        m_path->arcTo(x-r, y-r, r*2, r*2, -wxRadToDeg(startAngle), arcLength);
        if ( fixupFirst )
        {
            QPainterPath::Element
                element = m_path->elementAt(m_current_subpath_start+1);
            m_path->setElementPositionAt(m_current_subpath_start,
                                         element.x, element.y);
        }
    }

    // gets the last point of the current path, (0,0) if not yet set
    void GetCurrentPoint(double* x, double* y) const override
    {
        QPointF position = m_path->currentPosition();
        *x = position.x();
        *y = position.y();
    }

    // adds another path
    void AddPath(const wxGraphicsPathData* path) override
    {
        const wxQtGraphicsPathData*
            pathAdded = static_cast<const wxQtGraphicsPathData*>(path);
        m_path->addPath(*pathAdded->m_path);
    }

    // closes the current sub-path
    void CloseSubpath() override
    {
        // Current position must be end of last path after close, not (0,0) as
        // Qt sets.
        if ( !HasCurrentSubpath() )
            return;

        const QPainterPath::Element
            firstElement = m_path->elementAt(m_current_subpath_start);

        m_path->closeSubpath();
        MoveToPoint(firstElement.x, firstElement.y);
    }

    //
    // These are convenience functions which - if not available natively will
    // be assembled using the primitives from above
    //

    // appends a circle as a new closed subpath
    void AddCircle(double x, double y, double r) override
    {
        m_path->addEllipse(x - r, y - r, r*2, r*2);
    }

    // appends an ellipse as a new closed subpath fitting the passed rectangle
    virtual void
    AddEllipse(double x, double y, double w, double h) override
    {
        m_path->addEllipse(x, y, w, h);
    }

    // returns the native path
    void* GetNativePath() const override
    {
        return static_cast<void*>(m_path);
    }

    // give the native path returned by GetNativePath() back (there might be
    // some deallocations necessary)
    void UnGetNativePath(void *) const override
    {
    }

    // transforms each point of this path by the matrix
    void Transform(const wxGraphicsMatrixData* matrix) override
    {
        const wxQtMatrixData*
            qmatrix = static_cast<const wxQtMatrixData*>(matrix);
        *m_path = qmatrix->GetQTransform().map(*m_path);
    }

    // gets the bounding box enclosing all points (possibly including control
    // points)
    virtual void
    GetBox(double *x, double *y, double *w, double *h) const override
    {
        QRectF boundingRect = m_path->controlPointRect();

        if ( !boundingRect.isValid() )
            boundingRect = QRectF();

        if ( x ) *x = boundingRect.left();
        if ( y ) *y = boundingRect.top();
        if ( w ) *w = boundingRect.width();
        if ( h ) *h = boundingRect.height();
    }

    virtual bool
    Contains(double x, double y,
             wxPolygonFillMode /*fillStyle = wxPolygonFillMode::WindingRule*/) const override
    {
        return m_path->contains(QPointF(x, y));
    }

private:
    // for Clone
    wxQtGraphicsPathData(const wxQtGraphicsPathData& rhs)
        : wxGraphicsPathData(rhs.GetRenderer()),
          m_path(new QPainterPath(*rhs.m_path)),
          m_current_subpath_start(rhs.m_current_subpath_start)
    {
    }

    bool HasCurrentSubpath() const
    {
        return m_current_subpath_start != -1;
    }

    QPainterPath* m_path;
    int m_current_subpath_start;

    wxQtGraphicsPathData& operator=(const wxQtGraphicsPathData&);
};

class wxQtGraphicsContext : public wxGraphicsContext
{
    void InitFromDC(const wxDC& dc)
    {
        m_qtPainter = static_cast<QPainter*>(dc.GetHandle());

        const wxSize sz = dc.GetSize();
        m_width = sz.x;
        m_height = sz.y;
    }

protected:
    // Use the specified painter and take ownership of it, i.e. it will be
    // destroyed in this class dtor.
    void AttachPainter(QPainter* painter)
    {
        m_qtPainter = painter;

        // Ensure that it will be destroyed when this object is.
        m_ownedPainter.reset(m_qtPainter);
    }

    wxQtGraphicsContext(wxGraphicsRenderer* renderer)
        : wxGraphicsContext(renderer),
          m_qtPainter(NULL)
    {
    }

public:
    wxQtGraphicsContext(wxGraphicsRenderer* renderer, QPaintDevice* device)
        : wxGraphicsContext(renderer)
    {
        AttachPainter(new QPainter(device));

        m_width = device->width();
        m_height = device->height();
    }

    wxQtGraphicsContext(wxGraphicsRenderer* renderer, const wxWindowDC& dc)
        : wxGraphicsContext(renderer)
    {
        InitFromDC(dc);
    }

    wxQtGraphicsContext(wxGraphicsRenderer* renderer, const wxMemoryDC& dc)
        : wxGraphicsContext(renderer)
    {
        InitFromDC(dc);
    }

#if wxUSE_PRINTING_ARCHITECTURE
    wxQtGraphicsContext(wxGraphicsRenderer* renderer, const wxPrinterDC& dc)
        : wxGraphicsContext(renderer)
    {
        InitFromDC(dc);
    }

#endif

    wxQtGraphicsContext(wxGraphicsRenderer* renderer, wxWindow *window)
        : wxGraphicsContext(renderer)
    {
        m_qtPainter = static_cast<QPainter*>(window->QtGetPainter());

        const wxSize sz = window->GetClientSize();
        m_width = sz.x;
        m_height = sz.y;
    }

    bool ShouldOffset() const override
    {
        return false;
    }

    void Clip(const wxRegion& region) override
    {
        m_qtPainter->setClipRegion(region.GetHandle());
    }

    // clips drawings to the rect
    void Clip(double x, double y, double w, double h) override
    {
        m_qtPainter->setClipRect(x, y, w, h);
    }

    // resets the clipping to original extent
    void ResetClip() override
    {
        m_qtPainter->setClipping(false);
    }

    // returns bounding box of the clipping region
    virtual void
    GetClipBox(double* x, double* y, double* w, double* h) override
    {
        const QRectF box = m_qtPainter->clipBoundingRect();
        if ( x )
            *x = box.left();
        if ( y )
            *y = box.top();
        if ( w )
            *w = box.width();
        if ( h )
            *h = box.height();
    }

    void* GetNativeContext() override
    {
        return static_cast<void*>(m_qtPainter);
    }

    bool SetAntialiasMode(wxAntialiasMode antialias) override
    {
        bool enableAA = false;
        switch ( antialias )
        {
            case wxANTIALIAS_DEFAULT:
                enableAA = true;
                break;

            case wxANTIALIAS_NONE:
                enableAA = false;
                break;
        }

        m_qtPainter->setRenderHints(QPainter::Antialiasing |
                                    QPainter::TextAntialiasing,
                                    enableAA);

        m_antialias = antialias;
        return true;
    }

    virtual bool
    SetInterpolationQuality(wxInterpolationQuality interpolation) override
    {
        m_qtPainter->setRenderHint(QPainter::SmoothPixmapTransform,
                                   interpolation == wxInterpolationQuality::Best);
        m_interpolation = interpolation;
        return true;
    }

    bool SetCompositionMode(wxCompositionMode composition) override
    {
        QPainter::CompositionMode q_composition_mode;
        switch ( composition )
        {
            case wxCOMPOSITION_CLEAR:
                q_composition_mode = QPainter::CompositionMode_Clear;
                break;
            case wxCOMPOSITION_SOURCE:
                q_composition_mode = QPainter::CompositionMode_Source;
                break;
            case wxCOMPOSITION_OVER:
                q_composition_mode = QPainter::CompositionMode_SourceOver;
                break;
            case wxCOMPOSITION_IN:
                q_composition_mode = QPainter::CompositionMode_SourceIn;
                break;
            case wxCOMPOSITION_OUT:
                q_composition_mode = QPainter::CompositionMode_SourceOut;
                break;
            case wxCOMPOSITION_ATOP:
                q_composition_mode = QPainter::CompositionMode_SourceAtop;
                break;
            case wxCOMPOSITION_DEST:
                q_composition_mode = QPainter::CompositionMode_Destination;
                break;
            case wxCOMPOSITION_DEST_OVER:
                q_composition_mode = QPainter::CompositionMode_DestinationOver;
                break;
            case wxCOMPOSITION_DEST_IN:
                q_composition_mode = QPainter::CompositionMode_DestinationIn;
                break;
            case wxCOMPOSITION_DEST_OUT:
                q_composition_mode = QPainter::CompositionMode_DestinationOut;
                break;
            case wxCOMPOSITION_DEST_ATOP:
                q_composition_mode = QPainter::CompositionMode_DestinationAtop;
                break;
            case wxCOMPOSITION_XOR:
                q_composition_mode = QPainter::CompositionMode_Xor;
                break;
            case wxCOMPOSITION_ADD:
                q_composition_mode = QPainter::CompositionMode_Plus;
                break;
            default:
                return false;
        }
        m_qtPainter->setCompositionMode(q_composition_mode);
        m_composition = composition;
        return true;
    }

    void BeginLayer(double /*opacity*/) override
    {
        wxFAIL_MSG("BeginLayer not implemented");
    }

    void EndLayer() override
    {
        wxFAIL_MSG("EndLayer not implemented");
    }

    void StrokePath(const wxGraphicsPath& p) override
    {
        if ( m_pen.IsNull() )
        {
            return;
        }

        const QPainterPath*
            pathData = static_cast<QPainterPath*>(p.GetNativePath());
        const QPen&
            pen = static_cast<wxQtPenData*>(m_pen.GetRefData())->GetPen();
        m_qtPainter->strokePath(*pathData, pen);
    }

    virtual void
    FillPath(const wxGraphicsPath& p,
             wxPolygonFillMode /*fillStyle = wxPolygonFillMode::WindingRule*/) override
    {
        if ( m_brush.IsNull() )
        {
            return;
        }

        const QPainterPath*
            pathData = static_cast<QPainterPath*>(p.GetNativePath());
        const QBrush&
            brush = static_cast<wxQtBrushData*>(m_brush.GetRefData())->getBrush();
        m_qtPainter->fillPath(*pathData, brush);
    }

    virtual void
    ClearRectangle(double x, double y, double w, double h) override
    {
        m_qtPainter->fillRect(x, y, w, h, QBrush(QColor(0, 0, 0, 0)));
    }

    void Translate(double dx, double dy) override
    {
        m_qtPainter->translate(dx, dy);
    }

    void Scale(double xScale, double yScale) override
    {
        m_qtPainter->scale(xScale, yScale);
    }

    void Rotate(double angle) override
    {
        // wx angle is in radians. Qt angle is in degrees.
        m_qtPainter->rotate(wxRadToDeg(angle));
    }

    // concatenates this transform with the current transform of this context
    void ConcatTransform(const wxGraphicsMatrix& matrix) override
    {
        wxGraphicsMatrix currentMatrix = GetTransform();
        currentMatrix.Concat(matrix);
        SetTransform(currentMatrix);
    }

    // sets the transform of this context
    void SetTransform(const wxGraphicsMatrix& matrix) override
    {
        const wxQtMatrixData*
            qmatrix = static_cast<const wxQtMatrixData*>(matrix.GetRefData());
        m_qtPainter->setTransform(qmatrix->GetQTransform());
    }

    // gets the matrix of this context
    wxGraphicsMatrix GetTransform() const override
    {
        const QTransform& transform = m_qtPainter->transform();
        wxGraphicsMatrix m;
        m.SetRefData(new wxQtMatrixData(GetRenderer(), transform));
        return m;
    }

    virtual void
    DrawBitmap(const wxGraphicsBitmap& bmp,
               double x, double y, double w, double h) override
    {
        const QPixmap* pixmap = wxQtBitmapData::GetPixmapFromBitmap(bmp);
        m_qtPainter->drawPixmap(x, y, w, h, *pixmap);
    }

    virtual void
    DrawBitmap(const wxBitmap& bmp,
               double x, double y, double w, double h) override
    {
        DoDrawBitmap(bmp, x, y, w, h, true);
    }

    virtual void
    DrawIcon(const wxIcon& icon,
             double x, double y, double w, double h) override
    {
        DoDrawBitmap(icon, x, y, w, h, true);
    }

    void PushState() override
    {
        m_qtPainter->save();
    }

    void PopState() override
    {
        m_qtPainter->restore();
    }

    void Flush() override
    {
    }

    virtual void GetTextExtent(const wxString& str,
                               double *width,
                               double *height,
                               double *descent,
                               double *externalLeading) const override
    {
        wxCHECK_RET( !m_font.IsNull(),
                     "wxQtContext::GetTextExtent - no valid font set" );

        EnsurePainterIsActive active(m_qtPainter);

        const wxQtFontData*
            fontData = static_cast<wxQtFontData*>(m_font.GetRefData());
        m_qtPainter->setFont(fontData->GetFont());

        const QFontMetrics metrics = m_qtPainter->fontMetrics();
        const QRect boundingRect = metrics.boundingRect(QString(str));

        if ( width )
            *width = boundingRect.width();
        if ( height )
            *height = boundingRect.height();
        if ( descent )
            *descent = metrics.descent();
        if ( externalLeading )
            *externalLeading = metrics.leading() - (metrics.ascent() + metrics.descent());
    }

    virtual std::vector<double>
    GetPartialTextExtents(const wxString& text) const override
    {
        wxCHECK_RET( !m_font.IsNull(),
                     "wxQtContext::GetPartialTextExtents - no valid font set" );

        EnsurePainterIsActive active(m_qtPainter);

        const wxQtFontData*
            fontData = static_cast<wxQtFontData*>(m_font.GetRefData());
        m_qtPainter->setFont(fontData->GetFont());

        const QFontMetrics metrics = m_qtPainter->fontMetrics();

        const size_t textLength = text.length();

        for ( size_t i = 0; i < textLength; ++i )
        {
            const wxString subString = text.substr(0, i+1);
            const QRect
                boundingRect = metrics.boundingRect(QString(subString));
            widths.push_back(boundingRect.width());
        }

        return widths;
    }

protected:
    virtual void
    DoDrawText(const wxString& str, double x, double y) override
    {
        wxCHECK_RET( !m_font.IsNull(),
                     "wxQtContext::DrawText - no valid font set" );

        const wxQtFontData*
            fontData = static_cast<wxQtFontData*>(m_font.GetRefData());

        m_qtPainter->setFont(fontData->GetFont());
        m_qtPainter->setPen(QPen(fontData->GetColor()));

        QFontMetrics metrics = m_qtPainter->fontMetrics();

        wxStringTokenizer tokenizer(str, "\n");
        while ( tokenizer.HasMoreTokens() )
        {
            const wxString line = tokenizer.GetNextToken();
            m_qtPainter->drawText(x, y + metrics.ascent(), QString(line));
            y += metrics.lineSpacing();
        }
    }

    void
    DoDrawBitmap(const wxBitmap& bmp,
                 double x, double y,
                 double w, double h, bool useMask) const
    {
        QPixmap pix = *bmp.GetHandle();

        if (useMask && bmp.GetMask() && bmp.GetMask()->GetHandle())
            pix.setMask(*bmp.GetMask()->GetHandle());

        m_qtPainter->drawPixmap(x, y, w, h, pix);
    }

    QPainter* m_qtPainter;

private:
    // This pointer may be empty if we don't own m_qtPainter.
    wxScopedPtr<QPainter> m_ownedPainter;

    wxQtGraphicsContext(const wxQtGraphicsContext&) = delete;
	wxQtGraphicsContext& operator=(const wxQtGraphicsContext&) = delete;
};

class wxQtMeasuringContext : public wxQtGraphicsContext
{
public:
    wxQtMeasuringContext(wxGraphicsRenderer* renderer)
        : wxQtGraphicsContext(renderer)
    {
        AttachPainter(new QPainter());
    }
};

class wxQtImageContext : public wxQtGraphicsContext
{
public:
    wxQtImageContext(wxGraphicsRenderer* renderer, wxImage& image) :
        wxQtGraphicsContext(renderer),
        m_image(image)
    {
        const wxBitmap wxbitmap(image);
        m_pixmap = *wxbitmap.GetHandle();
        AttachPainter(new QPainter(&m_pixmap));
    }

    ~wxQtImageContext()
    {
        wxQtBitmapData bitmap(GetRenderer(), &m_pixmap);
        m_image = bitmap.DoConvertToImage();
    }

private:
    QPixmap m_pixmap;
    wxImage& m_image;
};

//-----------------------------------------------------------------------------
// wxQtGraphicsRenderer declaration
//-----------------------------------------------------------------------------

class wxQtGraphicsRenderer : public wxGraphicsRenderer
{
public:
    wxQtGraphicsRenderer() {}

    virtual ~wxQtGraphicsRenderer() {}

	wxQtGraphicsRenderer(const wxQtGraphicsRenderer&) = delete;
	wxQtGraphicsRenderer& operator=(const wxQtGraphicsRenderer&) = delete;

    // Context

    wxGraphicsContext* CreateContext(const wxWindowDC& dc) override;
    wxGraphicsContext* CreateContext(const wxMemoryDC& dc) override;
#if wxUSE_PRINTING_ARCHITECTURE
    wxGraphicsContext* CreateContext(const wxPrinterDC& dc) override;
#endif

    wxGraphicsContext* CreateContextFromNativeContext(void* context) override;

    wxGraphicsContext* CreateContextFromNativeWindow(void* window) override;

#if wxUSE_IMAGE
    wxGraphicsContext* CreateContextFromImage(wxImage& image) override;
#endif // wxUSE_IMAGE

    wxGraphicsContext* CreateContext(wxWindow* window) override;

    wxGraphicsContext* CreateMeasuringContext() override;

    // Path

    wxGraphicsPath CreatePath() override;

    // Matrix

    virtual wxGraphicsMatrix CreateMatrix(double a = 1.0, double b = 0.0,
                                          double c = 0.0, double d = 1.0,
                                          double tx = 0.0, double ty = 0.0) override;

    wxGraphicsPen CreatePen(const wxGraphicsPenInfo& info) override;

    wxGraphicsBrush CreateBrush(const wxBrush& brush) override;

    virtual wxGraphicsBrush
        CreateLinearGradientBrush(double x1, double y1,
        double x2, double y2,
        const wxGraphicsGradientStops& stops,
        const wxGraphicsMatrix& matrix = wxNullGraphicsMatrix) override;

    virtual wxGraphicsBrush
        CreateRadialGradientBrush(double startX, double startY,
        double endX, double endY,
        double radius,
        const wxGraphicsGradientStops& stops,
        const wxGraphicsMatrix& matrix = wxNullGraphicsMatrix) override;

    // sets the font
    virtual wxGraphicsFont CreateFont(const wxFont& font,
                                      const wxColour& col = *wxBLACK) override;
    virtual wxGraphicsFont CreateFont(double sizeInPixels,
        const wxString& facename,
        int flags = wxFONTFLAG_DEFAULT,
        const wxColour& col = *wxBLACK) override;
    virtual wxGraphicsFont CreateFontAtDPI(const wxFont& font,
                                           const wxRealPoint& dpi,
                                           const wxColour& col) override;

    // create a native bitmap representation
    wxGraphicsBitmap CreateBitmap(const wxBitmap& bitmap) override;
#if wxUSE_IMAGE
    wxGraphicsBitmap CreateBitmapFromImage(const wxImage& image) override;
    wxImage CreateImageFromBitmap(const wxGraphicsBitmap& bmp) override;
#endif // wxUSE_IMAGE

    // create a graphics bitmap from a native bitmap
    wxGraphicsBitmap CreateBitmapFromNativeBitmap(void* bitmap) override;

    // create a subimage from a native image representation
    virtual wxGraphicsBitmap CreateSubBitmap(const wxGraphicsBitmap& bitmap,
                                             double x, double y,
                                             double w, double h) override;

    wxString GetName() const override;
    virtual void GetVersion(int *major,
                            int *minor,
                            int *micro) const override;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

//-----------------------------------------------------------------------------
// wxQtGraphicsRenderer implementation
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxQtGraphicsRenderer, wxGraphicsRenderer);

wxGraphicsContext* wxQtGraphicsRenderer::CreateContext(const wxWindowDC& dc)
{
    return new wxQtGraphicsContext(this, dc);
}

wxGraphicsContext* wxQtGraphicsRenderer::CreateContext(const wxMemoryDC& dc)
{
    return new wxQtGraphicsContext(this, dc);
}

#if wxUSE_PRINTING_ARCHITECTURE
wxGraphicsContext* wxQtGraphicsRenderer::CreateContext(const wxPrinterDC& dc)
{
    return new wxQtGraphicsContext(this, dc);
}
#endif

wxGraphicsContext*
wxQtGraphicsRenderer::CreateContextFromNativeContext(void* context)
{
    return new wxQtGraphicsContext(this, static_cast<QPaintDevice*>(context));
}

wxGraphicsContext*
wxQtGraphicsRenderer::CreateContextFromNativeWindow(void* window)
{
    return new wxQtGraphicsContext(this, static_cast<QWidget*>(window));
}

#if wxUSE_IMAGE
wxGraphicsContext* wxQtGraphicsRenderer::CreateContextFromImage(wxImage& image)
{
    return new wxQtImageContext(this, image);
}
#endif // wxUSE_IMAGE

wxGraphicsContext* wxQtGraphicsRenderer::CreateMeasuringContext()
{
    return new wxQtMeasuringContext(this);
}

wxGraphicsContext* wxQtGraphicsRenderer::CreateContext(wxWindow* window)
{
    return new wxQtGraphicsContext(this, window);
}

// Path

wxGraphicsPath wxQtGraphicsRenderer::CreatePath()
{
    wxGraphicsPath path;
    path.SetRefData(new wxQtGraphicsPathData(this));
    return path;
}

// Matrix

wxGraphicsMatrix wxQtGraphicsRenderer::CreateMatrix(double a, double b,
                                                    double c, double d,
                                                    double tx, double ty)

{
    wxGraphicsMatrix m;
    wxQtMatrixData* data = new wxQtMatrixData(this);
    data->Set(a, b, c, d, tx, ty);
    m.SetRefData(data);
    return m;
}

wxGraphicsPen wxQtGraphicsRenderer::CreatePen(const wxGraphicsPenInfo& info)
{
    wxGraphicsPen p;
    if ( info.GetStyle() != wxPenStyle::Transparent )
    {
        p.SetRefData(new wxQtPenData(this, info));
    }
    return p;
}

wxGraphicsBrush wxQtGraphicsRenderer::CreateBrush(const wxBrush& brush)
{
    wxGraphicsBrush p;
    if ( brush.IsOk() && brush.GetStyle() != wxBrushStyle::Transparent )
    {
        p.SetRefData(new wxQtBrushData(this, brush));
    }
    return p;
}

wxGraphicsBrush wxQtGraphicsRenderer::CreateLinearGradientBrush(
    double x1, double y1,
    double x2, double y2,
    const wxGraphicsGradientStops& stops, 
    const wxGraphicsMatrix& WXUNUSED(matrix))
{
    wxGraphicsBrush p;
    wxQtBrushData* d = new wxQtBrushData(this);
    d->CreateLinearGradientBrush(x1, y1, x2, y2, stops);
    p.SetRefData(d);
    return p;
}

wxGraphicsBrush wxQtGraphicsRenderer::CreateRadialGradientBrush(
    double startX, double startY,
    double endX, double endY, double r,
    const wxGraphicsGradientStops& stops, 
    const wxGraphicsMatrix& WXUNUSED(matrix))
{
    wxGraphicsBrush p;
    wxQtBrushData* d = new wxQtBrushData(this);
    d->CreateRadialGradientBrush(startX, startY, endX, endY, r, stops);
    p.SetRefData(d);
    return p;
}

wxGraphicsFont
wxQtGraphicsRenderer::CreateFont(const wxFont& font, const wxColour& col)
{
    wxGraphicsFont p;
    if ( font.IsOk() )
    {
        p.SetRefData(new wxQtFontData(this, font, col));
    }
    return p;
}

wxGraphicsFont wxQtGraphicsRenderer::CreateFont(
    double sizeInPixels,
    const wxString& facename,
    int flags,
    const wxColour& col)
{
    wxGraphicsFont font;
    font.SetRefData(new wxQtFontData(this, sizeInPixels, facename, flags, col));
    return font;
}

wxGraphicsFont
wxQtGraphicsRenderer::CreateFontAtDPI(const wxFont& font,
                                      const wxRealPoint& WXUNUSED(dpi),
                                      const wxColour& col)
{
    return CreateFont(font, col);
}

wxGraphicsBitmap wxQtGraphicsRenderer::CreateBitmap(const wxBitmap& bmp)
{
    wxGraphicsBitmap p;
    if ( bmp.IsOk() )
    {
        p.SetRefData(new wxQtBitmapData(this, bmp));
    }
    return p;
}

#if wxUSE_IMAGE

wxGraphicsBitmap
wxQtGraphicsRenderer::CreateBitmapFromImage(const wxImage& image)
{
    wxGraphicsBitmap bmp;
    if ( image.IsOk() )
    {
        bmp.SetRefData(new wxQtBitmapData(this, image));
    }
    return bmp;
}

wxImage wxQtGraphicsRenderer::CreateImageFromBitmap(const wxGraphicsBitmap& bmp)
{
    const wxQtBitmapData* const
        data = static_cast<const wxQtBitmapData*>(bmp.GetBitmapData());
    return data->DoConvertToImage();
}
#endif // wxUSE_IMAGE

wxGraphicsBitmap
wxQtGraphicsRenderer::CreateBitmapFromNativeBitmap(void* bitmap)
{
    wxGraphicsBitmap p;
    if ( bitmap != NULL )
    {
        p.SetRefData(new wxQtBitmapData(this, (QPixmap*)bitmap));
    }
    return p;
}

wxGraphicsBitmap
wxQtGraphicsRenderer::CreateSubBitmap(const wxGraphicsBitmap& bitmap,
    double x, double y,
    double w, double h)
{
    wxCHECK_MSG(!bitmap.IsNull(), wxNullGraphicsBitmap, wxS("Invalid bitmap"));

    const QPixmap* sourcePixmap = wxQtBitmapData::GetPixmapFromBitmap(bitmap);
    wxCHECK_MSG(sourcePixmap, wxNullGraphicsBitmap, wxS("Invalid bitmap"));

    const int srcWidth = sourcePixmap->width();
    const int srcHeight = sourcePixmap->height();
    const int dstWidth = wxRound(w);
    const int dstHeight = wxRound(h);

    wxCHECK_MSG(x >= 0.0 && y >= 0.0 && dstWidth > 0 && dstHeight > 0 &&
            x + dstWidth <= srcWidth && y + dstHeight <= srcHeight,
            wxNullGraphicsBitmap, wxS("Invalid bitmap region"));

    QPixmap* subPixmap = new QPixmap(sourcePixmap->copy(x, y, w, h));

    wxGraphicsBitmap bmpRes;
    bmpRes.SetRefData(new wxQtBitmapData(this, subPixmap));
    return bmpRes;
}

wxString wxQtGraphicsRenderer::GetName() const
{
    return "qt";
}

void wxQtGraphicsRenderer::GetVersion(int *major, int *minor, int *micro) const
{
    if ( major ) *major = QT_VERSION_MAJOR;
    if ( minor ) *minor = QT_VERSION_MINOR;
    if ( micro ) *micro = QT_VERSION_PATCH;
}

static wxQtGraphicsRenderer gs_qtGraphicsRenderer;

wxGraphicsRenderer* wxGraphicsRenderer::GetDefaultRenderer()
{
    return &gs_qtGraphicsRenderer;
}

#endif // wxUSE_GRAPHICS_CONTEXT
