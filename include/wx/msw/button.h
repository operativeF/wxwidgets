/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/button.h
// Purpose:     wxButton class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_BUTTON_H_
#define _WX_MSW_BUTTON_H_

// ----------------------------------------------------------------------------
// Pushbutton
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxButton : public wxButtonBase
{
public:
    wxButton() = default;
    wxButton(wxWindow *parent,
             wxWindowID id,
             const std::string& label = {},
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = 0,
             const wxValidator& validator = wxDefaultValidator,
             const std::string& name = wxButtonNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    wxButton(const wxButton&) = delete;
    wxButton& operator=(const wxButton&) = delete;
    wxButton(wxButton&&) = default;
    wxButton& operator=(wxButton&&) = default;

    ~wxButton();

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& label = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxButtonNameStr);

    wxWindow *SetDefault() override;

    // implementation from now on
    void Command(wxCommandEvent& event) override;
    WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override;
    bool MSWCommand(WXUINT param, WXWORD id) override;

    WXDWORD MSWGetStyle(long style, WXDWORD *exstyle) const override;

protected:
    // send a notification event, return true if processed
    bool SendClickEvent();

    // default button handling
    void SetTmpDefault();
    void UnsetTmpDefault();

    // set or unset BS_DEFPUSHBUTTON style
    static void SetDefaultStyle(wxButton *btn, bool on);

    bool DoGetAuthNeeded() const override;
    void DoSetAuthNeeded(bool show) override;

    // true if the UAC symbol is shown
    bool m_authNeeded {false};

private:
    void OnCharHook(wxKeyEvent& event);

    wxDECLARE_EVENT_TABLE();

public:
	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_MSW_BUTTON_H_
