/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fs_filter.h
// Purpose:     Filter file system handler
// Author:      Mike Wetherell
// Copyright:   (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FS_FILTER_H_
#define _WX_FS_FILTER_H_

#if wxUSE_FILESYSTEM

#include "wx/filesys.h"

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

#endif // wxUSE_FILESYSTEM

#endif // _WX_FS_FILTER_H_
