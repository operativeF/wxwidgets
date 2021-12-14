/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/dlmsw.cpp
// Purpose:     Win32-specific part of wxDynamicLibrary and related classes
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2005-01-10 (partly extracted from common/dynlib.cpp)
// Copyright:   (c) 1998-2005 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_DYNLIB_CLASS

#include "wx/msw/private.h"
#include "wx/dynlib.h"
#include "wx/msw/debughlp.h"
#include "wx/filename.h"

import WX.WinDef;

import <string>;

// For MSVC we can link in the required library explicitly, for the other
// compilers (e.g. MinGW) this needs to be done at makefiles level.
#ifdef __VISUALC__
    #pragma comment(lib, "version")
#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// class used to create wxDynamicLibraryDetails objects
class wxDynamicLibraryDetailsCreator
{
public:
    // type of parameters being passed to EnumModulesProc
    struct EnumModulesProcParams
    {
        wxDynamicLibraryDetailsArray *dlls;
    };

    static BOOL CALLBACK
    EnumModulesProc(const std::string& name, DWORD64 base, ULONG size, PVOID data);
};

// ----------------------------------------------------------------------------
// DLL version operations
// ----------------------------------------------------------------------------

static std::string GetFileVersion(const std::string& filename)
{
    std::string ver;
    boost::nowide::wstackstring stackFilename{filename.c_str()};

    WXDWORD dummy;
    const WXDWORD sizeVerInfo = ::GetFileVersionInfoSizeW(stackFilename.get(), &dummy);
    if ( sizeVerInfo )
    {
        wxCharBuffer buf(sizeVerInfo);
        if ( ::GetFileVersionInfoW(stackFilename.get(), 0, sizeVerInfo, buf.data()) )
        {
            void *pVer;
            WXUINT sizeInfo;
            if ( ::VerQueryValueW(buf.data(),
                                    L"\\",
                                    &pVer,
                                    &sizeInfo) )
            {
                VS_FIXEDFILEINFO *info = (VS_FIXEDFILEINFO *)pVer;
                ver = fmt::format("%d.%d.%d.%d",
                            HIWORD(info->dwFileVersionMS),
                            LOWORD(info->dwFileVersionMS),
                            HIWORD(info->dwFileVersionLS),
                            LOWORD(info->dwFileVersionLS));
            }
        }
    }

    return ver;
}

// ============================================================================
// wxDynamicLibraryDetailsCreator implementation
// ============================================================================

/* static */
BOOL CALLBACK
wxDynamicLibraryDetailsCreator::EnumModulesProc(const std::string& name,
                                                DWORD64 base,
                                                ULONG size,
                                                void *data)
{
    EnumModulesProcParams *params = (EnumModulesProcParams *)data;

    wxDynamicLibraryDetails details;

    // fill in simple properties
    details.m_name = name;
    details.m_address = wxUIntToPtr(base);
    details.m_length = size;

    // to get the version, we first need the full path
    const WXHMODULE hmod = wxDynamicLibrary::MSWGetModuleHandle
                         (
                            details.m_name,
                            details.m_address
                         );
    if ( hmod )
    {
        std::string fullname = wxGetFullModuleName(hmod);
        if ( !fullname.empty() )
        {
            details.m_path = fullname;
            details.m_version = GetFileVersion(fullname);
        }
    }

    params->dlls->push_back(details);

    // continue enumeration (returning FALSE would have stopped it)
    return TRUE;
}

// ============================================================================
// wxDynamicLibrary implementation
// ============================================================================

// ----------------------------------------------------------------------------
// misc functions
// ----------------------------------------------------------------------------

wxDllType wxDynamicLibrary::GetProgramHandle()
{
    return (wxDllType)::GetModuleHandleW(nullptr);
}

// ----------------------------------------------------------------------------
// error handling
// ----------------------------------------------------------------------------

/* static */
void wxDynamicLibrary::ReportError(const std::string& message, const std::string& name)
{
    std::string msg{message};
    if ( name.empty() && msg.find("%s") == std::string::npos )
        msg += "%s";
    // msg needs a %s for the name
    wxASSERT(msg.find("%s") != std::string::npos);

    const unsigned long code = wxSysErrorCode();
    std::string errMsg = wxSysErrorMsgStr(code);

    // The error message (specifically code==193) may contain a
    // placeholder '%1' which stands for the filename.
    auto subReplace = std::ranges::search(errMsg, "%1");

    if(!subReplace.empty())
        errMsg.replace(subReplace.begin(), subReplace.end(), name);

    // Mimic the output of wxLogSysError(), but use our pre-processed
    // errMsg.
    wxLogError(msg + " " + _("(error %d: %s)"), name, code, errMsg);
}

// ----------------------------------------------------------------------------
// loading/unloading DLLs
// ----------------------------------------------------------------------------

/* static */
wxDllType
wxDynamicLibrary::RawLoad(const std::string& libname, unsigned int flags)
{
    boost::nowide::wstackstring stackLibname{libname.c_str()};
    if (flags & wxDL_GET_LOADED)
        return ::GetModuleHandleW(stackLibname.get());

    return ::LoadLibraryW(stackLibname.get());
}

/* static */
void wxDynamicLibrary::Unload(wxDllType handle)
{
    if ( !::FreeLibrary(handle) )
    {
        wxLogLastError("FreeLibrary");
    }
}

/* static */
void *wxDynamicLibrary::RawGetSymbol(wxDllType handle, const std::string& name)
{
    return (void *)::GetProcAddress(handle, name.c_str());
}

// ----------------------------------------------------------------------------
// enumerating loaded DLLs
// ----------------------------------------------------------------------------

/* static */
wxDynamicLibraryDetailsArray wxDynamicLibrary::ListLoaded()
{
    wxDynamicLibraryDetailsArray dlls;

#if wxUSE_DBGHELP
    if ( wxDbgHelpDLL::Init() )
    {
        wxDynamicLibraryDetailsCreator::EnumModulesProcParams params;
        params.dlls = &dlls;

        if ( !wxDbgHelpDLL::CallEnumerateLoadedModules
                            (
                                ::GetCurrentProcess(),
                                wxDynamicLibraryDetailsCreator::EnumModulesProc,
                                &params
                            ) )
        {
            wxLogLastError("EnumerateLoadedModules");
        }
    }
#endif // wxUSE_DBGHELP

    return dlls;
}

// ----------------------------------------------------------------------------
// Getting the module from an address inside it
// ----------------------------------------------------------------------------

namespace
{

// Tries to dynamically load GetModuleHandleEx() from kernel32.dll and call it
// to get the module handle from the given address. Returns NULL if it fails to
// either resolve the function (which can only happen on pre-Vista systems
// normally) or if the function itself failed.
WXHMODULE CallGetModuleHandleEx(const void* addr)
{
    using GetModuleHandleEx_t = BOOL (WINAPI*)(WXDWORD, LPCTSTR, WXHMODULE *);

    static const GetModuleHandleEx_t INVALID_FUNC_PTR = (GetModuleHandleEx_t)-1;

    static GetModuleHandleEx_t s_pfnGetModuleHandleEx = INVALID_FUNC_PTR;
    if ( s_pfnGetModuleHandleEx == INVALID_FUNC_PTR )
    {
        wxDynamicLibrary dll("kernel32.dll", wxDL_VERBATIM);

        wxDL_INIT_FUNC_AW(s_pfn, GetModuleHandleEx, dll);

        // dll object can be destroyed, kernel32.dll won't be unloaded anyhow
    }

    if ( !s_pfnGetModuleHandleEx )
        return nullptr;

    // flags are GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT |
    //           GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
    WXHMODULE hmod;
    if ( !s_pfnGetModuleHandleEx(6, (LPCTSTR)addr, &hmod) )
        return nullptr;

    return hmod;
}

} // anonymous namespace

/* static */
void* wxDynamicLibrary::GetModuleFromAddress(const void* addr, std::string* path)
{
    WXHMODULE hmod = CallGetModuleHandleEx(addr);
    if ( !hmod )
    {
        wxLogLastError("GetModuleHandleEx");
        return nullptr;
    }

    if ( path )
    {
        WCHAR libname[MAX_PATH];
        if ( !::GetModuleFileNameW(hmod, libname, MAX_PATH) )
        {
            // GetModuleFileName could also return extended-length paths (paths
            // prepended with "//?/", maximum length is 32767 charachters) so,
            // in principle, MAX_PATH could be unsufficient and we should try
            // increasing the buffer size here.
            wxLogLastError("GetModuleFromAddress");
            return nullptr;
        }

        libname[MAX_PATH-1] = wxT('\0');

        *path = boost::nowide::narrow(libname);
    }

    // In Windows WXHMODULE is actually the base address of the module so we
    // can just cast it to the address.
    return hmod;
}

/* static */
WXHMODULE wxDynamicLibrary::MSWGetModuleHandle(const std::string& name, void *addr)
{
    // we want to use GetModuleHandleEx() instead of usual GetModuleHandle()
    // because the former works correctly for comctl32.dll while the latter
    // returns NULL when comctl32.dll version 6 is used under XP (note that
    // GetModuleHandleEx() is only available under XP and later, coincidence?)
    WXHMODULE hmod = CallGetModuleHandleEx(addr);

    boost::nowide::wstackstring stackName{name.c_str()};

    return hmod ? hmod : ::GetModuleHandleW(stackName.get());
}

#endif // wxUSE_DYNLIB_CLASS

