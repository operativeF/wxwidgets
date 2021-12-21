///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/bitmaptogglebuttontest.cpp
// Purpose:     wxBitmapToggleButton unit test
// Author:      Steven Lamerton
// Created:     2010-07-17
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_TOGGLEBTN

#include "wx/tglbtn.h"

#ifdef wxHAS_BITMAPTOGGLEBUTTON

#include "wx/app.h"

#include "testableframe.h"
#include "wx/uiaction.h"
#include "wx/artprov.h"
#include "wx/utils.h"

import WX.Test.Prec;

TEST_CASE("Bitmap toggle button test")
{
    auto m_button = std::make_unique<wxBitmapToggleButton>(wxTheApp->GetTopWindow(),
                                                           wxID_ANY,
                                                           wxArtProvider::GetIcon(
                                                               wxART_INFORMATION,
                                                               wxART_OTHER,
                                                               wxSize(32, 32)));
    m_button->Update();
    m_button->Refresh();

    SUBCASE("Click")
    {
    #if wxUSE_UIACTIONSIMULATOR
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

    #ifdef __WXMSW__
        wxMilliSleep(1000);
    #endif

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, clicked.GetCount());
        CHECK(!m_button->GetValue());
    #endif // wxUSE_UIACTIONSIMULATOR
    }

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
#endif // wxHAS_BITMAPTOGGLEBUTTON

#endif // wxUSE_TOGGLEBTN
