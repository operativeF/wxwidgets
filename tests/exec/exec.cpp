///////////////////////////////////////////////////////////////////////////////
// Name:        tests/exec/exec.cpp
// Purpose:     test wxExecute()
// Author:      Francesco Montorsi
//              (based on console sample TestExecute() function)
// Created:     2009-01-10
// Copyright:   (c) 2009 Francesco Montorsi
//              (c) 2013 Rob Bresalier, Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#include "wx/utils.h"
#include "wx/process.h"
#include "wx/sstream.h"
#include "wx/evtloop.h"
#include "wx/file.h"
#include "wx/filename.h"
#include "wx/mstream.h"
#include "wx/scopeguard.h"
#include "wx/txtstrm.h"
#include "wx/timer.h"

#ifdef __UNIX__
    #define COMMAND "echo hi"
    #define COMMAND_STDERR "cat nonexistentfile"
    #define ASYNC_COMMAND "xclock"
    #define SHELL_COMMAND "echo hi from shell>/dev/null"
    #define COMMAND_NO_OUTPUT "echo -n"
#elif defined(WX_WINDOWS)
    #define COMMAND "cmd.exe /c \"echo hi\""
    #define COMMAND_STDERR "cmd.exe /c \"type nonexistentfile\""
    #define ASYNC_COMMAND "notepad"
    #define SHELL_COMMAND "echo hi > nul:"
    #define COMMAND_NO_OUTPUT COMMAND " > nul:"
#else
    #error "no command to exec"
#endif // OS

#define SLEEP_END_STRING "Done sleeping"


enum AsyncExecLoopExitEnum
{
    AsyncExec_DontExitLoop,
    AsyncExec_ExitLoop
};

// Helper of TestAsyncRedirect(): tests that the output of the given
// command on the given stream contains the expected string.
enum CheckStream { Check_Stdout, Check_Stderr };

// This class is used as a helper in order to run wxExecute(ASYNC)
// inside of an event loop.
class AsyncInEventLoop : public wxTimer
{
public:
    AsyncInEventLoop() { }

    long DoExecute(AsyncExecLoopExitEnum forceExitLoop_,
        const wxString& command_,
        int flags_ = wxEXEC_ASYNC,
        wxProcess* callback_ = nullptr)
    {
        forceExitLoop = forceExitLoop_;
        command = command_;
        flags = flags_;
        callback = callback_;

        wxEventLoop loop;

        // Trigger the timer to go off inside the event loop
        // so that we can run wxExecute there.
        StartOnce(10ms);

        // Run the event loop.
        loop.Run();

        return wxExecuteReturnCode;
    }

    void Notify() override
    {
        // Run wxExecute inside the event loop.
        wxExecuteReturnCode = wxExecute(command, flags, callback);

        if (forceExitLoop == AsyncExec_ExitLoop)
        {
            wxEventLoop::GetActive()->Exit();
        }
    }

private:
    AsyncExecLoopExitEnum forceExitLoop;
    wxString command;
    int flags;
    wxProcess* callback;
    long wxExecuteReturnCode;
};

// This class exits the event loop associated with it when the child process
// terminates.
class TestAsyncProcess : public wxProcess
{
public:
    explicit TestAsyncProcess()
    {
    }

    // may be overridden to be notified about process termination
    void OnTerminate(int WXUNUSED(pid), int WXUNUSED(status)) override
    {
        wxEventLoop::GetActive()->ScheduleExit();
    }

private:
    TestAsyncProcess(const TestAsyncProcess&) = delete;
    TestAsyncProcess& operator=(const TestAsyncProcess&) = delete;
};

static void DoTestAsyncRedirect(const wxString& command,
                                CheckStream check,
                                const char* expectedContaining)
{
    AsyncInEventLoop asyncInEventLoop;
    TestAsyncProcess proc;

    proc.Redirect();

    CHECK(asyncInEventLoop.DoExecute(
        AsyncExec_DontExitLoop,  // proc is expected (inside of its OnTerminate())
                // to trigger the exit of the event loop.
        command, wxEXEC_ASYNC, &proc) != 0);

    wxInputStream* streamToCheck = nullptr;
    switch (check)
    {
    case Check_Stdout:
        streamToCheck = proc.GetInputStream();
        break;

    case Check_Stderr:
        streamToCheck = proc.GetErrorStream();
        break;
    }

    wxTextInputStream tis(*streamToCheck);

    // Check that the first line of output contains what we expect.
    CHECK(tis.ReadLine().Contains(expectedContaining));
}

[[maybe_unused]] static wxString CreateSleepFile(const wxString& basename, int seconds)
{
#ifdef __UNIX__
    static const char* const scriptExt = ".sh";

    // The script text is a format string with a single "%d" appearing in it
    // which will be replaced by the number of seconds to sleep below.
    static const char* const scriptText =
        "sleep %d\n"
        "echo " SLEEP_END_STRING "\n";
#elif defined(WX_WINDOWS)
    static const char* const scriptExt = ".bat";

    // Notice that we need to ping N+1 times for it to take N seconds as the
    // first ping is sent out immediately, without waiting a second.
    static const char* const scriptText =
        "@ ping 127.0.0.1 -n 1 > nul\n"
        "@ ping 127.0.0.1 -n %d > nul\n"
        "@ echo " SLEEP_END_STRING "\n";
#else
#error "Need code to create sleep file for this platform"
#endif

    const wxString fnSleep = wxFileName(".", basename, scriptExt).GetFullPath();

    wxFile fileSleep;
    CHECK
    (
        fileSleep.Create(fnSleep, true, wxS_IRUSR | wxS_IWUSR | wxS_IXUSR)
    );

    fileSleep.Write(wxString::Format(scriptText, seconds));

    return fnSleep;
}

[[maybe_unused]] static wxString MakeShellCommand(const wxString& filename)
{
    wxString command;

#ifdef __UNIX__
    command = "/bin/sh " + filename;
#elif defined(WX_WINDOWS)
    command = wxString::Format("cmd.exe /c \"%s\"", filename);
#else
#error "Need to code to launch shell for this platform"
#endif

    return command;
}

TEST_CASE("TestShell")
{
    CHECK( wxShell(SHELL_COMMAND) );
}

TEST_CASE("TestExecute")
{
    // Launching interactive programs doesn't work without an interactive
    // session.
    if ( IsAutomaticTest() )
        return;

    AsyncInEventLoop asyncInEventLoop;

    // test asynch exec
    //
    // asyncInEventLoop.DoExecute will perform the
    // call to wxExecute(ASYNC) in an event loop, as required by
    // console test (and this same event loop will also
    // be used in GUI test too, even though not required, just to have
    // common code).
    long pid = asyncInEventLoop.DoExecute(AsyncExec_ExitLoop, // Force exit of event loop right
                                                // after the call to wxExecute()
                                          ASYNC_COMMAND, wxEXEC_ASYNC);
    CHECK( pid != 0 );

    // Give the system some time to launch the child.
    wxMilliSleep(200);

    // Try to terminate it gently first, but fall back to killing it
    // unconditionally if this fails.
    const int rc = wxKill(pid, wxSIGTERM);
    CHECK( rc == 0 );
    if ( rc != 0 )
        CHECK( wxKill(pid, wxSIGKILL) == 0 );

    int useNoeventsFlag;

    // Test the sync execution case with/without wxEXEC_NOEVENTS flag
    // because we use either an event loop or wxSelectDispatcher
    // depending on this flag, and we want to test both cases.
    for (useNoeventsFlag = 0; useNoeventsFlag <=1 ; ++useNoeventsFlag )
    {
        int execFlags = wxEXEC_SYNC;

        if (useNoeventsFlag)
        {
            execFlags |= wxEXEC_NOEVENTS;
        }

        // test sync exec (with a command not producing any output to avoid
        // interfering with the test):
        CHECK( wxExecute(COMMAND_NO_OUTPUT, execFlags) == 0 );

        // test running COMMAND again, but this time with redirection:
        // and the expected data is on stdout.
        std::vector<wxString> stdout_arr;
        CHECK_EQ( 0, wxExecute(COMMAND, stdout_arr, execFlags) );
        CHECK_EQ( "hi", stdout_arr[0] );

        // test running COMMAND_STDERR with redirection and the expected data
        // is on stderr.
        std::vector<wxString> stderr_arr;
        stdout_arr.clear();
        CHECK( wxExecute(COMMAND_STDERR, stdout_arr, stderr_arr, execFlags) != 0 );

        // Check that there is something on stderr.
        // In Unix systems, the 'cat' command has the name of the file it could not
        // find in the error output.
        // In Windows, the 'type' command outputs the following when it can't find
        // a file:
        // "The system cannot find the file specified"
        // In both cases, we expect the word 'file' to be in the stderr.
        CHECK( stderr_arr[0].Contains("file") );
    }
}

TEST_CASE("TestProcess")
{
    if ( IsAutomaticTest() )
        return;

    AsyncInEventLoop asyncInEventLoop;

    // test wxExecute with wxProcess
    wxProcess *proc = new wxProcess;

    // asyncInEventLoop.DoExecute will perform the
    // call to wxExecute(ASYNC) in an event loop, as required by
    // console test (and this same event loop will also
    // be used in GUI test too, even though not required, just to have
    // common code).
    long pid = asyncInEventLoop.DoExecute(AsyncExec_ExitLoop, // Force exit of event loop right
                                                // after the call to wxExecute()
                                          ASYNC_COMMAND, wxEXEC_ASYNC, proc);
    CHECK( proc->GetPid() == pid );
    CHECK( pid != 0 );

    // As above, give the system time to launch the process.
    wxMilliSleep(200);

    // we're not going to process the wxEVT_END_PROCESS event,
    // so the proc instance will auto-delete itself after we kill
    // the asynch process:
    const int rc = wxKill(pid, wxSIGTERM);
    CHECK( rc == 0 );
    if ( rc != 0 )
        CHECK( wxKill(pid, wxSIGKILL) == 0 );


    // test wxExecute with wxProcess and REDIRECTION

    // Test the sync execution case with/without wxEXEC_NOEVENTS flag
    // because we use either an event loop or wxSelectDispatcher
    // depending on this flag, and we want to test both cases.

    // First the default case, dispatching the events while waiting.
    {
        wxProcess proc2;
        proc2.Redirect();
        CHECK_EQ( 0, wxExecute(COMMAND, wxEXEC_SYNC, &proc2) );

        wxStringOutputStream procOutput;
        CHECK( proc2.GetInputStream() );
        CHECK_EQ( wxSTREAM_EOF,
            proc2.GetInputStream()->Read(procOutput).GetLastError() );

        wxString output = procOutput.GetString();
        CHECK_EQ( "hi", output.Trim() );
    }

    // And now without event dispatching.
    {
        wxProcess proc2;
        proc2.Redirect();
        CHECK_EQ( 0,
            wxExecute(COMMAND, wxEXEC_SYNC | wxEXEC_NOEVENTS, &proc2) );

        wxStringOutputStream procOutput;
        CHECK( proc2.GetInputStream() );
        CHECK_EQ( wxSTREAM_EOF,
            proc2.GetInputStream()->Read(procOutput).GetLastError() );

        wxString output = procOutput.GetString();
        CHECK_EQ( "hi", output.Trim() );
    }
}

TEST_CASE("TestAsync")
{
    // Test asynchronous execution with no redirection, just to make sure we
    // get the OnTerminate() call.
    TestAsyncProcess proc;
    AsyncInEventLoop asyncInEventLoop;

    CHECK( asyncInEventLoop.DoExecute(
                       AsyncExec_DontExitLoop,  // proc is expected (inside of its OnTerminate())
                               // to trigger the exit of the event loop.
                       COMMAND_NO_OUTPUT, wxEXEC_ASYNC, &proc) != 0 );
}

TEST_CASE("TestAsyncRedirect")
{
    // Test redirection with reading from the input stream after process termination.
    DoTestAsyncRedirect(COMMAND, Check_Stdout, "hi");

    // Test redirection with reading from the error stream after process termination.
    DoTestAsyncRedirect(COMMAND_STDERR, Check_Stderr, "file");
}

TEST_CASE("TestOverlappedSyncExecute")
{
    // Windows Synchronous wxExecute implementation does not currently
    // support overlapped event loops.  It is still using wxYield, which is
    // not nestable.  Therefore, this test would fail in Windows.
    // If someday somebody changes that in Windows, they could use this
    // test to verify it.
    //
    // Because MSW is not yet ready for this test, it may make sense to
    // separate it out to its own test suite, so we could register it under
    // "fixme" for Windows, but a real test for Unix.  But that is more work,
    // so just #ifndefing it here for now.
    //
    // Too bad you can't just register one test case of a test suite as a
    // "fixme".
#ifndef WX_WINDOWS
    // Simple helper delaying the call to wxExecute(): instead of running it
    // immediately, it runs it when we re-enter the event loop.
    class DelayedExecuteTimer : public wxTimer
    {
    public:
        DelayedExecuteTimer(const wxString& command, std::vector<wxString>& outputArray)
            : m_command(command),
              m_outputArray(outputArray)
        {
            // The exact delay doesn't matter, anything short enough will do.
            StartOnce(10);
        }

        void Notify() override
        {
            wxExecute(m_command, m_outputArray);
        }

    private:
        wxString m_command;
        std::vector<wxString>& m_outputArray;
    };

    // Create two scripts with one of them taking longer than the other one to
    // execute.
    const wxString shortSleepFile = CreateSleepFile("shortsleep", 1);
    wxON_BLOCK_EXIT1( wxRemoveFile, shortSleepFile );
    const wxString longSleepFile = CreateSleepFile("longsleep", 2);
    wxON_BLOCK_EXIT1( wxRemoveFile, longSleepFile );

    const wxString shortSleepCommand = MakeShellCommand(shortSleepFile);
    const wxString longSleepCommand = MakeShellCommand(longSleepFile);

    // Collect the child process output
    std::vector<wxString> shortSleepOutput;
    std::vector<wxString> longSleepOutput;

    // Test that launching a process taking a longer time to run while the
    // shorter process is running works, i.e. that our outer wxExecute()
    // doesn't return until both process terminate.
    DelayedExecuteTimer delayLongSleep(longSleepCommand, longSleepOutput);
    wxExecute(shortSleepCommand, shortSleepOutput);
    CHECK( !shortSleepOutput.empty() );
    CHECK_EQ( SLEEP_END_STRING, shortSleepOutput.Last() );

    CHECK( !longSleepOutput.empty() );
    CHECK_EQ( SLEEP_END_STRING, longSleepOutput.Last() );

    // And also that, vice versa, running a short-lived child process that both
    // starts and ends while a longer-lived parent process is still running
    // works too.
    DelayedExecuteTimer delayShortSleep(shortSleepCommand, shortSleepOutput);
    wxExecute(longSleepCommand, longSleepOutput);
    CHECK( !shortSleepOutput.empty() );
    CHECK_EQ( SLEEP_END_STRING, shortSleepOutput.Last() );

    CHECK( !longSleepOutput.empty() );
    CHECK_EQ( SLEEP_END_STRING, longSleepOutput.Last() );
#endif // !WX_WINDOWS
}

#ifdef __UNIX__

// This test is disabled by default because it must be run in French locale,
// i.e. with explicit LC_ALL=fr_FR.UTF-8 and only works with GNU ls, which
// produces the expected output.
TEST_CASE("wxExecute::RedirectUTF8", "[exec][unicode][.]")
{
    std::vector<wxString> output;
    REQUIRE( wxExecute("/bin/ls --version", output) == 0 );

    for ( size_t n = 0; n < output.size(); ++n )
    {
        // It seems unlikely that this part of the output will change for GNU
        // ls, so check for its presence as a sign that the program output was
        // decoded correctly.
        if ( output[n].find(wxString::FromUTF8("vous \xc3\xaates libre")) != wxString::npos )
            return;
    }

    INFO("output was:\n" << wxJoin(output, '\n'));
    FAIL("Expected output fragment not found.");
}

#endif // __UNIX__
