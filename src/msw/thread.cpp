/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/thread.cpp
// Purpose:     wxThread Implementation
// Author:      Original from Wolfram Gloger/Guilhem Lavaux
// Modified by: Vadim Zeitlin to make it work :-)
// Created:     04/22/98
// Copyright:   (c) Wolfram Gloger (1996, 1997), Guilhem Lavaux (1998);
//                  Vadim Zeitlin (1999-2002)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_THREADS

#include "wx/thread.h"

#include "wx/msw/private.h"

#include "wx/intl.h"
#include "wx/app.h"
#include "wx/log.h"
#include "wx/module.h"
#include "wx/msgout.h"
#include "wx/apptrait.h"
#include "wx/scopeguard.h"

#include "wx/msw/seh.h"

#include "wx/except.h"

#include <fmt/core.h>

#include <cassert>

import WX.Utils.Cast;

import WX.WinDef;

// must have this symbol defined to get _beginthread/_endthread declarations
#ifndef _MT
    #define _MT
#endif

// define wxUSE_BEGIN_THREAD if the compiler has _beginthreadex() function
// which should be used instead of Win32 ::CreateThread() if possible
#if defined(_MSC_VER) || \
    (defined(__GNUG__) && defined(__MSVCRT__))

    #undef wxUSE_BEGIN_THREAD
    #define wxUSE_BEGIN_THREAD

#endif

#ifdef wxUSE_BEGIN_THREAD
    // this is where _beginthreadex() is declared
    #include <process.h>

    // the return type of the thread function entry point: notice that this
    // type can't hold a pointer under Win64
    using THREAD_RETVAL = unsigned int;

    // the calling convention of the thread function entry point
    #define THREAD_CALLCONV __stdcall
#else
    // the settings for CreateThread()
    using THREAD_RETVAL = WXDWORD;
    #define THREAD_CALLCONV WINAPI
#endif

constexpr THREAD_RETVAL THREAD_ERROR_EXIT = (THREAD_RETVAL)-1;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// the possible states of the thread ("=>" shows all possible transitions from
// this state)
enum class wxThreadState
{
    New,          // didn't start execution yet (=> RUNNING)
    Running,      // thread is running (=> PAUSED, CANCELED)
    Paused,       // thread is temporarily suspended (=> RUNNING)
    Canceled,     // thread should terminate a.s.a.p. (=> EXITED)
    Exited        // thread is terminating
};

// ----------------------------------------------------------------------------
// this module globals
// ----------------------------------------------------------------------------

// TLS index of the slot where we store the pointer to the current thread, the
// initial value is special and means that it's not initialized yet
static WXDWORD gs_tlsThisThread = TLS_OUT_OF_INDEXES;

// id of the main thread - the one which can call GUI functions without first
// calling wxMutexGuiEnter()
wxThreadIdType wxThread::ms_idMainThread = 0;

// if it's false, some secondary thread is holding the GUI lock
static bool gs_bGuiOwnedByMainThread = true;

// critical section which controls access to all GUI functions: any secondary
// thread (i.e. except the main one) must enter this crit section before doing
// any GUI calls
static wxCriticalSection *gs_critsectGui = nullptr;

// critical section which protects gs_nWaitingForGui variable
static wxCriticalSection *gs_critsectWaitingForGui = nullptr;

// number of threads waiting for GUI in wxMutexGuiEnter()
static size_t gs_nWaitingForGui = 0;

// are we waiting for a thread termination?
static bool gs_waitingForThread = false;

// ============================================================================
// Windows implementation of thread and related classes
// ============================================================================

// ----------------------------------------------------------------------------
// wxCriticalSection
// ----------------------------------------------------------------------------

wxCriticalSection::wxCriticalSection( [[maybe_unused]] wxCriticalSectionType critSecType )
{
    static_assert(sizeof(CRITICAL_SECTION) <= sizeof(wxCritSectBuffer),
                  "Critical section buffer too small.");

    ::InitializeCriticalSection((CRITICAL_SECTION *)m_buffer);
}

wxCriticalSection::~wxCriticalSection()
{
    ::DeleteCriticalSection((CRITICAL_SECTION *)m_buffer);
}

void wxCriticalSection::Enter()
{
    ::EnterCriticalSection((CRITICAL_SECTION *)m_buffer);
}

bool wxCriticalSection::TryEnter()
{
    return ::TryEnterCriticalSection((CRITICAL_SECTION *)m_buffer) != 0;
}

void wxCriticalSection::Leave()
{
    ::LeaveCriticalSection((CRITICAL_SECTION *)m_buffer);
}

// ----------------------------------------------------------------------------
// wxMutex
// ----------------------------------------------------------------------------

class wxMutexInternal
{
public:
    explicit wxMutexInternal(wxMutexType mutexType);
    ~wxMutexInternal();

    wxMutexInternal(const wxMutexInternal&) = delete;
	wxMutexInternal& operator=(const wxMutexInternal&) = delete;

    bool IsOk() const { return m_mutex != nullptr; }

    wxMutexError Lock() { return LockTimeout(INFINITE); }
    wxMutexError Lock(unsigned long ms) { return LockTimeout(ms); }
    wxMutexError TryLock();
    wxMutexError Unlock();

private:
    wxMutexError LockTimeout(WXDWORD milliseconds);

    HANDLE m_mutex;

    unsigned long m_owningThread;
    wxMutexType m_type;
};

// all mutexes are recursive under Win32 so we don't use mutexType
wxMutexInternal::wxMutexInternal(wxMutexType mutexType)
{
    // create a nameless (hence intra process and always private) mutex
    m_mutex = ::CreateMutexW
                (
                    nullptr,       // default secutiry attributes
                    FALSE,      // not initially locked
                    nullptr        // no name
                );

    m_type = mutexType;
    m_owningThread = 0;

    if ( !m_mutex )
    {
        wxLogLastError("CreateMutex()");
    }

}

wxMutexInternal::~wxMutexInternal()
{
    if ( m_mutex )
    {
        if ( !::CloseHandle(m_mutex) )
        {
            wxLogLastError("CloseHandle(mutex)");
        }
    }
}

wxMutexError wxMutexInternal::TryLock()
{
    const wxMutexError rc = LockTimeout(0);

    // we have a special return code for timeout in this case
    return rc == wxMutexError::Timeout ? wxMutexError::Busy : rc;
}

wxMutexError wxMutexInternal::LockTimeout(WXDWORD milliseconds)
{
    if (m_type == wxMutexType::Default)
    {
        // Don't allow recursive
        if (m_owningThread != 0)
        {
            if (m_owningThread == wxThread::GetCurrentId())
                return wxMutexError::DeadLock;
        }
    }

    WXDWORD rc = ::WaitForSingleObject(m_mutex, milliseconds);
    switch ( rc )
    {
        case WAIT_ABANDONED:
            // the previous caller died without releasing the mutex, so even
            // though we did get it, log a message about this
            wxLogDebug("WaitForSingleObject() returned WAIT_ABANDONED");
            [[fallthrough]];

        case WAIT_OBJECT_0:
            // ok
            break;

        case WAIT_TIMEOUT:
            return wxMutexError::Timeout;

        default:
            wxFAIL_MSG("impossible return value in wxMutex::Lock");
            [[fallthrough]];

        case WAIT_FAILED:
            wxLogLastError("WaitForSingleObject(mutex)");
            return wxMutexError::MiscError;
    }

    if (m_type == wxMutexType::Default)
    {
        // required for checking recursiveness
        m_owningThread = wxThread::GetCurrentId();
    }

    return wxMutexError::None;
}

wxMutexError wxMutexInternal::Unlock()
{
    // required for checking recursiveness
    m_owningThread = 0;

    if ( !::ReleaseMutex(m_mutex) )
    {
        wxLogLastError("ReleaseMutex()");

        return wxMutexError::MiscError;
    }

    return wxMutexError::None;
}

// --------------------------------------------------------------------------
// wxSemaphore
// --------------------------------------------------------------------------

// a trivial wrapper around Win32 semaphore
class wxSemaphoreInternal
{
public:
    wxSemaphoreInternal(int initialcount, int maxcount);
    ~wxSemaphoreInternal();

    wxSemaphoreInternal(const wxSemaphoreInternal&) = delete;
	wxSemaphoreInternal& operator=(const wxSemaphoreInternal&) = delete;

    bool IsOk() const { return m_semaphore != nullptr; }

    wxSemaError Wait() { return WaitTimeout(INFINITE); }

    wxSemaError TryWait()
    {
        wxSemaError rc = WaitTimeout(0);
        if ( rc == wxSemaError::Timeout )
            rc = wxSemaError::Busy;

        return rc;
    }

    wxSemaError WaitTimeout(unsigned long milliseconds);

    wxSemaError Post();

private:
    HANDLE m_semaphore;
};

wxSemaphoreInternal::wxSemaphoreInternal(int initialcount, int maxcount)
{
    if ( maxcount == 0 )
    {
        // make it practically infinite
        maxcount = std::numeric_limits<int>::max();
    }

    m_semaphore = ::CreateSemaphoreW
                    (
                        nullptr,           // default security attributes
                        initialcount,
                        maxcount,
                        nullptr            // no name
                    );
    if ( !m_semaphore )
    {
        wxLogLastError("CreateSemaphore()");
    }
}

wxSemaphoreInternal::~wxSemaphoreInternal()
{
    if ( m_semaphore )
    {
        if ( !::CloseHandle(m_semaphore) )
        {
            wxLogLastError("CloseHandle(semaphore)");
        }
    }
}

wxSemaError wxSemaphoreInternal::WaitTimeout(unsigned long milliseconds)
{
    WXDWORD rc = ::WaitForSingleObject( m_semaphore, milliseconds );

    switch ( rc )
    {
        case WAIT_OBJECT_0:
           return wxSemaError::None;

        case WAIT_TIMEOUT:
           return wxSemaError::Timeout;

        default:
            wxLogLastError("WaitForSingleObject(semaphore)");
    }

    return wxSemaError::MiscError;
}

wxSemaError wxSemaphoreInternal::Post()
{
    if ( !::ReleaseSemaphore(m_semaphore, 1, nullptr /* ptr to previous count */) )
    {
        if ( GetLastError() == ERROR_TOO_MANY_POSTS )
        {
            return wxSemaError::Overflow;
        }
        else
        {
            wxLogLastError("ReleaseSemaphore");
            return wxSemaError::MiscError;
        }
    }

    return wxSemaError::None;
}

// ----------------------------------------------------------------------------
// wxThread implementation
// ----------------------------------------------------------------------------

// wxThreadInternal class
// ----------------------

class wxThreadInternal
{
public:
    explicit wxThreadInternal(wxThread *thread)
        : m_thread{thread}
    {
    }

    ~wxThreadInternal()
    {
        Free();
    }

    wxThreadInternal(const wxThreadInternal&) = delete;
	wxThreadInternal& operator=(const wxThreadInternal&) = delete;

    void Free()
    {
        if ( m_hThread )
        {
            if ( !::CloseHandle(m_hThread) )
            {
                wxLogLastError("CloseHandle(thread)");
            }

            m_hThread = nullptr;
        }
    }

    // create a new (suspended) thread (for the given thread object)
    bool Create(wxThread *thread, unsigned int stackSize);

    // wait for the thread to terminate, either by itself, or by asking it
    // (politely, this is not Kill()!) to do it
    wxThreadError WaitForTerminate(wxCriticalSection& cs,
                                   wxThread::ExitCode *pRc,
                                   wxThreadWait waitMode,
                                   wxThread *threadToDelete = nullptr);

    // kill the thread unconditionally
    wxThreadError Kill();

    // suspend/resume/terminate
    bool Suspend();
    bool Resume();
    void Cancel() { m_state = wxThreadState::Canceled; }

    // thread state
    void SetState(wxThreadState state) { m_state = state; }
    wxThreadState GetState() const { return m_state; }

    // thread priority
    void SetPriority(unsigned int priority);
    unsigned int GetPriority() const { return m_priority; }

    // thread handle and id
    HANDLE GetHandle() const { return m_hThread; }
    WXDWORD  GetId() const { return m_tid; }

    // the thread function forwarding to DoThreadStart
    static THREAD_RETVAL THREAD_CALLCONV WinThreadStart(void *thread);

    // really start the thread (if it's not already dead)
    static THREAD_RETVAL DoThreadStart(wxThread *thread);

    // call OnExit() on the thread
    static void DoThreadOnExit(wxThread *thread);


    void KeepAlive()
    {
        if ( m_thread->IsDetached() )
            ::InterlockedIncrement(&m_nRef);
    }

    void LetDie()
    {
        if ( m_thread->IsDetached() && !::InterlockedDecrement(&m_nRef) )
            delete m_thread;
    }

private:
    // the thread we're associated with
    wxThread *m_thread;

    HANDLE        m_hThread{nullptr};    // handle of the thread
    wxThreadState m_state{wxThreadState::New};      // state, see wxThreadState enum
    unsigned int  m_priority{wxPRIORITY_DEFAULT};   // thread priority in "wx" units
    WXDWORD         m_tid;        // thread id

    // number of threads which need this thread to remain alive, when the count
    // reaches 0 we kill the owning wxThread -- and die ourselves with it
    LONG m_nRef{1};
};

// small class which keeps a thread alive during its lifetime
class wxThreadKeepAlive
{
public:
    explicit wxThreadKeepAlive(wxThreadInternal& thrImpl) : m_thrImpl(thrImpl)
        { m_thrImpl.KeepAlive(); }
    ~wxThreadKeepAlive()
        { m_thrImpl.LetDie(); }

private:
    wxThreadInternal& m_thrImpl;
};

/* static */
void wxThreadInternal::DoThreadOnExit(wxThread *thread)
{
    wxTRY
    {
        thread->OnExit();
    }
    wxCATCH_ALL( wxTheApp->OnUnhandledException(); )
}

/* static */
THREAD_RETVAL wxThreadInternal::DoThreadStart(wxThread *thread)
{
    wxON_BLOCK_EXIT1(DoThreadOnExit, thread);

    THREAD_RETVAL rc = THREAD_ERROR_EXIT;

    wxTRY
    {
        // store the thread object in the TLS
        wxASSERT_MSG( gs_tlsThisThread != TLS_OUT_OF_INDEXES,
                      "TLS index not set. Is wx initialized?" );

        if ( !::TlsSetValue(gs_tlsThisThread, thread) )
        {
            wxLogSysError(_("Cannot start thread: error writing TLS."));

            return THREAD_ERROR_EXIT;
        }

        rc = wxPtrToUInt(thread->CallEntry());
    }
    wxCATCH_ALL( wxTheApp->OnUnhandledException(); )

    return rc;
}

/* static */
THREAD_RETVAL THREAD_CALLCONV wxThreadInternal::WinThreadStart(void *param)
{
    THREAD_RETVAL rc = THREAD_ERROR_EXIT;

    wxThread * const thread = (wxThread *)param;

    // each thread has its own SEH translator so install our own a.s.a.p.
    DisableAutomaticSETranslator();

    // NB: Notice that we can't use wxCriticalSectionLocker in this function as
    //     we use SEH and it's incompatible with C++ object dtors.

    // first of all, check whether we hadn't been cancelled already and don't
    // start the user code at all then
    thread->m_critsect.Enter();
    const bool hasExited = thread->m_internal->GetState() == wxThreadState::Exited;
    thread->m_critsect.Leave();

    // run the thread function itself inside a SEH try/except block
    wxSEH_TRY
    {
        if ( hasExited )
            DoThreadOnExit(thread);
        else
            rc = DoThreadStart(thread);
    }
    wxSEH_HANDLE(THREAD_ERROR_EXIT)


    // save IsDetached because thread object can be deleted by joinable
    // threads after state is changed to wxThreadState::Exited.
    const bool isDetached = thread->IsDetached();
    if ( !hasExited )
    {
        thread->m_critsect.Enter();
        thread->m_internal->SetState(wxThreadState::Exited);
        thread->m_critsect.Leave();
    }

    // the thread may delete itself now if it wants, we don't need it any more
    if ( isDetached )
        thread->m_internal->LetDie();

    return rc;
}

void wxThreadInternal::SetPriority(unsigned int priority)
{
    m_priority = priority;

    if ( !m_hThread )
    {
        // The thread hasn't been created yet, so calling SetThreadPriority()
        // right now would just result in an error -- just skip doing this, as
        // the priority will be set when Create() is called later.
        return;
    }

    // translate wxWidgets priority to the Windows one
    int win_priority;
    if (m_priority <= 20)
        win_priority = THREAD_PRIORITY_LOWEST;
    else if (m_priority <= 40)
        win_priority = THREAD_PRIORITY_BELOW_NORMAL;
    else if (m_priority <= 60)
        win_priority = THREAD_PRIORITY_NORMAL;
    else if (m_priority <= 80)
        win_priority = THREAD_PRIORITY_ABOVE_NORMAL;
    else if (m_priority <= 100)
        win_priority = THREAD_PRIORITY_HIGHEST;
    else
    {
        wxFAIL_MSG("invalid value of thread priority parameter");
        win_priority = THREAD_PRIORITY_NORMAL;
    }

    if ( !::SetThreadPriority(m_hThread, win_priority) )
    {
        wxLogSysError(_("Can't set thread priority"));
    }
}

bool wxThreadInternal::Create(wxThread *thread, unsigned int stackSize)
{
    wxASSERT_MSG( m_state == wxThreadState::New && !m_hThread,
                    "Create()ing thread twice?" );

    // for compilers which have it, we should use C RTL function for thread
    // creation instead of Win32 API one because otherwise we will have memory
    // leaks if the thread uses C RTL (and most threads do)
#ifdef wxUSE_BEGIN_THREAD
    m_hThread = (HANDLE)_beginthreadex
                        (
                          nullptr,                             // default security
                          stackSize,
                          wxThreadInternal::WinThreadStart, // entry point
                          thread,
                          CREATE_SUSPENDED,
                          (unsigned int *)&m_tid
                        );
#else // compiler doesn't have _beginthreadex
    m_hThread = ::CreateThread
                  (
                    NULL,                               // default security
                    stackSize,                          // stack size
                    wxThreadInternal::WinThreadStart,   // thread entry point
                    (LPVOID)thread,                     // parameter
                    CREATE_SUSPENDED,                   // flags
                    &m_tid                              // [out] thread id
                  );
#endif // _beginthreadex/CreateThread

    if ( m_hThread == nullptr )
    {
        wxLogSysError(_("Can't create thread"));

        return false;
    }

    if ( m_priority != wxPRIORITY_DEFAULT )
    {
        SetPriority(m_priority);
    }

    return true;
}

wxThreadError wxThreadInternal::Kill()
{
    m_thread->OnKill();

    if ( !::TerminateThread(m_hThread, THREAD_ERROR_EXIT) )
    {
        wxLogSysError(_("Couldn't terminate thread"));

        return wxThreadError::MiscError;
    }

    Free();

    return wxThreadError::None;
}

wxThreadError
wxThreadInternal::WaitForTerminate(wxCriticalSection& cs,
                                   wxThread::ExitCode *pRc,
                                   wxThreadWait waitMode,
                                   wxThread *threadToDelete)
{
    // prevent the thread C++ object from disappearing as long as we are using
    // it here
    wxThreadKeepAlive keepAlive(*this);


    // we may either wait passively for the thread to terminate (when called
    // from Wait()) or ask it to terminate (when called from Delete())
    bool shouldDelete = threadToDelete != nullptr;

    WXDWORD rc = 0;

    // we might need to resume the thread if it's currently stopped
    bool shouldResume = false;

    // as Delete() (which calls us) is always safe to call we need to consider
    // all possible states
    {
        wxCriticalSectionLocker lock(cs);

        if ( m_state == wxThreadState::New )
        {
            if ( shouldDelete )
            {
                // WinThreadStart() will see it and terminate immediately, no
                // need to cancel the thread -- but we still need to resume it
                // to let it run
                m_state = wxThreadState::Exited;

                // we must call Resume() as the thread hasn't been initially
                // resumed yet (and as Resume() it knows about wxThreadState::Exited
                // special case, it won't touch it and WinThreadStart() will
                // just exit immediately)
                shouldResume = true;
                shouldDelete = false;
            }
            //else: shouldResume is correctly set to false here, wait until
            //      someone else runs the thread and it finishes
        }
        else // running, paused, cancelled or even exited
        {
            shouldResume = m_state == wxThreadState::Paused;
        }
    }

    // resume the thread if it is paused
    if ( shouldResume )
        Resume();

    // ask the thread to terminate
    if ( shouldDelete )
    {
        wxCriticalSectionLocker lock(cs);

        Cancel();
    }

    if ( threadToDelete )
        threadToDelete->OnDelete();

    // now wait for thread to finish
    if ( wxThread::IsMain() )
    {
        // set flag for wxIsWaitingForThread()
        gs_waitingForThread = true;
    }

    wxAppTraits& traits = wxApp::GetValidTraits();

    // we can't just wait for the thread to terminate because it might be
    // calling some GUI functions and so it will never terminate before we
    // process the Windows messages that result from these functions
    // (note that even in console applications we might have to process
    // messages if we use wxExecute() or timers or ...)
    WXDWORD result wxDUMMY_INITIALIZE(0);
    do
    {
        if ( wxThread::IsMain() )
        {
            // give the thread we're waiting for chance to do the GUI call
            // it might be in
            if ( (gs_nWaitingForGui > 0) && wxGuiOwnedByMainThread() )
            {
                wxMutexGuiLeave();
            }
        }

        // Wait for the thread while still processing events in the GUI apps or
        // just simply wait for it in the console ones.
        result = traits.WaitForThread(m_hThread, waitMode);

        switch ( result )
        {
            case 0xFFFFFFFF:
                // error
                wxLogSysError(_("Cannot wait for thread termination"));
                Kill();
                return wxThreadError::Killed;

            case WAIT_OBJECT_0:
                // thread we're waiting for terminated
                break;

            case WAIT_OBJECT_0 + 1:
            case WAIT_OBJECT_0 + 2:
                // Wake up has been signaled or a new message arrived, process
                // it -- but only if we're the main thread as we don't support
                // processing messages in the other ones
                //
                // NB: we still must include QS_ALLINPUT even when waiting
                //     in a secondary thread because if it had created some
                //     window somehow (possible not even using wxWidgets)
                //     the system might dead lock then
                if ( wxThread::IsMain() )
                {
                    if ( !traits.DoMessageFromThreadWait() )
                    {
                        // WM_QUIT received: kill the thread
                        Kill();

                        return wxThreadError::Killed;
                    }
                }
                break;

            default:
                wxFAIL_MSG("unexpected result of MsgWaitForMultipleObject");
        }
    } while ( result != WAIT_OBJECT_0 );

    if ( wxThread::IsMain() )
    {
        gs_waitingForThread = false;
    }


    // although the thread might be already in the EXITED state it might not
    // have terminated yet and so we are not sure that it has actually
    // terminated if the "if" above hadn't been taken
    for ( ;; )
    {
        if ( !::GetExitCodeThread(m_hThread, &rc) )
        {
            wxLogLastError("GetExitCodeThread");

            rc = THREAD_ERROR_EXIT;

            break;
        }

        if ( rc != STILL_ACTIVE )
            break;

        // give the other thread some time to terminate, otherwise we may be
        // starving it
        ::Sleep(1);
    }

    if ( pRc )
        *pRc = wxUIntToPtr(rc);

    // we don't need the thread handle any more in any case
    Free();


    return rc == THREAD_ERROR_EXIT ? wxThreadError::MiscError : wxThreadError::None;
}

bool wxThreadInternal::Suspend()
{
    WXDWORD nSuspendCount = ::SuspendThread(m_hThread);
    if ( nSuspendCount == (WXDWORD)-1 )
    {
        wxLogSysError(_("Cannot suspend thread %lx"),
                      wx::narrow_cast<unsigned long>(wxPtrToUInt(m_hThread)));

        return false;
    }

    // Calling GetThreadContext() forces the thread to actually be suspended:
    // just calling SuspendThread() is not enough, it just asks the scheduler
    // to suspend the thread at the next opportunity and by then we may already
    // exit wxThread::Pause() and leave m_critsect, meaning that the thread
    // could enter it and end up suspended inside a CS, which will inevitably
    // result in a deadlock later.
    CONTEXT ctx;
    // We don't really need the context, but we still must initialize it.
    ctx.ContextFlags = CONTEXT_FULL;
    if ( !::GetThreadContext(m_hThread, &ctx) )
    {
        wxLogLastError("GetThreadContext");
    }

    m_state = wxThreadState::Paused;

    return true;
}

bool wxThreadInternal::Resume()
{
    const WXDWORD nSuspendCount = ::ResumeThread(m_hThread);
    if ( nSuspendCount == (WXDWORD)-1 )
    {
        wxLogSysError(_("Cannot resume thread %lx"),
                      wx::narrow_cast<unsigned long>(wxPtrToUInt(m_hThread)));

        return false;
    }

    // don't change the state from wxThreadState::Exited because it's special and means
    // we are going to terminate without running any user code - if we did it,
    // the code in WaitForTerminate() wouldn't work
    if ( m_state != wxThreadState::Exited )
    {
        m_state = wxThreadState::Running;
    }

    return true;
}

// static functions
// ----------------

wxThread *wxThread::This()
{
    wxASSERT_MSG( gs_tlsThisThread != TLS_OUT_OF_INDEXES,
                  "TLS index not set. Is wx initialized?" );
    wxThread *thread = (wxThread *)::TlsGetValue(gs_tlsThisThread);

    // be careful, 0 may be a valid return value as well
    if ( !thread && (::GetLastError() != NO_ERROR) )
    {
        wxLogSysError(_("Couldn't get the current thread pointer"));

        // return NULL...
    }

    return thread;
}

void wxThread::ThreadYield()
{
    // 0 argument to Sleep() is special and means to just give away the rest of
    // our timeslice
    ::Sleep(0);
}

int wxThread::GetCPUCount()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    return si.dwNumberOfProcessors;
}

unsigned long wxThread::GetCurrentId()
{
    return (unsigned long)::GetCurrentThreadId();
}

bool wxThread::SetConcurrency(size_t level)
{
    wxASSERT_MSG( IsMain(), "should only be called from the main thread" );

    // ok only for the default one
    if ( level == 0 )
        return false;

    // get system affinity mask first
    HANDLE hProcess = ::GetCurrentProcess();
    DWORD_PTR dwProcMask, dwSysMask;
    if ( ::GetProcessAffinityMask(hProcess, &dwProcMask, &dwSysMask) == 0 )
    {
        wxLogLastError("GetProcessAffinityMask");

        return false;
    }

    // how many CPUs have we got?
    if ( dwSysMask == 1 )
    {
        // don't bother with all this complicated stuff - on a single
        // processor system it doesn't make much sense anyhow
        return level == 1;
    }

    // calculate the process mask: it's a bit vector with one bit per
    // processor; we want to schedule the process to run on first level
    // CPUs
    WXDWORD bit = 1;
    while ( bit )
    {
        if ( dwSysMask & bit )
        {
            // ok, we can set this bit
            dwProcMask |= bit;

            // another process added
            if ( --level == 0 )
            {
                // and that's enough
                break;
            }
        }

        // next bit
        bit <<= 1;
    }

    // could we set all bits?
    if ( level != 0 )
    {
        wxLogDebug("bad level %u in wxThread::SetConcurrency()", level);

        return false;
    }

    if ( ::SetProcessAffinityMask(hProcess, dwProcMask) == 0 )
    {
        wxLogLastError("SetProcessAffinityMask");

        return false;
    }

    return true;
}

// ctor and dtor
// -------------

wxThread::wxThread(wxThreadKind kind)
{
    m_internal = new wxThreadInternal(this);

    m_isDetached = kind ==  wxThreadKind::Detached;
}

wxThread::~wxThread()
{
    delete m_internal;
}

// create/start thread
// -------------------

wxThreadError wxThread::Create(unsigned int stackSize)
{
    wxCriticalSectionLocker lock(m_critsect);

    if ( !m_internal->Create(this, stackSize) )
        return wxThreadError::NoResource;

    return wxThreadError::None;
}

wxThreadError wxThread::Run()
{
    wxCriticalSectionLocker lock(m_critsect);

    // Create the thread if it wasn't created yet with an explicit
    // Create() call:
    if ( !m_internal->GetHandle() )
    {
        if ( !m_internal->Create(this, 0) )
            return wxThreadError::NoResource;
    }

    wxCHECK_MSG( m_internal->GetState() == wxThreadState::New, wxThreadError::Running,
             "thread may only be started once after Create()" );

    // the thread has just been created and is still suspended - let it run
    return Resume();
}

// suspend/resume thread
// ---------------------

wxThreadError wxThread::Pause()
{
    wxCriticalSectionLocker lock(m_critsect);

    return m_internal->Suspend() ? wxThreadError::None : wxThreadError::MiscError;
}

wxThreadError wxThread::Resume()
{
    wxCriticalSectionLocker lock(m_critsect);

    return m_internal->Resume() ? wxThreadError::None : wxThreadError::MiscError;
}

// stopping thread
// ---------------

wxThread::ExitCode wxThread::Wait(wxThreadWait waitMode)
{
    ExitCode rc = wxUIntToPtr(THREAD_ERROR_EXIT);

    // although under Windows we can wait for any thread, it's an error to
    // wait for a detached one in wxWin API
    wxCHECK_MSG( !IsDetached(), rc,
                 "wxThread::Wait(): can't wait for detached thread" );

    std::ignore = m_internal->WaitForTerminate(m_critsect, &rc, waitMode);

    return rc;
}

wxThreadError wxThread::Delete(ExitCode *pRc, wxThreadWait waitMode)
{
    return m_internal->WaitForTerminate(m_critsect, pRc, waitMode, this);
}

wxThreadError wxThread::Kill()
{
    if ( !IsRunning() )
        return wxThreadError::NotRunning;

    const wxThreadError rc = m_internal->Kill();

    if ( IsDetached() )
    {
        delete this;
    }
    else // joinable
    {
        // update the status of the joinable thread
        wxCriticalSectionLocker lock(m_critsect);
        m_internal->SetState(wxThreadState::Exited);
    }

    return rc;
}

void wxThread::Exit(ExitCode status)
{
    wxThreadInternal::DoThreadOnExit(this);

    m_internal->Free();

    if ( IsDetached() )
    {
        delete this;
    }
    else // joinable
    {
        // update the status of the joinable thread
        wxCriticalSectionLocker lock(m_critsect);
        m_internal->SetState(wxThreadState::Exited);
    }

#ifdef wxUSE_BEGIN_THREAD
    _endthreadex(wxPtrToUInt(status));
#else // !VC++
    ::ExitThread(wxPtrToUInt(status));
#endif // VC++/!VC++

    wxFAIL_MSG("Couldn't return from ExitThread()!");
}

// priority setting
// ----------------

void wxThread::SetPriority(unsigned int prio)
{
    wxCriticalSectionLocker lock(m_critsect);

    m_internal->SetPriority(prio);
}

unsigned int wxThread::GetPriority() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return m_internal->GetPriority();
}

unsigned long wxThread::GetId() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return (unsigned long)m_internal->GetId();
}

WXHANDLE wxThread::MSWGetHandle() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return m_internal->GetHandle();
}

bool wxThread::IsRunning() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return m_internal->GetState() == wxThreadState::Running;
}

bool wxThread::IsAlive() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return (m_internal->GetState() == wxThreadState::Running) ||
           (m_internal->GetState() == wxThreadState::Paused);
}

bool wxThread::IsPaused() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return m_internal->GetState() == wxThreadState::Paused;
}

bool wxThread::TestDestroy()
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return m_internal->GetState() == wxThreadState::Canceled;
}

// ----------------------------------------------------------------------------
// Automatic initialization for thread module
// ----------------------------------------------------------------------------

class wxThreadModule : public wxModule
{
public:
    bool OnInit() override;
    void OnExit() override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxThreadModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxThreadModule, wxModule);

bool wxThreadModule::OnInit()
{
    // allocate TLS index for storing the pointer to the current thread
    gs_tlsThisThread = ::TlsAlloc();
    if ( gs_tlsThisThread == TLS_OUT_OF_INDEXES )
    {
        // in normal circumstances it will only happen if all other
        // TLS_MINIMUM_AVAILABLE (>= 64) indices are already taken - in other
        // words, this should never happen
        wxLogSysError(_("Thread module initialization failed: impossible to allocate index in thread local storage"));

        return false;
    }

    // main thread doesn't have associated wxThread object, so store 0 in the
    // TLS instead
    if ( !::TlsSetValue(gs_tlsThisThread, (LPVOID)nullptr) )
    {
        ::TlsFree(gs_tlsThisThread);
        gs_tlsThisThread = TLS_OUT_OF_INDEXES;

        wxLogSysError(_("Thread module initialization failed: cannot store value in thread local storage"));

        return false;
    }

    gs_critsectWaitingForGui = new wxCriticalSection();

    gs_critsectGui = new wxCriticalSection();
    gs_critsectGui->Enter();

    wxThread::ms_idMainThread = wxThread::GetCurrentId();

    return true;
}

void wxThreadModule::OnExit()
{
    if ( !::TlsFree(gs_tlsThisThread) )
    {
        wxLogLastError("TlsFree failed.");
    }

    // invalidate slot index to make the errors more obvious if we try to use
    // it from now on, e.g. if any wxThreads are still running (which shouldn't
    // be the case, of course, but might still happen)
    gs_tlsThisThread = TLS_OUT_OF_INDEXES;

    if ( gs_critsectGui )
    {
        gs_critsectGui->Leave();
        wxDELETE(gs_critsectGui);
    }

    wxDELETE(gs_critsectWaitingForGui);
}

// ----------------------------------------------------------------------------
// under Windows, these functions are implemented using a critical section and
// not a mutex, so the names are a bit confusing
// ----------------------------------------------------------------------------

void wxMutexGuiEnterImpl()
{
    // this would dead lock everything...
    wxASSERT_MSG( !wxThread::IsMain(),
                  "main thread doesn't want to block in wxMutexGuiEnter()!" );

    // the order in which we enter the critical sections here is crucial!!

    // set the flag telling to the main thread that we want to do some GUI
    {
        wxCriticalSectionLocker enter(*gs_critsectWaitingForGui);

        gs_nWaitingForGui++;
    }

    wxWakeUpMainThread();

    // now we may block here because the main thread will soon let us in
    // (during the next iteration of OnIdle())
    gs_critsectGui->Enter();
}

void wxMutexGuiLeaveImpl()
{
    wxCriticalSectionLocker enter(*gs_critsectWaitingForGui);

    if ( wxThread::IsMain() )
    {
        gs_bGuiOwnedByMainThread = false;
    }
    else
    {
        // decrement the number of threads waiting for GUI access now
        wxASSERT_MSG( gs_nWaitingForGui > 0,
                      "calling wxMutexGuiLeave() without entering it first?" );

        gs_nWaitingForGui--;

        wxWakeUpMainThread();
    }

    gs_critsectGui->Leave();
}

void wxMutexGuiLeaveOrEnter()
{
    wxASSERT_MSG( wxThread::IsMain(),
                  "only main thread may call wxMutexGuiLeaveOrEnter()!" );

    wxCriticalSectionLocker enter(*gs_critsectWaitingForGui);

    if ( gs_nWaitingForGui == 0 )
    {
        // no threads are waiting for GUI - so we may acquire the lock without
        // any danger (but only if we don't already have it)
        if ( !wxGuiOwnedByMainThread() )
        {
            gs_critsectGui->Enter();

            gs_bGuiOwnedByMainThread = true;
        }
        //else: already have it, nothing to do
    }
    else
    {
        // some threads are waiting, release the GUI lock if we have it
        if ( wxGuiOwnedByMainThread() )
        {
            wxMutexGuiLeave();
        }
        //else: some other worker thread is doing GUI
    }
}

bool wxGuiOwnedByMainThread()
{
    return gs_bGuiOwnedByMainThread;
}

// wake up the main thread if it's in ::GetMessage()
void wxWakeUpMainThread()
{
    // sending any message would do - hopefully WM_NULL is harmless enough
    if ( !::PostThreadMessageW(wxThread::GetMainId(), WM_NULL, 0, 0) )
    {
        // should never happen, but log an error if it does, however do not use
        // wxLog here as it would result in reentrancy because logging from a
        // thread calls wxWakeUpIdle() which calls this function itself again
        const unsigned long ec = wxSysErrorCode();
        // FIXME: Correct to print out from here?
        //fmt::print("Failed to wake up main thread: PostThreadMessage(WM_NULL) failed with error 0x%08lx (%s).", ec, wxSysErrorMsgStr(ec));
    }
}

bool wxIsWaitingForThread()
{
    return gs_waitingForThread;
}

// ----------------------------------------------------------------------------
// include common implementation code
// ----------------------------------------------------------------------------

#include "wx/thrimpl.cpp"

#endif // wxUSE_THREADS
