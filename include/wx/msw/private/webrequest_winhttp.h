///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/webrequest_winhttp.h
// Purpose:     wxWebRequest WinHTTP implementation
// Author:      Tobias Taschner
// Created:     2018-10-17
// Copyright:   (c) 2018 wxWidgets development team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_WEBREQUEST_WINHTTP_H
#define _WX_MSW_WEBREQUEST_WINHTTP_H

#include "wx/private/webrequest.h"

#include "wx/msw/wrapwin.h"
#include <winhttp.h>
#include "wx/buffer.h"

class wxWebSessionWinHTTP;
class wxWebRequestWinHTTP;

class wxWebResponseWinHTTP : public wxWebResponseImpl
{
public:
    wxWebResponseWinHTTP(wxWebRequestWinHTTP& request);

    wxFileOffset GetContentLength() const override { return m_contentLength; }

    wxString GetURL() const override;

    wxString GetHeader(const wxString& name) const override;

    int GetStatus() const override;

    wxString GetStatusText() const override;

    bool ReadData();

    bool ReportAvailableData(DWORD dataLen);

private:
    HINTERNET m_requestHandle;
    wxFileOffset m_contentLength;

    wxWebResponseWinHTTP(const wxWebResponseWinHTTP&) = delete;
	wxWebResponseWinHTTP& operator=(const wxWebResponseWinHTTP&) = delete;
};

class wxWebAuthChallengeWinHTTP : public wxWebAuthChallengeImpl
{
public:
    wxWebAuthChallengeWinHTTP(wxWebAuthChallenge::Source source,
                              wxWebRequestWinHTTP& request);

    bool Init();

    void SetCredentials(const wxWebCredentials& cred) override;

private:
    wxWebRequestWinHTTP& m_request;
    DWORD m_target;
    DWORD m_selectedScheme;

    wxWebAuthChallengeWinHTTP(const wxWebAuthChallengeWinHTTP&) = delete;
	wxWebAuthChallengeWinHTTP& operator=(const wxWebAuthChallengeWinHTTP&) = delete;
};

class wxWebRequestWinHTTP : public wxWebRequestImpl
{
public:
    wxWebRequestWinHTTP(wxWebSession& session,
                        wxWebSessionWinHTTP& sessionImpl,
                        wxEvtHandler* handler,
                        const std::string& url,
                        int id);

    ~wxWebRequestWinHTTP();

    void Start() override;

    wxWebResponseImplPtr GetResponse() const override
        { return m_response; }

    wxWebAuthChallengeImplPtr GetAuthChallenge() const override
        { return m_authChallenge; }

    wxFileOffset GetBytesSent() const override { return m_dataWritten; }

    wxFileOffset GetBytesExpectedToSend() const override { return m_dataSize; }

    void HandleCallback(DWORD dwInternetStatus, LPVOID lpvStatusInformation,
        DWORD dwStatusInformationLength);

    HINTERNET GetHandle() const { return m_request; }

    wxWebRequestHandle GetNativeHandle() const override
    {
        return (wxWebRequestHandle)GetHandle();
    }

private:
    void DoCancel() override;

    wxWebSessionWinHTTP& m_sessionImpl;
    wxString m_url;
    HINTERNET m_connect;
    HINTERNET m_request;
    wxObjectDataPtr<wxWebResponseWinHTTP> m_response;
    wxObjectDataPtr<wxWebAuthChallengeWinHTTP> m_authChallenge;
    wxMemoryBuffer m_dataWriteBuffer;
    wxFileOffset m_dataWritten;

    void SendRequest();

    void WriteData();

    void CreateResponse();

    // Set the state to State_Failed with the error string including the
    // provided description of the operation and the error message for this
    // error code.
    void SetFailed(const wxString& operation, DWORD errorCode);

    void SetFailedWithLastError(const wxString& operation)
    {
        SetFailed(operation, ::GetLastError());
    }

    friend class wxWebAuthChallengeWinHTTP;

    wxWebRequestWinHTTP(const wxWebRequestWinHTTP&) = delete;
	wxWebRequestWinHTTP& operator=(const wxWebRequestWinHTTP&) = delete;
};

class wxWebSessionWinHTTP : public wxWebSessionImpl
{
public:
    wxWebSessionWinHTTP();

    ~wxWebSessionWinHTTP();

    static bool Initialize();

    wxWebRequestImplPtr
    CreateRequest(wxWebSession& session,
                  wxEvtHandler* handler,
                  const wxString& url,
                  int id) override;

    wxVersionInfo GetLibraryVersionInfo() override;

    HINTERNET GetHandle() const { return m_handle; }

    wxWebSessionHandle GetNativeHandle() const override
    {
        return (wxWebSessionHandle)GetHandle();
    }

private:
    HINTERNET m_handle{nullptr};

    bool Open();

    wxWebSessionWinHTTP(const wxWebSessionWinHTTP&) = delete;
	wxWebSessionWinHTTP& operator=(const wxWebSessionWinHTTP&) = delete;
};

class wxWebSessionFactoryWinHTTP : public wxWebSessionFactory
{
public:
    wxWebSessionImpl* Create() override
    {
        return new wxWebSessionWinHTTP();
    }

    bool Initialize() override
    {
        return wxWebSessionWinHTTP::Initialize();
    }
};

#endif // _WX_MSW_WEBREQUEST_WINHTTP_H
