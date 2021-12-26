/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/dde.h
// Purpose:     DDE class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DDE_H_
#define _WX_DDE_H_

import WX.Cmn.IpcBase;

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

class wxDDEServer;
class wxDDEClient;

class wxDDEConnection : public wxConnectionBase
{
public:
  wxDDEConnection(void *buffer, size_t size); // use external buffer
  wxDDEConnection() = default; // use internal buffer
  ~wxDDEConnection();

	wxDDEConnection& operator=(wxDDEConnection&&) = delete;

  // implement base class pure virtual methods
  const void *Request(const wxString& item,
                              size_t *size = nullptr,
                              wxIPCFormat format = wxIPC_TEXT) override;
  bool StartAdvise(const wxString& item) override;
  bool StopAdvise(const wxString& item) override;
  bool Disconnect() override;

protected:
  bool DoExecute(const void *data, size_t size, wxIPCFormat format) override;
  bool DoPoke(const wxString& item, const void *data, size_t size,
                      wxIPCFormat format) override;
  bool DoAdvise(const wxString& item, const void *data, size_t size,
                        wxIPCFormat format) override;

public:
  wxString      m_topicName;
  wxDDEServer*  m_server{nullptr};
  wxDDEClient*  m_client{nullptr};

  WXHCONV       m_hConv{nullptr};
  const void*   m_sendingData{nullptr};
  std::size_t   m_dataSize{};

  wxDECLARE_DYNAMIC_CLASS(wxDDEConnection);
};

class wxDDEServer : public wxServerBase
{
public:
    wxDDEServer();
    [[maybe_unused]] bool Create(const wxString& server_name) override;
    ~wxDDEServer();

    wxConnectionBase *OnAcceptConnection(const wxString& topic) override;

    // Find/delete wxDDEConnection corresponding to the HCONV
    wxDDEConnection *FindConnection(WXHCONV conv);
    bool DeleteConnection(WXHCONV conv);
    wxString& GetServiceName() { return m_serviceName; }
    const wxString& GetServiceName() const { return m_serviceName; }

    wxDDEConnectionList& GetConnections() { return m_connections; }
    const wxDDEConnectionList& GetConnections() const { return m_connections; }

protected:
    int       m_lastError;
    wxString  m_serviceName;
    wxDDEConnectionList m_connections;

    wxDECLARE_DYNAMIC_CLASS(wxDDEServer);
};

class wxDDEClient: public wxClientBase
{
public:
    wxDDEClient();
    ~wxDDEClient();

    bool ValidHost(const wxString& host) override;

    // Call this to make a connection. Returns NULL if cannot.
    wxConnectionBase *MakeConnection(const wxString& host,
                                             const wxString& server,
                                             const wxString& topic) override;

    // Tailor this to return own connection.
    wxConnectionBase *OnMakeConnection() override;

    // Find/delete wxDDEConnection corresponding to the HCONV
    wxDDEConnection *FindConnection(WXHCONV conv);
    bool DeleteConnection(WXHCONV conv);

    wxDDEConnectionList& GetConnections() { return m_connections; }
    const wxDDEConnectionList& GetConnections() const { return m_connections; }

protected:
    int       m_lastError;
    wxDDEConnectionList m_connections;

    wxDECLARE_DYNAMIC_CLASS(wxDDEClient);
};

void wxDDEInitialize();
void wxDDECleanUp();

#endif // _WX_DDE_H_
