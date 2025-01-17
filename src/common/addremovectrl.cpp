///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/addremovectrl.cpp
// Purpose:     wxAddRemoveCtrl implementation.
// Author:      Vadim Zeitlin
// Created:     2015-01-29
// Copyright:   (c) 2015 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_ADDREMOVECTRL

#include "wx/addremovectrl.h"

#include "wx/private/addremovectrl.h"

import Utils.Geometry;

bool
wxAddRemoveCtrl::Create(wxWindow* parent,
                        wxWindowID winid,
                        const wxPoint& pos,
                        const wxSize& size,
                        unsigned int style,
                        std::string_view name)
{
    if ( !wxPanel::Create(parent, winid, pos, size, style, name) )
        return false;

    // We don't do anything here, the buttons are created when we're given the
    // adaptor to use them with SetAdaptor().
    return true;
}

wxAddRemoveCtrl::~wxAddRemoveCtrl()
{
    delete m_impl;
}

void wxAddRemoveCtrl::SetAdaptor(wxAddRemoveAdaptor* adaptor)
{
    wxCHECK_RET( !m_impl, "should be only called once" );

    wxCHECK_RET( adaptor, "should have a valid adaptor" );

    wxWindow* const ctrlItems = adaptor->GetItemsCtrl();
    wxCHECK_RET( ctrlItems, "should have a valid items control" );

    m_impl = new wxAddRemoveImpl(adaptor, this, ctrlItems);
}

void
wxAddRemoveCtrl::SetButtonsToolTips(const std::string& addtip,
                                    const std::string& removetip)
{
    wxCHECK_RET( m_impl, "can only be called after SetAdaptor()" );

    m_impl->SetButtonsToolTips(addtip, removetip);
}

wxSize wxAddRemoveCtrl::DoGetBestClientSize() const
{
    return m_impl ? m_impl->GetBestClientSize() : wxDefaultSize;
}

#endif // wxUSE_ADDREMOVECTRL
