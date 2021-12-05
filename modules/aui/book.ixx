//////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/auibook.h
// Purpose:     wxaui: wx advanced user interface - notebook
// Author:      Benjamin I. Williams
// Modified by: Jens Lody
// Created:     2006-06-28
// Copyright:   (C) Copyright 2006, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/bookctrl.h"
#include "wx/containr.h"
#include "wx/window.h"

export module WX.AUI.Book;

import WX.AUI.FrameManager;
import WX.AUI.Page;
import WX.AUI.TabArt;

import WX.Cfg.Flags;

import Utils.Geometry;

import <string>;

export
{

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

inline constexpr int wxAuiBaseTabCtrlId = 5380;

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
WX_DECLARE_OBJARRAY(wxAuiTabContainerButton, wxAuiTabContainerButtonArray);
#endif

class wxAuiTabContainer
{
public:
    wxAuiTabContainer();
    virtual ~wxAuiTabContainer() = default;

    void SetArtProvider(std::unique_ptr<wxAuiTabArt> art);
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
    
    std::unique_ptr<wxAuiTabArt> m_art;

    size_t m_tabOffset{0};
    unsigned int m_flags{0};
};

class wxAuiNotebookEvent;

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
    void SetArtProvider(std::unique_ptr<wxAuiTabArt> art);
    wxAuiTabArt* GetArtProvider() const;

    virtual void SetUniformBitmapSize(const wxSize& size);
    virtual void SetTabCtrlHeight(int height);

    bool AddPage(wxWindow* page,
                 const std::string& caption,
                 bool select = false,
                 const wxBitmap& bitmap = wxNullBitmap);

    bool InsertPage(size_t pageIdx,
                    wxWindow* page,
                    const std::string& caption,
                    bool select = false,
                    const wxBitmap& bitmap = wxNullBitmap);

    bool DeletePage(size_t page) override;
    bool RemovePage(size_t page) override;

    size_t GetPageCount() const override;
    wxWindow* GetPage(size_t pageIdx) const override;
    int GetPageIndex(wxWindow* pageWnd) const;

    bool SetPageText(size_t page, const std::string& text) override;
    std::string GetPageText(size_t pageIdx) const override;

    bool SetPageToolTip(size_t page, const std::string& text);
    std::string GetPageToolTip(size_t pageIdx) const;

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
    wxWindow *DoRemovePage([[maybe_unused]] size_t page) override { return nullptr; }

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

inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_PAGE_CLOSE( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_PAGE_CLOSED( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_PAGE_CHANGING( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_PAGE_CHANGED( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_BUTTON( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_BEGIN_DRAG( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_END_DRAG( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_CANCEL_DRAG( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_DRAG_MOTION( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_ALLOW_DND( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_BG_DCLICK( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_DRAG_DONE( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_TAB_MIDDLE_UP( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_TAB_MIDDLE_DOWN( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_TAB_RIGHT_UP( wxNewEventType() );
inline const wxEventTypeTag<wxAuiNotebookEvent> wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN( wxNewEventType() );

typedef void (wxEvtHandler::*wxAuiNotebookEventFunction)(wxAuiNotebookEvent&);


} // export
