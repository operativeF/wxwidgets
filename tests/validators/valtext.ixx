///////////////////////////////////////////////////////////////////////////////
// Name:        tests/validators/valtext.cpp
// Purpose:     wxTextValidator unit test
// Author:      Ali Kettab
// Created:     2019-01-01
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "testprec.h"

#include "wx/app.h"
#include "wx/textctrl.h"
#include "wx/valtext.h"

#include "wx/uiaction.h"

export module WX.Test.TextValidator;

#if wxUSE_VALIDATORS && wxUSE_UIACTIONSIMULATOR

class TextValidatorTestCase
{
public:
    TextValidatorTestCase()
        : m_text(std::make_unique<wxTextCtrl>(wxTheApp->GetTopWindow(), wxID_ANY))
    {
    }

protected:
    std::unique_ptr<wxTextCtrl> m_text;
};

#define TEXT_VALIDATOR_TEST_CASE(name) \
    TEST_CASE_FIXTURE(TextValidatorTestCase, name)

TEXT_VALIDATOR_TEST_CASE("wxTextValidator::IsValid")
{
    wxString value = "";
    wxTextValidator val(wxFILTER_NONE, &value);

    SUBCASE("wxFILTER_NONE - no filtering should take place")
    {
        CHECK( val.IsValid("wx-90.?! @_~E+{").empty() );
    }

    SUBCASE("wxFILTER_EMPTY - empty strings are filtered out")
    {
        val.SetStyle(wxFILTER_EMPTY);

        CHECK( !val.IsValid("").empty() );
        CHECK( val.IsValid(" ").empty() ); // space is valid
    }

    SUBCASE("wxFILTER_ASCII - non-ASCII characters are filtered out")
    {
        val.SetStyle(wxFILTER_ASCII);

        CHECK( val.IsValid("wx-90.?! @_~E+{").empty() );
    }

    SUBCASE("wxFILTER_ALPHA - non-alpha characters are filtered out")
    {
        val.SetStyle(wxFILTER_ALPHA);

        CHECK( val.IsValid("wx").empty() );
        CHECK( !val.IsValid("wx_").empty() ); // _ is not alpha
    }

    SUBCASE("wxFILTER_ALPHANUMERIC - non-alphanumeric characters are filtered out")
    {
        val.SetStyle(wxFILTER_ALPHANUMERIC);

        CHECK( val.IsValid("wx01").empty() );
        CHECK( !val.IsValid("wx 01").empty() ); // 'space' is not alphanumeric
    }

    SUBCASE("wxFILTER_DIGITS - non-digit characters are filtered out")
    {
        val.SetStyle(wxFILTER_DIGITS);

        CHECK( val.IsValid("97").empty() );
        CHECK( !val.IsValid("9.7").empty() ); // . is not digit
    }

    SUBCASE("wxFILTER_XDIGITS - non-xdigit characters are filtered out")
    {
        val.SetStyle(wxFILTER_XDIGITS);

        CHECK( val.IsValid("90AEF").empty() );
        CHECK( !val.IsValid("90GEF").empty() ); // G is not xdigit
    }

    SUBCASE("wxFILTER_NUMERIC - non-numeric characters are filtered out")
    {
        val.SetStyle(wxFILTER_NUMERIC);

        CHECK( val.IsValid("+90.e-2").empty() );
        CHECK( !val.IsValid("-8.5#0").empty() ); // # is not numeric
    }

    SUBCASE("wxFILTER_INCLUDE_LIST - use include list")
    {
        val.SetStyle(wxFILTER_INCLUDE_LIST);

        std::vector<wxString> includes = {
            "wxMSW",
            "wxGTK",
            "wxOSX"
        };

        val.SetIncludes(includes);

        CHECK( val.IsValid("wxGTK").empty() );
        CHECK( !val.IsValid("wxQT").empty() ); // wxQT is not included

        SUBCASE("wxFILTER_EXCLUDE_LIST - use exclude with include list")
        {
            std::vector<wxString> excludes;
            excludes.push_back("wxGTK");
            excludes.push_back("wxGTK1");
            val.SetExcludes(excludes);

            CHECK( val.IsValid("wxOSX").empty() );
            CHECK( !val.IsValid("wxGTK").empty() ); // wxGTK now excluded
        }
    }

    SUBCASE("wxFILTER_EXCLUDE_LIST - use exclude list")
    {
        val.SetStyle(wxFILTER_EXCLUDE_LIST);

        std::vector<wxString> excludes = {
            "wxMSW",
            "wxGTK",
            "wxOSX"
        };

        val.SetExcludes(excludes);

        CHECK( val.IsValid("wxQT & wxUNIV").empty() );
        CHECK( !val.IsValid("wxMSW").empty() ); // wxMSW is excluded

        SUBCASE("wxFILTER_INCLUDE_LIST - use include with exclude list")
        {
            std::vector<wxString> includes;
            includes.push_back("wxGTK");
            val.SetIncludes(includes); // exclusion takes priority over inclusion.

            CHECK( val.IsValid("wxUNIV").empty() );
            CHECK( !val.IsValid("wxMSW").empty() ); // wxMSW still excluded
        }
    }

    SUBCASE("wxFILTER_INCLUDE_CHAR_LIST - use include char list")
    {
        val.SetStyle(wxFILTER_INCLUDE_CHAR_LIST);
        val.SetCharIncludes("tuvwxyz.012+-");

        CHECK( val.IsValid("0.2t+z-1").empty() );
        CHECK( !val.IsValid("x*y").empty() ); // * is not included

        val.AddCharIncludes("*");

        CHECK( val.IsValid("x*y").empty() ); // * now included
        CHECK( !val.IsValid("x%y").empty() ); // % is not included

        val.AddCharExcludes("*"); // exclusion takes priority over inclusion.

        CHECK( !val.IsValid("x*y").empty() ); // * now excluded
    }

    SUBCASE("wxFILTER_EXCLUDE_CHAR_LIST - use exclude char list")
    {
        val.SetStyle(wxFILTER_EXCLUDE_CHAR_LIST);
        val.SetCharExcludes("tuvwxyz.012+-");

        CHECK( val.IsValid("A*B=?").empty() );
        CHECK( !val.IsValid("0.6/t").empty() ); // t is excluded

        val.AddCharIncludes("t"); // exclusion takes priority over inclusion.

        CHECK( !val.IsValid("0.6/t").empty() ); // t still excluded
    }
}

TEXT_VALIDATOR_TEST_CASE("wxTextValidator::TransferToWindow")
{
    wxString value = "wxwidgets";
    wxTextValidator val(wxFILTER_ALPHA, &value);
    m_text->SetValidator(val);

    CHECK( m_text->IsEmpty() );

    REQUIRE( m_text->TransferDataToWindow() );

    CHECK( m_text->GetValue() == "wxwidgets" );
}

TEXT_VALIDATOR_TEST_CASE("wxTextValidator::TransferFromWindow")
{
    wxString value;
    wxTextValidator val(wxFILTER_ALPHA, &value);
    m_text->SetValidator(val);

    m_text->ChangeValue("wxwidgets");

    REQUIRE( m_text->TransferDataFromWindow() );

    CHECK( value == "wxwidgets" );
}

#endif // wxUSE_VALIDATORS && wxUSE_UIACTIONSIMULATOR
