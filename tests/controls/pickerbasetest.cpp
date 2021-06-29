///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/pickerbasetest.cpp
// Purpose:     wxPickerBase unit test
// Author:      Steven Lamerton
// Created:     2010-08-07
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_COLOURPICKERCTRL || \
    wxUSE_DIRPICKERCTRL    || \
    wxUSE_FILEPICKERCTRL   || \
    wxUSE_FONTPICKERCTRL

#include "wx/pickerbase.h"
#include "pickerbasetest.h"

void PickerBaseTestCase::Margin()
{
    wxPickerBase* const base = GetBase();

    CHECK(base->HasTextCtrl());
    CHECK(base->GetInternalMargin() >= 0);

    base->SetInternalMargin(15);

    CHECK_EQ(15, base->GetInternalMargin());
}

void PickerBaseTestCase::Proportion()
{
    wxPickerBase* const base = GetBase();

    CHECK(base->HasTextCtrl());

    base->SetPickerCtrlProportion(1);
    base->SetTextCtrlProportion(1);

    CHECK_EQ(1, base->GetPickerCtrlProportion());
    CHECK_EQ(1, base->GetTextCtrlProportion());
}

void PickerBaseTestCase::Growable()
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

void PickerBaseTestCase::Controls()
{
    wxPickerBase* const base = GetBase();

    CHECK(base->HasTextCtrl());
    CHECK(base->GetTextCtrl() != NULL);
    CHECK(base->GetPickerCtrl() != NULL);
}

#endif
