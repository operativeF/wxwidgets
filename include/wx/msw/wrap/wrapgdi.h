
#ifndef _MSW_WRAPGDI_H
#define _MSW_WRAPGDI_H

#include <wx/geometry/size.h>
#include <wx/position.h>

#include <string>

#ifndef _WINDEF_
class HWND__;
using HWND = HWND__*;

class HMENU__;
using HMENU = HMENU__*;

class HINSTANCE__;
using HINSTANCE = HINSTANCE__*;

struct RECT;

#endif

namespace msw::wrap
{
static void winInvalidateRect(HWND hWnd, RECT* rect, bool eraseBgToo);

static void winSetWindowPos(HWND hWnd, HWND hWndInsertAfter, wxPosition pos, wxSize sz, UINT uFlags);

static HWND winCreateWindow(long exStyle,
                            const std::string& className,
                            const std::string& wndClass,
                            long style,
                            wxRect wndExtents,
                            HWND hParent,
                            HMENU hMenu,
                            HINSTANCE hInstance,
                            void* lpParam);
} // namespace msw::wrap

#endif // _MSW_WRAPGDI_H
