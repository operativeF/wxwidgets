/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/statline.h
// Purpose:     MSW version of wxStaticLine class
// Author:      Vadim Zeitlin
// Created:     28.06.99
// Copyright:   (c) 1998 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_STATLINE_H_
#define _WX_MSW_STATLINE_H_

// ----------------------------------------------------------------------------
// wxStaticLine
// ----------------------------------------------------------------------------

class wxStaticLine : public wxStaticLineBase
{
public:
    // constructors and pseudo-constructors
    wxStaticLine() = default;

    wxStaticLine( wxWindow *parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  unsigned int style = wxLI_HORIZONTAL,
                  const std::string &name = wxStaticLineNameStr)
    {
        Create(parent, id, pos, size, style, name);
    }

    wxStaticLine& operator=(wxStaticLine&&) = delete;

    [[maybe_unused]] bool Create( wxWindow *parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = wxLI_HORIZONTAL,
                 const std::string& name = wxStaticLineNameStr);

    // overridden base class virtuals
    bool AcceptsFocus() const override { return false; }

    // usually overridden base class virtuals
    DWORD MSWGetStyle(unsigned int style, DWORD *exstyle) const override;

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_MSW_STATLINE_H_


