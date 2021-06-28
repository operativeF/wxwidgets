///////////////////////////////////////////////////////////////////////////////
// Name:        tests/thread/tls.cpp
// Purpose:     wxTlsValue unit test
// Author:      Vadim Zeitlin
// Created:     2008-08-28
// Copyright:   (c) 2008 Vadim Zeitlin
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "doctest.h"

#include "testprec.h"

#ifndef WX_PRECOMP
#endif // WX_PRECOMP

#include "wx/thread.h"
#include "wx/tls.h"

#include <string>

// ----------------------------------------------------------------------------
// globals
// ----------------------------------------------------------------------------

namespace
{

// NB: this struct must be a POD, so don't use wxString as its member
struct PerThreadData
{
    const char *name;
    int number;
};

wxTLS_TYPE(PerThreadData) gs_threadDataVar;
#define gs_threadData wxTLS_VALUE(gs_threadDataVar)

wxTLS_TYPE(int) gs_threadIntVar;
#define gs_threadInt wxTLS_VALUE(gs_threadIntVar)

// ----------------------------------------------------------------------------
// test thread
// ----------------------------------------------------------------------------

// this thread arbitrarily modifies all global thread-specific variables to
// make sure that the changes in it are not visible from the main thread
class TLSTestThread : public wxThread
{
public:
    // ctor both creates and starts the thread
    TLSTestThread() : wxThread(wxTHREAD_JOINABLE) { Create(); Run(); }

    void *Entry() override
    {
        gs_threadInt = 17;

        gs_threadData.name = "worker";
        gs_threadData.number = 2;

        // We can't use Catch asserts outside of the main thread,
        // unfortunately.
        wxASSERT( gs_threadData.name == std::string("worker") );
        wxASSERT( gs_threadData.number == 2 );

        return nullptr;
    }
};

} // anonymous namespace

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

TEST_CASE("TestInt")
{
    CHECK_EQ( 0, gs_threadInt );

    gs_threadInt++;
    CHECK_EQ( 1, gs_threadInt );

    TLSTestThread().Wait();

    CHECK_EQ( 1, gs_threadInt );
}

TEST_CASE("TestStruct")
{
    CHECK_EQ( nullptr, gs_threadData.name );
    CHECK_EQ( 0, gs_threadData.number );

    gs_threadData.name = "main";
    gs_threadData.number = 1;

    CHECK_EQ( 1, gs_threadData.number );

    TLSTestThread().Wait();

    CHECK_EQ( std::string("main"), gs_threadData.name );
    CHECK_EQ( 1, gs_threadData.number );
}

