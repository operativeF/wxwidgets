/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_odcombo.cpp
// Purpose:     XRC resource for wxRadioBox
// Author:      Alex Bligh - Based on src/xrc/xh_combo.cpp
// Created:     2006/06/19
// Copyright:   (c) 2006 Alex Bligh
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_ODCOMBOBOX

#include "wx/xrc/xh_odcombo.h"

#include "wx/intl.h"
#include "wx/textctrl.h"

#include "wx/odcombo.h"

#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxOwnerDrawnComboBoxXmlHandler, wxXmlResourceHandler);

wxOwnerDrawnComboBoxXmlHandler::wxOwnerDrawnComboBoxXmlHandler()
                     
                     
{
    XRC_ADD_STYLE(wxCB_SIMPLE);
    XRC_ADD_STYLE(wxCB_SORT);
    XRC_ADD_STYLE(wxCB_READONLY);
    XRC_ADD_STYLE(wxCB_DROPDOWN);
    XRC_ADD_STYLE(wxODCB_STD_CONTROL_PAINT);
    XRC_ADD_STYLE(wxODCB_DCLICK_CYCLES);
    XRC_ADD_STYLE(wxTE_PROCESS_ENTER);
    AddWindowStyles();
}

wxObject *wxOwnerDrawnComboBoxXmlHandler::DoCreateResource()
{
    if( m_class == "wxOwnerDrawnComboBox")
    {
        // find the selection
        long selection = GetLong( "selection", -1 );

        // need to build the list of strings from children
        m_insideBox = true;
        CreateChildrenPrivately(nullptr, GetParamNode("content"));

        XRC_MAKE_INSTANCE(control, wxOwnerDrawnComboBox)

        control->Create(m_parentAsWindow,
                        GetID(),
                        GetText("value"),
                        GetPosition(), GetSize(),
                        strList,
                        GetStyle(),
                        wxValidator{},
                        GetName());

        wxSize sizeBtn=GetSize("buttonsize");

        if (sizeBtn != wxDefaultSize)
            control->SetButtonPosition(sizeBtn.x, sizeBtn.y);

        if (selection != -1)
            control->SetSelection(selection);

        SetupWindow(control);

        strList.clear();    // dump the strings

        return control;
    }
    else
    {
        // on the inside now.
        // handle <item>Label</item>

        // add to the list
        strList.push_back(GetNodeText(m_node, wxXRC_TEXT_NO_ESCAPE));

        return nullptr;
    }
}

bool wxOwnerDrawnComboBoxXmlHandler::CanHandle(wxXmlNode *node)
{
#if wxCHECK_VERSION(2,7,0)

    return (IsOfClass(node, "wxOwnerDrawnComboBox") ||
           (m_insideBox && node->GetName() == "item"));

#else

//  Avoid GCC bug - this fails on certain GCC 3.3 and 3.4 builds for an unknown reason
//  it is believed to be related to the fact IsOfClass is inline, and node->GetAttribute
//  gets passed an invalid "this" pointer. On 2.7, the function is out of line, so the
//  above should work fine. This code is left in here so this file can easily be used
//  in a version backported to 2.6. All we are doing here is expanding the macro

    bool fOurClass = node->GetAttribute("class"), {}) == wxT("wxOwnerDrawnComboBox";
    return (fOurClass ||
          (m_insideBox && node->GetName() == "item"));
#endif
}

#endif // wxUSE_XRC && wxUSE_ODCOMBOBOX
