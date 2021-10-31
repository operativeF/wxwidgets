///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/filectrlcmn.cpp
// Purpose:     Implementation for wxFileCtrlBase and other common functions used by
//              platform-specific wxFileCtrl's
// Author:      Diaa M. Sami
// Created:     2007-07-07
// Copyright:   (c) Diaa M. Sami
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILECTRL

#include "wx/filectrl.h"
#include "wx/debug.h"

wxDEFINE_EVENT( wxEVT_FILECTRL_SELECTIONCHANGED, wxFileCtrlEvent );
wxDEFINE_EVENT( wxEVT_FILECTRL_FILEACTIVATED, wxFileCtrlEvent );
wxDEFINE_EVENT( wxEVT_FILECTRL_FOLDERCHANGED, wxFileCtrlEvent );
wxDEFINE_EVENT( wxEVT_FILECTRL_FILTERCHANGED, wxFileCtrlEvent );

wxIMPLEMENT_DYNAMIC_CLASS( wxFileCtrlEvent, wxCommandEvent );

// some helper functions

void wxGenerateFilterChangedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd )
{
    wxFileCtrlEvent event( wxEVT_FILECTRL_FILTERCHANGED, wnd, wnd->GetId() );

    event.SetFilterIndex( fileCtrl->GetFilterIndex() );

    wnd->GetEventHandler()->ProcessEvent( event );
}

void wxGenerateFolderChangedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd )
{
    wxFileCtrlEvent event( wxEVT_FILECTRL_FOLDERCHANGED, wnd, wnd->GetId() );

    event.SetDirectory( fileCtrl->GetDirectory() );

    wnd->GetEventHandler()->ProcessEvent( event );
}

void wxGenerateSelectionChangedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd)
{
    wxFileCtrlEvent event( wxEVT_FILECTRL_SELECTIONCHANGED, wnd, wnd->GetId() );
    event.SetDirectory( fileCtrl->GetDirectory() );

    std::vector<wxString> filenames = fileCtrl->GetFilenames();
    event.SetFiles( filenames );

    wnd->GetEventHandler()->ProcessEvent( event );
}

void wxGenerateFileActivatedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd, const wxString& filename )
{
    wxFileCtrlEvent event( wxEVT_FILECTRL_FILEACTIVATED, wnd, wnd->GetId() );
    event.SetDirectory( fileCtrl->GetDirectory() );

    std::vector<wxString> filenames;

    if ( filename.empty() )
    {
        filenames = fileCtrl->GetFilenames();
    }
    else
    {
        filenames.push_back( filename );
    }

    event.SetFiles( filenames );

    wnd->GetEventHandler()->ProcessEvent( event );
}

///////////////////////////////////////////////////////////////////////////////
// wxFileCtrlEvent implementation
///////////////////////////////////////////////////////////////////////////////

wxString wxFileCtrlEvent::GetFile() const
{
    wxASSERT_MSG( !wxDynamicCast( GetEventObject(), wxFileCtrl )->HasMultipleFileSelection(),
                  wxT( "Please use GetFiles() to get all files instead of this function" ) );

    wxString string;
    if (!m_files.empty())
        string = m_files[0];
    return string;
}

#endif // wxUSE_FILECTRL
