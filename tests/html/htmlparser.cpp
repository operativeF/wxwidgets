///////////////////////////////////////////////////////////////////////////////
// Name:        tests/html/htmlparser.cpp
// Purpose:     wxHtmlParser tests
// Author:      Vadim Zeitlin
// Created:     2011-01-13
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_HTML

#include "wx/dcmemory.h"

#include "wx/html/winpars.h"

// Test that parsing invalid HTML simply fails but doesn't crash for example.
TEST_CASE("wxHtmlParser::ParseInvalid")
{
    class NullParser : public wxHtmlWinParser
    {
    protected:
        void AddText([[maybe_unused]] const wxString& txt) override { }
    };

    NullParser p;
    wxMemoryDC dc;
    p.SetDC(&dc);

    delete p.Parse("<");
    delete p.Parse("<foo");
    delete p.Parse("<!--");
    delete p.Parse("<!---");
}

TEST_CASE("wxHtmlCell::Detach")
{
    wxMemoryDC dc;

    std::unique_ptr<wxHtmlContainerCell> const top(new wxHtmlContainerCell(nullptr));
    wxHtmlContainerCell* const cont = new wxHtmlContainerCell(nullptr);
    wxHtmlCell* const cell1 = new wxHtmlWordCell("Hello", dc);
    wxHtmlCell* const cell2 = new wxHtmlColourCell(*wxRED);
    wxHtmlCell* const cell3 = new wxHtmlWordCell("world", dc);

    cont->InsertCell(cell1);
    cont->InsertCell(cell2);
    cont->InsertCell(cell3);
    top->InsertCell(cont);

    SUBCASE("container")
    {
        top->Detach(cont);
        CHECK( top->wxGetFirstChild() == nullptr );

        delete cont;
    }

    SUBCASE("first-child")
    {
        cont->Detach(cell1);
        CHECK( cont->wxGetFirstChild() == cell2 );

        delete cell1;
    }

    SUBCASE("middle-child")
    {
        cont->Detach(cell2);
        CHECK( cont->wxGetFirstChild() == cell1 );
        CHECK( cell1->GetNext() == cell3 );

        delete cell2;
    }

    SUBCASE("last-child")
    {
        cont->Detach(cell3);
        CHECK( cont->wxGetFirstChild() == cell1 );
        CHECK( cell1->GetNext() == cell2 );
        CHECK( cell2->GetNext() == nullptr );

        delete cell3;
    }

    SUBCASE("invalid")
    {
        // Expected assertion for detaching non-child
        CHECK_THROWS
        (
            top->Detach(cell1);
        );
    }
}

#endif //wxUSE_HTML
