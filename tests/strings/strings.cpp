///////////////////////////////////////////////////////////////////////////////
// Name:        tests/strings/strings.cpp
// Purpose:     wxString unit test
// Author:      Vadim Zeitlin, Wlodzimierz ABX Skiba
// Created:     2004-04-19
// Copyright:   (c) 2004 Vadim Zeitlin, Wlodzimierz Skiba
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

template<typename T> bool CheckStr(const wxString& expected, T s)
{
    return expected == wxString(s);
}

// flags used in ToLongData.flags
enum
{
    Number_Ok = 0,
    Number_Invalid = 1,
    Number_Unsigned = 2,    // if not specified, works for signed conversion
    Number_Signed = 4,    // if not specified, works for unsigned
    Number_LongLong = 8,    // only for long long tests
    Number_Long = 16    // only for long tests
};

#ifdef wxLongLong_t
typedef wxLongLong_t TestValue_t;
#else
typedef long TestValue_t;
#endif

static constexpr struct ToLongData
{
    const wxChar* str;
    TestValue_t value;
    int flags;
    int base;

    long LValue() const { return value; }
    unsigned long ULValue() const { return value; }
#ifdef wxLongLong_t
    wxLongLong_t LLValue() const { return value; }
    wxULongLong_t ULLValue() const { return (wxULongLong_t)value; }
#endif // wxLongLong_t

    bool IsOk() const { return !(flags & Number_Invalid); }
} longData[] =
{
    { wxT("1"), 1, Number_Ok },
    { wxT("0"), 0, Number_Ok },
    { wxT("a"), 0, Number_Invalid },
    { wxT("12345"), 12345, Number_Ok },
    { wxT("--1"), 0, Number_Invalid },

    { wxT("-1"), -1, Number_Signed | Number_Long },
    // this is surprising but consistent with strtoul() behaviour
    { wxT("-1"), (TestValue_t)ULONG_MAX, Number_Unsigned | Number_Long },

    // this must overflow, even with 64 bit long
    { wxT("922337203685477580711"), 0, Number_Invalid },

#ifdef wxLongLong_t
    { wxT("2147483648"), 2147483648LL, Number_LongLong },
    { wxT("-2147483648"), -2147483648LL, Number_LongLong | Number_Signed },
    { wxT("9223372036854775808"),
      TestValue_t(9223372036854775808ULL),
      Number_LongLong | Number_Unsigned },
#endif // wxLongLong_t

      // Base tests.
      { wxT("010"),  10, Number_Ok, 10 },
      { wxT("010"),   8, Number_Ok,  0 },
      { wxT("010"),   8, Number_Ok,  8 },
      { wxT("010"),  16, Number_Ok, 16 },

      { wxT("0010"), 10, Number_Ok, 10 },
      { wxT("0010"),  8, Number_Ok,  0 },
      { wxT("0010"),  8, Number_Ok,  8 },
      { wxT("0010"), 16, Number_Ok, 16 },

      { wxT("0x11"),  0, Number_Invalid, 10 },
      { wxT("0x11"), 17, Number_Ok,       0 },
      { wxT("0x11"),  0, Number_Invalid,  8 },
      { wxT("0x11"), 17, Number_Ok,      16 },
};

static void DoCStrDataTernaryOperator(bool cond)
{
    // test compilation of wxCStrData when used with operator?: (the asserts
    // are not very important, we're testing if the code compiles at all):

    wxString s("foo");

    const wchar_t* wcStr = L"foo";
    CHECK(CheckStr(s, (cond ? s.c_str() : wcStr)));
    CHECK(CheckStr(s, (cond ? s.c_str() : L"foo")));
    CHECK(CheckStr(s, (cond ? wcStr : s.c_str())));
    CHECK(CheckStr(s, (cond ? L"foo" : s.c_str())));

    const char* mbStr = "foo";
    CHECK(CheckStr(s, (cond ? s.c_str() : mbStr)));
    CHECK(CheckStr(s, (cond ? s.c_str() : "foo")));
    CHECK(CheckStr(s, (cond ? mbStr : s.c_str())));
    CHECK(CheckStr(s, (cond ? "foo" : s.c_str())));

    wxString empty("");
    CHECK(CheckStr(empty, (cond ? empty.c_str() : {})));
    CHECK(CheckStr(empty, (cond ? {} : empty.c_str())));
}

bool CheckStrChar(const wxString& expected, char* s)
{
    return CheckStr(expected, s);
}
bool CheckStrWChar(const wxString& expected, wchar_t* s)
{
    return CheckStr(expected, s);
}
bool CheckStrConstChar(const wxString& expected, const char* s)
{
    return CheckStr(expected, s);
}
bool CheckStrConstWChar(const wxString& expected, const wchar_t* s)
{
    return CheckStr(expected, s);
}

TEST_CASE("String")
{
    wxString a, b, c;

    a.reserve (128);
    b.reserve (128);
    c.reserve (128);

    for (int i = 0; i < 2; ++i)
    {
        a = wxT("Hello");
        b = wxT(" world");
        c = wxT("! How'ya doin'?");
        a += b;
        a += c;
        c = wxT("Hello world! What's up?");
        CHECK( c != a );
    }
}

TEST_CASE("PChar")
{
    wxChar a [128];
    wxChar b [128];
    wxChar c [128];

    for (int i = 0; i < 2; ++i)
    {
        wxStrcpy (a, wxT("Hello"));
        wxStrcpy (b, wxT(" world"));
        wxStrcpy (c, wxT("! How'ya doin'?"));
        wxStrcat (a, b);
        wxStrcat (a, c);
        wxStrcpy (c, wxT("Hello world! What's up?"));
        CHECK( wxStrcmp (c, a) != 0 );
    }
}

TEST_CASE("Format")
{
    wxString s1,s2;
    s1.Printf(wxT("%03d"), 18);
    CHECK( s1 == wxString::Format(wxT("%03d"), 18) );
    s2.Printf(wxT("Number 18: %s\n"), s1.c_str());
    CHECK( s2 == wxString::Format(wxT("Number 18: %s\n"), s1.c_str()) );

    static constexpr size_t lengths[] = { 1, 512, 1024, 1025, 2048, 4096, 4097 };
    for ( size_t n = 0; n < WXSIZEOF(lengths); n++ )
    {
        const size_t len = lengths[n];

        wxString s(wxT('Z'), len);
        CHECK_EQ( len, wxString::Format(wxT("%s"), s.c_str()).length());
    }


    // Positional parameters tests:
    CHECK_EQ
    (
        "two one",
        wxString::Format(wxT("%2$s %1$s"), wxT("one"), wxT("two"))
    );

    CHECK_EQ
    (
        "hello hello",
        wxString::Format("%1$s %1$s", "hello")
    );

    CHECK_EQ
    (
        "4 world hello world 3",
        wxString::Format("%4$d %2$s %1$s %2$s %3$d", "hello", "world", 3, 4)
    );

    CHECK( wxString::Format("%1$o %1$d %1$x", 20) == "24 20 14" );
}

TEST_CASE("FormatUnicode")
{
    const char *UNICODE_STR = "Iestat\xC4\xAB %i%i";
    //const char *UNICODE_STR = "Iestat\xCC\x84 %i%i";

    wxString fmt = wxString::FromUTF8(UNICODE_STR);
    wxString s = wxString::Format(fmt, 1, 1);
    wxString expected(fmt);
    expected.Replace("%i", "1");
    CHECK_EQ( expected, s );
}

TEST_CASE("Constructors")
{
    CHECK_EQ( "", wxString('Z', 0) );
    CHECK_EQ( "Z", wxString('Z') );
    CHECK_EQ( "ZZZZ", wxString('Z', 4) );
    CHECK_EQ( "Hell", wxString("Hello", 4) );
    CHECK_EQ( "Hello", wxString("Hello", 5) );

    CHECK_EQ( L"", wxString(L'Z', 0) );
    CHECK_EQ( L"Z", wxString(L'Z') );
    CHECK_EQ( L"ZZZZ", wxString(L'Z', 4) );
    CHECK_EQ( L"Hell", wxString(L"Hello", 4) );
    CHECK_EQ( L"Hello", wxString(L"Hello", 5) );

    CHECK_EQ( 0, wxString(wxString(), 17).length() );

#if wxUSE_UNICODE_UTF8
    // This string has 3 characters (<h>, <e'> and <l>), not 4 when using UTF-8
    // locale!
    if ( wxConvLibc.IsUTF8() )
    {
        wxString s3("h\xc3\xa9llo", 4);
        CHECK_EQ( 3, s3.length() );
        CHECK_EQ( 'l', (char)s3[2] );
    }
#endif // wxUSE_UNICODE_UTF8


    static const char *s = "?really!";
    const char *start = wxStrchr(s, 'r');
    const char *end = wxStrchr(s, '!');
    CHECK_EQ( "really", wxString(start, end) );

    // test if creating string from NULL C pointer works:
    CHECK_EQ( "", wxString((const char *)NULL) );
}

TEST_CASE("StaticConstructors")
{
    CHECK_EQ( "", wxString::FromAscii("") );
    CHECK_EQ( "", wxString::FromAscii("Hello", 0) );
    CHECK_EQ( "Hell", wxString::FromAscii("Hello", 4) );
    CHECK_EQ( "Hello", wxString::FromAscii("Hello", 5) );
    CHECK_EQ( "Hello", wxString::FromAscii("Hello") );

    // FIXME: this doesn't work currently but should!
    //CHECK_EQ( 1, wxString::FromAscii("", 1).length() );


    CHECK_EQ( "", wxString::FromUTF8("") );
    CHECK_EQ( "", wxString::FromUTF8("Hello", 0) );
    CHECK_EQ( "Hell", wxString::FromUTF8("Hello", 4) );
    CHECK_EQ( "Hello", wxString::FromUTF8("Hello", 5) );
    CHECK_EQ( "Hello", wxString::FromUTF8("Hello") );

    CHECK_EQ( 2, wxString::FromUTF8("h\xc3\xa9llo", 3).length() );

    //CHECK_EQ( 1, wxString::FromUTF8("", 1).length() );
}

TEST_CASE("Extraction")
{
    wxString s(wxT("Hello, world!"));

    CHECK( wxStrcmp( s.c_str() , wxT("Hello, world!") ) == 0 );
    CHECK( wxStrcmp( s.Left(5).c_str() , wxT("Hello") ) == 0 );
    CHECK( wxStrcmp( s.Right(6).c_str() , wxT("world!") ) == 0 );
    CHECK( wxStrcmp( s(3, 5).c_str() , wxT("lo, w") ) == 0 );
    CHECK( wxStrcmp( s.Mid(3).c_str() , wxT("lo, world!") ) == 0 );
    CHECK( wxStrcmp( s.substr(3, 5).c_str() , wxT("lo, w") ) == 0 );
    CHECK( wxStrcmp( s.substr(3).c_str() , wxT("lo, world!") ) == 0 );

    static const char *germanUTF8 = "Oberfl\303\244che";
    wxString strUnicode(wxString::FromUTF8(germanUTF8));

    CHECK( strUnicode.Mid(0, 10) == strUnicode );
    CHECK( strUnicode.Mid(7, 2) == "ch" );

    wxString rest;

    #define TEST_STARTS_WITH(prefix, correct_rest, result)                    \
        CHECK_EQ(result, s.StartsWith(prefix, &rest));            \
        if ( result )                                                         \
            CHECK_EQ(correct_rest, rest)

    TEST_STARTS_WITH( wxT("Hello"),           wxT(", world!"),      true  );
    TEST_STARTS_WITH( wxT("Hello, "),         wxT("world!"),        true  );
    TEST_STARTS_WITH( wxT("Hello, world!"),   wxT(""),              true  );
    TEST_STARTS_WITH( wxT("Hello, world!!!"), wxT(""),              false );
    TEST_STARTS_WITH( wxT(""),                wxT("Hello, world!"), true  );
    TEST_STARTS_WITH( wxT("Goodbye"),         wxT(""),              false );
    TEST_STARTS_WITH( wxT("Hi"),              wxT(""),              false );

    #undef TEST_STARTS_WITH

    rest = "Hello world";
    CHECK( rest.StartsWith("Hello ", &rest) );
    CHECK_EQ("world", rest);

    #define TEST_ENDS_WITH(suffix, correct_rest, result)                      \
        CHECK_EQ(result, s.EndsWith(suffix, &rest));              \
        if ( result )                                                         \
            CHECK_EQ(correct_rest, rest)

    TEST_ENDS_WITH( wxT(""),                 wxT("Hello, world!"), true  );
    TEST_ENDS_WITH( wxT("!"),                wxT("Hello, world"),  true  );
    TEST_ENDS_WITH( wxT(", world!"),         wxT("Hello"),         true  );
    TEST_ENDS_WITH( wxT("ello, world!"),     wxT("H"),             true  );
    TEST_ENDS_WITH( wxT("Hello, world!"),    wxT(""),              true  );
    TEST_ENDS_WITH( wxT("very long string"), wxT(""),              false );
    TEST_ENDS_WITH( wxT("?"),                wxT(""),              false );
    TEST_ENDS_WITH( wxT("Hello, world"),     wxT(""),              false );
    TEST_ENDS_WITH( wxT("Gello, world!"),    wxT(""),              false );

    #undef TEST_ENDS_WITH
}

TEST_CASE("Trim")
{
    #define TEST_TRIM( str , dir , result )  \
        CHECK( wxString(str).Trim(dir) == result )

    TEST_TRIM( wxT("  Test  "),  true, wxT("  Test") );
    TEST_TRIM( wxT("    "),      true, wxT("")       );
    TEST_TRIM( wxT(" "),         true, wxT("")       );
    TEST_TRIM( wxT(""),          true, wxT("")       );

    TEST_TRIM( wxT("  Test  "),  false, wxT("Test  ") );
    TEST_TRIM( wxT("    "),      false, wxT("")       );
    TEST_TRIM( wxT(" "),         false, wxT("")       );
    TEST_TRIM( wxT(""),          false, wxT("")       );

    #undef TEST_TRIM
}

TEST_CASE("Find")
{
    #define TEST_FIND( str , start , result )  \
        CHECK( wxString(str).find(wxT("ell"), start) == result );

    TEST_FIND( wxT("Well, hello world"),  0, 1              );
    TEST_FIND( wxT("Well, hello world"),  6, 7              );
    TEST_FIND( wxT("Well, hello world"),  9, wxString::npos );

    #undef TEST_FIND
}

TEST_CASE("Replace")
{
    #define TEST_REPLACE( original , pos , len , replacement , result ) \
        { \
            wxString s = original; \
            s.replace( pos , len , replacement ); \
            CHECK_EQ( result, s ); \
        }

    TEST_REPLACE( wxT("012-AWORD-XYZ"), 4, 5, wxT("BWORD"),  wxT("012-BWORD-XYZ") );
    TEST_REPLACE( wxT("increase"),      0, 2, wxT("de"),     wxT("decrease")      );
    TEST_REPLACE( wxT("wxWindow"),      8, 0, wxT("s"),      wxT("wxWindows")     );
    TEST_REPLACE( wxT("foobar"),        3, 0, wxT("-"),      wxT("foo-bar")       );
    TEST_REPLACE( wxT("barfoo"),        0, 6, wxT("foobar"), wxT("foobar")        );


    #define TEST_NULLCHARREPLACE( o , olen, pos , len , replacement , r, rlen ) \
        { \
            wxString s(o,olen); \
            s.replace( pos , len , replacement ); \
            CHECK_EQ( wxString(r,rlen), s ); \
        }

    TEST_NULLCHARREPLACE( wxT("null\0char"), 9, 5, 1, wxT("d"),
                          wxT("null\0dhar"), 9 );

    #define TEST_WXREPLACE( o , olen, olds, news, all, r, rlen ) \
        { \
            wxString s(o,olen); \
            s.Replace( olds, news, all ); \
            CHECK_EQ( wxString(r,rlen), s ); \
        }

    TEST_WXREPLACE( wxT("null\0char"), 9, wxT("c"), wxT("de"), true,
                          wxT("null\0dehar"), 10 );

    TEST_WXREPLACE( wxT("null\0dehar"), 10, wxT("de"), wxT("c"), true,
                          wxT("null\0char"), 9 );

    TEST_WXREPLACE( "life", 4, "f", "", false, "lie", 3 );
    TEST_WXREPLACE( "life", 4, "f", "", true, "lie", 3 );
    TEST_WXREPLACE( "life", 4, "fe", "ve", true, "live", 4 );
    TEST_WXREPLACE( "xx", 2, "x", "yy", true, "yyyy", 4 );
    TEST_WXREPLACE( "xxx", 3, "xx", "z", true, "zx", 2 );

    #undef TEST_WXREPLACE
    #undef TEST_NULLCHARREPLACE
    #undef TEST_REPLACE
}

TEST_CASE("Match")
{
    #define TEST_MATCH( s1 , s2 , result ) \
        CHECK( wxString(s1).Matches(s2) == result )

    TEST_MATCH( "foobar",       "foo*",        true  );
    TEST_MATCH( "foobar",       "*oo*",        true  );
    TEST_MATCH( "foobar",       "*bar",        true  );
    TEST_MATCH( "foobar",       "??????",      true  );
    TEST_MATCH( "foobar",       "f??b*",       true  );
    TEST_MATCH( "foobar",       "f?b*",        false );
    TEST_MATCH( "foobar",       "*goo*",       false );
    TEST_MATCH( "foobar",       "*foo",        false );
    TEST_MATCH( "foobarfoo",    "*foo",        true  );
    TEST_MATCH( "",             "*",           true  );
    TEST_MATCH( "",             "?",           false );

    #undef TEST_MATCH
}

TEST_CASE("CaseChanges")
{
    wxString s1(wxT("Hello!"));
    wxString s1u(s1);
    wxString s1l(s1);
    s1u.MakeUpper();
    s1l.MakeLower();

    CHECK_EQ( wxT("HELLO!"), s1u );
    CHECK_EQ( wxT("hello!"), s1l );

    wxString s2u, s2l;
    s2u.MakeUpper();
    s2l.MakeLower();

    CHECK_EQ( "", s2u );
    CHECK_EQ( "", s2l );


    wxString s3("good bye");
    CHECK_EQ( "Good bye", s3.Capitalize() );
    s3.MakeCapitalized();
    CHECK_EQ( "Good bye", s3 );

    CHECK_EQ( "Abc", wxString("ABC").Capitalize() );

    CHECK_EQ( "", wxString().Capitalize() );
}

TEST_CASE("Compare")
{
    wxString s1 = wxT("AHH");
    wxString eq = wxT("AHH");
    wxString neq1 = wxT("HAH");
    wxString neq2 = wxT("AH");
    wxString neq3 = wxT("AHHH");
    wxString neq4 = wxT("AhH");

    CHECK( s1 == eq );
    CHECK( s1 != neq1 );
    CHECK( s1 != neq2 );
    CHECK( s1 != neq3 );
    CHECK( s1 != neq4 );

    CHECK( s1 == wxT("AHH") );
    CHECK( s1 != wxT("no") );
    CHECK( s1 < wxT("AZ") );
    CHECK( s1 <= wxT("AZ") );
    CHECK( s1 <= wxT("AHH") );
    CHECK( s1 > wxT("AA") );
    CHECK( s1 >= wxT("AA") );
    CHECK( s1 >= wxT("AHH") );

    // test comparison with C strings in Unicode build (must work in ANSI as
    // well, of course):
    CHECK( s1 == "AHH" );
    CHECK( s1 != "no" );
    CHECK( s1 < "AZ" );
    CHECK( s1 <= "AZ" );
    CHECK( s1 <= "AHH" );
    CHECK( s1 > "AA" );
    CHECK( s1 >= "AA" );
    CHECK( s1 >= "AHH" );

//    wxString _s1 = wxT("A\0HH");
//    wxString _eq = wxT("A\0HH");
//    wxString _neq1 = wxT("H\0AH");
//    wxString _neq2 = wxT("A\0H");
//    wxString _neq3 = wxT("A\0HHH");
//    wxString _neq4 = wxT("A\0hH");
    s1.insert(1,1,'\0');
    eq.insert(1,1,'\0');
    neq1.insert(1,1,'\0');
    neq2.insert(1,1,'\0');
    neq3.insert(1,1,'\0');
    neq4.insert(1,1,'\0');

    CHECK( s1 == eq );
    CHECK( s1 != neq1 );
    CHECK( s1 != neq2 );
    CHECK( s1 != neq3 );
    CHECK( s1 != neq4 );

    CHECK( wxString("\n").Cmp(" ") < 0 );
    CHECK( wxString("'").Cmp("!") > 0 );
    CHECK( wxString("!").Cmp("z") < 0 );
}

TEST_CASE("CompareNoCase")
{
    wxString s1 = wxT("AHH");
    wxString eq = wxT("AHH");
    wxString eq2 = wxT("AhH");
    wxString eq3 = wxT("ahh");
    wxString neq = wxT("HAH");
    wxString neq2 = wxT("AH");
    wxString neq3 = wxT("AHHH");

    #define CNCEQ_ASSERT(s1, s2) CHECK( s1.CmpNoCase(s2) == 0)
    #define CNCNEQ_ASSERT(s1, s2) CHECK( s1.CmpNoCase(s2) != 0)

    CNCEQ_ASSERT( s1, eq );
    CNCEQ_ASSERT( s1, eq2 );
    CNCEQ_ASSERT( s1, eq3 );

    CNCNEQ_ASSERT( s1, neq );
    CNCNEQ_ASSERT( s1, neq2 );
    CNCNEQ_ASSERT( s1, neq3 );


//    wxString _s1 = wxT("A\0HH");
//    wxString _eq = wxT("A\0HH");
//    wxString _eq2 = wxT("A\0hH");
//    wxString _eq3 = wxT("a\0hh");
//    wxString _neq = wxT("H\0AH");
//    wxString _neq2 = wxT("A\0H");
//    wxString _neq3 = wxT("A\0HHH");

    s1.insert(1,1,'\0');
    eq.insert(1,1,'\0');
    eq2.insert(1,1,'\0');
    eq3.insert(1,1,'\0');
    neq.insert(1,1,'\0');
    neq2.insert(1,1,'\0');
    neq3.insert(1,1,'\0');

    CNCEQ_ASSERT( s1, eq );
    CNCEQ_ASSERT( s1, eq2 );
    CNCEQ_ASSERT( s1, eq3 );

    CNCNEQ_ASSERT( s1, neq );
    CNCNEQ_ASSERT( s1, neq2 );
    CNCNEQ_ASSERT( s1, neq3 );

    CHECK( wxString("\n").CmpNoCase(" ") < 0 );
    CHECK( wxString("'").CmpNoCase("!") > 0);
    CHECK( wxString("!").Cmp("Z") < 0 );
}

TEST_CASE("Contains")
{
    static constexpr struct ContainsData
    {
        const wxChar *hay;
        const wxChar *needle;
        bool contains;
    } containsData[] =
    {
        { wxT(""),       wxT(""),         true  },
        { wxT(""),       wxT("foo"),      false },
        { wxT("foo"),    wxT(""),         true  },
        { wxT("foo"),    wxT("f"),        true  },
        { wxT("foo"),    wxT("o"),        true  },
        { wxT("foo"),    wxT("oo"),       true  },
        { wxT("foo"),    wxT("ooo"),      false },
        { wxT("foo"),    wxT("oooo"),     false },
        { wxT("foo"),    wxT("fooo"),     false },
    };

    for ( size_t n = 0; n < WXSIZEOF(containsData); n++ )
    {
        const ContainsData& cd = containsData[n];
        CHECK_EQ( cd.contains, wxString(cd.hay).Contains(cd.needle) );
    }
}

TEST_CASE("ToLong")
{
    long l;
    for ( size_t n = 0; n < WXSIZEOF(longData); n++ )
    {
        const ToLongData& ld = longData[n];

        if ( ld.flags & (Number_LongLong | Number_Unsigned) )
            continue;

        // NOTE: unless you're using some exotic locale, ToCLong and ToLong
        //       should behave the same for our test data set:

        CHECK_EQ( ld.IsOk(),
                              wxString(ld.str).ToCLong(&l, ld.base) );
        if ( ld.IsOk() )
            CHECK_EQ( ld.LValue(), l );

        CHECK_EQ( ld.IsOk(),
                              wxString(ld.str).ToLong(&l, ld.base) );
        if ( ld.IsOk() )
            CHECK_EQ( ld.LValue(), l );
    }

    // special case: check that the output is not modified if the parsing
    // failed completely
    l = 17;
    CHECK( !wxString("foo").ToLong(&l) );
    CHECK_EQ( 17, l );

    // also check that it is modified if we did parse something successfully in
    // the beginning of the string
    CHECK( !wxString("9 cats").ToLong(&l) );
    CHECK_EQ( 9, l );
}

TEST_CASE("ToULong")
{
    unsigned long ul;
    for ( size_t n = 0; n < WXSIZEOF(longData); n++ )
    {
        const ToLongData& ld = longData[n];

        if ( ld.flags & (Number_LongLong | Number_Signed) )
            continue;

        // NOTE: unless you're using some exotic locale, ToCLong and ToLong
        //       should behave the same for our test data set:

        CHECK_EQ( ld.IsOk(),
                              wxString(ld.str).ToCULong(&ul, ld.base) );
        if ( ld.IsOk() )
            CHECK_EQ( ld.ULValue(), ul );

        CHECK_EQ( ld.IsOk(),
                              wxString(ld.str).ToULong(&ul, ld.base) );
        if ( ld.IsOk() )
            CHECK_EQ( ld.ULValue(), ul );
    }
}

#ifdef wxLongLong_t

TEST_CASE("ToLongLong")
{
    wxLongLong_t l;
    for ( size_t n = 0; n < WXSIZEOF(longData); n++ )
    {
        const ToLongData& ld = longData[n];

        if ( ld.flags & (Number_Long | Number_Unsigned) )
            continue;

        CHECK_EQ( ld.IsOk(),
                              wxString(ld.str).ToLongLong(&l, ld.base) );
        if ( ld.IsOk() )
            CHECK_EQ( ld.LLValue(), l );
    }
}

TEST_CASE("ToULongLong")
{
    wxULongLong_t ul;
    for ( size_t n = 0; n < WXSIZEOF(longData); n++ )
    {
        const ToLongData& ld = longData[n];

        if ( ld.flags & (Number_Long | Number_Signed) )
            continue;

        CHECK_EQ( ld.IsOk(),
                              wxString(ld.str).ToULongLong(&ul, ld.base) );
        if ( ld.IsOk() )
            CHECK_EQ( ld.ULLValue(), ul );
    }
}

#endif // wxLongLong_t

TEST_CASE("ToDouble")
{
    double d;
    static constexpr struct ToDoubleData
    {
        const wxChar *str;
        double value;
        bool ok;
    } doubleData[] =
    {
        { wxT("1"), 1, true },
        { wxT("1.23"), 1.23, true },
        { wxT(".1"), .1, true },
        { wxT("1."), 1, true },
        { wxT("1.."), 0, false },
        { wxT("0"), 0, true },
        { wxT("a"), 0, false },
        { wxT("12345"), 12345, true },
        { wxT("-1"), -1, true },
        { wxT("--1"), 0, false },
        { wxT("-3E-5"), -3E-5, true },
        { wxT("-3E-abcde5"), 0, false },
    };

    // test ToCDouble() first:

    size_t n;
    for ( n = 0; n < WXSIZEOF(doubleData); n++ )
    {
        const ToDoubleData& ld = doubleData[n];
        CHECK_EQ( ld.ok, wxString(ld.str).ToCDouble(&d) );
        if ( ld.ok )
            CHECK_EQ( ld.value, d );
    }


    // test ToDouble() now:
    // NOTE: for the test to be reliable, we need to set the locale explicitly
    //       so that we know the decimal point character to use

    if (!wxLocale::IsAvailable(wxLANGUAGE_FRENCH))
        return;     // you should have french support installed to continue this test!

    wxLocale locale;

    // don't load default catalog, it may be unavailable:
    CHECK( locale.Init(wxLANGUAGE_FRENCH, wxLOCALE_DONT_LOAD_DEFAULT) );

    static constexpr struct ToDoubleData doubleData2[] =
    {
        { wxT("1"), 1, true },
        { wxT("1,23"), 1.23, true },
        { wxT(",1"), .1, true },
        { wxT("1,"), 1, true },
        { wxT("1,,"), 0, false },
        { wxT("0"), 0, true },
        { wxT("a"), 0, false },
        { wxT("12345"), 12345, true },
        { wxT("-1"), -1, true },
        { wxT("--1"), 0, false },
        { wxT("-3E-5"), -3E-5, true },
        { wxT("-3E-abcde5"), 0, false },
    };

    for ( n = 0; n < WXSIZEOF(doubleData2); n++ )
    {
        const ToDoubleData& ld = doubleData2[n];
        CHECK_EQ( ld.ok, wxString(ld.str).ToDouble(&d) );
        if ( ld.ok )
            CHECK_EQ( ld.value, d );
    }
}

TEST_CASE("FromDouble")
{
    static constexpr struct FromDoubleTestData
    {
        double value;
        int prec;
        const char *str;
    } testData[] =
    {
        { 1.23,             -1, "1.23" },
        { -3e-10,           -1, "-3e-10" },
        { -0.45678,         -1, "-0.45678" },
        { 1.2345678,         0, "1" },
        { 1.2345678,         1, "1.2" },
        { 1.2345678,         2, "1.23" },
        { 1.2345678,         3, "1.235" },
    };

    for ( unsigned n = 0; n < WXSIZEOF(testData); n++ )
    {
        const FromDoubleTestData& td = testData[n];
        CHECK_EQ( td.str, wxString::FromCDouble(td.value, td.prec) );
    }

    if ( !wxLocale::IsAvailable(wxLANGUAGE_FRENCH) )
        return;

    wxLocale locale;
    CHECK( locale.Init(wxLANGUAGE_FRENCH, wxLOCALE_DONT_LOAD_DEFAULT) );

    for ( unsigned m = 0; m < WXSIZEOF(testData); m++ )
    {
        const FromDoubleTestData& td = testData[m];

        wxString str(td.str);
        str.Replace(".", ",");
        CHECK_EQ( str, wxString::FromDouble(td.value, td.prec) );
    }
}

TEST_CASE("StringBuf")
{
    // check that buffer can be used to write into the string
    wxString s;
    wxStrcpy(wxStringBuffer(s, 10), wxT("foo"));

    CHECK_EQ(3, s.length());
    CHECK(wxT('f') == s[0u]);
    CHECK(wxT('o') == s[1]);
    CHECK(wxT('o') == s[2]);

    {
        // also check that the buffer initially contains the original string
        // contents
        wxStringBuffer buf(s, 10);
        CHECK_EQ( wxT('f'), buf[0] );
        CHECK_EQ( wxT('o'), buf[1] );
        CHECK_EQ( wxT('o'), buf[2] );
        CHECK_EQ( wxT('\0'), buf[3] );
    }

    {
        wxStringBufferLength buf(s, 10);
        CHECK_EQ( wxT('f'), buf[0] );
        CHECK_EQ( wxT('o'), buf[1] );
        CHECK_EQ( wxT('o'), buf[2] );
        CHECK_EQ( wxT('\0'), buf[3] );

        // and check that it can be used to write only the specified number of
        // characters to the string
        wxStrcpy(buf, wxT("barrbaz"));
        buf.SetLength(4);
    }

    CHECK_EQ(4, s.length());
    CHECK(wxT('b') == s[0u]);
    CHECK(wxT('a') == s[1]);
    CHECK(wxT('r') == s[2]);
    CHECK(wxT('r') == s[3]);

    // check that creating buffer of length smaller than string works, i.e. at
    // least doesn't crash (it would if we naively copied the entire original
    // string contents in the buffer)
    *wxStringBuffer(s, 1) = '!';
}

TEST_CASE("UTF8Buf")
{
    // "czech" in Czech ("cestina"):
    static constexpr char textUTF8[] = "\304\215e\305\241tina";
    static constexpr wchar_t textUTF16[] = {0x10D, 0x65, 0x161, 0x74, 0x69, 0x6E, 0x61, 0};

    wxString s;
    wxStrcpy(wxUTF8StringBuffer(s, 9), textUTF8);
    CHECK(s == textUTF16);

    {
        wxUTF8StringBufferLength buf(s, 20);
        wxStrcpy(buf, textUTF8);
        buf.SetLength(5);
    }
    CHECK(s == wxString(textUTF16, 0, 3));
}

TEST_CASE("CStrDataTernaryOperator")
{
    DoCStrDataTernaryOperator(true);
    DoCStrDataTernaryOperator(false);
}

TEST_CASE("CStrDataOperators")
{
    wxString s("hello");

    CHECK( s.c_str()[0] == 'h' );
    CHECK( s.c_str()[1] == 'e' );

    // IMPORTANT: at least with the CRT coming with MSVC++ 2008 trying to access
    //            the final character results in an assert failure (with debug CRT)
    //CHECK( s.c_str()[5] == '\0' );

    CHECK( *s.c_str() == 'h' );
    CHECK( *(s.c_str() + 2) == 'l' );
    //CHECK( *(s.c_str() + 5) == '\0' );
}

TEST_CASE("CStrDataImplicitConversion")
{
    wxString s("foo");

    CHECK( CheckStrConstWChar(s, s.c_str()) );
    CHECK( CheckStrConstChar(s, s.c_str()) );
}

TEST_CASE("ExplicitConversion")
{
    wxString s("foo");

    CHECK( CheckStr(s, s.mb_str()) );
    CHECK( CheckStrConstChar(s, s.mb_str()) );
    CHECK( CheckStrChar(s, s.char_str()) );

    CHECK( CheckStr(s, s.wc_str()) );
    CHECK( CheckStrConstWChar(s, s.wc_str()) );
    CHECK( CheckStrWChar(s, s.wchar_str()) );
}

TEST_CASE("IndexedAccess")
{
    wxString s("bar");
    CHECK_EQ( 'r', (char)s[2] );

    // this tests for a possible bug in UTF-8 based wxString implementation:
    // the 3rd character of the underlying byte string is going to change, but
    // the 3rd character of wxString should remain the same
    s[0] = L'\xe9';
    CHECK_EQ( 'r', (char)s[2] );
}

TEST_CASE("BeforeAndAfter")
{
    // Construct a string with 2 equal signs in it by concatenating its three
    // parts: before the first "=", in between the two "="s and after the last
    // one. This allows to avoid duplicating the string contents (which has to
    // be different for Unicode and ANSI builds) in the tests below.
    #define FIRST_PART L"letter"
    #define MIDDLE_PART L"\xe9;\xe7a"
    #define LAST_PART L"l\xe0"

    const wxString s(FIRST_PART wxT("=") MIDDLE_PART wxT("=") LAST_PART);

    wxString r;

    CHECK_EQ( FIRST_PART, s.BeforeFirst('=', &r) );
    CHECK_EQ( MIDDLE_PART wxT("=") LAST_PART, r );

    CHECK_EQ( s, s.BeforeFirst('!', &r) );
    CHECK_EQ( "", r );


    CHECK_EQ( FIRST_PART wxT("=") MIDDLE_PART, s.BeforeLast('=', &r) );
    CHECK_EQ( LAST_PART, r );

    CHECK_EQ( "", s.BeforeLast('!', &r) );
    CHECK_EQ( s, r );


    CHECK_EQ( MIDDLE_PART wxT("=") LAST_PART, s.AfterFirst('=') );
    CHECK_EQ( "", s.AfterFirst('!') );


    CHECK_EQ( LAST_PART, s.AfterLast('=') );
    CHECK_EQ( s, s.AfterLast('!') );

    #undef LAST_PART
    #undef MIDDLE_PART
    #undef FIRST_PART
}

TEST_CASE("ScopedBuffers")
{
    // wxString relies on efficient buffers, verify they work as they should

    const char *literal = "Hello World!";

    // non-owned buffer points to the string passed to it
    wxScopedCharBuffer sbuf = wxScopedCharBuffer::CreateNonOwned(literal);
    CHECK( sbuf.data() == literal );

    // a copy of scoped non-owned buffer still points to the same string
    wxScopedCharBuffer sbuf2(sbuf);
    CHECK( sbuf.data() == sbuf2.data() );

    // but assigning it to wxCharBuffer makes a full copy
    wxCharBuffer buf(sbuf);
    CHECK( buf.data() != literal );
    CHECK_EQ( std::string(literal), buf.data() );

    wxCharBuffer buf2 = sbuf;
    CHECK( buf2.data() != literal );
    CHECK_EQ( std::string(literal), buf.data() );

    // Check that extending the buffer keeps it NUL-terminated.
    size_t len = 10;

    wxCharBuffer buf3(len);
    CHECK_EQ('\0', buf3.data()[len]);

    wxCharBuffer buf4;
    buf4.extend(len);
    CHECK_EQ('\0', buf4.data()[len]);

    wxCharBuffer buf5(5);
    buf5.extend(len);
    CHECK_EQ('\0', buf5.data()[len]);
}

TEST_CASE("SupplementaryUniChar")
{
    // Test wxString(wxUniChar ch, size_t nRepeat = 1),
    // which is implemented upon assign(size_t n, wxUniChar ch).
    {
        wxString s(wxUniChar(0x12345));
#if wxUSE_UNICODE_UTF16
        CHECK_EQ(2, s.length());
        CHECK_EQ(0xD808, s[0].GetValue());
        CHECK_EQ(0xDF45, s[1].GetValue());
#else
        CHECK_EQ(1, s.length());
        CHECK_EQ(0x12345, s[0].GetValue());
#endif
    }

    // Test operator=(wxUniChar ch).
    {
        wxString s;
        s = wxUniChar(0x23456);
#if wxUSE_UNICODE_UTF16
        CHECK_EQ(2, s.length());
        CHECK_EQ(0xD84D, s[0].GetValue());
        CHECK_EQ(0xDC56, s[1].GetValue());
#else
        CHECK_EQ(1, s.length());
        CHECK_EQ(0x23456, s[0].GetValue());
#endif
    }

    // Test operator+=(wxUniChar ch).
    {
        wxString s = "A";
        s += wxUniChar(0x34567);
#if wxUSE_UNICODE_UTF16
        CHECK_EQ(3, s.length());
        CHECK_EQ(0xD891, s[1].GetValue());
        CHECK_EQ(0xDD67, s[2].GetValue());
#else
        CHECK_EQ(2, s.length());
        CHECK_EQ(0x34567, s[1].GetValue());
#endif
    }

    // Test operator<<(wxUniChar ch),
    // which is implemented upon append(size_t n, wxUniChar ch).
    {
        wxString s = "A";
        s << wxUniChar(0x45678);
#if wxUSE_UNICODE_UTF16
        CHECK_EQ(3, s.length());
        CHECK_EQ(0xD8D5, s[1].GetValue());
        CHECK_EQ(0xDE78, s[2].GetValue());
#else
        CHECK_EQ(2, s.length());
        CHECK_EQ(0x45678, s[1].GetValue());
#endif
    }

    // Test insert(size_t nPos, size_t n, wxUniChar ch).
    {
        wxString s = L"\x3042\x208\x3059";
        s.insert(1, 2, wxUniChar(0x12345));
#if wxUSE_UNICODE_UTF16
        CHECK_EQ(7, s.length());
        CHECK_EQ(0xD808, s[1].GetValue());
        CHECK_EQ(0xDF45, s[2].GetValue());
        CHECK_EQ(0xD808, s[3].GetValue());
        CHECK_EQ(0xDF45, s[4].GetValue());
#else
        CHECK_EQ(5, s.length());
        CHECK_EQ(0x12345, s[1].GetValue());
        CHECK_EQ(0x12345, s[2].GetValue());
#endif
    }

    // Test insert(iterator it, wxUniChar ch).
    {
        wxString s = L"\x3042\x208\x3059";
        s.insert(s.begin() + 1, wxUniChar(0x23456));
#if wxUSE_UNICODE_UTF16
        CHECK_EQ(5, s.length());
        CHECK_EQ(0xD84D, s[1].GetValue());
        CHECK_EQ(0xDC56, s[2].GetValue());
#else
        CHECK_EQ(4, s.length());
        CHECK_EQ(0x23456, s[1].GetValue());
#endif
    }

    // Test insert(iterator it, size_type n, wxUniChar ch).
    {
        wxString s = L"\x3042\x208\x3059";
        s.insert(s.begin() + 1, 2, wxUniChar(0x34567));
#if wxUSE_UNICODE_UTF16
        CHECK_EQ(7, s.length());
        CHECK_EQ(0xD891, s[1].GetValue());
        CHECK_EQ(0xDD67, s[2].GetValue());
#else
        CHECK_EQ(5, s.length());
        CHECK_EQ(0x34567, s[1].GetValue());
#endif
    }

    // Test replace(size_t nStart, size_t nLen, size_t nCount, wxUniChar ch).
    {
        wxString s = L"\x3042\x208\x3059";
        s.replace(1, 2, 2, wxUniChar(0x45678));
#if wxUSE_UNICODE_UTF16
        CHECK_EQ(5, s.length());
        CHECK_EQ(0xD8D5, s[1].GetValue());
        CHECK_EQ(0xDE78, s[2].GetValue());
        CHECK_EQ(0xD8D5, s[3].GetValue());
        CHECK_EQ(0xDE78, s[4].GetValue());
#else
        CHECK_EQ(3, s.length());
        CHECK_EQ(0x45678, s[1].GetValue());
        CHECK_EQ(0x45678, s[2].GetValue());
#endif
    }

    // Test replace(iterator first, iterator last, size_type n, wxUniChar ch).
    {
        wxString s = L"\x3042\x208\x3059";
        s.replace(s.begin() + 1, s.end(), 2, wxUniChar(0x34567));
#if wxUSE_UNICODE_UTF16
        CHECK_EQ(5, s.length());
        CHECK_EQ(0xD891, s[1].GetValue());
        CHECK_EQ(0xDD67, s[2].GetValue());
        CHECK_EQ(0xD891, s[3].GetValue());
        CHECK_EQ(0xDD67, s[4].GetValue());
#else
        CHECK_EQ(3, s.length());
        CHECK_EQ(0x34567, s[1].GetValue());
        CHECK_EQ(0x34567, s[2].GetValue());
#endif
    }

    // Test find(wxUniChar ch, size_t nStart = 0)
    // and rfind(wxUniChar ch, size_t nStart = npos).
    {
        wxString s = L"\x308\x2063";
        s << wxUniChar(0x12345);
        s << "x";
        s += wxUniChar(0x12345);
        s += "y";
#if wxUSE_UNICODE_UTF16
        CHECK_EQ(8, s.length());
        CHECK_EQ(2, s.find(wxUniChar(0x12345)));
        CHECK_EQ(5, s.find(wxUniChar(0x12345), 3));
        CHECK_EQ(5, s.rfind(wxUniChar(0x12345)));
        CHECK_EQ(2, s.rfind(wxUniChar(0x12345), 4));
#else
        CHECK_EQ(6, s.length());
        CHECK_EQ(2, s.find(wxUniChar(0x12345)));
        CHECK_EQ(4, s.find(wxUniChar(0x12345), 3));
        CHECK_EQ(4, s.rfind(wxUniChar(0x12345)));
        CHECK_EQ(2, s.rfind(wxUniChar(0x12345), 3));
#endif
    }

    /* Not tested here:
         find_first_of, find_last_of, find_first_not_of, find_last_not_of
    */
}
