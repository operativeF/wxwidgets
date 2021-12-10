/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/app.cpp
// Purpose:     wxApp
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/wrapcctl.h"
#include "wx/msw/private.h"

#include "wx/app.h"
#include "wx/utils.h"
#include "wx/dialog.h"
#include "wx/crt.h"
#include "wx/log.h"
#include "wx/module.h"

#include "wx/apptrait.h"
#include "wx/dynlib.h"
#include "wx/evtloop.h"
#include "wx/thread.h"
#include "wx/weakref.h"

#include "wx/msw/dc.h"
#include "wx/msw/ole/oleutils.h"
#include "wx/msw/private/timer.h"

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

import WX.WinDef;

import Utils.Geometry;

import <string>;
import <vector>;

// instead of including <shlwapi.h> which is not part of the core SDK and not
// shipped at all with other compilers, we always define the parts of it we
// need here ourselves
//
// NB: DLLVER_PLATFORM_WINDOWS will be defined if shlwapi.h had been somehow
//     included already
#ifndef DLLVER_PLATFORM_WINDOWS
    // hopefully we don't need to change packing as DWORDs should be already
    // correctly aligned
    struct DLLVERSIONINFO
    {
        WXDWORD cbSize;
        WXDWORD dwMajorVersion;                   // Major version
        WXDWORD dwMinorVersion;                   // Minor version
        WXDWORD dwBuildNumber;                    // Build number
        WXDWORD dwPlatformID;                     // DLLVER_PLATFORM_*
    };

    using DLLGETVERSIONPROC = HRESULT (CALLBACK*)(DLLVERSIONINFO *);
#endif // defined(DLLVERSIONINFO)

// ---------------------------------------------------------------------------
// global variables
// ---------------------------------------------------------------------------

extern void wxSetKeyboardHook(bool doIt);

// because of mingw32 4.3 bug this struct can't be inside the namespace below:
// see http://article.gmane.org/gmane.comp.lib.wxwidgets.devel/110282
struct ClassRegInfo
{
    ClassRegInfo(const std::string& name, unsigned int flags)
    {
        if ( (flags & wxApp::RegClass_OnlyNR) == wxApp::RegClass_OnlyNR )
        {
            // We don't register the "normal" variant, so leave its name empty
            // to indicate that it's not used and use the given name for the
            // class that we do register: we don't need the "NR" suffix to
            // distinguish it in this case as there is only a single variant.
            regnameNR = name;
        }
        else // Register both normal and NR variants.
        {
            // Here we use a special suffix to make the class names unique.
            regname = name;
            regnameNR = regname + wxApp::GetNoRedrawClassSuffix;
        }
    }

    // Return the appropriate string depending on the presence of
    // RegClass_ReturnNR bit in the flags.
    const std::string& GetRequestedName(unsigned int flags) const
    {
        return flags & wxApp::RegClass_ReturnNR ? regnameNR : regname;
    }

    // the name of the registered class with and without CS_[HV]REDRAW styles
    std::string regname;
    std::string regnameNR;
};

namespace
{

std::vector<ClassRegInfo> gs_regClassesInfo;

} // anonymous namespace

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

LRESULT APIENTRY wxWndProc(WXHWND, WXUINT, WXWPARAM, WXLPARAM);

// ----------------------------------------------------------------------------
// Module for OLE initialization and cleanup
// ----------------------------------------------------------------------------

class wxOleInitModule : public wxModule
{
public:
    bool OnInit() override
    {
        return wxOleInitialize();
    }

    void OnExit() override
    {
        wxOleUninitialize();
    }

private:
    wxDECLARE_DYNAMIC_CLASS(wxOleInitModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxOleInitModule, wxModule);

// ===========================================================================
// wxGUIAppTraits implementation
// ===========================================================================

// private class which we use to pass parameters from BeforeChildWaitLoop() to
// AfterChildWaitLoop()
struct ChildWaitLoopData
{
    ChildWaitLoopData(wxWindowDisabler *wd_, wxWindow *focused_, wxWindow *winActive_)
        : wd(wd_),
          focused(focused_),
          winActive(winActive_)
    {
    }

    wxWindowDisabler *wd;
    wxWeakRef<wxWindow> focused;
    wxWindow *winActive;
};

void *wxGUIAppTraits::BeforeChildWaitLoop()
{
    /*
       We use a dirty hack here to disable all application windows (which we
       must do because otherwise the calls to wxYield() could lead to some very
       unexpected reentrancies in the users code) but to avoid losing
       focus/activation entirely when the child process terminates which would
       happen if we simply disabled everything using wxWindowDisabler. Indeed,
       remember that Windows will never activate a disabled window and when the
       last child's window is closed and Windows looks for a window to activate
       all our windows are still disabled. There is no way to enable them in
       time because we don't know when the child's windows are going to be
       closed, so the solution we use here is to keep one special tiny dialog
       enabled all the time. Then when the child terminates it will get
       activated and when we close it below -- after re-enabling all the other
       windows! -- the previously active window becomes activated again and
       everything is ok.
     */
    wxBeginBusyCursor();

    wxWindow* const focus = wxWindow::FindFocus();

    // first disable all existing windows
    wxWindowDisabler *wd = new wxWindowDisabler;

    // then create an "invisible" dialog: it has minimal size, is positioned
    // (hopefully) outside the screen and doesn't appear in the Alt-TAB list
    // (unlike the frames, which is why we use a dialog here)
    wxWindow *winActive = new wxDialog
                    (
                        wxTheApp->GetTopWindow(),
                        wxID_ANY,
                        "",
                        wxPoint(32600, 32600),
                        wxSize(1, 1)
                    );
    winActive->Show();

    return new ChildWaitLoopData(wd, focus, winActive);
}

void wxGUIAppTraits::AfterChildWaitLoop(void *dataOrig)
{
    wxEndBusyCursor();

    ChildWaitLoopData * const data = (ChildWaitLoopData *)dataOrig;

    delete data->wd;

    if ( data->focused )
        data->focused->SetFocus();

    // finally delete the dummy dialog and, as wd has been already destroyed
    // and the other windows reenabled, the activation is going to return to
    // the window which had had it before
    data->winActive->Destroy();

    // also delete the temporary data object itself
    delete data;
}

#if wxUSE_THREADS
bool wxGUIAppTraits::DoMessageFromThreadWait()
{
    // we should return false only if the app should exit, i.e. only if
    // Dispatch() determines that the main event loop should terminate
    wxEventLoopBase * const evtLoop = wxEventLoop::GetActive();
    if ( !evtLoop || !evtLoop->Pending() )
    {
        // no events means no quit event
        return true;
    }

    return evtLoop->Dispatch();
}

WXDWORD wxGUIAppTraits::WaitForThread(WXHANDLE hThread, wxThreadWait flags)
{
    // We only ever dispatch messages from the main thread and, additionally,
    // even from the main thread we shouldn't wait for the message if we don't
    // have a running event loop as we would never remove them from the message
    // queue then and so we would enter an infinite loop as
    // MsgWaitForMultipleObjects() keeps returning WAIT_OBJECT_0 + 1.
    if ( flags == wxThreadWait::Yield && wxIsMainThread() )
    {
        auto* const evtLoop = dynamic_cast<wxMSWEventLoopBase *>(wxEventLoop::GetActive());

        if ( evtLoop )
            return evtLoop->MSWWaitForThread(hThread);
    }

    // Simple blocking wait.
    return DoSimpleWaitForThread(hThread);
}
#endif // wxUSE_THREADS

wxPortId wxGUIAppTraits::GetToolkitVersion(int *majVer,
                                           int *minVer,
                                           int *microVer) const
{
    // on Windows, the toolkit version is the same of the OS version
    // as Windows integrates the OS kernel with the GUI toolkit.
    wxGetOsVersion(majVer, minVer, microVer);

    return wxPORT_MSW;
}

#if wxUSE_TIMER

wxTimerImpl *wxGUIAppTraits::CreateTimerImpl(wxTimer *timer)
{
    return new wxMSWTimerImpl(timer);
}

#endif // wxUSE_TIMER

std::unique_ptr<wxEventLoopBase> wxGUIAppTraits::CreateEventLoop()
{
    return std::make_unique<wxEventLoop>();
}

// ---------------------------------------------------------------------------
// Stuff for using console from the GUI applications
// ---------------------------------------------------------------------------

#if wxUSE_DYNLIB_CLASS

#include <wx/dynlib.h>

namespace
{

/*
    Helper class to manipulate console from a GUI app.

    Notice that console output is available in the GUI app only if:
    - AttachConsole() returns TRUE (which means it never works under pre-XP)
    - we have a valid STD_ERROR_HANDLE
    - command history hasn't been changed since our startup

    To check if all these conditions are verified, you need to simple call
    IsOkToUse(). It will check the first two conditions above the first time it
    is called (and if this fails, the subsequent calls will return immediately)
    and also recheck the last one every time it is called.
 */
class wxConsoleStderr
{
public:
    wxConsoleStderr() = default;

    ~wxConsoleStderr()
    {
        if ( m_hStderr != INVALID_HANDLE_VALUE )
        {
            if ( !::FreeConsole() )
            {
                wxLogLastError("FreeConsole");
            }
        }
    }

    wxConsoleStderr(const wxConsoleStderr&) = delete;
	wxConsoleStderr& operator=(const wxConsoleStderr&) = delete;

    // return true if we were successfully initialized and there had been no
    // console activity which would interfere with our output since then
    bool IsOkToUse() const
    {
        if ( m_ok == -1 )
        {
            wxConsoleStderr * const self = const_cast<wxConsoleStderr *>(this);
            self->m_ok = self->DoInit();

            // no need to call IsHistoryUnchanged() as we just initialized
            // m_history anyhow
            return m_ok == 1;
        }

        return m_ok && IsHistoryUnchanged();
    }


    // output the provided text on the console, return true if ok
    bool Write(const std::string& text);

private:
    // called by Init() once only to do the real initialization
    bool DoInit();

    // retrieve the command line history into the provided buffer and return
    // its length
    int GetCommandHistory(wxWxCharBuffer& buf) const;

    // check if the console history has changed
    bool IsHistoryUnchanged() const;

    std::wstring m_data;        // data between empty line and cursor position

    wxWxCharBuffer m_history;      // command history on startup

    wxDynamicLibrary m_dllKernel32;

    HANDLE m_hStderr{ INVALID_HANDLE_VALUE }; // console handle, if it's valid we must call
                                        // FreeConsole() (even if m_ok != 1)

    int m_ok{-1};                   // initially -1, set to true or false by Init()

    int m_historyLen{0};           // length command history buffer

    int m_dataLen{0};              // length data buffer
    int m_dataLine{0};             // line offset

    using GetConsoleCommandHistory_t = WXDWORD (WINAPI*)(LPWSTR sCommands,
                                                       WXDWORD nBufferLength,
                                                       LPCWSTR sExeName);
    using GetConsoleCommandHistoryLength_t = WXDWORD (WINAPI*)(LPCWSTR sExeName);

    GetConsoleCommandHistory_t m_pfnGetConsoleCommandHistory;
    GetConsoleCommandHistoryLength_t m_pfnGetConsoleCommandHistoryLength;
};

bool wxConsoleStderr::DoInit()
{
    HANDLE hStderr = ::GetStdHandle(STD_ERROR_HANDLE);

    if ( hStderr == INVALID_HANDLE_VALUE || !hStderr )
        return false;

    if ( !m_dllKernel32.Load("kernel32.dll") )
        return false;

    if ( !::AttachConsole(ATTACH_PARENT_PROCESS) )
        return false;

    // console attached, set m_hStderr now to ensure that we free it in the
    // dtor
    m_hStderr = hStderr;

    wxDL_INIT_FUNC_AW(m_pfn, GetConsoleCommandHistory, m_dllKernel32);
    if ( !m_pfnGetConsoleCommandHistory )
        return false;

    wxDL_INIT_FUNC_AW(m_pfn, GetConsoleCommandHistoryLength, m_dllKernel32);
    if ( !m_pfnGetConsoleCommandHistoryLength )
        return false;

    // remember the current command history to be able to compare with it later
    // in IsHistoryUnchanged()
    m_historyLen = GetCommandHistory(m_history);
    if ( !m_history )
        return false;


    // now find the first blank line above the current position
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if ( !::GetConsoleScreenBufferInfo(m_hStderr, &csbi) )
    {
        wxLogLastError("GetConsoleScreenBufferInfo");
        return false;
    }

    COORD pos;
    pos.X = 0;
    pos.Y = csbi.dwCursorPosition.Y + 1;

    // we decide that a line is empty if first 4 characters are spaces
    WXDWORD ret;
    wchar_t buf[4];
    do
    {
        pos.Y--;
        if ( !::ReadConsoleOutputCharacterW(m_hStderr, buf, WXSIZEOF(buf),
                                            pos, &ret) )
        {
            wxLogLastError("ReadConsoleOutputCharacterA");
            return false;
        }
    } while ( wxStrncmp("    ", buf, WXSIZEOF(buf)) != 0 );

    // calculate line offset and length of data
    m_dataLine = csbi.dwCursorPosition.Y - pos.Y;
    m_dataLen = m_dataLine*csbi.dwMaximumWindowSize.X + csbi.dwCursorPosition.X;

    if ( m_dataLen > 0 )
    {
        m_data.resize(m_dataLen);
        if ( !::ReadConsoleOutputCharacterW(m_hStderr, m_data.data(), m_dataLen,
                                            pos, &ret) )
        {
            wxLogLastError("ReadConsoleOutputCharacterA");
            return false;
        }
    }

    return true;
}

int wxConsoleStderr::GetCommandHistory(wxWxCharBuffer& buf) const
{
    // these functions are internal and may only be called by cmd.exe
    static constexpr auto CMD_EXE = L"cmd.exe";

    const int len = m_pfnGetConsoleCommandHistoryLength(CMD_EXE);
    if ( len )
    {
        buf.extend(len);

        int len2 = m_pfnGetConsoleCommandHistory(buf.data(), len, CMD_EXE);

        if ( len2 != len )
        {
            wxFAIL_MSG( "failed getting history?" );
        }
    }

    return len;
}

bool wxConsoleStderr::IsHistoryUnchanged() const
{
    wxASSERT_MSG( m_ok == 1, "shouldn't be called if not initialized" );

    // get (possibly changed) command history
    wxWxCharBuffer history;
    const int historyLen = GetCommandHistory(history);

    // and compare it with the original one
    return historyLen == m_historyLen && history &&
                memcmp(m_history, history, historyLen) == 0;
}

bool wxConsoleStderr::Write(const std::string& text)
{
    wxASSERT_MSG( m_hStderr != INVALID_HANDLE_VALUE,
                    "should only be called if Init() returned true" );

    // get current position
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if ( !::GetConsoleScreenBufferInfo(m_hStderr, &csbi) )
    {
        wxLogLastError("GetConsoleScreenBufferInfo");
        return false;
    }

    // and calculate new position (where is empty line)
    csbi.dwCursorPosition.X = 0;
    csbi.dwCursorPosition.Y -= m_dataLine;

    if ( !::SetConsoleCursorPosition(m_hStderr, csbi.dwCursorPosition) )
    {
        wxLogLastError("SetConsoleCursorPosition");
        return false;
    }

    WXDWORD ret;
    if ( !::FillConsoleOutputCharacterW(m_hStderr, ' ', m_dataLen,
                                       csbi.dwCursorPosition, &ret) )
    {
        wxLogLastError("FillConsoleOutputCharacter");
        return false;
    }

    if ( !::WriteConsoleW(m_hStderr, boost::nowide::widen(text).c_str(), text.length(), &ret, nullptr) )
    {
        wxLogLastError("WriteConsole");
        return false;
    }

    ::WriteConsoleW(m_hStderr, m_data.data(), m_dataLen, &ret, nullptr);

    return true;
}

wxConsoleStderr s_consoleStderr;

} // anonymous namespace

bool wxGUIAppTraits::CanUseStderr()
{
    return s_consoleStderr.IsOkToUse();
}

bool wxGUIAppTraits::WriteToStderr(const std::string& text)
{
    return s_consoleStderr.IsOkToUse() && s_consoleStderr.Write(text);
}

#else // !wxUSE_DYNLIB_CLASS

bool wxGUIAppTraits::CanUseStderr()
{
    return false;
}

bool wxGUIAppTraits::WriteToStderr([[maybe_unused]] const std::string& text)
{
    return false;
}

#endif // wxUSE_DYNLIB_CLASS/!wxUSE_DYNLIB_CLASS

WXHWND wxGUIAppTraits::GetMainHWND() const
{
    const wxWindow* const w = wxApp::GetMainTopWindow();
    return w ? w->GetHWND() : nullptr;
}

// ---------------------------------------------------------------------------
// wxWin macros
// ---------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxApp, wxEvtHandler)
    EVT_IDLE(wxApp::OnIdle)
    EVT_END_SESSION(wxApp::OnEndSession)
    EVT_QUERY_END_SESSION(wxApp::OnQueryEndSession)
wxEND_EVENT_TABLE()

// class to ensure that wxAppBase::CleanUp() is called if our Initialize()
// fails
class wxCallBaseCleanup
{
public:
    explicit wxCallBaseCleanup(wxApp *app) : m_app(app) { }
    ~wxCallBaseCleanup() { if ( m_app ) m_app->wxAppBase::CleanUp(); }

    void Dismiss() { m_app = nullptr; }

private:
    wxApp *m_app;
};

//// Initialize
bool wxApp::Initialize(int& argc_, wxChar **argv_)
{
    if ( !wxAppBase::Initialize(argc_, argv_) )
        return false;

    // ensure that base cleanup is done if we return too early
    wxCallBaseCleanup callBaseCleanup(this);

    InitCommonControls();

    wxSetKeyboardHook(true);

    callBaseCleanup.Dismiss();

    return true;
}

// ---------------------------------------------------------------------------
// Win32 window class registration
// ---------------------------------------------------------------------------

/* static */
const std::string& wxApp::GetRegisteredClassName(const std::string& name,
                                            int bgBrushCol,
                                            int extraStyles,
                                            unsigned int flags)
{
    const size_t count = gs_regClassesInfo.size();
    for ( size_t n = 0; n < count; n++ )
    {
        if ( gs_regClassesInfo[n].regname == name ||
                gs_regClassesInfo[n].regnameNR == name )
            return gs_regClassesInfo[n].GetRequestedName(flags);
    }

    // we need to register this class
    WNDCLASSW wndclass;
    wxZeroMemory(wndclass);
    
    wndclass.lpfnWndProc   = (WNDPROC)wxWndProc;
    wndclass.hInstance     = wxGetInstance();
    wndclass.hCursor       = static_cast<WXHCURSOR>(::LoadImageW(nullptr,
                                                               MAKEINTRESOURCEW(OCR_NORMAL),
                                                               IMAGE_CURSOR,
                                                               0, 0,
                                                               LR_SHARED | LR_DEFAULTSIZE));
    wndclass.hbrBackground = (WXHBRUSH)wxUIntToPtr(bgBrushCol + 1);
    wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | extraStyles;


    ClassRegInfo regClass(name, flags);
    if ( !regClass.regname.empty() )
    {
        boost::nowide::wstackstring stackRegname(regClass.regname.c_str());
        wndclass.lpszClassName = stackRegname.get();
        if ( !::RegisterClassW(&wndclass) )
        {
            wxLogLastError(fmt::format("RegisterClass(%s)",
                           regClass.regname));
            return {};
        }
    }

    wndclass.style &= ~(CS_HREDRAW | CS_VREDRAW);
    boost::nowide::wstackstring stackRegnameNR(regClass.regnameNR.c_str());
    wndclass.lpszClassName = stackRegnameNR.get();

    if ( !::RegisterClassW(&wndclass) )
    {
        wxLogLastError(wxString::Format("RegisterClass(%s)",
                       regClass.regname));
        boost::nowide::wstackstring stackRegname(regClass.regname.c_str());
        ::UnregisterClassW(stackRegname.get(), wxGetInstance());
        return {};
    }

    gs_regClassesInfo.push_back(regClass);

    // take care to return the pointer which will remain valid after the
    // function returns (it could be invalidated later if new elements are
    // added to the vector and it's reallocated but this shouldn't matter as
    // this pointer should be used right now, not stored)
    return gs_regClassesInfo.back().GetRequestedName(flags);
}

bool wxApp::IsRegisteredClassName(std::string_view name)
{
    const size_t count = gs_regClassesInfo.size();
    for ( size_t n = 0; n < count; n++ )
    {
        if ( gs_regClassesInfo[n].regname == name ||
                gs_regClassesInfo[n].regnameNR == name )
            return true;
    }

    return false;
}

void wxApp::UnregisterWindowClasses()
{
    const size_t count = gs_regClassesInfo.size();
    for ( size_t n = 0; n < count; n++ )
    {
        const ClassRegInfo& regClass = gs_regClassesInfo[n];
        if ( !regClass.regname.empty() )
        {
            if ( !::UnregisterClassW(boost::nowide::widen(regClass.regname).c_str(), wxGetInstance()) )
            {
                wxLogLastError(wxString::Format("UnregisterClass(%s)",
                               regClass.regname));
            }
        }

        if ( !::UnregisterClassW(boost::nowide::widen(regClass.regnameNR).c_str(), wxGetInstance()) )
        {
            wxLogLastError(wxString::Format("UnregisterClass(%s)",
                           regClass.regnameNR));
        }
    }

    gs_regClassesInfo.clear();
}

void wxApp::CleanUp()
{
    // all objects pending for deletion must be deleted first, otherwise
    // UnregisterWindowClasses() call wouldn't succeed (because windows
    // using the classes being unregistered still exist), so call the base
    // class method first and only then do our clean up
    wxAppBase::CleanUp();

    wxSetKeyboardHook(false);

    // for an EXE the classes are unregistered when it terminates but DLL may
    // be loaded several times (load/unload/load) into the same process in
    // which case the registration will fail after the first time if we don't
    // unregister the classes now
    UnregisterWindowClasses();
}

// ----------------------------------------------------------------------------
// wxApp idle handling
// ----------------------------------------------------------------------------

void wxApp::OnIdle([[maybe_unused]] wxIdleEvent& event)
{
#if wxUSE_DC_CACHEING
    // automated DC cache management: clear the cached DCs and bitmap
    // if it's likely that the app has finished with them, that is, we
    // get an idle event and we're not dragging anything.
    if (!::GetKeyState(MK_LBUTTON) && !::GetKeyState(MK_MBUTTON) && !::GetKeyState(MK_RBUTTON))
        wxMSWDCImpl::ClearCache();
#endif // wxUSE_DC_CACHEING
}

void wxApp::WakeUpIdle()
{
    wxEventLoopBase * const evtLoop = wxEventLoop::GetActive();
    if ( !evtLoop )
    {
        // We can't wake up the event loop if there is none and there is just
        // no need to do anything in this case, any pending events will be
        // handled when the event loop starts.
        return;
    }

    evtLoop->WakeUp();
}

void wxApp::MSWProcessPendingEventsIfNeeded()
{
    // The cast below is safe as wxEventLoop derives from wxMSWEventLoopBase in
    // both console and GUI applications.
    auto* const evtLoop =
        dynamic_cast<wxMSWEventLoopBase *>(wxEventLoop::GetActive());
    if ( evtLoop && evtLoop->MSWIsWakeUpRequested() )
        ProcessPendingEvents();
}

// ----------------------------------------------------------------------------
// other wxApp event handlers
// ----------------------------------------------------------------------------

void wxApp::OnEndSession([[maybe_unused]] wxCloseEvent& event)
{
    // Windows will terminate the process soon after we return from
    // WM_ENDSESSION handler or when we delete our last window, so make sure we
    // at least execute our cleanup code before

    // prevent the window from being destroyed when the corresponding wxTLW is
    // destroyed: this will result in a leak of a WXHWND, of course, but who
    // cares when the process is being killed anyhow
    if ( !wxTopLevelWindows.empty() )
        wxTopLevelWindows[0]->SetHWND(nullptr);

    // Destroy all the remaining TLWs before calling OnExit() to have the same
    // sequence of events in this case as in case of the normal shutdown,
    // otherwise we could have many problems due to wxApp being already
    // destroyed when window cleanup code (in close event handlers or dtor) is
    // executed.
    DeleteAllTLWs();

    const int rc = OnExit();

    wxEntryCleanup();

    // calling exit() instead of ExitProcess() or not doing anything at all and
    // being killed by Windows has the advantage of executing the dtors of
    // global objects
    exit(rc);
}

// Default behaviour: close the application with prompts. The
// user can veto the close, and therefore the end session.
void wxApp::OnQueryEndSession(wxCloseEvent& event)
{
    if (GetTopWindow())
    {
        if (!GetTopWindow()->Close(!event.CanVeto()))
            event.Veto(true);
    }
}

// ----------------------------------------------------------------------------
// system DLL versions
// ----------------------------------------------------------------------------

#if wxUSE_DYNLIB_CLASS

namespace
{

// helper function: retrieve the DLL version by using DllGetVersion(), returns
// 0 if the DLL doesn't export such function
int CallDllGetVersion(wxDynamicLibrary& dll)
{
    // now check if the function is available during run-time
    wxDYNLIB_FUNCTION( DLLGETVERSIONPROC, DllGetVersion, dll );
    if ( !pfnDllGetVersion )
        return 0;

    DLLVERSIONINFO dvi;
    dvi.cbSize = sizeof(dvi);

    HRESULT hr = (*pfnDllGetVersion)(&dvi);
    if ( FAILED(hr) )
    {
        wxLogApiError("DllGetVersion", hr);

        return 0;
    }

    return 100*dvi.dwMajorVersion + dvi.dwMinorVersion;
}

} // anonymous namespace

/* static */
int wxApp::GetComCtl32Version()
{
    // cache the result
    //
    // NB: this is MT-ok as in the worst case we'd compute s_verComCtl32 twice,
    //     but as its value should be the same both times it doesn't matter
    static int s_verComCtl32 = -1;

    if ( s_verComCtl32 == -1 )
    {
        // we're prepared to handle the errors
        wxLogNull noLog;

        // we don't want to load comctl32.dll, it should be already loaded but,
        // depending on the OS version and the presence of the manifest, it can
        // be either v5 or v6 and instead of trying to guess it just get the
        // handle of the already loaded version
        wxLoadedDLL dllComCtl32("comctl32.dll");
        if ( !dllComCtl32.IsLoaded() )
        {
            s_verComCtl32 = 0;
            return 0;
        }

        // try DllGetVersion() for recent DLLs
        s_verComCtl32 = CallDllGetVersion(dllComCtl32);

        // if DllGetVersion() is unavailable either during compile or
        // run-time, try to guess the version otherwise
        if ( !s_verComCtl32 )
        {
            // InitCommonControlsEx is unique to 4.70 and later
            void *pfn = dllComCtl32.GetSymbol("InitCommonControlsEx");
            if ( !pfn )
            {
                // not found, must be 4.00
                s_verComCtl32 = 400;
            }
            else // 4.70+
            {
                // many symbols appeared in comctl32 4.71, could use any of
                // them except may be DllInstall()
                pfn = dllComCtl32.GetSymbol("InitializeFlatSB");
                if ( !pfn )
                {
                    // not found, must be 4.70
                    s_verComCtl32 = 470;
                }
                else
                {
                    // found, must be 4.71 or later
                    s_verComCtl32 = 471;
                }
            }
        }
    }

    return s_verComCtl32;
}

#else // !wxUSE_DYNLIB_CLASS

/* static */
int wxApp::GetComCtl32Version()
{
    return 0;
}

#endif // wxUSE_DYNLIB_CLASS/!wxUSE_DYNLIB_CLASS

#if wxUSE_EXCEPTIONS

// ----------------------------------------------------------------------------
// exception handling
// ----------------------------------------------------------------------------

bool wxApp::OnExceptionInMainLoop()
{
    // ask the user about what to do: use the Win32 API function here as it
    // could be dangerous to use any wxWidgets code in this state
    switch (
            ::MessageBoxW
              (
                nullptr,
                L"An unhandled exception occurred. Press \"Abort\" to \
terminate the program,\r\n\
\"Retry\" to exit the program normally and \"Ignore\" to try to continue.",
                L"Unhandled exception",
                MB_ABORTRETRYIGNORE |
                MB_ICONERROR|
                MB_TASKMODAL
              )
           )
    {
        case IDABORT:
            throw;

        default:
            wxFAIL_MSG( "unexpected MessageBox() return code" );
            [[fallthrough]];

        case IDRETRY:
            return false;

        case IDIGNORE:
            return true;
    }
}

#endif // wxUSE_EXCEPTIONS

// ----------------------------------------------------------------------------
// Layout direction
// ----------------------------------------------------------------------------

/* static */
wxLayoutDirection wxApp::MSWGetDefaultLayout(wxWindow* parent)
{
    wxLayoutDirection dir = wxLayoutDirection::Default;

    if ( parent )
        dir = parent->GetLayoutDirection();

    if ( dir == wxLayoutDirection::Default )
    {
        if ( wxTheApp )
            dir = wxTheApp->GetLayoutDirection();
    }

    if ( dir == wxLayoutDirection::Default )
    {
        WXDWORD dwLayout;
        if ( ::GetProcessDefaultLayout(&dwLayout) )
        {
            dir = dwLayout == LAYOUT_RTL ? wxLayoutDirection::RightToLeft
                                         : wxLayoutDirection::LeftToRight;
        }
    }

    return dir;
}
