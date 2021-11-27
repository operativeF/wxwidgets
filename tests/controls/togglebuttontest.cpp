///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/togglebuttontest.cpp
// Purpose:     wxToggleButton unit test
// Author:      Steven Lamerton
// Created:     2010-07-14
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_TOGGLEBTN

#include "wx/app.h"

#include "wx/uiaction.h"
#include "wx/tglbtn.h"

#include "testableframe.h"

import WX.Test.Prec;

TEST_CASE("Toggle button test")
{
    auto m_button = std::make_unique<wxToggleButton>(wxTheApp->GetTopWindow(),
                                                     wxID_ANY, "wxToggleButton");

#if wxUSE_UIACTIONSIMULATOR
    SUBCASE("Click")
    {
        EventCounter clicked(m_button.get(), wxEVT_TOGGLEBUTTON);

        wxUIActionSimulator sim;

        //We move in slightly to account for window decorations
        sim.MouseMove(m_button->GetScreenPosition() + wxPoint(10, 10));
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, clicked.GetCount());
        CHECK(m_button->GetValue());
        clicked.Clear();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, clicked.GetCount());
        CHECK(!m_button->GetValue());
    }
#endif

    SUBCASE("Value")
    {
        EventCounter clicked(m_button.get(), wxEVT_BUTTON);

        m_button->SetValue(true);

        CHECK(m_button->GetValue());

        m_button->SetValue(false);

        CHECK(!m_button->GetValue());

        CHECK_EQ( 0, clicked.GetCount() );
    }
}
#endif //wxUSE_TOGGLEBTN
