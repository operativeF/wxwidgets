/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/datavcmn.cpp
// Purpose:     wxDataViewCtrl base classes and common parts
// Author:      Robert Roebling
// Created:     2006/02/20
// Copyright:   (c) 2006, Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_DATAVIEWCTRL

#include "wx/dataview.h"

#ifndef WX_PRECOMP
    #include <fmt/core.h>
    #include <gsl/gsl>
#endif

#include "wx/alignmentflags.h"
#include "wx/dc.h"
#include "wx/settings.h"
#include "wx/log.h"
#include "wx/crt.h"
#include "wx/datectrl.h"
#include "wx/except.h"
#include "wx/spinctrl.h"
#include "wx/choice.h"
#include "wx/renderer.h"
#include "wx/textctrl.h"

#if wxUSE_ACCESSIBILITY
    #include "wx/access.h"
#endif // wxUSE_ACCESSIBILITY

namespace
{

// Custom handler pushed on top of the edit control used by wxDataViewCtrl to
// forward some events to the main control itself.
class wxDataViewEditorCtrlEvtHandler: public wxEvtHandler
{
public:
    wxDataViewEditorCtrlEvtHandler(wxWindow *editor, wxDataViewRenderer *owner)
        : m_editorCtrl(editor),
          m_owner(owner)
    {
    }

    void SetFocusOnIdle( bool focus = true ) { m_focusOnIdle = focus; }

protected:
    void OnChar( wxKeyEvent &event );
    void OnTextEnter( wxCommandEvent &event );
    void OnKillFocus( wxFocusEvent &event );
    void OnIdle( wxIdleEvent &event );

private:
    bool IsEditorSubControl(wxWindow* win) const;

    wxDataViewRenderer     *m_owner;
    wxWindow               *m_editorCtrl;

    bool                    m_finished{false};
    bool                    m_focusOnIdle;

    wxDECLARE_EVENT_TABLE();
};

} // anonymous namespace

// ---------------------------------------------------------
// wxDataViewItemAttr
// ---------------------------------------------------------

wxFont wxDataViewItemAttr::GetEffectiveFont(const wxFont& font) const
{
    if ( !HasFont() )
        return font;

    wxFont f(font);
    if ( GetBold() )
        f.MakeBold();
    if ( GetItalic() )
        f.MakeItalic();
    if ( GetStrikethrough() )
        f.MakeStrikethrough();
    return f;
}


// ---------------------------------------------------------
// wxDataViewModelNotifier
// ---------------------------------------------------------

bool wxDataViewModelNotifier::ItemsAdded( const wxDataViewItem &parent, const wxDataViewItemArray &items )
{
    for (const auto& item : items)
        if (!ItemAdded( parent, item ))
            return false;

    return true;
}

bool wxDataViewModelNotifier::ItemsDeleted( const wxDataViewItem &parent, const wxDataViewItemArray &items )
{
    for (const auto& item : items)
        if (!ItemDeleted( parent, item ))
            return false;

    return true;
}

bool wxDataViewModelNotifier::ItemsChanged( const wxDataViewItemArray &items )
{
    for (const auto& item : items)
        if (!ItemChanged( item ))
            return false;

    return true;
}

// ---------------------------------------------------------
// wxDataViewModel
// ---------------------------------------------------------

wxDataViewModel::~wxDataViewModel()
{
    wxDataViewModelNotifiers::const_iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        delete *iter;
    }
}

bool wxDataViewModel::ItemAdded( const wxDataViewItem &parent, const wxDataViewItem &item )
{
    bool ret = true;

    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        if (!notifier->ItemAdded( parent, item ))
            ret = false;
    }

    return ret;
}

bool wxDataViewModel::ItemDeleted( const wxDataViewItem &parent, const wxDataViewItem &item )
{
    bool ret = true;

    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        if (!notifier->ItemDeleted( parent, item ))
            ret = false;
    }

    return ret;
}

bool wxDataViewModel::ItemChanged( const wxDataViewItem &item )
{
    bool ret = true;

    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        if (!notifier->ItemChanged( item ))
            ret = false;
    }

    return ret;
}

bool wxDataViewModel::ItemsAdded( const wxDataViewItem &parent, const wxDataViewItemArray &items )
{
    bool ret = true;

    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        if (!notifier->ItemsAdded( parent, items ))
            ret = false;
    }

    return ret;
}

bool wxDataViewModel::ItemsDeleted( const wxDataViewItem &parent, const wxDataViewItemArray &items )
{
    bool ret = true;

    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        if (!notifier->ItemsDeleted( parent, items ))
            ret = false;
    }

    return ret;
}

bool wxDataViewModel::ItemsChanged( const wxDataViewItemArray &items )
{
    bool ret = true;

    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        if (!notifier->ItemsChanged( items ))
            ret = false;
    }

    return ret;
}

bool wxDataViewModel::ValueChanged( const wxDataViewItem &item, unsigned int col )
{
    bool ret = true;

    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        if (!notifier->ValueChanged( item, col ))
            ret = false;
    }

    return ret;
}

bool wxDataViewModel::Cleared()
{
    bool ret = true;

    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        if (!notifier->Cleared())
            ret = false;
    }

    return ret;
}

bool wxDataViewModel::BeforeReset()
{
    bool ret = true;

    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        if (!notifier->BeforeReset())
            ret = false;
    }

    return ret;
}

bool wxDataViewModel::AfterReset()
{
    bool ret = true;

    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        if (!notifier->AfterReset())
            ret = false;
    }

    return ret;
}

void wxDataViewModel::Resort()
{
    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        wxDataViewModelNotifier* notifier = *iter;
        notifier->Resort();
    }
}

void wxDataViewModel::AddNotifier( wxDataViewModelNotifier *notifier )
{
    m_notifiers.push_back( notifier );
    notifier->SetOwner( this );
}

void wxDataViewModel::RemoveNotifier( wxDataViewModelNotifier *notifier )
{
    wxDataViewModelNotifiers::iterator iter;
    for (iter = m_notifiers.begin(); iter != m_notifiers.end(); ++iter)
    {
        if ( *iter == notifier )
        {
            delete notifier;
            m_notifiers.erase(iter);

            // Skip the assert below.
            return;
        }
    }

    wxFAIL_MSG(wxS("Removing non-registered notifier"));
}

int wxDataViewModel::Compare( const wxDataViewItem &item1, const wxDataViewItem &item2,
                              unsigned int column, bool ascending ) const
{
    wxVariant value1,value2;

    // Avoid calling GetValue() for the cells that are not supposed to have any
    // value, this might be unexpected.
    if ( HasValue(item1, column) )
        GetValue( value1, item1, column );
    if ( HasValue(item2, column) )
        GetValue( value2, item2, column );

    if (!ascending)
    {
        wxVariant temp = value1;
        value1 = value2;
        value2 = temp;
    }

    if (value1.GetType() == "string")
    {
        std::string str1 = value1.GetString();
        std::string str2 = value2.GetString();
        const int res = str1.compare( str2 );
        if (res)
            return res;
    }
    else if (value1.GetType() == "long")
    {
        const long l1 = value1.GetLong();
        const long l2 = value2.GetLong();
        if (l1 < l2)
            return -1;
        else if (l1 > l2)
            return 1;
    }
    else if (value1.GetType() == "double")
    {
        const double d1 = value1.GetDouble();
        const double d2 = value2.GetDouble();
        if (d1 < d2)
            return -1;
        else if (d1 > d2)
            return 1;
    }
#if wxUSE_DATETIME
    else if (value1.GetType() == "datetime")
    {
        const wxDateTime dt1 = value1.GetDateTime();
        const wxDateTime dt2 = value2.GetDateTime();
        if (dt1.IsEarlierThan(dt2))
            return -1;
        if (dt2.IsEarlierThan(dt1))
            return 1;
    }
#endif // wxUSE_DATETIME
    else if (value1.GetType() == "bool")
    {
        const bool b1 = value1.GetBool();
        const bool b2 = value2.GetBool();

        if (b1 != b2)
            return b1 ? 1 : -1;
    }
    else if (value1.GetType() == "wxDataViewIconText")
    {
        wxDataViewIconText iconText1, iconText2;

        iconText1 << value1;
        iconText2 << value2;

        int res = iconText1.GetText().compare(iconText2.GetText());
        if (res != 0)
          return res;
    }
    else
    {
        const int res = DoCompareValues(value1, value2);
        if (res != 0)
            return res;
    }


    // items must be different
    const wxUIntPtr id1 = wxPtrToUInt(item1.GetID());
    const wxUIntPtr id2 = wxPtrToUInt(item2.GetID());

    return ascending ? id1 - id2 : id2 - id1;
}

// ---------------------------------------------------------
// wxDataViewIndexListModel
// ---------------------------------------------------------

wxDataViewIndexListModel::wxDataViewIndexListModel( unsigned int initial_size )
{
    // build initial index
    for (unsigned int i = 1; i < initial_size+1; i++)
            m_hash.push_back( wxDataViewItem(wxUIntToPtr(i)) );
    m_nextFreeID = initial_size + 1;
}

void wxDataViewIndexListModel::Reset( unsigned int new_size )
{
    /* wxDataViewModel:: */ BeforeReset();

    m_hash.clear();

    // IDs are ordered until an item gets deleted or inserted
    m_ordered = true;

    // build initial index
    for (unsigned int i = 1; i < new_size + 1; i++)
            m_hash.push_back( wxDataViewItem(wxUIntToPtr(i)) );

    m_nextFreeID = new_size + 1;

    /* wxDataViewModel:: */ AfterReset();
}

void wxDataViewIndexListModel::RowPrepended()
{
    m_ordered = false;

    const unsigned int id = m_nextFreeID;
    m_nextFreeID++;

    const wxDataViewItem item( wxUIntToPtr(id) );
    m_hash.insert( m_hash.begin(), item );
    ItemAdded( wxDataViewItem(nullptr), item );

}

void wxDataViewIndexListModel::RowInserted( unsigned int before )
{
    m_ordered = false;

    const unsigned int id = m_nextFreeID;
    m_nextFreeID++;

    const wxDataViewItem item( wxUIntToPtr(id) );
    m_hash.insert( m_hash.begin() + before, item );
    ItemAdded( wxDataViewItem(nullptr), item );
}

void wxDataViewIndexListModel::RowAppended()
{
    const unsigned int id = m_nextFreeID;
    m_nextFreeID++;

    const wxDataViewItem item( wxUIntToPtr(id) );
    m_hash.push_back( item );
    ItemAdded( wxDataViewItem(nullptr), item );
}

void wxDataViewIndexListModel::RowDeleted( unsigned int row )
{
    m_ordered = false;

    const wxDataViewItem item( m_hash[row] );
    m_hash.erase(m_hash.begin() + row );
    /* wxDataViewModel:: */ ItemDeleted( wxDataViewItem(nullptr), item );
}

void wxDataViewIndexListModel::RowsDeleted( const std::vector<int> &rows )
{
    m_ordered = false;

    wxDataViewItemArray array;
    for (size_t i = 0; i < rows.size(); i++)
    {
            const wxDataViewItem item( m_hash[rows[i]] );
            array.push_back( item );
    }

    std::vector<int> sorted = rows;

    std::sort(sorted.begin(), sorted.end());

    for (size_t i = 0; i < sorted.size(); i++)
           m_hash.erase( m_hash.begin() + sorted[i] );

    /* wxDataViewModel:: */ ItemsDeleted( wxDataViewItem(nullptr), array );
}

void wxDataViewIndexListModel::RowChanged( unsigned int row )
{
    /* wxDataViewModel:: */ ItemChanged( GetItem(row) );
}

void wxDataViewIndexListModel::RowValueChanged( unsigned int row, unsigned int col )
{
    /* wxDataViewModel:: */ ValueChanged( GetItem(row), col );
}

unsigned int wxDataViewIndexListModel::GetRow( const wxDataViewItem &item ) const
{
    if (m_ordered)
        return wxPtrToUInt(item.GetID())-1;

    // assert for not found
    return std::distance(m_hash.begin(), std::find(m_hash.cbegin(), m_hash.cend(), item));
}

wxDataViewItem wxDataViewIndexListModel::GetItem( unsigned int row ) const
{
    wxASSERT( row < m_hash.size() );
    return {m_hash[row]};
}

unsigned int wxDataViewIndexListModel::GetChildren( const wxDataViewItem &item, wxDataViewItemArray &children ) const
{
    if (item.IsOk())
        return 0;

    children = m_hash;

    return m_hash.size();
}

// ---------------------------------------------------------
// wxDataViewVirtualListModel
// ---------------------------------------------------------

#ifndef __WXMAC__

wxDataViewVirtualListModel::wxDataViewVirtualListModel( unsigned int initial_size )
    : m_size(initial_size)
{
}

void wxDataViewVirtualListModel::Reset( unsigned int new_size )
{
    /* wxDataViewModel:: */ BeforeReset();

    m_size = new_size;

    /* wxDataViewModel:: */ AfterReset();
}

void wxDataViewVirtualListModel::RowPrepended()
{
    m_size++;
    wxDataViewItem item( wxUIntToPtr(1) );
    ItemAdded( wxDataViewItem(nullptr), item );
}

void wxDataViewVirtualListModel::RowInserted( unsigned int before )
{
    m_size++;
    wxDataViewItem item( wxUIntToPtr(before+1) );
    ItemAdded( wxDataViewItem(nullptr), item );
}

void wxDataViewVirtualListModel::RowAppended()
{
    m_size++;
    wxDataViewItem item( wxUIntToPtr(m_size) );
    ItemAdded( wxDataViewItem(nullptr), item );
}

void wxDataViewVirtualListModel::RowDeleted( unsigned int row )
{
    m_size--;
    wxDataViewItem item( wxUIntToPtr(row+1) );
    /* wxDataViewModel:: */ ItemDeleted( wxDataViewItem(nullptr), item );
}

void wxDataViewVirtualListModel::RowsDeleted( const std::vector<int> &rows )
{
    m_size -= rows.size();

    std::vector<int> sorted = rows;
    std::sort(sorted.begin(), sorted.end());

    wxDataViewItemArray array;

    for (unsigned int i = 0; i < sorted.size(); i++)
    {
        wxDataViewItem item( wxUIntToPtr(sorted[i]+1) );
        array.push_back( item );
    }
    /* wxDataViewModel:: */ ItemsDeleted( wxDataViewItem(nullptr), array );
}

void wxDataViewVirtualListModel::RowChanged( unsigned int row )
{
    /* wxDataViewModel:: */ ItemChanged( GetItem(row) );
}

void wxDataViewVirtualListModel::RowValueChanged( unsigned int row, unsigned int col )
{
    /* wxDataViewModel:: */ ValueChanged( GetItem(row), col );
}

unsigned int wxDataViewVirtualListModel::GetRow( const wxDataViewItem &item ) const
{
    return wxPtrToUInt( item.GetID() ) -1;
}

wxDataViewItem wxDataViewVirtualListModel::GetItem( unsigned int row ) const
{
    return wxDataViewItem( wxUIntToPtr(row+1) );
}

bool wxDataViewVirtualListModel::HasDefaultCompare() const
{
    return true;
}

int wxDataViewVirtualListModel::Compare(const wxDataViewItem& item1,
                                      const wxDataViewItem& item2,
                                      unsigned int WXUNUSED(column),
                                      bool ascending) const
{
    const unsigned int pos1 = wxPtrToUInt(item1.GetID());  // -1 not needed here
    const unsigned int pos2 = wxPtrToUInt(item2.GetID());  // -1 not needed here

    if (ascending)
       return pos1 - pos2;
    else
       return pos2 - pos1;
}

unsigned int wxDataViewVirtualListModel::GetChildren( const wxDataViewItem &WXUNUSED(item), wxDataViewItemArray &WXUNUSED(children) ) const
{
    return 0;  // should we report an error ?
}

#endif  // __WXMAC__

//-----------------------------------------------------------------------------
// wxDataViewIconText
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxDataViewIconText,wxObject);

IMPLEMENT_VARIANT_OBJECT_EXPORTED(wxDataViewIconText, WXDLLIMPEXP_CORE)

// ---------------------------------------------------------
// wxDataViewRendererBase
// ---------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxDataViewRendererBase, wxObject);

wxDataViewRendererBase::wxDataViewRendererBase( const std::string &varianttype,
                                                wxDataViewCellMode WXUNUSED(mode),
                                                int WXUNUSED(align) )
    : m_variantType(varianttype)
{
}

wxDataViewRendererBase::~wxDataViewRendererBase()
{
    if ( m_editorCtrl )
        DestroyEditControl();
    delete m_valueAdjuster;
}

wxDataViewCtrl* wxDataViewRendererBase::GetView() const
{
    return const_cast<wxDataViewRendererBase*>(this)->GetOwner()->GetOwner();
}

bool wxDataViewRendererBase::StartEditing( const wxDataViewItem &item, wxRect labelRect )
{
    wxDataViewColumn* const column = GetOwner();
    wxDataViewCtrl* const dv_ctrl = column->GetOwner();

    // Before doing anything we send an event asking if editing of this item is really wanted.
    wxDataViewEvent event(wxEVT_DATAVIEW_ITEM_START_EDITING, dv_ctrl, column, item);
    dv_ctrl->GetEventHandler()->ProcessEvent( event );
    if( !event.IsAllowed() )
        return false;

    // Remember the item being edited for use in FinishEditing() later.
    m_item = item;

    unsigned int col = GetOwner()->GetModelColumn();
    const wxVariant& value = CheckedGetValue(dv_ctrl->GetModel(), item, col);

    m_editorCtrl = CreateEditorCtrl( dv_ctrl->GetMainWindow(), labelRect, value );

    // there might be no editor control for the given item
    if(!m_editorCtrl)
    {
        m_item = wxDataViewItem();
        return false;
    }

    wxDataViewEditorCtrlEvtHandler *handler =
        new wxDataViewEditorCtrlEvtHandler( m_editorCtrl, (wxDataViewRenderer*) this );

    m_editorCtrl->PushEventHandler( handler );

#if defined(__WXGTK20__) && !defined(wxHAS_GENERIC_DATAVIEWCTRL)
    handler->SetFocusOnIdle();
#else
    m_editorCtrl->SetFocus();
#endif

    return true;
}

void wxDataViewRendererBase::NotifyEditingStarted(const wxDataViewItem& item)
{
    wxDataViewColumn* const column = GetOwner();
    wxDataViewCtrl* const dv_ctrl = column->GetOwner();

    wxDataViewEvent event(wxEVT_DATAVIEW_ITEM_EDITING_STARTED, dv_ctrl, column, item);
    dv_ctrl->GetEventHandler()->ProcessEvent( event );
}

void wxDataViewRendererBase::DestroyEditControl()
{
    // Remove our event handler first to prevent it from (recursively) calling
    // us again as it would do via a call to FinishEditing() when the editor
    // loses focus when we hide it below.
    wxEvtHandler * const handler = m_editorCtrl->PopEventHandler();

    // Hide the control immediately but don't delete it yet as there could be
    // some pending messages for it.
    m_editorCtrl->Hide();

    wxPendingDelete.Append(handler);
    wxPendingDelete.Append(m_editorCtrl);

    // Ensure that DestroyEditControl() is not called again for this control.
    m_editorCtrl.Release();
}

void wxDataViewRendererBase::CancelEditing()
{
    if ( m_editorCtrl )
        DestroyEditControl();

    DoHandleEditingDone(nullptr);
}

bool wxDataViewRendererBase::FinishEditing()
{
    if (!m_editorCtrl)
        return true;

    bool gotValue = false;

    wxVariant value;
    if ( GetValueFromEditorCtrl(m_editorCtrl, value) )
    {
        // This is the normal case and we will use this value below (if it
        // passes validation).
        gotValue = true;
    }
    //else: Not really supposed to happen, but still proceed with
    //      destroying the edit control if it does.

    DestroyEditControl();

    GetView()->GetMainWindow()->SetFocus();

    return DoHandleEditingDone(gotValue ? &value : nullptr);
}

bool
wxDataViewRendererBase::DoHandleEditingDone(wxVariant* value)
{
    if ( value )
    {
        if ( !Validate(*value) )
        {
            // Invalid value can't be used, so if it's the same as if we hadn't
            // got it in the first place.
            value = nullptr;
        }
    }

    wxDataViewColumn* const column = GetOwner();
    wxDataViewCtrl* const dv_ctrl = column->GetOwner();
    unsigned int col = column->GetModelColumn();

    // Now we should send Editing Done event
    wxDataViewEvent event(wxEVT_DATAVIEW_ITEM_EDITING_DONE, dv_ctrl, column, m_item);
    if ( value )
        event.SetValue(*value);
    else
        event.SetEditCancelled();

    dv_ctrl->GetEventHandler()->ProcessEvent( event );

    bool accepted = false;
    if ( value && event.IsAllowed() )
    {
        dv_ctrl->GetModel()->ChangeValue(*value, m_item, col);
        accepted = true;
    }

    m_item = wxDataViewItem();

    return accepted;
}

wxVariant
wxDataViewRendererBase::CheckedGetValue(const wxDataViewModel* model,
                                        const wxDataViewItem& item,
                                        unsigned column) const
{
    wxVariant value;
    // Avoid calling GetValue() if the model isn't supposed to have any values
    // in this cell (e.g. a non-first column of a container item), this could
    // be unexpected.
    if ( model->HasValue(item, column) )
        model->GetValue(value, item, column);

    // We always allow the cell to be null, regardless of the renderer type.
    if ( !value.IsNull() )
    {
        if ( value.GetType() != GetVariantType() )
        {
            // If you're seeing this message, this indicates that either your
            // renderer is using the wrong type, or your model returns values
            // of the wrong type.
            wxLogDebug("Wrong type returned from the model for column %u: "
                       "%s required but actual type is %s",
                       column,
                       GetVariantType(),
                       value.GetType());

            // Don't return data of mismatching type, this could be unexpected.
            value.MakeNull();
        }
    }

    return value;
}

bool
wxDataViewRendererBase::PrepareForItem(const wxDataViewModel *model,
                                       const wxDataViewItem& item,
                                       unsigned column)
{
    // This method is called by the native control, so we shouldn't allow
    // exceptions to escape from it.
    wxTRY
    {

    // Now check if we have a value and remember it for rendering it later.
    // Notice that we do it even if it's null, as the cell should be empty then
    // and not show the last used value.
    wxVariant value = CheckedGetValue(model, item, column);

    if ( m_valueAdjuster )
    {
        if ( IsHighlighted() )
            value = m_valueAdjuster->MakeHighlighted(value);
    }

    SetValue(value);

    if ( !value.IsNull() )
    {
        // Also set up the attributes for this item if it's not empty.
        wxDataViewItemAttr attr;
        model->GetAttr(item, column, attr);
        SetAttr(attr);
    }

    // Finally determine the enabled/disabled state and apply it, even to the
    // empty cells.
    SetEnabled(model->IsEnabled(item, column));

    }
    wxCATCH_ALL
    (
        // There is not much we can do about it here, just log it and don't
        // show anything in this cell.
        wxLogDebug("Retrieving the value from the model threw an exception");
        SetValue(wxVariant());
    )

    return true;
}


wxAlignment wxDataViewRendererBase::GetEffectiveAlignment() const
{
    wxAlignment alignment = GetEffectiveAlignmentIfKnown();
    wxASSERT( alignment != wxDVR_DEFAULT_ALIGNMENT );
    return alignment;
}


int wxDataViewRendererBase::GetEffectiveAlignmentIfKnown() const
{
    wxAlignment alignment = GetAlignment();

    if ( alignment == wxDVR_DEFAULT_ALIGNMENT )
    {
        if ( GetOwner() != nullptr )
        {
            // if we don't have an explicit alignment ourselves, use that of the
            // column in horizontal direction and default vertical alignment
            alignment = GetOwner()->GetAlignment() | wxAlignment::CenterVertical;
        }
    }

    return alignment;
}

// ----------------------------------------------------------------------------
// wxDataViewCustomRendererBase
// ----------------------------------------------------------------------------

void wxDataViewCustomRendererBase::RenderBackground(wxDC* dc, const wxRect& rect)
{
    if ( !m_attr.HasBackgroundColour() )
        return;

    const wxColour& colour = m_attr.GetBackgroundColour();
    wxDCPenChanger changePen(*dc, colour);
    wxDCBrushChanger changeBrush(*dc, colour);

    dc->DrawRectangle(rect);
}

void
wxDataViewCustomRendererBase::WXCallRender(wxRect rectCell, wxDC *dc, int state)
{
    wxCHECK_RET( dc, "no DC to draw on in custom renderer?" );

    // adjust the rectangle ourselves to account for the alignment
    wxRect rectItem = rectCell;
    const int align = GetEffectiveAlignment();

    const wxSize size = GetSize();

    // take alignment into account only if there is enough space, otherwise
    // show as much contents as possible
    //
    // notice that many existing renderers (e.g. wxDataViewSpinRenderer)
    // return hard-coded size which can be more than they need and if we
    // trusted their GetSize() we'd draw the text out of cell bounds
    // entirely

    if ( size.x >= 0 && size.x < rectCell.width )
    {
        if ( align & wxAlignment::CenterHorizontal )
            rectItem.x += (rectCell.width - size.x)/2;
        else if ( align & wxAlignment::Right )
            rectItem.x += rectCell.width - size.x;
        // else: wxAlignment::Left is the default

        rectItem.width = size.x;
    }

    if ( size.y >= 0 && size.y < rectCell.height )
    {
        if ( align & wxAlignment::CenterVertical )
            rectItem.y += (rectCell.height - size.y)/2;
        else if ( align & wxAlignment::Bottom )
            rectItem.y += rectCell.height - size.y;
        // else: wxAlignment::Top is the default

        rectItem.height = size.y;
    }


    // set up the DC attributes

    // override custom foreground with the standard one for the selected items
    // because we currently don't allow changing the selection background and
    // custom colours may be unreadable on it
    wxColour col;
    if ( state & wxDATAVIEW_CELL_SELECTED )
        col = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    else if ( m_attr.HasColour() )
        col = m_attr.GetColour();
    else // use default foreground
        col = GetOwner()->GetOwner()->GetForegroundColour();

    wxDCTextColourChanger changeFg(*dc, col);

    wxDCFontChanger changeFont(*dc);
    if ( m_attr.HasFont() )
        changeFont.Set(m_attr.GetEffectiveFont(dc->GetFont()));

    Render(rectItem, dc, state);
}

wxSize wxDataViewCustomRendererBase::GetTextExtent(const std::string& str) const
{
    const wxDataViewCtrl *view = GetView();

    if ( m_attr.HasFont() )
    {
        wxFont font(m_attr.GetEffectiveFont(view->GetFont()));
        return view->GetTextExtent(str, nullptr, nullptr, &font);
    }
    else
    {
        return view->GetTextExtent(str);
    }
}

void
wxDataViewCustomRendererBase::RenderText(const std::string& text,
                                         int xoffset,
                                         wxRect rect,
                                         wxDC *dc,
                                         int state)
{
    wxRect rectText = rect;
    rectText.x += xoffset;
    rectText.width -= xoffset;

    unsigned int flags = 0;
    if ( state & wxDATAVIEW_CELL_SELECTED )
        flags |= wxCONTROL_SELECTED;
    if ( !(GetOwner()->GetOwner()->IsEnabled() && GetEnabled()) )
        flags |= wxCONTROL_DISABLED;

    // Notice that we intentionally don't use any alignment here: it is not
    // necessary because the cell rectangle had been already adjusted to
    // account for the alignment in WXCallRender() and using the alignment here
    // results in problems with ellipsization when using native MSW renderer,
    // see https://trac.wxwidgets.org/ticket/17363, so just don't do it.
    wxRendererNative::Get().DrawItemText(
        GetOwner()->GetOwner(),
        *dc,
        text,
        rectText,
        wxAlignment::None,
        flags,
        GetEllipsizeMode());
}

void wxDataViewCustomRendererBase::SetEnabled(bool enabled)
{
    // The native base renderer needs to know about the enabled state as well
    // but in the generic case the base class method is pure, so we can't just
    // call it unconditionally.
#ifndef wxHAS_GENERIC_DATAVIEWCTRL
    wxDataViewRenderer::SetEnabled(enabled);
#endif // !wxHAS_GENERIC_DATAVIEWCTRL

    m_enabled = enabled;
}

//-----------------------------------------------------------------------------
// wxDataViewEditorCtrlEvtHandler
//-----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxDataViewEditorCtrlEvtHandler, wxEvtHandler)
    EVT_CHAR           (wxDataViewEditorCtrlEvtHandler::OnChar)
    EVT_KILL_FOCUS     (wxDataViewEditorCtrlEvtHandler::OnKillFocus)
    EVT_IDLE           (wxDataViewEditorCtrlEvtHandler::OnIdle)
    EVT_TEXT_ENTER     (-1, wxDataViewEditorCtrlEvtHandler::OnTextEnter)
wxEND_EVENT_TABLE()

void wxDataViewEditorCtrlEvtHandler::OnIdle( wxIdleEvent &event )
{
    if (m_focusOnIdle)
    {
        m_focusOnIdle = false;

        // Ignore focused items within the compound editor control
        wxWindow* win = wxWindow::FindFocus();
        if ( !IsEditorSubControl(win) )
        {
            m_editorCtrl->SetFocus();
        }
    }

    event.Skip();
}

void wxDataViewEditorCtrlEvtHandler::OnTextEnter( wxCommandEvent &WXUNUSED(event) )
{
    m_finished = true;
    m_owner->FinishEditing();
}

void wxDataViewEditorCtrlEvtHandler::OnChar( wxKeyEvent &event )
{
    switch ( event.m_keyCode )
    {
        case WXK_ESCAPE:
            m_finished = true;
            m_owner->CancelEditing();
            break;

        case WXK_RETURN:
            if ( !event.HasAnyModifiers() )
            {
                m_finished = true;
                m_owner->FinishEditing();
                break;
            }
            [[fallthrough]]; // Ctrl/Alt/Shift-Enter is not handled specially

        default:
            event.Skip();
    }
}

void wxDataViewEditorCtrlEvtHandler::OnKillFocus( wxFocusEvent &event )
{
    // Ignore focus changes within the compound editor control
    wxWindow* win = event.GetWindow();
    if ( IsEditorSubControl(win) )
    {
        event.Skip();
        return;
    }

    if (!m_finished)
    {
        m_finished = true;
        m_owner->FinishEditing();
    }

    event.Skip();
}

bool wxDataViewEditorCtrlEvtHandler::IsEditorSubControl(wxWindow* win) const
{
    // Checks whether the given window belongs to the editor control
    // (is either the editor itself or a child of the compound editor).
    while ( win )
    {
        if ( win == m_editorCtrl )
        {
            return true;
        }

        win = win->GetParent();
    }

    return false;
}

// ---------------------------------------------------------
// wxDataViewColumnBase
// ---------------------------------------------------------

void wxDataViewColumnBase::Init(wxDataViewRenderer *renderer,
                                unsigned int model_column)
{
    m_renderer = renderer;
    m_model_column = model_column;
    m_owner = nullptr;
    m_renderer->SetOwner( (wxDataViewColumn*) this );
}

wxDataViewColumnBase::~wxDataViewColumnBase()
{
    delete m_renderer;
}

// ---------------------------------------------------------
// wxDataViewCtrlBase
// ---------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxDataViewCtrlBase, wxControl);

wxDataViewCtrlBase::~wxDataViewCtrlBase()
{
    if (m_model)
    {
        m_model->DecRef();
        m_model = nullptr;
    }
}

bool wxDataViewCtrlBase::AssociateModel( wxDataViewModel *model )
{
    if (m_model)
    {
        m_model->DecRef();   // discard old model, if any
    }

    // add our own reference to the new model:
    m_model = model;
    if (m_model)
    {
        m_model->IncRef();
    }

    return true;
}

wxDataViewModel* wxDataViewCtrlBase::GetModel()
{
    return m_model;
}

const wxDataViewModel* wxDataViewCtrlBase::GetModel() const
{
    return m_model;
}

void wxDataViewCtrlBase::Expand(const wxDataViewItem& item)
{
    ExpandAncestors(item);

    DoExpand(item, false);
}

void wxDataViewCtrlBase::ExpandChildren(const wxDataViewItem& item)
{
    ExpandAncestors(item);

    DoExpand(item, true);
}

void wxDataViewCtrlBase::ExpandAncestors( const wxDataViewItem & item )
{
    if (!m_model) return;

    if (!item.IsOk()) return;

    std::vector<wxDataViewItem> parentChain;

    // at first we get all the parents of the selected item
    wxDataViewItem parent = m_model->GetParent(item);
    while (parent.IsOk())
    {
        parentChain.push_back(parent);
        parent = m_model->GetParent(parent);
    }

    // then we expand the parents, starting at the root
    while (!parentChain.empty())
    {
         DoExpand(parentChain.back(), false);
         parentChain.pop_back();
    }
}

wxDataViewItem wxDataViewCtrlBase::GetCurrentItem() const
{
    return HasFlag(wxDV_MULTIPLE) ? DoGetCurrentItem()
                                  : GetSelection();
}

void wxDataViewCtrlBase::SetCurrentItem(const wxDataViewItem& item)
{
    wxCHECK_RET( item.IsOk(), "Can't make current an invalid item." );

    if ( HasFlag(wxDV_MULTIPLE) )
        DoSetCurrentItem(item);
    else
        Select(item);
}

wxDataViewItem wxDataViewCtrlBase::GetSelection() const
{
    if ( GetSelectedItemsCount() != 1 )
        return {};

    wxDataViewItemArray selections;
    GetSelections(selections);
    return selections[0];
}

namespace
{

// Helper to account for inconsistent signature of wxDataViewProgressRenderer
// ctor: it takes an extra "label" argument as first parameter, unlike all the
// other renderers.
template <typename Renderer>
struct RendererFactory
{
    static Renderer*
    New(wxDataViewCellMode mode, int align)
    {
        return new Renderer(Renderer::DefaultType, mode, align);
    }
};

template <>
struct RendererFactory<wxDataViewProgressRenderer>
{
    static wxDataViewProgressRenderer*
    New(wxDataViewCellMode mode, int align)
    {
        return new wxDataViewProgressRenderer(
                        "",
                        wxDataViewProgressRenderer::DefaultType,
                        mode,
                        align
                    );
    }
};

template <typename Renderer, typename LabelType>
wxDataViewColumn*
CreateColumnWithRenderer(const LabelType& label,
                         unsigned model_column,
                         wxDataViewCellMode mode,
                         int width,
                         wxAlignment align,
                         unsigned int flags)
{
    // For compatibility reason, handle wxAlignment::None as wxDVR_DEFAULT_ALIGNMENT
    // when creating the renderer here because a lot of existing code,
    // including our own dataview sample, uses wxAlignment::None just because it's
    // the default value of the alignment argument in AppendXXXColumn()
    // methods, but this doesn't mean that it actually wants to top-align the
    // column text.
    //
    // This does make it impossible to create top-aligned text using these
    // functions, but it can always be done by creating the renderer with the
    // desired alignment explicitly and should be so rarely needed in practice
    // (without speaking that vertical alignment is completely unsupported in
    // native OS X version), that it's preferable to do the right thing by
    // default here rather than account for it.
    return new wxDataViewColumn(
                    label,
                    RendererFactory<Renderer>::New(
                        mode,
                        align & wxAlignment::Bottom
                            ? align
                            : align | wxAlignment::CenterVertical
                    ),
                    model_column,
                    width,
                    align,
                    flags
                );
}

// Common implementation of all {Append,Prepend}XXXColumn() below.
template <typename Renderer, typename LabelType>
wxDataViewColumn*
AppendColumnWithRenderer(wxDataViewCtrlBase* dvc,
                         const LabelType& label,
                         unsigned model_column,
                         wxDataViewCellMode mode,
                         int width,
                         wxAlignment align,
                         unsigned int flags)
{
    wxDataViewColumn* const
        col = CreateColumnWithRenderer<Renderer>(
                label, model_column, mode, width, align, flags
            );

    dvc->AppendColumn(col);
    return col;
}

template <typename Renderer, typename LabelType>
wxDataViewColumn*
PrependColumnWithRenderer(wxDataViewCtrlBase* dvc,
                          const LabelType& label,
                          unsigned model_column,
                          wxDataViewCellMode mode,
                          int width,
                          wxAlignment align,
                          unsigned int flags)
{
    wxDataViewColumn* const
        col = CreateColumnWithRenderer<Renderer>(
                label, model_column, mode, width, align, flags
            );

    dvc->PrependColumn(col);
    return col;
}

} // anonymous namespace

wxDataViewColumn *
wxDataViewCtrlBase::AppendTextColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewTextRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendIconTextColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewIconTextRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendToggleColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewToggleRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendProgressColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewProgressRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendDateColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewDateRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendBitmapColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewBitmapRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendTextColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewTextRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendIconTextColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewIconTextRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendToggleColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewToggleRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendProgressColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewProgressRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendDateColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewDateRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::AppendBitmapColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return AppendColumnWithRenderer<wxDataViewBitmapRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependTextColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewTextRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependIconTextColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewIconTextRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependToggleColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewToggleRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependProgressColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewProgressRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependDateColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewDateRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependBitmapColumn( const std::string &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewBitmapRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependTextColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewTextRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependIconTextColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewIconTextRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependToggleColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewToggleRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependProgressColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewProgressRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependDateColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewDateRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

wxDataViewColumn *
wxDataViewCtrlBase::PrependBitmapColumn( const wxBitmap &label, unsigned int model_column,
                            wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    return PrependColumnWithRenderer<wxDataViewBitmapRenderer>(
                this, label, model_column, mode, width, align, flags
            );
}

bool
wxDataViewCtrlBase::AppendColumn( wxDataViewColumn *col )
{
    col->SetOwner( (wxDataViewCtrl*) this );
    return true;
}

bool
wxDataViewCtrlBase::PrependColumn( wxDataViewColumn *col )
{
    col->SetOwner( (wxDataViewCtrl*) this );
    return true;
}

bool
wxDataViewCtrlBase::InsertColumn( unsigned int WXUNUSED(pos), wxDataViewColumn *col )
{
    col->SetOwner( (wxDataViewCtrl*) this );
    return true;
}

// ---------------------------------------------------------
// wxDataViewEvent
// ---------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxDataViewEvent,wxNotifyEvent);

wxDEFINE_EVENT( wxEVT_DATAVIEW_SELECTION_CHANGED, wxDataViewEvent );

wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_ACTIVATED, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_COLLAPSING, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_COLLAPSED, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_EXPANDING, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_EXPANDED, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_EDITING_STARTED, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_START_EDITING, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_EDITING_DONE, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, wxDataViewEvent );

wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEvent );

wxDEFINE_EVENT( wxEVT_DATAVIEW_COLUMN_HEADER_CLICK, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_COLUMN_SORTED, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_COLUMN_REORDERED, wxDataViewEvent );

wxDEFINE_EVENT( wxEVT_DATAVIEW_CACHE_HINT, wxDataViewEvent );

wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE, wxDataViewEvent );
wxDEFINE_EVENT( wxEVT_DATAVIEW_ITEM_DROP, wxDataViewEvent );

// Common part of non-copy ctors.
void wxDataViewEvent::Init(wxDataViewCtrlBase* dvc,
                           wxDataViewColumn* column,
                           const wxDataViewItem& item)
{
    m_item = item;
    m_col = column ? column->GetModelColumn() : -1;
    m_model = dvc ? dvc->GetModel() : nullptr;
    m_column = column;
    m_pos = wxDefaultPosition;
    m_cacheFrom = 0;
    m_cacheTo = 0;
    m_editCancelled = false;
#if wxUSE_DRAG_AND_DROP
    m_dataObject = nullptr;
    m_dataBuffer = nullptr;
    m_dataSize = 0;
    m_dragFlags = 0;
    m_dropEffect = wxDragResult::None;
    m_proposedDropIndex = -1;
#endif // wxUSE_DRAG_AND_DROP

    SetEventObject(dvc);
}

#if wxUSE_SPINCTRL

// -------------------------------------
// wxDataViewSpinRenderer
// -------------------------------------

wxDataViewSpinRenderer::wxDataViewSpinRenderer( int min, int max, wxDataViewCellMode mode, int alignment ) :
   wxDataViewCustomRenderer("long", mode, alignment ),
   m_min(min),
   m_max(max)
{
}

wxWindow* wxDataViewSpinRenderer::CreateEditorCtrl( wxWindow *parent, wxRect labelRect, const wxVariant &value )
{
    long l = value;
    std::string str = fmt::format("{:d}", l);

    wxSpinCtrl *sc = new wxSpinCtrl( parent, wxID_ANY, str,
               labelRect.GetTopLeft(), labelRect.GetSize(), wxSP_ARROW_KEYS|wxTE_PROCESS_ENTER, m_min, m_max, l );
#ifdef __WXMAC__
    const wxSize size = sc->GetSize();
    wxPoint pt = sc->GetPosition();
    sc->SetSize( pt.x - 4, pt.y - 4, size.x, size.y );
#endif

    return sc;
}

bool wxDataViewSpinRenderer::GetValueFromEditorCtrl( wxWindow* editor, wxVariant &value )
{
    wxSpinCtrl *sc = (wxSpinCtrl*) editor;
    long l = sc->GetValue();
    value = l;
    return true;
}

bool wxDataViewSpinRenderer::Render( wxRect rect, wxDC *dc, int state )
{
    std::string str = fmt::format("{:d}", static_cast<int>(m_data) );
    RenderText( str, 0, rect, dc, state );
    return true;
}

wxSize wxDataViewSpinRenderer::GetSize() const
{
    wxSize sz = GetTextExtent(fmt::format("{:d}", m_data));

    // Allow some space for the spin buttons, which is approximately the size
    // of a scrollbar (and getting pixel-exact value would be complicated).
    // Also add some whitespace between the text and the button:
    sz.x += wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, m_editorCtrl);
    sz.x += GetTextExtent("M").x;

    return sz;
}

bool wxDataViewSpinRenderer::SetValue( const wxVariant &value )
{
    m_data = value.GetLong();
    return true;
}

bool wxDataViewSpinRenderer::GetValue( wxVariant &value ) const
{
    value = m_data;
    return true;
}

#if wxUSE_ACCESSIBILITY
std::string wxDataViewSpinRenderer::GetAccessibleDescription() const
{
    return fmt::format("{:li}", m_data);
}
#endif // wxUSE_ACCESSIBILITY

#endif // wxUSE_SPINCTRL

// -------------------------------------
// wxDataViewChoiceRenderer
// -------------------------------------

#if defined(wxHAS_GENERIC_DATAVIEWCTRL)

wxDataViewChoiceRenderer::wxDataViewChoiceRenderer( const std::vector<std::string>& choices, wxDataViewCellMode mode, int alignment ) :
   wxDataViewCustomRenderer("string", mode, alignment ),
   m_choices(choices)
{
}

wxWindow* wxDataViewChoiceRenderer::CreateEditorCtrl( wxWindow *parent, wxRect labelRect, const wxVariant &value )
{
    wxChoice* c = new wxChoice
                      (
                          parent,
                          wxID_ANY,
                          labelRect.GetTopLeft(),
                          wxSize(labelRect.GetWidth(), -1),
                          m_choices
                      );
    c->Move(wxPoint{labelRect.GetRight() - c->GetRect().width, wxDefaultCoord});
    c->SetStringSelection( value.GetString() );
    return c;
}

bool wxDataViewChoiceRenderer::GetValueFromEditorCtrl( wxWindow* editor, wxVariant &value )
{
    wxChoice *c = (wxChoice*) editor;
    std::string s = c->GetStringSelection();
    value = s;
    return true;
}

bool wxDataViewChoiceRenderer::Render( wxRect rect, wxDC *dc, int state )
{
    RenderText( m_data, 0, rect, dc, state );
    return true;
}

wxSize wxDataViewChoiceRenderer::GetSize() const
{
    wxSize sz;

    for ( const auto& choice : m_choices )
        sz.IncTo(GetTextExtent(choice));

    // Allow some space for the right-side button, which is approximately the
    // size of a scrollbar (and getting pixel-exact value would be complicated).
    // Also add some whitespace between the text and the button:
    sz.x += wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, m_editorCtrl);
    sz.x += GetTextExtent("M").x;

    return sz;
}

bool wxDataViewChoiceRenderer::SetValue( const wxVariant &value )
{
    m_data = value.GetString();
    return true;
}

bool wxDataViewChoiceRenderer::GetValue( wxVariant &value ) const
{
    value = m_data;
    return true;
}

#if wxUSE_ACCESSIBILITY
std::string wxDataViewChoiceRenderer::GetAccessibleDescription() const
{
    return m_data;
}
#endif // wxUSE_ACCESSIBILITY

// ----------------------------------------------------------------------------
// wxDataViewChoiceByIndexRenderer
// ----------------------------------------------------------------------------

wxDataViewChoiceByIndexRenderer::wxDataViewChoiceByIndexRenderer( const std::vector<std::string>& choices,
                                  wxDataViewCellMode mode, int alignment ) :
      wxDataViewChoiceRenderer( choices, mode, alignment )
{
    m_variantType = "long";
}

wxWindow* wxDataViewChoiceByIndexRenderer::CreateEditorCtrl( wxWindow *parent, wxRect labelRect, const wxVariant &value )
{
    wxVariant string_value = GetChoice( value.GetLong() );

    return wxDataViewChoiceRenderer::CreateEditorCtrl( parent, labelRect, string_value );
}

bool wxDataViewChoiceByIndexRenderer::GetValueFromEditorCtrl( wxWindow* editor, wxVariant &value )
{
    // FIXME: Simplify this.
    wxVariant string_value;
    if (!wxDataViewChoiceRenderer::GetValueFromEditorCtrl( editor, string_value ))
        return false;
    // TODO: Verify this.
    const auto iter_idx = std::find_if(GetChoices().cbegin(), GetChoices().cend(),
        [string_value](const auto& choice) { return string_value.GetString().IsSameAs(choice); });

    value = gsl::narrow_cast<long>(std::distance(std::cbegin(GetChoices()), iter_idx));
    return true;
}

bool wxDataViewChoiceByIndexRenderer::SetValue( const wxVariant &value )
{
    wxVariant string_value = GetChoice( value.GetLong() );
    return wxDataViewChoiceRenderer::SetValue( string_value );
}

bool wxDataViewChoiceByIndexRenderer::GetValue( wxVariant &value ) const
{
    // FIXME: Simplify this.
    wxVariant string_value;
    if (!wxDataViewChoiceRenderer::GetValue( string_value ))
        return false;

    const auto iter_idx = std::find_if(GetChoices().cbegin(), GetChoices().cend(),
        [string_value](const auto& choice) { return string_value.GetString().IsSameAs(choice); });
    // TODO: Verify this.
    value = gsl::narrow_cast<long>(std::distance(std::cbegin(GetChoices()), iter_idx));
    return true;
}

#if wxUSE_ACCESSIBILITY
std::string wxDataViewChoiceByIndexRenderer::GetAccessibleDescription() const
{
    wxVariant strVal;
    if ( wxDataViewChoiceRenderer::GetValue(strVal) )
        return strVal;
    const auto iter_idx = std::find_if(GetChoices().cbegin(), GetChoices().cend(),
        [strVal](const auto& choice) { return strVal.GetString().IsSameAs(choice); });

    return fmt::format("{:li}", static_cast<long>(std::distance(std::cbegin(GetChoices()), iter_idx)));
}
#endif // wxUSE_ACCESSIBILITY

#endif // wxHAS_GENERIC_DATAVIEWCTRL

// ---------------------------------------------------------
// wxDataViewDateRenderer
// ---------------------------------------------------------

#if (defined(wxHAS_GENERIC_DATAVIEWCTRL) || defined(__WXGTK__)) && wxUSE_DATEPICKCTRL

wxDataViewDateRenderer::wxDataViewDateRenderer(const std::string& varianttype,
                                              wxDataViewCellMode mode, int align)
    : wxDataViewCustomRenderer(varianttype, mode, align)
{
}

wxWindow *
wxDataViewDateRenderer::CreateEditorCtrl(wxWindow *parent, wxRect labelRect, const wxVariant& value)
{
    return new wxDatePickerCtrl
               (
                   parent,
                   wxID_ANY,
                   value.GetDateTime(),
                   labelRect.GetTopLeft(),
                   labelRect.GetSize()
               );
}

bool wxDataViewDateRenderer::GetValueFromEditorCtrl(wxWindow *editor, wxVariant& value)
{
    auto* ctrl = dynamic_cast<wxDatePickerCtrl*>(editor);
    value = ctrl->GetValue();
    return true;
}

bool wxDataViewDateRenderer::SetValue(const wxVariant& value)
{
    m_date = value.GetDateTime();
    return true;
}

bool wxDataViewDateRenderer::GetValue(wxVariant& value) const
{
    value = m_date;
    return true;
}

#if wxUSE_ACCESSIBILITY
std::string wxDataViewDateRenderer::GetAccessibleDescription() const
{
    return m_date.FormatDate();
}
#endif // wxUSE_ACCESSIBILITY

bool wxDataViewDateRenderer::Render(wxRect cell, wxDC* dc, int state)
{
    std::string tmp = m_date.FormatDate();
    RenderText( tmp, 0, cell, dc, state );
    return true;
}

wxSize wxDataViewDateRenderer::GetSize() const
{
    return GetTextExtent(m_date.FormatDate());
}

#endif // (defined(wxHAS_GENERIC_DATAVIEWCTRL) || defined(__WXGTK__)) && wxUSE_DATEPICKCTRL

// ----------------------------------------------------------------------------
// wxDataViewCheckIconTextRenderer implementation
// ----------------------------------------------------------------------------

#if defined(wxHAS_GENERIC_DATAVIEWCTRL) || !defined(__WXOSX__)

IMPLEMENT_VARIANT_OBJECT_EXPORTED(wxDataViewCheckIconText, WXDLLIMPEXP_CORE)

wxIMPLEMENT_CLASS(wxDataViewCheckIconText, wxDataViewIconText);

wxIMPLEMENT_CLASS(wxDataViewCheckIconTextRenderer, wxDataViewRenderer);

wxDataViewCheckIconTextRenderer::wxDataViewCheckIconTextRenderer
                                 (
                                      wxDataViewCellMode mode,
                                      int align
                                 )
    : wxDataViewCustomRenderer(GetDefaultType(), mode, align)
{
}

void wxDataViewCheckIconTextRenderer::Allow3rdStateForUser(bool allow)
{
    m_allow3rdStateForUser = allow;
}

bool wxDataViewCheckIconTextRenderer::SetValue(const wxVariant& value)
{
    m_value << value;
    return true;
}

bool wxDataViewCheckIconTextRenderer::GetValue(wxVariant& value) const
{
    value << m_value;
    return true;
}

#if wxUSE_ACCESSIBILITY
std::string wxDataViewCheckIconTextRenderer::GetAccessibleDescription() const
{
    std::string text = m_value.GetText();
    if ( !text.empty() )
    {
        text += " ";
    }

    switch ( m_value.GetCheckedState() )
    {
        case wxCHK_CHECKED:
            /* TRANSLATORS: Checkbox state name */
            text += _("checked");
            break;
        case wxCHK_UNCHECKED:
            /* TRANSLATORS: Checkbox state name */
            text += _("unchecked");
            break;
        case wxCHK_UNDETERMINED:
            /* TRANSLATORS: Checkbox state name */
            text += _("undetermined");
            break;
    }

    return text;
}
#endif // wxUSE_ACCESSIBILITY

wxSize wxDataViewCheckIconTextRenderer::GetSize() const
{
    wxSize size = GetCheckSize();
    size.x += MARGIN_CHECK_ICON;

    if ( m_value.GetIcon().IsOk() )
    {
#ifdef __WXGTK3__
        const wxSize sizeIcon = m_value.GetIcon().GetScaledSize();
#else
        const wxSize sizeIcon = m_value.GetIcon().GetSize();
#endif
        if ( sizeIcon.y > size.y )
            size.y = sizeIcon.y;

        size.x += sizeIcon.x + MARGIN_ICON_TEXT;
    }

    std::string text = m_value.GetText();
    if ( text.empty() )
        text = "Dummy";

    const wxSize sizeText = GetTextExtent(text);
    if ( sizeText.y > size.y )
        size.y = sizeText.y;

    size.x += sizeText.x;

    return size;
}

bool wxDataViewCheckIconTextRenderer::Render(wxRect cell, wxDC* dc, int state)
{
    // Draw the checkbox first.
    int renderFlags = 0;
    switch ( m_value.GetCheckedState() )
    {
        case wxCHK_UNCHECKED:
            break;

        case wxCHK_CHECKED:
            renderFlags |= wxCONTROL_CHECKED;
            break;

        case wxCHK_UNDETERMINED:
            renderFlags |= wxCONTROL_UNDETERMINED;
            break;
    }

    if ( state & wxDATAVIEW_CELL_PRELIT )
        renderFlags |= wxCONTROL_CURRENT;

    const wxSize sizeCheck = GetCheckSize();

    wxRect rectCheck(cell.GetPosition(), sizeCheck);
    rectCheck = rectCheck.CentreIn(cell, wxVERTICAL);

    wxRendererNative::Get().DrawCheckBox
                            (
                                GetView(), *dc, rectCheck, renderFlags
                            );

    // Then the icon, if any.
    int xoffset = sizeCheck.x + MARGIN_CHECK_ICON;

    const wxIcon& icon = m_value.GetIcon();
    if ( icon.IsOk() )
    {
#ifdef __WXGTK3__
        const wxSize sizeIcon = icon.GetScaledSize();
#else
        const wxSize sizeIcon = icon.GetSize();
#endif
        wxRect rectIcon(cell.GetPosition(), sizeIcon);
        rectIcon.x += xoffset;
        rectIcon = rectIcon.CentreIn(cell, wxVERTICAL);

        dc->DrawIcon(icon, rectIcon.GetPosition());

        xoffset += sizeIcon.x + MARGIN_ICON_TEXT;
    }

    // Finally the text.
    RenderText(m_value.GetText(), xoffset, cell, dc, state);

    return true;
}

bool
wxDataViewCheckIconTextRenderer::ActivateCell(const wxRect& WXUNUSED(cell),
                                              wxDataViewModel *model,
                                              const wxDataViewItem & item,
                                              unsigned int col,
                                              const wxMouseEvent *mouseEvent)
{
    if ( mouseEvent )
    {
        if ( !wxRect(GetCheckSize()).Contains(mouseEvent->GetPosition()) )
            return false;
    }

    // If the 3rd state is user-settable then the cycle is
    // unchecked->checked->undetermined.
    wxCheckBoxState checkedState = m_value.GetCheckedState();
    switch ( checkedState )
    {
        case wxCHK_CHECKED:
            checkedState = m_allow3rdStateForUser ? wxCHK_UNDETERMINED
                                                  : wxCHK_UNCHECKED;
            break;

        case wxCHK_UNDETERMINED:
            // Whether 3rd state is user-settable or not, the next state is
            // unchecked.
            checkedState = wxCHK_UNCHECKED;
            break;

        case wxCHK_UNCHECKED:
            checkedState = wxCHK_CHECKED;
            break;
    }

    m_value.SetCheckedState(checkedState);

    wxVariant value;
    value << m_value;

    model->ChangeValue(value, item, col);
    return true;
}

wxSize wxDataViewCheckIconTextRenderer::GetCheckSize() const
{
    return wxRendererNative::Get().GetCheckBoxSize(GetView());
}

#endif // ! native __WXOSX__

//-----------------------------------------------------------------------------
// wxDataViewListStore
//-----------------------------------------------------------------------------

wxDataViewListStore::~wxDataViewListStore()
{
    for (auto* line : m_data)
    {
        delete line;
    }
}

void wxDataViewListStore::PrependColumn( const std::string &varianttype )
{
    m_cols.insert( std::cbegin(m_cols), varianttype);
}

void wxDataViewListStore::InsertColumn( unsigned int pos, const std::string &varianttype )
{
    m_cols.insert( std::cbegin(m_cols) + pos, varianttype );
}

void wxDataViewListStore::AppendColumn( const std::string &varianttype )
{
    m_cols.push_back( varianttype );
}

unsigned int wxDataViewListStore::GetColumnCount() const
{
    return m_cols.size();
}

unsigned int wxDataViewListStore::GetItemCount() const
{
    return m_data.size();
}

std::string wxDataViewListStore::GetColumnType( unsigned int pos ) const
{
    // FIXME: Hazardous with [] operator. Should return iterator instead.
    return m_cols.at(pos);
}

void wxDataViewListStore::AppendItem( const std::vector<wxVariant> &values, wxUIntPtr data )
{
    wxCHECK_RET( values.size() == GetColumnCount(), "wrong number of values" );
    wxDataViewListStoreLine *line = new wxDataViewListStoreLine( data );
    line->m_values = values;
    m_data.push_back( line );

    RowAppended();
}

void wxDataViewListStore::PrependItem( const std::vector<wxVariant> &values, wxUIntPtr data )
{
    wxCHECK_RET( values.size() == GetColumnCount(), "wrong number of values" );
    wxDataViewListStoreLine *line = new wxDataViewListStoreLine( data );
    line->m_values = values;
    m_data.insert( m_data.begin(), line );

    RowPrepended();
}

void wxDataViewListStore::InsertItem(  unsigned int row, const std::vector<wxVariant> &values,
                                       wxUIntPtr data )
{
    wxCHECK_RET( values.size() == GetColumnCount(), "wrong number of values" );
    wxDataViewListStoreLine *line = new wxDataViewListStoreLine( data );
    line->m_values = values;
    m_data.insert( m_data.begin()+row, line );

    RowInserted( row );
}

void wxDataViewListStore::DeleteItem( unsigned int row )
{
    std::vector<wxDataViewListStoreLine*>::iterator it = m_data.begin() + row;
    delete *it;
    m_data.erase( it );

    RowDeleted( row );
}

void wxDataViewListStore::DeleteAllItems()
{
    std::vector<wxDataViewListStoreLine*>::iterator it;
    for (it = m_data.begin(); it != m_data.end(); ++it)
    {
        wxDataViewListStoreLine* line = *it;
        delete line;
    }

    m_data.clear();

    Reset( 0 );
}

void wxDataViewListStore::ClearColumns()
{
    m_cols.clear();
}

void wxDataViewListStore::SetItemData( const wxDataViewItem& item, wxUIntPtr data )
{
    wxDataViewListStoreLine* line = m_data[GetRow(item)];
    if (!line) return;

    line->SetData( data );
}

wxUIntPtr wxDataViewListStore::GetItemData( const wxDataViewItem& item ) const
{
    wxDataViewListStoreLine* line = m_data[GetRow(item)];
    if (!line) return 0;

    return line->GetData();
}

void wxDataViewListStore::GetValueByRow( wxVariant &value, unsigned int row, unsigned int col ) const
{
    wxDataViewListStoreLine *line = m_data[row];
    value = line->m_values[col];
}

bool wxDataViewListStore::SetValueByRow( const wxVariant &value, unsigned int row, unsigned int col )
{
    wxDataViewListStoreLine *line = m_data[row];
    line->m_values[col] = value;

    return true;
}

//-----------------------------------------------------------------------------
// wxDataViewListCtrl
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxDataViewListCtrl,wxDataViewCtrl);

wxBEGIN_EVENT_TABLE(wxDataViewListCtrl,wxDataViewCtrl)
   EVT_SIZE( wxDataViewListCtrl::OnSize )
wxEND_EVENT_TABLE()

wxDataViewListCtrl::wxDataViewListCtrl( wxWindow *parent, wxWindowID id,
           const wxPoint& pos, const wxSize& size, unsigned int style,
           const wxValidator& validator )
{
    Create( parent, id, pos, size, style, validator );
}

bool wxDataViewListCtrl::Create( wxWindow *parent, wxWindowID id,
           const wxPoint& pos, const wxSize& size, unsigned int style,
           const wxValidator& validator )
{
    if ( !wxDataViewCtrl::Create( parent, id, pos, size, style, validator ) )
        return false;

    wxDataViewListStore *store = new wxDataViewListStore;
    AssociateModel( store );
    store->DecRef();

    return true;
}

bool wxDataViewListCtrl::AppendColumn( wxDataViewColumn *column, const std::string &varianttype )
{
    GetStore()->AppendColumn( varianttype );
    return wxDataViewCtrl::AppendColumn( column );
}

bool wxDataViewListCtrl::PrependColumn( wxDataViewColumn *column, const std::string &varianttype )
{
    GetStore()->PrependColumn( varianttype );
    return wxDataViewCtrl::PrependColumn( column );
}

bool wxDataViewListCtrl::InsertColumn( unsigned int pos, wxDataViewColumn *column, const std::string &varianttype )
{
    GetStore()->InsertColumn( pos, varianttype );
    return wxDataViewCtrl::InsertColumn( pos, column );
}

bool wxDataViewListCtrl::PrependColumn( wxDataViewColumn *col )
{
    return PrependColumn( col, col->GetRenderer()->GetVariantType() );
}

bool wxDataViewListCtrl::InsertColumn( unsigned int pos, wxDataViewColumn *col )
{
    return InsertColumn( pos, col, col->GetRenderer()->GetVariantType() );
}

bool wxDataViewListCtrl::AppendColumn( wxDataViewColumn *col )
{
    return AppendColumn( col, col->GetRenderer()->GetVariantType() );
}

bool wxDataViewListCtrl::ClearColumns()
{
    GetStore()->ClearColumns();
    return wxDataViewCtrl::ClearColumns();
}

wxDataViewColumn *wxDataViewListCtrl::AppendTextColumn( const std::string &label,
          wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    GetStore()->AppendColumn( "string" );

    wxDataViewColumn *ret = new wxDataViewColumn( label,
        new wxDataViewTextRenderer( "string", mode ),
        GetStore()->GetColumnCount()-1, width, align, flags );

    wxDataViewCtrl::AppendColumn( ret );

    return ret;
}

wxDataViewColumn *wxDataViewListCtrl::AppendToggleColumn( const std::string &label,
          wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    GetStore()->AppendColumn( "bool" );

    wxDataViewColumn *ret = new wxDataViewColumn( label,
        new wxDataViewToggleRenderer( "bool", mode ),
        GetStore()->GetColumnCount()-1, width, align, flags );

    return wxDataViewCtrl::AppendColumn( ret ) ? ret : nullptr;
}

wxDataViewColumn *wxDataViewListCtrl::AppendProgressColumn( const std::string &label,
          wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    GetStore()->AppendColumn( "long" );

    wxDataViewColumn *ret = new wxDataViewColumn( label,
        new wxDataViewProgressRenderer( "", "long", mode ),
        GetStore()->GetColumnCount()-1, width, align, flags );

    return wxDataViewCtrl::AppendColumn( ret ) ? ret : nullptr;
}

wxDataViewColumn *wxDataViewListCtrl::AppendIconTextColumn( const std::string &label,
          wxDataViewCellMode mode, int width, wxAlignment align, unsigned int flags )
{
    GetStore()->AppendColumn( "wxDataViewIconText" );

    wxDataViewColumn *ret = new wxDataViewColumn( label,
        new wxDataViewIconTextRenderer( "wxDataViewIconText", mode ),
        GetStore()->GetColumnCount()-1, width, align, flags );

    return wxDataViewCtrl::AppendColumn( ret ) ? ret : nullptr;
}

void wxDataViewListCtrl::OnSize( wxSizeEvent &event )
{
    event.Skip( true );
}

//-----------------------------------------------------------------------------
// wxDataViewTreeStore
//-----------------------------------------------------------------------------

wxDataViewTreeStoreNode::wxDataViewTreeStoreNode(
        wxDataViewTreeStoreNode *parent,
        const std::string &text, const wxIcon &icon, wxClientData *data )
    : m_text(text)
    , m_icon(icon)
    , m_parent(parent)
    , m_data(data)
{
}

wxDataViewTreeStoreNode::~wxDataViewTreeStoreNode()
{
    delete m_data;
}

wxDataViewTreeStoreContainerNode::wxDataViewTreeStoreContainerNode(
        wxDataViewTreeStoreNode *parent, const std::string &text,
        const wxIcon &icon, const wxIcon &expanded, wxClientData *data )
    : wxDataViewTreeStoreNode( parent, text, icon, data )
    , m_iconExpanded(expanded)
{
}

wxDataViewTreeStoreContainerNode::~wxDataViewTreeStoreContainerNode()
{
    DestroyChildren();
}

wxDataViewTreeStoreNodes::iterator
wxDataViewTreeStoreContainerNode::FindChild(wxDataViewTreeStoreNode* node)
{
    wxDataViewTreeStoreNodes::iterator iter;
    for (iter = m_children.begin(); iter != m_children.end(); ++iter)
    {
        if ( *iter == node )
            break;
    }

    return iter;
}

void wxDataViewTreeStoreContainerNode::DestroyChildren()
{
    wxDataViewTreeStoreNodes::const_iterator iter;
    for (iter = m_children.begin(); iter != m_children.end(); ++iter)
    {
        delete *iter;
    }

    m_children.clear();
}

//-----------------------------------------------------------------------------

wxDataViewTreeStore::wxDataViewTreeStore()
{
    m_root = new wxDataViewTreeStoreContainerNode( nullptr, "" );
}

wxDataViewTreeStore::~wxDataViewTreeStore()
{
    delete m_root;
}

wxDataViewItem wxDataViewTreeStore::AppendItem( const wxDataViewItem& parent,
        const std::string &text, const wxIcon &icon, wxClientData *data )
{
    wxDataViewTreeStoreContainerNode *parent_node = FindContainerNode( parent );
    if (!parent_node) return wxDataViewItem(nullptr);

    wxDataViewTreeStoreNode *node =
        new wxDataViewTreeStoreNode( parent_node, text, icon, data );
    parent_node->GetChildren().push_back( node );

    return node->GetItem();
}

wxDataViewItem wxDataViewTreeStore::PrependItem( const wxDataViewItem& parent,
        const std::string &text, const wxIcon &icon, wxClientData *data )
{
    wxDataViewTreeStoreContainerNode *parent_node = FindContainerNode( parent );
    if (!parent_node) return wxDataViewItem(nullptr);

    wxDataViewTreeStoreNode *node =
        new wxDataViewTreeStoreNode( parent_node, text, icon, data );
    wxDataViewTreeStoreNodes& children = parent_node->GetChildren();
    children.insert(children.begin(), node);

    return node->GetItem();
}

wxDataViewItem
wxDataViewTreeStore::InsertItem(const wxDataViewItem& parent,
                                const wxDataViewItem& previous,
                                const std::string& text,
                                const wxIcon& icon,
                                wxClientData *data)
{
    wxDataViewTreeStoreContainerNode *parent_node = FindContainerNode( parent );
    if (!parent_node) return wxDataViewItem(nullptr);

    wxDataViewTreeStoreNode *previous_node = FindNode( previous );
    wxDataViewTreeStoreNodes& children = parent_node->GetChildren();
    const wxDataViewTreeStoreNodes::iterator iter = parent_node->FindChild( previous_node );
    if (iter == children.end()) return wxDataViewItem(nullptr);

    wxDataViewTreeStoreNode *node =
        new wxDataViewTreeStoreNode( parent_node, text, icon, data );
    children.insert(iter, node);

    return node->GetItem();
}

wxDataViewItem wxDataViewTreeStore::PrependContainer( const wxDataViewItem& parent,
        const std::string &text, const wxIcon &icon, const wxIcon &expanded,
        wxClientData *data )
{
    wxDataViewTreeStoreContainerNode *parent_node = FindContainerNode( parent );
    if (!parent_node) return wxDataViewItem(nullptr);

    wxDataViewTreeStoreContainerNode *node =
        new wxDataViewTreeStoreContainerNode( parent_node, text, icon, expanded, data );
    wxDataViewTreeStoreNodes& children = parent_node->GetChildren();
    children.insert(children.begin(), node);

    return node->GetItem();
}

wxDataViewItem
wxDataViewTreeStore::AppendContainer(const wxDataViewItem& parent,
                                     const std::string &text,
                                     const wxIcon& icon,
                                     const wxIcon& expanded,
                                     wxClientData * data)
{
    wxDataViewTreeStoreContainerNode *parent_node = FindContainerNode( parent );
    if (!parent_node) return wxDataViewItem(nullptr);

    wxDataViewTreeStoreContainerNode *node =
        new wxDataViewTreeStoreContainerNode( parent_node, text, icon, expanded, data );
    parent_node->GetChildren().push_back( node );

    return node->GetItem();
}

wxDataViewItem
wxDataViewTreeStore::InsertContainer(const wxDataViewItem& parent,
                                     const wxDataViewItem& previous,
                                     const std::string& text,
                                     const wxIcon& icon,
                                     const wxIcon& expanded,
                                     wxClientData * data)
{
    wxDataViewTreeStoreContainerNode *parent_node = FindContainerNode( parent );
    if (!parent_node) return wxDataViewItem(nullptr);

    wxDataViewTreeStoreNode *previous_node = FindNode( previous );
    wxDataViewTreeStoreNodes& children = parent_node->GetChildren();
    const wxDataViewTreeStoreNodes::iterator iter = parent_node->FindChild( previous_node );
    if (iter == children.end()) return wxDataViewItem(nullptr);

    wxDataViewTreeStoreContainerNode *node =
        new wxDataViewTreeStoreContainerNode( parent_node, text, icon, expanded, data );
    children.insert(iter, node);

    return node->GetItem();
}

bool wxDataViewTreeStore::IsContainer( const wxDataViewItem& item ) const
{
    wxDataViewTreeStoreNode *node = FindNode( item );
    if (!node) return false;

    return node->IsContainer();
}

wxDataViewItem wxDataViewTreeStore::GetNthChild( const wxDataViewItem& parent, unsigned int pos ) const
{
    wxDataViewTreeStoreContainerNode *parent_node = FindContainerNode( parent );
    if (!parent_node) return wxDataViewItem(nullptr);

    wxDataViewTreeStoreNode* const node = parent_node->GetChildren()[pos];
    if (node)
        return node->GetItem();

    return wxDataViewItem(nullptr);
}

int wxDataViewTreeStore::GetChildCount( const wxDataViewItem& parent ) const
{
    wxDataViewTreeStoreNode *node = FindNode( parent );
    if (!node) return -1;

    if (!node->IsContainer())
        return 0;

    wxDataViewTreeStoreContainerNode *container_node = (wxDataViewTreeStoreContainerNode*) node;
    return (int) container_node->GetChildren().size();
}

void wxDataViewTreeStore::SetItemText( const wxDataViewItem& item, const std::string &text )
{
    wxDataViewTreeStoreNode *node = FindNode( item );
    if (!node) return;

    node->SetText( text );
}

std::string wxDataViewTreeStore::GetItemText( const wxDataViewItem& item ) const
{
    wxDataViewTreeStoreNode *node = FindNode( item );
    if (!node) return "";

    return node->GetText();
}

void wxDataViewTreeStore::SetItemIcon( const wxDataViewItem& item, const wxIcon &icon )
{
    wxDataViewTreeStoreNode *node = FindNode( item );
    if (!node) return;

    node->SetIcon( icon );
}

const wxIcon &wxDataViewTreeStore::GetItemIcon( const wxDataViewItem& item ) const
{
    wxDataViewTreeStoreNode *node = FindNode( item );
    if (!node) return wxNullIcon;

    return node->GetIcon();
}

void wxDataViewTreeStore::SetItemExpandedIcon( const wxDataViewItem& item, const wxIcon &icon )
{
    wxDataViewTreeStoreContainerNode *node = FindContainerNode( item );
    if (!node) return;

    node->SetExpandedIcon( icon );
}

const wxIcon &wxDataViewTreeStore::GetItemExpandedIcon( const wxDataViewItem& item ) const
{
    wxDataViewTreeStoreContainerNode *node = FindContainerNode( item );
    if (!node) return wxNullIcon;

    return node->GetExpandedIcon();
}

void wxDataViewTreeStore::SetItemData( const wxDataViewItem& item, wxClientData *data )
{
    wxDataViewTreeStoreNode *node = FindNode( item );
    if (!node) return;

    node->SetData( data );
}

wxClientData *wxDataViewTreeStore::GetItemData( const wxDataViewItem& item ) const
{
    wxDataViewTreeStoreNode *node = FindNode( item );
    if (!node) return nullptr;

    return node->GetData();
}

void wxDataViewTreeStore::DeleteItem( const wxDataViewItem& item )
{
    if (!item.IsOk()) return;

    wxDataViewItem parent_item = GetParent( item );

    wxDataViewTreeStoreContainerNode *parent_node = FindContainerNode( parent_item );
    if (!parent_node) return;

    const wxDataViewTreeStoreNodes::iterator
        iter = parent_node->FindChild(FindNode(item));
    if ( iter != parent_node->GetChildren().end() )
    {
        delete *iter;
        parent_node->GetChildren().erase(iter);
    }
}

void wxDataViewTreeStore::DeleteChildren( const wxDataViewItem& item )
{
    wxDataViewTreeStoreContainerNode *node = FindContainerNode( item );
    if (!node) return;

    node->DestroyChildren();
}

void wxDataViewTreeStore::DeleteAllItems()
{
    DeleteChildren(wxDataViewItem(m_root));
}

void
wxDataViewTreeStore::GetValue(wxVariant &variant,
                              const wxDataViewItem &item,
                              unsigned int WXUNUSED(col)) const
{
    // if (col != 0) return;

    wxDataViewTreeStoreNode *node = FindNode( item );
    if (!node) return;

    wxIcon icon( node->GetIcon());
    if (node->IsContainer())
    {
        wxDataViewTreeStoreContainerNode *container = (wxDataViewTreeStoreContainerNode*) node;
        if (container->IsExpanded() && container->GetExpandedIcon().IsOk())
           icon = container->GetExpandedIcon();
    }

    wxDataViewIconText data( node->GetText(), icon );

    variant << data;
}

bool
wxDataViewTreeStore::SetValue(const wxVariant& variant,
                              const wxDataViewItem& item,
                              unsigned int WXUNUSED(col))
{
    // if (col != 0) return false;

    wxDataViewTreeStoreNode *node = FindNode( item );
    if (!node) return false;

    wxDataViewIconText data;

    data << variant;

    node->SetText( data.GetText() );
    node->SetIcon( data.GetIcon() );

    return true;
}

wxDataViewItem wxDataViewTreeStore::GetParent( const wxDataViewItem &item ) const
{
    wxDataViewTreeStoreNode *node = FindNode( item );
    if (!node) return wxDataViewItem(nullptr);

    wxDataViewTreeStoreNode *parent = node->GetParent();
    if (!parent) return wxDataViewItem(nullptr);

    if (parent == m_root)
        return wxDataViewItem(nullptr);

    return parent->GetItem();
}

unsigned int wxDataViewTreeStore::GetChildren( const wxDataViewItem &item, wxDataViewItemArray &children ) const
{
    wxDataViewTreeStoreContainerNode *node = FindContainerNode( item );
    if (!node) return 0;

    for (const auto* child : node->GetChildren())
    {
        children.push_back( child->GetItem() );
    }

    return node->GetChildren().size();
}

int wxDataViewTreeStore::Compare( const wxDataViewItem &item1, const wxDataViewItem &item2,
                         unsigned int WXUNUSED(column), bool WXUNUSED(ascending) ) const
{
    wxDataViewTreeStoreNode *node1 = FindNode( item1 );
    wxDataViewTreeStoreNode *node2 = FindNode( item2 );

    if (!node1 || !node2 || (node1 == node2))
        return 0;

    wxDataViewTreeStoreContainerNode* const parent =
        (wxDataViewTreeStoreContainerNode*) node1->GetParent();

    wxCHECK_MSG( node2->GetParent() == parent, 0,
                 wxS("Comparing items with different parent.") );

    if (node1->IsContainer() && !node2->IsContainer())
        return -1;

    if (node2->IsContainer() && !node1->IsContainer())
        return 1;

    for (const auto* child : parent->GetChildren())
    {
        if ( child == node1 )
            return -1;

        if ( child == node2 )
            return 1;
    }

    wxFAIL_MSG(wxS("Unreachable"));
    return 0;
}

wxDataViewTreeStoreNode *wxDataViewTreeStore::FindNode( const wxDataViewItem &item ) const
{
    if (!item.IsOk())
        return m_root;

    return (wxDataViewTreeStoreNode*) item.GetID();
}

wxDataViewTreeStoreContainerNode *wxDataViewTreeStore::FindContainerNode( const wxDataViewItem &item ) const
{
    if (!item.IsOk())
        return (wxDataViewTreeStoreContainerNode*) m_root;

    wxDataViewTreeStoreNode* node = (wxDataViewTreeStoreNode*) item.GetID();

    if (!node->IsContainer())
        return nullptr;

    return (wxDataViewTreeStoreContainerNode*) node;
}

//-----------------------------------------------------------------------------
// wxDataViewTreeCtrl
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxDataViewTreeCtrl,wxDataViewCtrl);

wxBEGIN_EVENT_TABLE(wxDataViewTreeCtrl,wxDataViewCtrl)
   EVT_DATAVIEW_ITEM_EXPANDED(-1, wxDataViewTreeCtrl::OnExpanded)
   EVT_DATAVIEW_ITEM_COLLAPSED(-1, wxDataViewTreeCtrl::OnCollapsed)
   EVT_SIZE( wxDataViewTreeCtrl::OnSize )
wxEND_EVENT_TABLE()

bool wxDataViewTreeCtrl::Create( wxWindow *parent, wxWindowID id,
           const wxPoint& pos, const wxSize& size, unsigned int style, const wxValidator& validator )
{
    if ( !wxDataViewCtrl::Create( parent, id, pos, size, style, validator ) )
        return false;

    // create the standard model and a column in the tree
    wxDataViewTreeStore *store = new wxDataViewTreeStore;
    AssociateModel( store );
    store->DecRef();

    AppendIconTextColumn
    (
        "",                 // no label (header is not shown anyhow)
        0,                          // the only model column
        wxDATAVIEW_CELL_EDITABLE,
        -1,                         // default width
        wxAlignment::None,                //  and alignment
        0                           // not resizable
    );

    return true;
}

wxDataViewItem wxDataViewTreeCtrl::AppendItem( const wxDataViewItem& parent,
        const std::string &text, int iconIndex, wxClientData *data )
{
    wxDataViewItem res = GetStore()->
        AppendItem( parent, text, GetImage(iconIndex), data );

    GetStore()->ItemAdded( parent, res );

    return res;
}

wxDataViewItem wxDataViewTreeCtrl::PrependItem( const wxDataViewItem& parent,
        const std::string &text, int iconIndex, wxClientData *data )
{
    wxDataViewItem res = GetStore()->
        PrependItem( parent, text, GetImage(iconIndex), data );

    GetStore()->ItemAdded( parent, res );

    return res;
}

wxDataViewItem wxDataViewTreeCtrl::InsertItem( const wxDataViewItem& parent, const wxDataViewItem& previous,
        const std::string &text, int iconIndex, wxClientData *data )
{
    wxDataViewItem res = GetStore()->
        InsertItem( parent, previous, text, GetImage(iconIndex), data );

    GetStore()->ItemAdded( parent, res );

    return res;
}

wxDataViewItem wxDataViewTreeCtrl::PrependContainer( const wxDataViewItem& parent,
        const std::string &text, int iconIndex, int expandedIndex, wxClientData *data )
{
    wxDataViewItem res = GetStore()->
        PrependContainer( parent, text,
                          GetImage(iconIndex), GetImage(expandedIndex), data );

    GetStore()->ItemAdded( parent, res );

    return res;
}

wxDataViewItem wxDataViewTreeCtrl::AppendContainer( const wxDataViewItem& parent,
        const std::string &text, int iconIndex, int expandedIndex, wxClientData *data )
{
    wxDataViewItem res = GetStore()->
        AppendContainer( parent, text,
                         GetImage(iconIndex), GetImage(expandedIndex), data );

    GetStore()->ItemAdded( parent, res );

    return res;
}

wxDataViewItem wxDataViewTreeCtrl::InsertContainer( const wxDataViewItem& parent, const wxDataViewItem& previous,
        const std::string &text, int iconIndex, int expandedIndex, wxClientData *data )
{
    wxDataViewItem res = GetStore()->
        InsertContainer( parent, previous, text,
                         GetImage(iconIndex), GetImage(expandedIndex), data );

    GetStore()->ItemAdded( parent, res );

    return res;
}

void wxDataViewTreeCtrl::SetItemText( const wxDataViewItem& item, const std::string &text )
{
    GetStore()->SetItemText(item,text);

    // notify control
    GetStore()->ValueChanged( item, 0 );
}

void wxDataViewTreeCtrl::SetItemIcon( const wxDataViewItem& item, const wxIcon &icon )
{
    GetStore()->SetItemIcon(item,icon);

    // notify control
    GetStore()->ValueChanged( item, 0 );
}

void wxDataViewTreeCtrl::SetItemExpandedIcon( const wxDataViewItem& item, const wxIcon &icon )
{
    GetStore()->SetItemExpandedIcon(item,icon);

    // notify control
    GetStore()->ValueChanged( item, 0 );
}

void wxDataViewTreeCtrl::DeleteItem( const wxDataViewItem& item )
{
    wxDataViewItem parent_item = GetStore()->GetParent( item );

    GetStore()->DeleteItem(item);

    // notify control
    GetStore()->ItemDeleted( parent_item, item );
}

void wxDataViewTreeCtrl::DeleteChildren( const wxDataViewItem& item )
{
    wxDataViewTreeStoreContainerNode *node = GetStore()->FindContainerNode( item );
    if (!node) return;

    wxDataViewItemArray array;
    wxDataViewTreeStoreNodes::iterator iter;
    for (iter = node->GetChildren().begin(); iter != node->GetChildren().end(); ++iter)
    {
        wxDataViewTreeStoreNode* child = *iter;
        array.push_back( child->GetItem() );
    }

    GetStore()->DeleteChildren( item );

    // notify control
    GetStore()->ItemsDeleted( item, array );
}

void  wxDataViewTreeCtrl::DeleteAllItems()
{
    GetStore()->DeleteAllItems();

    GetStore()->Cleared();
}

void wxDataViewTreeCtrl::OnExpanded( wxDataViewEvent &event )
{
    if (!HasImageList()) return;

    wxDataViewTreeStoreContainerNode* container = GetStore()->FindContainerNode( event.GetItem() );
    if (!container) return;

    container->SetExpanded( true );

    GetStore()->ItemChanged( event.GetItem() );
}

void wxDataViewTreeCtrl::OnCollapsed( wxDataViewEvent &event )
{
    if (!HasImageList()) return;

    wxDataViewTreeStoreContainerNode* container = GetStore()->FindContainerNode( event.GetItem() );
    if (!container) return;

    container->SetExpanded( false );

    GetStore()->ItemChanged( event.GetItem() );
}

void wxDataViewTreeCtrl::OnSize( wxSizeEvent &event )
{
#if defined(wxHAS_GENERIC_DATAVIEWCTRL)
    // automatically resize our only column to take the entire control width
    if ( GetColumnCount() )
    {
        wxSize size = GetClientSize();
        GetColumn(0)->SetWidth(size.x);
    }
#endif
    event.Skip( true );
}

#endif // wxUSE_DATAVIEWCTRL
