/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richmsgdlg.h
// Purpose:     wxRichMessageDialogBase
// Author:      Rickard Westerlund
// Created:     2010-07-03
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RICHMSGDLG_H_BASE_
#define _WX_RICHMSGDLG_H_BASE_

#if wxUSE_RICHMSGDLG

#include "wx/dialogflags.h"
#include "wx/msgdlg.h"

#include <string>

// Extends a message dialog with an optional checkbox and user-expandable
// detailed text.
class WXDLLIMPEXP_CORE wxRichMessageDialogBase : public wxGenericMessageDialog
{
public:
    wxRichMessageDialogBase( wxWindow *parent,
                             const std::string& message,
                             const std::string& caption,
                             DialogFlags style )
        : wxGenericMessageDialog( parent, message, caption, style ),
          m_detailsExpanderCollapsedLabel( wxGetTranslation("&See details") ),
          m_detailsExpanderExpandedLabel( wxGetTranslation("&Hide details") )
    {}

    ~wxRichMessageDialogBase() = default;

    wxRichMessageDialogBase(const wxRichMessageDialogBase&) = delete;
    wxRichMessageDialogBase& operator=(const wxRichMessageDialogBase&) = delete;
    wxRichMessageDialogBase(wxRichMessageDialogBase&&) = default;
    wxRichMessageDialogBase& operator=(wxRichMessageDialogBase&&) = default;

    void ShowCheckBox(const wxString& checkBoxText, bool checked = false)
    {
        m_checkBoxText = checkBoxText;
        m_checkBoxValue = checked;
    }

    wxString GetCheckBoxText() const { return m_checkBoxText; }

    void ShowDetailedText(const wxString& detailedText)
        { m_detailedText = detailedText; }

    wxString GetDetailedText() const { return m_detailedText; }

    virtual bool IsCheckBoxChecked() const { return m_checkBoxValue; }

    void SetFooterText(const wxString& footerText)
        { m_footerText = footerText; }

    wxString GetFooterText() const { return m_footerText; }

    void SetFooterIcon(int icon)
        { m_footerIcon = icon; }

    int GetFooterIcon() const { return m_footerIcon; }

protected:
    const wxString m_detailsExpanderCollapsedLabel;
    const wxString m_detailsExpanderExpandedLabel;

    wxString m_checkBoxText;
    wxString m_detailedText;
    wxString m_footerText;
    int m_footerIcon{0};
    bool m_checkBoxValue{false};

private:
    void ShowDetails(bool shown);
};

// Always include the generic version as it's currently used as the base class
// by the MSW native implementation too.
#include "wx/generic/richmsgdlgg.h"

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    #include "wx/msw/richmsgdlg.h"
#else
    class WXDLLIMPEXP_CORE wxRichMessageDialog
                           : public wxGenericRichMessageDialog
    {
    public:
        wxRichMessageDialog( wxWindow *parent,
                             const std::string& message,
                             const std::string& caption = wxMessageBoxCaptionStr,
                             DialogFlags style = {wxDialogFlags::OK, wxDialogFlags::Centered} )
            : wxGenericRichMessageDialog( parent, message, caption, style )
            { }

        wxRichMessageDialog(const wxRichMessageDialog&) = delete;
        wxRichMessageDialog& operator=(const wxRichMessageDialog&) = delete;

        wxClassInfo *wxGetClassInfo() const;
        static wxClassInfo ms_classInfo;
        static wxObject* wxCreateObject();
    };
#endif

#endif // wxUSE_RICHMSGDLG

#endif // _WX_RICHMSGDLG_H_BASE_
