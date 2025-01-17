///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fontpickercmn.cpp
// Purpose:     wxFontPickerCtrl class implementation
// Author:      Francesco Montorsi
// Modified by:
// Created:     15/04/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_FONTPICKERCTRL

#include "wx/fontpicker.h"
#include "wx/textctrl.h"
#include "wx/fontenum.h"

#include <fmt/core.h>

import Utils.Strings;

import <charconv>;

// ============================================================================
// implementation
// ============================================================================

#if defined(__WXGTK20__) && !defined(__WXUNIVERSAL__)
    #define SetMinMaxPointSize(min, max)
#else
    #define SetMinMaxPointSize(min, max)  GetPickerWidget()->GetFontData()->SetRange((min), (max))
#endif

wxDEFINE_EVENT(wxEVT_FONTPICKER_CHANGED, wxFontPickerEvent);

// ----------------------------------------------------------------------------
// wxFontPickerCtrl
// ----------------------------------------------------------------------------

bool wxFontPickerCtrl::Create( wxWindow *parent,
                               wxWindowID id,
                               const wxFont &initial,
                               const wxPoint &pos,
                               const wxSize &size,
                               unsigned int style,
                               const wxValidator& validator,
                               std::string_view name )
{
    if (!wxPickerBase::CreateBase(parent, id,
                                  Font2String(initial.IsOk() ? initial
                                                             : *wxNORMAL_FONT),
                                  pos, size, style, validator, name))
        return false;

    // the picker of a wxFontPickerCtrl is a wxFontPickerWidget
    m_picker = new wxFontPickerWidget(this, wxID_ANY, initial,
                                      wxDefaultPosition, wxDefaultSize,
                                      GetPickerStyle(style));
    // complete sizer creation
    wxPickerBase::PostCreation();

    m_picker->Bind(wxEVT_FONTPICKER_CHANGED, &wxFontPickerCtrl::OnFontChange, this);

    return true;
}

std::string wxFontPickerCtrl::Font2String(const wxFont &f)
{
    std::string ret = f.GetNativeFontInfoUserDesc();
#ifdef __WXMSW__
    // on wxMSW the encoding of the font is appended at the end of the string;
    // since encoding is not very user-friendly we remove it.
    const wxFontEncoding enc = f.GetEncoding();
    if ( enc != wxFONTENCODING_DEFAULT && enc != wxFONTENCODING_SYSTEM )
        ret = wx::utils::BeforeLast(ret, ' ');
#endif
    return ret;
}

wxFont wxFontPickerCtrl::String2Font(const std::string &s)
{
    std::string str{s};
    wxFont ret;
    double n;

    // put a limit on the maximum point size which the user can enter
    // NOTE: we suppose the last word of given string is the pointsize
    std::string strSize = wx::utils::AfterLast(str, ' ');
    auto [p, ec] = std::from_chars(strSize.data(), strSize.data() + strSize.size(), n);
    if (ec == std::errc())
    {
        if (n < 1)
            str = str.substr(0, str.length() - strSize.length()) + "1";
        else if (n >= m_nMaxPointSize)
            str = str.substr(0, str.length() - strSize.length()) +
                  fmt::format("{:d}", m_nMaxPointSize);
    }

    if (!ret.SetNativeFontInfoUserDesc(str))
        return wxNullFont;

    return ret;
}

void wxFontPickerCtrl::SetSelectedFont(const wxFont &f)
{
    GetPickerWidget()->SetSelectedFont(f);
    UpdateTextCtrlFromPicker();
}

void wxFontPickerCtrl::UpdatePickerFromTextCtrl()
{
    wxASSERT(m_text);

    // NB: we don't use the wxFont::wxFont(const std::string&) constructor
    //     since that constructor expects the native font description
    //     string returned by wxFont::GetNativeFontInfoDesc() and not
    //     the user-friendly one returned by wxFont::GetNativeFontInfoUserDesc()
    wxFont f = String2Font(m_text->GetValue());
    if (!f.IsOk())
        return;     // invalid user input

    if (GetPickerWidget()->GetSelectedFont() != f)
    {
        GetPickerWidget()->SetSelectedFont(f);

        // fire an event
        wxFontPickerEvent event(this, GetId(), f);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxFontPickerCtrl::UpdateTextCtrlFromPicker()
{
    if (!m_text)
        return;     // no textctrl to update

    // Take care to use ChangeValue() here and not SetValue() to avoid
    // infinite recursion.
    m_text->ChangeValue(Font2String(GetPickerWidget()->GetSelectedFont()));
}

void wxFontPickerCtrl::SetMinPointSize(unsigned int min)
{
    m_nMinPointSize = min;
    SetMinMaxPointSize(m_nMinPointSize, m_nMaxPointSize);
}

void wxFontPickerCtrl::SetMaxPointSize(unsigned int max)
{
    m_nMaxPointSize = max;
    SetMinMaxPointSize(m_nMinPointSize, m_nMaxPointSize);
}

// ----------------------------------------------------------------------------
// wxFontPickerCtrl - event handlers
// ----------------------------------------------------------------------------

void wxFontPickerCtrl::OnFontChange(wxFontPickerEvent &ev)
{
    UpdateTextCtrlFromPicker();

    // the wxFontPickerWidget sent us a colour-change notification.
    // forward this event to our parent
    wxFontPickerEvent event(this, GetId(), ev.GetFont());
    GetEventHandler()->ProcessEvent(event);
}

#endif  // wxUSE_FONTPICKERCTRL
