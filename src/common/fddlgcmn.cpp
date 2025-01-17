/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fddlgcmn.cpp
// Purpose:     common parts of wxFindReplaceDialog implementations
// Author:      Vadim Zeitlin
// Modified by:
// Created:     01.08.01
// Copyright:   (c) 2001 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_FINDREPLDLG

#include "wx/fdrepdlg.h"

// ----------------------------------------------------------------------------
// wxWin macros
// ----------------------------------------------------------------------------

wxDEFINE_EVENT( wxEVT_FIND, wxFindDialogEvent );
wxDEFINE_EVENT( wxEVT_FIND_NEXT, wxFindDialogEvent );
wxDEFINE_EVENT( wxEVT_FIND_REPLACE, wxFindDialogEvent );
wxDEFINE_EVENT( wxEVT_FIND_REPLACE_ALL, wxFindDialogEvent );
wxDEFINE_EVENT( wxEVT_FIND_CLOSE, wxFindDialogEvent );

// ----------------------------------------------------------------------------
// wxFindReplaceDialogBase
// ----------------------------------------------------------------------------

void wxFindReplaceDialogBase::Send(wxFindDialogEvent& event)
{
    // we copy the data to dialog->GetData() as well

    m_FindReplaceData->m_flags = FindReplaceFlags{event.GetFlags()};
    m_FindReplaceData->m_FindWhat = event.GetFindString();
    if ( HasFlag(wxFR_REPLACEDIALOG) &&
         (event.GetEventType() == wxEVT_FIND_REPLACE ||
          event.GetEventType() == wxEVT_FIND_REPLACE_ALL) )
    {
        m_FindReplaceData->m_ReplaceWith = event.GetReplaceString();
    }

    // translate wxEVT_FIND_NEXT to wxEVT_FIND if needed
    if ( event.GetEventType() == wxEVT_FIND_NEXT )
    {
        if ( m_FindReplaceData->m_FindWhat != m_lastSearch )
        {
            event.SetEventType(wxEVT_FIND);

            m_lastSearch = m_FindReplaceData->m_FindWhat;
        }
    }

    if ( !GetEventHandler()->ProcessEvent(event) )
    {
        // the event is not propagated upwards to the parent automatically
        // because the dialog is a top level window, so do it manually as
        // in 9 cases of 10 the message must be processed by the dialog
        // owner and not the dialog itself
        GetParent()->GetEventHandler()->ProcessEvent(event);
    }
}

#endif // wxUSE_FINDREPLDLG

