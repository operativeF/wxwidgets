///////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/richtooltip.h
// Purpose:     wxRichToolTipImpl declaration.
// Author:      Vadim Zeitlin
// Created:     2011-10-18
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRIVATE_RICHTOOLTIP_H_
#define _WX_PRIVATE_RICHTOOLTIP_H_

#include "wx/richtooltip.h"

#include <chrono>
#include <memory>

class wxIcon;
class wxColour;

// ----------------------------------------------------------------------------
// wxRichToolTipImpl: defines wxRichToolTip implementation.
// ----------------------------------------------------------------------------

class wxRichToolTipImpl
{
public:
    // This is implemented in a platform-specific way.
    static std::unique_ptr<wxRichToolTipImpl> Create(const std::string& title,
                                     const std::string& message);

    // These methods simply mirror the public wxRichToolTip ones.
    virtual void SetBackgroundColour(const wxColour& col,
                                     const wxColour& colEnd) = 0;
    virtual void SetCustomIcon(const wxIcon& icon) = 0;
    virtual void SetStandardIcon(int icon) = 0;
    virtual void SetTimeout(std::chrono::milliseconds timeout,
                            std::chrono::milliseconds showdelay = {}) = 0;
    virtual void SetTipKind(wxTipKind tipKind) = 0;
    virtual void SetTitleFont(const wxFont& font) = 0;

    virtual void ShowFor(wxWindow* win, const wxRect* rect = nullptr) = 0;

    virtual ~wxRichToolTipImpl() = default;

protected:
    wxRichToolTipImpl() = default;
};

#endif // _WX_PRIVATE_RICHTOOLTIP_H_
