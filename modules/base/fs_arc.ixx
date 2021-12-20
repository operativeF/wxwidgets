/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fs_arc.h
// Purpose:     Archive file system
// Author:      Vaclav Slavik, Mike Wetherell
// Copyright:   (c) 1999 Vaclav Slavik, (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/filesys.h"
#include "wx/hashmap.h"

export module WX.FileSys.Arc;

import <string>;

#if wxUSE_FS_ARCHIVE

export
{

WX_DECLARE_STRING_HASH_MAP(int, wxArchiveFilenameHashMap);

//---------------------------------------------------------------------------
// wxArchiveFSHandler
//---------------------------------------------------------------------------

class wxArchiveFSHandler : public wxFileSystemHandler
{
public:
    wxArchiveFSHandler& operator=(wxArchiveFSHandler&&) = delete;

    bool CanOpen(const std::string& location) override;
    wxFSFile* OpenFile(wxFileSystem& fs, const std::string& location) override;
    std::string FindFirst(const std::string& spec, unsigned int flags = 0) override;
    std::string FindNext() override;
    void Cleanup();
    ~wxArchiveFSHandler();

private:
    class wxArchiveFSCache *m_cache{nullptr};
    wxFileSystem m_fs;

    // these vars are used by FindFirst/Next:
    class wxArchiveFSCacheData *m_Archive{nullptr};
    struct wxArchiveFSEntry *m_FindEntry{nullptr};
    std::string m_Pattern;
    std::string m_BaseDir;
    std::string m_ZipFile;
    bool m_AllowDirs{true};
    bool m_AllowFiles{true};
    wxArchiveFilenameHashMap *m_DirsFound{nullptr};

    std::string DoFind();
};

} // export

#endif // wxUSE_FS_ARCHIVE
