/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/main.cpp
// Purpose:     WinMain/DllMain
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"
#include "wx/app.h"
#include "wx/utils.h"

#include "wx/dynlib.h"

#include "wx/msw/seh.h"

#if wxUSE_ON_FATAL_EXCEPTION
    #include "wx/datetime.h"
    #include "wx/msw/crashrpt.h"
#endif // wxUSE_ON_FATAL_EXCEPTION

// defined in common/init.cpp
extern int wxEntryReal(int& argc, wxChar **argv);
extern int wxEntryCleanupReal(int& argc, wxChar **argv);

// ============================================================================
// implementation: various entry points
// ============================================================================

#if wxUSE_BASE

// ----------------------------------------------------------------------------
// wrapper wxEntry catching all Win32 exceptions occurring in a wx program
// ----------------------------------------------------------------------------

// wrap real wxEntry in a try-except block to be able to call
// OnFatalException() if necessary
#if wxUSE_ON_FATAL_EXCEPTION

// flag telling us whether the application wants to handle exceptions at all
static bool gs_handleExceptions = false;

static void wxFatalExit()
{
    // use the same exit code as abort()
    ::ExitProcess(3);
}

unsigned long wxGlobalSEHandler(EXCEPTION_POINTERS *pExcPtrs)
{
    if ( gs_handleExceptions && wxTheApp )
    {
        // store the pointer to exception info
        wxGlobalSEInformation = pExcPtrs;

        // give the user a chance to do something special about this
        wxSEH_TRY
        {
            wxTheApp->OnFatalException();
        }
        wxSEH_IGNORE      // ignore any exceptions inside the exception handler

        wxGlobalSEInformation = nullptr;

        // this will execute our handler and terminate the process
        return EXCEPTION_EXECUTE_HANDLER;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

#ifdef __VISUALC__

void wxSETranslator([[maybe_unused]] unsigned int code, EXCEPTION_POINTERS *ep)
{
    switch ( wxGlobalSEHandler(ep) )
    {
        default:
            wxFAIL_MSG( "unexpected wxGlobalSEHandler() return value" );
            // fall through

        case EXCEPTION_EXECUTE_HANDLER:
            // if wxApp::OnFatalException() had been called we should exit the
            // application -- but we shouldn't kill our host when we're a DLL
#ifndef WXMAKINGDLL
            wxFatalExit();
#endif // not a DLL
            break;

        case EXCEPTION_CONTINUE_SEARCH:
            // we're called for each "catch ( ... )" and if we (re)throw from
            // here, the catch handler body is not executed, so the effect is
            // as if had inhibited translation of SE to C++ ones because the
            // handler will never see any structured exceptions
            throw;
    }
}

#endif // __VISUALC__

bool wxHandleFatalExceptions(bool doit)
{
    // assume this can only be called from the main thread
    gs_handleExceptions = doit;

#if wxUSE_CRASHREPORT
    if ( doit )
    {
        // try to find a place where we can put out report file later
        wxChar fullname[MAX_PATH];
        if ( !::GetTempPathW(WXSIZEOF(fullname), fullname) )
        {
            wxLogLastError("GetTempPath");

            // when all else fails...
            wxStrcpy(fullname, "c:\\");
        }

        // use PID and date to make the report file name more unique
        wxString name = wxString::Format
                        (
#if wxUSE_DATETIME
                            "%s_%s_%lu.dmp",
#else
                            "%s_%lu.dmp",
#endif
                            wxTheApp ? (const wxChar*)wxTheApp->GetAppDisplayName().c_str()
                                     : L"wxwindows", // TODO: Temporary, convert to narrow string.
#if wxUSE_DATETIME
                            wxDateTime::Now().Format("%Y%m%dT%H%M%S").c_str(),
#endif
                            ::GetCurrentProcessId()
                        );

        wxStrncat(fullname, name, WXSIZEOF(fullname) - wxStrlen(fullname) - 1);

        wxCrashReport::SetFileName(fullname);
    }
#endif // wxUSE_CRASHREPORT

    return true;
}

int wxEntry(int& argc, wxChar **argv)
{
    DisableAutomaticSETranslator();

    wxSEH_TRY
    {
        return wxEntryReal(argc, argv);
    }
    wxSEH_HANDLE(-1)
}

#else // !wxUSE_ON_FATAL_EXCEPTION

int wxEntry(int& argc, wxChar **argv)
{
    return wxEntryReal(argc, argv);
}

#endif // wxUSE_ON_FATAL_EXCEPTION/!wxUSE_ON_FATAL_EXCEPTION

#endif // wxUSE_BASE

// ----------------------------------------------------------------------------
// Windows-specific wxEntry
// ----------------------------------------------------------------------------

struct wxMSWCommandLineArguments
{
    // Initialize this object from the current process command line.
    //
    // In Unicode build prefer to use the standard function for tokenizing the
    // command line, but we can't use it with narrow strings, so use our own
    // approximation instead then.
    void Init()
    {
        argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    }

    ~wxMSWCommandLineArguments()
    {
        if ( argc )
            ::LocalFree(argv);
    }

    wxChar** argv{ nullptr };

    int argc{0};
};

static wxMSWCommandLineArguments wxArgs;

#if wxUSE_GUI

// common part of wxMSW-specific wxEntryStart() and wxEntry() overloads
static bool
wxMSWEntryCommon(WXHINSTANCE hInstance, int nCmdShow)
{
    // remember the parameters Windows gave us
    wxSetInstance(hInstance);
#ifdef __WXMSW__
    wxApp::m_nCmdShow = nCmdShow;
#endif

    wxArgs.Init();

    return true;
}

bool wxEntryStart(WXHINSTANCE hInstance,
                              [[maybe_unused]] WXHINSTANCE hPrevInstance,
                              [[maybe_unused]] wxCmdLineArgType pCmdLine,
                              int nCmdShow)
{
    if ( !wxMSWEntryCommon(hInstance, nCmdShow) )
       return false;

    return wxEntryStart(wxArgs.argc, wxArgs.argv);
}

int wxEntry(WXHINSTANCE hInstance,
                        [[maybe_unused]] WXHINSTANCE hPrevInstance,
                        [[maybe_unused]] wxCmdLineArgType pCmdLine,
                        int nCmdShow)
{
    if ( !wxMSWEntryCommon(hInstance, nCmdShow) )
        return -1;

    return wxEntry(wxArgs.argc, wxArgs.argv);
}

#endif // wxUSE_GUI

// ----------------------------------------------------------------------------
// global WXHINSTANCE
// ----------------------------------------------------------------------------

#if wxUSE_BASE

int wxEntry()
{
    wxArgs.Init();

    return wxEntry(wxArgs.argc, wxArgs.argv);
}

WXHINSTANCE wxhInstance = nullptr;

extern "C" WXHINSTANCE wxGetInstance()
{
    return wxhInstance;
}

void wxSetInstance(WXHINSTANCE hInst)
{
    wxhInstance = hInst;
}

#endif // wxUSE_BASE
