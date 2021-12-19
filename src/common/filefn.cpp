/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/filefn.cpp
// Purpose:     File- and directory-related functions
// Author:      Julian Smart
// Modified by:
// Created:     29/01/98
// Copyright:   (c) 1998 Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/filefn.h"

#ifdef WX_WINDOWS
    #include "wx/msw/private.h"
#endif

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/utils.h"
#include "wx/crt.h"
#include "wx/file.h"
#include "wx/filename.h"
#include "wx/dir.h"

#if defined(__WXMAC__)
    #include  "wx/osx/private.h"  // includes mac headers
#endif

#ifdef WX_WINDOWS
    // sys/cygwin.h is needed for cygwin_conv_to_full_win32_path()
    // and for cygwin_conv_path()
    //
    // note that it must be included after <windows.h>
    #ifdef __GNUWIN32__
        #ifdef __CYGWIN__
            #include <sys/cygwin.h>
            #include <cygwin/version.h>
        #endif
    #endif // __GNUWIN32__

    // io.h is needed for _get_osfhandle()
    // Already included by filefn.h for many Windows compilers
    #if defined __CYGWIN__
        #include <io.h>
    #endif
#endif // WX_WINDOWS

#if defined(__VMS__)
    #include <fab.h>
#endif

#include <boost/nowide/convert.hpp>

import Utils.Chars;
import Utils.Strings;

import <algorithm>;
import <array>;
import <filesystem>;
import <vector>;

#if defined(_MSC_VER) || defined(__MINGW32__)
    wxDECL_FOR_STRICT_MINGW32(wchar_t*, _wgetcwd, (wchar_t*, int))

    #define HAVE_WGETCWD
#endif

wxDECL_FOR_STRICT_MINGW32(int, _fileno, (FILE*))

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

#ifndef _MAXPATHLEN
    #define _MAXPATHLEN 1024
#endif

// ----------------------------------------------------------------------------
// wxPathList
// ----------------------------------------------------------------------------

bool wxPathList::Add(const std::string& path)
{
    // add a path separator to force wxFileName to interpret it always as a directory
    // (i.e. if we are called with '/home/user' we want to consider it a folder and
    // not, as wxFileName would consider, a filename).
    wxFileName fn(path + wxFileName::GetPathSeparator());

    // add only normalized relative/absolute paths
    // NB: we won't do wxPATH_NORM_DOTS in order to avoid problems when trying to
    //     normalize paths which starts with ".." (which can be normalized only if
    //     we use also wxPATH_NORM_ABSOLUTE - which we don't want to use).
    if (!fn.Normalize(wxPATH_NORM_TILDE|wxPATH_NORM_LONG|wxPATH_NORM_ENV_VARS))
        return false;

    std::string toadd = fn.GetPath();

    if (std::ranges::find(m_paths, toadd) == m_paths.cend())
        m_paths.push_back(toadd);      // do not add duplicates

    return true;
}

void wxPathList::Add(const std::vector<std::string> &arr)
{
    for (size_t j=0; j < arr.size(); j++)
        Add(arr[j]);
}

// Add paths e.g. from the PATH environment variable
void wxPathList::AddEnvList (const std::string& envVariable)
{
    // The space has been removed from the tokenizers, otherwise a
    // path such as "C:\Program Files" would be split into 2 paths:
    // "C:\Program" and "Files"; this is true for both Windows and Unix.

    static constexpr char PATH_TOKS[] =
#if defined(WX_WINDOWS)
        ";"; // Don't separate with colon in DOS (used for drive)
#else
        ":;";
#endif

    std::string val;
    if ( wxGetEnv(envVariable, &val) )
    {
        // split into an array of string the value of the env var
        const std::vector<std::string> arr = wxStringTokenize(val, PATH_TOKS);

        m_paths.insert(m_paths.end(), arr.begin(), arr.end());
    }
}

// Given a full filename (with path), ensure that that file can
// be accessed again USING FILENAME ONLY by adding the path
// to the list if not already there.
bool wxPathList::EnsureFileAccessible (const std::string& path)
{
    return Add(wxPathOnly(path));
}

std::string wxPathList::FindValidPath (const std::string& file) const
{
    // normalize the given string as it could be a path + a filename
    // and not only a filename
    wxFileName fn(file);
    std::string strend;

    // NB: normalize without making absolute otherwise calling this function with
    //     e.g. "b/c.txt" would result in removing the directory 'b' and the for loop
    //     below would only add to the paths of this list the 'c.txt' part when doing
    //     the existence checks...
    // NB: we don't use wxPATH_NORM_DOTS here, too (see wxPathList::Add for more info)
    if (!fn.Normalize(wxPATH_NORM_TILDE|wxPATH_NORM_LONG|wxPATH_NORM_ENV_VARS))
        return {};

    wxASSERT_MSG(!fn.IsDir(), "Cannot search for directories; only for files");
    if (fn.IsAbsolute())
        strend = fn.GetFullName();      // search for the file name and ignore the path part
    else
        strend = fn.GetFullPath();

    for (size_t i=0; i < m_paths.size(); i++)
    {
        std::string strstart = m_paths[i];
        if (!strstart.empty() && strstart.back() != wxFileName::GetPathSeparator())
            strstart += wxFileName::GetPathSeparator();

        if (wxFileExists(strstart + strend))
            return strstart + strend;        // Found!
    }

    return {};                    // Not found
}

std::string wxPathList::FindAbsoluteValidPath (const std::string& file) const
{
    std::string f = FindValidPath(file);
    if ( f.empty() || wxIsAbsolutePath(f) )
        return f;

    std::string buf = ::wxGetCwd();

    if ( !wxEndsWithPathSeparator(buf) )
    {
        buf += wxFILE_SEP_PATH;
    }
    buf += f;

    return buf;
}

// ----------------------------------------------------------------------------
// miscellaneous global functions
// ----------------------------------------------------------------------------

bool
wxFileExists (const std::string& filename)
{
    return wxFileName::FileExists(filename);
}

bool
wxIsAbsolutePath (const std::string& filename)
{
    if (!filename.empty())
    {
        // Unix like or Windows
        if (filename[0] == '/')
            return true;
#ifdef __VMS__
        if (filename.size() > 1 && (filename[0] == '[' && filename[1] != '.'))
            return true;
#endif
#if defined(WX_WINDOWS)
        // MSDOS like
        if (filename[0] == '\\' ||
            (filename.size() > 1 && (wx::utils::isAlpha(filename[0]) && filename[1] == ':')))
            return true;
#endif
    }
    return false ;
}

// Return just the filename, not the path (basename)
char* wxFileNameFromPath (char *path)
{
    std::string p = path;
    std::string n = wxFileNameFromPath(p);

    return path + p.length() - n.length();
}

std::string wxFileNameFromPath (const std::string& path)
{
    return wxFileName(path).GetFullName();
}

// Return just the directory, or NULL if no directory
wxChar *
wxPathOnly (wxChar *path)
{
    if (path && *path)
    {
        static wxChar buf[_MAXPATHLEN];

        const auto l = wxStrlen(path);
        int i = l - 1;
        if ( i >= _MAXPATHLEN )
            return nullptr;

        // Local copy
        wxStrcpy (buf, path);

        // Search backward for a backward or forward slash
        while (i > -1)
        {
            // Unix like or Windows
            if (path[i] == '/' || path[i] == '\\')
            {
                buf[i] = 0;
                return buf;
            }
#ifdef __VMS__
            if (path[i] == ']')
            {
                buf[i+1] = 0;
                return buf;
            }
#endif
            i --;
        }

#if defined(WX_WINDOWS)
        // Try Drive specifier
        if (wx::utils::isAlpha(buf[0]) && buf[1] == ':')
        {
            // A:junk --> A:. (since A:.\junk Not A:\junk)
            buf[2] = '.';
            buf[3] = '\0';
            return buf;
        }
#endif
    }
    return nullptr;
}

// Return just the directory, or NULL if no directory
std::string wxPathOnly (const std::string& path)
{
    if (!path.empty())
    {
        char buf[_MAXPATHLEN];

        const int l = path.length();
        int i = l - 1;

        if ( i >= _MAXPATHLEN )
            return {};

        // Local copy
        wxStrcpy(buf, path);

        // Search backward for a backward or forward slash
        while (i > -1)
        {
            // Unix like or Windows
            if (path[i] == '/' || path[i] == '\\')
            {
                // Don't return an empty string
                if (i == 0)
                    i ++;
                buf[i] = 0;
                return {buf};
            }
#ifdef __VMS__
            if (path[i] == ']')
            {
                buf[i+1] = 0;
                return {buf};
            }
#endif
            i --;
        }

#if defined(WX_WINDOWS)
        // Try Drive specifier
        if (wx::utils::isAlpha(buf[0]) && buf[1] == ':')
        {
            // A:junk --> A:. (since A:.\junk Not A:\junk)
            buf[2] = '.';
            buf[3] = '\0';
            return {buf};
        }
#endif
    }
    return {};
}

// Utility for converting delimiters in DOS filenames to UNIX style
// and back again - or we get nasty problems with delimiters.
// Also, convert to lower case, since case is significant in UNIX.

#ifdef __WXOSX__

CFURLRef wxOSXCreateURLFromFileSystemPath( const std::string& path)
{
    wxCFRef<CFMutableStringRef> cfMutableString(CFStringCreateMutableCopy(NULL, 0, wxCFStringRef(path)));
    CFStringNormalize(cfMutableString,kCFStringNormalizationFormD);
    return CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfMutableString , kCFURLPOSIXPathStyle, false);
}

#endif // __WXMAC__

// Concatenate two files to form third
bool
wxConcatFiles (const std::string& file1, const std::string& file2, const std::string& file3)
{
#if wxUSE_FILE

    wxFile in1(file1), in2(file2);
    wxTempFile out(file3);

    if ( !in1.IsOpened() || !in2.IsOpened() || !out.IsOpened() )
        return false;

    std::size_t ofs{};
    std::array<unsigned char, 1024> buf;

    for( int i=0; i<2; i++)
    {
        wxFile *in = i==0 ? &in1 : &in2;
        do {
            // FIXME: Use span
            if ( (ofs = in->Read(buf.data(), buf.size())) == wxInvalidOffset ) return false;
            if ( ofs > 0 ) // FIXME: Use span
                if ( !out.Write(buf.data(), ofs) )
                    return false;
        } while ( ofs == (ssize_t)WXSIZEOF(buf) );
    }

    return out.Commit();

#else

    wxUnusedVar(file1);
    wxUnusedVar(file2);
    wxUnusedVar(file3);
    return false;

#endif
}

// helper of generic implementation of wxCopyFile()
#if !defined(__WIN32__) && wxUSE_FILE

static bool
wxDoCopyFile(wxFile& fileIn,
             const wxStructStat& fbuf,
             const std::string& filenameDst,
             bool overwrite)
{
    // reset the umask as we want to create the file with exactly the same
    // permissions as the original one
    wxCHANGE_UMASK(0);

    // create file2 with the same permissions than file1 and open it for
    // writing

    wxFile fileOut;
    if ( !fileOut.Create(filenameDst, overwrite, fbuf.st_mode & 0777) )
        return false;

    // copy contents of file1 to file2
    char buf[4096];
    for ( ;; )
    {
        ssize_t count = fileIn.Read(buf, WXSIZEOF(buf));
        if ( count == wxInvalidOffset )
            return false;

        // end of file?
        if ( !count )
            break;

        if ( fileOut.Write(buf, count) < (size_t)count )
            return false;
    }

    // we can expect fileIn to be closed successfully, but we should ensure
    // that fileOut was closed as some write errors (disk full) might not be
    // detected before doing this
    return fileIn.Close() && fileOut.Close();
}

#endif // generic implementation of wxCopyFile

// Copy files
bool
wxCopyFile (const std::string& file1, const std::string& file2, bool overwrite)
{
#if defined(__WIN32__)
    // CopyFile() copies file attributes and modification time too, so use it
    // instead of our code if available
    //
    // NB: 3rd parameter is bFailIfExists i.e. the inverse of overwrite
    if ( !::CopyFileW(boost::nowide::widen(file1).c_str(), boost::nowide::widen(file2).c_str(), !overwrite) )
    {
        wxLogSysError(_("Failed to copy the file '%s' to '%s'"),
                      file1.c_str(), file2.c_str());

        return false;
    }
#elif wxUSE_FILE // !Win32

    wxStructStat fbuf;
    // get permissions of file1
    if ( wxStat( file1, &fbuf) != 0 )
    {
        // the file probably doesn't exist or we haven't the rights to read
        // from it anyhow
        wxLogSysError(_("Impossible to get permissions for file '%s'"),
                      file1.c_str());
        return false;
    }

    // open file1 for reading
    wxFile fileIn(file1, wxFile::read);
    if ( !fileIn.IsOpened() )
        return false;

    // remove file2, if it exists. This is needed for creating
    // file2 with the correct permissions in the next step
    if ( wxFileExists(file2)  && (!overwrite || !wxRemoveFile(file2)))
    {
        wxLogSysError(_("Impossible to overwrite the file '%s'"),
                      file2.c_str());
        return false;
    }

    if ( !wxDoCopyFile(fileIn, fbuf, file2, overwrite) )
    {
        wxLogError(_("Error copying the file '%s' to '%s'."), file1, file2);
        return false;
    }

#if defined(__WXMAC__)
    // copy the resource fork of the file too if it's present
    std::string pathRsrcOut;
    wxFile fileRsrcIn;

    {
        // suppress error messages from this block as resource forks don't have
        // to exist
        wxLogNull noLog;

        // it's not enough to check for file existence: it always does on HFS
        // but is empty for files without resources
        if ( fileRsrcIn.Open(file1 + "/..namedfork/rsrc") &&
                fileRsrcIn.Length() > 0 )
        {
            // we must be using HFS or another filesystem with resource fork
            // support, suppose that destination file system also is HFS[-like]
            pathRsrcOut = file2 + "/..namedfork/rsrc";
        }
        else // check if we have resource fork in separate file (non-HFS case)
        {
            wxFileName fnRsrc(file1);
            fnRsrc.SetName("._" + fnRsrc.GetName());

            fileRsrcIn.Close();
            if ( fileRsrcIn.Open( fnRsrc.GetFullPath() ) )
            {
                fnRsrc = file2;
                fnRsrc.SetName("._" + fnRsrc.GetName());

                pathRsrcOut = fnRsrc.GetFullPath();
            }
        }
    }

    if ( !pathRsrcOut.empty() )
    {
        if ( !wxDoCopyFile(fileRsrcIn, fbuf, pathRsrcOut, overwrite) )
            return false;
    }
#endif // wxMac

    if ( chmod(file2.fn_str(), fbuf.st_mode) != 0 )
    {
        wxLogSysError(_("Impossible to set permissions for the file '%s'"),
                      file2.c_str());
        return false;
    }

#else // !Win32 && ! wxUSE_FILE

    // impossible to simulate with wxWidgets API
    wxUnusedVar(file1);
    wxUnusedVar(file2);
    wxUnusedVar(overwrite);
    return false;

#endif // WX_WINDOWS && __WIN32__

    return true;
}

bool
wxRenameFile(const std::string& file1, const std::string& file2, bool overwrite)
{
    if ( !overwrite && wxFileExists(file2) )
    {
        wxLogSysError
        (
            _("Failed to rename the file '%s' to '%s' because the destination file already exists."),
            file1.c_str(), file2.c_str()
        );

        return false;
    }

    // Normal system call
  if ( wxRename (file1, file2) == 0 )
    return true;

  // Try to copy
  if (wxCopyFile(file1, file2, overwrite)) {
    wxRemoveFile(file1);
    return true;
  }
  // Give up
  wxLogSysError(_("File '%s' couldn't be renamed '%s'"), file1, file2);
  return false;
}

bool wxRemoveFile(const std::string& file)
{
#if defined(__VISUALC__) \
 || defined(__GNUWIN32__)
    int res = wxRemove(file);
#elif defined(__WXMAC__)
    int res = unlink(file.fn_str());
#else
    int res = unlink(file.fn_str());
#endif
    if ( res )
    {
        wxLogSysError(_("File '%s' couldn't be removed"), file);
    }
    return res == 0;
}

bool wxMkdir(const std::string& dir, int perm)
{
#if defined(__WXMAC__) && !defined(__UNIX__)
    if ( mkdir(dir.fn_str(), 0) != 0 )

    // assume mkdir() has 2 args on all platforms
    // for the GNU compiler
#elif (!defined(WX_WINDOWS)) || \
      (defined(__GNUWIN32__) && !defined(__MINGW32__)) ||                \
      defined(__WINE__)
    const wxChar *dirname = dir.c_str();
  #if defined(MSVCRT)
    wxUnusedVar(perm);
    if ( mkdir(wxFNCONV(dirname)) != 0 )
  #else
    if ( mkdir(wxFNCONV(dirname), perm) != 0 )
  #endif
#else  // MSW and VC++
    wxUnusedVar(perm);
    if ( wxMkDir(dir) != 0 )
#endif // !MSW/MSW
    {
        wxLogSysError(_("Directory '%s' couldn't be created"), dir);
        return false;
    }

    return true;
}

bool wxRmdir(const std::string& dir, [[maybe_unused]] unsigned int flags)
{
#if defined(__VMS__)
    return false; //to be changed since rmdir exists in VMS7.x
#else
    if ( wxRmDir(dir) != 0 )
    {
        wxLogSysError(_("Directory '%s' couldn't be deleted"), dir);
        return false;
    }

    return true;
#endif
}

// does the path exists? (may have or not '/' or '\\' at the end)
bool wxDirExists(const std::string& pathName)
{
    return wxFileName::DirExists(pathName);
}

// Get first file name matching given wild card.

static std::unique_ptr<wxDir> gs_dir;
static std::string gs_dirPath;

std::string wxFindFirstFile(const std::string& spec, unsigned int flags)
{
    wxFileName::SplitPath(spec, &gs_dirPath, nullptr, nullptr);
    if ( gs_dirPath.empty() )
        gs_dirPath = ".";
    if ( !wxEndsWithPathSeparator(gs_dirPath ) )
        gs_dirPath += wxFILE_SEP_PATH;

    gs_dir.reset(new wxDir(gs_dirPath));

    if ( !gs_dir->IsOpened() )
    {
        wxLogSysError(_("Cannot enumerate files '%s'"), spec);
        return {};
    }

    int dirFlags;
    switch (flags)
    {
        case wxDIR:  dirFlags = wxDIR_DIRS; break;
        case wxFILE: dirFlags = wxDIR_FILES; break;
        default:     dirFlags = wxDIR_DIRS | wxDIR_FILES; break;
    }

    std::string result;
    gs_dir->GetFirst(&result, wxFileNameFromPath(spec), dirFlags);
    if ( result.empty() )
        return result;

    return gs_dirPath + result;
}

std::string wxFindNextFile()
{
    wxCHECK_MSG( gs_dir, "", "You must call wxFindFirstFile before!" );

    std::string result;
    if ( !gs_dir->GetNext(&result) || result.empty() )
        return result;

    return gs_dirPath + result;
}


// Get current working directory.
// If buf is NULL, allocates space using new, else copies into buf.
// wxGetWorkingDirectory() is obsolete, use wxGetCwd()
// wxDoGetCwd() is their common core to be moved
// to wxGetCwd() once wxGetWorkingDirectory() will be removed.
// Do not expose wxDoGetCwd in headers!

wxChar *wxDoGetCwd(wxChar *buf, int sz)
{
    if ( !buf )
    {
        buf = new wxChar[sz + 1];
    }

    bool ok = false;

    // for the compilers which have Unicode version of _getcwd(), call it
    // directly, for the others call the ANSI version and do the translation
    bool needsANSI = true;

    #if !defined(HAVE_WGETCWD)
        char cbuf[_MAXPATHLEN];
    #endif

    #ifdef HAVE_WGETCWD
            char *cbuf = nullptr; // never really used because needsANSI will always be false
            {
                ok = _wgetcwd(buf, sz) != nullptr;
                needsANSI = false;
            }
    #endif

    if ( needsANSI )
    {
    #if defined(_MSC_VER) || defined(__MINGW32__)
        ok = _getcwd(cbuf, sz) != nullptr;
    #else // !Win32/VC++ !Mac
        ok = getcwd(cbuf, sz) != NULL;
    #endif // platform

        // finally convert the result to Unicode if needed
        wxConvFile.MB2WC(buf, cbuf, sz);
    }

    if ( !ok )
    {
        wxLogSysError(_("Failed to get the working directory"));

        // VZ: the old code used to return "." on error which didn't make any
        //     sense at all to me - empty string is a better error indicator
        //     (NULL might be even better but I'm afraid this could lead to
        //     problems with the old code assuming the return is never NULL)
        buf[0] = '\0';
    }
    else // ok, but we might need to massage the path into the right format
    {
// MBN: we hope that in the case the user is compiling a GTK+/Motif app,
//      he needs Unix as opposed to Win32 pathnames
#if defined( __CYGWIN__ ) && defined( WX_WINDOWS )
        // another example of DOS/Unix mix (Cygwin)
        std::string pathUnix = buf;
    #if CYGWIN_VERSION_DLL_MAJOR >= 1007
        cygwin_conv_path(CCP_POSIX_TO_WIN_W, pathUnix.mb_str(wxConvFile), buf, sz);
    #else
        char bufA[_MAXPATHLEN];
        cygwin_conv_to_full_win32_path(pathUnix.mb_str(wxConvFile), bufA);
        wxConvFile.MB2WC(buf, bufA, sz);
    #endif
#endif // __CYGWIN__
    }

    return buf;
}

std::string wxGetCwd()
{
    return boost::nowide::narrow(std::filesystem::current_path().native());
}

bool wxSetWorkingDirectory(const std::string& d)
{
    bool success = false;
#if defined(__UNIX__) || defined(__WXMAC__)
    success = (chdir(d.fn_str()) == 0);
#elif defined(WX_WINDOWS)
    success = (::SetCurrentDirectoryW(boost::nowide::widen(d).c_str()) != 0);
#endif
    if ( !success )
    {
       wxLogSysError(_("Could not set current working directory"));
    }
    return success;
}

// Get the OS directory if appropriate (such as the Windows directory).
// On non-Windows platform, probably just return the empty string.
std::string wxGetOSDirectory()
{
#if defined(WX_WINDOWS)
    boost::nowide::wstackstring buf;

    if ( !::GetWindowsDirectoryW(buf.get(), MAX_PATH) )
    {
        wxLogLastError("GetWindowsDirectory");
    }

    return boost::nowide::narrow(buf.get());
#else
    return {};
#endif
}

bool wxEndsWithPathSeparator(const std::string& filename)
{
    return !filename.empty() && wxIsPathSeparator(filename.back());
}

// find a file in a list of directories, returns false if not found
bool wxFindFileInPath(std::string *pStr, const std::string& szPath, const std::string& szFile)
{
    // we assume that it's not empty
    wxCHECK_MSG( !szFile.empty(), false,
                 "empty file name in wxFindFileInPath");

    // skip path separator in the beginning of the file name if present
    std::string szFile2;
    if ( wxIsPathSeparator(szFile[0u]) )
        szFile2 = szFile.substr(1);
    else
        szFile2 = szFile;

    wxStringTokenizer tkn(szPath, wxPATH_SEP);

    while ( tkn.HasMoreTokens() )
    {
        std::string strFile = tkn.GetNextToken();
        if ( !wxEndsWithPathSeparator(strFile) )
            strFile += wxFILE_SEP_PATH;
        strFile += szFile2;

        if ( wxFileExists(strFile) )
        {
            *pStr = strFile;
            return true;
        }
    }

    return false;
}

#if wxUSE_DATETIME

time_t wxFileModificationTime(const std::string& filename)
{
    wxDateTime mtime;
    if ( !wxFileName(filename).GetTimes(nullptr, &mtime, nullptr) )
        return (time_t)-1;

    return mtime.GetTicks();
}

#endif // wxUSE_DATETIME


// Parses the filterStr, returning the number of filters.
// Returns 0 if none or if there's a problem.
// filterStr is in the form: "All files (*.*)|*.*|JPEG Files (*.jpeg)|*.jpeg"
// TODO: string_view
std::size_t wxParseCommonDialogsFilter(const std::string& filterStr,
                                           std::vector<std::string>& descriptions,
                                           std::vector<std::string>& filters)
{
    descriptions.clear();
    filters.clear();

    std::string str(filterStr);

    std::string description, filter;
    std::size_t pos{0};
    while( pos != std::string::npos )
    {
        pos = str.find('|');
        if ( pos == std::string::npos )
        {
            // if there are no '|'s at all in the string just take the entire
            // string as filter and make description empty for later autocompletion
            if ( filters.empty() )
            {
                descriptions.push_back({});
                filters.push_back(filterStr);
            }
            else
            {
                wxFAIL_MSG( "missing '|' in the wildcard string!" );
            }

            break;
        }

        description = str.substr(0, pos);
        str = str.substr(pos + 1);
        pos = str.find('|');
        if ( pos == std::string::npos )
        {
            filter = str;
        }
        else
        {
            filter = str.substr(0, pos);
            str = str.substr(pos + 1);
        }

        descriptions.push_back(description);
        filters.push_back(filter);
    }

#if defined(__WXMOTIF__)
    // split it so there is one wildcard per entry
    for( size_t i = 0 ; i < descriptions.GetCount() ; i++ )
    {
        pos = filters[i].Find(';');
        if (pos != wxNOT_FOUND)
        {
            // first split only filters
            descriptions.Insert(descriptions[i],i+1);
            filters.Insert(filters[i].Mid(pos+1),i+1);
            filters[i]=filters[i].Left(pos);

            // autoreplace new filter in description with pattern:
            //     C/C++ Files(*.cpp;*.c;*.h)|*.cpp;*.c;*.h
            // cause split into:
            //     C/C++ Files(*.cpp)|*.cpp
            //     C/C++ Files(*.c;*.h)|*.c;*.h
            // and next iteration cause another split into:
            //     C/C++ Files(*.cpp)|*.cpp
            //     C/C++ Files(*.c)|*.c
            //     C/C++ Files(*.h)|*.h
            for ( size_t k=i;k<i+2;k++ )
            {
                pos = descriptions[k].Find(filters[k]);
                if (pos != wxNOT_FOUND)
                {
                    std::string before = descriptions[k].Left(pos);
                    std::string after = descriptions[k].Mid(pos+filters[k].Len());
                    pos = before.Find('(',true);
                    if (pos>before.Find(')',true))
                    {
                        before = before.Left(pos+1);
                        before << filters[k];
                        pos = after.Find(')');
                        int pos1 = after.Find('(');
                        if (pos != wxNOT_FOUND && (pos<pos1 || pos1==wxNOT_FOUND))
                        {
                            before << after.Mid(pos);
                            descriptions[k] = before;
                        }
                    }
                }
            }
        }
    }
#endif

    // autocompletion
    for( size_t j = 0 ; j < descriptions.size() ; j++ )
    {
        if ( descriptions[j].empty() && !filters[j].empty() )
        {
            // FIXME: Translation removed for fmt lib
            descriptions[j] = fmt::format("Files (%s)", filters[j]);
        }
    }

    return filters.size();
}

#if defined(WX_WINDOWS) && !defined(__UNIX__)
static bool wxCheckWin32Permission(const std::string& path, DWORD access)
{
    // quoting the MSDN: "To obtain a handle to a directory, call the
    // CreateFile function with the FILE_FLAG_BACKUP_SEMANTICS flag"
    const DWORD dwAttr = ::GetFileAttributesW(boost::nowide::widen(path).c_str());
    if ( dwAttr == INVALID_FILE_ATTRIBUTES )
    {
        // file probably doesn't exist at all
        return false;
    }

    const HANDLE h = ::CreateFileW
                 (
                    boost::nowide::widen(path).c_str(),
                    access,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr,
                    OPEN_EXISTING,
                    dwAttr & FILE_ATTRIBUTE_DIRECTORY
                        ? FILE_FLAG_BACKUP_SEMANTICS
                        : 0,
                    nullptr
                 );
    if ( h != INVALID_HANDLE_VALUE )
        CloseHandle(h);

    return h != INVALID_HANDLE_VALUE;
}
#endif // WX_WINDOWS

bool wxIsWritable(const std::string &path)
{
#if defined( __UNIX__ )
    // access() will take in count also symbolic links
    return wxAccess(path.c_str(), W_OK) == 0;
#elif defined( WX_WINDOWS )
    return wxCheckWin32Permission(path, GENERIC_WRITE);
#else
    wxUnusedVar(path);
    // TODO
    return false;
#endif
}

bool wxIsReadable(const std::string &path)
{
#if defined( __UNIX__ )
    // access() will take in count also symbolic links
    return wxAccess(path.c_str(), R_OK) == 0;
#elif defined( WX_WINDOWS )
    return wxCheckWin32Permission(path, GENERIC_READ);
#else
    wxUnusedVar(path);
    // TODO
    return false;
#endif
}

bool wxIsExecutable(const std::string &path)
{
#if defined( __UNIX__ )
    // access() will take in count also symbolic links
    return wxAccess(path.c_str(), X_OK) == 0;
#elif defined( WX_WINDOWS )
   return wxCheckWin32Permission(path, GENERIC_EXECUTE);
#else
    wxUnusedVar(path);
    // TODO
    return false;
#endif
}

// Return the type of an open file
//
// Some file types on some platforms seem seekable but in fact are not.
// The main use of this function is to allow such cases to be detected
// (IsSeekable() is implemented as wxGetFileKind() == wxFileKind::Disk).
//
// This is important for the archive streams, which benefit greatly from
// being able to seek on a stream, but which will produce corrupt archives
// if they unknowingly seek on a non-seekable stream.
//
// wxFileKind::Disk is a good catch all return value, since other values
// disable features of the archive streams. Some other value must be returned
// for a file type that appears seekable but isn't.
//
// Known examples:
//   *  Pipes on Windows
//   *  Files on VMS with a record format other than StreamLF
//
wxFileKind wxGetFileKind(int fd)
{
#if defined WX_WINDOWS && defined wxGetOSFHandle
    switch (::GetFileType(wxGetOSFHandle(fd)) & ~FILE_TYPE_REMOTE)
    {
        case FILE_TYPE_CHAR:
            return wxFileKind::Terminal;
        case FILE_TYPE_DISK:
            return wxFileKind::Disk;
        case FILE_TYPE_PIPE:
            return wxFileKind::Pipe;
    }

    return wxFileKind::Unknown;

#elif defined(__UNIX__)
    if (isatty(fd))
        return wxFileKind::Terminal;

    struct stat st;
    fstat(fd, &st);

    if (S_ISFIFO(st.st_mode))
        return wxFileKind::Pipe;
    if (!S_ISREG(st.st_mode))
        return wxFileKind::Unknown;

    #if defined(__VMS__)
        if (st.st_fab_rfm != FAB$C_STMLF)
            return wxFileKind::Unknown;
    #endif

    return wxFileKind::Disk;

#else
    #define wxFILEKIND_STUB
    (void)fd;
    return wxFileKind::Disk;
#endif
}

wxFileKind wxGetFileKind(FILE *fp)
{
#if defined(wxFILEKIND_STUB)
    (void)fp;
    return wxFileKind::Disk;
#elif defined(WX_WINDOWS) && !defined(__CYGWIN__) && !defined(__WINE__)
    return fp ? wxGetFileKind(_fileno(fp)) : wxFileKind::Unknown;
#else
    return fp ? wxGetFileKind(fileno(fp)) : wxFileKind::Unknown;
#endif
}


//------------------------------------------------------------------------
// wild character routines
//------------------------------------------------------------------------

bool wxIsWild( const std::string& pattern )
{
    for ( std::string::const_iterator p = pattern.begin(); p != pattern.end(); ++p )
    {
        switch ( *p )
        {
            case '?':
            case '*':
            case '[':
            case '{':
                return true;

            case '\\':
                if ( ++p == pattern.end() )
                    return false;
        }
    }
    return false;
}

/*
* Written By Douglas A. Lewis <dalewis@cs.Buffalo.EDU>
*
* The match procedure is public domain code (from ircII's reg.c)
* but modified to suit our tastes (RN: No "%" syntax I guess)
*/

bool wxMatchWild( const std::string& pat, const std::string& text, bool dot_special )
{
    if (text.empty())
    {
        /* Match if both are empty. */
        return pat.empty();
    }

    const char* m = pat.c_str(),
    *n = text.c_str(),
    *ma = nullptr,
    *na = nullptr;
    int just = 0,
    acount = 0,
    count = 0;

    if (dot_special && (*n == '.'))
    {
        /* Never match so that hidden Unix files
         * are never found. */
        return false;
    }

    for (;;)
    {
        if (*m == '*')
        {
            ma = ++m;
            na = n;
            just = 1;
            acount = count;
        }
        else if (*m == '?')
        {
            m++;
            if (!*n++)
                return false;
        }
        else
        {
            if (*m == '\\')
            {
                m++;
                /* Quoting "nothing" is a bad thing */
                if (!*m)
                    return false;
            }
            if (!*m)
            {
                /*
                * If we are out of both strings or we just
                * saw a wildcard, then we can say we have a
                * match
                */
                if (!*n)
                    return true;
                if (just)
                    return true;
                just = 0;
                goto not_matched;
            }
            /*
            * We could check for *n == NULL at this point, but
            * since it's more common to have a character there,
            * check to see if they match first (m and n) and
            * then if they don't match, THEN we can check for
            * the NULL of n
            */
            just = 0;
            if (*m == *n)
            {
                m++;
                count++;
                n++;
            }
            else
            {

                not_matched:

                /*
                * If there are no more characters in the
                * string, but we still need to find another
                * character (*m != NULL), then it will be
                * impossible to match it
                */
                if (!*n)
                    return false;

                if (ma)
                {
                    m = ma;
                    n = ++na;
                    count = acount;
                }
                else
                    return false;
            }
        }
    }
}
