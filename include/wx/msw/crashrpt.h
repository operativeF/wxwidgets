///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/crashrpt.h
// Purpose:     helpers for the structured exception handling (SEH) under Win32
// Author:      Vadim Zeitlin
// Modified by:
// Created:     13.07.2003
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_CRASHRPT_H_
#define _WX_MSW_CRASHRPT_H_

#if wxUSE_CRASHREPORT

import <cstdint>;

struct _EXCEPTION_POINTERS;

// global pointer to exception information, only valid inside OnFatalException,
// used by wxStackWalker and wxCrashReport
inline EXCEPTION_POINTERS* wxGlobalSEInformation{nullptr};


// ----------------------------------------------------------------------------
// crash report generation flags
// ----------------------------------------------------------------------------

enum
{
    // we always report where the crash occurred
    wxCRASH_REPORT_LOCATION = 0,

    // if this flag is given, the call stack is dumped
    //
    // this results in dump/crash report as small as possible, this is the
    // default flag
    wxCRASH_REPORT_STACK = 1,

    // if this flag is given, the values of the local variables are dumped
    //
    // note that with the current implementation it requires dumping the full
    // process address space and so this will result in huge dump file and will
    // take some time to generate
    //
    // it's probably not a good idea to use this by default, start with default
    // mini dump and ask your users to set WX_CRASH_FLAGS environment variable
    // to 2 or 4 if you need more information in the dump
    wxCRASH_REPORT_LOCALS = 2,

    // if this flag is given, the values of all global variables are dumped
    //
    // this creates a much larger mini dump than just wxCRASH_REPORT_STACK but
    // still much smaller than wxCRASH_REPORT_LOCALS one
    wxCRASH_REPORT_GLOBALS = 4,

    // default is to create the smallest possible crash report
    wxCRASH_REPORT_DEFAULT = wxCRASH_REPORT_LOCATION | wxCRASH_REPORT_STACK
};

// ----------------------------------------------------------------------------
// wxCrashContext: information about the crash context
// ----------------------------------------------------------------------------

struct wxCrashContext
{
    // initialize this object with the given information or from the current
    // global exception info which is only valid inside wxApp::OnFatalException
    wxCrashContext(_EXCEPTION_POINTERS *ep = nullptr);

    // get the name for this exception code
    wxString GetExceptionString() const;


    // exception code
    size_t code;

    // exception address
    void *addr;

    // machine-specific registers vaues
    struct
    {
#ifdef __INTEL__
        std::int32_t eax, ebx, ecx, edx, esi, edi,
                ebp, esp, eip,
                cs, ds, es, fs, gs, ss,
                flags;
#endif // __INTEL__
    } regs;
};

// ----------------------------------------------------------------------------
// wxCrashReport: this class is used to create crash reports
// ----------------------------------------------------------------------------

struct wxCrashReport
{
    // set the name of the file to which the report is written, it is
    // constructed from the .exe name by default
    static void SetFileName(const std::string& filename);

    // return the current file name
    static std::string GetFileName();

    // write the exception report to the file, return true if it could be done
    // or false otherwise
    //
    // if ep pointer is NULL, the global exception info which is valid only
    // inside wxApp::OnFatalException() is used
    static bool Generate(unsigned int flags = wxCRASH_REPORT_DEFAULT,
                         _EXCEPTION_POINTERS *ep = nullptr);


    // generate a crash report from outside of wxApp::OnFatalException(), this
    // can be used to take "snapshots" of the program in wxApp::OnAssert() for
    // example
    static bool GenerateNow(unsigned int flags = wxCRASH_REPORT_DEFAULT);
};

#endif // wxUSE_CRASHREPORT

#endif // _WX_MSW_CRASHRPT_H_

