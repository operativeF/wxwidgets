/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/choicdgg.h
// Purpose:     Generic choice dialogs
// Author:      Julian Smart
// Modified by: 03.11.00: VZ to add wxArrayString and multiple sel functions
// Created:     01/02/97
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_CHOICDGG_H_
#define _WX_GENERIC_CHOICDGG_H_

#include "wx/dialog.h"

import <string>;
import <vector>;

class wxListBoxBase;

// ----------------------------------------------------------------------------
// some (ugly...) constants
// ----------------------------------------------------------------------------

inline constexpr int wxCHOICE_HEIGHT = 150;
inline constexpr int wxCHOICE_WIDTH  = 200;

#define wxCHOICEDLG_STYLE \
    (wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxOK | wxCANCEL | wxCENTRE)

// ----------------------------------------------------------------------------
// wxAnyChoiceDialog: a base class for dialogs containing a listbox
// ----------------------------------------------------------------------------

class wxAnyChoiceDialog : public wxDialog
{
public:
    wxAnyChoiceDialog()  = default;

    wxAnyChoiceDialog(wxWindow *parent,
                      const std::string& message,
                      const std::string& caption,
                      const std::vector<std::string>& choices,
                      long styleDlg = wxCHOICEDLG_STYLE,
                      const wxPoint& pos = wxDefaultPosition,
                      long styleLbox = wxLB_ALWAYS_SB)
    {
        Create(parent, message, caption, choices,
                     styleDlg, pos, styleLbox);
    }

	wxAnyChoiceDialog& operator=(wxAnyChoiceDialog&&) = delete;

    bool Create(wxWindow *parent,
                const std::string& message,
                const std::string& caption,
                const std::vector<std::string>& choices,
                long styleDlg = wxCHOICEDLG_STYLE,
                const wxPoint& pos = wxDefaultPosition,
                long styleLbox = wxLB_ALWAYS_SB);

protected:
    wxListBoxBase *m_listbox{nullptr};

    virtual wxListBoxBase *CreateList(const std::vector<std::string>& choices, long styleLbox);
};

// ----------------------------------------------------------------------------
// wxSingleChoiceDialog: a dialog with single selection listbox
// ----------------------------------------------------------------------------

class wxSingleChoiceDialog : public wxAnyChoiceDialog
{
public:
    wxSingleChoiceDialog() = default;

    wxSingleChoiceDialog(wxWindow *parent,
                         const std::string& message,
                         const std::string& caption,
                         const std::vector<std::string>& choices,
                         void **clientData = nullptr,
                         unsigned int style = wxCHOICEDLG_STYLE,
                         const wxPoint& pos = wxDefaultPosition)
    {
        Create(parent, message, caption, choices, clientData, style, pos);
    }

    wxSingleChoiceDialog(const wxSingleChoiceDialog&) = delete;
	wxSingleChoiceDialog& operator=(const wxSingleChoiceDialog&) = delete;

    bool Create(wxWindow *parent,
                const std::string& message,
                const std::string& caption,
                const std::vector<std::string>& choices,
                void **clientData = nullptr,
                unsigned int style = wxCHOICEDLG_STYLE,
                const wxPoint& pos = wxDefaultPosition);

    void SetSelection(int sel);
    int GetSelection() const { return m_selection; }
    std::string GetStringSelection() const { return m_stringSelection; }
    void* GetSelectionData() const { return m_clientData; }

    // implementation from now on
    void OnOK(wxCommandEvent& event);
    void OnListBoxDClick(wxCommandEvent& event);

protected:
    int            m_selection{-1};
    std::string    m_stringSelection;

    void DoChoice();

public:
    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// wxMultiChoiceDialog: a dialog with multi selection listbox
// ----------------------------------------------------------------------------

class wxMultiChoiceDialog : public wxAnyChoiceDialog
{
public:
    wxMultiChoiceDialog() = default;

    wxMultiChoiceDialog(wxWindow *parent,
                        const std::string& message,
                        const std::string& caption,
                        const std::vector<std::string>& choices,
                        unsigned int style = wxCHOICEDLG_STYLE,
                        const wxPoint& pos = wxDefaultPosition)
    {
        Create(parent, message, caption, choices, style, pos);
    }

    wxMultiChoiceDialog& operator=(wxMultiChoiceDialog&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                const std::string& message,
                const std::string& caption,
                const std::vector<std::string>& choices,
                unsigned int style = wxCHOICEDLG_STYLE,
                const wxPoint& pos = wxDefaultPosition);

    void SetSelections(const std::vector<int>& selections);
    std::vector<int> GetSelections() const { return m_selections; }

    // implementation from now on
    bool TransferDataFromWindow() override;

protected:
#if wxUSE_CHECKLISTBOX
    wxListBoxBase *CreateList(const std::vector<std::string>& choices, long styleLbox) override;
#endif // wxUSE_CHECKLISTBOX

    std::vector<int> m_selections;
};

// ----------------------------------------------------------------------------
// wrapper functions which can be used to get selection(s) from the user
// ----------------------------------------------------------------------------

// get the user selection as a string
std::string wxGetSingleChoice(const std::string& message,
                                       const std::string& caption,
                                       const std::vector<std::string>& choices,
                                       wxWindow *parent = nullptr,
                                       int x = wxDefaultCoord,
                                       int y = wxDefaultCoord,
                                       bool centre = true,
                                       int width = wxCHOICE_WIDTH,
                                       int height = wxCHOICE_HEIGHT,
                                       int initialSelection = 0);

std::string wxGetSingleChoice(const std::string& message,
                                       const std::string& caption,
                                       int n, const std::string *choices,
                                       wxWindow *parent = nullptr,
                                       int x = wxDefaultCoord,
                                       int y = wxDefaultCoord,
                                       bool centre = true,
                                       int width = wxCHOICE_WIDTH,
                                       int height = wxCHOICE_HEIGHT,
                                       int initialSelection = 0);

std::string wxGetSingleChoice(const std::string& message,
                                            const std::string& caption,
                                            int n, const std::string *choices,
                                            int initialSelection,
                                            wxWindow *parent = nullptr);

// Same as above but gets position in list of strings, instead of string,
// or -1 if no selection
int wxGetSingleChoiceIndex(const std::string& message,
                                       const std::string& caption,
                                       const std::vector<std::string>& choices,
                                       wxWindow *parent = nullptr,
                                       int x = wxDefaultCoord,
                                       int y = wxDefaultCoord,
                                       bool centre = true,
                                       int width = wxCHOICE_WIDTH,
                                       int height = wxCHOICE_HEIGHT,
                                       int initialSelection = 0);

// Return client data instead or NULL if canceled
void* wxGetSingleChoiceData(const std::string& message,
                                        const std::string& caption,
                                        const std::vector<std::string>& choices,
                                        void **client_data,
                                        wxWindow *parent = nullptr,
                                        int x = wxDefaultCoord,
                                        int y = wxDefaultCoord,
                                        bool centre = true,
                                        int width = wxCHOICE_WIDTH,
                                        int height = wxCHOICE_HEIGHT,
                                        int initialSelection = 0);

// fill the array with the indices of the chosen items, it will be empty
// if no items were selected or Cancel was pressed - return the number of
// selections or -1 if cancelled
int wxGetSelectedChoices(std::vector<int>& selections,
                                        const std::string& message,
                                        const std::string& caption,
                                        const std::vector<std::string>& choices,
                                        wxWindow *parent = nullptr,
                                        int x = wxDefaultCoord,
                                        int y = wxDefaultCoord,
                                        bool centre = true,
                                        int width = wxCHOICE_WIDTH,
                                        int height = wxCHOICE_HEIGHT);

#endif // _WX_GENERIC_CHOICDGG_H_
