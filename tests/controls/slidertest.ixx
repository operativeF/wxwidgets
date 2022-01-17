///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/slidertest.cpp
// Purpose:     wxSlider unit test
// Author:      Steven Lamerton
// Created:     2010-07-20
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "wx/app.h"
#include "wx/slider.h"

#include "wx/uiaction.h"
#include "testableframe.h"

export module WX.Test.Slider;

import WX.Test.Prec;
import WX.MetaTest;

#if wxUSE_SLIDER

namespace ut = boost::ut;

ut::suite SliderTests = []
{
    using namespace ut;

    unsigned int style = wxSL_HORIZONTAL;

    auto testSlider = std::make_unique<wxSlider>(wxTheApp->GetTopWindow(), wxID_ANY,
                                          50, 0, 100,
                                          wxDefaultPosition, wxDefaultSize,
                                          style);
    
    "Value"_test = [&]
    {
        testSlider->SetValue(30);

        expect(30_i == testSlider->GetValue());

        //When setting a value larger that max or smaller than min
        //max and min are set
        testSlider->SetValue(-1);

        expect(0_i == testSlider->GetValue());

        testSlider->SetValue(110);

        expect(100_i == testSlider->GetValue());
    };

    "Range"_test = [&]
    {
        expect(0_i == testSlider->GetMin());
        expect(100_i == testSlider->GetMax());

        // Changing range shouldn't change the value.
        testSlider->SetValue(17);
        testSlider->SetRange(0, 200);
        expect(17_i == testSlider->GetValue());

        //Test negative ranges
        testSlider->SetRange(-50, 0);

        expect(-50_i == testSlider->GetMin());
        expect(0_i == testSlider->GetMax());
    };

    // TODO: Figure out a better way to do this?
    testSlider = std::make_unique<wxSlider>(wxTheApp->GetTopWindow(), wxID_ANY,
                                        50, 0, 100,
                                        wxDefaultPosition, wxDefaultSize,
                                        wxSL_HORIZONTAL | wxSL_INVERSE);

    "Inverse Value"_test = [&]
    {
        testSlider->SetValue(30);

        expect(30_i == testSlider->GetValue());

        //When setting a value larger that max or smaller than min
        //max and min are set
        testSlider->SetValue(-1);

        expect(0_i == testSlider->GetValue());

        testSlider->SetValue(110);

        expect(100_i == testSlider->GetValue());
    };

    "Inverse Range"_test = [&]
    {
        expect(0_i == testSlider->GetMin());
        expect(100_i == testSlider->GetMax());

        // Changing range shouldn't change the value.
        testSlider->SetValue(17);
        testSlider->SetRange(0, 200);
        expect(17_i == testSlider->GetValue());

        //Test negative ranges
        testSlider->SetRange(-50, 0);

        expect(-50_i == testSlider->GetMin());
        expect(0_i == testSlider->GetMax());
    };
};

TEST_CASE("Slider control test")
{
    auto m_slider = std::make_unique<wxSlider>(wxTheApp->GetTopWindow(), wxID_ANY,
                                          50, 0, 100,
                                          wxDefaultPosition, wxDefaultSize,
                                          wxSL_HORIZONTAL);

#if wxUSE_UIACTIONSIMULATOR
    SUBCASE("PageUpDown")
    {
        EventCounter pageup(m_slider.get(), wxEVT_SCROLL_PAGEUP);
        EventCounter pagedown(m_slider.get(), wxEVT_SCROLL_PAGEDOWN);

        wxUIActionSimulator sim;

        m_slider->SetFocus();

        sim.Char(WXK_PAGEUP);
        sim.Char(WXK_PAGEDOWN);

        wxYield();

        CHECK_EQ(1, pageup.GetCount());
        CHECK_EQ(1, pagedown.GetCount());
    }

    SUBCASE("LineUpDown")
    {
        EventCounter lineup(m_slider.get(), wxEVT_SCROLL_LINEUP);
        EventCounter linedown(m_slider.get(), wxEVT_SCROLL_LINEDOWN);

        wxUIActionSimulator sim;
        wxYield();
        m_slider->SetFocus();

        sim.Char(WXK_UP);
        sim.Char(WXK_DOWN);

        wxYield();

        CHECK_EQ(1, lineup.GetCount());
        CHECK_EQ(1, linedown.GetCount());

    }

    SUBCASE("EvtSlider")
    {
        EventCounter slider(m_slider.get(), wxEVT_SLIDER);

        wxUIActionSimulator sim;
        wxYield();
        m_slider->SetFocus();

        sim.Char(WXK_UP);
        sim.Char(WXK_DOWN);

        wxYield();

        CHECK_EQ(2, slider.GetCount());
    }

    SUBCASE("LinePageSize")
    {
        wxUIActionSimulator sim;
        wxYield();
        m_slider->SetFocus();

        m_slider->SetPageSize(20);

        sim.Char(WXK_PAGEUP);

        wxYield();

        CHECK_EQ(20, m_slider->GetPageSize());
        CHECK_EQ(30, m_slider->GetValue());

        m_slider->SetLineSize(2);

        sim.Char(WXK_UP);

        wxYield();

        CHECK_EQ(2, m_slider->GetLineSize());
        CHECK_EQ(28, m_slider->GetValue());
    }

    SUBCASE("Thumb")
    {
        EventCounter track(m_slider.get(), wxEVT_SCROLL_THUMBTRACK);
        EventCounter release(m_slider.get(), wxEVT_SCROLL_THUMBRELEASE);
        EventCounter changed(m_slider.get(), wxEVT_SCROLL_CHANGED);

        wxUIActionSimulator sim;

        m_slider->SetValue(0);

        // use the slider real position for dragging the mouse.
        const int ypos = m_slider->GetSize().y / 2;
        sim.MouseDragDrop(m_slider->ClientToScreen(wxPoint(10, ypos)),m_slider->ClientToScreen(wxPoint(50, ypos)));
        wxYield();

        CHECK(track.GetCount() != 0);
        CHECK_EQ(1, release.GetCount());
    #if defined(__WXMSW__) || defined(__WXGTK__)
        CHECK_EQ(1, changed.GetCount());
    #endif
    }
#endif
}

#endif
