/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/filepickerg.h
// Purpose:     wxGenericFileDirButton, wxGenericFileButton, wxGenericDirButton
// Author:      Francesco Montorsi
// Modified by:
// Created:     14/4/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FILEDIRPICKER_H_
#define _WX_FILEDIRPICKER_H_

#include "wx/button.h"
#include "wx/filedlg.h"
#include "wx/dirdlg.h"


wxDECLARE_EVENT( wxEVT_DIRPICKER_CHANGED, wxFileDirPickerEvent );
wxDECLARE_EVENT( wxEVT_FILEPICKER_CHANGED, wxFileDirPickerEvent );


//-----------------------------------------------------------------------------
// wxGenericFileDirButton: a button which brings up a wx{File|Dir}Dialog
//-----------------------------------------------------------------------------

class wxGenericFileDirButton : public wxButton,
                                                public wxFileDirPickerWidgetBase
{
public:
    wxGenericFileDirButton() = default;

    wxGenericFileDirButton(wxWindow *parent,
                           wxWindowID id,
                           std::string_view label = wxFilePickerWidgetLabel,
                           const std::string& path = {},
                           std::string_view message = wxFileSelectorPromptStr,
                           const std::string& wildcard = wxFileSelectorDefaultWildcardStr,
                           const wxPoint& pos = wxDefaultPosition,
                           const wxSize& size = wxDefaultSize,
                           unsigned int style = 0,
                           const wxValidator& validator = {},
                           std::string_view name = wxFilePickerWidgetNameStr)
    {
        Create(parent, id, label, path, message, wildcard,
               pos, size, style, validator, name);
    }

    wxControl *AsControl() override { return this; }

public:     // overridable

    virtual wxDialog *wxCreateDialog() = 0;

    virtual wxWindow *GetDialogParent()
        { return GetParent(); }

    virtual wxEventType GetEventType() const = 0;

    void SetInitialDirectory(const std::string& dir) override;

public:

    bool Create(wxWindow *parent, wxWindowID id,
           std::string_view label = wxFilePickerWidgetLabel,
           const std::string& path = {},
           std::string_view message = wxFileSelectorPromptStr,
           const std::string &wildcard = wxFileSelectorDefaultWildcardStr,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize,
           unsigned int style = 0,
           const wxValidator& validator = {},
           std::string_view name = wxFilePickerWidgetNameStr);

    // event handler for the click
    void OnButtonClick(wxCommandEvent &);

protected:
    std::string m_message;
    std::string m_wildcard;

    // Initial directory set by SetInitialDirectory() call or empty.
    std::string m_initialDir;

    // we just store the style passed to the ctor here instead of passing it to
    // wxButton as some of our bits can conflict with wxButton styles and it
    // just doesn't make sense to use picker styles for wxButton anyhow
    long m_pickerStyle{-1};
};


//-----------------------------------------------------------------------------
// wxGenericFileButton: a button which brings up a wxFileDialog
//-----------------------------------------------------------------------------

#define wxFILEBTN_DEFAULT_STYLE                     (wxFLP_OPEN)

class wxGenericFileButton : public wxGenericFileDirButton
{
public:
    wxGenericFileButton() = default;
    wxGenericFileButton(wxWindow *parent,
                        wxWindowID id,
                        std::string_view label = wxFilePickerWidgetLabel,
                        const std::string& path = {},
                        std::string_view message = wxFileSelectorPromptStr,
                        const std::string &wildcard = wxFileSelectorDefaultWildcardStr,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        unsigned int style = wxFILEBTN_DEFAULT_STYLE,
                        const wxValidator& validator = {},
                        std::string_view name = wxFilePickerWidgetNameStr)
    {
        Create(parent, id, label, path, message, wildcard,
               pos, size, style, validator, name);
    }

public:     // overridable

    virtual unsigned int GetDialogStyle() const
    {
        // the derived class must initialize it if it doesn't use the
        // non-default wxGenericFileDirButton ctor
        wxASSERT_MSG( m_pickerStyle != -1,
                      "forgot to initialize m_pickerStyle?" );


        unsigned int filedlgstyle = 0;
        // FIXME: signed / unsigned bitwise
        if ( m_pickerStyle & wxFLP_OPEN )
            filedlgstyle |= wxFD_OPEN;
        if ( m_pickerStyle & wxFLP_SAVE )
            filedlgstyle |= wxFD_SAVE;
        if ( m_pickerStyle & wxFLP_OVERWRITE_PROMPT )
            filedlgstyle |= wxFD_OVERWRITE_PROMPT;
        if ( m_pickerStyle & wxFLP_FILE_MUST_EXIST )
            filedlgstyle |= wxFD_FILE_MUST_EXIST;
        if ( m_pickerStyle & wxFLP_CHANGE_DIR )
            filedlgstyle |= wxFD_CHANGE_DIR;

        return filedlgstyle;
    }

    wxDialog *wxCreateDialog() override;

    wxEventType GetEventType() const override
        { return wxEVT_FILEPICKER_CHANGED; }

protected:
    void UpdateDialogPath(wxDialog *p) override
        { dynamic_cast<wxFileDialog*>(p)->SetPath(m_path); }
    void UpdatePathFromDialog(wxDialog *p) override
        { m_path = dynamic_cast<wxFileDialog*>(p)->GetPath(); }
};


//-----------------------------------------------------------------------------
// wxGenericDirButton: a button which brings up a wxDirDialog
//-----------------------------------------------------------------------------

#define wxDIRBTN_DEFAULT_STYLE                     0

class wxGenericDirButton : public wxGenericFileDirButton
{
public:
    wxGenericDirButton() = default;
    wxGenericDirButton(wxWindow *parent,
                       wxWindowID id,
                       std::string_view label = wxDirPickerWidgetLabel,
                       const std::string& path = {},
                       std::string_view message = wxDirSelectorPromptStr,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       unsigned int style = wxDIRBTN_DEFAULT_STYLE,
                       const wxValidator& validator = {},
                       std::string_view name = wxDirPickerWidgetNameStr)
    {
        Create(parent, id, label, path, message, {},
               pos, size, style, validator, name);
    }

public:     // overridable

    virtual unsigned int GetDialogStyle() const
    {
        unsigned int dirdlgstyle = wxDD_DEFAULT_STYLE;

        if ( m_pickerStyle & wxDIRP_DIR_MUST_EXIST )
            dirdlgstyle |= wxDD_DIR_MUST_EXIST;
        if ( m_pickerStyle & wxDIRP_CHANGE_DIR )
            dirdlgstyle |= wxDD_CHANGE_DIR;

        return dirdlgstyle;
    }

    wxDialog *wxCreateDialog() override;

    wxEventType GetEventType() const override
        { return wxEVT_DIRPICKER_CHANGED; }

protected:
    void UpdateDialogPath(wxDialog *p) override
        { dynamic_cast<wxDirDialog*>(p)->SetPath(m_path); }
    void UpdatePathFromDialog(wxDialog *p) override
        { m_path = dynamic_cast<wxDirDialog*>(p)->GetPath(); }
};

#endif // _WX_FILEDIRPICKER_H_
