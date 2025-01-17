///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/custombgwin.h
// Purpose:     Generic implementation of wxCustomBackgroundWindow.
// Author:      Vadim Zeitlin
// Created:     2011-10-10
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_CUSTOMBGWIN_H_
#define _WX_GENERIC_CUSTOMBGWIN_H_

#include "wx/bitmap.h"
#include "wx/dc.h"
#include "wx/event.h"

class wxWindow;

// A helper to avoid template bloat: this class contains all type-independent
// code of wxCustomBackgroundWindow<> below.
class wxCustomBackgroundWindowGenericBase : public wxCustomBackgroundWindowBase
{
public:
    wxCustomBackgroundWindowGenericBase& operator=(wxCustomBackgroundWindowGenericBase&&) = delete;

protected:
    void DoEraseBackground(wxEraseEvent& event, wxWindow* win)
    {
        wxDC& dc = *event.GetDC();

        const wxSize clientSize = win->GetClientSize();
        const wxSize bitmapSize = m_bitmapBg.GetSize();

        for ( int x = 0; x < clientSize.x; x += bitmapSize.x )
        {
            for ( int y = 0; y < clientSize.y; y += bitmapSize.y )
            {
                dc.DrawBitmap(m_bitmapBg, x, y);
            }
        }
    }


    // The bitmap used for painting the background if valid.
    wxBitmap m_bitmapBg;
};

// ----------------------------------------------------------------------------
// wxCustomBackgroundWindow
// ----------------------------------------------------------------------------

template <class W>
class wxCustomBackgroundWindow : public W,
                                 public wxCustomBackgroundWindowGenericBase
{
public:
    using BaseWindowClass = W;

    wxCustomBackgroundWindow& operator=(wxCustomBackgroundWindow<W>&&) = delete;

protected:
    void DoSetBackgroundBitmap(const wxBitmap& bmp) override
    {
        m_bitmapBg = bmp;

        if ( m_bitmapBg.IsOk() )
        {
            BaseWindowClass::Bind
            (
                wxEVT_ERASE_BACKGROUND,
                &wxCustomBackgroundWindow::OnEraseBackground, this
            );
        }
        else
        {
            BaseWindowClass::Unbind
            (
                wxEVT_ERASE_BACKGROUND,
                &wxCustomBackgroundWindow::OnEraseBackground, this
            );
        }
    }

private:
    // Event handler for erasing the background which is only used when we have
    // a valid background bitmap.
    void OnEraseBackground(wxEraseEvent& event)
    {
        DoEraseBackground(event, this);
    }
};

#endif // _WX_GENERIC_CUSTOMBGWIN_H_
