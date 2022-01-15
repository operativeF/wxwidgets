///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/radioboxtest.cpp
// Purpose:     wxRadioBox unit test
// Author:      Steven Lamerton
// Created:     2010-07-14
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/radiobox.h"

#include "wx/tooltip.h"

export module WX.Test.Radiobox;

import WX.MetaTest;
import WX.Test.Prec;

#if wxUSE_RADIOBOX

namespace ut = boost::ut;

ut::suite RadioboxTests = []
{
    using namespace ut;

    const std::vector<std::string> choices = { "item 0", "item 1", "item 2" };

    auto m_radio = std::make_unique<wxRadioBox>(wxTheApp->GetTopWindow(), wxID_ANY,
                                                "RadioBox", wxDefaultPosition,
                                                wxDefaultSize, choices);

    "FindString"_test = [&]
    {
        expect(wxNOT_FOUND == m_radio->FindString("not here"));
        expect(1 == m_radio->FindString("item 1"));
        expect(2 == m_radio->FindString("ITEM 2"));
        expect(wxNOT_FOUND == m_radio->FindString("ITEM 2", true));
    };

#ifndef __WXGTK__
    "RowColCount"_test = [&]
    {
        const std::vector<std::string> choices = { "item 0", "item 1", "item 2" };

        m_radio = std::make_unique<wxRadioBox>(wxTheApp->GetTopWindow(), wxID_ANY,
                                  "RadioBox", wxDefaultPosition,
                                  wxDefaultSize, choices, 2);

        expect(2 == m_radio->GetColumnCount());
        expect(2 == m_radio->GetRowCount());

        m_radio = std::make_unique<wxRadioBox>(wxTheApp->GetTopWindow(), wxID_ANY,
                                  "RadioBox", wxDefaultPosition,
                                  wxDefaultSize, choices, 1,
                                  wxRA_SPECIFY_ROWS);

        expect(3 == m_radio->GetColumnCount());
        expect(1 == m_radio->GetRowCount());
    };
#endif

#ifndef __WXOSX__
    "Enable"_test = [&]
    {
        m_radio->Enable(false);

        expect(!m_radio->IsItemEnabled(0));

        m_radio->Enable(1, true);

        expect(!m_radio->IsItemEnabled(0));
        expect(m_radio->IsItemEnabled(1));
        expect(!m_radio->IsItemEnabled(2));

        m_radio->Enable(true);

        expect(m_radio->IsItemEnabled(0));
        expect(m_radio->IsItemEnabled(1));
        expect(m_radio->IsItemEnabled(2));

        m_radio->Enable(0, false);

        expect(!m_radio->IsItemEnabled(0));
        expect(m_radio->IsItemEnabled(1));
        expect(m_radio->IsItemEnabled(2));
    };
#endif

    "Show"_test = [&]
    {
        m_radio->Show(false);

        expect(!m_radio->IsItemShown(0));

        m_radio->Show(1, true);

        expect(!m_radio->IsItemShown(0));
        expect(m_radio->IsItemShown(1));
        expect(!m_radio->IsItemShown(2));

        m_radio->Show(true);

        expect(m_radio->IsItemShown(0));
        expect(m_radio->IsItemShown(1));
        expect(m_radio->IsItemShown(2));

        m_radio->Show(0, false);

        expect(!m_radio->IsItemShown(0));
        expect(m_radio->IsItemShown(1));
        expect(m_radio->IsItemShown(2));
    };

    "HelpText"_test = [&]
    {
        expect(m_radio->GetItemHelpText(0).empty());

        m_radio->SetItemHelpText(1, "Item 1 help");

        expect("Item 1 help" == m_radio->GetItemHelpText(1));

        m_radio->SetItemHelpText(1, "");

        expect(m_radio->GetItemHelpText(1).empty());
    };

#if defined (__WXMSW__) || defined(__WXGTK__)
    "ToolTip"_test = [&]
    {
        //GetItemToolTip returns nullptr if there is no tooltip set
        expect(!m_radio->GetItemToolTip(0));

        m_radio->SetItemToolTip(1, "Item 1 help");

        expect("Item 1 help" == m_radio->GetItemToolTip(1)->GetTip());

        m_radio->SetItemToolTip(1, "");

        //However if we set a blank tip this does count as a tooltip
        expect(!m_radio->GetItemToolTip(1));
    };
#endif

    "Selection"_test = [&]
    {
        //Until other item containers the first item is selected by default
        expect(0 == m_radio->GetSelection());
        expect("item 0" == m_radio->GetStringSelection());

        m_radio->SetSelection(1);

        expect(1 == m_radio->GetSelection());
        expect("item 1" == m_radio->GetStringSelection());

        m_radio->SetStringSelection("item 2");

        expect(2 == m_radio->GetSelection());
        expect("item 2" == m_radio->GetStringSelection());
    };

    "Count"_test = [&]
    {
        //A trivial test for the item count as items can neither
        //be added or removed
        expect(3 == m_radio->GetCount());
        expect(!m_radio->IsEmpty());
    };

    "SetString"_test = [&]
    {
        m_radio->SetString(0, "new item 0");
        m_radio->SetString(2, "");

        expect("new item 0" == m_radio->GetString(0));
        expect(m_radio->GetString(2).empty());
    };
};

#endif // wxUSE_RADIOBOX
