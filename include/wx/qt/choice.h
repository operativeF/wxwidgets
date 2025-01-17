/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/choice.h
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_CHOICE_H_
#define _WX_QT_CHOICE_H_

class QComboBox;

class wxChoice : public wxChoiceBase
{
public:
    wxChoice();

    wxChoice( wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            int n = 0, const wxString choices[] = (const wxString *) NULL,
            unsigned int style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxChoiceNameStr) );

    wxChoice( wxWindow *parent, wxWindowID id,
            const wxPoint& pos,
            const wxSize& size,
            const wxArrayString& choices,
            unsigned int style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxChoiceNameStr) );

    bool Create( wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            int n = 0, const wxString choices[] = NULL,
            unsigned int style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxChoiceNameStr) );

    bool Create( wxWindow *parent, wxWindowID id,
            const wxPoint& pos,
            const wxSize& size,
            const wxArrayString& choices,
            unsigned int style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxChoiceNameStr) );

    wxSize DoGetBestSize() const override;

    unsigned int GetCount() const override;
    wxString GetString(unsigned int n) const override;
    void SetString(unsigned int n, const wxString& s) override;

    void SetSelection(int n) override;
    int GetSelection() const override;

    QWidget *GetHandle() const override;

protected:
    virtual int DoInsertItems(const wxArrayStringsAdapter & items,
                              unsigned int pos,
                              void **clientData,
                              wxClientDataType type) override;
    int DoInsertOneItem(const wxString& item, unsigned int pos) override;

    void DoSetItemClientData(unsigned int n, void *clientData) override;
    void *DoGetItemClientData(unsigned int n) const override;

    void DoClear() override;
    void DoDeleteOneItem(unsigned int pos) override;

    void QtInitSort(QComboBox *combo);

    QComboBox *m_qtComboBox;

private:

    wxDECLARE_DYNAMIC_CLASS(wxChoice);
};

#endif // _WX_QT_CHOICE_H_
