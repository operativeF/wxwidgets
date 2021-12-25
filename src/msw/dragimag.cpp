/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/dragimag.cpp
// Purpose:     wxDragImage
// Author:      Julian Smart
// Modified by:
// Created:     08/04/99
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_DRAGIMAGE

#include "wx/msw/wrapcctl.h" // include <commctrl.h> "properly"
#include "wx/msw/private.h"
#include "wx/dcmemory.h"
#include "wx/dcscreen.h"
#include "wx/frame.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/window.h"
#include "wx/msw/dragimag.h"

import WX.Image;
import WX.WinDef;
import WX.Utils.Settings;

import <string>;

// Wine doesn't have this yet
#ifndef ListView_CreateDragImage
#define ListView_CreateDragImage(hwnd, i, lpptUpLeft) \
    (HIMAGELIST)SNDMSG((hwnd), LVM_CREATEDRAGIMAGE, (WXWPARAM)(int)(i), (WXLPARAM)(LPPOINT)(lpptUpLeft))
#endif

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

#define GetHimageList() ((HIMAGELIST) m_hImageList)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxDragImage dtor
// ----------------------------------------------------------------------------

wxDragImage::~wxDragImage()
{
    if ( m_hImageList )
        ImageList_Destroy(GetHimageList());
#if !wxUSE_SIMPLER_DRAGIMAGE
    if ( m_hCursorImageList )
        ImageList_Destroy((HIMAGELIST) m_hCursorImageList);
#endif
}

// Attributes
////////////////////////////////////////////////////////////////////////////


// Operations
////////////////////////////////////////////////////////////////////////////

// Create a drag image from a bitmap and optional cursor
bool wxDragImage::Create(const wxBitmap& image, const wxCursor& cursor)
{
    if ( m_hImageList )
        ImageList_Destroy(GetHimageList());
    m_hImageList = nullptr;

    WXUINT flags wxDUMMY_INITIALIZE(0) ;
    if (image.GetDepth() <= 4)
        flags = ILC_COLOR4;
    else if (image.GetDepth() <= 8)
        flags = ILC_COLOR8;
    else if (image.GetDepth() <= 16)
        flags = ILC_COLOR16;
    else if (image.GetDepth() <= 24)
        flags = ILC_COLOR24;
    else
        flags = ILC_COLOR32;

    bool mask = (image.GetMask() != nullptr);

    // Curiously, even if the image doesn't have a mask,
    // we still have to use ILC_MASK or the image won't show
    // up when dragged.
//    if ( mask )
    flags |= ILC_MASK;

    m_hImageList = (WXHIMAGELIST) ImageList_Create(image.GetWidth(), image.GetHeight(), flags, 1, 1);

    int index;
    if (!mask)
    {
        WXHBITMAP hBitmap1 = (WXHBITMAP) image.GetHBITMAP();
        index = ImageList_Add(GetHimageList(), hBitmap1, nullptr);
    }
    else
    {
        WXHBITMAP hBitmap1 = (WXHBITMAP) image.GetHBITMAP();
        WXHBITMAP hBitmap2 = (WXHBITMAP) image.GetMask()->GetMaskBitmap();
        WXHBITMAP hbmpMask = wxInvertMask(hBitmap2);

        index = ImageList_Add(GetHimageList(), hBitmap1, hbmpMask);
        ::DeleteObject(hbmpMask);
    }
    if ( index == -1 )
    {
        wxLogError(_("Couldn't add an image to the image list."));
    }
    m_cursor = cursor; // Can only combine with drag image after calling BeginDrag.

    return (index != -1) ;
}

// Create a drag image from an icon and optional cursor
bool wxDragImage::Create(const wxIcon& image, const wxCursor& cursor)
{
    if ( m_hImageList )
        ImageList_Destroy(GetHimageList());
    m_hImageList = nullptr;

    WXUINT flags wxDUMMY_INITIALIZE(0) ;
    if (image.GetDepth() <= 4)
        flags = ILC_COLOR4;
    else if (image.GetDepth() <= 8)
        flags = ILC_COLOR8;
    else if (image.GetDepth() <= 16)
        flags = ILC_COLOR16;
    else if (image.GetDepth() <= 24)
        flags = ILC_COLOR24;
    else
        flags = ILC_COLOR32;

    flags |= ILC_MASK;

    m_hImageList = (WXHIMAGELIST) ImageList_Create(image.GetWidth(), image.GetHeight(), flags, 1, 1);

    WXHICON hIcon = (WXHICON) image.GetHICON();

    int index = ImageList_AddIcon(GetHimageList(), hIcon);
    if ( index == -1 )
    {
        wxLogError(_("Couldn't add an image to the image list."));
    }

    m_cursor = cursor; // Can only combine with drag image after calling BeginDrag.

    return (index != -1) ;
}

// Create a drag image from a string and optional cursor
bool wxDragImage::Create(const std::string& str, const wxCursor& cursor)
{
    wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

    wxScreenDC dc;
    dc.SetFont(font);
    auto textExtents = dc.GetTextExtent(str);
    dc.SetFont(wxNullFont);

    wxMemoryDC dc2;
    dc2.SetFont(font);
    wxBitmap bitmap(wxSize{(int) textExtents.x + 2, (int) textExtents.y + 2});
    dc2.SelectObject(bitmap);

    dc2.SetBackground(* wxWHITE_BRUSH);
    dc2.Clear();
    dc2.SetBackgroundMode(wxBrushStyle::Transparent);
    dc2.SetTextForeground(* wxLIGHT_GREY);
    dc2.wxDrawText(str, wxPoint{0, 0});
    dc2.wxDrawText(str, wxPoint{1, 0});
    dc2.wxDrawText(str, wxPoint{2, 0});
    dc2.wxDrawText(str, wxPoint{1, 1});
    dc2.wxDrawText(str, wxPoint{2, 1});
    dc2.wxDrawText(str, wxPoint{1, 2});
    dc2.wxDrawText(str, wxPoint{2, 2});
    dc2.SetTextForeground(* wxBLACK);
    dc2.wxDrawText(str, wxPoint{1, 1});

    dc2.SelectObject(wxNullBitmap);

#if wxUSE_WXDIB
    // Make the bitmap masked
    wxImage image = bitmap.ConvertToImage();
    image.SetMaskColour(255, 255, 255);
    return Create(wxBitmap(image), cursor);
#else
    return false;
#endif
}

#if wxUSE_TREECTRL
// Create a drag image for the given tree control item
bool wxDragImage::Create(const wxTreeCtrl& treeCtrl, wxTreeItemId& id)
{
    if ( m_hImageList )
        ImageList_Destroy(GetHimageList());
    m_hImageList = (WXHIMAGELIST)
        TreeView_CreateDragImage(GetHwndOf(&treeCtrl), (HTREEITEM) id.m_pItem);
    if ( !m_hImageList )
    {
        // fall back on just the item text if there is no image
        return Create(treeCtrl.GetItemText(id));
    }

    return true;
}
#endif

#if wxUSE_LISTCTRL
// Create a drag image for the given list control item
bool wxDragImage::Create(const wxListCtrl& listCtrl, long id)
{
    if ( m_hImageList )
        ImageList_Destroy(GetHimageList());
    POINT pt
    {
        .x = 0,
        .y = 0
    };

    m_hImageList = (WXHIMAGELIST)
        ListView_CreateDragImage(GetHwndOf(&listCtrl), id, &pt);

    if ( !m_hImageList )
    {
        // as for wxTreeCtrl, fall back on dragging just the item text
        return Create(listCtrl.GetItemText(id));
    }

    return true;
}
#endif

// Begin drag
bool wxDragImage::BeginDrag(const wxPoint& hotspot, wxWindow* window, bool fullScreen, wxRect* rect)
{
    wxASSERT_MSG( (m_hImageList != nullptr), "Image list must not be null in BeginDrag.");
    wxASSERT_MSG( (window != nullptr), "Window must not be null in BeginDrag.");

    m_fullScreen = fullScreen;
    if (rect)
        m_boundingRect = * rect;

    bool ret = (ImageList_BeginDrag(GetHimageList(), 0, hotspot.x, hotspot.y) != 0);

    if (!ret)
    {
        wxFAIL_MSG( "BeginDrag failed." );

        return false;
    }

    if (m_cursor.IsOk())
    {
#if wxUSE_SIMPLER_DRAGIMAGE
        m_oldCursor = window->GetCursor();
        window->SetCursor(m_cursor);
#else
        if (!m_hCursorImageList)
        {
            int cxCursor = wxGetSystemMetrics(SM_CXCURSOR, window);
            int cyCursor = wxGetSystemMetrics(SM_CYCURSOR, window);
            m_hCursorImageList = (WXHIMAGELIST) ImageList_Create(cxCursor, cyCursor, ILC_MASK, 1, 1);
        }

        // See if we can find the cursor hotspot
        wxPoint curHotSpot(hotspot);

        // Although it seems to produce the right position, when the hotspot goeos
        // negative it has strange effects on the image.
        // How do we stop the cursor jumping right and below of where it should be?
#if 0
        ICONINFO iconInfo;
        if (::GetIconInfo((WXHICON) (WXHCURSOR) m_cursor.GetHCURSOR(), & iconInfo) != 0)
        {
            curHotSpot.x -= iconInfo.xHotspot;
            curHotSpot.y -= iconInfo.yHotspot;
        }
#endif
        //wxString msg;
        //msg.Printf("Hotspot = %d, %d", curHotSpot.x, curHotSpot.y);
        //wxLogDebug(msg);

        // First add the cursor to the image list
        WXHCURSOR hCursor = (WXHCURSOR) m_cursor.GetHCURSOR();
        int cursorIndex = ImageList_AddIcon((HIMAGELIST) m_hCursorImageList, (WXHICON) hCursor);

        wxASSERT_MSG( (cursorIndex != -1), "ImageList_AddIcon failed in BeginDrag.");

        if (cursorIndex != -1)
        {
            ImageList_SetDragCursorImage((HIMAGELIST) m_hCursorImageList, cursorIndex, curHotSpot.x, curHotSpot.y);
        }
#endif
    }

#if !wxUSE_SIMPLER_DRAGIMAGE
    if (m_cursor.IsOk())
        ::ShowCursor(FALSE);
#endif

    m_window = window;

    ::SetCapture(GetHwndOf(window));

    return true;
}

// Begin drag. hotspot is the location of the drag position relative to the upper-left
// corner of the image. This is full screen only. fullScreenRect gives the
// position of the window on the screen, to restrict the drag to.
bool wxDragImage::BeginDrag(const wxPoint& hotspot, wxWindow* window, wxWindow* fullScreenRect)
{
    wxRect rect;

    int x = fullScreenRect->GetPosition().x;
    int y = fullScreenRect->GetPosition().y;

    wxSize sz = fullScreenRect->GetSize();

    if (fullScreenRect->GetParent() && !dynamic_cast<wxFrame*>(fullScreenRect))
        fullScreenRect->GetParent()->ClientToScreen(& x, & y);

    rect.x = x; rect.y = y;
    rect.width = sz.x; rect.height = sz.y;

    return BeginDrag(hotspot, window, true, & rect);
}

// End drag
bool wxDragImage::EndDrag()
{
    wxASSERT_MSG( (m_hImageList != nullptr), "Image list must not be null in EndDrag.");

    ImageList_EndDrag();

    if ( !::ReleaseCapture() )
    {
        wxLogLastError("ReleaseCapture");
    }

#if wxUSE_SIMPLER_DRAGIMAGE
    if (m_cursor.IsOk() && m_oldCursor.IsOk())
        m_window->SetCursor(m_oldCursor);
#else
    ::ShowCursor(TRUE);
#endif

    m_window = nullptr;

    return true;
}

// Move the image: call from OnMouseMove. Pt is in window client coordinates if window
// is non-NULL, or in screen coordinates if NULL.
bool wxDragImage::Move(const wxPoint& pt)
{
    wxASSERT_MSG( (m_hImageList != nullptr), "Image list must not be null in Move.");

    // These are in window, not client coordinates.
    // So need to convert to client coordinates.
    wxPoint pt2(pt);
    if (m_window && !m_fullScreen)
    {
        RECT rect
        {
            .left = 0,
            .top = 0,
            .right = 0,
            .bottom = 0
        };

        WXDWORD style = ::GetWindowLongPtrW((WXHWND) m_window->GetHWND(), GWL_STYLE);
        WXDWORD exStyle = ::GetWindowLongPtrW((WXHWND) m_window->GetHWND(), GWL_EXSTYLE);
        ::AdjustWindowRectEx(& rect, style, FALSE, exStyle);
        // Subtract the (negative) values, i.e. add a small increment
        pt2.x -= rect.left; pt2.y -= rect.top;
    }
    else if (m_window && m_fullScreen)
    {
        pt2 = m_window->ClientToScreen(pt2);
    }

    const bool ret = (ImageList_DragMove( pt2.x, pt2.y ) != 0);

    m_position = pt2;

    return ret;
}

bool wxDragImage::Show()
{
    wxASSERT_MSG( (m_hImageList != nullptr), "Image list must not be null in Show.");

    WXHWND hWnd = nullptr;
    if (m_window && !m_fullScreen)
        hWnd = (WXHWND) m_window->GetHWND();

    bool ret = (ImageList_DragEnter( hWnd, m_position.x, m_position.y ) != 0);

    return ret;
}

bool wxDragImage::Hide()
{
    wxASSERT_MSG( (m_hImageList != nullptr), "Image list must not be null in Hide.");

    WXHWND hWnd = nullptr;
    if (m_window && !m_fullScreen)
        hWnd = (WXHWND) m_window->GetHWND();

    return (ImageList_DragLeave(hWnd) != 0);
}

#endif // wxUSE_DRAGIMAGE
