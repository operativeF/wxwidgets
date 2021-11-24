module;

#include <windows.h>

#include <CommCtrl.h>

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

using WXHIMAGELIST   = HIMAGELIST;
using WXHGLOBAL      = HGLOBAL;

using WXDPI_AWARENESS_CONTEXT = DPI_AWARENESS_CONTEXT;

using WXWidget    = WXHWND;

using WXLPARAM    = LPARAM;
using WXWPARAM    = WPARAM;
using WXLRESULT   = LRESULT;

using WXUINT      = UINT;
using WXWORD      = WORD;

using WXDRAWITEMSTRUCT    = void*;
using WXMEASUREITEMSTRUCT = void*;
using WXLPCREATESTRUCT    = void*;

typedef WXLRESULT (_stdcall *WXWNDPROC)(WXHWND, WXUINT, WXWPARAM, WXLPARAM);

} // export