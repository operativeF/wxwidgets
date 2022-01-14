///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/frametest.cpp
// Purpose:     wxFrame  unit test
// Author:      Steven Lamerton
// Created:     2010-07-10
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/frame.h"

#include "testableframe.h"

export module WX.Test.Frame;

import WX.Test.Prec;
import WX.MetaTest;

namespace ut = boost::ut;

ut::suite FrameTest = []
{
    using namespace ut;

    auto m_frame = std::make_unique<wxFrame>(nullptr, wxID_ANY, "test frame");
    m_frame->Show();

    "Iconize"_test = [&m_frame]
    {
    #ifdef __WXMSW__
        EventCounter iconize(m_frame.get(), wxEVT_ICONIZE);

        m_frame->Iconize();
        m_frame->Iconize(false);

        expect(iconize.GetCount() == 2);
    #endif
    };

    "Close"_test = [&m_frame]
    {
        EventCounter close(m_frame.get(), wxEVT_CLOSE_WINDOW);

        m_frame->Close();

        expect(close.GetCount() == 1);
    };
};
