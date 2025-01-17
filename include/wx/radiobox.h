///////////////////////////////////////////////////////////////////////////////
// Name:        wx/radiobox.h
// Purpose:     wxRadioBox declaration
// Author:      Vadim Zeitlin
// Modified by:
// Created:     10.09.00
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RADIOBOX_H_BASE_
#define _WX_RADIOBOX_H_BASE_

#if wxUSE_RADIOBOX

#include "wx/ctrlsub.h"

import WX.Cfg.Flags;

import <string>;
import <string_view>;
import <vector>;

#if wxUSE_TOOLTIPS

class wxToolTip;

using wxToolTipArray = std::vector<wxToolTip*>;

#endif // wxUSE_TOOLTIPS

inline constexpr std::string_view wxRadioBoxNameStr = "radioBox";

// ----------------------------------------------------------------------------
// wxRadioBoxBase is not a normal base class, but rather a mix-in because the
// real wxRadioBox derives from different classes on different platforms: for
// example, it is a wxStaticBox in wxUniv and wxMSW but not in other ports
// ----------------------------------------------------------------------------

class wxRadioBoxBase : public wxItemContainerImmutable
{
public:
    ~wxRadioBoxBase();

    // change/query the individual radio button state
    virtual bool Enable(unsigned int n, bool enable = true) = 0;
    virtual bool Show(unsigned int n, bool show = true) = 0;
    virtual bool IsItemEnabled(unsigned int n) const = 0;
    virtual bool IsItemShown(unsigned int n) const = 0;

    // return number of columns/rows in this radiobox
    unsigned int GetColumnCount() const { return m_numCols; }
    unsigned int GetRowCount() const { return m_numRows; }

    // return the next active (i.e. shown and not disabled) item above/below/to
    // the left/right of the given one
    int GetNextItem(int item, wxDirection dir, unsigned int style) const;

#if wxUSE_TOOLTIPS
    // set the tooltip text for a radio item, empty string unsets any tooltip
    void SetItemToolTip(unsigned int item, const std::string& text);

    // get the individual items tooltip; returns NULL if none
    wxToolTip *GetItemToolTip(unsigned int item) const
        { return m_itemsTooltips ? (*m_itemsTooltips)[item] : NULL; }
#endif // wxUSE_TOOLTIPS

#if wxUSE_HELP
    // set helptext for a particular item, pass an empty string to erase it
    void SetItemHelpText(unsigned int n, const std::string& helpText);

    // retrieve helptext for a particular item, empty string means no help text
    std::string GetItemHelpText(unsigned int n) const;
#else // wxUSE_HELP
    // just silently ignore the help text, it's better than requiring using
    // conditional compilation in all code using this function
    void SetItemHelpText([[maybe_unused]] unsigned int n,
                         [[maybe_unused]] const std::string& helpText)
    {
    }
#endif // wxUSE_HELP

    // returns the radio item at the given position or wxNOT_FOUND if none
    // (currently implemented only under MSW and GTK)
    virtual int GetItemFromPoint([[maybe_unused]] const wxPoint& pt) const
    {
        return wxNOT_FOUND;
    }


protected:
    wxRadioBoxBase() = default;
    
    virtual wxBorder GetDefaultBorder() const { return wxBORDER_NONE; }

    // return the number of items in major direction (which depends on whether
    // we have wxRA_SPECIFY_COLS or wxRA_SPECIFY_ROWS style)
    unsigned int GetMajorDim() const { return m_majorDim; }

    // sets m_majorDim and also updates m_numCols/Rows
    //
    // the style parameter should be the style of the radiobox itself
    void SetMajorDim(unsigned int majorDim, unsigned int style);

#if wxUSE_TOOLTIPS
    // called from SetItemToolTip() to really set the tooltip for the specified
    // item in the box (or, if tooltip is NULL, to remove any existing one).
    //
    // NB: this function should really be pure virtual but to avoid breaking
    //     the build of the ports for which it's not implemented yet we provide
    //     an empty stub in the base class for now
    virtual void DoSetItemToolTip(unsigned int item, wxToolTip *tooltip);

    // returns true if we have any item tooltips
    bool HasItemToolTips() const { return m_itemsTooltips != nullptr; }
#endif // wxUSE_TOOLTIPS

#if wxUSE_HELP
    // Retrieve help text for an item: this is a helper for the implementation
    // of wxWindow::GetHelpTextAtPoint() in the real radiobox class
    std::string DoGetHelpTextAtPoint(const wxWindow *derived,
                                  const wxPoint& pt,
                                  wxHelpEvent::Origin origin) const;
#endif // wxUSE_HELP

private:
#if wxUSE_TOOLTIPS
    // array of tooltips for the individual items

    wxToolTipArray *m_itemsTooltips{nullptr};
#endif

#if wxUSE_HELP
    // help text associated with a particular item or empty string if none
    std::vector<std::string> m_itemsHelpTexts;
#endif // wxUSE_HELP

    // the number of elements in major dimension (i.e. number of columns if
    // wxRA_SPECIFY_COLS or the number of rows if wxRA_SPECIFY_ROWS) and also
    // the number of rows/columns calculated from it
    unsigned int m_majorDim{0};
    unsigned int m_numCols{0};
    unsigned int m_numRows{0};
};

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/radiobox.h"
#elif defined(__WXMSW__)
    #include "wx/msw/radiobox.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/radiobox.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/radiobox.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/radiobox.h"
#elif defined(__WXMAC__)
    #include "wx/osx/radiobox.h"
#elif defined(__WXQT__)
    #include "wx/qt/radiobox.h"
#endif

#endif // wxUSE_RADIOBOX

#endif // _WX_RADIOBOX_H_BASE_
