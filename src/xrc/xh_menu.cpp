/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_menu.cpp
// Purpose:     XRC resource for menus and menubars
// Author:      Vaclav Slavik
// Created:     2000/03/05
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_MENUS

#include "wx/xrc/xh_menu.h"

#include "wx/frame.h"
#include "wx/log.h"
#include "wx/menu.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxMenuXmlHandler, wxXmlResourceHandler);

wxMenuXmlHandler::wxMenuXmlHandler() 
         
{
    XRC_ADD_STYLE(wxMENU_TEAROFF);
}

wxObject *wxMenuXmlHandler::DoCreateResource()
{
    if (m_class == "wxMenu")
    {
        wxMenu *menu = m_instance ? wxStaticCast(m_instance, wxMenu)
                                  : new wxMenu(GetStyle());

        wxString title = GetText("label");
        wxString help = GetText("help");

        bool oldins = m_insideMenu;
        m_insideMenu = true;
        CreateChildren(menu, true/*only this handler*/);
        m_insideMenu = oldins;

#if wxUSE_MENUBAR
        wxMenuBar *p_bar = wxDynamicCast(m_parent, wxMenuBar);
        if (p_bar)
        {
            p_bar->Append(menu, title);
        }
        else
#endif // wxUSE_MENUBAR
        {
            wxMenu *p_menu = wxDynamicCast(m_parent, wxMenu);
            if (p_menu)
            {
                p_menu->Append(GetID(), title, menu, help);
                if (HasParam("enabled"))
                    p_menu->Enable(GetID(), GetBool("enabled"));
            }
        }

        return menu;
    }

    else
    {
        wxMenu *p_menu = wxDynamicCast(m_parent, wxMenu);

        if (m_class == "separator")
            p_menu->AppendSeparator();
        else if (m_class == "break")
            p_menu->Break();
        else /*wxMenuItem*/
        {
            int id = GetID();
            wxString label = GetText("label");
#if wxUSE_ACCEL
            wxString accel = GetText("accel", false);
#endif // wxUSE_ACCEL

            wxItemKind kind = wxITEM_NORMAL;
            if (GetBool("radio"))
                kind = wxITEM_RADIO;
            if (GetBool("checkable"))
            {
                if ( kind != wxITEM_NORMAL )
                {
                    ReportParamError
                    (
                        "checkable",
                        "menu item can't have both <radio> and <checkable> properties"
                    );
                }

                kind = wxITEM_CHECK;
            }

            wxMenuItem *mitem = new wxMenuItem(p_menu, id, label,
                                               GetText("help"), kind);
#if wxUSE_ACCEL
            if (!accel.empty())
            {
                wxAcceleratorEntry entry;
                if (entry.FromString(accel))
                    mitem->SetAccel(&entry);
            }
#endif // wxUSE_ACCEL

#if !defined(__WXMSW__) || wxUSE_OWNER_DRAWN
            if (HasParam("bitmap"))
            {
                // currently only wxMSW has support for using different checked
                // and unchecked bitmaps for menu items
#ifdef __WXMSW__
                if (HasParam("bitmap2"))
                    mitem->SetBitmaps(GetBitmap("bitmap2", wxART_MENU),
                                      GetBitmap("bitmap", wxART_MENU));
                else
#endif // __WXMSW__
                    mitem->SetBitmap(GetBitmap("bitmap", wxART_MENU));
            }
#endif
            p_menu->Append(mitem);
            mitem->Enable(GetBool("enabled", true));
            if (kind == wxITEM_CHECK)
                mitem->Check(GetBool("checked"));
        }
        return nullptr;
    }
}



bool wxMenuXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxMenu") ||
           (m_insideMenu &&
               (IsOfClass(node, "wxMenuItem") ||
                IsOfClass(node, "break") ||
                IsOfClass(node, "separator"))
           );
}

#if wxUSE_MENUBAR

wxIMPLEMENT_DYNAMIC_CLASS(wxMenuBarXmlHandler, wxXmlResourceHandler);

wxMenuBarXmlHandler::wxMenuBarXmlHandler()  
{
    XRC_ADD_STYLE(wxMB_DOCKABLE);
}

wxObject *wxMenuBarXmlHandler::DoCreateResource()
{
    wxMenuBar *menubar = nullptr;

    const int style = GetStyle();
    wxASSERT_MSG(!style || !m_instance,
                 "cannot use <style> with pre-created menubar");

    if ( m_instance )
        menubar = wxDynamicCast(m_instance, wxMenuBar);
    if ( !menubar )
        menubar = new wxMenuBar(style);

    CreateChildren(menubar);

    if (m_parentAsWindow)
    {
        wxFrame *parentFrame = wxDynamicCast(m_parent, wxFrame);
        if (parentFrame)
            parentFrame->SetMenuBar(menubar);
    }

    return menubar;
}



bool wxMenuBarXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxMenuBar");
}

#endif // wxUSE_MENUBAR

#endif // wxUSE_XRC && wxUSE_MENUS
