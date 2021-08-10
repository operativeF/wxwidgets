/////////////////////////////////////////////////////////////////////////////
// Name:        wx/protocol/ftp.h
// Purpose:     FTP protocol
// Author:      Vadim Zeitlin
// Modified by: Mark Johnson, wxWindows@mj10777.de
//              20000917 : RmDir, GetLastResult, GetList
// Created:     07/07/1997
// Copyright:   (c) 1997, 1998 Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_FTP_H__
#define __WX_FTP_H__

#include "wx/defs.h"

#if wxUSE_PROTOCOL_FTP

#include "wx/sckaddr.h"
#include "wx/protocol/protocol.h"
#include "wx/url.h"

class WXDLLIMPEXP_NET wxFTP : public wxProtocol
{
public:
    enum TransferMode
    {
        NONE,       // not set by user explicitly
        ASCII,
        BINARY
    };

    wxFTP();
    ~wxFTP() override;

	wxFTP(const wxFTP&) = delete;
	wxFTP& operator=(const wxFTP&) = delete;

    // Connecting and disconnecting
    bool Connect(const wxSockAddress& addr, bool wait = true) override;
    bool Connect(const std::string& host) override { return Connect(host, 0); }
    virtual bool Connect(const std::string& host, unsigned short port);

    // disconnect
    bool Close() override;

    // Parameters set up

    // set transfer mode now
    void SetPassive(bool pasv) { m_bPassive = pasv; }
    bool SetBinary() { return SetTransferMode(BINARY); }
    bool SetAscii() { return SetTransferMode(ASCII); }
    bool SetTransferMode(TransferMode mode);

    // Generic FTP interface

    // FTP doesn't know the MIME type of the last downloaded/uploaded file
    std::string GetContentType() const override { return {}; }

    // the last FTP server reply
    const std::string& GetLastResult() const { return m_lastResult; }

    // send any FTP command (should be full FTP command line but without
    // trailing "\r\n") and return its return code
    char SendCommand(const std::string& command);

    // check that the command returned the given code
    bool CheckCommand(const std::string& command, char expectedReturn)
    {
        // SendCommand() does updates m_lastError
        return SendCommand(command) == expectedReturn;
    }

    // Filesystem commands
    bool ChDir(const std::string& dir);
    bool MkDir(const std::string& dir);
    bool RmDir(const std::string& dir);
    std::string Pwd();
    bool Rename(const std::string& src, const std::string& dst);
    bool RmFile(const std::string& path);

    // Get the size of a file in the current dir.
    // this function tries its best to deliver the size in bytes using BINARY
    // (the SIZE command reports different sizes depending on whether
    // type is set to ASCII or BINARY)
    // returns -1 if file is non-existent or size could not be found
    int GetFileSize(const std::string& fileName);

       // Check to see if a file exists in the current dir
    bool FileExists(const std::string& fileName);

    // Download methods
    bool Abort() override;

    wxInputStream *GetInputStream(const std::string& path) override;
    virtual wxOutputStream *GetOutputStream(const std::string& path);

    // Directory listing

    // get the list of full filenames, the format is fixed: one file name per
    // line
    bool GetFilesList(std::vector<std::string>& files,
                      const std::string& wildcard = {})
    {
        return GetList(files, wildcard, false);
    }

    // get a directory list in server dependent format - this can be shown
    // directly to the user
    bool GetDirList(std::vector<std::string>& files,
                    const std::string& wildcard = {})
    {
        return GetList(files, wildcard, true);
    }

    // equivalent to either GetFilesList() (default) or GetDirList()
    bool GetList(std::vector<std::string>& files,
                 const std::string& wildcard = {},
                 bool details = false);

protected:
    // this executes a simple ftp command with the given argument and returns
    // true if it its return code starts with '2'
    bool DoSimpleCommand(const char* command,
                         const std::string& arg = {});

    // get the server reply, return the first character of the reply code,
    // '1'..'5' for normal FTP replies, 0 (*not* '0') if an error occurred
    char GetResult();

    // check that the result is equal to expected value
    bool CheckResult(char ch) { return GetResult() == ch; }

    // return the socket to be used, Passive/Active versions are used only by
    // GetPort()
    wxSocketBase *GetPort();
    wxSocketBase *GetPassivePort();
    wxSocketBase *GetActivePort();

    // helper for GetPort()
    std::string GetPortCmdArgument(const wxIPV4address& Local, const wxIPV4address& New);

    // accept connection from server in active mode, returns the same socket as
    // passed in passive mode
    wxSocketBase *AcceptIfActive(wxSocketBase *sock);


    // internal variables:

    std::string        m_lastResult;

    // true if there is an FTP transfer going on
    bool            m_streaming{false};

    // although this should be set to ASCII by default according to STD9,
    // we will use BINARY transfer mode by default for backwards compatibility
    TransferMode    m_currentTransfermode{NONE};

    bool            m_bPassive{true};

    // following is true when  a read or write times out, we then assume
    // the connection is dead and abort. we avoid additional delays this way
    bool            m_bEncounteredError{false};


    friend class wxInputFTPStream;
    friend class wxOutputFTPStream;

public:
	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
    DECLARE_PROTOCOL(wxFTP)
};

// the trace mask used by assorted wxLogTrace() in ftp code, do
// wxLog::AddTraceMask(FTP_TRACE_MASK) to see them in output
#define FTP_TRACE_MASK wxT("ftp")

#endif // wxUSE_PROTOCOL_FTP

#endif // __WX_FTP_H__
