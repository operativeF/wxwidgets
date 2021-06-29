///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/frametest.cpp
// Purpose:     wxFrame  unit test
// Author:      Steven Lamerton
// Created:     2010-07-10
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/frame.h"
#endif // WX_PRECOMP

#include "testableframe.h"

static std::unique_ptr<wxFrame> setUp()
{
    auto m_frame = std::make_unique<wxFrame>(nullptr, wxID_ANY, "test frame");
    m_frame->Show();

    return m_frame;
}

static void tearDown(wxFrame* frame)
{
    frame->Destroy();
}

TEST_CASE("Frame test")
{
    auto m_frame = setUp();

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

    tearDown(m_frame.get());
}
