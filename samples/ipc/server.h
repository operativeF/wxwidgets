/////////////////////////////////////////////////////////////////////////////
// Name:        server.h
// Purpose:     DDE sample: server
// Author:      Julian Smart
// Modified by:
// Created:     25/01/99
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "connection.h"

enum
{
    ID_START = 10000,
    ID_DISCONNECT,
    ID_ADVISE,
    ID_SERVERNAME,
};

// Define a new application
class MyServer;
class MyFrame;

class MyApp : public wxApp
{
public:
    bool OnInit() override;
    MyFrame *GetFrame() { return m_frame; }

protected:
    MyFrame *m_frame;
};

wxDECLARE_APP(MyApp);

// Define a new frame
class MyFrame : public wxFrame
{
public:
    MyFrame(wxFrame *frame, const wxString& title);

    void OnClose(wxCloseEvent& event);

    void UpdateUI();
    void Disconnect();

protected:
    wxButton* GetStart()  { return (wxButton*) FindWindow( ID_START ); }
    wxChoice* GetServername()  { return (wxChoice*) FindWindow( ID_SERVERNAME ); }
    wxButton* GetDisconnect()  { return (wxButton*) FindWindow( ID_DISCONNECT ); }
    wxButton* GetAdvise()  { return (wxButton*) FindWindow( ID_ADVISE ); }


    MyServer *m_server;

    void OnStart( wxCommandEvent &event );
    void OnServerName( wxCommandEvent &event );
    void OnDisconnect( wxCommandEvent &event );
    void OnAdvise( wxCommandEvent &event );

    wxDECLARE_EVENT_TABLE();
};

class MyConnection : public MyConnectionBase
{
public:
    bool OnExecute(const wxString& topic, const void *data, size_t size, wxIPCFormat format) override;
    const void *OnRequest(const wxString& topic, const wxString& item, size_t *size, wxIPCFormat format) override;
    bool OnPoke(const wxString& topic, const wxString& item, const void *data, size_t size, wxIPCFormat format) override;
    bool OnStartAdvise(const wxString& topic, const wxString& item) override;
    bool OnStopAdvise(const wxString& topic, const wxString& item) override;
    bool DoAdvise(const wxString& item, const void *data, size_t size, wxIPCFormat format) override;
    bool OnDisconnect() override;

    // topic for which we advise the client or empty if none
    wxString m_advise;

protected:
    // the data returned by last OnRequest(): we keep it in this buffer to
    // ensure that the pointer we return from OnRequest() stays valid
    wxCharBuffer m_requestData;
};

class MyServer : public wxServer
{
public:
    MyServer();
    virtual ~MyServer();

    void Disconnect();
    bool IsConnected() { return m_connection != NULL; }
    MyConnection *GetConnection() { return m_connection; }

    void Advise();
    bool CanAdvise() { return m_connection && !m_connection->m_advise.empty(); }

    wxConnectionBase *OnAcceptConnection(const wxString& topic) override;

protected:
    MyConnection *m_connection;
};

