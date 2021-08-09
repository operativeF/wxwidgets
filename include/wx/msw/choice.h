/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/choice.h
// Purpose:     wxChoice class
// Author:      Julian Smart
// Modified by: Vadim Zeitlin to derive from wxChoiceBase
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CHOICE_H_
#define _WX_CHOICE_H_

#include <vector>

struct tagCOMBOBOXINFO;

// ----------------------------------------------------------------------------
// Choice item
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxChoice : public wxChoiceBase
{
public:
    // ctors
    wxChoice() = default;
    ~wxChoice() override;

    wxChoice(wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos,
        const wxSize& size,
        const std::vector<std::string>& choices = {},
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const std::string& name = wxChoiceNameStr)
    {
        Create(parent, id, pos, size, choices, style, validator, name);
    }

    wxChoice(const wxChoice&) = delete;
    wxChoice& operator=(const wxChoice&) = delete;
    wxChoice(wxChoice&&) = default;
    wxChoice& operator=(wxChoice&&) = default;

    [[maybe_unused]] bool Create(wxWindow* parent,
                wxWindowID id,
                const wxPoint& pos,
                const wxSize& size,
                const std::vector<std::string>& choices,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxChoiceNameStr);

    bool Show(bool show = true) override;

    void SetLabel(const std::string& label) override;

    size_t GetCount() const override;
    int GetSelection() const override;
    int GetCurrentSelection() const override;
    void SetSelection(int n) override;

    int FindString(std::string_view s, bool bCase = false) const override;
    std::string GetString(unsigned int n) const override;
    void SetString(unsigned int n, const std::string& s) override;

    wxVisualAttributes GetDefaultAttributes() const override
    {
        return GetClassDefaultAttributes(GetWindowVariant());
    }

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWINDOW_VARIANT_NORMAL);

    // MSW only
    bool MSWCommand(WXUINT param, WXWORD id) override;
    WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override;
    WXHBRUSH MSWControlColor(WXHDC hDC, WXHWND hWnd) override;
    bool MSWShouldPreProcessMessage(WXMSG *pMsg) override;
    WXDWORD MSWGetStyle(long style, WXDWORD *exstyle) const override;

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

protected:
    // choose the default border for this window
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    void DoDeleteOneItem(unsigned int n) override;
    void DoClear() override;

    int DoInsertItems(const std::vector<std::string>& items,
                              unsigned int pos,
                              void **clientData, wxClientDataType type) override;

    void DoMoveWindow(int x, int y, int width, int height) override;
    void DoSetItemClientData(unsigned int n, void* clientData) override;
    void* DoGetItemClientData(unsigned int n) const override;

    // MSW implementation
    wxSize DoGetBestSize() const override;
    wxSize DoGetSize() const override;
    void DoSetSize(int x, int y,
                           int width, int height,
                           int sizeFlags = wxSIZE_AUTO) override;
    wxSize DoGetSizeFromTextSize(int xlen, int ylen = -1) const override;

    // Show or hide the popup part of the control.
    void MSWDoPopupOrDismiss(bool show);

    // update the height of the drop down list to fit the number of items we
    // have (without changing the visible height)
    void MSWUpdateDropDownHeight();

    // set the height of the visible part of the control to m_heightOwn
    void MSWUpdateVisibleHeight();

    // create and initialize the control
    bool CreateAndInit(wxWindow *parent, wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       const std::vector<std::string>& choices,
                       long style,
                       const wxValidator& validator,
                       const std::string& name);

    // free all memory we have (used by Clear() and dtor)
    void Free();

    // set the height for simple combo box
    int SetHeightSimpleComboBox(int nItems) const;

#if wxUSE_DEFERRED_SIZING
    void MSWEndDeferWindowPos() override;
#endif // wxUSE_DEFERRED_SIZING

    // These variables are only used while the drop down is opened.
    //
    // The first one contains the item that had been originally selected before
    // the drop down was opened and the second one the item we should select
    // when the drop down is closed again.
    int m_lastAcceptedSelection{wxID_NONE};
    int m_pendingSelection{wxID_NONE};

    // the height of the control itself if it was set explicitly or
    // wxDefaultCoord if it hadn't
    int m_heightOwn{wxDefaultCoord};

public:
    wxClassInfo *GetClassInfo() const override;
    static wxClassInfo ms_classInfo;
    static wxObject* wxCreateObject();
};

#endif // _WX_CHOICE_H_
