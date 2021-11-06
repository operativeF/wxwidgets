//////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/auibook.h
// Purpose:     wxaui: wx advanced user interface - notebook
// Author:      Benjamin I. Williams
// Modified by: Jens Lody
// Created:     2006-06-28
// Copyright:   (C) Copyright 2006, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////



#ifndef _WX_AUINOTEBOOK_H_
#define _WX_AUINOTEBOOK_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/defs.h"

#if wxUSE_AUI

#include "wx/aui/tabart.h"
#include "wx/aui/framemanager.h"
#include "wx/bookctrl.h"
#include "wx/containr.h"
#include "wx/geometry/rect.h"

#include <string>

class wxAuiNotebook;


// FIXME: Bitfield
enum wxAuiNotebookOption
{
    wxAUI_NB_TOP                 = 1 << 0,
    wxAUI_NB_LEFT                = 1 << 1,  // not implemented yet
    wxAUI_NB_RIGHT               = 1 << 2,  // not implemented yet
    wxAUI_NB_BOTTOM              = 1 << 3,
    wxAUI_NB_TAB_SPLIT           = 1 << 4,
    wxAUI_NB_TAB_MOVE            = 1 << 5,
    wxAUI_NB_TAB_EXTERNAL_MOVE   = 1 << 6,
    wxAUI_NB_TAB_FIXED_WIDTH     = 1 << 7,
    wxAUI_NB_SCROLL_BUTTONS      = 1 << 8,
    wxAUI_NB_WINDOWLIST_BUTTON   = 1 << 9,
    wxAUI_NB_CLOSE_BUTTON        = 1 << 10,
    wxAUI_NB_CLOSE_ON_ACTIVE_TAB = 1 << 11,
    wxAUI_NB_CLOSE_ON_ALL_TABS   = 1 << 12,
    wxAUI_NB_MIDDLE_CLICK_CLOSE  = 1 << 13,

    wxAUI_NB_DEFAULT_STYLE = wxAUI_NB_TOP |
                             wxAUI_NB_TAB_SPLIT |
                             wxAUI_NB_TAB_MOVE |
                             wxAUI_NB_SCROLL_BUTTONS |
                             wxAUI_NB_CLOSE_ON_ACTIVE_TAB |
                             wxAUI_NB_MIDDLE_CLICK_CLOSE
};

wxALLOW_COMBINING_ENUMS(wxAuiNotebookOption, wxBorder)

constexpr int wxAuiBaseTabCtrlId = 5380;

// aui notebook event class

class wxAuiNotebookEvent : public wxBookCtrlEvent
{
public:
    wxAuiNotebookEvent(wxEventType commandType = wxEVT_NULL,
                       int winId = 0)
          : wxBookCtrlEvent{commandType, winId}
    {
    }

	wxAuiNotebookEvent& operator=(const wxAuiNotebookEvent&) = delete;

    wxEvent *Clone() const override { return new wxAuiNotebookEvent(*this); }

    void SetDragSource(wxAuiNotebook* s) { m_dragSource = s; }
    wxAuiNotebook* GetDragSource() const { return m_dragSource; }

private:
    wxAuiNotebook* m_dragSource{nullptr};

#ifndef SWIG
public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
#endif
};


struct wxAuiNotebookPage
{
    std::string caption;           // caption displayed on the tab
    wxString tooltip;              // tooltip displayed when hovering over tab title
    wxBitmap bitmap;               // tab's bitmap
    wxRect rect;                   // tab's hit rectangle
    wxWindow* window{nullptr};     // page's associated window
    bool active{false};            // true if the page is currently active
    bool hover{false};             // true if mouse hovering over tab
};

struct wxAuiTabContainerButton
{
    wxBitmap bitmap;     // button's hover bitmap
    wxBitmap disBitmap;  // button's disabled bitmap
    wxRect rect;         // button's hit rectangle
    unsigned int curState{}; // current state (normal, hover, pressed, etc.)
    int id{};            // button's id
    int location{};      // buttons location (wxLEFT, wxRIGHT, or wxCENTER)
};


#ifndef SWIG
WX_DECLARE_USER_EXPORTED_OBJARRAY(wxAuiNotebookPage, wxAuiNotebookPageArray, WXDLLIMPEXP_AUI);
WX_DECLARE_USER_EXPORTED_OBJARRAY(wxAuiTabContainerButton, wxAuiTabContainerButtonArray, WXDLLIMPEXP_AUI);
#endif


class wxAuiTabContainer
{
public:
    wxAuiTabContainer();
    virtual ~wxAuiTabContainer();

    void SetArtProvider(wxAuiTabArt* art);
    wxAuiTabArt* GetArtProvider() const;

    void SetFlags(unsigned int flags);
    unsigned int GetFlags() const;

    bool AddPage(wxWindow* page, const wxAuiNotebookPage& info);
    bool InsertPage(wxWindow* page, const wxAuiNotebookPage& info, size_t idx);
    bool MovePage(wxWindow* page, size_t newIdx);
    bool RemovePage(wxWindow* page);
    bool SetActivePage(wxWindow* page);
    bool SetActivePage(size_t page);
    void SetNoneActive();
    int GetActivePage() const;
    bool TabHitTest(int x, int y, wxWindow** hit) const;
    bool ButtonHitTest(int x, int y, wxAuiTabContainerButton** hit) const;
    wxWindow* GetWindowFromIdx(size_t idx) const;
    int GetIdxFromWindow(wxWindow* page) const;
    size_t GetPageCount() const;
    wxAuiNotebookPage& GetPage(size_t idx);
    const wxAuiNotebookPage& GetPage(size_t idx) const;
    wxAuiNotebookPageArray& GetPages();
    void SetNormalFont(const wxFont& normalFont);
    void SetSelectedFont(const wxFont& selectedFont);
    void SetMeasuringFont(const wxFont& measuringFont);
    void SetColour(const wxColour& colour);
    void SetActiveColour(const wxColour& colour);
    void DoShowHide();
    void SetRect(const wxRect& rect);

    void RemoveButton(int id);
    void AddButton(int id,
                   int location,
                   const wxBitmap& normalBitmap = wxNullBitmap,
                   const wxBitmap& disabledBitmap = wxNullBitmap);

    size_t GetTabOffset() const;
    void SetTabOffset(size_t offset);

    // Is the tab visible?
    bool IsTabVisible(int tabPage, int tabOffset, wxDC* dc, wxWindow* wnd);

    // Make the tab visible if it wasn't already
    void MakeTabVisible(int tabPage, wxWindow* win);

protected:
    virtual void Render(wxDC* dc, wxWindow* wnd);

    wxAuiNotebookPageArray m_pages;
    
    wxAuiTabContainerButtonArray m_buttons;
    wxAuiTabContainerButtonArray m_tabCloseButtons;
    
    wxRect m_rect;
    
    wxAuiTabArt* m_art;

    size_t m_tabOffset;
    unsigned int m_flags;
};



class wxAuiTabCtrl : public wxControl,
                                     public wxAuiTabContainer
{
public:

    wxAuiTabCtrl(wxWindow* parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = 0);

    bool IsDragging() const { return m_isDragging; }

protected:
    // choose the default border for this window
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    void OnPaint(wxPaintEvent& evt);
    void OnEraseBackground(wxEraseEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnLeftDown(wxMouseEvent& evt);
    void OnLeftDClick(wxMouseEvent& evt);
    void OnLeftUp(wxMouseEvent& evt);
    void OnMiddleDown(wxMouseEvent& evt);
    void OnMiddleUp(wxMouseEvent& evt);
    void OnRightDown(wxMouseEvent& evt);
    void OnRightUp(wxMouseEvent& evt);
    void OnMotion(wxMouseEvent& evt);
    void OnLeaveWindow(wxMouseEvent& evt);
    void OnButton(wxAuiNotebookEvent& evt);
    void OnSetFocus(wxFocusEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnCaptureLost(wxMouseCaptureLostEvent& evt);
    void OnSysColourChanged(wxSysColourChangedEvent& event);

protected:

    wxPoint m_clickPt;

    wxWindow* m_clickTab;
    wxAuiTabContainerButton* m_hoverButton;
    wxAuiTabContainerButton* m_pressedButton;

    bool m_isDragging;

    void SetHoverTab(wxWindow* wnd);

#ifndef SWIG
    wxDECLARE_CLASS(wxAuiTabCtrl);
    wxDECLARE_EVENT_TABLE();
#endif
};




class wxAuiNotebook : public wxNavigationEnabled<wxBookCtrlBase>
{

public:

    wxAuiNotebook() = default;

    wxAuiNotebook(wxWindow* parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  unsigned int style = wxAUI_NB_DEFAULT_STYLE)
    {
        Create(parent, id, pos, size, style);
    }

    ~wxAuiNotebook();

    [[maybe_unused]] bool Create(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0);

    void SetWindowStyleFlag(unsigned int style) override;
    void SetArtProvider(wxAuiTabArt* art);
    wxAuiTabArt* GetArtProvider() const;

    virtual void SetUniformBitmapSize(const wxSize& size);
    virtual void SetTabCtrlHeight(int height);

    bool AddPage(wxWindow* page,
                 const wxString& caption,
                 bool select = false,
                 const wxBitmap& bitmap = wxNullBitmap);

    bool InsertPage(size_t pageIdx,
                    wxWindow* page,
                    const wxString& caption,
                    bool select = false,
                    const wxBitmap& bitmap = wxNullBitmap);

    bool DeletePage(size_t page) override;
    bool RemovePage(size_t page) override;

    size_t GetPageCount() const override;
    wxWindow* GetPage(size_t pageIdx) const override;
    int GetPageIndex(wxWindow* pageWnd) const;

    bool SetPageText(size_t page, const std::string& text) override;
    std::string GetPageText(size_t pageIdx) const override;

    bool SetPageToolTip(size_t page, const wxString& text);
    wxString GetPageToolTip(size_t pageIdx) const;

    bool SetPageBitmap(size_t page, const wxBitmap& bitmap);
    wxBitmap GetPageBitmap(size_t pageIdx) const;

    int SetSelection(size_t newPage) override;
    int GetSelection() const override;

    virtual void Split(size_t page, int direction);

    const wxAuiManager& GetAuiManager() const { return m_mgr; }

    // Sets the normal font
    void SetNormalFont(const wxFont& font);

    // Sets the selected tab font
    void SetSelectedFont(const wxFont& font);

    // Sets the measuring font
    void SetMeasuringFont(const wxFont& font);

    // Sets the tab font
    bool SetFont(const wxFont& font) override;

    // Gets the tab control height
    int GetTabCtrlHeight() const;

    // Gets the height of the notebook for a given page height
    int GetHeightForPageHeight(int pageHeight);

    // Shows the window menu
    bool ShowWindowMenu();

    // we do have multiple pages
    bool HasMultiplePages() const override { return true; }

    // we don't want focus for ourselves
    // virtual bool AcceptsFocus() const { return false; }

    //wxBookCtrlBase functions

    void SetPageSize (const wxSize &size) override;
    int  HitTest (const wxPoint &pt, unsigned int* flags=nullptr) const override;

    int GetPageImage(size_t n) const override;
    bool SetPageImage(size_t n, int imageId) override;

    int ChangeSelection(size_t n) override;

    bool AddPage(wxWindow *page, const std::string &text, bool select,
                         int imageId) override;
    bool DeleteAllPages() override;
    bool InsertPage(size_t index, wxWindow *page, const std::string &text,
                            bool select, int imageId) override;

    wxSize DoGetBestSize() const override;

    wxAuiTabCtrl* GetTabCtrlFromPoint(const wxPoint& pt);
    wxAuiTabCtrl* GetActiveTabCtrl();
    bool FindTab(wxWindow* page, wxAuiTabCtrl** ctrl, int* idx);

protected:
    // choose the default border for this window
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    // Redo sizing after thawing
    void DoThaw() override;

    // these can be overridden

    // update the height, return true if it was done or false if the new height
    // calculated by CalculateTabCtrlHeight() is the same as the old one
    virtual bool UpdateTabCtrlHeight();

    virtual int CalculateTabCtrlHeight();
    virtual wxSize CalculateNewSplitSize();

    // remove the page and return a pointer to it
    wxWindow *DoRemovePage(size_t WXUNUSED(page)) override { return nullptr; }

    //A general selection function
    virtual int DoModifySelection(size_t n, bool events);

protected:

    void DoSizing();
    void InitNotebook(unsigned int style);
    wxWindow* GetTabFrameFromTabCtrl(wxWindow* tabCtrl);
    void RemoveEmptyTabFrames();
    void UpdateHintWindowSize();

protected:

    void OnChildFocusNotebook(wxChildFocusEvent& evt);
    void OnRender(wxAuiManagerEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnTabClicked(wxAuiNotebookEvent& evt);
    void OnTabBeginDrag(wxAuiNotebookEvent& evt);
    void OnTabDragMotion(wxAuiNotebookEvent& evt);
    void OnTabEndDrag(wxAuiNotebookEvent& evt);
    void OnTabCancelDrag(wxAuiNotebookEvent& evt);
    void OnTabButton(wxAuiNotebookEvent& evt);
    void OnTabMiddleDown(wxAuiNotebookEvent& evt);
    void OnTabMiddleUp(wxAuiNotebookEvent& evt);
    void OnTabRightDown(wxAuiNotebookEvent& evt);
    void OnTabRightUp(wxAuiNotebookEvent& evt);
    void OnTabBgDClick(wxAuiNotebookEvent& evt);
    void OnNavigationKeyNotebook(wxNavigationKeyEvent& event);
    void OnSysColourChanged(wxSysColourChangedEvent& event);

    // set selection to the given window (which must be non-NULL and be one of
    // our pages, otherwise an assert is raised)
    void SetSelectionToWindow(wxWindow *win);
    void SetSelectionToPage(const wxAuiNotebookPage& page)
    {
        SetSelectionToWindow(page.window);
    }

protected:
    wxAuiManager m_mgr;
    wxAuiTabContainer m_tabs;

    wxFont m_selectedFont;
    wxFont m_normalFont;

    wxSize m_requestedBmpSize{wxDefaultSize};

    wxWindow* m_dummyWnd{nullptr};

    unsigned int m_flags;

    int m_tabCtrlHeight{FromDIP(20)};
    int m_curPage{-1};
    int m_tabIdCounter{wxAuiBaseTabCtrlId};
    int m_lastDragX;
    int m_requestedTabCtrlHeight{-1};

#ifndef SWIG
    wxDECLARE_CLASS(wxAuiNotebook);
    wxDECLARE_EVENT_TABLE();
#endif
};




// wx event machinery

#ifndef SWIG

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_PAGE_CHANGING, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_PAGE_CLOSED, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_BUTTON, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_BEGIN_DRAG, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_END_DRAG, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_DRAG_MOTION, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_ALLOW_DND, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_TAB_MIDDLE_DOWN, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_TAB_MIDDLE_UP, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_TAB_RIGHT_UP, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_DRAG_DONE, wxAuiNotebookEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_AUI, wxEVT_AUINOTEBOOK_BG_DCLICK, wxAuiNotebookEvent);

typedef void (wxEvtHandler::*wxAuiNotebookEventFunction)(wxAuiNotebookEvent&);

#define wxAuiNotebookEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxAuiNotebookEventFunction, func)

#define EVT_AUINOTEBOOK_PAGE_CLOSE(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_PAGE_CLOSE, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_PAGE_CLOSED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_PAGE_CLOSED, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_PAGE_CHANGED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_PAGE_CHANGED, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_PAGE_CHANGING(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_PAGE_CHANGING, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_BUTTON(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_BUTTON, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_BEGIN_DRAG(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_BEGIN_DRAG, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_END_DRAG(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_END_DRAG, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_DRAG_MOTION(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_DRAG_MOTION, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_ALLOW_DND(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_ALLOW_DND, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_DRAG_DONE(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_DRAG_DONE, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_TAB_MIDDLE_DOWN(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_TAB_MIDDLE_DOWN, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_TAB_MIDDLE_UP(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_TAB_MIDDLE_UP, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_TAB_RIGHT_DOWN(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_TAB_RIGHT_UP(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_TAB_RIGHT_UP, winid, wxAuiNotebookEventHandler(fn))
#define EVT_AUINOTEBOOK_BG_DCLICK(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_AUINOTEBOOK_BG_DCLICK, winid, wxAuiNotebookEventHandler(fn))
#else

// wxpython/swig event work
%constant wxEventType wxEVT_AUINOTEBOOK_PAGE_CLOSE;
%constant wxEventType wxEVT_AUINOTEBOOK_PAGE_CLOSED;
%constant wxEventType wxEVT_AUINOTEBOOK_PAGE_CHANGED;
%constant wxEventType wxEVT_AUINOTEBOOK_PAGE_CHANGING;
%constant wxEventType wxEVT_AUINOTEBOOK_BUTTON;
%constant wxEventType wxEVT_AUINOTEBOOK_BEGIN_DRAG;
%constant wxEventType wxEVT_AUINOTEBOOK_END_DRAG;
%constant wxEventType wxEVT_AUINOTEBOOK_DRAG_MOTION;
%constant wxEventType wxEVT_AUINOTEBOOK_ALLOW_DND;
%constant wxEventType wxEVT_AUINOTEBOOK_DRAG_DONE;
%constant wxEventType wxEVT_AUINOTEBOOK_TAB_MIDDLE_DOWN;
%constant wxEventType wxEVT_AUINOTEBOOK_TAB_MIDDLE_UP;
%constant wxEventType wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN;
%constant wxEventType wxEVT_AUINOTEBOOK_TAB_RIGHT_UP;
%constant wxEventType wxEVT_AUINOTEBOOK_BG_DCLICK;

%pythoncode {
    EVT_AUINOTEBOOK_PAGE_CLOSE = wx.PyEventBinder( wxEVT_AUINOTEBOOK_PAGE_CLOSE, 1 )
    EVT_AUINOTEBOOK_PAGE_CLOSED = wx.PyEventBinder( wxEVT_AUINOTEBOOK_PAGE_CLOSED, 1 )
    EVT_AUINOTEBOOK_PAGE_CHANGED = wx.PyEventBinder( wxEVT_AUINOTEBOOK_PAGE_CHANGED, 1 )
    EVT_AUINOTEBOOK_PAGE_CHANGING = wx.PyEventBinder( wxEVT_AUINOTEBOOK_PAGE_CHANGING, 1 )
    EVT_AUINOTEBOOK_BUTTON = wx.PyEventBinder( wxEVT_AUINOTEBOOK_BUTTON, 1 )
    EVT_AUINOTEBOOK_BEGIN_DRAG = wx.PyEventBinder( wxEVT_AUINOTEBOOK_BEGIN_DRAG, 1 )
    EVT_AUINOTEBOOK_END_DRAG = wx.PyEventBinder( wxEVT_AUINOTEBOOK_END_DRAG, 1 )
    EVT_AUINOTEBOOK_DRAG_MOTION = wx.PyEventBinder( wxEVT_AUINOTEBOOK_DRAG_MOTION, 1 )
    EVT_AUINOTEBOOK_ALLOW_DND = wx.PyEventBinder( wxEVT_AUINOTEBOOK_ALLOW_DND, 1 )
    EVT_AUINOTEBOOK_DRAG_DONE = wx.PyEventBinder( wxEVT_AUINOTEBOOK_DRAG_DONE, 1 )
    EVT__AUINOTEBOOK_TAB_MIDDLE_DOWN = wx.PyEventBinder( wxEVT_AUINOTEBOOK_TAB_MIDDLE_DOWN, 1 )
    EVT__AUINOTEBOOK_TAB_MIDDLE_UP = wx.PyEventBinder( wxEVT_AUINOTEBOOK_TAB_MIDDLE_UP, 1 )
    EVT__AUINOTEBOOK_TAB_RIGHT_DOWN = wx.PyEventBinder( wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN, 1 )
    EVT__AUINOTEBOOK_TAB_RIGHT_UP = wx.PyEventBinder( wxEVT_AUINOTEBOOK_TAB_RIGHT_UP, 1 )
    EVT_AUINOTEBOOK_BG_DCLICK = wx.PyEventBinder( wxEVT_AUINOTEBOOK_BG_DCLICK, 1 )
}
#endif

#endif  // wxUSE_AUI
#endif  // _WX_AUINOTEBOOK_H_
