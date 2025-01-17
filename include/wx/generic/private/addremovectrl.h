///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/private/addremovectrl.h
// Purpose:     Generic wxAddRemoveImpl implementation, also used in wxMSW
// Author:      Vadim Zeitlin
// Created:     2015-02-05
// Copyright:   (c) 2015 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_PRIVATE_ADDREMOVECTRL_H_
#define _WX_GENERIC_PRIVATE_ADDREMOVECTRL_H_

// ----------------------------------------------------------------------------
// wxAddRemoveImpl
// ----------------------------------------------------------------------------

class wxAddRemoveImpl : public wxAddRemoveImplWithButtons
{
public:
    wxAddRemoveImpl(wxAddRemoveAdaptor* adaptor,
                    wxAddRemoveCtrl* parent,
                    wxWindow* ctrlItems)
        : wxAddRemoveImplWithButtons(adaptor, parent, ctrlItems)
    {
        m_btnAdd = new wxButton(parent, wxID_ADD, AddButtonLabel,
                                wxDefaultPosition, wxDefaultSize,
                                wxBU_EXACTFIT | wxBORDER_NONE);
        m_btnRemove = new wxButton(parent, wxID_REMOVE, RemoveButtonLabel,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxBU_EXACTFIT | wxBORDER_NONE);

        wxSizer* const sizerBtns = new wxBoxSizer(wxVERTICAL);
        sizerBtns->Add(m_btnAdd, wxSizerFlags().Expand());
        sizerBtns->Add(m_btnRemove, wxSizerFlags().Expand());

        wxSizer* const sizerTop = new wxBoxSizer(wxHORIZONTAL);
        sizerTop->Add(ctrlItems, wxSizerFlags(1).Expand());
        sizerTop->Add(sizerBtns, wxSizerFlags().Centre().Border(wxDirection::wxLEFT));
        parent->SetSizer(sizerTop);

        SetUpEvents();
    }

private:
    static constexpr char AddButtonLabel[] = "\uFF0B"; // FULLWIDTH PLUS SIGN
    static constexpr char RemoveButtonLabel[] = "\u2012"; // FIGURE DASH
};

#endif // _WX_GENERIC_PRIVATE_ADDREMOVECTRL_H_
