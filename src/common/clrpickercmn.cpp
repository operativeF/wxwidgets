///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/clrpickercmn.cpp
// Purpose:     wxColourPickerCtrl class implementation
// Author:      Francesco Montorsi (readapted code written by Vadim Zeitlin)
// Modified by:
// Created:     15/04/2006
// Copyright:   (c) Vadim Zeitlin, Francesco Montorsi
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_COLOURPICKERCTRL

#include "wx/clrpicker.h"
#include "wx/textctrl.h"

// ============================================================================
// implementation
// ============================================================================

wxDEFINE_EVENT(wxEVT_COLOURPICKER_CHANGED, wxColourPickerEvent);
wxDEFINE_EVENT(wxEVT_COLOURPICKER_CURRENT_CHANGED, wxColourPickerEvent);
wxDEFINE_EVENT(wxEVT_COLOURPICKER_DIALOG_CANCELLED, wxColourPickerEvent);

// ----------------------------------------------------------------------------
// wxColourPickerCtrl
// ----------------------------------------------------------------------------

#define M_PICKER     ((wxColourPickerWidget*)m_picker)

bool wxColourPickerCtrl::Create( wxWindow *parent,
                        wxWindowID id,
                        const wxColour &col,
                        const wxPoint &pos,
                        const wxSize &size,
                        unsigned int style,
                        const wxValidator& validator,
                        std::string_view name )
{
    if (!wxPickerBase::CreateBase(parent, id, col.GetAsString(), pos, size,
                                  style, validator, name))
        return false;

    // we are not interested to the ID of our picker as we connect
    // to its "changed" event dynamically...
    m_picker = new wxColourPickerWidget(this, wxID_ANY, col,
                                        wxDefaultPosition, wxDefaultSize,
                                        GetPickerStyle(style));

    // complete sizer creation
    wxPickerBase::PostCreation();

    m_picker->Bind(wxEVT_COLOURPICKER_CHANGED,
            &wxColourPickerCtrl::OnColourChange, this);

    return true;
}

void wxColourPickerCtrl::SetColour(const wxColour &col)
{
    M_PICKER->SetColour(col);
    UpdateTextCtrlFromPicker();
}

bool wxColourPickerCtrl::SetColour(const std::string& text)
{
    wxColour col{text};
    if ( !col.IsOk() )
        return false;
    M_PICKER->SetColour(col);
    UpdateTextCtrlFromPicker();

    return true;
}

void wxColourPickerCtrl::UpdatePickerFromTextCtrl()
{
    wxASSERT(m_text);

    // std::string -> wxColour conversion
    wxColour col = m_text->GetValue();
    if ( !col.IsOk() )
        return;     // invalid user input

    if (M_PICKER->GetColour() != col)
    {
        M_PICKER->SetColour(col);

        // fire an event
        wxColourPickerEvent event(this, GetId(), col);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxColourPickerCtrl::UpdateTextCtrlFromPicker()
{
    if (!m_text)
        return;     // no textctrl to update

    // Take care to use ChangeValue() here and not SetValue() to avoid
    // infinite recursion.
    m_text->ChangeValue(M_PICKER->GetColour().GetAsString());
}



// ----------------------------------------------------------------------------
// wxColourPickerCtrl - event handlers
// ----------------------------------------------------------------------------

void wxColourPickerCtrl::OnColourChange(wxColourPickerEvent &ev)
{
    UpdateTextCtrlFromPicker();

    ev.Skip();
}

#endif  // wxUSE_COLOURPICKERCTRL
