/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/bmpbuttn.h
// Purpose:     wxBitmapButton class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BMPBUTTN_H_
#define _WX_BMPBUTTN_H_

#include "wx/button.h"
#include "wx/bitmap.h"
#include "wx/brush.h"

#include <string>

class WXDLLIMPEXP_CORE wxBitmapButton : public wxBitmapButtonBase
{
public:
    wxBitmapButton() = default;

    wxBitmapButton(wxWindow *parent,
                   wxWindowID id,
                   const wxBitmap& bitmap,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const std::string& name = wxButtonNameStr)
    {
        Create(parent, id, bitmap, pos, size, style, validator, name);
    }

    wxBitmapButton(const wxBitmapButton&) = delete;
    wxBitmapButton& operator=(const wxBitmapButton&) = delete;
    wxBitmapButton(wxBitmapButton&&) = default;
    wxBitmapButton& operator=(wxBitmapButton&&) = default;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxBitmap& bitmap,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxButtonNameStr);

    bool CreateCloseButton(wxWindow* parent,
                           wxWindowID winid,
                           const std::string& name = "");
protected:
    wxDECLARE_EVENT_TABLE();

public:
	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_BMPBUTTN_H_
