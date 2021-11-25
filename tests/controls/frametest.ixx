///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/frametest.cpp
// Purpose:     wxFrame  unit test
// Author:      Steven Lamerton
// Created:     2010-07-10
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "testprec.h"

#include "wx/app.h"
#include "wx/frame.h"

#include "testableframe.h"

export module WX.Test.Frame;

TEST_CASE("Frame test")
{
    auto m_frame = std::make_unique<wxFrame>(nullptr, wxID_ANY, "test frame");
    m_frame->Show();

    SUBCASE("Iconize")
    {
    #ifdef __WXMSW__
        EventCounter iconize(m_frame.get(), wxEVT_ICONIZE);

        m_frame->Iconize();
        m_frame->Iconize(false);

        CHECK_EQ(2, iconize.GetCount());
    #endif
    }

    SUBCASE("Close")
    {
        EventCounter close(m_frame.get(), wxEVT_CLOSE_WINDOW);

        m_frame->Close();

        CHECK_EQ(1, close.GetCount());
    }
}
