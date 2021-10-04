///////////////////////////////////////////////////////////////////////////////
// Name:        wx/toolbook.h
// Purpose:     wxToolbook: wxToolBar and wxNotebook combination
// Author:      Julian Smart
// Modified by:
// Created:     2006-01-29
// Copyright:   (c) 2006 Julian Smart
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TOOLBOOK_H_
#define _WX_TOOLBOOK_H_

#include "wx/defs.h"

#if wxUSE_TOOLBOOK

#include "wx/bookctrl.h"
#include "wx/containr.h"

#include <string>

class WXDLLIMPEXP_FWD_CORE wxToolBarBase;
class WXDLLIMPEXP_FWD_CORE wxCommandEvent;

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_TOOLBOOK_PAGE_CHANGED,  wxBookCtrlEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_TOOLBOOK_PAGE_CHANGING, wxBookCtrlEvent );


// Use wxButtonToolBar
inline constexpr int wxTBK_BUTTONBAR = 0x0100;

// Use wxTB_HORZ_LAYOUT style for the controlling toolbar
inline constexpr int wxTBK_HORZ_LAYOUT = 0x8000;

// ----------------------------------------------------------------------------
// wxToolbook
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxToolbook : public wxNavigationEnabled<wxBookCtrlBase>
{
public:
    wxToolbook() = default;

    wxToolbook(wxWindow *parent,
               wxWindowID id,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = 0,
               const std::string& name = {})
    {
        Create(parent, id, pos, size, style, name);
    }

wxToolbook(const wxToolbook&) = delete;
   wxToolbook& operator=(const wxToolbook&) = delete;
   wxToolbook(wxToolbook&&) = default;
   wxToolbook& operator=(wxToolbook&&) = default;

    // quasi ctor
    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const std::string& name = {});


    // implement base class virtuals
    bool SetPageText(size_t n, const std::string& strText) override;
    std::string GetPageText(size_t n) const override;
    int GetPageImage(size_t n) const override;
    bool SetPageImage(size_t n, int imageId) override;
    bool InsertPage(size_t n,
                            wxWindow *page,
                            const std::string& text,
                            bool bSelect = false,
                            int imageId = NO_IMAGE) override;
    int SetSelection(size_t n) override { return DoSetSelection(n, SetSelection_SendEvent); }
    int ChangeSelection(size_t n) override { return DoSetSelection(n); }
    void SetImageList(wxImageList *imageList) override;

    bool DeleteAllPages() override;
    int HitTest(const wxPoint& pt, long *flags = nullptr) const override;


    // methods which are not part of base wxBookctrl API

    // get the underlying toolbar
    wxToolBarBase* GetToolBar() const { return (wxToolBarBase*)m_bookctrl; }

    // enable/disable a page
    bool EnablePage(wxWindow *page, bool enable);
    bool EnablePage(size_t page, bool enable);

    // must be called in OnIdle or by application to realize the toolbar and
    // select the initial page.
    void Realize();

protected:
    wxWindow *DoRemovePage(size_t page) override;

    // event handlers
    void OnToolSelected(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnIdle(wxIdleEvent& event);

    void UpdateSelectedPage(size_t newsel) override;

    wxBookCtrlEvent* CreatePageChangingEvent() const override;
    void MakeChangedEvent(wxBookCtrlEvent &event) override;

    // whether the toolbar needs to be realized
    bool m_needsRealizing{false};

    // maximum bitmap size
    wxSize m_maxBitmapSize;

private:    
    // returns the tool identifier for the specified page
    int PageToToolId(size_t page) const;

    // returns the page index for the specified tool ID or
    // wxNOT_FOUND if there is no page with that tool ID
    int ToolIdToPage(int toolId) const;


    wxDECLARE_EVENT_TABLE();

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ----------------------------------------------------------------------------
// listbook event class and related stuff
// ----------------------------------------------------------------------------

// wxToolbookEvent is obsolete and defined for compatibility only
#define wxToolbookEvent wxBookCtrlEvent
using wxToolbookEventFunction = wxBookCtrlEventFunction;
#define wxToolbookEventHandler(func) wxBookCtrlEventHandler(func)


#define EVT_TOOLBOOK_PAGE_CHANGED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_TOOLBOOK_PAGE_CHANGED, winid, wxBookCtrlEventHandler(fn))

#define EVT_TOOLBOOK_PAGE_CHANGING(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_TOOLBOOK_PAGE_CHANGING, winid, wxBookCtrlEventHandler(fn))

// old wxEVT_COMMAND_* constants
#define wxEVT_COMMAND_TOOLBOOK_PAGE_CHANGED    wxEVT_TOOLBOOK_PAGE_CHANGED
#define wxEVT_COMMAND_TOOLBOOK_PAGE_CHANGING   wxEVT_TOOLBOOK_PAGE_CHANGING

#endif // wxUSE_TOOLBOOK

#endif // _WX_TOOLBOOK_H_
