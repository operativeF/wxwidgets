/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/dc.h
// Author:      Peter Most, Javier Torres, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_DC_H_
#define _WX_QT_DC_H_

class QPainter;
class QImage;

class WXDLLIMPEXP_FWD_CORE wxRegion;

class wxQtDCImpl : public wxDCImpl
{
public:
    wxQtDCImpl( wxDC *owner );
    ~wxQtDCImpl();

    bool CanDrawBitmap() const override;
    bool CanGetTextExtent() const override;

    wxSize DoGetSize() const override;
    void DoGetSizeMM(int* width, int* height) const override;

    int GetDepth() const override;
    wxSize GetPPI() const override;

    void SetFont(const wxFont& font) override;
    void SetPen(const wxPen& pen) override;
    void SetBrush(const wxBrush& brush) override;
    void SetBackground(const wxBrush& brush) override;
    void SetBackgroundMode(wxBrushStyle mode) override;

#if wxUSE_PALETTE
    void SetPalette(const wxPalette& palette) override;
#endif // wxUSE_PALETTE

    void SetLogicalFunction(wxRasterOperationMode function) override;

    wxCoord GetCharHeight() const override;
    wxCoord GetCharWidth() const override;
    virtual void DoGetTextExtent(const wxString& string,
                                 wxCoord *x, wxCoord *y,
                                 wxCoord *descent = NULL,
                                 wxCoord *externalLeading = NULL,
                                 const wxFont *theFont = NULL) const override;

    void Clear() override;

    virtual void DoSetClippingRegion(wxCoord x, wxCoord y,
                                     wxCoord width, wxCoord height) override;

    void DoSetDeviceClippingRegion(const wxRegion& region) override;
    void DestroyClippingRegion() override;

    virtual bool DoFloodFill(wxCoord x, wxCoord y, const wxColour& col,
                             wxFloodFillStyle style = wxFloodFillStyle::Surface) override;
    bool DoGetPixel(wxCoord x, wxCoord y, wxColour *col) const override;

    void DoDrawPoint(wxCoord x, wxCoord y) override;
    void DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2) override;

    virtual void DoDrawArc(wxCoord x1, wxCoord y1,
                           wxCoord x2, wxCoord y2,
                           wxCoord xc, wxCoord yc) override;

    virtual void DoDrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
                                   double sa, double ea) override;

    void DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;
    virtual void DoDrawRoundedRectangle(wxCoord x, wxCoord y,
                                        wxCoord width, wxCoord height,
                                        double radius) override;
    virtual void DoDrawEllipse(wxCoord x, wxCoord y,
                               wxCoord width, wxCoord height) override;

    void DoCrossHair(wxCoord x, wxCoord y) override;

    void DoDrawIcon(const wxIcon& icon, wxCoord x, wxCoord y) override;
    virtual void DoDrawBitmap(const wxBitmap &bmp, wxCoord x, wxCoord y,
                              bool useMask = false) override;

    void DoDrawText(const wxString& text, wxCoord x, wxCoord y) override;
    virtual void DoDrawRotatedText(const wxString& text,
                                   wxCoord x, wxCoord y, double angle) override;

    virtual bool DoBlit(wxCoord xdest, wxCoord ydest,
                        wxCoord width, wxCoord height,
                        wxDC *source,
                        wxCoord xsrc, wxCoord ysrc,
                        wxRasterOperationMode rop = wxRasterOperationMode::Copy,
                        bool useMask = false,
                        wxCoord xsrcMask = wxDefaultCoord,
                        wxCoord ysrcMask = wxDefaultCoord) override;

    virtual void DoDrawLines(int n, const wxPoint points[],
                             wxCoord xoffset, wxCoord yoffset ) override;

    virtual void DoDrawPolygon(int n, const wxPoint points[],
                           wxCoord xoffset, wxCoord yoffset,
                           wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven) override;

    // Use Qt transformations, as they automatically scale pen widths, text...
    void ComputeScaleAndOrigin() override;

    void QtPreparePainter();

    void* GetHandle() const override { return (void*) m_qtPainter; }

protected:
    virtual QPixmap *GetQPixmap() { return m_qtPixmap; }

    QPainter *m_qtPainter;
    QPixmap *m_qtPixmap;

    wxRegion m_clippingRegion;
private:
    enum wxQtRasterColourOp
    {
        wxQtNONE,
        wxQtWHITE,
        wxQtBLACK,
        wxQtINVERT
    };
    wxQtRasterColourOp m_rasterColourOp;
    QColor *m_qtPenColor;
    QColor *m_qtBrushColor;
    void ApplyRasterColourOp();

    wxDECLARE_CLASS(wxQtDCImpl);
    wxQtDCImpl(const wxQtDCImpl&) = delete;
	wxQtDCImpl& operator=(const wxQtDCImpl&) = delete;

};

#endif // _WX_QT_DC_H_
