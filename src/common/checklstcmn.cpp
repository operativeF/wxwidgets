///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/checklstcmn.cpp
// Purpose:     wxCheckListBox common code
// Author:      Vadim Zeitlin
// Modified by:
// Created:     16.11.97
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////




#if wxUSE_CHECKLISTBOX

#include "wx/checklst.h"
#include "wx/object.h"
#include "wx/window.h"
#include "wx/listbox.h"
#include "wx/dcmemory.h"
#include "wx/settings.h"
#include "wx/log.h"

std::size_t wxCheckListBoxBase::GetCheckedItemsCount(std::vector<int>& checkedItems)
{
    // TODO: Should this really clear the vector every time?
    // Also, why bother pushing back the items if we're going to clear it every time?
    checkedItems.clear();

    for ( unsigned int i = 0; i < GetCount(); ++i )
    {
        if ( IsChecked(i) )
            checkedItems.push_back(i);
    }

    return checkedItems.size();
}

#endif
