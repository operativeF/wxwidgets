/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fswatcher.h
// Purpose:     wxFileSystemWatcherBase
// Author:      Bartosz Bekier
// Created:     2009-05-23
// Copyright:   (c) 2009 Bartosz Bekier <bartosz.bekier@gmail.com>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FSWATCHER_BASE_H_
#define _WX_FSWATCHER_BASE_H_

#if wxUSE_FSWATCHER

#include "wx/log.h"
#include "wx/event.h"
#include "wx/evtloop.h"
#include "wx/hashmap.h"

import WX.File.Filename;

import <vector>;

inline constexpr char wxTRACE_FSWATCHER[] = "fswatcher";

// ----------------------------------------------------------------------------
// wxFileSystemWatcherEventType & wxFileSystemWatcherEvent
// ----------------------------------------------------------------------------

/**
 * Possible types of file system events.
 * This is a subset that will work fine an all platforms (actually, we will
 * see how it works on Mac).
 *
 * We got 2 types of error events:
 * - warning: these are not fatal and further events can still be generated
 * - error: indicates fatal error and causes that no more events will happen
 */
enum
{
    wxFSW_EVENT_CREATE = 0x01,
    wxFSW_EVENT_DELETE = 0x02,
    wxFSW_EVENT_RENAME = 0x04,
    wxFSW_EVENT_MODIFY = 0x08,
    wxFSW_EVENT_ACCESS = 0x10,
    wxFSW_EVENT_ATTRIB = 0x20, // Currently this is wxGTK-only

    // error events
    wxFSW_EVENT_WARNING = 0x40,
    wxFSW_EVENT_ERROR = 0x80,
    wxFSW_EVENT_ALL = wxFSW_EVENT_CREATE | wxFSW_EVENT_DELETE |
                         wxFSW_EVENT_RENAME | wxFSW_EVENT_MODIFY |
                         wxFSW_EVENT_ACCESS | wxFSW_EVENT_ATTRIB |
                         wxFSW_EVENT_WARNING | wxFSW_EVENT_ERROR
#if defined(wxHAS_INOTIFY) || defined(wxHAVE_FSEVENTS_FILE_NOTIFICATIONS)
    ,wxFSW_EVENT_UNMOUNT = 0x2000
#endif
};

// Type of the path watched, used only internally for now.
enum class wxFSWPathType
{
    None,     // Invalid value for an initialized watch.
    File,     // Plain file.
    Dir,      // Watch a directory and the files in it.
    Tree      // Watch a directory and all its children recursively.
};

// Type of the warning for the events notifying about them.
enum class wxFSWWarningType
{
    None,
    General,
    Overflow
};

/**
 * Event containing information about file system change.
 */
class wxFileSystemWatcherEvent;
wxDECLARE_EVENT(wxEVT_FSWATCHER, wxFileSystemWatcherEvent);

class wxFileSystemWatcherEvent: public wxEvent
{
public:
    // Constructor for any kind of events, also used as default ctor.
    wxFileSystemWatcherEvent(int changeType = 0, int watchid = wxID_ANY) :
        wxEvent(watchid, wxEVT_FSWATCHER),
        m_changeType(changeType)
        
    {
    }

    // Constructor for the error or warning events.
    wxFileSystemWatcherEvent(int changeType,
                             wxFSWWarningType warningType,
                             const std::string& errorMsg = {},
                             int watchid = wxID_ANY) :
        wxEvent(watchid, wxEVT_FSWATCHER),
        m_changeType(changeType),
        m_warningType(warningType),
        m_errorMsg(errorMsg)
    {
    }

    // Constructor for the normal events carrying information about the changes.
    wxFileSystemWatcherEvent(int changeType,
                             const wxFileName& path, const wxFileName& newPath,
                             int watchid = wxID_ANY) :
         wxEvent(watchid, wxEVT_FSWATCHER),
         m_changeType(changeType),
         m_warningType(wxFSWWarningType::None),
         m_path(path),
         m_newPath(newPath)

    {
    }

	wxFileSystemWatcherEvent& operator=(const wxFileSystemWatcherEvent&) = delete;

    /**
     * Returns the path at which the event occurred.
     */
    const wxFileName& GetPath() const
    {
        return m_path;
    }

    /**
     * Sets the path at which the event occurred
     */
    void SetPath(const wxFileName& path)
    {
        m_path = path;
    }

    /**
     * In case of rename(move?) events, returns the new path related to the
     * event. The "new" means newer in the sense of time. In case of other
     * events it returns the same path as GetPath().
     */
    const wxFileName& GetNewPath() const
    {
        return m_newPath;
    }

    /**
     * Sets the new path related to the event. See above.
     */
    void SetNewPath(const wxFileName& path)
    {
        m_newPath = path;
    }

    /**
     * Returns the type of file system event that occurred.
     */
    int GetChangeType() const
    {
        return m_changeType;
    }

    std::unique_ptr<wxEvent> Clone() const override
    {
        auto evt = std::make_unique<wxFileSystemWatcherEvent>(*this);
        evt->m_errorMsg = m_errorMsg;
        evt->m_path = wxFileName(m_path.GetFullPath());
        evt->m_newPath = wxFileName(m_newPath.GetFullPath());
        evt->m_warningType = m_warningType;
        return evt;
    }

    wxEventCategory GetEventCategory() const override
    {
        // TODO this has to be merged with "similar" categories and changed
        return wxEVT_CATEGORY_UNKNOWN;
    }

    /**
     * Returns if this error is an error event
     */
    bool IsError() const
    {
        return (m_changeType & (wxFSW_EVENT_ERROR | wxFSW_EVENT_WARNING)) != 0;
    }

    std::string GetErrorDescription() const
    {
        return m_errorMsg;
    }

    wxFSWWarningType GetWarningType() const
    {
        return m_warningType;
    }

    /**
     * Returns a std::string describing an event useful for debugging or testing
     */
    std::string ToString() const;

protected:
    int m_changeType;
    wxFSWWarningType m_warningType{wxFSWWarningType::None};
    wxFileName m_path;
    wxFileName m_newPath;
    std::string m_errorMsg;
};

typedef void (wxEvtHandler::*wxFileSystemWatcherEventFunction)
                                                (wxFileSystemWatcherEvent&);

#define wxFileSystemWatcherEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxFileSystemWatcherEventFunction, func)

#define EVT_FSWATCHER(winid, func) \
    wx__DECLARE_EVT1(wxEVT_FSWATCHER, winid, wxFileSystemWatcherEventHandler(func))

// ----------------------------------------------------------------------------
// wxFileSystemWatcherBase: interface for wxFileSystemWatcher
// ----------------------------------------------------------------------------

// Simple container to store information about one watched path.
class wxFSWatchInfo
{
public:
    wxFSWatchInfo()  
    = default;

    wxFSWatchInfo(const std::string& path,
                  int events,
                  wxFSWPathType type,
                  const std::string& filespec = {}) :
        m_path(path), m_filespec(filespec), m_events(events), m_type(type),
        m_refcount(1)
    {
    }

    const std::string& GetPath() const
    {
        return m_path;
    }

    const std::string& GetFilespec() const { return m_filespec; }

    int GetFlags() const
    {
        return m_events;
    }

    wxFSWPathType GetType() const
    {
        return m_type;
    }

    // Reference counting of watch entries is used to avoid watching the same
    // file system path multiple times (this can happen even accidentally, e.g.
    // when you have a recursive watch and then decide to watch some file or
    // directory under it separately).
    int IncRef()
    {
        return ++m_refcount;
    }

    int DecRef()
    {
        wxASSERT_MSG( m_refcount > 0, "Trying to decrement a zero count" );
        return --m_refcount;
    }

protected:
    std::string m_path;
    std::string m_filespec;      // For tree watches, holds any filespec to apply
    int m_events{-1};
    wxFSWPathType m_type{wxFSWPathType::None};
    int m_refcount{-1};
};

WX_DECLARE_STRING_HASH_MAP(wxFSWatchInfo, wxFSWatchInfoMap);

/**
 * Encapsulation of platform-specific file system event mechanism
 */
class wxFSWatcherImpl;

/**
 * Main entry point for clients interested in file system events.
 * Defines interface that can be used to receive that kind of events.
 */
class wxFileSystemWatcherBase: public wxEvtHandler
{
public:
    wxFileSystemWatcherBase();

    ~wxFileSystemWatcherBase();

    /**
     * Adds path to currently watched files. Any events concerning this
     * particular path will be sent to handler. Optionally a filter can be
     * specified to receive only events of particular type.
     *
     * Please note that when adding a dir, immediate children will be watched
     * as well.
     */
    virtual bool Add(const wxFileName& path, int events = wxFSW_EVENT_ALL);

    /**
     * Like above, but recursively adds every file/dir in the tree rooted in
     * path. Additionally a file mask can be specified to include only files
     * of particular type.
     */
    virtual bool AddTree(const wxFileName& path, int events = wxFSW_EVENT_ALL,
                         const std::string& filespec = {});

    /**
     * Removes path from the list of watched paths.
     */
    virtual bool Remove(const wxFileName& path);

    /**
     * Same as above, but also removes every file belonging to the tree rooted
     * at path.
     */
    virtual bool RemoveTree(const wxFileName& path);

    /**
     * Clears the list of currently watched paths.
     */
    virtual bool RemoveAll();

    /**
     * Returns the number of watched paths
     */
    std::size_t GetWatchedPathsCount() const;

    /**
     * Retrieves all watched paths and places them in wxArrayString. Returns
     * the number of paths.
     *
     * TODO think about API here: we need to return more information (like is
     * the path watched recursively)
     */
    std::size_t GetWatchedPaths(std::vector<std::string>* paths) const;

    wxEvtHandler* GetOwner() const
    {
        return m_owner;
    }

    void SetOwner(wxEvtHandler* handler)
    {
        if (!handler)
            m_owner = this;
        else
            m_owner = handler;
    }


    // This is a semi-private function used by wxWidgets itself only.
    //
    // Delegates the real work of adding the path to wxFSWatcherImpl::Add() and
    // updates m_watches if the new path was successfully added.
    bool AddAny(const wxFileName& path, int events, wxFSWPathType type,
                const std::string& filespec = {});

protected:

    static std::string GetCanonicalPath(const wxFileName& path)
    {
        wxFileName path_copy = wxFileName(path);
        if ( !path_copy.Normalize() )
        {
            wxFAIL_MSG(fmt::format("Unable to normalize path '%s'",
                                         path.GetFullPath()));
            return {};
        }

        return path_copy.GetFullPath();
    }


    wxFSWatchInfoMap m_watches;        // path=>wxFSWatchInfo map
    wxFSWatcherImpl* m_service{nullptr};     // file system events service
    wxEvtHandler* m_owner;             // handler for file system events

    friend class wxFSWatcherImpl;
};

// include the platform specific file defining wxFileSystemWatcher
// inheriting from wxFileSystemWatcherBase

#ifdef wxHAS_INOTIFY
    #include "wx/unix/fswatcher_inotify.h"
    using wxFileSystemWatcher = wxInotifyFileSystemWatcher;
#elif  defined(wxHAS_KQUEUE) && defined(wxHAVE_FSEVENTS_FILE_NOTIFICATIONS)
    #include "wx/unix/fswatcher_kqueue.h"
    #include "wx/osx/fswatcher_fsevents.h"
    using wxFileSystemWatcher = wxFsEventsFileSystemWatcher;
#elif defined(wxHAS_KQUEUE)
    #include "wx/unix/fswatcher_kqueue.h"
    using wxFileSystemWatcher = wxKqueueFileSystemWatcher;
#elif defined(WX_WINDOWS)
    #include "wx/msw/fswatcher.h"
    using wxFileSystemWatcher = wxMSWFileSystemWatcher;
#else
    #include "wx/generic/fswatcher.h"
    using wxFileSystemWatcher = wxPollingFileSystemWatcher;
#endif

#endif // wxUSE_FSWATCHER

#endif /* _WX_FSWATCHER_BASE_H_ */
