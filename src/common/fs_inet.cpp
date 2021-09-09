/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fs_inet.cpp
// Purpose:     HTTP and FTP file system
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"


#if !wxUSE_SOCKETS
    #undef wxUSE_FS_INET
    #define wxUSE_FS_INET 0
#endif

#if wxUSE_FILESYSTEM && wxUSE_FS_INET

#include "wx/module.h"
#include "wx/wfstream.h"
#include "wx/url.h"
#include "wx/filesys.h"
#include "wx/fs_inet.h"
#include "wx/stringutils.h"


namespace
{

// This stream deletes the file when destroyed
class wxTemporaryFileInputStream : public wxFileInputStream
{
public:
    explicit wxTemporaryFileInputStream(const wxString& filename) :
        wxFileInputStream(filename), m_filename(filename) {}

    ~wxTemporaryFileInputStream()
    {
        // NB: copied from wxFileInputStream dtor, we need to do it before
        //     wxRemoveFile
        if (m_file_destroy)
        {
            delete m_file;
            m_file_destroy = false;
        }
        wxRemoveFile(m_filename);
    }

protected:
    wxString m_filename;
};

std::string StripProtocolAnchor(const std::string& location)
{
    std::string myloc(wx::utils::BeforeLast(location, '#'));
    if (myloc.empty()) myloc = wx::utils::AfterFirst(location, ':');
    else myloc = wx::utils::AfterFirst(myloc, ':');

    // fix malformed url:
    if (!wx::utils::IsSameAsCase(myloc.substr(2), "//"))
    {
        if (myloc.front() != '/') myloc = "//" + myloc;
        else myloc = "/" + myloc;
    }
    if (myloc.substr(2).find('/') == std::string::npos) myloc += '/';

    return myloc;
}

class wxFileSystemInternetModule : public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxFileSystemInternetModule);

    public:
        bool OnInit() override
        {
            m_handler = new wxInternetFSHandler;
            wxFileSystem::AddHandler(m_handler);
            return true;
        }

        void OnExit() override
        {
            delete wxFileSystem::RemoveHandler(m_handler);
        }

    private:
        wxFileSystemHandler* m_handler{nullptr};
};

wxIMPLEMENT_DYNAMIC_CLASS(wxFileSystemInternetModule, wxModule);


} // namespace anonymous

bool wxInternetFSHandler::CanOpen(const wxString& location)
{
#if wxUSE_URL
    std::string p = GetProtocol(location);
    if ((p == wxT("http")) || (p == wxT("ftp")))
    {
        wxURL url(p + ":" + StripProtocolAnchor(location));
        return (url.GetError() == wxURL_NOERR);
    }
#endif
    return false;
}


wxFSFile* wxInternetFSHandler::OpenFile(wxFileSystem& WXUNUSED(fs),
                                        const wxString& location)
{
#if !wxUSE_URL
    return NULL;
#else
    std::string right =
        GetProtocol(location) + ":" + StripProtocolAnchor(location);

    wxURL url(right);
    if (url.GetError() == wxURL_NOERR)
    {
        wxInputStream *s = url.GetInputStream();
        if (s)
        {
            wxString tmpfile =
                wxFileName::CreateTempFileName(wxT("wxhtml"));

            {   // now copy streams content to temporary file:
                wxFileOutputStream sout(tmpfile);
                s->Read(sout);
            }
            delete s;

            // Content-Type header, as defined by the RFC 2045, has the form of
            // "type/subtype" optionally followed by (multiple) "; parameter"
            // and we need just the MIME type here.
            const wxString& content = url.GetProtocol().GetContentType();
            wxString mimetype = content.BeforeFirst(';');
            mimetype.Trim();

            return new wxFSFile(new wxTemporaryFileInputStream(tmpfile),
                                right,
                                mimetype,
                                GetAnchor(location)
#if wxUSE_DATETIME
                                , wxDateTime::Now()
#endif // wxUSE_DATETIME
                        );
        }
    }

    return nullptr; // incorrect URL
#endif
}

#endif // wxUSE_FILESYSTEM && wxUSE_FS_INET
