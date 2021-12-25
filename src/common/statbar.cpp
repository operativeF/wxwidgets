///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/statbar.cpp
// Purpose:     wxStatusBarBase implementation
// Author:      Vadim Zeitlin
// Modified by: Francesco Montorsi
// Created:     14.10.01
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////




#if wxUSE_STATUSBAR

#include "wx/statusbr.h"
#include "wx/frame.h"

// ============================================================================
// wxStatusBarPane implementation
// ============================================================================

bool wxStatusBarPane::SetText(std::string_view text)
{
    if ( text == m_text )
        return false;

    /*
        If we have a message to restore on the stack, we update it to
        correspond to the current one so that a sequence of calls such as

        1. SetStatusText("foo")
        2. PushStatusText("bar")
        3. SetStatusText("new foo")
        4. PopStatusText()

        doesn't overwrite the "new foo" which should be shown at the end with
        the old value "foo". This would be unexpected and hard to avoid,
        especially when PushStatusText() is used internally by wxWidgets
        without knowledge of the user program, as it is for showing the menu
        and toolbar help strings.

        By updating the top of the stack we ensure that the next call to
        PopStatusText() basically becomes a NOP without breaking the balance
        between the calls to push and pop as we would have done if we really
        called PopStatusText() here.
     */
    if ( !m_arrStack.empty() )
    {
        m_arrStack.back() = {text.begin(), text.end()};
    }

    m_text = {text.begin(), text.end()};

    return true;
}

bool wxStatusBarPane::PushText(const std::string& text)
{
    // save the currently shown text
    m_arrStack.push_back(m_text);

    // and update the new one if necessary
    if ( text == m_text )
        return false;

    m_text = text;

    return true;
}

bool wxStatusBarPane::PopText()
{
    wxCHECK_MSG( !m_arrStack.empty(), false, "no status message to pop" );

    const std::string text = m_arrStack.back();

    m_arrStack.pop_back();

    if ( text == m_text )
        return false;

    m_text = text;

    return true;
}

// ----------------------------------------------------------------------------
// ctor/dtor
// ----------------------------------------------------------------------------

wxStatusBarBase::~wxStatusBarBase()
{
    // notify the frame that it doesn't have a status bar any longer to avoid
    // dangling pointers
    wxFrame *frame = dynamic_cast<wxFrame*>(GetParent());
    if ( frame && frame->GetStatusBar() == this )
        frame->SetStatusBar(nullptr);
}

// ----------------------------------------------------------------------------
// field widths
// ----------------------------------------------------------------------------

void wxStatusBarBase::SetFieldsCount(int number, const int *widths)
{
    wxCHECK_RET( number > 0, "invalid field number in SetFieldsCount" );

    if ( (size_t)number > m_panes.size() )
    {
        wxStatusBarPane newPane;

        // add more entries with the default style and zero width
        // (this will be set later)
        for (size_t i = m_panes.size(); i < (size_t)number; ++i)
            m_panes.push_back(newPane);
    }
    else if ( (size_t)number < m_panes.size() )
    {
        // remove entries in excess
        m_panes.erase(m_panes.begin() + number, m_panes.end());
    }

    // SetStatusWidths will automatically refresh
    SetStatusWidths(number, widths);
}

void wxStatusBarBase::SetStatusWidths([[maybe_unused]] int n,
                                    const int widths[])
{
    wxASSERT_MSG( (size_t)n == m_panes.size(), "field number mismatch" );

    if (widths == nullptr)
    {
        // special value meaning: override explicit pane widths and make them all
        // of the same size
        m_bSameWidthForAllPanes = true;
    }
    else
    {
        for ( size_t i = 0; i < m_panes.size(); i++ )
            m_panes[i].SetWidth(widths[i]);

        m_bSameWidthForAllPanes = false;
    }

    // update the display after the widths changed
    Refresh();
}

void wxStatusBarBase::SetStatusStyles([[maybe_unused]] int n,
                                    const int styles[])
{
    wxCHECK_RET( styles, "NULL pointer in SetStatusStyles" );

    wxASSERT_MSG( (size_t)n == m_panes.size(), "field number mismatch" );

    for ( size_t i = 0; i < m_panes.size(); i++ )
        m_panes[i].SetStyle(styles[i]);

    // update the display after the widths changed
    Refresh();
}

std::vector<int> wxStatusBarBase::CalculateAbsWidths(wxCoord widthTotal) const
{
    std::vector<int> widths;

    if ( m_bSameWidthForAllPanes )
    {
        // Default: all fields have the same width. This is not always
        // possible to do exactly (if widthTotal is not divisible by
        // m_panes.size()) - if that happens, we distribute the extra
        // pixels among all fields:
        int widthToUse = widthTotal;

        for ( size_t i = m_panes.size(); i > 0; i-- )
        {
            // divide the unassigned width evently between the
            // not yet processed fields:
            int w = widthToUse / i;
            widths.push_back(w);
            widthToUse -= w;
        }
    }
    else // do not override explicit pane widths
    {
        // calculate the total width of all the fixed width fields and the
        // total number of var field widths counting with multiplicity
        size_t nTotalWidth = 0;
        size_t nVarCount = 0;

        for ( size_t i{0}; i < m_panes.size(); i++ )
        {
            if ( m_panes[i].GetWidth() >= 0 )
                nTotalWidth += m_panes[i].GetWidth();
            else
                nVarCount += -m_panes[i].GetWidth();
        }

        // the amount of extra width we have per each var width field
        int widthExtra = widthTotal - nTotalWidth;

        // do fill the array
        for ( std::size_t i{0}; i < m_panes.size(); i++ )
        {
            if ( m_panes[i].GetWidth() >= 0 )
                widths.push_back(m_panes[i].GetWidth());
            else
            {
                int nVarWidth = widthExtra > 0 ? (widthExtra * (-m_panes[i].GetWidth())) / nVarCount : 0;
                nVarCount += m_panes[i].GetWidth();
                widthExtra -= nVarWidth;
                widths.push_back(nVarWidth);
            }
        }
    }

    return widths;
}

// ----------------------------------------------------------------------------
// setting/getting status text
// ----------------------------------------------------------------------------

void wxStatusBarBase::SetStatusText(std::string_view text, int number)
{
    wxCHECK_RET( (unsigned)number < m_panes.size(),
                    "invalid status bar field index" );

    if ( m_panes[number].SetText(text) )
        DoUpdateStatusText(number);
}

std::string wxStatusBarBase::GetStatusText(int number) const
{
    wxCHECK_MSG( (unsigned)number < m_panes.size(), "",
                    "invalid status bar field index" );

    return m_panes[number].GetText();
}

void wxStatusBarBase::SetEllipsizedFlag(int number, bool isEllipsized)
{
    wxCHECK_RET( (unsigned)number < m_panes.size(),
                    "invalid status bar field index" );

    m_panes[number].SetIsEllipsized(isEllipsized);
}

// ----------------------------------------------------------------------------
// pushing/popping status text
// ----------------------------------------------------------------------------

void wxStatusBarBase::PushStatusText(const std::string& text, int number)
{
    wxCHECK_RET( (unsigned)number < m_panes.size(),
                    "invalid status bar field index" );

    if ( m_panes[number].PushText(text) )
        DoUpdateStatusText(number);
}

void wxStatusBarBase::PopStatusText(int number)
{
    wxCHECK_RET( (unsigned)number < m_panes.size(),
                    "invalid status bar field index" );

    if ( m_panes[number].PopText() )
        DoUpdateStatusText(number);
}

#endif // wxUSE_STATUSBAR
