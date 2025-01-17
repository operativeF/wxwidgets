/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_toolb.cpp
// Purpose:     XRC resource for wxToolBar
// Author:      Vaclav Slavik
// Created:     2000/08/11
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_TOOLBAR

#include "wx/xrc/xh_toolb.h"

#include "wx/frame.h"
#include "wx/log.h"
#include "wx/menu.h"
#include "wx/toolbar.h"

#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxToolBarXmlHandler, wxXmlResourceHandler);

wxToolBarXmlHandler::wxToolBarXmlHandler()
 
{
    XRC_ADD_STYLE(wxTB_FLAT);
    XRC_ADD_STYLE(wxTB_DOCKABLE);
    XRC_ADD_STYLE(wxTB_VERTICAL);
    XRC_ADD_STYLE(wxTB_HORIZONTAL);
    XRC_ADD_STYLE(wxTB_TEXT);
    XRC_ADD_STYLE(wxTB_NOICONS);
    XRC_ADD_STYLE(wxTB_NODIVIDER);
    XRC_ADD_STYLE(wxTB_NOALIGN);
    XRC_ADD_STYLE(wxTB_HORZ_LAYOUT);
    XRC_ADD_STYLE(wxTB_HORZ_TEXT);

    XRC_ADD_STYLE(wxTB_TOP);
    XRC_ADD_STYLE(wxTB_LEFT);
    XRC_ADD_STYLE(wxTB_RIGHT);
    XRC_ADD_STYLE(wxTB_BOTTOM);

    AddWindowStyles();
}

wxObject *wxToolBarXmlHandler::DoCreateResource()
{
    if (m_class == "tool")
    {
        if ( !m_toolbar )
        {
            ReportError("tool only allowed inside a wxToolBar");
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
            if ( kind != wxITEM_NORMAL )
            {
                ReportParamError
                (
                    "dropdown",
                    "drop-down tool can't have neither <radio> nor <toggle> properties"
                );
            }

            kind = wxITEM_DROPDOWN;

            // also check for the menu specified inside dropdown (it is
            // optional and may be absent for e.g. dynamically-created
            // menus)
            wxXmlNode * const nodeMenu = nodeDropdown->GetChildren();
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

                if ( nodeMenu->GetNext() )
                {
                    ReportError
                    (
                        nodeMenu->GetNext(),
                        "unexpected extra contents under drop-down tool"
                    );
                }
            }
        }
#endif
        wxToolBarToolBase * const tool =
            m_toolbar->AddTool
                       (
                          GetID(),
                          GetText("label"),
                          GetBitmap("bitmap", wxART_TOOLBAR, m_toolSize),
                          GetBitmap("bitmap2", wxART_TOOLBAR, m_toolSize),
                          kind,
                          GetText("tooltip"),
                          GetText("longhelp")
                       );

        if ( GetBool("disabled") )
            m_toolbar->EnableTool(tool->GetId(), false);

        if ( GetBool("checked") )
        {
            if ( kind == wxITEM_NORMAL )
            {
                ReportParamError
                (
                    "checked",
                    "only <radio> nor <toggle> tools can be checked"
                );
            }
            else
            {
                m_toolbar->ToggleTool(tool->GetId(), true);
            }
        }

#if wxUSE_MENUS
        if ( menu )
            tool->SetDropdownMenu(menu);
#endif

        return m_toolbar; // must return non-NULL
    }

    else if (m_class == "separator") || m_class == wxT("space")
    {
        if ( !m_toolbar )
        {
            ReportError("separators only allowed inside wxToolBar");
            return nullptr;
        }

        if ( m_class == "separator" )
            m_toolbar->AddSeparator();
        else
            m_toolbar->AddStretchableSpace();

        return m_toolbar; // must return non-NULL
    }

    else /*<object class="wxToolBar">*/
    {
        int style = GetStyle("style", wxNO_BORDER | wxTB_HORIZONTAL);
#ifdef __WXMSW__
        if (!(style & wxNO_BORDER)) style |= wxNO_BORDER;
#endif

        XRC_MAKE_INSTANCE(toolbar, wxToolBar)

        toolbar->Create(m_parentAsWindow,
                         GetID(),
                         GetPosition(),
                         GetSize(),
                         style,
                         GetName());
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
            if ((n->GetType() == wxXML_ELEMENT_NODE) &&
                (n->GetName() == "object") || n->GetName() == wxT("object_ref"))
            {
                wxObject *created = CreateResFromNode(n, toolbar, nullptr);
                wxControl *control = dynamic_cast<wxControl*>(created);
                if (!IsOfClass(n, "tool") &&
                    !IsOfClass(n, "separator") &&
                    !IsOfClass(n, "space") &&
                    control != nullptr)
                    toolbar->AddControl(control);
            }
            n = n->GetNext();
        }

        m_isInside = false;
        m_toolbar = nullptr;

        if (m_parentAsWindow && !GetBool("dontattachtoframe"))
        {
            wxFrame *parentFrame = dynamic_cast<wxFrame*>(m_parent);
            if (parentFrame)
                parentFrame->SetToolBar(toolbar);
        }

        toolbar->Realize();

        return toolbar;
    }
}

bool wxToolBarXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!m_isInside && IsOfClass(node, "wxToolBar")) ||
            (m_isInside && IsOfClass(node, "tool")) ||
            (m_isInside && IsOfClass(node, "space")) ||
            (m_isInside && IsOfClass(node, "separator")));
}

#endif // wxUSE_XRC && wxUSE_TOOLBAR
