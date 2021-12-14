///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/dseldlg.cpp
// Purpose:     implementation of ::wxDirSelector()
// Author:      Paul Thiessen
// Modified by:
// Created:     20.02.01
// Copyright:   (c) 2001 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_DIRDLG

#include "wx/dirdlg.h"

// ============================================================================
// implementation
// ============================================================================

std::string wxDirSelector(std::string_view message,
                       const std::string& defaultPath,
                       unsigned int style,
                       const wxPoint& pos,
                       wxWindow *parent)
{
    std::string path;

    wxDirDialog dirDialog(parent, message, defaultPath, style, pos);
    if ( dirDialog.ShowModal() == wxID_OK )
    {
        path = dirDialog.GetPath();
    }

    return path;
}

#endif // wxUSE_DIRDLG
