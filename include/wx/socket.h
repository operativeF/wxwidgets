/////////////////////////////////////////////////////////////////////////////
// Name:        wx/socket.h
// Purpose:     Socket handling classes
// Authors:     Guilhem Lavaux, Guillermo Rodriguez Garcia
// Modified by:
// Created:     April 1997
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SOCKET_H_
#define _WX_SOCKET_H_

#if wxUSE_SOCKETS

// ---------------------------------------------------------------------------
// wxSocket headers
// ---------------------------------------------------------------------------

#include "wx/event.h"
#include "wx/sckaddr.h"
#include "wx/list.h"

import <cstdint>;
#include <memory>

class wxSocketImpl;

// ------------------------------------------------------------------------
// Types and constants
// ------------------------------------------------------------------------

// Define the type of native sockets.
#if defined(WX_WINDOWS)
    // Although socket descriptors are still 32 bit values, even under Win64,
    // the socket type is 64 bit there.
    using wxSOCKET_T = wxUIntPtr;
#else
    using wxSOCKET_T = int;
#endif


// Types of different socket notifications or events.
//
// NB: the values here should be consecutive and start with 0 as they are
//     used to construct the wxSOCKET_XXX_FLAG bit mask values below
enum wxSocketNotify
{
    wxSOCKET_INPUT,
    wxSOCKET_OUTPUT,
    wxSOCKET_CONNECTION,
    wxSOCKET_LOST
};

enum
{
    wxSOCKET_INPUT_FLAG = 1 << wxSOCKET_INPUT,
    wxSOCKET_OUTPUT_FLAG = 1 << wxSOCKET_OUTPUT,
    wxSOCKET_CONNECTION_FLAG = 1 << wxSOCKET_CONNECTION,
    wxSOCKET_LOST_FLAG = 1 << wxSOCKET_LOST
};

// this is a combination of the bit masks defined above
using wxSocketEventFlags = int;

enum class wxSocketError
{
    None,
    InvOp,
    IOErr,
    InvAddr,
    InvSock,
    NoHost,
    InvPort,
    WouldBlock,
    Timeout,
    MemErr,
    OptErr
};

// socket options/flags bit masks
enum
{
    wxSOCKET_NONE           = 0x0000,
    wxSOCKET_NOWAIT_READ    = 0x0001,
    wxSOCKET_NOWAIT_WRITE   = 0x0002,
    wxSOCKET_NOWAIT         = wxSOCKET_NOWAIT_READ | wxSOCKET_NOWAIT_WRITE,
    wxSOCKET_WAITALL_READ   = 0x0004,
    wxSOCKET_WAITALL_WRITE  = 0x0008,
    wxSOCKET_WAITALL        = wxSOCKET_WAITALL_READ | wxSOCKET_WAITALL_WRITE,
    wxSOCKET_BLOCK          = 0x0010,
    wxSOCKET_REUSEADDR      = 0x0020,
    wxSOCKET_BROADCAST      = 0x0040,
    wxSOCKET_NOBIND         = 0x0080
};

using wxSocketFlags = int;

// socket kind values (badly defined, don't use)
enum wxSocketType
{
    wxSOCKET_UNINIT,
    wxSOCKET_CLIENT,
    wxSOCKET_SERVER,
    wxSOCKET_BASE,
    wxSOCKET_DATAGRAM
};


// event
class wxSocketEvent;
wxDECLARE_EVENT(wxEVT_SOCKET, wxSocketEvent);

// --------------------------------------------------------------------------
// wxSocketBase
// --------------------------------------------------------------------------

class wxSocketBase : public wxObject
{
public:
    // Public interface
    // ----------------

    // ctors and dtors
    wxSocketBase();
    wxSocketBase(wxSocketFlags flags, wxSocketType type);
    ~wxSocketBase();

    void Init();
    bool Destroy();

    // state
    bool IsOk() const { return m_impl != nullptr; }
    bool Error() const { return LastError() != wxSocketError::None; }
    bool IsClosed() const { return m_closed; }
    bool IsConnected() const { return m_connected; }
    bool IsData() { return WaitForRead(0, 0); }
    bool IsDisconnected() const { return !IsConnected(); }
    std::uint32_t LastCount() const { return m_lcount; }
    std::uint32_t LastReadCount() const { return m_lcount_read; }
    std::uint32_t LastWriteCount() const { return m_lcount_write; }
    wxSocketError LastError() const;
    void SaveState();
    void RestoreState();

    // addresses
    virtual bool GetLocal(wxSockAddress& addr_man) const;
    virtual bool GetPeer(wxSockAddress& addr_man) const;
    virtual bool SetLocal(const wxIPV4address& local);

    // base IO
    virtual bool  Close();
    void ShutdownOutput();
    wxSocketBase& Discard();
    wxSocketBase& Peek(void* buffer, std::uint32_t nbytes);
    wxSocketBase& Read(void* buffer, std::uint32_t nbytes);
    wxSocketBase& ReadMsg(void *buffer, std::uint32_t nbytes);
    wxSocketBase& Unread(const void *buffer, std::uint32_t nbytes);
    wxSocketBase& Write(const void *buffer, std::uint32_t nbytes);
    wxSocketBase& WriteMsg(const void *buffer, std::uint32_t nbytes);

    // all Wait() functions wait until their condition is satisfied or the
    // timeout expires; if seconds == -1 (default) then m_timeout value is used
    //
    // it is also possible to call InterruptWait() to cancel any current Wait()

    // wait for anything at all to happen with this socket
    bool Wait(long seconds = -1, long milliseconds = 0);

    // wait until we can read from or write to the socket without blocking
    // (notice that this does not mean that the operation will succeed but only
    // that it will return immediately)
    bool WaitForRead(long seconds = -1, long milliseconds = 0);
    bool WaitForWrite(long seconds = -1, long milliseconds = 0);

    // wait until the connection is terminated
    bool WaitForLost(long seconds = -1, long milliseconds = 0);

    void InterruptWait() { m_interrupt = true; }


    wxSocketFlags GetFlags() const { return m_flags; }
    void SetFlags(wxSocketFlags flags);
    virtual void SetTimeout(long seconds);
    long GetTimeout() const { return m_timeout; }

    bool GetOption(int level, int optname, void *optval, int *optlen);
    bool SetOption(int level, int optname, const void *optval, int optlen);
    std::uint32_t GetLastIOSize() const { return m_lcount; }
    std::uint32_t GetLastIOReadSize() const { return m_lcount_read; }
    std::uint32_t GetLastIOWriteSize() const { return m_lcount_write; }

    // event handling
    void *GetClientData() const { return m_clientData; }
    void SetClientData(void *data) { m_clientData = data; }
    void SetEventHandler(wxEvtHandler& handler, int id = wxID_ANY);
    void SetNotify(wxSocketEventFlags flags);
    void Notify(bool notify);

    // Get the underlying socket descriptor.
    wxSOCKET_T GetSocket() const;

    // initialize/shutdown the sockets (done automatically so there is no need
    // to call these functions usually)
    //
    // should always be called from the main thread only so one of the cases
    // where they should indeed be called explicitly is when the first wxSocket
    // object in the application is created in a different thread
    static bool Initialize();
    static void Shutdown();

    // check if wxSocket had been already initialized
    //
    // notice that this function should be only called from the main thread as
    // otherwise it is inherently unsafe because Initialize/Shutdown() may be
    // called concurrently with it in the main thread
    static bool IsInitialized();

    // Implementation from now on
    // --------------------------

    // do not use, should be private (called from wxSocketImpl only)
    void OnRequest(wxSocketNotify notify);

    // do not use, not documented nor supported
    bool IsNoWait() const { return ((m_flags & wxSOCKET_NOWAIT) != 0); }
    wxSocketType GetType() const { return m_type; }

    // Helper returning wxSOCKET_NONE if non-blocking sockets can be used, i.e.
    // the socket is being created in the main thread and the event loop is
    // running, or wxSOCKET_BLOCK otherwise.
    //
    // This is an internal function used only by wxWidgets itself, user code
    // should decide if it wants blocking sockets or not and use the
    // appropriate style instead of using it (but wxWidgets has to do it like
    // this for compatibility with the original network classes behaviour).
    static int GetBlockingFlagIfNeeded();

private:
    friend class wxSocketClient;
    friend class wxSocketServer;
    friend class wxDatagramSocket;

    // low level IO
    std::uint32_t DoRead(void* buffer, std::uint32_t nbytes);
    std::uint32_t DoWrite(const void *buffer, std::uint32_t nbytes);

    // wait until the given flags are set for this socket or the given timeout
    // (or m_timeout) expires
    //
    // notice that wxSOCKET_LOST_FLAG is always taken into account and the
    // function returns -1 if the connection was lost; otherwise it returns
    // true if any of the events specified by flags argument happened or false
    // if the timeout expired
    int DoWait(long timeout, wxSocketEventFlags flags);

    // a helper calling DoWait() using the same convention as the public
    // WaitForXXX() functions use, i.e. use our timeout if seconds == -1 or the
    // specified timeout otherwise
    int DoWait(long seconds, long milliseconds, wxSocketEventFlags flags);

    // another helper calling DoWait() using our m_timeout
    int DoWaitWithTimeout(wxSocketEventFlags flags)
    {
        return DoWait(m_timeout*1000, flags);
    }

    // pushback buffer
    void     Pushback(const void *buffer, std::uint32_t size);
    std::uint32_t GetPushback(void *buffer, std::uint32_t size, bool peek);

    // store the given error as the LastError()
    void SetError(wxSocketError error);

private:
    // socket
    std::unique_ptr<wxSocketImpl> m_impl;             // port-specific implementation
    wxSocketType  m_type;             // wxSocket type

    // state
    wxSocketFlags m_flags;            // wxSocket flags
    bool          m_connected;        // connected?
    bool          m_establishing;     // establishing connection?
    bool          m_reading;          // busy reading?
    bool          m_writing;          // busy writing?
    bool          m_closed;           // was the other end closed?
    std::uint32_t      m_lcount;           // last IO transaction size
    std::uint32_t      m_lcount_read;      // last IO transaction size of Read() direction.
    std::uint32_t      m_lcount_write;     // last IO transaction size of Write() direction.
    unsigned long m_timeout;          // IO timeout value in seconds
                                      // (TODO: remove, wxSocketImpl has it too)
    wxList        m_states;           // stack of states (TODO: remove!)
    bool          m_interrupt;        // interrupt ongoing wait operations?
    bool          m_beingDeleted;     // marked for delayed deletion?
    wxIPV4address m_localAddress;     // bind to local address?

    // pushback buffer
    void         *m_unread;           // pushback buffer
    std::uint32_t      m_unrd_size;        // pushback buffer size
    std::uint32_t      m_unrd_cur;         // pushback pointer (index into buffer)

    // events
    int           m_id;               // socket id
    wxEvtHandler *m_handler;          // event handler
    void         *m_clientData;       // client data for events
    bool          m_notify;           // notify events to users?
    wxSocketEventFlags  m_eventmask;  // which events to notify?
    wxSocketEventFlags  m_eventsgot;  // collects events received in OnRequest()


    friend class wxSocketReadGuard;
    friend class wxSocketWriteGuard;

    wxDECLARE_CLASS(wxSocketBase);
};


// --------------------------------------------------------------------------
// wxSocketServer
// --------------------------------------------------------------------------

class wxSocketServer : public wxSocketBase
{
public:
    wxSocketServer(const wxSockAddress& addr,
                   wxSocketFlags flags = wxSOCKET_NONE);

    wxSocketServer& operator=(wxSocketServer&&) = delete;

    wxSocketBase* Accept(bool wait = true);
    bool AcceptWith(wxSocketBase& socket, bool wait = true);

    bool WaitForAccept(long seconds = -1, long milliseconds = 0);

    wxDECLARE_CLASS(wxSocketServer);
};


// --------------------------------------------------------------------------
// wxSocketClient
// --------------------------------------------------------------------------

class wxSocketClient : public wxSocketBase
{
public:
    wxSocketClient(wxSocketFlags flags = wxSOCKET_NONE);

    wxSocketClient& operator=(wxSocketClient&&) = delete;

    virtual bool Connect(const wxSockAddress& addr, bool wait = true);
    bool Connect(const wxSockAddress& addr,
                 const wxSockAddress& local,
                 bool wait = true);

    bool WaitOnConnect(long seconds = -1, long milliseconds = 0);

    // Sets initial socket buffer sizes using the SO_SNDBUF and SO_RCVBUF
    // options before calling connect (either one can be -1 to leave it
    // unchanged)
    void SetInitialSocketBuffers(int recv, int send)
    {
        m_initialRecvBufferSize = recv;
        m_initialSendBufferSize = send;
    }

private:
    virtual bool DoConnect(const wxSockAddress& addr,
                           const wxSockAddress* local,
                           bool wait = true);

    // buffer sizes, -1 if unset and defaults should be used
    int m_initialRecvBufferSize;
    int m_initialSendBufferSize;

    wxDECLARE_CLASS(wxSocketClient);
};


// --------------------------------------------------------------------------
// wxDatagramSocket
// --------------------------------------------------------------------------

// WARNING: still in alpha stage

class wxDatagramSocket : public wxSocketBase
{
public:
    wxDatagramSocket(const wxSockAddress& addr,
                     wxSocketFlags flags = wxSOCKET_NONE);

    wxDatagramSocket& operator=(wxDatagramSocket&&) = delete;

    wxDatagramSocket& RecvFrom(wxSockAddress& addr,
                               void *buf,
                               std::uint32_t nBytes);
    wxDatagramSocket& SendTo(const wxSockAddress& addr,
                             const void* buf,
                             std::uint32_t nBytes);

    /* TODO:
       bool Connect(wxSockAddress& addr);
     */

private:
    wxDECLARE_CLASS(wxDatagramSocket);
};


// --------------------------------------------------------------------------
// wxSocketEvent
// --------------------------------------------------------------------------

class wxSocketEvent : public wxEvent
{
public:
    wxSocketEvent(int id = 0)
        : wxEvent(id, wxEVT_SOCKET)
    {
    }

    wxSocketEvent& operator=(const wxSocketEvent&) = delete;

    wxSocketNotify GetSocketEvent() const { return m_event; }
    wxSocketBase *GetSocket() const
        { return (wxSocketBase *) GetEventObject(); }
    void *GetClientData() const { return m_clientData; }

    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxSocketEvent>(*this); }
    wxEventCategory GetEventCategory() const override { return wxEVT_CATEGORY_SOCKET; }

public:
    wxSocketNotify  m_event; // FIXME: Default value?
    void           *m_clientData{nullptr};
};


typedef void (wxEvtHandler::*wxSocketEventFunction)(wxSocketEvent&);

#define wxSocketEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxSocketEventFunction, func)

#define EVT_SOCKET(id, func) \
    wx__DECLARE_EVT1(wxEVT_SOCKET, id, wxSocketEventHandler(func))

#endif // wxUSE_SOCKETS

#endif // _WX_SOCKET_H_

