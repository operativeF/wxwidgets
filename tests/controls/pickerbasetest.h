///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/pickerbasetest.cpp
// Purpose:     wxPickerBase unit test
// Author:      Steven Lamerton
// Created:     2010-08-07
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TESTS_CONTROLS_PICKERBASETEST_H_
#define _WX_TESTS_CONTROLS_PICKERBASETEST_H_

#include "wx/pickerbase.h"

template<typename PickCtrlT>
struct PickerBaseTest
{
    wxPickerBase* GetBase() { return m_pickctrl.get(); }

    #define wxPICKER_BASE_TESTS() \
            SUBCASE( "Margin" ) { Margin(); } \
            SUBCASE( "Proportion" ) { Proportion(); } \
            SUBCASE( "Growable" ) { Growable(); } \
            SUBCASE( "Controls" ) { Controls(); }

    void Margin()
    {
        wxPickerBase* const base = GetBase();

        CHECK(base->HasTextCtrl());
        CHECK(base->GetInternalMargin() >= 0);

        base->SetInternalMargin(15);

        CHECK_EQ(15, base->GetInternalMargin());
    }

    void Proportion()
    {
        wxPickerBase* const base = GetBase();

        CHECK(base->HasTextCtrl());

        base->SetPickerCtrlProportion(1);
        base->SetTextCtrlProportion(1);

        CHECK_EQ(1, base->GetPickerCtrlProportion());
        CHECK_EQ(1, base->GetTextCtrlProportion());
    }

    void Growable()
    {
        wxPickerBase* const base = GetBase();

        CHECK(base->HasTextCtrl());

        base->SetPickerCtrlGrowable();
        base->SetTextCtrlGrowable();

        CHECK(base->IsPickerCtrlGrowable());
        CHECK(base->IsTextCtrlGrowable());

        base->SetPickerCtrlGrowable(false);
        base->SetTextCtrlGrowable(false);

        CHECK(!base->IsPickerCtrlGrowable());
        CHECK(!base->IsTextCtrlGrowable());
    }

    void Controls()
    {
        wxPickerBase* const base = GetBase();

        CHECK(base->HasTextCtrl());
        CHECK(base->GetTextCtrl() != nullptr);
        CHECK(base->GetPickerCtrl() != nullptr);
    }

    std::unique_ptr<PickCtrlT> m_pickctrl;

};

#endif // _WX_TESTS_CONTROLS_PICKERBASETEST_H_
