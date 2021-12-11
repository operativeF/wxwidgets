///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/ownerdrwcmn.cpp
// Purpose:     wxOwnerDrawn class methods common to all platforms
// Author:      Marcin Malich
// Modified by:
// Created:     2009-09-22
// Copyright:   (c) 2009 Marcin Malich <me@malcom.pl>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_OWNER_DRAWN

#include "wx/ownerdrw.h"
#include "wx/font.h"
#include "wx/colour.h"
#include "wx/dcmemory.h"
#include "wx/utils.h"

import WX.Utils.Settings;

// ----------------------------------------------------------------------------
// constants for base class
// ----------------------------------------------------------------------------

// ============================================================================
// implementation
// ============================================================================

bool wxOwnerDrawnBase::OnMeasureItem(size_t *width, size_t *height)
{
    if ( IsOwnerDrawn() )
    {
        wxMemoryDC dc;
        wxFont font;
        GetFontToUse(font);
        dc.SetFont(font);

        // item name/text without mnemonics
        std::string name = wxStripMenuCodes(GetName(), wxStrip_Mnemonics);

        auto textExtents = dc.GetTextExtent(name);

        *width = textExtents.x + m_margin;
        *height = textExtents.y;
    }
    else
    {
        *width = 0;
        *height = 0;
    }

    return true;
}

void wxOwnerDrawnBase::GetFontToUse(wxFont& font) const
{
    font = m_font.IsOk() ? m_font : *wxNORMAL_FONT;
}

void wxOwnerDrawnBase::GetColourToUse(wxODStatus stat, wxColour& colText, wxColour& colBack) const
{
    if ( stat & wxODSelected )
    {
        colText = wxSystemSettings::GetColour(
                !(stat & wxODDisabled) ? wxSYS_COLOUR_HIGHLIGHTTEXT
                                       : wxSYS_COLOUR_GRAYTEXT);

        colBack = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    }
    else
    {
        // fall back to default colors if none explicitly specified

        if ( stat & wxODDisabled )
        {
            colText = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
        }
        else
        {
            colText = m_colText.IsOk() ? m_colText
                                     : wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT);
        }

        colBack = m_colBack.IsOk() ? m_colBack
                                 : wxSystemSettings::GetColour(wxSYS_COLOUR_MENU);
    }
}

#endif // wxUSE_OWNER_DRAWN
