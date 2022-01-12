///////////////////////////////////////////////////////////////////////////////
// Name:        test.cpp
// Purpose:     Test program for wxWidgets
// Author:      Mike Wetherell
// Copyright:   (c) 2004 Mike Wetherell
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS // defined so the asserts are crazy fast - both for compilation and execution

#include "doctest.h"

#include "wx/apptrait.h"

#if wxUSE_GUI
    #include "testableframe.h"

#ifdef __WXGTK__
    #include <glib.h>
#endif // __WXGTK__
#endif // wxUSE_GUI

#include "wx/app.h"
#include "wx/wxcrtvararg.h"
#include "wx/log.h"
#include "wx/socket.h"
#include "wx/evtloop.h"
#include "wx/utils.h"

#include <boost/nowide/args.hpp>
#include <fmt/core.h>

import Utils.Strings;
import WX.MetaTest;
import WX.Test.Prec;
import WX.Cmn.CommandLine;

// for all others, include the necessary headers
import <iostream>;
import <exception>;

// Also define our own global variables.
namespace wxPrivate
{
    std::string wxTheCurrentTestClass, wxTheCurrentTestMethod;
}

// ----------------------------------------------------------------------------
// helper classes
// ----------------------------------------------------------------------------

// exception class for MSVC debug CRT assertion failures
#ifdef wxUSE_VC_CRTDBG

struct CrtAssertFailure
{
    CrtAssertFailure(const char *message) : m_msg(message) { }

    const wxString m_msg;

    CrtAssertFailure& operator=(const CrtAssertFailure&) = delete;
};

#endif // wxUSE_VC_CRTDBG

#if wxDEBUG_LEVEL

// Information about the last not yet handled assertion.
static wxString s_lastAssertMessage;

static wxString FormatAssertMessage(const wxString& file,
                                    int line,
                                    const wxString& func,
                                    const wxString& cond,
                                    const wxString& msg)
{
    wxString str;
    str << "wxWidgets assert: " << cond
        << " failed at " << file << ":" << line
        << " in " << func << " with message '"
        << msg << "'";
    return str;
}

static void TestAssertHandler(const std::string& file,
                              int line,
                              const std::string& func,
                              const std::string& cond,
                              const std::string& msg)
{
    // Determine whether we can safely throw an exception to just make the test
    // fail or whether we need to abort (in this case "msg" will contain the
    // explanation why did we decide to do it).
    wxString abortReason;

    const wxString
        assertMessage = FormatAssertMessage(file, line, func, cond, msg);

    if ( !wxIsMainThread() )
    {
        // Exceptions thrown from worker threads are not caught currently and
        // so we'd just die without any useful information -- abort instead.
        abortReason << assertMessage << " in a worker thread.";
    }
    else if ( std::uncaught_exceptions() )
    {
        // Throwing while already handling an exception would result in
        // terminate() being called and we wouldn't get any useful information
        // about why the test failed then.
        if ( s_lastAssertMessage.empty() )
        {
            abortReason << assertMessage
                        << "while handling an exception";
        }
        else // In this case the exception is due to a previous assert.
        {
            abortReason << s_lastAssertMessage
                        << "\n  and another " << assertMessage
                        << " while handling it.";
        }
    }
    else // Can "safely" throw from here.
    {
        // Remember this in case another assert happens while handling this
        // exception: we want to show the original assert as it's usually more
        // useful to determine the real root of the problem.
        s_lastAssertMessage = assertMessage;

        throw TestAssertFailure(file, line, func, cond, msg);
    }

#if wxUSE_STACKWALKER
    const wxString& stackTrace = wxApp::GetValidTraits().GetAssertStackTrace();
    if ( !stackTrace.empty() )
        abortReason << "\n\nAssert call stack:\n" << stackTrace;
#endif // wxUSE_STACKWALKER

    wxFputs(abortReason, stderr);
    fflush(stderr);
    _exit(-1);
}

#endif // wxDEBUG_LEVEL

#ifdef wxUSE_VC_CRTDBG

namespace 
{

int TestCrtReportHook(int reportType, char *message, int *)
{
    if ( reportType != _CRT_ASSERT )
        return FALSE;

    throw CrtAssertFailure(message);
}

} // namespace anonymous

#endif // wxUSE_VC_CRTDBG

int main(int argc, char** argv)
{

    boost::nowide::args a(argc, argv);
    // tests can be ran non-interactively so make sure we don't show any assert
    // dialog boxes -- neither our own nor from MSVC debug CRT -- which would
    // prevent them from completing

#if wxDEBUG_LEVEL
    wxSetAssertHandler(TestAssertHandler);
#endif // wxDEBUG_LEVEL

#ifdef wxUSE_VC_CRTDBG
    _CrtSetReportHook(TestCrtReportHook);
#endif // wxUSE_VC_CRTDBG

    wxEntryStart(argc, argv);
    wxTheApp->CallOnInit();

    int res = wxTheApp->OnRun();

    namespace ut = boost::ut;

    const auto result = ut::cfg<ut::override>.run(
        { .report_errors =
             true });  // explicitly run registered test suites and report errors
    std::cout << "After report\n";

    wxTheApp->OnExit();
    wxEntryCleanup();

    return res + result; // the result from doctest is propagated here as well
}

// Init
//
bool TestApp::OnInit()
{
    // Hack: don't call TestAppBase::OnInit() to let CATCH handle command line.
    // Output some important information about the test environment.

    static constexpr std::string_view TestTypeStr =
#if wxUSE_GUI
        "GUI";
#else
        "non-GUI";
#endif

    fmt::print("Test program for wxWidgets {} features\n"
               "build: {}\n"
               "running under {} as {}, locale is {}\n",
        TestTypeStr,
        WX_BUILD_OPTIONS_SIGNATURE,
        wxGetOsDescription(),
        wxGetUserId().ToStdString(),
        setlocale(LC_ALL, nullptr)
        );

#if wxUSE_GUI
    // create a parent window to be used as parent for the GUI controls
    new wxTestableFrame();

    Connect(wxEVT_IDLE, wxIdleEventHandler(TestApp::OnIdle));

#ifdef __WXGTK20__
    g_log_set_default_handler(wxTestGLogHandler, nullptr);
#endif // __WXGTK__

#ifdef GDK_WINDOWING_X11
    XSetErrorHandler(wxTestX11ErrorHandler);
#endif // GDK_WINDOWING_X11

#endif // wxUSE_GUI

    return true;
}

// Event handling
int TestApp::FilterEvent(wxEvent& event)
{
    if ( m_filterEventFunc )
        return (*m_filterEventFunc)(event);

    return TestAppBase::FilterEvent(event);
}

bool TestApp::ProcessEvent(wxEvent& event)
{
    if ( m_processEventFunc )
        return (*m_processEventFunc)(event);

    return TestAppBase::ProcessEvent(event);
}

// Run
//
int TestApp::RunTests()
{
#if wxUSE_LOG
    // Switch off logging to avoid interfering with the tests output unless
    // WXTRACE is set, as otherwise setting it would have no effect while
    // running the tests.
    if ( !wxGetEnv("WXTRACE"), nullptr) 
        wxLog::EnableLogging(false);
    else
        wxLog::SetTimestamp("%Y-%m-%d %H:%M:%S.%l");
#endif

    m_context.applyCommandLine(argc, argv);

    // overrides
    m_context.setOption("no-breaks", true);             // don't break in the debugger when assertions fail

    return m_context.run();
}

int TestApp::OnExit()
{
#if wxUSE_GUI
    delete GetTopWindow();
#endif // wxUSE_GUI
    if (m_context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
        return TestAppBase::OnExit();
}
