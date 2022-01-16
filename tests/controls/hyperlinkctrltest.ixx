///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/hyperlinkctrltest.cpp
// Purpose:     wxHyperlinkCtrl unit test
// Author:      Steven Lamerton
// Created:     2010-08-05
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "wx/app.h"
#include "wx/hyperlink.h"
#include "wx/uiaction.h"

#include "testableframe.h"

export module WX.Test.HyperlinkCtrl;

import WX.Test.Prec;
import WX.MetaTest;

#if wxUSE_HYPERLINKCTRL

namespace ut = boost::ut;

ut::suite HyperlinkTests = []
{
    using namespace ut;

    auto m_hyperlink = std::make_unique<wxHyperlinkCtrl>(wxTheApp->GetTopWindow(),
                                                        wxID_ANY, "wxWidgets",
                                                        "http://wxwidgets.org");

#ifndef __WXGTK__
    "Colour"_test = [&]
    {
        expect(m_hyperlink->GetHoverColour().IsOk());
        expect(m_hyperlink->GetNormalColour().IsOk());
        expect(m_hyperlink->GetVisitedColour().IsOk());

        m_hyperlink->SetHoverColour(*wxGREEN);
        m_hyperlink->SetNormalColour(*wxRED);
        m_hyperlink->SetVisitedColour(*wxBLUE);

        expect(*wxGREEN == m_hyperlink->GetHoverColour());
        expect(*wxRED == m_hyperlink->GetNormalColour());
        expect(*wxBLUE == m_hyperlink->GetVisitedColour());
    };
#endif

    "Url"_test = [&]
    {
        expect("http://wxwidgets.org" == m_hyperlink->GetURL());

        m_hyperlink->SetURL("http://google.com");

        expect("http://google.com" == m_hyperlink->GetURL());
    };
};

TEST_CASE("Hyperlink control test")
{
    auto m_hyperlink = std::make_unique<wxHyperlinkCtrl>(wxTheApp->GetTopWindow(),
                                                         wxID_ANY, "wxWidgets",
                                                         "http://wxwidgets.org");

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
