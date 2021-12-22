/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fs_mem.cpp
// Purpose:     in-memory file system
// Author:      Vaclav Slavik
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILESYSTEM && wxUSE_STREAMS

#include "wx/fs_mem.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/wxcrtvararg.h"

#if wxUSE_GUI
    import WX.Image;
#endif // wxUSE_GUI

import WX.Cmn.MemStream;

// represents a file entry in wxMemoryFS
class wxMemoryFSFile
{
public:
    wxMemoryFSFile(const void *data, size_t len, const std::string& mime)
        : m_Data(new char[len]),
          m_Len(len),
          m_MimeType(mime)
    {
        memcpy(m_Data, data, len);
        InitTime();
    }

    wxMemoryFSFile(const wxMemoryOutputStream& stream, const std::string& mime)
        : m_Len(stream.GetSize()),
          m_Data(new char[m_Len]),
          m_MimeType(mime)
    {
        stream.CopyTo(m_Data, m_Len);
        InitTime();
    }

    virtual ~wxMemoryFSFile()
    {
        delete[] m_Data;
    }

    wxMemoryFSFile(const wxMemoryFSFile&) = delete;
	wxMemoryFSFile& operator=(const wxMemoryFSFile&) = delete;

    char *m_Data;
    size_t m_Len;
    std::string m_MimeType;
#if wxUSE_DATETIME
    wxDateTime m_Time;
#endif // wxUSE_DATETIME

private:
    void InitTime()
    {
#if wxUSE_DATETIME
        m_Time = wxDateTime::Now();
#endif // wxUSE_DATETIME
    }
};

#if wxUSE_BASE


//--------------------------------------------------------------------------------
// wxMemoryFSHandler
//--------------------------------------------------------------------------------

wxMemoryFSHandlerBase::~wxMemoryFSHandlerBase()
{
    // as only one copy of FS handler is supposed to exist, we may silently
    // delete static data here. (There is no way how to remove FS handler from
    // wxFileSystem other than releasing _all_ handlers.)
    WX_CLEAR_HASH_MAP(wxMemoryFSHash, m_Hash);
}

bool wxMemoryFSHandlerBase::CanOpen(const std::string& location)
{
    return GetProtocol(location) == "memory";
}

wxFSFile * wxMemoryFSHandlerBase::OpenFile([[maybe_unused]] wxFileSystem& fs,
                                           const std::string& location)
{
    wxMemoryFSHash::const_iterator i = m_Hash.find(GetRightLocation(location));
    if ( i == m_Hash.end() )
        return nullptr;

    const wxMemoryFSFile * const obj = i->second;

    return new wxFSFile
               (
                    new wxMemoryInputStream(obj->m_Data, obj->m_Len),
                    location,
                    obj->m_MimeType,
                    GetAnchor(location)
#if wxUSE_DATETIME
                    , obj->m_Time
#endif // wxUSE_DATETIME
               );
}

std::string wxMemoryFSHandlerBase::FindFirst(const std::string& url, unsigned int flags)
{
    // Make sure to reset the find iterator, so that calling FindNext() doesn't
    // reuse its value from the last search that could well be invalid.
    m_findIter = m_Hash.end();

    if ( (flags & wxDIR) && !(flags & wxFILE) )
    {
        // we only store files, not directories, so we don't risk finding
        // anything
        return {};
    }

    const std::string spec = GetRightLocation(url);
    if ( spec.find_first_of("?*") == std::string::npos )
    {
        // simple case: there are no wildcard characters so we can return
        // either 0 or 1 results and we can find the potential match quickly
        return m_Hash.count(spec) ? url : std::string();
    }
    //else: deal with wildcards in FindNext()

    m_findArgument = spec;
    m_findIter = m_Hash.begin();

    return FindNext();
}

std::string wxMemoryFSHandlerBase::FindNext()
{
    while ( m_findIter != m_Hash.end() )
    {
        const wxString& path = m_findIter->first;

        // advance m_findIter first as we need to do it anyhow, whether it
        // matches or not
        ++m_findIter;

        if ( path.Matches(m_findArgument) )
            return "memory:" + path.ToStdString();
    }

    return {};
}

bool wxMemoryFSHandlerBase::CheckDoesntExist(const std::string& filename)
{
    if ( m_Hash.count(filename) )
    {
        wxLogError(_("Memory VFS already contains file '%s'!"), filename);
        return false;
    }

    return true;
}


/*static*/
void wxMemoryFSHandlerBase::AddFileWithMimeType(const std::string& filename,
                                                const std::string& textdata,
                                                const std::string& mimetype)
{
    const wxCharBuffer buf(textdata.c_str());

    AddFileWithMimeType(filename, buf.data(), buf.length(), mimetype);
}


/*static*/
void wxMemoryFSHandlerBase::AddFileWithMimeType(const std::string& filename,
                                                const void *binarydata, size_t size,
                                                const std::string& mimetype)
{
    if ( !CheckDoesntExist(filename) )
        return;

    m_Hash[filename] = new wxMemoryFSFile(binarydata, size, mimetype);
}

/*static*/
void wxMemoryFSHandlerBase::AddFile(const std::string& filename,
                                    const std::string& textdata)
{
    AddFileWithMimeType(filename, textdata, {});
}


/*static*/
void wxMemoryFSHandlerBase::AddFile(const std::string& filename,
                                    const void *binarydata, size_t size)
{
    AddFileWithMimeType(filename, binarydata, size, {});
}



/*static*/ void wxMemoryFSHandlerBase::RemoveFile(const std::string& filename)
{
    wxMemoryFSHash::iterator i = m_Hash.find(filename);
    if ( i == m_Hash.end() )
    {
        wxLogError(_("Trying to remove file '%s' from memory VFS, "
                     "but it is not loaded!"),
                   filename);
        return;
    }

    delete i->second;
    m_Hash.erase(i);
}

#endif // wxUSE_BASE

#if wxUSE_GUI

#if wxUSE_IMAGE
/*static*/ void
wxMemoryFSHandler::AddFile(const std::string& filename,
                           const wxImage& image,
                           wxBitmapType type)
{
    if ( !CheckDoesntExist(filename) )
        return;

    wxMemoryOutputStream mems;
    if ( image.IsOk() && image.SaveFile(mems, type) )
    {
        m_Hash[filename] = new wxMemoryFSFile
                               (
                                    mems,
                                    wxImage::FindHandler(type)->GetMimeType()
                               );
    }
    else
    {
        wxLogError(_("Failed to store image '%s' to memory VFS!"), filename);
    }
}

/*static*/ void
wxMemoryFSHandler::AddFile(const std::string& filename,
                           const wxBitmap& bitmap,
                           wxBitmapType type)
{
    wxImage img = bitmap.ConvertToImage();
    AddFile(filename, img, type);
}

#endif // wxUSE_IMAGE

#endif // wxUSE_GUI


#endif // wxUSE_FILESYSTEM && wxUSE_FS_ZIP
