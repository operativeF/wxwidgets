///////////////////////////////////////////////////////////////////////////////
// Name:        tests/html/htmlwindow.cpp
// Purpose:     wxHtmlWindow tests
// Author:      Vaclav Slavik
// Created:     2008-10-15
// Copyright:   (c) 2008 Vaclav Slavik <vslavik@fastmail.fm>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_HTML


#ifndef WX_PRECOMP
    #include "wx/app.h"

    #include <memory>
#endif // WX_PRECOMP

#include "wx/html/htmlwin.h"
#include "wx/uiaction.h"
#include "testableframe.h"


constexpr char TEST_MARKUP[] =
    "<html><body>"
    "<title>Page</title>"
    "  Title<p>"
    "  A longer line<br>"
    "  and the last line."
    "</body></html>";

constexpr char TEST_MARKUP_LINK[] =
    "<html><body>"
    "<a href=\"link\">link<\\a> "
    "</body></html>";

constexpr char TEST_PLAIN_TEXT[] =
    "Title\nA longer line\nand the last line.";

TEST_CASE("HTML Window")
{
    auto m_win = std::make_unique<wxHtmlWindow>(wxTheApp->GetTopWindow(), wxID_ANY,
                                                wxDefaultPosition,
                                                wxSize(400, 200));

    SUBCASE("SelectionToText")
    {
    #if wxUSE_CLIPBOARD
        m_win->SetPage(TEST_MARKUP);
        m_win->SelectAll();

        CHECK_EQ( TEST_PLAIN_TEXT, m_win->SelectionToText() );
    #endif // wxUSE_CLIPBOARD
    }

    SUBCASE("Title")
    {
        m_win->SetPage(TEST_MARKUP);

        CHECK_EQ("Page", m_win->GetOpenedPageTitle());
    }

#if wxUSE_UIACTIONSIMULATOR
    SUBCASE("CellClick")
    {
        EventCounter clicked(m_win.get(), wxEVT_HTML_CELL_CLICKED);

        wxUIActionSimulator sim;

        m_win->SetPage(TEST_MARKUP);
        m_win->Update();
        m_win->Refresh();

        sim.MouseMove(m_win->ClientToScreen(wxPoint(15, 15)));
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, clicked.GetCount());
    }

    SUBCASE("LinkClick")
    {
        EventCounter clicked(m_win.get(), wxEVT_HTML_LINK_CLICKED);

        wxUIActionSimulator sim;

        m_win->SetPage(TEST_MARKUP_LINK);
        m_win->Update();
        m_win->Refresh();

        sim.MouseMove(m_win->ClientToScreen(wxPoint(15, 15)));
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, clicked.GetCount());
    }
#endif // wxUSE_UIACTIONSIMULATOR

    SUBCASE("AppendToPage")
    {
    #if wxUSE_CLIPBOARD
        m_win->SetPage(TEST_MARKUP_LINK);
        m_win->AppendToPage("A new paragraph");

        CHECK_EQ("link A new paragraph", m_win->ToText());
    #endif // wxUSE_CLIPBOARD
    }
} // END TEST_CASE("HTML Window")

#endif //wxUSE_HTML
