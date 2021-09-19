/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/commandlinkbutton.h
// Purpose:     wxCommandLinkButton class
// Author:      Rickard Westerlund
// Created:     2010-06-11
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_COMMANDLINKBUTTON_H_
#define _WX_MSW_COMMANDLINKBUTTON_H_

#include "wx/stringutils.h"

#include <string>

// Derive from the generic version to be able to fall back to it during
// run-time if the command link buttons are not supported by the system we're
// running under.

class WXDLLIMPEXP_CORE wxCommandLinkButton : public wxGenericCommandLinkButton
{
public:
    wxCommandLinkButton ()  = default;

    wxCommandLinkButton(wxWindow *parent,
                        wxWindowID id,
                        const std::string& mainLabel = {},
                        const std::string& note = {},
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = 0,
                        const wxValidator& validator = wxDefaultValidator,
                        const std::string& name = wxButtonNameStr)
         
    {
        Create(parent, id, mainLabel, note, pos, size, style, validator, name);
    }

    wxCommandLinkButton(const wxCommandLinkButton&) = delete;
    wxCommandLinkButton& operator=(const wxCommandLinkButton&) = delete;
    wxCommandLinkButton(wxCommandLinkButton&&) = default;
    wxCommandLinkButton& operator=(wxCommandLinkButton&&) = default;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& mainLabel = {},
                const std::string& note = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxButtonNameStr);

    // do the same thing as in the generic case here
    void SetLabel(const std::string& label) override
    {
        SetMainLabelAndNote(wx::utils::BeforeFirst(label, '\n'), wx::utils::AfterFirst(label, '\n'));
    }

    void SetMainLabelAndNote(const std::string& mainLabel,
                             const std::string& note) override;

    WXDWORD MSWGetStyle(long style, WXDWORD *exstyle) const override;

protected:
    wxSize DoGetBestSize() const override;

    bool HasNativeBitmap() const override;

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_MSW_COMMANDLINKBUTTON_H_
