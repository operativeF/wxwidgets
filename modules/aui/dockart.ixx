///////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/dockart.h
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by:
// Created:     2005-05-17
// Copyright:   (C) Copyright 2005, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/brush.h"
#include "wx/bitmap.h"
#include "wx/colour.h"
#include "wx/dc.h"
#include "wx/font.h"
#include "wx/pen.h"
#include "wx/window.h"

#ifdef __WXMAC__
#include "wx/osx/private.h"
#include "wx/graphics.h"
#include "wx/dcgraph.h"
#endif

#ifdef __WXGTK__
#include "wx/renderer.h"
#ifdef __WXGTK20__
    #include "wx/gtk/private/wrapgtk.h"
#else
    #include <gtk/gtk.h>
    #define gtk_widget_is_drawable GTK_WIDGET_DRAWABLE
#endif
#ifdef __WXGTK3__
    #include "wx/graphics.h"
    #include "wx/gtk/private.h"
#endif
#endif

export module WX.AUI.DockArt;

import WX.AUI.Book;
import WX.AUI.Flags;
import WX.AUI.FrameManager;
import WX.AUI.PaneInfo;
import WX.AUI.TabArt;

import WX.Utils.Settings;

import WX.Image;
import Utils.Geometry;

import <cmath>;


export
{

enum wxAuiPaneDockArtSetting
{
    wxAUI_DOCKART_SASH_SIZE = 0,
    wxAUI_DOCKART_CAPTION_SIZE = 1,
    wxAUI_DOCKART_GRIPPER_SIZE = 2,
    wxAUI_DOCKART_PANE_BORDER_SIZE = 3,
    wxAUI_DOCKART_PANE_BUTTON_SIZE = 4,
    wxAUI_DOCKART_BACKGROUND_COLOUR = 5,
    wxAUI_DOCKART_SASH_COLOUR = 6,
    wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR = 7,
    wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR = 8,
    wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR = 9,
    wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR = 10,
    wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR = 11,
    wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR = 12,
    wxAUI_DOCKART_BORDER_COLOUR = 13,
    wxAUI_DOCKART_GRIPPER_COLOUR = 14,
    wxAUI_DOCKART_CAPTION_FONT = 15,
    wxAUI_DOCKART_GRADIENT_TYPE = 16
};

wxBitmap wxAuiBitmapFromBits(const unsigned char bits[], wxSize sz,
                             const wxColour& color);

// dock art provider code - a dock provider provides all drawing
// functionality to the wxAui dock manager.  This allows the dock
// manager to have pluggable look-and-feels

class wxAuiDockArt
{
public:
    virtual ~wxAuiDockArt() = default;

    virtual std::unique_ptr<wxAuiDockArt> Clone() = 0;
    virtual int GetMetric(int id) = 0;
    virtual void SetMetric(int id, int newVal) = 0;
    virtual void SetFont(int id, const wxFont& font) = 0;
    virtual wxFont GetFont(int id) = 0;
    virtual wxColour GetColour(int id) = 0;
    virtual void SetColour(int id, const wxColor& colour) = 0;
    wxColour GetColor(int id) { return GetColour(id); }
    void SetColor(int id, const wxColour& color) { SetColour(id, color); }

    virtual void DrawSash(wxDC& dc,
                          wxWindow* window,
                          int orientation,
                          const wxRect& rect) = 0;

    virtual void DrawBackground(wxDC& dc,
                          wxWindow* window,
                          int orientation,
                          const wxRect& rect) = 0;

    virtual void DrawCaption(wxDC& dc,
                          wxWindow* window,
                          const std::string& text,
                          const wxRect& rect,
                          wxAuiPaneInfo& pane) = 0;

    virtual void DrawGripper(wxDC& dc,
                          wxWindow* window,
                          const wxRect& rect,
                          wxAuiPaneInfo& pane) = 0;

    virtual void DrawBorder(wxDC& dc,
                          wxWindow* window,
                          const wxRect& rect,
                          wxAuiPaneInfo& pane) = 0;

    virtual void DrawPaneButton(wxDC& dc,
                          wxWindow* window,
                          int button,
                          int buttonState,
                          const wxRect& rect,
                          wxAuiPaneInfo& pane) = 0;

    // Provide opportunity for subclasses to recalculate colours
    virtual void UpdateColoursFromSystem() {}
};


// this is the default art provider for wxAuiManager.  Dock art
// can be customized by creating a class derived from this one,
// or replacing this class entirely

class wxAuiDefaultDockArt : public wxAuiDockArt
{
public:
    wxAuiDefaultDockArt();

    std::unique_ptr<wxAuiDockArt> Clone() override;
    int GetMetric(int metricId) override;
    void SetMetric(int metricId, int newVal) override;
    wxColour GetColour(int id) override;
    void SetColour(int id, const wxColor& colour) override;
    void SetFont(int id, const wxFont& font) override;
    wxFont GetFont(int id) override;

    void DrawSash(wxDC& dc,
                  wxWindow *window,
                  int orientation,
                  const wxRect& rect) override;

    void DrawBackground(wxDC& dc,
                  wxWindow *window,
                  int orientation,
                  const wxRect& rect) override;

    void DrawCaption(wxDC& dc,
                  wxWindow *window,
                  const std::string& text,
                  const wxRect& rect,
                  wxAuiPaneInfo& pane) override;

    void DrawGripper(wxDC& dc,
                  wxWindow *window,
                  const wxRect& rect,
                  wxAuiPaneInfo& pane) override;

    void DrawBorder(wxDC& dc,
                  wxWindow *window,
                  const wxRect& rect,
                  wxAuiPaneInfo& pane) override;

    void DrawPaneButton(wxDC& dc,
                  wxWindow *window,
                  int button,
                  int buttonState,
                  const wxRect& rect,
                  wxAuiPaneInfo& pane) override;

    void UpdateColoursFromSystem() override;


protected:

    void DrawCaptionBackground(wxDC& dc, const wxRect& rect, bool active);

    void DrawIcon(wxDC& dc, wxWindow *window, const wxRect& rect, wxAuiPaneInfo& pane);

    void InitBitmaps();

protected:
    wxBrush m_sashBrush;
    wxBrush m_backgroundBrush;
    wxBrush m_gripperBrush;

    wxFont m_captionFont;

    wxBitmap m_inactiveCloseBitmap;
    wxBitmap m_inactivePinBitmap;
    wxBitmap m_inactiveMaximizeBitmap;
    wxBitmap m_inactiveRestoreBitmap;
    wxBitmap m_activeCloseBitmap;
    wxBitmap m_activePinBitmap;
    wxBitmap m_activeMaximizeBitmap;
    wxBitmap m_activeRestoreBitmap;

    wxPen m_gripperPen1;
    wxPen m_gripperPen2;
    wxPen m_gripperPen3;
    wxPen m_borderPen;

    wxColour m_baseColour;
    wxColour m_activeCaptionColour;
    wxColour m_activeCaptionGradientColour;
    wxColour m_activeCaptionTextColour;
    wxColour m_inactiveCaptionColour;
    wxColour m_inactiveCaptionGradientColour;
    wxColour m_inactiveCaptionTextColour;

    int m_borderSize;
    int m_captionSize;
    int m_sashSize;
    int m_buttonSize;
    int m_gripperSize;
    int m_gradientType;
};

wxColor wxAuiLightContrastColour(const wxColour& c)
{
    int amount = 120;

    // if the color is especially dark, then
    // make the contrast even lighter
    if (c.Red() < 128 && c.Green() < 128 && c.Blue() < 128)
        amount = 160;

    return c.ChangeLightness(amount);
}

inline float wxAuiGetSRGB(float r) {
    return r <= 0.03928f ? r / 12.92f : std::pow((r + 0.055f) / 1.055f, 2.4f);
}

float wxAuiGetRelativeLuminance(const wxColour& c)
{
    // based on https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef
    return
        0.2126f * wxAuiGetSRGB(c.Red()   / 255.0f) +
        0.7152f * wxAuiGetSRGB(c.Green() / 255.0f) +
        0.0722f * wxAuiGetSRGB(c.Blue()  / 255.0f);
}

float wxAuiGetColourContrast(const wxColour& c1, const wxColour& c2)
{
    // based on https://www.w3.org/TR/UNDERSTANDING-WCAG20/visual-audio-contrast7.html
    float L1 = wxAuiGetRelativeLuminance(c1);
    float L2 = wxAuiGetRelativeLuminance(c2);
    return L1 > L2 ? (L1 + 0.05f) / (L2 + 0.05f) : (L2 + 0.05f) / (L1 + 0.05f);
}

// wxAuiBitmapFromBits() is a utility function that creates a
// masked bitmap from raw bits (XBM format)
wxBitmap wxAuiBitmapFromBits(const unsigned char bits[], wxSize sz,
                             const wxColour& color)
{
    wxImage img = wxBitmap((const char*)bits, sz).ConvertToImage();
    if (color.Alpha() == wxALPHA_OPAQUE)
    {
        img.Replace(0,0,0,123,123,123);
        img.Replace(255,255,255,color.Red(),color.Green(),color.Blue());
        img.SetMaskColour(123,123,123);
    }
    else
    {
        img.InitAlpha();
        const int newr = color.Red();
        const int newg = color.Green();
        const int newb = color.Blue();
        const int newa = color.Alpha();
        for (int x = 0; x < sz.x; x++)
        {
            for (int y = 0; y < sz.y; y++)
            {
                int r = img.GetRed(x, y);
                int g = img.GetGreen(x, y);
                int b = img.GetBlue(x, y);
                if (r == 0 && g == 0 && b == 0)
                {
                    img.SetAlpha(x, y, wxALPHA_TRANSPARENT);
                }
                else
                {
                    img.SetRGB(x, y, newr, newg, newb);
                    img.SetAlpha(x, y, newa);
                }
            }
        }
    }
    return {img};
}

// A utility function to scales a bitmap in place for use at the given scale
// factor.
void wxAuiScaleBitmap(wxBitmap& bmp, double scale)
{
#if wxUSE_IMAGE && !defined(__WXGTK3__) && !defined(__WXMAC__)
    // scale to a close round number to improve quality
    scale = std::floor(scale + 0.25);
    if (scale > 1.0 && !(bmp.GetScaleFactor() > 1.0))
    {
        wxImage img = bmp.ConvertToImage();
        img.Rescale(std::lround(bmp.GetWidth() * scale), std::lround(bmp.GetHeight() * scale),
            wxImageResizeQuality::BoxAverage);
        bmp = wxBitmap(img);
    }
#else
    wxUnusedVar(bmp);
    wxUnusedVar(scale);
#endif // wxUSE_IMAGE
}

void DrawGradientRectangle(wxDC& dc,
                                  const wxRect& rect,
                                  const wxColour& start_color,
                                  const wxColour& end_color,
                                  int direction)
{
    int rd, gd, bd, high = 0;
    rd = end_color.Red() - start_color.Red();
    gd = end_color.Green() - start_color.Green();
    bd = end_color.Blue() - start_color.Blue();

    if (direction == wxAUI_GRADIENT_VERTICAL)
        high = rect.GetHeight()-1;
    else
        high = rect.GetWidth()-1;

    for (int i = 0; i <= high; ++i)
    {
        int r = start_color.Red() + (high <= 0 ? 0 : (((i*rd*100)/high)/100));
        int g = start_color.Green() + (high <= 0 ? 0 : (((i*gd*100)/high)/100));
        int b = start_color.Blue() + (high <= 0 ? 0 : (((i*bd*100)/high)/100));

        wxPen p(wxColor((unsigned char)r,
                        (unsigned char)g,
                        (unsigned char)b));
        dc.SetPen(p);

        if (direction == wxAUI_GRADIENT_VERTICAL)
            dc.DrawLine(rect.x, rect.y+i, rect.x+rect.width, rect.y+i);
        else
            dc.DrawLine(rect.x+i, rect.y, rect.x+i, rect.y+rect.height);
    }
}

std::string wxAuiChopText(wxDC& dc, const std::string& text, int max_size)
{
    wxCoord x, y;

    // first check if the text fits with no problems
    dc.GetTextExtent(text, &x, &y);
    if (x <= max_size)
        return text;

    size_t last_good_length = 0;
    for (size_t i = 0; i < text.length(); ++i)
    {
        std::string s = text.substr(0, i);
        s += "...";

        dc.GetTextExtent(s, &x, &y);
        if (x > max_size)
            break;

        last_good_length = i;
    }

    std::string ret = text.substr(0, last_good_length);
    ret += "...";
    return ret;
}

} // export
