///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/metatest.cpp
// Purpose:     Test template meta-programming constructs
// Author:      Jaakko Salli
// Copyright:   (c) the wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/object.h"
#include "wx/utils.h"
#include "wx/meta/pod.h"
#include "wx/meta/movable.h"

#ifndef wxNO_RTTI
#include <typeinfo>
#endif

TEST_CASE("IsPod")
{
    CHECK(wxIsPod<bool>::value);
    CHECK(wxIsPod<signed int>::value);
    CHECK(wxIsPod<double>::value);
    CHECK(wxIsPod<wxObject*>::value);
    CHECK(!wxIsPod<wxObject>::value);
}

TEST_CASE("IsMovable")
{
    CHECK(wxIsMovable<bool>::value);
    CHECK(wxIsMovable<signed int>::value);
    CHECK(wxIsMovable<double>::value);
    CHECK(wxIsMovable<wxObject*>::value);
    CHECK(!wxIsMovable<wxObject>::value);
}

TEST_CASE("ImplicitConversion")
{
#ifndef wxNO_RTTI
    CHECK(typeid(wxImplicitConversionType<char,int>::value) == typeid(int));
    CHECK(typeid(wxImplicitConversionType<int,unsigned>::value) == typeid(unsigned));
#ifdef wxLongLong_t
    CHECK(typeid(wxImplicitConversionType<wxLongLong_t,float>::value) == typeid(float));
#endif
#endif // !wxNO_RTTI
}

TEST_CASE("MinMax")
{
    // test that wxMax(1.1,1) returns float, not long int
    float f = wxMax(1.1f, 1l);
    CHECK_EQ( 1.1f, f);

    // test that comparing signed and unsigned correctly returns unsigned: this
    // may seem counterintuitive in this case but this is consistent with the
    // standard C conversions
    CHECK_EQ( 1, wxMin(-1, 1u) );

    CHECK_EQ( -1., wxClip(-1.5, -1, 1) );
    CHECK_EQ( 0, wxClip(0, -1, 1) );
    CHECK_EQ( 1, wxClip(2l, -1, 1) );
}
