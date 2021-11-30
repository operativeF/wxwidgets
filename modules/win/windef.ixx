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
using WXHRESULT   = HRESULT;

using WXBOOL      = BOOL;
using WXUINT      = UINT;
using WXWORD      = WORD;
using WXDWORD     = DWORD;

using WXDWORD_PTR = DWORD_PTR;

using WXDRAWITEMSTRUCT    = void*;
using WXMEASUREITEMSTRUCT = void*;
using WXLPCREATESTRUCT    = void*;

using WXTASKDIALOG_BUTTON = TASKDIALOG_BUTTON;
using WXTASKDIALOGCONFIG  = TASKDIALOGCONFIG;

typedef WXLRESULT (_stdcall *WXWNDPROC)(WXHWND, WXUINT, WXWPARAM, WXLPARAM);

} // export