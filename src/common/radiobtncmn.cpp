/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/radiobtncmn.cpp
// Purpose:     wxRadioButton common code
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_RADIOBTN

#include "wx/radiobut.h"
#include "wx/settings.h"
#include "wx/dcscreen.h"

// ----------------------------------------------------------------------------
// wxRadioButton group navigation
// ----------------------------------------------------------------------------

wxRadioButton* wxRadioButtonBase::GetFirstInGroup() const
{
    auto* btn = dynamic_cast<wxRadioButton*>(const_cast<wxRadioButtonBase*>(this));

    while (true)
    {
        wxRadioButton* prevBtn = btn->GetPreviousInGroup();
        if (!prevBtn)
            return btn;

        btn = prevBtn;
    }
}

wxRadioButton* wxRadioButtonBase::GetLastInGroup() const
{
    auto* btn = dynamic_cast<wxRadioButton*>(const_cast<wxRadioButtonBase*>(this));

    while (true)
    {
        wxRadioButton* nextBtn = btn->GetNextInGroup();
        if (!nextBtn)
            return btn;

        btn = nextBtn;
    }
}

wxRadioButton* wxRadioButtonBase::GetPreviousInGroup() const
{
    if ( HasFlag(wxRB_GROUP) || HasFlag(wxRB_SINGLE) )
        return nullptr;

    const wxWindowList& siblings = GetParent()->GetChildren();
    wxWindowList::compatibility_iterator nodeThis = siblings.Find(this);
    wxCHECK_MSG( nodeThis, nullptr, "radio button not a child of its parent?" );

    // Iterate over all previous siblings until we find the next radio button
    wxWindowList::compatibility_iterator nodeBefore = nodeThis->GetPrevious();
    wxRadioButton *prevBtn = nullptr;
    while (nodeBefore)
    {
        prevBtn = dynamic_cast<wxRadioButton*>(nodeBefore->GetData());
        if (prevBtn)
            break;

        nodeBefore = nodeBefore->GetPrevious();
    }

    if (!prevBtn || prevBtn->HasFlag(wxRB_SINGLE))
    {
        // no more buttons in group
        return nullptr;
    }

    return prevBtn;
}

wxRadioButton* wxRadioButtonBase::GetNextInGroup() const
{
    if ( HasFlag(wxRB_SINGLE) )
        return nullptr;

    const wxWindowList& siblings = GetParent()->GetChildren();
    wxWindowList::compatibility_iterator nodeThis = siblings.Find(this);
    wxCHECK_MSG( nodeThis, nullptr, "radio button not a child of its parent?" );

    // Iterate over all previous siblings until we find the next radio button
    wxWindowList::compatibility_iterator nodeNext = nodeThis->GetNext();
    wxRadioButton *nextBtn = nullptr;
    while (nodeNext)
    {
        nextBtn = dynamic_cast<wxRadioButton*>(nodeNext->GetData());
        if (nextBtn)
            break;

        nodeNext = nodeNext->GetNext();
    }

    if ( !nextBtn || nextBtn->HasFlag(wxRB_GROUP) || nextBtn->HasFlag(wxRB_SINGLE) )
    {
        // no more buttons or the first button of the next group
        return nullptr;
    }

    return nextBtn;
}

#endif // wxUSE_RADIOBTN
