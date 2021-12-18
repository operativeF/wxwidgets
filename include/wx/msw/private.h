/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private.h
// Purpose:     Private declarations: as this header is only included by
//              wxWidgets itself, it may contain identifiers which don't start
//              with "wx".
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRIVATE_H_
#define _WX_PRIVATE_H_

#include "wx/log.h"

#if wxUSE_GUI
    #include "wx/window.h"
#endif // wxUSE_GUI

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

import WX.WinDef;

import Utils.Geometry;

import <string>;

class wxFont;
class wxWindow;
class wxWindowBase;

// ---------------------------------------------------------------------------
// private constants
// ---------------------------------------------------------------------------

// 260 was taken from windef.h
// FIXME: This is no longer true for all windows systems.
#ifndef MAX_PATH
    #define MAX_PATH  260
#endif

// Many MSW functions have parameters which are "reserved". Passing them this
// constant is more clear than just using "0" or "NULL".
#define wxRESERVED_PARAM    0

// ---------------------------------------------------------------------------
// global data
// ---------------------------------------------------------------------------

extern WXHINSTANCE wxhInstance;

extern "C"
{
    WXHINSTANCE wxGetInstance();
}

void wxSetInstance(WXHINSTANCE hInst);

// ---------------------------------------------------------------------------
// define things missing from some compilers' headers
// ---------------------------------------------------------------------------

// this defines a CASTWNDPROC macro which casts a pointer to the type of a
// window proc
#if defined(STRICT) || defined(__GNUC__)
    using WndProcCast = WNDPROC;
#else
    typedef FARPROC WndProcCast;
#endif

#define CASTWNDPROC (WndProcCast)


// ---------------------------------------------------------------------------
// misc macros
// ---------------------------------------------------------------------------

#if wxUSE_GUI

#define MEANING_CHARACTER '0'
#define DEFAULT_ITEM_WIDTH  100
#define DEFAULT_ITEM_HEIGHT 80

// Return the height of a native text control corresponding to the given
// character height (as returned by GetCharHeight() or wxGetCharSize()).
//
// The wxWindow parameter is currently not used but should still be valid.
inline int wxGetEditHeightFromCharHeight(int cy, [[maybe_unused]] const wxWindow* w)
{
    // The value 8 here is empiric, i.e. it's not necessarily correct, but
    // seems to work relatively well.
    // Don't use FromDIP(8), this seems not needed.
    return cy + 8;
}

// Compatibility macro used in the existing code. It assumes that it's called
// from a method of wxWindow-derived object.
#define EDIT_HEIGHT_FROM_CHAR_HEIGHT(cy) \
    wxGetEditHeightFromCharHeight((cy), this)

// Generic subclass proc, for panel item moving/sizing and intercept
// EDIT control VK_RETURN messages
extern LONG APIENTRY
  wxSubclassedGenericControlProc(WXHWND hWnd, WXUINT message, WXWPARAM wParam, WXLPARAM lParam);

#endif // wxUSE_GUI

// ---------------------------------------------------------------------------
// useful macros and functions
// ---------------------------------------------------------------------------

// a wrapper macro for ZeroMemory()
#define wxZeroMemory(obj)   ::ZeroMemory(&obj, sizeof(obj))

// This one is a macro so that it can be tested with #ifdef, it will be
// undefined if it cannot be implemented for a given compiler.
// Vc++, dmc, ow, mingw akk have _get_osfhandle() and Cygwin has
// get_osfhandle. Others are currently unknown, e.g. Salford, Intel, Visual
// Age.
#if defined(__CYGWIN__)
    #define wxGetOSFHandle(fd) ((HANDLE)get_osfhandle(fd))
#elif defined(__VISUALC__) \
   || defined(__MINGW32__)
    #define wxGetOSFHandle(fd) ((HANDLE)_get_osfhandle(fd))
    #define wxOpenOSFHandle(h, flags) (_open_osfhandle(wxPtrToUInt(h), flags))

    wxDECL_FOR_STRICT_MINGW32(FILE*, _fdopen, (int, const char*))
    #define wx_fdopen _fdopen
#endif

// close the handle in the class dtor
template <wxUIntPtr INVALID_VALUE>
class AutoHANDLE
{
public:
    explicit AutoHANDLE(HANDLE handle = InvalidHandle()) : m_handle(handle) { }

    bool IsOk() const { return m_handle != InvalidHandle(); }
    operator HANDLE() const { return m_handle; }

    ~AutoHANDLE() { if ( IsOk() ) DoClose(); }

    void Close()
    {
        wxCHECK_RET(IsOk(), "Handle must be valid");

        DoClose();

        m_handle = InvalidHandle();
    }

protected:
    // We need this helper function because integer INVALID_VALUE is not
    // implicitly convertible to HANDLE, which is a pointer.
    static HANDLE InvalidHandle()
    {
        wxUIntPtr h = INVALID_VALUE;
        return reinterpret_cast<HANDLE>(h);
    }

    void DoClose()
    {
        if ( !::CloseHandle(m_handle) )
            wxLogLastError("CloseHandle");
    }

    WXHANDLE m_handle;
};

// a template to make initializing Windows structs less painful: it zeros all
// the struct fields and also sets cbSize member to the correct value (and so
// can be only used with structures which have this member...)
template <class T>
struct WinStruct : public T
{
    WinStruct()
    {
        ::ZeroMemory(this, sizeof(T));

        // explicit qualification is required here for this to be valid C++
        this->cbSize = sizeof(T);
    }
};


// Macros for converting wxString to the type expected by API functions.
//
// Normally it is enough to just use wxString::t_str() which is implicitly
// convertible to LPCTSTR, but in some cases an explicit conversion is required.
//
// In such cases wxMSW_CONV_LPCTSTR() should be used. But if an API function
// takes a non-const pointer, wxMSW_CONV_LPTSTR() which casts away the
// constness (but doesn't make it possible to really modify the returned
// pointer, of course) should be used. And if a string is passed as LPARAM, use
// wxMSW_CONV_LPARAM() which does the required ugly reinterpret_cast<> too.
#define wxMSW_CONV_LPCTSTR(s) static_cast<const wxChar *>((s).t_str())
#define wxMSW_CONV_LPTSTR(s) const_cast<wxChar *>(wxMSW_CONV_LPCTSTR(s))
#define wxMSW_CONV_LPARAM(s) reinterpret_cast<LPARAM>(wxMSW_CONV_LPCTSTR(s))


#if wxUSE_GUI

#include "wx/colour.h"

import Utils.Geometry;

#ifdef COM_DECLSPEC_NOTHROW
    #define wxSTDMETHODIMP COM_DECLSPEC_NOTHROW STDMETHODIMP
#else
    #define wxSTDMETHODIMP STDMETHODIMP
#endif

// make conversion from wxColour and COLORREF a bit less painful
inline COLORREF wxColourToRGB(const wxColour& c)
{
    return RGB(c.Red(), c.Green(), c.Blue());
}

inline COLORREF wxColourToPalRGB(const wxColour& c)
{
    return PALETTERGB(c.Red(), c.Green(), c.Blue());
}

inline wxColour wxRGBToColour(COLORREF rgb)
{
    return wxColour(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
}

inline void wxRGBToColour(wxColour& c, COLORREF rgb)
{
    c.Set(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
}

// get the standard colour map for some standard colours - see comment in this
// function to understand why is it needed and when should it be used
//
// it returns a wxCOLORMAP (can't use COLORMAP itself here as comctl32.dll
// might be not included/available) array of size wxSTD_COLOUR_MAX
//
// NB: if you change these colours, update wxBITMAP_STD_COLOURS in the
//     resources as well: it must have the same number of pixels!
enum wxSTD_COLOUR
{
    wxSTD_COL_BTNTEXT,
    wxSTD_COL_BTNSHADOW,
    wxSTD_COL_BTNFACE,
    wxSTD_COL_BTNHIGHLIGHT,
    wxSTD_COL_MAX
};

struct wxCOLORMAP
{
    COLORREF from, to;
};

// this function is implemented in src/msw/window.cpp
extern wxCOLORMAP *wxGetStdColourMap();

// create a wxRect from Windows RECT
inline wxRect wxRectFromRECT(const RECT& rc)
{
    return wxRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}

// copy Windows RECT to our wxRect
inline void wxCopyRECTToRect(const RECT& rc, wxRect& rect)
{
    rect = wxRectFromRECT(rc);
}

// and vice versa
inline void wxCopyRectToRECT(const wxRect& rect, RECT& rc)
{
    // note that we don't use wxRect::GetRight() as it is one of compared to
    // wxRectFromRECT() above
    rc.top = rect.y;
    rc.left = rect.x;
    rc.right = rect.x + rect.width;
    rc.bottom = rect.y + rect.height;
}

// translations between HIMETRIC units (which OLE likes) and pixels (which are
// liked by all the others) - implemented in msw/utilsexc.cpp
extern void HIMETRICToPixel(LONG *x, LONG *y);
extern void HIMETRICToPixel(LONG *x, LONG *y, WXHDC hdcRef);
extern void PixelToHIMETRIC(LONG *x, LONG *y);
extern void PixelToHIMETRIC(LONG *x, LONG *y, WXHDC hdcRef);

// Windows convention of the mask is opposed to the wxWidgets one, so we need
// to invert the mask each time we pass one/get one to/from Windows
extern WXHBITMAP wxInvertMask(WXHBITMAP hbmpMask, int w = 0, int h = 0);

// Creates an icon or cursor depending from a bitmap
//
// The bitmap must be valid and it should have a mask. If it doesn't, a default
// mask is created using light grey as the transparent colour.
extern WXHICON wxBitmapToHICON(const wxBitmap& bmp);

// Same requirements as above apply and the bitmap must also have the correct
// size.
extern
WXHCURSOR wxBitmapToHCURSOR(const wxBitmap& bmp, int hotSpotX, int hotSpotY);

extern int wxGetSystemMetrics(int nIndex, const wxWindow* win);

extern bool wxSystemParametersInfo(WXUINT uiAction, WXUINT uiParam,
                                   PVOID pvParam, WXUINT fWinIni,
                                   const wxWindow* win);

#if wxUSE_OWNER_DRAWN

// Draw the bitmap in specified state (this is used by owner drawn controls)
enum wxDSBStates
{
    wxDSB_NORMAL = 0,
    wxDSB_SELECTED,
    wxDSB_DISABLED
};

extern
BOOL wxDrawStateBitmap(WXHDC hDC, WXHBITMAP hBitmap, int x, int y, WXUINT uState);

#endif // wxUSE_OWNER_DRAWN

// get the current state of SHIFT/CTRL/ALT keys
inline bool wxIsModifierDown(int vk)
{
    // GetKeyState() returns different negative values on WinME and WinNT,
    // so simply test for negative value.
    return ::GetKeyState(vk) < 0;
}

inline bool wxIsShiftDown()
{
    return wxIsModifierDown(VK_SHIFT);
}

inline bool wxIsCtrlDown()
{
    return wxIsModifierDown(VK_CONTROL);
}

inline bool wxIsAltDown()
{
    return wxIsModifierDown(VK_MENU);
}

inline bool wxIsAnyModifierDown()
{
    return wxIsShiftDown() || wxIsCtrlDown() || wxIsAltDown();
}

// wrapper around GetWindowRect() and GetClientRect() APIs doing error checking
// for Win32
inline RECT wxGetWindowRect(WXHWND hwnd)
{
    RECT rect;

    if ( !::GetWindowRect(hwnd, &rect) )
    {
        wxLogLastError("GetWindowRect");
    }

    return rect;
}

inline RECT wxGetClientRect(WXHWND hwnd)
{
    RECT rect;

    if ( !::GetClientRect(hwnd, &rect) )
    {
        wxLogLastError("GetClientRect");
    }

    return rect;
}

// ---------------------------------------------------------------------------
// small helper classes
// ---------------------------------------------------------------------------

// This class can only be used with wxMSW wxWindow, as it doesn't have
// {Set,Get}WXHWND() methods in the other ports, but this file is currently
// included for wxQt/MSW too. It's not clear whether it should be, really, but
// for now allow it to compile in this port too.
#ifdef __WXMSW__

// Temporarily assign the given WXHWND to the window in ctor and unset it back to
// the original value (usually 0) in dtor.
class TempHWNDSetter
{
public:
    TempHWNDSetter(wxWindow* win, WXHWND hWnd)
        : m_win(win), m_hWndOrig(m_win->GetHWND())
    {
        m_win->SetHWND(hWnd);
    }

    ~TempHWNDSetter()
    {
        m_win->SetHWND(m_hWndOrig);
    }

	TempHWNDSetter& operator=(TempHWNDSetter&&) = delete;

private:
    wxWindow* const m_win;
    WXHWND const m_hWndOrig;
};

#endif // __WXMSW__

// create an instance of this class and use it as the WXHDC for screen, will
// automatically release the DC going out of scope
class ScreenHDC
{
public:
    ScreenHDC() { m_hdc = ::GetDC(nullptr);    }
   ~ScreenHDC() { ::ReleaseDC(nullptr, m_hdc); }

	ScreenHDC& operator=(ScreenHDC&&) = delete;

    operator WXHDC() const { return m_hdc; }

private:
    WXHDC m_hdc;
};

// the same as ScreenHDC but for window DCs
class WindowHDC
{
public:
    WindowHDC()  = default;
    WindowHDC(WXHWND hwnd) { m_hdc = ::GetDC(m_hwnd = hwnd); }
   ~WindowHDC() { if ( m_hwnd && m_hdc ) { ::ReleaseDC(m_hwnd, m_hdc); } }
	
    WindowHDC& operator=(WindowHDC&&) = delete;

    operator WXHDC() const { return m_hdc; }

private:
   WXHWND m_hwnd{nullptr};
   WXHDC m_hdc{nullptr};
};

// the same as ScreenHDC but for memory DCs: creates the WXHDC compatible with
// the given one (screen by default) in ctor and destroys it in dtor
class MemoryHDC
{
public:
    MemoryHDC(WXHDC hdc = nullptr) { m_hdc = ::CreateCompatibleDC(hdc); }
   ~MemoryHDC() { ::DeleteDC(m_hdc); }

	MemoryHDC& operator=(MemoryHDC&&) = delete;

    operator WXHDC() const { return m_hdc; }

private:
    WXHDC m_hdc;
};

// a class which selects a GDI object into a DC in its ctor and deselects in
// dtor
class SelectInHDC
{
private:
    void DoInit(HGDIOBJ hgdiobj) { m_hgdiobj = ::SelectObject(m_hdc, hgdiobj); }

public:
    SelectInHDC()  = default;
    SelectInHDC(WXHDC hdc, HGDIOBJ hgdiobj) : m_hdc(hdc) { DoInit(hgdiobj); }

    void Init(WXHDC hdc, HGDIOBJ hgdiobj)
    {
        wxASSERT_MSG( !m_hdc, "initializing twice?" );

        m_hdc = hdc;

        DoInit(hgdiobj);
    }

	SelectInHDC& operator=(SelectInHDC&&) = delete;

    ~SelectInHDC() { if ( m_hdc ) ::SelectObject(m_hdc, m_hgdiobj); }

    // return true if the object was successfully selected
    operator bool() const { return m_hgdiobj != nullptr; }

private:
    WXHDC m_hdc{nullptr};
    HGDIOBJ m_hgdiobj{nullptr};
};

// Class automatically freeing ICONINFO struct fields after retrieving it using
// GetIconInfo().
class AutoIconInfo : public ICONINFO
{
public:
    AutoIconInfo() { wxZeroMemory(*this); }

    bool GetFrom(WXHICON hIcon)
    {
        if ( !::GetIconInfo(hIcon, this) )
        {
            wxLogLastError("GetIconInfo");
            return false;
        }

        return true;
    }

    ~AutoIconInfo()
    {
        if ( hbmColor )
            ::DeleteObject(hbmColor);
        if ( hbmMask )
            ::DeleteObject(hbmMask);
    }
};

// class sets the specified clipping region during its life time
class HDCClipper
{
public:
    HDCClipper(WXHDC hdc, WXHRGN hrgn)
        : m_hdc(hdc)
    {
        if ( !::SelectClipRgn(hdc, hrgn) )
        {
            wxLogLastError("SelectClipRgn");
        }
    }

    ~HDCClipper()
    {
        ::SelectClipRgn(m_hdc, nullptr);
    }

	HDCClipper& operator=(HDCClipper&&) = delete;

private:
    WXHDC m_hdc;
};

// set the given map mode for the life time of this object
    class HDCMapModeChanger
    {
    public:
        HDCMapModeChanger(WXHDC hdc, int mm)
            : m_hdc(hdc)
        {
            m_modeOld = ::SetMapMode(hdc, mm);
            if ( !m_modeOld )
            {
                wxLogLastError("SelectClipRgn");
            }
        }

        ~HDCMapModeChanger()
        {
            if ( m_modeOld )
                ::SetMapMode(m_hdc, m_modeOld);
        }
    
    	HDCMapModeChanger& operator=(HDCMapModeChanger&&) = delete;

    private:
        WXHDC m_hdc;
        int m_modeOld;
    };

    #define wxCHANGE_HDC_MAP_MODE(hdc, mm) \
        HDCMapModeChanger wxMAKE_UNIQUE_NAME(wxHDCMapModeChanger)(hdc, mm)

// smart pointer using GlobalAlloc/GlobalFree()
class GlobalPtr
{
public:
    // default ctor, call Init() later
    GlobalPtr() = default;

	GlobalPtr& operator=(GlobalPtr&&) = delete;

    // allocates a block of given size
    void Init(size_t size, unsigned flags = GMEM_MOVEABLE)
    {
        m_hGlobal = ::GlobalAlloc(flags, size);
        if ( !m_hGlobal )
        {
            wxLogLastError("GlobalAlloc");
        }
    }

    GlobalPtr(size_t size, unsigned flags = GMEM_MOVEABLE)
    {
        Init(size, flags);
    }

    ~GlobalPtr()
    {
        if ( m_hGlobal && ::GlobalFree(m_hGlobal) )
        {
            wxLogLastError("GlobalFree");
        }
    }

    // implicit conversion
    operator HGLOBAL() const { return m_hGlobal; }

private:
    HGLOBAL m_hGlobal{nullptr};
};

// when working with global pointers (which is unfortunately still necessary
// sometimes, e.g. for clipboard) it is important to unlock them exactly as
// many times as we lock them which just asks for using a "smart lock" class
class GlobalPtrLock
{
public:
    // default ctor, use Init() later -- should only be used if the HGLOBAL can
    // be NULL (in which case Init() shouldn't be called)
    GlobalPtrLock() = default;

	GlobalPtrLock& operator=(GlobalPtrLock&&) = delete;

    // initialize the object, may be only called if we were created using the
    // default ctor; HGLOBAL must not be NULL
    void Init(HGLOBAL hGlobal)
    {
        m_hGlobal = hGlobal;

        // NB: GlobalLock() is a macro, not a function, hence don't use the
        //     global scope operator with it (and neither with GlobalUnlock())
        m_ptr = GlobalLock(hGlobal);
        if ( !m_ptr )
        {
            wxLogLastError("GlobalLock");
        }
    }

    // initialize the object, HGLOBAL must not be NULL
    GlobalPtrLock(HGLOBAL hGlobal)
    {
        Init(hGlobal);
    }

    ~GlobalPtrLock()
    {
        if ( m_hGlobal && !GlobalUnlock(m_hGlobal) )
        {
            // this might happen simply because the block became unlocked
            const WXDWORD dwLastError = ::GetLastError();
            if ( dwLastError != NO_ERROR )
            {
                wxLogApiError("GlobalUnlock", dwLastError);
            }
        }
    }

    void *Get() const { return m_ptr; }
    operator void *() const { return m_ptr; }

    size_t GetSize() const
    {
        const size_t size = ::GlobalSize(m_hGlobal);
        if ( !size )
            wxLogLastError("GlobalSize");

        return size;
    }

private:
    HGLOBAL m_hGlobal{nullptr};
    void *m_ptr{nullptr};
};

// register the class when it is first needed and unregister it in dtor
class ClassRegistrar
{
public:
    // ctor doesn't register the class, call Initialize() for this
    ClassRegistrar() { m_registered = -1; }

    // return true if the class is already registered
    bool IsInitialized() const { return m_registered != -1; }

    // return true if the class had been already registered
    bool IsRegistered() const { return m_registered == 1; }

    // try to register the class if not done yet, return true on success
    bool Register(const WNDCLASSW& wc)
    {
        // we should only be called if we hadn't been initialized yet
        wxASSERT_MSG( m_registered == -1,
                        "calling ClassRegistrar::Register() twice?" );

        m_registered = ::RegisterClassW(&wc) ? 1 : 0;
        if ( !IsRegistered() )
        {
            wxLogLastError("RegisterClassEx()");
        }
        else
        {
            m_clsname = boost::nowide::narrow(wc.lpszClassName).c_str();
        }

        return m_registered == 1;
    }

    // get the name of the registered class (returns empty string if not
    // registered)
    const std::string& GetName() const { return m_clsname; }

    // unregister the class if it had been registered
    ~ClassRegistrar()
    {
        if ( IsRegistered() )
        {
            if ( !::UnregisterClassW(boost::nowide::widen(m_clsname).c_str(), wxGetInstance()) )
            {
                wxLogLastError("UnregisterClass");
            }
        }
    }

private:
    // initial value is -1 which means that we hadn't tried registering the
    // class yet, it becomes true or false (1 or 0) when Initialize() is called
    int m_registered;

    // the name of the class, only non empty if it had been registered
    std::string m_clsname;
};

// ---------------------------------------------------------------------------
// macros to make casting between WXFOO and FOO a bit easier: the GetFoo()
// returns Foo cast to the Windows type for ourselves, while GetFooOf() takes
// an argument which should be a pointer or reference to the object of the
// corresponding class (this depends on the macro)
// ---------------------------------------------------------------------------

#define GetHwnd()               ((WXHWND)GetHWND())
#define GetHwndOf(win)          ((WXHWND)((win)->GetHWND()))

#define GetHdc()                ((WXHDC)GetHDC())
#define GetHdcOf(dc)            ((WXHDC)(dc).GetHDC())

#define GetHbitmap()            ((WXHBITMAP)GetHBITMAP())
#define GetHbitmapOf(bmp)       ((WXHBITMAP)(bmp).GetHBITMAP())

#define GetHicon()              ((WXHICON)GetHICON())
#define GetHiconOf(icon)        ((WXHICON)(icon).GetHICON())

#define GetHaccel()             ((WXHACCEL)GetHACCEL())
#define GetHaccelOf(table)      ((WXHACCEL)((table).GetHACCEL()))

#define GetHbrush()             ((WXHBRUSH)GetResourceHandle())
#define GetHbrushOf(brush)      ((WXHBRUSH)(brush).GetResourceHandle())

#define GetHmenu()              ((WXHMENU)GetHMenu())
#define GetHmenuOf(menu)        ((WXHMENU)(menu)->GetHMenu())

#define GetHcursor()            ((WXHCURSOR)GetHCURSOR())
#define GetHcursorOf(cursor)    ((WXHCURSOR)(cursor).GetHCURSOR())

#define GetHfont()              ((WXHFONT)GetHFONT())
#define GetHfontOf(font)        ((WXHFONT)(font).GetHFONT())

#define GetHimagelist()         ((HIMAGELIST)GetHIMAGELIST())
#define GetHimagelistOf(imgl)   ((HIMAGELIST)(imgl)->GetHIMAGELIST())

#define GetHpalette()           ((WXHPALETTE)GetHPALETTE())
#define GetHpaletteOf(pal)      ((WXHPALETTE)(pal).GetHPALETTE())

#define GetHpen()               ((WXHPEN)GetResourceHandle())
#define GetHpenOf(pen)          ((WXHPEN)(pen).GetResourceHandle())

#endif // wxUSE_GUI

// ---------------------------------------------------------------------------
// global functions
// ---------------------------------------------------------------------------

// return the full path of the given module
inline std::string wxGetFullModuleName(WXHMODULE hmod)
{
    std::wstring fullname;

    fullname.resize(MAX_PATH);

    if ( !::GetModuleFileNameW
            (
                hmod,
                &fullname[0],
                MAX_PATH
            ) )
    {
        wxLogLastError("GetModuleFileName");
    }

    return boost::nowide::narrow(fullname);
}

// return the full path of the program file
inline std::string wxGetFullModuleName()
{
    return wxGetFullModuleName((WXHMODULE)wxGetInstance());
}

// return the run-time version of the OS in a format similar to
// WINVER/_WIN32_WINNT compile-time macros:
//
//      0x0501      Windows XP, 2003
//      0x0502      Windows XP SP2, 2003 SP1
//      0x0600      Windows Vista, 2008
//      0x0601      Windows 7
//      0x0602      Windows 8 (currently also returned for 8.1 if program does not have a manifest indicating 8.1 support)
//      0x0603      Windows 8.1 (currently only returned for 8.1 if program has a manifest indicating 8.1 support)
//      0x1000      Windows 10 (currently only returned for 10 if program has a manifest indicating 10 support)
//
// for the other Windows versions wxWinVersion_Unknown is currently returned.
enum wxWinVersion
{
    wxWinVersion_3 = 0x0300,
    wxWinVersion_NT3 = wxWinVersion_3,

    wxWinVersion_4 = 0x0400,
    wxWinVersion_95 = wxWinVersion_4,
    wxWinVersion_NT4 = wxWinVersion_4,
    wxWinVersion_98 = 0x0410,

    wxWinVersion_5 = 0x0500,
    wxWinVersion_ME = wxWinVersion_5,
    wxWinVersion_NT5 = wxWinVersion_5,
    wxWinVersion_2000 = wxWinVersion_5,
    wxWinVersion_XP = 0x0501,
    wxWinVersion_2003 = 0x0501,
    wxWinVersion_XP_SP2 = 0x0502,
    wxWinVersion_2003_SP1 = 0x0502,

    wxWinVersion_6 = 0x0600,
    wxWinVersion_Vista = wxWinVersion_6,
    wxWinVersion_NT6 = wxWinVersion_6,

    wxWinVersion_7 = 0x601,

    wxWinVersion_8 = 0x602,
    wxWinVersion_8_1 = 0x603,

    wxWinVersion_10 = 0x1000,

    // Any version we can't recognize will be later than the last currently
    // known one, so give it a value greater than any in the known range.
    wxWinVersion_Unknown = 0x7fff
};

wxWinVersion wxGetWinVersion();

#if wxUSE_LOG
// This is similar to wxSysErrorMsgStr(), but takes an extra WXHMODULE parameter
// specific to wxMSW.
std::string wxMSWFormatMessage(WXDWORD nErrCode, WXHMODULE hModule = nullptr);

#endif

#if wxUSE_GUI && defined(__WXMSW__)

// cursor stuff
extern WXHCURSOR wxGetCurrentBusyCursor();    // from msw/utils.cpp
extern const wxCursor *wxGetGlobalCursor(); // from msw/cursor.cpp

// GetCursorPos can fail without populating the POINT. This falls back to GetMessagePos.
void wxGetCursorPosMSW(POINT* pt);

wxSize wxGetCharSize(WXHWND wnd, const wxFont& the_font);
wxFontEncoding wxGetFontEncFromCharSet(int charset);

inline void wxSetWindowFont(WXHWND hwnd, const wxFont& font)
{
    ::SendMessageW(hwnd, WM_SETFONT,
                  (WXWPARAM)GetHfontOf(font), MAKELPARAM(TRUE, 0));
}

void wxSliderEvent(WXHWND control, WXWORD wParam, WXWORD pos);
void wxScrollBarEvent(WXHWND hbar, WXWORD wParam, WXWORD pos);

// Safely get the window text (i.e. without using fixed size buffer)
extern std::string wxGetWindowText(WXHWND hWnd);

// get the window class name
extern wxString wxGetWindowClass(WXHWND hWnd);

// get the window id (should be unsigned, hence this is not wxWindowID which
// is, for mainly historical reasons, signed)
extern int wxGetWindowId(WXHWND hWnd);

// check if hWnd's WNDPROC is wndProc. Return true if yes, false if they are
// different
//
// wndProc parameter is unused and only kept for compatibility
extern
bool wxCheckWindowWndProc(WXHWND hWnd, WXWNDPROC wndProc = nullptr);

// Does this window style specify any border?
constexpr bool wxStyleHasBorder(unsigned int style)
{
    return (style & (wxSIMPLE_BORDER | wxRAISED_BORDER |
                     wxSUNKEN_BORDER | wxDOUBLE_BORDER)) != 0;
}

inline bool wxHasWindowExStyle(const wxWindowMSW *win, unsigned int style)
{
    return (::GetWindowLongPtrW(GetHwndOf(win), GWL_EXSTYLE) & style) != 0;
}

// Common helper of wxUpdate{,Edit}LayoutDirection() below: sets or clears the
// given flag(s) depending on wxLayoutDirection and returns true if the flags
// really changed.
inline bool
wxUpdateExStyleForLayoutDirection(WXHWND hWnd,
                                  wxLayoutDirection dir,
                                  LONG_PTR flagsForRTL)
{
    wxCHECK_MSG( hWnd, false,
                 "Can't set layout direction for invalid window" );

    const LONG_PTR styleOld = ::GetWindowLongPtrW(hWnd, GWL_EXSTYLE);

    LONG_PTR styleNew = styleOld;
    switch ( dir )
    {
        case wxLayoutDirection::LeftToRight:
            styleNew &= ~flagsForRTL;
            break;

        case wxLayoutDirection::RightToLeft:
            styleNew |= flagsForRTL;
            break;

        case wxLayoutDirection::Default:
            wxFAIL_MSG("Invalid layout direction");
    }

    if ( styleNew == styleOld )
        return false;

    ::SetWindowLongPtrW(hWnd, GWL_EXSTYLE, styleNew);

    return true;
}

// Update layout direction flag for a generic window.
//
// See below for the special version that must be used with EDIT controls.
//
// Returns true if the layout direction did change.
inline bool wxUpdateLayoutDirection(WXHWND hWnd, wxLayoutDirection dir)
{
    return wxUpdateExStyleForLayoutDirection(hWnd, dir, WS_EX_LAYOUTRTL);
}

// Update layout direction flag for an EDIT control.
//
// Returns true if anything changed or false if the direction flag was already
// set to the desired direction (which can't be wxLayoutDirection::Default).
inline bool wxUpdateEditLayoutDirection(WXHWND hWnd, wxLayoutDirection dir)
{
    return wxUpdateExStyleForLayoutDirection(hWnd, dir,
                                             WS_EX_RIGHT |
                                             WS_EX_RTLREADING |
                                             WS_EX_LEFTSCROLLBAR);
}

// Companion of the above function checking if an EDIT control uses RTL.
inline wxLayoutDirection wxGetEditLayoutDirection(WXHWND hWnd)
{
    wxCHECK_MSG( hWnd, wxLayoutDirection::Default, "invalid window" );

    // While we set 3 style bits above, we're only really interested in one of
    // them here. In particularly, don't check for WS_EX_RIGHT as it can be set
    // for a right-aligned control even if it doesn't use RTL. And while we
    // could test WS_EX_LEFTSCROLLBAR, this doesn't really seem useful.
    const LONG_PTR style = ::GetWindowLongPtrW(hWnd, GWL_EXSTYLE);

    return style & WS_EX_RTLREADING ? wxLayoutDirection::RightToLeft
                                    : wxLayoutDirection::LeftToRight;
}

// ----------------------------------------------------------------------------
// functions mapping WXHWND to wxWindow
// ----------------------------------------------------------------------------

// this function simply checks whether the given hwnd corresponds to a wxWindow
// and returns either that window if it does or NULL otherwise
extern wxWindow* wxFindWinFromHandle(WXHWND hwnd);

// find the window for WXHWND which is part of some wxWindow, i.e. unlike
// wxFindWinFromHandle() above it will also work for "sub controls" of a
// wxWindow.
//
// returns the wxWindow corresponding to the given WXHWND or NULL.
extern wxWindow *wxGetWindowFromHWND(WXHWND hwnd);

// Get the size of an icon
extern wxSize wxGetHiconSize(WXHICON hicon);

void wxDrawLine(WXHDC hdc, int x1, int y1, int x2, int y2);

void wxDrawHVLine(WXHDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int width);

// fill the client rect of the given window on the provided dc using this brush
inline void wxFillRect(WXHWND hwnd, WXHDC hdc, WXHBRUSH hbr)
{
    RECT rc;
    ::GetClientRect(hwnd, &rc);
    ::FillRect(hdc, &rc, hbr);
}

// ----------------------------------------------------------------------------
// 32/64 bit helpers
// ----------------------------------------------------------------------------

// note that the casts to LONG_PTR here are required even on 32-bit machines
// for the 64-bit warning mode of later versions of MSVC (C4311/4312)
inline WNDPROC wxGetWindowProc(WXHWND hwnd)
{
    return (WNDPROC)(LONG_PTR)::GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
}

inline void *wxGetWindowUserData(WXHWND hwnd)
{
    return (void *)(LONG_PTR)::GetWindowLongPtrW(hwnd, GWLP_USERDATA);
}

inline WNDPROC wxSetWindowProc(WXHWND hwnd, WNDPROC func)
{
    return (WNDPROC)(LONG_PTR)::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)func);
}

inline void *wxSetWindowUserData(WXHWND hwnd, void *data)
{
    return (void *)(LONG_PTR)::SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)data);
}

#endif // wxUSE_GUI && __WXMSW__

#endif // _WX_PRIVATE_H_
