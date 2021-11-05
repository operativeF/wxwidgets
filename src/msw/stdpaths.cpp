///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/stdpaths.cpp
// Purpose:     wxStandardPaths implementation for Win32
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2004-10-19
// Copyright:   (c) 2004 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_STDPATHS

#include "wx/stdpaths.h"

#include "wx/msw/private.h"

#include "wx/utils.h"
#include "wx/dynlib.h"
#include "wx/filename.h"

#include "wx/msw/wrapshl.h"
#include "wx/msw/private/cotaskmemptr.h"

#include <initguid.h>

// ----------------------------------------------------------------------------
// types
// ----------------------------------------------------------------------------

using SHGetKnownFolderPath_t = HRESULT (WINAPI*)(const GUID&, DWORD, HANDLE, PWSTR *);

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// used in our wxLogTrace messages
constexpr char TRACE_MASK[] = "stdpaths";

#ifndef CSIDL_PERSONAL
    #define CSIDL_PERSONAL        0x0005
#endif

// ----------------------------------------------------------------------------
// module globals
// ----------------------------------------------------------------------------

namespace
{

DEFINE_GUID(wxFOLDERID_Downloads,
    0x374de290, 0x123f, 0x4565, 0x91, 0x64, 0x39, 0xc4, 0x92, 0x5e, 0x46, 0x7b);

struct ShellFunctions
{
    SHGetKnownFolderPath_t pSHGetKnownFolderPath{nullptr};
    bool initialized{false};
};

// in spite of using a static variable, this is MT-safe as in the worst case it
// results in initializing the function pointer several times -- but this is
// harmless
ShellFunctions gs_shellFuncs;

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

void ResolveShellFunctions()
{
#if wxUSE_DYNLIB_CLASS

    // start with the newest functions, fall back to the oldest ones
    // first check for SHGetFolderPath (shell32.dll 5.0)
    wxString shellDllName("shell32");

    wxDynamicLibrary dllShellFunctions( shellDllName );
    if ( !dllShellFunctions.IsLoaded() )
    {
        wxLogTrace(TRACE_MASK, "Failed to load %s.dll", shellDllName.c_str() );
    }

    // don't give errors if the functions are unavailable, we're ready to deal
    // with this
    wxLogNull noLog;

    gs_shellFuncs.pSHGetKnownFolderPath = (SHGetKnownFolderPath_t)
        dllShellFunctions.GetSymbol("SHGetKnownFolderPath");

    // shell32.dll is going to be unloaded, but it still remains in memory
    // because we also link to it statically, so it's ok

    gs_shellFuncs.initialized = true;
#endif
}

} // anonymous namespace

// ============================================================================
// wxStandardPaths implementation
// ============================================================================

// ----------------------------------------------------------------------------
// private helpers
// ----------------------------------------------------------------------------

wxString wxStandardPaths::DoGetKnownFolder(const GUID& rfid)
{
    if (!gs_shellFuncs.initialized)
        ResolveShellFunctions();

    wxString dir;

    if ( gs_shellFuncs.pSHGetKnownFolderPath )
    {
        wxCoTaskMemPtr<wchar_t> pDir;
        HRESULT hr = gs_shellFuncs.pSHGetKnownFolderPath(rfid, 0, nullptr, &pDir);
        if ( SUCCEEDED(hr) )
        {
            dir = pDir;
        }
    }

    return dir;
}


wxString wxStandardPaths::GetAppDir() const
{
    if ( m_appDir.empty() )
    {
        m_appDir = wxFileName(wxGetFullModuleName()).GetPath();
    }

    return m_appDir;
}

wxString wxStandardPaths::GetUserDir(Dir userDir) const
{
    const GUID knownID = [userDir]() {
        switch (userDir)
        {
            case Dir_Cache:
                return FOLDERID_LocalAppData;

            case Dir_Desktop:
                return FOLDERID_Desktop;

            case Dir_Downloads:
                return FOLDERID_Downloads;

            case Dir_Music:
                return FOLDERID_Music;

            case Dir_Pictures:
                return FOLDERID_Pictures;

            case Dir_Videos:
                return FOLDERID_Videos;

            default:
                return FOLDERID_Documents;
        }
    }();

    return DoGetKnownFolder(knownID);
}

// ----------------------------------------------------------------------------
// MSW-specific functions
// ----------------------------------------------------------------------------

void wxStandardPaths::IgnoreAppSubDir(const wxString& subdirPattern)
{
    wxFileName fn = wxFileName::DirName(GetAppDir());

    if ( !fn.GetDirCount() )
    {
        // no last directory to ignore anyhow
        return;
    }

    const wxString lastdir = fn.GetDirs().back().Lower();
    if ( lastdir.Matches(subdirPattern.Lower()) )
    {
        fn.RemoveLastDir();

        // store the cached value so that subsequent calls to GetAppDir() will
        // reuse it instead of using just the program binary directory
        m_appDir = fn.GetPath();
    }
}

void wxStandardPaths::IgnoreAppBuildSubDirs()
{
    IgnoreAppSubDir("debug");
    IgnoreAppSubDir("release");

    // there can also be an architecture-dependent parent directory, ignore it
    // as well
#ifdef __WIN64__
    IgnoreAppSubDir("Win64");
    IgnoreAppSubDir("x64");
    IgnoreAppSubDir("x86_64");
    IgnoreAppSubDir("ARM64");
#else // __WIN32__
    IgnoreAppSubDir("Win32");
    IgnoreAppSubDir("x86");
#endif // __WIN64__/__WIN32__

    wxString compilerPrefix;
#ifdef __VISUALC__
    compilerPrefix = "vc";
#elif defined(__GNUG__)
    compilerPrefix = "gcc";
#else
    return;
#endif

    IgnoreAppSubDir(compilerPrefix + "_msw*");
}

void wxStandardPaths::DontIgnoreAppSubDir()
{
    // this will force the next call to GetAppDir() to use the program binary
    // path as the application directory
    m_appDir.clear();
}

/* static */
wxString wxStandardPaths::MSWGetShellDir(const GUID& rfid)
{
    return DoGetKnownFolder(rfid);
}

// ----------------------------------------------------------------------------
// public functions
// ----------------------------------------------------------------------------

wxStandardPaths::wxStandardPaths()
{
    // make it possible to run uninstalled application from the build directory
    IgnoreAppBuildSubDirs();
}

std::string wxStandardPaths::GetExecutablePath() const
{
    return wxGetFullModuleName();
}

wxString wxStandardPaths::GetConfigDir() const
{
    return AppendAppInfo(DoGetKnownFolder(FOLDERID_ProgramData));
}

wxString wxStandardPaths::GetUserConfigDir() const
{
    return DoGetKnownFolder(FOLDERID_RoamingAppData);
}

wxString wxStandardPaths::GetDataDir() const
{
    // under Windows each program is usually installed in its own directory and
    // so its datafiles are in the same directory as its main executable
    return GetAppDir();
}

wxString wxStandardPaths::GetUserDataDir() const
{
    return AppendAppInfo(GetUserConfigDir());
}

wxString wxStandardPaths::GetUserLocalDataDir() const
{
    return AppendAppInfo(DoGetKnownFolder(FOLDERID_LocalAppData));
}

wxString wxStandardPaths::GetPluginsDir() const
{
    // there is no standard location for plugins, suppose they're in the same
    // directory as the .exe
    return GetAppDir();
}


wxString
wxStandardPaths::MakeConfigFileName(const wxString& basename,
                                    ConfigFileConv WXUNUSED(conv)) const
{
    wxFileName fn({}, basename);
    fn.SetExt("ini");
    return fn.GetFullName();
}

// ============================================================================
// wxStandardPathsWin16 implementation
// ============================================================================

wxString wxStandardPathsWin16::GetConfigDir() const
{
    // this is for compatibility with earlier wxFileConfig versions
    // which used the Windows directory for the global files
    wxString dir;
    if ( !::GetWindowsDirectoryW(wxStringBuffer(dir, MAX_PATH), MAX_PATH) )
    {
        wxLogLastError("GetWindowsDirectory");
    }

    return dir;
}

wxString wxStandardPathsWin16::GetUserConfigDir() const
{
    // again, for wxFileConfig which uses $HOME for its user config file
    return wxGetHomeDir();
}

#endif // wxUSE_STDPATHS
