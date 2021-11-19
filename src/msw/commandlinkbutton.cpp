/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/commandlinkbutton.cpp
// Purpose:     wxCommandLinkButton
// Author:      Rickard Westerlund
// Created:     2010-06-14
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_COMMANDLINKBUTTON

#include "wx/dcclient.h"
#include "wx/commandlinkbutton.h"
#include "wx/msw/private/button.h"
#include "wx/msw/private/dc.h"
#include "wx/private/window.h"
#include "wx/msw/private.h"

#include <boost/nowide/convert.hpp>

#ifndef BCM_SETNOTE
constexpr unsigned int BCM_SETNOTE = 0x1609;
#endif

#ifndef BS_COMMANDLINK
constexpr unsigned int BS_COMMANDLINK = 0xE;
#endif

// ----------------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------------

namespace
{

bool HasNativeCommandLinkButton()
{
    return wxGetWinVersion() >= wxWinVersion_6;
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// Command link button
// ----------------------------------------------------------------------------

bool wxCommandLinkButton::Create(wxWindow *parent,
                                 wxWindowID id,
                                 const std::string& mainLabel,
                                 const std::string& note,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 unsigned int style,
                                 const wxValidator& validator,
                                 std::string_view name)
{
    if ( ! wxGenericCommandLinkButton::Create(parent,
                                              id,
                                              mainLabel,
                                              note,
                                              pos,
                                              size,
                                              style,
                                              validator,
                                              name) )
        return false;

    SetMainLabelAndNote(mainLabel, note);
    SetInitialSize(size);

    return true;
}

void
wxCommandLinkButton::SetMainLabelAndNote(const std::string& mainLabel,
                                         const std::string& note)
{
    if ( HasNativeCommandLinkButton() )
    {
        wxButton::SetLabel(mainLabel);
        ::SendMessageW(m_hWnd, BCM_SETNOTE, 0, reinterpret_cast<LPARAM>(boost::nowide::widen(note).c_str()));

        // Preserve the user-specified label for GetLabel()
        m_labelOrig = mainLabel;
        // FIXME: use fmt lib
        if ( !note.empty() )
            m_labelOrig += '\n' + note;
    }
    else
    {
        wxGenericCommandLinkButton::SetMainLabelAndNote(mainLabel, note);
    }
}

DWORD wxCommandLinkButton::MSWGetStyle(unsigned int style, DWORD *exstyle) const
{
    DWORD ret = wxButton::MSWGetStyle(style, exstyle);
    if ( HasNativeCommandLinkButton() )
        ret |= BS_COMMANDLINK;

    return ret;
}

bool wxCommandLinkButton::HasNativeBitmap() const
{
    return HasNativeCommandLinkButton();
}

// ----------------------------------------------------------------------------
// size management including autosizing
// ----------------------------------------------------------------------------

// Margin measures can be found at
// http://expression.microsoft.com/en-us/ee662150.aspx
constexpr int MAINLABEL_TOP_MARGIN = 16; // Includes image top margin.
constexpr int MAINLABEL_NOTE_LEFT_MARGIN = 23;
constexpr int NOTE_TOP_MARGIN = 21;
constexpr int NOTE_BOTTOM_MARGIN = 1;
constexpr int MAINLABEL_NOTE_MARGIN = NOTE_TOP_MARGIN - MAINLABEL_TOP_MARGIN;

wxSize wxCommandLinkButton::DoGetBestSize() const
{
    wxSize size;

    // account for the text part if we have it or if we don't have any image at
    // all (buttons initially created with empty label should still have a non
    // zero size)
    if ( ShowsLabel() || !m_imageData )
    {
        unsigned int flags{};
        if ( GetAuthNeeded() )
            flags |= wxMSWButton::Size_AuthNeeded;

        wxCommandLinkButton *thisButton =
            const_cast<wxCommandLinkButton *>(this);
        wxClientDC dc(thisButton);

        wxFont noteFont = dc.GetFont();

        // 4/3 is the relationship between main label and note font sizes.
        dc.SetFont(noteFont.Scaled(4.0f/3.0f));
        size = dc.GetMultiLineTextExtent(GetLabelText(GetMainLabel()));

        dc.SetFont(noteFont);
        wxSize noteSize = dc.GetMultiLineTextExtent(GetLabelText(GetNote()));

        if ( noteSize.x > size.x )
            size.x = noteSize.x;
        size.y += noteSize.y;

        size = wxMSWButton::GetFittingSize(thisButton,
                                           size,
                                           flags);

        // The height of a standard command link button is 25 and 35 DLUs for
        // single lines and two lines respectively. Width is not accounted for.
        int heightDef = GetNote().AfterFirst('\n').empty() ? 25 : 35;
        wxSize sizeDef = thisButton->ConvertDialogToPixels(wxSize(50,
                                                                  heightDef));

        if ( size.y < sizeDef.y )
            size.y = sizeDef.y;
    }

    if ( m_imageData )
    {
        AdjustForBitmapSize(size);
    }
    else
    {
        // The default image size is 16x16.
        size.x += 16;
        if ( size.y < 16 )
            size.y = 16;
    }

    size.x += MAINLABEL_NOTE_LEFT_MARGIN;
    size.y += MAINLABEL_TOP_MARGIN + NOTE_BOTTOM_MARGIN;
    if ( !GetNote().empty() )
        size.y += MAINLABEL_NOTE_MARGIN;

    return size;
}

#endif // wxUSE_COMMANDLINKBUTTON
