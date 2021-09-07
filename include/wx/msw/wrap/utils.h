
#ifndef _MSW_UTILS_H
#define _MSW_UTILS_H

#include <memory>

#include <consoleapi.h>
#include <handleapi.h>
#include <windef.h>
#include <WinUser.h>
#include <wingdi.h>

namespace msw::utils
{

namespace detail
{
    // TODO: Use concepts
    // Must satisfy NullablePtr.
    // Must satisfy construction from other handle types.
    struct WndHandleDeleter
    {
        using pointer = HANDLE;

        void operator()(HANDLE h) noexcept { ::CloseHandle(h); }
    };

    struct WndWindowDeleter
    {
        using pointer = HWND;

        void operator()(HWND h) noexcept { ::DestroyWindow(h); }
    };

    template<typename GDIObjT>
    struct WndGDIObjDeleter
    {
        using pointer = GDIObjT;

        void operator()(GDIObjT h) noexcept { ::DeleteObject(h); }
    };

    struct WndConsoleDeleter
    {
        using pointer = HANDLE;
        
        // TODO: Mark variable unused?
        void operator()() noexcept { ::FreeConsole(); }
    };

    struct WndMenuDeleter
    {
        using pointer = HMENU;

        void operator()(HMENU h) noexcept { ::DestroyMenu(h); }
    };

    struct WndCursorDeleter
    {
        using pointer = HCURSOR;

        void operator()(HCURSOR h) noexcept { ::DestroyCursor(h); }
    };

    struct WndDCDeleter
    {
        using pointer = HDC;

        void operator()(HDC h) noexcept { ::DeleteDC(h); }
    };

    struct WndGLRCDeleter
    {
        using pointer = HGLRC;

        void operator()(HGLRC h) noexcept { ::wglDeleteContext(h); }
    };

    struct WndAccelDeleter
    {
        using pointer = HACCEL;

        void operator()(HACCEL h) noexcept { ::DestroyAcceleratorTable(h); }
    };

    struct WndIconDeleter
    {
        using pointer = HICON;

        void operator()(HICON h) noexcept { ::DestroyIcon(h); }
    };

    struct WndEnhMetafileDeleter
    {
        using pointer = HENHMETAFILE;

        void operator()(HENHMETAFILE h) noexcept { ::DeleteEnhMetaFile(h); }
    };

    struct WndHdcWndDeleter
    {
        using pointer = HDC;

        void operator()(HDC hDC) noexcept { ::ReleaseDC(nullptr, hDC); };
    };

    struct WndHdcPaintDeleter
    {
        using pointer = HDC;

        void operator()(HWND hWnd, const PAINTSTRUCT* lpPaint) noexcept { ::EndPaint(hWnd, lpPaint); };
    };

} // namespace detail

using unique_handle      = std::unique_ptr<HANDLE, detail::WndHandleDeleter>;
using unique_wnd         = std::unique_ptr<HWND, detail::WndWindowDeleter>;
using unique_console     = std::unique_ptr<HANDLE, detail::WndConsoleDeleter>;
using unique_menu        = std::unique_ptr<HMENU, detail::WndMenuDeleter>;
using unique_cursor      = std::unique_ptr<HCURSOR, detail::WndCursorDeleter>;
using unique_dc          = std::unique_ptr<HDC, detail::WndDCDeleter>;
using unique_glrc        = std::unique_ptr<HGLRC, detail::WndGLRCDeleter>;
using unique_accel       = std::unique_ptr<HACCEL, detail::WndAccelDeleter>;
using unique_icon        = std::unique_ptr<HICON, detail::WndIconDeleter>;
using unique_enhmetafile = std::unique_ptr<HENHMETAFILE, detail::WndEnhMetafileDeleter>;
using unique_dcwnd       = std::unique_ptr<HDC, detail::WndHdcWndDeleter>;
using unique_dcpaint     = std::unique_ptr<HDC, detail::WndHdcPaintDeleter>;

template<typename GDIObjT>
using unique_gdiobj  = std::unique_ptr<GDIObjT, detail::WndGDIObjDeleter<GDIObjT>>;

using unique_palette  = unique_gdiobj<HPALETTE>;
using unique_brush    = unique_gdiobj<HBRUSH>;
using unique_bitmap   = unique_gdiobj<HBITMAP>;
using unique_font     = unique_gdiobj<HFONT>;
using unique_pen      = unique_gdiobj<HPEN>;
using unique_region   = unique_gdiobj<HRGN>;

}

#endif // _MSW_WRAPUTILS_H
