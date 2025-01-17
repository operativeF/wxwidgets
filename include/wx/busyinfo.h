/////////////////////////////////////////////////////////////////////////////
// Name:        wx/busyinfo.h
// Purpose:     Information window (when app is busy)
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __BUSYINFO_H_BASE__
#define __BUSYINFO_H_BASE__

#if wxUSE_BUSYINFO

#include "wx/colour.h"
#include "wx/icon.h"

import <string>;

class wxWindow;

// This class is used to pass all the various parameters to wxBusyInfo ctor.
// According to the usual naming conventions (see wxAboutDialogInfo,
// wxFontInfo, ...) it would be called wxBusyInfoInfo, but this would have been
// rather strange, so we call it wxBusyInfoFlags instead.
//
// Methods are mostly self-explanatory except for the difference between "Text"
// and "Label": the former can contain markup, while the latter is just plain
// string which is not parsed in any way.
class wxBusyInfoFlags
{
public:
    wxBusyInfoFlags& Parent(wxWindow* parent)
        { m_parent = parent; return *this; }

    wxBusyInfoFlags& Icon(const wxIcon& icon)
        { m_icon = icon; return *this; }
    wxBusyInfoFlags& Title(const std::string& title)
        { m_title = title; return *this; }
    wxBusyInfoFlags& Text(const std::string& text)
        { m_text = text; return *this; }
    wxBusyInfoFlags& Label(const std::string& label)
        { m_label = label; return *this; }

    wxBusyInfoFlags& Foreground(const wxColour& foreground)
        { m_foreground = foreground; return *this; }
    wxBusyInfoFlags& Background(const wxColour& background)
        { m_background = background; return *this; }

    wxBusyInfoFlags& Transparency(wxByte alpha)
        { m_alpha = alpha; return *this; }

private:
    wxWindow* m_parent{nullptr};

    wxIcon m_icon;
    
    std::string m_title;
    std::string m_text;
    std::string m_label;

    wxColour m_foreground,
             m_background;

    wxByte m_alpha{wxALPHA_OPAQUE};

    friend class wxBusyInfo;
};

#include "wx/generic/busyinfo.h"

#endif // wxUSE_BUSYINFO

#endif // __BUSYINFO_H_BASE__
