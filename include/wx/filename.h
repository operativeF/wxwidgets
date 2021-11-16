/////////////////////////////////////////////////////////////////////////////
// Name:        wx/filename.h
// Purpose:     wxFileName - encapsulates a file path
// Author:      Robert Roebling, Vadim Zeitlin
// Modified by:
// Created:     28.12.00
// Copyright:   (c) 2000 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef   _WX_FILENAME_H_
#define   _WX_FILENAME_H_

#include "wx/filefn.h"
#include "wx/datetime.h"
#include "wx/intl.h"
#include "wx/longlong.h"
#include "wx/file.h"

import <vector>;

#if wxUSE_FILE
class wxFile;
#endif

#if wxUSE_FFILE
class wxFFile;
#endif

// this symbol is defined for the platforms where file systems use volumes in
// paths
#if defined(WX_WINDOWS)
    #define wxHAS_FILESYSTEM_VOLUMES
#endif

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// the various values for the path format: this mainly affects the path
// separator but also whether or not the path has the drive part (as under
// Windows)
enum wxPathFormat
{
    wxPATH_NATIVE = 0,      // the path format for the current platform
    wxPATH_UNIX,
    wxPATH_BEOS = wxPATH_UNIX,
    wxPATH_MAC,
    wxPATH_DOS,
    wxPATH_WIN = wxPATH_DOS,
    wxPATH_OS2 = wxPATH_DOS,
    wxPATH_VMS,

    wxPATH_MAX // Not a valid value for specifying path format
};

// different conventions that may be used with GetHumanReadableSize()
enum class wxSizeConvention
{
    Traditional,  // 1024 bytes = 1 KB
    IEC,          // 1024 bytes = 1 KiB
    SI            // 1000 bytes = 1 KB
};

// the kind of normalization to do with the file name: these values can be
// or'd together to perform several operations at once
enum wxPathNormalize
{
    wxPATH_NORM_ENV_VARS = 0x0001,  // replace env vars with their values
    wxPATH_NORM_DOTS     = 0x0002,  // squeeze all .. and .
    wxPATH_NORM_TILDE    = 0x0004,  // Unix only: replace ~ and ~user
    wxPATH_NORM_CASE     = 0x0008,  // if case insensitive => tolower
    wxPATH_NORM_ABSOLUTE = 0x0010,  // make the path absolute
    wxPATH_NORM_LONG =     0x0020,  // make the path the long form
    wxPATH_NORM_SHORTCUT = 0x0040,  // resolve the shortcut, if it is a shortcut
    wxPATH_NORM_ALL      = 0x00ff & ~wxPATH_NORM_CASE
};

// what exactly should GetPath() return?
enum
{
    wxPATH_NO_SEPARATOR  = 0x0000,  // for symmetry with wxPATH_GET_SEPARATOR
    wxPATH_GET_VOLUME    = 0x0001,  // include the volume if applicable
    wxPATH_GET_SEPARATOR = 0x0002   // terminate the path with the separator
};

// Mkdir flags
enum
{
    wxPATH_MKDIR_FULL    = 0x0001   // create directories recursively
};

// Rmdir flags
enum
{
    wxPATH_RMDIR_FULL       = 0x0001,  // delete with subdirectories if empty
    wxPATH_RMDIR_RECURSIVE  = 0x0002   // delete all recursively (dangerous!)
};

// FileExists flags
enum
{
    wxFILE_EXISTS_REGULAR   = 0x0001,  // check for existence of a regular file
    wxFILE_EXISTS_DIR       = 0x0002,  // check for existence of a directory
    wxFILE_EXISTS_SYMLINK   = 0x1004,  // check for existence of a symbolic link;
                                       // also sets wxFILE_EXISTS_NO_FOLLOW as
                                       // it would never be satisfied otherwise
    wxFILE_EXISTS_DEVICE    = 0x0008,  // check for existence of a device
    wxFILE_EXISTS_FIFO      = 0x0010,  // check for existence of a FIFO
    wxFILE_EXISTS_SOCKET    = 0x0020,  // check for existence of a socket
                                       // gap for future types
    wxFILE_EXISTS_NO_FOLLOW = 0x1000,  // don't dereference a contained symlink
    wxFILE_EXISTS_ANY       = 0x1FFF   // check for existence of anything
};

#if wxUSE_LONGLONG
// error code of wxFileName::GetSize()
extern WXDLLIMPEXP_DATA_BASE(const wxULongLong) wxInvalidSize;
#endif // wxUSE_LONGLONG

// ----------------------------------------------------------------------------
// wxFileName: encapsulates a file path
// ----------------------------------------------------------------------------

class wxFileName
{
public:
    // constructors and assignment

        // the usual stuff
    wxFileName() { Clear(); }
    wxFileName(const wxFileName& filepath) { Assign(filepath); }

        // from a full filename: if it terminates with a '/', a directory path
        // is constructed (the name will be empty), otherwise a file name and
        // extension are extracted from it
    wxFileName( const std::string& fullpath, wxPathFormat format = wxPATH_NATIVE )
        { Assign( fullpath, format ); m_dontFollowLinks = false; }

        // from a directory name and a file name
    wxFileName(const std::string& path,
               const std::string& name,
               wxPathFormat format = wxPATH_NATIVE)
        { Assign(path, name, format); m_dontFollowLinks = false; }

        // from a volume, directory name, file base name and extension
    wxFileName(const std::string& volume,
               const std::string& path,
               const std::string& name,
               const std::string& ext,
               wxPathFormat format = wxPATH_NATIVE)
        { Assign(volume, path, name, ext, format); m_dontFollowLinks = false; }

        // from a directory name, file base name and extension
    wxFileName(const std::string& path,
               const std::string& name,
               const std::string& ext,
               wxPathFormat format = wxPATH_NATIVE)
        { Assign(path, name, ext, format); m_dontFollowLinks = false; }

        // the same for delayed initialization

    void Assign(const wxFileName& filepath);

    void Assign(const std::string& fullpath,
                wxPathFormat format = wxPATH_NATIVE);

    void Assign(const std::string& volume,
                const std::string& path,
                const std::string& name,
                const std::string& ext,
                bool hasExt,
                wxPathFormat format = wxPATH_NATIVE);

    void Assign(const std::string& volume,
                const std::string& path,
                const std::string& name,
                const std::string& ext,
                wxPathFormat format = wxPATH_NATIVE)
        { Assign(volume, path, name, ext, !ext.empty(), format); }

    void Assign(const std::string& path,
                const std::string& name,
                wxPathFormat format = wxPATH_NATIVE);

    void Assign(const std::string& path,
                const std::string& name,
                const std::string& ext,
                wxPathFormat format = wxPATH_NATIVE);

    void AssignDir(const std::string& dir, wxPathFormat format = wxPATH_NATIVE);

        // assorted assignment operators

    wxFileName& operator=(const wxFileName& filename)
        { if (this != &filename) Assign(filename); return *this; }

    wxFileName& operator=(const std::string& filename)
        { Assign(filename); return *this; }

        // reset all components to default, uninitialized state
    void Clear();

        // static pseudo constructors
    static wxFileName FileName(const std::string& file,
                               wxPathFormat format = wxPATH_NATIVE);
    static wxFileName DirName(const std::string& dir,
                              wxPathFormat format = wxPATH_NATIVE);

    // file tests

        // is the filename valid at all?
    bool IsOk() const
    {
        // we're fine if we have the path or the name or if we're a root dir
        return m_dirs.size() != 0 || !m_name.empty() || !m_relative ||
                !m_ext.empty() || m_hasExt;
    }

        // does the file with this name exist?
    bool FileExists() const;
    static bool FileExists( const std::string &file );

        // does the directory with this name exist?
    bool DirExists() const;
    static bool DirExists( const std::string &dir );

        // does anything at all with this name (i.e. file, directory or some
        // other file system object such as a device, socket, ...) exist?
    bool Exists(unsigned int flags = wxFILE_EXISTS_ANY) const;
    static bool Exists(const std::string& path, unsigned int flags = wxFILE_EXISTS_ANY);


        // checks on most common flags for files/directories;
        // more platform-specific features (like e.g. Unix permissions) are not
        // available in wxFileName

    bool IsDirWritable() const { return wxIsWritable(GetPath()); }
    static bool IsDirWritable(const std::string &path) { return wxDirExists(path) && wxIsWritable(path); }

    bool IsDirReadable() const { return wxIsReadable(GetPath()); }
    static bool IsDirReadable(const std::string &path) { return wxDirExists(path) && wxIsReadable(path); }

    // NOTE: IsDirExecutable() is not present because the meaning of "executable"
    //       directory is very platform-dependent and also not so useful

    bool IsFileWritable() const { return wxIsWritable(GetFullPath()); }
    static bool IsFileWritable(const std::string &path) { return wxFileExists(path) && wxIsWritable(path); }

    bool IsFileReadable() const { return wxIsReadable(GetFullPath()); }
    static bool IsFileReadable(const std::string &path) { return wxFileExists(path) && wxIsReadable(path); }

    bool IsFileExecutable() const { return wxIsExecutable(GetFullPath()); }
    static bool IsFileExecutable(const std::string &path) { return wxFileExists(path) && wxIsExecutable(path); }

        // set the file permissions to a combination of wxPosixPermissions enum
        // values
    bool SetPermissions(int permissions);

    // Returns the native path for a file URL
    static wxFileName URLToFileName(const std::string& url);

    // Returns the file URL for a native path
    static std::string FileNameToURL(const wxFileName& filename);

    // time functions
#if wxUSE_DATETIME
        // set the file last access/mod and creation times
        // (any of the pointers may be NULL)
    bool SetTimes(const wxDateTime *dtAccess,
                  const wxDateTime *dtMod,
                  const wxDateTime *dtCreate) const;

        // set the access and modification times to the current moment
    bool Touch() const;

        // return the last access, last modification and create times
        // (any of the pointers may be NULL)
    [[maybe_unused]] bool GetTimes(wxDateTime *dtAccess,
                  wxDateTime *dtMod,
                  wxDateTime *dtCreate) const;

        // convenience wrapper: get just the last mod time of the file
    wxDateTime GetModificationTime() const
    {
        wxDateTime dtMod;
        GetTimes(nullptr, &dtMod, nullptr);
        return dtMod;
    }
#endif // wxUSE_DATETIME

    // various file/dir operations

        // retrieve the value of the current working directory
    void AssignCwd(const std::string& volume = {});
    static std::string GetCwd(const std::string& volume = {});

        // change the current working directory
    bool SetCwd() const;
    static bool SetCwd( const std::string &cwd );

        // get the value of user home (Unix only mainly)
    void AssignHomeDir();
    static std::string GetHomeDir();

        // get the system temporary directory
    static std::string GetTempDir();

#if wxUSE_FILE || wxUSE_FFILE
        // get a temp file name starting with the specified prefix
    void AssignTempFileName(const std::string& prefix);
    static std::string CreateTempFileName(const std::string& prefix);
#endif // wxUSE_FILE

#if wxUSE_FILE
        // get a temp file name starting with the specified prefix and open the
        // file passed to us using this name for writing (atomically if
        // possible)
    void AssignTempFileName(const std::string& prefix, wxFile *fileTemp);
    static std::string CreateTempFileName(const std::string& prefix,
                                       wxFile *fileTemp);
#endif // wxUSE_FILE

#if wxUSE_FFILE
        // get a temp file name starting with the specified prefix and open the
        // file passed to us using this name for writing (atomically if
        // possible)
    void AssignTempFileName(const std::string& prefix, wxFFile *fileTemp);
    static std::string CreateTempFileName(const std::string& prefix,
                                       wxFFile *fileTemp);
#endif // wxUSE_FFILE

    // directory creation and removal.
    bool Mkdir(int perm = wxS_DIR_DEFAULT, unsigned int flags = 0) const;
    static bool Mkdir(const std::string &dir, int perm = wxS_DIR_DEFAULT,
                      unsigned int flags = 0);

    bool Rmdir(unsigned int flags = 0) const;
    static bool Rmdir(const std::string &dir, unsigned int flags = 0);

    // operations on the path

        // normalize the path: with the default flags value, the path will be
        // made absolute, without any ".." and "." and all environment
        // variables will be expanded in it
        //
        // this may be done using another (than current) value of cwd
    bool Normalize(unsigned int flags = wxPATH_NORM_ALL,
                   const std::string& cwd = {},
                   wxPathFormat format = wxPATH_NATIVE);

        // get a path path relative to the given base directory, i.e. opposite
        // of Normalize
        //
        // pass an empty string to get a path relative to the working directory
        //
        // returns true if the file name was modified, false if we failed to do
        // anything with it (happens when the file is on a different volume,
        // for example)
    bool MakeRelativeTo(const std::string& pathBase = {},
                        wxPathFormat format = wxPATH_NATIVE);

        // make the path absolute
        //
        // this may be done using another (than current) value of cwd
    bool MakeAbsolute(const std::string& cwd = {},
                      wxPathFormat format = wxPATH_NATIVE)
        { return Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE |
                           wxPATH_NORM_TILDE, cwd, format); }


    // If the path is a symbolic link (Unix-only), indicate that all
    // filesystem operations on this path should be performed on the link
    // itself and not on the file it points to, as is the case by default.
    //
    // No effect if this is not a symbolic link.
    void DontFollowLink()
    {
        m_dontFollowLinks = true;
    }

    // If the path is a symbolic link (Unix-only), returns whether various
    // file operations should act on the link itself, or on its target.
    //
    // This does not test if the path is really a symlink or not.
    bool ShouldFollowLink() const
    {
        return !m_dontFollowLinks;
    }

    // Resolve a wxFileName object representing a link to its target
    wxFileName ResolveLink();

#if defined(__WIN32__) && wxUSE_OLE
        // if the path is a shortcut, return the target and optionally,
        // the arguments
    bool GetShortcutTarget(const std::string& shortcutPath,
                           std::string& targetFilename,
                           std::string* arguments = nullptr) const;
#endif

        // if the path contains the value of the environment variable named envname
        // then this function replaces it with the string obtained from
        //    std::string::Format(replacementFmtString, value_of_envname_variable)
        //
        // Example:
        //    wxFileName fn("/usr/openwin/lib/someFile");
        //    fn.ReplaceEnvVariable("OPENWINHOME");
        //         // now fn.GetFullPath() == "$OPENWINHOME/lib/someFile"
    bool ReplaceEnvVariable(const std::string& envname,
                            const std::string& replacementFmtString = "$%s",
                            wxPathFormat format = wxPATH_NATIVE);

        // replaces, if present in the path, the home directory for the given user
        // (see wxGetHomeDir) with a tilde
    bool ReplaceHomeDir(wxPathFormat format = wxPATH_NATIVE);


    // Comparison

        // compares with the rules of the given platforms format
    bool SameAs(const wxFileName& filepath,
                wxPathFormat format = wxPATH_NATIVE) const;

        // compare with another filename object
    bool operator==(const wxFileName& filename) const
        { return SameAs(filename); }
    bool operator!=(const wxFileName& filename) const
        { return !SameAs(filename); }

        // compare with a filename string interpreted as a native file name
    bool operator==(const std::string& filename) const
        { return SameAs(wxFileName(filename)); }
    bool operator!=(const std::string& filename) const
        { return !SameAs(wxFileName(filename)); }

        // are the file names of this type cases sensitive?
    static bool IsCaseSensitive( wxPathFormat format = wxPATH_NATIVE );

        // is this filename absolute?
    bool IsAbsolute(wxPathFormat format = wxPATH_NATIVE) const;

        // is this filename relative?
    bool IsRelative(wxPathFormat format = wxPATH_NATIVE) const
        { return !IsAbsolute(format); }

    // Returns the characters that aren't allowed in filenames
    // on the specified platform.
    static std::string GetForbiddenChars(wxPathFormat format = wxPATH_NATIVE);

    // Information about path format

    // get the string separating the volume from the path for this format,
    // return an empty string if this format doesn't support the notion of
    // volumes at all
    static std::string GetVolumeSeparator(wxPathFormat format = wxPATH_NATIVE);

    // get the string of path separators for this format
    static std::string GetPathSeparators(wxPathFormat format = wxPATH_NATIVE);

    // get the string of path terminators, i.e. characters which terminate the
    // path
    static std::string GetPathTerminators(wxPathFormat format = wxPATH_NATIVE);

    // get the canonical path separator for this format
    static wxUniChar GetPathSeparator(wxPathFormat format = wxPATH_NATIVE)
        { return GetPathSeparators(format)[0u]; }

    // is the char a path separator for this format?
    static bool IsPathSeparator(char ch, wxPathFormat format = wxPATH_NATIVE);

    // is this is a DOS path which beings with a windows unique volume name
    // ('\\?\Volume{guid}\')?
    static bool IsMSWUniqueVolumeNamePath(const std::string& path,
                                          wxPathFormat format = wxPATH_NATIVE);

    // Dir accessors
    size_t GetDirCount() const { return m_dirs.size(); }
    bool AppendDir(const std::string& dir);
    void PrependDir(const std::string& dir);
    bool InsertDir(size_t before, const std::string& dir);
    void RemoveDir(size_t pos);
    void RemoveLastDir() { RemoveDir(GetDirCount() - 1); }

    // Other accessors
    void SetExt( const std::string &ext )          { m_ext = ext; m_hasExt = !m_ext.empty(); }
    void ClearExt()                             { m_ext.clear(); m_hasExt = false; }
    void SetEmptyExt()                          { m_ext.clear(); m_hasExt = true; }
    std::string GetExt() const                     { return m_ext; }
    bool HasExt() const                         { return m_hasExt; }

    void SetName( const std::string &name )        { m_name = name; }
    std::string GetName() const                    { return m_name; }
    bool HasName() const                        { return !m_name.empty(); }

    void SetVolume( const std::string &volume )    { m_volume = volume; }
    std::string GetVolume() const                  { return m_volume; }
    bool HasVolume() const                      { return !m_volume.empty(); }

    // full name is the file name + extension (but without the path)
    void SetFullName(const std::string& fullname);
    std::string GetFullName() const;

    const std::vector<std::string>& GetDirs() const        { return m_dirs; }

    // flags are combination of wxPATH_GET_XXX flags
    std::string GetPath(unsigned int flags = wxPATH_GET_VOLUME,
                     wxPathFormat format = wxPATH_NATIVE) const;

    // Replace current path with this one
    void SetPath( const std::string &path, wxPathFormat format = wxPATH_NATIVE );

    // Construct full path with name and ext
    std::string GetFullPath( wxPathFormat format = wxPATH_NATIVE ) const;

    // Return the short form of the path (returns identity on non-Windows platforms)
    std::string GetShortPath() const;

    // Return the long form of the path (returns identity on non-Windows platforms)
    std::string GetLongPath() const;

    // Is this a file or directory (not necessarily an existing one)
    bool IsDir() const { return m_name.empty() && m_ext.empty(); }

    // various helpers

        // get the canonical path format for this platform
    static wxPathFormat GetFormat( wxPathFormat format = wxPATH_NATIVE );

        // split a fullpath into the volume, path, (base) name and extension
        // (all of the pointers can be NULL)
    static void SplitPath(const std::string& fullpath,
                          std::string *volume,
                          std::string *path,
                          std::string *name,
                          std::string *ext,
                          bool *hasExt = nullptr,
                          wxPathFormat format = wxPATH_NATIVE);

    static void SplitPath(const std::string& fullpath,
                          std::string *volume,
                          std::string *path,
                          std::string *name,
                          std::string *ext,
                          wxPathFormat format)
    {
        SplitPath(fullpath, volume, path, name, ext, nullptr, format);
    }

        // compatibility version: volume is part of path
    static void SplitPath(const std::string& fullpath,
                          std::string *path,
                          std::string *name,
                          std::string *ext,
                          wxPathFormat format = wxPATH_NATIVE);

        // split a path into volume and pure path part
    static void SplitVolume(const std::string& fullpathWithVolume,
                            std::string *volume,
                            std::string *path,
                            wxPathFormat format = wxPATH_NATIVE);

        // strip the file extension: "foo.bar" => "foo" (but ".baz" => ".baz")
    static std::string StripExtension(const std::string& fullpath);

#ifdef wxHAS_FILESYSTEM_VOLUMES
        // return the string representing a file system volume, or drive
    static std::string GetVolumeString(char drive, unsigned int flags = wxPATH_GET_SEPARATOR);
#endif // wxHAS_FILESYSTEM_VOLUMES

    // File size

#if wxUSE_LONGLONG
        // returns the size of the given filename
    wxULongLong GetSize() const;
    static wxULongLong GetSize(const std::string &file);

        // returns the size in a human readable form
    std::string
    GetHumanReadableSize(const std::string& nullsize = wxGetTranslation(wxASCII_STR("Not available")),
                         int precision = 1,
                         wxSizeConvention conv = wxSizeConvention::Traditional) const;
    static std::string
    GetHumanReadableSize(const wxULongLong& sz,
                         const std::string& nullsize = wxGetTranslation(wxASCII_STR("Not available")),
                         int precision = 1,
                         wxSizeConvention conv = wxSizeConvention::Traditional);
#endif // wxUSE_LONGLONG

private:
    // check whether this dir is valid for Append/Prepend/InsertDir()
    static bool IsValidDirComponent(const std::string& dir);

    // the drive/volume/device specification (always empty for Unix)
    std::string        m_volume;

    // the path components of the file
    std::vector<std::string>   m_dirs;

    // the file name and extension (empty for directories)
    std::string        m_name;
    std::string        m_ext;

    // when m_dirs is empty it may mean either that we have no path at all or
    // that our path is '/', i.e. the root directory
    //
    // we use m_relative to distinguish between these two cases, it will be
    // true in the former and false in the latter
    //
    // NB: the path is not absolute just because m_relative is false, it still
    //     needs the drive (i.e. volume) in some formats (Windows)
    bool            m_relative;

    // when m_ext is empty, it may be because we don't have any extension or
    // because we have an empty extension
    //
    // the difference is important as file with name "foo" and without
    // extension has full name "foo" while with empty extension it is "foo."
    bool            m_hasExt;

    // by default, symlinks are dereferenced but this flag can be set with
    // DontFollowLink() to change this and make different operations work on
    // this file path itself instead of the target of the symlink
    bool            m_dontFollowLinks;
};

#endif // _WX_FILENAME_H_

