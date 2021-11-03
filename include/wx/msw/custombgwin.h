///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/custombgwin.h
// Purpose:     wxMSW implementation of wxCustomBackgroundWindow
// Author:      Vadim Zeitlin
// Created:     2011-10-10
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_CUSTOMBGWIN_H_
#define _WX_MSW_CUSTOMBGWIN_H_

#include "wx/brush.h"

class wxBitmap;

template <class W>
class wxCustomBackgroundWindow : public W,
                                 public wxCustomBackgroundWindowBase
{
public:
    using BaseWindowClass = W;
protected:
    void DoSetBackgroundBitmap(const wxBitmap& bmp) override
    {
        m_backgroundBrush.reset();
        m_backgroundBrush = bmp.IsOk() ? std::make_unique<wxBrush>(bmp) : nullptr;

        // Our transparent children should use our background if we have it,
        // otherwise try to restore m_inheritBgCol to some reasonable value: true
        // if we also have non-default background colour or false otherwise.
        BaseWindowClass::m_inheritBgCol = bmp.IsOk()
                                            || BaseWindowClass::UseBgCol();
    }

    WXHBRUSH MSWGetCustomBgBrush() override
    {
        if ( m_backgroundBrush )
            return (WXHBRUSH)m_backgroundBrush->GetResourceHandle();

        return BaseWindowClass::MSWGetCustomBgBrush();
    }

    std::unique_ptr<wxBrush> m_backgroundBrush;
};

#endif // _WX_MSW_CUSTOMBGWIN_H_
