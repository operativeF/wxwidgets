///////////////////////////////////////////////////////////////////////////////
// Name:        tests/allheaders.cpp
// Purpose:     Compilation test for all headers
// Author:      Vadim Zeitlin, Arrigo Marchiori
// Created:     2020-04-20
// Copyright:   (c) 2010,2020 Vadim Zeitlin, Wlodzimierz Skiba, Arrigo Marchiori
///////////////////////////////////////////////////////////////////////////////

// Note: can't use wxCHECK_GCC_VERSION() here as it's not defined yet.
#if defined(__GNUC__)
    #define CHECK_GCC_VERSION(major, minor) \
      (__GNUC__ > (major) || (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
#else
    #define CHECK_GCC_VERSION(major, minor) 0
#endif

#if CHECK_GCC_VERSION(4, 6)
    // As above, we can't reuse wxCONCAT() and wxSTRINGIZE macros from wx/cpp.h
    // here, so define their equivalents here.
    #define CONCAT_HELPER(x, y) x ## y
    #define CONCAT(x1, x2)      CONCAT_HELPER(x1, x2)

    #define STRINGIZE_HELPER(x) #x
    #define STRINGIZE(x)        STRINGIZE_HELPER(x)

    #define GCC_TURN_ON(warn) \
        _Pragma(STRINGIZE(GCC diagnostic error STRINGIZE(CONCAT(-W,warn))))
    #define GCC_TURN_OFF(warn) \
        _Pragma(STRINGIZE(GCC diagnostic ignored STRINGIZE(CONCAT(-W,warn))))
#endif

#if defined(__WXMSW__)
    #include <windows.h>

    // Avoid warnings about redeclaring standard functions such as chmod() in
    // various standard headers when using MinGW/Cygwin.
    #if defined(__MINGW32__) || defined(__CYGWIN__)
        #include <stdio.h>
        #include <unistd.h>
        #include <sys/stat.h>
        #include <io.h>
    #endif
#elif defined(__WXQT__)
    #include <QtGui/QFont>
#endif

// ANSI build hasn't been updated to work without implicit wxString encoding
// and never will be, as it will be removed soon anyhow. And in UTF-8-only
// build we actually want to use implicit encoding (UTF-8).
#if !wxUSE_UTF8_LOCALE_ONLY
#define wxNO_IMPLICIT_WXSTRING_ENCODING
#endif

#include "testprec.h"

#include "allheaders.h"

// Check that using wx macros doesn't result in -Wsuggest-override or
// equivalent warnings in classes using and not using "override".
struct Base : wxEvtHandler
{
    virtual ~Base() { }

    virtual void Foo() { }
};

struct DerivedWithoutOverride : Base
{
    void OnIdle(wxIdleEvent&) { }

	DerivedWithoutOverride(const DerivedWithoutOverride&) = delete;
	DerivedWithoutOverride& operator=(const DerivedWithoutOverride&) = delete;

	wxClassInfo *wxGetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
    wxDECLARE_EVENT_TABLE();
};

struct DerivedWithOverride : Base
{
    void Foo() override { }

	DerivedWithOverride(const DerivedWithOverride&) = delete;
	DerivedWithOverride& operator=(const DerivedWithOverride&) = delete;

    void OnIdle(wxIdleEvent&) { }

	wxClassInfo *wxGetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
    wxDECLARE_EVENT_TABLE();
};

TEST_CASE("wxNO_IMPLICIT_WXSTRING_ENCODING", "[string]")
{
    wxString s = wxASCII_STR("Hello, ASCII");
    REQUIRE(s == L"Hello, ASCII");
#ifdef TEST_IMPLICIT_WXSTRING_ENCODING
    // Compilation of this should fail, because the macro
    // wxNO_IMPLICIT_WXSTRING_ENCODING must be set
    s = "Hello, implicit encoding";
#endif
}
