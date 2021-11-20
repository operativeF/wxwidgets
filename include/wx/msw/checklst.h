///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/checklst.h
// Purpose:     wxCheckListBox class - a listbox with checkable items
// Author:      Vadim Zeitlin
// Modified by:
// Created:     16.11.97
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef   __CHECKLST__H_
#define   __CHECKLST__H_

#if !wxUSE_OWNER_DRAWN
  #error  "wxCheckListBox class requires owner-drawn functionality."
#endif

#include <memory>
import <string>;
import <vector>;

struct wxOwnerDrawn;
class wxCheckListBoxItem; // fwd decl, defined in checklst.cpp

class wxCheckListBox : public wxCheckListBoxBase
{
public:
    // ctors
    wxCheckListBox() = default;
    wxCheckListBox(wxWindow *parent, wxWindowID id,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   const std::vector<std::string>& choices = {},
                   unsigned int style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   std::string_view name = wxListBoxNameStr);

    wxCheckListBox& operator=(wxCheckListBox&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                const std::vector<std::string>& choices = {},
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                std::string_view name = wxListBoxNameStr);

    // items may be checked
    bool IsChecked(unsigned int uiIndex) const override;
    void Check(unsigned int uiIndex, bool bCheck = true) override;
    virtual void Toggle(unsigned int uiIndex);

    // we create our items ourselves and they have non-standard size,
    // so we need to override these functions
    std::unique_ptr<wxOwnerDrawn> CreateLboxItem(size_t n) override;
    bool MSWOnMeasure(WXMEASUREITEMSTRUCT *item) override;

protected:
    wxSize MSWGetFullItemSize(int w, int h) const override;

    // pressing space or clicking the check box toggles the item
    void OnKeyDown(wxKeyEvent& event);
    void OnLeftClick(wxMouseEvent& event);

    // send an "item checked" event
    void SendEvent(unsigned int uiIndex)
    {
        wxCommandEvent event(wxEVT_CHECKLISTBOX, GetId());
        event.SetInt(uiIndex);
        event.SetEventObject(this);
        event.SetString(GetString(uiIndex));
        ProcessCommand(event);
    }

    void MSWUpdateFontOnDPIChange(const wxSize& newDPI) override;

    wxSize DoGetBestClientSize() const override;

    wxDECLARE_EVENT_TABLE();

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif    //_CHECKLST_H
