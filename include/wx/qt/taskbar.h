/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/taskbar.h
// Author:      Peter Most
// Copyright:   (c) Peter Most
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_TASKBAR_H_
#define _WX_QT_TASKBAR_H_

class QSystemTrayIcon;

class wxTaskBarIcon : public wxTaskBarIconBase
{
public:
    wxTaskBarIcon(wxTaskBarIconType iconType = wxTaskBarIconType::DefaultType);
    virtual ~wxTaskBarIcon();

	wxTaskBarIcon(const wxTaskBarIcon&) = delete;
	wxTaskBarIcon& operator=(const wxTaskBarIcon&) = delete;

    // Accessors
    bool IsOk() const { return false; }
    bool IsIconInstalled() const { return false; }

    // Operations
    virtual bool SetIcon(const wxIcon& icon,
                         const wxString& tooltip = wxEmptyString) override;
    bool RemoveIcon() override;
    bool PopupMenu(wxMenu *menu) override;

private:
    QSystemTrayIcon *m_qtSystemTrayIcon;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_QT_TASKBAR_H_
