///////////////////////////////////////////////////////////////////////////////
// Name:        wx/univ/statusbr.h
// Purpose:     wxStatusBarUniv: wxStatusBar for wxUniversal declaration
// Author:      Vadim Zeitlin
// Modified by:
// Created:     14.10.01
// Copyright:   (c) 2001 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIV_STATUSBR_H_
#define _WX_UNIV_STATUSBR_H_

#include "wx/univ/inpcons.h"
#include "wx/arrstr.h"

// ----------------------------------------------------------------------------
// wxStatusBarUniv: a window near the bottom of the frame used for status info
// ----------------------------------------------------------------------------

class wxStatusBarUniv : public wxStatusBarBase
{
public:
    wxStatusBarUniv() { Init(); }

    wxStatusBarUniv(wxWindow *parent,
                    wxWindowID id = wxID_ANY,
                    unsigned int style = wxSTB_DEFAULT_STYLE,
                    const wxString& name = wxASCII_STR(wxPanelNameStr))
    {
        Init();

        (void)Create(parent, id, style, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                unsigned int style = wxSTB_DEFAULT_STYLE,
                const wxString& name = wxASCII_STR(wxPanelNameStr));

    // implement base class methods
    void SetFieldsCount(int number = 1, const int *widths = NULL) override;
    void SetStatusWidths(int n, const int widths[]) override;

    bool GetFieldRect(int i, wxRect& rect) const override;
    void SetMinHeight(int height) override;

    int GetBorderX() const override;
    int GetBorderY() const override;

    // wxInputConsumer pure virtual
    wxWindow *GetInputWindow() const override
        { return const_cast<wxStatusBar*>(this); }

protected:
    void DoUpdateStatusText(int i) override;

    // recalculate the field widths
    void OnSize(wxSizeEvent& event);

    // draw the statusbar
    void DoDraw(wxControlRenderer *renderer) override;

    // tell them about our preferred height
    wxSize DoGetBestSize() const override;

    // override DoSetSize() to prevent the status bar height from changing
    virtual void DoSetSize(int x, int y,
                           int width, int height,
                           unsigned int sizeFlags = wxSIZE_AUTO) override;

    // get the (fixed) status bar height
    wxCoord GetHeight() const;

    // get the rectangle containing all the fields and the border between them
    //
    // also updates m_widthsAbs if necessary
    wxRect GetTotalFieldRect(wxCoord *borderBetweenFields);

    // get the rect for this field without ani side effects (see code)
    wxRect DoGetFieldRect(int n) const;

    // common part of all ctors
    void Init();

private:
    // the current status fields strings
    //wxArrayString m_statusText;

    // the absolute status fields widths
    wxArrayInt m_widthsAbs;

    wxDECLARE_DYNAMIC_CLASS(wxStatusBarUniv);
    wxDECLARE_EVENT_TABLE();
    WX_DECLARE_INPUT_CONSUMER()
};

#endif // _WX_UNIV_STATUSBR_H_

