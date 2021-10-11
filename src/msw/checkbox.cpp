/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/checkbox.cpp
// Purpose:     wxCheckBox
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_CHECKBOX

#ifndef WX_PRECOMP
    #include <cassert>
    
#endif

#include "wx/checkbox.h"
#include "wx/dcclient.h"
#include "wx/settings.h"
#include "wx/renderer.h"
#include "wx/msw/uxtheme.h"
#include "wx/msw/private/button.h"
#include "wx/private/window.h"

bool wxCheckBox::Create(wxWindow *parent,
                        wxWindowID id,
                        const std::string& label,
                        const wxPoint& pos,
                        const wxSize& size, unsigned int style,
                        const wxValidator& validator,
                        const std::string& name)
{
    m_state = wxCheckBoxState::Unchecked;

    WXValidateStyle(&style);
    if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

    DWORD exstyle;
    DWORD msStyle = MSWGetStyle(style, &exstyle);

    msStyle |= wxMSWButton::GetMultilineStyle(label);

    return MSWCreateControl("BUTTON", msStyle, pos, size, label, exstyle);
}

DWORD wxCheckBox::MSWGetStyle(unsigned int style, DWORD *exstyle) const
{
    // buttons never have an external border, they draw their own one
    DWORD msStyle = wxControl::MSWGetStyle(style, exstyle);

    if ( style & wxCHK_3STATE )
        msStyle |= BS_3STATE;
    else
        msStyle |= BS_CHECKBOX;

    if ( style & wxALIGN_RIGHT )
    {
        msStyle |= BS_LEFTTEXT | BS_RIGHT;
    }

    return msStyle;
}

// ----------------------------------------------------------------------------
// wxCheckBox geometry
// ----------------------------------------------------------------------------

wxSize wxCheckBox::DoGetBestClientSize() const
{
    static wxPrivate::DpiDependentValue<wxCoord> s_checkSize;

    if ( s_checkSize.HasChanged(this) )
    {
        wxClientDC dc(const_cast<wxCheckBox*>(this));
        dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

        s_checkSize.SetAtNewDPI(dc.GetCharHeight());
    }

    wxCoord& checkSize = s_checkSize.Get();
    std::string str = wxGetWindowText(GetHWND());

    int wCheckbox, hCheckbox;
    if ( !str.empty() )
    {
        wxClientDC dc(const_cast<wxCheckBox *>(this));
        dc.SetFont(GetFont());
        dc.GetMultiLineTextExtent(GetLabelText(str), &wCheckbox, &hCheckbox);
        wCheckbox += checkSize + wxGetCharWidth();

        if ( ::GetWindowLongPtrW(GetHwnd(), GWL_STYLE) & BS_MULTILINE )
        {
            // We need to make the checkbox even wider in this case because
            // otherwise it wraps lines automatically and not only on "\n"s as
            // we need and this makes the size computed here wrong resulting in
            // checkbox contents being truncated when it's actually displayed.
            // Without this hack simple checkbox with "Some thing\n and more"
            // label appears on 3 lines, not 2, under Windows 2003 using
            // classic look and feel (although it works fine under Windows 7,
            // with or without themes).
            wCheckbox += checkSize;
        }

        if ( hCheckbox < checkSize )
            hCheckbox = checkSize;
    }
    else
    {
        wCheckbox = checkSize;
        hCheckbox = checkSize;
    }

    return {wCheckbox, hCheckbox};
}

// ----------------------------------------------------------------------------
// wxCheckBox operations
// ----------------------------------------------------------------------------

void wxCheckBox::SetLabel(const std::string& label)
{
    wxMSWButton::UpdateMultilineStyle(GetHwnd(), label);

    wxCheckBoxBase::SetLabel(label);
}

void wxCheckBox::SetValue(bool val)
{
    Set3StateValue(val ? wxCheckBoxState::Checked : wxCheckBoxState::Unchecked);
}

bool wxCheckBox::GetValue() const
{
    return Get3StateValue() != wxCheckBoxState::Unchecked;
}

void wxCheckBox::Command(wxCommandEvent& event)
{
    auto state = static_cast<wxCheckBoxState>(event.GetInt());
    wxCHECK_RET( (state == wxCheckBoxState::Unchecked) || (state == wxCheckBoxState::Checked)
        || (state == wxCheckBoxState::Indeterminate),
        wxT("event.GetInt() returned an invalid checkbox state") );

    Set3StateValue(state);
    ProcessCommand(event);
}

void wxCheckBox::DoSet3StateValue(wxCheckBoxState state)
{
    m_state = state;
    if ( !IsOwnerDrawn() )
        ::SendMessageW(GetHwnd(), BM_SETCHECK, (WPARAM) state, 0);
    else // owner drawn buttons don't react to this message
        Refresh();
}

wxCheckBoxState wxCheckBox::DoGet3StateValue() const
{
    return m_state;
}

bool wxCheckBox::MSWCommand(WXUINT cmd, WXWORD WXUNUSED(id))
{
    if ( cmd != BN_CLICKED && cmd != BN_DBLCLK )
        return false;

    // first update the value so that user event handler gets the new checkbox
    // value

    // ownerdrawn buttons don't manage their state themselves unlike usual
    // auto checkboxes so do it ourselves in any case
    wxCheckBoxState state;
    if ( Is3rdStateAllowedForUser() )
    {
        state = (wxCheckBoxState)((static_cast<int>(m_state) + 1) % 3);
    }
    else // 2 state checkbox (at least from users point of view)
    {
        // note that wxCheckBoxState::Indeterminate also becomes unchecked when clicked
        state = m_state == wxCheckBoxState::Unchecked ? wxCheckBoxState::Checked : wxCheckBoxState::Unchecked;
    }

    DoSet3StateValue(state);


    // generate the event
    wxCommandEvent event(wxEVT_CHECKBOX, m_windowId);

    event.SetInt(static_cast<int>(state));
    event.SetEventObject(this);
    ProcessCommand(event);

    return true;
}

// ----------------------------------------------------------------------------
// owner drawn checkboxes support
// ----------------------------------------------------------------------------

int wxCheckBox::MSWGetButtonStyle() const
{
    return HasFlag(wxCHK_3STATE) ? BS_3STATE : BS_CHECKBOX;
}

void wxCheckBox::MSWOnButtonResetOwnerDrawn()
{
    // ensure that controls state is consistent with internal state
    DoSet3StateValue(m_state);
}

unsigned int wxCheckBox::MSWGetButtonCheckedFlag() const
{
    switch ( Get3StateValue() )
    {
        case wxCheckBoxState::Checked:
            return wxCONTROL_CHECKED;

        case wxCheckBoxState::Indeterminate:
            return wxCONTROL_UNDETERMINED;

        case wxCheckBoxState::Unchecked:
            // no extra styles needed
            return wxCONTROL_NONE;
    }

    wxFAIL_MSG( wxT("unexpected Get3StateValue() return value") );

    return 0;
}

void wxCheckBox::MSWDrawButtonBitmap(wxDC& dc, const wxRect& rect, unsigned int flags)
{
    wxRendererNative::Get().DrawCheckBox(this, dc, rect, flags);
}

#endif // wxUSE_CHECKBOX
