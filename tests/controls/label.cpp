///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/label.cpp
// Purpose:     wxControl and wxStaticText label tests
// Author:      Francesco Montorsi
// Created:     2010-3-21
// Copyright:   (c) 2010 Francesco Montorsi
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "wx/app.h"
#include "wx/checkbox.h"
#include "wx/control.h"
#include "wx/stattext.h"

#include "wx/generic/stattextg.h"

import WX.Test.Prec;


constexpr char ORIGINAL_LABEL[] = "origin label";

// The actual testing function. It will change the label of the provided
// control, which is assumed to be ORIGINAL_LABEL initially.
static void DoTestLabel(wxControl* c)
{
    CHECK( c->GetLabel() == ORIGINAL_LABEL );

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
        CHECK( c->GetLabel() == label );

        // GetLabelText() should always return unescaped version of the label
        CHECK( c->GetLabelText() == wxControl::RemoveMnemonics(label) );

        // GetLabelText() should always return the string passed to SetLabelText()
        c->SetLabelText(label);
        CHECK( c->GetLabelText() == label );

        // And GetLabel() should be the escaped version of the text
        CHECK( label == wxControl::RemoveMnemonics(c->GetLabel()) );
    }

    // Check that both "&" and "&amp;" work in markup.
#if wxUSE_MARKUP
    c->SetLabelMarkup("mnemonic in &amp;markup");
    CHECK( c->GetLabel() == "mnemonic in &markup" );
    CHECK( c->GetLabelText() == "mnemonic in markup" );

    c->SetLabelMarkup("mnemonic in &markup");
    CHECK( c->GetLabel() == "mnemonic in &markup" );
    CHECK( c->GetLabelText() == "mnemonic in markup" );

    c->SetLabelMarkup("&amp;&amp; finally");
    CHECK( c->GetLabel() == "&& finally" );
    CHECK( c->GetLabelText() == "& finally" );

    c->SetLabelMarkup("&& finally");
    CHECK( c->GetLabel() == "&& finally" );
    CHECK( c->GetLabelText() == "& finally" );
#endif // wxUSE_MARKUP
}

TEST_CASE("wxControl::Label")
{
    SUBCASE("wxStaticText")
    {
        const auto st = std::make_unique<wxStaticText>(wxTheApp->GetTopWindow(),
                                                       wxID_ANY, ORIGINAL_LABEL);
        DoTestLabel(st.get());
    }

    SUBCASE("wxStaticText/ellipsized")
    {
        const auto st = std::make_unique<wxStaticText>(wxTheApp->GetTopWindow(),
                                                       wxID_ANY, ORIGINAL_LABEL,
                                                       wxDefaultPosition,
                                                       wxDefaultSize,
                                                       wxST_ELLIPSIZE_START);
        DoTestLabel(st.get());
    }

    SUBCASE("wxGenericStaticText")
    {
        const auto gst = std::make_unique<wxGenericStaticText>(wxTheApp->GetTopWindow(),
                                                               wxID_ANY, ORIGINAL_LABEL);
        DoTestLabel(gst.get());
    }

    SUBCASE("wxCheckBox")
    {
        const auto cb = std::make_unique<wxCheckBox>(wxTheApp->GetTopWindow(),
                                                     wxID_ANY, ORIGINAL_LABEL);
        DoTestLabel(cb.get());
    }
}

TEST_CASE("wxControl::RemoveMnemonics")
{
    CHECK( "mnemonic"  == wxControl::RemoveMnemonics("&mnemonic") );
    CHECK( "&mnemonic" == wxControl::RemoveMnemonics("&&mnemonic") );
    CHECK( "&mnemonic" == wxControl::RemoveMnemonics("&&&mnemonic") );
}

TEST_CASE("wxControl::FindAccelIndex")
{
    CHECK( wxControl::FindAccelIndex("foo") == wxNOT_FOUND );
    CHECK( wxControl::FindAccelIndex("&foo") == 0 );
    CHECK( wxControl::FindAccelIndex("f&oo") == 1 );
    CHECK( wxControl::FindAccelIndex("foo && bar") == wxNOT_FOUND );
    CHECK( wxControl::FindAccelIndex("foo && &bar") == 6 );
}
