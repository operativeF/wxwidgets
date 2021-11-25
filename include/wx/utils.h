/////////////////////////////////////////////////////////////////////////////
// Name:        wx/utils.h
// Purpose:     Miscellaneous utilities
// Author:      Julian Smart
// Modified by:
// Created:     29/01/98
// Copyright:   (c) 1998 Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UTILS_H_
#define _WX_UTILS_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/object.h"
#include "wx/list.h"
#include "wx/filefn.h"
#include "wx/hashmap.h"
#include "wx/versioninfo.h"

#if wxUSE_GUI
    #include "wx/mousestate.h"
    #include "wx/gdicmn.h"
#endif

// need this for wxGetDiskSpace() as we can't, unfortunately, forward declare
// wxLongLong
#include "wx/longlong.h"

// needed for wxOperatingSystemId, wxLinuxDistributionInfo
#include "wx/platinfo.h"

#if defined(__X__)
    #include <dirent.h>
    #include <unistd.h>
#endif

import WX.WinDef;

import Utils.Geometry;

import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// Forward declaration
// ----------------------------------------------------------------------------

class wxProcess;
class wxFrame;
class wxWindow;

#if defined(__WXOSX__) && wxOSX_USE_COCOA
class wxEventLoop;
#endif

// ----------------------------------------------------------------------------
// wxMemorySize
// ----------------------------------------------------------------------------

// wxGetFreeMemory can return huge amount of memory on 32-bit platforms as well
// so to always use long long for its result type on all platforms which
// support it
#if wxUSE_LONGLONG
    using wxMemorySize = wxLongLong;
#else
    using wxMemorySize = long;
#endif

// ----------------------------------------------------------------------------
// Miscellaneous functions
// ----------------------------------------------------------------------------

// Sound the bell
void wxBell();

#if wxUSE_MSGDLG
// Show wxWidgets information
void wxInfoMessageBox(wxWindow* parent);
#endif // wxUSE_MSGDLG

wxVersionInfo wxGetLibraryVersionInfo();

// Get OS description as a user-readable string
wxString wxGetOsDescription();

// Get OS version
wxOperatingSystemId wxGetOsVersion(int *verMaj = nullptr,
                                                    int *verMin = nullptr,
                                                    int *verMicro = nullptr);

// Check is OS version is at least the specified major and minor version
bool wxCheckOsVersion(int majorVsn, int minorVsn = 0, int microVsn = 0);

// Get platform architecture bitness
bool wxIsPlatform64Bit();

// Get machine CPU architecture
wxString wxGetCpuArchitectureName();

#ifdef __LINUX__
// Get linux-distro information
wxLinuxDistributionInfo wxGetLinuxDistributionInfo();
#endif

// Return a string with the current date/time
wxString wxNow();

// Return path where wxWidgets is installed (mostly useful in Unices)
std::string wxGetInstallPrefix();
// Return path to wxWin data (/usr/share/wx/%{version}) (Unices)
std::string wxGetDataDir();

#if wxUSE_GUI

// Get the state of a key (true if pressed, false if not)
// This is generally most useful getting the state of
// the modifier or toggle keys.
bool wxGetKeyState(wxKeyCode key);

// Don't synthesize KeyUp events holding down a key and producing
// KeyDown events with autorepeat. On by default and always on
// in wxMSW.
bool wxSetDetectableAutoRepeat( bool flag );

// Returns the current state of the mouse position, buttons and modifiers
wxMouseState wxGetMouseState();

#endif // wxUSE_GUI

// ----------------------------------------------------------------------------
// wxPlatform
// ----------------------------------------------------------------------------

/*
 * Class to make it easier to specify platform-dependent values
 *
 * Examples:
 *  long val = wxPlatform::If(wxMac, 1).ElseIf(wxGTK, 2).ElseIf(stPDA, 5).Else(3);
 *  wxString strVal = wxPlatform::If(wxMac, "Mac")).ElseIf(wxMSW, wxT("MSW")).Else(wxT("Other");
 *
 * A custom platform symbol:
 *
 *  #define stPDA 100
 *  #ifdef __WXMSW__
 *      wxPlatform::AddPlatform(stPDA);
 *  #endif
 *
 *  long windowStyle = wxCAPTION | (long) wxPlatform::IfNot(stPDA, wxRESIZE_BORDER);
 *
 */

class wxPlatform
{
public:
    wxPlatform() = default;
    wxPlatform(const wxPlatform& platform) { Copy(platform); }
    void operator = (const wxPlatform& platform) { if (&platform != this) Copy(platform); }
    void Copy(const wxPlatform& platform);

    // Specify an optional default value
    wxPlatform(int defValue) {  m_longValue = 0; m_doubleValue = 0.0;  m_longValue = (long)defValue; }
    wxPlatform(long defValue) {  m_longValue = 0; m_doubleValue = 0.0;  m_longValue = defValue; }
    wxPlatform(const wxString& defValue) {  m_longValue = 0; m_doubleValue = 0.0;  m_stringValue = defValue; }
    wxPlatform(double defValue) {  m_longValue = 0; m_doubleValue = 0.0;  m_doubleValue = defValue; }

    static wxPlatform If(int platform, long value);
    static wxPlatform IfNot(int platform, long value);
    wxPlatform& ElseIf(int platform, long value);
    wxPlatform& ElseIfNot(int platform, long value);
    wxPlatform& Else(long value);

    static wxPlatform If(int platform, int value) { return If(platform, (long)value); }
    static wxPlatform IfNot(int platform, int value) { return IfNot(platform, (long)value); }
    wxPlatform& ElseIf(int platform, int value) { return ElseIf(platform, (long) value); }
    wxPlatform& ElseIfNot(int platform, int value) { return ElseIfNot(platform, (long) value); }
    wxPlatform& Else(int value) { return Else((long) value); }

    static wxPlatform If(int platform, double value);
    static wxPlatform IfNot(int platform, double value);
    wxPlatform& ElseIf(int platform, double value);
    wxPlatform& ElseIfNot(int platform, double value);
    wxPlatform& Else(double value);

    static wxPlatform If(int platform, const wxString& value);
    static wxPlatform IfNot(int platform, const wxString& value);
    wxPlatform& ElseIf(int platform, const wxString& value);
    wxPlatform& ElseIfNot(int platform, const wxString& value);
    wxPlatform& Else(const wxString& value);

    long GetInteger() const { return m_longValue; }
    const wxString& GetString() const { return m_stringValue; }
    double GetDouble() const { return m_doubleValue; }

    operator int() const { return (int) GetInteger(); }
    operator long() const { return GetInteger(); }
    operator double() const { return GetDouble(); }
    operator const wxString&() const { return GetString(); }

    static void AddPlatform(int platform);
    static bool Is(int platform);
    static void ClearPlatforms();

private:
    long                m_longValue{0};
    double              m_doubleValue{0.0};
    wxString            m_stringValue;
    inline static std::vector<int>*  sm_customPlatforms{nullptr};
};

/// Function for testing current platform
inline bool wxPlatformIs(int platform) { return wxPlatform::Is(platform); }

// ----------------------------------------------------------------------------
// Window ID management
// ----------------------------------------------------------------------------

// Ensure subsequent IDs don't clash with this one
void wxRegisterId(wxWindowID id);

// Return the current ID
wxWindowID wxGetCurrentId();

// Generate a unique ID
wxWindowID wxNewId();

// ----------------------------------------------------------------------------
// Various conversions
// ----------------------------------------------------------------------------

// Convert 2-digit hex number to decimal
int wxHexToDec(const wxString& buf);

// Convert 2-digit hex number to decimal
inline int wxHexToDec(const char* buf)
{
    int firstDigit{};
    int secondDigit{};

    if (buf[0] >= 'A')
        firstDigit = buf[0] - 'A' + 10;
    else if (buf[0] >= '0')
        firstDigit = buf[0] - '0';
    else
        firstDigit = -1;

    wxCHECK_MSG( firstDigit >= 0 && firstDigit <= 15, -1, "Invalid argument" );

    if (buf[1] >= 'A')
        secondDigit = buf[1] - 'A' + 10;
    else if (buf[1] >= '0')
        secondDigit = buf[1] - '0';
    else
        secondDigit = -1;

    wxCHECK_MSG( secondDigit >= 0 && secondDigit <= 15, -1, "Invalid argument" );

    return firstDigit * 16 + secondDigit;
}


// Convert decimal integer to 2-character hex string
void wxDecToHex(unsigned char dec, wxChar *buf);
void wxDecToHex(unsigned char dec, char* ch1, char* ch2);
wxString wxDecToHex(unsigned char dec);

// ----------------------------------------------------------------------------
// Process management
// ----------------------------------------------------------------------------

// NB: for backwards compatibility reasons the values of wxEXEC_[A]SYNC *must*
//     be 0 and 1, don't change!

enum
{
    // execute the process asynchronously
    wxEXEC_ASYNC    = 0,

    // execute it synchronously, i.e. wait until it finishes
    wxEXEC_SYNC     = 1,

    // under Windows, don't hide the child even if it's IO is redirected (this
    // is done by default)
    wxEXEC_SHOW_CONSOLE   = 2,

    // deprecated synonym for wxEXEC_SHOW_CONSOLE, use the new name as it's
    // more clear
    wxEXEC_NOHIDE = wxEXEC_SHOW_CONSOLE,

    // under Unix, if the process is the group leader then passing wxKILL_CHILDREN to wxKill
    // kills all children as well as pid
    // under Windows (NT family only), sets the CREATE_NEW_PROCESS_GROUP flag,
    // which allows to target Ctrl-Break signal to the spawned process.
    // applies to console processes only.
    wxEXEC_MAKE_GROUP_LEADER = 4,

    // by default synchronous execution disables all program windows to avoid
    // that the user interacts with the program while the child process is
    // running, you can use this flag to prevent this from happening
    wxEXEC_NODISABLE = 8,

    // by default, the event loop is run while waiting for synchronous execution
    // to complete and this flag can be used to simply block the main process
    // until the child process finishes
    wxEXEC_NOEVENTS = 16,

    // under Windows, hide the console of the child process if it has one, even
    // if its IO is not redirected
    wxEXEC_HIDE_CONSOLE = 32,

    // convenient synonym for flags given system()-like behaviour
    wxEXEC_BLOCK = wxEXEC_SYNC | wxEXEC_NOEVENTS
};

// Map storing environment variables.
using wxEnvVariableHashMap = wxStringToStringHashMap;

// Used to pass additional parameters for child process to wxExecute(). Could
// be extended with other fields later.
struct wxExecuteEnv
{
    wxString cwd;               // If empty, CWD is not changed.
    wxEnvVariableHashMap env;   // If empty, environment is unchanged.
};

// Execute another program.
//
// If flags contain wxEXEC_SYNC, return -1 on failure and the exit code of the
// process if everything was ok. Otherwise (i.e. if wxEXEC_ASYNC), return 0 on
// failure and the PID of the launched process if ok.
long wxExecute(const wxString& command,
                                unsigned int flags = wxEXEC_ASYNC,
                                wxProcess *process = nullptr,
                                const wxExecuteEnv *env = nullptr);
long wxExecute(const char* const* argv,
                                unsigned int flags = wxEXEC_ASYNC,
                                wxProcess *process = nullptr,
                                const wxExecuteEnv *env = nullptr);
long wxExecute(const wchar_t* const* argv,
                                unsigned int flags = wxEXEC_ASYNC,
                                wxProcess *process = nullptr,
                                const wxExecuteEnv *env = nullptr);

// execute the command capturing its output into an array line by line, this is
// always synchronous
long wxExecute(const wxString& command,
                                std::vector<wxString>& output,
                                unsigned int flags = 0,
                                const wxExecuteEnv *env = nullptr);

// also capture stderr (also synchronous)
long wxExecute(const wxString& command,
                                std::vector<wxString>& output,
                                std::vector<wxString>& error,
                                unsigned int flags = 0,
                                const wxExecuteEnv *env = nullptr);

#if defined(WX_WINDOWS) && wxUSE_IPC
// ask a DDE server to execute the DDE request with given parameters
bool wxExecuteDDE(const wxString& ddeServer,
                                   const wxString& ddeTopic,
                                   const wxString& ddeCommand);
#endif // WX_WINDOWS && wxUSE_IPC

enum wxSignal
{
    wxSIGNONE = 0,  // verify if the process exists under Unix
    wxSIGHUP,
    wxSIGINT,
    wxSIGQUIT,
    wxSIGILL,
    wxSIGTRAP,
    wxSIGABRT,
    wxSIGIOT = wxSIGABRT,   // another name
    wxSIGEMT,
    wxSIGFPE,
    wxSIGKILL,
    wxSIGBUS,
    wxSIGSEGV,
    wxSIGSYS,
    wxSIGPIPE,
    wxSIGALRM,
    wxSIGTERM

    // further signals are different in meaning between different Unix systems
};

enum wxKillError
{
    wxKILL_OK,              // no error
    wxKILL_BAD_SIGNAL,      // no such signal
    wxKILL_ACCESS_DENIED,   // permission denied
    wxKILL_NO_PROCESS,      // no such process
    wxKILL_ERROR            // another, unspecified error
};

enum wxKillFlags
{
    wxKILL_NOCHILDREN = 0,  // don't kill children
    wxKILL_CHILDREN = 1     // kill children
};

enum wxShutdownFlags
{
    wxSHUTDOWN_FORCE    = 1,// can be combined with other flags (MSW-only)
    wxSHUTDOWN_POWEROFF = 2,// power off the computer
    wxSHUTDOWN_REBOOT   = 4,// shutdown and reboot
    wxSHUTDOWN_LOGOFF   = 8 // close session (currently MSW-only)
};

// Shutdown or reboot the PC
bool wxShutdown(unsigned int flags = wxSHUTDOWN_POWEROFF);

// send the given signal to the process (only NONE and KILL are supported under
// Windows, all others mean TERM), return 0 if ok and -1 on error
//
// return detailed error in rc if not NULL
int wxKill(long pid,
                       wxSignal sig = wxSIGTERM,
                       wxKillError *rc = nullptr,
                       unsigned int flags = wxKILL_NOCHILDREN);

// Execute a command in an interactive shell window (always synchronously)
// If no command then just the shell
bool wxShell(const wxString& command = {});

// As wxShell(), but must give a (non interactive) command and its output will
// be returned in output array
bool wxShell(const wxString& command, std::vector<wxString>& output);

// Sleep for nSecs seconds
void wxSleep(int nSecs);

// Sleep for a given amount of milliseconds
void wxMilliSleep(unsigned long milliseconds);

// Sleep for a given amount of microseconds
void wxMicroSleep(unsigned long microseconds);

// Get the process id of the current process
unsigned long wxGetProcessId();

// Get free memory in bytes, or -1 if cannot determine amount (e.g. on UNIX)
wxMemorySize wxGetFreeMemory();

#if wxUSE_ON_FATAL_EXCEPTION

// should wxApp::OnFatalException() be called?
bool wxHandleFatalExceptions(bool doit = true);

#endif // wxUSE_ON_FATAL_EXCEPTION

// ----------------------------------------------------------------------------
// Environment variables
// ----------------------------------------------------------------------------

// returns true if variable exists (value may be NULL if you just want to check
// for this)
bool wxGetEnv(const std::string& var, std::string* value);

// set the env var name to the given value, return true on success
bool wxSetEnv(const std::string& var, const std::string& value);

// remove the env var from environment
bool wxUnsetEnv(const wxString& var);

// Retrieve the complete environment by filling specified map.
// Returns true on success or false if an error occurred.
bool wxGetEnvMap(wxEnvVariableHashMap *map);

// ----------------------------------------------------------------------------
// Network and username functions.
// ----------------------------------------------------------------------------

// NB: "char *" functions are deprecated, use wxString ones!

// Get eMail address
bool wxGetEmailAddress(wxChar *buf, int maxSize);
wxString wxGetEmailAddress();

// Get hostname.
bool wxGetHostName(wxChar *buf, int maxSize);
wxString wxGetHostName();

// Get FQDN
wxString wxGetFullHostName();
bool wxGetFullHostName(wxChar *buf, int maxSize);

// Get user ID e.g. jacs (this is known as login name under Unix)
bool wxGetUserId(wxChar *buf, int maxSize);
wxString wxGetUserId();

// Get user name e.g. Julian Smart
bool wxGetUserName(wxChar *buf, int maxSize);
wxString wxGetUserName();

// Get current Home dir and copy to dest (returns pstr->c_str())
std::string wxGetHomeDir();
std::string wxGetHomeDir(std::string* pstr);

// Get the user's (by default use the current user name) home dir,
// return empty string on error
std::string wxGetUserHome(const std::string& user = {});


#if wxUSE_LONGLONG
    using wxDiskspaceSize_t = wxLongLong;
#else
    using wxDiskspaceSize_t = long;
#endif

// get number of total/free bytes on the disk where path belongs
bool wxGetDiskSpace(const std::string& path,
                                     wxDiskspaceSize_t *pTotal = nullptr,
                                     wxDiskspaceSize_t *pFree = nullptr);



// See wx/vector.h for more about this hack.
#ifndef wxQSORT_DECLARED

#define wxQSORT_DECLARED

typedef int (*wxSortCallback)(const void* pItem1,
                              const void* pItem2,
                              const void* user_data);


void wxQsort(void* pbase, size_t total_elems,
                              size_t size, wxSortCallback cmp,
                              const void* user_data);

#endif // !wxQSORT_DECLARED


#if wxUSE_GUI // GUI only things from now on

// ----------------------------------------------------------------------------
// Launch default browser
// ----------------------------------------------------------------------------

// flags for wxLaunchDefaultBrowser
enum
{
    wxBROWSER_NEW_WINDOW   = 0x01,
    wxBROWSER_NOBUSYCURSOR = 0x02
};

// Launch url in the user's default internet browser
bool wxLaunchDefaultBrowser(const wxString& url, unsigned int flags = 0);

// Launch document in the user's default application
bool wxLaunchDefaultApplication(const std::string& path, unsigned int flags = 0);

// ----------------------------------------------------------------------------
// Menu accelerators related things
// ----------------------------------------------------------------------------

// flags for wxStripMenuCodes
enum
{
    // strip '&' characters
    wxStrip_Mnemonics = 1,

    // strip everything after '\t'
    wxStrip_Accel = 2,

    // strip mnemonics of the form "(&X)" appended to the string (used in CJK
    // translations)
    wxStrip_CJKMnemonics = 4,

    // strip everything (this doesn't include wxStrip_CJKMnemonics for
    // compatibility)
    wxStrip_All = wxStrip_Mnemonics | wxStrip_Accel,

    // strip everything including CJK mnemonics, suitable for menu items labels
    // only (despite its name, wxStripMenuCodes() is currently used for control
    // labels too)
    wxStrip_Menu = wxStrip_All | wxStrip_CJKMnemonics
};

// strip mnemonics and/or accelerators from the label
wxString
wxStripMenuCodes(const wxString& str, unsigned int flags = wxStrip_All);

// ----------------------------------------------------------------------------
// Window search
// ----------------------------------------------------------------------------

// Returns menu item id or wxNOT_FOUND if none.
int wxFindMenuItemId(wxFrame *frame, const wxString& menuString, const wxString& itemString);

// Find the wxWindow at the given point. wxGenericFindWindowAtPoint
// is always present but may be less reliable than a native version.
wxWindow* wxGenericFindWindowAtPoint(const wxPoint& pt);
wxWindow* wxFindWindowAtPoint(const wxPoint& pt);

// NB: this function is obsolete, use wxWindow::FindWindowByLabel() instead
//
// Find the window/widget with the given title or label.
// Pass a parent to begin the search from, or NULL to look through
// all windows.
wxWindow* wxFindWindowByLabel(const std::string& title, wxWindow *parent = nullptr);

// NB: this function is obsolete, use wxWindow::FindWindowByName() instead
//
// Find window by name, and if that fails, by label.
wxWindow* wxFindWindowByName(const wxString& name, wxWindow *parent = nullptr);

// ----------------------------------------------------------------------------
// Message/event queue helpers
// ----------------------------------------------------------------------------

// Yield to other apps/messages and disable user input
bool wxSafeYield(wxWindow *win = nullptr, bool onlyIfNeeded = false);

// Enable or disable input to all top level windows
void wxEnableTopLevelWindows(bool enable = true);

// Check whether this window wants to process messages, e.g. Stop button
// in long calculations.
bool wxCheckForInterrupt(wxWindow *wnd);

// Consume all events until no more left
void wxFlushEvents();

// a class which disables all windows (except, may be, the given one) in its
// ctor and enables them back in its dtor
class wxWindowDisabler
{
public:
    // this ctor conditionally disables all windows: if the argument is false,
    // it doesn't do anything
    wxWindowDisabler(bool disable = true);

    // ctor disables all windows except winToSkip
    wxWindowDisabler(wxWindow *winToSkip);

    // dtor enables back all windows disabled by the ctor
    ~wxWindowDisabler();

   wxWindowDisabler& operator=(wxWindowDisabler&&) = delete;

private:
    // disable all windows except the given one (used by both ctors)
    void DoDisable(wxWindow *winToSkip = nullptr);

#if defined(__WXOSX__) && wxOSX_USE_COCOA
    void AfterDisable(wxWindow* winToSkip);
    void BeforeEnable();

    wxEventLoop* m_modalEventLoop = NULL;
#endif
    std::vector<wxWindow*> m_winDisabled;
    bool m_disabled;
};

// ----------------------------------------------------------------------------
// Cursors
// ----------------------------------------------------------------------------

// Set the cursor to the busy cursor for all windows
void wxBeginBusyCursor(const wxCursor *cursor = wxHOURGLASS_CURSOR);

// Restore cursor to normal
void wxEndBusyCursor();

// true if we're between the above two calls
bool wxIsBusy();

// Convenience class so we can just create a wxBusyCursor object on the stack
class wxBusyCursor
{
public:
    wxBusyCursor(const wxCursor* cursor = wxHOURGLASS_CURSOR)
        { wxBeginBusyCursor(cursor); }
    ~wxBusyCursor()
        { wxEndBusyCursor(); }

    // FIXME: These two methods are currently only implemented (and needed?)
    //        in wxGTK.  BusyCursor handling should probably be moved to
    //        common code since the wxGTK and wxMSW implementations are very
    //        similar except for wxMSW using WXHCURSOR directly instead of
    //        wxCursor..  -- RL.
    static const wxCursor &GetStoredCursor();
    static const wxCursor GetBusyCursor();
};

void wxGetMousePosition( int* x, int* y );

// ----------------------------------------------------------------------------
// X11 Display access
// ----------------------------------------------------------------------------

#if defined(__X__) || (defined(__WXGTK__) && defined(__UNIX__))

#ifdef __WXGTK__
    void *wxGetDisplay();
    enum wxDisplayType
    {
        wxDisplayNone,
        wxDisplayX11,
        wxDisplayWayland
    };
    struct wxDisplayInfo
    {
        void* dpy;
        wxDisplayType type;
    };
    wxDisplayInfo wxGetDisplayInfo();
#endif

#ifdef __X__
    WXDisplay *wxGetDisplay();
    bool wxSetDisplay(const wxString& display_name);
    wxString wxGetDisplayName();
#endif // X or GTK+

// use this function instead of the functions above in implementation code
inline struct _XDisplay *wxGetX11Display()
{
    return (_XDisplay *)wxGetDisplay();
}

#endif // X11 || wxGTK

#endif // wxUSE_GUI

// ----------------------------------------------------------------------------
// wxYield(): these functions are obsolete, please use wxApp methods instead!
// ----------------------------------------------------------------------------

// avoid redeclaring this function here if it had been already declared by
// wx/app.h, this results in warnings from g++ with -Wredundant-decls
#ifndef wx_YIELD_DECLARED
#define wx_YIELD_DECLARED

// Yield to other apps/messages
bool wxYield();

#endif // wx_YIELD_DECLARED

// Like wxYield, but fails silently if the yield is recursive.
bool wxYieldIfNeeded();

// ----------------------------------------------------------------------------
// Windows resources access
// ----------------------------------------------------------------------------

// Windows only: get user-defined resource from the .res file.
#ifdef WX_WINDOWS
    // default resource type for wxLoadUserResource()
    extern WXDLLIMPEXP_DATA_BASE(const wxChar*) wxUserResourceStr;

    // Return the pointer to the resource data. This pointer is read-only, use
    // the overload below if you need to modify the data.
    //
    // Notice that the resource type can be either a real string or an integer
    // produced by MAKEINTRESOURCE(). In particular, any standard resource type,
    // i.e any RT_XXX constant, could be passed here.
    //
    // Returns true on success, false on failure. Doesn't log an error message
    // if the resource is not found (because this could be expected) but does
    // log one if any other error occurs.
    bool
    wxLoadUserResource(const void **outData,
                       size_t *outLen,
                       const std::string& resourceName,
                       const wxChar* resourceType = wxUserResourceStr,
                       WXHINSTANCE module = nullptr);

    // This function allocates a new buffer and makes a copy of the resource
    // data, remember to delete[] the buffer. And avoid using it entirely if
    // the overload above can be used.
    //
    // Returns NULL on failure.
    char*
    wxLoadUserResource(const wxString& resourceName,
                       const wxChar* resourceType = wxUserResourceStr,
                       int* pLen = nullptr,
                       WXHINSTANCE module = nullptr);
#endif // WX_WINDOWS

#endif
    // _WX_UTILSH__
