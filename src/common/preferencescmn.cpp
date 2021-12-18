///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/preferencescmn.cpp
// Purpose:     wxPreferencesEditor implementation common to all platforms.
// Author:      Vaclav Slavik
// Created:     2013-02-19
// Copyright:   (c) 2013 Vaclav Slavik <vslavik@fastmail.fm>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////




#if wxUSE_PREFERENCES_EDITOR

#include "wx/private/preferences.h"
#include "wx/intl.h"

// ============================================================================
// implementation
// ============================================================================

std::string wxStockPreferencesPage::GetName() const
{
    switch ( m_kind )
    {
        case Kind::General:
            return _("General");
        case Kind::Advanced:
            return _("Advanced");
    }

    return {};
}

wxPreferencesEditor::wxPreferencesEditor(const std::string& title)
    : m_impl(wxPreferencesEditorImpl::Create(title))
{
}

wxPreferencesEditor::~wxPreferencesEditor()
{
    delete m_impl;
}

void wxPreferencesEditor::AddPage(wxPreferencesPage* page)
{
    wxCHECK_RET( page, "can't set NULL page" );
    m_impl->AddPage(page);
}

void wxPreferencesEditor::Show(wxWindow* parent)
{
    m_impl->Show(parent);
}

void wxPreferencesEditor::Dismiss()
{
    m_impl->Dismiss();
}

#endif // wxUSE_PREFERENCES_EDITOR
