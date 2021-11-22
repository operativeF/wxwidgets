#ifndef WX_AUI_EVENTS_H
#define WX_AUI_EVENTS_H

// wx event machinery

#ifndef SWIG

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

#endif // WX_AUI_EVENTS_H
