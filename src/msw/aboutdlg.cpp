///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/aboutdlg.cpp
// Purpose:     implementation of wxAboutBox() for wxMSW
// Author:      Vadim Zeitlin
// Created:     2006-10-07
// Copyright:   (c) 2006 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_ABOUTDLG

#include "wx/msgdlg.h"
#include "wx/aboutdlg.h"
#include "wx/generic/aboutdlgg.h"

#include "wx/string.h"

// ============================================================================
// implementation
// ============================================================================

// our public entry point
void wxAboutBox(const wxAboutDialogInfo& info, wxWindow* parent)
{
    // we prefer to show a simple message box if we don't have any fields which
    // can't be shown in it because as much as there is a standard about box
    // under MSW at all, this is it
    if ( info.IsSimple() )
    {
        // build the text to show in the box
        const wxString name = info.GetName();
        wxString msg;
        msg << name;
        if ( info.HasVersion() )
        {
            msg << wxT('\n');
            msg << info.GetLongVersion();
        }

        msg << "\n\n";

        if ( info.HasCopyright() )
            msg << info.GetCopyrightToDisplay() << wxT('\n');

        // add everything remaining
        msg << info.GetDescriptionAndCredits();

        wxMessageBox(msg.ToStdString(), wxString::Format(_("About %s"), name).ToStdString(), wxOK | wxCENTRE, parent);
    }
    else // simple "native" version is not enough
    {
        // we need to use the full-blown generic version
        wxGenericAboutBox(info, parent);
    }
}

#endif // wxUSE_ABOUTDLG
