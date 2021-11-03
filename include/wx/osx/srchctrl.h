/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/srchctrl.h
// Purpose:     mac carbon wxSearchCtrl class
// Author:      Vince Harron
// Created:     2006-02-19
// Copyright:   Vince Harron
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SEARCHCTRL_H_
#define _WX_SEARCHCTRL_H_

#if wxUSE_SEARCHCTRL

class wxSearchWidgetImpl;

class wxSearchCtrl : public wxSearchCtrlBase
{
public:
    // creation
    // --------

    wxSearchCtrl();
    wxSearchCtrl(wxWindow *parent, wxWindowID id,
               const wxString& value = wxEmptyString,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxASCII_STR(wxSearchCtrlNameStr));

    virtual ~wxSearchCtrl();

    bool Create(wxWindow *parent, wxWindowID id,
                const wxString& value = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxSearchCtrlNameStr));

#if wxUSE_MENUS
    // get/set search button menu
    // --------------------------
    void SetMenu( wxMenu* menu ) override;
    wxMenu* GetMenu() override;
#endif  // wxUSE_MENUS

    // get/set search options
    // ----------------------
    void ShowSearchButton( bool show ) override;
    bool IsSearchButtonVisible() const override;

    void ShowCancelButton( bool show ) override;
    bool IsCancelButtonVisible() const override;

    void SetDescriptiveText(const wxString& text) override;
    wxString GetDescriptiveText() const override;

    virtual bool    HandleSearchFieldSearchHit() ;
    virtual bool    HandleSearchFieldCancelHit() ;

    wxSearchWidgetImpl * GetSearchPeer() const;

#if wxUSE_MENUS
    void OSXAfterMenuEvent() override;
#endif  // wxUSE_MENUS

protected:

    wxSize DoGetBestSize() const override;

    void Init();

#if wxUSE_MENUS
    wxMenu *m_menu;
#endif  // wxUSE_MENUS

    wxString m_descriptiveText;

private:
    wxDECLARE_DYNAMIC_CLASS(wxSearchCtrl);

    wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_SEARCHCTRL

#endif // _WX_SEARCHCTRL_H_

