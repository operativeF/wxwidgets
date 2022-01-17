///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/pickertest.cpp
// Purpose:     Tests for various wxPickerBase based classes
// Author:      Steven Lamerton
// Created:     2010-08-07
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#if wxUSE_COLOURPICKERCTRL || \
    wxUSE_DIRPICKERCTRL    || \
    wxUSE_FILEPICKERCTRL   || \
    wxUSE_FONTPICKERCTRL

#include "wx/app.h"

#include "wx/clrpicker.h"
#include "wx/filepicker.h"
#include "wx/fontpicker.h"

export module WX.Test.Picker;

import WX.MetaTest;
import WX.Test.PickerBase;
import WX.Test.Prec;

#if wxUSE_COLOURPICKERCTRL

namespace ut = boost::ut;

ut::suite ColourPickerCtrlTests = []
{
    PickerBaseTests(std::make_unique<wxColourPickerCtrl>(wxTheApp->GetTopWindow(), wxID_ANY,
                                                       *wxBLACK, wxDefaultPosition,
                                                       wxDefaultSize, wxCLRP_USE_TEXTCTRL));
};

#endif //wxUSE_COLOURPICKERCTRL

#if wxUSE_DIRPICKERCTRL

ut::suite DirPickerCtrlTests = []
{
    PickerBaseTests(std::make_unique<wxDirPickerCtrl>(
                                   wxTheApp->GetTopWindow(), wxID_ANY,
                                   "", wxDirSelectorPromptStr,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxDIRP_USE_TEXTCTRL));
};

#endif //wxUSE_DIRPICKERCTRL

#if wxUSE_FILEPICKERCTRL

ut::suite FilePickerCtrlTest = []
{
    PickerBaseTests(std::make_unique<wxFilePickerCtrl>(
                                  wxTheApp->GetTopWindow(), wxID_ANY,
                                  "", wxFileSelectorPromptStr,
                                  wxFileSelectorDefaultWildcardStr,
                                  wxDefaultPosition, wxDefaultSize,
                                  wxFLP_USE_TEXTCTRL));
};

#endif //wxUSE_FILEPICKERCTRL

#if wxUSE_FONTPICKERCTRL

ut::suite FontPickerCtrlTest = []
{
    using namespace ut;

    auto pickctrl = std::make_unique<wxFontPickerCtrl>(wxTheApp->GetTopWindow(), wxID_ANY,
                                                    wxNullFont, wxDefaultPosition, wxDefaultSize,
                                                    wxFNTP_USE_TEXTCTRL);

    "ColourSelection"_test = [&]
    {
        wxColour selectedColour(0xFF4269UL);

        expect(pickctrl->GetSelectedColour() == wxColour(*wxBLACK)) <<
                "Default font picker color must be black";

        pickctrl->SetSelectedColour(selectedColour);

        expect(pickctrl->GetSelectedColour() == selectedColour) <<
                "Font picker did not react to color selection";
    };

    PickerBaseTests(std::move(pickctrl));
};

#endif //wxUSE_FONTPICKERCTRL

#endif
