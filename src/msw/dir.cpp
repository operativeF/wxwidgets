/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/dir.cpp
// Purpose:     wxDir implementation for Win32
// Author:      Vadim Zeitlin
// Modified by:
// Created:     08.12.99
// Copyright:   (c) 1999 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/log.h"
#endif // PCH

#include "wx/dir.h"
#include "wx/stringutils.h"

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

#ifdef __WINDOWS__
    #include "wx/msw/private.h"
#endif

// ----------------------------------------------------------------------------
// define the types and functions used for file searching
// ----------------------------------------------------------------------------

namespace
{

using FIND_STRUCT = WIN32_FIND_DATA;
using FIND_DATA = HANDLE;
using FIND_ATTR = DWORD;

inline FIND_DATA InitFindData()
{
    return INVALID_HANDLE_VALUE;
}

inline bool IsFindDataOk(FIND_DATA fd)
{
        return fd != INVALID_HANDLE_VALUE;
}

inline void FreeFindData(FIND_DATA fd)
{
    if ( !::FindClose(fd) )
    {
        wxLogLastError(wxT("FindClose"));
    }
}

const char* GetNameFromFindData(const FIND_STRUCT *finddata)
{
    // FIXME: isn't this going to dangle?
    return boost::nowide::narrow(finddata->cFileName).c_str();
}

// Helper function checking that the contents of the given FIND_STRUCT really
// match our filter. We need to do it ourselves as native Windows functions
// apply the filter to both the long and the short names of the file, so
// something like "*.bar" matches "foo.bar.baz" too and not only "foo.bar", so
// we have to double check that we have a real match.
inline bool
CheckFoundMatch(const FIND_STRUCT* finddata, const std::string& filter)
{
    // If there is no filter, the found file must be the one we really are
    // looking for.
    if ( filter.empty() )
        return true;

    // Otherwise do check the match validity. Notice that we must do it
    // case-insensitively because the case of the file names is not supposed to
    // matter under Windows.
    std::string fn(GetNameFromFindData(finddata));

    // However if the filter contains only special characters (which is a
    // common case), we can skip the case conversion.
    if ( filter.find_first_not_of("*?.") == std::string::npos )
        return wx::utils::Matches(fn, filter);

    return wx::utils::Matches(wx::utils::ToUpperCopy(fn), wx::utils::ToUpperCopy(filter));
}

inline bool
FindNext(FIND_DATA fd, const std::string& filter, FIND_STRUCT *finddata)
{
    for ( ;; )
    {
        if ( !::FindNextFileW(fd, finddata) )
            return false;

        // If we did find something, check that it really matches.
        if ( CheckFoundMatch(finddata, filter) )
            return true;
    }
}

inline FIND_DATA
FindFirst(const std::string& spec,
          const std::string& filter,
          FIND_STRUCT *finddata)
{
    std::wstring stackSpec = boost::nowide::widen(spec.c_str());
    FIND_DATA fd = ::FindFirstFileW(&stackSpec[0], finddata);

    // As in FindNext() above, we need to check that the file name we found
    // really matches our filter and look for the next match if it doesn't.
    if ( IsFindDataOk(fd) && !CheckFoundMatch(finddata, filter) )
    {
        if ( !FindNext(fd, filter, finddata) )
        {
            // As we return the invalid handle from here to indicate that we
            // didn't find anything, close the one we initially received
            // ourselves.
            FreeFindData(fd);

            return INVALID_HANDLE_VALUE;
        }
    }

    return fd;
}

inline FIND_ATTR GetAttrFromFindData(FIND_STRUCT *finddata)
{
    return finddata->dwFileAttributes;
}

inline bool IsDir(FIND_ATTR attr)
{
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

inline bool IsHidden(FIND_ATTR attr)
{
    return (attr & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) != 0;
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

#ifndef MAX_PATH
    #define MAX_PATH 260        // from VC++ headers
#endif

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

#define M_DIR       ((wxDirData *)m_data)

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// this class stores everything we need to enumerate the files
class wxDirData
{
public:
    explicit wxDirData(const std::string& dirname);
    ~wxDirData();

    wxDirData(const wxDirData&) = delete;
	wxDirData& operator=(const wxDirData&) = delete;

    void SetFileSpec(const std::string& filespec) { m_filespec = filespec; }
    void SetFlags(int flags) { m_flags = flags; }

    void Close();
    void Rewind();
    bool Read(std::string *filename);

    const std::string& GetName() const { return m_dirname; }

private:
    FIND_DATA m_finddata;

    std::string m_dirname;
    std::string m_filespec;

    int      m_flags{0};
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxDirData
// ----------------------------------------------------------------------------

wxDirData::wxDirData(const std::string& dirname)
    : m_finddata(InitFindData())
    , m_dirname(dirname)
{
}

wxDirData::~wxDirData()
{
    Close();
}

void wxDirData::Close()
{
    if ( IsFindDataOk(m_finddata) )
    {
        FreeFindData(m_finddata);

        m_finddata = InitFindData();
    }
}

void wxDirData::Rewind()
{
    Close();
}

bool wxDirData::Read(std::string *filename)
{
    bool first = false;

    WIN32_FIND_DATA finddata;
    #define PTR_TO_FINDDATA (&finddata)

    if ( !IsFindDataOk(m_finddata) )
    {
        // open first
        std::string filespec = m_dirname;
        if ( !wxEndsWithPathSeparator(filespec) )
        {
            filespec += '\\';
        }
        if ( m_filespec.empty() )
            filespec += "*.*";
        else
            filespec += m_filespec;

        m_finddata = FindFirst(filespec, m_filespec, PTR_TO_FINDDATA);

        first = true;
    }

    if ( !IsFindDataOk(m_finddata) )
    {
        const DWORD err = ::GetLastError();

        if ( err != ERROR_FILE_NOT_FOUND && err != ERROR_NO_MORE_FILES )
        {
            wxLogSysError(err, _("Cannot enumerate files in directory '%s'"),
                          m_dirname.c_str());
        }
        //else: not an error, just no (such) files

        return false;
    }

    const char* name;
    FIND_ATTR attr;

    for ( ;; )
    {
        if ( first )
        {
            first = false;
        }
        else
        {
            if ( !FindNext(m_finddata, m_filespec, PTR_TO_FINDDATA) )
            {
                const DWORD err = ::GetLastError();

                if ( err != ERROR_NO_MORE_FILES )
                {
                    wxLogLastError(wxT("FindNext"));
                }
                //else: not an error, just no more (such) files

                return false;
            }
        }

        name = GetNameFromFindData(PTR_TO_FINDDATA);
        attr = GetAttrFromFindData(PTR_TO_FINDDATA);

        // don't return "." and ".." unless asked for
        if ( name[0] == wxT('.') &&
             ((name[1] == wxT('.') && name[2] == wxT('\0')) ||
              (name[1] == wxT('\0'))) )
        {
            if ( !(m_flags & wxDIR_DOTDOT) )
                continue;
        }

        // check the type now
        if ( !(m_flags & wxDIR_FILES) && !IsDir(attr) )
        {
            // it's a file, but we don't want them
            continue;
        }
        else if ( !(m_flags & wxDIR_DIRS) && IsDir(attr) )
        {
            // it's a dir, and we don't want it
            continue;
        }

        // finally, check whether it's a hidden file
        if ( !(m_flags & wxDIR_HIDDEN) )
        {
            if ( IsHidden(attr) )
            {
                // it's a hidden file, skip it
                continue;
            }
        }

        *filename = name;

        break;
    }

    return true;
}

// ----------------------------------------------------------------------------
// wxDir construction/destruction
// ----------------------------------------------------------------------------

wxDir::wxDir(const std::string& dirname)
{
    std::ignore = Open(dirname);
}

bool wxDir::Open(const std::string& dirname)
{
    delete M_DIR;

    // The Unix code does a similar test
    if (wxDirExists(dirname))
    {
        m_data = new wxDirData(dirname);

        return true;
    }
    else
    {
        m_data = nullptr;

        return false;
    }
}

bool wxDir::IsOpened() const
{
    return m_data != nullptr;
}

std::string wxDir::GetName() const
{
    std::string name;
    if ( m_data )
    {
        name = M_DIR->GetName();
        if ( !name.empty() )
        {
            // bring to canonical Windows form
            wx::utils::ReplaceAll(name, "/", "\\");

            if ( name.back() == '\\' )
            {
                // chop off the last (back)slash
                // TODO: Is this correct?
                name.resize(name.length() - 1);
            }
        }
    }

    return name;
}

void wxDir::Close()
{
    if ( m_data )
    {
        delete m_data;
        m_data = nullptr;
    }
}

// ----------------------------------------------------------------------------
// wxDir enumerating
// ----------------------------------------------------------------------------

bool wxDir::GetFirst(std::string *filename,
                     const std::string& filespec,
                     int flags) const
{
    wxCHECK_MSG( IsOpened(), false, wxT("must wxDir::Open() first") );

    M_DIR->Rewind();

    M_DIR->SetFileSpec(filespec);
    M_DIR->SetFlags(flags);

    return GetNext(filename);
}

bool wxDir::GetNext(std::string *filename) const
{
    wxCHECK_MSG( IsOpened(), false, wxT("must wxDir::Open() first") );

    wxCHECK_MSG( filename, false, wxT("bad pointer in wxDir::GetNext()") );

    return M_DIR->Read(filename);
}

// ----------------------------------------------------------------------------
// wxGetDirectoryTimes: used by wxFileName::GetTimes()
// ----------------------------------------------------------------------------

extern bool
wxGetDirectoryTimes(const std::string& dirname,
                    FILETIME *ftAccess, FILETIME *ftCreate, FILETIME *ftMod)
{
    // FindFirst() is going to fail
    wxASSERT_MSG( !dirname.empty() && dirname.back() != '\\',
                  wxT("incorrect directory name format in wxGetDirectoryTimes") );

    FIND_STRUCT fs;
    FIND_DATA fd = FindFirst(dirname, "", &fs);
    if ( !IsFindDataOk(fd) )
    {
        return false;
    }

    *ftAccess = fs.ftLastAccessTime;
    *ftCreate = fs.ftCreationTime;
    *ftMod = fs.ftLastWriteTime;

    FindClose(fd);

    return true;
}
