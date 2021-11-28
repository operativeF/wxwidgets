/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dir.h
// Purpose:     wxDir is a class for enumerating the files in a directory
// Author:      Vadim Zeitlin
// Modified by:
// Created:     08.12.99
// Copyright:   (c) 1999 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DIR_H_
#define _WX_DIR_H_

#include "wx/longlong.h"
#include "wx/filefn.h"      // for wxS_DIR_DEFAULT

import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// These flags affect the behaviour of GetFirst/GetNext() and Traverse().
// They define what types are included in the list of items they produce.
// Note that wxDIR_NO_FOLLOW is relevant only on Unix and ignored under systems
// not supporting symbolic links.
enum wxDirFlags
{
    wxDIR_FILES     = 0x0001,       // include files
    wxDIR_DIRS      = 0x0002,       // include directories
    wxDIR_HIDDEN    = 0x0004,       // include hidden files
    wxDIR_DOTDOT    = 0x0008,       // include '.' and '..'
    wxDIR_NO_FOLLOW = 0x0010,       // don't dereference any symlink

    // by default, enumerate everything except '.' and '..'
    wxDIR_DEFAULT   = wxDIR_FILES | wxDIR_DIRS | wxDIR_HIDDEN
};

// these constants are possible return value of wxDirTraverser::OnDir()
enum class wxDirTraverseResult
{
    Ignore,      // ignore this directory but continue with others
    Stop,             // stop traversing
    Continue          // continue into this directory
};

#if wxUSE_LONGLONG
// error code of wxDir::GetTotalSize()
extern const wxULongLong wxInvalidSize;
#endif // wxUSE_LONGLONG

// ----------------------------------------------------------------------------
// wxDirTraverser: helper class for wxDir::Traverse()
// ----------------------------------------------------------------------------

class wxDirTraverser
{
public:
    /// a virtual dtor has been provided since this class has virtual members
    virtual ~wxDirTraverser() = default;
    // called for each file found by wxDir::Traverse()
    //
    // return wxDirTraverseResult::Stop or wxDirTraverseResult::Continue from here (wxDirTraverseResult::Ignore doesn't
    // make sense)
    virtual wxDirTraverseResult OnFile(const std::string& filename) = 0;

    // called for each directory found by wxDir::Traverse()
    //
    // return one of the enum elements defined above
    virtual wxDirTraverseResult OnDir(const std::string& dirname) = 0;

    // called for each directory which we couldn't open during our traversal
    // of the directory tree
    //
    // this method can also return either wxDirTraverseResult::Stop, wxDirTraverseResult::Ignore or
    // wxDirTraverseResult::Continue but the latter is treated specially: it means to retry
    // opening the directory and so may lead to infinite loop if it is
    // returned unconditionally, be careful with this!
    //
    // the base class version always returns wxDirTraverseResult::Ignore
    virtual wxDirTraverseResult OnOpenError(const std::string& dirname);
};

// ----------------------------------------------------------------------------
// wxDir: portable equivalent of {open/read/close}dir functions
// ----------------------------------------------------------------------------

class wxDirData;

class wxDir
{
public:
    // default, use Open()
    wxDir() = default;

    // opens the directory for enumeration, use IsOpened() to test success
    wxDir(const std::string& dir);

    // dtor calls Close() automatically
    ~wxDir() { Close(); }

    wxDir& operator=(wxDir&&) = delete;

    // open the directory for enumerating
    bool Open(const std::string& dir);

    // close the directory, Open() can be called again later
    void Close();

    // returns true if the directory was successfully opened
    bool IsOpened() const;

    // get the full name of the directory (without '/' at the end)
    std::string GetName() const;

    // Same as GetName() but does include the trailing separator, unless the
    // string is empty (only for invalid directories).
    std::string GetNameWithSep() const;


    // file enumeration routines
    // -------------------------

    // start enumerating all files matching filespec (or all files if it is
    // empty) and flags, return true on success
    bool GetFirst(std::string *filename,
                  const std::string& filespec = {},
                  unsigned int flags = wxDIR_DEFAULT) const;

    // get next file in the enumeration started with GetFirst()
    bool GetNext(std::string *filename) const;

    // return true if this directory has any files in it
    bool HasFiles(const std::string& spec = {}) const;

    // return true if this directory has any subdirectories
    bool HasSubDirs(const std::string& spec = {}) const;

    // enumerate all files in this directory and its subdirectories
    //
    // return the number of files found
    size_t Traverse(wxDirTraverser& sink,
                    const std::string& filespec = {},
                    unsigned int flags = wxDIR_DEFAULT) const;

    // simplest version of Traverse(): get the names of all files under this
    // directory into filenames array, return the number of files
    static size_t GetAllFiles(const std::string& dirname,
                              std::vector<std::string>* files,
                              const std::string& filespec = {},
                              unsigned int flags = wxDIR_DEFAULT);

    // check if there any files matching the given filespec under the given
    // directory (i.e. searches recursively), return the file path if found or
    // empty string otherwise
    static std::string FindFirst(const std::string& dirname,
                              const std::string& filespec,
                              unsigned int flags = wxDIR_DEFAULT);

#if wxUSE_LONGLONG
    // returns the size of all directories recursively found in given path
    static wxULongLong GetTotalSize(const std::string &dir, std::vector<std::string>* filesSkipped = nullptr);
#endif // wxUSE_LONGLONG


    // static utilities for directory management
    // (alias to wxFileName's functions for dirs)
    // -----------------------------------------

    // test for existence of a directory with the given name
    static bool Exists(const std::string& dir);

    static bool Make(const std::string &dir, unsigned int perm = wxS_DIR_DEFAULT,
                     unsigned int flags = 0);

    static bool Remove(const std::string &dir, unsigned int flags = 0);


private:
    friend class wxDirData;

    wxDirData *m_data{nullptr};
};

#endif // _WX_DIR_H_

