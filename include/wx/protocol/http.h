/////////////////////////////////////////////////////////////////////////////
// Name:        wx/protocol/http.h
// Purpose:     HTTP protocol
// Author:      Guilhem Lavaux
// Modified by: Simo Virokannas (authentication, Dec 2005)
// Created:     August 1997
// Copyright:   (c) 1997, 1998 Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
#ifndef _WX_HTTP_H
#define _WX_HTTP_H

#include "wx/defs.h"

#if wxUSE_PROTOCOL_HTTP

#include "wx/hashmap.h"
#include "wx/protocol/protocol.h"
#include "wx/buffer.h"

class WXDLLIMPEXP_NET wxHTTP : public wxProtocol
{
public:
    wxHTTP() = default;
    ~wxHTTP() override;

    wxHTTP(const wxHTTP&) = delete;
	wxHTTP& operator=(const wxHTTP&) = delete;

    virtual bool Connect(const std::string& host, unsigned short port);
    bool Connect(const std::string& host) override { return Connect(host, 0); }
    bool Connect(const wxSockAddress& addr, bool wait = true) override;
    bool Abort() override;

    wxInputStream *GetInputStream(const std::string& path) override;

    std::string GetContentType() const override;
    std::string GetHeader(const std::string& header) const;
    int GetResponse() const { return m_http_response; }

    void SetMethod(const std::string& method) { m_method = method; }
    void SetHeader(const std::string& header, const std::string& h_data);
    bool SetPostText(const std::string& contentType,
                     const std::string& data,
                     const wxMBConv& conv = wxConvUTF8);
    bool SetPostBuffer(const std::string& contentType, const wxMemoryBuffer& data);
    void SetProxyMode(bool on);

    /* Cookies */
    std::string GetCookie(const std::string& cookie) const;
    bool HasCookies() const { return m_cookies.size() > 0; }

protected:
    using wxHeaderIterator = wxStringToStringHashMap::iterator;
    using wxHeaderConstIterator = wxStringToStringHashMap::const_iterator;
    using wxCookieIterator = wxStringToStringHashMap::iterator;
    using wxCookieConstIterator = wxStringToStringHashMap::const_iterator;

    bool BuildRequest(const std::string& path, const std::string& method);
    void SendHeaders();
    bool ParseHeaders();

    std::string GenerateAuthString(const std::string& user, const std::string& pass) const;

    // find the header in m_headers
    wxHeaderIterator FindHeader(const std::string& header);
    wxHeaderConstIterator FindHeader(const std::string& header) const;
    wxCookieIterator FindCookie(const std::string& cookie);
    wxCookieConstIterator FindCookie(const std::string& cookie) const;

    // deletes the header value strings
    void ClearHeaders();
    void ClearCookies();

    // internal variables:

    std::string m_method;
    std::string m_contentType;

    wxStringToStringHashMap m_cookies;

    wxStringToStringHashMap m_headers;
    bool m_read{false};
    bool m_proxy_mode{false};
    wxSockAddress *m_addr{nullptr};
    wxMemoryBuffer m_postBuffer;
    int m_http_response{0};

    wxDECLARE_DYNAMIC_CLASS(wxHTTP);
    DECLARE_PROTOCOL(wxHTTP)
};

#endif // wxUSE_PROTOCOL_HTTP

#endif // _WX_HTTP_H

