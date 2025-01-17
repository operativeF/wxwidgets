/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_dataview.cpp
// Purpose:     XRC resource for wxDataViewCtrl
// Author:      Anton Triest
// Created:     2019/06/27
// Copyright:   (c) 2019 Anton Triest, Bricsys
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_DATAVIEWCTRL

#include "wx/xrc/xh_dataview.h"
#include "wx/dataview.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxDataViewXmlHandler, wxXmlResourceHandler);

wxDataViewXmlHandler::wxDataViewXmlHandler()
     
{
    XRC_ADD_STYLE(wxDV_SINGLE);
    XRC_ADD_STYLE(wxDV_MULTIPLE);
    XRC_ADD_STYLE(wxDV_NO_HEADER);
    XRC_ADD_STYLE(wxDV_HORIZ_RULES);
    XRC_ADD_STYLE(wxDV_VERT_RULES);
    XRC_ADD_STYLE(wxDV_ROW_LINES);
    XRC_ADD_STYLE(wxDV_VARIABLE_LINE_HEIGHT);

    AddWindowStyles();
}

wxObject *wxDataViewXmlHandler::DoCreateResource()
{
    if (m_class == "wxDataViewCtrl")
    {
        return HandleCtrl();
    }
    else if (m_class == "wxDataViewListCtrl")
    {
        return HandleListCtrl();
    }
    else if (m_class == "wxDataViewTreeCtrl")
    {
        return HandleTreeCtrl();
    }
    return nullptr;
}

bool wxDataViewXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxDataViewCtrl") ||
           IsOfClass(node, "wxDataViewListCtrl") ||
           IsOfClass(node, "wxDataViewTreeCtrl");
}

wxObject* wxDataViewXmlHandler::HandleCtrl()
{
    XRC_MAKE_INSTANCE(control, wxDataViewCtrl)

    control->Create(m_parentAsWindow,
        GetID(),
        GetPosition(),
        GetSize(),
        GetStyle(),
        wxValidator{},
        GetName());

    SetupWindow(control);

    return control;
}

wxObject* wxDataViewXmlHandler::HandleListCtrl()
{
    XRC_MAKE_INSTANCE(control, wxDataViewListCtrl)

    control->Create(m_parentAsWindow,
        GetID(),
        GetPosition(),
        GetSize(),
        GetStyle(),
        wxValidator{});

    SetupWindow(control);

    return control;
}

wxObject* wxDataViewXmlHandler::HandleTreeCtrl()
{
    XRC_MAKE_INSTANCE(control, wxDataViewTreeCtrl)

    control->Create(m_parentAsWindow,
        GetID(),
        GetPosition(),
        GetSize(),
        GetStyle(),
        wxValidator{});

    wxImageList *imagelist = GetImageList();
    if (imagelist)
        control->AssignImageList(imagelist);

    SetupWindow(control);

    return control;
}

#endif // wxUSE_XRC && wxUSE_DATAVIEWCTRL
