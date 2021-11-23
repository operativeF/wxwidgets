#include <windows.h>

#include "wx/msw/wrap/wrapgdi.h"

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

#include <WinUser.h>

namespace msw::wrap
{

void winInvalidateRect(WXHWND hWnd, RECT* rect, bool eraseBgToo)
{
    ::InvalidateRect(hWnd, rect, static_cast<BOOL>(eraseBgToo));
}

void winSetWindowPos(WXHWND hWnd, WXHWND hWndInsertAfter, wxPoint pos, wxSize sz, UINT uFlags)
{
    ::SetWindowPos(hWnd, hWndInsertAfter, pos.x, pos.y, sz.x, sz.y, uFlags);
}

WXHWND winCreateWindow(long exStyle,
                     const std::string& className,
                     const std::string& wndClass,
                     unsigned int style,
                     wxRect wndExtents,
                     WXHWND hParent,
                     WXHMENU hMenu,
                     WXHINSTANCE hInstance,
                     void* lpParam)
{
    boost::nowide::wstackstring stackClassName{className.c_str()};
    boost::nowide::wstackstring stackWndClass{wndClass.c_str()};

    return ::CreateWindowExW(exStyle,
                             stackClassName.get(),
                             stackWndClass.get(),
                             style,
                             wndExtents.x,
                             wndExtents.y,
                             wndExtents.width,
                             wndExtents.height,
                             hParent,
                             hMenu,
                             hInstance,
                             lpParam);
}

} // namespace msw::wrap