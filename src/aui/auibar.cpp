///////////////////////////////////////////////////////////////////////////////
// Name:        src/aui/auibar.cpp
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by:
// Created:     2005-05-17
// Copyright:   (C) Copyright 2005-2006, Kirix Corporation, All Rights Reserved
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/bitmap.h"
#include "wx/dcclient.h"
#include "wx/dcbuffer.h"
#include "wx/sizer.h"
#include "wx/settings.h"

#ifdef __WXMAC__
#include "wx/osx/private.h"
#endif

module WX.AUI.AUIBar;

import WX.Utils.Cast;

import WX.AUI.FrameManager;

namespace
{

// missing wxITEM_* items
enum
{
    wxITEM_CONTROL = wxITEM_MAX,
    wxITEM_LABEL,
    wxITEM_SPACER
};

wxOrientation GetOrientation(unsigned int style)
{
    switch (style & wxAUI_ORIENTATION_MASK)
    {
        case wxAUI_TB_HORIZONTAL:
            return wxHORIZONTAL;
        case wxAUI_TB_VERTICAL:
            return wxVERTICAL;
        default:
            wxFAIL_MSG("toolbar cannot be locked in both horizontal and vertical orientations (maybe no lock was intended?)");
            [[fallthrough]];
        case 0:
            return wxBOTH;
    }
}

} // namespace anonymous

wxBEGIN_EVENT_TABLE(wxAuiToolBar, wxControl)
    EVT_SIZE(wxAuiToolBar::OnSize)
    EVT_IDLE(wxAuiToolBar::OnIdle)
    EVT_ERASE_BACKGROUND(wxAuiToolBar::OnEraseBackground)
    EVT_PAINT(wxAuiToolBar::OnPaint)
    EVT_LEFT_DOWN(wxAuiToolBar::OnLeftDown)
    EVT_LEFT_DCLICK(wxAuiToolBar::OnLeftDown)
    EVT_LEFT_UP(wxAuiToolBar::OnLeftUp)
    EVT_RIGHT_DOWN(wxAuiToolBar::OnRightDown)
    EVT_RIGHT_DCLICK(wxAuiToolBar::OnRightDown)
    EVT_RIGHT_UP(wxAuiToolBar::OnRightUp)
    EVT_MIDDLE_DOWN(wxAuiToolBar::OnMiddleDown)
    EVT_MIDDLE_DCLICK(wxAuiToolBar::OnMiddleDown)
    EVT_MIDDLE_UP(wxAuiToolBar::OnMiddleUp)
    EVT_MOTION(wxAuiToolBar::OnMotion)
    EVT_LEAVE_WINDOW(wxAuiToolBar::OnLeaveWindow)
    EVT_MOUSE_CAPTURE_LOST(wxAuiToolBar::OnCaptureLost)
    EVT_SET_CURSOR(wxAuiToolBar::OnSetCursor)
    EVT_SYS_COLOUR_CHANGED(wxAuiToolBar::OnSysColourChanged)
wxEND_EVENT_TABLE()

bool wxAuiToolBar::Create(wxWindow* parent,
                           wxWindowID id,
                           const wxPoint& pos,
                           const wxSize& size,
                           unsigned int style)
{
    style = style|wxBORDER_NONE;

    if (!wxControl::Create(parent, id, pos, size, style))
        return false;

    m_windowStyle = style;

    m_gripperVisible  = (style & wxAUI_TB_GRIPPER) != 0;
    m_overflowVisible = (style & wxAUI_TB_OVERFLOW) != 0;

    m_orientation = GetOrientation(style);
    if (m_orientation == wxBOTH)
    {
        m_orientation = wxHORIZONTAL;
    }

    wxSize margin_lt = FromDIP(wxSize(5, 5));
    wxSize margin_rb = FromDIP(wxSize(2, 2));
    SetMargins(margin_lt.x, margin_lt.y, margin_rb.x, margin_rb.y);
    SetFont(*wxNORMAL_FONT);
    SetArtFlags();
    SetExtraStyle(wxWS_EX_PROCESS_IDLE);
    if (style & wxAUI_TB_HORZ_LAYOUT)
        SetToolTextOrientation(wxAUI_TBTOOL_TEXT_RIGHT);
    SetBackgroundStyle(wxBackgroundStyle::Paint);

    return true;
}

void wxAuiToolBar::SetWindowStyleFlag(unsigned int style)
{
    GetOrientation(style);      // assert if style is invalid
    wxCHECK_RET(IsPaneValid(style),
                "window settings and pane settings are incompatible");

    wxControl::SetWindowStyleFlag(style);

    m_windowStyle = style;

    if (m_art)
    {
        SetArtFlags();
    }

    m_gripperVisible = (m_windowStyle & wxAUI_TB_GRIPPER) != 0;


    m_overflowVisible = (m_windowStyle & wxAUI_TB_OVERFLOW) != 0;

    if (style & wxAUI_TB_HORZ_LAYOUT)
        SetToolTextOrientation(wxAUI_TBTOOL_TEXT_RIGHT);
    else
        SetToolTextOrientation(wxAUI_TBTOOL_TEXT_BOTTOM);
}

void wxAuiToolBar::SetArtProvider(std::unique_ptr<wxAuiToolBarArt> art)
{
    m_art = std::move(art);

    if (m_art)
    {
        SetArtFlags();
        m_art->SetTextOrientation(m_toolTextOrientation);
    }
}

wxAuiToolBarArt* wxAuiToolBar::GetArtProvider() const
{
    return m_art.get();
}

wxAuiToolBarItem* wxAuiToolBar::AddTool(int tool_id,
                           const std::string& label,
                           const wxBitmap& bitmap,
                           const std::string& shortHelp_string,
                           wxItemKind kind)
{
    return AddTool(tool_id,
            label,
            bitmap,
            wxNullBitmap,
            kind,
            shortHelp_string,
            {},
            nullptr);
}


wxAuiToolBarItem* wxAuiToolBar::AddTool(int tool_id,
                           const std::string& label,
                           const wxBitmap& bitmap,
                           const wxBitmap& disabledBitmap,
                           wxItemKind kind,
                           const std::string& shortHelpString,
                           const std::string& longHelpString,
                           [[maybe_unused]] wxObject* client_data)
{
    wxAuiToolBarItem item;
    item.m_window = nullptr;
    item.m_label = label;
    item.m_bitmap = bitmap;
    item.m_disabledBitmap = disabledBitmap;
    item.m_shortHelp = shortHelpString;
    item.m_longHelp = longHelpString;
    item.m_active = true;
    item.m_dropDown = false;
    item.m_spacerPixels = 0;
    item.m_toolId = tool_id;
    item.m_state = 0;
    item.m_proportion = 0;
    item.m_kind = kind;
    item.m_sizerItem = nullptr;
    item.m_minSize = wxDefaultSize;
    item.m_userData = 0;
    item.m_sticky = false;

    if (item.m_toolId == wxID_ANY)
        item.m_toolId = wxNewId();

    if (!item.m_disabledBitmap.IsOk())
    {
        // no disabled bitmap specified, we need to make one
        if (item.m_bitmap.IsOk())
        {
            item.m_disabledBitmap = item.m_bitmap.ConvertToDisabled();
        }
    }
    m_items.push_back(item);
    return &m_items.back();
}

wxAuiToolBarItem* wxAuiToolBar::AddControl(wxControl* control,
                              const std::string& label)
{
    wxAuiToolBarItem item;
    item.m_window = (wxWindow*)control;
    item.m_label = label;
    item.m_bitmap = wxNullBitmap;
    item.m_disabledBitmap = wxNullBitmap;
    item.m_active = true;
    item.m_dropDown = false;
    item.m_spacerPixels = 0;
    item.m_toolId = control->GetId();
    item.m_state = 0;
    item.m_proportion = 0;
    item.m_kind = wxITEM_CONTROL;
    item.m_sizerItem = nullptr;
    item.m_minSize = control->GetEffectiveMinSize();
    item.m_userData = 0;
    item.m_sticky = false;

    m_items.push_back(item);
    return &m_items.back();
}

wxAuiToolBarItem* wxAuiToolBar::AddLabel(int tool_id,
                            const std::string& label,
                            const int width)
{
    wxSize min_size = wxDefaultSize;
    if (width != -1)
        min_size.x = width;

    wxAuiToolBarItem item;
    item.m_window = nullptr;
    item.m_label = label;
    item.m_bitmap = wxNullBitmap;
    item.m_disabledBitmap = wxNullBitmap;
    item.m_active = true;
    item.m_dropDown = false;
    item.m_spacerPixels = 0;
    item.m_toolId = tool_id;
    item.m_state = 0;
    item.m_proportion = 0;
    item.m_kind = wxITEM_LABEL;
    item.m_sizerItem = nullptr;
    item.m_minSize = min_size;
    item.m_userData = 0;
    item.m_sticky = false;

    if (item.m_toolId == wxID_ANY)
        item.m_toolId = wxNewId();

    m_items.push_back(item);
    return &m_items.back();
}

wxAuiToolBarItem* wxAuiToolBar::AddSeparator()
{
    wxAuiToolBarItem item;
    item.m_window = nullptr;
    item.m_label = {};
    item.m_bitmap = wxNullBitmap;
    item.m_disabledBitmap = wxNullBitmap;
    item.m_active = true;
    item.m_dropDown = false;
    item.m_toolId = -1;
    item.m_state = 0;
    item.m_proportion = 0;
    item.m_kind = wxITEM_SEPARATOR;
    item.m_sizerItem = nullptr;
    item.m_minSize = wxDefaultSize;
    item.m_userData = 0;
    item.m_sticky = false;

    m_items.push_back(item);
    return &m_items.back();
}

wxAuiToolBarItem* wxAuiToolBar::AddSpacer(int pixels)
{
    wxAuiToolBarItem item;
    item.m_window = nullptr;
    item.m_label = {};
    item.m_bitmap = wxNullBitmap;
    item.m_disabledBitmap = wxNullBitmap;
    item.m_active = true;
    item.m_dropDown = false;
    item.m_spacerPixels = pixels;
    item.m_toolId = -1;
    item.m_state = 0;
    item.m_proportion = 0;
    item.m_kind = wxITEM_SPACER;
    item.m_sizerItem = nullptr;
    item.m_minSize = wxDefaultSize;
    item.m_userData = 0;
    item.m_sticky = false;

    m_items.push_back(item);
    return &m_items.back();
}

wxAuiToolBarItem* wxAuiToolBar::AddStretchSpacer(int proportion)
{
    wxAuiToolBarItem item;
    item.m_window = nullptr;
    item.m_label = {};
    item.m_bitmap = wxNullBitmap;
    item.m_disabledBitmap = wxNullBitmap;
    item.m_active = true;
    item.m_dropDown = false;
    item.m_spacerPixels = 0;
    item.m_toolId = -1;
    item.m_state = 0;
    item.m_proportion = proportion;
    item.m_kind = wxITEM_SPACER;
    item.m_sizerItem = nullptr;
    item.m_minSize = wxDefaultSize;
    item.m_userData = 0;
    item.m_sticky = false;

    m_items.push_back(item);
    return &m_items.back();
}

void wxAuiToolBar::Clear()
{
    m_items.clear();
    m_sizerElementCount = 0;
}

bool wxAuiToolBar::DeleteTool(int tool_id)
{
    return DeleteByIndex(GetToolIndex(tool_id));
}

bool wxAuiToolBar::DeleteByIndex(int idx)
{
    if (idx >= 0 && idx < (int)m_items.size())
    {
        m_items.erase(m_items.begin() + idx);
        Realize();
        return true;
    }

    return false;
}

bool wxAuiToolBar::DestroyTool(int tool_id)
{
    return DestroyToolByIndex(GetToolIndex(tool_id));
}

bool wxAuiToolBar::DestroyToolByIndex(int idx)
{
    if ( idx < 0 || wx::narrow_cast<unsigned>(idx) >= m_items.size() )
        return false;

    if ( wxWindow* window = m_items[idx].GetWindow() )
        window->Destroy();

    return DeleteByIndex(idx);
}

wxControl* wxAuiToolBar::FindControl(int id)
{
    wxWindow* wnd = wxFindWindow(id);
    return (wxControl*)wnd;
}

wxAuiToolBarItem* wxAuiToolBar::FindToolByPosition(wxCoord x, wxCoord y)
{
    for (size_t i = 0, count = m_items.size(); i < count; ++i)
    {
        wxAuiToolBarItem& item = m_items[i];

        if (!item.m_sizerItem)
            continue;

        wxRect rect = item.m_sizerItem->GetRect();
        if (rect.Contains(x,y))
        {
            // if the item doesn't fit on the toolbar, return NULL
            if (!GetToolFitsByIndex(i))
                return nullptr;

            return &item;
        }
    }

    return nullptr;
}

wxAuiToolBarItem* wxAuiToolBar::FindToolByPositionWithPacking(wxCoord x, wxCoord y)
{
    for (size_t i = 0, count = m_items.size(); i < count; ++i)
    {
        wxAuiToolBarItem& item = m_items[i];

        if (!item.m_sizerItem)
            continue;

        wxRect rect = item.m_sizerItem->GetRect();

        // apply tool packing
        if (i+1 < count)
            rect.width += m_toolPacking;

        if (rect.Contains(x,y))
        {
            // if the item doesn't fit on the toolbar, return NULL
            if (!GetToolFitsByIndex(i))
                return nullptr;

            return &item;
        }
    }

    return nullptr;
}

void wxAuiToolBar::SetToolBitmapSize([[maybe_unused]] const wxSize& size)
{
    // TODO: wxToolBar compatibility
}

wxSize wxAuiToolBar::GetToolBitmapSize() const
{
    // TODO: wxToolBar compatibility
    return FromDIP(wxSize(16,15));
}

void wxAuiToolBar::SetToolProportion(int tool_id, int proportion)
{
    auto item = FindTool(tool_id);
    if (item == m_items.end())
        return;

    item->m_proportion = proportion;
}

int wxAuiToolBar::GetToolProportion(int tool_id) const
{
    const auto item = FindTool(tool_id);
    if (item == m_items.end())
        return 0;

    return item->m_proportion;
}

void wxAuiToolBar::SetToolSeparation(int separation)
{
    if (m_art)
        m_art->SetElementSize(wxAUI_TBART_SEPARATOR_SIZE, separation);
}

int wxAuiToolBar::GetToolSeparation() const
{
    if (m_art)
        return m_art->GetElementSize(wxAUI_TBART_SEPARATOR_SIZE);
    else
        return FromDIP(5);
}

void wxAuiToolBar::SetToolDropDown(int tool_id, bool dropdown)
{
    auto item = FindTool(tool_id);

    if (item == m_items.end())
        return;

    item->SetHasDropDown(dropdown);
}

bool wxAuiToolBar::GetToolDropDown(int tool_id) const
{
    const auto item = FindTool(tool_id);
    if (item == m_items.end())
        return false;

    return item->HasDropDown();
}

void wxAuiToolBar::SetToolSticky(int tool_id, bool sticky)
{
    // ignore separators
    if (tool_id == -1)
        return;

    auto item = FindTool(tool_id);
    if (item == m_items.end())
        return;

    if (item->m_sticky == sticky)
        return;

    item->m_sticky = sticky;

    Refresh(false);
    Update();
}

bool wxAuiToolBar::GetToolSticky(int tool_id) const
{
    const auto item = FindTool(tool_id);
    if (item == m_items.end())
        return false;

    return item->m_sticky;
}

void wxAuiToolBar::SetToolBorderPadding(int padding)
{
    m_toolBorderPadding = padding;
}

int wxAuiToolBar::GetToolBorderPadding() const
{
    return m_toolBorderPadding;
}

void wxAuiToolBar::SetToolTextOrientation(int orientation)
{
    m_toolTextOrientation = orientation;

    if (m_art)
    {
        m_art->SetTextOrientation(orientation);
    }
}

int wxAuiToolBar::GetToolTextOrientation() const
{
    return m_toolTextOrientation;
}

void wxAuiToolBar::SetToolPacking(int packing)
{
    m_toolPacking = packing;
}

int wxAuiToolBar::GetToolPacking() const
{
    return m_toolPacking;
}

void wxAuiToolBar::SetOrientation(int orientation)
{
    wxCHECK_RET(orientation == wxHORIZONTAL ||
                orientation == wxVERTICAL,
                "invalid orientation value");
    if (orientation != m_orientation)
    {
        m_orientation = wxOrientation(orientation);
        SetArtFlags();
    }
}

void wxAuiToolBar::SetMargins(int left, int right, int top, int bottom)
{
    if (left != -1)
        m_leftPadding = left;
    if (right != -1)
        m_rightPadding = right;
    if (top != -1)
        m_topPadding = top;
    if (bottom != -1)
        m_bottomPadding = bottom;
}

bool wxAuiToolBar::GetGripperVisible() const
{
    return m_gripperVisible;
}

void wxAuiToolBar::SetGripperVisible(bool visible)
{
    m_gripperVisible = visible;
    if (visible)
        m_windowStyle |= wxAUI_TB_GRIPPER;
    else
        m_windowStyle &= ~wxAUI_TB_GRIPPER;
    Realize();
    Refresh(false);
}

bool wxAuiToolBar::GetOverflowVisible() const
{
    return m_overflowVisible;
}

void wxAuiToolBar::SetOverflowVisible(bool visible)
{
    m_overflowVisible = visible;
    if (visible)
        m_windowStyle |= wxAUI_TB_OVERFLOW;
    else
        m_windowStyle &= ~wxAUI_TB_OVERFLOW;
    Refresh(false);
}

bool wxAuiToolBar::SetFont(const wxFont& font)
{
    bool res = wxWindow::SetFont(font);

    if (m_art)
    {
        m_art->SetFont(font);
    }

    return res;
}

void wxAuiToolBar::SetHoverItem(wxAuiToolBarItem* pitem)
{
    if (pitem && (pitem->m_state & wxAUI_BUTTON_STATE_DISABLED))
        pitem = nullptr;

    wxAuiToolBarItem* former_hover = nullptr;

    for (size_t i = 0, count = m_items.size(); i < count; ++i)
    {
        wxAuiToolBarItem& item = m_items[i];
        if (item.m_state & wxAUI_BUTTON_STATE_HOVER)
            former_hover = &item;
        item.m_state &= ~wxAUI_BUTTON_STATE_HOVER;
    }

    if (pitem)
    {
        pitem->m_state |= wxAUI_BUTTON_STATE_HOVER;
    }

    if (former_hover != pitem)
    {
        Refresh(false);
        Update();
    }
}

void wxAuiToolBar::SetPressedItem(wxAuiToolBarItem* pitem)
{
    wxAuiToolBarItem* former_item = nullptr;

    for (size_t i = 0, count = m_items.size(); i < count; ++i)
    {
        wxAuiToolBarItem& item = m_items[i];
        if (item.m_state & wxAUI_BUTTON_STATE_PRESSED)
            former_item = &item;
        item.m_state &= ~wxAUI_BUTTON_STATE_PRESSED;
    }

    if (pitem)
    {
        pitem->m_state &= ~wxAUI_BUTTON_STATE_HOVER;
        pitem->m_state |= wxAUI_BUTTON_STATE_PRESSED;
    }

    if (former_item != pitem)
    {
        Refresh(false);
        Update();
    }
}

void wxAuiToolBar::RefreshOverflowState()
{
    if (!m_overflowSizerItem)
    {
        m_overflowState = 0;
        return;
    }

    int overflow_state = 0;

    wxRect overflow_rect = GetOverflowRect();


    // find out the mouse's current position
    wxPoint pt = ::wxGetMousePosition();
    pt = this->ScreenToClient(pt);

    // find out if the mouse cursor is inside the dropdown rectangle
    if (overflow_rect.Contains(pt.x, pt.y))
    {
        if (::wxGetMouseState().LeftIsDown())
            overflow_state = wxAUI_BUTTON_STATE_PRESSED;
        else
            overflow_state = wxAUI_BUTTON_STATE_HOVER;
    }

    if (overflow_state != m_overflowState)
    {
        m_overflowState = overflow_state;
        Refresh(false);
        Update();
    }

    m_overflowState = overflow_state;
}

void wxAuiToolBar::ToggleTool(int tool_id, bool state)
{
    auto tool = FindTool(tool_id);

    if ( (tool != m_items.end()) && tool->CanBeToggled() )
    {
        if (tool->m_kind == wxITEM_RADIO)
        {
            int idx, count;
            idx = GetToolIndex(tool_id);
            count = (int)m_items.size();

            if (idx >= 0 && idx < count)
            {
                for (int i = idx + 1; i < count; ++i)
                {
                    if (m_items[i].m_kind != wxITEM_RADIO)
                        break;
                    m_items[i].m_state &= ~wxAUI_BUTTON_STATE_CHECKED;
                }
                for (int i = idx - 1; i >= 0; i--)
                {
                    if (m_items[i].m_kind != wxITEM_RADIO)
                        break;
                    m_items[i].m_state &= ~wxAUI_BUTTON_STATE_CHECKED;
                }
            }

            tool->m_state |= wxAUI_BUTTON_STATE_CHECKED;
        }
         else if (tool->m_kind == wxITEM_CHECK)
        {
            if (state)
                tool->m_state |= wxAUI_BUTTON_STATE_CHECKED;
            else
                tool->m_state &= ~wxAUI_BUTTON_STATE_CHECKED;
        }
    }
}

bool wxAuiToolBar::GetToolToggled(int tool_id) const
{
    const auto tool = FindTool(tool_id);

    if ( (tool != m_items.end()) && tool->CanBeToggled() )
        return (tool->m_state & wxAUI_BUTTON_STATE_CHECKED) != 0;

    return false;
}

void wxAuiToolBar::EnableTool(int tool_id, bool state)
{
    auto tool = FindTool(tool_id);

    if (tool != m_items.end())
    {
        if (state)
            tool->m_state &= ~wxAUI_BUTTON_STATE_DISABLED;
        else
            tool->m_state |= wxAUI_BUTTON_STATE_DISABLED;
    }
}

bool wxAuiToolBar::GetToolEnabled(int tool_id) const
{
    const auto tool = FindTool(tool_id);

    if (tool != m_items.end())
        return (tool->m_state & wxAUI_BUTTON_STATE_DISABLED) == 0;

    return false;
}

std::string wxAuiToolBar::GetToolLabel(int tool_id) const
{
    const auto tool = FindTool(tool_id);

    if (tool == m_items.end())
        return {};

    return tool->m_label;
}

void wxAuiToolBar::SetToolLabel(int tool_id, const std::string& label)
{
    auto tool = FindTool(tool_id);
    if (tool != m_items.end())
    {
        tool->m_label = label;
    }
}

wxBitmap wxAuiToolBar::GetToolBitmap(int tool_id) const
{
    const auto tool = FindTool(tool_id);

    if (tool == m_items.end())
        return wxNullBitmap;

    return tool->m_bitmap;
}

void wxAuiToolBar::SetToolBitmap(int tool_id, const wxBitmap& bitmap)
{
    auto tool = FindTool(tool_id);
    if (tool != m_items.end())
    {
        tool->m_bitmap = bitmap;
    }
}

std::string wxAuiToolBar::GetToolShortHelp(int tool_id) const
{
    const auto tool = FindTool(tool_id);

    if (tool == m_items.end())
        return {};

    return tool->m_shortHelp;
}

void wxAuiToolBar::SetToolShortHelp(int tool_id, const std::string& help_string)
{
    auto tool = FindTool(tool_id);
    if (tool != m_items.end())
    {
        tool->m_shortHelp = help_string;
    }
}

std::string wxAuiToolBar::GetToolLongHelp(int tool_id) const
{
    const auto tool = FindTool(tool_id);

    if (tool == m_items.end())
        return {};

    return tool->m_longHelp;
}

void wxAuiToolBar::SetToolLongHelp(int tool_id, const std::string& help_string)
{
    auto tool = FindTool(tool_id);
    if (tool != m_items.end())
    {
        tool->m_longHelp = help_string;
    }
}

void wxAuiToolBar::SetCustomOverflowItems(const wxAuiToolBarItemArray& prepend,
                                          const wxAuiToolBarItemArray& append)
{
    m_customOverflowPrepend = prepend;
    m_customOverflowAppend = append;
}

// get size of hint rectangle for a particular dock location
wxSize wxAuiToolBar::GetHintSize(int dock_direction) const
{
    switch (dock_direction)
    {
        case wxAUI_DOCK_TOP:
        case wxAUI_DOCK_BOTTOM:
            return m_horzHintSize;
        case wxAUI_DOCK_RIGHT:
        case wxAUI_DOCK_LEFT:
            return m_vertHintSize;
        default:
            wxFAIL_MSG("invalid dock location value");
    }
    return wxDefaultSize;
}

bool wxAuiToolBar::IsPaneValid(const wxAuiPaneInfo& pane) const
{
    return IsPaneValid(m_windowStyle, pane);
}

bool wxAuiToolBar::IsPaneValid(unsigned int style, const wxAuiPaneInfo& pane)
{
    if (style & wxAUI_TB_HORIZONTAL)
    {
        if (pane.IsLeftDockable() || pane.IsRightDockable())
        {
            return false;
        }
    }
    else if (style & wxAUI_TB_VERTICAL)
    {
        if (pane.IsTopDockable() || pane.IsBottomDockable())
        {
            return false;
        }
    }
    return true;
}

bool wxAuiToolBar::IsPaneValid(unsigned int style) const
{
    wxAuiManager* manager = wxAuiManager::GetManager(const_cast<wxAuiToolBar*>(this));
    if (manager)
    {
        return IsPaneValid(style, manager->GetPane(const_cast<wxAuiToolBar*>(this)));
    }
    return true;
}

void wxAuiToolBar::SetArtFlags() const
{
    unsigned int artflags = m_windowStyle & ~wxAUI_ORIENTATION_MASK;
    if (m_orientation == wxVERTICAL)
    {
        artflags |= wxAUI_TB_VERTICAL;
    }
    m_art->SetFlags(artflags);
}

size_t wxAuiToolBar::GetToolCount() const
{
    return m_items.size();
}

int wxAuiToolBar::GetToolIndex(int tool_id) const
{
    // this will prevent us from returning the index of the
    // first separator in the toolbar since its id is equal to -1
    if (tool_id == -1)
        return wxNOT_FOUND;

    for (size_t i = 0; i < m_items.size(); ++i)
    {
        const wxAuiToolBarItem& item = m_items[i];
        if (item.m_toolId == tool_id)
            return i;
    }

    return wxNOT_FOUND;
}

bool wxAuiToolBar::GetToolFitsByIndex(int tool_idx) const
{
    if (tool_idx < 0 || tool_idx >= (int)m_items.size())
        return false;

    if (!m_items[tool_idx].m_sizerItem)
        return false;

    wxSize client_size = GetClientSize();

    wxRect rect = m_items[tool_idx].m_sizerItem->GetRect();

    if (m_orientation == wxVERTICAL)
    {
        // take the dropdown size into account
        if (m_overflowVisible && m_overflowSizerItem)
            client_size.y -= m_overflowSizerItem->GetMinSize().y;

        if (rect.y+rect.height < client_size.y)
            return true;
    }
    else
    {
        // take the dropdown size into account
        if (m_overflowVisible && m_overflowSizerItem)
            client_size.x -= m_overflowSizerItem->GetMinSize().x;

        if (rect.x+rect.width < client_size.x)
            return true;
    }

    return false;
}

bool wxAuiToolBar::GetToolFits(int tool_id) const
{
    return GetToolFitsByIndex(GetToolIndex(tool_id));
}

wxRect wxAuiToolBar::GetToolRect(int tool_id) const
{
    const auto tool = FindTool(tool_id);
    if ((tool != m_items.end()) && tool->m_sizerItem)
    {
        return tool->m_sizerItem->GetRect();
    }

    return {};
}

bool wxAuiToolBar::GetToolBarFits() const
{
    if (m_items.size() == 0)
    {
        // empty toolbar always 'fits'
        return true;
    }

    // entire toolbar content fits if the last tool fits
    return GetToolFitsByIndex(m_items.size() - 1);
}

bool wxAuiToolBar::Realize()
{
    wxClientDC dc(this);
    if (!dc.IsOk())
        return false;

    // calculate hint sizes for both horizontal and vertical
    // in the order that leaves toolbar in correct final state
    bool retval = false;
    if (m_orientation == wxHORIZONTAL)
    {
        if (RealizeHelper(dc, false))
        {
            m_vertHintSize = GetSize();
            if (RealizeHelper(dc, true))
            {
                m_horzHintSize = GetSize();
                retval = true;
            }
        }
    }
    else
    {
        if (RealizeHelper(dc, true))
        {
            m_horzHintSize = GetSize();
            if (RealizeHelper(dc, false))
            {
                m_vertHintSize = GetSize();
                retval = true;
            }
        }
    }

    Refresh(false);
    return retval;
}

bool wxAuiToolBar::RealizeHelper(wxClientDC& dc, bool horizontal)
{
    // Remove old sizer before adding any controls in this tool bar, which are
    // elements of this sizer, to the new sizer below.
    m_sizer.reset();

    // create the new sizer to add toolbar elements to
    wxBoxSizer* sizer = new wxBoxSizer(horizontal ? wxHORIZONTAL : wxVERTICAL);

    // add gripper area
    int separatorSize = m_art->GetElementSize(wxAUI_TBART_SEPARATOR_SIZE);
    int gripperSize = m_art->GetElementSize(wxAUI_TBART_GRIPPER_SIZE);
    if (gripperSize > 0 && m_gripperVisible)
    {
        if (horizontal)
            m_gripperSizerItem = sizer->Add(gripperSize, 1, 0, wxEXPAND);
        else
            m_gripperSizerItem = sizer->Add(1, gripperSize, 0, wxEXPAND);
    }
    else
    {
        m_gripperSizerItem = nullptr;
    }

    // add "left" padding
    if (m_leftPadding > 0)
    {
        if (horizontal)
            sizer->Add(m_leftPadding, 1);
        else
            sizer->Add(1, m_leftPadding);
    }

    for (size_t i = 0, count = m_items.size(); i < count; ++i)
    {
        wxAuiToolBarItem& item = m_items[i];
        wxSizerItem* sizerItem = nullptr;

        switch (item.m_kind)
        {
            case wxITEM_LABEL:
            {
                wxSize size = m_art->GetLabelSize(dc, this, item);
                sizerItem = sizer->Add(size.x + (m_toolBorderPadding*2),
                                        size.y + (m_toolBorderPadding*2),
                                        item.m_proportion,
                                        item.m_alignment);
                if (i+1 < count)
                {
                    sizer->AddSpacer(m_toolPacking);
                }

                break;
            }

            case wxITEM_CHECK:
            case wxITEM_NORMAL:
            case wxITEM_RADIO:
            {
                wxSize size = m_art->GetToolSize(dc, this, item);
                sizerItem = sizer->Add(size.x + (m_toolBorderPadding*2),
                                        size.y + (m_toolBorderPadding*2),
                                        0,
                                        item.m_alignment);
                // add tool packing
                if (i+1 < count)
                {
                    sizer->AddSpacer(m_toolPacking);
                }

                break;
            }

            case wxITEM_SEPARATOR:
            {
                if (horizontal)
                    sizerItem = sizer->Add(separatorSize, 1, 0, wxEXPAND);
                else
                    sizerItem = sizer->Add(1, separatorSize, 0, wxEXPAND);

                // add tool packing
                if (i+1 < count)
                {
                    sizer->AddSpacer(m_toolPacking);
                }

                break;
            }

            case wxITEM_SPACER:
                if (item.m_proportion > 0)
                    sizerItem = sizer->AddStretchSpacer(item.m_proportion);
                else
                    sizerItem = sizer->Add(item.m_spacerPixels, 1);
                break;

            case wxITEM_CONTROL:
            {
                wxSizerItem* ctrl_m_sizerItem;

                wxBoxSizer* vert_sizer = new wxBoxSizer(wxVERTICAL);
                vert_sizer->AddStretchSpacer(1);
                ctrl_m_sizerItem = vert_sizer->Add(item.m_window, 0, wxEXPAND);
                vert_sizer->AddStretchSpacer(1);
                if ( (m_windowStyle & wxAUI_TB_TEXT) &&
                     m_toolTextOrientation == wxAUI_TBTOOL_TEXT_BOTTOM &&
                     !item.GetLabel().empty() )
                {
                    wxSize s = GetLabelSize(item.GetLabel());
                    vert_sizer->Add(1, s.y);
                }


                sizerItem = sizer->Add(vert_sizer, item.m_proportion, wxEXPAND);

                wxSize min_size = item.m_minSize;


                // proportional items will disappear from the toolbar if
                // their min width is not set to something really small
                if (item.m_proportion != 0)
                {
                    min_size.x = 1;
                }

                if (min_size.IsFullySpecified())
                {
                    sizerItem->SetMinSize(min_size);
                    ctrl_m_sizerItem->SetMinSize(min_size);
                }

                // add tool packing
                if (i+1 < count)
                {
                    sizer->AddSpacer(m_toolPacking);
                }
            }
        }

        item.m_sizerItem = sizerItem;
    }

    // add "right" padding
    if (m_rightPadding > 0)
    {
        if (horizontal)
            sizer->Add(m_rightPadding, 1);
        else
            sizer->Add(1, m_rightPadding);
    }

    // add drop down area
    m_overflowSizerItem = nullptr;

    if (m_windowStyle & wxAUI_TB_OVERFLOW)
    {
        int overflow_size = m_art->GetElementSize(wxAUI_TBART_OVERFLOW_SIZE);
        if (overflow_size > 0 && m_overflowVisible)
        {
            if (horizontal)
                m_overflowSizerItem = sizer->Add(overflow_size, 1, 0, wxEXPAND);
            else
                m_overflowSizerItem = sizer->Add(1, overflow_size, 0, wxEXPAND);
            m_overflowSizerItem->SetMinSize(m_overflowSizerItem->GetSize());
        }
        else
        {
            m_overflowSizerItem = nullptr;
        }
    }


    // the outside sizer helps us apply the "top" and "bottom" padding
    auto outside_sizer = std::make_unique<wxBoxSizer>(horizontal ? wxVERTICAL : wxHORIZONTAL);

    // add "top" padding
    if (m_topPadding > 0)
    {
        if (horizontal)
            outside_sizer->Add(1, m_topPadding);
        else
            outside_sizer->Add(m_topPadding, 1);
    }

    // add the sizer that contains all of the toolbar elements
    outside_sizer->Add(sizer, 1, wxEXPAND);

    // add "bottom" padding
    if (m_bottomPadding > 0)
    {
        if (horizontal)
            outside_sizer->Add(1, m_bottomPadding);
        else
            outside_sizer->Add(m_bottomPadding, 1);
    }

    m_sizer = std::move(outside_sizer);

    // calculate the rock-bottom minimum size
    for (size_t i = 0, count = m_items.size(); i < count; ++i)
    {
        wxAuiToolBarItem& item = m_items[i];
        if (item.m_sizerItem && item.m_proportion > 0 && item.m_minSize.IsFullySpecified())
            item.m_sizerItem->SetMinSize(wxSize{0, 0});
    }

    m_absoluteMinSize = m_sizer->GetMinSize();

    // reset the min sizes to what they were
    for (size_t i = 0, count = m_items.size(); i < count; ++i)
    {
        wxAuiToolBarItem& item = m_items[i];
        if (item.m_sizerItem && item.m_proportion > 0 && item.m_minSize.IsFullySpecified())
            item.m_sizerItem->SetMinSize(item.m_minSize);
    }

    // set control size
    wxSize size = m_sizer->GetMinSize();
    m_minWidth = size.x;
    m_minHeight = size.y;

    if ((m_windowStyle & wxAUI_TB_NO_AUTORESIZE) == 0)
    {
        wxSize curSize = GetClientSize();
        wxSize new_size = GetMinSize();
        if (new_size != curSize)
        {
            SetClientSize(new_size);
        }
        else
        {
            m_sizer->SetDimension(0, 0, curSize.x, curSize.y);
        }
    }
    else
    {
        wxSize curSize = GetClientSize();
        m_sizer->SetDimension(0, 0, curSize.x, curSize.y);
    }

    return true;
}

int wxAuiToolBar::GetOverflowState() const
{
    return m_overflowState;
}

wxRect wxAuiToolBar::GetOverflowRect() const
{
    wxRect cli_rect(wxPoint(0,0), GetClientSize());
    wxRect overflow_rect = m_overflowSizerItem->GetRect();
    int overflow_size = m_art->GetElementSize(wxAUI_TBART_OVERFLOW_SIZE);

    if (m_orientation == wxVERTICAL)
    {
        overflow_rect.y = cli_rect.height - overflow_size;
        overflow_rect.x = 0;
        overflow_rect.width = cli_rect.width;
        overflow_rect.height = overflow_size;
    }
    else
    {
        overflow_rect.x = cli_rect.width - overflow_size;
        overflow_rect.y = 0;
        overflow_rect.width = overflow_size;
        overflow_rect.height = cli_rect.height;
    }

    return overflow_rect;
}

wxSize wxAuiToolBar::GetLabelSize(const std::string& label)
{
    wxClientDC dc(this);

    int tx, ty;
    int textWidth = 0, textHeight = 0;

    dc.SetFont(m_font);

    // get the text height
    dc.GetTextExtent("ABCDHgj", &tx, &textHeight);

    // get the text width
    dc.GetTextExtent(label, &textWidth, &ty);

    return {textWidth, textHeight};
}

void wxAuiToolBar::DoIdleUpdate()
{
    wxEvtHandler* handler = GetEventHandler();

    bool need_refresh = false;

    for (size_t i = 0, count = m_items.size(); i < count; ++i)
    {
        wxAuiToolBarItem& item = m_items[i];

        if (item.m_toolId == -1)
            continue;

        wxUpdateUIEvent evt(item.m_toolId);
        evt.SetEventObject(this);

        if ( !item.CanBeToggled() )
            evt.DisallowCheck();

        if (handler->ProcessEvent(evt))
        {
            if (evt.GetSetEnabled())
            {
                bool is_enabled;
                if (item.m_window)
                    is_enabled = item.m_window->IsThisEnabled();
                else
                    is_enabled = (item.m_state & wxAUI_BUTTON_STATE_DISABLED) == 0;

                bool new_enabled = evt.GetEnabled();
                if (new_enabled != is_enabled)
                {
                    if (item.m_window)
                    {
                        item.m_window->Enable(new_enabled);
                    }
                    else
                    {
                        if (new_enabled)
                            item.m_state &= ~wxAUI_BUTTON_STATE_DISABLED;
                        else
                            item.m_state |= wxAUI_BUTTON_STATE_DISABLED;
                    }
                    need_refresh = true;
                }
            }

            if (evt.GetSetChecked())
            {
                // make sure we aren't checking an item that can't be
                if (item.m_kind != wxITEM_CHECK && item.m_kind != wxITEM_RADIO)
                    continue;

                bool is_checked = (item.m_state & wxAUI_BUTTON_STATE_CHECKED) != 0;
                bool new_checked = evt.GetChecked();

                if (new_checked != is_checked)
                {
                    if (new_checked)
                        item.m_state |= wxAUI_BUTTON_STATE_CHECKED;
                    else
                        item.m_state &= ~wxAUI_BUTTON_STATE_CHECKED;

                    need_refresh = true;
                }
            }

        }
    }


    if (need_refresh)
    {
        Refresh(false);
    }
}

void wxAuiToolBar::OnSize([[maybe_unused]] wxSizeEvent& evt)
{
    wxSize client_size = GetClientSize();

    if (((client_size.x >= client_size.y) && m_absoluteMinSize.x > client_size.x) ||
        ((client_size.y > client_size.x) && m_absoluteMinSize.y > client_size.y))
    {
        // hide all flexible items
        for (size_t i = 0; i < m_items.size(); ++i)
        {
            wxAuiToolBarItem& item = m_items[i];
            if (item.m_sizerItem && item.m_proportion > 0 && item.m_sizerItem->IsShown())
            {
                item.m_sizerItem->Show(false);
                item.m_sizerItem->SetProportion(0);
            }
        }
    }
    else
    {
        // show all flexible items
        for (size_t i = 0; i < m_items.size(); ++i)
        {
            wxAuiToolBarItem& item = m_items[i];
            if (item.m_sizerItem && item.m_proportion > 0 && !item.m_sizerItem->IsShown())
            {
                item.m_sizerItem->Show(true);
                item.m_sizerItem->SetProportion(item.m_proportion);
            }
        }
    }

    m_sizer->SetDimension(0, 0, client_size.x, client_size.y);

    Refresh(false);
    Update();

    // idle events aren't sent while user is resizing frame (why?),
    // but resizing toolbar here causes havoc,
    // so force idle handler to run after size handling complete
    QueueEvent(new wxIdleEvent);
}

void wxAuiToolBar::OnIdle(wxIdleEvent& evt)
{
    // if orientation doesn't match dock, fix it
    wxAuiManager* manager = wxAuiManager::GetManager(this);
    if (manager)
    {
        wxAuiPaneInfo& pane = manager->GetPane(this);
        // pane state member is public, so it might have been changed
        // without going through wxPaneInfo::SetFlag() check
        bool ok = pane.IsOk();
        wxCHECK2_MSG(!ok || IsPaneValid(m_windowStyle, pane), ok = false,
                    "window settings and pane settings are incompatible");
        if (ok)
        {
            wxOrientation newOrientation = m_orientation;
            if (pane.IsDocked())
            {
                switch (pane.dock_direction)
                {
                    case wxAUI_DOCK_TOP:
                    case wxAUI_DOCK_BOTTOM:
                        newOrientation = wxHORIZONTAL;
                        break;
                    case wxAUI_DOCK_LEFT:
                    case wxAUI_DOCK_RIGHT:
                        newOrientation = wxVERTICAL;
                        break;
                    default:
                        wxFAIL_MSG("invalid dock location value");
                }
            }
            else if (pane.IsResizable() &&
                    GetOrientation(m_windowStyle) == wxBOTH)
            {
                // changing orientation in OnSize causes havoc
                wxSize client_size = GetClientSize();

                if (client_size.x > client_size.y)
                {
                    newOrientation = wxHORIZONTAL;
                }
                else
                {
                    newOrientation = wxVERTICAL;
                }
            }
            if (newOrientation != m_orientation)
            {
                SetOrientation(newOrientation);
                Realize();
                if (newOrientation == wxHORIZONTAL)
                {
                    pane.best_size = GetHintSize(wxAUI_DOCK_TOP);
                }
                else
                {
                    pane.best_size = GetHintSize(wxAUI_DOCK_LEFT);
                }
                if (pane.IsDocked())
                {
                    pane.floating_size = wxDefaultSize;
                }
                else
                {
                    SetSize(GetParent()->GetClientSize());
                }
                manager->Update();
            }
        }
    }
    evt.Skip();
}

void wxAuiToolBar::UpdateWindowUI(unsigned int flags)
{
    if ( flags & wxUPDATE_UI_FROMIDLE )
    {
        DoIdleUpdate();
    }

    wxControl::UpdateWindowUI(flags);
}

void wxAuiToolBar::OnSysColourChanged(wxSysColourChangedEvent& event)
{
    event.Skip();

    m_art->UpdateColoursFromSystem();
    Refresh();
}

void wxAuiToolBar::OnPaint([[maybe_unused]] wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    wxRect cli_rect(wxPoint(0,0), GetClientSize());


    bool horizontal = m_orientation == wxHORIZONTAL;

    if (m_windowStyle & wxAUI_TB_PLAIN_BACKGROUND)
        m_art->DrawPlainBackground(dc, this, cli_rect);
    else
        m_art->DrawBackground(dc, this, cli_rect);

    int gripperSize = m_art->GetElementSize(wxAUI_TBART_GRIPPER_SIZE);
    int overflowSize = m_art->GetElementSize(wxAUI_TBART_OVERFLOW_SIZE);

    // paint the gripper
    if (gripperSize > 0 && m_gripperSizerItem)
    {
        wxRect gripper_rect = m_gripperSizerItem->GetRect();
        if (horizontal)
            gripper_rect.width = gripperSize;
        else
            gripper_rect.height = gripperSize;
        m_art->DrawGripper(dc, this, gripper_rect);
    }

    // calculated how far we can draw items
    int last_extent;
    if (horizontal)
        last_extent = cli_rect.width;
    else
        last_extent = cli_rect.height;
    if (m_overflowVisible)
        last_extent -= overflowSize;

    // paint each individual tool
    size_t count = m_items.size();
    for (size_t i = 0; i < count; ++i)
    {
        wxAuiToolBarItem& item = m_items[i];

        if (!item.m_sizerItem)
            continue;

        wxRect item_rect = item.m_sizerItem->GetRect();


        if ((horizontal  && item_rect.x + item_rect.width >= last_extent) ||
            (!horizontal && item_rect.y + item_rect.height >= last_extent))
        {
            break;
        }

        switch ( item.m_kind )
        {
            case wxITEM_NORMAL:
                // draw a regular or dropdown button
                if (!item.m_dropDown)
                    m_art->DrawButton(dc, this, item, item_rect);
                else
                    m_art->DrawDropDownButton(dc, this, item, item_rect);
                break;

            case wxITEM_CHECK:
            case wxITEM_RADIO:
                // draw a toggle button
                m_art->DrawButton(dc, this, item, item_rect);
                break;

            case wxITEM_SEPARATOR:
                // draw a separator
                m_art->DrawSeparator(dc, this, item_rect);
                break;

            case wxITEM_LABEL:
                // draw a text label only
                m_art->DrawLabel(dc, this, item, item_rect);
                break;

            case wxITEM_CONTROL:
                // draw the control's label
                m_art->DrawControlLabel(dc, this, item, item_rect);
                break;
        }

        // fire a signal to see if the item wants to be custom-rendered
        OnCustomRender(dc, item, item_rect);
    }

    // paint the overflow button
    if (overflowSize > 0 && m_overflowSizerItem && m_overflowVisible)
    {
        wxRect dropDownRect = GetOverflowRect();
        m_art->DrawOverflowButton(dc, this, dropDownRect, m_overflowState);
    }
}

void wxAuiToolBar::OnEraseBackground([[maybe_unused]] wxEraseEvent& evt)
{
    // empty
}

void wxAuiToolBar::OnLeftDown(wxMouseEvent& evt)
{
    wxRect cli_rect(wxPoint(0,0), GetClientSize());

    if (m_gripperSizerItem)
    {
        wxRect gripper_rect = m_gripperSizerItem->GetRect();
        if (gripper_rect.Contains(evt.GetX(), evt.GetY()))
        {
            // find aui manager
            wxAuiManager* manager = wxAuiManager::GetManager(this);
            if (!manager)
                return;

            int x_drag_offset = evt.GetX() - gripper_rect.GetX();
            int y_drag_offset = evt.GetY() - gripper_rect.GetY();

            // gripper was clicked
            manager->StartPaneDrag(this, wxPoint(x_drag_offset, y_drag_offset));
            return;
        }
    }

    if (m_overflowSizerItem && m_overflowVisible && m_art)
    {
        wxRect overflow_rect = GetOverflowRect();

        if (overflow_rect.Contains(evt.m_x, evt.m_y))
        {
            wxAuiToolBarEvent e(wxEVT_AUITOOLBAR_OVERFLOW_CLICK, -1);
            e.SetEventObject(this);
            e.SetToolId(-1);
            e.SetClickPoint(wxPoint(evt.GetX(), evt.GetY()));
            bool processed = GetEventHandler()->ProcessEvent(e);

            if (processed)
            {
                DoIdleUpdate();
            }
            else
            {
                wxAuiToolBarItemArray overflow_items;

                // add custom overflow prepend items, if any
                size_t count = m_customOverflowPrepend.size();
                for (size_t i = 0; i < count; ++i)
                    overflow_items.push_back(m_customOverflowPrepend[i]);

                // only show items that don't fit in the dropdown
                count = m_items.size();
                for (size_t i = 0; i < count; ++i)
                {
                    if (!GetToolFitsByIndex(i))
                        overflow_items.push_back(m_items[i]);
                }

                // add custom overflow append items, if any
                count = m_customOverflowAppend.size();
                for (size_t i = 0; i < count; ++i)
                    overflow_items.push_back(m_customOverflowAppend[i]);

                int res = m_art->ShowDropDown(this, overflow_items);
                m_overflowState = 0;
                Refresh(false);
                if (res != -1)
                {
                    wxCommandEvent event(wxEVT_MENU, res);
                    event.SetEventObject(this);
                    GetEventHandler()->ProcessEvent(event);
                }
            }

            return;
        }
    }

    m_dragging = false;
    m_actionPos = wxPoint(evt.GetX(), evt.GetY());
    m_actionItem = FindToolByPosition(evt.GetX(), evt.GetY());

    if (m_actionItem)
    {
        if (m_actionItem->m_state & wxAUI_BUTTON_STATE_DISABLED)
        {
            m_actionPos = wxPoint(-1,-1);
            m_actionItem = nullptr;
            return;
        }

        UnsetToolTip();

        // fire the tool dropdown event
        wxAuiToolBarEvent e(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, m_actionItem->m_toolId);
        e.SetEventObject(this);
        e.SetToolId(m_actionItem->m_toolId);

        int mouse_x = evt.GetX();
        wxRect rect = m_actionItem->m_sizerItem->GetRect();
        int dropdownWidth = m_art->GetElementSize(wxAUI_TBART_DROPDOWN_SIZE);
        const bool dropDownHit = m_actionItem->m_dropDown &&
                                 mouse_x >= (rect.x+rect.width-dropdownWidth) &&
                                 mouse_x < (rect.x+rect.width);
        e.SetDropDownClicked(dropDownHit);

        e.SetClickPoint(evt.GetPosition());
        e.SetItemRect(rect);

        // we only set the 'pressed button' state if we hit the actual button
        // and not just the drop-down
        SetPressedItem(dropDownHit ? nullptr : m_actionItem);

        if(dropDownHit)
        {
            m_actionPos = wxPoint(-1,-1);
            m_actionItem = nullptr;
        }

        if(!GetEventHandler()->ProcessEvent(e) || e.GetSkipped())
            CaptureMouse();

        // Ensure hovered item is really ok, as mouse may have moved during
        //  event processing
        wxPoint cursor_pos_after_evt = ScreenToClient(wxGetMousePosition());
        SetHoverItem(FindToolByPosition(cursor_pos_after_evt.x, cursor_pos_after_evt.y));

        DoIdleUpdate();
    }
}

void wxAuiToolBar::OnLeftUp(wxMouseEvent& evt)
{
    if (!HasCapture())
        return;

    SetPressedItem(nullptr);

    wxAuiToolBarItem* hitItem;
    hitItem = FindToolByPosition(evt.GetX(), evt.GetY());
    SetHoverItem(hitItem);

    if (m_dragging)
    {
        // TODO: it would make sense to send out an 'END_DRAG' event here,
        // otherwise a client would never know what to do with the 'BEGIN_DRAG'
        // event

        // OnCaptureLost() will be called now and this will reset all our state
        // tracking variables
        ReleaseMouse();
    }
    else
    {
        if (m_actionItem && hitItem == m_actionItem)
        {
            UnsetToolTip();

            wxCommandEvent e(wxEVT_MENU, m_actionItem->m_toolId);
            e.SetEventObject(this);

            if (hitItem->m_kind == wxITEM_CHECK || hitItem->m_kind == wxITEM_RADIO)
            {
                const bool toggle = !(m_actionItem->m_state & wxAUI_BUTTON_STATE_CHECKED);

                ToggleTool(m_actionItem->m_toolId, toggle);

                // repaint immediately
                Refresh(false);
                Update();

                e.SetInt(toggle);
            }

            // we have to release the mouse *before* sending the event, because
            // we don't know what a handler might do. It could open up a popup
            // menu for example and that would make us lose our capture anyway.

            ReleaseMouse();

            GetEventHandler()->ProcessEvent(e);

            // Ensure hovered item is really ok, as mouse may have moved during
            // event processing
            wxPoint cursor_pos_after_evt = ScreenToClient(wxGetMousePosition());
            SetHoverItem(FindToolByPosition(cursor_pos_after_evt.x, cursor_pos_after_evt.y));

            DoIdleUpdate();
        }
        else
            ReleaseMouse();
    }
}

void wxAuiToolBar::OnRightDown(wxMouseEvent& evt)
{
    if (HasCapture())
        return;

    wxRect cli_rect(wxPoint(0,0), GetClientSize());

    if (m_gripperSizerItem)
    {
        wxRect gripper_rect = m_gripperSizerItem->GetRect();
        if (gripper_rect.Contains(evt.GetX(), evt.GetY()))
            return;
    }

    if (m_overflowSizerItem && m_art)
    {
        int overflowSize = m_art->GetElementSize(wxAUI_TBART_OVERFLOW_SIZE);
        if (overflowSize > 0 &&
            evt.m_x > cli_rect.width - overflowSize &&
            evt.m_y >= 0 &&
            evt.m_y < cli_rect.height)
        {
            return;
        }
    }

    m_actionPos = wxPoint(evt.GetX(), evt.GetY());
    m_actionItem = FindToolByPosition(evt.GetX(), evt.GetY());

    if (m_actionItem && m_actionItem->m_state & wxAUI_BUTTON_STATE_DISABLED)
    {
        m_actionPos = wxPoint(-1,-1);
        m_actionItem = nullptr;
        return;
    }

    UnsetToolTip();
}

void wxAuiToolBar::OnRightUp(wxMouseEvent& evt)
{
    if (HasCapture())
        return;

    wxAuiToolBarItem* hitItem;
    hitItem = FindToolByPosition(evt.GetX(), evt.GetY());

    if (m_actionItem && hitItem == m_actionItem)
    {
        wxAuiToolBarEvent e(wxEVT_AUITOOLBAR_RIGHT_CLICK, m_actionItem->m_toolId);
        e.SetEventObject(this);
        e.SetToolId(m_actionItem->m_toolId);
        e.SetClickPoint(m_actionPos);
        GetEventHandler()->ProcessEvent(e);
        DoIdleUpdate();
    }
    else
    {
        // right-clicked on the invalid area of the toolbar
        wxAuiToolBarEvent e(wxEVT_AUITOOLBAR_RIGHT_CLICK, -1);
        e.SetEventObject(this);
        e.SetToolId(-1);
        e.SetClickPoint(m_actionPos);
        GetEventHandler()->ProcessEvent(e);
        DoIdleUpdate();
    }

    // reset member variables
    m_actionPos = wxPoint(-1,-1);
    m_actionItem = nullptr;
}

void wxAuiToolBar::OnMiddleDown(wxMouseEvent& evt)
{
    if (HasCapture())
        return;

    wxRect cli_rect(wxPoint(0,0), GetClientSize());

    if (m_gripperSizerItem)
    {
        wxRect gripper_rect = m_gripperSizerItem->GetRect();
        if (gripper_rect.Contains(evt.GetX(), evt.GetY()))
            return;
    }

    if (m_overflowSizerItem && m_art)
    {
        int overflowSize = m_art->GetElementSize(wxAUI_TBART_OVERFLOW_SIZE);
        if (overflowSize > 0 &&
            evt.m_x > cli_rect.width - overflowSize &&
            evt.m_y >= 0 &&
            evt.m_y < cli_rect.height)
        {
            return;
        }
    }

    m_actionPos = wxPoint(evt.GetX(), evt.GetY());
    m_actionItem = FindToolByPosition(evt.GetX(), evt.GetY());

    if (m_actionItem)
    {
        if (m_actionItem->m_state & wxAUI_BUTTON_STATE_DISABLED)
        {
            m_actionPos = wxPoint(-1,-1);
            m_actionItem = nullptr;
            return;
        }
    }

    UnsetToolTip();
}

void wxAuiToolBar::OnMiddleUp(wxMouseEvent& evt)
{
    if (HasCapture())
        return;

    wxAuiToolBarItem* hitItem;
    hitItem = FindToolByPosition(evt.GetX(), evt.GetY());

    if (m_actionItem && hitItem == m_actionItem)
    {
        if (hitItem->m_kind == wxITEM_NORMAL)
        {
            wxAuiToolBarEvent e(wxEVT_AUITOOLBAR_MIDDLE_CLICK, m_actionItem->m_toolId);
            e.SetEventObject(this);
            e.SetToolId(m_actionItem->m_toolId);
            e.SetClickPoint(m_actionPos);
            GetEventHandler()->ProcessEvent(e);
            DoIdleUpdate();
        }
    }

    // reset member variables
    m_actionPos = wxPoint(-1,-1);
    m_actionItem = nullptr;
}

void wxAuiToolBar::OnMotion(wxMouseEvent& evt)
{
    const bool button_pressed = HasCapture();

    // start a drag event
    if (!m_dragging && button_pressed && m_actionItem &&
        std::abs(evt.GetX() - m_actionPos.x) + std::abs(evt.GetY() - m_actionPos.y) > 5)
    {
        // TODO: sending this event only makes sense if there is an 'END_DRAG'
        // event sent sometime in the future (see OnLeftUp())
        wxAuiToolBarEvent e(wxEVT_AUITOOLBAR_BEGIN_DRAG, GetId());
        e.SetEventObject(this);
        e.SetToolId(m_actionItem->m_toolId);
        m_dragging = GetEventHandler()->ProcessEvent(e) && !e.GetSkipped();

        DoIdleUpdate();
    }

    if(m_dragging)
        return;

    wxAuiToolBarItem* hitItem = FindToolByPosition(evt.GetX(), evt.GetY());
    if(button_pressed)
    {
        // if we have a button pressed we want it to be shown in 'depressed'
        // state unless we move the mouse outside the button, then we want it
        // to show as just 'highlighted'
        if (hitItem == m_actionItem)
            SetPressedItem(m_actionItem);
        else
        {
            SetPressedItem(nullptr);
            SetHoverItem(m_actionItem);
        }
    }
    else
    {
        SetHoverItem(hitItem);

        // tooltips handling
        if ( !HasFlag(wxAUI_TB_NO_TOOLTIPS) )
        {
            wxAuiToolBarItem* packingHitItem;
            packingHitItem = FindToolByPositionWithPacking(evt.GetX(), evt.GetY());
            if ( packingHitItem )
            {
                if (packingHitItem != m_tipItem)
                {
                    m_tipItem = packingHitItem;

                    if ( !packingHitItem->m_shortHelp.empty() )
                        SetToolTip(packingHitItem->m_shortHelp);
                    else
                        UnsetToolTip();
                }
            }
            else
            {
                UnsetToolTip();
                m_tipItem = nullptr;
            }
        }

        // figure out the dropdown button state (are we hovering or pressing it?)
        RefreshOverflowState();
    }
}

void wxAuiToolBar::DoResetMouseState()
{
    RefreshOverflowState();
    SetHoverItem(nullptr);
    SetPressedItem(nullptr);

    m_tipItem = nullptr;

    // we have to reset those here, because the mouse-up handlers which do
    // it usually won't be called if we let go of a mouse button while we
    // are outside of the window
    m_actionPos = wxPoint(-1,-1);
    m_actionItem = nullptr;
}

void wxAuiToolBar::OnLeaveWindow(wxMouseEvent& evt)
{
    if(HasCapture())
    {
        evt.Skip();
        return;
    }

    DoResetMouseState();
}

void wxAuiToolBar::OnCaptureLost([[maybe_unused]] wxMouseCaptureLostEvent& evt)
{
    m_dragging = false;

    DoResetMouseState();
}

void wxAuiToolBar::OnSetCursor(wxSetCursorEvent& evt)
{
    wxCursor cursor = wxNullCursor;

    if (m_gripperSizerItem)
    {
        wxRect gripper_rect = m_gripperSizerItem->GetRect();
        if (gripper_rect.Contains(evt.GetX(), evt.GetY()))
        {
            cursor = wxCursor(wxCURSOR_SIZING);
        }
    }

    evt.SetCursor(cursor);
}

