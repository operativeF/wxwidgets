/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/crashrpt.cpp
// Purpose:     code to generate crash dumps (minidumps)
// Author:      Vadim Zeitlin
// Modified by:
// Created:     13.07.03
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_CRASHREPORT

#include "wx/msw/private.h"
#include "wx/wxcrtvararg.h"
#include "wx/msw/debughlp.h"
#include "wx/msw/crashrpt.h"

import WX.WinDef;

// ----------------------------------------------------------------------------
// classes
// ----------------------------------------------------------------------------

namespace
{

// low level wxBusyCursor replacement: we use Win32 API directly here instead
// of going through wxWidgets calls as this could be dangerous
class BusyCursor
{
public:
    BusyCursor()
    {
        m_hcursorOld = ::SetCursor(static_cast<WXHCURSOR>(::LoadImageW(nullptr,
                                                                     MAKEINTRESOURCEW(OCR_WAIT),
                                                                     IMAGE_CURSOR,
                                                                     0, 0,
                                                                     LR_SHARED | LR_DEFAULTSIZE)));
    }

    ~BusyCursor()
    {
        if ( m_hcursorOld )
        {
            ::SetCursor(m_hcursorOld);
        }
    }

private:
    WXHCURSOR m_hcursorOld;
};

// the real crash report generator
class wxCrashReportImpl
{
public:
    explicit wxCrashReportImpl(const std::string& filename);

    bool Generate(unsigned int flags, EXCEPTION_POINTERS *ep);

    ~wxCrashReportImpl()
    {
        if ( m_hFile != INVALID_HANDLE_VALUE )
        {
            ::CloseHandle(m_hFile);
        }
    }

private:
    // formatted output to m_hFile
    void Output(const char* format, ...);

    // output end of line
    void OutputEndl() { Output("\r\n"); } // FIXME: Change to narrow string.

    // the handle of the report file
    HANDLE m_hFile;
};

} // namespace anonymous

// ----------------------------------------------------------------------------
// globals
// ----------------------------------------------------------------------------

// the file name where the report about exception is written
//
// we use fixed buffer to avoid (big) dynamic allocations when the program
// crashes
static char gs_reportFilename[MAX_PATH];

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxCrashReportImpl
// ----------------------------------------------------------------------------

wxCrashReportImpl::wxCrashReportImpl(const std::string& filename)
{
    boost::nowide::wstackstring stackFilename{filename.c_str()};
    m_hFile = ::CreateFileW
                (
                    stackFilename.get(),
                    GENERIC_WRITE,
                    0,                          // no sharing
                    nullptr,                       // default security
                    CREATE_ALWAYS,
                    FILE_FLAG_WRITE_THROUGH,
                    nullptr                        // no template file
                );
}

void wxCrashReportImpl::Output(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);

    WXDWORD cbWritten;

    wxString s = wxString::FormatV(format, argptr);

    wxCharBuffer buf(s.mb_str(wxConvUTF8));
    ::WriteFile(m_hFile, buf.data(), strlen(buf.data()), &cbWritten, nullptr);

    va_end(argptr);
}

bool wxCrashReportImpl::Generate(unsigned int flags, EXCEPTION_POINTERS *ep)
{
    if ( m_hFile == INVALID_HANDLE_VALUE )
        return false;

#if wxUSE_DBGHELP
    if ( !ep )
        ep = wxGlobalSEInformation;

    if ( !ep )
    {
        Output("Context for crash report generation not available.");
        return false;
    }

    // show to the user that we're doing something...
    BusyCursor busyCursor;

    // user-specified crash report flags override those specified by the
    // programmer
    TCHAR envFlags[64];
    const WXDWORD dwLen = ::GetEnvironmentVariableW
                    (
                        "WX_CRASH_FLAGS",
                        envFlags,
                        WXSIZEOF(envFlags)
                    );

    unsigned int flagsEnv;
    if ( dwLen && dwLen < WXSIZEOF(envFlags) &&
            wxSscanf(envFlags, "%d", &flagsEnv) == 1 )
    {
        flags = flagsEnv;
    }

    if ( wxDbgHelpDLL::Init() )
    {
        MINIDUMP_EXCEPTION_INFORMATION minidumpExcInfo;

        minidumpExcInfo.ThreadId = ::GetCurrentThreadId();
        minidumpExcInfo.ExceptionPointers = ep;
        minidumpExcInfo.ClientPointers = FALSE; // in our own address space

        // do generate the dump
        const MINIDUMP_TYPE dumpFlags = [flags]() {
            if ( flags & wxCRASH_REPORT_LOCALS )
            {
                // the only way to get local variables is to dump the entire
                // process memory space -- but this makes for huge (dozens or
                // even hundreds of Mb) files
                return MiniDumpWithFullMemory;
            }
            else if ( flags & wxCRASH_REPORT_GLOBALS )
            {
                // MiniDumpWriteDump() has the option for dumping just the data
                // segment which contains all globals -- exactly what we need
                return MiniDumpWithDataSegs;
            }
            else // minimal dump
            {
                // the file size is not much bigger than when using MiniDumpNormal
                // if we use the flags below, but the minidump is much more useful
                // as it contains the values of many (but not all) local variables
                return (MINIDUMP_TYPE)(MiniDumpScanMemory
                                       | MiniDumpWithIndirectlyReferencedMemory
                                       );
            }
        }();

        if ( !wxDbgHelpDLL::MiniDumpWriteDump
              (
                ::GetCurrentProcess(),
                ::GetCurrentProcessId(),
                m_hFile,                    // file to write to
                dumpFlags,                  // kind of dump to craete
                &minidumpExcInfo,
                nullptr,                       // no extra user-defined data
                nullptr                        // no callbacks
              ) )
        {
            Output("MiniDumpWriteDump() failed.");

            return false;
        }

        return true;
    }
    else // dbghelp.dll couldn't be loaded
    {
        Output("%s", static_cast<const wxChar*>(
                    wxDbgHelpDLL::GetErrorMessage().c_str()
              ));
    }
#else // !wxUSE_DBGHELP
    wxUnusedVar(flags);
    wxUnusedVar(ep);

    Output("Support for crash report generation was not included "
           "in this wxWidgets version.");
#endif // wxUSE_DBGHELP/!wxUSE_DBGHELP

    return false;
}

// ----------------------------------------------------------------------------
// wxCrashReport
// ----------------------------------------------------------------------------

/* static */
void wxCrashReport::SetFileName(const std::string& filename)
{
    wxStrlcpy(gs_reportFilename, filename.c_str(), WXSIZEOF(gs_reportFilename));
}

/* static */
std::string wxCrashReport::GetFileName()
{
    return gs_reportFilename;
}

/* static */
bool wxCrashReport::Generate(unsigned int flags, EXCEPTION_POINTERS *ep)
{
    wxCrashReportImpl impl(gs_reportFilename);

    return impl.Generate(flags, ep);
}

/* static */
bool wxCrashReport::GenerateNow(unsigned int flags)
{
    bool rc = false;

    __try
    {
        RaiseException(0x1976, 0, 0, nullptr);
    }
    __except( rc = Generate(flags, (EXCEPTION_POINTERS *)GetExceptionInformation()),
              EXCEPTION_CONTINUE_EXECUTION )
    {
        // never executed because of EXCEPTION_CONTINUE_EXECUTION above
    }

    return rc;
}

// ----------------------------------------------------------------------------
// wxCrashContext
// ----------------------------------------------------------------------------

wxCrashContext::wxCrashContext(_EXCEPTION_POINTERS *ep)
{
    wxZeroMemory(*this);

    if ( !ep )
    {
        wxCHECK_RET( wxGlobalSEInformation, "no exception info available" );
        ep = wxGlobalSEInformation;
    }

    // TODO: we could also get the operation (read/write) and address for which
    //       it failed for EXCEPTION_ACCESS_VIOLATION code
    const EXCEPTION_RECORD& rec = *ep->ExceptionRecord;
    code = rec.ExceptionCode;
    addr = rec.ExceptionAddress;

#ifdef __INTEL__
    const CONTEXT& ctx = *ep->ContextRecord;
    regs.eax = ctx.Eax;
    regs.ebx = ctx.Ebx;
    regs.ecx = ctx.Ecx;
    regs.edx = ctx.Edx;
    regs.esi = ctx.Esi;
    regs.edi = ctx.Edi;

    regs.ebp = ctx.Ebp;
    regs.esp = ctx.Esp;
    regs.eip = ctx.Eip;

    regs.cs = ctx.SegCs;
    regs.ds = ctx.SegDs;
    regs.es = ctx.SegEs;
    regs.fs = ctx.SegFs;
    regs.gs = ctx.SegGs;
    regs.ss = ctx.SegSs;

    regs.flags = ctx.EFlags;
#endif // __INTEL__
}

wxString wxCrashContext::GetExceptionString() const
{
    wxString s;

    #define CASE_EXCEPTION( x ) case EXCEPTION_##x: s = wxT(#x); break

    switch ( code )
    {
        CASE_EXCEPTION(ACCESS_VIOLATION);
        CASE_EXCEPTION(DATATYPE_MISALIGNMENT);
        CASE_EXCEPTION(BREAKPOINT);
        CASE_EXCEPTION(SINGLE_STEP);
        CASE_EXCEPTION(ARRAY_BOUNDS_EXCEEDED);
        CASE_EXCEPTION(FLT_DENORMAL_OPERAND);
        CASE_EXCEPTION(FLT_DIVIDE_BY_ZERO);
        CASE_EXCEPTION(FLT_INEXACT_RESULT);
        CASE_EXCEPTION(FLT_INVALID_OPERATION);
        CASE_EXCEPTION(FLT_OVERFLOW);
        CASE_EXCEPTION(FLT_STACK_CHECK);
        CASE_EXCEPTION(FLT_UNDERFLOW);
        CASE_EXCEPTION(INT_DIVIDE_BY_ZERO);
        CASE_EXCEPTION(INT_OVERFLOW);
        CASE_EXCEPTION(PRIV_INSTRUCTION);
        CASE_EXCEPTION(IN_PAGE_ERROR);
        CASE_EXCEPTION(ILLEGAL_INSTRUCTION);
        CASE_EXCEPTION(NONCONTINUABLE_EXCEPTION);
        CASE_EXCEPTION(STACK_OVERFLOW);
        CASE_EXCEPTION(INVALID_DISPOSITION);
        CASE_EXCEPTION(GUARD_PAGE);
        CASE_EXCEPTION(INVALID_HANDLE);

        default:
            // unknown exception, ask NTDLL for the name
            s = wxMSWFormatMessage(code, ::GetModuleHandleW(L"NTDLL.DLL"));
    }

    #undef CASE_EXCEPTION

    return s;
}

#endif // wxUSE_CRASHREPORT/!wxUSE_CRASHREPORT

