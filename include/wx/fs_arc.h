/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fs_arc.h
// Purpose:     Archive file system
// Author:      Vaclav Slavik, Mike Wetherell
// Copyright:   (c) 1999 Vaclav Slavik, (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FS_ARC_H_
#define _WX_FS_ARC_H_

#include "wx/defs.h"

#if wxUSE_FS_ARCHIVE

#include "wx/filesys.h"
#include "wx/hashmap.h"

WX_DECLARE_STRING_HASH_MAP(int, wxArchiveFilenameHashMap);

//---------------------------------------------------------------------------
// wxArchiveFSHandler
//---------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxArchiveFSHandler : public wxFileSystemHandler
{
public:
    wxArchiveFSHandler();

    wxArchiveFSHandler(const wxArchiveFSHandler&) = delete;
	wxArchiveFSHandler& operator=(const wxArchiveFSHandler&) = delete;

    bool CanOpen(const wxString& location) override;
    wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location) override;
    wxString FindFirst(const wxString& spec, int flags = 0) override;
    wxString FindNext() override;
    void Cleanup();
    ~wxArchiveFSHandler() override;

private:
    class wxArchiveFSCache *m_cache{nullptr};
    wxFileSystem m_fs;

    // these vars are used by FindFirst/Next:
    class wxArchiveFSCacheData *m_Archive{nullptr};
    struct wxArchiveFSEntry *m_FindEntry{nullptr};
    wxString m_Pattern;
    wxString m_BaseDir;
    wxString m_ZipFile;
    bool m_AllowDirs{true};
    bool m_AllowFiles{true};
    wxArchiveFilenameHashMap *m_DirsFound{nullptr};

    wxString DoFind();

    wxDECLARE_DYNAMIC_CLASS(wxArchiveFSHandler);
};

#endif // wxUSE_FS_ARCHIVE

#endif // _WX_FS_ARC_H_
