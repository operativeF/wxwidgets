///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/fontpickerg.cpp
// Purpose:     wxGenericFontButton class implementation
// Author:      Francesco Montorsi
// Modified by:
// Created:     15/04/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_FONTPICKERCTRL

#include "wx/fontpicker.h"

#include "wx/fontdlg.h"

#include <fmt/core.h>

// ----------------------------------------------------------------------------
// wxGenericFontButton
// ----------------------------------------------------------------------------

bool wxGenericFontButton::Create( wxWindow *parent, wxWindowID id,
                        const wxFont &initial, const wxPoint &pos,
                        const wxSize &size, unsigned int style,
                        const wxValidator& validator, std::string_view name)
{
    std::string label = (style & wxFNTP_FONTDESC_AS_LABEL) ?
                        std::string{} : // label will be updated by UpdateFont
                        _("Choose font");

    // create this button
    if (!wxButton::Create( parent, id, label, pos,
                           size, style, validator, name ))
    {
        wxFAIL_MSG( "wxGenericFontButton creation failed" );
        return false;
    }

    // and handle user clicks on it
    Bind(wxEVT_BUTTON, &wxGenericFontButton::OnButtonClick, this, GetId());

    InitFontData();

    m_selectedFont = initial.IsOk() ? initial : *wxNORMAL_FONT;
    UpdateFont();

    return true;
}

void wxGenericFontButton::InitFontData()
{
    m_data.SetAllowSymbols(true);
    m_data.SetColour(*wxBLACK);
    m_data.EnableEffects(true);
}

void wxGenericFontButton::OnButtonClick([[maybe_unused]] wxCommandEvent& ev)
{
    // update the wxFontData to be shown in the dialog
    m_data.SetInitialFont(m_selectedFont);

    // create the font dialog and display it
    wxFontDialog dlg(this, m_data);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_data = dlg.GetFontData();
        SetSelectedFont(m_data.GetChosenFont());

        // fire an event
        wxFontPickerEvent event(this, GetId(), m_selectedFont);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxGenericFontButton::UpdateFont()
{
    if ( !m_selectedFont.IsOk() )
        return;

    SetForegroundColour(m_data.GetColour());

    if (HasFlag(wxFNTP_USEFONT_FOR_LABEL))
    {
        // use currently selected font for the label...
        wxButton::SetFont(m_selectedFont);
    }

    if (HasFlag(wxFNTP_FONTDESC_AS_LABEL))
    {
        SetLabel(fmt::format("{:s}, {:d}",
                 m_selectedFont.GetFaceName().c_str(),
                 m_selectedFont.GetPointSize()));
    }
}

#endif // wxUSE_FONTPICKERCTRL
