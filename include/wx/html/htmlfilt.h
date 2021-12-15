/////////////////////////////////////////////////////////////////////////////
// Name:        wx/html/htmlfilt.h
// Purpose:     filters
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HTMLFILT_H_
#define _WX_HTMLFILT_H_

#if wxUSE_HTML

#include "wx/filesys.h"


//--------------------------------------------------------------------------------
// wxHtmlFilter
//                  This class is input filter. It can "translate" files
//                  in non-HTML format to HTML format
//                  interface to access certain
//                  kinds of files (HTPP, FTP, local, tar.gz etc..)
//--------------------------------------------------------------------------------

class wxHtmlFilter : public wxObject
{
    wxDECLARE_ABSTRACT_CLASS(wxHtmlFilter);
public:
    // returns true if this filter is able to open&read given file
    virtual bool CanRead(const wxFSFile& file) const = 0;

    // Reads given file and returns HTML document.
    // Returns empty string if opening failed
    virtual std::string ReadFile(const wxFSFile& file) const = 0;
};



//--------------------------------------------------------------------------------
// wxHtmlFilterPlainText
//                  This filter is used as default filter if no other can
//                  be used (= unknown type of file). It is used by
//                  wxHtmlWindow itself.
//--------------------------------------------------------------------------------


class wxHtmlFilterPlainText : public wxHtmlFilter
{
public:
    bool CanRead(const wxFSFile& file) const override;
    std::string ReadFile(const wxFSFile& file) const override;
};

//--------------------------------------------------------------------------------
// wxHtmlFilterHTML
//          filter for text/html
//--------------------------------------------------------------------------------

class wxHtmlFilterHTML : public wxHtmlFilter
{
public:
    bool CanRead(const wxFSFile& file) const override;
    std::string ReadFile(const wxFSFile& file) const override;
};

#endif // wxUSE_HTML

#endif // _WX_HTMLFILT_H_

