///////////////////////////////////////////////////////////////////////////////
// Name:        tests/net/socket.cpp
// Purpose:     wxSocket unit tests
// Author:      Vadim Zeitlin
// Copyright:   (c) 2008 Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/*
    IMPORTANT NOTE: the environment variable WX_TEST_SERVER must be set to the
    hostname of the server to use for the tests below, if it is not set all
    tests are silently skipped (rationale: this makes it possible to run the
    test in the restricted environments (e.g. sandboxes) without any network
    connectivity).
 */

#include "doctest.h"

#include "testprec.h"


#if wxUSE_SOCKETSNO

#include "wx/socket.h"
#include "wx/url.h"
#include "wx/evtloop.h"

import WX.Cmn.StrStream;

typedef std::unique_ptr<wxSockAddress> wxSockAddressPtr;
typedef std::unique_ptr<wxSocketClient> wxSocketClientPtr;

static wxString gs_serverHost(wxGetenv("WX_TEST_SERVER"));

class SocketTestCase : public CppUnit::TestCase
{
public:
    SocketTestCase() { }

    // get the address to connect to, if NULL is returned it means that the
    // test is disabled and shouldn't run at all
    static wxSockAddress* GetServer();

    // get the socket to read HTTP reply from, returns NULL if the test is
    // disabled
    static wxSocketClient* GetHTTPSocket(int flags = wxSOCKET_NONE);

private:
    // we need to repeat the tests twice as the sockets behave differently when
    // there is an active event loop and without it
    #define ALL_SOCKET_TESTS() \
        CPPUNIT_TEST( BlockingConnect ); \
        CPPUNIT_TEST( NonblockingConnect ); \
        CPPUNIT_TEST( ReadNormal ); \
        CPPUNIT_TEST( ReadBlock ); \
        CPPUNIT_TEST( ReadNowait ); \
        CPPUNIT_TEST( ReadWaitall ); \
        CPPUNIT_TEST( ReadAnotherThread ); \
        CPPUNIT_TEST( UrlTest )

    CPPUNIT_TEST_SUITE( SocketTestCase );
        ALL_SOCKET_TESTS();
        CPPUNIT_TEST( PseudoTest_SetUseEventLoop );
        ALL_SOCKET_TESTS();
    CPPUNIT_TEST_SUITE_END();

    // helper event loop class which sets itself as active only if we pass it
    // true in ctor
    class SocketTestEventLoop : public wxEventLoop
    {
    public:
        SocketTestEventLoop(bool useLoop)
        {
            m_useLoop = useLoop;
            if ( useLoop )
            {
                m_evtLoopOld = wxEventLoopBase::GetActive();
                SetActive(this);
            }
        }

        virtual ~SocketTestEventLoop()
        {
            if ( m_useLoop )
            {
                wxEventLoopBase::SetActive(m_evtLoopOld);
            }
        }

    private:
        bool m_useLoop;
        wxEventLoopBase *m_evtLoopOld;
    };

    void PseudoTest_SetUseEventLoop() { ms_useLoop = true; }

    void BlockingConnect();
    void NonblockingConnect();
    void ReadNormal();
    void ReadBlock();
    void ReadNowait();
    void ReadWaitall();
    void ReadAnotherThread();

    void UrlTest();

    static bool ms_useLoop;

    SocketTestCase(const SocketTestCase&) = delete;
	SocketTestCase& operator=(const SocketTestCase&) = delete;
};

bool SocketTestCase::ms_useLoop = false;

CPPUNIT_TEST_SUITE_REGISTRATION( SocketTestCase );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SocketTestCase, "SocketTestCase" );

wxSockAddress* SocketTestCase::GetServer()
{
    if ( gs_serverHost.empty() )
        return NULL;

    wxIPV4address *addr = new wxIPV4address;
    addr->Hostname(gs_serverHost);
    addr->Service("www");

    return addr;
}

wxSocketClient* SocketTestCase::GetHTTPSocket(int flags)
{
    wxSockAddress *addr = GetServer();
    if ( !addr )
        return NULL;

    wxSocketClient *sock = new wxSocketClient(flags);
    sock->SetTimeout(1);
    CHECK( sock->Connect(*addr) );

    const wxString httpGetRoot =
        "GET / HTTP/1.1\r\n"
        "Host: " + gs_serverHost + "\r\n"
        "\r\n";

    sock->Write(httpGetRoot.ToAscii(), httpGetRoot.length());

    return sock;
}

void SocketTestCase::BlockingConnect()
{
    wxSockAddressPtr addr(GetServer());
    if ( !addr.get() )
        return;

    wxSocketClient sock;
    CHECK( sock.Connect(*addr) );
}

void SocketTestCase::NonblockingConnect()
{
    wxSockAddressPtr addr(GetServer());
    if ( !addr.get() )
        return;

    SocketTestEventLoop loop(ms_useLoop);

    wxSocketClient sock;
    sock.Connect(*addr, false);
    sock.WaitOnConnect(10);

    CHECK( sock.IsConnected() );
}

void SocketTestCase::ReadNormal()
{
    SocketTestEventLoop loop(ms_useLoop);

    wxSocketClientPtr sock(GetHTTPSocket());
    if ( !sock.get() )
        return;

    char bufSmall[128];
    sock->Read(bufSmall, WXSIZEOF(bufSmall));

    CHECK_EQ( wxSocketError::None, sock->LastError() );
    CHECK_EQ( WXSIZEOF(bufSmall), (size_t)sock->LastCount() );
    CHECK_EQ( WXSIZEOF(bufSmall), (size_t)sock->LastReadCount() );


    char bufBig[102400];
    sock->Read(bufBig, WXSIZEOF(bufBig));

    CHECK_EQ( wxSocketError::None, sock->LastError() );
    CHECK( WXSIZEOF(bufBig) >= sock->LastCount() );
    CHECK( WXSIZEOF(bufBig) >= sock->LastReadCount() );
}

void SocketTestCase::ReadBlock()
{
    wxSocketClientPtr sock(GetHTTPSocket(wxSOCKET_BLOCK));
    if ( !sock.get() )
        return;

    char bufSmall[128];
    sock->Read(bufSmall, WXSIZEOF(bufSmall));

    CHECK_EQ( wxSocketError::None, sock->LastError() );
    CHECK_EQ( WXSIZEOF(bufSmall), (size_t)sock->LastCount() );
    CHECK_EQ( WXSIZEOF(bufSmall), (size_t)sock->LastReadCount() );


    char bufBig[102400];
    sock->Read(bufBig, WXSIZEOF(bufBig));

    CHECK_EQ( wxSocketError::None, sock->LastError() );
    CHECK( WXSIZEOF(bufBig) >= sock->LastCount() );
    CHECK( WXSIZEOF(bufBig) >= sock->LastReadCount() );
}

void SocketTestCase::ReadNowait()
{
    wxSocketClientPtr sock(GetHTTPSocket(wxSOCKET_NOWAIT));
    if ( !sock.get() )
        return;

    char buf[1024];
    sock->Read(buf, WXSIZEOF(buf));
    if ( sock->LastError() != wxSocketError::WouldBlock )
    {
        CHECK_EQ( wxSocketError::None, sock->LastError() );
    }
}

void SocketTestCase::ReadWaitall()
{
    SocketTestEventLoop loop(ms_useLoop);

    wxSocketClientPtr sock(GetHTTPSocket(wxSOCKET_WAITALL));
    if ( !sock.get() )
        return;

    char buf[128];
    sock->Read(buf, WXSIZEOF(buf));

    CHECK_EQ( wxSocketError::None, sock->LastError() );
    CHECK_EQ( WXSIZEOF(buf), (size_t)sock->LastCount() );
    CHECK_EQ( WXSIZEOF(buf), (size_t)sock->LastReadCount() );
}

void SocketTestCase::ReadAnotherThread()
{
    class SocketThread : public wxThread
    {
    public:
        SocketThread()
            : wxThread( wxThreadKind::Joinable)
        {
        }

        void* Entry() override
        {
            wxSocketClientPtr sock(SocketTestCase::GetHTTPSocket(wxSOCKET_BLOCK));
            if ( !sock )
                return NULL;

            char bufSmall[128];
            sock->Read(bufSmall, WXSIZEOF(bufSmall));

            REQUIRE( sock->LastError() == wxSocketError::None );
            CHECK( sock->LastCount() == WXSIZEOF(bufSmall) );
            CHECK( sock->LastReadCount() == WXSIZEOF(bufSmall) );

            REQUIRE_NOTHROW( sock.reset() );

            return NULL;
        }
    };

    SocketThread thr;

    SocketTestEventLoop loop(ms_useLoop);

    thr.Run();

    CHECK( thr.Wait() == NULL );
}

void SocketTestCase::UrlTest()
{
    if ( gs_serverHost.empty() )
        return;

    SocketTestEventLoop loop(ms_useLoop);

    wxURL url("http://" + gs_serverHost);

    const std::unique_ptr<wxInputStream> in(url.GetInputStream());
    CHECK( in.get() );

    wxStringOutputStream out;
    CHECK_EQ( wxSTREAM_EOF, in->Read(out).GetLastError() );
}

#endif // wxUSE_SOCKETS
