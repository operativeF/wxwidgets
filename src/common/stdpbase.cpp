///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/stdpbase.cpp
// Purpose:     wxStandardPathsBase methods common to all ports
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2004-10-19
// Copyright:   (c) 2004 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/app.h"
#include "wx/apptrait.h"
#include "wx/stdpaths.h"

import WX.File.Filename;

// ----------------------------------------------------------------------------
// module globals
// ----------------------------------------------------------------------------

namespace
{

// Derive a class just to be able to create it: wxStandardPaths ctor is
// protected to prevent its misuse, but it also means we can't create an object
// of this class directly.
class wxStandardPathsDefault : public wxStandardPaths
{
};

static wxStandardPathsDefault gs_stdPaths;

} // anonymous namespace

// ============================================================================
// implementation
// ============================================================================

/* static */
wxStandardPaths& wxStandardPathsBase::Get()
{
    wxAppTraits * const traits = wxApp::GetTraitsIfExists();
    wxCHECK_MSG( traits, gs_stdPaths, "create wxApp before calling this" );

    return traits->GetStandardPaths();
}

std::string wxStandardPathsBase::GetExecutablePath() const
{
    if ( !wxTheApp || !wxTheApp->argv )
        return {};

    wxString argv0 = wxTheApp->argv[0];
    if (wxIsAbsolutePath(argv0))
        return argv0.ToStdString();

    // Search PATH.environment variable...
    wxPathList pathlist;
    pathlist.AddEnvList("PATH");
    wxString path = pathlist.FindAbsoluteValidPath(argv0);
    if ( path.empty() )
        return argv0.ToStdString();       // better than nothing

    wxFileName filename(path);
    filename.Normalize();
    return filename.GetFullPath();
}

wxStandardPaths& wxAppTraitsBase::GetStandardPaths()
{
    return gs_stdPaths;
}

wxStandardPathsBase::wxStandardPathsBase()
{
    // Set the default information that is used when
    // forming some paths (by AppendAppInfo).
    // Derived classes can call this in their constructors
    // to set the platform-specific settings
    UseAppInfo(AppInfo_AppName);

    // Default for compatibility with the existing config files.
    SetFileLayout(FileLayout_Classic);
}

std::string wxStandardPathsBase::GetLocalDataDir() const
{
    return GetDataDir();
}

std::string wxStandardPathsBase::GetUserLocalDataDir() const
{
    return GetUserDataDir();
}

std::string wxStandardPathsBase::GetAppDocumentsDir() const
{
    const std::string docsDir = GetDocumentsDir();
    std::string appDocsDir = AppendAppInfo(docsDir);

    return wxDirExists(appDocsDir) ? appDocsDir : docsDir;
}

// return the temporary directory for the current user
std::string wxStandardPathsBase::GetTempDir() const
{
    return wxFileName::GetTempDir();
}

std::string wxStandardPathsBase::GetUserDir([[maybe_unused]] Dir userDir) const
{
    return wxFileName::GetHomeDir();
}

/* static */
std::string
wxStandardPathsBase::AppendPathComponent(const std::string& dir,
                                         const std::string& component)
{
    wxString subdir(dir);

    // empty string indicates that an error has occurred, don't touch it then
    if ( !subdir.empty() )
    {
        if ( !component.empty() )
        {
            const wxChar ch = *(subdir.end() - 1);
            if ( !wxFileName::IsPathSeparator(ch) && ch != wxT('.') )
                subdir += wxFileName::GetPathSeparator();

            subdir += component;
        }
    }

    return subdir.ToStdString();
}


std::string wxStandardPathsBase::AppendAppInfo(const std::string& dir) const
{
    std::string subdir(dir);

    if ( UsesAppInfo(AppInfo_VendorName) )
    {
        subdir = AppendPathComponent(subdir, wxTheApp->GetVendorName());
    }

    if ( UsesAppInfo(AppInfo_AppName) )
    {
        subdir = AppendPathComponent(subdir, wxTheApp->GetAppName());
    }

    return subdir;
}

