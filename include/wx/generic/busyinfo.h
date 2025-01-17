/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/busyinfo.h
// Purpose:     Information window (when app is busy)
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BUSYINFO_H_
#define _WX_BUSYINFO_H_

#if wxUSE_BUSYINFO

#include "wx/object.h"

import <string>;

class wxFrame;
class wxWindow;
class wxControl;

//--------------------------------------------------------------------------------
// wxBusyInfo
//                  Displays progress information
//                  Can be used in exactly same way as wxBusyCursor
//--------------------------------------------------------------------------------

class wxBusyInfo : public wxObject
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

	wxBusyInfo& operator=(wxBusyInfo&&) = delete;

    void UpdateText(const std::string& str);
    void UpdateLabel(const std::string& str);

    ~wxBusyInfo();

private:
    void Init(const wxBusyInfoFlags& flags);

    wxFrame *m_InfoFrame;
    wxControl *m_text;
};

#endif // wxUSE_BUSYINFO
#endif // _WX_BUSYINFO_H_
