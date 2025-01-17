/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/valtext.cpp
// Purpose:     wxTextValidator
// Author:      Julian Smart
// Modified by: Francesco Montorsi
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_VALIDATORS && (wxUSE_TEXTCTRL || wxUSE_COMBOBOX)

#include "wx/valtext.h"
#include "wx/combo.h"
#include "wx/combobox.h"
#include "wx/intl.h"
#include "wx/msgdlg.h"
#include "wx/textctrl.h"
#include "wx/utils.h"

import <cctype>;
import <cstdio>;
import <cstdlib>;


// ----------------------------------------------------------------------------
// global helpers
// ----------------------------------------------------------------------------

static bool wxIsNumeric(const wxString& val)
{
    for ( wxString::const_iterator i = val.begin(); i != val.end(); ++i )
    {
        // Allow for "," (French) as well as "." -- in future we should
        // use wxSystemSettings or other to do better localisation
        if ((!wxIsdigit(*i)) &&
            (*i != wxS('.')) && (*i != wxS(',')) && (*i != wxS('e')) &&
            (*i != wxS('E')) && (*i != wxS('+')) && (*i != wxS('-')))
            return false;
    }
    return true;
}

// ----------------------------------------------------------------------------
// wxTextValidator
// ----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxTextValidator, wxValidator);
wxBEGIN_EVENT_TABLE(wxTextValidator, wxValidator)
    EVT_CHAR(wxTextValidator::OnChar)
wxEND_EVENT_TABLE()

wxTextValidator::wxTextValidator(unsigned int style, wxString *val)
    : m_stringValue(val)
{
    SetStyle(style);
}

wxTextValidator::wxTextValidator(const wxTextValidator& val)
     
{
    Copy(val);
}

void wxTextValidator::SetStyle(unsigned int style)
{
    m_validatorStyle = style;
}

bool wxTextValidator::Copy(const wxTextValidator& val)
{
    wxValidator::Copy(val);

    m_validatorStyle = val.m_validatorStyle;
    m_stringValue    = val.m_stringValue;

    m_charIncludes = val.m_charIncludes;
    m_charExcludes = val.m_charExcludes;
    m_includes     = val.m_includes;
    m_excludes     = val.m_excludes;

    return true;
}

wxTextEntry *wxTextValidator::GetTextEntry()
{
#if wxUSE_TEXTCTRL
    if (dynamic_cast<wxTextCtrl*>(m_validatorWindow))
    {
        return (wxTextCtrl*)m_validatorWindow;
    }
#endif

#if wxUSE_COMBOBOX
    if (dynamic_cast<wxComboBox*>(m_validatorWindow))
    {
        return (wxComboBox*)m_validatorWindow;
    }
#endif

#if wxUSE_COMBOCTRL
    if (dynamic_cast<wxComboCtrl*>(m_validatorWindow))
    {
        return (wxComboCtrl*)m_validatorWindow;
    }
#endif

    wxFAIL_MSG(
        "wxTextValidator can only be used with wxTextCtrl, wxComboBox, "
        "or wxComboCtrl"
    );

    return nullptr;
}

// Called when the value in the window must be validated.
// This function can pop up an error message.
bool wxTextValidator::Validate(wxWindow *parent)
{
    // If window is disabled, simply return
    if ( !m_validatorWindow->IsEnabled() )
        return true;

    wxTextEntry * const text = GetTextEntry();
    if ( !text )
        return false;

    const wxString& errormsg = IsValid(text->GetValue());

    if ( !errormsg.empty() )
    {
        m_validatorWindow->SetFocus();
        wxMessageBox(errormsg.ToStdString(), _("Validation conflict"),
                     wxOK | wxICON_EXCLAMATION, parent);

        return false;
    }

    return true;
}

// Called to transfer data to the window
bool wxTextValidator::TransferToWindow()
{
    if ( m_stringValue )
    {
        wxTextEntry * const text = GetTextEntry();
        if ( !text )
            return false;

        text->SetValue(m_stringValue->ToStdString());
    }

    return true;
}

// Called to transfer data to the window
bool wxTextValidator::TransferFromWindow()
{
    if ( m_stringValue )
    {
        wxTextEntry * const text = GetTextEntry();
        if ( !text )
            return false;

        *m_stringValue = text->GetValue();
    }

    return true;
}

std::string wxTextValidator::IsValid(std::string_view str) const
{
    if ( HasFlag(wxFILTER_EMPTY) && str.empty() )
        return _("Required information entry is empty.");
    else if ( IsExcluded(std::string{str.begin(), str.end()}) )
        return fmt::format(fmt::runtime(_("'%s' is one of the invalid strings")), str);
    else if ( !IsIncluded(std::string{str.begin(), str.end()}) )
        return fmt::format(fmt::runtime(_("'%s' is not one of the valid strings")), str);

    // check the whole string for invalid chars.
    for ( auto i = str.begin(), end = str.end(); i != end; ++i )
    {
        if ( !IsValidChar(*i) )
        {
            return fmt::format(fmt::runtime(_("'{:s}' contains invalid character(s)")), str);
        }
    }

    return {};
}


void wxTextValidator::SetCharIncludes(const wxString& chars)
{
    m_charIncludes.clear();

    AddCharIncludes(chars);
}

void wxTextValidator::AddCharIncludes(const wxString& chars)
{
    m_charIncludes += chars;
}

void wxTextValidator::SetCharExcludes(const wxString& chars)
{
    m_charExcludes.clear();

    AddCharExcludes(chars);
}

void wxTextValidator::AddCharExcludes(const wxString& chars)
{
    m_charExcludes += chars;
}

// TODO: Use a span.
void wxTextValidator::SetIncludes(const std::vector<wxString>& includes)
{
    // preserve compatibily with versions prior 3.1.3 which used m_includes
    // to store the list of char includes.
    if ( HasFlag(wxFILTER_INCLUDE_CHAR_LIST) )
    {
        for ( const auto& i : includes )
        {
            AddCharIncludes(i);
        }

        return;
    }

    m_includes = includes;
}

void wxTextValidator::AddInclude(const wxString& include)
{
    m_includes.push_back(include);
}

// TODO: Use a span
void wxTextValidator::SetExcludes(const std::vector<wxString>& excludes)
{
    // preserve compatibily with versions prior 3.1.3 which used m_excludes
    // to store the list of char excludes.
    if ( HasFlag(wxFILTER_EXCLUDE_CHAR_LIST) )
    {
        for ( const auto& i : excludes )
        {
            AddCharExcludes(i);
        }

        return;
    }

    m_excludes = excludes;
}

void wxTextValidator::AddExclude(const wxString& exclude)
{
    m_excludes.push_back(exclude);
}

void wxTextValidator::OnChar(wxKeyEvent& event)
{
    // Let the event propagate by default.
    event.Skip();

    if (!m_validatorWindow)
        return;

    // We only filter normal, printable characters.
    int keyCode = event.GetUnicodeKey();

    // we don't filter special keys and delete
    if (keyCode < WXK_SPACE || keyCode == WXK_DELETE)
        return;

    // Filter out invalid characters
    if ( IsValidChar(static_cast<wxUniChar>(keyCode)) )
        return;

    if ( !wxValidator::IsSilent() )
        wxBell();

    // eat message
    event.Skip(false);
}

bool wxTextValidator::IsValidChar(const wxUniChar& c) const
{
    if ( !m_validatorStyle ) // no filtering if HasFlag(wxFILTER_NONE)
        return true;

    if ( IsCharExcluded(c) ) // disallow any char in the m_charExcludes.
        return false;

    if ( IsCharIncluded(c) ) // allow any char in the m_charIncludes.
        return true;

    if ( !HasFlag(wxFILTER_CC) )
    {
        // Validity is entirely determined by the exclude/include char lists
        // and this character is in neither, so consider that it is valid if
        // and only if we accept anything.
        return !HasFlag(wxFILTER_INCLUDE_CHAR_LIST);
    }

    if ( HasFlag(wxFILTER_SPACE) && wxIsspace(c) )
        return true;
    if ( HasFlag(wxFILTER_ASCII) && c.IsAscii() )
        return true;
    if ( HasFlag(wxFILTER_NUMERIC) && wxIsNumeric(c) )
        return true;
    if ( HasFlag(wxFILTER_ALPHANUMERIC) && wxIsalnum(c) )
        return true;
    if ( HasFlag(wxFILTER_ALPHA) && wxIsalpha(c) )
        return true;
    if ( HasFlag(wxFILTER_DIGITS) && wxIsdigit(c) )
        return true;
    if ( HasFlag(wxFILTER_XDIGITS) && wxIsxdigit(c) )
        return true;

    // If we are here, this means that the char c does not belong to any of the
    // character classes checked above (e.g. emoji chars) so just return false.

    return false;
}

// kept for compatibility reasons.
bool wxTextValidator::ContainsOnlyIncludedCharacters(const wxString& str) const
{
    for ( wxString::const_iterator i = str.begin(), end = str.end();
          i != end; ++i )
    {
        if ( !IsCharIncluded(*i) )
            return false;
    }

    return true;
}

// kept for compatibility reasons.
bool wxTextValidator::ContainsExcludedCharacters(const wxString& str) const
{
    for ( wxString::const_iterator i = str.begin(), end = str.end();
          i != end; ++i )
    {
        if ( IsCharExcluded(*i) )
            return true;
    }

    return false;
}

#endif
  // wxUSE_VALIDATORS && (wxUSE_TEXTCTRL || wxUSE_COMBOBOX)
