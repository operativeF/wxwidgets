///////////////////////////////////////////////////////////////////////////////
// Name:        tests/strings/numformat.cpp
// Purpose:     wxNumberFormatter unit test
// Author:      Vadim Zeitlin
// Created:     2011-01-15
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#include "wx/numformatter.h"
#include "wx/intl.h"

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

namespace
{

class NumFormatterTestCase
{
public:
    NumFormatterTestCase() :
        // We need to use a locale with known decimal point and which uses the
        // thousands separator for the tests to make sense.
        m_locale(wxLANGUAGE_ENGLISH_UK, wxLOCALE_DONT_LOAD_DEFAULT)
    {
    }

protected:
    bool CanRunTest() const { return m_locale.IsOk(); }

private:
    wxLocale m_locale;

    NumFormatterTestCase(const NumFormatterTestCase&) = delete;
	NumFormatterTestCase& operator=(const NumFormatterTestCase&) = delete;
};

// A couple of helpers to avoid writing over long expressions.
wxString ToStringWithoutTrailingZeroes(double val, int precision)
{
    return wxNumberFormatter::ToString
           (
            val,
            precision,
            wxNumberFormatter::Style_NoTrailingZeroes
           );
}

wxString ToStringWithTrailingZeroes(double val, int precision)
{
    return wxNumberFormatter::ToString
           (
            val,
            precision,
            wxNumberFormatter::Style_None
           );
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// tests themselves
// ----------------------------------------------------------------------------

TEST_CASE_FIXTURE(NumFormatterTestCase, "NumFormatter::LongToString")
{
    if ( !CanRunTest() )
        return;

    CHECK( wxNumberFormatter::ToString(         1L) ==           "1" );
    CHECK( wxNumberFormatter::ToString(        -1L) ==          "-1" );
    CHECK( wxNumberFormatter::ToString(        12L) ==          "12" );
    CHECK( wxNumberFormatter::ToString(       -12L) ==         "-12" );
    CHECK( wxNumberFormatter::ToString(       123L) ==         "123" );
    CHECK( wxNumberFormatter::ToString(      -123L) ==        "-123" );
    CHECK( wxNumberFormatter::ToString(      1234L) ==       "1,234" );
    CHECK( wxNumberFormatter::ToString(     -1234L) ==      "-1,234" );
    CHECK( wxNumberFormatter::ToString(     12345L) ==      "12,345" );
    CHECK( wxNumberFormatter::ToString(    -12345L) ==     "-12,345" );
    CHECK( wxNumberFormatter::ToString(    123456L) ==     "123,456" );
    CHECK( wxNumberFormatter::ToString(   -123456L) ==    "-123,456" );
    CHECK( wxNumberFormatter::ToString(   1234567L) ==   "1,234,567" );
    CHECK( wxNumberFormatter::ToString(  -1234567L) ==  "-1,234,567" );
    CHECK( wxNumberFormatter::ToString(  12345678L) ==  "12,345,678" );
    CHECK( wxNumberFormatter::ToString( -12345678L) == "-12,345,678" );
    CHECK( wxNumberFormatter::ToString( 123456789L) == "123,456,789" );
}

#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

TEST_CASE_FIXTURE(NumFormatterTestCase, "NumFormatter::LongLongToString")
{
    if ( !CanRunTest() )
        return;

    CHECK( wxNumberFormatter::ToString(        1LL ==           "1" );
    CHECK( wxNumberFormatter::ToString(       12LL ==          "12" );
    CHECK( wxNumberFormatter::ToString(      123LL ==         "123" );
    CHECK( wxNumberFormatter::ToString(     1234LL ==       "1,234" );
    CHECK( wxNumberFormatter::ToString(    12345LL ==      "12,345" );
    CHECK( wxNumberFormatter::ToString(   123456LL ==     "123,456" );
    CHECK( wxNumberFormatter::ToString(  1234567LL ==   "1,234,567" );
    CHECK( wxNumberFormatter::ToString( 12345678LL ==  "12,345,678" );
    CHECK( wxNumberFormatter::ToString(123456789LL == "123,456,789" );
}

#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

TEST_CASE_FIXTURE(NumFormatterTestCase, "NumFormatter::DoubleToString")
{
    if ( !CanRunTest() )
        return;

    CHECK( wxNumberFormatter::ToString(           1.,  1) ==             "1.0" );
    CHECK( wxNumberFormatter::ToString(     0.123456,  6) ==        "0.123456" );
    CHECK( wxNumberFormatter::ToString(     1.234567,  6) ==        "1.234567" );
    CHECK( wxNumberFormatter::ToString(     12.34567,  5) ==        "12.34567" );
    CHECK( wxNumberFormatter::ToString(     123.4567,  4) ==        "123.4567" );
    CHECK( wxNumberFormatter::ToString(      1234.56,  2) ==        "1,234.56" );
    CHECK( wxNumberFormatter::ToString(      12345.6,  1) ==        "12,345.6" );
    CHECK( wxNumberFormatter::ToString(123456789.012,  3) == "123,456,789.012" );
    CHECK( wxNumberFormatter::ToString(    12345.012, -1) ==          "12,345" );

    CHECK( ToStringWithTrailingZeroes(-123.123, 4) == "-123.1230" );
    CHECK( ToStringWithTrailingZeroes(    0.02, 1) ==       "0.0" );
    CHECK( ToStringWithTrailingZeroes(   -0.02, 1) ==      "-0.0" );
}

TEST_CASE_FIXTURE(NumFormatterTestCase, "NumFormatter::NoTrailingZeroes")
{
    //CHECK_THROWS
    //(
    //    wxNumberFormatter::ToString(123L, wxNumberFormatter::Style_NoTrailingZeroes)
    //);

    if ( !CanRunTest() )
        return;

    CHECK( ToStringWithTrailingZeroes   (      123., 3) ==       "123.000" );
    CHECK( ToStringWithoutTrailingZeroes(      123., 3) ==           "123" );
    CHECK( ToStringWithoutTrailingZeroes(      123., 9) ==           "123" );
    CHECK( ToStringWithoutTrailingZeroes(   123.456, 3) ==       "123.456" );
    CHECK( ToStringWithTrailingZeroes   (   123.456, 9) == "123.456000000" );
    CHECK( ToStringWithoutTrailingZeroes(   123.456, 9) ==       "123.456" );
    CHECK( ToStringWithoutTrailingZeroes(   123.123, 2) ==        "123.12" );
    CHECK( ToStringWithoutTrailingZeroes(   123.123, 0) ==           "123" );
    CHECK( ToStringWithoutTrailingZeroes( -0.000123, 3) ==             "0" );
    CHECK( ToStringWithoutTrailingZeroes(     123., -1) ==           "123" );
    CHECK( ToStringWithoutTrailingZeroes(   1e-120, -1) ==        "1e-120" );
}

TEST_CASE_FIXTURE(NumFormatterTestCase, "NumFormatter::LongFromString")
{
    if ( !CanRunTest() )
        return;

    //CHECK_THROWS
    //(
    //    wxNumberFormatter::FromString("123", static_cast<long *>(0))
    //);

    long l;
    CHECK_FALSE( wxNumberFormatter::FromString("", &l) );
    CHECK_FALSE( wxNumberFormatter::FromString("foo", &l) );
    CHECK_FALSE( wxNumberFormatter::FromString("1.234", &l) );
    CHECK_FALSE( wxNumberFormatter::FromString("-", &l) );

    CHECK( wxNumberFormatter::FromString("0", &l) );
    CHECK( l == 0 );

    CHECK( wxNumberFormatter::FromString("123", &l) );
    CHECK( l == 123 );

    CHECK( wxNumberFormatter::FromString("1234", &l) );
    CHECK( l == 1234 );

    CHECK( wxNumberFormatter::FromString("1,234", &l) );
    CHECK( l == 1234 );

    CHECK( wxNumberFormatter::FromString("12,345", &l) );
    CHECK( l == 12345 );

    CHECK( wxNumberFormatter::FromString("123,456", &l) );
    CHECK( l == 123456 );

    CHECK( wxNumberFormatter::FromString("1,234,567", &l) );
    CHECK( l == 1234567 );

    CHECK( wxNumberFormatter::FromString("-123", &l) );
    CHECK( l == -123 );

    CHECK_FALSE( wxNumberFormatter::FromString("9223372036854775808", &l) );
}

#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

TEST_CASE_FIXTURE(NumFormatterTestCase, "NumFormatter::LongLongFromString")
{
    if ( !CanRunTest() )
        return;

    //CHECK_THROWS
    //(
    //    wxNumberFormatter::FromString("123", static_cast<wxLongLong_t *>(0))
    //);

    wxLongLong_t l;
    CHECK_FALSE( wxNumberFormatter::FromString("", &l) );
    CHECK_FALSE( wxNumberFormatter::FromString("foo", &l) );
    CHECK_FALSE( wxNumberFormatter::FromString("1.234", &l) );

    // This somehow succeeds with gcc 4.8.4 under Ubuntu and MinGW 5.3, so
    // don't use CHECK() for it.
    // FIXME: Doctest: too complex
    //if ( wxNumberFormatter::FromString("-", &l) )
    //{
    //    WARN("Converting \"-\" to long long unexpectedly succeeded, result: " << l);
    //}

    CHECK( wxNumberFormatter::FromString("0", &l) );
    CHECK( l == 0 );

    CHECK( wxNumberFormatter::FromString("123", &l) );
    CHECK( l == 123 );

    CHECK( wxNumberFormatter::FromString("1234", &l) );
    CHECK( l == 1234 );

    CHECK( wxNumberFormatter::FromString("1,234", &l) );
    CHECK( l == 1234 );

    CHECK( wxNumberFormatter::FromString("12,345", &l) );
    CHECK( l == 12345 );

    CHECK( wxNumberFormatter::FromString("123,456", &l) );
    CHECK( l == 123456 );

    CHECK( wxNumberFormatter::FromString("1,234,567", &l) );
    CHECK( l == 1234567 );

    CHECK( wxNumberFormatter::FromString("-123", &l) );
    CHECK( l == -123 );

    CHECK( wxNumberFormatter::FromString("9223372036854775807", &l) );
    CHECK( l == std::numeric_limits<std::int64_t>::max()  );

    CHECK_FALSE( wxNumberFormatter::FromString("9223372036854775808", &l) );
}

#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

TEST_CASE_FIXTURE(NumFormatterTestCase, "NumFormatter::ULongLongFromString")
{
    if ( !CanRunTest() )
        return;

    wxULongLong_t u;
    CHECK_FALSE( wxNumberFormatter::FromString("", &u) );
    CHECK_FALSE( wxNumberFormatter::FromString("bar", &u) );
    CHECK_FALSE( wxNumberFormatter::FromString("1.234", &u) );
    CHECK_FALSE( wxNumberFormatter::FromString("-2", &u) );
    CHECK_FALSE( wxNumberFormatter::FromString("-", &u) );

    CHECK( wxNumberFormatter::FromString("0", &u) );
    CHECK( u == 0 );

    CHECK( wxNumberFormatter::FromString("123", &u) );
    CHECK( u == 123 );

    CHECK( wxNumberFormatter::FromString("1234", &u) );
    CHECK( u == 1234 );

    CHECK( wxNumberFormatter::FromString("1,234", &u) );
    CHECK( u == 1234 );

    CHECK( wxNumberFormatter::FromString("12,345", &u) );
    CHECK( u == 12345 );

    CHECK( wxNumberFormatter::FromString("123,456", &u) );
    CHECK( u == 123456 );

    CHECK( wxNumberFormatter::FromString("1,234,567", &u) );
    CHECK( u == 1234567 );

    CHECK( wxNumberFormatter::FromString("9223372036854775807", &u) );
    CHECK( u == static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) );

    CHECK( wxNumberFormatter::FromString("9223372036854775808", &u) );
    CHECK( u == static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max() ) + 1 );

    CHECK( wxNumberFormatter::FromString("18446744073709551615", &u) );
    CHECK( u == std::numeric_limits<std::uint64_t>::max() );

    CHECK_FALSE( wxNumberFormatter::FromString("18446744073709551616", &u) );
}

TEST_CASE_FIXTURE(NumFormatterTestCase, "NumFormatter::DoubleFromString")
{
    if ( !CanRunTest() )
        return;

    //CHECK_THROWS
    //(
    //    wxNumberFormatter::FromString("123", static_cast<double *>(0))
    //);

    double d;
    CHECK_FALSE( wxNumberFormatter::FromString("", &d) );
    CHECK_FALSE( wxNumberFormatter::FromString("bar", &d) );

    CHECK( wxNumberFormatter::FromString("123", &d) );
    CHECK( d == 123. );

    CHECK( wxNumberFormatter::FromString("123.456789012", &d) );
    CHECK( d == 123.456789012 );

    CHECK( wxNumberFormatter::FromString("1,234.56789012", &d) );
    CHECK( d == 1234.56789012 );

    CHECK( wxNumberFormatter::FromString("12,345.6789012", &d) );
    CHECK( d == 12345.6789012 );

    CHECK( wxNumberFormatter::FromString("123,456.789012", &d) );
    CHECK( d == 123456.789012 );

    CHECK( wxNumberFormatter::FromString("1,234,567.89012", &d) );
    CHECK( d == 1234567.89012 );

    CHECK( wxNumberFormatter::FromString("12,345,678.9012", &d) );
    CHECK( d == 12345678.9012 );

    CHECK( wxNumberFormatter::FromString("123,456,789.012", &d) );
    CHECK( d == 123456789.012 );

    CHECK( wxNumberFormatter::FromString("123456789.012", &d) );
    CHECK( d == 123456789.012 );
}
