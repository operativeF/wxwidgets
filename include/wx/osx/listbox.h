/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/listbox.h
// Purpose:     wxListBox class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_LISTBOX_H_
#define _WX_LISTBOX_H_

// ----------------------------------------------------------------------------
// simple types
// ----------------------------------------------------------------------------
#include  "wx/dynarray.h"
#include  "wx/arrstr.h"

// forward decl for GetSelections()
class wxArrayInt;

// forward decl for wxListWidgetImpl implementation type.
class wxListWidgetImpl;

// List box item

WX_DEFINE_ARRAY( char* , wxListDataArray );

// ----------------------------------------------------------------------------
// List box control
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_FWD_CORE wxListWidgetColumn;

class WXDLLIMPEXP_FWD_CORE wxListWidgetCellValue;

class wxListBox : public wxListBoxBase
{
public:
    
    wxListBox();

    wxListBox(
        wxWindow *parent,
        wxWindowID winid,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        int n = 0, const wxString choices[] = NULL,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxASCII_STR(wxListBoxNameStr))
    {
        Create(parent, winid, pos, size, n, choices, style, validator, name);
    }

    wxListBox(
        wxWindow *parent,
        wxWindowID winid,
        const wxPoint& pos,
        const wxSize& size,
        const wxArrayString& choices,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxASCII_STR(wxListBoxNameStr))
    {
        Create(parent, winid, pos, size, choices, style, validator, name);
    }

    bool Create(
        wxWindow *parent,
        wxWindowID winid,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        int n = 0,
        const wxString choices[] = NULL,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxASCII_STR(wxListBoxNameStr));

    bool Create(
        wxWindow *parent,
        wxWindowID winid,
        const wxPoint& pos,
        const wxSize& size,
        const wxArrayString& choices,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxASCII_STR(wxListBoxNameStr));

    virtual ~wxListBox();

    
    unsigned int GetCount() const override;
    wxString GetString(unsigned int n) const override;
    void SetString(unsigned int n, const wxString& s) override;
    int FindString(const wxString& s, bool bCase = false) const override;

    // data callbacks
    virtual void GetValueCallback( unsigned int n, wxListWidgetColumn* col , wxListWidgetCellValue& value );
    virtual void SetValueCallback( unsigned int n, wxListWidgetColumn* col , wxListWidgetCellValue& value );

    bool IsSelected(int n) const override;
    int GetSelection() const override;
    int GetSelections(wxArrayInt& aSelections) const override;

    void EnsureVisible(int n) override;

    int GetTopItem() const override;
    int GetCountPerPage() const override;

    wxVisualAttributes GetDefaultAttributes() const override
    {
        return GetClassDefaultAttributes(GetWindowVariant());
    }

    // wxCheckListBox support
    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWINDOW_VARIANT_NORMAL);

    wxListWidgetImpl* GetListPeer() const;

    virtual void HandleLineEvent( unsigned int n, bool doubleClick );

    // This is called by wxNSTableView
    void MacHandleSelectionChange(int row);

protected:
    // callback for derived classes which may have to insert additional data
    // at a certain line - which cannot be predetermined for sorted list data
    virtual void OnItemInserted(unsigned int pos);

    void DoClear() override;
    void DoDeleteOneItem(unsigned int n) override;

    // from wxItemContainer
    virtual int DoInsertItems(const wxArrayStringsAdapter& items,
                              unsigned int pos,
                              void **clientData, wxClientDataType type) override;

    void DoSetItemClientData(unsigned int n, void* clientData) override;
    void* DoGetItemClientData(unsigned int n) const override;

    // from wxListBoxBase
    void DoSetSelection(int n, bool select) override;
    void DoSetFirstItem(int n) override;
    int DoListHitTest(const wxPoint& point) const override;

    // free memory (common part of Clear() and dtor)
    // prevent collision with some BSD definitions of macro Free()
    void FreeData();

    wxSize DoGetBestSize() const override;

    bool m_blockEvents;

    wxListWidgetColumn* m_textColumn;

    // data storage (copied from univ)

    // the array containing all items (it is sorted if the listbox has
    // wxLB_SORT style)
    union
    {
        wxArrayString *unsorted;
        wxSortedArrayString *sorted;
    } m_strings;

    // and this one the client data (either void or wxClientData)
    std::vector<void*> m_itemsClientData;

private:
    // Mostly the same as DoSetSelection() but doesn't call EnsureVisible().
    void DoSetSelectionWithoutEnsureVisible(int n, bool select);

    wxDECLARE_DYNAMIC_CLASS(wxListBox);
    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_LISTBOX_H_
