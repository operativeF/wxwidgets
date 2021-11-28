/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_sizer.h
// Purpose:     XML resource handler for wxBoxSizer
// Author:      Vaclav Slavik
// Created:     2000/04/24
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_SIZER_H_
#define _WX_XH_SIZER_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC

#include "wx/sizer.h"
#include "wx/gbsizer.h"

class wxSizerXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxSizerXmlHandler);

public:
    wxSizerXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

protected:
    virtual wxSizer* DoCreateSizer(const wxString& name);
    virtual bool IsSizerNode(wxXmlNode *node) const;

private:
    bool m_isInside{false};
    bool m_isGBS{false};

    wxSizer *m_parentSizer{nullptr};


    wxObject* Handle_sizeritem();
    wxObject* Handle_spacer();
    wxObject* Handle_sizer();
    wxSizer*  Handle_wxBoxSizer();
#if wxUSE_STATBOX
    wxSizer*  Handle_wxStaticBoxSizer();
#endif
    wxSizer*  Handle_wxGridSizer();
    wxFlexGridSizer* Handle_wxFlexGridSizer();
    wxGridBagSizer* Handle_wxGridBagSizer();
    wxSizer*  Handle_wxWrapSizer();

    bool ValidateGridSizerChildren();
    void SetFlexibleMode(wxFlexGridSizer* fsizer);
    void SetGrowables(wxFlexGridSizer* fsizer, const wxChar* param, bool rows);
    wxGBPosition GetGBPos();
    wxGBSpan GetGBSpan();
    wxSizerItem* MakeSizerItem();
    void SetSizerItemAttributes(wxSizerItem* sitem);
    void AddSizerItem(wxSizerItem* sitem);
    int GetSizerFlags();
};

#if wxUSE_BUTTON

class wxStdDialogButtonSizerXmlHandler
    : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxStdDialogButtonSizerXmlHandler);

public:
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    bool m_isInside{false};
    wxStdDialogButtonSizer *m_parentSizer{nullptr};
};

#endif // wxUSE_BUTTON

#endif // wxUSE_XRC

#endif // _WX_XH_SIZER_H_
