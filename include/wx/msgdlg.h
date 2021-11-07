/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msgdlg.h
// Purpose:     common header and base class for wxMessageDialog
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSGDLG_H_BASE_
#define _WX_MSGDLG_H_BASE_

#include "wx/defs.h"

#if wxUSE_MSGDLG

#include "wx/dialog.h"
#include "wx/stockitem.h"

#include <string>

#include <fmt/core.h>

inline constexpr char wxMessageBoxCaptionStr[] = "Message";

// ----------------------------------------------------------------------------
// wxMessageDialogBase: base class defining wxMessageDialog interface
// ----------------------------------------------------------------------------

class wxMessageDialogBase : public wxDialog
{
public:
    // helper class for SetXXXLabels() methods: it makes it possible to pass
    // either a stock id (wxID_CLOSE) or a string ("&Close") to them
    class ButtonLabel
    {
    public:
        // ctors are not explicit, objects of this class can be implicitly
        // constructed from either stock ids or strings
        ButtonLabel(int stockId)
            : m_stockId(stockId)
        {
            wxASSERT_MSG( wxIsStockID(stockId), "invalid stock id" );
        }

        ButtonLabel(const std::string& label)
            : m_label(label), m_stockId(wxID_NONE)
        {
        }

        ButtonLabel(const char *label)
            : m_label(label), m_stockId(wxID_NONE)
        {
        }

        // default copy ctor and dtor are ok

        // get the string label, whether it was originally specified directly
        // or as a stock id -- this is only useful for platforms without native
        // stock items id support
        std::string GetAsString() const
        {
            return m_stockId == wxID_NONE
                    ? m_label
                    : wxGetStockLabel(m_stockId, wxSTOCK_FOR_BUTTON);
        }

        // return the stock id or wxID_NONE if this is not a stock label
        int GetStockId() const { return m_stockId; }

    private:
        // the label if explicitly given or empty if this is a stock item
        const std::string m_label;

        // the stock item id or wxID_NONE if m_label should be used
        const int m_stockId;
    };

    wxMessageDialogBase() = default;
    wxMessageDialogBase(wxWindow *parent,
                        const std::string& message,
                        const std::string& caption,
                        unsigned int style)
        : m_message(message),
          m_caption(caption)
    {
        m_parent = GetParentForModalDialog(parent, style);
        SetMessageDialogStyle(style);
    }

    wxMessageDialogBase& operator=(wxMessageDialogBase&&) = delete;

    std::string GetCaption() const { return m_caption; }

    // Title and caption are the same thing, GetCaption() mostly exists just
    // for compatibility.
    void SetTitle(const std::string& title) override { m_caption = title; }
    std::string GetTitle() const override { return m_caption; }


    virtual void SetMessage(const std::string& message)
    {
        m_message = message;
    }

    const std::string& wxGetMessage() const { return m_message; }

    void SetExtendedMessage(const std::string& extendedMessage)
    {
        m_extendedMessage = extendedMessage;
    }

    const std::string& GetExtendedMessage() const { return m_extendedMessage; }

    // change the dialog style flag
    void SetMessageDialogStyle(unsigned int style)
    {
        wxASSERT_MSG( ((style & wxYES_NO) == wxYES_NO) || !(style & wxYES_NO),
                      "wxYES and wxNO may only be used together" );

        wxASSERT_MSG( !(style & wxYES) || !(style & wxOK),
                      "wxOK and wxYES/wxNO can't be used together" );

        // It is common to specify just the icon, without wxOK, in the existing
        // code, especially one written by Windows programmers as MB_OK is 0
        // and so they're used to omitting wxOK. Don't complain about it but
        // just add wxOK implicitly for compatibility.
        if ( !(style & wxYES) && !(style & wxOK) )
            style |= wxOK;

        wxASSERT_MSG( (style & wxID_OK) != wxID_OK,
                      "wxMessageBox: Did you mean wxOK (and not wxID_OK)?" );

        wxASSERT_MSG( !(style & wxNO_DEFAULT) || (style & wxNO),
                      "wxNO_DEFAULT is invalid without wxNO" );

        wxASSERT_MSG( !(style & wxCANCEL_DEFAULT) || (style & wxCANCEL),
                      "wxCANCEL_DEFAULT is invalid without wxCANCEL" );

        wxASSERT_MSG( !(style & wxCANCEL_DEFAULT) || !(style & wxNO_DEFAULT),
                      "only one default button can be specified" );

        m_dialogStyle = style;
    }

    unsigned int GetMessageDialogStyle() const { return m_dialogStyle; }

    // customization of the message box buttons
    virtual bool SetYesNoLabels(const ButtonLabel& yes,const ButtonLabel& no)
    {
        DoSetCustomLabel(m_yes, yes);
        DoSetCustomLabel(m_no, no);
        return true;
    }

    virtual bool SetYesNoCancelLabels(const ButtonLabel& yes,
                                      const ButtonLabel& no,
                                      const ButtonLabel& cancel)
    {
        DoSetCustomLabel(m_yes, yes);
        DoSetCustomLabel(m_no, no);
        DoSetCustomLabel(m_cancel, cancel);
        return true;
    }

    virtual bool SetOKLabel(const ButtonLabel& ok)
    {
        DoSetCustomLabel(m_ok, ok);
        return true;
    }

    virtual bool SetOKCancelLabels(const ButtonLabel& ok,
                                   const ButtonLabel& cancel)
    {
        DoSetCustomLabel(m_ok, ok);
        DoSetCustomLabel(m_cancel, cancel);
        return true;
    }

    virtual bool SetHelpLabel(const ButtonLabel& help)
    {
        DoSetCustomLabel(m_help, help);
        return true;
    }
    // test if any custom labels were set
    bool HasCustomLabels() const
    {
        return !(m_ok.empty() && m_cancel.empty() && m_help.empty() &&
                 m_yes.empty() && m_no.empty());
    }

    // these functions return the label to be used for the button which is
    // either a custom label explicitly set by the user or the default label,
    // i.e. they always return a valid string
    std::string GetYesLabel() const
        { return m_yes.empty() ? GetDefaultYesLabel() : m_yes; }
    std::string GetNoLabel() const
        { return m_no.empty() ? GetDefaultNoLabel() : m_no; }
    std::string GetOKLabel() const
        { return m_ok.empty() ? GetDefaultOKLabel() : m_ok; }
    std::string GetCancelLabel() const
        { return m_cancel.empty() ? GetDefaultCancelLabel() : m_cancel; }
    std::string GetHelpLabel() const
        { return m_help.empty() ? GetDefaultHelpLabel() : m_help; }

    // based on message dialog style, returns exactly one of: wxICON_NONE,
    // wxICON_ERROR, wxICON_WARNING, wxICON_QUESTION, wxICON_INFORMATION,
    // wxICON_AUTH_NEEDED
    virtual unsigned int GetEffectiveIcon() const
    {
        if ( m_dialogStyle & wxICON_NONE )
            return wxICON_NONE;
        else if ( m_dialogStyle & wxICON_ERROR )
            return wxICON_ERROR;
        else if ( m_dialogStyle & wxICON_WARNING )
            return wxICON_WARNING;
        else if ( m_dialogStyle & wxICON_QUESTION )
            return wxICON_QUESTION;
        else if ( m_dialogStyle & wxICON_INFORMATION )
            return wxICON_INFORMATION;
        else if ( m_dialogStyle & wxYES )
            return wxICON_QUESTION;
        else
            return wxICON_INFORMATION;
    }

protected:
    // for the platforms not supporting separate main and extended messages
    // this function should be used to combine both of them in a single string
    std::string GetFullMessage() const
    {
        std::string msg = m_message;
        if ( !m_extendedMessage.empty() )
            msg += fmt::format("\n\n{}", m_extendedMessage);

        return msg;
    }

    std::string m_message;
    std::string m_extendedMessage;
    std::string m_caption;

private:
    // labels for the buttons, initially empty meaning that the defaults should
    // be used, use GetYes/No/OK/CancelLabel() to access them
    std::string m_yes;
    std::string m_no;
    std::string m_ok;
    std::string m_cancel;
    std::string m_help;

// FIXME: Make protected variables private.
protected:
    unsigned int m_dialogStyle{0};

    // this function is called by our public SetXXXLabels() and should assign
    // the value to var with possibly some transformation (e.g. Cocoa version
    // currently uses this to remove any accelerators from the button strings
    // while GTK+ one handles stock items specifically here)
    virtual void DoSetCustomLabel(std::string& var, const ButtonLabel& label)
    {
        var = label.GetAsString();
    }

    // these functions return the custom label or empty string and should be
    // used only in specific circumstances such as creating the buttons with
    // these labels (in which case it makes sense to only use a custom label if
    // it was really given and fall back on stock label otherwise), use the
    // Get{Yes,No,OK,Cancel}Label() methods above otherwise
    const std::string& GetCustomYesLabel() const { return m_yes; }
    const std::string& GetCustomNoLabel() const { return m_no; }
    const std::string& GetCustomOKLabel() const { return m_ok; }
    const std::string& GetCustomHelpLabel() const { return m_help; }
    const std::string& GetCustomCancelLabel() const { return m_cancel; }

private:
    // these functions may be overridden to provide different defaults for the
    // default button labels (this is used by wxGTK)
    virtual std::string GetDefaultYesLabel() const { return wxGetTranslation("Yes"); }
    virtual std::string GetDefaultNoLabel() const { return wxGetTranslation("No"); }
    virtual std::string GetDefaultOKLabel() const { return wxGetTranslation("OK"); }
    virtual std::string GetDefaultCancelLabel() const { return wxGetTranslation("Cancel"); }
    virtual std::string GetDefaultHelpLabel() const { return wxGetTranslation("Help"); }
};

#include "wx/generic/msgdlgg.h"

#if defined(__WX_COMPILING_MSGDLGG_CPP__) || \
    defined(__WXUNIVERSAL__) || defined(__WXGPE__) || \
    (defined(__WXGTK__) && !defined(__WXGTK20__))

    #define wxMessageDialog wxGenericMessageDialog
#elif defined(__WXMSW__)
    #include "wx/msw/msgdlg.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/msgdlg.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/msgdlg.h"
#elif defined(__WXMAC__)
    #include "wx/osx/msgdlg.h"
#elif defined(__WXQT__)
    #include "wx/qt/msgdlg.h"
#endif

// ----------------------------------------------------------------------------
// wxMessageBox: the simplest way to use wxMessageDialog
// ----------------------------------------------------------------------------

int wxMessageBox(const std::string& message,
                             const std::string& caption = wxMessageBoxCaptionStr,
                             unsigned int style = wxOK | wxCENTRE,
                             wxWindow *parent = nullptr,
                             int x = wxDefaultCoord, int y = wxDefaultCoord);

#endif // wxUSE_MSGDLG

#endif // _WX_MSGDLG_H_BASE_
