///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/appbase.cpp
// Purpose:     implements wxAppConsoleBase class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     19.06.2003 (extracted from common/appcmn.cpp)
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/list.h"
#include "wx/app.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/utils.h"
#include "wx/wxcrtvararg.h"

#include "wx/apptrait.h"
#include "wx/cmdline.h"
#include "wx/confbase.h"
#include "wx/evtloop.h"
#include "wx/filename.h"
#include "wx/msgout.h"

#include "wx/sysopt.h"
#include "wx/thread.h"
#include "wx/stdpaths.h"

#include <boost/nowide/convert.hpp>

#ifdef HAS_EXCEPTION_PTR
    import <exception>;        // for std::current_exception()
    import <utility>;          // for std::swap()
#endif

import Utils.Strings;

import <clocale>;
import <ranges>;
import <string>;
import <typeinfo>;

#if wxUSE_EXCEPTIONS
    // Any conforming C++11 compiler should have it, but g++ implementation
    // of exception handling depends on the availability of the atomic int
    // operations apparently, so all known version of g++ with C++11 support
    // (4.7..4.9) fail to provide exception_ptr if the symbol below is not
    // set to 2 (meaning "always available"), which is notably the case for
    // MinGW-w64 without -march=486 switch, see #16634.
    #ifdef __GNUC__
        // This symbol is always defined in the known g++ version, so
        // assume that if it isn't defined, things changed for the better
        // and optimistically suppose that exception_ptr is available.
        #if !defined(__GCC_ATOMIC_INT_LOCK_FREE) \
                || __GCC_ATOMIC_INT_LOCK_FREE > 1
            #define HAS_EXCEPTION_PTR
        #endif
    #else
        #define HAS_EXCEPTION_PTR
    #endif
#endif // wxUSE_EXCEPTIONS

#if !defined(WX_WINDOWS)
  #include  <signal.h>      // for SIGTRAP used by wxTrap()
#endif  //Win/Unix

#if wxUSE_FONTMAP
    #include "wx/fontmap.h"
#endif // wxUSE_FONTMAP

#if wxDEBUG_LEVEL
    #if wxUSE_STACKWALKER
        #include "wx/stackwalk.h"
        #ifdef WX_WINDOWS
            #include "wx/msw/debughlp.h"
        #endif
    #endif // wxUSE_STACKWALKER

    #include "wx/recguard.h"
#endif // wxDEBUG_LEVEL

// wxABI_VERSION can be defined when compiling applications but it should be
// left undefined when compiling the library itself, it is then set to its
// default value in version.h
#if wxABI_VERSION != wxMAJOR_VERSION * 10000 + wxMINOR_VERSION * 100 + 99
#error "wxABI_VERSION should not be defined when compiling the library"
#endif

// ----------------------------------------------------------------------------
// private functions prototypes
// ----------------------------------------------------------------------------

#if wxDEBUG_LEVEL
// really just show the assert dialog
static bool DoShowAssertDialog(const std::string& msg);

// prepare for showing the assert dialog, use the given traits or
// DoShowAssertDialog() as last fallback to really show it
static
void ShowAssertDialog(const std::string& file,
                        int line,
                        const std::string& func,
                        const std::string& cond,
                        const std::string& msg,
                        wxAppTraits *traits = nullptr);
#endif // wxDEBUG_LEVEL

#ifdef __WXDEBUG__
    // turn on the trace masks specified in the env variable WXTRACE
    static void SetTraceMasks();
#endif // __WXDEBUG__

// ----------------------------------------------------------------------------
// wxEventLoopPtr
// ----------------------------------------------------------------------------

// ============================================================================
// wxAppConsoleBase implementation
// ============================================================================

// ----------------------------------------------------------------------------
// ctor/dtor
// ----------------------------------------------------------------------------

wxAppConsoleBase::wxAppConsoleBase()
{
    ms_appInstance = dynamic_cast<wxAppConsole *>(this);

#ifdef __WXDEBUG__
    SetTraceMasks();
    // In unicode mode the SetTraceMasks call can cause an apptraits to be
    // created, but since we are still in the constructor the wrong kind will
    // be created for GUI apps.  Destroy it so it can be created again later.
    wxDELETE(m_traits);
#endif

    wxEvtHandler::AddFilter(this);
}

wxAppConsoleBase::~wxAppConsoleBase()
{
    wxEvtHandler::RemoveFilter(this);

    // we're being destroyed and using this object from now on may not work or
    // even crash so don't leave dangling pointers to it
    ms_appInstance = nullptr;

    delete m_traits;
}

// ----------------------------------------------------------------------------
// initialization/cleanup
// ----------------------------------------------------------------------------

bool wxAppConsoleBase::Initialize([[maybe_unused]] int& argc, [[maybe_unused]] wxChar** argv)
{
#if defined(WX_WINDOWS)
    ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
#endif

    return true;
}

std::string wxAppConsoleBase::GetAppName() const
{
    std::string name = m_appName;
    if ( name.empty() )
    {
        if ( argv )
        {
            // the application name is, by default, the name of its executable file
            wxFileName::SplitPath(argv[0], nullptr, &name, nullptr);
        }
#if wxUSE_STDPATHS
        else // fall back to the executable file name, if we can determine it
        {
            const std::string pathExe = wxStandardPaths::Get().GetExecutablePath();
            if ( !pathExe.empty() )
            {
                wxFileName::SplitPath(pathExe, nullptr, &name, nullptr);
            }
        }
#endif // wxUSE_STDPATHS
    }
    return name;
}

std::string wxAppConsoleBase::GetAppDisplayName() const
{
    // use the explicitly provided display name, if any
    if ( !m_appDisplayName.empty() )
        return m_appDisplayName;

    // if the application name was explicitly set, use it as is as capitalizing
    // it won't always produce good results
    if ( !m_appName.empty() )
        return m_appName;

    // if neither is set, use the capitalized version of the program file as
    // it's the most reasonable default
    return wx::utils::ToUpperCopy(GetAppName());
}

std::unique_ptr<wxEventLoopBase> wxAppConsoleBase::CreateMainLoop()
{
    return GetTraits()->CreateEventLoop();
}

void wxAppConsoleBase::CleanUp()
{
}

// ----------------------------------------------------------------------------
// OnXXX() callbacks
// ----------------------------------------------------------------------------

bool wxAppConsoleBase::OnInit()
{
#if wxUSE_CMDLINE_PARSER
    wxCmdLineParser parser(argc, argv);

    OnInitCmdLine(parser);

    const bool cont = [&parser, this]() {
        switch ( parser.Parse(false /* don't show usage */) )
        {
            case -1:
                return OnCmdLineHelp(parser);

            case 0:
                return OnCmdLineParsed(parser);

            default:
                return OnCmdLineError(parser);
        }
    }();

    if ( !cont )
        return false;
#endif // wxUSE_CMDLINE_PARSER

    return true;
}

int wxAppConsoleBase::OnRun()
{
    return MainLoop();
}

void wxAppConsoleBase::OnLaunched()
{
}

int wxAppConsoleBase::OnExit()
{
    // Delete all pending objects first, they might use wxConfig to save their
    // state during their destruction.
    DeletePendingObjects();

#if wxUSE_CONFIG
    // delete the config object if any (don't use Get() here, but Set()
    // because Get() could create a new config object)
    delete wxConfigBase::Set(nullptr);
#endif // wxUSE_CONFIG

    return 0;
}

void wxAppConsoleBase::Exit()
{
    if (m_mainLoop != nullptr)
        ExitMainLoop();
    else
        exit(-1);
}

// ----------------------------------------------------------------------------
// traits stuff
// ----------------------------------------------------------------------------

wxAppTraits *wxAppConsoleBase::CreateTraits()
{
    return new wxConsoleAppTraits;
}

wxAppTraits *wxAppConsoleBase::GetTraits()
{
    // FIXME-MT: protect this with a CS?
    if ( !m_traits )
    {
        m_traits = CreateTraits();

        wxASSERT_MSG( m_traits, "wxApp::CreateTraits() failed?" );
    }

    return m_traits;
}

/* static */
wxAppTraits *wxAppConsoleBase::GetTraitsIfExists()
{
    wxAppConsole * const app = GetInstance();
    return app ? app->GetTraits() : nullptr;
}

/* static */
wxAppTraits& wxAppConsoleBase::GetValidTraits()
{
    static wxConsoleAppTraits s_traitsConsole;
    wxAppTraits* const traits = GetTraitsIfExists();

    return *(traits ? traits : &s_traitsConsole);
}

// ----------------------------------------------------------------------------
// wxEventLoop redirection
// ----------------------------------------------------------------------------

int wxAppConsoleBase::MainLoop()
{
    m_mainLoop = CreateMainLoop();

    if (wxTheApp)
        wxTheApp->OnLaunched();

    return m_mainLoop ? m_mainLoop->Run() : -1;
}

void wxAppConsoleBase::ExitMainLoop()
{
    // we should exit from the main event loop, not just any currently active
    // (e.g. modal dialog) event loop
    if ( m_mainLoop && m_mainLoop->IsRunning() )
    {
        m_mainLoop->Exit(0);
    }
}

bool wxAppConsoleBase::Pending()
{
    // use the currently active message loop here, not m_mainLoop, because if
    // we're showing a modal dialog (with its own event loop) currently the
    // main event loop is not running anyhow
    wxEventLoopBase * const loop = wxEventLoopBase::GetActive();

    return loop && loop->Pending();
}

bool wxAppConsoleBase::Dispatch()
{
    // see comment in Pending()
    wxEventLoopBase * const loop = wxEventLoopBase::GetActive();

    return loop && loop->Dispatch();
}

bool wxAppConsoleBase::AppYield(bool onlyIfNeeded)
{
    wxEventLoopBase * const loop = wxEventLoopBase::GetActive();
    if ( loop )
       return loop->EvtYield(onlyIfNeeded);

    std::unique_ptr<wxEventLoopBase> tmpLoop(CreateMainLoop());
    return tmpLoop->EvtYield(onlyIfNeeded);
}

void wxAppConsoleBase::WakeUpIdle()
{
    wxEventLoopBase * const loop = wxEventLoopBase::GetActive();

    if ( loop )
        loop->WakeUp();
}

bool wxAppConsoleBase::ProcessIdle()
{
    // synthesize an idle event and check if more of them are needed
    wxIdleEvent event;
    event.SetEventObject(this);
    ProcessEvent(event);

#if wxUSE_LOG
    // flush the logged messages if any (do this after processing the events
    // which could have logged new messages)
    wxLog::FlushActive();
#endif

    // Garbage collect all objects previously scheduled for destruction.
    DeletePendingObjects();

    return event.MoreRequested();
}

bool wxAppConsoleBase::UsesEventLoop() const
{
    // in console applications we don't know whether we're going to have an
    // event loop so assume we won't -- unless we already have one running
    return wxEventLoopBase::GetActive() != nullptr;
}

// ----------------------------------------------------------------------------
// events
// ----------------------------------------------------------------------------

/* static */
bool wxAppConsoleBase::IsMainLoopRunning()
{
    const wxAppConsole * const app = GetInstance();

    return app && app->m_mainLoop != nullptr;
}

int wxAppConsoleBase::FilterEvent([[maybe_unused]] wxEvent& event)
{
    // process the events normally by default
    return Event_Skip;
}

void wxAppConsoleBase::DelayPendingEventHandler(wxEvtHandler* toDelay)
{
    wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);

    // move the handler from the list of handlers with processable pending events
    // to the list of handlers with pending events which needs to be processed later
    // FIXME: This could all be done more elegantly.
    std::erase(m_handlersWithPendingEvents, toDelay);

    if(std::ranges::find(m_handlersWithPendingDelayedEvents, toDelay) == m_handlersWithPendingDelayedEvents.end())
        m_handlersWithPendingDelayedEvents.push_back(toDelay);

    wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);
}

void wxAppConsoleBase::RemovePendingEventHandler(wxEvtHandler* toRemove)
{
    wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);

    // FIXME: Could be done more elegantly.
    if(std::ranges::find(m_handlersWithPendingEvents, toRemove) != m_handlersWithPendingEvents.end())
    {
        std::erase(m_handlersWithPendingEvents, toRemove);

        // check that the handler was present only once in the list
        wxASSERT_MSG( std::ranges::find(m_handlersWithPendingEvents, toRemove) == m_handlersWithPendingEvents.end(),
                        "Handler occurs twice in the m_handlersWithPendingEvents list!" );
    }

    //else: it wasn't in this list at all, it's ok
    if(std::ranges::find(m_handlersWithPendingDelayedEvents, toRemove) != m_handlersWithPendingDelayedEvents.end())
    {
        std::erase(m_handlersWithPendingDelayedEvents, toRemove);

        // check that the handler was present only once in the list
        wxASSERT_MSG( std::ranges::find(m_handlersWithPendingDelayedEvents, toRemove) == m_handlersWithPendingDelayedEvents.end(),
                        "Handler occurs twice in m_handlersWithPendingDelayedEvents list!" );
    }
    //else: it wasn't in this list at all, it's ok

    wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);
}

void wxAppConsoleBase::AppendPendingEventHandler(wxEvtHandler* toAppend)
{
    wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);

    if(std::ranges::find(m_handlersWithPendingEvents, toAppend) == m_handlersWithPendingEvents.end())
        m_handlersWithPendingEvents.push_back(toAppend);

    wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);
}

bool wxAppConsoleBase::HasPendingEvents() const
{
    wxENTER_CRIT_SECT(const_cast<wxAppConsoleBase*>(this)->m_handlersWithPendingEventsLocker);

    const bool has = !m_handlersWithPendingEvents.empty();

    wxLEAVE_CRIT_SECT(const_cast<wxAppConsoleBase*>(this)->m_handlersWithPendingEventsLocker);

    return has;
}

void wxAppConsoleBase::SuspendProcessingOfPendingEvents()
{
    m_bDoPendingEventProcessing = false;
}

void wxAppConsoleBase::ResumeProcessingOfPendingEvents()
{
    m_bDoPendingEventProcessing = true;
}

void wxAppConsoleBase::ProcessPendingEvents()
{
    if ( m_bDoPendingEventProcessing )
    {
        wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);

        wxCHECK_RET( m_handlersWithPendingDelayedEvents.empty(),
                     "this helper list should be empty" );

        // iterate until the list becomes empty: the handlers remove themselves
        // from it when they don't have any more pending events
        while (!m_handlersWithPendingEvents.empty())
        {
            // In ProcessPendingEvents(), new handlers might be added
            // and we can safely leave the critical section here.
            wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);

            // NOTE: we always call ProcessPendingEvents() on the first event handler
            //       with pending events because handlers auto-remove themselves
            //       from this list (see RemovePendingEventHandler) if they have no
            //       more pending events.
            m_handlersWithPendingEvents[0]->ProcessPendingEvents();

            wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);
        }

        // now the wxHandlersWithPendingEvents is surely empty; however some event
        // handlers may have moved themselves into wxHandlersWithPendingDelayedEvents
        // because of a selective wxYield call in progress.
        // Now we need to move them back to wxHandlersWithPendingEvents so the next
        // call to this function has the chance of processing them:
        if (!m_handlersWithPendingDelayedEvents.empty())
        {
            std::ranges::move(m_handlersWithPendingDelayedEvents, std::back_inserter(m_handlersWithPendingEvents));
            m_handlersWithPendingDelayedEvents.clear();
        }

        wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);
    }
}

void wxAppConsoleBase::DeletePendingEvents()
{
    wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);

    wxCHECK_RET( m_handlersWithPendingDelayedEvents.empty(),
                 "this helper list should be empty" );

    // FIXME: Have events clear events on destruction instead.
    std::ranges::for_each(m_handlersWithPendingEvents, [](auto* handler){ handler->DeletePendingEvents(); });

    m_handlersWithPendingEvents.clear();

    wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);
}

// ----------------------------------------------------------------------------
// delayed objects destruction
// ----------------------------------------------------------------------------

bool wxAppConsoleBase::IsScheduledForDestruction(wxObject *object) const
{
    return wxPendingDelete.Member(object);
}

void wxAppConsoleBase::ScheduleForDestruction(wxObject *object)
{
    if ( !UsesEventLoop() )
    {
        // we won't be able to delete it later so do it right now
        delete object;
        return;
    }
    //else: we either already have or will soon start an event loop

    if ( !wxPendingDelete.Member(object) )
        wxPendingDelete.Append(object);
}

void wxAppConsoleBase::DeletePendingObjects()
{
    wxList::compatibility_iterator node = wxPendingDelete.GetFirst();
    while (node)
    {
        wxObject *obj = node->GetData();

        // remove it from the list first so that if we get back here somehow
        // during the object deletion (e.g. wxYield called from its dtor) we
        // wouldn't try to delete it the second time
        if ( wxPendingDelete.Member(obj) )
            wxPendingDelete.Erase(node);

        delete obj;

        // Deleting one object may have deleted other pending
        // objects, so start from beginning of list again.
        node = wxPendingDelete.GetFirst();
    }
}

// ----------------------------------------------------------------------------
// exception handling
// ----------------------------------------------------------------------------

#if wxUSE_EXCEPTIONS

void
wxAppConsoleBase::HandleEvent(wxEvtHandler *handler,
                              wxEventFunction func,
                              wxEvent& event) const
{
    // by default, simply call the handler
    (handler->*func)(event);
}

void wxAppConsoleBase::CallEventHandler(wxEvtHandler *handler,
                                        wxEventFunctor& functor,
                                        wxEvent& event) const
{
    // If the functor holds a method then, for backward compatibility, call
    // HandleEvent():
    const wxEventFunction eventFunction = functor.GetEvtMethod();

    if ( eventFunction )
        HandleEvent(handler, eventFunction, event);
    else
        functor(handler, event);
}

void wxAppConsoleBase::OnUnhandledException()
{
    // we're called from an exception handler so we can re-throw the exception
    // to recover its type
    std::string what;
    try
    {
        throw;
    }
    catch ( std::exception& e )
    {
#ifdef wxNO_RTTI
        what = fmt::format("standard exception with message \"{:s}\"", e.what());
#else
        what = fmt::format("(standard exception of type \"{:s}\" with message \"{:s}\")",
                    typeid(e).name(), e.what());
#endif
    }
    catch ( ... )
    {
        what = "unknown exception";
    }
    
    fmt::print(stderr, "Unhandled %s; terminating %s.\n",
               what,
               wxIsMainThread() ? "the application" : "the thread in which it happened");
}

// ----------------------------------------------------------------------------
// exceptions support
// ----------------------------------------------------------------------------

bool wxAppConsoleBase::OnExceptionInMainLoop()
{
    throw;
}

#ifdef HAS_EXCEPTION_PTR
static std::exception_ptr gs_storedException;

bool wxAppConsoleBase::StoreCurrentException()
{
    if ( gs_storedException )
    {
        // We can't store more than one exception currently: while we could
        // support this by just using a vector<exception_ptr>, it shouldn't be
        // actually necessary because we should never have more than one active
        // exception anyhow.
        return false;
    }

    gs_storedException = std::current_exception();

    return true;
}

void wxAppConsoleBase::RethrowStoredException()
{
    if ( gs_storedException )
    {
        std::exception_ptr storedException;
        std::swap(storedException, gs_storedException);

        std::rethrow_exception(storedException);
    }
}

#else // !HAS_EXCEPTION_PTR

bool wxAppConsoleBase::StoreCurrentException()
{
    return false;
}

void wxAppConsoleBase::RethrowStoredException()
{
}

#endif // HAS_EXCEPTION_PTR/!HAS_EXCEPTION_PTR

#endif // wxUSE_EXCEPTIONS

// ----------------------------------------------------------------------------
// cmd line parsing
// ----------------------------------------------------------------------------

#if wxUSE_CMDLINE_PARSER

constexpr char OPTION_VERBOSE[] = "verbose";

void wxAppConsoleBase::OnInitCmdLine(wxCmdLineParser& parser)
{
    // the standard command line options
    static constexpr wxCmdLineEntryDesc cmdLineDesc[] =
    {
        {
            wxCmdLineEntryType::Switch,
            "h",
            "help",
            gettext_noop("show this help message"),
            wxCmdLineParamType::None,
            wxCMD_LINE_OPTION_HELP
        },

#if wxUSE_LOG
        {
            wxCmdLineEntryType::Switch,
            nullptr,
            OPTION_VERBOSE,
            gettext_noop("generate verbose log messages"),
            wxCmdLineParamType::None,
            0x0
        },
#endif // wxUSE_LOG

        // terminator
        wxCMD_LINE_DESC_END
    };

    parser.SetDesc(cmdLineDesc);
}

bool wxAppConsoleBase::OnCmdLineParsed(wxCmdLineParser& parser)
{
#if wxUSE_LOG
    if ( parser.Found(OPTION_VERBOSE) )
    {
        wxLog::SetVerbose(true);
    }
#else
    wxUnusedVar(parser);
#endif // wxUSE_LOG

    return true;
}

bool wxAppConsoleBase::OnCmdLineHelp(wxCmdLineParser& parser)
{
    parser.Usage();

    return false;
}

bool wxAppConsoleBase::OnCmdLineError(wxCmdLineParser& parser)
{
    parser.Usage();

    return false;
}

#endif // wxUSE_CMDLINE_PARSER

// ----------------------------------------------------------------------------
// debugging support
// ----------------------------------------------------------------------------

/* static */
bool wxAppConsoleBase::CheckBuildOptions(const char *optionsSignature,
                                         const char *componentName)
{
    if ( strcmp(optionsSignature, WX_BUILD_OPTIONS_SIGNATURE) != 0 )
    {
        std::string lib = WX_BUILD_OPTIONS_SIGNATURE;
        std::string prog = optionsSignature;
        std::string progName = componentName;
        std::string msg = fmt::format("Mismatch between the program and library build versions detected.\nThe library used %s,\nand %s used %s.",
                   lib.c_str(), progName.c_str(), prog.c_str());

        wxLogFatalError(msg.c_str());

        // normally wxLogFatalError doesn't return
        return false;
    }

    return true;
}

void wxAppConsoleBase::OnAssertFailure(const char* file,
                                       int line,
                                       const char* func,
                                       const char* cond,
                                       const char* msg)
{
#if wxDEBUG_LEVEL
    ShowAssertDialog(file, line, func, cond, msg, GetTraits());
#else
    // this function is still present even in debug level 0 build for ABI
    // compatibility reasons but is never called there and so can simply do
    // nothing in it
    wxUnusedVar(file);
    wxUnusedVar(line);
    wxUnusedVar(func);
    wxUnusedVar(cond);
    wxUnusedVar(msg);
#endif // wxDEBUG_LEVEL/!wxDEBUG_LEVEL
}

void wxAppConsoleBase::OnAssert(const char* file,
                                int line,
                                const char* cond,
                                const char* msg)
{
    OnAssertFailure(file, line, nullptr, cond, msg);
}

// ----------------------------------------------------------------------------
// Miscellaneous other methods
// ----------------------------------------------------------------------------

void wxAppConsoleBase::SetCLocale()
{
    // We want to use the user locale by default in GUI applications in order
    // to show the numbers, dates &c in the familiar format -- and also accept
    // this format on input (especially important for decimal comma/dot).
    wxSetlocale(LC_ALL, "");
}

// ============================================================================
// other classes implementations
// ============================================================================

// ----------------------------------------------------------------------------
// wxConsoleAppTraitsBase
// ----------------------------------------------------------------------------

#if wxUSE_LOG

wxLog *wxConsoleAppTraitsBase::CreateLogTarget()
{
    return new wxLogStderr;
}

#endif // wxUSE_LOG

wxMessageOutput *wxConsoleAppTraitsBase::CreateMessageOutput()
{
    return new wxMessageOutputStderr;
}

#if wxUSE_FONTMAP

wxFontMapper *wxConsoleAppTraitsBase::CreateFontMapper()
{
    return (wxFontMapper *)new wxFontMapperBase;
}

#endif // wxUSE_FONTMAP

wxRendererNative *wxConsoleAppTraitsBase::CreateRenderer()
{
    // console applications don't use renderers
    return nullptr;
}

bool wxConsoleAppTraitsBase::ShowAssertDialog(const std::string& msg)
{
    return wxAppTraitsBase::ShowAssertDialog(msg);
}

bool wxConsoleAppTraitsBase::HasStderr()
{
    // console applications always have stderr, even under Mac/Windows
    return true;
}

bool wxConsoleAppTraitsBase::SafeMessageBox([[maybe_unused]] std::string_view text,
                                            [[maybe_unused]] std::string_view title)
{
    // console applications don't show message boxes by default, although this
    // can be done in platform-specific cases
    return false;
}

// ----------------------------------------------------------------------------
// wxAppTraits
// ----------------------------------------------------------------------------

#if wxUSE_THREADS
void wxMutexGuiEnterImpl();
void wxMutexGuiLeaveImpl();

void wxAppTraitsBase::MutexGuiEnter()
{
    wxMutexGuiEnterImpl();
}

void wxAppTraitsBase::MutexGuiLeave()
{
    wxMutexGuiLeaveImpl();
}

void wxMutexGuiEnter()
{
    wxAppTraits * const traits = wxAppConsoleBase::GetTraitsIfExists();
    if ( traits )
        traits->MutexGuiEnter();
}

void wxMutexGuiLeave()
{
    wxAppTraits * const traits = wxAppConsoleBase::GetTraitsIfExists();
    if ( traits )
        traits->MutexGuiLeave();
}
#endif // wxUSE_THREADS

bool wxAppTraitsBase::ShowAssertDialog(const std::string& msgOriginal)
{
#if wxDEBUG_LEVEL
    std::string msg;

#if wxUSE_STACKWALKER
    const std::string stackTrace = GetAssertStackTrace();
    if ( !stackTrace.empty() )
    {
        msg += "\n\nCall stack:\n" + stackTrace;

        wxMessageOutputDebug().Output(msg);
    }
#endif // wxUSE_STACKWALKER

    return DoShowAssertDialog(msgOriginal + msg);
#else // !wxDEBUG_LEVEL
    wxUnusedVar(msgOriginal);

    return false;
#endif // wxDEBUG_LEVEL/!wxDEBUG_LEVEL
}

#if wxUSE_STACKWALKER
std::string wxAppTraitsBase::GetAssertStackTrace()
{
#if wxDEBUG_LEVEL

#if !defined(WX_WINDOWS)
    // on Unix stack frame generation may take some time, depending on the
    // size of the executable mainly... warn the user that we are working
    wxFprintf(stderr, "Collecting stack trace information, please wait...");
    fflush(stderr);
#endif // !WX_WINDOWS


    class StackDump : public wxStackWalker
    {
    public:
        const std::string& GetStackTrace() const { return m_stackTrace; }

    protected:
        void OnStackFrame(const wxStackFrame& frame) override
        {
            // don't show more than maxLines or we could get a dialog too tall
            // to be shown on screen: 20 should be ok everywhere as even with
            // 15 pixel high characters it is still only 300 pixels...
            if ( m_numFrames++ > 20 )
                return;
            m_stackTrace += fmt::format("{%02u} ", m_numFrames);

            const auto name = frame.GetName();

            if ( name.starts_with("wxOnAssert") )
            {
                // Ignore all frames until the wxOnAssert() one, they are
                // internal to wxWidgets and not interesting for the user
                // (but notice that if we never find the wxOnAssert() frame,
                // e.g. because we don't have symbol info at all, we would show
                // everything which is better than not showing anything).
                m_stackTrace.clear();
                m_numFrames = 0;
                return;
            }

            if ( !name.empty() )
            {
                m_stackTrace += fmt::format("{}-40s", name);
            }
            else
            {
                m_stackTrace += fmt::format("{%p}", frame.GetAddress());
            }

            if ( frame.HasSourceLocation() )
            {
                m_stackTrace += fmt::format("\t{}:{}", frame.GetFileName(), frame.GetLine());
            }

            m_stackTrace += '\n';
        }

    private:
        std::string m_stackTrace;
        unsigned m_numFrames{0};
    };

    StackDump dump;
    dump.Walk();
    return dump.GetStackTrace();
#else // !wxDEBUG_LEVEL
    // this function is still present for ABI-compatibility even in debug level
    // 0 build but is not used there and so can simply do nothing
    return {};
#endif // wxDEBUG_LEVEL/!wxDEBUG_LEVEL
}
#endif // wxUSE_STACKWALKER


// ============================================================================
// global functions implementation
// ============================================================================

void wxExit()
{
    if ( wxTheApp )
    {
        wxTheApp->Exit();
    }
    else
    {
        // what else can we do?
        exit(-1);
    }
}

void wxWakeUpIdle()
{
    if ( wxTheApp )
    {
        wxTheApp->WakeUpIdle();
    }
    //else: do nothing, what can we do?
}

void wxAbort()
{
    abort();
}

#if wxDEBUG_LEVEL

// break into the debugger
#ifndef wxTrap

void wxTrap()
{
#if defined(WX_WINDOWS)
    DebugBreak();
#elif defined(_MSL_USING_MW_C_HEADERS) && _MSL_USING_MW_C_HEADERS
    Debugger();
#elif defined(__UNIX__)
    raise(SIGTRAP);
#else
    // TODO
#endif // Win/Unix
}

#endif // wxTrap already defined as a macro

// default assert handler
static void
wxDefaultAssertHandler(const std::string& file,
                       int line,
                       const std::string& func,
                       const std::string& cond,
                       const std::string& msg)
{
    // If this option is set, we should abort immediately when assert happens.
    if ( wxSystemOptions::GetOptionInt("exit-on-assert") )
        wxAbort();

    // FIXME MT-unsafe
    static int s_bInAssert = 0;

    wxRecursionGuard guard(s_bInAssert);
    if ( guard.IsInside() )
    {
        // can't use assert here to avoid infinite loops, so just trap
        wxTrap();

        return;
    }

    if ( !wxTheApp )
    {
        // by default, show the assert dialog box -- we can't customize this
        // behaviour
        ShowAssertDialog(file, line, func, cond, msg);
    }
    else
    {
        // let the app process it as it wants
        wxTheApp->OnAssertFailure(file.c_str(), line, func.c_str(),
                                  cond.c_str(), msg.c_str());
    }
}

wxAssertHandler_t wxTheAssertHandler = wxDefaultAssertHandler;

void wxSetDefaultAssertHandler()
{
    wxTheAssertHandler = wxDefaultAssertHandler;
}

void wxOnAssert(const std::string& file,
                int line,
                const std::string& func,
                const std::string& cond,
                const std::string& msg)
{
    wxTheAssertHandler(file, line, func, cond, msg);
}

void wxOnAssert(const std::string& file,
                int line,
                const std::string& func,
                const std::string& cond)
{
    wxTheAssertHandler(file, line, func, cond, std::string());
}

void wxOnAssert(const char* file,
                int line,
                const char *func,
                const char* cond,
                const char* msg)
{
    // this is the backwards-compatible version (unless we don't use Unicode)
    // so it could be called directly from the user code and this might happen
    // even when wxTheAssertHandler is NULL
    if ( wxTheAssertHandler )
        wxTheAssertHandler(file, line, func, cond, msg);
}

void wxOnAssert(const char *file,
                int line,
                const char *func,
                const char *cond,
                const std::string& msg)
{
    wxTheAssertHandler(file, line, func, cond, msg);
}

void wxOnAssert(const char *file,
                int line,
                const char *func,
                const char *cond)
{
    wxTheAssertHandler(file, line, func, cond, std::string());
}

#endif // wxDEBUG_LEVEL

// ============================================================================
// private functions implementation
// ============================================================================

#ifdef __WXDEBUG__

static void SetTraceMasks()
{
#if wxUSE_LOG

    std::string mask;

    if ( wxGetEnv("WXTRACE", &mask) )
    {
        wxStringTokenizer tkn(mask, ",;:");
        while ( tkn.HasMoreTokens() )
            wxLog::AddTraceMask(tkn.GetNextToken());
    }

#endif // wxUSE_LOG
}

#endif // __WXDEBUG__

#if wxDEBUG_LEVEL

bool wxTrapInAssert = false;

static
bool DoShowAssertDialog(const std::string& msg)
{
    // under Windows we can show the dialog even in the console mode
#if defined(WX_WINDOWS)
    std::string msgDlg = msg;

    // this message is intentionally not translated -- it is for developers
    // only -- and the less code we use here, less is the danger of recursively
    // asserting and dying
    msgDlg += "\nDo you want to stop the program?\n"
              "You can also choose [Cancel] to suppress "
              "further warnings.";

    switch ( ::MessageBoxW(nullptr, boost::nowide::widen(msgDlg).c_str(), L"wxWidgets Debug Alert",
                          MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONSTOP ) )
    {
        case IDYES:
            // If we called wxTrap() directly from here, the programmer would
            // see this function and a few more calls between his own code and
            // it in the stack trace which would be perfectly useless and often
            // confusing. So instead just set the flag here and let the macros
            // defined in wx/debug.h call wxTrap() themselves, this ensures
            // that the debugger will show the line in the user code containing
            // the failing assert.
            wxTrapInAssert = true;
            break;

        case IDCANCEL:
            // stop the asserts
            return true;

        //case IDNO: nothing to do
    }
#else // !WX_WINDOWS
    wxUnusedVar(msg);
#endif // WX_WINDOWS/!WX_WINDOWS

    // continue with the asserts by default
    return false;
}

// show the standard assert dialog
static
void ShowAssertDialog(const std::string& file,
                      int line,
                      const std::string& func,
                      const std::string& cond,
                      const std::string& msgUser,
                      wxAppTraits *traits)
{
    // this variable can be set to true to suppress "assert failure" messages
    static bool s_bNoAsserts = false;

    std::string msg;
    msg.reserve(2048);

    // make life easier for people using VC++ IDE by using this format: like
    // this, clicking on the message will take us immediately to the place of
    // the failed assert
    msg += fmt::format("{:s}({:d}): assert \"{:s}\" failed", file, line, cond);

    // add the function name, if any
    if ( !func.empty() )
        msg += fmt::format(" in {}()", func);

    // and the message itself
    if ( !msgUser.empty() )
    {
        msg += fmt::format(": {}", msgUser);
    }
    else // no message given
    {
        msg += '.';
    }

#if wxUSE_THREADS
    if ( !wxThread::IsMain() )
    {
        msg += fmt::format(" [in thread {%lx}", wxThread::GetCurrentId());
    }
#endif // wxUSE_THREADS

    // log the assert in any case
    wxMessageOutputDebug().Output(msg);

    if ( !s_bNoAsserts )
    {
        if ( traits )
        {
            // delegate showing assert dialog (if possible) to that class
            s_bNoAsserts = traits->ShowAssertDialog(msg);
        }
        else // no traits object
        {
            // fall back to the function of last resort
            s_bNoAsserts = DoShowAssertDialog(msg);
        }
    }
}

#endif // wxDEBUG_LEVEL
