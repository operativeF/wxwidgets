/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/statbox.cpp
// Purpose:     wxStaticBox
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_STATBOX

#include <windowsx.h>

#include "wx/statbox.h"

#include "wx/msw/private.h"

#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/sizer.h"

#include "wx/notebook.h"
#include "wx/sysopt.h"

#include "wx/msw/uxtheme.h"

#include "wx/msw/dc.h"
#include "wx/msw/private/winstyle.h"

#include <boost/nowide/convert.hpp>

import WX.WinDef;
import WX.Win.UniqueHnd;

namespace
{

// Offset of the first pixel of the label from the box left border.
//
// FIXME: value is hardcoded as this is what it is on my system, no idea if
//        it's true everywhere
const int LABEL_HORZ_OFFSET = 9;

// Extra borders around the label on left/right and bottom sides.
const int LABEL_HORZ_BORDER = 2;
const int LABEL_VERT_BORDER = 2;

// Offset of the box contents from left/right/bottom edge (top one is
// different, see GetBordersForSizer()). This one is completely arbitrary.
const int CHILDREN_OFFSET = 5;

} // anonymous namespace

// ----------------------------------------------------------------------------
// wxWin macros
// ----------------------------------------------------------------------------

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxStaticBox
// ----------------------------------------------------------------------------

bool wxStaticBox::Create(wxWindow *parent,
                         wxWindowID id,
                         const std::string& label,
                         const wxPoint& pos,
                         const wxSize& size,
                         unsigned int style,
                         std::string_view name)
{
    if ( !CreateControl(parent, id, pos, size, style, wxValidator{}, name) )
        return false;

    if ( !MSWCreateControl("BUTTON", label, pos, size) )
        return false;

    if (!wxSystemOptions::IsFalse("msw.staticbox.optimized-paint"))
    {
        Bind(wxEVT_PAINT, &wxStaticBox::OnPaint, this);

        // Our OnPaint() completely erases our background, so don't do it in
        // WM_ERASEBKGND too to avoid flicker.
        SetBackgroundStyle(wxBackgroundStyle::Paint);
    }

    return true;
}

bool wxStaticBox::Create(wxWindow* parent,
                         wxWindowID id,
                         wxWindow* labelWin,
                         const wxPoint& pos,
                         const wxSize& size,
                         unsigned int style,
                         std::string_view name)
{
    wxCHECK_MSG( labelWin, false, "Label window can't be null" );

    if ( !Create(parent, id, "", pos, size, style, name) )
        return false;

    m_labelWin = labelWin;
    m_labelWin->Reparent(this);

    PositionLabelWindow();

    return true;
}

void wxStaticBox::PositionLabelWindow()
{
    m_labelWin->SetSize(m_labelWin->GetBestSize());
    m_labelWin->Move(wxPoint{FromDIP(LABEL_HORZ_OFFSET), 0});
}

wxWindowList wxStaticBox::GetCompositeWindowParts() const
{
    wxWindowList parts;
    if ( m_labelWin )
        parts.push_back(m_labelWin);
    return parts;
}

WXDWORD wxStaticBox::MSWGetStyle(unsigned int style, WXDWORD *exstyle) const
{
    WXDWORD styleWin = wxStaticBoxBase::MSWGetStyle(style, exstyle);

    // no need for it anymore, must be removed for wxRadioBox child
    // buttons to be able to repaint themselves
    styleWin &= ~WS_CLIPCHILDREN;

    if ( exstyle )
    {
        // We may have children inside this static box, so use this style for
        // TAB navigation to work if we ever use IsDialogMessage() to implement
        // it (currently we don't because it's too buggy and implement TAB
        // navigation ourselves, but this could change in the future).
        *exstyle |= WS_EX_CONTROLPARENT;

        if (wxSystemOptions::IsFalse("msw.staticbox.optimized-paint"))
            *exstyle |= WS_EX_TRANSPARENT;
    }

    styleWin |= BS_GROUPBOX;

    return styleWin;
}

wxSize wxStaticBox::DoGetBestSize() const
{
    wxSize best;

    // Calculate the size needed by the label
    wxSize char_size = wxGetCharSize(GetHWND(), GetFont());

    int wBox = GetTextExtent(GetLabelText(wxGetWindowText(m_hWnd))).x;

    wBox += 3 * char_size.x;
    int hBox = EDIT_HEIGHT_FROM_CHAR_HEIGHT(char_size.y);

    // If there is a sizer then the base best size is the sizer's minimum
    if (GetSizer() != nullptr)
    {
        wxSize cm(GetSizer()->CalcMin());
        best = ClientToWindowSize(cm);
        // adjust for a long label if needed
        best.x = std::max(best.x, wBox);
    }
    // otherwise the best size falls back to the label size
    else
    {
        best = wxSize(wBox, hBox);
    }
    return best;
}

void wxStaticBox::GetBordersForSizer(int *borderTop, int *borderOther) const
{
    // Base class version doesn't leave enough space at the top when the label
    // is empty, so we can't use it here, even though the code is pretty
    // similar.
    if ( m_labelWin )
    {
        *borderTop = m_labelWin->GetSize().y;
    }
    else if ( !GetLabel().empty() )
    {
        *borderTop = GetCharHeight();
    }
    else // No label window nor text.
    {
        // This is completely arbitrary, but using the full char height in
        // this case too seems bad as it leaves too much space at the top
        // (although it does have the advantage of aligning the controls
        // inside static boxes with and without labels vertically).
        *borderTop = 2*FromDIP(CHILDREN_OFFSET);
    }

    *borderTop += FromDIP(LABEL_VERT_BORDER);

    *borderOther = FromDIP(CHILDREN_OFFSET);
}

bool wxStaticBox::SetBackgroundColour(const wxColour& colour)
{
    // Do _not_ call the immediate base class method, we don't need to set the
    // label window (which is the only sub-window of this composite window)
    // background explicitly because it will almost always be a wxCheckBox or
    // wxRadioButton which inherits its background from the box anyhow, so
    // setting it would be at best useless.
    return wxStaticBoxBase::SetBackgroundColour(colour);
}

bool wxStaticBox::SetFont(const wxFont& font)
{
    if ( !wxCompositeWindowSettersOnly<wxStaticBoxBase>::SetFont(font) )
        return false;

    // We need to reposition the label as its size may depend on the font.
    if ( m_labelWin )
    {
        PositionLabelWindow();
    }

    return true;
}

WXLRESULT wxStaticBox::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    if ( nMsg == WM_NCHITTEST )
    {
        // This code breaks some other processing such as enter/leave tracking
        // so it's off by default.

        static int s_useHTClient = -1;
        if (s_useHTClient == -1)
            s_useHTClient = wxSystemOptions::GetOptionInt("msw.staticbox.htclient");
        if (s_useHTClient == 1)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            ScreenToClient(&xPos, &yPos);

            // Make sure you can drag by the top of the groupbox, but let
            // other (enclosed) controls get mouse events also
            if ( yPos < 10 )
                return (long)HTCLIENT;
        }
    }

    if ( nMsg == WM_PRINTCLIENT )
    {
        // we have to process WM_PRINTCLIENT ourselves as otherwise child
        // windows' background (eg buttons in radio box) would never be drawn
        // unless we have a parent with non default background

        // so check first if we have one
        if ( !HandlePrintClient((WXHDC)wParam) )
        {
            // no, we don't, erase the background ourselves
            RECT rc;
            ::GetClientRect(GetHwnd(), &rc);
            wxDCTemp dc((WXHDC)wParam);
            PaintBackground(dc, rc);
        }

        return 0;
    }

    if ( nMsg == WM_UPDATEUISTATE )
    {
        // DefWindowProc() redraws just the static box text when it gets this
        // message and it does it using the standard (blue in standard theme)
        // colour and not our own label colour that we use in PaintForeground()
        // resulting in the label mysteriously changing the colour when e.g.
        // "Alt" is pressed anywhere in the window, see #12497.
        //
        // To avoid this we simply refresh the window forcing our own code
        // redrawing the label in the correct colour to be called. This is
        // inefficient but there doesn't seem to be anything else we can do.
        //
        // Notice that the problem is XP-specific and doesn't arise under later
        // systems.
        if ( m_hasFgCol && wxGetWinVersion() == wxWinVersion_XP )
            Refresh();
    }

    return wxControl::MSWWindowProc(nMsg, wParam, lParam);
}

// ----------------------------------------------------------------------------
// static box drawing
// ----------------------------------------------------------------------------

using msw::utils::unique_region;

/*
   We draw the static box ourselves because it's the only way to prevent it
   from flickering horribly on resize (because everything inside the box is
   erased twice: once when the box itself is repainted and second time when
   the control inside it is repainted) without using WS_EX_TRANSPARENT style as
   we used to do and which resulted in other problems.
 */

// MSWGetRegionWithoutSelf helper: removes the given rectangle from region
static inline void
SubtractRectFromRgn(WXHRGN hrgn, int left, int top, int right, int bottom)
{    
    auto hrgnRect = unique_region(::CreateRectRgn(left, top, right, bottom));

    if ( !hrgnRect )
    {
        wxLogLastError("CreateRectRgn()");
        return;
    }

    ::CombineRgn(hrgn, hrgn, hrgnRect.get(), RGN_DIFF);
}

void wxStaticBox::MSWGetRegionWithoutSelf(WXHRGN hRgn, int w, int h)
{
    WXHRGN hrgn = (WXHRGN)hRgn;

    // remove the area occupied by the static box borders from the region
    int borderTop, border;
    GetBordersForSizer(&borderTop, &border);

    // top
    if ( m_labelWin )
    {
        // Don't exclude the entire rectangle at the top, we do need to paint
        // the background of the gap between the label window and the box
        // frame.
        const wxRect labelRect = m_labelWin->GetRect();
        const int gap = FromDIP(LABEL_HORZ_BORDER);

        SubtractRectFromRgn(hrgn, 0, 0, labelRect.GetLeft() - gap, borderTop);
        SubtractRectFromRgn(hrgn, labelRect.GetRight() + gap, 0, w, borderTop);
    }
    else
    {
        SubtractRectFromRgn(hrgn, 0, 0, w, borderTop);
    }

    // bottom
    SubtractRectFromRgn(hrgn, 0, h - border, w, h);

    // left
    SubtractRectFromRgn(hrgn, 0, 0, border, h);

    // right
    SubtractRectFromRgn(hrgn, w - border, 0, w, h);
}

namespace {
RECT AdjustRectForRtl(wxLayoutDirection dir, RECT const& childRect, RECT const& boxRect) {
    RECT ret = childRect;
    if( dir == wxLayoutDirection::RightToLeft ) {
        // The clipping region too is mirrored in RTL layout.
        // We need to mirror screen coordinates relative to static box window priot to
        // intersecting with region.
        ret.right = boxRect.right - childRect.left - boxRect.left;
        ret.left = boxRect.right - childRect.right - boxRect.left;
    }

    return ret;
}
}

WXHRGN wxStaticBox::MSWGetRegionWithoutChildren()
{
    RECT boxRc;
    ::GetWindowRect(GetHwnd(), &boxRc);
    WXHRGN hrgn = ::CreateRectRgn(boxRc.left, boxRc.top, boxRc.right + 1, boxRc.bottom + 1);
    bool foundThis = false;

    // Iterate over all sibling windows as in the old wxWidgets API the
    // controls appearing inside the static box were created as its siblings
    // and not children. This is now deprecated but should still work.
    //
    // Also notice that we must iterate over all windows, not just all
    // wxWindows, as there may be composite windows etc.
    WXHWND child;
    for ( child = ::GetWindow(GetHwndOf(GetParent()), GW_CHILD);
          child;
          child = ::GetWindow(child, GW_HWNDNEXT) )
    {
        if ( ! ::IsWindowVisible(child) )
        {
            // if the window isn't visible then it doesn't need clipped
            continue;
        }

        wxMSWWinStyleUpdater updateStyle(child);
        wxString str(wxGetWindowClass(child));
        str.UpperCase();
        if ( str == "BUTTON" && updateStyle.IsOn(BS_GROUPBOX) )
        {
            if ( child == GetHwnd() )
                foundThis = true;

            // Any static boxes below this one in the Z-order can't be clipped
            // since if we have the case where a static box with a low Z-order
            // is nested inside another static box with a high Z-order then the
            // nested static box would be painted over. Doing it this way
            // unfortunately results in flicker if the Z-order of nested static
            // boxes is not inside (lowest) to outside (highest) but at least
            // they are still shown.
            if ( foundThis )
                continue;
        }

        RECT rc;
        ::GetWindowRect(child, &rc);
        rc = AdjustRectForRtl(GetLayoutDirection(), rc, boxRc );
        if ( ::RectInRegion(hrgn, &rc) )
        {
            // need to remove WS_CLIPSIBLINGS from all sibling windows
            // that are within this staticbox if set
            if ( updateStyle.IsOn(WS_CLIPSIBLINGS) )
            {
                updateStyle.TurnOff(WS_CLIPSIBLINGS).Apply();

                // MSDN: "If you have changed certain window data using
                // SetWindowLong, you must call SetWindowPos to have the
                // changes take effect."
                ::SetWindowPos(child, nullptr, 0, 0, 0, 0,
                               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                               SWP_FRAMECHANGED);
            }

            auto hrgnChild = unique_region(::CreateRectRgnIndirect(&rc));
            ::CombineRgn(hrgn, hrgn, hrgnChild.get(), RGN_DIFF);
        }
    }

    // Also iterate over all children of the static box, we need to clip them
    // out as well.
    for ( child = ::GetWindow(GetHwnd(), GW_CHILD);
          child;
          child = ::GetWindow(child, GW_HWNDNEXT) )
    {
        if ( !::IsWindowVisible(child) )
        {
            // if the window isn't visible then it doesn't need clipped
            continue;
        }

        RECT rc;
        ::GetWindowRect(child, &rc);
        rc = AdjustRectForRtl(GetLayoutDirection(), rc, boxRc );
        auto hrgnChild = unique_region(::CreateRectRgnIndirect(&rc));
        ::CombineRgn(hrgn, hrgn, hrgnChild.get(), RGN_DIFF);
    }

    return (WXHRGN)hrgn;
}

// helper for OnPaint(): really erase the background, i.e. do it even if we
// don't have any non default brush for doing it (DoEraseBackground() doesn't
// do anything in such case)
void wxStaticBox::PaintBackground(wxDC& dc, const RECT& rc)
{
    wxMSWDCImpl *impl = (wxMSWDCImpl*) dc.GetImpl();
    WXHBRUSH hbr = MSWGetBgBrush(impl->GetHDC());

    // if there is no special brush for painting this control, just use the
    // solid background colour
    wxBrush brush;
    if ( !hbr )
    {
        brush = wxBrush(GetParent()->GetBackgroundColour());
        hbr = GetHbrushOf(brush);
    }

    ::FillRect(GetHdcOf(*impl), &rc, hbr);
}

void wxStaticBox::PaintForeground(wxDC& dc, const RECT&)
{
    wxMSWDCImpl *impl = (wxMSWDCImpl*) dc.GetImpl();
    MSWDefWindowProc(WM_PAINT, (WXWPARAM)GetHdcOf(*impl), 0);

#if wxUSE_UXTHEME
    // when using XP themes, neither setting the text colour nor transparent
    // background mode doesn't change anything: the static box def window proc
    // still draws the label in its own colours, so we need to redraw the text
    // ourselves if we have a non default fg colour
    if ( m_hasFgCol && wxUxThemeIsActive() && !m_labelWin )
    {
        // draw over the text in default colour in our colour
        WXHDC hdc = GetHdcOf(*impl);
        ::SetTextColor(hdc, GetForegroundColour().GetPixel());

        // Get dimensions of the label
        const std::string label = GetLabel();

        // choose the correct font

        using msw::utils::unique_font;

        unique_font font;
        SelectInHDC selFont;
        if ( m_hasFont )
        {
            selFont.Init(hdc, GetHfontOf(GetFont()));
        }
        else // no font set, use the one set by the theme
        {
            wxUxThemeHandle hTheme(this, "BUTTON");
            if ( hTheme )
            {
                wxUxThemeFont themeFont;
                if ( ::GetThemeFont
                                    (
                                    hTheme,
                                    hdc,
                                    BP_GROUPBOX,
                                    GBS_NORMAL,
                                    TMT_FONT,
                                    themeFont.GetPtr()
                                    ) == S_OK )
                {
                    font = unique_font(::CreateFontIndirectW(&themeFont.GetLOGFONT()));
                    if ( font )
                        selFont.Init(hdc, font.get());
                }
            }
        }

        // Get the font extent
        auto textExtent = dc.GetTextExtent(wxStripMenuCodes(label, wxStrip_Mnemonics).ToStdString());

        // first we need to correctly paint the background of the label
        // as Windows ignores the brush offset when doing it
        // NOTE: Border intentionally does not use DIPs in order to match native look
        const int x = LABEL_HORZ_OFFSET;
        RECT dimensions = { x, 0, 0, textExtent.y };
        dimensions.left = x;
        dimensions.right = x + textExtent.x;

        // need to adjust the rectangle to cover all the label background
        dimensions.left -= LABEL_HORZ_BORDER;
        dimensions.right += LABEL_HORZ_BORDER;
        dimensions.bottom += LABEL_VERT_BORDER;

        if ( UseBgCol() )
        {
            // our own background colour should be used for the background of
            // the label: this is consistent with the behaviour under pre-XP
            // systems (i.e. without visual themes) and generally makes sense
            wxBrush brush = wxBrush(GetBackgroundColour());
            ::FillRect(hdc, &dimensions, GetHbrushOf(brush));
        }
        else // paint parent background
        {
            PaintBackground(dc, dimensions);
        }

        WXUINT drawTextFlags = DT_SINGLELINE | DT_VCENTER;

        // determine the state of UI queues to draw the text correctly under XP
        // and later systems
        static const bool isXPorLater = wxGetWinVersion() >= wxWinVersion_XP;
        if ( isXPorLater )
        {
            if ( ::SendMessageW(GetHwnd(), WM_QUERYUISTATE, 0, 0) &
                    UISF_HIDEACCEL )
            {
                drawTextFlags |= DT_HIDEPREFIX;
            }
        }

        // now draw the text
        RECT rc2 = { x, 0, x + textExtent.x, textExtent.y };
        ::DrawTextW(hdc, boost::nowide::widen(label).c_str(), label.length(), &rc2,
                   drawTextFlags);
    }
#endif // wxUSE_UXTHEME
}

void wxStaticBox::OnPaint([[maybe_unused]] wxPaintEvent& event)
{
    RECT rc;
    ::GetClientRect(GetHwnd(), &rc);
    wxPaintDC dc(this);

    // No need to do anything if the client rectangle is empty and, worse,
    // doing it would result in an assert when creating the bitmap below.
    if ( !rc.right || !rc.bottom )
        return;

    // draw the entire box in a memory DC
    wxMemoryDC memdc(&dc);
    wxBitmap bitmap(wxSize{rc.right, rc.bottom});
    memdc.SelectObject(bitmap);

    PaintBackground(memdc, rc);
    PaintForeground(memdc, rc);

    // now only blit the static box border itself, not the interior, to avoid
    // flicker when background is drawn below
    //
    // note that it seems to be faster to do 4 small blits here and then paint
    // directly into wxPaintDC than painting background in wxMemoryDC and then
    // blitting everything at once to wxPaintDC, this is why we do it like this
    int borderTop, border;
    GetBordersForSizer(&borderTop, &border);

    // top
    if ( m_labelWin )
    {
        // We also have to exclude the area taken by the label window,
        // otherwise there would be flicker when it draws itself on top of it.
        const wxRect labelRect = m_labelWin->GetRect();

        // We also leave a small border around label window to make it appear
        // more similarly to a plain text label.
        const int gap = FromDIP(LABEL_HORZ_BORDER);

        dc.Blit(wxPoint{border, 0},
                wxSize{labelRect.GetLeft() - gap - border, borderTop},
                &memdc,
                wxPoint{border, 0});
        dc.Blit(wxPoint{labelRect.GetRight() + gap, 0},
                wxSize{rc.right - (labelRect.GetRight() + gap), borderTop},
                &memdc,
                wxPoint{border, 0});
    }
    else
    {
        dc.Blit(wxPoint{border, 0},
                wxSize{rc.right - border, borderTop},
                &memdc,
                wxPoint{border, 0});
    }

    // bottom
    dc.Blit(wxPoint{border, rc.bottom - border},
            wxSize{rc.right - border, border},
            &memdc,
            wxPoint{border, rc.bottom - border});
    // left
    dc.Blit(wxPoint{0, 0},
            wxSize{border, rc.bottom},
            &memdc,
            wxPoint{0, 0});
    // right (note that upper and bottom right corners were already part of the
    // first two blits so we shouldn't overwrite them here to avoid flicker)
    dc.Blit(wxPoint{rc.right - border, borderTop},
            wxSize{border, rc.bottom - borderTop - border},
            &memdc,
            wxPoint{rc.right - border, borderTop});


    // create the region excluding box children
    auto hrgn = unique_region((WXHRGN)MSWGetRegionWithoutChildren());
    RECT rcWin;
    ::GetWindowRect(GetHwnd(), &rcWin);
    ::OffsetRgn(hrgn.get(), -rcWin.left, -rcWin.top);

    // and also the box itself
    MSWGetRegionWithoutSelf((WXHRGN) hrgn.get(), rc.right, rc.bottom);
    wxMSWDCImpl *impl = (wxMSWDCImpl*) dc.GetImpl();
    HDCClipper clipToBg(GetHdcOf(*impl), hrgn.get());

    // paint the inside of the box (excluding box itself and child controls)
    PaintBackground(dc, rc);
}

#endif // wxUSE_STATBOX
