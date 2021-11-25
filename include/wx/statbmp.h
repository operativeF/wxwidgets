/////////////////////////////////////////////////////////////////////////////
// Name:        wx/statbmp.h
// Purpose:     wxStaticBitmap class interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     25.08.00
// Copyright:   (c) 2000 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_STATBMP_H_BASE_
#define _WX_STATBMP_H_BASE_

#if wxUSE_STATBMP

#include "wx/control.h"
#include "wx/icon.h"

import WX.Cfg.Flags;

class wxBitmap;

inline constexpr std::string_view wxStaticBitmapNameStr = "staticBitmap";

// a control showing an icon or a bitmap
class wxStaticBitmapBase : public wxControl
{
public:
    enum class ScaleMode
    {
        None,
        Fill,
        AspectFit,
        AspectFill
    };

    wxStaticBitmapBase& operator=(wxStaticBitmapBase&&) = delete;

    virtual void SetIcon(const wxIcon& icon) = 0;
    virtual void SetBitmap(const wxBitmap& bitmap) = 0;
    virtual wxBitmap GetBitmap() const = 0;
    virtual wxIcon GetIcon() const /* = 0 -- should be pure virtual */
    {
        // stub it out here for now as not all ports implement it (but they
        // should)
        return wxIcon();
    }
    virtual void SetScaleMode([[maybe_unused]] ScaleMode scaleMode) { }
    virtual ScaleMode GetScaleMode() const { return ScaleMode::None; }

    // overridden base class virtuals
    bool AcceptsFocus() const override { return false; }
    bool HasTransparentBackground() override { return true; }

protected:
    // choose the default border for this window
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    wxSize DoGetBestSize() const override;
};

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/statbmp.h"
#elif defined(__WXMSW__)
    #include "wx/msw/statbmp.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/statbmp.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/statbmp.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/statbmp.h"
#elif defined(__WXMAC__)
    #include "wx/osx/statbmp.h"
#elif defined(__WXQT__)
    #include "wx/qt/statbmp.h"
#endif

#endif // wxUSE_STATBMP

#endif
    // _WX_STATBMP_H_BASE_
