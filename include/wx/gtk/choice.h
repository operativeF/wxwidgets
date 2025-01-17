/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/choice.h
// Purpose:
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_CHOICE_H_
#define _WX_GTK_CHOICE_H_

class WXDLLIMPEXP_FWD_BASE wxSortedArrayString;
class WXDLLIMPEXP_FWD_BASE wxArrayString;

//-----------------------------------------------------------------------------
// wxChoice
//-----------------------------------------------------------------------------

class wxGtkCollatedArrayString;

class wxChoice : public wxChoiceBase
{
public:
    wxChoice()
    {
        Init();
    }
    wxChoice( wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            int n = 0, const wxString choices[] = (const wxString *) NULL,
            long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxChoiceNameStr) )
    {
        Init();
        Create(parent, id, pos, size, n, choices, style, validator, name);
    }
    wxChoice( wxWindow *parent, wxWindowID id,
            const wxPoint& pos,
            const wxSize& size,
            const wxArrayString& choices,
            long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxChoiceNameStr) )
    {
        Init();
        Create(parent, id, pos, size, choices, style, validator, name);
    }
    virtual ~wxChoice();
    bool Create( wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            int n = 0, const wxString choices[] = NULL,
            long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxChoiceNameStr) );
    bool Create( wxWindow *parent, wxWindowID id,
            const wxPoint& pos,
            const wxSize& size,
            const wxArrayString& choices,
            long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxChoiceNameStr) );

    int GetSelection() const override;
    void SetSelection(int n) override;

    unsigned int GetCount() const override;
    int FindString(const wxString& s, bool bCase = false) const override;
    wxString GetString(unsigned int n) const override;
    void SetString(unsigned int n, const wxString& string) override;

    void SetColumns(int n=1) override;
    int GetColumns() const override;

    virtual void GTKDisableEvents();
    virtual void GTKEnableEvents();

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWINDOW_VARIANT_NORMAL);

protected:
    // this array is only used for controls with wxCB_SORT style, so only
    // allocate it if it's needed (hence using pointer)
    wxGtkCollatedArrayString *m_strings;

    // contains the client data for the items
    std::vector<void*> m_clientData;

    // index to GtkListStore cell which displays the item text
    int m_stringCellIndex;

    wxSize DoGetBestSize() const override;
    wxSize DoGetSizeFromTextSize(int xlen, int ylen = -1) const override;
    virtual int DoInsertItems(const wxArrayStringsAdapter& items,
                              unsigned int pos,
                              void **clientData, wxClientDataType type) override;
    void DoSetItemClientData(unsigned int n, void* clientData) override;
    void* DoGetItemClientData(unsigned int n) const override;
    void DoClear() override;
    void DoDeleteOneItem(unsigned int n) override;

    bool GTKHandleFocusOut() override;
    GdkWindow *GTKGetWindow(wxArrayGdkWindows& windows) const override;
    void DoApplyWidgetStyle(GtkRcStyle *style) override;

    // in derived classes, implement this to insert list store entry
    // with all items default except text
    virtual void GTKInsertComboBoxTextItem( unsigned int n, const wxString& text );

private:
    void Init();

    wxDECLARE_DYNAMIC_CLASS(wxChoice);
};


#endif // _WX_GTK_CHOICE_H_
