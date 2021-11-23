module;

#include <windows.h>

export module WX.WinDef;

export
{

using WXHACCEL       = HACCEL;
using WXHBITMAP      = HBITMAP;
using WXHBRUSH       = HBRUSH;
using WXHCURSOR      = HCURSOR;
using WXHANDLE       = void*;
using WXHDC          = HDC;
using WXHENHMETAFILE = HENHMETAFILE;
using WXHFONT        = HFONT;
using WXHGLRC        = HGLRC;
using WXHICON        = HICON;
using WXHINSTANCE    = HINSTANCE;
using WXHMENU        = HMENU;
using WXHMODULE      = WXHINSTANCE;
using WXHPALETTE     = HPALETTE;
using WXHPEN         = HPEN;
using WXHRGN         = HRGN;
using WXHWND         = HWND;

using WXWidget    = WXHWND;

using WXLPARAM    = LPARAM;
using WXWPARAM    = WPARAM;
using WXLRESULT   = LRESULT;

typedef WXLRESULT (_stdcall *WXWNDPROC)(WXHWND, UINT, WXWPARAM, WXLPARAM);

} // export