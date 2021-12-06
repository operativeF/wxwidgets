/////////////////////////////////////////////////////////////////////////////
// Name:        wx/filefn.h
// Purpose:     File- and directory-related functions
// Author:      Julian Smart
// Modified by:
// Created:     29/01/98
// Copyright:   (c) 1998 Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef   _FILEFN_H_
#define   _FILEFN_H_

#include "wx/list.h"

import <ctime>;

#include <sys/types.h>
#include <sys/stat.h>

#if defined(__UNIX__)
    #include <unistd.h>
    #include <dirent.h>
#endif

#if defined(WX_WINDOWS)
#if !defined( __GNUWIN32__ ) && !defined(__CYGWIN__)
    #include <direct.h>
    #include <dos.h>
    #include <io.h>
#endif // WX_WINDOWS
#endif // native Win compiler

#include  <fcntl.h>       // O_RDONLY &c

import WX.Cfg.Flags;

import <vector>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// MSVC doesn't define mode_t, so do it ourselves unless someone else
// had already predefined it.
#if defined(__VISUALC__) && !defined(wxHAS_MODE_T)
    #define wxHAS_MODE_T
    using mode_t = unsigned int;
#endif

// define off_t
#include  <sys/types.h>

#if defined(__VISUALC__)
    using off_t = _off_t;
#endif

enum class wxSeekMode
{
  FromStart,
  FromCurrent,
  FromEnd
};

enum class wxFileKind
{
  Unknown,
  Disk,     // a file supporting seeking to arbitrary offsets
  Terminal, // a tty
  Pipe      // a pipe
};

// we redefine these constants here because S_IREAD &c are _not_ standard
// however, we do assume that the values correspond to the Unix umask bits
enum wxPosixPermissions
{
    // standard Posix names for these permission flags:
    wxS_IRUSR = 00400,
    wxS_IWUSR = 00200,
    wxS_IXUSR = 00100,

    wxS_IRGRP = 00040,
    wxS_IWGRP = 00020,
    wxS_IXGRP = 00010,

    wxS_IROTH = 00004,
    wxS_IWOTH = 00002,
    wxS_IXOTH = 00001,

    // longer but more readable synonyms for the constants above:
    wxPOSIX_USER_READ = wxS_IRUSR,
    wxPOSIX_USER_WRITE = wxS_IWUSR,
    wxPOSIX_USER_EXECUTE = wxS_IXUSR,

    wxPOSIX_GROUP_READ = wxS_IRGRP,
    wxPOSIX_GROUP_WRITE = wxS_IWGRP,
    wxPOSIX_GROUP_EXECUTE = wxS_IXGRP,

    wxPOSIX_OTHERS_READ = wxS_IROTH,
    wxPOSIX_OTHERS_WRITE = wxS_IWOTH,
    wxPOSIX_OTHERS_EXECUTE = wxS_IXOTH,

    // default mode for the new files: allow reading/writing them to everybody but
    // the effective file mode will be set after anding this value with umask and
    // so won't include wxS_IW{GRP,OTH} for the default 022 umask value
    wxS_DEFAULT = (wxPOSIX_USER_READ | wxPOSIX_USER_WRITE | \
                   wxPOSIX_GROUP_READ | wxPOSIX_GROUP_WRITE | \
                   wxPOSIX_OTHERS_READ | wxPOSIX_OTHERS_WRITE),

    // default mode for the new directories (see wxFileName::Mkdir): allow
    // reading/writing/executing them to everybody, but just like wxS_DEFAULT
    // the effective directory mode will be set after anding this value with umask
    wxS_DIR_DEFAULT = (wxPOSIX_USER_READ | wxPOSIX_USER_WRITE | wxPOSIX_USER_EXECUTE | \
                       wxPOSIX_GROUP_READ | wxPOSIX_GROUP_WRITE | wxPOSIX_GROUP_EXECUTE | \
                       wxPOSIX_OTHERS_READ | wxPOSIX_OTHERS_WRITE | wxPOSIX_OTHERS_EXECUTE)
};

// ----------------------------------------------------------------------------
// declare our versions of low level file functions: some compilers prepend
// underscores to the usual names, some also have Unicode versions of them
// ----------------------------------------------------------------------------

#if defined(WX_WINDOWS) && \
      ( \
        defined(__VISUALC__) || \
        defined(__MINGW64_TOOLCHAIN__) || \
        (defined(__MINGW32__) && !defined(__WINE__)) \
      )

    // temporary defines just used immediately below
    #undef wxHAS_HUGE_FILES
    #undef wxHAS_HUGE_STDIO_FILES

    // detect compilers which have support for huge files
    #if defined(__VISUALC__)
        #define wxHAS_HUGE_FILES 1
    #elif defined(__MINGW32__)
        #define wxHAS_HUGE_FILES 1
    #elif defined(_LARGE_FILES)
        #define wxHAS_HUGE_FILES 1
    #endif

    // detect compilers which have support for huge stdio files
    #if wxCHECK_VISUALC_VERSION(8)
        #define wxHAS_HUGE_STDIO_FILES
        #define wxFseek _fseeki64
        #define wxFtell _ftelli64
    #elif wxCHECK_MINGW32_VERSION(3, 5) // mingw-runtime version (not gcc)
        #define wxHAS_HUGE_STDIO_FILES

        wxDECL_FOR_STRICT_MINGW32(int, fseeko64, (FILE*, long long, int))
        #define wxFseek fseeko64

        #ifdef wxNEEDS_STRICT_ANSI_WORKAROUNDS
            // Unfortunately ftello64() is not defined in the library for
            // whatever reason but as an inline function, so define wxFtell()
            // here similarly.
            inline long long wxFtell(FILE* fp)
            {
                fpos_t pos;
                if ( fgetpos(fp, &pos) != 0 )
                    return -1LL;

                // Unfortunately our interface assumes that the file position
                // is representable as "long long", so we have to get it from
                // fpos_t, even though it's an opaque type. And its exact
                // representation has changed in MinGW, so we have to test for
                // mingwrt version.
                #if wxCHECK_MINGW32_VERSION(5, 2)
                    // In 5.2.2 it's a union with a __value field.
                    return pos.__value;
                #else
                    // Up to 5.1.1 it was a simple typedef.
                    return pos;
                #endif
            }
        #else
            #define wxFtell ftello64
        #endif
    #endif


    // types

    #ifdef wxHAS_HUGE_FILES
        using wxFileOffset = long long;
        #define wxFileOffsetFmtSpec wxLongLongFmtSpec
    #else
        using wxFileOffset = off_t;
    #endif


    #define wxPOSIX_STRUCT(s) struct wxPOSIX_IDENT(s)

    #ifdef wxHAS_HUGE_FILES
        #define wxStructStat struct _stati64
    #else
        #define wxStructStat struct _stat
    #endif


    // functions

    // MSVC and compatible compilers prepend underscores to the POSIX function
    // names, other compilers don't and even if their later versions usually do
    // define the versions with underscores for MSVC compatibility, it's better
    // to avoid using them as they're not present in earlier versions and
    // always using the native functions spelling is easier than testing for
    // the versions
    #if defined(__MINGW64_TOOLCHAIN__)
        #define wxPOSIX_IDENT(func)    ::func
    #else // by default assume MSVC-compatible names
        #define wxPOSIX_IDENT(func)    _ ## func
        #define wxHAS_UNDERSCORES_IN_POSIX_IDENTS
    #endif

    // first functions not working with strings, i.e. without ANSI/Unicode
    // complications
    #define   wxClose      wxPOSIX_IDENT(close)

    #define wxRead         wxPOSIX_IDENT(read)
    #define wxWrite        wxPOSIX_IDENT(write)

    #ifdef wxHAS_HUGE_FILES
        #ifndef __MINGW64_TOOLCHAIN__
            #define   wxSeek       wxPOSIX_IDENT(lseeki64)
            #define   wxLseek      wxPOSIX_IDENT(lseeki64)
            #define   wxTell       wxPOSIX_IDENT(telli64)
        #else
            // unfortunately, mingw-W64 is somewhat inconsistent...
            #define   wxSeek       _lseeki64
            #define   wxLseek      _lseeki64
            #define   wxTell       _telli64
        #endif
    #else // !wxHAS_HUGE_FILES
        #define   wxSeek       wxPOSIX_IDENT(lseek)
        #define   wxLseek      wxPOSIX_IDENT(lseek)
        #define   wxTell       wxPOSIX_IDENT(tell)
    #endif // wxHAS_HUGE_FILES/!wxHAS_HUGE_FILES


     #define   wxFsync      _commit

     // could be already defined by configure (Cygwin)
     #ifndef HAVE_FSYNC
         #define HAVE_FSYNC
     #endif

    #define   wxEof        wxPOSIX_IDENT(eof)

    // then the functions taking strings

    // first the ANSI versions
    #define   wxCRT_OpenA       wxPOSIX_IDENT(open)
    #define   wxCRT_AccessA     wxPOSIX_IDENT(access)
    #define   wxCRT_ChmodA      wxPOSIX_IDENT(chmod)
    #define   wxCRT_MkDirA      wxPOSIX_IDENT(mkdir)
    #define   wxCRT_RmDirA      wxPOSIX_IDENT(rmdir)
    #ifdef wxHAS_HUGE_FILES
        // MinGW-64 provides underscore-less versions of all file functions
        // except for this one.
        #ifdef __MINGW64_TOOLCHAIN__
            #define   wxCRT_StatA       _stati64
        #else
            #define   wxCRT_StatA       wxPOSIX_IDENT(stati64)
        #endif
    #else
        #define   wxCRT_StatA       wxPOSIX_IDENT(stat)
    #endif

    // then wide char ones
    #define wxCRT_OpenW         _wopen

    wxDECL_FOR_STRICT_MINGW32(int, _wopen, (const wchar_t*, int, ...))
    wxDECL_FOR_STRICT_MINGW32(int, _waccess, (const wchar_t*, int))
    wxDECL_FOR_STRICT_MINGW32(int, _wchmod, (const wchar_t*, int))
    wxDECL_FOR_STRICT_MINGW32(int, _wmkdir, (const wchar_t*))
    wxDECL_FOR_STRICT_MINGW32(int, _wrmdir, (const wchar_t*))
    wxDECL_FOR_STRICT_MINGW32(int, _wstati64, (const wchar_t*, struct _stati64*))

    #define   wxCRT_AccessW     _waccess
    #define   wxCRT_ChmodW      _wchmod
    #define   wxCRT_MkDirW      _wmkdir
    #define   wxCRT_RmDirW      _wrmdir
    #ifdef wxHAS_HUGE_FILES
        #define   wxCRT_StatW       _wstati64
    #else
        #define   wxCRT_StatW       _wstat
    #endif


    // finally the default char-type versions
    #define wxCRT_Open      wxCRT_OpenW
    #define wxCRT_Access    wxCRT_AccessW
    #define wxCRT_Chmod     wxCRT_ChmodW
    #define wxCRT_MkDir     wxCRT_MkDirW
    #define wxCRT_RmDir     wxCRT_RmDirW
    #define wxCRT_Stat      wxCRT_StatW

    // constants (unless already defined by the user code)
    #ifdef wxHAS_UNDERSCORES_IN_POSIX_IDENTS
        #ifndef O_RDONLY
            #define   O_RDONLY    _O_RDONLY
            #define   O_WRONLY    _O_WRONLY
            #define   O_RDWR      _O_RDWR
            #define   O_EXCL      _O_EXCL
            #define   O_CREAT     _O_CREAT
            #define   O_BINARY    _O_BINARY
        #endif

        #ifndef S_IFMT
            #define   S_IFMT      _S_IFMT
            #define   S_IFDIR     _S_IFDIR
            #define   S_IFREG     _S_IFREG
        #endif
    #endif // wxHAS_UNDERSCORES_IN_POSIX_IDENTS

    #ifdef wxHAS_HUGE_FILES
        // wxFile is present and supports large files.
        #if wxUSE_FILE
            #define wxHAS_LARGE_FILES
        #endif
        // wxFFile is present and supports large files
        #if wxUSE_FFILE && defined wxHAS_HUGE_STDIO_FILES
            #define wxHAS_LARGE_FFILES
        #endif
    #endif

    // private defines, undefine so that nobody gets tempted to use
    #undef wxHAS_HUGE_FILES
    #undef wxHAS_HUGE_STDIO_FILES
#else // Unix or Windows using unknown compiler, assume POSIX supported
    using wxFileOffset = off_t;
    #ifdef HAVE_LARGEFILE_SUPPORT
        #define wxFileOffsetFmtSpec wxLongLongFmtSpec
        static_assert( sizeof(off_t) == sizeof(wxLongLong_t), "Bad file size type");
        // wxFile is present and supports large files
        #if wxUSE_FILE
            #define wxHAS_LARGE_FILES
        #endif
        // wxFFile is present and supports large files
        #if wxUSE_FFILE && (SIZEOF_LONG == 8 || defined HAVE_FSEEKO)
            #define wxHAS_LARGE_FFILES
        #endif
        #ifdef HAVE_FSEEKO
            #define wxFseek fseeko
            #define wxFtell ftello
        #endif
    #else
        inline constexpr wxChar wxFileOffsetFmtSpec[] = wxT("");
    #endif
    // functions
    #define   wxClose      close
    #define   wxRead       ::read
    #define   wxWrite      ::write
    #define   wxLseek      lseek
    #define   wxSeek       lseek
    #define   wxFsync      fsync
    #define   wxEof        eof
    #define   wxCRT_MkDir      mkdir
    #define   wxCRT_RmDir      rmdir

    #define   wxTell(fd)   lseek(fd, 0, SEEK_CUR)

    #define   wxStructStat struct stat

    #define   wxCRT_Open       open
    #define   wxCRT_Stat       stat
    #define   wxCRT_Lstat      lstat
    #define   wxCRT_Access     access
    #define   wxCRT_Chmod      chmod

    #define   wxCRT_Readlink   readlink

    #define wxHAS_NATIVE_LSTAT
    #define wxHAS_NATIVE_READLINK
#endif // platforms

// if the platform doesn't have symlinks, define wxCRT_Lstat to be the same as
// wxCRT_Stat to avoid #ifdefs in the code using it
#ifndef wxHAS_NATIVE_LSTAT
    #define wxCRT_Lstat wxCRT_Stat
#endif

// define wxFseek/wxFtell to large file versions if available (done above) or
// to fseek/ftell if not, to save ifdefs in using code
#ifndef wxFseek
    #define wxFseek fseek
#endif
#ifndef wxFtell
    #define wxFtell ftell
#endif

inline int wxAccess(const wxString& path, mode_t mode)
    { return wxCRT_Access(path.fn_str(), mode); }
inline int wxChmod(const wxString& path, mode_t mode)
    { return wxCRT_Chmod(path.fn_str(), mode); }
inline int wxOpen(const wxString& path, unsigned int flags, mode_t mode)
    { return wxCRT_Open(path.fn_str(), flags, mode); }

#if defined(wxHAS_NATIVE_READLINK)
inline ssize_t wxReadlink(const wxString& path, char* buf, int size)
    { return wxCRT_Readlink(path.fn_str(), buf, size); }
#endif

inline int wxStat(const wxString& path, wxStructStat *buf)
    { return wxCRT_Stat(path.fn_str(), buf); }
inline int wxLstat(const wxString& path, wxStructStat *buf)
    { return wxCRT_Lstat(path.fn_str(), buf); }
inline int wxRmDir(const wxString& path)
    { return wxCRT_RmDir(path.fn_str()); }
#if (defined(WX_WINDOWS) && !defined(__CYGWIN__))
inline int wxMkDir(const wxString& path, [[maybe_unused]] mode_t mode = 0)
    { return wxCRT_MkDir(path.fn_str()); }
#else
inline int wxMkDir(const wxString& path, mode_t mode)
    { return wxCRT_MkDir(path.fn_str(), mode); }
#endif

#ifdef O_BINARY
    #define wxO_BINARY O_BINARY
#else
    #define wxO_BINARY 0
#endif

inline constexpr auto wxInvalidOffset = wx::narrow_cast<unsigned int>(-1);

// ----------------------------------------------------------------------------
// functions
// ----------------------------------------------------------------------------
bool wxFileExists(const std::string& filename);

// does the path exist? (may have or not '/' or '\\' at the end)
bool wxDirExists(const std::string& pathName);

bool wxIsAbsolutePath(const std::string& filename);

// Get filename
wxChar* wxFileNameFromPath(wxChar *path);
std::string wxFileNameFromPath(const std::string& path);

// Get directory
std::string wxPathOnly(const std::string& path);

// Get first file name matching given wild card.
// Flags are reserved for future use.
#define wxFILE  1
#define wxDIR   2
std::string wxFindFirstFile(const std::string& spec, unsigned int flags = wxFILE);
std::string wxFindNextFile();

// Does the pattern contain wildcards?
bool wxIsWild(const std::string& pattern);

// Does the pattern match the text (usually a filename)?
// If dot_special is true, doesn't match * against . (eliminating
// `hidden' dot files)
bool wxMatchWild(const std::string& pattern,  const std::string& text, bool dot_special = true);

// Concatenate two files to form third
bool wxConcatFiles(const std::string& src1, const std::string& src2, const std::string& dest);

// Copy file
bool wxCopyFile(const std::string& src, const std::string& dest,
                                 bool overwrite = true);

// Remove file
bool wxRemoveFile(const std::string& file);

// Rename file
bool wxRenameFile(const std::string& oldpath, const std::string& newpath, bool overwrite = true);

// Get current working directory.
std::string wxGetCwd();

// Set working directory
bool wxSetWorkingDirectory(const std::string& d);

// Make directory
bool wxMkdir(const std::string& dir, int perm = wxS_DIR_DEFAULT);

// Remove directory. Flags reserved for future use.
bool wxRmdir(const std::string& dir, unsigned int flags = 0);

// Return the type of an open file
wxFileKind wxGetFileKind(int fd);
wxFileKind wxGetFileKind(FILE *fp);

// permissions; these functions work both on files and directories:
bool wxIsWritable(const std::string &path);
bool wxIsReadable(const std::string &path);
bool wxIsExecutable(const std::string &path);

// ----------------------------------------------------------------------------
// separators in file names
// ----------------------------------------------------------------------------

// between file name and extension
inline constexpr char wxFILE_SEP_EXT        = '.';

// between drive/volume name and the path
inline constexpr char wxFILE_SEP_DSK        = ':';

// between the path components
inline constexpr char wxFILE_SEP_PATH_DOS   = '\\';
inline constexpr char wxFILE_SEP_PATH_UNIX  = '/';
inline constexpr char wxFILE_SEP_PATH_MAC   = ':';
inline constexpr char wxFILE_SEP_PATH_VMS   = '.'; // VMS also uses '[' and ']'

// separator in the path list (as in PATH environment variable)
// there is no PATH variable in Classic Mac OS so just use the
// semicolon (it must be different from the file name separator)
// NB: these are strings and not characters on purpose!
inline constexpr char wxPATH_SEP_DOS[]        = ";";
inline constexpr char wxPATH_SEP_UNIX[]       = ":";
inline constexpr char wxPATH_SEP_MAC[]        = ";";

// platform independent versions
#if defined(__UNIX__)
  // CYGWIN also uses UNIX settings
  #define wxFILE_SEP_PATH     wxFILE_SEP_PATH_UNIX
  #define wxPATH_SEP          wxPATH_SEP_UNIX
#elif defined(__MAC__)
  #define wxFILE_SEP_PATH     wxFILE_SEP_PATH_MAC
  #define wxPATH_SEP          wxPATH_SEP_MAC
#else   // Windows
  #define wxFILE_SEP_PATH     wxFILE_SEP_PATH_DOS
  #define wxPATH_SEP          wxPATH_SEP_DOS
#endif  // Unix/Windows

// this is useful for wxString::IsSameAs(): to compare two file names use
// filename1.IsSameAs(filename2, wxARE_FILENAMES_CASE_SENSITIVE)
#if defined(__UNIX__) && !defined(__DARWIN__)
  #define wxARE_FILENAMES_CASE_SENSITIVE  true
#else   // Windows and OSX
  #define wxARE_FILENAMES_CASE_SENSITIVE  false
#endif  // Unix/Windows

// is the char a path separator?
constexpr bool wxIsPathSeparator(wxChar c)
{
    // under DOS/Windows we should understand both Unix and DOS file separators
#if defined(__UNIX__) || defined(__MAC__)
    return c == wxFILE_SEP_PATH;
#else
    return c == wxFILE_SEP_PATH_DOS || c == wxFILE_SEP_PATH_UNIX;
#endif
}

// does the string ends with path separator?
bool wxEndsWithPathSeparator(const std::string& filename);

// find a file in a list of directories, returns false if not found
bool wxFindFileInPath(std::string *pStr, const std::string& szPath, const std::string& szFile);

// Get the OS directory if appropriate (such as the Windows directory).
// On non-Windows platform, probably just return the empty string.
std::string wxGetOSDirectory();

#if wxUSE_DATETIME

// Get file modification time
time_t wxFileModificationTime(const std::string& filename);

#endif // wxUSE_DATETIME

// Parses the wildCard, returning the number of filters.
// Returns 0 if none or if there's a problem,
// The arrays will contain an equal number of items found before the error.
// wildCard is in the form:
// "All files (*)|*|Image Files (*.jpeg *.png)|*.jpg;*.png"
std::size_t wxParseCommonDialogsFilter(const std::string& wildCard, std::vector<std::string>& descriptions, std::vector<std::string>& filters);

// ----------------------------------------------------------------------------
// classes
// ----------------------------------------------------------------------------

#ifdef __UNIX__

// set umask to the given value in ctor and reset it to the old one in dtor
class wxUmaskChanger
{
public:
    // change the umask to the given one if it is not -1: this allows to write
    // the same code whether you really want to change umask or not, as is in
    // wxFileConfig::Flush() for example
    wxUmaskChanger(int umaskNew)
    {
        m_umaskOld = umaskNew == -1 ? -1 : (int)umask((mode_t)umaskNew);
    }

    ~wxUmaskChanger()
    {
        if ( m_umaskOld != -1 )
            umask((mode_t)m_umaskOld);
    }

private:
    int m_umaskOld;
};

// this macro expands to an "anonymous" wxUmaskChanger object under Unix and
// nothing elsewhere
#define wxCHANGE_UMASK(m) wxUmaskChanger wxMAKE_UNIQUE_NAME(umaskChanger_)(m)

#else // !__UNIX__

#define wxCHANGE_UMASK(m)

#endif // __UNIX__/!__UNIX__


// Path searching
class wxPathList
{
public:
    wxPathList() = default;
    wxPathList(const std::vector<std::string> &arr)
        { Add(arr); }

    // Adds all paths in environment variable
    void AddEnvList(const std::string& envVariable);

    // Adds given path to this list
    bool Add(const std::string& path);
    void Add(const std::vector<std::string> &paths);

    // Find the first full path for which the file exists
    std::string FindValidPath(const std::string& filename) const;

    // Find the first full path for which the file exists; ensure it's an
    // absolute path that gets returned.
    std::string FindAbsoluteValidPath(const std::string& filename) const;

    // Given full path and filename, add path to list
    bool EnsureFileAccessible(const std::string& path);

private:
    std::vector<std::string> m_paths;
};

#endif // _WX_FILEFN_H_
