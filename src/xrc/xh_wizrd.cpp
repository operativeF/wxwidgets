/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_wizrd.cpp
// Purpose:     XRC resource for wxWizard
// Author:      Vaclav Slavik
// Created:     2003/03/01
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_WIZARDDLG

#include "wx/xrc/xh_wizrd.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
#endif

#include "wx/wizard.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxWizardXmlHandler, wxXmlResourceHandler);

wxWizardXmlHandler::wxWizardXmlHandler()  
{
    XRC_ADD_STYLE(wxWIZARD_EX_HELPBUTTON);
    AddWindowStyles();
}

wxObject *wxWizardXmlHandler::DoCreateResource()
{
    if (m_class == "wxWizard")
    {
        XRC_MAKE_INSTANCE(wiz, wxWizard)

        unsigned int style = GetStyle("exstyle", 0);
        if (style != 0)
            wiz->SetExtraStyle(style);
        wiz->Create(m_parentAsWindow,
                    GetID(),
                    GetText("title"),
                    GetBitmap(),
                    GetPosition());
        SetupWindow(wiz);

        wxWizard *old = m_wizard;
        m_wizard = wiz;
        m_lastSimplePage = nullptr;
        CreateChildren(wiz, true /*this handler only*/);
        m_wizard = old;
        return wiz;
    }
    else
    {
        wxWizardPage *page;

        if (m_class == "wxWizardPageSimple")
        {
            XRC_MAKE_INSTANCE(p, wxWizardPageSimple)
            p->Create(m_wizard, nullptr, nullptr, GetBitmap());
            if (m_lastSimplePage)
                wxWizardPageSimple::Chain(m_lastSimplePage, p);
            page = p;
            m_lastSimplePage = p;
        }
        else /*if (m_class == "wxWizardPage")*/
        {
            if ( !m_instance )
            {
                ReportError("wxWizardPage is abstract class and must be subclassed");
                return nullptr;
            }

            page = wxStaticCast(m_instance, wxWizardPage);
            page->Create(m_wizard, GetBitmap());
        }

        page->SetName(GetName());
        page->SetId(GetID());

        SetupWindow(page);
        CreateChildren(page);
        return page;
    }
}

bool wxWizardXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxWizard") ||
           (m_wizard != nullptr &&
                (IsOfClass(node, "wxWizardPage") ||
                 IsOfClass(node, "wxWizardPageSimple"))
           );
}

#endif // wxUSE_XRC && wxUSE_WIZARDDLG
