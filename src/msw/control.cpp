/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/control.cpp
// Purpose:     wxControl class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_CONTROLS

#include "wx/control.h"
#include "wx/event.h"
#include "wx/log.h"
#include "wx/ctrlsub.h"
#include "wx/utils.h"

#include "wx/renderer.h"
#include "wx/msw/uxtheme.h"
#include "wx/msw/dc.h"          // for wxDCTemp
#include "wx/msw/ownerdrawnbutton.h"
#include "wx/msw/private/winstyle.h"

#include "wx/msw/wrapcctl.h" // include <commctrl.h> "properly"
#include "wx/msw/private.h"

#include <boost/nowide/convert.hpp>

import WX.Utils.Settings;

import WX.Cfg.Flags;
import WX.WinDef;

import <algorithm>;
import <ranges>;
import <string>;
import <string_view>;
import <vector>;

// ============================================================================
// wxControl implementation
// ============================================================================

// ----------------------------------------------------------------------------
// control window creation
// ----------------------------------------------------------------------------

bool wxControl::Create(wxWindow *parent,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       unsigned int style,
                       const wxValidator& wxVALIDATOR_PARAM(validator),
                       std::string_view name)
{
    if ( !wxWindow::Create(parent, id, pos, size, style, name) )
        return false;

#if wxUSE_VALIDATORS
    SetValidator(validator);
#endif

    return true;
}

bool wxControl::MSWCreateControl(const std::string& classname,
                                 const std::string& label,
                                 const wxPoint& pos,
                                 const wxSize& size)
{
    WXDWORD exstyle;
    WXDWORD msStyle = MSWGetStyle(wxGetWindowStyle(), &exstyle);

    return MSWCreateControl(classname, msStyle, pos, size, label, exstyle);
}

bool wxControl::MSWCreateControl(const std::string& classname,
                                 WXDWORD style,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 const std::string& label,
                                 WXDWORD exstyle)
{
    // if no extended style given, determine it ourselves
    if ( exstyle == (WXDWORD)-1 )
    {
        exstyle = 0;
        (void) MSWGetStyle(wxGetWindowStyle(), &exstyle);
    }

    // all controls should have this style
    style |= WS_CHILD;

    // create the control visible if it's currently shown for wxWidgets
    if ( m_isShown )
    {
        style |= WS_VISIBLE;
    }

    // choose the position for the control: we have a problem with default size
    // here as we can't calculate the best size before the control exists
    // (DoGetBestSize() may need to use m_hWnd), so just choose the minimal
    // possible but non 0 size because 0 window width/height result in problems
    // elsewhere
    int x = pos.x == wxDefaultCoord ? 0 : pos.x,
        y = pos.y == wxDefaultCoord ? 0 : pos.y,
        w = size.x == wxDefaultCoord ? 1 : size.x,
        h = size.y == wxDefaultCoord ? 1 : size.y;

    // ... and adjust it to account for a possible parent frames toolbar
    AdjustForParentClientOrigin(x, y);

    m_hWnd = MSWCreateWindowAtAnyPosition
             (
              exstyle,            // extended style
              classname,          // the kind of control to create
              label,      // the window name
              style,              // the window style
              wxRect{x, y, w, h}, // the window position and size
              GetHwndOf(GetParent()),         // parent
              GetId()             // child id
             );

    if ( !m_hWnd )
    {
        return false;
    }

    // saving the label in m_labelOrig to return it verbatim
    // later in GetLabel()
    m_labelOrig = label;

    // install wxWidgets window proc for this window
    SubclassWin(m_hWnd);

    // set up fonts and colours
    InheritAttributes();
    if ( !m_hasFont )
    {
        if ( MSWShouldSetDefaultFont() )
        {
            SetFont(GetDefaultAttributes().font);
        }
    }

    // set the size now if no initial size specified
    SetInitialSize(size);

    if ( size.IsFullySpecified() )
    {
        // Usually all windows get WM_NCCALCSIZE when their size is set,
        // however if the initial size is fixed, it's not going to change and
        // this message won't be sent at all, meaning that we won't get a
        // chance to tell Windows that we need extra space for our custom
        // themed borders, when using them. So force sending this message by
        // using SWP_FRAMECHANGED (and use SWP_NOXXX to avoid doing anything
        // else) to fix themed borders when they're used (if they're not, this
        // is harmless, and it's simpler and more fool proof to always do it
        // rather than try to determine whether we need to do it or not).
        ::SetWindowPos(GetHwnd(), nullptr, 0, 0, 0, 0,
                       SWP_FRAMECHANGED |
                       SWP_NOSIZE |
                       SWP_NOMOVE |
                       SWP_NOZORDER |
                       SWP_NOREDRAW |
                       SWP_NOACTIVATE |
                       SWP_NOCOPYBITS |
                       SWP_NOOWNERZORDER |
                       SWP_NOSENDCHANGING);
    }

    return true;
}

// ----------------------------------------------------------------------------
// various accessors
// ----------------------------------------------------------------------------

WXDWORD wxControl::MSWGetStyle(unsigned int style, WXDWORD *exstyle) const
{
    long msStyle = wxWindow::MSWGetStyle(style, exstyle);

    if ( AcceptsFocusFromKeyboard() )
    {
        msStyle |= WS_TABSTOP;
    }

    return msStyle;
}

wxSize wxControl::DoGetBestSize() const
{
    if (m_windowSizer)
       return wxControlBase::DoGetBestSize();

    return FromDIP(wxSize(DEFAULT_ITEM_WIDTH, DEFAULT_ITEM_HEIGHT));
}

wxBorder wxControl::GetDefaultBorder() const
{
    return wxControlBase::GetDefaultBorder();
}

/* static */ wxVisualAttributes
wxControl::GetClassDefaultAttributes([[maybe_unused]] wxWindowVariant variant)
{
    wxVisualAttributes attrs;

    // old school (i.e. not "common") controls use the standard dialog font
    // by default
    attrs.font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    // most, or at least many, of the controls use the same colours as the
    // buttons -- others will have to override this (and possibly simply call
    // GetCompositeControlsDefaultAttributes() from their versions)
    attrs.colFg = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    attrs.colBg = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);

    return attrs;
}

// ----------------------------------------------------------------------------
// message handling
// ----------------------------------------------------------------------------

bool wxControl::ProcessCommand(wxCommandEvent& event)
{
    return HandleWindowEvent(event);
}

bool wxControl::MSWOnNotify(int idCtrl,
                            WXLPARAM lParam,
                            WXLPARAM* result)
{
    wxEventType eventType wxDUMMY_INITIALIZE(wxEVT_NULL);

    NMHDR *hdr = (NMHDR*) lParam;
    switch ( hdr->code )
    {
        case NM_CLICK:
            eventType = wxEVT_COMMAND_LEFT_CLICK;
            break;

        case NM_DBLCLK:
            eventType = wxEVT_COMMAND_LEFT_DCLICK;
            break;

        case NM_RCLICK:
            eventType = wxEVT_COMMAND_RIGHT_CLICK;
            break;

        case NM_RDBLCLK:
            eventType = wxEVT_COMMAND_RIGHT_DCLICK;
            break;

        case NM_SETFOCUS:
            eventType = wxEVT_COMMAND_SET_FOCUS;
            break;

        case NM_KILLFOCUS:
            eventType = wxEVT_COMMAND_KILL_FOCUS;
            break;

        case NM_RETURN:
            eventType = wxEVT_COMMAND_ENTER;
            break;

        default:
            return wxWindow::MSWOnNotify(idCtrl, lParam, result);
    }

    wxCommandEvent event(wxEVT_NULL, m_windowId);
    event.SetEventType(eventType);
    event.SetEventObject(this);

    return HandleWindowEvent(event);
}

WXHBRUSH wxControl::DoMSWControlColor(WXHDC pDC, wxColour colBg, WXHWND hWnd)
{
    WXHDC hdc = (WXHDC)pDC;

    WXHBRUSH hbr = nullptr;
    if ( !colBg.IsOk() )
    {
        wxWindow *win = wxFindWinFromHandle( hWnd );
        if ( !win )
        {
            // If this WXHWND doesn't correspond to a wxWindow, it still might be
            // one of its children for which we need to set the background
            // brush, e.g. this is the case for the EDIT control that is part
            // of wxComboBox but also e.g. of wxSlider label HWNDs which are
            // logically part of it, but are siblings of the main control at
            // Windows level.
            //
            // So check whether it's a sibling of this window which is part of
            // the same wx object.
            if ( ContainsHWND(hWnd) )
            {
                win = this;
            }
            else // Or maybe a child sub-window of this one.
            {
                WXHWND parent = ::GetParent(hWnd);
                if ( parent )
                {
                    wxWindow *winParent = wxFindWinFromHandle( parent );
                    if( winParent && winParent->ContainsHWND( hWnd ) )
                        win = winParent;
                 }
            }
        }

        if ( win )
            hbr = win->MSWGetBgBrush(pDC);

        // if the control doesn't have any bg colour, foreground colour will be
        // ignored as the return value would be 0 -- so forcefully give it a
        // non default background brush in this case
        if ( !hbr && m_hasFgCol )
            colBg = GetBackgroundColour();
    }

    // use the background colour override if a valid colour is given: this is
    // used when the control is disabled to grey it out and also if colBg was
    // set just above
    if ( colBg.IsOk() )
    {
        wxBrush *brush = wxTheBrushList->FindOrCreateBrush(colBg);
        hbr = (WXHBRUSH)brush->GetResourceHandle();
    }

    // always set the foreground colour if we changed the background, whether
    // m_hasFgCol is true or not: if it true, we must do it, of course, but
    // even if it isn't, we must set the default foreground explicitly as by
    // default just the simple black is used
    if ( hbr )
    {
        ::SetTextColor(hdc, wxColourToRGB(GetForegroundColour()));
    }

    // finally also set the background colour for text drawing: without this,
    // the text in an edit control is drawn using the default background even
    // if we return a valid brush
    if ( colBg.IsOk() || m_hasBgCol )
    {
        if ( !colBg.IsOk() )
            colBg = GetBackgroundColour();

        ::SetBkColor(hdc, wxColourToRGB(colBg));
    }

    return hbr;
}

WXHBRUSH wxControl::MSWControlColor(WXHDC pDC, WXHWND hWnd)
{
    if ( HasTransparentBackground() )
        ::SetBkMode((WXHDC)pDC, TRANSPARENT);

    // don't pass any background colour to DoMSWControlColor(), our own
    // background colour will be used by it only if it is set, otherwise the
    // defaults will be used
    return DoMSWControlColor(pDC, wxColour(), hWnd);
}

WXHBRUSH wxControl::MSWControlColorDisabled(WXHDC pDC)
{
    return DoMSWControlColor(pDC,
                             wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE),
                             GetHWND());
}

wxWindow* wxControl::MSWFindItem(long id, WXHWND hWnd) const
{
    const auto& sub_controls = GetSubcontrols();

    // is it us or one of our "internal" children?
    if ( id == GetId() || (std::ranges::find(sub_controls, id) != std::end(sub_controls)))
        return const_cast<wxControl *>(this);

    return wxControlBase::MSWFindItem(id, hWnd);
}

// ----------------------------------------------------------------------------
// Owner drawn buttons support.
// ----------------------------------------------------------------------------

void
wxMSWOwnerDrawnButtonBase::MSWMakeOwnerDrawnIfNecessary(const wxColour& colFg)
{
    // The only way to change the checkbox foreground colour when using
    // themes is to owner draw it.
    if ( wxUxThemeIsActive() )
        MSWMakeOwnerDrawn(colFg.IsOk());
}

bool wxMSWOwnerDrawnButtonBase::MSWIsOwnerDrawn() const
{
    return
        (::GetWindowLongPtrW(GetHwndOf(m_win), GWL_STYLE) & BS_OWNERDRAW) == BS_OWNERDRAW;
}

void wxMSWOwnerDrawnButtonBase::MSWMakeOwnerDrawn(bool ownerDrawn)
{
    wxMSWWinStyleUpdater updateStyle(GetHwndOf(m_win));

    // note that BS_CHECKBOX & BS_OWNERDRAW != 0 so we can't operate on
    // them as on independent style bits
    if ( ownerDrawn )
    {
        updateStyle.TurnOff(BS_TYPEMASK).TurnOn(BS_OWNERDRAW);

        m_win->Bind(wxEVT_ENTER_WINDOW,
                    &wxMSWOwnerDrawnButtonBase::OnMouseEnterOrLeave, this);
        m_win->Bind(wxEVT_LEAVE_WINDOW,
                    &wxMSWOwnerDrawnButtonBase::OnMouseEnterOrLeave, this);

        m_win->Bind(wxEVT_LEFT_DOWN,
                    &wxMSWOwnerDrawnButtonBase::OnMouseLeft, this);
        m_win->Bind(wxEVT_LEFT_UP,
                    &wxMSWOwnerDrawnButtonBase::OnMouseLeft, this);

        m_win->Bind(wxEVT_SET_FOCUS,
                    &wxMSWOwnerDrawnButtonBase::OnFocus, this);

        m_win->Bind(wxEVT_KILL_FOCUS,
                    &wxMSWOwnerDrawnButtonBase::OnFocus, this);
    }
    else // reset to default colour
    {
        updateStyle.TurnOff(BS_OWNERDRAW).TurnOn(MSWGetButtonStyle());

        m_win->Unbind(wxEVT_ENTER_WINDOW,
                      &wxMSWOwnerDrawnButtonBase::OnMouseEnterOrLeave, this);
        m_win->Unbind(wxEVT_LEAVE_WINDOW,
                      &wxMSWOwnerDrawnButtonBase::OnMouseEnterOrLeave, this);

        m_win->Unbind(wxEVT_LEFT_DOWN,
                      &wxMSWOwnerDrawnButtonBase::OnMouseLeft, this);
        m_win->Unbind(wxEVT_LEFT_UP,
                      &wxMSWOwnerDrawnButtonBase::OnMouseLeft, this);

        m_win->Unbind(wxEVT_SET_FOCUS,
                      &wxMSWOwnerDrawnButtonBase::OnFocus, this);
        m_win->Unbind(wxEVT_KILL_FOCUS,
                      &wxMSWOwnerDrawnButtonBase::OnFocus, this);
    }

    updateStyle.Apply();

    if ( !ownerDrawn )
        MSWOnButtonResetOwnerDrawn();
}

void wxMSWOwnerDrawnButtonBase::OnMouseEnterOrLeave(wxMouseEvent& event)
{
    if ( event.GetEventType() == wxEVT_LEAVE_WINDOW )
        m_isPressed = false;

    m_win->Refresh();

    event.Skip();
}

void wxMSWOwnerDrawnButtonBase::OnMouseLeft(wxMouseEvent& event)
{
    // TODO: we should capture the mouse here to be notified about left up
    //       event but this interferes with BN_CLICKED generation so if we
    //       want to do this we'd need to generate them ourselves
    m_isPressed = event.GetEventType() == wxEVT_LEFT_DOWN;
    m_win->Refresh();

    event.Skip();
}

void wxMSWOwnerDrawnButtonBase::OnFocus(wxFocusEvent& event)
{
    m_win->Refresh();

    event.Skip();
}

bool wxMSWOwnerDrawnButtonBase::MSWDrawButton(WXDRAWITEMSTRUCT *item)
{
    DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)item;

    if ( !MSWIsOwnerDrawn() || dis->CtlType != ODT_BUTTON )
        return false;

    // shall we draw a focus rect?
    const bool isFocused = m_isPressed || m_win->HasFocus();

    unsigned int flags = MSWGetButtonCheckedFlag();

    if ( dis->itemState & ODS_SELECTED )
        flags |= wxCONTROL_SELECTED | wxCONTROL_PRESSED;

    if ( !m_win->IsEnabled() )
        flags |= wxCONTROL_DISABLED;

    if ( m_isPressed )
        flags |= wxCONTROL_PRESSED;

    if ( wxFindWindowAtPoint(wxGetMousePosition()) == m_win )
        flags |= wxCONTROL_CURRENT;


    // calculate the rectangles for the button itself and the label
    WXHDC hdc = dis->hDC;
    const RECT& rect = dis->rcItem;

    // calculate the rectangles for the button itself and the label
    const wxSize bestSize = m_win->GetBestSize();
    RECT rectButton,
         rectLabel;
    rectLabel.top = rect.top + (rect.bottom - rect.top - bestSize.y) / 2;
    rectLabel.bottom = rectLabel.top + bestSize.y;

    // choose the values consistent with those used for native, non
    // owner-drawn, buttons

    const int spacing = m_win->FromDIP(3);
    const wxSize cbSize = wxRendererNative::Get().GetCheckBoxSize(m_win, flags);

    const int buttonSize = std::min(cbSize.y, m_win->GetSize().y);
    rectButton.top = rect.top + (rect.bottom - rect.top - buttonSize) / 2;
    rectButton.bottom = rectButton.top + buttonSize;

    const bool isRightAligned = m_win->HasFlag(wxALIGN_RIGHT);
    if ( isRightAligned )
    {
        rectButton.right = rect.right;
        rectButton.left = rectButton.right - buttonSize;

        rectLabel.right = rectButton.left - spacing;
        rectLabel.left = rect.left;
    }
    else // normal, left-aligned button
    {
        rectButton.left = rect.left;
        rectButton.right = rectButton.left + buttonSize;

        rectLabel.left = spacing + rectButton.right;
        rectLabel.right = rect.right;
    }

    // Erase the background.
    ::FillRect(hdc, &rect, m_win->MSWGetBgBrush(hdc));

    // draw the button itself
    wxDCTemp dc(hdc);

    MSWDrawButtonBitmap(dc, wxRectFromRECT(rectButton), flags);

    // draw the text
    const std::string& label = m_win->GetLabel();

    // first we need to measure it
    WXUINT fmt = DT_NOCLIP;

    // drawing underlying doesn't look well with focus rect (and the native
    // control doesn't do it)
    if ( isFocused )
        fmt |= DT_HIDEPREFIX;
    if ( isRightAligned )
        fmt |= DT_RIGHT;
    // TODO: also use DT_HIDEPREFIX if the system is configured so

    // we need to get the label real size first if we have to draw a focus rect
    // around it
    if ( isFocused )
    {
        RECT oldLabelRect = rectLabel; // needed if right aligned

        if ( !::DrawTextW(hdc, boost::nowide::widen(label).c_str(), label.length(), &rectLabel,
                         fmt | DT_CALCRECT) )
        {
            wxLogLastError("DrawText(DT_CALCRECT)");
        }

        if ( isRightAligned )
        {
            // move the label rect to the right
            const int labelWidth = rectLabel.right - rectLabel.left;
            rectLabel.right = oldLabelRect.right;
            rectLabel.left = rectLabel.right - labelWidth;
        }
    }

    if ( flags & wxCONTROL_DISABLED )
    {
        ::SetTextColor(hdc, ::GetSysColor(COLOR_GRAYTEXT));
    }

    if ( !::DrawTextW(hdc, boost::nowide::widen(label).c_str(), label.length(), &rectLabel, fmt) )
    {
        wxLogLastError("DrawText()");
    }

    // finally draw the focus
    if ( isFocused )
    {
        rectLabel.left--;
        rectLabel.right++;
        if ( !::DrawFocusRect(hdc, &rectLabel) )
        {
            wxLogLastError("DrawFocusRect()");
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
// wxControlWithItems
// ----------------------------------------------------------------------------

void wxControlWithItems::MSWAllocStorage(const std::vector<std::string>& items,
                                         unsigned wm)
{
    // FIXME: use reduce
    const auto numItems = items.size();
    std::size_t totalTextLength = numItems; // for trailing '\0' characters
    for ( std::size_t i{0}; i != numItems; ++i )
    {
        totalTextLength += items[i].length();
    }

    if ( ::SendMessageW((WXHWND)MSWGetItemsHWND(), wm, numItems,
                     (WXLPARAM)totalTextLength*sizeof(wxChar)) == LB_ERRSPACE )
    {
        wxLogLastError("SendMessage(XX_INITSTORAGE)");
    }
}

int wxControlWithItems::MSWInsertOrAppendItem(unsigned pos,
                                              const std::string& item,
                                              unsigned wm)
{
    LRESULT n = ::SendMessageW((WXHWND)MSWGetItemsHWND(), wm, pos,
                            reinterpret_cast<WXLPARAM>(boost::nowide::widen(item).c_str()));
    if ( n == CB_ERR || n == CB_ERRSPACE )
    {
        wxLogLastError("SendMessage(XX_ADD/INSERTSTRING)");
        return wxNOT_FOUND;
    }

    return n;
}

#endif // wxUSE_CONTROLS
