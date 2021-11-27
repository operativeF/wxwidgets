module;

#include "doctest.h"

#include "wx/app.h"
#include "wx/apptrait.h"
#include "wx/event.h"
#include "wx/utils.h"
#include "wx/window.h"
#include "wx/wxcrtvararg.h"

#include <clocale>

export module WX.Test.Prec;

import Utils.Strings;

import <string>;

export
{

// thrown when assert fails in debug build
class TestAssertFailure
{
public:
    TestAssertFailure(const wxString& file,
                      int line,
                      const wxString& func,
                      const wxString& cond,
                      const wxString& msg)
        : m_file(file),
          m_line(line),
          m_func(func),
          m_cond(cond),
          m_msg(msg)
    {
    }

    const wxString m_file;
    const int m_line;
    const wxString m_func;
    const wxString m_cond;
    const wxString m_msg;

    TestAssertFailure& operator=(const TestAssertFailure&) = delete;
};

// these functions can be used to hook into wxApp event processing and are
// currently used by the events propagation test
typedef int (*FilterEventFunc)(wxEvent&);
typedef bool (*ProcessEventFunc)(wxEvent&);

#if wxUSE_GUI
    typedef wxApp TestAppBase;
    typedef wxGUIAppTraits TestAppTraitsBase;
#else
    typedef wxAppConsole TestAppBase;
    typedef wxConsoleAppTraits TestAppTraitsBase;
#endif

// The application class
//
class TestApp : public TestAppBase
{
public:
    TestApp();

    // standard overrides
    bool OnInit() override;
    int  OnExit() override;

#ifdef __WIN32__
    wxAppTraits *CreateTraits() override
    {
        // Define a new class just to customize CanUseStderr() behaviour.
        class TestAppTraits : public TestAppTraitsBase
        {
        public:
            // We want to always use stderr, tests are also run unattended and
            // in this case we really don't want to show any message boxes, as
            // wxMessageOutputBest, used e.g. from the default implementation
            // of wxApp::OnUnhandledException(), would do by default.
            bool CanUseStderr() override { return true; }

            // Overriding CanUseStderr() is not enough, we also need to
            // override this one to avoid returning false from it.
            bool WriteToStderr(const std::string& text) override
            {
                wxFputs(text, stderr);
                fflush(stderr);

                // Intentionally ignore any errors, we really don't want to
                // show any message boxes in any case.
                return true;
            }
        };

        return new TestAppTraits;
    }
#endif // __WIN32__

    // Also override this method to avoid showing any dialogs from here -- and
    // show some details about the exception along the way.
    bool OnExceptionInMainLoop() override
    {
        wxFprintf(stderr, wxASCII_STR("Unhandled exception in the main loop: %s\n"),
                  wxASCII_STR("Nil")); // FIXME: Doctest

        throw;
    }

    // used by events propagation test
    int FilterEvent(wxEvent& event) override;
    bool ProcessEvent(wxEvent& event) override;

    void SetFilterEventFunc(FilterEventFunc f) { m_filterEventFunc = f; }
    void SetProcessEventFunc(ProcessEventFunc f) { m_processEventFunc = f; }

    // In console applications we run the tests directly from the overridden
    // OnRun(), but in the GUI ones we run them when we get the first call to
    // our EVT_IDLE handler to ensure that we do everything from inside the
    // main event loop. This is especially important under wxOSX/Cocoa where
    // the main event loop is different from the others but it's also safer to
    // do it like this in the other ports as we test the GUI code in the same
    // context as it's used usually, in normal programs, and it might behave
    // differently without the event loop.
#if wxUSE_GUI
    void OnIdle(wxIdleEvent& event)
    {
        if ( m_runTests )
        {
            m_runTests = false;

#ifdef __WXOSX__
            // we need to wait until the window is activated and fully ready
            // otherwise no events can be posted
            wxEventLoopBase* const loop = wxEventLoop::GetActive();
            if ( loop )
            {
                loop->DispatchTimeout(1000);
                loop->EvtYield();
            }
#endif // __WXOSX__

            m_exitcode = RunTests();
            ExitMainLoop();
        }

        event.Skip();
    }

    int OnRun() override
    {
        if ( TestAppBase::OnRun() != 0 )
            m_exitcode = EXIT_FAILURE;

        return m_exitcode;
    }
#else // !wxUSE_GUI
    int OnRun() override
    {
        return RunTests();
    }
#endif // wxUSE_GUI/!wxUSE_GUI

private:
    int RunTests();

    // flag telling us whether we should run tests from our EVT_IDLE handler
    bool m_runTests;

    // event handling hooks
    FilterEventFunc m_filterEventFunc;
    ProcessEventFunc m_processEventFunc;

#if wxUSE_GUI
    // the program exit code
    int m_exitcode;
#endif // wxUSE_GUI

    doctest::Context m_context;
};

wxIMPLEMENT_APP_NO_MAIN(TestApp);

// ----------------------------------------------------------------------------
// global functions
// ----------------------------------------------------------------------------

inline void SetFilterEventFunc(FilterEventFunc func)
{
    wxGetApp().SetFilterEventFunc(func);
}

inline void SetProcessEventFunc(ProcessEventFunc func)
{
    wxGetApp().SetProcessEventFunc(func);
}

#if wxUSE_SOCKETS
inline bool IsNetworkAvailable()
{
    // Somehow even though network is available on Travis CI build machines,
    // attempts to open remote URIs sporadically fail, so don't run these tests
    // under Travis to avoid false positives.
    static int s_isTravis = -1;
    if ( s_isTravis == -1 )
        s_isTravis = wxGetEnv(wxASCII_STR("TRAVIS"), nullptr);

    if ( s_isTravis )
        return false;

    // NOTE: we could use wxDialUpManager here if it was in wxNet; since it's in
    //       wxCore we use a simple rough test:

    wxSocketBase::Initialize();

    wxIPV4address addr;
    if (!addr.Hostname(wxASCII_STR("www.google.com")) || !addr.Service(wxASCII_STR("www")))
    {
        wxSocketBase::Shutdown();
        return false;
    }

    wxSocketClient sock;
    sock.SetTimeout(10);    // 10 secs
    bool online = sock.Connect(addr);

    wxSocketBase::Shutdown();

    return online;
}
#endif

inline bool IsAutomaticTest()
{
    static int s_isAutomatic = -1;
    if ( s_isAutomatic == -1 )
    {
        // Allow setting an environment variable to emulate buildslave user for
        // testing.
        std::string username;
        if ( !wxGetEnv("WX_TEST_USER", &username) )
            username = wxGetUserId();

        wx::utils::ToLower(username);
        s_isAutomatic = username == "buildbot" ||
                        username == "sandbox*";

        // Also recognize various CI environments.
        if ( !s_isAutomatic )
        {
            s_isAutomatic = wxGetEnv("TRAVIS", nullptr) ||
                              wxGetEnv("GITHUB_ACTIONS", nullptr) ||
                                wxGetEnv("APPVEYOR", nullptr);
        }
    }

    return s_isAutomatic == 1;
}

inline bool IsRunningUnderXVFB()
{
    static int s_isRunningUnderXVFB = -1;
    if ( s_isRunningUnderXVFB == -1 )
    {
        std::string value;
        s_isRunningUnderXVFB = wxGetEnv("wxUSE_XVFB", &value) && value == "1";
    }

    return s_isRunningUnderXVFB == 1;
}

#ifdef __LINUX__

inline bool IsRunningInLXC()
{
    // We're supposed to be able to detect running in LXC by checking for
    // /dev/lxd existency, but this doesn't work under Travis for some reason,
    // so just rely on having the environment variable defined for the
    // corresponding builds in our .travis.yml.
    wxString value;
    return wxGetEnv("wxLXC", &value) && value == "1";
}

#endif // __LINUX__

#if wxUSE_GUI

inline bool EnableUITests()
{
    static int s_enabled = -1;
    if ( s_enabled == -1 )
    {
        // Allow explicitly configuring this via an environment variable under
        // all platforms.
        std::string enabled;
        if ( wxGetEnv(wxASCII_STR("WX_UI_TESTS"), &enabled) )
        {
            if ( enabled == "1" )
                s_enabled = 1;
            else if ( enabled == "0" )
                s_enabled = 0;
            else
                wxFprintf(stderr, wxASCII_STR("Unknown \"WX_UI_TESTS\" value \"%s\" ignored.\n"), enabled);
        }

        if ( s_enabled == -1 )
        {
#if defined(__WXMSW__) || defined(__WXGTK__)
            s_enabled = 1;
#else // !(__WXMSW__ || __WXGTK__)
            s_enabled = 0;
#endif // (__WXMSW__ || __WXGTK__)
        }
    }

    return s_enabled == 1;
}

inline void DeleteTestWindow(wxWindow* win)
{
    if ( !win )
        return;

    wxWindow* const capture = wxWindow::GetCapture();
    if ( capture )
    {
        if ( capture == win ||
                capture->GetMainWindowOfCompositeControl() == win )
            capture->ReleaseMouse();
    }

    delete win;
}

#ifdef __WXGTK__

#ifdef GDK_WINDOWING_X11

#include "X11/Xlib.h"

extern "C"
inline int wxTestX11ErrorHandler(Display*, XErrorEvent*)
{
    fprintf(stderr, "\n*** X11 error while running %s(): ",
            wxGetCurrentTestName().c_str());
    return 0;
}

#endif // GDK_WINDOWING_X11

extern "C"
inline void
wxTestGLogHandler(const gchar* domain,
                  GLogLevelFlags level,
                  const gchar* message,
                  gpointer data)
{
    // Check if debug messages in this domain will be logged.
    if ( level == G_LOG_LEVEL_DEBUG )
    {
        static const char* const allowed = getenv("G_MESSAGES_DEBUG");

        // By default debug messages are dropped, but if G_MESSAGES_DEBUG is
        // defined, they're logged for the domains specified in it and if it
        // has the special value "all", then all debug messages are shown.
        //
        // Note that the check here can result in false positives, e.g. domain
        // "foo" would pass it even if G_MESSAGES_DEBUG only contains "foobar",
        // but such cases don't seem to be important enough to bother
        // accounting for them.
        if ( !allowed ||
                (strcmp(allowed, "all") != 0 && !strstr(allowed, domain)) )
        {
            return;
        }
    }

    fprintf(stderr, "\n*** GTK log message while running %s(): ",
            wxGetCurrentTestName().c_str());

    g_log_default_handler(domain, level, message, data);
}

#endif // __WXGTK__

#endif // wxUSE_GUI

// Helper class setting the locale to the given one for its lifetime.
class LocaleSetter
{
public:
    LocaleSetter(const char *loc)
        : m_locOld(wxStrdupA(setlocale(LC_ALL, nullptr)))
    {
        setlocale(LC_ALL, loc);
    }

    ~LocaleSetter()
    {
        setlocale(LC_ALL, m_locOld);
        free(m_locOld);
    }

private:
    char * const m_locOld;

    LocaleSetter(const LocaleSetter&) = delete;
	LocaleSetter& operator=(const LocaleSetter&) = delete;
};

// An even simpler helper for setting the locale to "C" one during its lifetime.
class CLocaleSetter : private LocaleSetter
{
public:
    CLocaleSetter() : LocaleSetter("C") { }

private:
    CLocaleSetter(const CLocaleSetter&) = delete;
	CLocaleSetter& operator=(const CLocaleSetter&) = delete;
};

} // export
