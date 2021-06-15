///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/headerctrlcmn.cpp
// Purpose:     implementation of wxHeaderCtrlBase
// Author:      Vadim Zeitlin
// Created:     2008-12-02
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_HEADERCTRL

#ifndef WX_PRECOMP
    #include "wx/menu.h"
#endif // WX_PRECOMP

#include "wx/headerctrl.h"
#include "wx/rearrangectrl.h"
#include "wx/renderer.h"

#include <numeric>

namespace
{

// ----------------------------------------------------------------------------
// wxHeaderColumnsRearrangeDialog: dialog for customizing our columns
// ----------------------------------------------------------------------------

#if wxUSE_REARRANGECTRL

class wxHeaderColumnsRearrangeDialog : public wxRearrangeDialog
{
public:
    wxHeaderColumnsRearrangeDialog(wxWindow *parent,
                                   const std::vector<int>& order,
                                   const std::vector<wxString>& items)
        : wxRearrangeDialog
          (
            parent,
            _("Please select the columns to show and define their order:"),
            _("Customize Columns"),
            order,
            items
          )
    {
    }
};

#endif // wxUSE_REARRANGECTRL

} // anonymous namespace

// ============================================================================
// wxHeaderCtrlBase implementation
// ============================================================================

wxBEGIN_EVENT_TABLE(wxHeaderCtrlBase, wxControl)
    EVT_HEADER_SEPARATOR_DCLICK(wxID_ANY, wxHeaderCtrlBase::OnSeparatorDClick)
#if wxUSE_MENUS
    EVT_HEADER_RIGHT_CLICK(wxID_ANY, wxHeaderCtrlBase::OnRClick)
#endif // wxUSE_MENUS
wxEND_EVENT_TABLE()

void wxHeaderCtrlBase::ScrollWindow(int dx,
                                    int WXUNUSED_UNLESS_DEBUG(dy),
                                    const wxRect * WXUNUSED_UNLESS_DEBUG(rect))

{
    // this doesn't make sense at all
    wxASSERT_MSG( !dy, "header window can't be scrolled vertically" );

    // this would actually be nice to support for "frozen" headers but it isn't
    // supported currently
    wxASSERT_MSG( !rect, "header window can't be scrolled partially" );

    DoScrollHorz(dx);
}

void wxHeaderCtrlBase::SetColumnCount(unsigned int count)
{
    if ( count != GetColumnCount() )
        OnColumnCountChanging(count);

    // still call DoSetCount() even if the count didn't really change in order
    // to update all the columns
    DoSetCount(count);
}

int wxHeaderCtrlBase::GetColumnTitleWidth(const wxHeaderColumn& col)
{
    int w = wxWindowBase::GetTextExtent(col.GetTitle()).x +
        // add some margin:
        wxRendererNative::Get().GetHeaderButtonMargin(this);

    // if a bitmap is used, add space for it and 2px border:
    wxBitmap bmp = col.GetBitmap();
    if ( bmp.IsOk() )
        w += bmp.GetWidth() + 2;

    return w;
}

// ----------------------------------------------------------------------------
// wxHeaderCtrlBase event handling
// ----------------------------------------------------------------------------

void wxHeaderCtrlBase::OnSeparatorDClick(wxHeaderCtrlEvent& event)
{
    const unsigned col = event.GetColumn();
    const wxHeaderColumn& column = GetColumn(col);

    if ( !column.IsResizeable() )
    {
        event.Skip();
        return;
    }

    int w = GetColumnTitleWidth(column);

    if ( !UpdateColumnWidthToFit(col, w) )
        event.Skip();
    else
        UpdateColumn(col);
}

#if wxUSE_MENUS

void wxHeaderCtrlBase::OnRClick(wxHeaderCtrlEvent& event)
{
    if ( !HasFlag(wxHD_ALLOW_HIDE) )
    {
        event.Skip();
        return;
    }

    ShowColumnsMenu(ScreenToClient(wxGetMousePosition()));
}

#endif // wxUSE_MENUS

// ----------------------------------------------------------------------------
// wxHeaderCtrlBase column reordering
// ----------------------------------------------------------------------------

void wxHeaderCtrlBase::SetColumnsOrder(const std::vector<int>& order)
{
    const unsigned count = GetColumnCount();
    wxCHECK_RET( order.size() == count, "wrong number of columns" );

    // check the array validity
    wxCHECK_RET( *std::max_element(order.cbegin(), order.cend()) < count, "invalid column index" );
    wxCHECK_RET( std::adjacent_find(order.cbegin(), order.cend()) == order.cend(), "duplicate column index" );

    DoSetColumnsOrder(order);

    // TODO-RTL: do we need to reverse the array?
}

void wxHeaderCtrlBase::ResetColumnsOrder()
{
    std::vector<int> order(GetColumnCount());

    std::iota(order.begin(), order.end(), 0);

    DoSetColumnsOrder(order);
}

std::vector<int> wxHeaderCtrlBase::GetColumnsOrder() const
{
    // TODO: Is this even necessary anymore?
    // wxASSERT_MSG( order.size() == GetColumnCount(), "invalid order array" );

    return DoGetColumnsOrder();
}

unsigned int wxHeaderCtrlBase::GetColumnAt(unsigned int pos) const
{
    return GetColumnsOrder().at(pos);
}

unsigned int wxHeaderCtrlBase::GetColumnPos(unsigned int idx) const
{
    wxCHECK_MSG( idx < GetColumnCount(), wxNO_COLUMN, "invalid index" );

    const std::vector<int> order = GetColumnsOrder();
    const auto pos = std::find(order.begin(), order.end(), idx);
    // TODO: This is impossible now, yeah?
    // wxCHECK_MSG( pos != std::end(order), wxNO_COLUMN, "column unexpectedly not displayed at all" );

    return std::distance(std::cbegin(order), pos);
}

/* static */
// TODO: This is a rotate.
void wxHeaderCtrlBase::MoveColumnInOrderArray(std::vector<int>& order,
                                              unsigned int idx,
                                              unsigned int pos)
{
    auto posOld = std::find(order.begin(), order.end(), idx);
    wxASSERT_MSG( posOld != std::end(order), "invalid index" );

    if ( pos != std::distance(std::begin(order), posOld) )
    {
        order.erase(posOld);
        order.insert(std::begin(order) + pos, idx);
    }
}

void
wxHeaderCtrlBase::DoResizeColumnIndices(std::vector<int>& colIndices, size_t count)
{
    // update the column indices array if necessary
    const auto countOld = colIndices.size();
    if ( count > countOld )
    {
        // all new columns have default positions equal to their indices
        for ( auto n = countOld; n < count; n++ )
            colIndices.push_back(n);
    }
    else if ( count < countOld )
    {
        // filter out all the positions which are invalid now while keeping the
        // order of the remaining ones
        // FIXME: Verify this is an off-by-one problem if greater than is used instead
        // of greater-than-or-equal-to.
        colIndices.erase(std::remove_if(colIndices.begin(), colIndices.end(),
            [&](const auto idx){ return idx >= count; }), colIndices.end());
    }
    //else: count didn't really change, nothing to do

    wxASSERT_MSG( colIndices.size() == count, "logic error" );
}

// ----------------------------------------------------------------------------
// wxHeaderCtrl extra UI
// ----------------------------------------------------------------------------

#if wxUSE_MENUS

void wxHeaderCtrlBase::AddColumnsItems(wxMenu& menu, int idColumnsBase)
{
    const unsigned count = GetColumnCount();
    for ( unsigned n = 0; n < count; n++ )
    {
        const wxHeaderColumn& col = GetColumn(n);
        menu.AppendCheckItem(idColumnsBase + n, col.GetTitle());
        if ( col.IsShown() )
            menu.Check(idColumnsBase + n, true);
    }
}

bool wxHeaderCtrlBase::ShowColumnsMenu(const wxPoint& pt, const wxString& title)
{
    // construct the menu with the entries for all columns
    wxMenu menu;
    if ( !title.empty() )
        menu.SetTitle(title);

    AddColumnsItems(menu, wxID_COLUMNS_BASE);

    // ... and an extra one to show the customization dialog if the user is
    // allowed to reorder the columns too
    const unsigned idCustomize = GetColumnCount() + wxID_COLUMNS_BASE;
    if ( HasFlag(wxHD_ALLOW_REORDER) )
    {
        menu.AppendSeparator();
        menu.Append(idCustomize, _("&Customize..."));
    }

    // do show the menu and get the user selection
    const int rc = GetPopupMenuSelectionFromUser(menu, pt);
    if ( rc == wxID_NONE )
        return false;

    if ( static_cast<unsigned>(rc) == idCustomize )
    {
        return ShowCustomizeDialog();
    }
    else // a column selected from the menu
    {
        const int columnIndex = rc - wxID_COLUMNS_BASE;
        UpdateColumnVisibility(columnIndex, !GetColumn(columnIndex).IsShown());
    }

    return true;
}

#endif // wxUSE_MENUS

bool wxHeaderCtrlBase::ShowCustomizeDialog()
{
#if wxUSE_REARRANGECTRL
    // prepare the data for showing the dialog
    std::vector<int> order = GetColumnsOrder();

    const unsigned count = GetColumnCount();

    // notice that titles are always in the index order, they will be shown
    // rearranged according to the display order in the dialog
    std::vector<wxString> titles;
    titles.reserve(count);
    
    for ( unsigned n = 0; n < count; n++ )
        titles.push_back(GetColumn(n).GetTitle());

    // this loop is however over positions and not indices
    for ( unsigned pos = 0; pos < count; pos++ )
    {
        int& idx = order[pos];
        if ( GetColumn(idx).IsHidden() )
        {
            // indicate that this one is hidden
            idx = ~idx;
        }
    }

    // do show it
    wxHeaderColumnsRearrangeDialog dlg(this, order, titles);
    if ( dlg.ShowModal() == wxID_OK )
    {
        // and apply the changes
        order = dlg.GetOrder();
        for ( unsigned pos = 0; pos < count; pos++ )
        {
            int& idx = order[pos];
            const bool show = idx >= 0;
            if ( !show )
            {
                // make all indices positive for passing them to SetColumnsOrder()
                idx = ~idx;
            }

            if ( show != GetColumn(idx).IsShown() )
                UpdateColumnVisibility(idx, show);
        }

        UpdateColumnsOrder(order);
        SetColumnsOrder(order);

        return true;
    }
#endif // wxUSE_REARRANGECTRL

    return false;
}

// ============================================================================
// wxHeaderCtrlSimple implementation
// ============================================================================

wxBEGIN_EVENT_TABLE(wxHeaderCtrlSimple, wxHeaderCtrl)
    EVT_HEADER_RESIZING(wxID_ANY, wxHeaderCtrlSimple::OnHeaderResizing)
wxEND_EVENT_TABLE()



const wxHeaderColumn& wxHeaderCtrlSimple::GetColumn(unsigned int idx) const
{
    return m_cols[idx];
}

void wxHeaderCtrlSimple::DoInsert(const wxHeaderColumnSimple& col, unsigned int idx)
{
    m_cols.insert(m_cols.begin() + idx, col);

    UpdateColumnCount();
}

void wxHeaderCtrlSimple::DoDelete(unsigned int idx)
{
    m_cols.erase(m_cols.begin() + idx);
    if ( idx == m_sortKey )
        m_sortKey = wxNO_COLUMN;

    UpdateColumnCount();
}

void wxHeaderCtrlSimple::DeleteAllColumns()
{
    m_cols.clear();
    m_sortKey = wxNO_COLUMN;

    UpdateColumnCount();
}


void wxHeaderCtrlSimple::DoShowColumn(unsigned int idx, bool show)
{
    if ( show != m_cols[idx].IsShown() )
    {
        m_cols[idx].SetHidden(!show);

        UpdateColumn(idx);
    }
}

void wxHeaderCtrlSimple::DoShowSortIndicator(unsigned int idx, bool ascending)
{
    RemoveSortIndicator();

    m_cols[idx].SetSortOrder(ascending);
    m_sortKey = idx;

    UpdateColumn(idx);
}

void wxHeaderCtrlSimple::RemoveSortIndicator()
{
    if ( m_sortKey != wxNO_COLUMN )
    {
        const unsigned sortOld = m_sortKey;
        m_sortKey = wxNO_COLUMN;

        m_cols[sortOld].UnsetAsSortKey();

        UpdateColumn(sortOld);
    }
}

bool
wxHeaderCtrlSimple::UpdateColumnWidthToFit(unsigned int idx, int widthTitle)
{
    const int widthContents = GetBestFittingWidth(idx);
    if ( widthContents == -1 )
        return false;

    m_cols[idx].SetWidth(wxMax(widthContents, widthTitle));

    return true;
}

void wxHeaderCtrlSimple::OnHeaderResizing(wxHeaderCtrlEvent& evt)
{
    m_cols[evt.GetColumn()].SetWidth(evt.GetWidth());
    Refresh();
}

// ============================================================================
// wxHeaderCtrlEvent implementation
// ============================================================================

wxIMPLEMENT_DYNAMIC_CLASS(wxHeaderCtrlEvent, wxNotifyEvent);

wxDEFINE_EVENT( wxEVT_HEADER_CLICK, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_RIGHT_CLICK, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_MIDDLE_CLICK, wxHeaderCtrlEvent);

wxDEFINE_EVENT( wxEVT_HEADER_DCLICK, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_RIGHT_DCLICK, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_MIDDLE_DCLICK, wxHeaderCtrlEvent);

wxDEFINE_EVENT( wxEVT_HEADER_SEPARATOR_DCLICK, wxHeaderCtrlEvent);

wxDEFINE_EVENT( wxEVT_HEADER_BEGIN_RESIZE, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_RESIZING, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_END_RESIZE, wxHeaderCtrlEvent);

wxDEFINE_EVENT( wxEVT_HEADER_BEGIN_REORDER, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_END_REORDER, wxHeaderCtrlEvent);

wxDEFINE_EVENT( wxEVT_HEADER_DRAGGING_CANCELLED, wxHeaderCtrlEvent);

#endif // wxUSE_HEADERCTRL
