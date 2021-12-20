///////////////////////////////////////////////////////////////////////////////
// Name:        src/aui/book.cpp
// Purpose:     wxaui: wx advanced user interface - notebook
// Author:      Benjamin I. Williams
// Modified by: Jens Lody
// Created:     2006-06-28
// Copyright:   (C) Copyright 2006, Kirix Corporation, All Rights Reserved
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/aui/events.h"

#include "wx/app.h"
#include "wx/bookctrl.h"
#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/frame.h"
#include "wx/window.h"

#ifdef __WXMAC__
#include "wx/osx/private.h"
#endif

#include <memory>

module WX.AUI.Book;

import WX.AUI.Flags;
import WX.AUI.FrameManager;
import WX.AUI.Page;
import WX.AUI.TabArt;
import WX.AUI.TabMDI;

import WX.Utils.Settings;

import <numeric>;
import <ranges>;
import <utility>;

wxIMPLEMENT_CLASS(wxAuiNotebook, wxControl);
wxIMPLEMENT_CLASS(wxAuiTabCtrl, wxControl);
wxIMPLEMENT_DYNAMIC_CLASS(wxAuiNotebookEvent, wxBookCtrlEvent);

// -- wxAuiTabContainer class implementation --


// wxAuiTabContainer is a class which contains information about each
// tab.  It also can render an entire tab control to a specified DC.
// It's not a window class itself, because this code will be used by
// the wxFrameMananger, where it is disadvantageous to have separate
// windows for each tab control in the case of "docked tabs"

// A derived class, wxAuiTabCtrl, is an actual wxWindow-derived window
// which can be used as a tab control in the normal sense.


wxAuiTabContainer::wxAuiTabContainer()
    : m_art{std::make_unique<wxAuiDefaultTabArt>()}
{
    AddButton(wxAUI_BUTTON_LEFT, wxLEFT);
    AddButton(wxAUI_BUTTON_RIGHT, wxRIGHT);
    AddButton(wxAUI_BUTTON_WINDOWLIST, wxRIGHT);
    AddButton(wxAUI_BUTTON_CLOSE, wxRIGHT);
}

void wxAuiTabContainer::SetArtProvider(std::unique_ptr<wxAuiTabArt> art)
{
    m_art = std::move(art);

    if (m_art)
    {
        m_art->SetFlags(m_flags);
    }
}

wxAuiTabArt* wxAuiTabContainer::GetArtProvider() const
{
    return m_art.get();
}

void wxAuiTabContainer::SetFlags(unsigned int flags)
{
    m_flags = flags;

    // check for new close button settings
    RemoveButton(wxAUI_BUTTON_LEFT);
    RemoveButton(wxAUI_BUTTON_RIGHT);
    RemoveButton(wxAUI_BUTTON_WINDOWLIST);
    RemoveButton(wxAUI_BUTTON_CLOSE);

    if (flags & wxAUI_NB_SCROLL_BUTTONS)
    {
        AddButton(wxAUI_BUTTON_LEFT, wxLEFT);
        AddButton(wxAUI_BUTTON_RIGHT, wxRIGHT);
    }

    if (flags & wxAUI_NB_WINDOWLIST_BUTTON)
    {
        AddButton(wxAUI_BUTTON_WINDOWLIST, wxRIGHT);
    }

    if (flags & wxAUI_NB_CLOSE_BUTTON)
    {
        AddButton(wxAUI_BUTTON_CLOSE, wxRIGHT);
    }

    if (m_art)
    {
        m_art->SetFlags(m_flags);
    }
}

unsigned int wxAuiTabContainer::GetFlags() const
{
    return m_flags;
}


void wxAuiTabContainer::SetNormalFont(const wxFont& font)
{
    m_art->SetNormalFont(font);
}

void wxAuiTabContainer::SetSelectedFont(const wxFont& font)
{
    m_art->SetSelectedFont(font);
}

void wxAuiTabContainer::SetMeasuringFont(const wxFont& font)
{
    m_art->SetMeasuringFont(font);
}

void wxAuiTabContainer::SetColour(const wxColour& colour)
{
    m_art->SetColour(colour);
}

void wxAuiTabContainer::SetActiveColour(const wxColour& colour)
{
    m_art->SetActiveColour(colour);
}

void wxAuiTabContainer::SetRect(const wxRect& rect)
{
    m_rect = rect;

    if (m_art)
    {
        m_art->SetSizingInfo(rect.GetSize(), m_pages.size());
    }
}

bool wxAuiTabContainer::AddPage(wxWindow* page,
                                const wxAuiNotebookPage& info)
{
    wxAuiNotebookPage page_info;
    page_info = info;
    page_info.window = page;
    page_info.hover = false;

    m_pages.push_back(page_info);

    // let the art provider know how many pages we have
    if (m_art)
    {
        m_art->SetSizingInfo(m_rect.GetSize(), m_pages.size());
    }

    return true;
}

bool wxAuiTabContainer::InsertPage(wxWindow* page,
                                   const wxAuiNotebookPage& info,
                                   size_t idx)
{
    wxAuiNotebookPage page_info;
    page_info = info;
    page_info.window = page;
    page_info.hover = false;

    if (idx >= m_pages.size())
        m_pages.push_back(page_info);
    else
        m_pages.insert(m_pages.begin() + idx, page_info);

    // let the art provider know how many pages we have
    if (m_art)
    {
        m_art->SetSizingInfo(m_rect.GetSize(), m_pages.size());
    }

    return true;
}

bool wxAuiTabContainer::MovePage(wxWindow* page,
                                 size_t new_idx)
{
    int idx = GetIdxFromWindow(page);
    if (idx == -1)
        return false;

    // get page entry, make a copy of it
    wxAuiNotebookPage p = GetPage(idx);

    // remove old page entry
    RemovePage(page);

    // insert page where it should be
    InsertPage(page, p, new_idx);

    return true;
}

bool wxAuiTabContainer::RemovePage(wxWindow* wnd)
{
    for (auto page = m_pages.begin(); page != m_pages.end(); ++page)
    {
        if (page->window == wnd)
        {
            m_pages.erase(page);

            // let the art provider know how many pages we have
            if (m_art)
            {
                m_art->SetSizingInfo(m_rect.GetSize(), m_pages.size());
            }

            return true;
        }
    }

    return false;
}

bool wxAuiTabContainer::SetActivePage(wxWindow* wnd)
{
    bool found = false;

    for(auto& page : m_pages)
    {
        if (page.window == wnd)
        {
            page.active = true;
            found = true;
        }
        else
        {
            page.active = false;
        }
    }

    return found;
}

void wxAuiTabContainer::SetNoneActive()
{
    std::ranges::for_each(m_pages, [](auto& page){ page.active = false; });
}

bool wxAuiTabContainer::SetActivePage(size_t page)
{
    if (page >= m_pages.size())
        return false;

    return SetActivePage(m_pages[page].window);
}

int wxAuiTabContainer::GetActivePage() const
{
    for (int i{}; const auto& page : m_pages)
    {
        if (page.active)
            return i;

        ++i;
    }

    return wxNOT_FOUND;
}

wxWindow* wxAuiTabContainer::GetWindowFromIdx(size_t idx) const
{
    if (idx >= m_pages.size())
        return nullptr;

    return m_pages[idx].window;
}

int wxAuiTabContainer::GetIdxFromWindow(wxWindow* wnd) const
{
    for(int idx{}; const auto& page : m_pages)
    {
        if (page.window == wnd)
            return idx;
        
        ++idx;
    }

    return wxNOT_FOUND;
}

wxAuiNotebookPage& wxAuiTabContainer::GetPage(size_t idx)
{
    wxASSERT_MSG(idx < m_pages.size(), "Invalid Page index");

    return m_pages[idx];
}

const wxAuiNotebookPage& wxAuiTabContainer::GetPage(size_t idx) const
{
    wxASSERT_MSG(idx < m_pages.size(), "Invalid Page index");

    return m_pages[idx];
}

wxAuiNotebookPageArray& wxAuiTabContainer::GetPages()
{
    return m_pages;
}

size_t wxAuiTabContainer::GetPageCount() const
{
    return m_pages.size();
}

void wxAuiTabContainer::AddButton(int id,
                                  int location,
                                  const wxBitmap& normalBitmap,
                                  const wxBitmap& disabledBitmap)
{
    wxAuiTabContainerButton button
    {
        .bitmap = normalBitmap,
        .disBitmap = disabledBitmap,
        .curState = wxAUI_BUTTON_STATE_NORMAL,
        .id = id,
        .location = location
    };

    m_buttons.push_back(button);
}

void wxAuiTabContainer::RemoveButton(int id)
{
    size_t i, button_count = m_buttons.size();

    for (i = 0; i < button_count; ++i)
    {
        if (m_buttons[i].id == id)
        {
            m_buttons.erase(m_buttons.begin() + i);
            return;
        }
    }
}



size_t wxAuiTabContainer::GetTabOffset() const
{
    return m_tabOffset;
}

void wxAuiTabContainer::SetTabOffset(size_t offset)
{
    m_tabOffset = offset;
}




// Render() renders the tab catalog to the specified DC
// It is a virtual function and can be overridden to
// provide custom drawing capabilities
void wxAuiTabContainer::Render(wxDC* raw_dc, wxWindow* wnd)
{
    if (!raw_dc || !raw_dc->IsOk())
        return;

    if (m_rect.IsEmpty())
        return;

    wxMemoryDC dc;

    // use the same layout direction as the window DC uses to ensure that the
    // text is rendered correctly
    dc.SetLayoutDirection(raw_dc->GetLayoutDirection());

    wxBitmap bmp;
    size_t i;
    size_t page_count = m_pages.size();
    size_t button_count = m_buttons.size();

    // create off-screen bitmap
    bmp.Create(m_rect.GetSize(),*raw_dc);
    dc.SelectObject(bmp);

    if (!dc.IsOk())
        return;

    // ensure we show as many tabs as possible
    while (m_tabOffset > 0 && IsTabVisible(page_count-1, m_tabOffset-1, &dc, wnd))
        --m_tabOffset;

    // find out if size of tabs is larger than can be
    // afforded on screen
    int total_width = 0;
    int visible_width = 0;
    for (int i{}; auto& page : m_pages)
    {
        // determine if a close button is on this tab
        bool close_button = false;
        if ((m_flags & wxAUI_NB_CLOSE_ON_ALL_TABS) != 0 ||
            ((m_flags & wxAUI_NB_CLOSE_ON_ACTIVE_TAB) != 0 && page.active))
        {
            close_button = true;
        }

        int x_extent = 0;
        wxSize size = m_art->GetTabSize(dc,
                            wnd,
                            page.caption,
                            page.bitmap,
                            page.active,
                            close_button ?
                              wxAUI_BUTTON_STATE_NORMAL :
                              wxAUI_BUTTON_STATE_HIDDEN,
                            &x_extent);

        if (i+1 < m_pages.size())
            total_width += x_extent;
        else
            total_width += size.x;

        if (i >= m_tabOffset)
        {
            if (i+1 < m_pages.size())
                visible_width += x_extent;
            else
                visible_width += size.x;
        }
    }

    if (total_width > m_rect.GetWidth() || m_tabOffset != 0)
    {
        // show left/right buttons
        for (auto& button : m_buttons)
        {
            if (button.id == wxAUI_BUTTON_LEFT ||
                button.id == wxAUI_BUTTON_RIGHT)
            {
                button.curState &= ~wxAUI_BUTTON_STATE_HIDDEN;
            }
        }
    }
    else
    {
        // hide left/right buttons
        for (auto& button : m_buttons)
        {
            if (button.id == wxAUI_BUTTON_LEFT ||
                button.id == wxAUI_BUTTON_RIGHT)
            {
                button.curState |= wxAUI_BUTTON_STATE_HIDDEN;
            }
        }
    }

    // determine whether left button should be enabled
    for (auto& button : m_buttons)
    {
        if (button.id == wxAUI_BUTTON_LEFT)
        {
            if (m_tabOffset == 0)
                button.curState |= wxAUI_BUTTON_STATE_DISABLED;
            else
                button.curState &= ~wxAUI_BUTTON_STATE_DISABLED;
        }

        if (button.id == wxAUI_BUTTON_RIGHT)
        {
            int button_width = 0;
            for (i = 0; i != button_count; ++i)
                button_width += m_buttons[button_count - i - 1].rect.GetWidth();

            if (visible_width < m_rect.GetWidth() - button_width)
                button.curState |= wxAUI_BUTTON_STATE_DISABLED;
            else
                button.curState &= ~wxAUI_BUTTON_STATE_DISABLED;
        }
    }



    // draw background
    m_art->DrawBackground(dc, wnd, m_rect);

    // draw buttons
    int left_buttons_width = 0;
    int right_buttons_width = 0;

    // draw the buttons on the right side
    int offset = m_rect.x + m_rect.width;
    for (auto& button : std::ranges::reverse_view(m_buttons))
    {
        if (button.location != wxRIGHT)
            continue;
        if (button.curState & wxAUI_BUTTON_STATE_HIDDEN)
            continue;

        wxRect button_rect = m_rect;
        button_rect.SetY(1);
        button_rect.SetWidth(offset);

        m_art->DrawButton(dc,
                          wnd,
                          button_rect,
                          button.id,
                          button.curState,
                          wxRIGHT,
                          &button.rect);

        offset -= button.rect.GetWidth();
        right_buttons_width += button.rect.GetWidth();
    }



    offset = 0;

    // draw the buttons on the left side

    for(auto& button : std::ranges::reverse_view(m_buttons))
    {
        if (button.location != wxLEFT)
            continue;
        if (button.curState & wxAUI_BUTTON_STATE_HIDDEN)
            continue;

        wxRect button_rect(offset, 1, 1000, m_rect.height);

        m_art->DrawButton(dc,
                          wnd,
                          button_rect,
                          button.id,
                          button.curState,
                          wxLEFT,
                          &button.rect);

        offset += button.rect.GetWidth();
        left_buttons_width += button.rect.GetWidth();
    }

    offset = left_buttons_width;

    if (offset == 0)
        offset += m_art->GetIndentSize();


    // prepare the tab-close-button array
    // make sure tab button entries which aren't used are marked as hidden
    for (i = page_count; i < m_tabCloseButtons.size(); ++i)
        m_tabCloseButtons[i].curState = wxAUI_BUTTON_STATE_HIDDEN;

    // make sure there are enough tab button entries to accommodate all tabs
    while (m_tabCloseButtons.size() < page_count)
    {
        wxAuiTabContainerButton tempbtn;
        tempbtn.id = wxAUI_BUTTON_CLOSE;
        tempbtn.location = wxCENTER;
        tempbtn.curState = wxAUI_BUTTON_STATE_HIDDEN;
        m_tabCloseButtons.push_back(tempbtn);
    }


    // buttons before the tab offset must be set to hidden
    for (i = 0; i < m_tabOffset; ++i)
    {
        m_tabCloseButtons[i].curState = wxAUI_BUTTON_STATE_HIDDEN;
    }


    // draw the tabs

    size_t active = 999;
    int active_offset = 0;
    wxRect active_rect;

    int x_extent = 0;
    wxRect rect = m_rect;
    rect.y = 0;
    rect.height = m_rect.height;

    for (i = m_tabOffset; i < page_count; ++i)
    {
        wxAuiNotebookPage& page = m_pages[i];
        wxAuiTabContainerButton& tab_button = m_tabCloseButtons[i];

        // determine if a close button is on this tab
        if ((m_flags & wxAUI_NB_CLOSE_ON_ALL_TABS) != 0 ||
            ((m_flags & wxAUI_NB_CLOSE_ON_ACTIVE_TAB) != 0 && page.active))
        {
            if (tab_button.curState == wxAUI_BUTTON_STATE_HIDDEN)
            {
                tab_button.id = wxAUI_BUTTON_CLOSE;
                tab_button.curState = wxAUI_BUTTON_STATE_NORMAL;
                tab_button.location = wxCENTER;
            }
        }
        else
        {
            tab_button.curState = wxAUI_BUTTON_STATE_HIDDEN;
        }

        rect.x = offset;
        rect.width = m_rect.width - right_buttons_width - offset - wnd->FromDIP(2);

        if (rect.width <= 0)
            break;

        m_art->DrawTab(dc,
                       wnd,
                       page,
                       rect,
                       tab_button.curState,
                       &page.rect,
                       &tab_button.rect,
                       &x_extent);

        if (page.active)
        {
            active = i;
            active_offset = offset;
            active_rect = rect;
        }

        offset += x_extent;
    }


    // make sure to deactivate buttons which are off the screen to the right
    for (++i; i < m_tabCloseButtons.size(); ++i)
    {
        m_tabCloseButtons[i].curState = wxAUI_BUTTON_STATE_HIDDEN;
    }


    // draw the active tab again so it stands in the foreground
    if (active >= m_tabOffset && active < m_pages.size())
    {
        wxAuiNotebookPage& page = m_pages[active];

        wxAuiTabContainerButton& tab_button = m_tabCloseButtons[active];

        rect.x = active_offset;
        m_art->DrawTab(dc,
                       wnd,
                       page,
                       active_rect,
                       tab_button.curState,
                       &page.rect,
                       &tab_button.rect,
                       &x_extent);
    }


    raw_dc->Blit(m_rect.GetPosition(),
                 m_rect.GetSize(),
                 &dc, wxPoint{0, 0});
}

// Is the tab visible?
bool wxAuiTabContainer::IsTabVisible(int tabPage, int tabOffset, wxDC* dc, wxWindow* wnd)
{
    if (!dc || !dc->IsOk())
        return false;

    size_t i;
    size_t page_count = m_pages.size();
    size_t button_count = m_buttons.size();

    // Hasn't been rendered yet; assume it's visible
    if (m_tabCloseButtons.size() < page_count)
        return true;

    // First check if both buttons are disabled - if so, there's no need to
    // check further for visibility.
    int arrowButtonVisibleCount = 0;
    for (auto& button : m_buttons)
    {
        if (button.id == wxAUI_BUTTON_LEFT ||
            button.id == wxAUI_BUTTON_RIGHT)
        {
            if ((button.curState & wxAUI_BUTTON_STATE_HIDDEN) == 0)
                ++arrowButtonVisibleCount;
        }
    }

    // Tab must be visible
    if (arrowButtonVisibleCount == 0)
        return true;

    // If tab is less than the given offset, it must be invisible by definition
    if (tabPage < tabOffset)
        return false;

    // draw buttons
    int left_buttons_width = 0;
    int right_buttons_width = 0;

    // calculate size of the buttons on the right side
    int offset = m_rect.x + m_rect.width;
    for (auto& button : std::ranges::reverse_view(m_buttons))
    {
        if (button.location != wxRIGHT)
            continue;
        if (button.curState & wxAUI_BUTTON_STATE_HIDDEN)
            continue;

        offset -= button.rect.GetWidth();
        right_buttons_width += button.rect.GetWidth();
    }

    offset = 0;

    // calculate size of the buttons on the left side
    for (auto& button : std::ranges::reverse_view(m_buttons))
    {
        if (button.location != wxLEFT)
            continue;
        if (button.curState & wxAUI_BUTTON_STATE_HIDDEN)
            continue;

        offset += button.rect.GetWidth();
        left_buttons_width += button.rect.GetWidth();
    }

    offset = left_buttons_width;

    if (offset == 0)
        offset += m_art->GetIndentSize();

    wxRect rect = m_rect;

    // See if the given page is visible at the given tab offset (effectively scroll position)
    for (i = tabOffset; i < page_count; ++i)
    {
        wxAuiNotebookPage& page = m_pages[i];
        wxAuiTabContainerButton& tab_button = m_tabCloseButtons[i];

        rect.width = m_rect.width - right_buttons_width - offset - wnd->FromDIP(2);

        if (rect.width <= 0)
            return false; // haven't found the tab, and we've run out of space, so return false

        int x_extent = 0;
        m_art->GetTabSize(*dc,
                            wnd,
                            page.caption,
                            page.bitmap,
                            page.active,
                            tab_button.curState,
                            &x_extent);

        offset += x_extent;

        if (i == (size_t) tabPage)
        {
            // If not all of the tab is visible, and supposing there's space to display it all,
            // we could do better so we return false.
            return !(((m_rect.width - right_buttons_width - offset - wnd->FromDIP(2)) <= 0) && ((m_rect.width - right_buttons_width - left_buttons_width) > x_extent));
        }
    }

    // Shouldn't really get here, but if it does, assume the tab is visible to prevent
    // further looping in calling code.
    return true;
}

// Make the tab visible if it wasn't already
void wxAuiTabContainer::MakeTabVisible(int tabPage, wxWindow* win)
{
    wxClientDC dc(win);
    if (!IsTabVisible(tabPage, GetTabOffset(), & dc, win))
    {
        int i;
        for (i = 0; i < (int) m_pages.size(); i++)
        {
            if (IsTabVisible(tabPage, i, & dc, win))
            {
                SetTabOffset(i);
                win->Refresh();
                return;
            }
        }
    }
}

// TabHitTest() tests if a tab was hit, passing the window pointer
// back if that condition was fulfilled.  The function returns
// true if a tab was hit, otherwise false
bool wxAuiTabContainer::TabHitTest(int x, int y, wxWindow** hit)
{
    if (!m_rect.Contains(x,y))
        return false;

    wxAuiTabContainerButton* btn = nullptr;
    if (ButtonHitTest(x, y, &btn) && !(btn->curState & wxAUI_BUTTON_STATE_DISABLED))
    {
        if(std::ranges::find_if(m_buttons, [btn](auto otherBtn){ return btn == &otherBtn; }) != m_buttons.end())
            return false;
    }

    for (const auto& page : std::ranges::drop_view(m_pages, m_tabOffset))
    {
        if (page.rect.Contains(x,y))
        {
            if (hit)
                *hit = page.window;
            return true;
        }
    }

    return false;
}

// ButtonHitTest() tests if a button was hit. The function returns
// true if a button was hit, otherwise false
bool wxAuiTabContainer::ButtonHitTest(int x, int y,
                                      wxAuiTabContainerButton** hit)
{
    if (!m_rect.Contains(x,y))
        return false;

    for (auto& button : m_buttons)
    {
        if (button.rect.Contains(x,y) &&
            !(button.curState & wxAUI_BUTTON_STATE_HIDDEN ))
        {
            if (hit)
                *hit = &button;
            return true;
        }
    }

    for (auto& button : m_tabCloseButtons)
    {
        if (button.rect.Contains(x,y) &&
            !(button.curState & (wxAUI_BUTTON_STATE_HIDDEN |
                                   wxAUI_BUTTON_STATE_DISABLED)))
        {
            if (hit)
                *hit = &button;
            return true;
        }
    }

    return false;
}

namespace
{

// the utility function ShowWnd() is the same as show,
// except it handles wxAuiMDIChildFrame windows as well,
// as the Show() method on this class is "unplugged"
void ShowWnd(wxWindow* wnd, bool show)
{
#if wxUSE_MDI
    if (wxDynamicCast(wnd, wxAuiMDIChildFrame))
    {
        wxAuiMDIChildFrame* cf = (wxAuiMDIChildFrame*)wnd;
        cf->wxWindow::Show(show);
    }
    else
#endif
    {
        wnd->Show(show);
    }
}

} // namespace anonymous


// DoShowHide() this function shows the active window, then
// hides all of the other windows (in that order)
void wxAuiTabContainer::DoShowHide()
{
    wxAuiNotebookPageArray& pages = GetPages();

    // show new active page first
    for (auto& page : pages)
    {
        if (page.active)
        {
            ShowWnd(page.window, true);
            break;
        }
    }

    // hide all other pages
    for (auto& page : pages)
    {
        if (!page.active)
            ShowWnd(page.window, false);
    }
}

// -- wxAuiTabCtrl class implementation --
wxBEGIN_EVENT_TABLE(wxAuiTabCtrl, wxControl)
    EVT_PAINT(wxAuiTabCtrl::OnPaint)
    EVT_ERASE_BACKGROUND(wxAuiTabCtrl::OnEraseBackground)
    EVT_SIZE(wxAuiTabCtrl::OnSize)
    EVT_LEFT_DOWN(wxAuiTabCtrl::OnLeftDown)
    EVT_LEFT_DCLICK(wxAuiTabCtrl::OnLeftDClick)
    EVT_LEFT_UP(wxAuiTabCtrl::OnLeftUp)
    EVT_MIDDLE_DOWN(wxAuiTabCtrl::OnMiddleDown)
    EVT_MIDDLE_UP(wxAuiTabCtrl::OnMiddleUp)
    EVT_RIGHT_DOWN(wxAuiTabCtrl::OnRightDown)
    EVT_RIGHT_UP(wxAuiTabCtrl::OnRightUp)
    EVT_MOTION(wxAuiTabCtrl::OnMotion)
    EVT_LEAVE_WINDOW(wxAuiTabCtrl::OnLeaveWindow)
    EVT_AUINOTEBOOK_BUTTON(wxID_ANY, wxAuiTabCtrl::OnButton)
    EVT_SET_FOCUS(wxAuiTabCtrl::OnSetFocus)
    EVT_KILL_FOCUS(wxAuiTabCtrl::OnKillFocus)
    EVT_CHAR(wxAuiTabCtrl::OnChar)
    EVT_MOUSE_CAPTURE_LOST(wxAuiTabCtrl::OnCaptureLost)
    EVT_SYS_COLOUR_CHANGED(wxAuiTabCtrl::OnSysColourChanged)
wxEND_EVENT_TABLE()


wxAuiTabCtrl::wxAuiTabCtrl(wxWindow* parent,
                           wxWindowID id,
                           const wxPoint& pos,
                           const wxSize& size,
                           unsigned int style) : wxControl(parent, id, pos, size, style)
{
    SetName("wxAuiTabCtrl");
    m_clickPt = wxDefaultPosition;
    m_isDragging = false;
    m_hoverButton = nullptr;
    m_pressedButton = nullptr;
}

void wxAuiTabCtrl::OnPaint(wxPaintEvent&)
{
    wxPaintDC dc(this);

    dc.SetFont(GetFont());

    if (GetPageCount() > 0)
        Render(&dc, this);
}

void wxAuiTabCtrl::OnSysColourChanged(wxSysColourChangedEvent &event)
{
    event.Skip();

    if (m_art)
    {
        m_art->UpdateColoursFromSystem();
    }
}

void wxAuiTabCtrl::OnEraseBackground([[maybe_unused]] wxEraseEvent& evt)
{
}

void wxAuiTabCtrl::OnSize(wxSizeEvent& evt)
{
    wxSize s = evt.GetSize();
    wxRect r(0, 0, s.x, s.y);
    SetRect(r);
}

void wxAuiTabCtrl::OnLeftDown(wxMouseEvent& evt)
{
    CaptureMouse();
    m_clickPt = wxDefaultPosition;
    m_isDragging = false;
    m_clickTab = nullptr;
    m_pressedButton = nullptr;


    wxWindow* wnd;
    if (TabHitTest(evt.m_x, evt.m_y, &wnd))
    {
        int new_selection = GetIdxFromWindow(wnd);

        // wxAuiNotebooks always want to receive this event
        // even if the tab is already active, because they may
        // have multiple tab controls
        if ((new_selection != GetActivePage() ||
            wxDynamicCast(GetParent(), wxAuiNotebook)) && !m_hoverButton)
        {
            wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_PAGE_CHANGING, m_windowId);
            e.SetSelection(new_selection);
            e.SetOldSelection(GetActivePage());
            e.SetEventObject(this);
            GetEventHandler()->ProcessEvent(e);
        }

        m_clickPt.x = evt.m_x;
        m_clickPt.y = evt.m_y;
        m_clickTab = wnd;
    }

    if (m_hoverButton)
    {
        m_pressedButton = m_hoverButton;
        m_pressedButton->curState = wxAUI_BUTTON_STATE_PRESSED;
        Refresh();
        Update();
    }
}

void wxAuiTabCtrl::OnCaptureLost([[maybe_unused]] wxMouseCaptureLostEvent& event)
{
    if (m_isDragging)
    {
        m_isDragging = false;

        wxAuiNotebookEvent evt(wxEVT_AUINOTEBOOK_CANCEL_DRAG, m_windowId);
        evt.SetSelection(GetIdxFromWindow(m_clickTab));
        evt.SetOldSelection(evt.GetSelection());
        evt.SetEventObject(this);
        GetEventHandler()->ProcessEvent(evt);
    }
}

void wxAuiTabCtrl::OnLeftUp(wxMouseEvent& evt)
{
    if (GetCapture() == this)
        ReleaseMouse();

    if (m_isDragging)
    {
        m_isDragging = false;

        wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_END_DRAG, m_windowId);
        e.SetSelection(GetIdxFromWindow(m_clickTab));
        e.SetOldSelection(e.GetSelection());
        e.SetEventObject(this);
        GetEventHandler()->ProcessEvent(e);

        return;
    }

    if (m_pressedButton)
    {
        // make sure we're still clicking the button
        wxAuiTabContainerButton* button = nullptr;
        if (!ButtonHitTest(evt.m_x, evt.m_y, &button) ||
            button->curState & wxAUI_BUTTON_STATE_DISABLED)
            return;

        if (button != m_pressedButton)
        {
            m_pressedButton = nullptr;
            return;
        }

        Refresh();
        Update();

        if (!(m_pressedButton->curState & wxAUI_BUTTON_STATE_DISABLED))
        {
            wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_BUTTON, m_windowId);
            e.SetSelection(GetIdxFromWindow(m_clickTab));
            e.SetInt(m_pressedButton->id);
            e.SetEventObject(this);
            GetEventHandler()->ProcessEvent(e);
        }

        m_pressedButton = nullptr;
    }

    m_clickPt = wxDefaultPosition;
    m_isDragging = false;
    m_clickTab = nullptr;
}

void wxAuiTabCtrl::OnMiddleUp(wxMouseEvent& evt)
{
    wxWindow* wnd = nullptr;
    if (!TabHitTest(evt.m_x, evt.m_y, &wnd))
        return;

    wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_TAB_MIDDLE_UP, m_windowId);
    e.SetEventObject(this);
    e.SetSelection(GetIdxFromWindow(wnd));
    GetEventHandler()->ProcessEvent(e);
}

void wxAuiTabCtrl::OnMiddleDown(wxMouseEvent& evt)
{
    wxWindow* wnd = nullptr;
    if (!TabHitTest(evt.m_x, evt.m_y, &wnd))
        return;

    wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_TAB_MIDDLE_DOWN, m_windowId);
    e.SetEventObject(this);
    e.SetSelection(GetIdxFromWindow(wnd));
    GetEventHandler()->ProcessEvent(e);
}

void wxAuiTabCtrl::OnRightUp(wxMouseEvent& evt)
{
    wxWindow* wnd = nullptr;
    if (!TabHitTest(evt.m_x, evt.m_y, &wnd))
        return;

    wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_TAB_RIGHT_UP, m_windowId);
    e.SetEventObject(this);
    e.SetSelection(GetIdxFromWindow(wnd));
    GetEventHandler()->ProcessEvent(e);
}

void wxAuiTabCtrl::OnRightDown(wxMouseEvent& evt)
{
    wxWindow* wnd = nullptr;
    if (!TabHitTest(evt.m_x, evt.m_y, &wnd))
        return;

    wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN, m_windowId);
    e.SetEventObject(this);
    e.SetSelection(GetIdxFromWindow(wnd));
    GetEventHandler()->ProcessEvent(e);
}

void wxAuiTabCtrl::OnLeftDClick(wxMouseEvent& evt)
{
    wxWindow* wnd;
    wxAuiTabContainerButton* button;
    if (!TabHitTest(evt.m_x, evt.m_y, &wnd) && !ButtonHitTest(evt.m_x, evt.m_y, &button))
    {
        wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_BG_DCLICK, m_windowId);
        e.SetEventObject(this);
        GetEventHandler()->ProcessEvent(e);
    }
}

void wxAuiTabCtrl::OnMotion(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();

    // check if the mouse is hovering above a button
    wxAuiTabContainerButton* button;
    if (ButtonHitTest(pos.x, pos.y, &button) && !(button->curState & wxAUI_BUTTON_STATE_DISABLED))
    {
        if (m_hoverButton && button != m_hoverButton)
        {
            m_hoverButton->curState = wxAUI_BUTTON_STATE_NORMAL;
            m_hoverButton = nullptr;
            Refresh();
            Update();
        }

        if (button->curState != wxAUI_BUTTON_STATE_HOVER)
        {
            button->curState = wxAUI_BUTTON_STATE_HOVER;
            Refresh();
            Update();

            m_hoverButton = button;
            return;
        }
    }
    else
    {
        if (m_hoverButton)
        {
            m_hoverButton->curState = wxAUI_BUTTON_STATE_NORMAL;
            m_hoverButton = nullptr;
            Refresh();
            Update();
        }
    }

    wxWindow* wnd = nullptr;
    if (evt.Moving() && TabHitTest(evt.m_x, evt.m_y, &wnd))
    {
        SetHoverTab(wnd);

#if wxUSE_TOOLTIPS
        std::string tooltip(m_pages[GetIdxFromWindow(wnd)].tooltip);

        // If the text changes, set it else, keep old, to avoid
        // 'moving tooltip' effect
        if (GetToolTipText() != tooltip)
            SetToolTip(tooltip);
#endif // wxUSE_TOOLTIPS
    }
    else
    {
        SetHoverTab(nullptr);

#if wxUSE_TOOLTIPS
        UnsetToolTip();
#endif // wxUSE_TOOLTIPS
    }

    if (!evt.LeftIsDown() || m_clickPt == wxDefaultPosition)
        return;

    if (m_isDragging)
    {
        wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_DRAG_MOTION, m_windowId);
        e.SetSelection(GetIdxFromWindow(m_clickTab));
        e.SetOldSelection(e.GetSelection());
        e.SetEventObject(this);
        GetEventHandler()->ProcessEvent(e);
        return;
    }


    int drag_x_threshold = wxSystemSettings::GetMetric(wxSYS_DRAG_X, this);
    int drag_y_threshold = wxSystemSettings::GetMetric(wxSYS_DRAG_Y, this);

    if (std::abs(pos.x - m_clickPt.x) > drag_x_threshold ||
        std::abs(pos.y - m_clickPt.y) > drag_y_threshold)
    {
        wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_BEGIN_DRAG, m_windowId);
        e.SetSelection(GetIdxFromWindow(m_clickTab));
        e.SetOldSelection(e.GetSelection());
        e.SetEventObject(this);
        GetEventHandler()->ProcessEvent(e);

        m_isDragging = true;
    }
}

void wxAuiTabCtrl::OnLeaveWindow([[maybe_unused]] wxMouseEvent& event)
{
    if (m_hoverButton)
    {
        m_hoverButton->curState = wxAUI_BUTTON_STATE_NORMAL;
        m_hoverButton = nullptr;
        Refresh();
        Update();
    }

    SetHoverTab(nullptr);
}

void wxAuiTabCtrl::OnButton(wxAuiNotebookEvent& event)
{
    int button = event.GetInt();

    if (button == wxAUI_BUTTON_LEFT || button == wxAUI_BUTTON_RIGHT)
    {
        if (button == wxAUI_BUTTON_LEFT)
        {
            if (GetTabOffset() > 0)
            {
                SetTabOffset(GetTabOffset()-1);
                Refresh();
                Update();
            }
        }
        else
        {
            SetTabOffset(GetTabOffset()+1);
            Refresh();
            Update();
        }
    }
    else if (button == wxAUI_BUTTON_WINDOWLIST)
    {
        int idx = GetArtProvider()->ShowDropDown(this, m_pages, GetActivePage());

        if (idx != -1)
        {
            wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_PAGE_CHANGING, m_windowId);
            e.SetSelection(idx);
            e.SetOldSelection(GetActivePage());
            e.SetEventObject(this);
            GetEventHandler()->ProcessEvent(e);
        }
    }
    else
    {
        event.Skip();
    }
}

void wxAuiTabCtrl::OnSetFocus([[maybe_unused]] wxFocusEvent& event)
{
    Refresh();
}

void wxAuiTabCtrl::OnKillFocus([[maybe_unused]] wxFocusEvent& event)
{
    Refresh();
}

void wxAuiTabCtrl::OnChar(wxKeyEvent& event)
{
    if (GetActivePage() == -1)
    {
        event.Skip();
        return;
    }

    // We can't leave tab processing to the system; on Windows, tabs and keys
    // get eaten by the system and not processed properly if we specify both
    // wxTAB_TRAVERSAL and wxWANTS_CHARS. And if we specify just wxTAB_TRAVERSAL,
    // we don't key arrow key events.

    int key = event.GetKeyCode();

    if (key == WXK_NUMPAD_PAGEUP)
        key = WXK_PAGEUP;
    if (key == WXK_NUMPAD_PAGEDOWN)
        key = WXK_PAGEDOWN;
    if (key == WXK_NUMPAD_HOME)
        key = WXK_HOME;
    if (key == WXK_NUMPAD_END)
        key = WXK_END;
    if (key == WXK_NUMPAD_LEFT)
        key = WXK_LEFT;
    if (key == WXK_NUMPAD_RIGHT)
        key = WXK_RIGHT;

    if (key == WXK_TAB || key == WXK_PAGEUP || key == WXK_PAGEDOWN)
    {
        bool bCtrlDown = event.ControlDown();
        bool bShiftDown = event.ShiftDown();

        bool bForward = (key == WXK_TAB && !bShiftDown) || (key == WXK_PAGEDOWN);
        bool bWindowChange = (key == WXK_PAGEUP) || (key == WXK_PAGEDOWN) || bCtrlDown;
        bool bFromTab = (key == WXK_TAB);

        if (bFromTab && !bWindowChange)
        {
            // Handle ordinary tabs via Navigate. This is needed at least for wxGTK to tab properly.
            Navigate(bForward ? wxNavigationKeyEvent::IsForward : wxNavigationKeyEvent::IsBackward);
            return;
        }

        wxAuiNotebook* nb = wxDynamicCast(GetParent(), wxAuiNotebook);
        if (!nb)
        {
            event.Skip();
            return;
        }

        wxNavigationKeyEvent keyEvent;
        keyEvent.SetDirection(bForward);
        keyEvent.SetWindowChange(bWindowChange);
        keyEvent.SetFromTab(bFromTab);
        keyEvent.SetEventObject(nb);

        if (!nb->GetEventHandler()->ProcessEvent(keyEvent))
        {
            // Not processed? Do an explicit tab into the page.
            wxWindow* win = GetWindowFromIdx(GetActivePage());
            if (win)
                win->SetFocus();
        }
        return;
    }

    if (m_pages.size() < 2)
    {
        event.Skip();
        return;
    }

    int newPage = -1;

    int forwardKey, backwardKey;
    if (GetLayoutDirection() == wxLayoutDirection::RightToLeft)
    {
        forwardKey = WXK_LEFT;
        backwardKey = WXK_RIGHT;
    }
    else
     {
        forwardKey = WXK_RIGHT;
        backwardKey = WXK_LEFT;
    }

    if (key == forwardKey)
    {
        if (m_pages.size() > 1)
        {
            if (GetActivePage() == -1)
                newPage = 0;
            else if (GetActivePage() < (int) (m_pages.size() - 1))
                newPage = GetActivePage() + 1;
        }
    }
    else if (key == backwardKey)
    {
        if (m_pages.size() > 1)
        {
            if (GetActivePage() == -1)
                newPage = (int) (m_pages.size() - 1);
            else if (GetActivePage() > 0)
                newPage = GetActivePage() - 1;
        }
    }
    else if (key == WXK_HOME)
    {
        newPage = 0;
    }
    else if (key == WXK_END)
    {
        newPage = (int) (m_pages.size() - 1);
    }
    else
        event.Skip();

    if (newPage != -1)
    {
        wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_PAGE_CHANGING, m_windowId);
        e.SetSelection(newPage);
        e.SetOldSelection(newPage);
        e.SetEventObject(this);
        this->GetEventHandler()->ProcessEvent(e);
    }
    else
        event.Skip();
}

// wxTabFrame is an interesting case.  It's important that all child pages
// of the multi-notebook control are all actually children of that control
// (and not grandchildren).  wxTabFrame facilitates this.  There is one
// instance of wxTabFrame for each tab control inside the multi-notebook.
// It's important to know that wxTabFrame is not a real window, but it merely
// used to capture the dimensions/positioning of the internal tab control and
// it's managed page windows

namespace
{

class wxTabFrame : public wxWindow
{
public:

    wxTabFrame()
    {
        m_tabs = nullptr;
        m_rect = wxRect(wxPoint(0,0), FromDIP(wxSize(200,200)));
        m_tabCtrlHeight = FromDIP(20);
    }

    ~wxTabFrame()
    {
        wxDELETE(m_tabs);
    }

    void SetTabCtrlHeight(int h)
    {
        m_tabCtrlHeight = h;
    }

protected:
    void DoSetSize(wxRect boundary, [[maybe_unused]] unsigned int sizeFlags) override
    {
        m_rect = boundary;
        DoSizing();
    }

    wxSize DoGetClientSize() const override
    {
        return {m_rect.width, m_rect.height};
    }

public:
    bool Show( [[maybe_unused]] bool show ) override { return false; }

    void DoSizing()
    {
        if (!m_tabs)
            return;

        if (m_tabs->IsFrozen() || m_tabs->GetParent()->IsFrozen())
            return;

        m_tab_rect = wxRect(m_rect.x, m_rect.y, m_rect.width, m_tabCtrlHeight);
        if (m_tabs->GetFlags() & wxAUI_NB_BOTTOM)
        {
            m_tab_rect = wxRect (m_rect.x, m_rect.y + m_rect.height - m_tabCtrlHeight, m_rect.width, m_tabCtrlHeight);
            m_tabs->SetSize     (wxRect{m_rect.x, m_rect.y + m_rect.height - m_tabCtrlHeight, m_rect.width, m_tabCtrlHeight});
            m_tabs->SetRect     (wxRect{0, 0, m_rect.width, m_tabCtrlHeight});
        }
        else //TODO: if (GetFlags() & wxAUI_NB_TOP)
        {
            m_tab_rect = wxRect (m_rect.x, m_rect.y, m_rect.width, m_tabCtrlHeight);
            m_tabs->SetSize     (wxRect{m_rect.x, m_rect.y, m_rect.width, m_tabCtrlHeight});
            m_tabs->SetRect     (wxRect{0, 0,        m_rect.width, m_tabCtrlHeight});
        }
        // TODO: else if (GetFlags() & wxAUI_NB_LEFT){}
        // TODO: else if (GetFlags() & wxAUI_NB_RIGHT){}

        m_tabs->Refresh();
        m_tabs->Update();

        for (auto& page : m_tabs->GetPages())
        {
            int border_space = m_tabs->GetArtProvider()->GetAdditionalBorderSpace(page.window);

            int height = m_rect.height - m_tabCtrlHeight - border_space;
            if ( height < 0 )
            {
                // avoid passing negative height to wxWindow::SetSize(), this
                // results in assert failures/GTK+ warnings
                height = 0;
            }
            int width = m_rect.width - 2 * border_space;
            if (width < 0)
                width = 0;

            if (m_tabs->GetFlags() & wxAUI_NB_BOTTOM)
            {
                page.window->SetSize(wxRect{m_rect.x + border_space,
                                            m_rect.y + border_space,
                                            width,
                                            height});
            }
            else //TODO: if (GetFlags() & wxAUI_NB_TOP)
            {
                page.window->SetSize(wxRect{m_rect.x + border_space,
                                            m_rect.y + m_tabCtrlHeight,
                                            width,
                                            height});
            }
            // TODO: else if (GetFlags() & wxAUI_NB_LEFT){}
            // TODO: else if (GetFlags() & wxAUI_NB_RIGHT){}
        }
    }

protected:
    wxSize DoGetSize() const override
    {
        return {m_rect.GetWidth(), m_rect.GetHeight()};
    }

public:
    void Update() override
    {
        // does nothing
    }

    wxRect m_rect;
    wxRect m_tab_rect;
    wxAuiTabCtrl* m_tabs;
    int m_tabCtrlHeight;
};

} // namespace anonymous

// -- wxAuiNotebook class implementation --

#define EVT_AUI_RANGE(id1, id2, event, func) \
    wx__DECLARE_EVT2(event, id1, id2, wxAuiNotebookEventHandler(func))

wxBEGIN_EVENT_TABLE(wxAuiNotebook, wxControl)
    EVT_SIZE(wxAuiNotebook::OnSize)
    EVT_CHILD_FOCUS(wxAuiNotebook::OnChildFocusNotebook)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_PAGE_CHANGING,
                      wxAuiNotebook::OnTabClicked)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_BEGIN_DRAG,
                      wxAuiNotebook::OnTabBeginDrag)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_END_DRAG,
                      wxAuiNotebook::OnTabEndDrag)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_CANCEL_DRAG,
                      wxAuiNotebook::OnTabCancelDrag)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_DRAG_MOTION,
                      wxAuiNotebook::OnTabDragMotion)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_BUTTON,
                      wxAuiNotebook::OnTabButton)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_TAB_MIDDLE_DOWN,
                      wxAuiNotebook::OnTabMiddleDown)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_TAB_MIDDLE_UP,
                      wxAuiNotebook::OnTabMiddleUp)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN,
                      wxAuiNotebook::OnTabRightDown)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_TAB_RIGHT_UP,
                      wxAuiNotebook::OnTabRightUp)
    EVT_AUI_RANGE(wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId+500,
                      wxEVT_AUINOTEBOOK_BG_DCLICK,
                      wxAuiNotebook::OnTabBgDClick)
    EVT_NAVIGATION_KEY(wxAuiNotebook::OnNavigationKeyNotebook)
    EVT_SYS_COLOUR_CHANGED(wxAuiNotebook::OnSysColourChanged)
wxEND_EVENT_TABLE()

void wxAuiNotebook::OnSysColourChanged(wxSysColourChangedEvent &event)
{
    event.Skip(true);
    wxAuiTabArt* art = m_tabs.GetArtProvider();
    art->UpdateColoursFromSystem();

    for (auto& pane : m_mgr.GetAllPanes())
    {
        if (pane.name == "dummy")
            continue;

        wxTabFrame* tab_frame = (wxTabFrame*)pane.window;
        wxAuiTabCtrl* tabctrl = tab_frame->m_tabs;
        tabctrl->GetArtProvider()->UpdateColoursFromSystem();
        tabctrl->Refresh();
    }

    Refresh();
}

bool wxAuiNotebook::Create(wxWindow* parent,
                           wxWindowID id,
                           const wxPoint& pos,
                           const wxSize& size,
                           unsigned int style)
{
    if (!wxControl::Create(parent, id, pos, size, style))
        return false;

    InitNotebook(style);

    return true;
}

// InitNotebook() contains common initialization
// code called by all constructors
void wxAuiNotebook::InitNotebook(unsigned int style)
{
    SetName("wxAuiNotebook");
    m_curPage = -1;
    m_tabIdCounter = wxAuiBaseTabCtrlId;
    m_dummyWnd = nullptr;
    m_flags = (unsigned int)style;
    m_tabCtrlHeight = FromDIP(20);

    m_normalFont = *wxNORMAL_FONT;
    m_selectedFont = *wxNORMAL_FONT;
    m_selectedFont.SetWeight(wxFONTWEIGHT_BOLD);

    SetArtProvider(std::make_unique<wxAuiDefaultTabArt>());

    m_dummyWnd = new wxWindow(this, wxID_ANY, wxPoint(0,0), wxSize(0,0));
    m_dummyWnd->SetSize(FromDIP(wxSize(200, 200)));
    m_dummyWnd->Show(false);

    m_mgr.SetManagedWindow(this);
    m_mgr.SetFlags(wxAUI_MGR_DEFAULT);
    m_mgr.SetDockSizeConstraint(1.0, 1.0); // no dock size constraint

    m_mgr.AddPane(m_dummyWnd,
              wxAuiPaneInfo().Name("dummy").Bottom().CaptionVisible(false).Show(false));

    m_mgr.Update();
}

wxAuiNotebook::~wxAuiNotebook()
{
    // Indicate we're deleting pages
    SendDestroyEvent();

    while ( GetPageCount() > 0 )
        DeletePage(0);

    m_mgr.UnInit();
}

void wxAuiNotebook::SetArtProvider(std::unique_ptr<wxAuiTabArt> art)
{
    m_tabs.SetArtProvider(std::move(art));

    // Update the height and do nothing else if it did something but otherwise
    // (i.e. if the new art provider uses the same height as the old one) we
    // need to manually set the art provider for all tabs ourselves.
    if ( !UpdateTabCtrlHeight() )
    {
        for (auto& pane : m_mgr.GetAllPanes())
        {
            if (pane.name == "dummy")
                continue;

            auto* tab_frame = dynamic_cast<wxTabFrame*>(pane.window);
            wxAuiTabCtrl* tabctrl = tab_frame->m_tabs;
            tabctrl->SetArtProvider(art->Clone());
        }
    }
}

// SetTabCtrlHeight() is the highest-level override of the
// tab height.  A call to this function effectively enforces a
// specified tab ctrl height, overriding all other considerations,
// such as text or bitmap height.  It overrides any call to
// SetUniformBitmapSize().  Specifying a height of -1 reverts
// any previous call and returns to the default behaviour

void wxAuiNotebook::SetTabCtrlHeight(int height)
{
    m_requestedTabCtrlHeight = height;

    // if window is already initialized, recalculate the tab height
    if (m_dummyWnd)
    {
        UpdateTabCtrlHeight();
    }
}


// SetUniformBitmapSize() ensures that all tabs will have
// the same height, even if some tabs don't have bitmaps
// Passing wxDefaultSize to this function will instruct
// the control to use dynamic tab height-- so when a tab
// with a large bitmap is added, the tab ctrl's height will
// automatically increase to accommodate the bitmap

void wxAuiNotebook::SetUniformBitmapSize(const wxSize& size)
{
    m_requestedBmpSize = size;

    // if window is already initialized, recalculate the tab height
    if (m_dummyWnd)
    {
        UpdateTabCtrlHeight();
    }
}

// UpdateTabCtrlHeight() does the actual tab resizing. It's meant
// to be used internally
bool wxAuiNotebook::UpdateTabCtrlHeight()
{
    // get the tab ctrl height we will use
    int height = CalculateTabCtrlHeight();

    // if the tab control height needs to change, update
    // all of our tab controls with the new height
    if (m_tabCtrlHeight == height)
        return false;

    wxAuiTabArt* art = m_tabs.GetArtProvider();

    m_tabCtrlHeight = height;

    for (auto& pane : m_mgr.GetAllPanes())
    {
        if (pane.name == "dummy")
            continue;

        auto* tab_frame = dynamic_cast<wxTabFrame*>(pane.window);
        wxAuiTabCtrl* tabctrl = tab_frame->m_tabs;
        tab_frame->SetTabCtrlHeight(m_tabCtrlHeight);
        tabctrl->SetArtProvider(art->Clone());
        tab_frame->DoSizing();
    }

    return true;
}

void wxAuiNotebook::UpdateHintWindowSize()
{
    wxSize size = CalculateNewSplitSize();

    // the placeholder hint window should be set to this size
    auto info = m_mgr.GetPane("dummy");
    if (info != m_mgr.GetAllPanes().end())
    {
        info->MinSize(size);
        info->BestSize(size);
        m_dummyWnd->SetSize(size);
    }
}


// calculates the size of the new split
wxSize wxAuiNotebook::CalculateNewSplitSize()
{
    // count number of tab controls
    int tab_ctrl_count = std::ranges::count_if(m_mgr.GetAllPanes(),
        [](const auto& pane){ return pane.name == "dummy"; });

    wxSize new_split_size;

    // if there is only one tab control, the first split
    // should happen around the middle
    if (tab_ctrl_count < 2)
    {
        new_split_size = GetClientSize();
        new_split_size.x /= 2;
        new_split_size.y /= 2;
    }
    else
    {
        // this is in place of a more complicated calculation
        // that needs to be implemented
        new_split_size = FromDIP(wxSize(180,180));
    }

    return new_split_size;
}

int wxAuiNotebook::CalculateTabCtrlHeight()
{
    // if a fixed tab ctrl height is specified,
    // just return that instead of calculating a
    // tab height
    if (m_requestedTabCtrlHeight != -1)
        return m_requestedTabCtrlHeight;

    // find out new best tab height
    wxAuiTabArt* art = m_tabs.GetArtProvider();

    return art->GetBestTabCtrlSize(this,
                                   m_tabs.GetPages(),
                                   m_requestedBmpSize);
}


wxAuiTabArt* wxAuiNotebook::GetArtProvider() const
{
    return m_tabs.GetArtProvider();
}

void wxAuiNotebook::SetWindowStyleFlag(unsigned int style)
{
    wxControl::SetWindowStyleFlag(style);

    m_flags = (unsigned int)style;

    // if the control is already initialized
    if (m_mgr.GetManagedWindow() == (wxWindow*)this)
    {
        // let all of the tab children know about the new style

        for (auto& pane : m_mgr.GetAllPanes())
        {
            if (pane.name == "dummy")
                continue;

            wxTabFrame* tabframe = dynamic_cast<wxTabFrame*>(pane.window);
            wxAuiTabCtrl* tabctrl = tabframe->m_tabs;
            tabctrl->SetFlags(m_flags);
            tabframe->DoSizing();
            tabctrl->Refresh();
            tabctrl->Update();
        }
    }
}


bool wxAuiNotebook::AddPage(wxWindow* page,
                            const std::string& caption,
                            bool select,
                            const wxBitmap& bitmap)
{
    return InsertPage(GetPageCount(), page, caption, select, bitmap);
}

bool wxAuiNotebook::InsertPage(size_t page_idx,
                               wxWindow* page,
                               const std::string& caption,
                               bool select,
                               const wxBitmap& bitmap)
{
    wxASSERT_MSG(page, "page pointer must be non-NULL");
    if (!page)
        return false;

    page->Reparent(this);

    wxAuiNotebookPage info;
    info.window = page;
    info.caption = caption;
    info.bitmap = bitmap;
    info.active = false;

    // if there are currently no tabs, the first added
    // tab must be active
    if (m_tabs.GetPageCount() == 0)
        info.active = true;

    m_tabs.InsertPage(page, info, page_idx);

    // if that was the first page added, even if
    // select is false, it must become the "current page"
    // (though no select events will be fired)
    if (!select && m_tabs.GetPageCount() == 1)
        select = true;
        //m_curPage = GetPageIndex(page);

    wxAuiTabCtrl* active_tabctrl = GetActiveTabCtrl();
    if (page_idx >= active_tabctrl->GetPageCount())
        active_tabctrl->AddPage(page, info);
    else
        active_tabctrl->InsertPage(page, info, page_idx);

    // Note that we don't need to call DoSizing() if the height has changed, as
    // it's already called from UpdateTabCtrlHeight() itself in this case.
    if ( !UpdateTabCtrlHeight() )
        DoSizing();

    active_tabctrl->DoShowHide();

    // adjust selected index
    if(m_curPage >= (int) page_idx)
        m_curPage++;

    if (select)
    {
        SetSelectionToWindow(page);
    }

    return true;
}


// DeletePage() removes a tab from the multi-notebook,
// and destroys the window as well
bool wxAuiNotebook::DeletePage(size_t page_idx)
{
    if (page_idx >= m_tabs.GetPageCount())
        return false;

    wxWindow* wnd = m_tabs.GetWindowFromIdx(page_idx);

    // hide the window in advance, as this will
    // prevent flicker
    ShowWnd(wnd, false);

    if (!RemovePage(page_idx))
        return false;

#if wxUSE_MDI
    // actually destroy the window now
    if (wxDynamicCast(wnd, wxAuiMDIChildFrame))
    {
        // delete the child frame with pending delete, as is
        // customary with frame windows
        if (!wxPendingDelete.Member(wnd))
            wxPendingDelete.Append(wnd);
    }
    else
#endif
    {
        wnd->Destroy();
    }

    return true;
}



// RemovePage() removes a tab from the multi-notebook,
// but does not destroy the window
bool wxAuiNotebook::RemovePage(size_t page_idx)
{
    // save active window pointer
    wxWindow* active_wnd = nullptr;
    if (m_curPage >= 0)
        active_wnd = m_tabs.GetWindowFromIdx(m_curPage);

    // save pointer of window being deleted
    wxWindow* wnd = m_tabs.GetWindowFromIdx(page_idx);
    wxWindow* new_active = nullptr;

    // make sure we found the page
    if (!wnd)
        return false;

    ShowWnd(wnd, false);

    // find out which onscreen tab ctrl owns this tab
    wxAuiTabCtrl* ctrl;
    int ctrl_idx;
    if (!FindTab(wnd, &ctrl, &ctrl_idx))
        return false;

    bool is_curpage = (m_curPage == (int)page_idx);
    bool is_active_in_split = ctrl->GetPage(ctrl_idx).active;


    // remove the tab from main catalog
    if (!m_tabs.RemovePage(wnd))
        return false;

    // remove the tab from the onscreen tab ctrl
    ctrl->RemovePage(wnd);

    if (is_active_in_split)
    {
        int ctrl_new_page_count = (int)ctrl->GetPageCount();

        if (ctrl_idx >= ctrl_new_page_count)
            ctrl_idx = ctrl_new_page_count-1;

        if (ctrl_idx >= 0 && ctrl_idx < (int)ctrl->GetPageCount())
        {
            // set new page as active in the tab split
            ctrl->SetActivePage(ctrl_idx);

            // if the page deleted was the current page for the
            // entire tab control, then record the window
            // pointer of the new active page for activation
            if (is_curpage)
            {
                new_active = ctrl->GetWindowFromIdx(ctrl_idx);
            }
        }
    }
    else
    {
        // we are not deleting the active page, so keep it the same
        new_active = active_wnd;
    }


    if (!new_active)
    {
        // we haven't yet found a new page to active,
        // so select the next page from the main tab
        // catalogue

        if (page_idx < m_tabs.GetPageCount())
        {
            new_active = m_tabs.GetPage(page_idx).window;
        }

        if (!new_active && m_tabs.GetPageCount() > 0)
        {
            new_active = m_tabs.GetPage(0).window;
        }
    }


    RemoveEmptyTabFrames();

    m_curPage = wxNOT_FOUND;

    // set new active pane unless we're being destroyed anyhow
    if (new_active && !m_isBeingDeleted)
        SetSelectionToWindow(new_active);

    return true;
}

// GetPageIndex() returns the index of the page, or -1 if the
// page could not be located in the notebook
int wxAuiNotebook::GetPageIndex(wxWindow* page_wnd) const
{
    return m_tabs.GetIdxFromWindow(page_wnd);
}



// SetPageText() changes the tab caption of the specified page
bool wxAuiNotebook::SetPageText(size_t page_idx, const std::string& text)
{
    if (page_idx >= m_tabs.GetPageCount())
        return false;

    // update our own tab catalog
    wxAuiNotebookPage& page_info = m_tabs.GetPage(page_idx);
    page_info.caption = text;

    // update what's on screen
    wxAuiTabCtrl* ctrl;
    int ctrl_idx;
    if (FindTab(page_info.window, &ctrl, &ctrl_idx))
    {
        wxAuiNotebookPage& info = ctrl->GetPage(ctrl_idx);
        info.caption = text;
        ctrl->Refresh();
        ctrl->Update();
    }

    return true;
}

// returns the page caption
std::string wxAuiNotebook::GetPageText(size_t page_idx) const
{
    if (page_idx >= m_tabs.GetPageCount())
        return {};

    // update our own tab catalog
    const wxAuiNotebookPage& page_info = m_tabs.GetPage(page_idx);
    return page_info.caption;
}

bool wxAuiNotebook::SetPageToolTip(size_t page_idx, const std::string& text)
{
    if (page_idx >= m_tabs.GetPageCount())
        return false;

    // update our own tab catalog
    wxAuiNotebookPage& page_info = m_tabs.GetPage(page_idx);
    page_info.tooltip = text;

    wxAuiTabCtrl* ctrl;
    int ctrl_idx;
    if (!FindTab(page_info.window, &ctrl, &ctrl_idx))
        return false;

    wxAuiNotebookPage& info = ctrl->GetPage(ctrl_idx);
    info.tooltip = text;

    // NB: we don't update the tooltip if it is already being displayed, it
    //     typically never happens, no need to code that
    return true;
}

std::string wxAuiNotebook::GetPageToolTip(size_t page_idx) const
{
    if (page_idx >= m_tabs.GetPageCount())
        return {};

    const wxAuiNotebookPage& page_info = m_tabs.GetPage(page_idx);
    return page_info.tooltip;
}

bool wxAuiNotebook::SetPageBitmap(size_t page_idx, const wxBitmap& bitmap)
{
    if (page_idx >= m_tabs.GetPageCount())
        return false;

    // update our own tab catalog
    wxAuiNotebookPage& page_info = m_tabs.GetPage(page_idx);
    page_info.bitmap = bitmap;

    // tab height might have changed
    UpdateTabCtrlHeight();

    // update what's on screen
    wxAuiTabCtrl* ctrl;
    int ctrl_idx;
    if (FindTab(page_info.window, &ctrl, &ctrl_idx))
    {
        wxAuiNotebookPage& info = ctrl->GetPage(ctrl_idx);
        info.bitmap = bitmap;
        ctrl->Refresh();
        ctrl->Update();
    }

    return true;
}

// returns the page bitmap
wxBitmap wxAuiNotebook::GetPageBitmap(size_t page_idx) const
{
    if (page_idx >= m_tabs.GetPageCount())
        return {};

    // update our own tab catalog
    const wxAuiNotebookPage& page_info = m_tabs.GetPage(page_idx);
    return page_info.bitmap;
}

// GetSelection() returns the index of the currently active page
int wxAuiNotebook::GetSelection() const
{
    return m_curPage;
}

// SetSelection() sets the currently active page
int wxAuiNotebook::SetSelection(size_t new_page)
{
    return DoModifySelection(new_page, true);
}

void wxAuiNotebook::SetSelectionToWindow(wxWindow *win)
{
    const int idx = m_tabs.GetIdxFromWindow(win);
    wxCHECK_RET( idx != wxNOT_FOUND, "invalid notebook page" );


    // since a tab was clicked, let the parent know that we received
    // the focus, even if we will assign that focus immediately
    // to the child tab in the SetSelection call below
    // (the child focus event will also let wxAuiManager, if any,
    // know that the notebook control has been activated)

    wxWindow* parent = GetParent();
    if (parent)
    {
        wxChildFocusEvent eventFocus(this);
        parent->GetEventHandler()->ProcessEvent(eventFocus);
    }


    SetSelection(idx);
}

// GetPageCount() returns the total number of
// pages managed by the multi-notebook
size_t wxAuiNotebook::GetPageCount() const
{
    return m_tabs.GetPageCount();
}

// GetPage() returns the wxWindow pointer of the
// specified page
wxWindow* wxAuiNotebook::GetPage(size_t page_idx) const
{
    wxASSERT(page_idx < m_tabs.GetPageCount());

    return m_tabs.GetWindowFromIdx(page_idx);
}

// DoSizing() performs all sizing operations in each tab control
void wxAuiNotebook::DoSizing()
{
    for (auto& pane : m_mgr.GetAllPanes())
    {
        if (pane.name == "dummy")
            continue;

        auto* tabframe = dynamic_cast<wxTabFrame*>(pane.window);
        tabframe->DoSizing();
    }
}

// GetActiveTabCtrl() returns the active tab control.  It is
// called to determine which control gets new windows being added
wxAuiTabCtrl* wxAuiNotebook::GetActiveTabCtrl()
{
    if (m_curPage >= 0 && m_curPage < (int)m_tabs.GetPageCount())
    {
        wxAuiTabCtrl* ctrl;
        int idx;

        // find the tab ctrl with the current page
        if (FindTab(m_tabs.GetPage(m_curPage).window,
                    &ctrl, &idx))
        {
            return ctrl;
        }
    }

    // no current page, just find the first tab ctrl
    for (auto& pane : m_mgr.GetAllPanes())
    {
        if (pane.name == "dummy")
            continue;

        auto* tabframe = dynamic_cast<wxTabFrame*>(pane.window);
        return tabframe->m_tabs;
    }

    // If there is no tabframe at all, create one
    wxTabFrame* tabframe = new wxTabFrame;
    tabframe->SetTabCtrlHeight(m_tabCtrlHeight);
    tabframe->m_tabs = new wxAuiTabCtrl(this,
                                        m_tabIdCounter++,
                                        wxDefaultPosition,
                                        wxDefaultSize,
                                        wxNO_BORDER|wxWANTS_CHARS);
    tabframe->m_tabs->SetFlags(m_flags);
    tabframe->m_tabs->SetArtProvider(m_tabs.GetArtProvider()->Clone());
    m_mgr.AddPane(tabframe,
                  wxAuiPaneInfo().Center().CaptionVisible(false));

    m_mgr.Update();

    return tabframe->m_tabs;
}

// FindTab() finds the tab control that currently contains the window as well
// as the index of the window in the tab control.  It returns true if the
// window was found, otherwise false.
bool wxAuiNotebook::FindTab(wxWindow* page, wxAuiTabCtrl** ctrl, int* idx)
{    
    for (auto& pane : m_mgr.GetAllPanes())
    {
        if (pane.name == "dummy")
            continue;

        auto* tabframe = dynamic_cast<wxTabFrame*>(pane.window);

        int page_idx = tabframe->m_tabs->GetIdxFromWindow(page);
        if (page_idx != -1)
        {
            *ctrl = tabframe->m_tabs;
            *idx = page_idx;
            return true;
        }
    }

    return false;
}

void wxAuiNotebook::Split(size_t page, int direction)
{
    wxSize cli_size = GetClientSize();

    // get the page's window pointer
    wxWindow* wnd = GetPage(page);
    if (!wnd)
        return;

    // notebooks with 1 or less pages can't be split
    if (GetPageCount() < 2)
        return;

    // find out which tab control the page currently belongs to
    wxAuiTabCtrl *src_tabs, *dest_tabs;
    int src_idx = -1;
    src_tabs = nullptr;
    if (!FindTab(wnd, &src_tabs, &src_idx))
        return;
    if (!src_tabs || src_idx == -1)
        return;

    // choose a split size
    wxSize split_size;
    if (GetPageCount() > 2)
    {
        split_size = CalculateNewSplitSize();
    }
    else
    {
        // because there are two panes, always split them
        // equally
        split_size = GetClientSize();
        split_size.x /= 2;
        split_size.y /= 2;
    }


    // create a new tab frame
    wxTabFrame* new_tabs = new wxTabFrame;
    new_tabs->m_rect = wxRect(wxPoint(0,0), split_size);
    new_tabs->SetTabCtrlHeight(m_tabCtrlHeight);
    new_tabs->m_tabs = new wxAuiTabCtrl(this,
                                        m_tabIdCounter++,
                                        wxDefaultPosition,
                                        wxDefaultSize,
                                        wxNO_BORDER|wxWANTS_CHARS);
    new_tabs->m_tabs->SetArtProvider(m_tabs.GetArtProvider()->Clone());
    new_tabs->m_tabs->SetFlags(m_flags);
    dest_tabs = new_tabs->m_tabs;

    // create a pane info structure with the information
    // about where the pane should be added
    wxAuiPaneInfo paneInfo = wxAuiPaneInfo().Bottom().CaptionVisible(false);
    wxPoint mouse_pt;

    if (direction == wxLEFT)
    {
        paneInfo.Left();
        mouse_pt = wxPoint(0, cli_size.y/2);
    }
    else if (direction == wxRIGHT)
    {
        paneInfo.Right();
        mouse_pt = wxPoint(cli_size.x, cli_size.y/2);
    }
    else if (direction == wxTOP)
    {
        paneInfo.Top();
        mouse_pt = wxPoint(cli_size.x/2, 0);
    }
    else if (direction == wxBOTTOM)
    {
        paneInfo.Bottom();
        mouse_pt = wxPoint(cli_size.x/2, cli_size.y);
    }

    m_mgr.AddPane(new_tabs, paneInfo, mouse_pt);
    m_mgr.Update();

    // remove the page from the source tabs
    wxAuiNotebookPage page_info = src_tabs->GetPage(src_idx);
    page_info.active = false;
    src_tabs->RemovePage(page_info.window);
    if (src_tabs->GetPageCount() > 0)
    {
        src_tabs->SetActivePage((size_t)0);
        src_tabs->DoShowHide();
        src_tabs->Refresh();
    }


    // add the page to the destination tabs
    dest_tabs->InsertPage(page_info.window, page_info, 0);

    if (src_tabs->GetPageCount() == 0)
    {
        RemoveEmptyTabFrames();
    }

    DoSizing();
    dest_tabs->DoShowHide();
    dest_tabs->Refresh();

    // force the set selection function reset the selection
    m_curPage = -1;

    // set the active page to the one we just split off
    SetSelectionToPage(page_info);

    UpdateHintWindowSize();
}


void wxAuiNotebook::OnSize(wxSizeEvent& evt)
{
    UpdateHintWindowSize();

    evt.Skip();
}

void wxAuiNotebook::OnTabClicked(wxAuiNotebookEvent& evt)
{
    wxAuiTabCtrl* ctrl = (wxAuiTabCtrl*)evt.GetEventObject();
    wxASSERT(ctrl != nullptr);

    wxWindow* wnd = ctrl->GetWindowFromIdx(evt.GetSelection());
    wxASSERT(wnd != nullptr);

    SetSelectionToWindow(wnd);
}

void wxAuiNotebook::OnTabBgDClick(wxAuiNotebookEvent& evt)
{
    // select the tab ctrl which received the db click
    int selection;
    wxWindow* wnd;
    wxAuiTabCtrl* ctrl = (wxAuiTabCtrl*)evt.GetEventObject();
    if (   (ctrl != nullptr)
        && ((selection = ctrl->GetActivePage()) != wxNOT_FOUND)
        && ((wnd = ctrl->GetWindowFromIdx(selection)) != nullptr))
    {
        SetSelectionToWindow(wnd);
    }

    // notify owner that the tabbar background has been double-clicked
    wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_BG_DCLICK, m_windowId);
    e.SetEventObject(this);
    GetEventHandler()->ProcessEvent(e);
}

void wxAuiNotebook::OnTabBeginDrag(wxAuiNotebookEvent&)
{
    m_lastDragX = 0;
}

void wxAuiNotebook::OnTabDragMotion(wxAuiNotebookEvent& evt)
{
    wxPoint screen_pt = wxGetMousePosition();
    wxPoint client_pt = ScreenToClient(screen_pt);
    wxPoint zero(0,0);

    wxAuiTabCtrl* src_tabs = (wxAuiTabCtrl*)evt.GetEventObject();
    wxAuiTabCtrl* dest_tabs = GetTabCtrlFromPoint(client_pt);

    if (dest_tabs == src_tabs)
    {
        if (src_tabs)
        {
            src_tabs->SetCursor(wxCursor(wxCURSOR_ARROW));
        }

        // always hide the hint for inner-tabctrl drag
        m_mgr.HideHint();

        // if tab moving is not allowed, leave
        if (!(m_flags & wxAUI_NB_TAB_MOVE))
        {
            return;
        }

        wxPoint pt = dest_tabs->ScreenToClient(screen_pt);
        wxWindow* dest_location_tab;

        // this is an inner-tab drag/reposition
        if (dest_tabs->TabHitTest(pt.x, pt.y, &dest_location_tab))
        {
            int src_idx = evt.GetSelection();
            int dest_idx = dest_tabs->GetIdxFromWindow(dest_location_tab);

            // prevent jumpy drag
            if ((src_idx == dest_idx) || dest_idx == -1 ||
                (src_idx > dest_idx && m_lastDragX <= pt.x) ||
                (src_idx < dest_idx && m_lastDragX >= pt.x))
            {
                m_lastDragX = pt.x;
                return;
            }


            wxWindow* src_tab = dest_tabs->GetWindowFromIdx(src_idx);
            dest_tabs->MovePage(src_tab, dest_idx);
            m_tabs.MovePage(m_tabs.GetPage(src_idx).window, dest_idx);
            dest_tabs->SetActivePage((size_t)dest_idx);
            dest_tabs->DoShowHide();
            dest_tabs->Refresh();
            m_lastDragX = pt.x;

        }

        return;
    }


    // if external drag is allowed, check if the tab is being dragged
    // over a different wxAuiNotebook control
    if (m_flags & wxAUI_NB_TAB_EXTERNAL_MOVE)
    {
        wxWindow* tab_ctrl = ::wxFindWindowAtPoint(screen_pt);

        // if we aren't over any window, stop here
        if (!tab_ctrl)
            return;

        // make sure we are not over the hint window
        if (!wxDynamicCast(tab_ctrl, wxFrame))
        {
            while (tab_ctrl)
            {
                if (wxDynamicCast(tab_ctrl, wxAuiTabCtrl))
                    break;
                tab_ctrl = tab_ctrl->GetParent();
            }

            if (tab_ctrl)
            {
                wxAuiNotebook* nb = (wxAuiNotebook*)tab_ctrl->GetParent();

                if (nb != this)
                {
                    wxRect hint_rect = tab_ctrl->GetClientRect();
                    tab_ctrl->ClientToScreen(&hint_rect.x, &hint_rect.y);
                    m_mgr.ShowHint(hint_rect);
                    return;
                }
            }
        }
        else
        {
            if (!dest_tabs)
            {
                // we are either over a hint window, or not over a tab
                // window, and there is no where to drag to, so exit
                return;
            }
        }
    }


    // if there are less than two panes, split can't happen, so leave
    if (m_tabs.GetPageCount() < 2)
        return;

    // if tab moving is not allowed, leave
    if (!(m_flags & wxAUI_NB_TAB_SPLIT))
        return;


    if (src_tabs)
    {
        src_tabs->SetCursor(wxCursor(wxCURSOR_SIZING));
    }


    if (dest_tabs)
    {
        wxRect hint_rect = dest_tabs->GetRect();
        ClientToScreen(&hint_rect.x, &hint_rect.y);
        m_mgr.ShowHint(hint_rect);
    }
    else
    {
        m_mgr.DrawHintRect(m_dummyWnd, client_pt, zero);
    }
}



void wxAuiNotebook::OnTabEndDrag(wxAuiNotebookEvent& evt)
{
    m_mgr.HideHint();


    wxAuiTabCtrl* src_tabs = (wxAuiTabCtrl*)evt.GetEventObject();
    wxCHECK_RET( src_tabs, "no source object?" );

    src_tabs->SetCursor(wxCursor(wxCURSOR_ARROW));

    // get the mouse position, which will be used to determine the drop point
    wxPoint mouse_screen_pt = wxGetMousePosition();
    wxPoint mouse_client_pt = ScreenToClient(mouse_screen_pt);

    // Update our selection (it may be updated again below but the code below
    // can also return without doing anything else and this ensures that the
    // selection is updated even then).
    m_curPage = src_tabs->GetActivePage();

    // check for an external move
    if (m_flags & wxAUI_NB_TAB_EXTERNAL_MOVE)
    {
        wxWindow* tab_ctrl = ::wxFindWindowAtPoint(mouse_screen_pt);

        while (tab_ctrl)
        {
            if (wxDynamicCast(tab_ctrl, wxAuiTabCtrl))
                break;
            tab_ctrl = tab_ctrl->GetParent();
        }

        if (tab_ctrl)
        {
            wxAuiNotebook* nb = (wxAuiNotebook*)tab_ctrl->GetParent();

            if (nb != this)
            {
                // find out from the destination control
                // if it's ok to drop this tab here
                wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_ALLOW_DND, m_windowId);
                e.SetSelection(evt.GetSelection());
                e.SetOldSelection(evt.GetSelection());
                e.SetEventObject(this);
                e.SetDragSource(this);
                e.Veto(); // dropping must be explicitly approved by control owner

                nb->GetEventHandler()->ProcessEvent(e);

                if (!e.IsAllowed())
                {
                    // no answer or negative answer
                    m_mgr.HideHint();
                    return;
                }

                // drop was allowed
                int src_idx = evt.GetSelection();
                wxWindow* src_page = src_tabs->GetWindowFromIdx(src_idx);

                // Check that it's not an impossible parent relationship
                wxWindow* p = nb;
                while (p && !p->IsTopLevel())
                {
                    if (p == src_page)
                    {
                        return;
                    }
                    p = p->GetParent();
                }

                // get main index of the page
                int main_idx = m_tabs.GetIdxFromWindow(src_page);
                wxCHECK_RET( main_idx != wxNOT_FOUND, "no source page?" );


                // make a copy of the page info
                wxAuiNotebookPage page_info = m_tabs.GetPage(main_idx);

                // remove the page from the source notebook
                RemovePage(main_idx);

                // reparent the page
                src_page->Reparent(nb);


                // found out the insert idx
                wxAuiTabCtrl* dest_tabs = (wxAuiTabCtrl*)tab_ctrl;
                wxPoint pt = dest_tabs->ScreenToClient(mouse_screen_pt);

                wxWindow* target = nullptr;
                int insert_idx = -1;
                dest_tabs->TabHitTest(pt.x, pt.y, &target);
                if (target)
                {
                    insert_idx = dest_tabs->GetIdxFromWindow(target);
                }


                // add the page to the new notebook
                if (insert_idx == -1)
                    insert_idx = dest_tabs->GetPageCount();
                dest_tabs->InsertPage(page_info.window, page_info, insert_idx);
                nb->m_tabs.InsertPage(page_info.window, page_info, insert_idx);

                nb->DoSizing();
                dest_tabs->SetActivePage(insert_idx);
                dest_tabs->DoShowHide();
                dest_tabs->Refresh();

                // set the selection in the destination tab control
                nb->DoModifySelection(insert_idx, false);

                // notify owner that the tab has been dragged
                wxAuiNotebookEvent e2(wxEVT_AUINOTEBOOK_DRAG_DONE, m_windowId);
                e2.SetSelection(evt.GetSelection());
                e2.SetOldSelection(evt.GetSelection());
                e2.SetEventObject(this);
                GetEventHandler()->ProcessEvent(e2);

                return;
            }
        }
    }




    // only perform a tab split if it's allowed
    wxAuiTabCtrl* dest_tabs = nullptr;

    if ((m_flags & wxAUI_NB_TAB_SPLIT) && m_tabs.GetPageCount() >= 2)
    {
        // If the pointer is in an existing tab frame, do a tab insert
        wxWindow* hit_wnd = ::wxFindWindowAtPoint(mouse_screen_pt);
        wxTabFrame* tab_frame = (wxTabFrame*)GetTabFrameFromTabCtrl(hit_wnd);
        int insert_idx = -1;
        if (tab_frame)
        {
            dest_tabs = tab_frame->m_tabs;

            if (dest_tabs == src_tabs)
            {
                m_curPage = evt.GetSelection();
                return;
            }

            wxPoint pt = dest_tabs->ScreenToClient(mouse_screen_pt);
            wxWindow* target = nullptr;
            dest_tabs->TabHitTest(pt.x, pt.y, &target);
            if (target)
            {
                insert_idx = dest_tabs->GetIdxFromWindow(target);
            }
        }
        else
        {
            wxPoint zero(0,0);
            wxRect rect = m_mgr.CalculateHintRect(m_dummyWnd,
                                                  mouse_client_pt,
                                                  zero);
            if (rect.IsEmpty())
            {
                // there is no suitable drop location here, exit out
                return;
            }

            // If there is no tabframe at all, create one
            wxTabFrame* new_tabs = new wxTabFrame;
            new_tabs->m_rect = wxRect(wxPoint(0,0), CalculateNewSplitSize());
            new_tabs->SetTabCtrlHeight(m_tabCtrlHeight);
            new_tabs->m_tabs = new wxAuiTabCtrl(this,
                                                m_tabIdCounter++,
                                                wxDefaultPosition,
                                                wxDefaultSize,
                                                wxNO_BORDER|wxWANTS_CHARS);
            new_tabs->m_tabs->SetArtProvider(m_tabs.GetArtProvider()->Clone());
            new_tabs->m_tabs->SetFlags(m_flags);

            m_mgr.AddPane(new_tabs,
                          wxAuiPaneInfo().Bottom().CaptionVisible(false),
                          mouse_client_pt);
            m_mgr.Update();
            dest_tabs = new_tabs->m_tabs;
        }



        // remove the page from the source tabs
        wxAuiNotebookPage page_info = src_tabs->GetPage(evt.GetSelection());
        page_info.active = false;
        src_tabs->RemovePage(page_info.window);
        if (src_tabs->GetPageCount() > 0)
        {
            src_tabs->SetActivePage((size_t)0);
            src_tabs->DoShowHide();
            src_tabs->Refresh();
        }



        // add the page to the destination tabs
        if (insert_idx == -1)
            insert_idx = dest_tabs->GetPageCount();
        dest_tabs->InsertPage(page_info.window, page_info, insert_idx);

        if (src_tabs->GetPageCount() == 0)
        {
            RemoveEmptyTabFrames();
        }

        DoSizing();
        dest_tabs->DoShowHide();
        dest_tabs->Refresh();

        // force the set selection function reset the selection
        m_curPage = -1;

        // set the active page to the one we just split off
        SetSelectionToPage(page_info);

        UpdateHintWindowSize();
    }

    // notify owner that the tab has been dragged
    wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_DRAG_DONE, m_windowId);
    e.SetSelection(evt.GetSelection());
    e.SetOldSelection(evt.GetSelection());
    e.SetEventObject(this);
    GetEventHandler()->ProcessEvent(e);
}



void wxAuiNotebook::OnTabCancelDrag(wxAuiNotebookEvent& command_evt)
{
    wxAuiNotebookEvent& evt = (wxAuiNotebookEvent&)command_evt;

    m_mgr.HideHint();

    wxAuiTabCtrl* src_tabs = (wxAuiTabCtrl*)evt.GetEventObject();
    wxCHECK_RET( src_tabs, "no source object?" );

    src_tabs->SetCursor(wxCursor(wxCURSOR_ARROW));
}

wxAuiTabCtrl* wxAuiNotebook::GetTabCtrlFromPoint(const wxPoint& pt)
{
    // if we've just removed the last tab from the source
    // tab set, the remove the tab control completely
    for (auto& pane : m_mgr.GetAllPanes())
    {
        if (pane.name == "dummy")
            continue;

        auto* tabframe = dynamic_cast<wxTabFrame*>(pane.window);
        if (tabframe->m_tab_rect.Contains(pt))
            return tabframe->m_tabs;
    }

    return nullptr;
}

wxWindow* wxAuiNotebook::GetTabFrameFromTabCtrl(wxWindow* tab_ctrl)
{
    // if we've just removed the last tab from the source
    // tab set, the remove the tab control completely
    for (auto& pane : m_mgr.GetAllPanes())
    {
        if (pane.name == "dummy")
            continue;

        auto* tabframe = dynamic_cast<wxTabFrame*>(pane.window);
        if (tabframe->m_tabs == tab_ctrl)
        {
            return tabframe;
        }
    }

    return nullptr;
}

void wxAuiNotebook::RemoveEmptyTabFrames()
{
    // if we've just removed the last tab from the source
    // tab set, the remove the tab control completely
    for (auto& pane : m_mgr.GetAllPanes())
    {
        if (pane.name == "dummy")
            continue;

        auto* tab_frame = dynamic_cast<wxTabFrame*>(pane.window);
        if (tab_frame->m_tabs->GetPageCount() == 0)
        {
            m_mgr.DetachPane(tab_frame);

            // use pending delete because sometimes during
            // window closing, refreshs are pending
            if (!wxPendingDelete.Member(tab_frame->m_tabs))
                wxPendingDelete.Append(tab_frame->m_tabs);

            tab_frame->m_tabs = nullptr;

            delete tab_frame;
        }
    }


    // check to see if there is still a center pane;
    // if there isn't, make a frame the center pane
    wxWindow* first_good = nullptr;
    bool center_found = false;
    for (const auto& pane : m_mgr.GetAllPanes())
    {
        if (pane.name == "dummy")
            continue;
        if (pane.dock_direction == wxAUI_DOCK_CENTRE)
            center_found = true;
        if (!first_good)
            first_good = pane.window;
    }

    if (!center_found && first_good)
    {
        m_mgr.GetPane(first_good)->Centre();
    }

    if (!m_isBeingDeleted)
        m_mgr.Update();
}

void wxAuiNotebook::OnChildFocusNotebook(wxChildFocusEvent& evt)
{
    evt.Skip();

    // if we're dragging a tab, don't change the current selection.
    // This code prevents a bug that used to happen when the hint window
    // was hidden.  In the bug, the focus would return to the notebook
    // child, which would then enter this handler and call
    // SetSelection, which is not desired turn tab dragging.

    for (auto& pane : m_mgr.GetAllPanes())
    {
        if (pane.name == "dummy")
            continue;

        auto* tabframe = dynamic_cast<wxTabFrame*>(pane.window);
        if (tabframe->m_tabs->IsDragging())
            return;
    }


    // find the page containing the focused child
    wxWindow* win = evt.GetWindow();
    while ( win )
    {
        // pages have the notebook as the parent, so stop when we reach one
        // (and also stop in the impossible case of no parent at all)
        wxWindow* const parent = win->GetParent();
        if ( !parent || parent == this )
            break;

        win = parent;
    }

    // change the tab selection to this page
    int idx = m_tabs.GetIdxFromWindow(win);
    if (idx != -1 && idx != m_curPage)
    {
        SetSelection(idx);
    }
}

void wxAuiNotebook::OnNavigationKeyNotebook(wxNavigationKeyEvent& event)
{
    if ( event.IsWindowChange() ) {
        // change pages
        // FIXME: the problem with this is that if we have a split notebook,
        // we selection may go all over the place.
        AdvanceSelection(event.GetDirection());
    }
    else {
        // we get this event in 3 cases
        //
        // a) one of our pages might have generated it because the user TABbed
        // out from it in which case we should propagate the event upwards and
        // our parent will take care of setting the focus to prev/next sibling
        //
        // or
        //
        // b) the parent panel wants to give the focus to us so that we
        // forward it to our selected page. We can't deal with this in
        // OnSetFocus() because we don't know which direction the focus came
        // from in this case and so can't choose between setting the focus to
        // first or last panel child
        //
        // or
        //
        // c) we ourselves (see MSWTranslateMessage) generated the event
        //
        wxWindow * const parent = GetParent();

        // the wxObject* casts are required to avoid MinGW GCC 2.95.3 ICE
        const bool isFromParent = event.GetEventObject() == (wxObject*) parent;
        const bool isFromSelf = event.GetEventObject() == (wxObject*) this;

        if ( isFromParent || isFromSelf )
        {
            // no, it doesn't come from child, case (b) or (c): forward to a
            // page but only if direction is backwards (TAB) or from ourselves,
            if ( GetSelection() != wxNOT_FOUND &&
                    (!event.GetDirection() || isFromSelf) )
            {
                // so that the page knows that the event comes from it's parent
                // and is being propagated downwards
                event.SetEventObject(this);

                wxWindow *page = GetPage(GetSelection());
                if ( !page->GetEventHandler()->ProcessEvent(event) )
                {
                    page->SetFocus();
                }
                //else: page manages focus inside it itself
            }
            else // otherwise set the focus to the notebook itself
            {
                SetFocus();
            }
        }
        else
        {
            // it comes from our child, case (a), pass to the parent, but only
            // if the direction is forwards. Otherwise set the focus to the
            // notebook itself. The notebook is always the 'first' control of a
            // page.
            if ( !event.GetDirection() )
            {
                SetFocus();
            }
            else if ( parent )
            {
                event.SetCurrentFocus(this);
                parent->GetEventHandler()->ProcessEvent(event);
            }
        }
    }
}

void wxAuiNotebook::OnTabButton(wxAuiNotebookEvent& evt)
{
    wxAuiTabCtrl* tabs = (wxAuiTabCtrl*)evt.GetEventObject();

    int button_id = evt.GetInt();

    if (button_id == wxAUI_BUTTON_CLOSE)
    {
        int selection = evt.GetSelection();

        if (selection == -1)
        {
            // if the close button is to the right, use the active
            // page selection to determine which page to close
            selection = tabs->GetActivePage();
        }

        if (selection != -1)
        {
            wxWindow* close_wnd = tabs->GetWindowFromIdx(selection);

            // ask owner if it's ok to close the tab
            wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_PAGE_CLOSE, m_windowId);
            e.SetSelection(m_tabs.GetIdxFromWindow(close_wnd));
            const int idx = m_tabs.GetIdxFromWindow(close_wnd);
            e.SetSelection(idx);
            e.SetOldSelection(evt.GetSelection());
            e.SetEventObject(this);
            GetEventHandler()->ProcessEvent(e);
            if (!e.IsAllowed())
                return;


#if wxUSE_MDI
            if (wxDynamicCast(close_wnd, wxAuiMDIChildFrame))
            {
                close_wnd->Close();
            }
            else
#endif
            {
                int main_idx = m_tabs.GetIdxFromWindow(close_wnd);
                wxCHECK_RET( main_idx != wxNOT_FOUND, "no page to delete?" );

                DeletePage(main_idx);
            }

            // notify owner that the tab has been closed
            wxAuiNotebookEvent e2(wxEVT_AUINOTEBOOK_PAGE_CLOSED, m_windowId);
            e2.SetSelection(idx);
            e2.SetEventObject(this);
            GetEventHandler()->ProcessEvent(e2);
        }
    }
}


void wxAuiNotebook::OnTabMiddleDown(wxAuiNotebookEvent& evt)
{
    // patch event through to owner
    wxAuiTabCtrl* tabs = (wxAuiTabCtrl*)evt.GetEventObject();
    wxWindow* wnd = tabs->GetWindowFromIdx(evt.GetSelection());

    wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_TAB_MIDDLE_DOWN, m_windowId);
    e.SetSelection(m_tabs.GetIdxFromWindow(wnd));
    e.SetEventObject(this);
    GetEventHandler()->ProcessEvent(e);
}

void wxAuiNotebook::OnTabMiddleUp(wxAuiNotebookEvent& evt)
{
    // if the wxAUI_NB_MIDDLE_CLICK_CLOSE is specified, middle
    // click should act like a tab close action.  However, first
    // give the owner an opportunity to handle the middle up event
    // for custom action

    wxAuiTabCtrl* tabs = (wxAuiTabCtrl*)evt.GetEventObject();
    wxWindow* wnd = tabs->GetWindowFromIdx(evt.GetSelection());

    wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_TAB_MIDDLE_UP, m_windowId);
    e.SetSelection(m_tabs.GetIdxFromWindow(wnd));
    e.SetEventObject(this);
    if (GetEventHandler()->ProcessEvent(e))
        return;
    if (!e.IsAllowed())
        return;

    // check if we are supposed to close on middle-up
    if ((m_flags & wxAUI_NB_MIDDLE_CLICK_CLOSE) == 0)
        return;

    // simulate the user pressing the close button on the tab
    evt.SetInt(wxAUI_BUTTON_CLOSE);
    OnTabButton(evt);
}

void wxAuiNotebook::OnTabRightDown(wxAuiNotebookEvent& evt)
{
    // patch event through to owner
    wxAuiTabCtrl* tabs = (wxAuiTabCtrl*)evt.GetEventObject();
    wxWindow* wnd = tabs->GetWindowFromIdx(evt.GetSelection());

    wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN, m_windowId);
    e.SetSelection(m_tabs.GetIdxFromWindow(wnd));
    e.SetEventObject(this);
    GetEventHandler()->ProcessEvent(e);
}

void wxAuiNotebook::OnTabRightUp(wxAuiNotebookEvent& evt)
{
    // patch event through to owner
    wxAuiTabCtrl* tabs = (wxAuiTabCtrl*)evt.GetEventObject();
    wxWindow* wnd = tabs->GetWindowFromIdx(evt.GetSelection());

    wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_TAB_RIGHT_UP, m_windowId);
    e.SetSelection(m_tabs.GetIdxFromWindow(wnd));
    e.SetEventObject(this);
    GetEventHandler()->ProcessEvent(e);
}

// Sets the normal font
void wxAuiNotebook::SetNormalFont(const wxFont& font)
{
    m_normalFont = font;
    GetArtProvider()->SetNormalFont(font);
}

// Sets the selected tab font
void wxAuiNotebook::SetSelectedFont(const wxFont& font)
{
    m_selectedFont = font;
    GetArtProvider()->SetSelectedFont(font);
}

// Sets the measuring font
void wxAuiNotebook::SetMeasuringFont(const wxFont& font)
{
    GetArtProvider()->SetMeasuringFont(font);
}

// Sets the tab font
bool wxAuiNotebook::SetFont(const wxFont& font)
{
    wxControl::SetFont(font);

    wxFont normalFont(font);
    wxFont selectedFont(normalFont);
    selectedFont.SetWeight(wxFONTWEIGHT_BOLD);

    SetNormalFont(normalFont);
    SetSelectedFont(selectedFont);
    SetMeasuringFont(selectedFont);

    return true;
}

// Gets the tab control height
int wxAuiNotebook::GetTabCtrlHeight() const
{
    return m_tabCtrlHeight;
}

// Gets the height of the notebook for a given page height
int wxAuiNotebook::GetHeightForPageHeight(int pageHeight)
{
    UpdateTabCtrlHeight();

    int tabCtrlHeight = GetTabCtrlHeight();
    int decorHeight = 2;
    return tabCtrlHeight + pageHeight + decorHeight;
}

// Shows the window menu
bool wxAuiNotebook::ShowWindowMenu()
{
    wxAuiTabCtrl* tabCtrl = GetActiveTabCtrl();

    int idx = tabCtrl->GetArtProvider()->ShowDropDown(tabCtrl, tabCtrl->GetPages(), tabCtrl->GetActivePage());

    if (idx != -1)
    {
        wxAuiNotebookEvent e(wxEVT_AUINOTEBOOK_PAGE_CHANGING, tabCtrl->GetId());
        e.SetSelection(idx);
        e.SetOldSelection(tabCtrl->GetActivePage());
        e.SetEventObject(tabCtrl);
        GetEventHandler()->ProcessEvent(e);

        return true;
    }
    else
        return false;
}

void wxAuiNotebook::DoThaw()
{
    DoSizing();

    wxBookCtrlBase::DoThaw();
}

void wxAuiNotebook::SetPageSize ([[maybe_unused]] const wxSize& size)
{
    wxFAIL_MSG("Not implemented for wxAuiNotebook");
}

int wxAuiNotebook::HitTest (const wxPoint &pt, unsigned int* flags) const
{
    wxWindow *w = nullptr;
    long position = wxBK_HITTEST_NOWHERE;
    const wxAuiPaneInfoArray& all_panes = const_cast<wxAuiManager&>(m_mgr).GetAllPanes();
    const size_t pane_count = all_panes.size();
    for (size_t i = 0; i < pane_count; ++i)
    {
        if (all_panes[i].name == "dummy")
            continue;

        wxTabFrame* tabframe = (wxTabFrame*) all_panes[i].window;
        if (tabframe->m_tab_rect.Contains(pt))
        {
            wxPoint tabpos = tabframe->m_tabs->ScreenToClient(ClientToScreen(pt));
            if (tabframe->m_tabs->TabHitTest(tabpos.x, tabpos.y, &w))
                position = wxBK_HITTEST_ONITEM;
            break;
        }
        else if (tabframe->m_rect.Contains(pt))
        {
            w = tabframe->m_tabs->GetWindowFromIdx(tabframe->m_tabs->GetActivePage());
            if (w)
                position = wxBK_HITTEST_ONPAGE;
            break;
        }
    }

    if (flags)
        *flags = position;
    return w ? GetPageIndex(w) : wxNOT_FOUND;
}

int wxAuiNotebook::GetPageImage([[maybe_unused]] size_t n) const
{
    wxFAIL_MSG("Not implemented for wxAuiNotebook");
    return -1;
}

bool wxAuiNotebook::SetPageImage(size_t n, int imageId)
{
    return SetPageBitmap(n, GetImageList()->GetBitmap(imageId));
}

int wxAuiNotebook::ChangeSelection(size_t n)
{
    return DoModifySelection(n, false);
}

bool wxAuiNotebook::AddPage(wxWindow *page, const std::string &text, bool select,
                            int imageId)
{
    if(HasImageList())
    {
        return AddPage(page, text, select, GetImageList()->GetBitmap(imageId));
    }
    else
    {
        return AddPage(page, text, select, wxNullBitmap);
    }
}

bool wxAuiNotebook::DeleteAllPages()
{
    size_t count = GetPageCount();
    for(size_t i = 0; i < count; i++)
    {
        DeletePage(0);
    }
    return true;
}

bool wxAuiNotebook::InsertPage(size_t index, wxWindow *page,
                               const std::string &text, bool select,
                               int imageId)
{
    if(HasImageList())
    {
        return InsertPage(index, page, text, select,
                          GetImageList()->GetBitmap(imageId));
    }
    else
    {
        return InsertPage(index, page, text, select, wxNullBitmap);
    }
}

namespace
{

// Helper class to calculate the best size of a wxAuiNotebook
class wxAuiLayoutObject
{
public:
    enum
    {
        DockDir_Center,
        DockDir_Left,
        DockDir_Right,
        DockDir_Vertical,   // Merge elements from here vertically
        DockDir_Top,
        DockDir_Bottom,
        DockDir_None
    };

    wxAuiLayoutObject(const wxSize &size, const wxAuiPaneInfo &pInfo)
        : m_size(size)
    {
        m_pInfo = &pInfo;
        /*
            To speed up the sorting of the panes, the direction is mapped to a
            useful increasing value. This avoids complicated comparison of the
            enum values during the sort. The size calculation is done from the
            inner to the outermost direction. Therefore CENTER < LEFT/RIGHT <
            TOP/BOTTOM (It doesn't matter it LEFT or RIGHT is done first, as
            both extend the best size horizontally; the same applies for
            TOP/BOTTOM in vertical direction)
         */
        switch ( pInfo.dock_direction )
        {
            case wxAUI_DOCK_CENTER: m_dir = DockDir_Center; break;
            case wxAUI_DOCK_LEFT:   m_dir = DockDir_Left; break;
            case wxAUI_DOCK_RIGHT:  m_dir = DockDir_Right; break;
            case wxAUI_DOCK_TOP:    m_dir = DockDir_Top; break;
            case wxAUI_DOCK_BOTTOM: m_dir = DockDir_Bottom; break;
            default:                m_dir = DockDir_None;
        }
    }
    void MergeLayout(const wxAuiLayoutObject &lo2)
    {
        if ( this == &lo2 )
            return;

        bool mergeHorizontal;
        if ( m_pInfo->dock_layer != lo2.m_pInfo->dock_layer || m_dir != lo2.m_dir )
            mergeHorizontal = lo2.m_dir < DockDir_Vertical;
        else if ( m_pInfo->dock_row != lo2.m_pInfo->dock_row )
            mergeHorizontal = true;
        else
            mergeHorizontal = lo2.m_dir >= DockDir_Vertical;

        if ( mergeHorizontal )
        {
            m_size.x += lo2.m_size.x;
            if ( lo2.m_size.y > m_size.y )
                m_size.y = lo2.m_size.y;
        }
        else
        {
            if ( lo2.m_size.x > m_size.x )
                m_size.x = lo2.m_size.x;
            m_size.y += lo2.m_size.y;
        }
    }

    wxSize m_size;
    const wxAuiPaneInfo *m_pInfo;
    unsigned char m_dir;

    /*
        As the caulculation is done from the inner to the outermost pane, the
        panes are sorted in the following order: layer, direction, row,
        position.
     */
    bool operator<(const wxAuiLayoutObject& lo2) const
    {
        int diff = m_pInfo->dock_layer - lo2.m_pInfo->dock_layer;
        if ( diff )
            return diff < 0;
        diff = m_dir - lo2.m_dir;
        if ( diff )
            return diff < 0;
        diff = m_pInfo->dock_row - lo2.m_pInfo->dock_row;
        if ( diff )
            return diff < 0;
        return m_pInfo->dock_pos < lo2.m_pInfo->dock_pos;
    }
};

} // anonymous namespace

wxSize wxAuiNotebook::DoGetBestSize() const
{
    /*
        The best size of the wxAuiNotebook is a combination of all panes inside
        the object. To be able to efficiently  calculate the dimensions (i.e.
        without iterating over the panes multiple times) the panes need to be
        processed in a specific order. Therefore we need to collect them in the
        following variable which is sorted later on.
     */
    std::vector<wxAuiLayoutObject> layouts;
    const wxAuiPaneInfoArray& all_panes =
        const_cast<wxAuiManager&>(m_mgr).GetAllPanes();
    const size_t pane_count = all_panes.size();
    const int tabHeight = GetTabCtrlHeight();

    for(const auto& pInfo : all_panes)
    {
        if ( pInfo.name == "dummy" || pInfo.IsFloating() )
            continue;

        const auto* tabframe = dynamic_cast<wxTabFrame*>(pInfo.window);
        const wxAuiNotebookPageArray &pages = tabframe->m_tabs->GetPages();

        wxSize bestPageSize;

        for(const auto& page : pages)
            bestPageSize.IncTo(page.window->GetBestSize());

        bestPageSize.y += tabHeight;
        // Store the current pane with its largest window dimensions
        layouts.push_back(wxAuiLayoutObject(bestPageSize, pInfo));
    }

    if ( layouts.empty() )
        return {0, 0};

    std::sort(layouts.begin(), layouts.end());

    /*
        The sizes of the panes are merged here. As the center pane is always at
        position 0 all sizes are merged there. As panes can be stacked using
        the dock_pos property, different positions are merged at the first
        (i.e. dock_pos = 0) element before being merged with the center pane.
     */
    size_t pos = 0;
    for ( size_t n = 1; n < layouts.size(); n++ )
    {
        if ( layouts[n].m_pInfo->dock_layer == layouts[pos].m_pInfo->dock_layer &&
             layouts[n].m_dir == layouts[pos].m_dir &&
             layouts[n].m_pInfo->dock_row == layouts[pos].m_pInfo->dock_row )
        {
            layouts[pos].MergeLayout(layouts[n]);
        }
        else
        {
            layouts.front().MergeLayout(layouts[pos]);
            pos = n;
        }
    }

    layouts.front().MergeLayout(layouts[pos]);

    return layouts.front().m_size;
}

int wxAuiNotebook::DoModifySelection(size_t n, bool events)
{
    wxWindow* wnd = m_tabs.GetWindowFromIdx(n);
    if (!wnd)
        return m_curPage;

    // don't change the page unless necessary;
    // however, clicking again on a tab should give it the focus.
    if ((int)n == m_curPage)
    {
        wxAuiTabCtrl* ctrl;
        int ctrl_idx;
        if (FindTab(wnd, &ctrl, &ctrl_idx))
        {
            if (FindFocus() != ctrl)
                ctrl->SetFocus();
        }
        return m_curPage;
    }

    bool vetoed = false;

    wxAuiNotebookEvent evt(wxEVT_AUINOTEBOOK_PAGE_CHANGING, m_windowId);

    if(events)
    {
        evt.SetSelection(n);
        evt.SetOldSelection(m_curPage);
        evt.SetEventObject(this);
        GetEventHandler()->ProcessEvent(evt);
        vetoed = !evt.IsAllowed();
    }

    if (!vetoed)
    {
        int old_curpage = m_curPage;
        m_curPage = n;

        wxAuiTabCtrl* ctrl;
        int ctrl_idx;
        if (FindTab(wnd, &ctrl, &ctrl_idx))
        {
            m_tabs.SetActivePage(wnd);

            ctrl->SetActivePage(ctrl_idx);
            DoSizing();
            ctrl->DoShowHide();

            ctrl->MakeTabVisible(ctrl_idx, ctrl);

            // set fonts
            for (auto& pane : m_mgr.GetAllPanes())
            {
                if (pane.name == "dummy")
                    continue;

                wxAuiTabCtrl* tabctrl = ((wxTabFrame*)pane.window)->m_tabs;

                if (tabctrl != ctrl)
                    tabctrl->SetSelectedFont(m_normalFont);
                else
                    tabctrl->SetSelectedFont(m_selectedFont);

                tabctrl->Refresh();
            }

            // Set the focus to the page if we're not currently focused on the tab.
            // This is Firefox-like behaviour.
            if (wnd->IsShownOnScreen() && FindFocus() != ctrl)
                wnd->SetFocus();

            // program allows the page change
            if(events)
            {
                evt.SetEventType(wxEVT_AUINOTEBOOK_PAGE_CHANGED);
                std::ignore = GetEventHandler()->ProcessEvent(evt);
            }

            return old_curpage;
        }
    }

    return m_curPage;
}

void wxAuiTabCtrl::SetHoverTab(wxWindow* wnd)
{
    bool hoverChanged = false;

    for(auto& page : m_pages)
    {
        bool oldHover = page.hover;
        page.hover = (page.window == wnd);
        if ( oldHover != page.hover )
            hoverChanged = true;
    }

    if ( hoverChanged )
    {
        Refresh();
        Update();
    }
}


