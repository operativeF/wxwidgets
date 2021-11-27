#ifndef WX_TESTPREC_INCLUDED
#define WX_TESTPREC_INCLUDED 1

#include "wx/window.h"

#include <clocale>

// define wxHAVE_U_ESCAPE if the compiler supports \uxxxx character constants
#if defined(__VISUALC__) || defined(__GNUC__)
    #define wxHAVE_U_ESCAPE
#endif

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
class wxEvent;

typedef int (*FilterEventFunc)(wxEvent&);
typedef bool (*ProcessEventFunc)(wxEvent&);

extern void SetFilterEventFunc(FilterEventFunc func);
extern void SetProcessEventFunc(ProcessEventFunc func);

extern bool IsNetworkAvailable();

extern bool IsAutomaticTest();

extern bool IsRunningUnderXVFB();

#ifdef __LINUX__
extern bool IsRunningInLXC();
#endif // __LINUX__

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

#if wxUSE_GUI

// Return true if the UI tests are enabled, used by WXUISIM_TEST().
extern bool EnableUITests();

// Helper function deleting the window without asserts (and hence exceptions
// thrown from its dtor!) even if it has mouse capture.
void DeleteTestWindow(wxWindow* win);

#endif // wxUSE_GUI

// Convenience macro which registers a test case using just its "base" name,
// i.e. without the common "TestCase" suffix, as its tag.
#define wxREGISTER_UNIT_TEST(testclass) \
    wxREGISTER_UNIT_TEST_WITH_TAGS(testclass ## TestCase, "[" #testclass "]")

#endif
