/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/bmpbtncmn.cpp
// Purpose:     wxBitmapButton common code
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_BMPBUTTON

#include "wx/bmpbuttn.h"
#include "wx/log.h"
#include "wx/dcmemory.h"
#include "wx/artprov.h"
#include "wx/renderer.h"

namespace
{

#ifdef wxHAS_DRAW_TITLE_BAR_BITMAP

wxBitmap
GetCloseButtonBitmap(wxWindow *win,
                     wxSize sz,
                     const wxColour& colBg,
                     int flags = 0)
{
    // size is physical here because it comes from wxArtProvider::GetSizeHint
    wxBitmap bmp;
    bmp.Create(sz, wxBITMAP_SCREEN_DEPTH);
    wxMemoryDC dc(bmp);
    dc.SetBackground(colBg);
    dc.Clear();
    wxRendererNative::Get().DrawTitleBarBitmap(win, dc, sz, wxTitleBarButton::Close, flags);
    return bmp;
}

#endif // wxHAS_DRAW_TITLE_BAR_BITMAP

} // anonymous namespace

bool
wxBitmapButton::CreateCloseButton(wxWindow* parent,
                                  wxWindowID winid,
                                  std::string_view name)
{
    wxCHECK_MSG( parent, false, "Must have a valid parent" );

    const wxColour colBg = parent->GetBackgroundColour();

#ifdef wxHAS_DRAW_TITLE_BAR_BITMAP
    const wxSize sizeBmp = wxArtProvider::GetSizeHint(wxART_BUTTON);
    wxBitmap bmp = GetCloseButtonBitmap(parent, sizeBmp, colBg);
#else // !wxHAS_DRAW_TITLE_BAR_BITMAP
    wxBitmap bmp = wxArtProvider::GetBitmap(wxART_CLOSE, wxART_BUTTON);
#endif // wxHAS_DRAW_TITLE_BAR_BITMAP

    if ( !Create(parent, winid, bmp,
                 wxDefaultPosition, wxDefaultSize,
                 wxBORDER_NONE, wxValidator{}, name) )
        return false;

#ifdef wxHAS_DRAW_TITLE_BAR_BITMAP
    SetBitmapPressed(
        GetCloseButtonBitmap(parent, sizeBmp, colBg, wxCONTROL_PRESSED));

    SetBitmapCurrent(
        GetCloseButtonBitmap(parent, sizeBmp, colBg, wxCONTROL_CURRENT));
#endif // wxHAS_DRAW_TITLE_BAR_BITMAP

    // The button should blend with its parent background.
    SetBackgroundColour(colBg);

    return true;
}

/* static */
wxBitmapButton*
wxBitmapButtonBase::NewCloseButton(wxWindow* parent,
                                   wxWindowID winid,
                                   std::string_view name)
{
    wxBitmapButton* const button = new wxBitmapButton();

    button->CreateCloseButton(parent, winid, name);

    return button;
}

#endif // wxUSE_BMPBUTTON
