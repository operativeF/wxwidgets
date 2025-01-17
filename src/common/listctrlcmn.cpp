////////////////////////////////////////////////////////////////////////////////
// Name:        src/common/listctrlcmn.cpp
// Purpose:     Common defines for wxListCtrl and wxListCtrl-based classes.
// Author:      Kevin Ollivier
// Created:     09/15/06
// Copyright:   (c) Kevin Ollivier
// Licence:     wxWindows licence
////////////////////////////////////////////////////////////////////////////////

#if wxUSE_LISTCTRL

#include "wx/listctrl.h"
#include "wx/imaglist.h"
#include "wx/dcclient.h"

// ListCtrl events
wxDEFINE_EVENT( wxEVT_LIST_BEGIN_DRAG, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_BEGIN_RDRAG, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_BEGIN_LABEL_EDIT, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_END_LABEL_EDIT, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_DELETE_ITEM, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_DELETE_ALL_ITEMS, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_SELECTED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_DESELECTED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_KEY_DOWN, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_INSERT_ITEM, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_COL_CLICK, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_COL_RIGHT_CLICK, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_COL_BEGIN_DRAG, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_COL_DRAGGING, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_COL_END_DRAG, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_RIGHT_CLICK, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_MIDDLE_CLICK, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_ACTIVATED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_FOCUSED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_CHECKED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_UNCHECKED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_CACHE_HINT, wxListEvent );

// -----------------------------------------------------------------------------
// XTI
// -----------------------------------------------------------------------------

wxDEFINE_FLAGS( wxListCtrlStyle )
wxBEGIN_FLAGS( wxListCtrlStyle )
// new style border flags, we put them first to
// use them for streaming out
wxFLAGS_MEMBER(wxBORDER_SIMPLE)
wxFLAGS_MEMBER(wxBORDER_SUNKEN)
wxFLAGS_MEMBER(wxBORDER_DOUBLE)
wxFLAGS_MEMBER(wxBORDER_RAISED)
wxFLAGS_MEMBER(wxBORDER_STATIC)
wxFLAGS_MEMBER(wxBORDER_NONE)

// old style border flags
wxFLAGS_MEMBER(wxSIMPLE_BORDER)
wxFLAGS_MEMBER(wxSUNKEN_BORDER)
wxFLAGS_MEMBER(wxDOUBLE_BORDER)
wxFLAGS_MEMBER(wxRAISED_BORDER)
wxFLAGS_MEMBER(wxSTATIC_BORDER)
wxFLAGS_MEMBER(wxBORDER)

// standard window styles
wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
wxFLAGS_MEMBER(wxCLIP_CHILDREN)
wxFLAGS_MEMBER(wxTRANSPARENT_WINDOW)
wxFLAGS_MEMBER(wxWANTS_CHARS)
wxFLAGS_MEMBER(wxFULL_REPAINT_ON_RESIZE)
wxFLAGS_MEMBER(wxALWAYS_SHOW_SB )
wxFLAGS_MEMBER(wxVSCROLL)
wxFLAGS_MEMBER(wxHSCROLL)

wxFLAGS_MEMBER(wxLC_LIST)
wxFLAGS_MEMBER(wxLC_REPORT)
wxFLAGS_MEMBER(wxLC_ICON)
wxFLAGS_MEMBER(wxLC_SMALL_ICON)
wxFLAGS_MEMBER(wxLC_ALIGN_TOP)
wxFLAGS_MEMBER(wxLC_ALIGN_LEFT)
wxFLAGS_MEMBER(wxLC_AUTOARRANGE)
wxFLAGS_MEMBER(wxLC_USER_TEXT)
wxFLAGS_MEMBER(wxLC_EDIT_LABELS)
wxFLAGS_MEMBER(wxLC_NO_HEADER)
wxFLAGS_MEMBER(wxLC_SINGLE_SEL)
wxFLAGS_MEMBER(wxLC_SORT_ASCENDING)
wxFLAGS_MEMBER(wxLC_SORT_DESCENDING)
wxFLAGS_MEMBER(wxLC_VIRTUAL)
wxEND_FLAGS( wxListCtrlStyle )

#if ((!defined(__WXMSW__) && !defined(__WXQT__) && !(defined(__WXMAC__) && wxOSX_USE_CARBON)) || defined(__WXUNIVERSAL__))
wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxListCtrl, wxGenericListCtrl, "wx/listctrl.h");
#else
wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxListCtrl, wxControl, "wx/listctrl.h");
#endif

wxBEGIN_PROPERTIES_TABLE(wxListCtrl)
wxEVENT_PROPERTY( TextUpdated, wxEVT_TEXT, wxCommandEvent )

wxPROPERTY_FLAGS( WindowStyle, wxListCtrlStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 /*flags*/, \
                 "Helpstring", "group") // style
wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxListCtrl)

wxCONSTRUCTOR_5( wxListCtrl, wxWindow*, Parent, wxWindowID, Id, \
                wxPoint, Position, wxSize, Size, long, WindowStyle )

/*
 TODO : Expose more information of a list's layout etc. via appropriate objects
 (see NotebookPageInfo)
 */

wxIMPLEMENT_DYNAMIC_CLASS(wxListView, wxListCtrl);
wxIMPLEMENT_DYNAMIC_CLASS(wxListEvent, wxNotifyEvent);

// ----------------------------------------------------------------------------
// wxListCtrlBase implementation
// ----------------------------------------------------------------------------

long
wxListCtrlBase::AppendColumn(const std::string& heading,
                             wxListColumnFormat format,
                             int width)
{
    return InsertColumn(GetColumnCount(), heading, format, width);
}

long
wxListCtrlBase::InsertColumn(long col,
                             const std::string& heading,
                             wxListColumnFormat format,
                             int width)
{
    wxListItem item;
    item.m_mask = {ListMasks::Text, ListMasks::Format};
    item.m_text = boost::nowide::widen(heading);
    if ( width >= 0
            || width == wxLIST_AUTOSIZE
                || width == wxLIST_AUTOSIZE_USEHEADER )
    {
        item.m_mask |= ListMasks::Width;
        item.m_width = width;
    }
    item.m_format = format;

    return InsertColumn(col, item);
}

long wxListCtrlBase::InsertColumn(long col, const wxListItem& info)
{
    long rc = DoInsertColumn(col, info);
    if ( rc != -1 )
    {
        // As our best size calculation depends on the column headers,
        // invalidate the previously cached best size when a column is added.
        InvalidateBestSize();
    }

    return rc;
}

wxSize wxListCtrlBase::DoGetBestClientSize() const
{
    // There is no obvious way to determine the best size in icon and list
    // modes so just don't do it for now.
    if ( !InReportView() )
        return wxControl::DoGetBestClientSize();

    int totalWidth;
    wxClientDC dc(const_cast<wxListCtrlBase*>(this));

    // In report mode, we use only the column headers, not items, to determine
    // the best width. The reason for this is that it's easier (we can't just
    // iterate over all items, especially not in a virtual control, so we'd
    // have to do something relatively complicated such as checking the size of
    // some items in the beginning and the end only) and also because the
    // columns are usually static while the list contents is dynamic so it
    // usually doesn't make much sense to adjust the control size to it anyhow.
    // And finally, scrollbars can always be used with the items while the
    // headers are just truncated if there is not enough place for them.
    const int columns = GetColumnCount();
    if ( HasFlag(wxLC_NO_HEADER) || !columns )
    {
        // Use some arbitrary width.
        totalWidth = 50*dc.wxGetCharWidth();
    }
    else // We do have columns, use them to determine the best width.
    {
        totalWidth = 0;
        for ( int col = 0; col < columns; col++ )
        {
            totalWidth += GetColumnWidth(col);
        }
    }

    // Use some arbitrary height, there is no good way to determine it.
    return {totalWidth, 10 * dc.GetCharHeight()};
}

void wxListCtrlBase::SetAlternateRowColour(const wxColour& colour)
{
    wxASSERT(HasFlag(wxLC_VIRTUAL));
    m_alternateRowColour.SetBackgroundColour(colour);
}

void wxListCtrlBase::EnableAlternateRowColours(bool enable)
{
    if ( enable )
    {
        // This code is copied from wxDataViewMainWindow::OnPaint()

        // Determine the alternate rows colour automatically from the
        // background colour.
        const wxColour bgColour = GetBackgroundColour();

        // Depending on the background, alternate row color
        // will be 3% more dark or 50% brighter.
        int alpha = bgColour.GetRGB() > 0x808080 ? 97 : 150;
        SetAlternateRowColour(bgColour.ChangeLightness(alpha));
    }
    else // Disable striping by setting invalid alternative colour.
    {
        SetAlternateRowColour(wxColour());
    }
}

wxItemAttr *wxListCtrlBase::OnGetItemAttr(long item) const
{
    return (m_alternateRowColour.GetBackgroundColour().IsOk() && (item % 2))
        ? const_cast<wxItemAttr *>(&m_alternateRowColour)
        : nullptr; // no attributes by default
}

std::string wxListCtrlBase::OnGetItemText([[maybe_unused]] long item, [[maybe_unused]] long col) const
{
    // this is a pure virtual function, in fact - which is not really pure
    // because the controls which are not virtual don't need to implement it
    wxFAIL_MSG("wxListCtrl::OnGetItemText not supposed to be called");

    return {};
}

bool wxListCtrlBase::OnGetItemIsChecked([[maybe_unused]] long item) const
{
    // this is a pure virtual function, in fact - which is not really pure
    // because the controls which are not virtual don't need to implement it
    wxFAIL_MSG("wxListCtrl::OnGetItemIsChecked not supposed to be called");

    return false;
}

int wxListCtrlBase::OnGetItemImage([[maybe_unused]] long item) const
{
    wxCHECK_MSG(!GetImageList(wxIMAGE_LIST_SMALL),
                -1,
                "List control has an image list, OnGetItemImage or OnGetItemColumnImage should be overridden.");
    return -1;
}

int wxListCtrlBase::OnGetItemColumnImage(long item, long column) const
{
    if ( !column )
        return OnGetItemImage(item);

    return -1;
}

wxItemAttr* wxListCtrlBase::OnGetItemColumnAttr(long item, [[maybe_unused]] long column) const
{
    return OnGetItemAttr(item);
}

#endif // wxUSE_LISTCTRL
