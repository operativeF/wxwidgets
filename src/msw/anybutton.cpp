/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/anybutton.cpp
// Purpose:     wxAnyButton
// Author:      Julian Smart
// Created:     1998-01-04 (extracted from button.cpp)
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef wxHAS_ANY_BUTTON

#include "wx/anybutton.h"

#include "wx/brush.h"
#include "wx/bmpbuttn.h"
#include "wx/dcclient.h"
#include "wx/toplevel.h"

#include "wx/imaglist.h"
#include "wx/stockitem.h"
#include "wx/msw/private/button.h"
#include "wx/msw/private/dc.h"
#include "wx/msw/private/winstyle.h"
#include "wx/msw/uxtheme.h"
#include "wx/private/window.h"

#if wxUSE_MARKUP
    #include "wx/generic/private/markuptext.h"
#endif // wxUSE_MARKUP

#include "wx/msw/wrapcctl.h"
#include "wx/msw/private.h"

#include <boost/nowide/convert.hpp>

#include <memory>

import Utils.Strings;
import WX.Utils.Settings;

import WX.WinDef;
import WX.Win.UniqueHnd;

import <string>;
import <string_view>;
import <vector>;

using namespace wxMSWImpl;

using msw::utils::unique_brush;

#if wxUSE_UXTHEME
extern wxWindowMSW *wxWindowBeingErased; // From src/msw/window.cpp
#endif // wxUSE_UXTHEME

// the gap between button edge and the interior area used by Windows for the
// standard buttons
constexpr int OD_BUTTON_MARGIN = 4;

namespace
{

class wxODButtonImageData : public wxButtonImageData
{
public:
    wxODButtonImageData(wxAnyButton *btn, const wxBitmap& bitmap)
    {
        SetBitmap(bitmap, wxAnyButton::State_Normal);
#if wxUSE_IMAGE
        SetBitmap(bitmap.ConvertToDisabled(), wxAnyButton::State_Disabled);
#endif
        // we use margins when we have both bitmap and text, but when we have
        // only the bitmap it should take up the entire button area
        if ( btn->ShowsLabel() )
        {
            m_margin.x = btn->wxGetCharWidth();
            m_margin.y = btn->GetCharHeight() / 2;
        }
    }

	wxODButtonImageData& operator=(wxODButtonImageData&&) = delete;

    wxBitmap GetBitmap(wxAnyButton::State which) const override
    {
        return m_bitmaps[which];
    }

    void SetBitmap(const wxBitmap& bitmap, wxAnyButton::State which) override
    {
        m_bitmaps[which] = bitmap;
    }

    wxSize GetBitmapMargins() const override
    {
        return m_margin;
    }

    void SetBitmapMargins(wxCoord x, wxCoord y) override
    {
        m_margin = wxSize(x, y);
    }

    wxDirection GetBitmapPosition() const override
    {
        return m_dir;
    }

    void SetBitmapPosition(wxDirection dir) override
    {
        m_dir = dir;
    }

private:
    // just store the values passed to us to be able to retrieve them later
    // from the drawing code
    wxBitmap m_bitmaps[wxAnyButton::State_Max];
    wxSize m_margin;
    wxDirection m_dir{wxDirection::wxLEFT};
};

#if wxUSE_UXTHEME

// somehow the margin is one pixel greater than the value returned by
// GetThemeMargins() call
constexpr int XP_BUTTON_EXTRA_MARGIN = 1;

class wxXPButtonImageData : public wxButtonImageData
{
public:
    // we must be constructed with the size of our images as we need to create
    // the image list
    wxXPButtonImageData(wxAnyButton *btn, const wxBitmap& bitmap)
        : m_iml(bitmap.GetWidth(), bitmap.GetHeight(),
                !bitmap.HasAlpha() /* use mask only if no alpha */,
                wxAnyButton::State_Max + 1 /* see "pulse" comment below */),
          m_hwndBtn(GetHwndOf(btn))
    {
        // initialize all bitmaps except for the disabled one to normal state
        for ( int n = 0; n < wxAnyButton::State_Max; n++ )
        {
#if wxUSE_IMAGE
            m_iml.Add(n == wxAnyButton::State_Disabled ? bitmap.ConvertToDisabled()
                                                    : bitmap);
#else
            m_iml.Add(bitmap);
#endif
        }

        // In addition to the states supported by wxWidgets such as normal,
        // hot, pressed, disabled and focused, we need to add bitmap for
        // another state when running under Windows 7 -- the so called "stylus
        // hot" state corresponding to PBS_STYLUSHOT constant. While it's
        // documented in MSDN as being only used with tablets, it is a lie as
        // a focused button actually alternates between the image list elements
        // with PBS_DEFAULTED and PBS_STYLUSHOT indices and, in particular,
        // just disappears during half of the time if the latter is not set so
        // we absolutely must set it.
        //
        // This also explains why we need to allocate an extra slot in the
        // image list ctor above, the slot State_Max is used for this one.
        m_iml.Add(bitmap);

        m_data.himl = GetHimagelistOf(&m_iml);

        // no margins by default
        ::SetRectEmpty(&m_data.margin);

        // use default alignment
        m_data.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

        UpdateImageInfo();
    }

    wxXPButtonImageData(const wxXPButtonImageData&) = delete;
	wxXPButtonImageData& operator=(const wxXPButtonImageData&) = delete;

    wxBitmap GetBitmap(wxAnyButton::State which) const override
    {
        return m_iml.GetBitmap(which);
    }

    void SetBitmap(const wxBitmap& bitmap, wxAnyButton::State which) override
    {
        m_iml.Replace(which, bitmap);

        // As we want the focused button to always show its bitmap, we need to
        // update the "stylus hot" one to match it to avoid any pulsing.
        if ( which == wxAnyButton::State_Focused )
            m_iml.Replace(wxAnyButton::State_Max, bitmap);

        UpdateImageInfo();
    }

    wxSize GetBitmapMargins() const override
    {
        return {m_data.margin.left, m_data.margin.top};
    }

    void SetBitmapMargins(wxCoord x, wxCoord y) override
    {
        ::SetRect(&m_data.margin, x, y, x, y);

        UpdateImageInfo();
    }

    wxDirection GetBitmapPosition() const override
    {
        switch ( m_data.uAlign )
        {
            default:
                wxFAIL_MSG( "invalid image alignment" );
                [[fallthrough]];

            case BUTTON_IMAGELIST_ALIGN_LEFT:
                return wxDirection::wxLEFT;

            case BUTTON_IMAGELIST_ALIGN_RIGHT:
                return wxDirection::wxRIGHT;

            case BUTTON_IMAGELIST_ALIGN_TOP:
                return wxDirection::wxTOP;

            case BUTTON_IMAGELIST_ALIGN_BOTTOM:
                return wxDirection::wxBOTTOM;
        }
    }

    void SetBitmapPosition(wxDirection dir) override
    {
        WXUINT alignNew;
        switch ( dir )
        {
            default:
                wxFAIL_MSG( "invalid direction" );
                [[fallthrough]];

            case wxDirection::wxLEFT:
                alignNew = BUTTON_IMAGELIST_ALIGN_LEFT;
                break;

            case wxDirection::wxRIGHT:
                alignNew = BUTTON_IMAGELIST_ALIGN_RIGHT;
                break;

            case wxDirection::wxTOP:
                alignNew = BUTTON_IMAGELIST_ALIGN_TOP;
                break;

            case wxDirection::wxBOTTOM:
                alignNew = BUTTON_IMAGELIST_ALIGN_BOTTOM;
                break;
        }

        if ( alignNew != m_data.uAlign )
        {
            m_data.uAlign = alignNew;
            UpdateImageInfo();
        }
    }

private:
    void UpdateImageInfo()
    {
        if ( !::SendMessageW(m_hwndBtn, BCM_SETIMAGELIST, 0, (WXLPARAM)&m_data) )
        {
            wxLogDebug("SendMessage(BCM_SETIMAGELIST) failed");
        }
    }

    // we store image list separately to be able to use convenient wxImageList
    // methods instead of working with raw HIMAGELIST
    wxImageList m_iml;

    // store the rest of the data in BCM_SETIMAGELIST-friendly form
    BUTTON_IMAGELIST m_data;

    // the button we're associated with
    const WXHWND m_hwndBtn;
};

#endif // wxUSE_UXTHEME

// Right- and bottom-aligned images stored in the image list
// (BUTTON_IMAGELIST) for some reasons are not drawn with proper
// margins so for such alignments we need to switch to owner-drawn
// mode a do the job on our own.
static inline bool NeedsOwnerDrawnForImageLayout(wxDirection dir, int margH, int margV)
{
    return (dir == wxRIGHT && margH != 0) || (dir == wxBOTTOM && margV != 0);
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// helper functions from wx/msw/private/button.h
// ----------------------------------------------------------------------------

void wxMSWButton::UpdateMultilineStyle(WXHWND hwnd, std::string_view label)
{
    // update BS_MULTILINE style depending on the new label (resetting it
    // doesn't seem to do anything very useful but it shouldn't hurt and we do
    // have to set it whenever the label becomes multi line as otherwise it
    // wouldn't be shown correctly as we don't use BS_MULTILINE when creating
    // the control unless it already has new lines in its label)
    wxMSWWinStyleUpdater updateStyle(hwnd);
    if ( label.find('\n') != std::string_view::npos )
        updateStyle.TurnOn(BS_MULTILINE);
    else
        updateStyle.TurnOff(BS_MULTILINE);
}

wxSize wxMSWButton::GetFittingSize(wxWindow *win,
                                   const wxSize& sizeLabel,
                                   unsigned int flags)
{
    wxSize sizeBtn = sizeLabel;

    // FIXME: The numbers here are pure guesswork, no idea how should the
    //        button margins be really calculated.
    if ( flags & Size_ExactFit )
    {
        // We still need some margin or the text would be overwritten, just
        // make it as small as possible.
        sizeBtn.x += 2*win->wxGetCharWidth();
    }
    else
    {
        sizeBtn.x += 3*win->wxGetCharWidth();
    }

    // account for the shield UAC icon if we have it
    if ( flags & Size_AuthNeeded )
        sizeBtn.x += wxSystemSettings::GetMetric(wxSYS_SMALLICON_X, win);

    return sizeBtn;
}

wxSize wxMSWButton::ComputeBestFittingSize(wxControl *btn, unsigned int flags)
{
    wxClientDC dc(btn);

    wxSize sizeBtn;
    dc.GetMultiLineTextExtent(btn->GetLabelText(), &sizeBtn.x, &sizeBtn.y);

    return GetFittingSize(btn, sizeBtn, flags);
}

wxSize wxMSWButton::IncreaseToStdSizeAndCache(wxControl *btn, const wxSize& size)
{
    wxSize sizeBtn(size);

    // By default all buttons have at least the standard size.
    if ( !btn->HasFlag(wxBU_EXACTFIT) )
    {
        // The "Recommended sizing and spacing" section of MSDN layout article
        // documents the default button size as being 50*14 dialog units or
        // 75*23 relative pixels (what we call DIPs). As dialog units don't
        // scale well in high DPI because of rounding errors, just DIPs here.
        //
        // Moreover, it looks like the extra 2px borders around the visible
        // part of the button are not scaled correctly in higher than normal
        // DPI, so add them without scaling.
        const wxSize sizeDef = btn->FromDIP(wxSize(73, 21)) + wxSize(2, 2);

        sizeBtn.IncTo(sizeDef);
    }

    // wxBU_EXACTFIT is typically used alongside a text control or similar,
    // so make them as high as it.
    // The standard height is generally higher than this, but if not (e.g. when
    // using a larger font) increase the button height as well.
    int yText = wxGetCharSize(GetHwndOf(btn), btn->GetFont()).y;
    yText = wxGetEditHeightFromCharHeight(yText, btn);

    sizeBtn.IncTo(wxSize(-1, yText));

    btn->CacheBestSize(sizeBtn);

    return sizeBtn;
}

// ----------------------------------------------------------------------------
// creation/destruction
// ----------------------------------------------------------------------------

wxAnyButton::~wxAnyButton()
{
#if wxUSE_MARKUP
    delete m_markupText;
#endif // wxUSE_MARKUP
}

void wxAnyButton::SetLabel(std::string_view label)
{
    wxMSWButton::UpdateMultilineStyle(GetHwnd(), label);

    wxAnyButtonBase::SetLabel(label);

#if wxUSE_MARKUP
    // If we have a plain text label, we shouldn't be using markup any longer.
    if ( m_markupText )
    {
        delete m_markupText;
        m_markupText = nullptr;

        // Unfortunately we don't really know whether we can reset the button
        // to be non-owner-drawn or not: if we had made it owner-drawn just
        // because of a call to SetLabelMarkup(), we could, but not if there
        // were [also] calls to Set{Fore,Back}groundColour(). If it's really a
        // problem to have button remain owner-drawn forever just because it
        // had markup label once, we should record the reason for our current
        // owner-drawnness and check it here.
    }
#endif // wxUSE_MARKUP
}

// ----------------------------------------------------------------------------
// size management including autosizing
// ----------------------------------------------------------------------------

void wxAnyButton::AdjustForBitmapSize(wxSize &size) const
{
    wxCHECK_RET( m_imageData, "shouldn't be called if no image" );

    // account for the bitmap size, including the user-specified margins
    const wxSize sizeBmp = m_imageData->GetBitmap(State_Normal).GetSize()
                                + 2*m_imageData->GetBitmapMargins();
    const wxDirection dirBmp = m_imageData->GetBitmapPosition();
    if ( dirBmp == wxDirection::wxLEFT || dirBmp == wxDirection::wxRIGHT )
    {
        size.x += sizeBmp.x;
        if ( sizeBmp.y > size.y )
            size.y = sizeBmp.y;
    }
    else // bitmap on top/below the text
    {
        size.y += sizeBmp.y;
        if ( sizeBmp.x > size.x )
            size.x = sizeBmp.x;
    }

    // and also for the margins we always add internally (unless we have no
    // border at all in which case the button has exactly the same size as
    // bitmap and so no margins should be used)
    AdjustForBitmapMargins(size);
}

void wxAnyButton::AdjustForBitmapMargins(wxSize& size) const
{
    wxCHECK_RET(m_imageData, "shouldn't be called if no image");

    // and also for the margins we always add internally (unless we have no
    // border at all in which case the button has exactly the same size as
    // bitmap and so no margins should be used)
    if ( !HasFlag(wxBORDER_NONE) )
    {
        int marginH = 0,
            marginV = 0;
#if wxUSE_UXTHEME
        if ( wxUxThemeIsActive() )
        {
            wxUxThemeHandle theme(const_cast<wxAnyButton *>(this), "BUTTON");

            // Initialize margins with the default values (at least under
            // Windows 7) in case GetThemeMargins() fails.
            MARGINS margins = {3, 3, 3, 3};
            ::GetThemeMargins(theme, nullptr,
                                                    BP_PUSHBUTTON,
                                                    PBS_NORMAL,
                                                    TMT_CONTENTMARGINS,
                                                    nullptr,
                                                    &margins);

            // XP doesn't draw themed buttons correctly when the client
            // area is smaller than 8x8 - enforce this minimum size for
            // small bitmaps
            size.IncTo(wxSize(8, 8));

            marginH = margins.cxLeftWidth + margins.cxRightWidth
                        + 2*XP_BUTTON_EXTRA_MARGIN;
            marginV = margins.cyTopHeight + margins.cyBottomHeight
                        + 2*XP_BUTTON_EXTRA_MARGIN;
        }
        else
#endif // wxUSE_UXTHEME
        {
            marginH =
            marginV = OD_BUTTON_MARGIN;
        }

        size.IncBy(marginH, marginV);
    }
}

wxSize wxAnyButton::DoGetBestSize() const
{
    wxAnyButton * const self = const_cast<wxAnyButton *>(this);

    wxSize size;

    // BCM_GETIDEALSIZE works properly only it there is a text label in the button.
    if ( !IsOwnerDrawn() && ShowsLabel() )
    {
        SIZE idealSize = { 0, 0 };
        ::SendMessageW(GetHwnd(), BCM_GETIDEALSIZE, 0, (WXLPARAM)&idealSize);
        size.Set(idealSize.cx, idealSize.cy);

        if ( m_imageData )
            AdjustForBitmapMargins(size);
    }
    else
    {
        // Account for the text part if we have it.
        if ( ShowsLabel() )
        {
            unsigned int flags{};
            if ( HasFlag(wxBU_EXACTFIT) )
                flags |= wxMSWButton::Size_ExactFit;
            if ( DoGetAuthNeeded() )
                flags |= wxMSWButton::Size_AuthNeeded;

#if wxUSE_MARKUP
            if ( m_markupText )
            {
                wxClientDC dc(self);
                size = wxMSWButton::GetFittingSize(self,
                                                   m_markupText->Measure(dc),
                                                   flags);
            }
            else // Normal plain text (but possibly multiline) label.
#endif // wxUSE_MARKUP
            {
                size = wxMSWButton::ComputeBestFittingSize(self, flags);
            }
        }

        if ( m_imageData )
            AdjustForBitmapSize(size);
    }

    return wxMSWButton::IncreaseToStdSizeAndCache(self, size);
}

// ----------------------------------------------------------------------------
// event/message handlers
// ----------------------------------------------------------------------------

WXLRESULT wxAnyButton::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    if ( nMsg == WM_LBUTTONDBLCLK )
    {
        // emulate a click event to force an owner-drawn button to change its
        // appearance - without this, it won't do it
        std::ignore = wxControl::MSWWindowProc(WM_LBUTTONDOWN, wParam, lParam);

        // and continue with processing the message normally as well
    }
#if wxUSE_UXTHEME
    else if ( nMsg == WM_THEMECHANGED )
    {
        // need to recalculate the best size here
        // as the theme size might have changed
        InvalidateBestSize();
    }
#endif // wxUSE_UXTHEME
    // must use m_mouseInWindow here instead of IsMouseInWindow()
    // since we need to know the first time the mouse enters the window
    // and IsMouseInWindow() would return true in this case
    else if ( (nMsg == WM_MOUSEMOVE && !m_mouseInWindow) ||
                nMsg == WM_MOUSELEAVE )
    {
        if (
                IsEnabled() &&
                ( wxUxThemeIsActive() ||
                 (m_imageData && m_imageData->GetBitmap(State_Current).IsOk())
                )
           )
        {
            Refresh();
        }
    }

    // let the base class do all real processing
    return wxControl::MSWWindowProc(nMsg, wParam, lParam);
}

// ----------------------------------------------------------------------------
// button images
// ----------------------------------------------------------------------------

wxBitmap wxAnyButton::DoGetBitmap(State which) const
{
    return m_imageData ? m_imageData->GetBitmap(which) : wxBitmap();
}

void wxAnyButton::DoSetBitmap(const wxBitmap& bitmap, State which)
{
    if ( !bitmap.IsOk() )
    {
        if ( m_imageData  )
        {
            // Normal image is special: setting it enables images for the
            // button and resetting it to nothing disables all of them.
            if ( which == State_Normal )
            {
                m_imageData.reset();
            }
            else
            {
                // Replace the removed bitmap with the normal one.
                wxBitmap bmpNormal = m_imageData->GetBitmap(State_Normal);
                m_imageData->SetBitmap(which == State_Disabled
                                            ? bmpNormal.ConvertToDisabled()
                                            : bmpNormal,
                                       which);
            }
        }

        return;
    }

#if wxUSE_UXTHEME
    std::unique_ptr<wxXPButtonImageData> oldData;
#endif // wxUSE_UXTHEME

    // Check if we already had bitmaps of different size.
    if ( m_imageData &&
          bitmap.GetSize() != m_imageData->GetBitmap(State_Normal).GetSize() )
    {
        wxASSERT_MSG( (which == State_Normal) || bitmap.IsNull(),
                      "Must set normal bitmap with the new size first" );

#if wxUSE_UXTHEME
        // We can't change the size of the images stored in wxImageList
        // in wxXPButtonImageData::m_impl so force recreating it below but
        // keep the current data to copy its values into the new one.
        oldData.reset(dynamic_cast<wxXPButtonImageData*>(m_imageData.release()));

#endif // wxUSE_UXTHEME
        //else: wxODButtonImageData doesn't require anything special
    }

    // allocate the image data when the first bitmap is set
    if ( !m_imageData )
    {
#if wxUSE_UXTHEME
        // using image list doesn't work correctly if we don't have any label
        // (even if we use BUTTON_IMAGELIST_ALIGN_CENTER alignment and
        // BS_BITMAP style), at least under Windows 2003 so use owner drawn
        // strategy for bitmap-only buttons
        if ( ShowsLabel() && wxUxThemeIsActive() )
        {
            m_imageData = std::make_unique<wxXPButtonImageData>(this, bitmap);

            if ( oldData )
            {
                // Preserve the old values in case the user changed them.
                m_imageData->SetBitmapPosition(oldData->GetBitmapPosition());

                const wxSize oldMargins = oldData->GetBitmapMargins();
                m_imageData->SetBitmapMargins(oldMargins.x, oldMargins.y);

                // No need to preserve the bitmaps though as they were of wrong
                // size anyhow.
            }
        }
        else
#endif // wxUSE_UXTHEME
        {
            m_imageData = std::make_unique<wxODButtonImageData>(this, bitmap);
            MakeOwnerDrawn();
        }
    }
    else
    {
        m_imageData->SetBitmap(bitmap, which);

        // if the focus bitmap is specified but current one isn't, use
        // the focus bitmap for hovering as well if this is consistent
        // with the current Windows version look and feel.
        //
        // rationale: this is compatible with the old wxGTK behaviour
        // and also makes it much easier to do "the right thing" for
        // all platforms (some of them, such as Windows, have "hot"
        // buttons while others don't)
        if ( which == State_Focused && !m_imageData->GetBitmap(State_Current).IsOk() )
        {
            m_imageData->SetBitmap(bitmap, State_Current);
        }
    }

    // it should be enough to only invalidate the best size when the normal
    // bitmap changes as all bitmaps assigned to the button should be of the
    // same size anyhow
    if ( which == State_Normal )
        InvalidateBestSize();

    Refresh();
}

wxSize wxAnyButton::DoGetBitmapMargins() const
{
    return m_imageData ? m_imageData->GetBitmapMargins() : wxSize(0, 0);
}

void wxAnyButton::DoSetBitmapMargins(wxCoord x, wxCoord y)
{
    wxCHECK_RET( m_imageData, "SetBitmap() must be called first" );
    if ( NeedsOwnerDrawnForImageLayout(m_imageData->GetBitmapPosition(), x, y) )
    {
        MakeOwnerDrawn();
    }
    m_imageData->SetBitmapMargins(x, y);
    InvalidateBestSize();
}

void wxAnyButton::DoSetBitmapPosition(wxDirection dir)
{
    if ( m_imageData )
    {
        wxSize margs = m_imageData->GetBitmapMargins();
        if ( NeedsOwnerDrawnForImageLayout(dir, margs.x, margs.y) )
        {
            MakeOwnerDrawn();
        }
        m_imageData->SetBitmapPosition(dir);
    }
    InvalidateBestSize();
}

// ----------------------------------------------------------------------------
// markup support
// ----------------------------------------------------------------------------

#if wxUSE_MARKUP

bool wxAnyButton::DoSetLabelMarkup(const std::string& markup)
{
    if ( !wxAnyButtonBase::DoSetLabelMarkup(markup) )
        return false;

    if ( !m_markupText )
    {
        m_markupText = new wxMarkupText(markup);
        MakeOwnerDrawn();
    }
    else
    {
        // We are already owner-drawn so just update the text.
        m_markupText->SetMarkup(markup);
    }

    Refresh();

    return true;
}

#endif // wxUSE_MARKUP

// ----------------------------------------------------------------------------
// owner-drawn buttons support
// ----------------------------------------------------------------------------

// drawing helpers
namespace
{

// return the button state using both the ODS_XXX flags specified in state
// parameter and the current button state
wxAnyButton::State GetButtonState(wxAnyButton *btn, WXUINT state)
{
    if ( state & ODS_DISABLED )
        return wxAnyButton::State_Disabled;

    // We need to check for the pressed state of the button itself before the
    // other checks because even if it is selected or current, it it still
    // pressed first and foremost.
    const wxAnyButton::State btnState = btn->GetNormalState();

    if ( btnState == wxAnyButton::State_Pressed || state & ODS_SELECTED )
        return wxAnyButton::State_Pressed;

    if ( btn->HasCapture() || btn->IsMouseInWindow() )
        return wxAnyButton::State_Current;

    if ( state & ODS_FOCUS )
        return wxAnyButton::State_Focused;

    return btnState;
}

void DrawButtonText(WXHDC hdc,
                    RECT *pRect,
                    wxAnyButton *btn,
                    unsigned int flags)
{
    const std::string text = btn->GetLabel();

    // To get a native look for owner-drawn button in disabled state (without
    // theming) we must use DrawState() to draw the text label.
    //
    // Notice that we use the enabled state at MSW, not wx, level because we
    // don't want to grey it out when it's disabled just because its parent is
    // disabled by MSW as it happens when showing a modal dialog, but we do
    // want to grey it out if either it or its parent are explicitly disabled
    // at wx level, see #18011.
    if ( !wxUxThemeIsActive() && !::IsWindowEnabled(GetHwndOf(btn)) )
    {
        // However using DrawState() has some drawbacks:
        // 1. It generally doesn't support alignment flags (except right
        //    alignment), so we need to align the text on our own.
        // 2. It doesn't support multliline texts and there is necessary to
        //    draw/align multiline text line by line.

        // Compute bounding rect for the whole text.
        RECT rc;
        ::SetRectEmpty(&rc);
        ::DrawTextW(hdc, boost::nowide::widen(text).c_str(), text.length(), &rc, DT_CALCRECT);

        const LONG h = rc.bottom - rc.top;

        // Based on wxButton flags determine bottom edge of the drawing rect
        // inside the entire button area.
        int y0;
        if ( btn->HasFlag(wxBU_BOTTOM) )
        {
            y0 = pRect->bottom - h;
        }
        else if ( !btn->HasFlag(wxBU_TOP) )
        {
            // DT_VCENTER
            y0 = pRect->top + (pRect->bottom - pRect->top)/2 - h/2;
        }
        else // DT_TOP is the default
        {
            y0 = pRect->top;
        }

        WXUINT dsFlags = DSS_DISABLED;
        if( flags & DT_HIDEPREFIX )
            dsFlags |= (DSS_HIDEPREFIX | DST_PREFIXTEXT);
        else
            dsFlags |= DST_TEXT;

        const std::vector<std::string_view> lines = wx::unsafe::StrViewSplit(text, '\n');
        const int hLine = h / lines.size();
        size_t lineNum{ 0 };

        for ( auto line : lines )
        {
            // Each line must be aligned in horizontal direction individually.
            ::SetRectEmpty(&rc);
            ::DrawTextW(hdc, boost::nowide::widen(line).c_str(), line.length(),
                       &rc, DT_CALCRECT);
            const LONG w = rc.right - rc.left;

            // Based on wxButton flags set horizontal position of the rect
            // inside the entire button area. Text is always centered for
            // multiline label.
            if ( (!btn->HasFlag(wxBU_LEFT) && !btn->HasFlag(wxBU_RIGHT)) ||
                    lines.size() > 1 )
            {
                // DT_CENTER
                rc.left = pRect->left + (pRect->right - pRect->left)/2 - w/2;
                rc.right = rc.left + w;
            }
            else if ( btn->HasFlag(wxBU_RIGHT) )
            {
                rc.right = pRect->right;
                rc.left = rc.right - w;
            }
            else // DT_LEFT is the default
            {
                rc.left = pRect->left;
                rc.right = rc.left  + w;
            }

            ::OffsetRect(&rc, 0, y0 + lineNum * hLine);

            ::DrawStateW(hdc, nullptr, nullptr, reinterpret_cast<WXLPARAM>(boost::nowide::widen(line).c_str()),
                        line.length(),
                        rc.left, rc.top, rc.right, rc.bottom, dsFlags);
            ++lineNum;
        }
    }
    else // Button is enabled or using themes.
    {
        if ( text.find('\n') != std::string::npos )
        {
            // draw multiline label

            // first we need to compute its bounding rect
            RECT rc;
            ::CopyRect(&rc, pRect);
            ::DrawTextW(hdc, boost::nowide::widen(text).c_str(), text.length(), &rc,
                       flags | DT_CALCRECT);

            // now position this rect inside the entire button area: notice
            // that DrawText() doesn't respect alignment flags for multiline
            // text, which is why we have to do it on our own (but still use
            // the horizontal alignment flags for the individual lines to be
            // aligned correctly)
            const LONG w = rc.right - rc.left;
            const LONG h = rc.bottom - rc.top;

            if ( btn->HasFlag(wxBU_RIGHT) )
            {
                rc.left = pRect->right - w;

                flags |= DT_RIGHT;
            }
            else if ( !btn->HasFlag(wxBU_LEFT) )
            {
                rc.left = pRect->left + (pRect->right - pRect->left)/2 - w/2;

                flags |= DT_CENTER;
            }
            else // wxBU_LEFT
            {
                rc.left = pRect->left;
            }

            if ( btn->HasFlag(wxBU_BOTTOM) )
            {
                rc.top = pRect->bottom - h;
            }
            else if ( !btn->HasFlag(wxBU_TOP) )
            {
                rc.top = pRect->top + (pRect->bottom - pRect->top)/2 - h/2;
            }
            else // wxBU_TOP
            {
                rc.top = pRect->top;
            }

            rc.right = rc.left+w;
            rc.bottom = rc.top+h;

            ::DrawTextW(hdc, boost::nowide::widen(text).c_str(), text.length(), &rc, flags);
        }
        else // single line label
        {
            // translate wx button flags to alignment flags for DrawText()
            if ( btn->HasFlag(wxBU_RIGHT) )
            {
                flags |= DT_RIGHT;
            }
            else if ( !btn->HasFlag(wxBU_LEFT) )
            {
                flags |= DT_CENTER;
            }
            //else: DT_LEFT is the default anyhow (and its value is 0 too)

            if ( btn->HasFlag(wxBU_BOTTOM) )
            {
                flags |= DT_BOTTOM;
            }
            else if ( !btn->HasFlag(wxBU_TOP) )
            {
                flags |= DT_VCENTER;
            }
            //else: as above, DT_TOP is the default

            // notice that we must have DT_SINGLELINE for vertical alignment
            // flags to work
            ::DrawTextW(hdc, boost::nowide::widen(text).c_str(), text.length(), pRect,
                       flags | DT_SINGLELINE );
        }
    }
}

void DrawRect(WXHDC hdc, const RECT& r, COLORREF color)
{
    wxDrawHVLine(hdc, r.left, r.top, r.right, r.top, color, 1);
    wxDrawHVLine(hdc, r.right, r.top, r.right, r.bottom, color, 1);
    wxDrawHVLine(hdc, r.right, r.bottom, r.left, r.bottom, color, 1);
    wxDrawHVLine(hdc, r.left, r.bottom, r.left, r.top, color, 1);
}

/*
   The button frame looks like this normally:

   WWWWWWWWWWWWWWWWWWB
   WHHHHHHHHHHHHHHHHGB  W = white       (HILIGHT)
   WH               GB  H = light grey  (LIGHT)
   WH               GB  G = dark grey   (SHADOW)
   WH               GB  B = black       (DKSHADOW)
   WH               GB
   WGGGGGGGGGGGGGGGGGB
   BBBBBBBBBBBBBBBBBBB

   When the button is selected, the button becomes like this (the total button
   size doesn't change):

   BBBBBBBBBBBBBBBBBBB
   BWWWWWWWWWWWWWWWWBB
   BWHHHHHHHHHHHHHHGBB
   BWH             GBB
   BWH             GBB
   BWGGGGGGGGGGGGGGGBB
   BBBBBBBBBBBBBBBBBBB
   BBBBBBBBBBBBBBBBBBB

   When the button is pushed (while selected) it is like:

   BBBBBBBBBBBBBBBBBBB
   BGGGGGGGGGGGGGGGGGB
   BG               GB
   BG               GB
   BG               GB
   BG               GB
   BGGGGGGGGGGGGGGGGGB
   BBBBBBBBBBBBBBBBBBB
*/
void DrawButtonFrame(WXHDC hdc, RECT& rectBtn,
                     bool selected, bool pushed)
{
    RECT r;
    ::CopyRect(&r, &rectBtn);

    COLORREF clrBlack = ::GetSysColor(COLOR_3DDKSHADOW),
             clrGrey = ::GetSysColor(COLOR_3DSHADOW),
             clrLightGr = ::GetSysColor(COLOR_3DLIGHT),
             clrWhite = ::GetSysColor(COLOR_3DHILIGHT);

    r.right--;
    r.bottom--;

    if ( pushed )
    {
        DrawRect(hdc, r, clrBlack);

        ::InflateRect(&r, -1, -1);

        DrawRect(hdc, r, clrGrey);
    }
    else // !pushed
    {
        if ( selected )
        {
            DrawRect(hdc, r, clrBlack);

            ::InflateRect(&r, -1, -1);
        }

        wxDrawHVLine(hdc, r.left, r.bottom, r.right, r.bottom, clrBlack, 1);
        wxDrawHVLine(hdc, r.right, r.bottom, r.right, r.top - 1, clrBlack, 1);

        wxDrawHVLine(hdc, r.left, r.bottom - 1, r.left, r.top, clrWhite, 1);
        wxDrawHVLine(hdc, r.left, r.top, r.right, r.top, clrWhite, 1);

        wxDrawHVLine(hdc, r.left + 1, r.bottom - 2, r.left + 1, r.top + 1, clrLightGr, 1);
        wxDrawHVLine(hdc, r.left + 1, r.top + 1, r.right - 1, r.top + 1, clrLightGr, 1);

        wxDrawHVLine(hdc, r.left + 1, r.bottom - 1, r.right - 1, r.bottom - 1, clrGrey, 1);
        wxDrawHVLine(hdc, r.right - 1, r.bottom - 1, r.right - 1, r.top, clrGrey, 1);
    }

    InflateRect(&rectBtn, -OD_BUTTON_MARGIN, -OD_BUTTON_MARGIN);
}

#if wxUSE_UXTHEME
void DrawXPBackground(wxAnyButton *button, WXHDC hdc, RECT& rectBtn, WXUINT state)
{
    wxUxThemeHandle theme(button, "BUTTON");

    // this array is indexed by wxAnyButton::State values and so must be kept in
    // sync with it
    static constexpr int uxStates[] =
    {
        PBS_NORMAL, PBS_HOT, PBS_PRESSED, PBS_DISABLED, PBS_DEFAULTED
    };

    int iState = uxStates[GetButtonState(button, state)];

    // draw parent background if needed
    if ( ::IsThemeBackgroundPartiallyTransparent
                 (
                    theme,
                    BP_PUSHBUTTON,
                    iState
                 ) )
    {
        // Set this button as the one whose background is being erased: this
        // allows our WM_ERASEBKGND handler used by DrawThemeParentBackground()
        // to correctly align the background brush with this window instead of
        // the parent window to which WM_ERASEBKGND is sent. Notice that this
        // doesn't work with custom user-defined EVT_ERASE_BACKGROUND handlers
        // as they won't be aligned but unfortunately all the attempts to fix
        // it by shifting DC origin before calling DrawThemeParentBackground()
        // failed to work so we at least do this, even though this is far from
        // being the perfect solution.
        wxWindowBeingErased = button;

        ::DrawThemeParentBackground(GetHwndOf(button), hdc, &rectBtn);

        wxWindowBeingErased = nullptr;
    }

    // draw background
    ::DrawThemeBackground(theme, hdc, BP_PUSHBUTTON, iState,
                                &rectBtn, nullptr);

    // calculate content area margins, using the defaults in case we fail to
    // retrieve the current theme margins
    MARGINS margins = {3, 3, 3, 3};
    ::GetThemeMargins(theme, hdc, BP_PUSHBUTTON, iState,
                            TMT_CONTENTMARGINS, &rectBtn, &margins);
    ::InflateRect(&rectBtn, -margins.cxLeftWidth, -margins.cyTopHeight);

    if ( button->UseBgCol() && iState != PBS_HOT )
    {
        COLORREF colBg = wxColourToRGB(button->GetBackgroundColour());
        auto hbrushBackground = unique_brush(::CreateSolidBrush(colBg));

        ::FillRect(hdc, &rectBtn, hbrushBackground.get());
    }

    ::InflateRect(&rectBtn, -XP_BUTTON_EXTRA_MARGIN, -XP_BUTTON_EXTRA_MARGIN);
}
#endif // wxUSE_UXTHEME

} // anonymous namespace

// ----------------------------------------------------------------------------
// owner drawn buttons support
// ----------------------------------------------------------------------------

void wxAnyButton::MakeOwnerDrawn()
{
    if ( !IsOwnerDrawn() )
    {
        // We need to use owner-drawn specific data structure so we have
        // to create it and copy the data from native data structure,
        // if necessary.
        if ( m_imageData && dynamic_cast<wxODButtonImageData*>(m_imageData.get()) == nullptr )
        {
            auto newData = std::make_unique<wxODButtonImageData>(this, m_imageData->GetBitmap(State_Normal));
            for ( int n = 0; n < State_Max; n++ )
            {
                State st = static_cast<State>(n);
                newData->SetBitmap(m_imageData->GetBitmap(st), st);
            }
            newData->SetBitmapPosition(m_imageData->GetBitmapPosition());
            wxSize margs = m_imageData->GetBitmapMargins();
            newData->SetBitmapMargins(margs.x, margs.y);

            m_imageData = std::move(newData);
        }
        // make it so
        wxMSWWinStyleUpdater(GetHwnd()).TurnOff(BS_TYPEMASK).TurnOn(BS_OWNERDRAW);
    }
}

bool wxAnyButton::IsOwnerDrawn() const
{
    unsigned int style = ::GetWindowLongPtrW(GetHwnd(), GWL_STYLE);
    return ( (style & BS_OWNERDRAW) == BS_OWNERDRAW );
}

bool wxAnyButton::SetBackgroundColour(const wxColour &colour)
{
    if ( !wxControl::SetBackgroundColour(colour) )
    {
        // nothing to do
        return false;
    }

    MakeOwnerDrawn();

    Refresh();

    return true;
}

bool wxAnyButton::SetForegroundColour(const wxColour &colour)
{
    if ( !wxControl::SetForegroundColour(colour) )
    {
        // nothing to do
        return false;
    }

    MakeOwnerDrawn();

    Refresh();

    return true;
}

bool wxAnyButton::MSWIsPushed() const
{
    return (::SendMessageW(GetHwnd(), BM_GETSTATE, 0, 0) & BST_PUSHED) != 0;
}

#ifdef __WXDEBUG__
static inline bool IsNonTransformedDC(WXHDC hdc)
{
    if ( ::GetGraphicsMode(hdc) == GM_ADVANCED )
        return false;

    SIZE devExt;
    ::GetViewportExtEx(hdc, &devExt);
    SIZE logExt;
    ::GetWindowExtEx(hdc, &logExt);
    return (devExt.cx == logExt.cx && devExt.cy == logExt.cy);
}
#endif // __WXDEBUG__

bool wxAnyButton::MSWOnDraw(WXDRAWITEMSTRUCT *wxdis)
{
    LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)wxdis;
    WXHDC hdc = lpDIS->hDC;
    // We expect here a DC with default settings (in GM_COMPATIBLE mode
    // with non-scaled coordinates system) but will check this because
    // our line drawing function wxDrawHVLine() works effectively only
    // on a non-transformed DC.
    wxASSERT(IsNonTransformedDC(hdc));

    WXUINT state = lpDIS->itemState;
    switch ( GetButtonState(this, state) )
    {
        case State_Disabled:
            state |= ODS_DISABLED;
            break;
        case State_Pressed:
            state |= ODS_SELECTED;
            break;
        case State_Focused:
            state |= ODS_FOCUS;
            break;
        default:
            break;
    }

    bool pushed = MSWIsPushed();

    RECT rectBtn;
    CopyRect(&rectBtn, &lpDIS->rcItem);

    // draw the button background
    if ( !HasFlag(wxBORDER_NONE) )
    {
#if wxUSE_UXTHEME
        if ( wxUxThemeIsActive() )
        {
            DrawXPBackground(this, hdc, rectBtn, state);
        }
        else
#endif // wxUSE_UXTHEME
        {
            COLORREF colBg = wxColourToRGB(GetBackgroundColour());

            // first, draw the background
            auto hbrushBackground = unique_brush(::CreateSolidBrush(colBg));
            ::FillRect(hdc, &rectBtn, hbrushBackground.get());

            // draw the border for the current state
            bool selected = (state & ODS_SELECTED) != 0;
            if ( !selected )
            {
                wxTopLevelWindow *
                    tlw = dynamic_cast<wxTopLevelWindow*>(wxGetTopLevelParent(this));
                if ( tlw )
                {
                    selected = tlw->GetDefaultItem() == this;
                }
            }

            DrawButtonFrame(hdc, rectBtn, selected, pushed);
        }

        // draw the focus rectangle if we need it
        if ( (state & ODS_FOCUS) && !(state & ODS_NOFOCUSRECT) )
        {
            DrawFocusRect(hdc, &rectBtn);

#if wxUSE_UXTHEME
            if ( !wxUxThemeIsActive() )
#endif // wxUSE_UXTHEME
            {
                if ( pushed )
                {
                    // the label is shifted by 1 pixel to create "pushed" effect
                    OffsetRect(&rectBtn, 1, 1);
                }
            }
        }
    }
    else
    {
        // clear the background (and erase any previous bitmap)
        COLORREF colBg = wxColourToRGB(GetBackgroundColour());
        auto hbrushBackground = unique_brush(::CreateSolidBrush(colBg));
        ::FillRect(hdc, &rectBtn, hbrushBackground.get());
    }

    // draw the image, if any
    if ( m_imageData )
    {
        wxBitmap bmp = m_imageData->GetBitmap(GetButtonState(this, state));
        if ( !bmp.IsOk() )
            bmp = m_imageData->GetBitmap(State_Normal);

        const wxSize sizeBmp = bmp.GetSize();
        const wxSize margin = m_imageData->GetBitmapMargins();
        const wxSize sizeBmpWithMargins(sizeBmp + 2*margin);
        wxRect rectButton(wxRectFromRECT(rectBtn));

        // for simplicity, we start with centred rectangle and then move it to
        // the appropriate edge
        wxRect rectBitmap = wxRect(sizeBmp).CentreIn(rectButton);

        // move bitmap only if we have a label, otherwise keep it centered
        if ( ShowsLabel() )
        {
            switch ( m_imageData->GetBitmapPosition() )
            {
                default:
                    wxFAIL_MSG( "invalid direction" );
                    [[fallthrough]];

                case wxDirection::wxLEFT:
                    rectBitmap.x = rectButton.x + margin.x;
                    rectButton.x += sizeBmpWithMargins.x;
                    rectButton.width -= sizeBmpWithMargins.x;
                    break;

                case wxDirection::wxRIGHT:
                    rectBitmap.x = rectButton.GetRight() - sizeBmp.x - margin.x;
                    rectButton.width -= sizeBmpWithMargins.x;
                    break;

                case wxDirection::wxTOP:
                    rectBitmap.y = rectButton.y + margin.y;
                    rectButton.y += sizeBmpWithMargins.y;
                    rectButton.height -= sizeBmpWithMargins.y;
                    break;

                case wxDirection::wxBOTTOM:
                    rectBitmap.y = rectButton.GetBottom() - sizeBmp.y - margin.y;
                    rectButton.height -= sizeBmpWithMargins.y;
                    break;
            }
        }

        wxDCTemp dst((WXHDC)hdc);
        dst.DrawBitmap(bmp, rectBitmap.GetPosition(), true);

        wxCopyRectToRECT(rectButton, rectBtn);
    }


    // finally draw the label
    if ( ShowsLabel() )
    {
        COLORREF colFg = state & ODS_DISABLED
                            ? ::GetSysColor(COLOR_GRAYTEXT)
                            : wxColourToRGB(GetForegroundColour());

        wxTextColoursChanger changeFg(hdc, colFg, CLR_INVALID);
        wxBkModeChanger changeBkMode(hdc, wxBrushStyle::Transparent);

#if wxUSE_MARKUP
        if ( m_markupText )
        {
            wxDCTemp dc((WXHDC)hdc);
            dc.SetTextForeground(wxColour(colFg));
            dc.SetFont(GetFont());

            m_markupText->Render(dc, wxRectFromRECT(rectBtn),
                                 state & ODS_NOACCEL
                                    ? wxMarkupText::Render_Default
                                    : wxMarkupText::Render_ShowAccels);
        }
        else // Plain text label
#endif // wxUSE_MARKUP
        {
            // notice that DT_HIDEPREFIX doesn't work on old (pre-Windows 2000)
            // systems but by happy coincidence ODS_NOACCEL is not used under
            // them neither so DT_HIDEPREFIX should never be used there
            DrawButtonText(hdc, &rectBtn, this,
                           state & ODS_NOACCEL ? DT_HIDEPREFIX : 0);
        }
    }

    return true;
}

#endif // wxHAS_ANY_BUTTON
