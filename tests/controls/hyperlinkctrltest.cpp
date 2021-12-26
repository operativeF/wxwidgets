///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/hyperlinkctrltest.cpp
// Purpose:     wxHyperlinkCtrl unit test
// Author:      Steven Lamerton
// Created:     2010-08-05
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_HYPERLINKCTRL

#include "wx/app.h"
#include "wx/hyperlink.h"
#include "wx/uiaction.h"

#include "testableframe.h"

import WX.Test.Prec;

TEST_CASE("Hyperlink control test")
{
    auto m_hyperlink = std::make_unique<wxHyperlinkCtrl>(wxTheApp->GetTopWindow(),
                                                         wxID_ANY, "wxWidgets",
                                                         "http://wxwidgets.org");

    SUBCASE("Colour")
    {
    #ifndef __WXGTK__
        CHECK(m_hyperlink->GetHoverColour().IsOk());
        CHECK(m_hyperlink->GetNormalColour().IsOk());
        CHECK(m_hyperlink->GetVisitedColour().IsOk());

        m_hyperlink->SetHoverColour(*wxGREEN);
        m_hyperlink->SetNormalColour(*wxRED);
        m_hyperlink->SetVisitedColour(*wxBLUE);

        CHECK_EQ(*wxGREEN, m_hyperlink->GetHoverColour());
        CHECK_EQ(*wxRED, m_hyperlink->GetNormalColour());
        CHECK_EQ(*wxBLUE, m_hyperlink->GetVisitedColour());
    #endif
    }

    SUBCASE("Url")
    {
        CHECK_EQ("http://wxwidgets.org", m_hyperlink->GetURL());

        m_hyperlink->SetURL("http://google.com");

        CHECK_EQ("http://google.com", m_hyperlink->GetURL());
    }

#if wxUSE_UIACTIONSIMULATOR
    SUBCASE("Click")
    {
        EventCounter hyperlink(m_hyperlink.get(), wxEVT_HYPERLINK);

        wxUIActionSimulator sim;

        sim.MouseMove(m_hyperlink->GetScreenPosition() + wxPoint(10, 10));
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, hyperlink.GetCount());
    }
#endif

}

#endif //wxUSE_HYPERLINKCTRL
