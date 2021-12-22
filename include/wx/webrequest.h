///////////////////////////////////////////////////////////////////////////////
// Name:        wx/webrequest.h
// Purpose:     wxWebRequest base classes
// Author:      Tobias Taschner
// Created:     2018-10-17
// Copyright:   (c) 2018 wxWidgets development team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WEBREQUEST_H
#define _WX_WEBREQUEST_H

#include "wx/secretstore.h"

// Note that this class is intentionally defined outside of wxUSE_WEBREQUEST
// test as it's also used in wxCredentialEntryDialog and can be made available
// even if wxWebRequest itself is disabled.
class wxWebCredentials
{
public:
    wxWebCredentials(const wxString& user = {},
                     const wxSecretValue& password = wxSecretValue())
        : m_user(user), m_password(password)
    {
    }

    const wxString& GetUser() const { return m_user; }
    const wxSecretValue& GetPassword() const { return m_password; }

private:
    wxString m_user;
    wxSecretValue m_password;
};

#if wxUSE_WEBREQUEST

#include "wx/event.h"
#include "wx/object.h"
import WX.Cmn.Stream;

import WX.Utils.VersionInfo;

class wxWebResponse;
class wxWebSession;
class wxWebSessionFactory;

typedef struct wxWebRequestHandleOpaque* wxWebRequestHandle;
typedef struct wxWebSessionHandleOpaque* wxWebSessionHandle;

class wxWebAuthChallengeImpl;
class wxWebRequestImpl;
class wxWebResponseImpl;
class wxWebSessionImpl;

using wxWebAuthChallengeImplPtr = wxObjectDataPtr<wxWebAuthChallengeImpl>;
using wxWebRequestImplPtr = wxObjectDataPtr<wxWebRequestImpl>;
using wxWebResponseImplPtr = wxObjectDataPtr<wxWebResponseImpl>;
using wxWebSessionImplPtr = wxObjectDataPtr<wxWebSessionImpl>;

class wxWebAuthChallenge
{
public:
    enum Source
    {
        Source_Server,
        Source_Proxy
    };

    wxWebAuthChallenge();
    wxWebAuthChallenge(const wxWebAuthChallenge& other);
    wxWebAuthChallenge& operator=(const wxWebAuthChallenge& other);
    ~wxWebAuthChallenge();

    bool IsOk() const { return m_impl.get() != nullptr; }

    Source GetSource() const;

    void SetCredentials(const wxWebCredentials& cred);

private:
    // Ctor is used by wxWebRequest only.
    friend class wxWebRequest;
    explicit wxWebAuthChallenge(const wxWebAuthChallengeImplPtr& impl);

    wxWebAuthChallengeImplPtr m_impl;
};

class wxWebResponse
{
public:
    wxWebResponse();
    wxWebResponse(const wxWebResponse& other);
    wxWebResponse& operator=(const wxWebResponse& other);
    ~wxWebResponse();

    bool IsOk() const { return m_impl.get() != nullptr; }

    wxFileOffset GetContentLength() const;

    wxString GetURL() const;

    wxString GetHeader(const wxString& name) const;

    wxString GetMimeType() const;

    int GetStatus() const;

    wxString GetStatusText() const;

    wxInputStream* GetStream() const;

    wxString GetSuggestedFileName() const;

    wxString AsString() const;

    wxString GetDataFile() const;

protected:
    // Ctor is used by wxWebRequest and wxWebRequestImpl.
    friend class wxWebRequest;
    friend class wxWebRequestImpl;
    explicit wxWebResponse(const wxWebResponseImplPtr& impl);

    wxWebResponseImplPtr m_impl;
};

class wxWebRequest
{
public:
    enum State
    {
        State_Idle,
        State_Unauthorized,
        State_Active,
        State_Completed,
        State_Failed,
        State_Cancelled
    };

    enum Storage
    {
        Storage_Memory,
        Storage_File,
        Storage_None
    };

    wxWebRequest();
    wxWebRequest(const wxWebRequest& other);
    wxWebRequest& operator=(const wxWebRequest& other);
    ~wxWebRequest();

    bool IsOk() const { return m_impl.get() != nullptr; }

    void SetHeader(const wxString& name, const wxString& value);

    void SetMethod(const wxString& method);

    void SetData(const wxString& text, const wxString& contentType, const wxMBConv& conv = wxConvUTF8);

    bool SetData(wxInputStream* dataStream, const wxString& contentType, wxFileOffset dataSize = wxInvalidOffset);

    void SetStorage(Storage storage);

    Storage GetStorage() const;

    void Start();

    void Cancel();

    wxWebResponse GetResponse() const;

    wxWebAuthChallenge GetAuthChallenge() const;

    int GetId() const;

    wxWebSession& GetSession() const;

    State GetState() const;

    wxFileOffset GetBytesSent() const;

    wxFileOffset GetBytesExpectedToSend() const;

    wxFileOffset GetBytesReceived() const;

    wxFileOffset GetBytesExpectedToReceive() const;

    wxWebRequestHandle GetNativeHandle() const;

    void DisablePeerVerify(bool disable = true);

    bool IsPeerVerifyDisabled() const;

private:
    // Ctor is only used by wxWebSession.
    friend class wxWebSession;
    explicit wxWebRequest(const wxWebRequestImplPtr& impl);

    wxWebRequestImplPtr m_impl;
};

extern const char wxWebSessionBackendWinHTTP[];
extern const char wxWebSessionBackendURLSession[];
extern const char wxWebSessionBackendCURL[];

class wxWebSession
{
public:
    // Default ctor creates an invalid session object, only IsOpened() can be
    // called on it.
    wxWebSession();

    wxWebSession(const wxWebSession& other);
    wxWebSession& operator=(const wxWebSession& other);
    ~wxWebSession();

    // Objects of this class can't be created directly, use the following
    // factory functions to get access to them.
    static wxWebSession& GetDefault();

    static wxWebSession New(const std::string& backend = {});

    // Can be used to check if the given backend is available without actually
    // creating a session using it.
    static bool IsBackendAvailable(const wxString& backend);

    wxWebRequest
    CreateRequest(wxEvtHandler* handler, const wxString& url, int id = wxID_ANY);

    wxVersionInfo GetLibraryVersionInfo();

    void AddCommonHeader(const wxString& name, const wxString& value);

    void SetTempDir(const wxString& dir);
    wxString GetTempDir() const;

    bool IsOpened() const;

    void Close();

    wxWebSessionHandle GetNativeHandle() const;

private:
    static void RegisterFactory(const wxString& backend,
                                wxWebSessionFactory* factory);

    static void InitFactoryMap();

    explicit wxWebSession(const wxWebSessionImplPtr& impl);

    wxWebSessionImplPtr m_impl;
};

class wxWebRequestEvent : public wxEvent
{
public:
    wxWebRequestEvent(wxEventType type = wxEVT_NULL,
                      int id = wxID_ANY,
                      wxWebRequest::State state = wxWebRequest::State_Idle,
                      const wxWebResponse& response = wxWebResponse(),
                      const wxString& errorDesc = {})
        : wxEvent(id, type),
        m_state(state), m_response(response),
        m_errorDescription(errorDesc)
    { }

    wxWebRequest::State GetState() const { return m_state; }

    const wxWebResponse& GetResponse() const { return m_response; }

    const wxString& GetErrorDescription() const { return m_errorDescription; }

    const wxString& GetDataFile() const { return m_dataFile; }

    void SetDataFile(const wxString& dataFile) { m_dataFile = dataFile; }

    const void* GetDataBuffer() const { return m_dataBuf.GetData(); }

    size_t GetDataSize() const { return m_dataBuf.GetDataLen(); }

    void SetDataBuffer(const wxMemoryBuffer& dataBuf) { m_dataBuf = dataBuf; }

    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxWebRequestEvent>(*this); }

private:
    wxWebRequest::State m_state;
    const wxWebResponse m_response; // may be invalid
    wxString m_dataFile;
    wxMemoryBuffer m_dataBuf;
    wxString m_errorDescription;
};

wxDECLARE_EVENT(wxEVT_WEBREQUEST_STATE, wxWebRequestEvent);
wxDECLARE_EVENT(wxEVT_WEBREQUEST_DATA, wxWebRequestEvent);

#endif // wxUSE_WEBREQUEST

#endif // _WX_WEBREQUEST_H
