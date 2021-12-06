///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/notifmsg.h
// Purpose:     generic implementation of wxGenericNotificationMessage
// Author:      Vadim Zeitlin
// Created:     2007-11-24
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_NOTIFMSG_H_
#define _WX_GENERIC_NOTIFMSG_H_

// ----------------------------------------------------------------------------
// wxGenericNotificationMessage
// ----------------------------------------------------------------------------

class wxGenericNotificationMessage : public wxNotificationMessageBase
{
public:
    wxGenericNotificationMessage()
    {
        Init();
    }

    wxGenericNotificationMessage(const std::string& title,
                                 const std::string& message = {},
                                 wxWindow *parent = nullptr,
                                 int flags = wxICON_INFORMATION)
    {
        Init();
        Create(title, message, parent, flags);
    }

    wxGenericNotificationMessage& operator=(wxGenericNotificationMessage&&) = delete;

    // generic implementation-specific methods

    // get/set the default timeout (used if Timeout_Auto is specified)
    static int GetDefaultTimeout();
    static void SetDefaultTimeout(int timeout);

private:
    void Init();
};

#endif // _WX_GENERIC_NOTIFMSG_H_

