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

#include "wx/defs.h"

#if wxUSE_TEXTDLG

#include "wx/dialog.h"

#if wxUSE_VALIDATORS
#include "wx/valtext.h"
#include "wx/textctrl.h"
#endif

class WXDLLIMPEXP_FWD_CORE wxTextCtrl;

constexpr char wxGetTextFromUserPromptStr[] = "Input Text";
constexpr char wxGetPasswordFromUserPromptStr[] = "Enter Password";

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
                      const std::string& message,
                      const std::string& caption = wxGetTextFromUserPromptStr,
                      const std::string& value = {},
                      unsigned int style = wxTextEntryDialogStyle,
                      const wxPoint& pos = wxDefaultPosition)
    {
        Create(parent, message, caption, value, style, pos);
    }

    bool Create(wxWindow *parent,
                const std::string& message,
                const std::string& caption = wxGetTextFromUserPromptStr,
                const std::string& value = {},
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
                      const std::string& message,
                      const std::string& caption = wxGetPasswordFromUserPromptStr,
                      const std::string& value = {},
                      unsigned int style = wxTextEntryDialogStyle,
                      const wxPoint& pos = wxDefaultPosition)
    {
        Create(parent, message, caption, value, style, pos);
    }

    bool Create(wxWindow *parent,
                const std::string& message,
                const std::string& caption = wxGetPasswordFromUserPromptStr,
                const std::string& value = {},
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
    wxGetTextFromUser(const std::string& message,
                    const std::string& caption = wxGetTextFromUserPromptStr,
                    const std::string& default_value = {},
                    wxWindow *parent = nullptr,
                    wxCoord x = wxDefaultCoord,
                    wxCoord y = wxDefaultCoord,
                    bool centre = true);

std::string
    wxGetPasswordFromUser(const std::string& message,
                        const std::string& caption = wxGetPasswordFromUserPromptStr,
                        const std::string& default_value = {},
                        wxWindow *parent = nullptr,
                        wxCoord x = wxDefaultCoord,
                        wxCoord y = wxDefaultCoord,
                        bool centre = true);

#endif
    // wxUSE_TEXTDLG
#endif // _WX_TEXTDLGG_H_
