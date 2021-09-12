///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/richtooltip.cpp
// Purpose:     Native MSW implementation of wxRichToolTip.
// Author:      Vadim Zeitlin
// Created:     2011-10-18
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_RICHTOOLTIP

#ifndef WX_PRECOMP
    #include "wx/msw/private.h"

    #include <boost/nowide/stackstring.hpp>

    #include <memory>
#endif // WX_PRECOMP

#include "wx/treectrl.h"

#include "wx/private/richtooltip.h"
#include "wx/generic/private/richtooltip.h"
#include "wx/msw/uxtheme.h"


// Provide definitions missing from some compilers SDK headers.

#ifndef TTI_NONE
enum
{
    TTI_NONE,
    TTI_INFO,
    TTI_WARNING,
    TTI_ERROR
};
#endif // !defined(TTI_XXX)

#ifndef Edit_ShowBalloonTip
struct EDITBALLOONTIP
{
    DWORD cbStruct;
    LPCWSTR pszTitle;
    LPCWSTR pszText;
    int ttiIcon;
};

#define Edit_ShowBalloonTip(hwnd, pebt) \
    (BOOL)::SendMessageW((hwnd), 0x1503 /* EM_SHOWBALLOONTIP */, 0, (LPARAM)(pebt))

#endif // !defined(Edit_ShowBalloonTip)

// ============================================================================
// wxRichToolTipMSWImpl: the real implementation.
// ============================================================================

class wxRichToolTipMSWImpl : public wxRichToolTipGenericImpl
{
public:
    wxRichToolTipMSWImpl(const std::string& title, const std::string& message) :
        wxRichToolTipGenericImpl(title, message)
    {
        // So far so good...
        m_canUseNative = true;

        m_ttiIcon = TTI_NONE;
    }

    void SetBackgroundColour(const wxColour& col,
                                     const wxColour& colEnd) override
    {
        // Setting background colour is not supported neither.
        m_canUseNative = false;

        wxRichToolTipGenericImpl::SetBackgroundColour(col, colEnd);
    }

    void SetCustomIcon(const wxIcon& icon) override
    {
        // Custom icons are not supported by EM_SHOWBALLOONTIP.
        m_canUseNative = false;

        wxRichToolTipGenericImpl::SetCustomIcon(icon);
    }

    void SetStandardIcon(int icon) override
    {
        wxRichToolTipGenericImpl::SetStandardIcon(icon);
        if ( !m_canUseNative )
            return;

        switch ( icon & wxICON_MASK )
        {
            case wxICON_WARNING:
                m_ttiIcon = TTI_WARNING;
                break;

            case wxICON_ERROR:
                m_ttiIcon = TTI_ERROR;
                break;

            case wxICON_INFORMATION:
                m_ttiIcon = TTI_INFO;
                break;

            case wxICON_QUESTION:
                wxFAIL_MSG("Question icon doesn't make sense for a tooltip");
                break;

            case wxICON_NONE:
                m_ttiIcon = TTI_NONE;
                break;
        }
    }

    void SetTimeout(unsigned millisecondsTimeout,
                            unsigned millisecondsDelay) override
    {
        // We don't support changing the timeout or the delay
        // (maybe TTM_SETDELAYTIME could be used for this?).
        m_canUseNative = false;

        wxRichToolTipGenericImpl::SetTimeout(millisecondsTimeout,
                                             millisecondsDelay);
    }

    void SetTipKind(wxTipKind tipKind) override
    {
        // Setting non-default tip is not supported.
        if ( tipKind != wxTipKind::Auto )
            m_canUseNative = false;

        wxRichToolTipGenericImpl::SetTipKind(tipKind);
    }

    void SetTitleFont(const wxFont& font) override
    {
        // Setting non-default font is not supported.
        m_canUseNative = false;

        wxRichToolTipGenericImpl::SetTitleFont(font);
    }

    void ShowFor(wxWindow* win, const wxRect* rect) override
    {
        // TODO: We could use native tooltip control to show native balloon
        //       tooltips for any window but right now we use the simple
        //       EM_SHOWBALLOONTIP API which can only be used with text
        //       controls.
        if ( m_canUseNative && !rect )
        {
            wxTextCtrl* const text = wxDynamicCast(win, wxTextCtrl);
            if ( text )
            {
                boost::nowide::wstackstring stackTitle(m_title.c_str());
                boost::nowide::wstackstring stackMessage(m_message.c_str());

                EDITBALLOONTIP ebt
                {
                    .cbStruct = sizeof(EDITBALLOONTIP),
                    .pszTitle = stackTitle.get(),
                    .pszText = stackMessage.get(),
                    .ttiIcon = m_ttiIcon
                };

                if ( Edit_ShowBalloonTip(GetHwndOf(text), &ebt) )
                    return;
            }
        }

        // Don't set m_canUseNative to false here, we could be able to use the
        // native tooltips if we're called for a different window the next
        // time.
        wxRichToolTipGenericImpl::ShowFor(win, rect);
    }

private:
    // If this is false, we've been requested to do something that the native
    // version doesn't support and so need to fall back to the generic one.
    bool m_canUseNative;

    // One of TTI_NONE, TTI_INFO, TTI_WARNING or TTI_ERROR.
    int m_ttiIcon;
};

/* static */
std::unique_ptr<wxRichToolTipImpl>
wxRichToolTipImpl::Create(const std::string& title, const std::string& message)
{
    // EM_SHOWBALLOONTIP is only implemented by comctl32.dll v6 so don't even
    // bother using the native implementation if we're not using themes.
    if ( wxUxThemeIsActive() )
        return std::make_unique<wxRichToolTipMSWImpl>(title, message);

    return std::make_unique<wxRichToolTipGenericImpl>(title, message);
}

#endif // wxUSE_RICHTOOLTIP
