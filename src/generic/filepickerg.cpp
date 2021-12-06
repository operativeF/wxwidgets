///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/filepickerg.cpp
// Purpose:     wxGenericFileDirButton class implementation
// Author:      Francesco Montorsi
// Modified by:
// Created:     15/04/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILEPICKERCTRL || wxUSE_DIRPICKERCTRL

#include "wx/filename.h"
#include "wx/filepicker.h"

// ----------------------------------------------------------------------------
// wxGenericFileButton
// ----------------------------------------------------------------------------

bool wxGenericFileDirButton::Create(wxWindow *parent,
                                    wxWindowID id,
                                    std::string_view label,
                                    const std::string& path,
                                    std::string_view message,
                                    const std::string& wildcard,
                                    const wxPoint& pos,
                                    const wxSize& size,
                                    unsigned int style,
                                    const wxValidator& validator,
                                    std::string_view name)
{
    m_pickerStyle = style;

    // If the special wxPB_SMALL flag is used, ignore the provided label and
    // use the shortest possible label and the smallest possible button fitting
    // it.
    long styleButton = 0;
    std::string labelButton;
    if ( m_pickerStyle & wxPB_SMALL )
    {
        labelButton = _("...").ToStdString();
        styleButton = wxBU_EXACTFIT;
    }
    else
    {
        labelButton = label;
    }

    // create this button
    if ( !wxButton::Create(parent, id, labelButton,
                           pos, size, styleButton, validator, name) )
    {
        wxFAIL_MSG( "wxGenericFileButton creation failed" );
        return false;
    }

    // and handle user clicks on it
    Bind(wxEVT_BUTTON, &wxGenericFileDirButton::OnButtonClick, this, GetId());

    // create the dialog associated with this button
    m_path = path;
    m_message = message;
    m_wildcard = wildcard;

    return true;
}

void wxGenericFileDirButton::OnButtonClick([[maybe_unused]] wxCommandEvent& ev)
{
    std::unique_ptr<wxDialog> p(wxCreateDialog());
    if (p->ShowModal() == wxID_OK)
    {
        // save updated path in m_path
        UpdatePathFromDialog(p.get());

        // fire an event
        wxFileDirPickerEvent event(GetEventType(), this, GetId(), m_path);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxGenericFileDirButton::SetInitialDirectory(const std::string& dir)
{
    m_initialDir = dir;
}

// ----------------------------------------------------------------------------
// wxGenericFileButton
// ----------------------------------------------------------------------------

wxDialog *wxGenericFileButton::wxCreateDialog()
{
    // Determine the initial directory for the dialog: it comes either from the
    // default path, if it has it, or from the separately specified initial
    // directory that can be set even if the path is e.g. empty.
    wxFileName fn(m_path);
    std::string initialDir = fn.GetPath();
    if ( initialDir.empty() )
        initialDir = m_initialDir;

    return new wxFileDialog
               (
                    GetDialogParent(),
                    m_message,
                    initialDir,
                    fn.GetFullName(),
                    m_wildcard,
                    GetDialogStyle()
               );
}

// ----------------------------------------------------------------------------
// wxGenericDirButton
// ----------------------------------------------------------------------------

wxDialog *wxGenericDirButton::wxCreateDialog()
{
    wxDirDialog* const dialog = new wxDirDialog
                                    (
                                        GetDialogParent(),
                                        m_message,
                                        m_path.empty() ? m_initialDir : m_path,
                                        GetDialogStyle()
                                    );
    return dialog;
}

#endif      // wxUSE_FILEPICKERCTRL || wxUSE_DIRPICKERCTRL
