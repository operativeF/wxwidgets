/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/sashwin.h
// Purpose:     wxSashWindow implementation. A sash window has an optional
//              sash on each edge, allowing it to be dragged. An event
//              is generated when the sash is released.
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SASHWIN_H_G_
#define _WX_SASHWIN_H_G_

#if wxUSE_SASH

#include "wx/defs.h"
#include "wx/window.h"
#include "wx/string.h"

#include <memory>

#define wxSASH_DRAG_NONE       0
#define wxSASH_DRAG_DRAGGING   1
#define wxSASH_DRAG_LEFT_DOWN  2

enum wxSashEdgePosition {
    wxSASH_TOP = 0,
    wxSASH_RIGHT,
    wxSASH_BOTTOM,
    wxSASH_LEFT,
    wxSASH_NONE = 100
};

/*
 * wxSashEdge represents one of the four edges of a window.
 */

struct WXDLLIMPEXP_CORE wxSashEdge
{
    bool    m_show{false};     // Is the sash showing?
    int     m_margin{0};   // The margin size
};

/*
 * wxSashWindow flags
 */

// FIXME: Bitfield
#define wxSW_NOBORDER         0x0000
//#define wxSW_3D               0x0010
#define wxSW_BORDER           0x0020
#define wxSW_3DSASH           0x0040
#define wxSW_3DBORDER         0x0080
#define wxSW_3D (wxSW_3DSASH | wxSW_3DBORDER)

/*
 * wxSashWindow allows any of its edges to have a sash which can be dragged
 * to resize the window. The actual content window will be created as a child
 * of wxSashWindow.
 */

class WXDLLIMPEXP_CORE wxSashWindow: public wxWindow
{
public:
    // Default constructor
    wxSashWindow()
    {
        // Eventually, we'll respond to colour change messages
        InitColours();
    }

    // Normal constructor
    wxSashWindow(wxWindow *parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = wxSW_3D|wxCLIP_CHILDREN,
                 const std::string& name = "sashWindow")
    {
        // Eventually, we'll respond to colour change messages
        InitColours();

        Create(parent, id, pos, size, style, name);
    }

    wxSashWindow(const wxSashWindow&) = delete;
    wxSashWindow& operator=(const wxSashWindow&) = delete;

    ~wxSashWindow();

    bool Create(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, unsigned int style = wxSW_3D|wxCLIP_CHILDREN, const std::string& name = "sashWindow");

    // Set whether there's a sash in this position
    void SetSashVisible(wxSashEdgePosition edge, bool sash);

    // Get whether there's a sash in this position
    bool GetSashVisible(wxSashEdgePosition edge) const { return m_sashes[edge].m_show; }

    // Get border size
    int GetEdgeMargin(wxSashEdgePosition edge) const { return m_sashes[edge].m_margin; }

    // Sets the default sash border size
    void SetDefaultBorderSize(int width) { m_borderSize = width; }

    // Gets the default sash border size
    int GetDefaultBorderSize() const { return m_borderSize; }

    // Sets the addition border size between child and sash window
    void SetExtraBorderSize(int width) { m_extraBorderSize = width; }

    // Gets the addition border size between child and sash window
    int GetExtraBorderSize() const { return m_extraBorderSize; }

    virtual void SetMinimumSizeX(int min) { m_minimumPaneSizeX = min; }
    virtual void SetMinimumSizeY(int min) { m_minimumPaneSizeY = min; }
    virtual int GetMinimumSizeX() const { return m_minimumPaneSizeX; }
    virtual int GetMinimumSizeY() const { return m_minimumPaneSizeY; }

    virtual void SetMaximumSizeX(int max) { m_maximumPaneSizeX = max; }
    virtual void SetMaximumSizeY(int max) { m_maximumPaneSizeY = max; }
    virtual int GetMaximumSizeX() const { return m_maximumPaneSizeX; }
    virtual int GetMaximumSizeY() const { return m_maximumPaneSizeY; }

////////////////////////////////////////////////////////////////////////////
// Implementation

    // Paints the border and sash
    void OnPaint(wxPaintEvent& event);

    // Handles mouse events
    void OnMouseEvent(wxMouseEvent& ev);

    // Adjusts the panes
    void OnSize(wxSizeEvent& event);

#if defined(WX_WINDOWS) || defined(__WXMAC__)
    // Handle cursor correctly
    void OnSetCursor(wxSetCursorEvent& event);
#endif // wxMSW

    // Draws borders
    void DrawBorders(wxDC& dc);

    // Draws the sashes
    void DrawSash(wxSashEdgePosition edge, wxDC& dc);

    // Draws the sashes
    void DrawSashes(wxDC& dc);

    // Draws the sash tracker (for whilst moving the sash)
    void DrawSashTracker(wxSashEdgePosition edge, int x, int y);

    // Tests for x, y over sash
    wxSashEdgePosition SashHitTest(int x, int y, int tolerance = 2);

    // Resizes subwindows
    void SizeWindows();

    // Initialize colours
    void InitColours();

private:
    wxSashEdge  m_sashes[4];

    wxColour    m_lightShadowColour;
    wxColour    m_mediumShadowColour;
    wxColour    m_darkShadowColour;
    wxColour    m_hilightColour;
    wxColour    m_faceColour;

    std::unique_ptr<wxCursor>   m_sashCursorWE{std::make_unique<wxCursor>(wxCURSOR_SIZEWE)};
    std::unique_ptr<wxCursor>   m_sashCursorNS{std::make_unique<wxCursor>(wxCURSOR_SIZEWE)};
    wxCursor*   m_currentCursor{nullptr};

    wxSashEdgePosition m_draggingEdge{wxSASH_NONE};

    int         m_dragMode{wxSASH_DRAG_NONE};
    int         m_oldX{0};
    int         m_oldY{0};
    int         m_borderSize{3};
    int         m_extraBorderSize{0};
    int         m_firstX{0};
    int         m_firstY{0};
    int         m_minimumPaneSizeX{0};
    int         m_minimumPaneSizeY{0};
    int         m_maximumPaneSizeX{10000};
    int         m_maximumPaneSizeY{10000};

    bool        m_mouseCaptured{false};

private:
    wxDECLARE_DYNAMIC_CLASS(wxSashWindow);
    wxDECLARE_EVENT_TABLE();
};

class WXDLLIMPEXP_FWD_CORE wxSashEvent;

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_SASH_DRAGGED, wxSashEvent );

enum class wxSashDragStatus
{
    OK,
    OutOfRange
};

class WXDLLIMPEXP_CORE wxSashEvent: public wxCommandEvent
{
public:
    wxSashEvent(int id = 0, wxSashEdgePosition edge = wxSASH_NONE)
        : m_edge(edge)
    {
        m_id = id;
        m_eventType = (wxEventType) wxEVT_SASH_DRAGGED;
    }

    wxSashEvent(const wxSashEvent& event) = default;
    wxSashEvent& operator=(const wxSashEvent&) = delete;

    void SetEdge(wxSashEdgePosition edge) { m_edge = edge; }
    wxSashEdgePosition GetEdge() const { return m_edge; }

    //// The rectangle formed by the drag operation
    void SetDragRect(const wxRect& rect) { m_dragRect = rect; }
    wxRect GetDragRect() const { return m_dragRect; }

    //// Whether the drag caused the rectangle to be reversed (e.g.
    //// dragging the top below the bottom)
    void SetDragStatus(wxSashDragStatus status) { m_dragStatus = status; }
    wxSashDragStatus GetDragStatus() const { return m_dragStatus; }

    wxEvent *Clone() const override { return new wxSashEvent(*this); }

private:
    wxSashEdgePosition  m_edge;
    wxRect              m_dragRect;
    wxSashDragStatus    m_dragStatus{};

	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

typedef void (wxEvtHandler::*wxSashEventFunction)(wxSashEvent&);

#define wxSashEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxSashEventFunction, func)

#define EVT_SASH_DRAGGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_SASH_DRAGGED, id, wxSashEventHandler(fn))
#define EVT_SASH_DRAGGED_RANGE(id1, id2, fn) \
    wx__DECLARE_EVT2(wxEVT_SASH_DRAGGED, id1, id2, wxSashEventHandler(fn))

#endif // wxUSE_SASH

#endif
  // _WX_SASHWIN_H_G_
