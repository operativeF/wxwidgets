/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/busyinfo.h
// Purpose:     Information window (when app is busy)
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BUSYINFO_H_
#define _WX_BUSYINFO_H_

#include "wx/defs.h"

#if wxUSE_BUSYINFO

#include "wx/object.h"

class WXDLLIMPEXP_FWD_CORE wxFrame;
class WXDLLIMPEXP_FWD_CORE wxWindow;
class WXDLLIMPEXP_FWD_CORE wxControl;

//--------------------------------------------------------------------------------
// wxBusyInfo
//                  Displays progress information
//                  Can be used in exactly same way as wxBusyCursor
//--------------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxBusyInfo : public wxObject
{
public:
    wxBusyInfo(const wxBusyInfoFlags& flags)
    {
        Init(flags);
    }

    wxBusyInfo(const std::string& message, wxWindow *parent = nullptr)
    {
        Init(wxBusyInfoFlags().Parent(parent).Label(message));
    }

    void UpdateText(const std::string& str);
    void UpdateLabel(const std::string& str);

    ~wxBusyInfo() override;

private:
    void Init(const wxBusyInfoFlags& flags);

    wxFrame *m_InfoFrame;
    wxControl *m_text;

    wxBusyInfo(const wxBusyInfo&) = delete;
	wxBusyInfo& operator=(const wxBusyInfo&) = delete;
};

#endif // wxUSE_BUSYINFO
#endif // _WX_BUSYINFO_H_
