/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/filename.cpp
// Purpose:     wxFileName - encapsulates a file path
// Author:      Robert Roebling, Vadim Zeitlin
// Modified by:
// Created:     28.12.2000
// Copyright:   (c) 2000 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

/*
   Here are brief descriptions of the filename formats supported by this class:

   wxPATH_UNIX: standard Unix format, used under Darwin as well, absolute file
                names have the form:
                /dir1/dir2/.../dirN/filename, "." and ".." stand for the
                current and parent directory respectively, "~" is parsed as the
                user HOME and "~username" as the HOME of that user

   wxPATH_DOS:  DOS/Windows format, absolute file names have the form:
                drive:\dir1\dir2\...\dirN\filename.ext where drive is a single
                letter. "." and ".." as for Unix but no "~".

                There are also UNC names of the form \\share\fullpath and
                MSW unique volume names of the form \\?\Volume{GUID}\fullpath.

                The latter provide a uniform way to access a volume regardless of
                its current mount point, i.e. you can change a volume's mount
                point from D: to E:, or even remove it, and still be able to
                access it through its unique volume name. More on the subject can
                be found in MSDN's article "Naming a Volume" that is currently at
                http://msdn.microsoft.com/en-us/library/aa365248(VS.85).aspx.


   wxPATH_MAC:  Mac OS 8/9 only, not used any longer, absolute file
                names have the form
                    volume:dir1:...:dirN:filename
                and the relative file names are either
                    :dir1:...:dirN:filename
                or just
                    filename
                (although :filename works as well).
                Since the volume is just part of the file path, it is not
                treated like a separate entity as it is done under DOS and
                VMS, it is just treated as another dir.

   wxPATH_VMS:  VMS native format, absolute file names have the form
                    <device>:[dir1.dir2.dir3]file.txt
                or
                    <device>:[000000.dir1.dir2.dir3]file.txt

                the <device> is the physical device (i.e. disk). 000000 is the
                root directory on the device which can be omitted.

                Note that VMS uses different separators unlike Unix:
                 : always after the device. If the path does not contain : than
                   the default (the device of the current directory) is assumed.
                 [ start of directory specification
                 . separator between directory and subdirectory
                 ] between directory and file
 */

module;

#ifdef WX_WINDOWS
    #include "wx/msw/private.h"
#endif

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/utils.h"
#include "wx/crt.h"
#include "wx/private/filename.h"
#include "wx/config.h"          // for wxExpandEnvVars
#include "wx/dynlib.h"
#include "wx/dir.h"
#include "wx/longlong.h"

#if defined(__WIN32__) && defined(__MINGW32__)
    #include "wx/msw/gccpriv.h"
#endif

#ifdef WX_WINDOWS
    #include "wx/msw/ole/oleutils.h"
    #include "wx/msw/private/comptr.h"
    #include "wx/msw/wrapshl.h"         // for CLSID_ShellLink
#endif

#if defined(__WXMAC__)
  #include  "wx/osx/private.h"  // includes mac headers
#endif

// utime() is POSIX so should normally be available on all Unices
#ifdef __UNIX_LIKE__
#include <sys/types.h>
#include <utime.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <cassert>

module WX.File.Filename;

import Utils.Strings;
import WX.Cmn.Uri;

#if defined(wxHAS_NATIVE_READLINK)
    import <vector>;
#endif

#ifndef S_ISREG
    #define S_ISREG(mode) ((mode) & S_IFREG)
#endif
#ifndef S_ISDIR
    #define S_ISDIR(mode) ((mode) & S_IFDIR)
#endif

#if wxUSE_LONGLONG
extern const wxULongLong wxInvalidSize = (unsigned)-1;
#endif // wxUSE_LONGLONG

namespace
{

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// small helper class which opens and closes the file - we use it just to get
// a file handle for the given file name to pass it to some Win32 API function
#if defined(__WIN32__)

class wxFileHandle
{
public:
    enum class OpenMode
    {
        ReadAttr,
        WriteAttr
    };

    // be careful and use FILE_{READ,WRITE}_ATTRIBUTES here instead of the
    // usual GENERIC_{READ,WRITE} as we don't want the file access time to
    // be changed when we open it because this class is used for setting
    // access time (see #10567)
    wxFileHandle(const std::string& filename, OpenMode mode, unsigned int flags = 0)
    {
        boost::nowide::wstackstring stackFileName{filename.c_str()};
        m_hFile = ::CreateFileW
                    (
                     stackFileName.get(),             // name
                     mode == OpenMode::ReadAttr ? FILE_READ_ATTRIBUTES    // access mask
                                      : FILE_WRITE_ATTRIBUTES,
                     FILE_SHARE_READ |              // sharing mode
                     FILE_SHARE_WRITE,              // (allow everything)
                     nullptr,                          // no secutity attr
                     OPEN_EXISTING,                 // creation disposition
                     flags,                         // flags
                     nullptr                           // no template file
                    );

        if ( m_hFile == INVALID_HANDLE_VALUE )
        {
            if ( mode == OpenMode::ReadAttr )
            {
                wxLogSysError(_("Failed to open '%s' for reading"),
                              filename.c_str());
            }
            else
            {
                wxLogSysError(_("Failed to open '%s' for writing"),
                              filename.c_str());
            }
        }
    }

    ~wxFileHandle()
    {
        if ( m_hFile != INVALID_HANDLE_VALUE )
        {
            if ( !::CloseHandle(m_hFile) )
            {
                wxLogSysError(_("Failed to close file handle"));
            }
        }
    }

    // return true only if the file could be opened successfully
    bool IsOk() const { return m_hFile != INVALID_HANDLE_VALUE; }

    // get the handle
    operator HANDLE() const { return m_hFile; }

private:
    HANDLE m_hFile;
};

#endif // __WIN32__

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

#if wxUSE_DATETIME && defined(__WIN32__)

// Convert between wxDateTime and FILETIME which is a 64-bit value representing
// the number of 100-nanosecond intervals since January 1, 1601 UTC.
//
// This is the offset between FILETIME epoch and the Unix/wxDateTime Epoch.
constexpr std::int64_t EPOCH_OFFSET_IN_MSEC = 11644473600000LL;

void ConvertFileTimeToWx(wxDateTime *dt, const FILETIME &ft)
{
    wxLongLong t(ft.dwHighDateTime, ft.dwLowDateTime);
    t /= 10000; // Convert hundreds of nanoseconds to milliseconds.
    t -= EPOCH_OFFSET_IN_MSEC;

    *dt = wxDateTime(t);
}

void ConvertWxToFileTime(FILETIME *ft, const wxDateTime& dt)
{
    // Undo the conversions above.
    wxLongLong t(dt.GetValue());
    t += EPOCH_OFFSET_IN_MSEC;
    t *= 10000;

    ft->dwHighDateTime = t.GetHi();
    ft->dwLowDateTime = t.GetLo();
}

#endif // wxUSE_DATETIME && __WIN32__

// return a string with the volume par
static std::string wxGetVolumeString(const std::string& volume, wxPathFormat format)
{
    std::string path;

    if ( !volume.empty() )
    {
        format = wxFileName::GetFormat(format);

        // Special Windows UNC paths hack, part 2: undo what we did in
        // SplitPath() and make an UNC path if we have a drive which is not a
        // single letter (hopefully the network shares can't be one letter only
        // although I didn't find any authoritative docs on this)
        if ( format == wxPATH_DOS && volume.length() > 1 )
        {
            // We also have to check for Windows unique volume names here and
            // return it with '\\?\' prepended to it
            if ( wxFileName::IsMSWUniqueVolumeNamePath(R"(\\?\)" + volume + "\\",
                                                       format) )
            {
                path += R"(\\?\)" + volume;
            }
            else
            {
                // it must be a UNC path
                path += fmt::format("{}{}{}", wxFILE_SEP_PATH_DOS, wxFILE_SEP_PATH_DOS, volume);
            }
        }
        else if  ( format == wxPATH_DOS || format == wxPATH_VMS )
        {
            path += volume + wxFileName::GetVolumeSeparator(format);
        }
        // else ignore
    }

    return path;
}

// return true if the character is a DOS path separator i.e. either a slash or
// a backslash
inline bool IsDOSPathSep(wxUniChar ch)
{
    return ch == wxFILE_SEP_PATH_DOS || ch == wxFILE_SEP_PATH_UNIX;
}

// return true if the format used is the DOS/Windows one and the string looks
// like a UNC path
static bool IsUNCPath(const std::string& path, wxPathFormat format)
{
    return format == wxPATH_DOS &&
                path.length() >= 4 && // "\\a" can't be a UNC path
                    IsDOSPathSep(path[0u]) &&
                        IsDOSPathSep(path[1u]) &&
                            !IsDOSPathSep(path[2u]);
}

// Under Unix-ish systems (basically everything except Windows but we can't
// just test for non-__WIN32__ because Cygwin defines it, yet we want to use
// lstat() under it, so test for all the rest explicitly) we may work either
// with the file itself or its target if it's a symbolic link and we should
// dereference it, as determined by wxFileName::ShouldFollowLink() and the
// absence of the wxFILE_EXISTS_NO_FOLLOW flag. StatAny() can be used to stat
// the appropriate file with an extra twist that it also works when there is no
// wxFileName object at all, as is the case in static methods.

#if defined(__UNIX_LIKE__) || defined(__WXMAC__)
    #define wxHAVE_LSTAT
#endif

#ifdef wxHAVE_LSTAT

// Private implementation, don't call directly, use one of the overloads below.
bool DoStatAny(wxStructStat& st, std::string path, bool dereference)
{
    // We need to remove any trailing slashes from the path because they could
    // interfere with the symlink following decision: even if we use lstat(),
    // it would still follow the symlink if we pass it a path with a slash at
    // the end because the symlink resolution would happen while following the
    // path and not for the last path element itself.

    while ( wxEndsWithPathSeparator(path) )
    {
        const size_t posLast = path.length() - 1;
        if ( !posLast )
        {
            // Don't turn "/" into empty string.
            break;
        }

        path.erase(posLast);
    }

    int ret = dereference ? wxStat(path, &st) : wxLstat(path, &st);
    return ret == 0;
}

// Overloads to use for a case when we don't have wxFileName object and when we
// do have one.
inline
bool StatAny(wxStructStat& st, const std::string& path, unsigned int flags)
{
    return DoStatAny(st, path, !(flags & wxFILE_EXISTS_NO_FOLLOW));
}

inline
bool StatAny(wxStructStat& st, const wxFileName& fn)
{
    return DoStatAny(st, fn.GetFullPath(), fn.ShouldFollowLink());
}

#endif // wxHAVE_LSTAT

// ----------------------------------------------------------------------------
// private constants
// ----------------------------------------------------------------------------

// length of \\?\Volume{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}\ string
// FIXME: calculate from actual string
constexpr size_t wxMSWUniqueVolumePrefixLength = 49;

} // anonymous namespace

void wxFileName::Assign( const wxFileName &filepath )
{
    m_volume = filepath.GetVolume();
    m_dirs = filepath.GetDirs();
    m_name = filepath.GetName();
    m_ext = filepath.GetExt();
    m_relative = filepath.m_relative;
    m_hasExt = filepath.m_hasExt;
    m_dontFollowLinks = filepath.m_dontFollowLinks;
}

void wxFileName::Assign(const std::string& volume,
                        const std::string& path,
                        const std::string& name,
                        const std::string& ext,
                        bool hasExt,
                        wxPathFormat format)
{
    // we should ignore paths which look like UNC shares because we already
    // have the volume here and the UNC notation (\\server\path) is only valid
    // for paths which don't start with a volume, so prevent SetPath() from
    // recognizing "\\foo\bar" in "c:\\foo\bar" as an UNC path
    //
    // note also that this is a rather ugly way to do what we want (passing
    // some kind of flag telling to ignore UNC paths to SetPath() would be
    // better) but this is the safest thing to do to avoid breaking backwards
    // compatibility in 2.8
    if ( IsUNCPath(path, format) )
    {
        // remove one of the 2 leading backslashes to ensure that it's not
        // recognized as an UNC path by SetPath()
        std::string pathNonUNC(path, 1, std::string::npos);
        SetPath(pathNonUNC, format);
    }
    else // no UNC complications
    {
        SetPath(path, format);
    }

    m_volume = volume;
    m_ext = ext;
    m_name = name;

    m_hasExt = hasExt;
}

void wxFileName::SetPath( const std::string& pathOrig, wxPathFormat format )
{
    m_dirs.clear();

    if ( pathOrig.empty() )
    {
        // no path at all
        m_relative = true;

        return;
    }

    format = GetFormat( format );

    // 0) deal with possible volume part first
    std::string volume,
             path;
    SplitVolume(pathOrig, &volume, &path, format);
    if ( !volume.empty() )
    {
        m_relative = false;

        SetVolume(volume);
    }

    // 1) Determine if the path is relative or absolute.

    if ( path.empty() )
    {
        // we had only the volume
        return;
    }

    const char leadingChar = path[0u];

    switch (format)
    {
        case wxPATH_MAC:
            m_relative = leadingChar == ':';

            // We then remove a leading ":". The reason is in our
            // storage form for relative paths:
            // ":dir:file.txt" actually means "./dir/file.txt" in
            // DOS notation and should get stored as
            // (relative) (dir) (file.txt)
            // "::dir:file.txt" actually means "../dir/file.txt"
            // stored as (relative) (..) (dir) (file.txt)
            // This is important only for the Mac as an empty dir
            // actually means <UP>, whereas under DOS, double
            // slashes can be ignored: "\\\\" is the same as "\\".
            if (m_relative)
                path.erase( 0, 1 );
            break;

        case wxPATH_VMS:
            // TODO: what is the relative path format here?
            m_relative = false;
            break;

        default:
            wxFAIL_MSG( "Unknown path format" );
            [[fallthrough]];

        case wxPATH_UNIX:
            m_relative = leadingChar != '/';
            break;

        case wxPATH_DOS:
            m_relative = !IsPathSeparator(leadingChar, format);
            break;

    }

    // 2) Break up the path into its members. If the original path
    //    was just "/" or "\\", m_dirs will be empty. We know from
    //    the m_relative field, if this means "nothing" or "root dir".

    wxStringTokenizer tn( path, GetPathSeparators(format) );

    while ( tn.HasMoreTokens() )
    {
        std::string token = tn.GetNextToken();

        // Remove empty token under DOS and Unix, interpret them
        // as .. under Mac.
        if (token.empty())
        {
            if (format == wxPATH_MAC)
                m_dirs.push_back( ".." );
            // else ignore
        }
        else
        {
           m_dirs.push_back( token );
        }
    }
}

void wxFileName::Assign(const std::string& fullpath,
                        wxPathFormat format)
{
    std::string volume, path, name, ext;
    bool hasExt;
    SplitPath(fullpath, &volume, &path, &name, &ext, &hasExt, format);

    Assign(volume, path, name, ext, hasExt, format);
}

void wxFileName::Assign(const std::string& fullpathOrig,
                        const std::string& fullname,
                        wxPathFormat format)
{
    // always recognize fullpath as directory, even if it doesn't end with a
    // slash
    std::string fullpath = fullpathOrig;
    if ( !fullpath.empty() && !wxEndsWithPathSeparator(fullpath) )
    {
        fullpath += GetPathSeparator(format);
    }

    std::string volume, path, name, ext;
    bool hasExt;

    // do some consistency checks: the name should be really just the filename
    // and the path should be really just a path
    std::string volDummy, pathDummy, nameDummy, extDummy;

    SplitPath(fullname, &volDummy, &pathDummy, &name, &ext, &hasExt, format);

    wxASSERT_MSG( volDummy.empty() && pathDummy.empty(),
                  "the file name shouldn't contain the path" );

    SplitPath(fullpath, &volume, &path, &nameDummy, &extDummy, format);

#ifndef __VMS
   // This test makes no sense on an OpenVMS system.
   wxASSERT_MSG( nameDummy.empty() && extDummy.empty(),
                  "the path shouldn't contain file name nor extension" );
#endif
    Assign(volume, path, name, ext, hasExt, format);
}

void wxFileName::Assign(const std::string& pathOrig,
                        const std::string& name,
                        const std::string& ext,
                        wxPathFormat format)
{
    std::string volume,
             path;
    SplitVolume(pathOrig, &volume, &path, format);

    Assign(volume, path, name, ext, format);
}

void wxFileName::AssignDir(const std::string& dir, wxPathFormat format)
{
    Assign(dir, {}, format);
}

void wxFileName::Clear()
{
    m_dirs.clear();
    m_volume.clear();
    m_name.clear();
    m_ext.clear();

    // we don't have any absolute path for now
    m_relative = true;

    // nor any extension
    m_hasExt = false;

    // follow symlinks by default
    m_dontFollowLinks = false;
}

/* static */
wxFileName wxFileName::FileName(const std::string& file, wxPathFormat format)
{
    return {file, format};
}

/* static */
wxFileName wxFileName::DirName(const std::string& dir, wxPathFormat format)
{
    wxFileName fn;
    fn.AssignDir(dir, format);
    return fn;
}

// ----------------------------------------------------------------------------
// existence tests
// ----------------------------------------------------------------------------

namespace
{

#if defined(WX_WINDOWS)

void RemoveTrailingSeparatorsFromPath(std::string& strPath)
{
    // Windows fails to find directory named "c:\dir\" even if "c:\dir" exists,
    // so remove all trailing backslashes from the path - but don't do this for
    // the paths "d:\" (which are different from "d:"), for just "\" or for
    // windows unique volume names ("\\?\Volume{GUID}\")
    while ( wxEndsWithPathSeparator( strPath ) )
    {
        const size_t len = strPath.length();
        if ( len == 1 || (len == 3 && strPath[len - 2] == wxT(':')) ||
                (len == wxMSWUniqueVolumePrefixLength &&
                 wxFileName::IsMSWUniqueVolumeNamePath(strPath)))
        {
            break;
        }

        strPath.pop_back();
    }
}

#endif // __WINDOWS_

bool
wxFileSystemObjectExists(const std::string& path, unsigned int flags)
{

    // Should the existence of file/directory with this name be accepted, i.e.
    // result in the true return value from this function?
    const bool acceptFile = (flags & wxFILE_EXISTS_REGULAR) != 0;
    const bool acceptDir  = (flags & wxFILE_EXISTS_DIR)  != 0;

    std::string strPath(path);

#if defined(WX_WINDOWS)
    if ( acceptDir )
    {
        // Ensure that the path doesn't have any trailing separators when
        // checking for directories.
        RemoveTrailingSeparatorsFromPath(strPath);
    }

    // we must use GetFileAttributes() instead of the ANSI C functions because
    // it can cope with network (UNC) paths unlike them
    boost::nowide::wstackstring stackStrPath{strPath.c_str()};
    const DWORD ret = ::GetFileAttributesW(stackStrPath.get());

    if ( ret == INVALID_FILE_ATTRIBUTES )
        return false;

    if ( ret & FILE_ATTRIBUTE_DIRECTORY )
        return acceptDir;

    // Anything else must be a file (perhaps we should check for
    // FILE_ATTRIBUTE_REPARSE_POINT?)
    return acceptFile;
#else // Non-MSW
    wxStructStat st;
    if ( !StatAny(st, strPath, flags) )
        return false;

    if ( S_ISREG(st.st_mode) )
        return acceptFile;
    if ( S_ISDIR(st.st_mode) )
        return acceptDir;
    if ( S_ISLNK(st.st_mode) )
    {
        // Take care to not test for "!= 0" here as this would erroneously
        // return true if only wxFILE_EXISTS_NO_FOLLOW, which is part of
        // wxFILE_EXISTS_SYMLINK, is set too.
        return (flags & wxFILE_EXISTS_SYMLINK) == wxFILE_EXISTS_SYMLINK;
    }
    if ( S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode) )
        return (flags & wxFILE_EXISTS_DEVICE) != 0;
    if ( S_ISFIFO(st.st_mode) )
        return (flags & wxFILE_EXISTS_FIFO) != 0;
    if ( S_ISSOCK(st.st_mode) )
        return (flags & wxFILE_EXISTS_SOCKET) != 0;

    return flags & wxFILE_EXISTS_ANY;
#endif // Platforms
}

} // anonymous namespace

bool wxFileName::FileExists() const
{
    unsigned int flags = wxFILE_EXISTS_REGULAR;
    if ( !ShouldFollowLink() )
        flags |= wxFILE_EXISTS_NO_FOLLOW;

    return wxFileSystemObjectExists(GetFullPath(), flags);
}

/* static */
bool wxFileName::FileExists( const std::string &filePath )
{
    return wxFileSystemObjectExists(filePath, wxFILE_EXISTS_REGULAR);
}

bool wxFileName::DirExists() const
{
    unsigned int flags = wxFILE_EXISTS_DIR;
    if ( !ShouldFollowLink() )
        flags |= wxFILE_EXISTS_NO_FOLLOW;

    return Exists(GetPath(), flags);
}

/* static */
bool wxFileName::DirExists( const std::string &dirPath )
{
    return wxFileSystemObjectExists(dirPath, wxFILE_EXISTS_DIR);
}

bool wxFileName::Exists(unsigned int flags) const
{
    // Notice that wxFILE_EXISTS_NO_FOLLOW may be specified in the flags even
    // if our DontFollowLink() hadn't been called and we do honour it then. But
    // if the user took the care of calling DontFollowLink(), it is always
    // taken into account.
    if ( !ShouldFollowLink() )
        flags |= wxFILE_EXISTS_NO_FOLLOW;

    return wxFileSystemObjectExists(GetFullPath(), flags);
}

/* static */
bool wxFileName::Exists(const std::string& path, unsigned int flags)
{
    return wxFileSystemObjectExists(path, flags);
}

// ----------------------------------------------------------------------------
// CWD and HOME stuff
// ----------------------------------------------------------------------------

void wxFileName::AssignCwd(const std::string& volume)
{
    AssignDir(wxFileName::GetCwd(volume));
}

/* static */
std::string wxFileName::GetCwd(const std::string& volume)
{
    // if we have the volume, we must get the current directory on this drive
    // and to do this we have to chdir to this volume - at least under Windows,
    // I don't know how to get the current drive on another volume elsewhere
    // (TODO)
    std::string cwdOld;
    if ( !volume.empty() )
    {
        cwdOld = wxGetCwd();
        SetCwd(volume + GetVolumeSeparator());
    }

    std::string cwd = ::wxGetCwd();

    if ( !volume.empty() )
    {
        SetCwd(cwdOld);
    }

    return cwd;
}

bool wxFileName::SetCwd() const
{
    return wxFileName::SetCwd( GetPath() );
}

bool wxFileName::SetCwd( const std::string &cwd )
{
    return ::wxSetWorkingDirectory( cwd );
}

void wxFileName::AssignHomeDir()
{
    AssignDir(wxFileName::GetHomeDir());
}

std::string wxFileName::GetHomeDir()
{
    return ::wxGetHomeDir();
}


// ----------------------------------------------------------------------------
// CreateTempFileName
// ----------------------------------------------------------------------------

#if wxUSE_FILE || wxUSE_FFILE


#if !defined wx_fdopen && defined HAVE_FDOPEN
    #define wx_fdopen fdopen
#endif

// NB: GetTempFileName() under Windows creates the file, so using
//     O_EXCL there would fail
#ifdef WX_WINDOWS
    #define wxOPEN_EXCL 0
#else
    #define wxOPEN_EXCL O_EXCL
#endif


#ifdef wxOpenOSFHandle
#define WX_HAVE_DELETE_ON_CLOSE
// On Windows create a file with the FILE_FLAGS_DELETE_ON_CLOSE flags.
//
static int wxOpenWithDeleteOnClose(const std::string& filename)
{
    static constexpr DWORD access = GENERIC_READ | GENERIC_WRITE;

    static constexpr DWORD disposition = OPEN_ALWAYS;

    static constexpr DWORD attributes = FILE_ATTRIBUTE_TEMPORARY |
                       FILE_FLAG_DELETE_ON_CLOSE;

    boost::nowide::wstackstring stackFileName{filename.c_str()};
    HANDLE h = ::CreateFileW(stackFileName.get(), access, 0, nullptr,
                            disposition, attributes, nullptr);

    return wxOpenOSFHandle(h, wxO_BINARY);
}
#endif // wxOpenOSFHandle


// Helper to open the file
//
static int wxTempOpen(const std::string& path, bool *deleteOnClose)
{
#ifdef WX_HAVE_DELETE_ON_CLOSE
    if (*deleteOnClose)
        return wxOpenWithDeleteOnClose(path);
#endif

    *deleteOnClose = false;

    return wxOpen(path, wxO_BINARY | O_RDWR | O_CREAT | wxOPEN_EXCL, 0600);
}


#if wxUSE_FFILE
// Helper to open the file and attach it to the wxFFile
//
static bool wxTempOpen(wxFFile *file, const std::string& path, bool *deleteOnClose)
{
#ifndef wx_fdopen
    *deleteOnClose = false;
    return file->Open(path, "w+b");
#else // wx_fdopen
    const int fd = wxTempOpen(path, deleteOnClose);
    if (fd == -1)
        return false;
    file->Attach(wx_fdopen(fd, "w+b"), path);
    return file->IsOpened();
#endif // wx_fdopen
}
#endif // wxUSE_FFILE


#if !wxUSE_FILE
    #define WXFILEARGS(x, y) y
#elif !wxUSE_FFILE
    #define WXFILEARGS(x, y) x
#else
    #define WXFILEARGS(x, y) x, y
#endif


// Implementation of wxFileName::CreateTempFileName().
//
static std::string wxCreateTempImpl(
        const std::string& prefix,
        WXFILEARGS(wxFile *fileTemp, wxFFile *ffileTemp),
        bool *deleteOnClose = nullptr)
{
#if wxUSE_FILE && wxUSE_FFILE
    wxASSERT(fileTemp == nullptr || ffileTemp == nullptr);
#endif
    bool wantDeleteOnClose = false;

    if (deleteOnClose)
    {
        // set the result to false initially
        wantDeleteOnClose = *deleteOnClose;
        *deleteOnClose = false;
    }
    else
    {
        // easier if it alwasys points to something
        deleteOnClose = &wantDeleteOnClose;
    }

    std::string dir;
    std::string name;
    
    // use the directory specified by the prefix
    wxFileName::SplitPath(prefix, &dir, &name, nullptr /* extension */);

    if (dir.empty())
    {
        dir = wxFileName::GetTempDir();
    }

    std::string path;

#if defined(WX_WINDOWS)
    boost::nowide::wstackstring stackDir{dir.c_str()};
    boost::nowide::wstackstring stackName{name.c_str()};
    
    std::wstring wPath;
    wPath.resize(MAX_PATH + 1);
    
    if (!::GetTempFileNameW(stackDir.get(), stackName.get(), 0,
                            &wPath[0]))
    {
        wxLogLastError("GetTempFileName");
    }
    path = boost::nowide::narrow(wPath);
#else // !Windows
    std::string path = dir;

    if ( !wxEndsWithPathSeparator(dir) &&
            (name.empty() || !wxIsPathSeparator(name[0u])) )
    {
        path += wxFILE_SEP_PATH;
    }

    path += name;

#if defined(HAVE_MKSTEMP)
    // scratch space for mkstemp()
    path += "XXXXXX";

    // we need to copy the path to the buffer in which mkstemp() can modify it
    wxCharBuffer buf(path.fn_str());

    // cast is safe because the string length doesn't change
    int fdTemp = mkstemp(const_cast<char*>(static_cast<const char*>(buf)));
    if ( fdTemp == -1 )
    {
        // this might be not necessary as mkstemp() on most systems should have
        // already done it but it doesn't hurt neither...
        path.clear();
    }
    else // mkstemp() succeeded
    {
        path = wxConvFile.cMB2WX( (const char*) buf );

    #if wxUSE_FILE
        // avoid leaking the fd
        if ( fileTemp )
        {
            fileTemp->Attach(fdTemp);
        }
        else
    #endif

    #if wxUSE_FFILE
        if ( ffileTemp )
        {
        #ifdef wx_fdopen
            ffileTemp->Attach(wx_fdopen(fdTemp, "r+b"), path);
        #else
            ffileTemp->Open(path, "r+b");
            close(fdTemp);
        #endif
        }
        else
    #endif

        {
            close(fdTemp);
        }
    }
#else // !HAVE_MKSTEMP

#ifdef HAVE_MKTEMP
    // same as above
    path += "XXXXXX";

    wxCharBuffer buf = wxConvFile.cWX2MB( path );
    if ( !mktemp( (char*)(const char*) buf ) )
    {
        path.clear();
    }
    else
    {
        path = wxConvFile.cMB2WX( (const char*) buf );
    }
#else // !HAVE_MKTEMP
    // generate the unique file name ourselves
    path << (unsigned int)getpid();

    std::string pathTry;

    static constexpr size_t numTries = 1000;
    for ( size_t n = 0; n < numTries; n++ )
    {
        // 3 hex digits is enough for numTries == 1000 < 4096
        pathTry = path + fmt::format("%.03x", (unsigned int) n);
        if ( !wxFileName::FileExists(pathTry) )
        {
            break;
        }

        pathTry.clear();
    }

    path = pathTry;
#endif // HAVE_MKTEMP/!HAVE_MKTEMP

#endif // HAVE_MKSTEMP/!HAVE_MKSTEMP

#endif // Windows/!Windows

    if ( path.empty() )
    {
        wxLogSysError(_("Failed to create a temporary file name"));
    }
    else
    {
        bool ok = true;

        // open the file - of course, there is a race condition here, this is
        // why we always prefer using mkstemp()...
    #if wxUSE_FILE
        if ( fileTemp && !fileTemp->IsOpened() )
        {
            *deleteOnClose = wantDeleteOnClose;
            int fd = wxTempOpen(path, deleteOnClose);
            if (fd != -1)
                fileTemp->Attach(fd);
            else
                ok = false;
        }
    #endif

    #if wxUSE_FFILE
        if ( ffileTemp && !ffileTemp->IsOpened() )
        {
            *deleteOnClose = wantDeleteOnClose;
            ok = wxTempOpen(ffileTemp, path, deleteOnClose);
        }
    #endif

        if ( !ok )
        {
            // FIXME: If !ok here should we loop and try again with another
            //        file name?  That is the standard recourse if open(O_EXCL)
            //        fails, though of course it should be protected against
            //        possible infinite looping too.

            wxLogError(_("Failed to open temporary file."));

            path.clear();
        }
    }

    return path;
}


static bool wxCreateTempImpl(
        const std::string& prefix,
        WXFILEARGS(wxFile *fileTemp, wxFFile *ffileTemp),
        std::string *name)
{
    bool deleteOnClose = true;

    *name = wxCreateTempImpl(prefix,
                             WXFILEARGS(fileTemp, ffileTemp),
                             &deleteOnClose);

    const bool ok = !name->empty();

    if (deleteOnClose)
        name->clear();
#ifdef __UNIX__
    else if (ok && wxRemoveFile(*name))
        name->clear();
#endif

    return ok;
}


static void wxAssignTempImpl(
        wxFileName *fn,
        const std::string& prefix,
        WXFILEARGS(wxFile *fileTemp, wxFFile *ffileTemp))
{
    std::string tempname;
    tempname = wxCreateTempImpl(prefix, WXFILEARGS(fileTemp, ffileTemp));

    if ( tempname.empty() )
    {
        // error, failed to get temp file name
        fn->Clear();
    }
    else // ok
    {
        fn->Assign(tempname);
    }
}


void wxFileName::AssignTempFileName(const std::string& prefix)
{
    wxAssignTempImpl(this, prefix, WXFILEARGS(nullptr, nullptr));
}

/* static */
std::string wxFileName::CreateTempFileName(const std::string& prefix)
{
    return wxCreateTempImpl(prefix, WXFILEARGS(nullptr, nullptr));
}

#endif // wxUSE_FILE || wxUSE_FFILE


#if wxUSE_FILE

std::string wxCreateTempFileName(const std::string& prefix,
                              wxFile *fileTemp,
                              bool *deleteOnClose)
{
    return wxCreateTempImpl(prefix, WXFILEARGS(fileTemp, nullptr), deleteOnClose);
}

bool wxCreateTempFile(const std::string& prefix,
                      wxFile *fileTemp,
                      std::string *name)
{
    return wxCreateTempImpl(prefix, WXFILEARGS(fileTemp, nullptr), name);
}

void wxFileName::AssignTempFileName(const std::string& prefix, wxFile *fileTemp)
{
    wxAssignTempImpl(this, prefix, WXFILEARGS(fileTemp, nullptr));
}

/* static */
std::string
wxFileName::CreateTempFileName(const std::string& prefix, wxFile *fileTemp)
{
    return wxCreateTempFileName(prefix, fileTemp);
}

#endif // wxUSE_FILE


#if wxUSE_FFILE

std::string wxCreateTempFileName(const std::string& prefix,
                              wxFFile *fileTemp,
                              bool *deleteOnClose)
{
    return wxCreateTempImpl(prefix, WXFILEARGS(nullptr, fileTemp), deleteOnClose);
}

bool wxCreateTempFile(const std::string& prefix,
                      wxFFile *fileTemp,
                      std::string *name)
{
    return wxCreateTempImpl(prefix, WXFILEARGS(nullptr, fileTemp), name);

}

void wxFileName::AssignTempFileName(const std::string& prefix, wxFFile *fileTemp)
{
    wxAssignTempImpl(this, prefix, WXFILEARGS(nullptr, fileTemp));
}

/* static */
std::string
wxFileName::CreateTempFileName(const std::string& prefix, wxFFile *fileTemp)
{
    return wxCreateTempFileName(prefix, fileTemp);
}

#endif // wxUSE_FFILE


// ----------------------------------------------------------------------------
// directory operations
// ----------------------------------------------------------------------------

// helper of GetTempDir(): check if the given directory exists and return it if
// it does or an empty string otherwise
namespace
{

std::string CheckIfDirExists(const std::string& dir)
{
    return wxFileName::DirExists(dir) ? dir : std::string();
}

} // anonymous namespace

std::string wxFileName::GetTempDir()
{
    // first try getting it from environment: this allows overriding the values
    // used by default if the user wants to create temporary files in another
    // directory
    std::string dir = CheckIfDirExists(wxGetenv("TMPDIR"));
    if ( dir.empty() )
    {
        dir = CheckIfDirExists(wxGetenv("TMP"));
        if ( dir.empty() )
            dir = CheckIfDirExists(wxGetenv("TEMP"));
    }

    // if no environment variables are set, use the system default
    if ( dir.empty() )
    {
#if defined(WX_WINDOWS)
        boost::nowide::wstackstring stackDir;
        if ( !::GetTempPathW(MAX_PATH, stackDir.get()) )
        {
            wxLogLastError("GetTempPath");
        }

        dir = boost::nowide::narrow(stackDir.get());
#endif // systems with native way
    }

    if ( !dir.empty() )
    {
        // remove any trailing path separators, we don't want to ever return
        // them from this function for consistency
        const size_t lastNonSep = dir.find_last_not_of(GetPathSeparators());
        if ( lastNonSep == std::string::npos )
        {
            // the string consists entirely of separators, leave only one
            dir = GetPathSeparator();
        }
        else
        {
            dir.erase(lastNonSep + 1);
        }
    }

    // fall back to hard coded value
    else
    {
#ifdef __UNIX_LIKE__
        dir = CheckIfDirExists("/tmp");
        if ( dir.empty() )
#endif // __UNIX_LIKE__
            dir = ".";
    }

    return dir;
}

bool wxFileName::Mkdir( int perm, unsigned int flags ) const
{
    return wxFileName::Mkdir(GetPath(), perm, flags);
}

bool wxFileName::Mkdir( const std::string& dir, int perm, unsigned int flags )
{
    if ( flags & wxPATH_MKDIR_FULL )
    {
        // split the path in components
        wxFileName filename;
        filename.AssignDir(dir);

        std::string currPath;
        if ( filename.HasVolume())
        {
            currPath += wxGetVolumeString(filename.GetVolume(), wxPATH_NATIVE);
        }

        std::vector<std::string> dirs = filename.GetDirs();

        for ( size_t i = 0; i < dirs.size(); i++ )
        {
            if ( i > 0 || filename.IsAbsolute() )
                currPath += wxFILE_SEP_PATH;
            currPath += dirs[i];

            if (!DirExists(currPath))
            {
                if (!wxMkdir(currPath, perm))
                {
                    // no need to try creating further directories
                    return false;
                }
            }
        }

        return true;

    }

    return ::wxMkdir( dir, perm );
}

bool wxFileName::Rmdir(unsigned int flags) const
{
    return wxFileName::Rmdir( GetPath(), flags );
}

bool wxFileName::Rmdir(const std::string& dir, unsigned int flags)
{
#ifdef WX_WINDOWS
    if ( flags & wxPATH_RMDIR_RECURSIVE )
    {
        // SHFileOperation needs double null termination string
        // but without separator at the end of the path
        std::string path(dir);
        if ( path.back() == wxFILE_SEP_PATH )
            path.pop_back();
        path += '\0';

        SHFILEOPSTRUCTW fileop;
        wxZeroMemory(fileop);

        boost::nowide::wstackstring stackPath{path.c_str()};
        fileop.wFunc = FO_DELETE;
        fileop.pFrom = stackPath.get();
        fileop.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
        fileop.fFlags |= FOF_NOERRORUI;

        const int ret = ::SHFileOperationW(&fileop);
        if ( ret != 0 )
        {
            // SHFileOperation may return non-Win32 error codes, so the error
            // message can be incorrect
            wxLogApiError("SHFileOperation", ret);
            return false;
        }

        return true;
    }
    else if ( flags & wxPATH_RMDIR_FULL )
#else // !WX_WINDOWS
    if ( flags != 0 )   // wxPATH_RMDIR_FULL or wxPATH_RMDIR_RECURSIVE
#endif // !WX_WINDOWS
    {
#ifndef WX_WINDOWS
        if ( flags & wxPATH_RMDIR_RECURSIVE )
        {
            // When deleting the tree recursively, we are supposed to delete
            // this directory itself even when it is a symlink -- but without
            // following it. Do it here as wxRmdir() would simply follow if
            // called for a symlink.
            if ( wxFileName::Exists(dir, wxFILE_EXISTS_SYMLINK) )
            {
                return wxRemoveFile(dir);
            }
        }
#endif // !WX_WINDOWS

        std::string path(dir);
        if ( path.back() != wxFILE_SEP_PATH )
            path += wxFILE_SEP_PATH;

        wxDir d(path);

        if ( !d.IsOpened() )
            return false;

        std::string filename;

        // First delete all subdirectories: notice that we don't follow
        // symbolic links, potentially leading outside this directory, to avoid
        // unpleasant surprises.
        bool cont = d.GetFirst(&filename, std::string{},
                               wxDIR_DIRS | wxDIR_HIDDEN | wxDIR_NO_FOLLOW);
        while ( cont )
        {
            wxFileName::Rmdir(path + filename, flags);
            cont = d.GetNext(&filename);
        }

#ifndef WX_WINDOWS
        if ( flags & wxPATH_RMDIR_RECURSIVE )
        {
            // Delete all files too and, for the same reasons as above, don't
            // follow symlinks which could refer to the files outside of this
            // directory and just delete the symlinks themselves.
            cont = d.GetFirst(&filename, std::string(),
                              wxDIR_FILES | wxDIR_HIDDEN | wxDIR_NO_FOLLOW);
            while ( cont )
            {
                ::wxRemoveFile(path + filename);
                cont = d.GetNext(&filename);
            }
        }
#endif // !WX_WINDOWS
    }

    return ::wxRmdir(dir);
}

// ----------------------------------------------------------------------------
// path normalization
// ----------------------------------------------------------------------------

bool wxFileName::Normalize(unsigned int flags,
                           const std::string& cwd,
                           wxPathFormat format)
{
    // deal with env vars renaming first as this may seriously change the path
    if ( flags & wxPATH_NORM_ENV_VARS )
    {
        std::string pathOrig = GetFullPath(format);
        std::string path = wxExpandEnvVars(pathOrig);
        if ( path != pathOrig )
        {
            Assign(path);
        }
    }

    // the existing path components
    std::vector<std::string> dirs = GetDirs();

    // the path to prepend in front to make the path absolute
    wxFileName curDir;

    format = GetFormat(format);

    // set up the directory to use for making the path absolute later
    if ( (flags & wxPATH_NORM_ABSOLUTE) && !IsAbsolute(format) )
    {
        if ( cwd.empty() )
        {
            curDir.AssignCwd(GetVolume());
        }
        else // cwd provided
        {
            curDir.AssignDir(cwd);
        }
    }

    // handle ~ stuff under Unix only
    if ( (format == wxPATH_UNIX) && (flags & wxPATH_NORM_TILDE) && m_relative )
    {
        if ( !dirs.empty() )
        {
            std::string dir = dirs[0u];
            if ( !dir.empty() && dir[0u] == wxT('~') )
            {
                // to make the path absolute use the home directory
                curDir.AssignDir(wxGetUserHome(dir.c_str() + 1));
                dirs.erase(std::begin(dirs));
            }
        }
    }

    // transform relative path into abs one
    if ( curDir.IsOk() )
    {
        // this path may be relative because it doesn't have the volume name
        // and still have m_relative=true; in this case we shouldn't modify
        // our directory components but just set the current volume
        if ( !HasVolume() && curDir.HasVolume() )
        {
            SetVolume(curDir.GetVolume());

            if ( !m_relative )
        {
                // yes, it was the case - we don't need curDir then
                curDir.Clear();
            }
        }

        // finally, prepend curDir to the dirs array
        std::vector<std::string> dirsNew = curDir.GetDirs();
        dirs.insert(dirs.begin(), dirsNew.begin(), dirsNew.end());

        // if we used e.g. tilde expansion previously and wxGetUserHome didn't
        // return for some reason an absolute path, then curDir maybe not be absolute!
        if ( !curDir.m_relative )
        {
            // we have prepended an absolute path and thus we are now an absolute
            // file name too
            m_relative = false;
        }
        // else if (flags & wxPATH_NORM_ABSOLUTE):
        //   should we warn the user that we didn't manage to make the path absolute?
    }

    // now deal with ".", ".." and the rest
    m_dirs.clear();

    size_t count = dirs.size();
    for ( size_t n = 0; n < count; n++ )
    {
        std::string dir = dirs[n];

        if ( flags & wxPATH_NORM_DOTS )
        {
            if ( dir == "." )
            {
                // just ignore
                continue;
            }

            if ( dir == ".." )
            {
                if ( m_dirs.empty() )
                {
                    // We have more ".." than directory components so far.
                    // Don't treat this as an error as the path could have been
                    // entered by user so try to handle it reasonably: if the
                    // path is absolute, just ignore the extra ".." because
                    // "/.." is the same as "/". Otherwise, i.e. for relative
                    // paths, keep ".." unchanged because removing it would
                    // modify the file a relative path refers to.
                    if ( !m_relative )
                        continue;

                }
                else // Normal case, go one step up unless it's .. as well.
                {
                    if (m_dirs.back() != ".." )
                    {
                        m_dirs.pop_back();
                        continue;
                    }
                }
            }
        }

        m_dirs.push_back(dir);
    }

#if defined(__WIN32__) && wxUSE_OLE
    if ( (flags & wxPATH_NORM_SHORTCUT) )
    {
        std::string filename;
        if (GetShortcutTarget(GetFullPath(format), filename))
        {
            m_relative = false;
            Assign(filename);
        }
    }
#endif

#if defined(__WIN32__)
    if ( (flags & wxPATH_NORM_LONG) && (format == wxPATH_DOS) )
    {
        Assign(GetLongPath());
    }
#endif // Win32

    // Change case  (this should be kept at the end of the function, to ensure
    // that the path doesn't change any more after we normalize its case)
    if ( (flags & wxPATH_NORM_CASE) && !IsCaseSensitive(format) )
    {
        wx::utils::ToLower(m_volume);
        wx::utils::ToLower(m_name);
        wx::utils::ToLower(m_ext);

        // directory entries must be made lower case as well
        count = m_dirs.size();
        for ( size_t i = 0; i < count; i++ )
        {
            wx::utils::ToLower(m_dirs[i]);
        }
    }

    return true;
}

bool wxFileName::ReplaceEnvVariable(const std::string& envname,
                                    const std::string& replacementFmtString,
                                    wxPathFormat format)
{
    // look into stringForm for the contents of the given environment variable
    std::string val;
    if (envname.empty() ||
        !wxGetEnv(envname, &val))
        return false;
    if (val.empty())
        return false;

    std::string stringForm = GetPath(wxPATH_GET_VOLUME, format);
        // do not touch the file name and the extension

    // FIXME: What is format doing here?
    //std::string replacement = wxString::Format(replacementFmtString, envname);
    std::string replacement = fmt::format("{}{}", replacementFmtString, envname);

    wx::utils::ReplaceAll(stringForm, val, replacement);

    // Now assign ourselves the modified path:
    Assign(stringForm, GetFullName(), format);

    return true;
}

bool wxFileName::ReplaceHomeDir(wxPathFormat format)
{
    std::string homedir = wxGetHomeDir();
    if (homedir.empty())
        return false;

    std::string stringForm = GetPath(wxPATH_GET_VOLUME, format);
        // do not touch the file name and the extension

    wx::utils::ReplaceAll(stringForm, homedir, "~");

    // Now assign ourselves the modified path:
    Assign(stringForm, GetFullName(), format);

    return true;
}

// ----------------------------------------------------------------------------
// get the shortcut target
// ----------------------------------------------------------------------------

#if defined(__WIN32__) && wxUSE_OLE

bool wxFileName::GetShortcutTarget(const std::string& shortcutPath,
                                   std::string& targetFilename,
                                   std::string* arguments) const
{
    std::string path, file, ext;
    wxFileName::SplitPath(shortcutPath, & path, & file, & ext);

    wxCOMPtr<IShellLinkW> psl;
    bool success = false;

    // Assume it's not a shortcut if it doesn't end with lnk
    if (wx::utils::CmpNoCase(ext, "lnk")!=0)
        return false;

    // Ensure OLE is initialized.
    wxOleInitializer oleInit;

    // create a ShellLink object
    HRESULT hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID*) &psl);

    if (SUCCEEDED(hres))
    {
        wxCOMPtr<IPersistFile> ppf;
        hres = psl->QueryInterface( IID_IPersistFile, (LPVOID *) &ppf);
        if (SUCCEEDED(hres))
        {
            boost::nowide::wstackstring stackShortcutPath{shortcutPath.c_str()};

            hres = ppf->Load(stackShortcutPath.get(), 0);
            if (SUCCEEDED(hres))
            {
                boost::nowide::wstackstring stackBuf;
                //wxChar buf[2048];
                psl->GetPath(stackBuf.get(), 2048, nullptr, SLGP_UNCPRIORITY);
                targetFilename = boost::nowide::narrow(stackBuf.get());
                success = (shortcutPath != targetFilename);

                // FIXME: clear buffer?
                psl->GetArguments(stackBuf.get(), 2048);
                std::string args = boost::nowide::narrow(stackBuf.get());
                if (!args.empty() && arguments)
                {
                    *arguments = args;
                }
            }
        }
    }
    return success;
}

#endif // __WIN32__

// ----------------------------------------------------------------------------
// Resolve links
// ----------------------------------------------------------------------------

wxFileName wxFileName::ResolveLink()
{
    wxFileName linkTarget( *this );

// Only resolve links on platforms with readlink (e.g. Unix-like platforms)
#if defined(wxHAS_NATIVE_READLINK)
    const std::string link = GetFullPath();
    wxStructStat st;

    // This means the link itself doesn't exist, so return an empty filename
    if ( !StatAny(st, link, wxFILE_EXISTS_NO_FOLLOW) )
    {
        linkTarget.Clear();
        return linkTarget;
    }

    // If it isn't an actual link, bail out just and return the link as the result
    if ( !S_ISLNK(st.st_mode) )
        return linkTarget;

    // Dynamically compute the buffer size from the stat call, but fallback if it isn't valid
    int bufSize = 4096;
    if( st.st_size != 0 )
        bufSize = st.st_size + 1;

    std::vector<char> bufData(bufSize);
    char* const buf = &bufData[0];
    ssize_t result = wxReadlink(link, buf, bufSize - 1);

    if ( result != -1 )
    {
        buf[result] = '\0'; // readlink() doesn't NULL-terminate the buffer
        linkTarget.Assign( std::string(buf, wxConvLibc) );

        // Ensure the resulting path is absolute since readlink can return paths relative to the link
        if ( !linkTarget.IsAbsolute() )
            linkTarget.MakeAbsolute(GetPath());
    }
    else
    {
        // This means the lookup failed for some reason
        linkTarget.Clear();
    }
#endif

    return linkTarget;
}

// ----------------------------------------------------------------------------
// absolute/relative paths
// ----------------------------------------------------------------------------

bool wxFileName::IsAbsolute(wxPathFormat format) const
{
    // unix paths beginning with ~ are reported as being absolute
    if ( format == wxPATH_UNIX )
    {
        if ( !m_dirs.empty() )
        {
            std::string dir = m_dirs[0u];

            if (!dir.empty() && dir[0u] == wxT('~'))
                return true;
        }
    }

    // if our path doesn't start with a path separator, it's not an absolute
    // path
    if ( m_relative )
        return false;

    if ( !GetVolumeSeparator(format).empty() )
    {
        // this format has volumes and an absolute path must have one, it's not
        // enough to have the full path to be an absolute file under Windows
        if ( GetVolume().empty() )
            return false;
    }

    return true;
}

bool wxFileName::MakeRelativeTo(const std::string& pathBase, wxPathFormat format)
{
    wxFileName fnBase = wxFileName::DirName(pathBase, format);

    // get cwd only once - small time saving
    std::string cwd = wxGetCwd();

    // Normalize both paths to be absolute but avoid expanding environment
    // variables in them, this could be unexpected.
    static constexpr unsigned int normFlags = wxPATH_NORM_DOTS |
                                              wxPATH_NORM_TILDE |
                                              wxPATH_NORM_ABSOLUTE |
                                              wxPATH_NORM_LONG;
    Normalize(normFlags, cwd, format);
    fnBase.Normalize(normFlags, cwd, format);

    const bool withCase = IsCaseSensitive(format);

    // we can't do anything if the files live on different volumes
    if ( !wx::utils::IsSameAsCase(GetVolume(), fnBase.GetVolume()) )
    {
        // nothing done
        return false;
    }

    // same drive, so we don't need our volume
    m_volume.clear();

    // remove common directories starting at the top
    if(!m_dirs.empty() && !fnBase.m_dirs.empty())
    {
        const auto first_mismatching_dirs = std::mismatch(m_dirs.cbegin(), m_dirs.cend(), fnBase.m_dirs.cbegin(), fnBase.m_dirs.cend(),
            [withCase](const auto& dir, const auto& base_dir){
                return wx::utils::IsSameAs(dir, base_dir, withCase);
            });

        m_dirs.erase(std::begin(m_dirs), first_mismatching_dirs.first);
        fnBase.m_dirs.erase(std::begin(fnBase.m_dirs), first_mismatching_dirs.second);
    }

    // add as many ".." as needed
    size_t count = fnBase.m_dirs.size();
    for ( size_t i = 0; i < count; i++ )
    {
        m_dirs.insert(std::begin(m_dirs), "..");
    }

    switch ( GetFormat(format) )
    {
        case wxPATH_NATIVE:
        case wxPATH_MAX:
            wxFAIL_MSG( "unreachable" );
            [[fallthrough]];

        case wxPATH_UNIX:
        case wxPATH_DOS:
            // a directory made relative with respect to itself is '.' under
            // Unix and DOS, by definition (but we don't have to insert "./"
            // for the files)
            if ( m_dirs.empty() && IsDir() )
            {
                m_dirs.push_back(".");
            }
            break;

        case wxPATH_MAC:
        case wxPATH_VMS:
            break;
    }

    m_relative = true;

    // we were modified
    return true;
}

// ----------------------------------------------------------------------------
// filename kind tests
// ----------------------------------------------------------------------------

bool wxFileName::SameAs(const wxFileName& filepath, wxPathFormat format) const
{
    wxFileName fn1 = *this,
               fn2 = filepath;

    // get cwd only once - small time saving
    std::string cwd = wxGetCwd();
    fn1.Normalize(wxPATH_NORM_ALL | wxPATH_NORM_CASE, cwd, format);
    fn2.Normalize(wxPATH_NORM_ALL | wxPATH_NORM_CASE, cwd, format);

    if ( fn1.GetFullPath() == fn2.GetFullPath() )
        return true;

#ifdef wxHAVE_LSTAT
    wxStructStat st1, st2;
    if ( StatAny(st1, fn1) && StatAny(st2, fn2) )
    {
        if ( st1.st_ino == st2.st_ino && st1.st_dev == st2.st_dev )
            return true;
    }
    //else: It's not an error if one or both files don't exist.
#endif // wxHAVE_LSTAT

    return false;
}

/* static */
bool wxFileName::IsCaseSensitive( wxPathFormat format )
{
    // only Unix filenames are truly case-sensitive
    return GetFormat(format) == wxPATH_UNIX;
}

/* static */
std::string wxFileName::GetForbiddenChars(wxPathFormat format)
{
    // Inits to forbidden characters that are common to (almost) all platforms.
    std::string strForbiddenChars = "*?";

    // If asserts, wxPathFormat has been changed. In case of a new path format
    // addition, the following code might have to be updated.
    static_assert(wxPATH_MAX == 5, "Path format changed.");

    switch ( GetFormat(format) )
    {
        default :
            wxFAIL_MSG( "Unknown path format" );
            [[fallthrough]];

        case wxPATH_UNIX:
            break;

        case wxPATH_MAC:
            // On a Mac even names with * and ? are allowed (Tested with OS
            // 9.2.1 and OS X 10.2.5)
            strForbiddenChars.clear();
            break;

        case wxPATH_DOS:
            strForbiddenChars += "\\/:\"<>|";
            break;

        case wxPATH_VMS:
            break;
    }

    return strForbiddenChars;
}

/* static */
std::string wxFileName::GetVolumeSeparator(wxPathFormat format)
{
    std::string sepVol;

    if ( (GetFormat(format) == wxPATH_DOS) ||
         (GetFormat(format) == wxPATH_VMS) )
    {
        sepVol = wxFILE_SEP_DSK;
    }
    //else: leave empty

    return sepVol;
}

/* static */
std::string wxFileName::GetPathSeparators(wxPathFormat format)
{
    std::string seps;
    switch ( GetFormat(format) )
    {
        case wxPATH_DOS:
            // accept both as native APIs do but put the native one first as
            // this is the one we use in GetFullPath()
            seps += fmt::format("{}{}", wxFILE_SEP_PATH_DOS, wxFILE_SEP_PATH_UNIX);
            break;

        default:
            wxFAIL_MSG( "Unknown wxPATH_XXX style" );
            [[fallthrough]];

        case wxPATH_UNIX:
            seps = wxFILE_SEP_PATH_UNIX;
            break;

        case wxPATH_MAC:
            seps = wxFILE_SEP_PATH_MAC;
            break;

        case wxPATH_VMS:
            seps = wxFILE_SEP_PATH_VMS;
            break;
    }

    return seps;
}

/* static */
std::string wxFileName::GetPathTerminators(wxPathFormat format)
{
    format = GetFormat(format);

    // under VMS the end of the path is ']', not the path separator used to
    // separate the components
    return format == wxPATH_VMS ? "]" : GetPathSeparators(format);
}

/* static */
bool wxFileName::IsPathSeparator(char ch, wxPathFormat format)
{
    // std::string::Find() doesn't work as expected with NUL - it will always find
    // it, so test for it separately
    return ch != '\0' && GetPathSeparators(format).find(ch) != std::string::npos;
}

/* static */
bool
wxFileName::IsMSWUniqueVolumeNamePath(const std::string& path, wxPathFormat format)
{
    // return true if the format used is the DOS/Windows one and the string begins
    // with a Windows unique volume name ("\\?\Volume{guid}\")
    return format == wxPATH_DOS &&
            path.length() >= wxMSWUniqueVolumePrefixLength &&
             path.rfind("\\\\?\\Volume{", 0) == 0 &&
              path[wxMSWUniqueVolumePrefixLength - 1] == wxFILE_SEP_PATH_DOS;
}

// ----------------------------------------------------------------------------
// path components manipulation
// ----------------------------------------------------------------------------

/* static */ bool wxFileName::IsValidDirComponent(const std::string& dir)
{
    if ( dir.empty() )
    {
        wxFAIL_MSG( "empty directory passed to wxFileName::InsertDir()" );

        return false;
    }

    const size_t len = dir.length();
    for ( size_t n = 0; n < len; n++ )
    {
        if ( dir[n] == GetVolumeSeparator() || IsPathSeparator(dir[n]) )
        {
            wxFAIL_MSG( "invalid directory component in wxFileName" );

            return false;
        }
    }

    return true;
}

bool wxFileName::AppendDir( const std::string& dir )
{
    if (!IsValidDirComponent(dir))
        return false;
    m_dirs.push_back(dir);
    return true;
}

void wxFileName::PrependDir( const std::string& dir )
{
    InsertDir(0, dir);
}

bool wxFileName::InsertDir(size_t before, const std::string& dir)
{
    if (!IsValidDirComponent(dir))
        return false;
    m_dirs.insert(std::begin(m_dirs) + before, dir);
    return true;
}

void wxFileName::RemoveDir(size_t pos)
{
    m_dirs.erase(std::begin(m_dirs) + pos);
}

// ----------------------------------------------------------------------------
// accessors
// ----------------------------------------------------------------------------

void wxFileName::SetFullName(const std::string& fullname)
{
    SplitPath(fullname, nullptr /* no volume */, nullptr /* no path */,
                        &m_name, &m_ext, &m_hasExt);
}

std::string wxFileName::GetFullName() const
{
    std::string fullname = m_name;
    if ( m_hasExt )
    {
        fullname += fmt::format("{}{}", wxFILE_SEP_EXT, m_ext);
    }

    return fullname;
}

std::string wxFileName::GetPath( unsigned int flags, wxPathFormat format ) const
{
    format = GetFormat( format );

    std::string fullpath;

    // return the volume with the path as well if requested
    if ( flags & wxPATH_GET_VOLUME )
    {
        fullpath += wxGetVolumeString(GetVolume(), format);
    }

    // the leading character
    switch ( format )
    {
        case wxPATH_MAC:
            if ( m_relative )
                fullpath += wxFILE_SEP_PATH_MAC;
            break;

        case wxPATH_DOS:
            if ( !m_relative )
                fullpath += wxFILE_SEP_PATH_DOS;
            break;

        default:
            wxFAIL_MSG( "Unknown path format" );
            [[fallthrough]];

        case wxPATH_UNIX:
            if ( !m_relative )
            {
                fullpath += wxFILE_SEP_PATH_UNIX;
            }
            break;

        case wxPATH_VMS:
            // no leading character here but use this place to unset
            // wxPATH_GET_SEPARATOR flag: under VMS it doesn't make sense
            // as, if I understand correctly, there should never be a dot
            // before the closing bracket
            flags &= ~wxPATH_GET_SEPARATOR;
    }

    if ( m_dirs.empty() )
    {
        // there is nothing more
        return fullpath;
    }

    // then concatenate all the path components using the path separator
    if ( format == wxPATH_VMS )
    {
        fullpath += wxT('[');
    }

    const size_t dirCount = m_dirs.size();
    for ( size_t i = 0; i < dirCount; i++ )
    {
        switch (format)
        {
            case wxPATH_MAC:
                if ( m_dirs[i] == "." )
                {
                    // skip appending ':', this shouldn't be done in this
                    // case as "::" is interpreted as ".." under Unix
                    continue;
                }

                // convert back from ".." to nothing
                if ( !wx::utils::IsSameAsCase("..", m_dirs[i]) )
                     fullpath += m_dirs[i];
                break;

            default:
                wxFAIL_MSG( "Unexpected path format" );
                [[fallthrough]];

            case wxPATH_DOS:
            case wxPATH_UNIX:
                fullpath += m_dirs[i];
                break;

            case wxPATH_VMS:
                // TODO: What to do with ".." under VMS

                // convert back from ".." to nothing
                if ( !wx::utils::IsSameAsCase("..", m_dirs[i]) )
                    fullpath += m_dirs[i];
                break;
        }

        if ( (flags & wxPATH_GET_SEPARATOR) || (i != dirCount - 1) )
            fullpath += GetPathSeparator(format);
    }

    if ( format == wxPATH_VMS )
    {
        fullpath += wxT(']');
    }

    return fullpath;
}

std::string wxFileName::GetFullPath( wxPathFormat format ) const
{
    // we already have a function to get the path
    std::string fullpath = GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR,
                                format);

    // now just add the file name and extension to it
    fullpath += GetFullName();

    return fullpath;
}

// Return the short form of the path (returns identity on non-Windows platforms)
std::string wxFileName::GetShortPath() const
{
    std::string path = GetFullPath();

#if defined(WX_WINDOWS) && defined(__WIN32__)
    boost::nowide::wstackstring stackPath{path.c_str()};
    DWORD sz = ::GetShortPathNameW(stackPath.get(), nullptr, 0);
    if ( sz != 0 )
    {
        boost::nowide::wstackstring pathOut;
        if ( ::GetShortPathNameW
               (
                stackPath.get(),
                pathOut.get(),
                sz
               ) != 0 )
        {
            return boost::nowide::narrow(pathOut.get());
        }
    }
#endif // Windows

    return path;
}

// Return the long form of the path (returns identity on non-Windows platforms)
std::string wxFileName::GetLongPath() const
{
    std::string path = GetFullPath();

#if defined(__WIN32__)

    boost::nowide::wstackstring stackPath{path.c_str()};
    DWORD dwSize = ::GetLongPathNameW(stackPath.get(), nullptr, 0);
    if ( dwSize > 0 )
    {
        boost::nowide::wstackstring stackPathOut;
        if ( ::GetLongPathNameW
                (
                stackPath.get(),
                stackPathOut.get(),
                dwSize
                ) != 0 )
        {
            return boost::nowide::narrow(stackPathOut.get());
        }
    }

    // Some other error occured.
    // We need to call FindFirstFile on each component in turn.
    std::string pathOut;

    if ( HasVolume() )
        pathOut = GetVolume() +
                  GetVolumeSeparator(wxPATH_DOS) +
                  GetPathSeparator(wxPATH_DOS);

    std::vector<std::string> dirs = GetDirs();
    dirs.push_back(GetFullName());

    std::string tmpPath;

    size_t count = dirs.size();
    for ( size_t i = 0; i < count; i++ )
    {
        const std::string& dir = dirs[i];

        // We're using pathOut to collect the long-name path, but using a
        // temporary for appending the last path component which may be
        // short-name
        tmpPath = pathOut + dir;

        // We must not process "." or ".." here as they would be (unexpectedly)
        // replaced by the corresponding directory names so just leave them
        // alone
        //
        // And we can't pass a drive and root dir to FindFirstFile (VZ: why?)
        if ( tmpPath.empty() || dir == '.' || dir == ".." ||
                tmpPath.back() == GetVolumeSeparator(wxPATH_DOS) )
        {
            tmpPath += wxFILE_SEP_PATH;
            pathOut = tmpPath;
            continue;
        }

        WIN32_FIND_DATA findFileData;

        boost::nowide::wstackstring stackTmpPath{tmpPath.c_str()};

        HANDLE hFind = ::FindFirstFileW(stackTmpPath.get(), &findFileData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            // Error: most likely reason is that path doesn't exist, so
            // append any unprocessed parts and return
            for ( i += 1; i < count; i++ )
                tmpPath += wxFILE_SEP_PATH + dirs[i];

            return tmpPath;
        }

        pathOut += boost::nowide::narrow(findFileData.cFileName);
        if ( (i < (count-1)) )
            pathOut += wxFILE_SEP_PATH;

        ::FindClose(hFind);
    }
#else // !Win32
    pathOut = path;
#endif // Win32/!Win32

    return pathOut;
}

wxPathFormat wxFileName::GetFormat( wxPathFormat format )
{
    if (format == wxPATH_NATIVE)
    {
#if defined(WX_WINDOWS)
        format = wxPATH_DOS;
#elif defined(__VMS)
        format = wxPATH_VMS;
#else
        format = wxPATH_UNIX;
#endif
    }
    return format;
}

#ifdef WX_WINDOWS

/* static */
std::string wxFileName::GetVolumeString(char drive, unsigned int flags)
{
    wxASSERT_MSG( !(flags & ~wxPATH_GET_SEPARATOR), "invalid flag specified" );

    std::string vol{drive};
    vol += wxFILE_SEP_DSK;
    if ( flags & wxPATH_GET_SEPARATOR )
        vol += wxFILE_SEP_PATH;

    return vol;
}

#endif // wxHAS_FILESYSTEM_VOLUMES (WX_WINDOWS)

// ----------------------------------------------------------------------------
// path splitting function
// ----------------------------------------------------------------------------

/* static */
void
wxFileName::SplitVolume(const std::string& fullpathWithVolume,
                        std::string *pstrVolume,
                        std::string *pstrPath,
                        wxPathFormat format)
{
    format = GetFormat(format);

    std::string fullpath = fullpathWithVolume;

    if ( IsMSWUniqueVolumeNamePath(fullpath, format) )
    {
        // special Windows unique volume names hack: transform
        // \\?\Volume{guid}\path into Volume{guid}:path
        // note: this check must be done before the check for UNC path

        // we know the last backslash from the unique volume name is located
        // there from IsMSWUniqueVolumeNamePath
        fullpath[wxMSWUniqueVolumePrefixLength - 1] = wxFILE_SEP_DSK;

        // paths starting with a unique volume name should always be absolute
        fullpath.insert(wxMSWUniqueVolumePrefixLength, 1, wxFILE_SEP_PATH_DOS);

        // remove the leading "\\?\" part
        fullpath.erase(0, 4);
    }
    else if ( IsUNCPath(fullpath, format) )
    {
        // special Windows UNC paths hack: transform \\share\path into share:path

        fullpath.erase(0, 2);

        size_t posFirstSlash =
            fullpath.find_first_of(GetPathTerminators(format));
        if ( posFirstSlash != std::string::npos )
        {
            fullpath[posFirstSlash] = wxFILE_SEP_DSK;

            // UNC paths are always absolute, right? (FIXME)
            fullpath.insert(posFirstSlash + 1, 1, wxFILE_SEP_PATH_DOS);
        }
    }

    // We separate the volume here
    if ( format == wxPATH_DOS || format == wxPATH_VMS )
    {
        std::string sepVol = GetVolumeSeparator(format);

        // we have to exclude the case of a colon in the very beginning of the
        // string as it can't be a volume separator (nor can this be a valid
        // DOS file name at all but we'll leave dealing with this to our caller)
        size_t posFirstColon = fullpath.find_first_of(sepVol);
        if ( posFirstColon && posFirstColon != std::string::npos )
        {
            if ( pstrVolume )
            {
                *pstrVolume = fullpath.substr(0, posFirstColon);
            }

            // remove the volume name and the separator from the full path
            fullpath.erase(0, posFirstColon + sepVol.length());
        }
    }

    if ( pstrPath )
        *pstrPath = fullpath;
}

/* static */
void wxFileName::SplitPath(const std::string& fullpathWithVolume,
                           std::string *pstrVolume,
                           std::string *pstrPath,
                           std::string *pstrName,
                           std::string *pstrExt,
                           bool *hasExt,
                           wxPathFormat format)
{
    format = GetFormat(format);

    std::string fullpath;
    SplitVolume(fullpathWithVolume, pstrVolume, &fullpath, format);

    // find the positions of the last dot and last path separator in the path
    size_t posLastDot = fullpath.find_last_of(wxFILE_SEP_EXT);
    size_t posLastSlash = fullpath.find_last_of(GetPathTerminators(format));

    // check whether this dot occurs at the very beginning of a path component
    if ( (posLastDot != std::string::npos) &&
         (posLastDot == 0 ||
            IsPathSeparator(fullpath[posLastDot - 1]) ||
            (format == wxPATH_VMS && fullpath[posLastDot - 1] == wxT(']'))) )
    {
        // dot may be (and commonly -- at least under Unix -- is) the first
        // character of the filename, don't treat the entire filename as
        // extension in this case
        posLastDot = std::string::npos;
    }

    // if we do have a dot and a slash, check that the dot is in the name part
    if ( (posLastDot != std::string::npos) &&
         (posLastSlash != std::string::npos) &&
         (posLastDot < posLastSlash) )
    {
        // the dot is part of the path, not the start of the extension
        posLastDot = std::string::npos;
    }

    // now fill in the variables provided by user
    if ( pstrPath )
    {
        if ( posLastSlash == std::string::npos )
        {
            // no path at all
            pstrPath->clear();
        }
        else
        {
            // take everything up to the path separator but take care to make
            // the path equal to something like '/', not empty, for the files
            // immediately under root directory
            size_t len = posLastSlash;

            // this rule does not apply to mac since we do not start with colons (sep)
            // except for relative paths
            if ( !len && format != wxPATH_MAC)
                len++;

            *pstrPath = fullpath.substr(0, len);

            // special VMS hack: remove the initial bracket
            if ( format == wxPATH_VMS )
            {
                if ( (*pstrPath)[0u] == wxT('[') )
                    pstrPath->erase(0, 1);
            }
        }
    }

    if ( pstrName )
    {
        // take all characters starting from the one after the last slash and
        // up to, but excluding, the last dot
        size_t nStart = posLastSlash == std::string::npos ? 0 : posLastSlash + 1;
        
        const size_t count = [posLastDot, posLastSlash]() {
            if ( posLastDot == std::string::npos )
            {
                // take all until the end
                return std::string::npos;
            }
            else if ( posLastSlash == std::string::npos )
            {
                return posLastDot;
            }
            else // have both dot and slash
            {
                return posLastDot - posLastSlash - 1;
            }
        }();

        *pstrName = fullpath.substr(nStart, count);
    }

    // finally deal with the extension here: we have an added complication that
    // extension may be empty (but present) as in "foo." where trailing dot
    // indicates the empty extension at the end -- and hence we must remember
    // that we have it independently of pstrExt
    if ( posLastDot == std::string::npos )
    {
        // no extension
        if ( pstrExt )
            pstrExt->clear();
        if ( hasExt )
            *hasExt = false;
    }
    else
    {
        // take everything after the dot
        if ( pstrExt )
            *pstrExt = fullpath.substr(posLastDot + 1);
        if ( hasExt )
            *hasExt = true;
    }
}

/* static */
void wxFileName::SplitPath(const std::string& fullpath,
                           std::string *path,
                           std::string *name,
                           std::string *ext,
                           wxPathFormat format)
{
    std::string volume;
    SplitPath(fullpath, &volume, path, name, ext, format);

    if ( path )
    {
        path->insert(0, wxGetVolumeString(volume, format));
    }
}

/* static */
std::string wxFileName::StripExtension(const std::string& fullpath)
{
    wxFileName fn(fullpath);
    fn.SetExt(std::string());
    return fn.GetFullPath();
}

// ----------------------------------------------------------------------------
// file permissions functions
// ----------------------------------------------------------------------------

bool wxFileName::SetPermissions(int permissions)
{
    // Don't do anything for a symlink but first make sure it is one.
    if ( m_dontFollowLinks &&
            Exists(GetFullPath(), wxFILE_EXISTS_SYMLINK|wxFILE_EXISTS_NO_FOLLOW) )
    {
        // Looks like changing permissions for a symlinc is only supported
        // on BSD where lchmod is present and correctly implemented.
        // http://lists.gnu.org/archive/html/bug-coreutils/2009-09/msg00268.html
        return false;
    }

#ifdef WX_WINDOWS
    int accMode = 0;

    if ( permissions & (wxS_IRUSR|wxS_IRGRP|wxS_IROTH) )
        accMode = _S_IREAD;

    if ( permissions & (wxS_IWUSR|wxS_IWGRP|wxS_IWOTH) )
        accMode |= _S_IWRITE;

    permissions = accMode;
#endif // WX_WINDOWS

    return wxChmod(GetFullPath(), permissions) == 0;
}

// Returns the native path for a file URL
wxFileName wxFileName::URLToFileName(const std::string& url)
{
    std::string path;
    if ( !wx::utils::StartsWith(url, "file://", path) &&
            !wx::utils::StartsWith(url, "file:", path) )
    {
        // Consider it's just the path without any schema.
        path = url;
    }

    path = wxURI::Unescape(path);

#ifdef WX_WINDOWS
    // file urls either start with a forward slash (local harddisk),
    // otherwise they have a servername/sharename notation,
    // which only exists on msw and corresponds to a unc
    if ( path.length() > 1 && (path[0u] == '/' && path [1u] != '/') )
    {
        path = path.substr(1);
    }
    else if ( (url.rfind("file://", 0) == 0) &&
              (path.find('/') != std::string::npos) &&
              (path.length() > 1) && (path[1u] != ':') )
    {
        path = fmt::format("{}{}", '//', path);
    }
#endif

    wx::utils::ReplaceAll(path, "/", std::string{wxFILE_SEP_PATH});

    return {path, wxPATH_NATIVE};
}

// Escapes non-ASCII and others characters in file: URL to be valid URLs
static std::string EscapeFileNameCharsInURL(const std::string& in)
{
    std::string s;

    for ( auto ch : in )
    {
        // https://tools.ietf.org/html/rfc1738#section-5
        if ( (ch >= '0' && ch <= '9') ||
             (ch >= 'a' && ch <= 'z') ||
             (ch >= 'A' && ch <= 'Z') ||
             std::strchr("/:$-_.+!*'(),", ch) ) // Plus '/' and ':'
        {
            s += ch;
        }
        else
        {
            s += fmt::format("%%%02x", ch);
        }
    }

    return s;
}

// Returns the file URL for a native path
std::string wxFileName::FileNameToURL(const wxFileName& filename)
{
    wxFileName fn = filename;
    fn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE);
    std::string url = fn.GetFullPath(wxPATH_NATIVE);

#ifndef __UNIX__
    // unc notation, wxMSW
    if ( url.find("\\\\") == 0 )
    {
        url = url.substr(2);
    }
    else
    {
        url = "/" + url;
    }
#endif
    wx::utils::ReplaceAll(url, std::string{wxFILE_SEP_PATH}, "/");

    // Do wxURI- and common practice-compatible escaping
    return "file://" + EscapeFileNameCharsInURL(url);
}

// ----------------------------------------------------------------------------
// time functions
// ----------------------------------------------------------------------------

#if wxUSE_DATETIME

bool wxFileName::SetTimes(const wxDateTime *dtAccess,
                          const wxDateTime *dtMod,
                          const wxDateTime *dtCreate) const
{
#if defined(__WIN32__)
    FILETIME ftAccess, ftCreate, ftWrite;

    if ( dtCreate )
        ConvertWxToFileTime(&ftCreate, *dtCreate);
    if ( dtAccess )
        ConvertWxToFileTime(&ftAccess, *dtAccess);
    if ( dtMod )
        ConvertWxToFileTime(&ftWrite, *dtMod);

    std::string path;
    unsigned int flags;
    if ( IsDir() )
    {
        path = GetPath();
        flags = FILE_FLAG_BACKUP_SEMANTICS;
    }
    else // file
    {
        path = GetFullPath();
        flags = 0;
    }

    wxFileHandle fh(path, wxFileHandle::OpenMode::WriteAttr, flags);
    if ( fh.IsOk() )
    {
        if ( ::SetFileTime(fh,
                           dtCreate ? &ftCreate : nullptr,
                           dtAccess ? &ftAccess : nullptr,
                           dtMod ? &ftWrite : nullptr) )
        {
            return true;
        }
    }
#elif defined(__UNIX_LIKE__)
    wxUnusedVar(dtCreate);

    if ( !dtAccess && !dtMod )
    {
        // can't modify the creation time anyhow, don't try
        return true;
    }

    // if dtAccess or dtMod is not specified, use the other one (which must be
    // non NULL because of the test above) for both times
    utimbuf utm;
    utm.actime = dtAccess ? dtAccess->GetTicks() : dtMod->GetTicks();
    utm.modtime = dtMod ? dtMod->GetTicks() : dtAccess->GetTicks();
    if ( utime(GetFullPath().fn_str(), &utm) == 0 )
    {
        return true;
    }
#else // other platform
    wxUnusedVar(dtAccess);
    wxUnusedVar(dtMod);
    wxUnusedVar(dtCreate);
#endif // platforms

    wxLogSysError(_("Failed to modify file times for '%s'"),
                  GetFullPath().c_str());

    return false;
}

bool wxFileName::Touch() const
{
#if defined(__UNIX_LIKE__)
    // under Unix touching file is simple: just pass NULL to utime()
    if ( utime(GetFullPath().fn_str(), NULL) == 0 )
    {
        return true;
    }

    wxLogSysError(_("Failed to touch the file '%s'"), GetFullPath().c_str());

    return false;
#else // other platform
    wxDateTime dtNow = wxDateTime::Now();

    return SetTimes(&dtNow, &dtNow, nullptr /* don't change create time */);
#endif // platforms
}

bool wxFileName::GetTimes(wxDateTime *dtAccess,
                          wxDateTime *dtMod,
                          wxDateTime *dtCreate) const
{
#if defined(__WIN32__)
    // we must use different methods for the files and directories under
    // Windows as CreateFile(GENERIC_READ) doesn't work for the directories and
    // CreateFile(FILE_FLAG_BACKUP_SEMANTICS) works -- but only under NT and
    // not 9x
    bool ok;
    FILETIME ftAccess, ftCreate, ftWrite;
    if ( IsDir() )
    {
        // implemented in msw/dir.cpp
        extern bool wxGetDirectoryTimes(const std::string& dirname,
                                        FILETIME *, FILETIME *, FILETIME *);

        // we should pass the path without the trailing separator to
        // wxGetDirectoryTimes()
        ok = wxGetDirectoryTimes(GetPath(wxPATH_GET_VOLUME),
                                 &ftAccess, &ftCreate, &ftWrite);
    }
    else // file
    {
        wxFileHandle fh(GetFullPath(), wxFileHandle::OpenMode::ReadAttr);
        if ( fh.IsOk() )
        {
            ok = ::GetFileTime(fh,
                               dtCreate ? &ftCreate : nullptr,
                               dtAccess ? &ftAccess : nullptr,
                               dtMod ? &ftWrite : nullptr) != 0;
        }
        else
        {
            ok = false;
        }
    }

    if ( ok )
    {
        if ( dtCreate )
            ConvertFileTimeToWx(dtCreate, ftCreate);
        if ( dtAccess )
            ConvertFileTimeToWx(dtAccess, ftAccess);
        if ( dtMod )
            ConvertFileTimeToWx(dtMod, ftWrite);

        return true;
    }
#elif defined(wxHAVE_LSTAT)
    // no need to test for IsDir() here
    wxStructStat stBuf;
    if ( StatAny(stBuf, *this) )
    {
        // Android defines st_*time fields as unsigned long, but time_t as long,
        // hence the static_casts.
        if ( dtAccess )
            dtAccess->Set(static_cast<time_t>(stBuf.st_atime));
        if ( dtMod )
            dtMod->Set(static_cast<time_t>(stBuf.st_mtime));
        if ( dtCreate )
            dtCreate->Set(static_cast<time_t>(stBuf.st_ctime));

        return true;
    }
#else // other platform
    wxUnusedVar(dtAccess);
    wxUnusedVar(dtMod);
    wxUnusedVar(dtCreate);
#endif // platforms

    wxLogSysError(_("Failed to retrieve file times for '%s'"),
                  GetFullPath().c_str());

    return false;
}

#endif // wxUSE_DATETIME


// ----------------------------------------------------------------------------
// file size functions
// ----------------------------------------------------------------------------

#if wxUSE_LONGLONG

/* static */
wxULongLong wxFileName::GetSize(const std::string &filename)
{
    if (!wxFileExists(filename))
        return wxInvalidSize;

#if defined(__WIN32__)
    wxFileHandle f(filename, wxFileHandle::OpenMode::ReadAttr);
    if (!f.IsOk())
        return wxInvalidSize;

    DWORD lpFileSizeHigh;
    DWORD ret = GetFileSize(f, &lpFileSizeHigh);
    if ( ret == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR )
        return wxInvalidSize;

    return {lpFileSizeHigh, ret};
#else // ! __WIN32__
    wxStructStat st;
    if (wxStat( filename, &st) != 0)
        return wxInvalidSize;
    return {st.st_size};
#endif
}

/* static */
std::string wxFileName::GetHumanReadableSize(const wxULongLong &bs,
                                          const std::string &nullsize,
                                          int precision,
                                          wxSizeConvention conv)
{
    // deal with trivial case first
    if ( bs == 0 || bs == wxInvalidSize )
        return nullsize;

    // depending on the convention used the multiplier may be either 1000 or
    // 1024 and the binary infix may be empty (for "KB") or "i" (for "KiB")
    double multiplier = 1024.;
    std::string biInfix;

    switch ( conv )
    {
        case wxSizeConvention::Traditional:
            // nothing to do, this corresponds to the default values of both
            // the multiplier and infix string
            break;

        case wxSizeConvention::IEC:
            biInfix = "i";
            break;

        case wxSizeConvention::SI:
            multiplier = 1000;
            break;
    }

    const double kiloByteSize = multiplier;
    const double megaByteSize = multiplier * kiloByteSize;
    const double gigaByteSize = multiplier * megaByteSize;
    const double teraByteSize = multiplier * gigaByteSize;

    const double bytesize = bs.ToDouble();

    std::string result;
    if ( bytesize < kiloByteSize )
        result = fmt::format("%s B", bs.ToString().ToStdString());
    else if ( bytesize < megaByteSize )
        result = fmt::format("%.*f K%sB", precision, bytesize/kiloByteSize, biInfix);
    else if (bytesize < gigaByteSize)
        result = fmt::format("%.*f M%sB", precision, bytesize/megaByteSize, biInfix);
    else if (bytesize < teraByteSize)
        result = fmt::format("%.*f G%sB", precision, bytesize/gigaByteSize, biInfix);
    else
        result = fmt::format("%.*f T%sB", precision, bytesize/teraByteSize, biInfix);

    return result;
}

wxULongLong wxFileName::GetSize() const
{
    return GetSize(GetFullPath());
}

std::string wxFileName::GetHumanReadableSize(const std::string& failmsg,
                                          int precision,
                                          wxSizeConvention conv) const
{
    return GetHumanReadableSize(GetSize(), failmsg, precision, conv);
}

#endif // wxUSE_LONGLONG

