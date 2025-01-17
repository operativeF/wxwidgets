///////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_editlbox.h
// Purpose:     declaration of wxEditableListBox XRC handler
// Author:      Vadim Zeitlin
// Created:     2009-06-04
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XRC_XH_EDITLBOX_H_
#define _WX_XRC_XH_EDITLBOX_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_EDITABLELISTBOX

// ----------------------------------------------------------------------------
// wxEditableListBoxXmlHandler: XRC handler for wxEditableListBox
// ----------------------------------------------------------------------------

class wxEditableListBoxXmlHandler : public wxXmlResourceHandler
{
public:
    wxEditableListBoxXmlHandler();

    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    bool m_insideBox{false};
    std::vector<std::string> m_items;

    wxDECLARE_DYNAMIC_CLASS(wxEditableListBoxXmlHandler);
};

#endif // wxUSE_XRC && wxUSE_EDITABLELISTBOX

#endif // _WX_XRC_XH_EDITLBOX_H_

