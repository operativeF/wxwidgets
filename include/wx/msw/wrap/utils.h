
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

        void operator()(HANDLE h) { ::CloseHandle(h); }
    };

    struct WndWindowDeleter
    {
        using pointer = HWND;

        void operator()(HWND h) { ::DestroyWindow(h); }
    };

    template<typename GDIObjT>
    struct WndGDIObjDeleter
    {
        using pointer = GDIObjT;

        void operator()(GDIObjT h) { ::DeleteObject(h); }
    };

    struct WndConsoleDeleter
    {
        using pointer = HANDLE;
        
        // TODO: Mark variable unused?
        void operator()(HANDLE h) { ::FreeConsole(); }
    };

    struct WndMenuDeleter
    {
        using pointer = HMENU;

        void operator()(HMENU h) { ::DestroyMenu(h); }
    };

} // namespace detail

using unique_handle  = std::unique_ptr<HANDLE, detail::WndHandleDeleter>;
using unique_window  = std::unique_ptr<HWND, detail::WndWindowDeleter>;
using unique_console = std::unique_ptr<HANDLE, detail::WndConsoleDeleter>;
using unique_menu    = std::unique_ptr<HMENU, detail::WndMenuDeleter>;

template<typename GDIObjT>
using unique_gdiobj  = std::unique_ptr<GDIObjT, detail::WndGDIObjDeleter<GDIObjT>>;

using unique_palettte = unique_gdiobj<HPALETTE>;
using unique_brush    = unique_gdiobj<HBRUSH>;
using unique_bitmap   = unique_gdiobj<HBITMAP>;
using unique_font     = unique_gdiobj<HFONT>;
using unique_hpen     = unique_gdiobj<HPEN>;

}

#endif // _MSW_WRAPUTILS_H