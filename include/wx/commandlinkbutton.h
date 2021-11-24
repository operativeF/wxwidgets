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

#if wxUSE_COMMANDLINKBUTTON

#include "wx/button.h"

import Utils.Strings;

import <string>;

// ----------------------------------------------------------------------------
// Command link button common base class
// ----------------------------------------------------------------------------

// This class has separate "main label" (title-like string) and (possibly
// multiline) "note" which can be set and queried separately but can also be
// set both at once by joining them with a new line and setting them as a
// label and queried by breaking the label into the parts before the first new
// line and after it.

class wxCommandLinkButtonBase : public wxButton
{
public:
    wxCommandLinkButtonBase()  = default;

    wxCommandLinkButtonBase(wxWindow *parent,
                            wxWindowID id,
                            const std::string& mainLabel = {},
                            const std::string& note = {},
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxDefaultSize,
                            unsigned int style = 0,
                            const wxValidator& validator =
                                wxDefaultValidator,
                            std::string_view name = wxButtonNameStr)
        : wxButton(parent,
                   id,
                   mainLabel + '\n' + note,
                   pos,
                   size,
                   style,
                   validator,
                   name)
    {}

	wxCommandLinkButtonBase& operator=(wxCommandLinkButtonBase&&) = delete;

    virtual void SetMainLabelAndNote(const std::string& mainLabel,
                                     const std::string& note) = 0;

    virtual void SetMainLabel(const std::string& mainLabel)
    {
        SetMainLabelAndNote(mainLabel, GetNote());
    }

    virtual void SetNote(const wxString& note)
    {
        SetMainLabelAndNote(GetMainLabel(), note);
    }

    virtual wxString GetMainLabel() const
    {
        return wx::utils::BeforeFirst(GetLabel(), '\n');
    }

    virtual wxString GetNote() const
    {
        return wx::utils::AfterFirst(GetLabel(), '\n');
    }

protected:
    virtual bool HasNativeBitmap() const { return false; }
};

// ----------------------------------------------------------------------------
// Generic command link button
// ----------------------------------------------------------------------------

// Trivial generic implementation simply using a multiline label to show both
// the main label and the note.

class wxGenericCommandLinkButton
                      : public wxCommandLinkButtonBase
{
public:
    wxGenericCommandLinkButton()  = default;

    wxGenericCommandLinkButton(wxWindow *parent,
                               wxWindowID id,
                               const std::string& mainLabel = {},
                               const std::string& note = {},
                               const wxPoint& pos = wxDefaultPosition,
                               const wxSize& size = wxDefaultSize,
                               unsigned int style = 0,
                               const wxValidator& validator = wxDefaultValidator,
                               std::string_view name = wxButtonNameStr)
         
    {
        Create(parent, id, mainLabel, note, pos, size, style, validator, name);
    }

	wxGenericCommandLinkButton& operator=(wxGenericCommandLinkButton&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& mainLabel = {},
                const std::string& note = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                std::string_view name = wxButtonNameStr);

    void SetMainLabelAndNote(const std::string& mainLabel,
                             const std::string& note) override
    {
        wxButton::SetLabel(mainLabel + '\n' + note);
    }

private:
    void SetDefaultBitmap();
};

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    #include "wx/msw/commandlinkbutton.h"
#else
    class wxCommandLinkButton : public wxGenericCommandLinkButton
    {
    public:
        wxCommandLinkButton() : wxGenericCommandLinkButton() { }

        wxCommandLinkButton(wxWindow *parent,
                            wxWindowID id,
                            const std::string& mainLabel = {},
                            const std::string& note = {},
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxDefaultSize,
                            unsigned int style = 0,
                            const wxValidator& validator = wxDefaultValidator,
                            std::string_view name = wxButtonNameStr)
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

        wxCommandLinkButton& operator=(wxCommandLinkButton&&) = delete;

        wxClassInfo *wxGetClassInfo() const;
        static wxClassInfo ms_classInfo;
        static wxObject* wxCreateObject();
    };
#endif // __WXMSW__/!__WXMSW__

#endif // wxUSE_COMMANDLINKBUTTON

#endif // _WX_COMMANDLINKBUTTON_H_
