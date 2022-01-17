///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/pickerbasetest.cpp
// Purpose:     wxPickerBase unit test
// Author:      Steven Lamerton
// Created:     2010-08-07
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/pickerbase.h"

export module WX.Test.PickerBase;

import WX.MetaTest;

namespace ut = boost::ut;

export inline void PickerBaseTests(auto&& picker)
{
    using namespace ut;

    std::unique_ptr<wxPickerBase> m_pickctrl{std::move(picker)};

    "Margin"_test = [&]
    {
        wxPickerBase* const base = m_pickctrl.get();

        expect(base->HasTextCtrl());
        expect(base->GetInternalMargin() >= 0);

        base->SetInternalMargin(15);

        expect(15 == base->GetInternalMargin());
    };

    "Proportion"_test = [&]
    {
        wxPickerBase* const base = m_pickctrl.get();

        expect(base->HasTextCtrl());

        base->SetPickerCtrlProportion(1);
        base->SetTextCtrlProportion(1);

        expect(1 == base->GetPickerCtrlProportion());
        expect(1 == base->GetTextCtrlProportion());
    };

    "Growable"_test = [&]
    {
        wxPickerBase* const base = m_pickctrl.get();

        expect(base->HasTextCtrl());

        base->SetPickerCtrlGrowable();
        base->SetTextCtrlGrowable();

        expect(base->IsPickerCtrlGrowable());
        expect(base->IsTextCtrlGrowable());

        base->SetPickerCtrlGrowable(false);
        base->SetTextCtrlGrowable(false);

        expect(!base->IsPickerCtrlGrowable());
        expect(!base->IsTextCtrlGrowable());
    };

    "Controls"_test = [&]
    {
        wxPickerBase* const base = m_pickctrl.get();

        expect(base->HasTextCtrl());
        expect(base->GetTextCtrl() != nullptr);
        expect(base->GetPickerCtrl() != nullptr);
    };
}
