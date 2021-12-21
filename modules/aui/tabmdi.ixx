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
#include "wx/mdi.h"
#include "wx/menu.h"
#include "wx/window.h"

#include <memory>

export module WX.AUI.TabMDI;

import WX.AUI.Book;
import WX.AUI.Flags;
import WX.AUI.TabArt;

import <string>;

//-----------------------------------------------------------------------------
// wxAuiMDIParentFrame
//-----------------------------------------------------------------------------

class wxAuiMDIChildFrame;
class wxAuiMDIClientWindow;

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
