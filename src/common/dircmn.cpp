///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/dircmn.cpp
// Purpose:     wxDir methods common to all implementations
// Author:      Vadim Zeitlin
// Modified by:
// Created:     19.05.01
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/log.h"
#include "wx/intl.h"
#include "wx/filefn.h"
#include "wx/dir.h"
#include "wx/filename.h"

import <string>;

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxDirTraverser
// ----------------------------------------------------------------------------

wxDirTraverseResult
wxDirTraverser::OnOpenError([[maybe_unused]] const std::string& dirname)
{
    return wxDirTraverseResult::Ignore;
}

// ----------------------------------------------------------------------------
// wxDir::HasFiles() and HasSubDirs()
// ----------------------------------------------------------------------------

// dumb generic implementation

bool wxDir::HasFiles(const std::string& spec) const
{
    std::string s;
    return GetFirst(&s, spec, wxDIR_FILES | wxDIR_HIDDEN);
}

// we have a (much) faster version for Unix
#if (defined(__CYGWIN__) && defined(WX_WINDOWS)) || !defined(__UNIX_LIKE__) || defined(__WINE__)

bool wxDir::HasSubDirs(const std::string& spec) const
{
    std::string s;
    return GetFirst(&s, spec, wxDIR_DIRS | wxDIR_HIDDEN);
}

#endif // !Unix

// ----------------------------------------------------------------------------
// wxDir::GetNameWithSep()
// ----------------------------------------------------------------------------

std::string wxDir::GetNameWithSep() const
{
    // Note that for historical reasons (i.e. because GetName() was there
    // first) we implement this one in terms of GetName() even though it might
    // actually make more sense to reverse this logic.

    std::string name = GetName();
    if ( !name.empty() )
    {
        // Notice that even though GetName() isn't supposed to return the
        // separator, it can still be present for the root directory name.
        if ( name.back() != wxFILE_SEP_PATH )
            name += wxFILE_SEP_PATH;
    }

    return name;
}

// ----------------------------------------------------------------------------
// wxDir::Traverse()
// ----------------------------------------------------------------------------

size_t wxDir::Traverse(wxDirTraverser& sink,
                       const std::string& filespec,
                       unsigned int flags) const
{
    wxCHECK_MSG( IsOpened(), (size_t)-1,
                 "dir must be opened before traversing it" );

    // the total number of files found
    size_t nFiles = 0;

    // the name of this dir with path delimiter at the end
    const std::string prefix = GetNameWithSep();

    // first, recurse into subdirs
    if ( flags & wxDIR_DIRS )
    {
        std::string dirname;
        for ( bool cont = GetFirst(&dirname, {},
                                   (flags & ~(wxDIR_FILES | wxDIR_DOTDOT))
                                   | wxDIR_DIRS);
              cont;
              cont = cont && GetNext(&dirname) )
        {
            const std::string fulldirname = prefix + dirname;

            switch ( sink.OnDir(fulldirname) )
            {
                default:
                    wxFAIL_MSG("unexpected OnDir() return value" );
                    [[fallthrough]];

                case wxDirTraverseResult::Stop:
                    cont = false;
                    break;

                case wxDirTraverseResult::Continue:
                    {
                        wxDir subdir;

                        // don't give the error messages for the directories
                        // which we can't open: there can be all sorts of good
                        // reason for this (e.g. insufficient privileges) and
                        // this shouldn't be treated as an error -- instead
                        // let the user code decide what to do
                        bool ok;
                        do
                        {
                            wxLogNull noLog;
                            ok = subdir.Open(fulldirname);
                            if ( !ok )
                            {
                                // ask the user code what to do
                                bool tryagain;
                                switch ( sink.OnOpenError(fulldirname) )
                                {
                                    default:
                                        wxFAIL_MSG("unexpected OnOpenError() return value" );
                                        [[fallthrough]];

                                    case wxDirTraverseResult::Stop:
                                        cont = false;
                                        [[fallthrough]];

                                    case wxDirTraverseResult::Ignore:
                                        tryagain = false;
                                        break;

                                    case wxDirTraverseResult::Continue:
                                        tryagain = true;
                                }

                                if ( !tryagain )
                                    break;
                            }
                        }
                        while ( !ok );

                        if ( ok )
                        {
                            nFiles += subdir.Traverse(sink, filespec, flags);
                        }
                    }
                    break;

                case wxDirTraverseResult::Ignore:
                    // nothing to do
                    break;
            }
        }
    }

    // now enum our own files
    if ( flags & wxDIR_FILES )
    {
        flags &= ~wxDIR_DIRS;

        std::string filename;
        bool cont = GetFirst(&filename, filespec, flags);
        while ( cont )
        {
            wxDirTraverseResult res = sink.OnFile(prefix + filename);
            if ( res == wxDirTraverseResult::Stop )
                break;

            wxASSERT_MSG( res == wxDirTraverseResult::Continue,
                          "unexpected OnFile() return value" );

            nFiles++;

            cont = GetNext(&filename);
        }
    }

    return nFiles;
}

// ----------------------------------------------------------------------------
// wxDir::GetAllFiles()
// ----------------------------------------------------------------------------

class wxDirTraverserSimple : public wxDirTraverser
{
public:
    explicit wxDirTraverserSimple(std::vector<std::string>& files) : m_files(files) { }

    wxDirTraverserSimple(const wxDirTraverserSimple&) = delete;
	wxDirTraverserSimple& operator=(const wxDirTraverserSimple&) = delete;

    wxDirTraverseResult OnFile(const std::string& filename) override
    {
        m_files.push_back(filename);
        return wxDirTraverseResult::Continue;
    }

    wxDirTraverseResult OnDir([[maybe_unused]] const std::string& dirname) override
    {
        return wxDirTraverseResult::Continue;
    }

private:
    std::vector<std::string>& m_files;
};

/* static */
size_t wxDir::GetAllFiles(const std::string& dirname,
                          std::vector<std::string>* files,
                          const std::string& filespec,
                          unsigned int flags)
{
    wxCHECK_MSG( files, (size_t)-1, "NULL pointer in wxDir::GetAllFiles" );

    size_t nFiles = 0;

    wxDir dir(dirname);
    if ( dir.IsOpened() )
    {
        wxDirTraverserSimple traverser(*files);

        nFiles += dir.Traverse(traverser, filespec, flags);
    }

    return nFiles;
}

// ----------------------------------------------------------------------------
// wxDir::FindFirst()
// ----------------------------------------------------------------------------

class wxDirTraverserFindFirst : public wxDirTraverser
{
public:
    wxDirTraverserFindFirst& operator=(wxDirTraverserFindFirst&&) = delete;

    wxDirTraverseResult OnFile(const std::string& filename) override
    {
        m_file = filename;
        return wxDirTraverseResult::Stop;
    }

    wxDirTraverseResult OnDir([[maybe_unused]] const std::string& dirname) override
    {
        return wxDirTraverseResult::Continue;
    }

    const std::string& GetFile() const
    {
        return m_file;
    }

private:
    std::string m_file;
};

/* static */
std::string wxDir::FindFirst(const std::string& dirname,
                          const std::string& filespec,
                          unsigned int flags)
{
    wxDir dir(dirname);
    if ( dir.IsOpened() )
    {
        wxDirTraverserFindFirst traverser;

        dir.Traverse(traverser, filespec, flags | wxDIR_FILES);
        return traverser.GetFile();
    }

    return {};
}


// ----------------------------------------------------------------------------
// wxDir::GetTotalSize()
// ----------------------------------------------------------------------------

#if wxUSE_LONGLONG

class wxDirTraverserSumSize : public wxDirTraverser
{
public:
    wxDirTraverseResult OnFile(const std::string& filename) override
    {
        // wxFileName::GetSize won't use this class again as
        // we're passing it a file and not a directory;
        // thus we are sure to avoid an endless loop
        const wxULongLong sz = wxFileName::GetSize(filename);

        if (sz == wxInvalidSize)
        {
            // if the GetSize() failed (this can happen because e.g. a
            // file is locked by another process), we can proceed but
            // we need to at least warn the user that the resulting
            // final size could be not reliable (if e.g. the locked
            // file is very big).
            m_skippedFiles.push_back(filename);
            return wxDirTraverseResult::Continue;
        }

        m_sz += sz;
        return wxDirTraverseResult::Continue;
    }

    wxDirTraverseResult OnDir([[maybe_unused]] const std::string& dirname) override
    {
        return wxDirTraverseResult::Continue;
    }

    wxULongLong GetTotalSize() const
        { return m_sz; }
    const std::vector<std::string>& GetSkippedFiles() const
        { return m_skippedFiles; }

protected:
    wxULongLong m_sz;
    std::vector<std::string> m_skippedFiles;
};

wxULongLong wxDir::GetTotalSize(const std::string &dirname, std::vector<std::string>* filesSkipped)
{
    if (!wxDirExists(dirname))
        return wxInvalidSize;

    // to get the size of this directory and its contents we need
    // to recursively walk it...
    wxDir dir(dirname);
    if ( !dir.IsOpened() )
        return wxInvalidSize;

    wxDirTraverserSumSize traverser;
    if (dir.Traverse(traverser) == (size_t)-1 )
        return wxInvalidSize;

    if (filesSkipped)
        *filesSkipped = traverser.GetSkippedFiles();

    return traverser.GetTotalSize();
}

#endif // wxUSE_LONGLONG

// ----------------------------------------------------------------------------
// wxDir helpers
// ----------------------------------------------------------------------------

/* static */
bool wxDir::Exists(const std::string& dir)
{
    return wxFileName::DirExists(dir);
}

/* static */
bool wxDir::Make(const std::string &dir, unsigned int perm, unsigned int flags)
{
    return wxFileName::Mkdir(dir, perm, flags);
}

/* static */
bool wxDir::Remove(const std::string &dir, unsigned int flags)
{
    return wxFileName::Rmdir(dir, flags);
}

