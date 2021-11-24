/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/textdlgg.h
// Purpose:     wxTextEntryDialog class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TEXTDLGG_H_
#define _WX_TEXTDLGG_H_

#if wxUSE_TEXTDLG

#include "wx/dialog.h"

#if wxUSE_VALIDATORS
#include "wx/valtext.h"
#include "wx/textctrl.h"
#endif

class wxTextCtrl;

inline constexpr std::string_view wxGetTextFromUserPromptStr = "Input Text";
inline constexpr std::string_view wxGetPasswordFromUserPromptStr = "Enter Password";

#define wxTextEntryDialogStyle (wxOK | wxCANCEL | wxCENTRE)

// ----------------------------------------------------------------------------
// wxTextEntryDialog: a dialog with text control, [ok] and [cancel] buttons
// ----------------------------------------------------------------------------

class wxTextEntryDialog : public wxDialog
{
public:
    wxTextEntryDialog()
    {
        m_textctrl = nullptr;
        m_dialogStyle = 0;
    }

    wxTextEntryDialog(wxWindow *parent,
                      std::string_view message,
                      std::string_view caption = wxGetTextFromUserPromptStr,
                      std::string_view value = {},
                      unsigned int style = wxTextEntryDialogStyle,
                      const wxPoint& pos = wxDefaultPosition)
    {
        Create(parent, message, caption, value, style, pos);
    }

    bool Create(wxWindow *parent,
                std::string_view message,
                std::string_view caption = wxGetTextFromUserPromptStr,
                std::string_view value = {},
                unsigned int style = wxTextEntryDialogStyle,
                const wxPoint& pos = wxDefaultPosition);

    void SetValue(const std::string& val);
    std::string GetValue() const { return m_value; }

    void SetMaxLength(unsigned long len);

    void ForceUpper();

#if wxUSE_VALIDATORS
    void SetTextValidator( const wxTextValidator& validator );
    void SetTextValidator( wxTextValidatorStyle style = wxFILTER_NONE );
    wxTextValidator* GetTextValidator() { return (wxTextValidator*)m_textctrl->GetValidator(); }
#endif // wxUSE_VALIDATORS

    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    // implementation only
    void OnOK(wxCommandEvent& event);

protected:
    wxTextCtrl *m_textctrl;
    std::string    m_value;
    long        m_dialogStyle;

private:
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxTextEntryDialog);
    wxTextEntryDialog(const wxTextEntryDialog&) = delete;
	wxTextEntryDialog& operator=(const wxTextEntryDialog&) = delete;
};

// ----------------------------------------------------------------------------
// wxPasswordEntryDialog: dialog with password control, [ok] and [cancel]
// ----------------------------------------------------------------------------

class wxPasswordEntryDialog : public wxTextEntryDialog
{
public:
    wxPasswordEntryDialog() = default;
    wxPasswordEntryDialog(wxWindow *parent,
                      std::string_view message,
                      std::string_view caption = wxGetPasswordFromUserPromptStr,
                      std::string_view value = {},
                      unsigned int style = wxTextEntryDialogStyle,
                      const wxPoint& pos = wxDefaultPosition)
    {
        Create(parent, message, caption, value, style, pos);
    }

    bool Create(wxWindow *parent,
                std::string_view message,
                std::string_view caption = wxGetPasswordFromUserPromptStr,
                std::string_view value = {},
                unsigned int style = wxTextEntryDialogStyle,
                const wxPoint& pos = wxDefaultPosition);


private:
    wxDECLARE_DYNAMIC_CLASS(wxPasswordEntryDialog);
    wxPasswordEntryDialog(const wxPasswordEntryDialog&) = delete;
	wxPasswordEntryDialog& operator=(const wxPasswordEntryDialog&) = delete;
};

// ----------------------------------------------------------------------------
// function to get a string from user
// ----------------------------------------------------------------------------

std::string
    wxGetTextFromUser(std::string_view message,
                      std::string_view caption = wxGetTextFromUserPromptStr,
                      std::string_view default_value = {},
                      wxWindow *parent = nullptr,
                      wxCoord x = wxDefaultCoord,
                      wxCoord y = wxDefaultCoord,
                      bool centre = true);

std::string
    wxGetPasswordFromUser(std::string_view message,
                          std::string_view caption = wxGetPasswordFromUserPromptStr,
                          std::string_view default_value = {},
                          wxWindow *parent = nullptr,
                          wxCoord x = wxDefaultCoord,
                          wxCoord y = wxDefaultCoord,
                          bool centre = true);

#endif
    // wxUSE_TEXTDLG
#endif // _WX_TEXTDLGG_H_
