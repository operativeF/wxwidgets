///////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_aui.cpp
// Purpose:     Implementation of wxAUI XRC handler.
// Author:      Andrea Zanellato, Steve Lamerton (wxAuiNotebook)
// Created:     2011-09-18
// Copyright:   (c) 2011 wxWidgets Team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_AUI

#include "wx/xrc/xh_aui.h"
#include "wx/aui/framemanager.h"
#include "wx/aui/auibook.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxAuiXmlHandler, wxXmlResourceHandler)

wxAuiXmlHandler::wxAuiXmlHandler()
                
                  
{
    XRC_ADD_STYLE(wxAUI_MGR_ALLOW_ACTIVE_PANE);
    XRC_ADD_STYLE(wxAUI_MGR_ALLOW_FLOATING);
    XRC_ADD_STYLE(wxAUI_MGR_DEFAULT);
    XRC_ADD_STYLE(wxAUI_MGR_HINT_FADE);
    XRC_ADD_STYLE(wxAUI_MGR_LIVE_RESIZE);
    XRC_ADD_STYLE(wxAUI_MGR_NO_VENETIAN_BLINDS_FADE);
    XRC_ADD_STYLE(wxAUI_MGR_RECTANGLE_HINT);
    XRC_ADD_STYLE(wxAUI_MGR_TRANSPARENT_DRAG);
    XRC_ADD_STYLE(wxAUI_MGR_TRANSPARENT_HINT);
    XRC_ADD_STYLE(wxAUI_MGR_VENETIAN_BLINDS_HINT);

    XRC_ADD_STYLE(wxAUI_NB_DEFAULT_STYLE);
    XRC_ADD_STYLE(wxAUI_NB_TAB_SPLIT);
    XRC_ADD_STYLE(wxAUI_NB_TAB_MOVE);
    XRC_ADD_STYLE(wxAUI_NB_TAB_EXTERNAL_MOVE);
    XRC_ADD_STYLE(wxAUI_NB_TAB_FIXED_WIDTH);
    XRC_ADD_STYLE(wxAUI_NB_SCROLL_BUTTONS);
    XRC_ADD_STYLE(wxAUI_NB_WINDOWLIST_BUTTON);
    XRC_ADD_STYLE(wxAUI_NB_CLOSE_BUTTON);
    XRC_ADD_STYLE(wxAUI_NB_CLOSE_ON_ACTIVE_TAB);
    XRC_ADD_STYLE(wxAUI_NB_CLOSE_ON_ALL_TABS);
    XRC_ADD_STYLE(wxAUI_NB_MIDDLE_CLICK_CLOSE);
    XRC_ADD_STYLE(wxAUI_NB_TOP);
    XRC_ADD_STYLE(wxAUI_NB_BOTTOM);

    AddWindowStyles();
}

wxAuiManager *wxAuiXmlHandler::GetAuiManager( wxWindow *managed ) const
{
    for ( Managers::const_iterator it = m_managers.begin();
          it != m_managers.end();
          ++it )
    {
        wxAuiManager* const mgr = *it;
        if ( mgr->GetManagedWindow() == managed )
            return mgr;
    }

    return nullptr;
}

void wxAuiXmlHandler::OnManagedWindowClose( wxWindowDestroyEvent &event )
{
    auto window = dynamic_cast<wxWindow*>( event.GetEventObject() );
    for ( Managers::iterator it = m_managers.begin();
          it != m_managers.end();
          ++it )
    {
        wxAuiManager* const mgr = *it;
        if ( mgr->GetManagedWindow() == window )
        {
            mgr->UnInit();
            m_managers.erase(it);
            break;
        }
    }

    event.Skip();
}

wxObject *wxAuiXmlHandler::DoCreateResource()
{
    if (m_class == "wxAuiManager")
    {
        wxAuiManager *manager = nullptr;

        if (m_parentAsWindow)
        {
            // Cache the previous values
            bool          old_ins = m_mgrInside;
            wxAuiManager *old_mgr = m_manager;
            wxWindow     *old_win = m_window;

            // Create the manager with the specified or default style and
            // assign the new values related to this manager
            m_window    = m_parentAsWindow;
            manager     = new wxAuiManager( m_window,
                                            GetStyle("style", wxAUI_MGR_DEFAULT) );
            m_manager   = manager;
            m_mgrInside = true;

            // Add this manager to our manager vector
            m_managers.push_back(m_manager);

            // Connect the managed window destroy event to
            // automatically UnInit() later this manager
            m_window ->Bind(wxEVT_DESTROY, &wxAuiXmlHandler::OnManagedWindowClose, this);

            // Add AUI panes to this manager
            CreateChildren(m_manager);

            // Load a custom perspective if any
            if (HasParam("perspective"))
                m_manager->LoadPerspective( GetParamValue("perspective") );

            m_manager->Update();

            // Restore the previous values
            m_window    = old_win;
            m_manager   = old_mgr;
            m_mgrInside = old_ins;
        }
        else
        {
            ReportError("No wxWindow derived class to manage for this wxAuiManager.");
        }

        return manager;
    }
    else if (m_class == "wxAuiPaneInfo")
    {
        wxXmlNode *node   = GetParamNode("object");
        wxWindow  *window = nullptr;

        if (!node)
            node = GetParamNode("object_ref");

        if (node)
        {
            bool old_ins = m_mgrInside;
            m_mgrInside = false;

            wxObject *object = CreateResFromNode(node, m_window, nullptr);

            m_mgrInside = old_ins;
            window = dynamic_cast<wxWindow*>( object );

            if (!window && object)
            {
                ReportError( node, "wxAuiPaneInfo child must be a window." );
            }
        }

        if (window)
        {
            wxAuiPaneInfo paneInfo = wxAuiPaneInfo();

            wxString name = GetName();              paneInfo.Name( name );
// Caption
            if ( HasParam("caption")) )         paneInfo.Caption( GetText(wxS("caption") );
            if ( HasParam("caption_visible")) ) paneInfo.CaptionVisible( GetBool(wxS("caption_visible") );
// Buttons
            if ( HasParam("close_button")) )    paneInfo.CloseButton( GetBool(wxS("close_button") );
            if ( HasParam("minimize_button")) ) paneInfo.MinimizeButton( GetBool(wxS("minimize_button") );
            if ( HasParam("maximize_button")) ) paneInfo.MaximizeButton( GetBool(wxS("maximize_button") );
            if ( HasParam("pin_button")) )      paneInfo.PinButton( GetBool(wxS("pin_button") );
            if ( HasParam("gripper")) )         paneInfo.Gripper( GetBool(wxS("gripper") );
// Appearance
            if ( HasParam("pane_border")) )     paneInfo.PaneBorder( GetBool(wxS("pane_border") );
// State
            if ( HasParam("dock") )            paneInfo.Dock();
            else if ( HasParam("float") )      paneInfo.Float();

// Dockable Directions
            if ( HasParam("top_dockable")) )    paneInfo.TopDockable( GetBool(wxS("top_dockable") );
            if ( HasParam("bottom_dockable")) ) paneInfo.BottomDockable( GetBool(wxS("bottom_dockable") );
            if ( HasParam("left_dockable")) )   paneInfo.LeftDockable( GetBool(wxS("left_dockable") );
            if ( HasParam("right_dockable")) )  paneInfo.RightDockable( GetBool(wxS("right_dockable") );
// Behaviours
            if ( HasParam("dock_fixed")) )      paneInfo.DockFixed( GetBool(wxS("dock_fixed") );
            if ( HasParam("resizable")) )       paneInfo.Resizable( GetBool(wxS("resizable") );
            if ( HasParam("movable")) )         paneInfo.Movable( GetBool(wxS("movable") );
            if ( HasParam("floatable")) )       paneInfo.Floatable( GetBool(wxS("floatable") );
// Sizes
            if ( HasParam("floating_size")) )   paneInfo.FloatingSize( GetSize(wxS("floating_size") );
            if ( HasParam("min_size")) )        paneInfo.MinSize( GetSize(wxS("min_size") );
            if ( HasParam("max_size")) )        paneInfo.MaxSize( GetSize(wxS("max_size") );
            if ( HasParam("best_size")) )       paneInfo.BestSize( GetSize(wxS("best_size") );
// Positions
            if ( HasParam("row")) )             paneInfo.Row( GetLong(wxS("row") );
            if ( HasParam("layer")) )           paneInfo.Layer( GetLong(wxS("layer") );
            if ( HasParam("default_pane") )    paneInfo.DefaultPane();
            else if( HasParam("toolbar_pane") ) paneInfo.ToolbarPane();

// Directions - CenterPane()/CentrePane != Center()/Centre()
            if ( HasParam("center_pane" ) ||
                 HasParam("centre_pane") )     paneInfo.CenterPane();
            if ( HasParam("direction")) )       paneInfo.Direction( GetLong(wxS("direction") );
            else if ( HasParam("top") )        paneInfo.Top();
            else if ( HasParam("bottom") )     paneInfo.Bottom();
            else if ( HasParam("left") )       paneInfo.Left();
            else if ( HasParam("right") )      paneInfo.Right();
            else if ( HasParam("center") ||
                      HasParam("centre") )     paneInfo.Center();

            m_manager->AddPane(window, paneInfo);
        }
        else
        {
            ReportError("No wxWindow derived class object specified inside wxAuiPaneInfo.");
        }

        return window;
    }
    else if (m_class == "notebookpage")
    {
        wxXmlNode *anb = GetParamNode("object");

        if (!anb)
            anb = GetParamNode("object_ref");

        if (anb)
        {
            bool old_ins = m_anbInside;
            m_anbInside = false;
            wxObject *item = CreateResFromNode(anb, m_notebook, nullptr);
            m_anbInside = old_ins;
            wxWindow *wnd = dynamic_cast<wxWindow*>(item);

            if (wnd)
            {
                if ( HasParam("bitmap") )
                {
                    m_notebook->AddPage(wnd,
                                        GetText("label"),
                                        GetBool("selected"),
                                        GetBitmap("bitmap", wxART_OTHER));
                }
                else
                {
                    m_notebook->AddPage(wnd,
                                        GetText("label"),
                                        GetBool("selected"));
                }
            }
            else
            {
                ReportError(anb, "notebookpage child must be a window");
            }
            return wnd;
        }
        else
        {
            ReportError("notebookpage must have a window child");
            return nullptr;
        }
    }
    else // if (m_class == "wxAuiNotebook")
    {
        XRC_MAKE_INSTANCE(anb, wxAuiNotebook)

        anb->Create(m_parentAsWindow,
                    GetID(),
                    GetPosition(),
                    GetSize(),
                    GetStyle("style"));

        SetupWindow(anb);

        wxAuiNotebook *old_par = m_notebook;
        m_notebook = anb;
        bool old_ins = m_anbInside;
        m_anbInside = true;
        CreateChildren(m_notebook, true/*only this handler*/);
        m_anbInside = old_ins;
        m_notebook = old_par;

        return anb;
    }
}

bool wxAuiXmlHandler::CanHandle( wxXmlNode *node )
{
    return ((!m_mgrInside && IsOfClass(node, "wxAuiManager"))  ||
            (m_mgrInside && IsOfClass(node, "wxAuiPaneInfo"))  ||
            (!m_anbInside && IsOfClass(node, "wxAuiNotebook")) ||
            (m_anbInside && IsOfClass(node, "notebookpage"))   );
}

#endif // wxUSE_XRC && wxUSE_AUI
