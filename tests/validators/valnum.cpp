///////////////////////////////////////////////////////////////////////////////
// Name:        tests/validators/valnum.cpp
// Purpose:     Unit tests for numeric validators.
// Author:      Vadim Zeitlin
// Created:     2011-01-18
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "wx/app.h"
#include "wx/intl.h"
#include "wx/textctrl.h"
#include "wx/validate.h"
#include "wx/valnum.h"
#include "wx/scopeguard.h"
#include "wx/uiaction.h"

#include "testableframe.h"

import WX.Test.Prec;

class NumValidatorTestCase
{
public:
    NumValidatorTestCase() :
        m_text(std::make_unique<wxTextCtrl>(wxTheApp->GetTopWindow(), wxID_ANY)) {}

protected:
    std::unique_ptr<wxTextCtrl> m_text;
};


TEST_CASE_FIXTURE(NumValidatorTestCase, "ValNum::TransferInt")
{
    int value = 0;
    wxIntegerValidator<int> valInt(&value);
    valInt.SetWindow(m_text.get());

    CHECK( valInt.TransferToWindow() );
    CHECK( m_text->GetValue() == "0" );

    value = 17;
    CHECK( valInt.TransferToWindow() );
    CHECK( m_text->GetValue() == "17" );


    m_text->ChangeValue("foobar");
    CHECK( !valInt.TransferFromWindow() );

    m_text->ChangeValue("-234");
    CHECK( valInt.TransferFromWindow() );
    CHECK( value == -234 );

    m_text->ChangeValue("9223372036854775808"); // == LLONG_MAX + 1
    CHECK( !valInt.TransferFromWindow() );

    m_text->Clear();
    CHECK( !valInt.TransferFromWindow() );
}

TEST_CASE_FIXTURE(NumValidatorTestCase, "ValNum::TransferUnsigned")
{
    unsigned value = 0;
    wxIntegerValidator<unsigned> valUnsigned(&value);
    valUnsigned.SetWindow(m_text.get());

    CHECK( valUnsigned.TransferToWindow() );
    CHECK( m_text->GetValue() == "0" );

    value = 17;
    CHECK( valUnsigned.TransferToWindow() );
    CHECK( m_text->GetValue() == "17" );


    m_text->ChangeValue("foobar");
    CHECK( !valUnsigned.TransferFromWindow() );

    m_text->ChangeValue("-234");
    CHECK( !valUnsigned.TransferFromWindow() );

    m_text->ChangeValue("234");
    CHECK( valUnsigned.TransferFromWindow() );
    CHECK( value == 234 );

    m_text->ChangeValue("4294967295"); // == ULONG_MAX in 32 bits
    CHECK( valUnsigned.TransferFromWindow() );
    CHECK( value == std::numeric_limits<std::uint32_t>::max() );
    CHECK( valUnsigned.TransferToWindow() );
    CHECK( m_text->GetValue() == "4294967295" );

    m_text->ChangeValue("4294967296"); // == ULONG_MAX + 1
    CHECK( !valUnsigned.TransferFromWindow() );

    m_text->ChangeValue("18446744073709551616"); // == ULLONG_MAX + 1
    CHECK( !valUnsigned.TransferFromWindow() );

    m_text->Clear();
    CHECK( !valUnsigned.TransferFromWindow() );
}

TEST_CASE_FIXTURE(NumValidatorTestCase, "ValNum::TransferULL")
{
    unsigned long long value = 0;
    wxIntegerValidator<unsigned long long> valULL(&value);
    valULL.SetWindow(m_text.get());

    SUBCASE("LLONG_MAX")
    {
        m_text->ChangeValue("9223372036854775807"); // == int64_t max
        REQUIRE( valULL.TransferFromWindow() );
        CHECK( value == static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max() ) );

        REQUIRE( valULL.TransferToWindow() );
        CHECK( m_text->GetValue() == "9223372036854775807" );
    }

    SUBCASE("LLONG_MAX+1")
    {
        m_text->ChangeValue("9223372036854775808"); // == int64_t max + 1
        REQUIRE( valULL.TransferFromWindow() );
        CHECK( value == static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max() ) + 1 );

        REQUIRE( valULL.TransferToWindow() );
        CHECK( m_text->GetValue() == "9223372036854775808" );
    }

    SUBCASE("ULLONG_MAX")
    {
        m_text->ChangeValue("18446744073709551615"); // == uint64_t max
        REQUIRE( valULL.TransferFromWindow() );
        CHECK( value == std::numeric_limits<std::uint64_t>::max() );

        REQUIRE( valULL.TransferToWindow() );
        CHECK( m_text->GetValue() == "18446744073709551615" );
    }

    SUBCASE("ULLONG_MAX + 1")
    {
        m_text->ChangeValue("18446744073709551616"); // == uint64_t max + 1
        CHECK( !valULL.TransferFromWindow() );
    }
}

TEST_CASE_FIXTURE(NumValidatorTestCase, "ValNum::TransferFloat")
{
    // We need a locale with point as decimal separator.
    wxLocale loc(wxLANGUAGE_ENGLISH_UK, wxLOCALE_DONT_LOAD_DEFAULT);

    float value = 0;
    wxFloatingPointValidator<float> valFloat(3, &value);
    valFloat.SetWindow(m_text.get());

    CHECK( valFloat.TransferToWindow() );
    CHECK( m_text->GetValue() == "0.000" );

    value = 1.234f;
    CHECK( valFloat.TransferToWindow() );
    CHECK( m_text->GetValue() == "1.234" );

    value = 1.2345678f;
    CHECK( valFloat.TransferToWindow() );
    CHECK( m_text->GetValue() == "1.235" );


    m_text->ChangeValue("foobar");
    CHECK( !valFloat.TransferFromWindow() );

    m_text->ChangeValue("-234.567");
    CHECK( valFloat.TransferFromWindow() );
    CHECK( value == -234.567f );

    m_text->Clear();
    CHECK( !valFloat.TransferFromWindow() );
}

TEST_CASE_FIXTURE(NumValidatorTestCase, "ValNum::ZeroAsBlank")
{
    long value = 0;
    m_text->SetValidator(
        wxMakeIntegerValidator(&value, wxNUM_VAL_ZERO_AS_BLANK));

    wxValidator * const val = m_text->GetValidator();

    CHECK( val->TransferToWindow() );
    CHECK( m_text->GetValue().empty() );

    value++;
    CHECK( val->TransferFromWindow() );
    CHECK( value == 0 );
}

TEST_CASE_FIXTURE(NumValidatorTestCase, "ValNum::NoTrailingZeroes")
{
    // We need a locale with point as decimal separator.
    wxLocale loc(wxLANGUAGE_ENGLISH_UK, wxLOCALE_DONT_LOAD_DEFAULT);

    double value = 1.2;
    m_text->SetValidator(
        wxMakeFloatingPointValidator(3, &value, wxNUM_VAL_NO_TRAILING_ZEROES));

    wxValidator * const val = m_text->GetValidator();

    CHECK( val->TransferToWindow() );
    CHECK( m_text->GetValue() == "1.2" );

    value = 1.234;
    CHECK( val->TransferToWindow() );
    CHECK( m_text->GetValue() == "1.234" );
}

#if wxUSE_UIACTIONSIMULATOR

TEST_CASE_FIXTURE(NumValidatorTestCase, "ValNum::Interactive")
{
#ifdef __WXMSW__
    // FIXME: This test fails on MSW buildbot slaves although works fine on
    //        development machine, no idea why. It seems to be a problem with
    //        wxUIActionSimulator rather the wxListCtrl control itself however.
    if ( IsAutomaticTest() )
        return;
#endif // __WXMSW__

    // Set a locale using comma as thousands separator character.
    wxLocale loc(wxLANGUAGE_ENGLISH_UK, wxLOCALE_DONT_LOAD_DEFAULT);

    m_text->SetValidator(
        wxIntegerValidator<unsigned>(nullptr, wxNUM_VAL_THOUSANDS_SEPARATOR));

    // Create a sibling text control to be able to switch focus and thus
    // trigger the control validation/normalization.
    wxTextCtrl * const text2 = new wxTextCtrl(m_text->GetParent(), wxID_ANY);
    wxON_BLOCK_EXIT_OBJ0( *text2, wxWindow::Destroy );
    text2->Move(wxPoint{10, 80}); // Just to see it better while debugging...
    wxFloatingPointValidator<float> valFloat(3);
    valFloat.SetRange(-10., 10.);
    text2->SetValidator(valFloat);

    wxUIActionSimulator sim;

    // Entering '-' in a control with positive range is not allowed.
    m_text->SetFocus();
    sim.Char('-');
    wxYield();
    CHECK( m_text->GetValue().empty() );

    // Neither is entering '.' or any non-digit character.
    sim.Text(".a+/");
    wxYield();
    CHECK( m_text->GetValue().empty() );

    // Entering digits should work though and after leaving the control the
    // contents should be normalized.
    sim.Text("1234567");
    wxYield();
    text2->SetFocus();
    wxYield();
    if ( loc.IsOk() )
        CHECK( m_text->GetValue() == "1,234,567" );
    else
        CHECK( m_text->GetValue() == "1234567" );


    // Entering both '-' and '.' in this control should work but only in the
    // correct order.
    sim.Char('-');
    wxYield();
    CHECK( text2->GetValue() == "-" );

    text2->SetInsertionPoint(0);
    sim.Char('.');
    wxYield();
    CHECK( text2->GetValue() == "-" );

    text2->SetInsertionPointEnd();
    sim.Char('.');
    wxYield();
    CHECK( text2->GetValue() == "-." );

    // Adding up to three digits after the point should work.
    sim.Text("987");
    wxYield();
    CHECK( text2->GetValue() == "-.987" );

    // But no more.
    sim.Text("654");
    wxYield();
    CHECK( text2->GetValue() == "-.987" );

    // We can remove one digit and another one though.
    sim.Char(WXK_BACK);
    sim.Char(WXK_BACK);
    sim.Char('6');
    wxYield();
    CHECK( text2->GetValue() == "-.96" );


    // Also test the range constraint.
    text2->Clear();

    sim.Char('9');
    wxYield();
    CHECK( text2->GetValue() == "9" );

    sim.Char('9');
    wxYield();
    CHECK( text2->GetValue() == "9" );
}

#endif // wxUSE_UIACTIONSIMULATOR
