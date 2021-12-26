/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fs_arc.cpp
// Purpose:     wxArchive file system
// Author:      Vaclav Slavik, Mike Wetherell
// Copyright:   (c) 1999 Vaclav Slavik, (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/hashmap.h"
#include "wx/intl.h"
#include "wx/private/fileback.h"

#include <fmt/core.h>

module WX.FileSys.Arc;

import WX.Cmn.ArchStream;
import WX.File.Filename;

//---------------------------------------------------------------------------
// wxArchiveFSCacheDataImpl
//
// Holds the catalog of an archive file, and if it is being read from a
// non-seekable stream, a copy of its backing file.
//
// This class is actually the reference counted implementation for the
// wxArchiveFSCacheData class below. It was done that way to allow sharing
// between instances of wxFileSystem, though that's a feature not used in this
// version.
//---------------------------------------------------------------------------

WX_DECLARE_STRING_HASH_MAP(wxArchiveEntry*, wxArchiveFSEntryHash);

struct wxArchiveFSEntry
{
    wxArchiveEntry *entry;
    wxArchiveFSEntry *next;
};

class wxArchiveFSCacheDataImpl
{
public:
    wxArchiveFSCacheDataImpl(const wxArchiveClassFactory& factory,
                             const wxBackingFile& backer);
    wxArchiveFSCacheDataImpl(const wxArchiveClassFactory& factory,
                             wxInputStream *stream);

    ~wxArchiveFSCacheDataImpl();

    void Release() { if (--m_refcount == 0) delete this; }
    wxArchiveFSCacheDataImpl *AddRef() { m_refcount++; return this; }

    wxArchiveEntry *Get(const std::string& name);
    wxInputStream *NewStream() const;

    wxArchiveFSEntry *GetNext(wxArchiveFSEntry *fse);

private:
    wxArchiveFSEntry *AddToCache(wxArchiveEntry *entry);
    void CloseStreams();

    wxArchiveFSEntryHash m_hash;
    wxBackingFile m_backer;

    wxArchiveFSEntry *m_begin;
    wxArchiveFSEntry **m_endptr;

    wxInputStream *m_stream;
    wxArchiveInputStream *m_archive;

    int m_refcount;
};

wxArchiveFSCacheDataImpl::wxArchiveFSCacheDataImpl(
        const wxArchiveClassFactory& factory,
        const wxBackingFile& backer)
 :  m_refcount(1),
    m_begin(nullptr),
    m_endptr(&m_begin),
    m_backer(backer),
    m_stream(new wxBackedInputStream(backer)),
    m_archive(factory.NewStream(*m_stream))
{
}

wxArchiveFSCacheDataImpl::wxArchiveFSCacheDataImpl(
        const wxArchiveClassFactory& factory,
        wxInputStream *stream)
 :  m_refcount(1),
    m_begin(nullptr),
    m_endptr(&m_begin),
    m_stream(stream),
    m_archive(factory.NewStream(*m_stream))
{
}

wxArchiveFSCacheDataImpl::~wxArchiveFSCacheDataImpl()
{
    WX_CLEAR_HASH_MAP(wxArchiveFSEntryHash, m_hash);

    wxArchiveFSEntry *entry = m_begin;

    while (entry)
    {
        wxArchiveFSEntry *next = entry->next;
        delete entry;
        entry = next;
    }

    CloseStreams();
}

wxArchiveFSEntry *wxArchiveFSCacheDataImpl::AddToCache(wxArchiveEntry *entry)
{
    m_hash[entry->GetName(wxPATH_UNIX)] = entry;
    wxArchiveFSEntry *fse = new wxArchiveFSEntry;
    *m_endptr = fse;
    (*m_endptr)->entry = entry;
    (*m_endptr)->next = nullptr;
    m_endptr = &(*m_endptr)->next;
    return fse;
}

void wxArchiveFSCacheDataImpl::CloseStreams()
{
    wxDELETE(m_archive);
    wxDELETE(m_stream);
}

wxArchiveEntry *wxArchiveFSCacheDataImpl::Get(const std::string& name)
{
    auto it = m_hash.find(name);

    if (it != m_hash.end())
        return it->second;

    if (!m_archive)
        return nullptr;

    wxArchiveEntry *entry;

    while ((entry = m_archive->GetNextEntry()) != nullptr)
    {
        AddToCache(entry);

        if (entry->GetName(wxPATH_UNIX) == name)
            return entry;
    }

    CloseStreams();

    return nullptr;
}

wxInputStream* wxArchiveFSCacheDataImpl::NewStream() const
{
    if (m_backer)
        return new wxBackedInputStream(m_backer);
    else
        return nullptr;
}

wxArchiveFSEntry *wxArchiveFSCacheDataImpl::GetNext(wxArchiveFSEntry *fse)
{
    wxArchiveFSEntry *next = fse ? fse->next : m_begin;

    if (!next && m_archive)
    {
        wxArchiveEntry *entry = m_archive->GetNextEntry();

        if (entry)
            next = AddToCache(entry);
        else
            CloseStreams();
    }

    return next;
}

//---------------------------------------------------------------------------
// wxArchiveFSCacheData
//
// This is the inteface for wxArchiveFSCacheDataImpl above. Holds the catalog
// of an archive file, and if it is being read from a non-seekable stream, a
// copy of its backing file.
//---------------------------------------------------------------------------

class wxArchiveFSCacheData
{
public:
    wxArchiveFSCacheData()  = default;
    wxArchiveFSCacheData(const wxArchiveClassFactory& factory,
                         const wxBackingFile& backer);
    wxArchiveFSCacheData(const wxArchiveClassFactory& factory,
                         wxInputStream *stream);

    wxArchiveFSCacheData(const wxArchiveFSCacheData& data);
    wxArchiveFSCacheData& operator=(const wxArchiveFSCacheData& data);

    ~wxArchiveFSCacheData() { if (m_impl) m_impl->Release(); }

    wxArchiveEntry *Get(const std::string& name) { return m_impl->Get(name); }
    wxInputStream *NewStream() const { return m_impl->NewStream(); }
    wxArchiveFSEntry *GetNext(wxArchiveFSEntry *fse)
        { return m_impl->GetNext(fse); }

private:
    wxArchiveFSCacheDataImpl *m_impl{nullptr};
};

wxArchiveFSCacheData::wxArchiveFSCacheData(
        const wxArchiveClassFactory& factory,
        const wxBackingFile& backer)
  : m_impl(new wxArchiveFSCacheDataImpl(factory, backer))
{
}

wxArchiveFSCacheData::wxArchiveFSCacheData(
        const wxArchiveClassFactory& factory,
        wxInputStream *stream)
  : m_impl(new wxArchiveFSCacheDataImpl(factory, stream))
{
}

wxArchiveFSCacheData::wxArchiveFSCacheData(const wxArchiveFSCacheData& data)
  : m_impl(data.m_impl ? data.m_impl->AddRef() : nullptr)
{
}

wxArchiveFSCacheData& wxArchiveFSCacheData::operator=(
        const wxArchiveFSCacheData& data)
{
    if (data.m_impl != m_impl)
    {
        if (m_impl)
            m_impl->Release();

        m_impl = data.m_impl;

        if (m_impl)
            m_impl->AddRef();
    }

    return *this;
}

//---------------------------------------------------------------------------
// wxArchiveFSCache
//
// wxArchiveFSCacheData caches a single archive, and this class holds a
// collection of them to cache all the archives accessed by this instance
// of wxFileSystem.
//---------------------------------------------------------------------------

WX_DECLARE_STRING_HASH_MAP(wxArchiveFSCacheData, wxArchiveFSCacheDataHash);

class wxArchiveFSCache
{
public:
    wxArchiveFSCacheData* Add(const std::string& name,
                              const wxArchiveClassFactory& factory,
                              wxInputStream *stream);

    wxArchiveFSCacheData *Get(const std::string& name);

private:
    wxArchiveFSCacheDataHash m_hash;
};

wxArchiveFSCacheData* wxArchiveFSCache::Add(
        const std::string& name,
        const wxArchiveClassFactory& factory,
        wxInputStream *stream)
{
    wxArchiveFSCacheData& data = m_hash[name];

    if (stream->IsSeekable())
        data = wxArchiveFSCacheData(factory, stream);
    else
        data = wxArchiveFSCacheData(factory, wxBackingFile(stream));

    return &data;
}

wxArchiveFSCacheData *wxArchiveFSCache::Get(const std::string& name)
{
    wxArchiveFSCacheDataHash::iterator it;

    if ((it = m_hash.find(name)) != m_hash.end())
        return &it->second;

    return nullptr;
}

//----------------------------------------------------------------------------
// wxArchiveFSHandler
//----------------------------------------------------------------------------

bool wxArchiveFSHandler::CanOpen(const std::string& location)
{
    std::string p = GetProtocol(location);
    return wxArchiveClassFactory::Find(p) != nullptr;
}

wxFSFile* wxArchiveFSHandler::OpenFile(
        [[maybe_unused]] wxFileSystem& fs,
        const std::string& location)
{
    std::string right = GetRightLocation(location);
    std::string left = GetLeftLocation(location);
    std::string protocol = GetProtocol(location);

    std::string key = fmt::format("{}#{}:", left, protocol);

    if (wx::utils::Contains(right, "./"))
    {
        if (right[0] != '/') right = '/' + right;
        wxFileName rightPart(right, wxPATH_UNIX);
        rightPart.Normalize(wxPATH_NORM_DOTS, "/", wxPATH_UNIX);
        right = rightPart.GetFullPath(wxPATH_UNIX);
    }

    if (!right.empty() && right[0] == '/') right = right.substr(1);

    if (!m_cache)
        m_cache = new wxArchiveFSCache;

    const wxArchiveClassFactory* factory = wxArchiveClassFactory::Find(protocol);
    if (!factory)
        return nullptr;

    wxArchiveFSCacheData *cached = m_cache->Get(key);
    if (!cached)
    {
        wxFSFile *leftFile = m_fs.OpenFile(left);
        if (!leftFile)
            return nullptr;
        cached = m_cache->Add(key, *factory, leftFile->DetachStream());
        delete leftFile;
    }

    wxArchiveEntry *entry = cached->Get(right);
    if (!entry)
        return nullptr;

    wxInputStream *leftStream = cached->NewStream();
    if (!leftStream)
    {
        wxFSFile *leftFile = m_fs.OpenFile(left);
        if (!leftFile)
            return nullptr;
        leftStream = leftFile->DetachStream();
        delete leftFile;
    }

    wxArchiveInputStream *s = factory->NewStream(leftStream);
    if ( !s )
        return nullptr;

    s->OpenEntry(*entry);

    if (!s->IsOk())
    {
        delete s;
        return nullptr;
    }

    return new wxFSFile(s,
                        key + right,
                        {},
                        GetAnchor(location)
#if wxUSE_DATETIME
                        , entry->GetDateTime()
#endif // wxUSE_DATETIME
                        );
}

std::string wxArchiveFSHandler::FindFirst(const std::string& spec, unsigned int flags)
{
    std::string right = GetRightLocation(spec);
    std::string left = GetLeftLocation(spec);
    std::string protocol = GetProtocol(spec);

    std::string key = fmt::format("{}#{}:", left, protocol);

    if (!right.empty() && right.back() == '/') right.pop_back();

    if (!m_cache)
        m_cache = new wxArchiveFSCache;

    const wxArchiveClassFactory* factory = wxArchiveClassFactory::Find(protocol);
    if (!factory)
        return {};

    m_Archive = m_cache->Get(key);
    if (!m_Archive)
    {
        wxFSFile *leftFile = m_fs.OpenFile(left);
        if (!leftFile)
            return {};
        m_Archive = m_cache->Add(key, *factory, leftFile->DetachStream());
        delete leftFile;
    }

    m_FindEntry = nullptr;

    switch (flags)
    {
        case wxFILE:
            m_AllowDirs = false, m_AllowFiles = true; break;
        case wxDIR:
            m_AllowDirs = true, m_AllowFiles = false; break;
        default:
            m_AllowDirs = m_AllowFiles = true; break;
    }

    m_ZipFile = key;

    m_Pattern = wx::utils::AfterLast(right, '/');
    m_BaseDir = wx::utils::BeforeLast(right, '/');
    if (m_BaseDir.starts_with('/'))
        m_BaseDir = m_BaseDir.substr(1);

    if (m_Archive)
    {
        if (m_AllowDirs)
        {
            m_DirsFound.clear();

            if (right.empty())  // allow "/" to match the archive root
                return spec;
        }
        return DoFind();
    }
    return {};
}

std::string wxArchiveFSHandler::FindNext()
{
    if (!m_Archive) return {};
    return DoFind();
}

std::string wxArchiveFSHandler::DoFind()
{
    std::string namestr, dir, filename;
    std::string match;

    while (match.empty())
    {
        m_FindEntry = m_Archive->GetNext(m_FindEntry);

        if (!m_FindEntry)
        {
            m_Archive = nullptr;
            m_FindEntry = nullptr;
            break;
        }
        namestr = m_FindEntry->entry->GetName(wxPATH_UNIX);

        if (m_AllowDirs)
        {
            dir = wx::utils::BeforeLast(namestr, '/');
            while (!dir.empty())
            {
                if( !m_DirsFound.contains(dir) )
                {
                    m_DirsFound[dir] = 1;
                    filename = wx::utils::AfterLast(dir, '/');
                    dir = wx::utils::BeforeLast(dir, '/');
                    if (!filename.empty() && m_BaseDir == dir &&
                                wxMatchWild(m_Pattern, filename, false))
                        match = m_ZipFile + dir + "/" + filename;
                }
                else
                    break; // already tranversed
            }
        }

        filename = wx::utils::AfterLast(namestr, '/');
        dir = wx::utils::BeforeLast(namestr, '/');
        if (m_AllowFiles && !filename.empty() && m_BaseDir == dir &&
                            wxMatchWild(m_Pattern, filename, false))
            match = m_ZipFile + namestr;
    }

    return match;
}
