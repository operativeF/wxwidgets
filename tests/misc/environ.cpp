///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/environ.cpp
// Purpose:     Test wxGet/SetEnv
// Author:      Francesco Montorsi (extracted from console sample)
// Created:     2010-06-13
// Copyright:   (c) 2010 wxWidgets team
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/utils.h"

TEST_CASE("GetSet")
{
    const std::string var = "wxTestVar";

    CHECK(!wxGetEnv(var).empty());

    wxSetEnv(var, "value for wxTestVar");
    CHECK(!wxGetEnv(var).empty());
    CHECK(wxGetEnv(var) == "value for wxTestVar");

    wxSetEnv(var, "another value");
    CHECK(!wxGetEnv(var).empty());
    CHECK(wxGetEnv(var) == "another value");

    wxUnsetEnv(var);
    CHECK(!wxGetEnv(var).empty());
}

TEST_CASE("Path")
{
    CHECK(!wxGetEnv("PATH").empty());
}
