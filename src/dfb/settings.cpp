/////////////////////////////////////////////////////////////////////////////
// Name:        src/dfb/settings.cpp
// Purpose:     wxSystemSettings implementation
// Author:      Vaclav Slavik
// Created:     2006-08-08
// Copyright:   (c) 2006 REA Elektronik GmbH
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#include "wx/settings.h"

#ifndef WX_PRECOMP
    #include "wx/colour.h"
    #include "wx/font.h"
    #include "wx/gdicmn.h"
#endif


wxColour wxSystemSettingsNative::GetColour([[maybe_unused]] wxSystemColour index)
{
    // overridden by wxSystemSettings::GetColour in wxUniversal
    return wxColour(0,0,0);
}

wxFont wxSystemSettingsNative::GetFont(wxSystemFont index)
{
    switch (index)
    {
        case wxSYS_OEM_FIXED_FONT:
        case wxSYS_ANSI_FIXED_FONT:
        case wxSYS_SYSTEM_FIXED_FONT:
        {
            // FIXME_DFB
            return wxFont(12,
                          wxFontFamily::Teletype,
                          wxFontStyle::Normal,
                          wxFONTWEIGHT_NORMAL);
        }

        case wxSYS_ANSI_VAR_FONT:
        case wxSYS_SYSTEM_FONT:
        case wxSYS_DEVICE_DEFAULT_FONT:
        case wxSYS_DEFAULT_GUI_FONT:
        {
            // FIXME_DFB
            return wxFont(12,
                          wxFontFamily::Default,
                          wxFontStyle::Normal,
                          wxFONTWEIGHT_NORMAL);
        }

        default:
            wxFAIL_MSG( "unknown font type" );
            return wxNullFont;
    }
}

int wxSystemSettingsNative::GetMetric(wxSystemMetric index,
                                      [[maybe_unused]] const wxWindow* win)
{
    switch (index)
    {
        case wxSYS_SCREEN_X:
            return wxDisplay().GetGeometry().GetWidth();
        case wxSYS_SCREEN_Y:
            return wxDisplay().GetGeometry().GetHeight();

        default:
            return -1;
    }
}

bool wxSystemSettingsNative::HasFeature(wxSystemFeature index)
{
    switch (index)
    {
        case wxSYS_CAN_ICONIZE_FRAME:
        case wxSYS_CAN_DRAW_FRAME_DECORATIONS:
        case wxSYS_TABLET_PRESENT:
            return false;

        default:
            wxFAIL_MSG( "unknown feature" );
            return false;
    }
}
