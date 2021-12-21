///////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/auibar.ixx
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by: Thomas Figueroa
// Created:     2008-08-04
// Copyright:   (C) Copyright 2005, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/control.h"
#include "wx/dcclient.h"
#include "wx/event.h"
#include "wx/window.h"

#ifdef __WXMAC__
#include "wx/osx/private.h"
#endif

#include <memory>

export module WX.AUI.AUIBar;

import WX.AUI.PaneInfo;
import WX.AUI.ToolBarArt.Base;
import WX.AUI.ToolBarArt;
import WX.AUI.ToolBar.Item;
import WX.AUI.Flags;

import WX.Core.Sizer;

import <string>;

export
{

// aui toolbar event class

class wxAuiToolBarEvent : public wxNotifyEvent
{
public:
    wxAuiToolBarEvent(wxEventType commandType = wxEVT_NULL,
                      int winId = 0)
          : wxNotifyEvent{commandType, winId}
    {
    }

	wxAuiToolBarEvent& operator=(const wxAuiToolBarEvent&) = delete;

    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxAuiToolBarEvent>(*this); }

    bool IsDropDownClicked() const  { return m_isDropdownClicked; }
    void SetDropDownClicked(bool c) { m_isDropdownClicked = c;    }

    wxPoint GetClickPoint() const        { return m_clickPt; }
    void SetClickPoint(const wxPoint& p) { m_clickPt = p;    }

    wxRect GetItemRect() const        { return m_rect; }
    void SetItemRect(const wxRect& r) { m_rect = r;    }

    int GetToolId() const  { return m_toolId; }
    void SetToolId(int toolId) { m_toolId = toolId; }

private:
    wxPoint m_clickPt{-1, -1};
    wxRect m_rect{-1, -1, 0, 0};
    int m_toolId{-1};
    bool m_isDropdownClicked{false};
};

inline const wxEventTypeTag<wxAuiToolBarEvent> wxEVT_AUITOOLBAR_TOOL_DROPDOWN( wxNewEventType() );
inline const wxEventTypeTag<wxAuiToolBarEvent> wxEVT_AUITOOLBAR_OVERFLOW_CLICK( wxNewEventType() );
inline const wxEventTypeTag<wxAuiToolBarEvent> wxEVT_AUITOOLBAR_RIGHT_CLICK( wxNewEventType() );
inline const wxEventTypeTag<wxAuiToolBarEvent> wxEVT_AUITOOLBAR_MIDDLE_CLICK( wxNewEventType() );
inline const wxEventTypeTag<wxAuiToolBarEvent> wxEVT_AUITOOLBAR_BEGIN_DRAG( wxNewEventType() );

typedef void (wxEvtHandler::*wxAuiToolBarEventFunction)(wxAuiToolBarEvent&);

class wxAuiToolBar : public wxControl
{
public:
    wxAuiToolBar()
        : m_art{std::make_unique<wxAuiDefaultToolBarArt>()},
          m_sizer{std::make_unique<wxBoxSizer>(wxHORIZONTAL)}
    {
        m_toolPacking = FromDIP(2);
        m_toolBorderPadding = FromDIP(3);
    }

    wxAuiToolBar(wxWindow* parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = wxAUI_TB_DEFAULT_STYLE)
        : m_art{std::make_unique<wxAuiDefaultToolBarArt>()},
          m_sizer{std::make_unique<wxBoxSizer>(wxHORIZONTAL)}
    {
        m_toolPacking = FromDIP(2);
        m_toolBorderPadding = FromDIP(3);
        
        Create(parent, id, pos, size, style);
    }

    [[maybe_unused]] bool Create(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxAUI_TB_DEFAULT_STYLE);

    void SetWindowStyleFlag(unsigned int style) override;

    void SetArtProvider(std::unique_ptr<wxAuiToolBarArt> art);
    wxAuiToolBarArt* GetArtProvider() const;

    bool SetFont(const wxFont& font) override;


    wxAuiToolBarItem* AddTool(int toolId,
                 const std::string& label,
                 const wxBitmap& bitmap,
                 const std::string& shortHelpString = {},
                 wxItemKind kind = wxITEM_NORMAL);

    wxAuiToolBarItem* AddTool(int toolId,
                 const std::string& label,
                 const wxBitmap& bitmap,
                 const wxBitmap& disabledBitmap,
                 wxItemKind kind,
                 const std::string& shortHelpString,
                 const std::string& longHelpString,
                 wxObject* clientData);

    wxAuiToolBarItem* AddTool(int toolId,
                 const wxBitmap& bitmap,
                 const wxBitmap& disabledBitmap,
                 bool toggle = false,
                 wxObject* clientData = nullptr,
                 const std::string& shortHelpString = {},
                 const std::string& longHelpString = {})
    {
        return AddTool(toolId,
                {},
                bitmap,
                disabledBitmap,
                toggle ? wxITEM_CHECK : wxITEM_NORMAL,
                shortHelpString,
                longHelpString,
                clientData);
    }

    wxAuiToolBarItem* AddLabel(int toolId,
                  const std::string& label = {},
                  const int width = -1);
    wxAuiToolBarItem* AddControl(wxControl* control,
                    const std::string& label = {});
    wxAuiToolBarItem* AddSeparator();
    wxAuiToolBarItem* AddSpacer(int pixels);
    wxAuiToolBarItem* AddStretchSpacer(int proportion = 1);

    bool Realize();

    wxControl* FindControl(int windowId);
    wxAuiToolBarItem* FindToolByPosition(wxCoord x, wxCoord y);

    auto FindTool(int toolId)
    {
        return std::ranges::find_if(m_items, [toolId](auto item){ return item.m_toolId == toolId; });
    }

    const auto FindTool(int toolId) const
    {
        return std::ranges::find_if(m_items, [toolId](auto item) { return item.m_toolId == toolId; });
    }

    void ClearTools() { Clear() ; }
    void Clear();

    bool DestroyTool(int toolId);
    bool DestroyToolByIndex(int idx);

    // Note that these methods do _not_ delete the associated control, if any.
    // Use DestroyTool() or DestroyToolByIndex() if this is wanted.
    bool DeleteTool(int toolId);
    bool DeleteByIndex(int toolId);

    size_t GetToolCount() const;
    int GetToolPos(int toolId) const { return GetToolIndex(toolId); }
    int GetToolIndex(int toolId) const;
    bool GetToolFits(int toolId) const;
    wxRect GetToolRect(int toolId) const;
    bool GetToolFitsByIndex(int toolId) const;
    bool GetToolBarFits() const;

    void SetMargins(const wxSize& size) { SetMargins(size.x, size.x, size.y, size.y); }
    void SetMargins(int x, int y) { SetMargins(x, x, y, y); }
    void SetMargins(int left, int right, int top, int bottom);

    void SetToolBitmapSize(const wxSize& size);
    wxSize GetToolBitmapSize() const;

    bool GetOverflowVisible() const;
    void SetOverflowVisible(bool visible);

    bool GetGripperVisible() const;
    void SetGripperVisible(bool visible);

    void ToggleTool(int toolId, bool state);
    bool GetToolToggled(int toolId) const;

    void EnableTool(int toolId, bool state);
    bool GetToolEnabled(int toolId) const;

    void SetToolDropDown(int toolId, bool dropdown);
    bool GetToolDropDown(int toolId) const;

    void SetToolBorderPadding(int padding);
    int  GetToolBorderPadding() const;

    void SetToolTextOrientation(int orientation);
    int  GetToolTextOrientation() const;

    void SetToolPacking(int packing);
    int  GetToolPacking() const;

    void SetToolProportion(int toolId, int proportion);
    int  GetToolProportion(int toolId) const;

    void SetToolSeparation(int separation);
    int GetToolSeparation() const;

    void SetToolSticky(int toolId, bool sticky);
    bool GetToolSticky(int toolId) const;

    std::string GetToolLabel(int toolId) const;
    void SetToolLabel(int toolId, const std::string& label);

    wxBitmap GetToolBitmap(int toolId) const;
    void SetToolBitmap(int toolId, const wxBitmap& bitmap);

    std::string GetToolShortHelp(int toolId) const;
    void SetToolShortHelp(int toolId, const std::string& helpString);

    std::string GetToolLongHelp(int toolId) const;
    void SetToolLongHelp(int toolId, const std::string& helpString);

    void SetCustomOverflowItems(const wxAuiToolBarItemArray& prepend,
                                const wxAuiToolBarItemArray& append);

    // get size of hint rectangle for a particular dock location
    wxSize GetHintSize(int dockDirection) const;
    bool IsPaneValid(const wxAuiPaneInfo& pane) const;

    // Override to call DoIdleUpdate().
    void UpdateWindowUI(unsigned int flags = wxUPDATE_UI_NONE) override;

protected:
    virtual void OnCustomRender([[maybe_unused]] wxDC& dc,
                                [[maybe_unused]] const wxAuiToolBarItem& item,
                                [[maybe_unused]] const wxRect& rect) { }

protected:

    void DoIdleUpdate();
    void SetOrientation(int orientation);
    void SetHoverItem(wxAuiToolBarItem* item);
    void SetPressedItem(wxAuiToolBarItem* item);
    void RefreshOverflowState();

    int GetOverflowState() const;
    wxRect GetOverflowRect() const;
    wxSize GetLabelSize(const std::string& label);
    wxAuiToolBarItem* FindToolByPositionWithPacking(wxCoord x, wxCoord y);

protected: // handlers

    void OnSize(wxSizeEvent& evt);
    void OnIdle(wxIdleEvent& evt);
    void OnPaint(wxPaintEvent& evt);
    void OnEraseBackground(wxEraseEvent& evt);
    void OnLeftDown(wxMouseEvent& evt);
    void OnLeftUp(wxMouseEvent& evt);
    void OnRightDown(wxMouseEvent& evt);
    void OnRightUp(wxMouseEvent& evt);
    void OnMiddleDown(wxMouseEvent& evt);
    void OnMiddleUp(wxMouseEvent& evt);
    void OnMotion(wxMouseEvent& evt);
    void OnLeaveWindow(wxMouseEvent& evt);
    void OnCaptureLost(wxMouseCaptureLostEvent& evt);
    void OnSetCursor(wxSetCursorEvent& evt);
    void OnSysColourChanged(wxSysColourChangedEvent& event);

protected:
    wxAuiToolBarItemArray m_items;      // array of toolbar items
    std::unique_ptr<wxAuiToolBarArt> m_art;             // art provider
    std::unique_ptr<wxBoxSizer> m_sizer;                // main sizer for toolbar
    wxAuiToolBarItem* m_actionItem{nullptr};    // item that's being acted upon (pressed)
    wxAuiToolBarItem* m_tipItem{nullptr};       // item that has its tooltip shown
    wxBitmap m_bitmap;                  // double-buffer bitmap
    wxSizerItem* m_gripperSizerItem{nullptr};
    wxSizerItem* m_overflowSizerItem{nullptr};
    wxSize m_absoluteMinSize;
    wxPoint m_actionPos{wxDefaultPosition}; // position of left-mouse down
    wxAuiToolBarItemArray m_customOverflowPrepend;
    wxAuiToolBarItemArray m_customOverflowAppend;

    int m_buttonWidth{-1};
    int m_buttonHeight{-1};
    int m_sizerElementCount{0};
    int m_leftPadding{0};
    int m_rightPadding{0};
    int m_topPadding{0};
    int m_bottomPadding{0};
    int m_toolPacking{0};
    int m_toolBorderPadding{0};
    int m_toolTextOrientation{wxAUI_TBTOOL_TEXT_BOTTOM};
    int m_overflowState{0};
    bool m_dragging{false};
    bool m_gripperVisible{false};
    bool m_overflowVisible{false};

    bool RealizeHelper(wxClientDC& dc, bool horizontal);
    static bool IsPaneValid(unsigned int style, const wxAuiPaneInfo& pane);
    bool IsPaneValid(unsigned int style) const;
    void SetArtFlags() const;
    wxOrientation m_orientation{wxHORIZONTAL};
    wxSize m_horzHintSize;
    wxSize m_vertHintSize;

private:
    // Common part of OnLeaveWindow() and OnCaptureLost().
    void DoResetMouseState();

    wxDECLARE_EVENT_TABLE();
};

} // export
