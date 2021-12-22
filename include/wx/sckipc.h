/////////////////////////////////////////////////////////////////////////////
// Name:        wx/sckipc.h
// Purpose:     Interprocess communication implementation (wxSocket version)
// Author:      Julian Smart
// Modified by: Guilhem Lavaux (big rewrite) May 1997, 1998
//              Guillermo Rodriguez (updated for wxSocket v2) Jan 2000
//                                  (callbacks deprecated)    Mar 2000
// Created:     1993
// Copyright:   (c) Julian Smart 1993
//              (c) Guilhem Lavaux 1997, 1998
//              (c) 2000 Guillermo Rodriguez <guille@iies.es>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SCKIPC_H
#define _WX_SCKIPC_H

#if wxUSE_SOCKETS && wxUSE_IPC

#include "wx/ipcbase.h"
#include "wx/socket.h"
#include "wx/sckstrm.h"

import WX.Cmn.DataStream;

/*
 * Mini-DDE implementation

   Most transactions involve a topic name and an item name (choose these
   as befits your application).

   A client can:

   - ask the server to execute commands (data) associated with a topic
   - request data from server by topic and item
   - poke data into the server
   - ask the server to start an advice loop on topic/item
   - ask the server to stop an advice loop

   A server can:

   - respond to execute, request, poke and advice start/stop
   - send advise data to client

   Note that this limits the server in the ways it can send data to the
   client, i.e. it can't send unsolicited information.
 *
 */

class wxTCPServer;
class wxTCPClient;

class wxIPCSocketStreams;

class wxTCPConnection : public wxConnectionBase
{
public:
    wxTCPConnection() = default;
    wxTCPConnection(void *buffer, size_t size)
        : wxConnectionBase(buffer, size)
    {
    }

    ~wxTCPConnection();

   wxTCPConnection& operator=(wxTCPConnection&&) = delete;

    // implement base class pure virtual methods
    const void *Request(const wxString& item,
                                size_t *size = nullptr,
                                wxIPCFormat format = wxIPC_TEXT) override;
    bool StartAdvise(const wxString& item) override;
    bool StopAdvise(const wxString& item) override;
    bool Disconnect() override;

    // Will be used in the future to enable the compression but does nothing
    // for now.
    void Compress(bool on);


protected:
    bool DoExecute(const void *data, size_t size, wxIPCFormat format) override;
    bool DoPoke(const wxString& item, const void *data, size_t size,
                        wxIPCFormat format) override;
    bool DoAdvise(const wxString& item, const void *data, size_t size,
                          wxIPCFormat format) override;


    // notice that all the members below are only initialized once the
    // connection is made, i.e. in MakeConnection() for the client objects and
    // after OnAcceptConnection() in the server ones

    // the underlying socket (wxSocketClient for IPC client and wxSocketServer
    // for IPC server)
    wxSocketBase *m_sock {nullptr};

    // various streams that we use
    wxIPCSocketStreams *m_streams {nullptr};

    // the topic of this connection
    wxString m_topic;

private:
    friend class wxTCPServer;
    friend class wxTCPClient;
    friend class wxTCPEventHandler;

    wxDECLARE_DYNAMIC_CLASS(wxTCPConnection);
};

class wxTCPServer : public wxServerBase
{
public:
    ~wxTCPServer();

   wxTCPServer& operator=(wxTCPServer&&) = delete;

    // Returns false on error (e.g. port number is already in use)
    bool Create(const wxString& serverName) override;

    wxConnectionBase *OnAcceptConnection(const wxString& topic) override;

protected:
    wxSocketServer *m_server{nullptr};

#ifdef __UNIX_LIKE__
    // the name of the file associated to the Unix domain socket, may be empty
    wxString m_filename;
#endif // __UNIX_LIKE__

    wxDECLARE_DYNAMIC_CLASS(wxTCPServer);
};

class wxTCPClient : public wxClientBase
{
public:
    wxTCPClient() = default;

    bool ValidHost(const wxString& host) override;

    // Call this to make a connection. Returns NULL if cannot.
    wxConnectionBase *MakeConnection(const wxString& host,
                                             const wxString& server,
                                             const wxString& topic) override;

    // Callbacks to CLIENT - override at will
    wxConnectionBase *OnMakeConnection() override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxTCPClient);
};

#endif // wxUSE_SOCKETS && wxUSE_IPC

#endif // _WX_SCKIPC_H
