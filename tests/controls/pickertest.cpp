///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/pickertest.cpp
// Purpose:     Tests for various wxPickerBase based classes
// Author:      Steven Lamerton
// Created:     2010-08-07
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_COLOURPICKERCTRL || \
    wxUSE_DIRPICKERCTRL    || \
    wxUSE_FILEPICKERCTRL   || \
    wxUSE_FONTPICKERCTRL

#include "wx/app.h"

#include "wx/clrpicker.h"
#include "wx/filepicker.h"
#include "wx/fontpicker.h"

#include "pickerbasetest.h"

import WX.Test.Prec;

#if wxUSE_COLOURPICKERCTRL

using ColourPickerCtrlTest = PickerBaseTest<wxColourPickerCtrl>;

TEST_CASE_FIXTURE(ColourPickerCtrlTest, "Colour picker control test")
{
    m_pickctrl = std::make_unique<wxColourPickerCtrl>(
                                  wxTheApp->GetTopWindow(), wxID_ANY,
                                  *wxBLACK, wxDefaultPosition,
                                  wxDefaultSize, wxCLRP_USE_TEXTCTRL);

    wxPICKER_BASE_TESTS();
}

#endif //wxUSE_COLOURPICKERCTRL

#if wxUSE_DIRPICKERCTRL

using DirPickerCtrlTest = PickerBaseTest<wxDirPickerCtrl>;

TEST_CASE_FIXTURE(DirPickerCtrlTest, "Directory picker control test")
{
    m_pickctrl = std::make_unique<wxDirPickerCtrl>(
                                   wxTheApp->GetTopWindow(), wxID_ANY,
                                   "", wxDirSelectorPromptStr,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxDIRP_USE_TEXTCTRL);

    wxPICKER_BASE_TESTS();
}

#endif //wxUSE_DIRPICKERCTRL

#if wxUSE_FILEPICKERCTRL

using FilePickerCtrlTest = PickerBaseTest<wxFilePickerCtrl>;

TEST_CASE_FIXTURE(FilePickerCtrlTest, "Directory picker control test")
{
    m_pickctrl = std::make_unique<wxFilePickerCtrl>(
                                  wxTheApp->GetTopWindow(), wxID_ANY,
                                  "", wxFileSelectorPromptStr,
                                  wxFileSelectorDefaultWildcardStr,
                                  wxDefaultPosition, wxDefaultSize,
                                  wxFLP_USE_TEXTCTRL);

    wxPICKER_BASE_TESTS();
}

#endif //wxUSE_FILEPICKERCTRL

#if wxUSE_FONTPICKERCTRL

using FontPickerCtrlTest = PickerBaseTest<wxFontPickerCtrl>;

TEST_CASE_FIXTURE(FontPickerCtrlTest, "Directory picker control test")
{
    m_pickctrl = std::make_unique<wxFontPickerCtrl>(wxTheApp->GetTopWindow(), wxID_ANY,
                                                    wxNullFont, wxDefaultPosition, wxDefaultSize,
                                                    wxFNTP_USE_TEXTCTRL);

    SUBCASE("ColourSelection")
    {
        wxColour selectedColour(0xFF4269UL);

        CHECK_MESSAGE(m_pickctrl->GetSelectedColour() == wxColour(*wxBLACK),
                      "Default font picker color must be black");

        m_pickctrl->SetSelectedColour(selectedColour);

        CHECK_MESSAGE(m_pickctrl->GetSelectedColour() == selectedColour,
                      "Font picker did not react to color selection");
    }

    wxPICKER_BASE_TESTS();
}

#endif //wxUSE_FONTPICKERCTRL

#endif
