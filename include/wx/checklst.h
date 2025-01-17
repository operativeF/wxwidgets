///////////////////////////////////////////////////////////////////////////////
// Name:        wx/checklst.h
// Purpose:     wxCheckListBox class interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     12.09.00
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CHECKLST_H_BASE_
#define _WX_CHECKLST_H_BASE_

#if wxUSE_CHECKLISTBOX

#include "wx/listbox.h"

import <cstdint>;
import <vector>;

// ----------------------------------------------------------------------------
// wxCheckListBox: a listbox whose items may be checked
// ----------------------------------------------------------------------------

class wxCheckListBoxBase : public wxListBox
{
public:
    wxCheckListBoxBase& operator=(wxCheckListBoxBase&&) = delete;
    
    // check list box specific methods
    virtual bool IsChecked(unsigned int item) const = 0;
    virtual void Check(unsigned int item, bool check = true) = 0;

    virtual std::size_t GetCheckedItemsCount(std::vector<int>& checkedItems);
};

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/checklst.h"
#elif defined(__WXMSW__)
    #include "wx/msw/checklst.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/checklst.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/checklst.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/checklst.h"
#elif defined(__WXMAC__)
    #include "wx/osx/checklst.h"
#elif defined(__WXQT__)
    #include "wx/qt/checklst.h"
#endif

#endif // wxUSE_CHECKLISTBOX

#endif
    // _WX_CHECKLST_H_BASE_
