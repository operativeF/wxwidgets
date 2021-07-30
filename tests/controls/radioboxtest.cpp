///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/radioboxtest.cpp
// Purpose:     wxRadioBox unit test
// Author:      Steven Lamerton
// Created:     2010-07-14
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_RADIOBOX


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/radiobox.h"
#endif // WX_PRECOMP

#include "wx/tooltip.h"

TEST_CASE("Radiobox test")
{
    const std::vector<std::string> choices = { "item 0", "item 1", "item 2" };

    auto m_radio = std::make_unique<wxRadioBox>(wxTheApp->GetTopWindow(), wxID_ANY,
                                                "RadioBox", wxDefaultPosition,
                                                wxDefaultSize, choices);

    SUBCASE("FindString")
    {
        CHECK_EQ(wxNOT_FOUND, m_radio->FindString("not here"));
        CHECK_EQ(1, m_radio->FindString("item 1"));
        CHECK_EQ(2, m_radio->FindString("ITEM 2"));
        CHECK_EQ(wxNOT_FOUND, m_radio->FindString("ITEM 2", true));
    }

#ifndef __WXGTK__
    SUBCASE("RowColCount")
    {
        const std::vector<std::string> choices = { "item 0", "item 1", "item 2" };

        m_radio = std::make_unique<wxRadioBox>(wxTheApp->GetTopWindow(), wxID_ANY,
                                  "RadioBox", wxDefaultPosition,
                                  wxDefaultSize, choices, 2);

        CHECK_EQ(2, m_radio->GetColumnCount());
        CHECK_EQ(2, m_radio->GetRowCount());

        m_radio = std::make_unique<wxRadioBox>(wxTheApp->GetTopWindow(), wxID_ANY,
                                  "RadioBox", wxDefaultPosition,
                                  wxDefaultSize, choices, 1,
                                  wxRA_SPECIFY_ROWS);

        CHECK_EQ(3, m_radio->GetColumnCount());
        CHECK_EQ(1, m_radio->GetRowCount());
    }
#endif

#ifndef __WXOSX__
    SUBCASE("Enable")
    {
        m_radio->Enable(false);

        CHECK(!m_radio->IsItemEnabled(0));

        m_radio->Enable(1, true);

        CHECK(!m_radio->IsItemEnabled(0));
        CHECK(m_radio->IsItemEnabled(1));
        CHECK(!m_radio->IsItemEnabled(2));

        m_radio->Enable(true);

        CHECK(m_radio->IsItemEnabled(0));
        CHECK(m_radio->IsItemEnabled(1));
        CHECK(m_radio->IsItemEnabled(2));

        m_radio->Enable(0, false);

        CHECK(!m_radio->IsItemEnabled(0));
        CHECK(m_radio->IsItemEnabled(1));
        CHECK(m_radio->IsItemEnabled(2));
    }
#endif

    SUBCASE("Show")
    {
        m_radio->Show(false);

        CHECK(!m_radio->IsItemShown(0));

        m_radio->Show(1, true);

        CHECK(!m_radio->IsItemShown(0));
        CHECK(m_radio->IsItemShown(1));
        CHECK(!m_radio->IsItemShown(2));

        m_radio->Show(true);

        CHECK(m_radio->IsItemShown(0));
        CHECK(m_radio->IsItemShown(1));
        CHECK(m_radio->IsItemShown(2));

        m_radio->Show(0, false);

        CHECK(!m_radio->IsItemShown(0));
        CHECK(m_radio->IsItemShown(1));
        CHECK(m_radio->IsItemShown(2));
    }

    SUBCASE("HelpText")
    {
        CHECK_EQ("", m_radio->GetItemHelpText(0));

        m_radio->SetItemHelpText(1, "Item 1 help");

        CHECK_EQ("Item 1 help", m_radio->GetItemHelpText(1));

        m_radio->SetItemHelpText(1, "");

        CHECK_EQ("", m_radio->GetItemHelpText(1));
    }

#if defined (__WXMSW__) || defined(__WXGTK__)
    SUBCASE("ToolTip")
    {
        //GetItemToolTip returns nullptr if there is no tooltip set
        CHECK(!m_radio->GetItemToolTip(0));

        m_radio->SetItemToolTip(1, "Item 1 help");

        CHECK_EQ("Item 1 help", m_radio->GetItemToolTip(1)->GetTip());

        m_radio->SetItemToolTip(1, "");

        //However if we set a blank tip this does count as a tooltip
        CHECK(!m_radio->GetItemToolTip(1));
    }
#endif

    SUBCASE("Selection")
    {
        //Until other item containers the first item is selected by default
        CHECK_EQ(0, m_radio->GetSelection());
        CHECK_EQ("item 0", m_radio->GetStringSelection());

        m_radio->SetSelection(1);

        CHECK_EQ(1, m_radio->GetSelection());
        CHECK_EQ("item 1", m_radio->GetStringSelection());

        m_radio->SetStringSelection("item 2");

        CHECK_EQ(2, m_radio->GetSelection());
        CHECK_EQ("item 2", m_radio->GetStringSelection());
    }

    SUBCASE("Count")
    {
        //A trivial test for the item count as items can neither
        //be added or removed
        CHECK_EQ(3, m_radio->GetCount());
        CHECK(!m_radio->IsEmpty());
    }

    SUBCASE("SetString")
    {
        m_radio->SetString(0, "new item 0");
        m_radio->SetString(2, "");

        CHECK_EQ("new item 0", m_radio->GetString(0));
        CHECK_EQ("", m_radio->GetString(2));
    }
}

#endif // wxUSE_RADIOBOX
