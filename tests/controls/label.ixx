///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/label.cpp
// Purpose:     wxControl and wxStaticText label tests
// Author:      Francesco Montorsi
// Created:     2010-3-21
// Copyright:   (c) 2010 Francesco Montorsi
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/checkbox.h"
#include "wx/control.h"
#include "wx/stattext.h"

#include "wx/generic/stattextg.h"

export module WX.Test.Label;

import WX.MetaTest;
import WX.Test.Prec;

namespace ut = boost::ut;

constexpr char ORIGINAL_LABEL[] = "origin label";

// The actual testing function. It will change the label of the provided
// control, which is assumed to be ORIGINAL_LABEL initially.
static void DoTestLabel(wxControl* c)
{
    using namespace ut;

    expect( c->GetLabel() == ORIGINAL_LABEL );

    static const std::array<std::string, 7> testLabelArray = {
        "label without mnemonics and markup",
        "label with &mnemonic",
        "label with <span foreground='blue'>some</span> <b>markup</b>",
        "label with <span foreground='blue'>some</span> <b>markup</b> and &mnemonic",
        "label with an && (ampersand)",
        "label with an && (&ampersand)",
        "", // empty label should work too
    };

    for ( const auto& label : testLabelArray )
    {
        // GetLabel() should always return the string passed to SetLabel()
        c->SetLabel(label);
        expect( c->GetLabel() == label );

        // GetLabelText() should always return unescaped version of the label
        expect( c->GetLabelText() == wxControl::RemoveMnemonics(label) );

        // GetLabelText() should always return the string passed to SetLabelText()
        c->SetLabelText(label);
        expect( c->GetLabelText() == label );

        // And GetLabel() should be the escaped version of the text
        expect( label == wxControl::RemoveMnemonics(c->GetLabel()) );
    }

    // Check that both "&" and "&amp;" work in markup.
#if wxUSE_MARKUP
    c->SetLabelMarkup("mnemonic in &amp;markup");
    expect( c->GetLabel() == "mnemonic in &markup" );
    expect( c->GetLabelText() == "mnemonic in markup" );

    c->SetLabelMarkup("mnemonic in &markup");
    expect( c->GetLabel() == "mnemonic in &markup" );
    expect( c->GetLabelText() == "mnemonic in markup" );

    c->SetLabelMarkup("&amp;&amp; finally");
    expect( c->GetLabel() == "&& finally" );
    expect( c->GetLabelText() == "& finally" );

    c->SetLabelMarkup("&& finally");
    expect( c->GetLabel() == "&& finally" );
    expect( c->GetLabelText() == "& finally" );
#endif // wxUSE_MARKUP
}

ut::suite ControlLabelTest = []
{
    using namespace ut;

    "wxStaticText"_test = [&]
    {
        const auto st = std::make_unique<wxStaticText>(wxTheApp->GetTopWindow(),
                                                       wxID_ANY, ORIGINAL_LABEL);
        DoTestLabel(st.get());
    };

    "wxStaticText/ellipsized"_test = [&]
    {
        const auto st = std::make_unique<wxStaticText>(wxTheApp->GetTopWindow(),
                                                       wxID_ANY, ORIGINAL_LABEL,
                                                       wxDefaultPosition,
                                                       wxDefaultSize,
                                                       wxST_ELLIPSIZE_START);
        DoTestLabel(st.get());
    };

    "wxGenericStaticText"_test = [&]
    {
        const auto gst = std::make_unique<wxGenericStaticText>(wxTheApp->GetTopWindow(),
                                                               wxID_ANY, ORIGINAL_LABEL);
        DoTestLabel(gst.get());
    };

    "wxCheckBox"_test = [&]
    {
        const auto cb = std::make_unique<wxCheckBox>(wxTheApp->GetTopWindow(),
                                                     wxID_ANY, ORIGINAL_LABEL);
        DoTestLabel(cb.get());
    };
};

ut::suite ControlRemoveMnemonicsTest = []
{
    using namespace ut;

    expect( "mnemonic"  == wxControl::RemoveMnemonics("&mnemonic") );
    expect( "&mnemonic" == wxControl::RemoveMnemonics("&&mnemonic") );
    expect( "&mnemonic" == wxControl::RemoveMnemonics("&&&mnemonic") );
};

ut::suite ControlFindAccelIndexTest = []
{
    using namespace ut;

    expect( wxControl::FindAccelIndex("foo") == wxNOT_FOUND );
    expect( wxControl::FindAccelIndex("&foo") == 0 );
    expect( wxControl::FindAccelIndex("f&oo") == 1 );
    expect( wxControl::FindAccelIndex("foo && bar") == wxNOT_FOUND );
    expect( wxControl::FindAccelIndex("foo && &bar") == 6 );
};
