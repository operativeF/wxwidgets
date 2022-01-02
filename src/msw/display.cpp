/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/display.cpp
// Purpose:     MSW Implementation of wxDisplay class
// Author:      Royce Mitchell III, Vadim Zeitlin
// Modified by: Ryan Norton (IsPrimary override)
// Created:     06/21/02
// Copyright:   (c) wxWidgets team
// Copyright:   (c) 2002-2006 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#include "wx/msw/private.h"

#include "wx/app.h"
#include "wx/frame.h"
#include "wx/private/display.h"
#include "wx/dynlib.h"

#include <boost/nowide/convert.hpp>

#include <memory>

import WX.Win.UniqueHnd;

import <string>;
import <vector>;

namespace
{

int wxGetHDCDepth(WXHDC hdc)
{
    return ::GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
}

using msw::utils::unique_dcwnd;

// This implementation is always available, whether wxUSE_DISPLAY is 1 or not,
// as we fall back to it in case of error.
class wxDisplayImplSingleMSW : public wxDisplayImplSingle
{
public:
    wxRect GetGeometry() const override
    {
        unique_dcwnd dc{::GetDC(nullptr)};

        return {0, 0, ::GetDeviceCaps(dc.get(), HORZRES), ::GetDeviceCaps(dc.get(), VERTRES)};
    }

    wxRect GetClientArea() const override
    {
        RECT rc;
        ::SystemParametersInfoW(SPI_GETWORKAREA, 0, &rc, 0);

        wxRect rectClient;
        wxCopyRECTToRect(rc, rectClient);
        return rectClient;
    }

    int GetDepth() const override
    {
        unique_dcwnd dc{::GetDC(nullptr)};

        return wxGetHDCDepth(dc.get());
    }
};

class wxDisplayFactorySingleMSW : public wxDisplayFactorySingle
{
protected:
    wxDisplayImpl *CreateSingleDisplay() override
    {
        return new wxDisplayImplSingleMSW;
    }
};

} // anonymous namespace

#if wxUSE_DISPLAY

#include "wx/app.h"
#include "wx/frame.h"
#include "wx/dynlib.h"

#include "wx/msw/private/hiddenwin.h"

#ifndef DPI_ENUMS_DECLARED
    #define MDT_EFFECTIVE_DPI 0
#endif

import WX.Cmn.SysOpt;

namespace
{

// Simple struct storing the information needed by wxDisplayMSW.
struct wxDisplayInfo
{
    wxDisplayInfo(HMONITOR hmon_,
                  const MONITORINFOEXW& monInfo_,
                  int depth_)
        : hmon(hmon_), monInfo(monInfo_), depth(depth_)
    {}

    MONITORINFOEXW monInfo;

    HMONITOR hmon;

    int depth;
};

} // anonymous namespace

// ----------------------------------------------------------------------------
// wxDisplayMSW declaration
// ----------------------------------------------------------------------------

class wxDisplayMSW : public wxDisplayImpl
{
public:
    wxDisplayMSW(unsigned n, const wxDisplayInfo& info)
        : wxDisplayImpl(n),
          m_info(info)
    {
    }

    wxDisplayMSW(const wxDisplayMSW&) = delete;
	wxDisplayMSW& operator=(const wxDisplayMSW&) = delete;

    wxRect GetGeometry() const override;
    wxRect GetClientArea() const override;
    int GetDepth() const override;
    wxSize GetPPI() const override;
    double GetScaleFactor() const override;

    std::string GetName() const override;
    bool IsPrimary() const override;

    wxVideoMode GetCurrentMode() const override;
    wxArrayVideoModes GetModes(const wxVideoMode& mode) const override;
    bool ChangeMode(const wxVideoMode& mode) override;

protected:
    // convert a DEVMODE to our wxVideoMode
    static wxVideoMode ConvertToVideoMode(const DEVMODE& dm)
    {
        // note that dmDisplayFrequency may be 0 or 1 meaning "standard one"
        // and although 0 is ok for us we don't want to return modes with 1hz
        // refresh
        return {wx::narrow_cast<int>(dm.dmPelsWidth),
                wx::narrow_cast<int>(dm.dmPelsHeight),
                wx::narrow_cast<int>(dm.dmBitsPerPel),
                wx::narrow_cast<int>(dm.dmDisplayFrequency > 1 ? dm.dmDisplayFrequency : 0)};
    }

    wxDisplayInfo m_info;
};


// ----------------------------------------------------------------------------
// wxDisplayFactoryMSW declaration
// ----------------------------------------------------------------------------

class wxDisplayFactoryMSW : public wxDisplayFactory
{
public:
    // ctor checks if the current system supports multimon API and dynamically
    // bind the functions we need if this is the case and fills the
    // m_displays array if they're available
    wxDisplayFactoryMSW();

    // Dtor destroys the hidden window we use for getting WM_SETTINGCHANGE.
    ~wxDisplayFactoryMSW();

    wxDisplayFactoryMSW(const wxDisplayFactoryMSW&) = delete;
	wxDisplayFactoryMSW& operator=(const wxDisplayFactoryMSW&) = delete;

    bool IsOk() const { return !m_displays.empty(); }

    wxDisplayImpl *CreateDisplay(unsigned n) override;
    unsigned GetCount() override { return unsigned(m_displays.size()); }
    int GetFromPoint(const wxPoint& pt) override;
    int GetFromWindow(const wxWindow *window) override;

    void InvalidateCache() override
    {
        wxDisplayFactory::InvalidateCache();
        DoRefreshMonitors();
    }

    // Declare the second argument as int to avoid problems with older SDKs not
    // declaring MONITOR_DPI_TYPE enum.
    using GetDpiForMonitor_t = HRESULT (WINAPI*)(HMONITOR, int, WXUINT*, WXUINT*);

    // Return the pointer to GetDpiForMonitor() function which may be null if
    // not running under new enough Windows version.
    static GetDpiForMonitor_t GetDpiForMonitorPtr();

private:
    // EnumDisplayMonitors() callback
    static BOOL CALLBACK MultimonEnumProc(HMONITOR hMonitor,
                                          WXHDC hdcMonitor,
                                          LPRECT lprcMonitor,
                                          WXLPARAM dwData);

    // find the monitor corresponding to the given handle,
    // return wxNOT_FOUND if not found
    int FindDisplayFromHMONITOR(HMONITOR hmon) const;

    // Update m_displays array, used initially and by InvalidateCache().
    void DoRefreshMonitors();

    // The pointer to GetDpiForMonitorPtr(), retrieved on demand, and the
    // related data, including the DLL containing the function that we must
    // keep loaded.
    struct GetDpiForMonitorData
    {
        bool TryLoad()
        {
            if ( !m_dllShcore.Load("shcore.dll", wxDL_VERBATIM | wxDL_QUIET) )
                return false;

            wxDL_INIT_FUNC(m_pfn, GetDpiForMonitor, m_dllShcore);

            if ( !m_pfnGetDpiForMonitor )
            {
                m_dllShcore.Unload();
                return false;
            }

            return true;
        }

        void UnloadIfNecessary()
        {
            if ( m_dllShcore.IsLoaded() )
            {
                m_dllShcore.Unload();
                m_pfnGetDpiForMonitor = nullptr;
            }
        }

        wxDynamicLibrary m_dllShcore;

        GetDpiForMonitor_t m_pfnGetDpiForMonitor{nullptr};

        bool m_initialized{false};
    };

    inline static GetDpiForMonitorData ms_getDpiForMonitorData{};

    // the array containing information about all available displays, filled by
    // MultimonEnumProc()
    std::vector<wxDisplayInfo> m_displays;

    // The hidden window we use for receiving WM_SETTINGCHANGE and its class
    // name.
    WXHWND m_hiddenHwnd{nullptr};
    const wchar_t* m_hiddenClass{nullptr}; // FIXME: Use narrow string.
};

// ----------------------------------------------------------------------------
// wxDisplay implementation
// ----------------------------------------------------------------------------

// TODO: Return unique_ptr
/* static */ std::unique_ptr<wxDisplayFactory> wxDisplay::CreateFactory()
{
    auto factoryMM = std::make_unique<wxDisplayFactoryMSW>();

    if ( factoryMM->IsOk() )
        return factoryMM;

    // fall back to a stub implementation if no multimon support (Win95?)
    return std::make_unique<wxDisplayFactorySingleMSW>();
}


// ----------------------------------------------------------------------------
// wxDisplayMSW implementation
// ----------------------------------------------------------------------------

wxRect wxDisplayMSW::GetGeometry() const
{
    return wxRectFromRECT(m_info.monInfo.rcMonitor);
}

wxRect wxDisplayMSW::GetClientArea() const
{
    return wxRectFromRECT(m_info.monInfo.rcWork);
}

int wxDisplayMSW::GetDepth() const
{
    return m_info.depth;
}

wxSize wxDisplayMSW::GetPPI() const
{
    if ( const wxDisplayFactoryMSW::GetDpiForMonitor_t
            getFunc = wxDisplayFactoryMSW::GetDpiForMonitorPtr() )
    {
        WXUINT dpiX = 0,
             dpiY = 0;
        const HRESULT
            hr = (*getFunc)(m_info.hmon, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
        if ( SUCCEEDED(hr) )
            return wxSize(dpiX, dpiY);

        wxLogApiError("GetDpiForMonitor", hr);
    }

    return IsPrimary() ? wxDisplayImplSingleMSW().GetPPI() : wxSize{0, 0};
}

double wxDisplayMSW::GetScaleFactor() const
{
    const int ppi = GetPPI().y;
    return ppi ? ppi / (double)wxDisplay::GetStdPPIValue() : 1.0;
}

std::string wxDisplayMSW::GetName() const
{
    return boost::nowide::narrow(m_info.monInfo.szDevice);
}

bool wxDisplayMSW::IsPrimary() const
{
    return (m_info.monInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;
}

wxVideoMode wxDisplayMSW::GetCurrentMode() const
{
    wxVideoMode mode;

    // The first parameter of EnumDisplaySettings() must be NULL according
    // to MSDN, in order to specify the current display on the computer
    // on which the calling thread is running.
    const std::string name = GetName();

    DEVMODEW dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;

    boost::nowide::wstackstring stackDeviceName{name.c_str()};

    if ( !::EnumDisplaySettingsW(stackDeviceName.get(), ENUM_CURRENT_SETTINGS, &dm) )
    {
        wxLogLastError("EnumDisplaySettings(ENUM_CURRENT_SETTINGS)");
    }
    else
    {
        mode = ConvertToVideoMode(dm);
    }

    return mode;
}

wxArrayVideoModes wxDisplayMSW::GetModes(const wxVideoMode& modeMatch) const
{
    wxArrayVideoModes modes;

    // The first parameter of EnumDisplaySettings() must be NULL according
    // to MSDN, in order to specify the current display on the computer
    // on which the calling thread is running.
    const std::string name = GetName();
    boost::nowide::wstackstring stackDeviceName{name.c_str()};

    DEVMODEW dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;

    for ( int iModeNum = 0;
          ::EnumDisplaySettingsW(stackDeviceName.get(), iModeNum, &dm);
          iModeNum++ )
    {
        // Only care about the default display output, this prevents duplicate
        // entries in the modes list.
        if ( dm.dmFields & DM_DISPLAYFIXEDOUTPUT &&
             dm.dmDisplayFixedOutput != DMDFO_DEFAULT )
        {
            continue;
        }

        const wxVideoMode mode = ConvertToVideoMode(dm);
        if ( mode.Matches(modeMatch) )
        {
            modes.push_back(mode);
        }
    }

    return modes;
}

bool wxDisplayMSW::ChangeMode(const wxVideoMode& mode)
{
    // prepare ChangeDisplaySettingsEx() parameters
    DEVMODEW dm;
    DEVMODEW *pDevMode;

    unsigned int flags{};

    if ( mode == wxDefaultVideoMode )
    {
        // reset the video mode to default
        pDevMode = nullptr;
    }
    else // change to the given mode
    {
        wxCHECK_MSG( mode.GetWidth() && mode.GetHeight(), false,
                        "at least the width and height must be specified" );

        wxZeroMemory(dm);
        dm.dmSize = sizeof(dm);
        dm.dmDriverExtra = 0;
        dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
        dm.dmPelsWidth = mode.GetWidth();
        dm.dmPelsHeight = mode.GetHeight();

        if ( mode.GetDepth() )
        {
            dm.dmFields |= DM_BITSPERPEL;
            dm.dmBitsPerPel = mode.GetDepth();
        }

        if ( mode.GetRefresh() )
        {
            dm.dmFields |= DM_DISPLAYFREQUENCY;
            dm.dmDisplayFrequency = mode.GetRefresh();
        }

        pDevMode = &dm;

        flags = CDS_FULLSCREEN;
    }


    // do change the mode
    switch ( ::ChangeDisplaySettingsExW
             (
                boost::nowide::widen(GetName()).c_str(),  // display name
                pDevMode,           // dev mode or NULL to reset
                wxRESERVED_PARAM,
                flags,
                nullptr                // pointer to video parameters (not used)
             ) )
    {
        case DISP_CHANGE_SUCCESSFUL:
            // ok
            {
                // If we have a top-level, full-screen frame, emulate
                // the DirectX behaviour and resize it.  This makes this
                // API quite a bit easier to use.
                wxWindow *winTop = wxTheApp->GetTopWindow();
                wxFrame *frameTop = dynamic_cast<wxFrame*>(winTop);
                if (frameTop && frameTop->IsFullScreen())
                {
                    wxVideoMode current = GetCurrentMode();
                    frameTop->SetClientSize(current.GetWidth(), current.GetHeight());
                }
            }
            return true;

        case DISP_CHANGE_BADMODE:
            // don't complain about this, this is the only "expected" error
            break;

        default:
            wxFAIL_MSG( "unexpected ChangeDisplaySettingsEx() return value" );
    }

    return false;
}


// ----------------------------------------------------------------------------
// wxDisplayFactoryMSW implementation
// ----------------------------------------------------------------------------

LRESULT APIENTRY
wxDisplayWndProc(WXHWND hwnd, WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
{
    if ( (msg == WM_SETTINGCHANGE && wParam == SPI_SETWORKAREA) ||
            msg == WM_DISPLAYCHANGE )
    {
        wxDisplay::InvalidateCache();

        return 0;
    }

    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}

wxDisplayFactoryMSW::wxDisplayFactoryMSW()
{
    DoRefreshMonitors();

    // Also create a hidden window to listen for WM_SETTINGCHANGE that we
    // receive when a monitor is added to or removed from the system as we must
    // refresh our monitor handles information then.
    m_hiddenHwnd = wxCreateHiddenWindow
                   (
                    &m_hiddenClass,
                    L"wxDisplayHiddenWindow", // FIXME: Use narrow string.
                    wxDisplayWndProc
                   );
}

wxDisplayFactoryMSW::~wxDisplayFactoryMSW()
{
    if ( m_hiddenHwnd )
    {
        if ( !::DestroyWindow(m_hiddenHwnd) )
        {
            wxLogLastError("DestroyWindow(wxDisplayHiddenWindow)");
        }

        if ( m_hiddenClass )
        {
            if ( !::UnregisterClassW(m_hiddenClass, wxGetInstance()) )
            {
                wxLogLastError("UnregisterClass(wxDisplayHiddenWindow)");
            }
        }
    }

    if ( ms_getDpiForMonitorData.m_initialized )
    {
        ms_getDpiForMonitorData.UnloadIfNecessary();
        ms_getDpiForMonitorData.m_initialized = false;
    }
}

/* static */
wxDisplayFactoryMSW::GetDpiForMonitor_t
wxDisplayFactoryMSW::GetDpiForMonitorPtr()
{
    if ( !ms_getDpiForMonitorData.m_initialized )
    {
        ms_getDpiForMonitorData.m_initialized = true;
        ms_getDpiForMonitorData.TryLoad();
    }

    return ms_getDpiForMonitorData.m_pfnGetDpiForMonitor;
}

void wxDisplayFactoryMSW::DoRefreshMonitors()
{
    m_displays.clear();

    // Note that we pass NULL as first parameter here because using screen WXHDC
    // doesn't work reliably: notably, it doesn't enumerate any displays if
    // this code is executed while a UAC prompt is shown or during log-off.
    if ( !::EnumDisplayMonitors(nullptr, nullptr, MultimonEnumProc, (WXLPARAM)this) )
    {
        wxLogLastError("EnumDisplayMonitors");
    }
}

/* static */
BOOL CALLBACK
wxDisplayFactoryMSW::MultimonEnumProc(
    HMONITOR hMonitor,              // handle to display monitor
    WXHDC /* hdcMonitor */,           // handle to monitor-appropriate device context:
                                    // not set due to our use of EnumDisplayMonitors(NULL, ...)
    [[maybe_unused]] LPRECT lprcMonitor,   // pointer to monitor intersection rectangle
    WXLPARAM dwData)                  // data passed from EnumDisplayMonitors (this)
{
    wxDisplayFactoryMSW *const self = (wxDisplayFactoryMSW *)dwData;

    WinStruct<MONITORINFOEXW> monInfo;
    if ( !::GetMonitorInfoW(hMonitor, &monInfo) )
    {
        wxLogLastError("GetMonitorInfo");
    }

    WXHDC hdcMonitor = ::CreateDCW(nullptr, monInfo.szDevice, nullptr, nullptr);
    const int hdcDepth = wxGetHDCDepth(hdcMonitor);
    ::DeleteDC(hdcMonitor);

    self->m_displays.push_back(wxDisplayInfo(hMonitor, monInfo, hdcDepth));

    // continue the enumeration
    return TRUE;
}

wxDisplayImpl *wxDisplayFactoryMSW::CreateDisplay(unsigned n)
{
    wxCHECK_MSG( n < m_displays.size(), nullptr, "An invalid index was passed to wxDisplay" );

    return new wxDisplayMSW(n, m_displays[n]);
}

// helper for GetFromPoint() and GetFromWindow()
// FIXME: use iterators.
int wxDisplayFactoryMSW::FindDisplayFromHMONITOR(HMONITOR hmon) const
{
    if ( hmon )
    {
        const size_t count = m_displays.size();
        for ( size_t n = 0; n < count; n++ )
        {
            if ( hmon == m_displays[n].hmon )
                return n;
        }
    }

    return wxNOT_FOUND;
}

int wxDisplayFactoryMSW::GetFromPoint(const wxPoint& pt)
{
    POINT pt2
    {
        .x = pt.x,
        .y = pt.y
    };

    return FindDisplayFromHMONITOR(::MonitorFromPoint(pt2,
                                                       MONITOR_DEFAULTTONULL));
}

int wxDisplayFactoryMSW::GetFromWindow(const wxWindow *window)
{
#ifdef __WXMSW__
    return FindDisplayFromHMONITOR(::MonitorFromWindow(GetHwndOf(window),
                                                        MONITOR_DEFAULTTONULL));
#else
    const wxSize halfsize = window->GetSize() / 2;
    wxPoint pt = window->GetScreenPosition();
    pt.x += halfsize.x;
    pt.y += halfsize.y;
    return GetFromPoint(pt);
#endif
}

#else // !wxUSE_DISPLAY

// In this case, wxDisplayFactorySingleMSW is the only implementation.
wxDisplayFactory* wxDisplay::CreateFactory()
{
    return new wxDisplayFactorySingleMSW;
}

#endif // wxUSE_DISPLAY/!wxUSE_DISPLAY
