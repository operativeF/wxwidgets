///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/misctests.cpp
// Purpose:     test miscellaneous stuff
// Author:      Peter Most, Vadim Zeitlin
// Created:     2008-07-10
// Copyright:   (c) 2008 Peter Most
//              (c) 2009 Vadim Zeitlin
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/defs.h"

#include "wx/math.h"

// just some classes using wxRTTI for wxStaticCast() test
#include "wx/tarstrm.h"
#include "wx/zipstrm.h"


static bool AssertIfOdd(int n)
{
    wxCHECK_MSG(!(n % 2), false, "parameter must be even");

    return true;
}

TEST_CASE("Assert")
{
    AssertIfOdd(0);
    CHECK_THROWS(AssertIfOdd(1));

    // doesn't fail any more
    wxAssertHandler_t oldHandler = wxSetAssertHandler(nullptr);
    AssertIfOdd(17);
    wxSetAssertHandler(oldHandler);
}

TEST_CASE("CallForEach")
{
    #define MY_MACRO(pos, str) s += str;

    wxString s;
    wxCALL_FOR_EACH(MY_MACRO, "foo", "bar", "baz");

    CHECK_EQ( "foobarbaz", s );

    #undef MY_MACRO
}

TEST_CASE("Delete")
{
    // Allocate some arbitrary memory to get a valid pointer:
    long *pointer = new long;
    CHECK( pointer != nullptr );

    // Check that wxDELETE sets the pointer to nullptr:
    wxDELETE( pointer );
    CHECK( pointer == nullptr );

    // Allocate some arbitrary array to get a valid pointer:
    long *array = new long[ 3 ];
    CHECK( array != nullptr );

    // Check that wxDELETEA sets the pointer to nullptr:
    wxDELETEA( array );
    CHECK( array == nullptr );
}

// helper function used just to avoid warnings about value computed not being
// used in WX_ASSERT_FAILS_WITH_ASSERT() in StaticCast() below
static bool IsNull(void *p)
{
    return p == nullptr;
}

TEST_CASE("wxCTZ")
{
    CHECK( wxCTZ(1) == 0 );
    CHECK( wxCTZ(4) == 2 );
    CHECK( wxCTZ(17) == 0 );
    CHECK( wxCTZ(0x80000000) == 31 );

    //WX_ASSERT_FAILS_WITH_ASSERT( wxCTZ(0) );
}

TEST_CASE("wxRound")
{
    CHECK( wxRound(2.3) == 2 );
    CHECK( wxRound(3.7) == 4 );
    CHECK( wxRound(-0.5f) == -1 );

    CHECK_THROWS( wxRound(2.0*INT_MAX) );
    CHECK_THROWS( wxRound(1.1*INT_MIN) );
}
