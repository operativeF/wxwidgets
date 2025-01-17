/////////////////////////////////////////////////////////////////////////////
// Name:        webviewfshandler.h
// Purpose:     Custom webview handler for virtual file system
// Author:      Nick Matthews
// Copyright:   (c) 2012 Steven Lamerton
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// Based on webviewarchivehandler.h file by Steven Lamerton

#ifndef _WX_WEBVIEW_FS_HANDLER_H_
#define _WX_WEBVIEW_FS_HANDLER_H_

#if wxUSE_WEBVIEW

class wxFSFile;
class wxFileSystem;

#include "wx/webview.h"

//Loads from uris such as scheme:example.html

class wxWebViewFSHandler : public wxWebViewHandler
{
public:
    wxWebViewFSHandler(const wxString& scheme);
    ~wxWebViewFSHandler();
    wxFSFile* GetFile(const wxString &uri) override;
private:
    wxFileSystem* m_fileSystem;
};

#endif // wxUSE_WEBVIEW

#endif // _WX_WEBVIEW_FS_HANDLER_H_
