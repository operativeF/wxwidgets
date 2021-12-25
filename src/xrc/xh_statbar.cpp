/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_statbar.cpp
// Purpose:     XRC resource for wxStatusBar
// Author:      Brian Ravnsgaard Riis
// Created:     2004/01/21
// Copyright:   (c) 2004 Brian Ravnsgaard Riis
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_STATUSBAR

#include "wx/xrc/xh_statbar.h"

#include "wx/string.h"
#include "wx/log.h"
#include "wx/frame.h"
#include "wx/statusbr.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxStatusBarXmlHandler, wxXmlResourceHandler);

wxStatusBarXmlHandler::wxStatusBarXmlHandler()
                      
{
    XRC_ADD_STYLE(wxSTB_SIZEGRIP);
    XRC_ADD_STYLE(wxSTB_SHOW_TIPS);
    XRC_ADD_STYLE(wxSTB_ELLIPSIZE_START);
    XRC_ADD_STYLE(wxSTB_ELLIPSIZE_MIDDLE);
    XRC_ADD_STYLE(wxSTB_ELLIPSIZE_END);
    XRC_ADD_STYLE(wxSTB_DEFAULT_STYLE);

    // compat style name:
    XRC_ADD_STYLE(wxST_SIZEGRIP);

    AddWindowStyles();
}

wxObject *wxStatusBarXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(statbar, wxStatusBar)

    statbar->Create(m_parentAsWindow,
                    GetID(),
                    GetStyle(),
                    GetName());

    int fields = GetLong("fields", 1);
    wxString widths = GetParamValue("widths");
    wxString styles = GetParamValue("styles");

    if (fields > 1 && !widths.IsEmpty())
    {
        int *width = new int[fields];

        for (int i = 0; i < fields; ++i)
        {
            width[i] = wxAtoi(widths.BeforeFirst(wxT(',')));
            if(widths.Find(wxT(',')))
                widths.Remove(0, widths.Find(wxT(',')) + 1);
        }
        statbar->SetFieldsCount(fields, width);
        delete[] width;
    }
    else
        statbar->SetFieldsCount(fields);

    if (!styles.empty())
    {
        int *style = new int[fields];
        for (int i = 0; i < fields; ++i)
        {
            style[i] = wxSB_NORMAL;

            wxString first = styles.BeforeFirst(wxT(','));
            if (first == "wxSB_NORMAL")
                style[i] = wxSB_NORMAL;
            else if (first == "wxSB_FLAT")
                style[i] = wxSB_FLAT;
            else if (first == "wxSB_RAISED")
                style[i] = wxSB_RAISED;
            else if (first == "wxSB_SUNKEN")
                style[i] = wxSB_SUNKEN;
            else if (!first.empty())
            {
                ReportParamError
                (
                    "styles",
                    wxString::Format
                    (
                        "unknown status bar field style \"%s\"",
                        first
                    )
                );
            }

            if(styles.Find(wxT(',')))
                styles.Remove(0, styles.Find(wxT(',')) + 1);
        }
        statbar->SetStatusStyles(fields, style);
        delete [] style;
    }

    CreateChildren(statbar);

    if (m_parentAsWindow)
    {
        wxFrame *parentFrame = dynamic_cast<wxFrame*>(m_parent);
        if (parentFrame)
            parentFrame->SetStatusBar(statbar);
    }

    return statbar;
}

bool wxStatusBarXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxStatusBar");
}

#endif // wxUSE_XRC && wxUSE_STATUSBAR
