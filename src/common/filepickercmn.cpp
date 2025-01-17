///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/filepickercmn.cpp
// Purpose:     wxFilePickerCtrl class implementation
// Author:      Francesco Montorsi (readapted code written by Vadim Zeitlin)
// Modified by:
// Created:     15/04/2006
// Copyright:   (c) Vadim Zeitlin, Francesco Montorsi
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILEPICKERCTRL || wxUSE_DIRPICKERCTRL

#include "wx/filepicker.h"
#include "wx/textctrl.h"

import WX.File.Filename;

// ============================================================================
// implementation
// ============================================================================

wxDEFINE_EVENT( wxEVT_FILEPICKER_CHANGED, wxFileDirPickerEvent );
wxDEFINE_EVENT( wxEVT_DIRPICKER_CHANGED,  wxFileDirPickerEvent );

// ----------------------------------------------------------------------------
// wxFileDirPickerCtrlBase
// ----------------------------------------------------------------------------

bool wxFileDirPickerCtrlBase::CreateBase(wxWindow *parent,
                                         wxWindowID id,
                                         const std::string& path,
                                         std::string_view message,
                                         const std::string& wildcard,
                                         const wxPoint &pos,
                                         const wxSize &size,
                                         unsigned int style,
                                         const wxValidator& validator,
                                         std::string_view name )
{
    if (!wxPickerBase::CreateBase(parent, id, path, pos, size,
                                   style, validator, name))
        return false;

    if (!HasFlag(wxFLP_OPEN) && !HasFlag(wxFLP_SAVE))
        m_windowStyle |= wxFLP_OPEN;     // wxFD_OPEN is the default

    // check that the styles are not contradictory
    wxASSERT_MSG( !(HasFlag(wxFLP_SAVE) && HasFlag(wxFLP_OPEN)),
                  "can't specify both wxFLP_SAVE and wxFLP_OPEN at once" );

    wxASSERT_MSG( !HasFlag(wxFLP_SAVE) || !HasFlag(wxFLP_FILE_MUST_EXIST),
                   "wxFLP_FILE_MUST_EXIST can't be used with wxFLP_SAVE" );

    wxASSERT_MSG( !HasFlag(wxFLP_OPEN) || !HasFlag(wxFLP_OVERWRITE_PROMPT),
                  "wxFLP_OVERWRITE_PROMPT can't be used with wxFLP_OPEN" );

    // create a wxFilePickerWidget or a wxDirPickerWidget...
    m_pickerIface = CreatePicker(this, path, message, wildcard);
    if ( !m_pickerIface )
        return false;
    m_picker = m_pickerIface->AsControl();

    // complete sizer creation
    wxPickerBase::PostCreation();

    DoConnect( m_picker, this );

    // default's wxPickerBase textctrl limit is too small for this control:
    // make it bigger
    if (m_text) m_text->SetMaxLength(512);

    return true;
}

std::string wxFileDirPickerCtrlBase::GetPath() const
{
    return m_pickerIface->GetPath();
}

void wxFileDirPickerCtrlBase::SetPath(const std::string &path)
{
    m_pickerIface->SetPath(path);
    UpdateTextCtrlFromPicker();
}

void wxFileDirPickerCtrlBase::UpdatePickerFromTextCtrl()
{
    wxASSERT(m_text);

    // remove the eventually present path-separator from the end of the textctrl
    // string otherwise we would generate a wxFileDirPickerEvent when changing
    // from e.g. /home/user to /home/user/ and we want to avoid it !
    std::string newpath(GetTextCtrlValue());

    // Notice that we use to check here whether the current path is valid, i.e.
    // if the corresponding file or directory exists for the controls with
    // wxFLP_FILE_MUST_EXIST or wxDIRP_DIR_MUST_EXIST flag, however we don't do
    // this any more as we still must notify the program about any changes in
    // the control, otherwise its view of it would be different from what is
    // actually shown on the screen, resulting in very confusing UI.

    if (m_pickerIface->GetPath() != newpath)
    {
        m_pickerIface->SetPath(newpath);

        // update current working directory, if necessary
        // NOTE: the path separator is required because if newpath is "C:"
        //       then no change would happen
        if (IsCwdToUpdate())
            wxSetWorkingDirectory(newpath);

        // fire an event
        wxFileDirPickerEvent event(GetEventType(), this, GetId(), newpath);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxFileDirPickerCtrlBase::UpdateTextCtrlFromPicker()
{
    if (!m_text)
        return;     // no textctrl to update

    // Take care to use ChangeValue() here and not SetValue() to avoid
    // generating an event that would trigger UpdateTextCtrlFromPicker()
    // resulting in infinite recursion.
    m_text->ChangeValue(m_pickerIface->GetPath());
}



// ----------------------------------------------------------------------------
// wxFileDirPickerCtrlBase - event handlers
// ----------------------------------------------------------------------------

void wxFileDirPickerCtrlBase::OnFileDirChange(wxFileDirPickerEvent &ev)
{
    UpdateTextCtrlFromPicker();

    // the wxFilePickerWidget sent us a colour-change notification.
    // forward this event to our parent
    wxFileDirPickerEvent event(GetEventType(), this, GetId(), ev.GetPath());
    GetEventHandler()->ProcessEvent(event);
}

#endif  // wxUSE_FILEPICKERCTRL || wxUSE_DIRPICKERCTRL

// ----------------------------------------------------------------------------
// wxFileDirPickerCtrl
// ----------------------------------------------------------------------------

#if wxUSE_FILEPICKERCTRL

bool wxFilePickerCtrl::Create(wxWindow *parent,
                              wxWindowID id,
                              const std::string& path,
                              std::string_view message,
                              const std::string& wildcard,
                              const wxPoint& pos,
                              const wxSize& size,
                              unsigned int style,
                              const wxValidator& validator,
                              std::string_view name)
{
    if ( !wxFileDirPickerCtrlBase::CreateBase
                                   (
                                        parent, id, path, message, wildcard,
                                        pos, size, style, validator, name
                                   ) )
        return false;

    if ( HasTextCtrl() )
        GetTextCtrl()->AutoCompleteFileNames();

    return true;
}

std::string wxFilePickerCtrl::GetTextCtrlValue() const
{
    wxCHECK_MSG( m_text, std::string(), "Can't be used if no text control" );

    // filter it through wxFileName to remove any spurious path separator
    return wxFileName(m_text->GetValue()).GetFullPath();
}

#endif // wxUSE_FILEPICKERCTRL

// ----------------------------------------------------------------------------
// wxDirPickerCtrl
// ----------------------------------------------------------------------------

#if wxUSE_DIRPICKERCTRL

bool wxDirPickerCtrl::Create(wxWindow *parent,
                             wxWindowID id,
                             const std::string& path,
                             std::string_view message,
                             const wxPoint& pos,
                             const wxSize& size,
                             unsigned int style,
                             const wxValidator& validator,
                             std::string_view name)
{
    if ( !wxFileDirPickerCtrlBase::CreateBase
                                   (
                                        parent, id, path, message, "",
                                        pos, size, style, validator, name
                                   ) )
        return false;

    if ( HasTextCtrl() )
        GetTextCtrl()->AutoCompleteDirectories();

    return true;
}

std::string wxDirPickerCtrl::GetTextCtrlValue() const
{
    wxCHECK_MSG( m_text, std::string(), "Can't be used if no text control" );

    // filter it through wxFileName to remove any spurious path separator
    return wxFileName::DirName(m_text->GetValue()).GetPath();
}

#endif // wxUSE_DIRPICKERCTRL
