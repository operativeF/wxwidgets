///////////////////////////////////////////////////////////////////////////////
// Name:        wx/tipwin.h
// Purpose:     wxTipWindow is a window like the one typically used for
//              showing the tooltips
// Author:      Vadim Zeitlin
// Modified by:
// Created:     10.09.00
// Copyright:   (c) 2000 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/popupwin.h"

export module WX.Core.TipWin;

import Utils.Geometry;

import <string>;


#if wxUSE_TIPWINDOW

export
{

class wxTipWindowView;

// ----------------------------------------------------------------------------
// wxTipWindow
// ----------------------------------------------------------------------------

class wxTipWindow : public wxPopupTransientWindow
{
public:
    // the mandatory ctor parameters are: the parent window and the text to
    // show
    //
    // optionally you may also specify the length at which the lines are going
    // to be broken in rows (100 pixels by default)
    //
    // windowPtr and rectBound are just passed to SetTipWindowPtr() and
    // SetBoundingRect() - see below
    wxTipWindow(wxWindow *parent,
                const std::string& text,
                wxCoord maxLength = 100,
                wxTipWindow** windowPtr = nullptr,
                wxRect *rectBound = nullptr);

    ~wxTipWindow();

    wxTipWindow& operator=(wxTipWindow&&) = delete;

    // If windowPtr is not NULL the given address will be NULLed when the
    // window has closed
    void SetTipWindowPtr(wxTipWindow** windowPtr) { m_windowPtr = windowPtr; }

    // If rectBound is not NULL, the window will disappear automatically when
    // the mouse leave the specified rect: note that rectBound should be in the
    // screen coordinates!
    void SetBoundingRect(const wxRect& rectBound);

    // Hide and destroy the window
    void Close();

protected:
    // called by wxTipWindowView only
    bool CheckMouseInBounds(const wxPoint& pos);

    // event handlers
    void OnMouseClick(wxMouseEvent& event);

    void OnDismiss() override;

private:
    wxTipWindowView *m_view;

    wxTipWindow** m_windowPtr;
    wxRect m_rectBound;

    wxDECLARE_EVENT_TABLE();

    friend class wxTipWindowView;
};

} // export

#endif // wxUSE_TIPWINDOW
