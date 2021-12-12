///////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/framemanager.h
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by:
// Created:     2005-05-17
// Copyright:   (C) Copyright 2005, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/timer.h"
#include "wx/sizer.h"
#include "wx/bitmap.h"
#include "wx/window.h"

export module WX.AUI.FrameManager;

import WX.Cfg.Flags;

import WX.AUI.Flags;
import WX.AUI.DockInfo;
import WX.AUI.PaneInfo;

import Utils.Geometry;

import <vector>;

export
{

enum wxAuiManagerOption
{
    wxAUI_MGR_ALLOW_FLOATING           = 1 << 0,
    wxAUI_MGR_ALLOW_ACTIVE_PANE        = 1 << 1,
    wxAUI_MGR_TRANSPARENT_DRAG         = 1 << 2,
    wxAUI_MGR_TRANSPARENT_HINT         = 1 << 3,
    wxAUI_MGR_VENETIAN_BLINDS_HINT     = 1 << 4,
    wxAUI_MGR_RECTANGLE_HINT           = 1 << 5,
    wxAUI_MGR_HINT_FADE                = 1 << 6,
    wxAUI_MGR_NO_VENETIAN_BLINDS_FADE  = 1 << 7,
    wxAUI_MGR_LIVE_RESIZE              = 1 << 8,

    wxAUI_MGR_DEFAULT = wxAUI_MGR_ALLOW_FLOATING |
                        wxAUI_MGR_TRANSPARENT_HINT |
                        wxAUI_MGR_HINT_FADE |
                        wxAUI_MGR_NO_VENETIAN_BLINDS_FADE
};

enum wxAuiPaneDockArtGradients
{
    wxAUI_GRADIENT_NONE = 0,
    wxAUI_GRADIENT_VERTICAL = 1,
    wxAUI_GRADIENT_HORIZONTAL = 2
};

enum wxAuiPaneButtonState
{
    wxAUI_BUTTON_STATE_NORMAL   = 0,
    wxAUI_BUTTON_STATE_HOVER    = 1 << 1,
    wxAUI_BUTTON_STATE_PRESSED  = 1 << 2,
    wxAUI_BUTTON_STATE_DISABLED = 1 << 3,
    wxAUI_BUTTON_STATE_HIDDEN   = 1 << 4,
    wxAUI_BUTTON_STATE_CHECKED  = 1 << 5
};

enum wxAuiButtonId
{
    wxAUI_BUTTON_CLOSE = 101,
    wxAUI_BUTTON_MAXIMIZE_RESTORE = 102,
    wxAUI_BUTTON_MINIMIZE = 103,
    wxAUI_BUTTON_PIN = 104,
    wxAUI_BUTTON_OPTIONS = 105,
    wxAUI_BUTTON_WINDOWLIST = 106,
    wxAUI_BUTTON_LEFT = 107,
    wxAUI_BUTTON_RIGHT = 108,
    wxAUI_BUTTON_UP = 109,
    wxAUI_BUTTON_DOWN = 110,
    wxAUI_BUTTON_CUSTOM1 = 201,
    wxAUI_BUTTON_CUSTOM2 = 202,
    wxAUI_BUTTON_CUSTOM3 = 203
};

enum wxAuiPaneInsertLevel
{
    wxAUI_INSERT_PANE = 0,
    wxAUI_INSERT_ROW = 1,
    wxAUI_INSERT_DOCK = 2
};

// forwards and array declarations
class wxAuiDockArt;
class wxAuiManagerEvent;

struct wxAuiDockUIPart;

using wxAuiDockUIPartArray  = std::vector<wxAuiDockUIPart>;

class wxAuiFloatingFrame;

class wxAuiManager : public wxEvtHandler
{
    friend class wxAuiFloatingFrame;

public:
    using PaneInfoIter = wxAuiPaneInfoArray::iterator;

    wxAuiManager(wxWindow* managedWnd = nullptr,
                   unsigned int flags = wxAUI_MGR_DEFAULT);
    ~wxAuiManager();
    void UnInit();

    void SetFlags(unsigned int flags);
    unsigned int GetFlags() const;

    static bool AlwaysUsesLiveResize();
    bool HasLiveResize() const;

    void SetManagedWindow(wxWindow* managedWnd);
    wxWindow* GetManagedWindow() const;

    static wxAuiManager* GetManager(wxWindow* window);

    void SetArtProvider(std::unique_ptr<wxAuiDockArt> artProvider);
    wxAuiDockArt* GetArtProvider() const;

    PaneInfoIter GetPane(wxWindow* window);
    PaneInfoIter GetPane(std::string_view name);
    wxAuiPaneInfoArray& GetAllPanes();

    bool AddPane(wxWindow* window,
                 const wxAuiPaneInfo& paneInfo);

    bool AddPane(wxWindow* window,
                 const wxAuiPaneInfo& paneInfo,
                 const wxPoint& dropPos);

    bool AddPane(wxWindow* window,
                 int direction = wxLEFT,
                 const std::string& caption = {});

    bool InsertPane(wxWindow* window,
                 const wxAuiPaneInfo& insertLocation,
                 int insertLevel = wxAUI_INSERT_PANE);

    bool DetachPane(wxWindow* window);

    void Update();

    std::string SavePaneInfo(const wxAuiPaneInfo& pane);
    void LoadPaneInfo(std::string panePart, wxAuiPaneInfo &pane);
    std::string SavePerspective();
    bool LoadPerspective(const std::string& perspective, bool update = true);

    void SetDockSizeConstraint(double widthPct, double heightPct);
    void GetDockSizeConstraint(double* widthPct, double* heightPct) const;

    void ClosePane(wxAuiPaneInfo& paneInfo);
    void MaximizePane(wxAuiPaneInfo& paneInfo);
    void RestorePane(wxAuiPaneInfo& paneInfo);
    void RestoreMaximizedPane();

public:

    virtual wxAuiFloatingFrame* CreateFloatingFrame(wxWindow* parent, const wxAuiPaneInfo& p);
    virtual bool CanDockPanel(const wxAuiPaneInfo & p);

    void StartPaneDrag(
                 wxWindow* paneWindow,
                 const wxPoint& offset);

    wxRect CalculateHintRect(
                 wxWindow* paneWindow,
                 const wxPoint& pt,
                 const wxPoint& offset);

    void DrawHintRect(
                 wxWindow* paneWindow,
                 const wxPoint& pt,
                 const wxPoint& offset);

    virtual void ShowHint(const wxRect& rect);
    virtual void HideHint();

    void OnHintActivate(wxActivateEvent& event);

protected:

    void UpdateHintWindowConfig();

    void DoFrameLayout();

    void LayoutAddPane(wxSizer* container,
                       wxAuiDockInfo& dock,
                       wxAuiPaneInfo& pane,
                       wxAuiDockUIPartArray& uiparts,
                       bool spacerOnly);

    void LayoutAddDock(wxSizer* container,
                       wxAuiDockInfo& dock,
                       wxAuiDockUIPartArray& uiParts,
                       bool spacerOnly);

    wxSizer* LayoutAll(wxAuiPaneInfoArray& panes,
                       wxAuiDockInfoArray& docks,
                          wxAuiDockUIPartArray & uiParts,
                       bool spacerOnly = false);

    virtual bool ProcessDockResult(wxAuiPaneInfo& target,
                                   const wxAuiPaneInfo& newPos);

    bool DoDrop(wxAuiDockInfoArray& docks,
                wxAuiPaneInfoArray& panes,
                wxAuiPaneInfo& drop,
                const wxPoint& pt,
                const wxPoint& actionOffset = wxPoint(0,0));

    wxAuiDockUIPart* HitTest(int x, int y);
    wxAuiDockUIPart* GetPanePart(wxWindow* pane);
    int GetDockPixelOffset(wxAuiPaneInfo& test);
    void OnFloatingPaneMoveStart(wxWindow* window);
    void OnFloatingPaneMoving(wxWindow* window, wxDirection dir );
    void OnFloatingPaneMoved(wxWindow* window, wxDirection dir);
    void OnFloatingPaneActivated(wxWindow* window);
    void OnFloatingPaneClosed(wxWindow* window, wxCloseEvent& evt);
    void OnFloatingPaneResized(wxWindow* window, const wxRect& rect);
    void Render(wxDC* dc);
    void Repaint(wxDC* dc = nullptr);
    void ProcessMgrEvent(wxAuiManagerEvent& event);
    void UpdateButtonOnScreen(wxAuiDockUIPart* buttonUiPart,
                              const wxMouseEvent& event);
    void GetPanePositionsAndSizes(wxAuiDockInfo& dock,
                              std::vector<int>& positions,
                              std::vector<int>& sizes);

    /// Ends a resize action, or for live update, resizes the sash
    bool DoEndResizeAction(wxMouseEvent& event);

    void SetActivePane(wxWindow* active_pane);

public:
    // public events (which can be invoked externally)
    void OnRender(wxAuiManagerEvent& evt);
    void OnPaneButton(wxAuiManagerEvent& evt);

protected:

    // protected events
    void OnDestroy(wxWindowDestroyEvent& evt);
    void OnPaint(wxPaintEvent& evt);
    void OnEraseBackground(wxEraseEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnSetCursor(wxSetCursorEvent& evt);
    void OnLeftDown(wxMouseEvent& evt);
    void OnLeftUp(wxMouseEvent& evt);
    void OnMotion(wxMouseEvent& evt);
    void OnCaptureLost(wxMouseCaptureLostEvent& evt);
    void OnLeaveWindow(wxMouseEvent& evt);
    void OnChildFocus(wxChildFocusEvent& evt);
    void OnHintFadeTimer(wxTimerEvent& evt);
    void OnFindManager(wxAuiManagerEvent& evt);
    void OnSysColourChanged(wxSysColourChangedEvent& event);

    enum
    {
        actionNone = 0,
        actionResize,
        actionClickButton,
        actionClickCaption,
        actionDragToolbarPane,
        actionDragFloatingPane
    };

    wxWindow* m_frame{nullptr};           // the window being managed
    std::unique_ptr<wxAuiDockArt> m_art;            // dock art object which does all drawing
    unsigned int m_flags;        // manager flags wxAUI_MGR_*

    wxAuiPaneInfoArray m_panes;     // array of panes structures
    wxAuiDockInfoArray m_docks;     // array of docks structures
    wxAuiDockUIPartArray m_uiParts; // array of UI parts (captions, buttons, etc)

    int m_action{actionNone};                // current mouse action
    wxPoint m_actionStart;      // position where the action click started
    wxPoint m_actionOffset;     // offset from upper left of the item clicked
    wxAuiDockUIPart* m_actionPart{nullptr}; // ptr to the part the action happened to
    wxWindow* m_actionWindow{nullptr};   // action frame or window (NULL if none)
    wxRect m_actionHintRect;    // hint rectangle for the action
    wxRect m_lastRect;
    wxAuiDockUIPart* m_hoverButton{nullptr};// button uipart being hovered over
    wxRect m_lastHint;          // last hint rectangle
    wxPoint m_lastMouseMove;   // last mouse move position (see OnMotion)
    int  m_currentDragItem{-1};
    bool m_skipping{false};
    bool m_hasMaximized{false};

    double m_dockConstraintX{0.3};  // 0.0 .. 1.0; max pct of window width a dock can consume
    double m_dockConstraintY{0.3};  // 0.0 .. 1.0; max pct of window height a dock can consume

    wxFrame* m_hintWnd{nullptr};         // transparent hint window, if supported by platform
    wxTimer m_hintFadeTimer;    // transparent fade timer
    wxByte m_hintFadeAmt{};       // transparent fade amount
    wxByte m_hintFadeMax{};       // maximum value of hint fade

    void* m_reserved{nullptr};

#ifndef SWIG
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_CLASS(wxAuiManager);
#endif // SWIG
};

// event declarations/classes

class wxAuiManagerEvent : public wxEvent
{
public:
    wxAuiManagerEvent(wxEventType type=wxEVT_NULL) : wxEvent(0, type)
    {
    }

	wxAuiManagerEvent& operator=(const wxAuiManagerEvent&) = delete;

    wxEvent *Clone() const override { return new wxAuiManagerEvent(*this); }

    void SetManager(wxAuiManager* mgr) { manager = mgr; }
    void SetPane(wxAuiPaneInfo* p) { pane = p; }
    void SetButton(int b) { button = b; }
    void SetDC(wxDC* pdc) { dc = pdc; }

    wxAuiManager* GetManager() const { return manager; }
    wxAuiPaneInfo* GetPane() const { return pane; }
    int GetButton() const { return button; }
    wxDC* GetDC() const { return dc; }

    void Veto(bool veto = true) { veto_flag = veto; }
    bool GetVeto() const { return veto_flag; }
    void SetCanVeto(bool can_veto) { canveto_flag = can_veto; }
    bool CanVeto() const { return  canveto_flag && veto_flag; }

public:
    wxDC* dc{nullptr};
    wxAuiManager* manager{nullptr};
    wxAuiPaneInfo* pane{nullptr};
    
    int button{0};
    
    bool veto_flag{false};
    bool canveto_flag{true};

#ifndef SWIG
public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
#endif
};

inline wxAuiPaneInfo wxAuiNullPaneInfo;

struct wxAuiDockUIPart
{
    enum
    {
        typeCaption,
        typeGripper,
        typeDock,
        typeDockSizer,
        typePane,
        typePaneSizer,
        typeBackground,
        typePaneBorder,
        typePaneButton
    };

    bool operator==(const wxAuiDockUIPart&) const = default;

    int type{};                       // ui part type (see enum above)
    int orientation{};                // orientation (either wxHORIZONTAL or wxVERTICAL)
    wxAuiDockInfo* dock{nullptr};     // which dock the item is associated with
    wxAuiPaneInfo* pane{nullptr};     // which pane the item is associated with
    int button{};                     // which pane button the item is associated with
    wxSizer* cont_sizer{nullptr};     // the part's containing sizer
    wxSizerItem* sizer_item{nullptr}; // the sizer item of the part
    wxRect rect;                      // client coord rectangle of the part itself
};

#ifndef SWIG

inline const wxEventTypeTag<wxAuiManagerEvent> wxEVT_AUI_PANE_BUTTON( wxNewEventType() );
inline const wxEventTypeTag<wxAuiManagerEvent> wxEVT_AUI_PANE_CLOSE( wxNewEventType() );
inline const wxEventTypeTag<wxAuiManagerEvent> wxEVT_AUI_PANE_MAXIMIZE( wxNewEventType() );
inline const wxEventTypeTag<wxAuiManagerEvent> wxEVT_AUI_PANE_RESTORE( wxNewEventType() );
inline const wxEventTypeTag<wxAuiManagerEvent> wxEVT_AUI_PANE_ACTIVATED( wxNewEventType() );
inline const wxEventTypeTag<wxAuiManagerEvent> wxEVT_AUI_RENDER( wxNewEventType() );
inline const wxEventTypeTag<wxAuiManagerEvent> wxEVT_AUI_FIND_MANAGER( wxNewEventType() );

typedef void (wxEvtHandler::*wxAuiManagerEventFunction)(wxAuiManagerEvent&);

#endif

} // export
