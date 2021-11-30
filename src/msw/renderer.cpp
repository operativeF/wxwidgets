///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/renderer.cpp
// Purpose:     implementation of wxRendererNative for Windows
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.07.2003
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"
#include "wx/msw/wrapcctl.h"

#include "wx/window.h"
#include "wx/control.h"     // for wxControl::Ellipsize()
#include "wx/dc.h"
#include "wx/settings.h"

#include "wx/dcgraph.h"
#include "wx/scopeguard.h"
#include "wx/splitter.h"
#include "wx/renderer.h"
#include "wx/msw/uxtheme.h"
#include "wx/dynlib.h"

import WX.WinDef;

import <string>;

// These Vista+ only types used by DrawThemeTextEx may not be available in older SDK headers
using WXDTT_CALLBACK_PROC = int(__stdcall*)(WXHDC hdc, const wchar_t * pszText,
    int cchText, RECT * prc, unsigned int dwFlags, WXLPARAM lParam);

// FIXME: Figure out what is redundant with new SDKs.
typedef struct _WXDTTOPTS
{
    WXDWORD             dwSize;
    WXDWORD             dwFlags;
    COLORREF          crText;
    COLORREF          crBorder;
    COLORREF          crShadow;
    int               iTextShadowType;
    POINT             ptShadowOffset;
    int               iBorderSize;
    int               iFontPropId;
    int               iColorPropId;
    int               iStateId;
    BOOL              fApplyOverlay;
    int               iGlowSize;
    WXDTT_CALLBACK_PROC pfnDrawTextCallback;
    WXLPARAM          lParam;
} WXDTTOPTS, *WXPDTTOPTS;


// ----------------------------------------------------------------------------
// methods common to wxRendererMSW and wxRendererXP
// ----------------------------------------------------------------------------

class wxRendererMSWBase : public wxDelegateRendererNative
{
public:
    wxRendererMSWBase() = default;

    explicit wxRendererMSWBase(wxRendererNative& rendererNative)
        : wxDelegateRendererNative(rendererNative)
    {}

    void DrawFocusRect(wxWindow * win,
                        wxDC& dc,
                        const wxRect& rect,
                        unsigned int flags = 0) override;

    void DrawItemSelectionRect(wxWindow *win,
                                wxDC& dc,
                                const wxRect& rect,
                                unsigned int flags = 0) override;

    void DrawChoice(wxWindow* win,
                     wxDC& dc,
                     const wxRect& rect,
                     unsigned int flags = 0) override;

    void DrawComboBox(wxWindow* win,
                       wxDC& dc,
                       const wxRect& rect,
                       unsigned int flags = 0) override;

    void DrawComboBoxDropButton(wxWindow *win,
                                         wxDC& dc,
                                         const wxRect& rect,
                                         unsigned int flags = 0) override = 0;

protected:
    // Helper function returning the MSW RECT corresponding to the wxRect
    // adjusted for the given wxDC.
    static RECT ConvertToRECT(wxDC& dc, const wxRect& rect)
    {
        RECT rc;
        wxCopyRectToRECT(dc.GetImpl()->MSWApplyGDIPlusTransform(rect), rc);
        return rc;
    }
};

// ----------------------------------------------------------------------------
// wxRendererMSW: wxRendererNative implementation for "old" Win32 systems
// ----------------------------------------------------------------------------

class wxRendererMSW : public wxRendererMSWBase
{
public:
    wxRendererMSW() = default;

    wxRendererMSW(const wxRendererMSW&) = delete;
	wxRendererMSW& operator=(const wxRendererMSW&) = delete;
    
    static wxRendererNative& Get();

    void DrawComboBoxDropButton(wxWindow *win,
                                        wxDC& dc,
                                        const wxRect& rect,
                                        unsigned int flags = 0) override;

    void DrawCheckBox(wxWindow *win,
                              wxDC& dc,
                              const wxRect& rect,
                              unsigned int flags = 0) override
    {
        DoDrawButton(DFCS_BUTTONCHECK, win, dc, rect, flags);
    }

    void DrawCheckMark(wxWindow *win,
                               wxDC& dc,
                               const wxRect& rect,
                               unsigned int flags = 0) override
    {
        DoDrawFrameControl(DFC_MENU, DFCS_MENUCHECK, win, dc, rect, flags);
    }

    void DrawPushButton(wxWindow *win,
                                wxDC& dc,
                                const wxRect& rect,
                                unsigned int flags = 0) override;

    void DrawRadioBitmap(wxWindow* win,
                                 wxDC& dc,
                                 const wxRect& rect,
                                 unsigned int flags = 0) override
    {
        DoDrawButton(DFCS_BUTTONRADIO, win, dc, rect, flags);
    }

    void DrawTitleBarBitmap(wxWindow *win,
                                    wxDC& dc,
                                    const wxRect& rect,
                                    wxTitleBarButton button,
                                    unsigned int flags = 0) override;

    wxSize GetCheckBoxSize(wxWindow *win, unsigned int flags = 0) override;

    int GetHeaderButtonHeight(wxWindow *win) override;

    int GetHeaderButtonMargin(wxWindow *win) override;

private:
    // wrapper of DrawFrameControl()
    void DoDrawFrameControl(WXUINT type,
                            WXUINT kind,
                            wxWindow *win,
                            wxDC& dc,
                            const wxRect& rect,
                            unsigned int flags);

    // common part of Draw{PushButton,CheckBox,RadioBitmap}(): wraps
    // DrawFrameControl(DFC_BUTTON)
    void DoDrawButton(WXUINT kind,
                      wxWindow *win,
                      wxDC& dc,
                      const wxRect& rect,
                      unsigned int flags)
    {
        DoDrawFrameControl(DFC_BUTTON, kind, win, dc, rect, flags);
    }
};

// ----------------------------------------------------------------------------
// wxRendererXP: wxRendererNative implementation for Windows XP and later
// ----------------------------------------------------------------------------

#if wxUSE_UXTHEME

class wxRendererXP : public wxRendererMSWBase
{
public:
    wxRendererXP() : wxRendererMSWBase(wxRendererMSW::Get()) {}

    wxRendererXP(const wxRendererXP&) = delete;
	wxRendererXP& operator=(const wxRendererXP&) = delete;

    static wxRendererNative& Get();

    int DrawHeaderButton(wxWindow *win,
                                  wxDC& dc,
                                  const wxRect& rect,
                                  unsigned int flags = 0,
                                  wxHeaderSortIconType sortArrow = wxHeaderSortIconType::None,
                                  wxHeaderButtonParams* params = nullptr) override;

    void DrawTreeItemButton(wxWindow *win,
                                    wxDC& dc,
                                    const wxRect& rect,
                                    unsigned int flags = 0) override;
    void DrawSplitterBorder(wxWindow *win,
                                    wxDC& dc,
                                    const wxRect& rect,
                                    unsigned int flags = 0) override;
    void DrawSplitterSash(wxWindow *win,
                                  wxDC& dc,
                                  const wxSize& size,
                                  wxCoord position,
                                  wxOrientation orient,
                                  unsigned int flags = 0) override;
    void DrawComboBoxDropButton(wxWindow *win,
                                        wxDC& dc,
                                        const wxRect& rect,
                                        unsigned int flags = 0) override;
    void DrawCheckBox(wxWindow *win,
                              wxDC& dc,
                              const wxRect& rect,
                              unsigned int flags = 0) override
    {
        if ( !DoDrawXPButton(BP_CHECKBOX, win, dc, rect, flags) )
            m_rendererNative.DrawCheckBox(win, dc, rect, flags);
    }

    void DrawCheckMark(wxWindow *win,
                               wxDC& dc,
                               const wxRect& rect,
                               unsigned int flags = 0) override
    {
        if ( !DoDrawCheckMark(MENU_POPUPCHECK, win, dc, rect, flags) )
            m_rendererNative.DrawCheckMark(win, dc, rect, flags);
    }

    void DrawPushButton(wxWindow *win,
                                wxDC& dc,
                                const wxRect& rect,
                                unsigned int flags = 0) override
    {
        if ( !DoDrawXPButton(BP_PUSHBUTTON, win, dc, rect, flags) )
            m_rendererNative.DrawPushButton(win, dc, rect, flags);
    }

    void DrawCollapseButton(wxWindow *win,
                                    wxDC& dc,
                                    const wxRect& rect,
                                    unsigned int flags = 0) override;

    wxSize GetCollapseButtonSize(wxWindow *win, wxDC& dc) override;

    void DrawItemSelectionRect(wxWindow *win,
                                       wxDC& dc,
                                       const wxRect& rect,
                                       unsigned int flags = 0) override;

    void DrawTextCtrl(wxWindow* win,
                              wxDC& dc,
                              const wxRect& rect,
                              unsigned int flags = 0) override;

    void DrawRadioBitmap(wxWindow *win,
                                 wxDC& dc,
                                 const wxRect& rect,
                                 unsigned int flags = 0) override
    {
        if ( !DoDrawXPButton(BP_RADIOBUTTON, win, dc, rect, flags) )
            m_rendererNative.DrawRadioBitmap(win, dc, rect, flags);
    }

    void DrawTitleBarBitmap(wxWindow *win,
                                    wxDC& dc,
                                    const wxRect& rect,
                                    wxTitleBarButton button,
                                    unsigned int flags = 0) override;

    wxSize GetCheckBoxSize(wxWindow *win, unsigned int flags = 0) override;

    wxSize GetCheckMarkSize(wxWindow* win) override;

    wxSize GetExpanderSize(wxWindow *win) override;

    void DrawGauge(wxWindow* win,
                           wxDC& dc,
                           const wxRect& rect,
                           int value,
                           int max,
                           unsigned int flags = 0) override;

    void DrawItemText(wxWindow* win,
                              wxDC& dc,
                              const std::string& text,
                              const wxRect& rect,
                              unsigned int align = wxALIGN_LEFT | wxALIGN_TOP,
                              unsigned int flags = 0,
                              wxEllipsizeMode ellipsizeMode = wxEllipsizeMode::End) override;

    wxSplitterRenderParams GetSplitterParams(const wxWindow *win) override;

private:
    // wrapper around DrawThemeBackground() translating flags to NORMAL/HOT/
    // PUSHED/DISABLED states (and so suitable for drawing anything
    // button-like)
    void DoDrawButtonLike(HTHEME htheme,
                          int part,
                          wxDC& dc,
                          const wxRect& rect,
                          unsigned int flags);

    // common part of DrawCheckBox(), DrawPushButton() and DrawRadioBitmap()
    bool DoDrawXPButton(int kind,
                        wxWindow *win,
                        wxDC& dc,
                        const wxRect& rect,
                        unsigned int flags);

    bool DoDrawCheckMark(int kind,
                         wxWindow *win,
                         wxDC& dc,
                         const wxRect& rect,
                         unsigned int flags);
};

#endif // wxUSE_UXTHEME


// ============================================================================
// wxRendererMSWBase implementation
// ============================================================================

void wxRendererMSWBase::DrawFocusRect([[maybe_unused]] wxWindow * win,
                                      wxDC& dc,
                                      const wxRect& rect,
                                      [[maybe_unused]] unsigned int flags)
{
    RECT rc = ConvertToRECT(dc, rect);

    ::DrawFocusRect(GetHdcOf(dc.GetTempHDC()), &rc);
}

void wxRendererMSWBase::DrawItemSelectionRect(wxWindow *win,
                                              wxDC& dc,
                                              const wxRect& rect,
                                              unsigned int flags)
{
    if ( flags & wxCONTROL_CELL )
    {
        m_rendererNative.DrawItemSelectionRect(win, dc, rect, flags);
        return;
    }

    wxBrush brush;
    if ( flags & wxCONTROL_SELECTED )
    {
        if ( flags & wxCONTROL_FOCUSED )
        {
            brush = wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
        }
        else // !focused
        {
            brush = wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
        }
    }
    else // !selected
    {
        brush = *wxTRANSPARENT_BRUSH;
    }

    dc.SetBrush(brush);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle( rect );

    if ((flags & wxCONTROL_FOCUSED) && (flags & wxCONTROL_CURRENT))
        DrawFocusRect( win, dc, rect, flags );
}

void wxRendererMSWBase::DrawChoice(wxWindow* win,
                                   wxDC& dc,
                                   const wxRect& rect,
                                   unsigned int flags)
{
    DrawComboBox(win, dc, rect, flags);
}

void wxRendererMSWBase::DrawComboBox(wxWindow* win,
                                     wxDC& dc,
                                     const wxRect& rect,
                                     unsigned int flags)
{
    // Draw the main part of the control same as TextCtrl
    DrawTextCtrl(win, dc, rect, flags);

    // Draw the button inside the border, on the right side
    wxRect br(rect);
    br.height -= 2;
    br.x += br.width - br.height - 1;
    br.width = br.height;
    br.y += 1;

    DrawComboBoxDropButton(win, dc, br, flags);
}

// ============================================================================
// wxRendererNative and wxRendererMSW implementation
// ============================================================================

/* static */
wxRendererNative& wxRendererNative::GetDefault()
{
#if wxUSE_UXTHEME
    if ( wxUxThemeIsActive() )
        return wxRendererXP::Get();
#endif // wxUSE_UXTHEME

    return wxRendererMSW::Get();
}

/* static */
wxRendererNative& wxRendererMSW::Get()
{
    static wxRendererMSW s_rendererMSW;

    return s_rendererMSW;
}

void
wxRendererMSW::DrawComboBoxDropButton([[maybe_unused]] wxWindow * win,
                                      wxDC& dc,
                                      const wxRect& rect,
                                      unsigned int flags)
{
    wxCHECK_RET( dc.GetImpl(), "Invalid wxDC" );

    RECT r = ConvertToRECT(dc, rect);

    int style = DFCS_SCROLLCOMBOBOX;
    if ( flags & wxCONTROL_DISABLED )
        style |= DFCS_INACTIVE;
    if ( flags & wxCONTROL_PRESSED )
        style |= DFCS_PUSHED | DFCS_FLAT;

    ::DrawFrameControl(GetHdcOf(dc.GetTempHDC()), &r, DFC_SCROLL, style);
}

void
wxRendererMSW::DoDrawFrameControl(WXUINT type,
                                  WXUINT kind,
                                  [[maybe_unused]] wxWindow * win,
                                  wxDC& dc,
                                  const wxRect& rect,
                                  unsigned int flags)
{
    wxCHECK_RET( dc.GetImpl(), "Invalid wxDC" );

    RECT r = ConvertToRECT(dc, rect);

    unsigned int style = kind;
    if ( flags & wxCONTROL_CHECKED )
        style |= DFCS_CHECKED;
    if ( flags & wxCONTROL_DISABLED )
        style |= DFCS_INACTIVE;
    if ( flags & wxCONTROL_FLAT )
        style |= DFCS_MONO;
    if ( flags & wxCONTROL_PRESSED )
        style |= DFCS_PUSHED;
    if ( flags & wxCONTROL_CURRENT )
        style |= DFCS_HOT;
    if ( flags & wxCONTROL_UNDETERMINED )
        // Using DFCS_BUTTON3STATE here doesn't work (as might be expected),
        // use the following two styles to get the same look of a check box
        // in the undetermined state.
        style |= DFCS_INACTIVE | DFCS_CHECKED;

    ::DrawFrameControl(GetHdcOf(dc.GetTempHDC()), &r, type, style);
}

void
wxRendererMSW::DrawPushButton(wxWindow *win,
                              wxDC& dc,
                              const wxRect& rectOrig,
                              unsigned int flags)
{
    wxRect rect(rectOrig);
    if ( flags & wxCONTROL_ISDEFAULT )
    {
        // DrawFrameControl() doesn't seem to support default buttons so we
        // have to draw the border ourselves
        wxDCPenChanger pen(dc, *wxBLACK_PEN);
        wxDCBrushChanger brush(dc, *wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(rect);
        rect.Deflate(1);
    }

    DoDrawButton(DFCS_BUTTONPUSH, win, dc, rect, flags);
}

void
wxRendererMSW::DrawTitleBarBitmap(wxWindow *win,
                                  wxDC& dc,
                                  const wxRect& rect,
                                  wxTitleBarButton button,
                                  unsigned int flags)
{
    WXUINT kind;
    switch ( button )
    {
        case wxTitleBarButton::Close:
            kind = DFCS_CAPTIONCLOSE;
            break;

        case wxTitleBarButton::Maximize:
            kind = DFCS_CAPTIONMAX;
            break;

        case wxTitleBarButton::Iconize:
            kind = DFCS_CAPTIONMIN;
            break;

        case wxTitleBarButton::Restore:
            kind = DFCS_CAPTIONRESTORE;
            break;

        case wxTitleBarButton::Help:
            kind = DFCS_CAPTIONHELP;
            break;

        default:
            wxFAIL_MSG( "unsupported title bar button" );
            return;
    }

    DoDrawFrameControl(DFC_CAPTION, kind, win, dc, rect, flags);
}

wxSize wxRendererMSW::GetCheckBoxSize(wxWindow* win, [[maybe_unused]] unsigned int flags)
{
    // We must have a valid window in order to return the size which is correct
    // for the display this window is on.
    wxCHECK_MSG( win, wxSize(0, 0), "Must have a valid window" );

    return {wxGetSystemMetrics(SM_CXMENUCHECK, win),
            wxGetSystemMetrics(SM_CYMENUCHECK, win)};
}

int wxRendererMSW::GetHeaderButtonHeight(wxWindow * win)
{
    // some "reasonable" value returned in case of error, it doesn't really
    // correspond to anything but it's better than returning 0
    static const int DEFAULT_HEIGHT = wxWindow::FromDIP(20, win);


    // create a temporary header window just to get its geometry
    WXHWND hwndHeader = ::CreateWindowW(WC_HEADER, nullptr, 0,
                                     0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
    if ( !hwndHeader )
        return DEFAULT_HEIGHT;

    wxON_BLOCK_EXIT1( ::DestroyWindow, hwndHeader );

    // Set the font, even if it's the default one, before measuring the window.
    wxFont font;
    if ( win )
        font = win->GetFont();
    if ( !font.IsOk() )
        wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    wxSetWindowFont(hwndHeader, font);

    // initialize the struct filled with the values by Header_Layout()
    RECT parentRect = { 0, 0, 100, 100 };
    WINDOWPOS wp = { nullptr, nullptr, 0, 0, 0, 0, 0 };
    HDLAYOUT hdl = { &parentRect, &wp };

    return Header_Layout(hwndHeader, &hdl) ? wp.cy : DEFAULT_HEIGHT;
}

int wxRendererMSW::GetHeaderButtonMargin(wxWindow *win)
{
    // The native control seems to use 3*SM_CXEDGE margins on each size.
    return 6*wxGetSystemMetrics(SM_CXEDGE, win);
}

// ============================================================================
// wxRendererXP implementation
// ============================================================================

#if wxUSE_UXTHEME

namespace
{

int GetListItemState(unsigned int flags)
{
    int itemState = (flags & wxCONTROL_CURRENT) ? LISS_HOT : LISS_NORMAL;
    if ( flags & wxCONTROL_SELECTED )
    {
        itemState = (flags & wxCONTROL_CURRENT) ? LISS_HOTSELECTED : LISS_SELECTED;
        if ( !(flags & wxCONTROL_FOCUSED) )
            itemState = LISS_SELECTEDNOTFOCUS;
    }

    if ( flags & wxCONTROL_DISABLED )
        itemState = LISS_DISABLED;

    return itemState;
}

} // anonymous namespace

/* static */
wxRendererNative& wxRendererXP::Get()
{
    static wxRendererXP s_rendererXP;

    return s_rendererXP;
}

// NOTE: There is no guarantee that the button drawn fills the entire rect (XP
// default theme, for example), so the caller should have cleared button's
// background before this call. This is quite likely a wxMSW-specific thing.
void
wxRendererXP::DrawComboBoxDropButton(wxWindow * win,
                                      wxDC& dc,
                                      const wxRect& rect,
                                      unsigned int flags)
{
    wxUxThemeHandle hTheme(win, L"COMBOBOX");
    if ( !hTheme )
    {
        m_rendererNative.DrawComboBoxDropButton(win, dc, rect, flags);
        return;
    }

    wxCHECK_RET( dc.GetImpl(), "Invalid wxDC" );

    RECT r = ConvertToRECT(dc, rect);

    int state;
    if ( flags & wxCONTROL_PRESSED )
        state = CBXS_PRESSED;
    else if ( flags & wxCONTROL_CURRENT )
        state = CBXS_HOT;
    else if ( flags & wxCONTROL_DISABLED )
        state = CBXS_DISABLED;
    else
        state = CBXS_NORMAL;

    ::DrawThemeBackground
                            (
                                hTheme,
                                GetHdcOf(dc.GetTempHDC()),
                                CP_DROPDOWNBUTTON,
                                state,
                                &r,
                                nullptr
                            );

}

int
wxRendererXP::DrawHeaderButton(wxWindow *win,
                               wxDC& dc,
                               const wxRect& rect,
                               unsigned int flags,
                               wxHeaderSortIconType sortArrow,
                               wxHeaderButtonParams* params)
{
    wxUxThemeHandle hTheme(win, L"HEADER");
    if ( !hTheme )
    {
        return m_rendererNative.DrawHeaderButton(win, dc, rect, flags, sortArrow, params);
    }

    wxCHECK_MSG( dc.GetImpl(), -1, "Invalid wxDC" );

    RECT r = ConvertToRECT(dc, rect);

    unsigned int state;
    if ( flags & wxCONTROL_PRESSED )
        state = HIS_PRESSED;
    else if ( flags & wxCONTROL_CURRENT )
        state = HIS_HOT;
    else
        state = HIS_NORMAL;
    ::DrawThemeBackground
                            (
                                hTheme,
                                GetHdcOf(dc.GetTempHDC()),
                                HP_HEADERITEM,
                                state,
                                &r,
                                nullptr
                            );

    // NOTE: Using the theme to draw HP_HEADERSORTARROW doesn't do anything.
    // Why?  If this can be fixed then draw the sort arrows using the theme
    // and then clear those flags before calling DrawHeaderButtonContents.

    // Add any extras that are specified in flags and params
    return DrawHeaderButtonContents(win, dc, rect, flags, sortArrow, params);
}


void
wxRendererXP::DrawTreeItemButton(wxWindow *win,
                                 wxDC& dc,
                                 const wxRect& rect,
                                 unsigned int flags)
{
    wxUxThemeHandle hTheme(win, L"TREEVIEW");
    if ( !hTheme )
    {
        m_rendererNative.DrawTreeItemButton(win, dc, rect, flags);
        return;
    }

    wxCHECK_RET( dc.GetImpl(), "Invalid wxDC" );

    RECT r = ConvertToRECT(dc, rect);

    unsigned int state = flags & wxCONTROL_EXPANDED ? GLPS_OPENED : GLPS_CLOSED;
    ::DrawThemeBackground
                            (
                                hTheme,
                                GetHdcOf(dc.GetTempHDC()),
                                TVP_GLYPH,
                                state,
                                &r,
                                nullptr
                            );
}

bool
wxRendererXP::DoDrawXPButton(int kind,
                             wxWindow *win,
                             wxDC& dc,
                             const wxRect& rect,
                             unsigned int flags)
{
    wxUxThemeHandle hTheme(win, L"BUTTON");
    if ( !hTheme )
        return false;

    DoDrawButtonLike(hTheme, kind, dc, rect, flags);

    return true;
}

bool
wxRendererXP::DoDrawCheckMark(int kind,
                              wxWindow *win,
                              wxDC& dc,
                              const wxRect& rect,
                              unsigned int flags)
{
    wxUxThemeHandle hTheme(win, L"MENU");
    if ( !hTheme )
        return false;

    wxCHECK_MSG( dc.GetImpl(), false, "Invalid wxDC" );

    RECT r = ConvertToRECT(dc, rect);

    int state = MC_CHECKMARKNORMAL;
    if ( flags & wxCONTROL_DISABLED )
        state = MC_CHECKMARKDISABLED;

    ::DrawThemeBackground
                            (
                                hTheme,
                                GetHdcOf(dc.GetTempHDC()),
                                kind,
                                state,
                                &r,
                                nullptr
                            );

    return true;
}

void
wxRendererXP::DoDrawButtonLike(HTHEME htheme,
                               int part,
                               wxDC& dc,
                               const wxRect& rect,
                               unsigned int flags)
{
    wxCHECK_RET( dc.GetImpl(), "Invalid wxDC" );

    RECT r = ConvertToRECT(dc, rect);

    // the base state is always 1, whether it is PBS_NORMAL,
    // {CBS,RBS}_UNCHECKEDNORMAL or CBS_NORMAL
    int state = 1;

    // XBS_XXX is followed by XBX_XXXHOT, then XBS_XXXPRESSED and DISABLED
    enum
    {
        NORMAL_OFFSET,
        HOT_OFFSET,
        PRESSED_OFFSET,
        DISABLED_OFFSET,
        STATES_COUNT
    };

    // in both RBS_ and CBS_ enums CHECKED elements are offset by 4 from base
    // (UNCHECKED) ones and MIXED are offset by 4 again as there are all states
    // from the above enum in between them
    if ( flags & wxCONTROL_CHECKED )
        state += STATES_COUNT;
    else if ( flags & wxCONTROL_UNDETERMINED )
        state += 2*STATES_COUNT;

    if ( flags & wxCONTROL_DISABLED )
        state += DISABLED_OFFSET;
    else if ( flags & wxCONTROL_PRESSED )
        state += PRESSED_OFFSET;
    else if ( flags & wxCONTROL_CURRENT )
        state += HOT_OFFSET;
    // wxCONTROL_ISDEFAULT flag is only valid for push buttons
    else if ( part == BP_PUSHBUTTON && (flags & wxCONTROL_ISDEFAULT) )
        state = PBS_DEFAULTED;

    ::DrawThemeBackground
                            (
                                htheme,
                                GetHdcOf(dc.GetTempHDC()),
                                part,
                                state,
                                &r,
                                nullptr
                            );
}

void
wxRendererXP::DrawTitleBarBitmap(wxWindow *win,
                                 wxDC& dc,
                                 const wxRect& rect,
                                 wxTitleBarButton button,
                                 unsigned int flags)
{
    wxUxThemeHandle hTheme(win, L"WINDOW");
    if ( !hTheme )
    {
        m_rendererNative.DrawTitleBarBitmap(win, dc, rect, button, flags);
        return;
    }

    int part;
    switch ( button )
    {
        case wxTitleBarButton::Close:
            part = WP_CLOSEBUTTON;
            break;

        case wxTitleBarButton::Maximize:
            part = WP_MAXBUTTON;
            break;

        case wxTitleBarButton::Iconize:
            part = WP_MINBUTTON;
            break;

        case wxTitleBarButton::Restore:
            part = WP_RESTOREBUTTON;
            break;

        case wxTitleBarButton::Help:
            part = WP_HELPBUTTON;
            break;

        default:
            wxFAIL_MSG( "unsupported title bar button" );
            return;
    }

    DoDrawButtonLike(hTheme, part, dc, rect, flags);
}

wxSize wxRendererXP::GetCheckBoxSize(wxWindow* win, unsigned int flags)
{
    wxCHECK_MSG( win, wxSize(0, 0), "Must have a valid window" );

    wxUxThemeHandle hTheme(win, L"BUTTON");
    if (hTheme)
    {
        if (::IsThemePartDefined(hTheme, BP_CHECKBOX, 0))
        {
            SIZE checkSize;
            if (::GetThemePartSize(hTheme, nullptr, BP_CHECKBOX, CBS_UNCHECKEDNORMAL, nullptr, TS_DRAW, &checkSize) == S_OK)
                return {checkSize.cx, checkSize.cy};
        }
    }
    return m_rendererNative.GetCheckBoxSize(win, flags);
}

wxSize wxRendererXP::GetCheckMarkSize(wxWindow* win)
{
    wxCHECK_MSG(win, wxSize(0, 0), "Must have a valid window");

    wxUxThemeHandle hTheme(win, L"MENU");
    if (hTheme)
    {
        if (::IsThemePartDefined(hTheme, MENU_POPUPCHECK, 0))
        {
            SIZE checkSize;
            if (::GetThemePartSize(hTheme, nullptr, MENU_POPUPCHECK, MC_CHECKMARKNORMAL, nullptr, TS_DRAW, &checkSize) == S_OK)
                return {checkSize.cx, checkSize.cy};
        }
    }
    return m_rendererNative.GetCheckMarkSize(win);
}

wxSize wxRendererXP::GetExpanderSize(wxWindow* win)
{
    wxCHECK_MSG( win, wxSize(0, 0), "Must have a valid window" );

    wxUxThemeHandle hTheme(win, L"TREEVIEW");
    if ( hTheme )
    {
        if ( ::IsThemePartDefined(hTheme, TVP_GLYPH, 0) )
        {
            SIZE expSize;
            if (::GetThemePartSize(hTheme, nullptr, TVP_GLYPH, GLPS_CLOSED, nullptr,
                                   TS_DRAW, &expSize) == S_OK)
                return {expSize.cx, expSize.cy};

        }
    }

    return m_rendererNative.GetExpanderSize(win);
}

void
wxRendererXP::DrawCollapseButton(wxWindow *win,
                                 wxDC& dc,
                                 const wxRect& rect,
                                 unsigned int flags)
{
    wxUxThemeHandle hTheme(win, L"TASKDIALOG");

    if ( ::IsThemePartDefined(hTheme, TDLG_EXPANDOBUTTON, 0) )
    {
        int state;
        if (flags & wxCONTROL_PRESSED)
            state = TDLGEBS_PRESSED;
        else if (flags & wxCONTROL_CURRENT)
            state = TDLGEBS_HOVER;
        else
            state = TDLGEBS_NORMAL;

        if ( flags & wxCONTROL_EXPANDED )
            state += 3;

        RECT r = ConvertToRECT(dc, rect);

        ::DrawThemeBackground
            (
            hTheme,
            GetHdcOf(dc.GetTempHDC()),
            TDLG_EXPANDOBUTTON,
            state,
            &r,
            nullptr
            );
    }
    else
        m_rendererNative.DrawCollapseButton(win, dc, rect, flags);
}

wxSize wxRendererXP::GetCollapseButtonSize(wxWindow *win, wxDC& dc)
{
    wxUxThemeHandle hTheme(win, L"TASKDIALOG");

    // EXPANDOBUTTON scales ugly if not using the correct size, get size from theme

    if ( ::IsThemePartDefined(hTheme, TDLG_EXPANDOBUTTON, 0) )
    {
        SIZE s;
        ::GetThemePartSize(hTheme,
            GetHdcOf(dc.GetTempHDC()),
            TDLG_EXPANDOBUTTON,
            TDLGEBS_NORMAL,
            nullptr,
            TS_TRUE,
            &s);

        return {s.cx, s.cy};
    }
    else
        return m_rendererNative.GetCollapseButtonSize(win, dc);
}

void
wxRendererXP::DrawItemSelectionRect(wxWindow *win,
                                    wxDC& dc,
                                    const wxRect& rect,
                                    unsigned int flags)
{
    wxUxThemeHandle hTheme(win, L"LISTVIEW");

    const int itemState = GetListItemState(flags);

    if ( ::IsThemePartDefined(hTheme, LVP_LISTITEM, 0) )
    {
        RECT rc = ConvertToRECT(dc, rect);

        if ( ::IsThemeBackgroundPartiallyTransparent(hTheme, LVP_LISTITEM, itemState) )
            ::DrawThemeParentBackground(GetHwndOf(win), GetHdcOf(dc.GetTempHDC()), &rc);

        ::DrawThemeBackground(hTheme, GetHdcOf(dc.GetTempHDC()), LVP_LISTITEM, itemState, &rc, nullptr);
    }
    else
    {
        m_rendererNative.DrawItemSelectionRect(win, dc, rect, flags);
    }
}

void wxRendererXP::DrawItemText(wxWindow* win,
                                wxDC& dc,
                                const std::string& text,
                                const wxRect& rect,
                                unsigned int align,
                                unsigned int flags,
                                wxEllipsizeMode ellipsizeMode)
{
    wxUxThemeHandle hTheme(win, L"LISTVIEW");

    const int itemState = GetListItemState(flags);

    using DrawThemeTextEx_t = HRESULT(__stdcall*)(HTHEME, WXHDC, int, int, const wchar_t *, int, WXDWORD, RECT *, const WXDTTOPTS *);
    static DrawThemeTextEx_t s_DrawThemeTextEx = nullptr;
    static bool s_initDone = false;

    if ( !s_initDone )
    {
        if (wxGetWinVersion() >= wxWinVersion_Vista)
        {
            wxLoadedDLL dllUxTheme("uxtheme.dll");
            wxDL_INIT_FUNC(s_, DrawThemeTextEx, dllUxTheme);
        }

        s_initDone = true;
    }

    if ( s_DrawThemeTextEx && // Might be not available if we're under XP
            ::IsThemePartDefined(hTheme, LVP_LISTITEM, 0) )
    {
        RECT rc = ConvertToRECT(dc, rect);

        WXDTTOPTS textOpts;
        textOpts.dwSize = sizeof(textOpts);
        textOpts.dwFlags = DTT_STATEID;
        textOpts.iStateId = itemState;

        wxColour textColour = dc.GetTextForeground();
        if (flags & wxCONTROL_SELECTED)
        {
            textColour = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
        }
        else if (flags & wxCONTROL_DISABLED)
        {
            textColour = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
        }

        if (textColour.IsOk()) {
            textOpts.dwFlags |= DTT_TEXTCOLOR;
            textOpts.crText = textColour.GetPixel();
        }

        WXDWORD textFlags = DT_NOPREFIX;
        if ( align & wxALIGN_CENTER_HORIZONTAL )
            textFlags |= DT_CENTER;
        else if ( align & wxALIGN_RIGHT )
        {
            textFlags |= DT_RIGHT;
            rc.right--; // Alignment is inconsistent with DrawLabel otherwise
        }
        else
            textFlags |= DT_LEFT;

        if ( align & wxALIGN_BOTTOM )
            textFlags |= DT_BOTTOM;
        else if ( align & wxALIGN_CENTER_VERTICAL )
            textFlags |= DT_VCENTER;
        else
            textFlags |= DT_TOP;

        const std::string* drawText = &text;
        std::string ellipsizedText;
        switch ( ellipsizeMode )
        {
            case wxEllipsizeMode::None:
                // no flag required
                break;

            case wxEllipsizeMode::Start:
            case wxEllipsizeMode::Middle:
                // no native support for this ellipsize modes, use wxWidgets
                // implementation (may not be 100% accurate because per
                // definition the theme defines the font but should be close
                // enough with current windows themes)
                drawText = &ellipsizedText;
                ellipsizedText = wxControl::Ellipsize(text, dc, ellipsizeMode,
                                                      rect.width,
                                                      wxEllipsizeFlags::None);
                break;

            case wxEllipsizeMode::End:
                textFlags |= DT_END_ELLIPSIS;
                break;
        }

        s_DrawThemeTextEx(hTheme, dc.GetHDC(), LVP_LISTITEM, itemState,
                            boost::nowide::widen(*drawText).c_str(), -1, textFlags, &rc, &textOpts);
    }
    else
    {
        m_rendererNative.DrawItemText(win, dc, text, rect, align, flags, ellipsizeMode);
    }
}

// Uses the theme to draw the border and fill for something like a wxTextCtrl
void wxRendererXP::DrawTextCtrl(wxWindow* win,
                                wxDC& dc,
                                const wxRect& rect,
                                unsigned int flags)
{
    wxUxThemeHandle hTheme(win, L"EDIT");
    if ( !hTheme )
    {
        m_rendererNative.DrawTextCtrl(win,dc,rect,flags);
        return;
    }

    wxColour fill;
    wxColour bdr;
    COLORREF cref;

    ::GetThemeColor(hTheme, EP_EDITTEXT,
                                          ETS_NORMAL, TMT_FILLCOLOR, &cref);
    fill = wxRGBToColour(cref);

    int etsState;
    if ( flags & wxCONTROL_DISABLED )
        etsState = ETS_DISABLED;
    else
        etsState = ETS_NORMAL;

    ::GetThemeColor(hTheme, EP_EDITTEXT,
                                              etsState, TMT_BORDERCOLOR, &cref);
    bdr = wxRGBToColour(cref);

    dc.SetPen( bdr );
    dc.SetBrush( fill );
    dc.DrawRectangle(rect);
}

void wxRendererXP::DrawGauge(wxWindow* win,
    wxDC& dc,
    const wxRect& rect,
    int value,
    int max,
    unsigned int flags)
{
    wxUxThemeHandle hTheme(win, L"PROGRESS");
    if ( !hTheme )
    {
        m_rendererNative.DrawGauge(win, dc, rect, value, max, flags);
        return;
    }

    RECT r = ConvertToRECT(dc, rect);

    ::DrawThemeBackground(
        hTheme,
        GetHdcOf(dc.GetTempHDC()),
        flags & wxCONTROL_SPECIAL ? PP_BARVERT : PP_BAR,
        0,
        &r,
        nullptr);

    RECT contentRect;
    ::GetThemeBackgroundContentRect(
        hTheme,
        GetHdcOf(dc.GetTempHDC()),
        flags & wxCONTROL_SPECIAL ? PP_BARVERT : PP_BAR,
        0,
        &r,
        &contentRect);

    if ( flags & wxCONTROL_SPECIAL )
    {
        // For a vertical gauge, the value grows from the bottom to the top.
        contentRect.top = contentRect.bottom -
                          wxMulDivInt32(contentRect.bottom - contentRect.top,
                                        value,
                                        max);
    }
    else // Horizontal.
    {
        contentRect.right = contentRect.left +
                            wxMulDivInt32(contentRect.right - contentRect.left,
                                          value,
                                          max);
    }

    ::DrawThemeBackground(
        hTheme,
        GetHdcOf(dc.GetTempHDC()),
        flags & wxCONTROL_SPECIAL ? PP_CHUNKVERT : PP_CHUNK,
        0,
        &contentRect,
        nullptr);
}

// ----------------------------------------------------------------------------
// splitter drawing
// ----------------------------------------------------------------------------

// the width of the sash: this is the same as used by Explorer...
constexpr wxCoord SASH_WIDTH = 4;

wxSplitterRenderParams
wxRendererXP::GetSplitterParams(const wxWindow * win)
{
    if ( win->HasFlag(wxSP_NO_XP_THEME) )
        return m_rendererNative.GetSplitterParams(win);
    else
        return {SASH_WIDTH, 0, false};
}

void
wxRendererXP::DrawSplitterBorder(wxWindow * win,
                                 wxDC& dc,
                                 const wxRect& rect,
                                 unsigned int flags)
{
    if ( win->HasFlag(wxSP_NO_XP_THEME) )
    {
        m_rendererNative.DrawSplitterBorder(win, dc, rect, flags);
    }
}

void
wxRendererXP::DrawSplitterSash(wxWindow *win,
                               wxDC& dc,
                               const wxSize& size,
                               wxCoord position,
                               wxOrientation orient,
                               unsigned int flags)
{
    if ( !win->HasFlag(wxSP_NO_XP_THEME) )
    {
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
        if ( orient == wxVERTICAL )
        {
            dc.DrawRectangle(position, 0, SASH_WIDTH, size.y);
        }
        else // wxHORIZONTAL
        {
            dc.DrawRectangle(0, position, size.x, SASH_WIDTH);
        }

        return;
    }

    m_rendererNative.DrawSplitterSash(win, dc, size, position, orient, flags);
}

#endif // wxUSE_UXTHEME
