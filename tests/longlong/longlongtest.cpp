///////////////////////////////////////////////////////////////////////////////
// Name:        tests/longlong/longlong.cpp
// Purpose:     wxLongLong unit test
// Author:      Vadim Zeitlin, Wlodzimierz ABX Skiba
// Created:     2004-04-01
// Copyright:   (c) 2004 Vadim Zeitlin, Wlodzimierz Skiba
///////////////////////////////////////////////////////////////////////////////


#include "doctest.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif // WX_PRECOMP

#include "wx/longlong.h"
#include "wx/timer.h"

#if wxUSE_LONGLONG

// ----------------------------------------------------------------------------
// helpers for testing
// ----------------------------------------------------------------------------

// number of iterations in loops
constexpr int ITEMS = 1000;

// make a 64 bit number from 4 16 bit ones
#define MAKE_LL(x1, x2, x3, x4) wxLongLong((x1 << 16) | x2, (x3 << 16) | x3)

// get a random 64 bit number
#define RAND_LL()   MAKE_LL(rand(), rand(), rand(), rand())

constexpr long testLongs[] =
{
    0,
    1,
    -1,
    LONG_MAX,
    LONG_MIN,
    0x1234,
    -0x1234
};

TEST_CASE("Conversions")
{
    for ( size_t n = 0; n < ITEMS; n++ )
    {
        wxLongLong a = RAND_LL();

        wxLongLong b(a.GetHi(), a.GetLo());
        CHECK( a == b );

#if wxUSE_LONGLONG_WX
        wxLongLongWx c(a.GetHi(), a.GetLo());
        CHECK( a == c );
#endif

#if wxUSE_LONGLONG_NATIVE
        wxLongLongNative d(a.GetHi(), a.GetLo());
        CHECK( a == d );
#endif
    }
}

TEST_CASE("Comparison")
{
    static constexpr long ls[2] =
    {
        0x1234,
       -0x1234,
    };

    wxLongLong lls[2];
    lls[0] = ls[0];
    lls[1] = ls[1];

    for ( size_t n = 0; n < WXSIZEOF(testLongs); n++ )
    {
        for ( size_t m = 0; m < WXSIZEOF(lls); m++ )
        {
            CHECK( (lls[m] < testLongs[n]) == (ls[m] < testLongs[n]) );
            CHECK( (lls[m] > testLongs[n]) == (ls[m] > testLongs[n]) );
            CHECK( (lls[m] <= testLongs[n]) == (ls[m] <= testLongs[n]) );
            CHECK( (lls[m] >= testLongs[n]) == (ls[m] >= testLongs[n]) );
            CHECK( (lls[m] != testLongs[n]) == (ls[m] != testLongs[n]) );
            CHECK( (lls[m] == testLongs[n]) == (ls[m] == testLongs[n]) );
        }
    }
}

TEST_CASE("Addition")
{
    for ( size_t n = 0; n < ITEMS; n++ )
    {
        wxLongLong a = RAND_LL();
        wxLongLong b = RAND_LL();
        wxLongLong c = a + b;

#if wxUSE_LONGLONG_NATIVE
        wxLongLongNative a1 = a;
        wxLongLongNative b1 = b;
        wxLongLongNative c1 = a1 + b1;
        CHECK( c == c1 );
#endif

#if wxUSE_LONGLONG_WX
        wxLongLongWx a2 = a;
        wxLongLongWx b2 = b;
        wxLongLongWx c2 = a2 + b2;
        CHECK( c == c2 );
#endif
    }
}

TEST_CASE("Multiplication")
{
    for ( size_t n = 0; n < ITEMS; n++ )
    {
        wxLongLong a = RAND_LL();
        wxLongLong b = RAND_LL();
        wxLongLong c = a*b;

        wxLongLong a1(a.GetHi(), a.GetLo());
        wxLongLong b1(b.GetHi(), b.GetLo());
        wxLongLong c1 = a1*b1;
        CHECK( c1 == c );

#if wxUSE_LONGLONG_WX
        wxLongLongWx a2(a.GetHi(), a.GetLo());
        wxLongLongWx b2(b.GetHi(), b.GetLo());
        wxLongLongWx c2 = a2*b2;
        CHECK( c2 == c );
#endif

#if wxUSE_LONGLONG_NATIVE
        wxLongLongNative a3(a.GetHi(), a.GetLo());
        wxLongLongNative b3(b.GetHi(), b.GetLo());
        wxLongLongNative c3 = a3*b3;
        CHECK( c3 == c );
#endif
    }
}

TEST_CASE("Division")
{
    for ( size_t n = 0; n < ITEMS; n++ )
    {
        // get a random wxLongLong (shifting by 12 the MSB ensures that the
        // multiplication will not overflow)
        wxLongLong a = MAKE_LL((rand() >> 12), rand(), rand(), rand());

        // get a random (but non null) long (not wxLongLong for now) divider
        long l;
        do
        {
           l = rand();
        }
        while ( !l );

        wxLongLong q = a / l;
        wxLongLong r = a % l;

        CHECK( a == ( q * l + r ) );

#if wxUSE_LONGLONG_WX
        wxLongLongWx a1(a.GetHi(), a.GetLo());
        wxLongLongWx q1 = a1 / l;
        wxLongLongWx r1 = a1 % l;
        CHECK( q == q1 );
        CHECK( r == r1 );
        CHECK( a1 == ( q1 * l + r1 ) );
#endif

#if wxUSE_LONGLONG_NATIVE
        wxLongLongNative a2(a.GetHi(), a.GetLo());
        wxLongLongNative q2 = a2 / l;
        wxLongLongNative r2 = a2 % l;
        CHECK( q == q2 );
        CHECK( r == r2 );
        CHECK( a2 == ( q2 * l + r2 ) );
#endif
    }
}

TEST_CASE("Bit Operations")
{
    for ( size_t m = 0; m < ITEMS; m++ )
    {
        wxLongLong a = RAND_LL();

        for ( size_t n = 0; n < 33; n++ )
        {
            wxLongLong b(a.GetHi(), a.GetLo()), c, d = b, e;
            d >>= n;
            c = b >> n;
            CHECK( c == d );
            d <<= n;
            e = c << n;
            CHECK( d == e );

#if wxUSE_LONGLONG_WX
            wxLongLongWx b1(a.GetHi(), a.GetLo()), c1, d1 = b1, e1;
            d1 >>= n;
            c1 = b1 >> n;
            CHECK( c1 == d1 );
            d1 <<= n;
            e1 = c1 << n;
            CHECK( d1 == e1 );
#endif

#if wxUSE_LONGLONG_NATIVE
            wxLongLongNative b2(a.GetHi(), a.GetLo()), c2, d2 = b2, e2;
            d2 >>= n;
            c2 = b2 >> n;
            CHECK( c2 == d2 );
            d2 <<= n;
            e2 = c2 << n;
            CHECK( d2 == e2 );
#endif
        }
    }
}

TEST_CASE("To String")
{
    wxString s1, s2;

    for ( size_t n = 0; n < WXSIZEOF(testLongs); n++ )
    {
        wxLongLong a = testLongs[n];
        s1 = wxString::Format(wxT("%ld"), testLongs[n]);
        s2 = a.ToString();
        CHECK( s1 == s2 );

        s2 = {};
        s2 << a;
        CHECK( s1 == s2 );

#if wxUSE_LONGLONG_WX
        wxLongLongWx a1 = testLongs[n];
        s2 = a1.ToString();
        CHECK( s1 == s2 );
#endif

#if wxUSE_LONGLONG_NATIVE
        wxLongLongNative a2 = testLongs[n];
        s2 = a2.ToString();
        CHECK( s1 == s2 );
#endif
    }

    wxLongLong a(0x12345678, 0x87654321);
    CHECK( a.ToString() == wxT("1311768467139281697") );
    a.Negate();
    CHECK( a.ToString() == wxT("-1311768467139281697") );

    wxLongLong llMin(-2147483647L - 1L, 0);
    CHECK( llMin.ToString() == wxT("-9223372036854775808") );

#if wxUSE_LONGLONG_WX
    wxLongLongWx a1(a.GetHi(), a.GetLo());
    CHECK( a1.ToString() == wxT("-1311768467139281697") );
    a1.Negate();
    CHECK( a1.ToString() == wxT("1311768467139281697") );
#endif

#if wxUSE_LONGLONG_NATIVE
    wxLongLongNative a2(a.GetHi(), a.GetLo());
    CHECK( a2.ToString() == wxT("-1311768467139281697") );
    a2.Negate();
    CHECK( a2.ToString() == wxT("1311768467139281697") );
#endif

}

TEST_CASE("Lo - Hi")
{
    wxLongLong ll(123, 456);
    CHECK_EQ( 456u, ll.GetLo() );
    CHECK_EQ( 123, ll.GetHi() );

    wxULongLong ull(987, 654);
    CHECK_EQ( 654u, ull.GetLo() );
    CHECK_EQ( 987u, ull.GetHi() );
}

TEST_CASE("Limits")
{
#if wxUSE_LONGLONG_NATIVE
    CHECK( std::numeric_limits<wxLongLong>::is_specialized );
    CHECK( std::numeric_limits<wxULongLong>::is_specialized );

    wxULongLong maxval = std::numeric_limits<wxULongLong>::max();
    CHECK( maxval.ToDouble() > 0 );
#endif // wxUSE_LONGLONG_NATIVE
}

#endif // wxUSE_LONGLONG
