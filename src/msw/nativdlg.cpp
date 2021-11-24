/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/nativdlg.cpp
// Purpose:     Native dialog loading code (part of wxWindow)
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"

#include "wx/bmpbuttn.h"
#include "wx/button.h"
#include "wx/checkbox.h"
#include "wx/combobox.h"
#include "wx/listbox.h"
#include "wx/radiobut.h"
#include "wx/scrolbar.h"
#include "wx/slider.h"
#include "wx/spinbutt.h"
#include "wx/statbmp.h"
#include "wx/statbox.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"

import <string>;

// ---------------------------------------------------------------------------
// global functions
// ---------------------------------------------------------------------------

extern INT_PTR APIENTRY
wxDlgProc(WXHWND hWnd, WXUINT message, WXWPARAM wParam, WXLPARAM lParam);

// ===========================================================================
// implementation
// ===========================================================================

bool wxWindow::LoadNativeDialog(wxWindow* parent, wxWindowID id)
{
    m_windowId = id;

    wxWindowCreationHook hook(this);
    m_hWnd = (WXHWND)::CreateDialogW((WXHINSTANCE)wxGetInstance(),
                                    MAKEINTRESOURCEW(id),
                                    parent ? (WXHWND)parent->GetHWND() : nullptr,
                                    (DLGPROC) wxDlgProc);

    if ( !m_hWnd )
        return false;

    SubclassWin(GetHWND());

    if ( parent )
        parent->AddChild(this);
    else
        wxTopLevelWindows.Append(this);

    // Enumerate all children
    WXHWND hWndNext;
    hWndNext = ::GetWindow((WXHWND) m_hWnd, GW_CHILD);

    if (hWndNext)
        CreateWindowFromHWND(this, (WXHWND) hWndNext);

    while (hWndNext != (WXHWND) nullptr)
    {
        hWndNext = ::GetWindow(hWndNext, GW_HWNDNEXT);
        if (hWndNext)
            CreateWindowFromHWND(this, (WXHWND) hWndNext);
    }

    return true;
}

bool wxWindow::LoadNativeDialog(wxWindow* parent, const std::string& name)
{
    SetName(name);

    wxWindowCreationHook hook(this);
    m_hWnd = (WXHWND)::CreateDialogW((WXHINSTANCE) wxGetInstance(),
                                    boost::nowide::widen(name).c_str(),
                                    parent ? (WXHWND)parent->GetHWND() : nullptr,
                                    (DLGPROC)wxDlgProc);

    if ( !m_hWnd )
        return false;

    SubclassWin(GetHWND());

    if ( parent )
        parent->AddChild(this);
    else
        wxTopLevelWindows.Append(this);

    // Enumerate all children
    WXHWND hWndNext;
    hWndNext = ::GetWindow((WXHWND) m_hWnd, GW_CHILD);

    if (hWndNext)
        CreateWindowFromHWND(this, (WXHWND) hWndNext);

    while (hWndNext != (WXHWND) nullptr)
    {
        hWndNext = ::GetWindow(hWndNext, GW_HWNDNEXT);
        if (hWndNext)
            CreateWindowFromHWND(this, (WXHWND) hWndNext);
    }

    return true;
}

// ---------------------------------------------------------------------------
// look for child by id
// ---------------------------------------------------------------------------

wxWindow* wxWindow::GetWindowChild1(wxWindowID id)
{
    if ( m_windowId == id )
        return this;

    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while ( node )
    {
        wxWindow* child = node->GetData();
        wxWindow* win = child->GetWindowChild1(id);
        if ( win )
            return win;

        node = node->GetNext();
    }

    return nullptr;
}

wxWindow* wxWindow::GetWindowChild(wxWindowID id)
{
    wxWindow* win = GetWindowChild1(id);
    if ( !win )
    {
        WXHWND hwnd = ::GetDlgItem(GetHwnd(), id);
        if ( hwnd )
        {
            win = CreateWindowFromHWND(this, (WXHWND) hwnd);
        }
    }

    return win;
}

// ---------------------------------------------------------------------------
// create wxWin window from a native WXHWND
// ---------------------------------------------------------------------------

wxWindow* wxWindow::CreateWindowFromHWND(wxWindow* parent, WXHWND hWnd)
{
    wxCHECK_MSG( parent, nullptr, "must have valid parent for a control" );

    std::string str = wxGetWindowClass(hWnd);
    wx::utils::ToUpper(str);

    long id = wxGetWindowId(hWnd);
    unsigned int style = ::GetWindowLongPtrW((WXHWND) hWnd, GWL_STYLE);

    wxWindow* win = nullptr;

    if (str == "BUTTON")
    {
        int style1 = (style & 0xFF);
#if wxUSE_CHECKBOX
        if ((style1 == BS_3STATE) || (style1 == BS_AUTO3STATE) || (style1 == BS_AUTOCHECKBOX) ||
            (style1 == BS_CHECKBOX))
        {
            win = new wxCheckBox;
        }
        else
#endif
#if wxUSE_RADIOBTN
        if ((style1 == BS_AUTORADIOBUTTON) || (style1 == BS_RADIOBUTTON))
        {
            win = new wxRadioButton;
        }
        else
#endif
#if wxUSE_BMPBUTTON
        if (style & BS_BITMAP)
        {
            // TODO: how to find the bitmap?
            win = new wxBitmapButton;
            wxLogError("Have not yet implemented bitmap button as BS_BITMAP button.");
        }
        else
        if (style1 == BS_OWNERDRAW)
        {
            // TODO: how to find the bitmap?
            // TODO: can't distinguish between bitmap button and bitmap static.
            // Change implementation of wxStaticBitmap to SS_BITMAP.
            // PROBLEM: this assumes that we're using resource-based bitmaps.
            // So maybe need 2 implementations of bitmap buttons/static controls,
            // with a switch in the drawing code. Call default proc if BS_BITMAP.
            win = new wxBitmapButton;
        }
        else
#endif
#if wxUSE_BUTTON
        if ((style1 == BS_PUSHBUTTON) || (style1 == BS_DEFPUSHBUTTON))
        {
            win = new wxButton;
        }
        else
#endif
#if wxUSE_STATBOX
        if (style1 == BS_GROUPBOX)
        {
            win = new wxStaticBox;
        }
        else
#endif
        {
            wxLogError("Don't know what kind of button this is: id = %ld",
                       id);
        }
    }
#if wxUSE_COMBOBOX
    else if (str == "COMBOBOX")
    {
        win = new wxComboBox;
    }
#endif
#if wxUSE_TEXTCTRL
    // TODO: Problem if the user creates a multiline - but not rich text - text control,
    // since wxWin assumes RichEdit control for this. Should have m_isRichText in
    // wxTextCtrl. Also, convert as much of the window style as is necessary
    // for correct functioning.
    // Could have wxWindow::AdoptAttributesFromHWND(WXHWND)
    // to be overridden by each control class.
    else if (str == "EDIT")
    {
        win = new wxTextCtrl;
    }
#endif
#if wxUSE_LISTBOX
    else if (str == "LISTBOX")
    {
        win = new wxListBox;
    }
#endif
#if wxUSE_SCROLLBAR
    else if (str == "SCROLLBAR")
    {
        win = new wxScrollBar;
    }
#endif
#if wxUSE_SPINBTN
    else if (str == "MSCTLS_UPDOWN32")
    {
        win = new wxSpinButton;
    }
#endif
#if wxUSE_SLIDER
    else if (str == "MSCTLS_TRACKBAR32")
    {
        // Need to ascertain if it's horiz or vert
        win = new wxSlider;
    }
#endif // wxUSE_SLIDER
#if wxUSE_STATTEXT
    else if (str == "STATIC")
    {
        int style1 = (style & 0xFF);

        if ((style1 == SS_LEFT) || (style1 == SS_RIGHT)
            || (style1 == SS_SIMPLE)
            )
            win = new wxStaticText;
#if wxUSE_STATBMP
        else if (style1 == SS_BITMAP)
        {
            win = new wxStaticBitmap;

            // Help! this doesn't correspond with the wxWin implementation.
            wxLogError("Please make SS_BITMAP statics into owner-draw buttons.");
        }
#endif /* wxUSE_STATBMP */
    }
#endif
    else
    {
        std::string msg{ fmt::format("Don't know how to convert from Windows class {}", str) };
        wxLogError(msg.c_str());
    }

    if (win)
    {
        parent->AddChild(win);
        win->SubclassWin(hWnd);
        win->AdoptAttributesFromHWND();
    }

    return win;
}

// Make sure the window style (etc.) reflects the WXHWND style (roughly)
void wxWindow::AdoptAttributesFromHWND()
{
    SetId(wxGetWindowId(m_hWnd));

    unsigned int style = ::GetWindowLongPtrW(GetHwnd(), GWL_STYLE);

    if (style & WS_VSCROLL)
        m_windowStyle |= wxVSCROLL;
    if (style & WS_HSCROLL)
        m_windowStyle |= wxHSCROLL;
}
