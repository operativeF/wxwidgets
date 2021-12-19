/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/filesys.cpp
// Purpose:     wxFileSystem class - interface for opening files
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILESYSTEM

#include "wx/filesys.h"
#include "wx/module.h"
#include "wx/sysopt.h"
#include "wx/wfstream.h"
#include "wx/mimetype.h"
#include "wx/filename.h"
#include "wx/private/fileback.h"
#include "wx/utils.h"

import Utils.Chars;
import Utils.Strings;

// ----------------------------------------------------------------------------
// wxFSFile
// ----------------------------------------------------------------------------

const std::string& wxFSFile::GetMimeType() const
{
    if ( m_MimeType.empty() && !m_Location.empty() )
    {
        const_cast<wxFSFile *>(this)->m_MimeType =
            wxFileSystemHandler::GetMimeTypeFromExt(m_Location);
    }

    return m_MimeType;
}

// ----------------------------------------------------------------------------
// wxFileSystemHandler
// ----------------------------------------------------------------------------


/* static */
std::string wxFileSystemHandler::GetMimeTypeFromExt(const std::string& location)
{
    std::string ext, mime;
    std::string loc = GetRightLocation(location);
    int l = loc.length();
    int l2 = l;

    for (int i = l-1; i >= 0; i--)
    {
        wxChar c = loc[(unsigned int) i];

        if ( c == wxT('#') )
            l2 = i + 1;
        if ( c == '.' )
        {
            ext = loc.substr(l2-i-1);
            break;
        }
        if ( (c == '/') || (c == wxT('\\')) || (c == ':') )
            return {};
    }

#if wxUSE_MIMETYPE
    static bool s_MinimalMimeEnsured = false;

    // Don't use mime types manager if the application doesn't need it and it would be
    // cause an unacceptable delay, especially on startup.
#if wxUSE_SYSTEM_OPTIONS
    if ( !wxSystemOptions::GetOptionInt("filesys.no-mimetypesmanager") )
#endif
    {
        if (!s_MinimalMimeEnsured)
        {
            static const wxFileTypeInfo fallbacks[] =
            {
                wxFileTypeInfo("image/jpeg",
                    wxString{},
                    wxString{},
                    wxString{"JPEG image (from fallback)"},
                    wxString{"jpg"},
                    wxString{"jpeg"},
                    wxString{"JPG"},
                    wxString{"JPEG"},
                    nullptr),
                wxFileTypeInfo("image/gif",
                    wxString{},
                    wxString{},
                    wxString{"GIF image (from fallback)"},
                    wxString{"gif"},
                    wxString{"GIF"},
                    nullptr),
                wxFileTypeInfo("image/png",
                    wxString{},
                    wxString{},
                    wxString{"PNG image (from fallback)"},
                    wxString{"png"},
                    wxString{"PNG"},
                    nullptr),
                wxFileTypeInfo("image/bmp",
                    wxString{},
                    wxString{},
                    wxString{"windows bitmap image (from fallback)"},
                    wxString{"bmp"},
                    wxString{"BMP"},
                    nullptr),
                wxFileTypeInfo("text/html",
                    wxString{},
                    wxString{},
                    wxString{"HTML document (from fallback)"},
                    wxString{"htm"},
                    wxString{"html"},
                    wxString{"HTM"},
                    wxString{"HTML"},
                    nullptr),
                // must terminate the table with this!
                wxFileTypeInfo()
            };
            wxTheMimeTypesManager->AddFallbacks(fallbacks);
            s_MinimalMimeEnsured = true;
        }

        wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension(ext);
        if ( !ft || ft->GetMimeType(&mime) )
        {
            mime.clear();
        }

        delete ft;

        return mime;
    }
#endif // wxUSE_MIMETYPE

    // Without wxUSE_MIMETYPE, recognize just a few hardcoded special cases.
    if ( wx::utils::IsSameAsNoCase("htm", ext) || wx::utils::IsSameAsNoCase("html", ext) )
        return "text/html";
    if ( wx::utils::IsSameAsNoCase("jpg", ext) || wx::utils::IsSameAsNoCase("jpeg", ext) )
        return "image/jpeg";
    if ( wx::utils::IsSameAsNoCase("gif", ext) )
        return "image/gif";
    if ( wx::utils::IsSameAsNoCase("png", ext) )
        return "image/png";
    if ( wx::utils::IsSameAsNoCase("bmp", ext) )
        return "image/bmp";

    return {};
}



/* static */
std::string wxFileSystemHandler::GetProtocol(const std::string& location)
{
    std::string s;
    int i, l = location.length();
    bool fnd = false;

    for (i = l-1; (i >= 0) && ((location[i] != wxT('#')) || (!fnd)); i--) {
        if ((location[i] == ':') && (i != 1 /*win: C:\path*/)) fnd = true;
    }
    if (!fnd) return "file";
    for (++i; (i < l) && (location[i] != ':'); i++) s += location[i];
    return s;
}


/* static */
std::string wxFileSystemHandler::GetLeftLocation(const std::string& location)
{
    if(location.empty())
        return {};

    bool fnd = false;

    for (int i = location.length()-1; i >= 0; i--) {
        if ((location[i] == ':') && (i != 1 /*win: C:\path*/))
        {
            fnd = true;
        }
        else if (fnd && (location[i] == wxT('#')))
        {
            return location.substr(0, i);
        }
    }

    return {};
}

/* static */
std::string wxFileSystemHandler::GetRightLocation(const std::string& location)
{
    int i, len = location.length();
    for (i = len-1; i >= 0; i--)
    {
        if (location[i] == wxT('#'))
            len = i;
        if (location[i] != ':')
            continue;

        // C: on Windows
        if (i == 1)
            continue;
        if (i >= 2 && wx::utils::isAlpha(location[i-1]) && location[i-2] == '/')
            continue;

        // Could be the protocol
        break;
    }
    if (i == 0) return {};

    const static std::string protocol("file:");
    if (i < (int)protocol.length() - 1 || location.compare(0, i + 1, protocol))
        return location.substr(i + 1, len - i - 1);

    const int s = ++i; // Start position
    // Check if there are three '/'s after "file:"
    const int end = std::min(len, s + 3);
    while (i < end && location[i] == '/')
        i++;
    if (i == s + 2) // Host is specified, e.g. "file://host/path"
        return location.substr(s, len - s);
    if (i > s)
    {
        // Remove the last '/' if it is preceding "C:/...".
        // Otherwise, keep it.
        if (i + 1 >= len || location[i + 1] != ':')
            i--;
        else if (i + 4 < len)
        {
            // Check if ':' was encoded
            const static std::string colonLower("%3a");
            const static std::string colonUpper("%3A");
            std::string sub = location.substr(i + 1, 3);
            if (sub == colonLower || sub == colonUpper)
                i--;
        }
    }
    return location.substr(i, len - i);
}

/* static */
std::string wxFileSystemHandler::GetAnchor(const std::string& location)
{
    const int l = location.length();

    for (int i = l-1; i >= 0; i--) {
        wxChar c;
        c = location[i];
        if (c == wxT('#'))
            return location.substr(l-i-1);
        else if ((c == '/') || (c == wxT('\\')) || (c == ':'))
            break;
    }
    return {};
}


std::string wxFileSystemHandler::FindFirst([[maybe_unused]] const std::string& spec,
                                        [[maybe_unused]] unsigned int flags)
{
    return {};
}

std::string wxFileSystemHandler::FindNext()
{
    return {};
}

//--------------------------------------------------------------------------------
// wxLocalFSHandler
//--------------------------------------------------------------------------------

bool wxLocalFSHandler::CanOpen(const std::string& location)
{
    return GetProtocol(location) == "file";
}

wxFSFile* wxLocalFSHandler::OpenFile([[maybe_unused]] wxFileSystem& fs, const std::string& location)
{
    // location has Unix path separators
    std::string right = GetRightLocation(location);
    wxFileName fn = wxFileName::URLToFileName(right);
    std::string fullpath = ms_root + fn.GetFullPath();

    if (!wxFileExists(fullpath))
        return nullptr;

    // we need to check whether we can really read from this file, otherwise
    // wxFSFile is not going to work
#if wxUSE_FFILE
    wxFFileInputStream *is = new wxFFileInputStream(fullpath);
#elif wxUSE_FILE
    wxFileInputStream *is = new wxFileInputStream(fullpath);
#else
#error One of wxUSE_FILE or wxUSE_FFILE must be set to 1 for wxFSHandler to work
#endif
    if ( !is->IsOk() )
    {
        delete is;
        return nullptr;
    }

    return new wxFSFile(is,
                        location,
                        {},
                        GetAnchor(location)
#if wxUSE_DATETIME
                        ,wxDateTime(wxFileModificationTime(fullpath))
#endif // wxUSE_DATETIME
                        );
}

// TODO: Lambda
std::string wxLocalFSHandler::FindFirst(const std::string& spec, unsigned int flags)
{
    wxFileName fn = wxFileName::URLToFileName(GetRightLocation(spec));
    std::string found = wxFindFirstFile(ms_root + fn.GetFullPath(), flags);
    if ( found.empty() )
        return found;
    return wxFileSystem::FileNameToURL(found);
}

// TODO: Lambda
std::string wxLocalFSHandler::FindNext()
{
    std::string found = wxFindNextFile();
    if ( found.empty() )
        return found;
    return wxFileSystem::FileNameToURL(found);
}

//-----------------------------------------------------------------------------
// wxFileSystem
//-----------------------------------------------------------------------------

wxFileSystem::~wxFileSystem()
{
    WX_CLEAR_HASH_MAP(wxFSHandlerHash, m_LocalHandlers)
}


static std::string MakeCorrectPath(const std::string& path)
{
    std::string p(path);
    std::string r;
    int i, j;

    int cnt = p.length();
    
    for (i = 0; i < cnt; i++)
    {
      if (p[i] == '\\')
        p[i] = '/'; // Want to be windows-safe
    }

    if (p.substr(0, 2) == "./") { p = p.substr(2); cnt -= 2; }

    if (cnt < 3) return p;

    r += p[0] + p[1];

    // skip trailing ../.., if any
    for (i = 2; i < cnt && (p[i] == '/' || p[i] == '.'); i++) r += p[i];

    // remove back references: translate dir1/../dir2 to dir2
    for (; i < cnt; i++)
    {
        r += p[i];
        if (p[i] == '/' && p[i-1] == '.' && p[i-2] == '.')
        {
            for (j = r.length() - 2; j >= 0 && r[j] != '/' && r[j] != ':'; j--) {}
            if (j >= 0 && r[j] != ':')
            {
                for (j = j - 1; j >= 0 && r[j] != '/' && r[j] != ':'; j--) {}
                r.erase(j + 1);
            }
        }
    }

    for (; i < cnt; i++) r += p[i];

    return r;
}


void wxFileSystem::ChangePathTo(const std::string& location, bool is_dir)
{

    m_Path = MakeCorrectPath(location);

    if (is_dir)
    {
        if (!m_Path.empty() && m_Path.back() != '/' && m_Path.back() != ':')
            m_Path += '/';
    }
    else
    {
        int i, pathpos = -1;
        for (i = m_Path.length()-1; i >= 0; i--)
        {
            if (m_Path[(unsigned int) i] == '/')
            {
                if ((i > 1) && (m_Path[(unsigned int) (i-1)] == '/') && (m_Path[(unsigned int) (i-2)] == ':'))
                {
                    i -= 2;
                    continue;
                }
                else
                {
                    pathpos = i;
                    break;
                }
            }
            else if (m_Path[(unsigned int) i] == ':') {
                pathpos = i;
                break;
            }
        }
        if (pathpos == -1)
        {
            for (i = 0; i < (int) m_Path.length(); i++)
            {
                if (m_Path[(unsigned int) i] == ':')
                {
                    m_Path.erase(i+1);
                    break;
                }
            }
            if (i == (int) m_Path.length())
                m_Path.clear();
        }
        else
        {
            m_Path.erase(pathpos+1);
        }
    }
}



wxFileSystemHandler *wxFileSystem::MakeLocal(wxFileSystemHandler *h)
{
    wxClassInfo *classinfo = h->wxGetClassInfo();

    if (classinfo->IsDynamic())
    {
        wxFileSystemHandler*& local = m_LocalHandlers[classinfo];
        if (!local)
            local = (wxFileSystemHandler*)classinfo->CreateObject();
        return local;
    }
    else
    {
        return h;
    }
}



wxFSFile* wxFileSystem::OpenFile(const std::string& location, unsigned int flags)
{
    if ((flags & wxFS_READ) == 0)
        return nullptr;

    std::string loc = MakeCorrectPath(location);
    wxFSFile *s = nullptr;
    wxList::compatibility_iterator node;

    auto ln = loc.length();
    wxChar meta = 0;
    for (std::size_t i{}; i != ln; i++)
    {
        switch ( loc[i] )
        {
            case '/' : case ':' : case '#' :
                meta = loc[i];
                break;
        }
        if (meta != 0) break;
    }
    m_LastName.clear();

    // try relative paths first :
    if (meta != ':' && !m_Path.empty())
    {
        const std::string fullloc = m_Path + loc;
        node = m_Handlers.GetFirst();
        while (node)
        {
            wxFileSystemHandler *h = (wxFileSystemHandler*) node -> GetData();
            if (h->CanOpen(fullloc))
            {
                s = MakeLocal(h)->OpenFile(*this, fullloc);
                if (s) { m_LastName = fullloc; break; }
            }
            node = node->GetNext();
        }
    }

    // if failed, try absolute paths :
    if (s == nullptr)
    {
        node = m_Handlers.GetFirst();
        while (node)
        {
            wxFileSystemHandler *h = (wxFileSystemHandler*) node->GetData();
            if (h->CanOpen(loc))
            {
                s = MakeLocal(h)->OpenFile(*this, loc);
                if (s) { m_LastName = loc; break; }
            }
            node = node->GetNext();
        }
    }

    if (s && (flags & wxFS_SEEKABLE) != 0 && !s->GetStream()->IsSeekable())
    {
        wxBackedInputStream *stream;
        stream = new wxBackedInputStream(s->DetachStream());
        stream->FindLength();
        s->SetStream(stream);
    }

    return (s);
}



std::string wxFileSystem::FindFirst(const std::string& spec, unsigned int flags)
{
    wxList::compatibility_iterator node;
    std::string spec2(spec);

    m_FindFileHandler = nullptr;

    for (int i = spec2.length()-1; i >= 0; i--)
        if (spec2[(unsigned int) i] == wxT('\\')) spec2[i] = '/'; // Want to be windows-safe

    node = m_Handlers.GetFirst();
    while (node)
    {
        wxFileSystemHandler *h = (wxFileSystemHandler*) node -> GetData();
        if (h -> CanOpen(m_Path + spec2))
        {
            m_FindFileHandler = MakeLocal(h);
            return m_FindFileHandler -> FindFirst(m_Path + spec2, flags);
        }
        node = node->GetNext();
    }

    node = m_Handlers.GetFirst();
    while (node)
    {
        wxFileSystemHandler *h = (wxFileSystemHandler*) node -> GetData();
        if (h -> CanOpen(spec2))
        {
            m_FindFileHandler = MakeLocal(h);
            return m_FindFileHandler -> FindFirst(spec2, flags);
        }
        node = node->GetNext();
    }

    return {};
}



std::string wxFileSystem::FindNext()
{
    if (m_FindFileHandler == nullptr) return {};
    else return m_FindFileHandler -> FindNext();
}

bool wxFileSystem::FindFileInPath(std::string *pStr,
                                  const std::string& path,
                                  const std::string& basename)
{
    // we assume that it's not empty
    wxCHECK_MSG( !basename.empty(), false,
                "empty file name in wxFileSystem::FindFileInPath");

    std::string name;
    // skip path separator in the beginning of the file name if present
    if ( wxIsPathSeparator(basename[0u]) )
        name = basename.substr(1);
    else
        name = basename;

    wxStringTokenizer tokenizer(path, wxPATH_SEP);
    while ( tokenizer.HasMoreTokens() )
    {
        std::string strFile = tokenizer.GetNextToken();
        if ( !wxEndsWithPathSeparator(strFile) )
            strFile += wxFILE_SEP_PATH;
        strFile += name;

        wxFSFile *file = OpenFile(strFile);
        if ( file )
        {
            delete file;
            *pStr = strFile;
            return true;
        }
    }

    return false;
}

void wxFileSystem::AddHandler(wxFileSystemHandler *handler)
{
    // prepend the handler to the beginning of the list because handlers added
    // last should have the highest priority to allow overriding them
    m_Handlers.Insert((size_t)0, handler);
}

wxFileSystemHandler* wxFileSystem::RemoveHandler(wxFileSystemHandler *handler)
{
    // if handler has already been removed (or deleted)
    // we return NULL. This is by design in case
    // CleanUpHandlers() is called before RemoveHandler
    // is called, as we cannot control the order
    // which modules are unloaded
    if (!m_Handlers.DeleteObject(handler))
        return nullptr;

    return handler;
}


bool wxFileSystem::HasHandlerForPath(const std::string &location)
{
    for ( wxList::compatibility_iterator node = m_Handlers.GetFirst();
           node; node = node->GetNext() )
    {
        wxFileSystemHandler *h = (wxFileSystemHandler*) node->GetData();
        if (h->CanOpen(location))
            return true;
    }

    return false;
}

void wxFileSystem::CleanUpHandlers()
{
    WX_CLEAR_LIST(wxList, m_Handlers);
}

// Returns the native path for a file URL
wxFileName wxFileSystem::URLToFileName(const std::string& url)
{
    return wxFileName::URLToFileName( url );
}

// Returns the file URL for a native path
std::string wxFileSystem::FileNameToURL(const wxFileName& filename)
{
    return wxFileName::FileNameToURL( filename );
}


///// Module:

class wxFileSystemModule : public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxFileSystemModule);

    public:
        bool OnInit() override
        {
            m_handler = new wxLocalFSHandler;
            wxFileSystem::AddHandler(m_handler);
            return true;
        }
        void OnExit() override
        {
            delete wxFileSystem::RemoveHandler(m_handler);

            wxFileSystem::CleanUpHandlers();
        }

    private:
        wxFileSystemHandler* m_handler{nullptr};

};

wxIMPLEMENT_DYNAMIC_CLASS(wxFileSystemModule, wxModule);

//// wxFSInputStream

wxFSInputStream::wxFSInputStream(const std::string& filename, unsigned int flags)
{
    wxFileSystem fs;
    m_file = fs.OpenFile(filename, flags | wxFS_READ);

    if ( m_file )
    {
        wxInputStream* const stream = m_file->GetStream();
        if ( stream )
        {
            // Notice that we pass the stream by reference: it shouldn't be
            // deleted by us as it's owned by m_file already.
            InitParentStream(*stream);
        }
    }
}

wxFSInputStream::~wxFSInputStream()
{
    delete m_file;
}

#endif
  // wxUSE_FILESYSTEM
