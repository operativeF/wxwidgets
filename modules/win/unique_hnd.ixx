module;

#include <windows.h>

#include <consoleapi.h>
#include <handleapi.h>
#include <WinUser.h>
#include <wingdi.h>
#include <Uxtheme.h>

#include <memory>

export module WX.Win.UniqueHnd;

import WX.WinDef;

namespace msw::utils
{

// TODO: Use concepts
// Must satisfy NullablePtr.
// Must satisfy construction from other handle types.
struct WndHandleDeleter
{
    using pointer = WXHANDLE;

    void operator()(WXHANDLE h) noexcept { ::CloseHandle(h); }
};

struct WndWindowDeleter
{
    using pointer = WXHWND;

    void operator()(WXHWND h) noexcept { ::DestroyWindow(h); }
};

template<typename GDIObjT>
struct WndGDIObjDeleter
{
    using pointer = GDIObjT;

    void operator()(GDIObjT h) noexcept { ::DeleteObject(h); }
};

struct WndConsoleDeleter
{
    using pointer = WXHANDLE;
    
    // TODO: Mark variable unused?
    void operator()() noexcept { ::FreeConsole(); }
};

struct WndMenuDeleter
{
    using pointer = WXHMENU;

    void operator()(WXHMENU h) noexcept { ::DestroyMenu(h); }
};

struct WndCursorDeleter
{
    using pointer = WXHCURSOR;

    void operator()(WXHCURSOR h) noexcept { ::DestroyCursor(h); }
};

struct WndDCDeleter
{
    using pointer = WXHDC;

    void operator()(HDC h) noexcept { ::DeleteDC(h); }
};

struct WndGLRCDeleter
{
    using pointer = WXHGLRC;

    void operator()(WXHGLRC h) noexcept { ::wglDeleteContext(h); }
};

struct WndAccelDeleter
{
    using pointer = WXHACCEL;

    void operator()(WXHACCEL h) noexcept { ::DestroyAcceleratorTable(h); }
};

struct WndIconDeleter
{
    using pointer = WXHICON;

    void operator()(WXHICON h) noexcept { ::DestroyIcon(h); }
};

struct WndEnhMetafileDeleter
{
    using pointer = WXHENHMETAFILE;

    void operator()(WXHENHMETAFILE h) noexcept { ::DeleteEnhMetaFile(h); }
};

struct WndHdcWndDeleter
{
    using pointer = WXHDC;

    void operator()(WXHDC hDC) noexcept { ::ReleaseDC(nullptr, hDC); };
};

struct WndHdcPaintDeleter
{
    using pointer = WXHDC;

    void operator()(WXHWND hWnd, const PAINTSTRUCT* lpPaint) noexcept { ::EndPaint(hWnd, lpPaint); };
};

struct WndThemeDeleter
{
    using pointer = WXHTHEME;

    void operator()(WXHTHEME h) noexcept { ::CloseThemeData(h); }
};

export
{

using unique_handle      = std::unique_ptr<WXHANDLE, WndHandleDeleter>;
using unique_wnd         = std::unique_ptr<WXHWND, WndWindowDeleter>;
using unique_console     = std::unique_ptr<WXHANDLE, WndConsoleDeleter>;
using unique_menu        = std::unique_ptr<WXHMENU, WndMenuDeleter>;
using unique_cursor      = std::unique_ptr<WXHCURSOR, WndCursorDeleter>;
using unique_dc          = std::unique_ptr<WXHDC, WndDCDeleter>;
using unique_glrc        = std::unique_ptr<WXHGLRC, WndGLRCDeleter>;
using unique_accel       = std::unique_ptr<WXHACCEL, WndAccelDeleter>;
using unique_icon        = std::unique_ptr<WXHICON, WndIconDeleter>;
using unique_enhmetafile = std::unique_ptr<WXHENHMETAFILE, WndEnhMetafileDeleter>;
using unique_dcwnd       = std::unique_ptr<WXHDC, WndHdcWndDeleter>;
using unique_dcpaint     = std::unique_ptr<WXHDC, WndHdcPaintDeleter>;
using unique_theme       = std::unique_ptr<WXHTHEME, WndThemeDeleter>;

template<typename GDIObjT>
using unique_gdiobj  = std::unique_ptr<GDIObjT, WndGDIObjDeleter<GDIObjT>>;

using unique_palette  = unique_gdiobj<WXHPALETTE>;
using unique_brush    = unique_gdiobj<WXHBRUSH>;
using unique_bitmap   = unique_gdiobj<WXHBITMAP>;
using unique_font     = unique_gdiobj<WXHFONT>;
using unique_pen      = unique_gdiobj<WXHPEN>;
using unique_region   = unique_gdiobj<WXHRGN>;

} // export

} // namespace msw::utils
