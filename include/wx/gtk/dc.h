/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/dc.h
// Purpose:
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTKDC_H_
#define _WX_GTKDC_H_

#ifdef __WXGTK3__

#include "wx/dcgraph.h"

class wxGTKCairoDCImpl: public wxGCDCImpl
{
    typedef wxGCDCImpl BaseType;
public:
    wxGTKCairoDCImpl(wxDC* owner);
    wxGTKCairoDCImpl(wxDC* owner, wxWindow* window, wxLayoutDirection dir = wxLayoutDirection::Default, int width = 0);

    void DoDrawBitmap(const wxBitmap& bitmap, int x, int y, bool useMask) override;
    void DoDrawCheckMark(int x, int y, int width, int height) override;
    void DoDrawIcon(const wxIcon& icon, int x, int y) override;
    void DoDrawText(const wxString& text, int x, int y) override;
    void DoDrawRotatedText(const wxString& text, int x, int y, double angle) override;
#if wxUSE_IMAGE
    bool DoFloodFill(int x, int y, const wxColour& col, wxFloodFillStyle style) override;
#endif
    wxBitmap DoGetAsBitmap(const wxRect* subrect) const override;
    bool DoGetPixel(int x, int y, wxColour* col) const override;
    wxSize DoGetSize() const override;
    bool DoStretchBlit(int xdest, int ydest, int dstWidth, int dstHeight, wxDC* source, int xsrc, int ysrc, int srcWidth, int srcHeight, wxRasterOperationMode rop, bool useMask, int xsrcMask, int ysrcMask) override;
    void* GetCairoContext() const override;

    wxSize GetPPI() const override;
    void SetLayoutDirection(wxLayoutDirection dir) override;
    wxLayoutDirection GetLayoutDirection() const override;

protected:
    // Set m_size from the given (valid) GdkWindow.
    void InitSize(GdkWindow* window);
    void AdjustForRTL(cairo_t* cr);

    wxSize m_size;
    wxLayoutDirection m_layoutDir;

    wxGTKCairoDCImpl(const wxGTKCairoDCImpl&) = delete;
	wxGTKCairoDCImpl& operator=(const wxGTKCairoDCImpl&) = delete;
};
//-----------------------------------------------------------------------------

class wxWindowDCImpl: public wxGTKCairoDCImpl
{
public:
    wxWindowDCImpl(wxWindowDC* owner, wxWindow* window);

    wxWindowDCImpl(const wxWindowDCImpl&) = delete;
	wxWindowDCImpl& operator=(const wxWindowDCImpl&) = delete;
};
//-----------------------------------------------------------------------------

class wxClientDCImpl: public wxGTKCairoDCImpl
{
public:
    wxClientDCImpl(wxClientDC* owner, wxWindow* window);

    wxClientDCImpl(const wxClientDCImpl&) = delete;
	wxClientDCImpl& operator=(const wxClientDCImpl&) = delete;
};
//-----------------------------------------------------------------------------

class wxPaintDCImpl: public wxGTKCairoDCImpl
{
    typedef wxGTKCairoDCImpl BaseType;
public:
    wxPaintDCImpl(wxPaintDC* owner, wxWindow* window);
    void DestroyClippingRegion() override;

private:
    const wxRegion& m_clip;
};
//-----------------------------------------------------------------------------

class wxScreenDCImpl: public wxGTKCairoDCImpl
{
public:
    wxScreenDCImpl(wxScreenDC* owner);

    wxSize GetPPI() const override;

    wxScreenDCImpl(const wxScreenDCImpl&) = delete;
	wxScreenDCImpl& operator=(const wxScreenDCImpl&) = delete;
};
//-----------------------------------------------------------------------------

class wxMemoryDCImpl: public wxGTKCairoDCImpl
{
public:
    wxMemoryDCImpl(wxMemoryDC* owner);
    wxMemoryDCImpl(wxMemoryDC* owner, wxBitmap& bitmap);
    wxMemoryDCImpl(wxMemoryDC* owner, wxDC* dc);
    wxBitmap DoGetAsBitmap(const wxRect* subrect) const override;
    void DoSelect(const wxBitmap& bitmap) override;
    const wxBitmap& GetSelectedBitmap() const override;
    wxBitmap& GetSelectedBitmap() override;

private:
    void Setup();
    wxBitmap m_bitmap;

    wxMemoryDCImpl(const wxMemoryDCImpl&) = delete;
	wxMemoryDCImpl& operator=(const wxMemoryDCImpl&) = delete;
};
//-----------------------------------------------------------------------------

class wxGTKCairoDC: public wxDC
{
public:
    wxGTKCairoDC(cairo_t* cr, wxWindow* window, wxLayoutDirection dir = wxLayoutDirection::LeftToRight, int width = 0);

    wxGTKCairoDC(const wxGTKCairoDC&) = delete;
	wxGTKCairoDC& operator=(const wxGTKCairoDC&) = delete;
};

#else

#include "wx/dc.h"

//-----------------------------------------------------------------------------
// wxDC
//-----------------------------------------------------------------------------

class wxGTKDCImpl : public wxDCImpl
{
public:
    wxGTKDCImpl( wxDC *owner );
    virtual ~wxGTKDCImpl();

#if wxUSE_PALETTE
    void SetColourMap( const wxPalette& palette ) { SetPalette(palette); }
#endif // wxUSE_PALETTE

    // Resolution in pixels per logical inch
    wxSize GetPPI() const override;

    bool StartDoc( const wxString& WXUNUSED(message) ) override { return true; }
    void EndDoc() override { }
    void StartPage() override { }
    void EndPage() override { }

    virtual GdkWindow* GetGDKWindow() const { return NULL; }
    void* GetHandle() const override { return GetGDKWindow(); }

    // base class pure virtuals implemented here
    void DoSetClippingRegion(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;
    void DoGetSizeMM(int* width, int* height) const override;

    wxDECLARE_ABSTRACT_CLASS(wxGTKDCImpl);
};

// this must be defined when wxDC::Blit() honours the DC origin and needed to
// allow wxUniv code in univ/winuniv.cpp to work with versions of wxGTK
// 2.3.[23]
#ifndef wxHAS_WORKING_GTK_DC_BLIT
    #define wxHAS_WORKING_GTK_DC_BLIT
#endif

#endif
#endif // _WX_GTKDC_H_
