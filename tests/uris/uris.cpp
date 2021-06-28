///////////////////////////////////////////////////////////////////////////////
// Name:        tests/uris/uris.cpp
// Purpose:     wxURI unit test
// Author:      Ryan Norton
// Created:     2004-08-14
// Copyright:   (c) 2004 Ryan Norton
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "doctest.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif // WX_PRECOMP

#include "wx/uri.h"
#include "wx/url.h"

// Test wxURL & wxURI compat?
#define TEST_URL wxUSE_URL


// apply the given accessor to the URI, check that the result is as expected
#define URI_ASSERT_PART_EQUAL(uri, expected, accessor) \
    CHECK_EQ(wxURI(uri).accessor, expected)

#define URI_ASSERT_HOSTTYPE_EQUAL(uri, expected) \
    URI_ASSERT_PART_EQUAL((uri), (expected), GetHostType())

#define URI_ASSERT_SERVER_EQUAL(uri, expected) \
    URI_ASSERT_PART_EQUAL((uri), (expected), GetServer())

#define URI_ASSERT_PATH_EQUAL(uri, expected) \
    URI_ASSERT_PART_EQUAL((uri), (expected), GetPath())

#define URI_ASSERT_USER_EQUAL(uri, expected) \
    URI_ASSERT_PART_EQUAL((uri), (expected), GetUser())

TEST_CASE("IPv4")
{
    URI_ASSERT_HOSTTYPE_EQUAL("http://user:password@192.168.1.100:5050/path",
                            wxURI_IPV4ADDRESS);

    URI_ASSERT_HOSTTYPE_EQUAL("http://user:password@192.255.1.100:5050/path",
                            wxURI_IPV4ADDRESS);

    // bogus ipv4
    CHECK( wxURI("http://user:password@192.256.1.100:5050/path").
                    GetHostType() != wxURI_IPV4ADDRESS);
}

TEST_CASE("IPv6")
{
    // IPv6address   =                            6( h16 ":" ) ls32
    //               /                       "::" 5( h16 ":" ) ls32
    //               / [               h16 ] "::" 4( h16 ":" ) ls32
    //               / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
    //               / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
    //               / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
    //               / [ *4( h16 ":" ) h16 ] "::"              ls32
    //               / [ *5( h16 ":" ) h16 ] "::"              h16
    //               / [ *6( h16 ":" ) h16 ] "::"
    // ls32          = ( h16 ":" h16 ) / IPv4address

    URI_ASSERT_HOSTTYPE_EQUAL
    (
        "http://user:password@[aa:aa:aa:aa:aa:aa:192.168.1.100]:5050/path",
        wxURI_IPV6ADDRESS
    );

    URI_ASSERT_HOSTTYPE_EQUAL
    (
        "http://user:password@[aa:aa:aa:aa:aa:aa:aa:aa]:5050/path",
        wxURI_IPV6ADDRESS
    );

    URI_ASSERT_HOSTTYPE_EQUAL
    (
        "http://user:password@[aa:aa:aa:aa::192.168.1.100]:5050/path",
        wxURI_IPV6ADDRESS
    );

    URI_ASSERT_HOSTTYPE_EQUAL
    (
        "http://user:password@[aa:aa:aa:aa::aa:aa]:5050/path",
        wxURI_IPV6ADDRESS
    );
}

TEST_CASE("Server")
{
    URI_ASSERT_SERVER_EQUAL("http://foo/", "foo");
    URI_ASSERT_SERVER_EQUAL("http://foo-bar/", "foo-bar");
    URI_ASSERT_SERVER_EQUAL("http://foo/bar/", "foo");
    URI_ASSERT_SERVER_EQUAL("http://192.168.1.0/", "192.168.1.0");
    URI_ASSERT_SERVER_EQUAL("http://192.168.1.17/", "192.168.1.17");
    URI_ASSERT_SERVER_EQUAL("http://192.168.1.255/", "192.168.1.255");
    URI_ASSERT_SERVER_EQUAL("http://192.168.1.1/index.html", "192.168.1.1");
    URI_ASSERT_SERVER_EQUAL("http://[aa:aa:aa:aa::aa:aa]/foo", "aa:aa:aa:aa::aa:aa");
}

TEST_CASE("Paths")
{
    URI_ASSERT_PATH_EQUAL("http://user:password@192.256.1.100:5050/../path",
                          "/path");

    URI_ASSERT_PATH_EQUAL("http://user:password@192.256.1.100:5050/path/../",
                          "/");

    URI_ASSERT_PATH_EQUAL("http://user:password@192.256.1.100:5050/path/.",
                          "/path/");

    URI_ASSERT_PATH_EQUAL("http://user:password@192.256.1.100:5050/path/./",
                          "/path/");

    URI_ASSERT_PART_EQUAL("path/john/../../../joe",
                          "../joe", BuildURI());

    // According to RFC 3986, when the authority is present, the path must
    // begin with a slash (or be empty) and when there is no authority, the
    // path cannot begin with two slashes, so check for this.
    URI_ASSERT_PATH_EQUAL("http://good.com:8042BADPATH", "");
    URI_ASSERT_PATH_EQUAL("http://good.com:8042/GOODPATH", "/GOODPATH");
    URI_ASSERT_PATH_EQUAL("//BADPATH", "");
}

TEST_CASE("User and Password")
{
    URI_ASSERT_USER_EQUAL("http://user:pass@host/path/", "user");
    URI_ASSERT_USER_EQUAL("http://user@host/path/", "user");
    URI_ASSERT_USER_EQUAL("http://host/path/", "");
}

#define URI_TEST_RESOLVE_IMPL(string, eq, strict) \
    { \
        wxURI uri(string); \
        uri.Resolve(masteruri, strict); \
        CHECK_EQ(eq, uri.BuildURI()); \
    }

#define URI_TEST_RESOLVE(string, eq) \
        URI_TEST_RESOLVE_IMPL(string, eq, true);

#define URI_TEST_RESOLVE_LAX(string, eq) \
        URI_TEST_RESOLVE_IMPL(string, eq, false);


//examples taken from RFC 2396.bis
/*
TEST_CASE("Normal Resolving")
{
    wxURI masteruri("http://a/b/c/d;p?q");

    URI_TEST_RESOLVE("g:h"  ,"g:h")
    URI_TEST_RESOLVE("g"    ,"http://a/b/c/g")
    URI_TEST_RESOLVE("./g"  ,"http://a/b/c/g")
    URI_TEST_RESOLVE("g/"   ,"http://a/b/c/g/")
    URI_TEST_RESOLVE("/g"   ,"http://a/g")
    URI_TEST_RESOLVE("//g"  ,"http://g")
    URI_TEST_RESOLVE("?y"   ,"http://a/b/c/d;p?y")
    URI_TEST_RESOLVE("g?y"  ,"http://a/b/c/g?y")
    URI_TEST_RESOLVE("#s"   ,"http://a/b/c/d;p?q#s")
    URI_TEST_RESOLVE("g#s"  ,"http://a/b/c/g#s")
    URI_TEST_RESOLVE("g?y#s","http://a/b/c/g?y#s")
    URI_TEST_RESOLVE(";x"   ,"http://a/b/c/;x")
    URI_TEST_RESOLVE("g;x"  ,"http://a/b/c/g;x")
    URI_TEST_RESOLVE("g;x?y#s","http://a/b/c/g;x?y#s")

    URI_TEST_RESOLVE(""     ,"http://a/b/c/d;p?q")
    URI_TEST_RESOLVE("."    ,"http://a/b/c/")
    URI_TEST_RESOLVE("./"   ,"http://a/b/c/")
    URI_TEST_RESOLVE(".."   ,"http://a/b/")
    URI_TEST_RESOLVE("../"  ,"http://a/b/")
    URI_TEST_RESOLVE("../g" ,"http://a/b/g")
    URI_TEST_RESOLVE("../..","http://a/")
    URI_TEST_RESOLVE("../../"        ,  "http://a/")
    URI_TEST_RESOLVE("../../g"       ,  "http://a/g")
}
*/
TEST_CASE("Complex resolving")
{
    wxURI masteruri("http://a/b/c/d;p?q");

    //odd path examples
    URI_TEST_RESOLVE("../../../g"   , "http://a/g")
    URI_TEST_RESOLVE("../../../../g", "http://a/g")

    URI_TEST_RESOLVE("/./g"   ,"http://a/g")
    URI_TEST_RESOLVE("/../g"  ,"http://a/g")
    URI_TEST_RESOLVE("g."     ,"http://a/b/c/g.")
    URI_TEST_RESOLVE(".g"     ,"http://a/b/c/.g")
    URI_TEST_RESOLVE("g.."    ,"http://a/b/c/g..")
    URI_TEST_RESOLVE("..g"    ,"http://a/b/c/..g")
}

TEST_CASE("Really complex resolving")
{
    wxURI masteruri("http://a/b/c/d;p?q");

    //even more odder path examples
    URI_TEST_RESOLVE("./../g" ,"http://a/b/g")
    URI_TEST_RESOLVE("./g/."  ,"http://a/b/c/g/")
    URI_TEST_RESOLVE("g/./h"  ,"http://a/b/c/g/h")
    URI_TEST_RESOLVE("g/../h" ,"http://a/b/c/h")
    URI_TEST_RESOLVE("g;x=1/./y"     ,  "http://a/b/c/g;x=1/y")
    URI_TEST_RESOLVE("g;x=1/../y"    ,  "http://a/b/c/y")
}

TEST_CASE("Query fragment resolving")
{
    wxURI masteruri("http://a/b/c/d;p?q");

    //query/fragment ambigiousness
    URI_TEST_RESOLVE("g?y/./x","http://a/b/c/g?y/./x")
    URI_TEST_RESOLVE("g?y/../x"      ,  "http://a/b/c/g?y/../x")
    URI_TEST_RESOLVE("g#s/./x","http://a/b/c/g#s/./x")
    URI_TEST_RESOLVE("g#s/../x"      ,  "http://a/b/c/g#s/../x")
}

TEST_CASE("Backwards resolving")
{
    wxURI masteruri("http://a/b/c/d;p?q");

    //"NEW"
    URI_TEST_RESOLVE("http:g" ,  "http:g")         //strict
    //bw compat
    URI_TEST_RESOLVE_LAX("http:g", "http://a/b/c/g");
}

TEST_CASE("Assignment")
{
    wxURI uri1("http://mysite.com"),
          uri2("http://mysite2.com");

    uri2 = uri1;

    CHECK_EQ(uri1.BuildURI(), uri2.BuildURI());
}

TEST_CASE("Comparison")
{
    CHECK(wxURI("http://mysite.com") == wxURI("http://mysite.com"));
}

TEST_CASE("Unescaping")
{
    wxString escaped,
             unescaped;

    escaped = "http://test.com/of/file%3A%2F%2FC%3A%5Curi%5C"
              "escaping%5Cthat%5Cseems%5Cbroken%5Csadly%5B1%5D.rss";

    unescaped = wxURI(escaped).BuildUnescapedURI();

    CHECK_EQ( "http://test.com/of/file://C:\\uri\\"
              "escaping\\that\\seems\\broken\\sadly[1].rss",
              unescaped );

    CHECK_EQ( unescaped, wxURI::Unescape(escaped) );


    escaped = "http://ru.wikipedia.org/wiki/"
              "%D0%A6%D0%B5%D0%BB%D0%BE%D0%B5_%D1%87%D0%B8%D1%81%D0%BB%D0%BE";

    unescaped = wxURI::Unescape(escaped);

    CHECK_EQ( wxString::FromUTF8(
              "http://ru.wikipedia.org/wiki/"
              "\xD0\xA6\xD0\xB5\xD0\xBB\xD0\xBE\xD0\xB5_"
              "\xD1\x87\xD0\xB8\xD1\x81\xD0\xBB\xD0\xBE"
              ),
              unescaped );

    escaped = L"file://\u043C\u043E\u0439%5C%d1%84%d0%b0%d0%b9%d0%bb";
    unescaped = wxURI::Unescape(escaped);

    CHECK_EQ
    (
        L"file://\u043C\u043E\u0439\\\u0444\u0430\u0439\u043B",
        unescaped
    );
}

TEST_CASE("File scheme")
{
    //file:// variety (NOT CONFORMING TO THE RFC)
    URI_ASSERT_PATH_EQUAL( "file://e:/wxcode/script1.xml",
                    "e:/wxcode/script1.xml" );

    //file:/// variety
    URI_ASSERT_PATH_EQUAL( "file:///e:/wxcode/script1.xml",
                    "/e:/wxcode/script1.xml" );

    //file:/ variety
    URI_ASSERT_PATH_EQUAL( "file:/e:/wxcode/script1.xml",
                    "/e:/wxcode/script1.xml" );

    //file: variety
    URI_ASSERT_PATH_EQUAL( "file:e:/wxcode/script1.xml",
                    "e:/wxcode/script1.xml" );
}

#if TEST_URL

#include "wx/url.h"
#include "wx/file.h"

TEST_CASE("URL Compat")
{
    wxURL url("http://user:password@wxwidgets.org");

    CHECK( url.GetError() == wxURL_NOERR );
    CHECK( url == wxURL("http://user:password@wxwidgets.org") );

    wxURI uri("http://user:password@wxwidgets.org");

    CHECK( url == uri );

    wxURL urlcopy(uri);

    CHECK( urlcopy == url );
    CHECK( urlcopy == uri );

    wxURI uricopy(url);

    CHECK( uricopy == url );
    CHECK( uricopy == urlcopy );
    CHECK( uricopy == uri );
    CHECK_EQ( " A ", wxURI::Unescape("%20%41%20") );

    wxURI test("file:\"myf\"ile.txt");

    CHECK_EQ( "file:%22myf%22ile.txt" , test.BuildURI() );
    CHECK_EQ( "file", test.GetScheme() );
    CHECK_EQ( "%22myf%22ile.txt", test.GetPath() );

    // these could be put under a named registry since they take some
    // time to complete
#if 0
    // Test problem urls (reported not to work some time ago by a user...)
    const wxChar* pszProblemUrls[] = { "http://www.csdn.net",
                                       "http://www.163.com",
                                       "http://www.sina.com.cn" };

    for ( size_t i = 0; i < WXSIZEOF(pszProblemUrls); ++i )
    {
        wxURL urlProblem(pszProblemUrls[i]);
        CPPUNIT_ASSERT(urlProblem.GetError() == wxURL_NOERR);

        wxInputStream* is = urlProblem.GetInputStream();
        CPPUNIT_ASSERT(is != NULL);

        wxFile fOut(wxT("test.html"), wxFile::write);
        wxASSERT(fOut.IsOpened());

        char buf[1001];
        for( ;; )
        {
            is->Read(buf, 1000);
            size_t n = is->LastRead();
            if ( n == 0 )
                break;
            buf[n] = 0;
            fOut.Write(buf, n);
        }

        delete is;
    }
#endif
}

// the purpose of this test is unclear, it seems to be unfinished so disabling
// it for now
#if 0 && wxUSE_PROTOCOL_HTTP
void URITestCase::URLProxy()
{
    wxURL url(wxT("http://www.asite.com/index.html"));
    url.SetProxy(wxT("pserv:3122"));

    wxURL::SetDefaultProxy(wxT("fol.singnet.com.sg:8080"));
    wxURL url2(wxT("http://server-name/path/to/file?query_data=value"));
    wxInputStream *data = url2.GetInputStream();
    CPPUNIT_ASSERT(data != NULL);
}
#endif // wxUSE_PROTOCOL_HTTP

#endif // TEST_URL
