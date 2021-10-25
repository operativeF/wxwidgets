/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/stattext.h
// Purpose:     wxStaticText class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_STATTEXT_H_
#define _WX_STATTEXT_H_

#include "wx/geometry/rect.h"

class WXDLLIMPEXP_CORE wxStaticText : public wxStaticTextBase
{
public:
    wxStaticText() = default;

    wxStaticText(wxWindow *parent,
                 wxWindowID id,
                 const std::string& label,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = 0,
                 const std::string& name = wxStaticTextNameStr)
    {
        Create(parent, id, label, pos, size, style, name);
    }

    wxStaticText& operator=(wxStaticText&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const std::string& name = wxStaticTextNameStr);

    // override some methods to resize the window properly
    void SetLabel(const std::string& label) override;
    bool SetFont( const wxFont &font ) override;

    DWORD MSWGetStyle(unsigned int flags, DWORD *exstyle = nullptr) const override;

protected:
    // implement/override some base class virtuals
    void DoSetSize(wxRect boundary, unsigned int sizeFlags = wxSIZE_AUTO) override;
    wxSize DoGetBestClientSize() const override;

    std::string WXGetVisibleLabel() const override;
    void WXSetVisibleLabel(const std::string& str) override;

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif
    // _WX_STATTEXT_H_
