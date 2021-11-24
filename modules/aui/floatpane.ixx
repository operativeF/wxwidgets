///////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/floatpane.h
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by:
// Created:     2005-05-17
// Copyright:   (C) Copyright 2005, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/weakref.h"

#if wxUSE_MINIFRAME
    #include "wx/minifram.h"
#else
    #include "wx/frame.h"
#endif

export module WX.AUI.FloatPane;

import WX.Cfg.Flags;

import WX.AUI.FrameManager;

import Utils.Geometry;

export
{

#if wxUSE_MINIFRAME
using wxAuiFloatingFrameBaseClass = wxMiniFrame;
#else
using wxAuiFloatingFrameBaseClass = wxFrame;
#endif

class wxAuiFloatingFrame : public wxAuiFloatingFrameBaseClass
{
public:
    wxAuiFloatingFrame(wxWindow* parent,
                   wxAuiManager* ownerMgr,
                   const wxAuiPaneInfo& pane,
                   wxWindowID id = wxID_ANY,
                   unsigned int style = wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION |
                                wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT |
                                wxCLIP_CHILDREN
                   );
    ~wxAuiFloatingFrame();
    void SetPaneWindow(const wxAuiPaneInfo& pane);
    wxAuiManager* GetOwnerManager() const;

    // Allow processing accelerators to the parent frame
    bool IsTopNavigationDomain(NavigationKind kind) const override;

    wxAuiManager& GetAuiManager()  { return m_mgr; }

protected:
    virtual void OnMoveStart();
    virtual void OnMoving(const wxRect& windowRect, wxDirection dir);
    virtual void OnMoveFinished();

private:
    void OnSize(wxSizeEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnMoveEvent(wxMoveEvent& event);
    void OnIdle(wxIdleEvent& event);
    void OnActivate(wxActivateEvent& event);
    static bool isMouseDown();

private:
    wxWeakRef<wxAuiManager> m_ownerMgr;
    wxAuiManager m_mgr;
    
    wxRect m_lastRect;
    wxRect m_last2Rect;
    wxRect m_last3Rect;

    wxSize m_lastSize;

    wxWindow* m_paneWindow{nullptr};    // pane window being managed

    wxDirection m_lastDirection;

    bool m_solidDrag{false};          // true if system uses solid window drag
    bool m_moving{false};

#ifndef SWIG
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_CLASS(wxAuiFloatingFrame);
#endif // SWIG
};

} // export
