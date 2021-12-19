/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/dynlib.cpp
// Purpose:     Dynamic library management
// Author:      Guilhem Lavaux
// Modified by:
// Created:     20/07/98
// Copyright:   (c) 1998 Guilhem Lavaux
//                  2000-2005 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

//FIXME:  This class isn't really common at all, it should be moved into
//        platform dependent files (already done for Windows and Unix)

#if wxUSE_DYNLIB_CLASS

#include "wx/dynlib.h"

#include "wx/intl.h"
#include "wx/utils.h"
#include "wx/filefn.h"
#include "wx/filename.h"        // for SplitPath()

import WX.Cmn.PlatInfo;

// ============================================================================
// implementation
// ============================================================================

// ---------------------------------------------------------------------------
// wxDynamicLibrary
// ---------------------------------------------------------------------------

// for MSW/Unix it is defined in platform-specific file
#if !(defined(WX_WINDOWS) || defined(__UNIX__))

wxDllType wxDynamicLibrary::GetProgramHandle()
{
   wxFAIL_MSG( "GetProgramHandle() is not implemented under this platform");
   return 0;
}

#endif // WX_WINDOWS || __UNIX__


bool wxDynamicLibrary::Load(const std::string& libnameOrig, unsigned int flags)
{
    wxASSERT_MSG(m_handle == nullptr, "Library already loaded.");

    // add the proper extension for the DLL ourselves unless told not to
    std::string libname = libnameOrig;
    if ( !(flags & wxDL_VERBATIM) )
    {
        // and also check that the libname doesn't already have it
        std::string ext;
        wxFileName::SplitPath(libname, nullptr, nullptr, &ext);
        if ( ext.empty() )
        {
            libname += GetDllExt(wxDynamicLibraryCategory::Module);
        }
    }

    m_handle = RawLoad(libname, flags);

    if ( m_handle == nullptr && !(flags & wxDL_QUIET) )
    {
        ReportError(_("Failed to load shared library '%s'"), libname);
    }

    return IsLoaded();
}

void *wxDynamicLibrary::DoGetSymbol(const std::string &name, bool *success) const
{
    wxCHECK_MSG( IsLoaded(), nullptr,
                 "Can't load symbol from unloaded library" );

    void *symbol = RawGetSymbol(m_handle, name);

    if ( success )
        *success = symbol != nullptr;

    return symbol;
}

void *wxDynamicLibrary::GetSymbol(const std::string& name, bool *success) const
{
    void *symbol = DoGetSymbol(name, success);
    if ( !symbol )
    {
        ReportError(_("Couldn't find symbol '%s' in a dynamic library"), name);
    }

    return symbol;
}

// ----------------------------------------------------------------------------
// informational methods
// ----------------------------------------------------------------------------

/*static*/
std::string wxDynamicLibrary::GetDllExt(wxDynamicLibraryCategory cat)
{
    wxUnusedVar(cat);
#if defined(WX_WINDOWS)
    return ".dll";
#elif defined(__HPUX__)
    return ".sl";
#elif defined(__DARWIN__)
    switch ( cat )
    {
        case wxDynamicLibraryCategory::Library:
            return ".dylib";
        case wxDynamicLibraryCategory::Module:
            return ".bundle";
    }
    wxFAIL_MSG("unreachable");
    return {}; // silence gcc warning
#else
    return ".so";
#endif
}

/*static*/
std::string
wxDynamicLibrary::CanonicalizeName(const std::string& name,
                                   wxDynamicLibraryCategory cat)
{
    std::string nameCanonic;

    // under Unix the library names usually start with "lib" prefix, add it
#if defined(__UNIX__)
    switch ( cat )
    {
        case wxDynamicLibraryCategory::Library:
            // Library names should start with "lib" under Unix.
            nameCanonic = "lib";
            break;
        case wxDynamicLibraryCategory::Module:
            // Module names are arbitrary and should have no prefix added.
            break;
    }
#endif

    nameCanonic += fmt::format("{}{}", name, GetDllExt(cat));

    return nameCanonic;
}

/*static*/
std::string wxDynamicLibrary::CanonicalizePluginName(const std::string& name,
                                                  wxPluginCategory cat)
{
    std::string suffix;
    if ( cat == wxPluginCategory::Gui )
    {
        suffix = wxPlatformInfo::Get().GetPortIdShortName();
    }
    suffix += 'u';
#ifdef __WXDEBUG__
    suffix += 'd';
#endif

    if ( !suffix.empty() )
        suffix = "_" + suffix;

#define WXSTRINGIZE(x)  #x
#if defined(__UNIX__)
    #if (wxMINOR_VERSION % 2) == 0
        #define wxDLLVER(x,y,z) "-" WXSTRINGIZE(x) "." WXSTRINGIZE(y)
    #else
        #define wxDLLVER(x,y,z) "-" WXSTRINGIZE(x) "." WXSTRINGIZE(y) "." WXSTRINGIZE(z)
    #endif
#else
    #if (wxMINOR_VERSION % 2) == 0
        #define wxDLLVER(x,y,z) WXSTRINGIZE(x) WXSTRINGIZE(y)
    #else
        #define wxDLLVER(x,y,z) WXSTRINGIZE(x) WXSTRINGIZE(y) WXSTRINGIZE(z)
    #endif
#endif

    suffix += wxDLLVER(wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER);

#undef wxDLLVER
#undef WXSTRINGIZE

#ifdef WX_WINDOWS
    // Add compiler identification:
    #if defined(__GNUG__)
        suffix += "_gcc";
    #elif defined(__VISUALC__)
        suffix += "_vc";
    #endif
#endif

    return CanonicalizeName(name + suffix, wxDynamicLibraryCategory::Module);
}

/*static*/
std::string wxDynamicLibrary::GetPluginsDirectory()
{
#ifdef __UNIX__
    std::string format = wxGetInstallPrefix();
    if ( format.empty() )
        return {};
    std::string dir;
    format << wxFILE_SEP_PATH
           << "lib" << wxFILE_SEP_PATH
           << "wx" << wxFILE_SEP_PATH
#if (wxMINOR_VERSION % 2) == 0
           << "%i.%i";
    dir.Printf(format.c_str(), wxMAJOR_VERSION, wxMINOR_VERSION);
#else
           << "%i.%i.%i";
    dir.Printf(format.c_str(),
               wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER);
#endif
    return dir;

#else // ! __UNIX__
    return {};
#endif
}


#endif // wxUSE_DYNLIB_CLASS
