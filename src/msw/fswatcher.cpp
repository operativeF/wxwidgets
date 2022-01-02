/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/fswatcher.cpp
// Purpose:     wxMSWFileSystemWatcher
// Author:      Bartosz Bekier
// Created:     2009-05-26
// Copyright:   (c) 2009 Bartosz Bekier <bartosz.bekier@gmail.com>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FSWATCHER

#include "wx/msw/private.h"
#include "wx/fswatcher.h"
#include "wx/thread.h"
#include "wx/msw/fswatcher.h"
#include "wx/private/fswatcher.h"

#include <memory>

import WX.WinDef;

import <string>;
import <tuple>;
import <vector>;

class wxFSWatcherImplMSW : public wxFSWatcherImpl
{
public:
    explicit wxFSWatcherImplMSW(wxFileSystemWatcherBase* watcher);

    ~wxFSWatcherImplMSW();

    bool SetUpWatch(wxFSWatchEntryMSW& watch);

    void SendEvent(wxFileSystemWatcherEvent& evt);

protected:
    bool Init() override;

    // adds watch to be monitored for file system changes
    bool DoAdd(std::shared_ptr<wxFSWatchEntryMSW> watch) override;

    bool DoRemove(std::shared_ptr<wxFSWatchEntryMSW> watch) override;

private:
    bool DoSetUpWatch(wxFSWatchEntryMSW& watch);

    static int Watcher2NativeFlags(int flags);

    wxIOCPService m_iocp;
    wxIOCPThread m_workerThread;
};

wxFSWatcherImplMSW::wxFSWatcherImplMSW(wxFileSystemWatcherBase* watcher) :
    wxFSWatcherImpl(watcher),
    m_workerThread(this, &m_iocp)
{
}

wxFSWatcherImplMSW::~wxFSWatcherImplMSW()
{
    // order the worker thread to finish & wait
    m_workerThread.Finish();
    if (m_workerThread.Wait() != nullptr)
    {
        wxLogError(_("Ungraceful worker thread termination"));
    }

    // remove all watches
    std::ignore = RemoveAll();
}

bool wxFSWatcherImplMSW::Init()
{
    wxCHECK_MSG( !m_workerThread.IsAlive(), false,
                 "Watcher service is already initialized" );

    if (m_workerThread.Create() != wxThreadError::None)
    {
        wxLogError(_("Unable to create IOCP worker thread"));
        return false;
    }

    // we have valid iocp service and thread
    if (m_workerThread.Run() != wxThreadError::None)
    {
        wxLogError(_("Unable to start IOCP worker thread"));
        return false;
    }

    return true;
}

// adds watch to be monitored for file system changes
bool wxFSWatcherImplMSW::DoAdd(std::shared_ptr<wxFSWatchEntryMSW> watch)
{
    // setting up wait for directory changes
    if (!DoSetUpWatch(*watch))
        return false;

    // associating handle with completion port
    return m_iocp.Add(watch);
}

bool
wxFSWatcherImplMSW::DoRemove(std::shared_ptr<wxFSWatchEntryMSW> watch)
{
    return m_iocp.ScheduleForRemoval(watch);
}

// TODO ensuring that we have not already set watch for this handle/dir?
bool wxFSWatcherImplMSW::SetUpWatch(wxFSWatchEntryMSW& watch)
{
    wxCHECK_MSG( watch.IsOk(), false, "Invalid watch" );
    if (m_watches.find(watch.GetPath()) == m_watches.end())
    {
        wxLogTrace(wxTRACE_FSWATCHER, "Path '%s' is not watched",
                   watch.GetPath());
        return false;
    }

    wxLogTrace(wxTRACE_FSWATCHER, "Setting up watch for file system changes...");
    return DoSetUpWatch(watch);
}

void wxFSWatcherImplMSW::SendEvent(wxFileSystemWatcherEvent& evt)
{
    // called from worker thread, so posting event in thread-safe way
    wxQueueEvent(m_watcher->GetOwner(), evt.Clone());
}

bool wxFSWatcherImplMSW::DoSetUpWatch(wxFSWatchEntryMSW& watch)
{
    BOOL bWatchSubtree = FALSE;

    switch ( watch.GetType() )
    {
        case wxFSWPathType::File:
            wxLogError(_("Monitoring individual files for changes is not "
                         "supported currently."));
            return false;

        case wxFSWPathType::Dir:
            bWatchSubtree = FALSE;
            break;

        case wxFSWPathType::Tree:
            bWatchSubtree = TRUE;
            break;

        case wxFSWPathType::None:
            wxFAIL_MSG( "Invalid watch type." );
            return false;
    }

    int flags = Watcher2NativeFlags(watch.GetFlags());
    int ret = ReadDirectoryChangesW(watch.GetHandle(), watch.GetBuffer(),
                                    wxFSWatchEntryMSW::BUFFER_SIZE,
                                    bWatchSubtree,
                                    flags, nullptr,
                                    watch.GetOverlapped(), nullptr);
    if (!ret)
    {
        wxLogSysError(_("Unable to set up watch for '%s'"),
                        watch.GetPath());
    }

    return ret != 0;
}

// TODO we should only specify those flags, which interest us
// this needs a bit of thinking, quick impl for now
int wxFSWatcherImplMSW::Watcher2NativeFlags([[maybe_unused]] int flags)
{
    static WXDWORD all_events = FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
            FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE |
            FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION |
            FILE_NOTIFY_CHANGE_SECURITY;

    return all_events;
}


// ============================================================================
// wxFSWatcherImplMSW helper classes implementation
// ============================================================================

wxIOCPThread::wxIOCPThread(wxFSWatcherImplMSW* service, wxIOCPService* iocp) :
    wxThread( wxThreadKind::Joinable),
    m_service(service), m_iocp(iocp)
{
}

// finishes this thread
bool wxIOCPThread::Finish()
{
    wxLogTrace(wxTRACE_FSWATCHER, "Posting empty status!");

    // send "empty" packet to i/o completion port, just to wake
    return m_iocp->PostEmptyStatus();
}

wxThread::ExitCode wxIOCPThread::Entry()
{
    wxLogTrace(wxTRACE_FSWATCHER, "[iocp] Started IOCP thread");

    // read events in a loop until we get false, which means we should exit
    while ( ReadEvents() );

    wxLogTrace(wxTRACE_FSWATCHER, "[iocp] Ended IOCP thread");
    return (ExitCode)nullptr;
}

// wait for events to occur, read them and send to interested parties
// returns false it empty status was read, which means we would exit
//         true otherwise
bool wxIOCPThread::ReadEvents()
{
    WXDWORD count = 0;
    wxFSWatchEntryMSW* watch = nullptr;
    OVERLAPPED* overlapped = nullptr;
    switch ( m_iocp->GetStatus(&count, &watch, &overlapped) )
    {
        case wxIOCPService::Status_OK:
            break; // nothing special to do, continue normally

        case wxIOCPService::Status_Error:
            return true; // error was logged already, we don't want to exit

        case wxIOCPService::Status_Deleted:
            {
                wxFileSystemWatcherEvent
                    removeEvent(wxFSW_EVENT_DELETE,
                                watch->GetPath(),
                                wxFileName());
                SendEvent(removeEvent);
            }

            // It isn't useful to continue watching this directory as it
            // doesn't exist any more -- and even recreating a directory with
            // the same name still wouldn't resume generating events for the
            // existing wxIOCPService, so it's useless to continue.
            return false;

        case wxIOCPService::Status_Exit:
            return false; // stop reading events
    }

    // if the thread got woken up but we got an empty packet it means that
    // there was an overflow, too many events and not all could fit in
    // the watch buffer.  In this case, ReadDirectoryChangesW dumps the
    // buffer.
    if (!count && watch)
    {
         wxLogTrace(wxTRACE_FSWATCHER, "[iocp] Event queue overflowed: path=\"%s\"",
                    watch->GetPath());

        if (watch->GetFlags() & wxFSW_EVENT_WARNING)
        {
            wxFileSystemWatcherEvent
                overflowEvent(wxFSW_EVENT_WARNING, wxFSWWarningType::Overflow);
            overflowEvent.SetPath(watch->GetPath());
            SendEvent(overflowEvent);
        }

        // overflow is not a fatal error, we still want to get future events
        // reissue the watch
        std::ignore = m_service->SetUpWatch(*watch);
        return true;
    }

    // in case of spurious wakeup
    if (!count || !watch)
        return true;

    wxLogTrace( wxTRACE_FSWATCHER, "[iocp] Read entry: path='%s'",
                watch->GetPath());

    // First check if we're still interested in this watch, we could have
    // removed it in the meanwhile.
    if ( m_iocp->CompleteRemoval(watch) )
        return true;

    // extract events from buffer info our vector container
    std::vector<wxEventProcessingData> events;
    const char* memory = static_cast<const char*>(watch->GetBuffer());
    int offset = 0;
    do
    {
        const FILE_NOTIFY_INFORMATION* e =
              static_cast<const FILE_NOTIFY_INFORMATION*>((const void*)memory);

        events.push_back(wxEventProcessingData(e, watch));

        offset = e->NextEntryOffset;
        memory += offset;
    }
    while (offset);

    // process events
    ProcessNativeEvents(events);

    // reissue the watch. ignore possible errors, we will return true anyway
    std::ignore = m_service->SetUpWatch(*watch);

    return true;
}

void wxIOCPThread::ProcessNativeEvents(std::vector<wxEventProcessingData>& events)
{
    std::vector<wxEventProcessingData>::iterator it = events.begin();
    for ( ; it != events.end(); ++it )
    {
        const FILE_NOTIFY_INFORMATION& e = *(it->nativeEvent);
        const wxFSWatchEntryMSW* watch = it->watch;

        wxLogTrace( wxTRACE_FSWATCHER, "[iocp] %s",
                    FileNotifyInformationToString(e));

        const unsigned int nativeFlags = e.Action;
        const unsigned int flags = Native2WatcherFlags(nativeFlags);
        if (flags & wxFSW_EVENT_WARNING || flags & wxFSW_EVENT_ERROR)
        {
            wxFileSystemWatcherEvent
                event(flags,
                      flags & wxFSW_EVENT_ERROR ? wxFSWWarningType::None
                                                : wxFSWWarningType::General);
            SendEvent(event);
        }
        // filter out ignored events and those not asked for.
        // we never filter out warnings or exceptions
        else if ((flags == 0) || !(flags & watch->GetFlags()))
        {
            return;
        }
        // rename case
        else if (nativeFlags == FILE_ACTION_RENAMED_OLD_NAME)
        {
            wxFileName oldpath = GetEventPath(*watch, e);
            wxFileName newpath;

            // FIXME: Look ahead loop
            // newpath should be in the next entry. what if there isn't?
            ++it;
            if ( it != events.end() )
            {
                newpath = GetEventPath(*(it->watch), *(it->nativeEvent));
            }
            wxFileSystemWatcherEvent event(flags, oldpath, newpath);
            SendEvent(event);
        }
        // all other events
        else
        {
            // CHECK I heard that returned path can be either in short on long
            // form...need to account for that!
            wxFileName path = GetEventPath(*watch, e);
            // For files, check that it matches any filespec
            if ( m_service->MatchesFilespec(path, watch->GetFilespec()) )
            {
                wxFileSystemWatcherEvent event(flags, path, path);
                SendEvent(event);
            }
        }
    }
}

void wxIOCPThread::SendEvent(wxFileSystemWatcherEvent& evt)
{
    wxLogTrace(wxTRACE_FSWATCHER, "[iocp] EVT: %s", evt.ToString());
    m_service->SendEvent(evt);
}

int wxIOCPThread::Native2WatcherFlags(int flags)
{
    static constexpr int flag_mapping[][2] = {
        { FILE_ACTION_ADDED,            wxFSW_EVENT_CREATE },
        { FILE_ACTION_REMOVED,          wxFSW_EVENT_DELETE },

        // TODO take attributes into account to see what happened
        { FILE_ACTION_MODIFIED,         wxFSW_EVENT_MODIFY },

        { FILE_ACTION_RENAMED_OLD_NAME, wxFSW_EVENT_RENAME },

        // ignored as it should always be matched with ***_OLD_NAME
        { FILE_ACTION_RENAMED_NEW_NAME, 0 },
        // ignore invalid event
        { 0, 0 },
    };

    for (unsigned int i=0; i < WXSIZEOF(flag_mapping); ++i) {
        if (flags == flag_mapping[i][0])
            return flag_mapping[i][1];
    }

    // never reached
    wxFAIL_MSG(fmt::format("Unknown file notify change %u", flags));
    return -1;
}

std::string wxIOCPThread::FileNotifyInformationToString(
                                              const FILE_NOTIFY_INFORMATION& e)
{
    std::string fname(boost::nowide::narrow(e.FileName), e.FileNameLength / sizeof(e.FileName[0]));
    return fmt::format("Event: offset=%d, action=%d, len=%d, "
                            "name(approx)='%s'", e.NextEntryOffset, e.Action,
                            e.FileNameLength, fname);
}

wxFileName wxIOCPThread::GetEventPath(const wxFSWatchEntryMSW& watch,
                                      const FILE_NOTIFY_INFORMATION& e)
{
    wxFileName path = watch.GetPath();
    if (path.IsDir())
    {
        std::string rel{boost::nowide::narrow(e.FileName), e.FileNameLength / sizeof(e.FileName[0])};
        static constexpr int pathFlags = wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR;
        path = wxFileName(path.GetPath(pathFlags) + rel);
    }
    return path;
}


// ============================================================================
// wxMSWFileSystemWatcher implementation
// ============================================================================

wxMSWFileSystemWatcher::wxMSWFileSystemWatcher() 
    
{
    std::ignore = Init();
}

wxMSWFileSystemWatcher::wxMSWFileSystemWatcher(const wxFileName& path,
                                               int events)
{
    if (!Init())
        return;

    Add(path, events);
}

bool wxMSWFileSystemWatcher::Init()
{
    m_service = new wxFSWatcherImplMSW(this);
    const bool ret = m_service->Init();
    if (!ret)
    {
        delete m_service;
    }

    return ret;
}

bool
wxMSWFileSystemWatcher::AddTree(const wxFileName& path,
                                int events,
                                const std::string& filter)
{
    if ( !filter.empty() )
    {
        // Use the inefficient generic version as we can only monitor
        // everything under the given directory.
        //
        // Notice that it would probably be better to still monitor everything
        // natively and filter out the changes we're not interested in.
        return wxFileSystemWatcherBase::AddTree(path, events, filter);
    }


    if ( !path.DirExists() )
    {
        wxLogError(_("Can't monitor non-existent directory \"%s\" for changes."),
                   path.GetFullPath());
        return false;
    }

    return AddAny(path, events, wxFSWPathType::Tree);
}

#endif // wxUSE_FSWATCHER
