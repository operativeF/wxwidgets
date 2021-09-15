///////////////////////////////////////////////////////////////////////////////
// Name:        tests/cmdline/cmdlinetest.cpp
// Purpose:     wxCmdLineParser unit test
// Author:      Vadim Zeitlin
// Created:     2008-04-12
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#ifndef WX_PRECOMP
#endif // WX_PRECOMP

#include "wx/cmdline.h"
#include "wx/msgout.h"
#include "wx/scopeguard.h"

#include "testdate.h"

// --------------------------------------------------------------------------
// test class
// --------------------------------------------------------------------------

// Use this macro to compare a wxArrayString with the pipe-separated elements
// of the given string
//
// NB: it's a macro and not a function to have the correct line numbers in the
//     test failure messages
#define WX_ASSERT_STRARRAY_EQUAL(s, a)                                        \
    {                                                                         \
        std::vector<wxString> expected(wxSplit(s, '|', '\0'));                \
                                                                              \
        CHECK_EQ( expected.size(), a.size() );                    \
                                                                              \
        for ( size_t n = 0; n < a.size(); n++ )                               \
        {                                                                     \
            CHECK_EQ( expected[n], a[n] );                        \
        }                                                                     \
    }

// ============================================================================
// implementation
// ============================================================================

TEST_CASE("ConvertStringTestCase")
{
    #define WX_ASSERT_DOS_ARGS_EQUAL(s, args)                                 \
        {                                                                     \
            const std::vector<wxString>                                       \
                argsDOS(wxCmdLineParser::ConvertStringToArgs(args,            \
                                            wxCMD_LINE_SPLIT_DOS));           \
            WX_ASSERT_STRARRAY_EQUAL(s, argsDOS);                             \
        }

    #define WX_ASSERT_UNIX_ARGS_EQUAL(s, args)                                \
        {                                                                     \
            const std::vector<wxString>                                       \
                argsUnix(wxCmdLineParser::ConvertStringToArgs(args,           \
                                            wxCMD_LINE_SPLIT_UNIX));          \
            WX_ASSERT_STRARRAY_EQUAL(s, argsUnix);                            \
        }

    #define WX_ASSERT_ARGS_EQUAL(s, args)                                     \
        WX_ASSERT_DOS_ARGS_EQUAL(s, args)                                     \
        WX_ASSERT_UNIX_ARGS_EQUAL(s, args)

    // normal cases
    WX_ASSERT_ARGS_EQUAL( "foo", "foo" )
    WX_ASSERT_ARGS_EQUAL( "foo bar", "\"foo bar\"" )
    WX_ASSERT_ARGS_EQUAL( "foo|bar", "foo bar" )
    WX_ASSERT_ARGS_EQUAL( "foo|bar|baz", "foo bar baz" )
    WX_ASSERT_ARGS_EQUAL( "foo|bar baz", "foo \"bar baz\"" )

    // special cases
    WX_ASSERT_ARGS_EQUAL( "", "" )
    WX_ASSERT_ARGS_EQUAL( "foo", "foo " )
    WX_ASSERT_ARGS_EQUAL( "foo", "foo \t   " )
    WX_ASSERT_ARGS_EQUAL( "foo|bar", "foo bar " )
    WX_ASSERT_ARGS_EQUAL( "foo|bar|", "foo bar \"" )
    WX_ASSERT_DOS_ARGS_EQUAL( "foo|bar|\\", "foo bar \\" )
    WX_ASSERT_UNIX_ARGS_EQUAL( "foo|bar|", "foo bar \\" )

    WX_ASSERT_ARGS_EQUAL( "12 34", "1\"2 3\"4" );
    WX_ASSERT_ARGS_EQUAL( "1|2 34", "1 \"2 3\"4" );
    WX_ASSERT_ARGS_EQUAL( "1|2 3|4", "1 \"2 3\" 4" );

    // check for (broken) Windows semantics: backslash doesn't escape spaces
    WX_ASSERT_DOS_ARGS_EQUAL( "\\\\foo\\\\|/bar", "\"\\\\foo\\\\\" /bar" );
    WX_ASSERT_DOS_ARGS_EQUAL( "foo|bar\\|baz", "foo bar\\ baz" );
    WX_ASSERT_DOS_ARGS_EQUAL( "foo|bar\\\"baz", "foo \"bar\\\"baz\"" );

    // check for more sane Unix semantics: backslash does escape spaces and
    // quotes
    WX_ASSERT_UNIX_ARGS_EQUAL( "foo|bar baz", "foo bar\\ baz" );
    WX_ASSERT_UNIX_ARGS_EQUAL( "foo|bar\"baz", "foo \"bar\\\"baz\"" );

    // check that single quotes work too with Unix semantics
    WX_ASSERT_UNIX_ARGS_EQUAL( "foo bar", "'foo bar'" )
    WX_ASSERT_UNIX_ARGS_EQUAL( "foo|bar baz", "foo 'bar baz'" )
    WX_ASSERT_UNIX_ARGS_EQUAL( "foo|bar baz", "foo 'bar baz'" )
    WX_ASSERT_UNIX_ARGS_EQUAL( "O'Henry", "\"O'Henry\"" )
    WX_ASSERT_UNIX_ARGS_EQUAL( "O'Henry", "O\\'Henry" )

    #undef WX_ASSERT_DOS_ARGS_EQUAL
    #undef WX_ASSERT_UNIX_ARGS_EQUAL
    #undef WX_ASSERT_ARGS_EQUAL
}

TEST_CASE("ParseSwitches")
{
    // install a dummy message output object just suppress error messages from
    // wxCmdLineParser::Parse()
    class NoMessageOutput : public wxMessageOutput
    {
    public:
        void Output(const wxString& WXUNUSED(str)) override { }
    } noMessages;

    wxMessageOutput * const old = wxMessageOutput::Set(&noMessages);
    wxON_BLOCK_EXIT1( wxMessageOutput::Set, old );

    wxCmdLineParser p;
    p.AddSwitch("a");
    p.AddSwitch("b");
    p.AddSwitch("c");
    p.AddSwitch("d");
    p.AddSwitch("n", "neg", "Switch that can be negated",
                wxCMD_LINE_SWITCH_NEGATABLE);

    p.SetCmdLine("");
    CHECK_EQ(0, p.Parse(false) );
    CHECK( !p.Found("a") );

    p.SetCmdLine("-z");
    CHECK( p.Parse(false) != 0 );

    p.SetCmdLine("-a");
    CHECK_EQ(0, p.Parse(false) );
    CHECK( p.Found("a") );
    CHECK( !p.Found("b") );

    p.SetCmdLine("-a -d");
    CHECK_EQ(0, p.Parse(false) );
    CHECK( p.Found("a") );
    CHECK( !p.Found("b") );
    CHECK( !p.Found("c") );
    CHECK( p.Found("d") );

    p.SetCmdLine("-abd");
    CHECK_EQ(0, p.Parse(false) );
    CHECK( p.Found("a") );
    CHECK( p.Found("b") );
    CHECK( !p.Found("c") );
    CHECK( p.Found("d") );

    p.SetCmdLine("-abdz");
    CHECK( p.Parse(false) != 0 );

    p.SetCmdLine("-ab -cd");
    CHECK_EQ(0, p.Parse(false) );
    CHECK( p.Found("a") );
    CHECK( p.Found("b") );
    CHECK( p.Found("c") );
    CHECK( p.Found("d") );

    p.SetCmdLine("-da");
    CHECK_EQ(0, p.Parse(false) );
    CHECK( p.Found("a") );
    CHECK( !p.Found("b") );
    CHECK( !p.Found("c") );
    CHECK( p.Found("d") );

    p.SetCmdLine("-n");
    CHECK_EQ(0, p.Parse(false) );
    CHECK_EQ(wxCmdLineSwitchState::NotFound, p.FoundSwitch("a") );
    CHECK_EQ(wxCmdLineSwitchState::On, p.FoundSwitch("n") );

    p.SetCmdLine("-n-");
    CHECK_EQ(0, p.Parse(false) );
    CHECK_EQ(wxCmdLineSwitchState::Off, p.FoundSwitch("neg") );

    p.SetCmdLine("--neg");
    CHECK_EQ(0, p.Parse(false) );
    CHECK_EQ(wxCmdLineSwitchState::On, p.FoundSwitch("neg") );

    p.SetCmdLine("--neg-");
    CHECK_EQ(0, p.Parse(false) );
    CHECK_EQ(wxCmdLineSwitchState::Off, p.FoundSwitch("n") );
}

TEST_CASE("ArgumentsCollection")
{
    wxCmdLineParser p;

    p.AddLongSwitch ("verbose");
    p.AddOption ("l", "long", wxEmptyString, wxCmdLineParamType::Number);
    p.AddOption ("d", "date", wxEmptyString, wxCmdLineParamType::Date);
    p.AddOption ("f", "double", wxEmptyString, wxCmdLineParamType::Double);
    p.AddOption ("s", "string", wxEmptyString, wxCmdLineParamType::String);
    p.AddParam (wxEmptyString, wxCmdLineParamType::String, wxCMD_LINE_PARAM_MULTIPLE);

    wxDateTime wasNow = wxDateTime::Now().GetDateOnly();
    p.SetCmdLine (wxString::Format ("--verbose param1 -l 22 -d \"%s\" -f 50.12e-1 param2 --string \"some string\"",
        wasNow.FormatISODate()));

    CHECK_EQ(0, p.Parse(false) );

    wxCmdLineArgs::const_iterator itargs = p.GetArguments().begin();

    // --verbose
    CHECK_EQ(wxCmdLineEntryType::Switch, itargs->GetKind());
    CHECK_EQ("verbose", itargs->GetLongName());
    CHECK_EQ(false, itargs->IsNegated());

    // param1
    ++itargs; // pre incrementation test
    CHECK_EQ(wxCmdLineEntryType::Param, itargs->GetKind());
    CHECK_EQ("param1", itargs->GetStrVal());

    // -l 22
    itargs++; // post incrementation test
    CHECK_EQ(wxCmdLineEntryType::Option, itargs->GetKind());
    CHECK_EQ(wxCmdLineParamType::Number, itargs->GetType());
    CHECK_EQ("l", itargs->GetShortName());
    CHECK_EQ(22, itargs->GetLongVal());

    // -d (some date)
    ++itargs;
    CHECK_EQ(wxCmdLineEntryType::Option, itargs->GetKind());
    CHECK_EQ(wxCmdLineParamType::Date, itargs->GetType());
    CHECK_EQ("d", itargs->GetShortName());
    CHECK_EQ(wasNow, itargs->GetDateVal());

    // -f 50.12e-1
    ++itargs;
    CHECK_EQ(wxCmdLineEntryType::Option, itargs->GetKind());
    CHECK_EQ(wxCmdLineParamType::Double, itargs->GetType());
    CHECK_EQ("f", itargs->GetShortName());
    CHECK_EQ(50.12e-1, doctest::Approx(itargs->GetDoubleVal()).epsilon(0.000001));

    // param2
    ++itargs;
    CHECK_EQ (wxCmdLineEntryType::Param, itargs->GetKind());
    CHECK_EQ ("param2", itargs->GetStrVal());

    // --string "some string"
    ++itargs;
    CHECK_EQ(wxCmdLineEntryType::Option, itargs->GetKind());
    CHECK_EQ(wxCmdLineParamType::String, itargs->GetType());
    CHECK_EQ("s", itargs->GetShortName());
    CHECK_EQ("string", itargs->GetLongName());
    CHECK_EQ("some string", itargs->GetStrVal());

    // testing pre and post-increment
    --itargs;
    itargs--;
    CHECK_EQ(wxCmdLineParamType::Double, itargs->GetType());

    ++itargs;++itargs;++itargs;
    CHECK(itargs == p.GetArguments().end());
}

TEST_CASE("Usage")
{
    wxGCC_WARNING_SUPPRESS(missing-field-initializers)

    // check that Usage() returns roughly what we expect (don't check all the
    // details, its format can change in the future)
    static constexpr wxCmdLineEntryDesc desc[] =
    {
        { wxCmdLineEntryType::UsageText, nullptr, nullptr, "Verbosity options" },
        { wxCmdLineEntryType::Switch, "v", "verbose", "be verbose" },
        { wxCmdLineEntryType::Switch, "q", "quiet",   "be quiet" },

        { wxCmdLineEntryType::UsageText, nullptr, nullptr, "Output options" },
        { wxCmdLineEntryType::Option, "o", "output",  "output file" },
        { wxCmdLineEntryType::Option, "s", "size",    "output block size", wxCmdLineParamType::Number },
        { wxCmdLineEntryType::Option, "d", "date",    "output file date", wxCmdLineParamType::Date },
        { wxCmdLineEntryType::Option, "f", "double",  "output double", wxCmdLineParamType::Double },

        { wxCmdLineEntryType::Param,  nullptr, nullptr, "input file", },

        { wxCmdLineEntryType::UsageText, nullptr, nullptr, "\nEven more usage text" },
        { wxCmdLineEntryType::None }
    };

    wxGCC_WARNING_RESTORE(missing-field-initializers)

    wxCmdLineParser p(desc);
    const std::vector<wxString> usageLines = wxSplit(p.GetUsageString(), '\n');

    enum
    {
        Line_Synopsis,
        Line_Text_Verbosity,
        Line_Verbose,
        Line_Quiet,
        Line_Text_Output,
        Line_Output_File,
        Line_Output_Size,
        Line_Output_Date,
        Line_Output_Double,
        Line_Text_Dummy1,
        Line_Text_Dummy2,
        Line_Last,
        Line_Max
    };

    CHECK_EQ((size_t)Line_Max, usageLines.size());
    CHECK_EQ("Verbosity options", usageLines[Line_Text_Verbosity]);
    CHECK_EQ("", usageLines[Line_Text_Dummy1]);
    CHECK_EQ("Even more usage text", usageLines[Line_Text_Dummy2]);
    CHECK_EQ("", usageLines[Line_Last]);
}

TEST_CASE("Found")
{
    wxGCC_WARNING_SUPPRESS(missing-field-initializers)

    static constexpr wxCmdLineEntryDesc desc[] =
    {
        { wxCmdLineEntryType::Switch, "v", "verbose", "be verbose" },
        { wxCmdLineEntryType::Option, "o", "output",  "output file" },
        { wxCmdLineEntryType::Option, "s", "size",    "output block size", wxCmdLineParamType::Number },
        { wxCmdLineEntryType::Option, "d", "date",    "output file date", wxCmdLineParamType::Date },
        { wxCmdLineEntryType::Option, "f", "double",  "output double", wxCmdLineParamType::Double },
        { wxCmdLineEntryType::Param,  nullptr, nullptr, "input file", },
        { wxCmdLineEntryType::None }
    };

    wxGCC_WARNING_RESTORE(missing-field-initializers)

    wxCmdLineParser p(desc);
    p.SetCmdLine ("-v --output hello -s 2 --date=2014-02-17 -f 0.2 input-file.txt");

    CHECK(p.Parse() == 0);

    wxString dummys;
    wxDateTime dummydate;
    long dummyl;
    double dummyd;
    // now verify that any option/switch badly queried actually generates an exception
    CHECK_THROWS(p.Found("v", &dummyd));
    CHECK_THROWS(p.Found("v", &dummydate));
    CHECK_THROWS(p.Found("v", &dummyl));
    CHECK_THROWS(p.Found("v", &dummys));
    CHECK(p.FoundSwitch("v") != wxCmdLineSwitchState::NotFound);
    CHECK(p.Found("v"));

    CHECK_THROWS(p.Found("o", &dummyd));
    CHECK_THROWS(p.Found("o", &dummydate));
    CHECK_THROWS(p.Found("o", &dummyl));
    CHECK_THROWS(p.FoundSwitch("o"));
    CHECK(p.Found("o", &dummys));
    CHECK(p.Found("o"));

    CHECK_THROWS(p.Found("s", &dummyd));
    CHECK_THROWS(p.Found("s", &dummydate));
    CHECK_THROWS(p.Found("s", &dummys));
    CHECK_THROWS(p.FoundSwitch("s"));
    CHECK(p.Found("s", &dummyl));
    CHECK(p.Found("s"));

    CHECK_THROWS(p.Found("d", &dummyd));
    CHECK_THROWS(p.Found("d", &dummyl));
    CHECK_THROWS(p.Found("d", &dummys));
    CHECK_THROWS(p.FoundSwitch("d"));
    CHECK(p.Found("d", &dummydate));
    CHECK(p.Found("d"));

    CHECK_THROWS(p.Found("f", &dummydate));
    CHECK_THROWS(p.Found("f", &dummyl));
    CHECK_THROWS(p.Found("f", &dummys));
    CHECK_THROWS(p.FoundSwitch("f"));
    CHECK(p.Found("f", &dummyd));
    CHECK(p.Found("f"));
}
