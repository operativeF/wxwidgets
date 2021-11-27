///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/slidertest.cpp
// Purpose:     wxSlider unit test
// Author:      Steven Lamerton
// Created:     2010-07-20
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_SLIDER

#include "wx/app.h"
#include "wx/slider.h"

#include "wx/uiaction.h"
#include "testableframe.h"

import WX.Test.Prec;

TEST_CASE("Slider control test")
{
    unsigned int style = wxSL_HORIZONTAL;

    auto m_slider = std::make_unique<wxSlider>(wxTheApp->GetTopWindow(), wxID_ANY,
                                          50, 0, 100,
                                          wxDefaultPosition, wxDefaultSize,
                                          style);

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

#endif

    SUBCASE("Value")
    {
        m_slider->SetValue(30);

        CHECK_EQ(30, m_slider->GetValue());

        //When setting a value larger that max or smaller than min
        //max and min are set
        m_slider->SetValue(-1);

        CHECK_EQ(0, m_slider->GetValue());

        m_slider->SetValue(110);

        CHECK_EQ(100, m_slider->GetValue());
    }

    SUBCASE("Range")
    {
        CHECK_EQ(0, m_slider->GetMin());
        CHECK_EQ(100, m_slider->GetMax());

        // Changing range shouldn't change the value.
        m_slider->SetValue(17);
        m_slider->SetRange(0, 200);
        CHECK_EQ(17, m_slider->GetValue());

        //Test negative ranges
        m_slider->SetRange(-50, 0);

        CHECK_EQ(-50, m_slider->GetMin());
        CHECK_EQ(0, m_slider->GetMax());
    }

#if wxUSE_UIACTIONSIMULATOR
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
#endif
    }

    // TODO: Figure out a better way to do this.
    style |= wxSL_INVERSE;

    SUBCASE("Inverse Value")
    {
        m_slider->SetValue(30);

        CHECK_EQ(30, m_slider->GetValue());

        //When setting a value larger that max or smaller than min
        //max and min are set
        m_slider->SetValue(-1);

        CHECK_EQ(0, m_slider->GetValue());

        m_slider->SetValue(110);

        CHECK_EQ(100, m_slider->GetValue());
    }

    SUBCASE("Inverse Range")
    {
        CHECK_EQ(0, m_slider->GetMin());
        CHECK_EQ(100, m_slider->GetMax());

        // Changing range shouldn't change the value.
        m_slider->SetValue(17);
        m_slider->SetRange(0, 200);
        CHECK_EQ(17, m_slider->GetValue());

        //Test negative ranges
        m_slider->SetRange(-50, 0);

        CHECK_EQ(-50, m_slider->GetMin());
        CHECK_EQ(0, m_slider->GetMax());
    }
}

#endif
