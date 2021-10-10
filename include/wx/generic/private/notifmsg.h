///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/private/notifmsg.h
// Purpose:     wxGenericNotificationMessage declarations
// Author:      Tobias Taschner
// Created:     2015-08-04
// Copyright:   (c) 2015 wxWidgets development team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_PRIVATE_NOTIFMSG_H_
#define _WX_GENERIC_PRIVATE_NOTIFMSG_H_

#include "wx/private/notifmsg.h"

#include "wx/dialogflags.h"

#include <string>

class wxGenericNotificationMessageImpl : public wxNotificationMessageImpl
{
public:
    wxGenericNotificationMessageImpl(wxNotificationMessageBase* notification);

    ~wxGenericNotificationMessageImpl();

    bool Show(int timeout) override;

    bool Close() override;

    void SetTitle(const std::string& title) override;

    void SetMessage(const std::string& message) override;

    void SetParent(wxWindow *parent) override;

    void SetFlags(wxDialogIconFlags flags) override;

    void SetIcon(const wxIcon& icon) override;

    bool AddAction(wxWindowID actionid, const std::string &label) override;

    // get/set the default timeout (used if Timeout_Auto is specified)
    static int GetDefaultTimeout() { return ms_timeout; }
    static void SetDefaultTimeout(int timeout);

private:
    // default timeout
    static int ms_timeout;

    // notification message is represented by a frame in this implementation
    class wxNotificationMessageWindow *m_window;

    wxGenericNotificationMessageImpl(const wxGenericNotificationMessageImpl&) = delete;
	wxGenericNotificationMessageImpl& operator=(const wxGenericNotificationMessageImpl&) = delete;
};

#endif // _WX_GENERIC_PRIVATE_NOTIFMSG_H_
