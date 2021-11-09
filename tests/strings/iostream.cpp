//////////////////////////////////////////////////////////////////////////////
// Name:        tests/strings/iostream.cpp
// Purpose:     unit test of wxString interaction with std::[io]stream
// Author:      Vadim Zeitlin
// Created:     2007-10-09
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#ifndef WX_PRECOMP
    #include "wx/string.h"
#endif // WX_PRECOMP

import <sstream>;

#define ASSERT_OSTREAM_EQUAL(p, s) CHECK_EQ(std::string(p), s.str())
#define ASSERT_WOSTREAM_EQUAL(p, s) CHECK_EQ(std::wstring(p), s.str())

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------


TEST_CASE("Out")
{
    std::ostringstream s;
    s << wxString("hello");
    ASSERT_OSTREAM_EQUAL("hello", s);

#if defined(HAVE_WOSTREAM)
    std::wostringstream ws;
    ws << wxString("bye");
    ASSERT_WOSTREAM_EQUAL(L"bye", ws);
#endif
}
