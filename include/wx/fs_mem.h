/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fs_mem.h
// Purpose:     in-memory file system
// Author:      Vaclav Slavik
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FS_MEM_H_
#define _WX_FS_MEM_H_

#if wxUSE_FILESYSTEM

#include "wx/filesys.h"

#include "wx/hashmap.h"

class wxMemoryFSFile;
WX_DECLARE_STRING_HASH_MAP(wxMemoryFSFile *, wxMemoryFSHash);

#if wxUSE_GUI
    #include "wx/bitmap.h"
#endif // wxUSE_GUI

// ----------------------------------------------------------------------------
// wxMemoryFSHandlerBase
// ----------------------------------------------------------------------------

class wxMemoryFSHandlerBase : public wxFileSystemHandler
{
public:
    wxMemoryFSHandlerBase() = default;
    ~wxMemoryFSHandlerBase();

    // Add file to list of files stored in memory. Stored data (bitmap, text or
    // raw data) will be copied into private memory stream and available under
    // name "memory:" + filename
    static void AddFile(const std::string& filename, const std::string& textdata);
    static void AddFile(const std::string& filename, const void *binarydata, size_t size);
    static void AddFileWithMimeType(const std::string& filename,
                                    const std::string& textdata,
                                    const std::string& mimetype);
    static void AddFileWithMimeType(const std::string& filename,
                                    const void *binarydata, size_t size,
                                    const std::string& mimetype);

    // Remove file from memory FS and free occupied memory
    static void RemoveFile(const std::string& filename);

    bool CanOpen(const std::string& location) override;
    wxFSFile* OpenFile(wxFileSystem& fs, const std::string& location) override;
    std::string FindFirst(const std::string& spec, unsigned int flags = 0) override;
    std::string FindNext() override;

protected:
    // check that the given file is not already present in m_Hash; logs an
    // error and returns false if it does exist
    static bool CheckDoesntExist(const std::string& filename);

    // the hash map indexed by the names of the files stored in the memory FS
    inline static wxMemoryFSHash m_Hash{};

    // the file name currently being searched for, i.e. the argument of the
    // last FindFirst() call or empty string if FindFirst() hasn't been called
    // yet
    std::string m_findArgument;

    // iterator into m_Hash used by FindFirst/Next(), possibly m_Hash.end()
    wxMemoryFSHash::const_iterator m_findIter;
};

// ----------------------------------------------------------------------------
// wxMemoryFSHandler
// ----------------------------------------------------------------------------

#if wxUSE_GUI

// add GUI-only operations to the base class
class wxMemoryFSHandler : public wxMemoryFSHandlerBase
{
public:
    // bring the base class versions into the scope, otherwise they would be
    // inaccessible in wxMemoryFSHandler
    // (unfortunately "using" can't be used as gcc 2.95 doesn't have it...)
    static void AddFile(const std::string& filename, const std::string& textdata)
    {
        wxMemoryFSHandlerBase::AddFile(filename, textdata);
    }

    static void AddFile(const std::string& filename,
                        const void *binarydata,
                        size_t size)
    {
        wxMemoryFSHandlerBase::AddFile(filename, binarydata, size);
    }
    static void AddFileWithMimeType(const std::string& filename,
                                    const std::string& textdata,
                                    const std::string& mimetype)
    {
        wxMemoryFSHandlerBase::AddFileWithMimeType(filename,
                                                   textdata,
                                                   mimetype);
    }
    static void AddFileWithMimeType(const std::string& filename,
                                    const void *binarydata, size_t size,
                                    const std::string& mimetype)
    {
        wxMemoryFSHandlerBase::AddFileWithMimeType(filename,
                                                   binarydata, size,
                                                   mimetype);
    }

#if wxUSE_IMAGE
    static void AddFile(const std::string& filename,
                        const wxImage& image,
                        wxBitmapType type);

    static void AddFile(const std::string& filename,
                        const wxBitmap& bitmap,
                        wxBitmapType type);
#endif // wxUSE_IMAGE

};

#else // !wxUSE_GUI

// just the same thing as the base class in wxBase
class wxMemoryFSHandler : public wxMemoryFSHandlerBase
{
};

#endif // wxUSE_GUI/!wxUSE_GUI

#endif // wxUSE_FILESYSTEM

#endif // _WX_FS_MEM_H_

