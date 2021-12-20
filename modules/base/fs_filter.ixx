/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fs_filter.h
// Purpose:     Filter file system handler
// Author:      Mike Wetherell
// Copyright:   (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/filesys.h"

export module WX.FileSys.Filter;

#if wxUSE_FILESYSTEM

export
{

//---------------------------------------------------------------------------
// wxFilterFSHandler
//---------------------------------------------------------------------------

class wxFilterFSHandler : public wxFileSystemHandler
{
public:
    wxFilterFSHandler& operator=(wxFilterFSHandler&&) = delete;

    bool CanOpen(const std::string& location) override;
    wxFSFile* OpenFile(wxFileSystem& fs, const std::string& location) override;

    std::string FindFirst(const std::string& spec, unsigned int flags = 0) override;
    std::string FindNext() override;
};

} // export

#endif // wxUSE_FILESYSTEM
