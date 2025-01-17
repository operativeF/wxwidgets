///////////////////////////////////////////////////////////////////////////////
// Name:        wx/choicebk.h
// Purpose:     wxChoicebook: wxChoice and wxNotebook combination
// Author:      Vadim Zeitlin
// Modified by: Wlodzimierz ABX Skiba from wx/listbook.h
// Created:     15.09.04
// Copyright:   (c) Vadim Zeitlin, Wlodzimierz Skiba
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CHOICEBOOK_H_
#define _WX_CHOICEBOOK_H_

#if wxUSE_CHOICEBOOK

#include "wx/bookctrl.h"
#include "wx/choice.h"
#include "wx/containr.h"

import <string>;

class wxChoice;

wxDECLARE_EVENT( wxEVT_CHOICEBOOK_PAGE_CHANGED,  wxBookCtrlEvent );
wxDECLARE_EVENT( wxEVT_CHOICEBOOK_PAGE_CHANGING, wxBookCtrlEvent );

// wxChoicebook flags
inline constexpr unsigned int wxCHB_DEFAULT          = wxBK_DEFAULT;
inline constexpr unsigned int wxCHB_TOP              = wxBK_TOP;
inline constexpr unsigned int wxCHB_BOTTOM           = wxBK_BOTTOM;
inline constexpr unsigned int wxCHB_LEFT             = wxBK_LEFT;
inline constexpr unsigned int wxCHB_RIGHT            = wxBK_RIGHT;
inline constexpr unsigned int wxCHB_ALIGN_MASK       = wxBK_ALIGN_MASK;

// ----------------------------------------------------------------------------
// wxChoicebook
// ----------------------------------------------------------------------------

class wxChoicebook : public wxNavigationEnabled<wxBookCtrlBase>
{
public:
    wxChoicebook() = default;

    wxChoicebook(wxWindow *parent,
                 wxWindowID id,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = 0,
                 std::string_view name = {})
    {
        Create(parent, id, pos, size, style, name);
    }

    wxChoicebook& operator=(wxChoicebook&&) = delete;

    // quasi ctor
    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                std::string_view name = {});


    bool SetPageText(size_t n, const std::string& strText) override;
    std::string GetPageText(size_t n) const override;
    int GetPageImage(size_t n) const override;
    bool SetPageImage(size_t n, int imageId) override;
    bool InsertPage(size_t n,
                            wxWindow *page,
                            const std::string& text,
                            bool bSelect = false,
                            int imageId = NO_IMAGE) override;
    int SetSelection(size_t n) override
        { return DoSetSelection(n, SetSelection_SendEvent); }
    int ChangeSelection(size_t n) override { return DoSetSelection(n); }
    void SetImageList(wxImageList *imageList) override;

    bool DeleteAllPages() override;

    // returns the choice control
    wxChoice* GetChoiceCtrl() const { return (wxChoice*)m_bookctrl; }

protected:
    void DoSetWindowVariant(wxWindowVariant variant) override;

    wxWindow *DoRemovePage(size_t page) override;

    void UpdateSelectedPage(size_t newsel) override
    {
        GetChoiceCtrl()->Select(newsel);
    }

    wxBookCtrlEvent* CreatePageChangingEvent() const override;
    void MakeChangedEvent(wxBookCtrlEvent &event) override;

    // event handlers
    void OnChoiceSelected(wxCommandEvent& event);

private:
    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// choicebook event class and related stuff
// ----------------------------------------------------------------------------

// wxChoicebookEvent is obsolete and defined for compatibility only
#define wxChoicebookEvent wxBookCtrlEvent
using wxChoicebookEventFunction = wxBookCtrlEventFunction;
#define wxChoicebookEventHandler(func) wxBookCtrlEventHandler(func)

#define EVT_CHOICEBOOK_PAGE_CHANGED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_CHOICEBOOK_PAGE_CHANGED, winid, wxBookCtrlEventHandler(fn))

#define EVT_CHOICEBOOK_PAGE_CHANGING(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_CHOICEBOOK_PAGE_CHANGING, winid, wxBookCtrlEventHandler(fn))

#endif // wxUSE_CHOICEBOOK

#endif // _WX_CHOICEBOOK_H_
