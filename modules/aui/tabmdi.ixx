/////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/tabmdi.h
// Purpose:     Generic MDI (Multiple Document Interface) classes
// Author:      Hans Van Leemputten
// Modified by: Benjamin I. Williams / Kirix Corporation
// Created:     29/07/2002
// Copyright:   (c) Hans Van Leemputten
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/frame.h"
#include "wx/iconbndl.h"
#include "wx/mdi.h"
#include "wx/aui/events.h"

#include "wx/menu.h"
#include "wx/intl.h"
#include "wx/stockitem.h" // FIXME: only included for enum value
#include "wx/toplevel.h"


export module WX.AUI.TabMDI;

import WX.AUI.DockArt;
import WX.AUI.Book;
import WX.AUI.TabArt;

import Utils.Geometry;
import WX.Utils.Settings;

class wxAuiMDIParentFrame;
class wxAuiMDIClientWindow;
class wxAuiMDIChildFrame;

//-----------------------------------------------------------------------------
// wxAuiMDIParentFrame
//-----------------------------------------------------------------------------

export
{

class wxAuiMDIParentFrame : public wxFrame
{
public:
    wxAuiMDIParentFrame() = default;
    wxAuiMDIParentFrame(wxWindow *parent,
                        wxWindowID winid,
                        const std::string& title,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        unsigned int style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                        std::string_view name = wxFrameNameStr);

    ~wxAuiMDIParentFrame();

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID winid,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                std::string_view name = wxFrameNameStr);

    void SetArtProvider(std::unique_ptr<wxAuiTabArt> provider);
    wxAuiTabArt* GetArtProvider();
    wxAuiNotebook* GetNotebook() const;

#if wxUSE_MENUS
    wxMenu* GetWindowMenu() const { return m_pWindowMenu; }
    void SetWindowMenu(wxMenu* pMenu);

    void SetMenuBar(wxMenuBar *pMenuBar) override;
#endif // wxUSE_MENUS

    void SetChildMenuBar(wxAuiMDIChildFrame *pChild);

    wxAuiMDIChildFrame *GetActiveChild() const;
    void SetActiveChild(wxAuiMDIChildFrame* pChildFrame);

    wxAuiMDIClientWindow *GetClientWindow() const;
    virtual wxAuiMDIClientWindow *OnCreateClient();

    virtual void Cascade() { /* Has no effect */ }
    virtual void Tile(wxOrientation orient = wxHORIZONTAL);
    virtual void ArrangeIcons() { /* Has no effect */ }
    virtual void ActivateNext();
    virtual void ActivatePrevious();

protected:
    wxAuiMDIClientWindow*   m_pClientWindow{nullptr};
    wxEvent*                m_pLastEvt{nullptr};

#if wxUSE_MENUS
    wxMenu*              m_pWindowMenu{nullptr};
    wxMenuBar*           m_pMyMenuBar{nullptr};
#endif // wxUSE_MENUS

protected:
#if wxUSE_MENUS
    void RemoveWindowMenu(wxMenuBar *pMenuBar);
    void AddWindowMenu(wxMenuBar *pMenuBar);

    void DoHandleMenu(wxCommandEvent &event);
    void DoHandleUpdateUI(wxUpdateUIEvent &event);
#endif // wxUSE_MENUS

    bool ProcessEvent(wxEvent& event) override;

    wxSize DoGetClientSize() const override;

private:
    void OnClose(wxCloseEvent& event);

    // close all children, return false if any of them vetoed it
    bool CloseAll();

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxAuiMDIParentFrame);
};

//-----------------------------------------------------------------------------
// wxAuiMDIChildFrame
//-----------------------------------------------------------------------------

class wxAuiMDIChildFrame : public wxTDIChildFrame
{
public:
    wxAuiMDIChildFrame() = default;
    wxAuiMDIChildFrame(wxAuiMDIParentFrame *parent,
                       wxWindowID winid,
                       const std::string& title,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       unsigned int style = wxDEFAULT_FRAME_STYLE,
                       std::string_view name = wxFrameNameStr);

    ~wxAuiMDIChildFrame();
    [[maybe_unused]] bool Create(wxAuiMDIParentFrame *parent,
                wxWindowID winid,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE,
                std::string_view name = wxFrameNameStr);

#if wxUSE_MENUS
    void SetMenuBar(wxMenuBar *menuBar) override;
    wxMenuBar *GetMenuBar() const override;
#endif // wxUSE_MENUS

    void SetTitle(const std::string& title) override;

    void SetIcons(const wxIconBundle& icons) override;

    void Activate() override;
    bool Destroy() override;

    bool Show(bool show = true) override;

    void OnMenuHighlight(wxMenuEvent& evt);

    void SetMDIParentFrame(wxAuiMDIParentFrame* parent);
    wxAuiMDIParentFrame* GetMDIParentFrame() const;

public:
    // This function needs to be called when a size change is confirmed,
    // we needed this function to prevent anybody from the outside
    // changing the panel... it messes the UI layout when we would allow it.
    void ApplyMDIChildFrameRect();

protected:
    wxAuiMDIParentFrame* m_pMDIParentFrame{nullptr};
#if wxUSE_MENUS
    wxMenuBar* m_pMenuBar{nullptr};
#endif // wxUSE_MENUS

    bool m_activateOnCreate{true};

private:
    wxDECLARE_DYNAMIC_CLASS(wxAuiMDIChildFrame);
    wxDECLARE_EVENT_TABLE();

    friend class wxAuiMDIClientWindow;
};

//-----------------------------------------------------------------------------
// wxAuiMDIClientWindow
//-----------------------------------------------------------------------------

class wxAuiMDIClientWindow : public wxAuiNotebook
{
public:
    wxAuiMDIClientWindow() = default;
    wxAuiMDIClientWindow(wxAuiMDIParentFrame *parent, unsigned int style = 0);

    virtual bool CreateClient(wxAuiMDIParentFrame *parent,
                              unsigned int style = wxVSCROLL | wxHSCROLL);

    virtual wxAuiMDIChildFrame* GetActiveChild();
    virtual void SetActiveChild(wxAuiMDIChildFrame* pChildFrame)
    {
        SetSelection(GetPageIndex(pChildFrame));
    }

protected:

    void PageChanged(int oldSelection, int newSelection);
    void OnPageClose(wxAuiNotebookEvent& evt);
    void OnPageChanged(wxAuiNotebookEvent& evt);

private:
    wxDECLARE_DYNAMIC_CLASS(wxAuiMDIClientWindow);
    wxDECLARE_EVENT_TABLE();
};

} // export


enum MDI_MENU_ID
{
    wxWINDOWCLOSE = 4001,
    wxWINDOWCLOSEALL,
    wxWINDOWNEXT,
    wxWINDOWPREV
};

//-----------------------------------------------------------------------------
// wxAuiMDIParentFrame
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxAuiMDIParentFrame, wxFrame);

wxBEGIN_EVENT_TABLE(wxAuiMDIParentFrame, wxFrame)
    EVT_CLOSE(wxAuiMDIParentFrame::OnClose)
#if wxUSE_MENUS
    EVT_MENU (wxID_ANY, wxAuiMDIParentFrame::DoHandleMenu)
    EVT_UPDATE_UI (wxID_ANY, wxAuiMDIParentFrame::DoHandleUpdateUI)
#endif
wxEND_EVENT_TABLE()

wxAuiMDIParentFrame::wxAuiMDIParentFrame(wxWindow *parent,
                                         wxWindowID id,
                                         const std::string& title,
                                         const wxPoint& pos,
                                         const wxSize& size,
                                         unsigned int style,
                                         std::string_view name)
{
    Create(parent, id, title, pos, size, style, name);
}

wxAuiMDIParentFrame::~wxAuiMDIParentFrame()
{
    // Avoid having GetActiveChild() called after m_pClientWindow is destroyed
    SendDestroyEvent();
    // Make sure the client window is destructed before the menu bars are!
    wxDELETE(m_pClientWindow);

#if wxUSE_MENUS
    wxDELETE(m_pMyMenuBar);
    RemoveWindowMenu(GetMenuBar());
    wxDELETE(m_pWindowMenu);
#endif // wxUSE_MENUS
}

bool wxAuiMDIParentFrame::Create(wxWindow *parent,
                                 wxWindowID id,
                                 const std::string& title,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 unsigned int style,
                                 std::string_view name)
{
#if wxUSE_MENUS
    // this style can be used to prevent a window from having the standard MDI
    // "Window" menu
    if (!(style & wxFRAME_NO_WINDOW_MENU))
    {
        m_pWindowMenu = new wxMenu;
        m_pWindowMenu->Append(wxWINDOWCLOSE,    _("Cl&ose"));
        m_pWindowMenu->Append(wxWINDOWCLOSEALL, _("Close All"));
        m_pWindowMenu->AppendSeparator();
        m_pWindowMenu->Append(wxWINDOWNEXT,     _("&Next"));
        m_pWindowMenu->Append(wxWINDOWPREV,     _("&Previous"));
    }
#endif // wxUSE_MENUS

    if ( !wxFrame::Create(parent, id, title, pos, size, style, name) )
        return false;

    m_pClientWindow = OnCreateClient();
    return m_pClientWindow != nullptr;
}


void wxAuiMDIParentFrame::SetArtProvider(std::unique_ptr<wxAuiTabArt> provider)
{
    if (m_pClientWindow)
    {
        m_pClientWindow->SetArtProvider(std::move(provider));
    }
}

wxAuiTabArt* wxAuiMDIParentFrame::GetArtProvider()
{
    if (!m_pClientWindow)
        return nullptr;

    return m_pClientWindow->GetArtProvider();
}

wxAuiNotebook* wxAuiMDIParentFrame::GetNotebook() const
{
    return m_pClientWindow;
}



#if wxUSE_MENUS
void wxAuiMDIParentFrame::SetWindowMenu(wxMenu* pMenu)
{
    // Replace the window menu from the currently loaded menu bar.
    wxMenuBar *pMenuBar = GetMenuBar();

    if (m_pWindowMenu)
    {
        RemoveWindowMenu(pMenuBar);
        wxDELETE(m_pWindowMenu);
    }

    if (pMenu)
    {
        m_pWindowMenu = pMenu;
        AddWindowMenu(pMenuBar);
    }
}

void wxAuiMDIParentFrame::SetMenuBar(wxMenuBar* pMenuBar)
{
    // Remove the Window menu from the old menu bar
    RemoveWindowMenu(GetMenuBar());

    // Add the Window menu to the new menu bar.
    AddWindowMenu(pMenuBar);

    wxFrame::SetMenuBar(pMenuBar);
    //m_pMyMenuBar = GetMenuBar();
}
#endif // wxUSE_MENUS

void wxAuiMDIParentFrame::SetChildMenuBar(wxAuiMDIChildFrame* pChild)
{
#if wxUSE_MENUS
    if (!pChild)
    {
        // No Child, set Our menu bar back.
        if (m_pMyMenuBar)
            SetMenuBar(m_pMyMenuBar);
        else
            SetMenuBar(GetMenuBar());

        // Make sure we know our menu bar is in use
        m_pMyMenuBar = nullptr;
    }
    else
    {
        if (pChild->GetMenuBar() == nullptr)
            return;

        // Do we need to save the current bar?
        if (m_pMyMenuBar == nullptr)
            m_pMyMenuBar = GetMenuBar();

        SetMenuBar(pChild->GetMenuBar());
    }
#endif // wxUSE_MENUS
}

bool wxAuiMDIParentFrame::ProcessEvent(wxEvent& event)
{
    // stops the same event being processed repeatedly
    if (m_pLastEvt == &event)
        return false;
    m_pLastEvt = &event;

    // let the active child (if any) process the event first.
    bool res = false;
    wxAuiMDIChildFrame* pActiveChild = GetActiveChild();
    if (pActiveChild &&
        event.IsCommandEvent() &&
        event.GetEventObject() != m_pClientWindow &&
           !(event.GetEventType() == wxEVT_ACTIVATE ||
             event.GetEventType() == wxEVT_SET_FOCUS ||
             event.GetEventType() == wxEVT_KILL_FOCUS ||
             event.GetEventType() == wxEVT_CHILD_FOCUS ||
             event.GetEventType() == wxEVT_COMMAND_SET_FOCUS ||
             event.GetEventType() == wxEVT_COMMAND_KILL_FOCUS )
       )
    {
        res = pActiveChild->GetEventHandler()->ProcessEvent(event);
    }

    if (!res)
    {
        // if the event was not handled this frame will handle it,
        // which is why we need the protection code at the beginning
        // of this method
        res = wxEvtHandler::ProcessEvent(event);
    }

    m_pLastEvt = nullptr;

    return res;
}

void wxAuiMDIParentFrame::OnClose(wxCloseEvent& event)
{
    if (!CloseAll())
        event.Veto();
    else
        event.Skip();
}

bool wxAuiMDIParentFrame::CloseAll()
{
    wxAuiMDIChildFrame* pActiveChild;
    while ((pActiveChild = GetActiveChild()) != nullptr)
    {
        if (!pActiveChild->Close())
        {
            // it refused to close, don't close the remaining ones neither
            return false;
        }
    }

    return true;
}

wxAuiMDIChildFrame *wxAuiMDIParentFrame::GetActiveChild() const
{
    // We can be called before the client window is created, so check for its
    // existence.
    wxAuiMDIClientWindow* const client = GetClientWindow();
    return client ? client->GetActiveChild() : nullptr;
}

void wxAuiMDIParentFrame::SetActiveChild(wxAuiMDIChildFrame* pChildFrame)
{
    wxAuiMDIClientWindow* const client = GetClientWindow();
    if (client && client->GetActiveChild() != pChildFrame)
    {
        client->SetActiveChild(pChildFrame);
    }
}

wxAuiMDIClientWindow *wxAuiMDIParentFrame::GetClientWindow() const
{
    return m_pClientWindow;
}

wxAuiMDIClientWindow *wxAuiMDIParentFrame::OnCreateClient()
{
    return new wxAuiMDIClientWindow( this );
}

void wxAuiMDIParentFrame::ActivateNext()
{
    if (m_pClientWindow && m_pClientWindow->GetSelection() != wxNOT_FOUND)
    {
        size_t active = m_pClientWindow->GetSelection() + 1;
        if (active >= m_pClientWindow->GetPageCount())
            active = 0;

        m_pClientWindow->SetSelection(active);
    }
}

void wxAuiMDIParentFrame::ActivatePrevious()
{
    if (m_pClientWindow && m_pClientWindow->GetSelection() != wxNOT_FOUND)
    {
        int active = m_pClientWindow->GetSelection() - 1;
        if (active < 0)
            active = m_pClientWindow->GetPageCount() - 1;

        m_pClientWindow->SetSelection(active);
    }
}

#if wxUSE_MENUS
void wxAuiMDIParentFrame::RemoveWindowMenu(wxMenuBar* pMenuBar)
{
    if (pMenuBar && m_pWindowMenu)
    {
        // Remove old window menu
        int pos = pMenuBar->FindMenu(_("&Window"));
        if (pos != wxNOT_FOUND)
        {
            // DBG:: We're going to delete the wrong menu!!!
            wxASSERT(m_pWindowMenu == pMenuBar->GetMenu(pos));
            pMenuBar->Remove(pos);
        }
    }
}

void wxAuiMDIParentFrame::AddWindowMenu(wxMenuBar *pMenuBar)
{
    if (pMenuBar && m_pWindowMenu)
    {
        int pos = pMenuBar->FindMenu(wxGetStockLabel(wxID_HELP,wxSTOCK_NOFLAGS));
        if (pos == wxNOT_FOUND)
            pMenuBar->Append(m_pWindowMenu, _("&Window"));
        else
            pMenuBar->Insert(pos, m_pWindowMenu, _("&Window"));
    }
}

void wxAuiMDIParentFrame::DoHandleMenu(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case wxWINDOWCLOSE:
        {
            wxAuiMDIChildFrame* pActiveChild = GetActiveChild();
            if (pActiveChild)
                pActiveChild->Close();
            break;
        }
        case wxWINDOWCLOSEALL:
        {
            CloseAll();
            break;
        }
        case wxWINDOWNEXT:
            ActivateNext();
            break;
        case wxWINDOWPREV:
            ActivatePrevious();
            break;
        default:
            event.Skip();
    }
}

void wxAuiMDIParentFrame::DoHandleUpdateUI(wxUpdateUIEvent& event)
{
    switch (event.GetId())
    {
        case wxWINDOWCLOSE:
        case wxWINDOWCLOSEALL:
        {
            wxAuiMDIClientWindow* client_window = GetClientWindow();
            wxCHECK_RET(client_window, "Missing MDI Client Window");
            size_t pages = client_window->GetPageCount();
            event.Enable(pages >= 1);
            break;
        }

        case wxWINDOWNEXT:
        case wxWINDOWPREV:
        {
            wxAuiMDIClientWindow* client_window = GetClientWindow();
            wxCHECK_RET(client_window, "Missing MDI Client Window");
            size_t pages = client_window->GetPageCount();
            event.Enable(pages >= 2);
            break;
        }

        default:
            event.Skip();
    }
}
#endif // wxUSE_MENUS

wxSize wxAuiMDIParentFrame::DoGetClientSize() const
{
    return wxFrame::DoGetClientSize();
}

void wxAuiMDIParentFrame::Tile(wxOrientation orient)
{
    wxAuiMDIClientWindow* client_window = GetClientWindow();
    wxASSERT_MSG(client_window, "Missing MDI Client Window");

    int cur_idx = client_window->GetSelection();
    if (cur_idx == -1)
        return;

    if (orient == wxVERTICAL)
    {
        client_window->Split(cur_idx, wxLEFT);
    }
    else if (orient == wxHORIZONTAL)
    {
        client_window->Split(cur_idx, wxTOP);
    }
}


//-----------------------------------------------------------------------------
// wxAuiMDIChildFrame
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxAuiMDIChildFrame, wxFrame);

wxBEGIN_EVENT_TABLE(wxAuiMDIChildFrame, wxFrame)
    EVT_MENU_HIGHLIGHT_ALL(wxAuiMDIChildFrame::OnMenuHighlight)
wxEND_EVENT_TABLE()

wxAuiMDIChildFrame::wxAuiMDIChildFrame(wxAuiMDIParentFrame *parent,
                                       wxWindowID id,
                                       const std::string& title,
                                       [[maybe_unused]] const wxPoint& pos,
                                       const wxSize& size,
                                       unsigned int style,
                                       std::string_view name)
{
    // There are two ways to create an tabbed mdi child fram without
    // making it the active document.  Either Show(false) can be called
    // before Create() (as is customary on some ports with wxFrame-type
    // windows), or wxMINIMIZE can be passed in the style flags.  Note that
    // wxAuiMDIChildFrame is not really derived from wxFrame, as wxMDIChildFrame
    // is, but those are the expected symantics.  No style flag is passed
    // onto the panel underneath.
    if (style & wxMINIMIZE)
        m_activateOnCreate = false;

    Create(parent, id, title, wxDefaultPosition, size, 0, name);
}

wxAuiMDIChildFrame::~wxAuiMDIChildFrame()
{
    wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
    if (pParentFrame)
    {
        if (pParentFrame->GetActiveChild() == this)
        {
            pParentFrame->SetActiveChild(nullptr);
            pParentFrame->SetChildMenuBar(nullptr);
        }
        wxAuiMDIClientWindow* pClientWindow = pParentFrame->GetClientWindow();
        wxASSERT(pClientWindow);
        int idx = pClientWindow->GetPageIndex(this);
        if (idx != wxNOT_FOUND)
        {
            pClientWindow->RemovePage(idx);
        }
    }

#if wxUSE_MENUS
    wxDELETE(m_pMenuBar);
#endif // wxUSE_MENUS
}

bool wxAuiMDIChildFrame::Create(wxAuiMDIParentFrame* parent,
                                wxWindowID id,
                                const std::string& title,
                                [[maybe_unused]] const wxPoint& pos,
                                const wxSize& size,
                                unsigned int style,
                                std::string_view name)
{
    wxAuiMDIClientWindow* pClientWindow = parent->GetClientWindow();
    wxASSERT_MSG((pClientWindow != nullptr), "Missing MDI client window.");

    // see comment in constructor
    if (style & wxMINIMIZE)
        m_activateOnCreate = false;

    // create the window hidden to prevent flicker
    wxWindow::Show(false);
    wxWindow::Create(pClientWindow,
                    id,
                    wxDefaultPosition,
                    size,
                    wxNO_BORDER, name);

    SetMDIParentFrame(parent);

    m_title = title;

    pClientWindow->AddPage(this, title, m_activateOnCreate);

    // Check that the parent notion of the active child coincides with our one.
    // This is less obvious that it seems because we must honour
    // m_activateOnCreate flag but only if it's not the first child because
    // this one becomes active unconditionally.
    wxASSERT_MSG
    (
        (m_activateOnCreate || pClientWindow->GetPageCount() == 1)
            == (parent->GetActiveChild() == this),
        "Logic error: child [not] activated when it should [not] have been."
    );

    pClientWindow->Refresh();

    return true;
}

bool wxAuiMDIChildFrame::Destroy()
{
    wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
    wxASSERT_MSG(pParentFrame, "Missing MDI Parent Frame");

    wxAuiMDIClientWindow* pClientWindow = pParentFrame->GetClientWindow();
    wxASSERT_MSG(pClientWindow, "Missing MDI Client Window");

    if (pParentFrame->GetActiveChild() == this)
    {
        // deactivate ourself
        wxActivateEvent event(wxEVT_ACTIVATE, false, GetId());
        event.SetEventObject(this);
        GetEventHandler()->ProcessEvent(event);

        pParentFrame->SetChildMenuBar(nullptr);
    }

    size_t page_count = pClientWindow->GetPageCount();
    for (size_t pos = 0; pos < page_count; pos++)
    {
        if (pClientWindow->GetPage(pos) == this)
            return pClientWindow->DeletePage(pos);
    }

    return false;
}

#if wxUSE_MENUS
void wxAuiMDIChildFrame::SetMenuBar(wxMenuBar *menu_bar)
{
    wxMenuBar *pOldMenuBar = m_pMenuBar;
    m_pMenuBar = menu_bar;

    if (m_pMenuBar)
    {
        wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
        wxASSERT_MSG(pParentFrame, "Missing MDI Parent Frame");

        m_pMenuBar->SetParent(pParentFrame);
        if (pParentFrame->GetActiveChild() == this)
        {
            // replace current menu bars
            if (pOldMenuBar)
                pParentFrame->SetChildMenuBar(nullptr);
            pParentFrame->SetChildMenuBar(this);
        }
    }
}

wxMenuBar *wxAuiMDIChildFrame::GetMenuBar() const
{
    return m_pMenuBar;
}
#endif // wxUSE_MENUS

void wxAuiMDIChildFrame::SetTitle(const std::string& title)
{
    m_title = title;

    wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
    wxASSERT_MSG(pParentFrame, "Missing MDI Parent Frame");

    wxAuiMDIClientWindow* pClientWindow = pParentFrame->GetClientWindow();
    if (pClientWindow != nullptr)
    {
        for (size_t pos = 0; pos < pClientWindow->GetPageCount(); pos++)
        {
            if (pClientWindow->GetPage(pos) == this)
            {
                pClientWindow->SetPageText(pos, m_title);
                break;
            }
        }
    }
}

void wxAuiMDIChildFrame::SetIcons(const wxIconBundle& icons)
{
    wxTDIChildFrame::SetIcons(icons);

    wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
    wxASSERT_MSG(pParentFrame, "Missing MDI Parent Frame");

    const wxSize sizeIcon(wxSystemSettings::GetMetric(wxSYS_SMALLICON_X, this),
                          wxSystemSettings::GetMetric(wxSYS_SMALLICON_Y, this));
    wxBitmap bmp;
    bmp.CopyFromIcon(icons.GetIcon(sizeIcon));

    wxAuiMDIClientWindow* pClientWindow = pParentFrame->GetClientWindow();
    if (pClientWindow != nullptr)
    {
        int idx = pClientWindow->GetPageIndex(this);

        if (idx != -1)
        {
            pClientWindow->SetPageBitmap((size_t)idx, bmp);
        }
    }
}

void wxAuiMDIChildFrame::Activate()
{
    wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
    wxASSERT_MSG(pParentFrame, "Missing MDI Parent Frame");

    wxAuiMDIClientWindow* pClientWindow = pParentFrame->GetClientWindow();

    if (pClientWindow != nullptr)
    {
        for (size_t pos = 0; pos < pClientWindow->GetPageCount(); pos++)
        {
            if (pClientWindow->GetPage(pos) == this)
            {
                pClientWindow->SetSelection(pos);
                break;
            }
        }
    }
}

void wxAuiMDIChildFrame::OnMenuHighlight(wxMenuEvent& event)
{
#if wxUSE_STATUSBAR
    if (m_pMDIParentFrame)
    {
        // we don't have any help text for this item,
        // but may be the MDI frame does?
        m_pMDIParentFrame->OnMenuHighlight(event);
    }
#else
    wxUnusedVar(event);
#endif // wxUSE_STATUSBAR
}

void wxAuiMDIChildFrame::SetMDIParentFrame(wxAuiMDIParentFrame* parentFrame)
{
    m_pMDIParentFrame = parentFrame;
}

wxAuiMDIParentFrame* wxAuiMDIChildFrame::GetMDIParentFrame() const
{
    return m_pMDIParentFrame;
}

bool wxAuiMDIChildFrame::Show(bool show)
{
    m_activateOnCreate = show;

    if ( show )
    {
        // This is not a real TLW, so it won't get a size event when it's
        // really "mapped", i.e. appears on the screen for the first time.
        // Instead, its size had been already set when it was created and we
        // didn't have any opportunity to lay it out since then, i.e. since
        // before its children were created. Do it now to allow the same code
        // that would work with a "real" wxMDIChildFrame to also work with this
        // class.
        Layout();
    }

    return true;
}

//-----------------------------------------------------------------------------
// wxAuiMDIClientWindow
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxAuiMDIClientWindow, wxAuiNotebook);

wxBEGIN_EVENT_TABLE(wxAuiMDIClientWindow, wxAuiNotebook)
    EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, wxAuiMDIClientWindow::OnPageChanged)
    EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, wxAuiMDIClientWindow::OnPageClose)
wxEND_EVENT_TABLE()

wxAuiMDIClientWindow::wxAuiMDIClientWindow(wxAuiMDIParentFrame* parent, unsigned int style)
{
    CreateClient(parent, style);
}

bool wxAuiMDIClientWindow::CreateClient(wxAuiMDIParentFrame* parent, unsigned int style)
{
    SetWindowStyleFlag(style);

    if (!wxAuiNotebook::Create(parent,
                               wxID_ANY,
                               wxPoint(0,0),
                               parent->FromDIP(wxSize(100, 100)),
                               wxAUI_NB_DEFAULT_STYLE | wxNO_BORDER))
    {
        return false;
    }

    wxColour bkcolour = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);
    SetOwnBackgroundColour(bkcolour);

    m_mgr.GetArtProvider()->SetColour(wxAUI_DOCKART_BACKGROUND_COLOUR, bkcolour);

    return true;
}

wxAuiMDIChildFrame* wxAuiMDIClientWindow::GetActiveChild()
{
    const int sel = GetSelection();
    if ( sel == wxNOT_FOUND || sel >= (int)GetPageCount() )
        return nullptr;

    return wxStaticCast(GetPage(sel), wxAuiMDIChildFrame);
}

void wxAuiMDIClientWindow::PageChanged(int old_selection, int new_selection)
{
    // don't do anything if the page doesn't actually change
    if (old_selection == new_selection)
        return;

    /*
    // don't do anything if the new page is already active
    if (new_selection != -1)
    {
        wxAuiMDIChildFrame* child = (wxAuiMDIChildFrame*)GetPage(new_selection);
        if (child->GetMDIParentFrame()->GetActiveChild() == child)
            return;
    }*/


    // notify old active child that it has been deactivated
    if ((old_selection != -1) && (old_selection < (int)GetPageCount()))
    {
        wxAuiMDIChildFrame* old_child = (wxAuiMDIChildFrame*)GetPage(old_selection);
        wxASSERT_MSG(old_child, "wxAuiMDIClientWindow::PageChanged - null page pointer");

        wxActivateEvent event(wxEVT_ACTIVATE, false, old_child->GetId());
        event.SetEventObject(old_child);
        old_child->GetEventHandler()->ProcessEvent(event);
    }

    // notify new active child that it has been activated
    if (new_selection != -1)
    {
        wxAuiMDIChildFrame* active_child = (wxAuiMDIChildFrame*)GetPage(new_selection);
        wxASSERT_MSG(active_child, "wxAuiMDIClientWindow::PageChanged - null page pointer");

        wxActivateEvent event(wxEVT_ACTIVATE, true, active_child->GetId());
        event.SetEventObject(active_child);
        active_child->GetEventHandler()->ProcessEvent(event);

        if (active_child->GetMDIParentFrame())
        {
            active_child->GetMDIParentFrame()->SetActiveChild(active_child);
            active_child->GetMDIParentFrame()->SetChildMenuBar(active_child);
        }
    }


}

void wxAuiMDIClientWindow::OnPageClose(wxAuiNotebookEvent& evt)
{
    auto* wnd = dynamic_cast<wxAuiMDIChildFrame*>(GetPage(evt.GetSelection()));

    wnd->Close();

    // regardless of the result of wnd->Close(), we've
    // already taken care of the close operations, so
    // suppress further processing
    evt.Veto();
}

void wxAuiMDIClientWindow::OnPageChanged(wxAuiNotebookEvent& evt)
{
    PageChanged(evt.GetOldSelection(), evt.GetSelection());
}
