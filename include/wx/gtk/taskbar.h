///////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/taskbar.h
// Purpose:     wxTaskBarIcon class for GTK2
// Author:      Paul Cornett
// Created:     2009-02-08
// Copyright:   (c) 2009 Paul Cornett
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_TASKBARICON_H_
#define _WX_GTK_TASKBARICON_H_

class wxTaskBarIcon: public wxTaskBarIconBase
{
public:
    wxTaskBarIcon(wxTaskBarIconType iconType = wxTaskBarIconType::DefaultType);
    ~wxTaskBarIcon();
    bool SetIcon(const wxIcon& icon, const wxString& tooltip = wxString()) override;
    bool RemoveIcon() override;
    bool PopupMenu(wxMenu* menu) override;
    bool IsOk() const { return true; }
    bool IsIconInstalled() const;

    class Private;

private:
    Private* m_priv;

    wxDECLARE_DYNAMIC_CLASS(wxTaskBarIcon);
    wxTaskBarIcon(const wxTaskBarIcon&) = delete;
	wxTaskBarIcon& operator=(const wxTaskBarIcon&) = delete;
};

#endif // _WX_GTK_TASKBARICON_H_
