///////////////////////////////////////////////////////////////////////////////
// Name:        wx/webrequest_curl.h
// Purpose:     wxWebRequest implementation using libcurl
// Author:      Tobias Taschner
// Created:     2018-10-25
// Copyright:   (c) 2018 wxWidgets development team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WEBREQUEST_CURL_H
#define _WX_WEBREQUEST_CURL_H

#if wxUSE_WEBREQUEST_CURL

#include "wx/private/webrequest.h"

#include "wx/thread.h"
#include "wx/timer.h"
#include "wx/hashmap.h"

#include "curl/curl.h"

import <vector>;

class wxWebRequestCURL;
class wxWebResponseCURL;
class wxWebSessionCURL;
class SocketPoller;

class wxWebAuthChallengeCURL : public wxWebAuthChallengeImpl
{
public:
    wxWebAuthChallengeCURL(wxWebAuthChallenge::Source source,
                           wxWebRequestCURL& request);

    void SetCredentials(const wxWebCredentials& cred) override;

private:
    wxWebRequestCURL& m_request;
};

class wxWebRequestCURL : public wxWebRequestImpl
{
public:
    wxWebRequestCURL(wxWebSession& session,
                     wxWebSessionCURL& sessionImpl,
                     wxEvtHandler* handler,
                     const wxString& url,
                     int id);

    ~wxWebRequestCURL();

    void Start() override;

    wxWebResponseImplPtr GetResponse() const override
        { return m_response; }

    wxWebAuthChallengeImplPtr GetAuthChallenge() const override
        { return m_authChallenge; }

    wxFileOffset GetBytesSent() const override;

    wxFileOffset GetBytesExpectedToSend() const override;

    CURL* GetHandle() const { return m_handle; }

    wxWebRequestHandle GetNativeHandle() const override
    {
        return (wxWebRequestHandle)GetHandle();
    }

    bool StartRequest();

    void HandleCompletion();

    wxString GetError() const;

    // Method called from libcurl callback
    size_t CURLOnRead(char* buffer, size_t size);

private:
    void DoCancel() override;

    wxWebSessionCURL& m_sessionImpl;

    CURL* m_handle;
    char m_errorBuffer[CURL_ERROR_SIZE];
    struct curl_slist *m_headerList;
    wxObjectDataPtr<wxWebResponseCURL> m_response;
    wxObjectDataPtr<wxWebAuthChallengeCURL> m_authChallenge;
    wxFileOffset m_bytesSent;

    void DestroyHeaderList();
};

class wxWebResponseCURL : public wxWebResponseImpl
{
public:
    explicit wxWebResponseCURL(wxWebRequestCURL& request);

    wxFileOffset GetContentLength() const override;

    wxString GetURL() const override;

    wxString GetHeader(const wxString& name) const override;

    int GetStatus() const override;

    wxString GetStatusText() const override { return m_statusText; }


    // Methods called from libcurl callbacks
    size_t CURLOnWrite(void *buffer, size_t size);
    size_t CURLOnHeader(const char* buffer, size_t size);
    int CURLOnProgress(curl_off_t);

private:
    wxWebRequestHeaderMap m_headers;
    wxString m_statusText;
    wxFileOffset m_knownDownloadSize;

    CURL* GetHandle() const
    { return static_cast<wxWebRequestCURL&>(m_request).GetHandle(); }

    wxWebResponseCURL(const wxWebResponseCURL&) = delete;
	wxWebResponseCURL& operator=(const wxWebResponseCURL&) = delete;
};

class wxWebSessionCURL : public wxWebSessionImpl, public wxEvtHandler
{
public:
    wxWebSessionCURL();

    ~wxWebSessionCURL();

    wxWebRequestImplPtr
    CreateRequest(wxWebSession& session,
                  wxEvtHandler* handler,
                  const wxString& url,
                  int id = wxID_ANY) override;

    wxVersionInfo GetLibraryVersionInfo() override;

    wxWebSessionHandle GetNativeHandle() const override
    {
        return (wxWebSessionHandle)m_handle;
    }

    bool StartRequest(wxWebRequestCURL& request);

    void CancelRequest(wxWebRequestCURL* request);

    void RequestHasTerminated(wxWebRequestCURL* request);

    static bool CurlRuntimeAtLeastVersion(unsigned int, unsigned int,
                                          unsigned int);

private:
    static int TimerCallback(CURLM*, long, void*);
    static int SocketCallback(CURL*, curl_socket_t, int, void*, void*);

    void ProcessTimerCallback(long);
    void TimeoutNotification(wxTimerEvent&);
    void ProcessTimeoutNotification();
    void ProcessSocketCallback(CURL*, curl_socket_t, int);
    void ProcessSocketPollerResult(wxThreadEvent&);
    void CheckForCompletedTransfers();
    void FailRequest(CURL*, const wxString&);
    void StopActiveTransfer(CURL*);
    void RemoveActiveSocket(CURL*);

    WX_DECLARE_HASH_MAP(CURL*, wxWebRequestCURL*, wxPointerHash, \
                        wxPointerEqual, TransferSet);

    WX_DECLARE_HASH_MAP(CURL*, curl_socket_t, wxPointerHash, \
                        wxPointerEqual, CurlSocketMap);

    TransferSet m_activeTransfers;
    CurlSocketMap m_activeSockets;

    SocketPoller* m_socketPoller;
    wxTimer m_timeoutTimer;
    CURLM* m_handle;

    static int ms_activeSessions;
    static unsigned int ms_runtimeVersion;

    wxWebSessionCURL(const wxWebSessionCURL&) = delete;
	wxWebSessionCURL& operator=(const wxWebSessionCURL&) = delete;
};

class wxWebSessionFactoryCURL : public wxWebSessionFactory
{
public:
    wxWebSessionImpl* Create() override
    { return new wxWebSessionCURL(); }
};

#endif // wxUSE_WEBREQUEST_CURL

#endif
