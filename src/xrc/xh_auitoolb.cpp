/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_toolb.cpp
// Purpose:     XRC resource for wxAuiToolBar
// Author:      Vaclav Slavik
// Created:     2000/08/11
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_AUI

#include "wx/bitmap.h"
#include "wx/xml/xml.h"

#include "wx/frame.h"
#include "wx/log.h"
#include "wx/toolbar.h"

#include "wx/xrc/xh_auitoolb.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxAuiToolBarXmlHandler, wxXmlResourceHandler);

wxAuiToolBarXmlHandler::wxAuiToolBarXmlHandler()
     
{
    XRC_ADD_STYLE(wxAUI_TB_TEXT);
    XRC_ADD_STYLE(wxAUI_TB_NO_TOOLTIPS);
    XRC_ADD_STYLE(wxAUI_TB_NO_AUTORESIZE);
    XRC_ADD_STYLE(wxAUI_TB_GRIPPER);
    XRC_ADD_STYLE(wxAUI_TB_OVERFLOW);
    XRC_ADD_STYLE(wxAUI_TB_VERTICAL);
    XRC_ADD_STYLE(wxAUI_TB_HORZ_LAYOUT);
    XRC_ADD_STYLE(wxAUI_TB_HORIZONTAL);
    XRC_ADD_STYLE(wxAUI_TB_PLAIN_BACKGROUND);
    XRC_ADD_STYLE(wxAUI_TB_HORZ_TEXT);

    AddWindowStyles();
}

wxObject *wxAuiToolBarXmlHandler::DoCreateResource()
{
    if (m_class == "tool")
    {
        if ( !m_toolbar )
        {
            ReportError("tool only allowed inside a wxAuiToolBar");
            return nullptr;
        }

        wxItemKind kind = wxITEM_NORMAL;
        if (GetBool("radio"))
            kind = wxITEM_RADIO;

        if (GetBool("toggle"))
        {
            if ( kind != wxITEM_NORMAL )
            {
                ReportParamError
                (
                    "toggle",
                    "tool can't have both <radio> and <toggle> properties"
                );
            }

            kind = wxITEM_CHECK;
        }
#if wxUSE_MENUS
        // check whether we have dropdown tag inside
        wxMenu *menu = nullptr; // menu for drop down items
        wxXmlNode * const nodeDropdown = GetParamNode("dropdown");
        if ( nodeDropdown )
        {
            // also check for the menu specified inside dropdown (it is
            // optional and may be absent for e.g. dynamically-created
            // menus)
            wxXmlNode * const nodeMenu = GetNodeChildren(nodeDropdown);
            if ( nodeMenu )
            {
                wxObject *res = CreateResFromNode(nodeMenu, nullptr);
                menu = dynamic_cast<wxMenu*>(res);
                if ( !menu )
                {
                    ReportError
                    (
                        nodeMenu,
                        "drop-down tool contents can only be a wxMenu"
                    );
                }

                if ( GetNodeNext(nodeMenu) )
                {
                    ReportError
                    (
                        GetNodeNext(nodeMenu),
                        "unexpected extra contents under drop-down tool"
                    );
                }
            }
        }
#endif
        wxAuiToolBarItem * const tool =
            m_toolbar->AddTool
                       (
                          GetID(),
                          GetText("label"),
                          GetBitmap("bitmap", wxART_TOOLBAR, m_toolSize),
                          GetBitmap("bitmap2", wxART_TOOLBAR, m_toolSize),
                          kind,
                          GetText("tooltip"),
                          GetText("longhelp"),
                          nullptr
                       );

        if ( GetBool("disabled") )
            m_toolbar->EnableTool(GetID(), false);

#if wxUSE_MENUS
        if (menu)
        {
            tool->SetHasDropDown(true);
            tool->SetUserData(m_menuHandler.RegisterMenu(m_toolbar, GetID(), menu));
        }
#endif

        return m_toolbar; // must return non-NULL
    }

    else if (m_class == "separator") || m_class == wxS("space") || m_class == wxS("label")
    {
        if ( !m_toolbar )
        {
            ReportError("separators only allowed inside wxAuiToolBar");
            return nullptr;
        }

        if ( m_class == "separator" )
            m_toolbar->AddSeparator();

        else if (m_class == "space")
        {
            // This may be a stretch spacer (the default) or a non-stretch one
            bool hasProportion = HasParam("proportion");
            bool hasWidth = HasParam("width");
            if (hasProportion && hasWidth)
            {
                ReportError("A space can't both stretch and have width");
                return nullptr;
            }

            if (hasWidth)
            {
                m_toolbar->AddSpacer
                (
                    GetLong("width")
                );
            }
            else
            {
                m_toolbar->AddStretchSpacer
                (
                    GetLong("proportion", 1l)
                );
            }
        }

        else if (m_class == "label")
        {
            m_toolbar->AddLabel
            (
                GetID(),
                GetText("label"),
                GetLong("width", -1l)
            );
        }

        return m_toolbar; // must return non-NULL
    }

    else /*<object class="wxAuiToolBar">*/
    {
        int style = GetStyle("style", wxNO_BORDER | wxTB_HORIZONTAL);
#ifdef __WXMSW__
        if (!(style & wxNO_BORDER)) style |= wxNO_BORDER;
#endif

        XRC_MAKE_INSTANCE(toolbar, wxAuiToolBar)

        toolbar->Create(m_parentAsWindow,
                         GetID(),
                         GetPosition(),
                         GetSize(),
                         style);
        toolbar->SetName(GetName());
        SetupWindow(toolbar);

        m_toolSize = GetSize("bitmapsize");
        if (!(m_toolSize == wxDefaultSize))
            toolbar->SetToolBitmapSize(m_toolSize);
        wxSize margins = GetSize("margins");
        if (!(margins == wxDefaultSize))
            toolbar->SetMargins(margins.x, margins.y);
        long packing = GetLong("packing", -1);
        if (packing != -1)
            toolbar->SetToolPacking(packing);
        long separation = GetLong("separation", -1);
        if (separation != -1)
            toolbar->SetToolSeparation(separation);

        wxXmlNode *children_node = GetParamNode("object");
        if (!children_node)
           children_node = GetParamNode("object_ref");

        if (children_node == nullptr) return toolbar;

        m_isInside = true;
        m_toolbar = toolbar;

        wxXmlNode *n = children_node;

        while (n)
        {
            if (IsObjectNode(n))
            {
                wxObject *created = CreateResFromNode(n, toolbar, nullptr);
                wxControl *control = dynamic_cast<wxControl*>(created);
                if (!IsOfClass(n, "tool") &&
                    !IsOfClass(n, "separator") &&
                    !IsOfClass(n, "label") &&
                    !IsOfClass(n, "space") &&
                    control != nullptr)
                    toolbar->AddControl(control);
            }
            n = GetNodeNext(n);
        }

        m_isInside = false;
        m_toolbar = nullptr;

        toolbar->Realize();

        return toolbar;
    }
}

bool wxAuiToolBarXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!m_isInside && IsOfClass(node, "wxAuiToolBar")) ||
            (m_isInside && IsOfClass(node, "tool")) ||
            (m_isInside && IsOfClass(node, "label")) ||
            (m_isInside && IsOfClass(node, "space")) ||
            (m_isInside && IsOfClass(node, "separator")));
}

void wxAuiToolBarXmlHandler::MenuHandler::OnDropDown(wxAuiToolBarEvent& event)
{
    if (event.IsDropDownClicked())
    {
        wxAuiToolBar *toobar = dynamic_cast<wxAuiToolBar*>(event.GetEventObject());
        if (toobar != nullptr)
        {
            wxAuiToolBarItem *item = toobar->FindTool(event.GetId());
            if (item != nullptr)
            {
                wxMenu * const menu = m_menus[item->GetUserData()];
                if (menu != nullptr)
                {
                    wxRect rect = item->GetSizerItem()->GetRect();
                    toobar->PopupMenu(menu, rect.GetRight() - toobar->FromDIP(10), rect.GetBottom());
                }
            }
        }
    }
    else
    {
        event.Skip();
    }
}

unsigned
wxAuiToolBarXmlHandler::MenuHandler::RegisterMenu(wxAuiToolBar *toolbar,
                                                  int id,
                                                  wxMenu *menu)
 {
    m_menus.push_back(menu);
    toolbar->Bind(wxEVT_COMMAND_AUITOOLBAR_TOOL_DROPDOWN,
                  &wxAuiToolBarXmlHandler::MenuHandler::OnDropDown,
                  this,
                  id);

    return m_menus.size() - 1;
}

#endif // wxUSE_XRC && wxUSE_AUI
