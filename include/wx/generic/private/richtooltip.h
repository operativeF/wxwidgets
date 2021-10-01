///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/private/richtooltip.h
// Purpose:     wxRichToolTipGenericImpl declaration.
// Author:      Vadim Zeitlin
// Created:     2011-10-18
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_PRIVATE_RICHTOOLTIP_H_
#define _WX_GENERIC_PRIVATE_RICHTOOLTIP_H_

#include "wx/icon.h"
#include "wx/colour.h"

#include <chrono>

// ----------------------------------------------------------------------------
// wxRichToolTipGenericImpl: defines generic wxRichToolTip implementation.
// ----------------------------------------------------------------------------

using namespace std::chrono_literals;

class wxRichToolTipGenericImpl : public wxRichToolTipImpl
{
public:
    wxRichToolTipGenericImpl(const std::string& title, const std::string& message) :
        m_title(title),
        m_message(message)
    {
        m_tipKind = wxTipKind::Auto;

        // This is pretty arbitrary, we could follow MSW and use some multiple
        // of double-click time here.
        m_timeout = 5000ms;
        m_delay = 0ms;
    }

    void SetBackgroundColour(const wxColour& col,
                                     const wxColour& colEnd) override;
    void SetCustomIcon(const wxIcon& icon) override;
    void SetStandardIcon(int icon) override;
    void SetTimeout(std::chrono::milliseconds timeout,
                    std::chrono::milliseconds delay = 0ms) override;
    void SetTipKind(wxTipKind tipKind) override;
    void SetTitleFont(const wxFont& font) override;

    void ShowFor(wxWindow* win, const wxRect* rect = nullptr) override;

protected:
    std::string m_title;
    std::string m_message;

private:
    wxColour m_colStart,
             m_colEnd;
             
    wxIcon m_icon;
    wxFont m_titleFont;

    std::chrono::milliseconds m_timeout;
    std::chrono::milliseconds m_delay;

    wxTipKind m_tipKind;
};

#endif // _WX_GENERIC_PRIVATE_RICHTOOLTIP_H_
