///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/dynamiclib.cpp
// Purpose:     Test wxDynamicLibrary
// Author:      Francesco Montorsi (extracted from console sample)
// Created:     2010-06-13
// Copyright:   (c) 2010 wxWidgets team
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/dynlib.h"

#ifdef __UNIX__
    #include "wx/filename.h"
    #include "wx/log.h"
#endif

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

TEST_CASE("Load")
{
#if defined(WX_WINDOWS)
    static constexpr wxChar LIB_NAME[] = wxT("kernel32.dll");
    static constexpr wxChar FUNC_NAME[] = wxT("lstrlenA");
#elif defined(__UNIX__)
#ifdef __DARWIN__
    static constexpr wxChar LIB_NAME[] = wxT("/usr/lib/libc.dylib");
#else
    // weird: using just libc.so does *not* work!
    static constexpr wxChar LIB_NAME[] = wxT("/lib/libc.so.6");
#endif
    static constexpr wxChar FUNC_NAME[] = wxT("strlen");

    if ( !wxFileName::Exists(LIB_NAME) )
    {
        wxLogWarning("Shared library \"%s\" doesn't exist, "
                     "skipping DynamicLibraryTestCase::Load() test.");
        return;
    }
#else
    #error "don't know how to test wxDllLoader on this platform"
#endif

    wxDynamicLibrary lib(LIB_NAME);
    CHECK( lib.IsLoaded() );

    typedef int (wxSTDCALL *wxStrlenType)(const char *);
    wxStrlenType pfnStrlen = (wxStrlenType)lib.GetSymbol(FUNC_NAME);

    wxString errMsg = wxString::Format("ERROR: function '%s' wasn't found in '%s'.\n",
                                       FUNC_NAME, LIB_NAME);
    CHECK_MESSAGE((pfnStrlen != nullptr), errMsg.ToStdString() );

    // Call the function dynamically loaded
    CHECK( pfnStrlen("foo") == 3 );

#ifdef WX_WINDOWS
    static constexpr wxChar FUNC_NAME_AW[] = wxT("lstrlen");

    typedef int (wxSTDCALL *wxStrlenTypeAorW)(const wxChar *);
    wxStrlenTypeAorW
        pfnStrlenAorW = (wxStrlenTypeAorW)lib.GetSymbolAorW(FUNC_NAME_AW);

    wxString errMsg2 = wxString::Format("ERROR: function '%s' wasn't found in '%s'.\n",
                                       FUNC_NAME_AW, LIB_NAME);
    CHECK_MESSAGE((pfnStrlenAorW != nullptr), errMsg2.ToStdString());

    CHECK( pfnStrlenAorW(wxT("foobar")) == 6 );
#endif // WX_WINDOWS
}
