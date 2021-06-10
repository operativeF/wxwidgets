/////////////////////////////////////////////////////////////////////////////
// Name:        wx/commandlinkbutton.h
// Purpose:     wxCommandLinkButtonBase and wxGenericCommandLinkButton classes
// Author:      Rickard Westerlund
// Created:     2010-06-11
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COMMANDLINKBUTTON_H_
#define _WX_COMMANDLINKBUTTON_H_

#include "wx/defs.h"

#if wxUSE_COMMANDLINKBUTTON

#include "wx/button.h"

// ----------------------------------------------------------------------------
// Command link button common base class
// ----------------------------------------------------------------------------

// This class has separate "main label" (title-like string) and (possibly
// multiline) "note" which can be set and queried separately but can also be
// set both at once by joining them with a new line and setting them as a
// label and queried by breaking the label into the parts before the first new
// line and after it.

class WXDLLIMPEXP_CORE wxCommandLinkButtonBase : public wxButton
{
public:
    wxCommandLinkButtonBase()  = default;

    wxCommandLinkButtonBase(wxWindow *parent,
                            wxWindowID id,
                            const wxString& mainLabel = wxEmptyString,
                            const wxString& note = wxEmptyString,
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxDefaultSize,
                            long style = 0,
                            const wxValidator& validator =
                                wxDefaultValidator,
                            const wxString& name = wxASCII_STR(wxButtonNameStr))
        : wxButton(parent,
                   id,
                   mainLabel + '\n' + note,
                   pos,
                   size,
                   style,
                   validator,
                   name)
        { }

    wxCommandLinkButtonBase(const wxCommandLinkButtonBase&) = delete;
	wxCommandLinkButtonBase& operator=(const wxCommandLinkButtonBase&) = delete;

    virtual void SetMainLabelAndNote(const wxString& mainLabel,
                                     const wxString& note) = 0;

    virtual void SetMainLabel(const wxString& mainLabel)
    {
        SetMainLabelAndNote(mainLabel, GetNote());
    }

    virtual void SetNote(const wxString& note)
    {
        SetMainLabelAndNote(GetMainLabel(), note);
    }

    virtual wxString GetMainLabel() const
    {
        return GetLabel().BeforeFirst('\n');
    }

    virtual wxString GetNote() const
    {
        return GetLabel().AfterFirst('\n');
    }

protected:
    virtual bool HasNativeBitmap() const { return false; }
};

// ----------------------------------------------------------------------------
// Generic command link button
// ----------------------------------------------------------------------------

// Trivial generic implementation simply using a multiline label to show both
// the main label and the note.

class WXDLLIMPEXP_CORE wxGenericCommandLinkButton
                      : public wxCommandLinkButtonBase
{
public:
    wxGenericCommandLinkButton()  = default;


    wxGenericCommandLinkButton(wxWindow *parent,
                               wxWindowID id,
                               const wxString& mainLabel = wxEmptyString,
                               const wxString& note = wxEmptyString,
                               const wxPoint& pos = wxDefaultPosition,
                               const wxSize& size = wxDefaultSize,
                               long style = 0,
                               const wxValidator& validator = wxDefaultValidator,
                               const wxString& name = wxASCII_STR(wxButtonNameStr))
         
    {
        Create(parent, id, mainLabel, note, pos, size, style, validator, name);
    }

    wxGenericCommandLinkButton(const wxGenericCommandLinkButton&) = delete;
	wxGenericCommandLinkButton& operator=(const wxGenericCommandLinkButton&) = delete;

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& mainLabel = wxEmptyString,
                const wxString& note = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxButtonNameStr));

    void SetMainLabelAndNote(const wxString& mainLabel,
                                     const wxString& note) override
    {
        wxButton::SetLabel(mainLabel + '\n' + note);
    }

private:
    void SetDefaultBitmap();
};

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    #include "wx/msw/commandlinkbutton.h"
#else
    class WXDLLIMPEXP_CORE wxCommandLinkButton : public wxGenericCommandLinkButton
    {
    public:
        wxCommandLinkButton() : wxGenericCommandLinkButton() { }

        wxCommandLinkButton(wxWindow *parent,
                            wxWindowID id,
                            const wxString& mainLabel = wxEmptyString,
                            const wxString& note = wxEmptyString,
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxDefaultSize,
                            long style = 0,
                            const wxValidator& validator = wxDefaultValidator,
                            const wxString& name = wxASCII_STR(wxButtonNameStr))
            : wxGenericCommandLinkButton(parent,
                                         id,
                                         mainLabel,
                                         note,
                                         pos,
                                         size,
                                         style,
                                         validator,
                                         name)
            { }

        wxCommandLinkButton(const wxCommandLinkButton&) = delete;
        wxCommandLinkButton& operator=(const wxCommandLinkButton&) = delete;

        wxClassInfo *GetClassInfo() const;
        static wxClassInfo ms_classInfo;
        static wxObject* wxCreateObject();
    };
#endif // __WXMSW__/!__WXMSW__

#endif // wxUSE_COMMANDLINKBUTTON

#endif // _WX_COMMANDLINKBUTTON_H_
