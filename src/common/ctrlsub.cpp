///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/ctrlsub.cpp
// Purpose:     wxItemContainer implementation
// Author:      Vadim Zeitlin
// Modified by:
// Created:     22.10.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_CONTROLS

#include "wx/ctrlsub.h"

wxIMPLEMENT_ABSTRACT_CLASS(wxControlWithItems, wxControl);

// ============================================================================
// wxItemContainerImmutable implementation
// ============================================================================

// ----------------------------------------------------------------------------
// selection
// ----------------------------------------------------------------------------

std::string wxItemContainerImmutable::GetStringSelection() const
{
    std::string s;

    const int sel = GetSelection();
    if ( sel != wxNOT_FOUND )
        s = GetString((unsigned int)sel);

    return s;
}

bool wxItemContainerImmutable::SetStringSelection(std::string_view s)
{
    const int sel = FindString(s);
    if ( sel == wxNOT_FOUND )
        return false;

    SetSelection(sel);

    return true;
}

std::vector<std::string> wxItemContainerImmutable::GetStrings() const
{
    std::vector<std::string> result;

    const unsigned int count = GetCount();
    result.reserve(count);

    for ( unsigned int n = 0; n < count; n++ )
        result.push_back(GetString(n));

    return result;
}

// ============================================================================
// wxItemContainer implementation
// ============================================================================

// ----------------------------------------------------------------------------
// deleting items
// ----------------------------------------------------------------------------

void wxItemContainer::Clear()
{
    if ( HasClientObjectData() )
    {
        const unsigned count = GetCount();
        for ( unsigned i = 0; i < count; ++i )
            ResetItemClientObject(i);
    }

    SetClientDataType(wxClientDataType::None);

    DoClear();
}

void wxItemContainer::Delete(unsigned int pos)
{
    wxCHECK_RET( pos < GetCount(), "invalid index" );

    if ( HasClientObjectData() )
        ResetItemClientObject(pos);

    DoDeleteOneItem(pos);

    if ( IsEmpty() )
    {
        SetClientDataType(wxClientDataType::None);
    }
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------

int wxItemContainer::DoInsertOneItem([[maybe_unused]] const std::string& item,
                                 [[maybe_unused]] unsigned int pos)
{
    wxFAIL_MSG( "Must be overridden if DoInsertItemsInLoop() is used" );

    return wxNOT_FOUND;
}


// ----------------------------------------------------------------------------
// client data
// ----------------------------------------------------------------------------

void wxItemContainer::SetClientObject(unsigned int n, wxClientData *data)
{
    wxASSERT_MSG( !HasClientUntypedData(),
                  "can't have both object and void client data" );

    wxCHECK_RET( IsValid(n), "Invalid index passed to SetClientObject()" );

    if ( HasClientObjectData() )
    {
        wxClientData * clientDataOld =
            static_cast<wxClientData *>(DoGetItemClientData(n));
        delete clientDataOld;
    }
    else // didn't have any client data so far
    {
        // now we have object client data
        DoInitItemClientData();

        SetClientDataType(wxClientDataType::Object);
    }

    DoSetItemClientData(n, data);
}

wxClientData *wxItemContainer::GetClientObject(unsigned int n) const
{
    wxCHECK_MSG( HasClientObjectData(), nullptr,
                  "this window doesn't have object client data" );

    wxCHECK_MSG( IsValid(n), nullptr,
                 "Invalid index passed to GetClientObject()" );

    return static_cast<wxClientData *>(DoGetItemClientData(n));
}

wxClientData *wxItemContainer::DetachClientObject(unsigned int n)
{
    wxClientData * const data = GetClientObject(n);
    if ( data )
    {
        // reset the pointer as we don't own it any more
        DoSetItemClientData(n, nullptr);
    }

    return data;
}

void wxItemContainer::SetClientData(unsigned int n, void *data)
{
    if ( !HasClientData() )
    {
        DoInitItemClientData();
        SetClientDataType(wxClientDataType::Void);
    }

    wxASSERT_MSG( HasClientUntypedData(),
                  "can't have both object and void client data" );

    wxCHECK_RET( IsValid(n), "Invalid index passed to SetClientData()" );

    DoSetItemClientData(n, data);
}

void *wxItemContainer::GetClientData(unsigned int n) const
{
    wxCHECK_MSG( HasClientUntypedData(), nullptr,
                  "this window doesn't have void client data" );

    wxCHECK_MSG( IsValid(n), nullptr,
                 "Invalid index passed to GetClientData()" );

    return DoGetItemClientData(n);
}

void wxItemContainer::AssignNewItemClientData(unsigned int pos,
                                              void **clientData,
                                              unsigned int n,
                                              wxClientDataType type)
{
    switch ( type )
    {
        case wxClientDataType::Object:
            SetClientObject
            (
                pos,
                (reinterpret_cast<wxClientData **>(clientData))[n]
            );
            break;

        case wxClientDataType::Void:
            SetClientData(pos, clientData[n]);
            break;

        default:
            wxFAIL_MSG( "unknown client data type" );
            [[fallthrough]];

        case wxClientDataType::None:
            // nothing to do
            break;
    }
}

void wxItemContainer::ResetItemClientObject(unsigned int n)
{
    wxClientData * const data = GetClientObject(n);
    if ( data )
    {
        delete data;
        DoSetItemClientData(n, nullptr);
    }
}

// ============================================================================
// wxControlWithItems implementation
// ============================================================================

void
wxControlWithItemsBase::InitCommandEventWithItems(wxCommandEvent& event, int n)
{
    InitCommandEvent(event);

    if ( n != wxNOT_FOUND )
    {
        if ( HasClientObjectData() )
            event.SetClientObject(GetClientObject(n));
        else if ( HasClientUntypedData() )
            event.SetClientData(GetClientData(n));
    }
}

void wxControlWithItemsBase::SendSelectionChangedEvent(wxEventType eventType)
{
    const int n = GetSelection();
    if ( n == wxNOT_FOUND )
        return;

    wxCommandEvent event(eventType, m_windowId);
    event.SetInt(n);
    event.SetString(GetStringSelection());
    InitCommandEventWithItems(event, n);

    HandleWindowEvent(event);
}

#endif // wxUSE_CONTROLS
