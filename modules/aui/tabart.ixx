//////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/tabart.h
// Purpose:     wxaui: wx advanced user interface - notebook
// Author:      Benjamin I. Williams
// Modified by: Jens Lody (extracted from wx/aui/auibook.h)
// Created:     2012-03-21
// Copyright:   (C) Copyright 2006, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/defs.h"

#include "wx/dc.h"
#include "wx/colour.h"
#include "wx/font.h"
#include "wx/pen.h"
#include "wx/brush.h"
#include "wx/bitmap.h"
#include "wx/window.h"

export module WX.AUI.TabArt.Base;

import WX.AUI.Page;

import Utils.Geometry;

import <string>;

// tab art class

export
{

class wxAuiTabArt
{
public:
    virtual ~wxAuiTabArt() = default;

    virtual std::unique_ptr<wxAuiTabArt> Clone() = 0;
    virtual void SetFlags(unsigned int flags) = 0;

    virtual void SetSizingInfo(const wxSize& tabCtrlSize,
                               size_t tabCount) = 0;

    virtual void SetNormalFont(const wxFont& font) = 0;
    virtual void SetSelectedFont(const wxFont& font) = 0;
    virtual void SetMeasuringFont(const wxFont& font) = 0;
    virtual void SetColour(const wxColour& colour) = 0;
    virtual void SetActiveColour(const wxColour& colour) = 0;

    virtual void DrawBorder(
                 wxDC& dc,
                 wxWindow* wnd,
                 const wxRect& rect) = 0;

    virtual void DrawBackground(
                         wxDC& dc,
                         wxWindow* wnd,
                         const wxRect& rect) = 0;

    virtual void DrawTab(wxDC& dc,
                         wxWindow* wnd,
                         const wxAuiNotebookPage& pane,
                         const wxRect& inRect,
                         int closeButtonState,
                         wxRect* outTabRect,
                         wxRect* outButtonRect,
                         int* xExtent) = 0;

    virtual void DrawButton(
                         wxDC& dc,
                         wxWindow* wnd,
                         const wxRect& inRect,
                         int bitmapId,
                         int buttonState,
                         int orientation,
                         wxRect* outRect) = 0;

    virtual wxSize GetTabSize(
                         wxDC& dc,
                         wxWindow* wnd,
                         const std::string& caption,
                         const wxBitmap& bitmap,
                         bool active,
                         int closeButtonState,
                         int* xExtent) = 0;

    virtual int ShowDropDown(
                         wxWindow* wnd,
                         const wxAuiNotebookPageArray& items,
                         int activeIdx) = 0;

    virtual int GetIndentSize() = 0;

    virtual int GetBorderWidth(
                         wxWindow* wnd) = 0;

    virtual int GetAdditionalBorderSpace(
                         wxWindow* wnd) = 0;

    virtual int GetBestTabCtrlSize(
                         wxWindow* wnd,
                         const wxAuiNotebookPageArray& pages,
                         const wxSize& requiredBmpSize) = 0;

    // Provide opportunity for subclasses to recalculate colours
    virtual void UpdateColoursFromSystem() {}
};


class wxAuiGenericTabArt : public wxAuiTabArt
{
public:
    wxAuiGenericTabArt();

    std::unique_ptr<wxAuiTabArt> Clone() override;
    void SetFlags(unsigned int flags) override;
    void SetSizingInfo(const wxSize& tabCtrlSize,
                       size_t tabCount) override;

    void SetNormalFont(const wxFont& font) override;
    void SetSelectedFont(const wxFont& font) override;
    void SetMeasuringFont(const wxFont& font) override;
    void SetColour(const wxColour& colour) override;
    void SetActiveColour(const wxColour& colour) override;

    void DrawBorder(
                 wxDC& dc,
                 wxWindow* wnd,
                 const wxRect& rect) override;

    void DrawBackground(
                 wxDC& dc,
                 wxWindow* wnd,
                 const wxRect& rect) override;

    void DrawTab(wxDC& dc,
                 wxWindow* wnd,
                 const wxAuiNotebookPage& pane,
                 const wxRect& inRect,
                 int closeButtonState,
                 wxRect* outTabRect,
                 wxRect* outButtonRect,
                 int* xExtent) override;

    void DrawButton(
                 wxDC& dc,
                 wxWindow* wnd,
                 const wxRect& inRect,
                 int bitmapId,
                 int buttonState,
                 int orientation,
                 wxRect* outRect) override;

    int GetIndentSize() override;

    int GetBorderWidth(
                 wxWindow* wnd) override;

    int GetAdditionalBorderSpace(
                 wxWindow* wnd) override;

    wxSize GetTabSize(
                 wxDC& dc,
                 wxWindow* wnd,
                 const std::string& caption,
                 const wxBitmap& bitmap,
                 bool active,
                 int closeButtonState,
                 int* xExtent) override;

    int ShowDropDown(
                 wxWindow* wnd,
                 const wxAuiNotebookPageArray& items,
                 int activeIdx) override;

    int GetBestTabCtrlSize(wxWindow* wnd,
                 const wxAuiNotebookPageArray& pages,
                 const wxSize& requiredBmpSize) override;

    // Provide opportunity for subclasses to recalculate colours
    void UpdateColoursFromSystem() override;

protected:

    wxFont m_normalFont;
    wxFont m_selectedFont;
    wxFont m_measuringFont;
    
    wxColour m_activeColour;
    wxColour m_baseColour;
    
    wxPen m_baseColourPen;
    wxPen m_borderPen;
    
    wxBrush m_baseColourBrush;
    
    wxBitmap m_activeCloseBmp;
    wxBitmap m_disabledCloseBmp;
    wxBitmap m_activeLeftBmp;
    wxBitmap m_disabledLeftBmp;
    wxBitmap m_activeRightBmp;
    wxBitmap m_disabledRightBmp;
    wxBitmap m_activeWindowListBmp;
    wxBitmap m_disabledWindowListBmp;

    int m_fixedTabWidth;
    int m_tabCtrlHeight;
    
    unsigned int m_flags;
};


class wxAuiSimpleTabArt : public wxAuiTabArt
{
public:
    wxAuiSimpleTabArt();

    std::unique_ptr<wxAuiTabArt> Clone() override;
    void SetFlags(unsigned int flags) override;

    void SetSizingInfo(const wxSize& tabCtrlSize,
                       size_t tabCount) override;

    void SetNormalFont(const wxFont& font) override;
    void SetSelectedFont(const wxFont& font) override;
    void SetMeasuringFont(const wxFont& font) override;
    void SetColour(const wxColour& colour) override;
    void SetActiveColour(const wxColour& colour) override;

    void DrawBorder(
                 wxDC& dc,
                 wxWindow* wnd,
                 const wxRect& rect) override;

    void DrawBackground(
                 wxDC& dc,
                 wxWindow* wnd,
                 const wxRect& rect) override;

    void DrawTab(wxDC& dc,
                 wxWindow* wnd,
                 const wxAuiNotebookPage& pane,
                 const wxRect& inRect,
                 int closeButtonState,
                 wxRect* outTabRect,
                 wxRect* outButtonRect,
                 int* xExtent) override;

    void DrawButton(
                 wxDC& dc,
                 wxWindow* wnd,
                 const wxRect& inRect,
                 int bitmapId,
                 int buttonState,
                 int orientation,
                 wxRect* outRect) override;

    int GetIndentSize() override;

    int GetBorderWidth(
                 wxWindow* wnd) override;

    int GetAdditionalBorderSpace(
                 wxWindow* wnd) override;

    wxSize GetTabSize(
                 wxDC& dc,
                 wxWindow* wnd,
                 const std::string& caption,
                 const wxBitmap& bitmap,
                 bool active,
                 int closeButtonState,
                 int* xExtent) override;

    int ShowDropDown(
                 wxWindow* wnd,
                 const wxAuiNotebookPageArray& items,
                 int activeIdx) override;

    int GetBestTabCtrlSize(wxWindow* wnd,
                 const wxAuiNotebookPageArray& pages,
                 const wxSize& requiredBmpSize) override;

protected:

    wxFont m_normalFont;
    wxFont m_selectedFont;
    wxFont m_measuringFont;

    wxPen m_normalBkPen;
    wxPen m_selectedBkPen;

    wxBrush m_normalBkBrush;
    wxBrush m_selectedBkBrush;
    wxBrush m_bkBrush;

    wxBitmap m_activeCloseBmp;
    wxBitmap m_disabledCloseBmp;
    wxBitmap m_activeLeftBmp;
    wxBitmap m_disabledLeftBmp;
    wxBitmap m_activeRightBmp;
    wxBitmap m_disabledRightBmp;
    wxBitmap m_activeWindowListBmp;
    wxBitmap m_disabledWindowListBmp;

    int m_fixedTabWidth;
    unsigned int m_flags;
};

#ifndef __WXUNIVERSAL__
    #if defined(__WXGTK20__) && !defined(__WXGTK3__)
        #define wxHAS_NATIVE_TABART
    #elif defined(__WXMSW__) && wxUSE_UXTHEME
        #define wxHAS_NATIVE_TABART
    #endif
#endif // !__WXUNIVERSAL__

#ifndef wxHAS_NATIVE_TABART
    using wxAuiDefaultTabArt = wxAuiGenericTabArt;
#endif

} // export
