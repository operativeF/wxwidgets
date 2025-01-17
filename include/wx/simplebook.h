///////////////////////////////////////////////////////////////////////////////
// Name:        wx/simplebook.h
// Purpose:     wxBookCtrlBase-derived class without any controller.
// Author:      Vadim Zeitlin
// Created:     2012-08-21
// Copyright:   (c) 2012 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SIMPLEBOOK_H_
#define _WX_SIMPLEBOOK_H_

#include "wx/bookctrl.h"
#include "wx/notebook.h"

#if wxUSE_BOOKCTRL

import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// wxSimplebook: a book control without any user-actionable controller.
// ----------------------------------------------------------------------------

// NB: This class doesn't use DLL export declaration as it's fully inline.

class wxSimplebook : public wxBookCtrlBase
{
public:
    wxSimplebook()
    {    
        // We don't need any border as we don't have anything to separate the
        // page contents from.
        SetInternalBorder(0);
    }

    wxSimplebook(wxWindow *parent,
                 wxWindowID winid = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = 0,
                 const std::string& name = {})
        : wxBookCtrlBase(parent, winid, pos, size, style | wxBK_TOP, name)
    {
        // We don't need any border as we don't have anything to separate the
        // page contents from.
        SetInternalBorder(0);
    }

    wxSimplebook& operator=(wxSimplebook&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const std::string& name = {})
    {
        return wxBookCtrlBase::Create(parent, winid, pos, size, style | wxBK_TOP, name);
    }


    // Methods specific to this class.

    // A method allowing to add a new page without any label (which is unused
    // by this control) and show it immediately.
    bool ShowNewPage(wxWindow* page)
    {
        return AddPage(page, std::string(), true /* select it */);
    }


    // Set effect to use for showing/hiding pages.
    void SetEffects(wxShowEffect showEffect, wxShowEffect hideEffect)
    {
        m_showEffect = showEffect;
        m_hideEffect = hideEffect;
    }

    // Or the same effect for both of them.
    void SetEffect(wxShowEffect effect)
    {
        SetEffects(effect, effect);
    }

    // And the same for time outs.
    void SetEffectsTimeouts(unsigned showTimeout, unsigned hideTimeout)
    {
        m_showTimeout = showTimeout;
        m_hideTimeout = hideTimeout;
    }

    void SetEffectTimeout(unsigned timeout)
    {
        SetEffectsTimeouts(timeout, timeout);
    }


    // Implement base class pure virtual methods.

    // Page management
    bool InsertPage(size_t n,
                            wxWindow *page,
                            const std::string& text,
                            bool bSelect = false,
                            int imageId = NO_IMAGE) override
    {
        if ( !wxBookCtrlBase::InsertPage(n, page, text, bSelect, imageId) )
            return false;

        m_pageTexts.insert(m_pageTexts.begin() + n, text);

        if ( !DoSetSelectionAfterInsertion(n, bSelect) )
            page->Hide();

        return true;
    }

    int SetSelection(size_t n) override
    {
        return DoSetSelection(n, SetSelection_SendEvent);
    }

    int ChangeSelection(size_t n) override
    {
        return DoSetSelection(n);
    }

    // Neither labels nor images are supported but we still store the labels
    // just in case the user code attaches some importance to them.
    bool SetPageText(size_t n, const std::string& strText) override
    {
        wxCHECK_MSG( n < GetPageCount(), false, "Invalid page" );

        m_pageTexts[n] = strText;

        return true;
    }

    std::string GetPageText(size_t n) const override
    {
        wxCHECK_MSG( n < GetPageCount(), "", "Invalid page" );

        return m_pageTexts[n];
    }

    bool SetPageImage([[maybe_unused]] size_t n, [[maybe_unused]] int imageId) override
    {
        return false;
    }

    int GetPageImage([[maybe_unused]] size_t n) const override
    {
        return NO_IMAGE;
    }

    // Override some wxWindow methods too.
    void SetFocus() override
    {
        wxWindow* const page = GetCurrentPage();
        if ( page )
            page->SetFocus();
    }

protected:
    void UpdateSelectedPage([[maybe_unused]] size_t newsel) override
    {
        // Nothing to do here, but must be overridden to avoid the assert in
        // the base class version.
    }

    wxBookCtrlEvent* CreatePageChangingEvent() const override
    {
        return new wxBookCtrlEvent(wxEVT_BOOKCTRL_PAGE_CHANGING,
                                   GetId());
    }

    void MakeChangedEvent(wxBookCtrlEvent& event) override
    {
        event.SetEventType(wxEVT_BOOKCTRL_PAGE_CHANGED);
    }

    wxWindow *DoRemovePage(size_t page) override
    {
        wxWindow* const win = wxBookCtrlBase::DoRemovePage(page);
        if ( win )
        {
            m_pageTexts.erase(m_pageTexts.begin() + page);

            DoSetSelectionAfterRemoval(page);
        }

        return win;
    }

    void DoSize() override
    {
        wxWindow* const page = GetCurrentPage();
        if ( page )
            page->SetSize(GetPageRect());
    }

    void DoShowPage(wxWindow* page, bool show) override
    {
        if ( show )
            page->ShowWithEffect(m_showEffect, m_showTimeout);
        else
            page->HideWithEffect(m_hideEffect, m_hideTimeout);
    }

private:
    std::vector<std::string> m_pageTexts;

    wxShowEffect m_showEffect{wxShowEffect::None};
    wxShowEffect m_hideEffect{wxShowEffect::None};

    unsigned int m_showTimeout{0};
    unsigned int m_hideTimeout{0};
};

#endif // wxUSE_BOOKCTRL

#endif // _WX_SIMPLEBOOK_H_
