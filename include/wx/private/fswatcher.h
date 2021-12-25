/////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/fswatcher.h
// Purpose:     File system watcher impl classes
// Author:      Bartosz Bekier
// Created:     2009-05-26
// Copyright:   (c) 2009 Bartosz Bekier <bartosz.bekier@gmail.com>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef WX_PRIVATE_FSWATCHER_H_
#define WX_PRIVATE_FSWATCHER_H_

#ifdef wxHAS_INOTIFY
    class wxFSWatchEntryUnix;
    using wxFSWatchEntry = wxFSWatchEntryUnix;
    WX_DECLARE_STRING_HASH_MAP(std::shared_ptr<wxFSWatchEntry>,wxFSWatchEntries);
    #include "wx/unix/private/fswatcher_inotify.h"
#elif defined(wxHAS_KQUEUE)
    class wxFSWatchEntryKq;
    using wxFSWatchEntry = wxFSWatchEntryKq;
    WX_DECLARE_STRING_HASH_MAP(std::shared_ptr<wxFSWatchEntry>,wxFSWatchEntries);
    #include "wx/unix/private/fswatcher_kqueue.h"
#elif defined(WX_WINDOWS)
    class wxFSWatchEntryMSW;
    using wxFSWatchEntry = wxFSWatchEntryMSW;
    WX_DECLARE_STRING_HASH_MAP(std::shared_ptr<wxFSWatchEntry>,wxFSWatchEntries);
    #include "wx/msw/private/fswatcher.h"
#else
    using wxFSWatchEntry = wxFSWatchEntryPolling;
#endif

#include "wx/filefn.h"

class wxFSWatcherImpl
{
public:
    wxFSWatcherImpl(wxFileSystemWatcherBase* watcher) :
        m_watcher(watcher)
    {
    }

    virtual ~wxFSWatcherImpl()
    {
        std::ignore = RemoveAll();
    }

    virtual bool Init() = 0;

    virtual bool Add(const wxFSWatchInfo& winfo)
    {
        if ( m_watches.find(winfo.GetPath()) != m_watches.end() )
        {
            wxLogTrace(wxTRACE_FSWATCHER,
                       "Path '%s' is already watched", winfo.GetPath());
            // This can happen if a dir is watched, then a parent tree added
            return true;
        }

        // construct watch entry
        std::shared_ptr<wxFSWatchEntry> watch(new wxFSWatchEntry(winfo));

        if (!DoAdd(watch))
            return false;

        // add watch to our map (always succeeds, checked above)
        wxFSWatchEntries::value_type val(watch->GetPath(), watch);
        return m_watches.insert(val).second;
    }

    virtual bool Remove(const wxFSWatchInfo& winfo)
    {
        wxFSWatchEntries::iterator it = m_watches.find(winfo.GetPath());
        if ( it == m_watches.end() )
        {
            wxLogTrace(wxTRACE_FSWATCHER,
                       "Path '%s' is not watched", winfo.GetPath());
            // This can happen if a dir is watched, then a parent tree added
            return true;
        }
        std::shared_ptr<wxFSWatchEntry> watch = it->second;
        m_watches.erase(it);
        return DoRemove(watch);
    }

    virtual bool RemoveAll()
    {
        bool ret = true;
        for ( wxFSWatchEntries::iterator it = m_watches.begin();
              it != m_watches.end();
              ++it )
        {
            if ( !DoRemove(it->second) )
               ret = false;
        }
        m_watches.clear();
        return ret;
    }

    // Check whether any filespec matches the file's ext (if present)
    bool MatchesFilespec(const wxFileName& fn, const wxString& filespec) const
    {
        return filespec.empty() || wxMatchWild(filespec, fn.GetFullName());
    }

protected:
    virtual bool DoAdd(std::shared_ptr<wxFSWatchEntry> watch) = 0;

    virtual bool DoRemove(std::shared_ptr<wxFSWatchEntry> watch) = 0;

    wxFSWatchEntries m_watches;
    wxFileSystemWatcherBase* m_watcher;
};


#endif /* WX_PRIVATE_FSWATCHER_H_ */
