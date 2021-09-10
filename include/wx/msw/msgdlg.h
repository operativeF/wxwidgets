/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/msgdlg.h
// Purpose:     wxMessageDialog class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSGBOXDLG_H_
#define _WX_MSGBOXDLG_H_

#include <string>

class WXDLLIMPEXP_CORE wxMessageDialog : public wxMessageDialogBase
{
public:
    wxMessageDialog(wxWindow *parent,
                    const std::string& message,
                    const std::string& caption = wxMessageBoxCaptionStr,
                    long style = wxOK|wxCENTRE,
                    const wxPoint& WXUNUSED(pos) = wxDefaultPosition)
        : wxMessageDialogBase(parent, message, caption, style)
    {
    }

    wxMessageDialog(const wxMessageDialog&) = delete;
    wxMessageDialog& operator=(const wxMessageDialog&) = delete;
    wxMessageDialog(wxMessageDialog&&) = default;
    wxMessageDialog& operator=(wxMessageDialog&&) = default;

    ~wxMessageDialog() = default;

    int ShowModal() override;

    long GetEffectiveIcon() const override;

    // implementation-specific

    // return the font used for the text in the message box
    static wxFont GetMessageFont();

protected:
    // Override this as task dialogs are always centered on parent.
    void DoCentre(int dir) override;

private:
    // hook procedure used to adjust the message box beyond what the standard
    // MessageBox() function can do for us
    static WXLRESULT wxCALLBACK HookFunction(int code, WXWPARAM, WXLPARAM);

    static const struct ButtonAccessors
    {
        int id;
        std::string (wxMessageDialog::*getter)() const;
    } ms_buttons[];

    // replace the static text control with a text control in order to show
    // scrollbar (and also, incidentally, allow text selection)
    void ReplaceStaticWithEdit();

    // adjust the button labels
    //
    // this is called from HookFunction() and our HWND is valid at this moment
    void AdjustButtonLabels();

    // offset all buttons starting from the first one given by dx to the right
    void OffsetButtonsStartingFrom(int first, int dx);

    // used by ShowModal() to display a message box when task dialogs
    // aren't available.
    int ShowMessageBox();


    WXHANDLE m_hook{nullptr}; // HHOOK used to position the message box

public:
	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};


#endif // _WX_MSGBOXDLG_H_
