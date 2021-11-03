/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/bmpcbox.h
// Purpose:     wxBitmapComboBox
// Author:      Jaakko Salli
// Created:     2008-04-06
// Copyright:   (c) 2008 Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_BMPCBOX_H_
#define _WX_MSW_BMPCBOX_H_


#include "wx/combobox.h"

#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// wxBitmapComboBox: a wxComboBox that allows images to be shown
// in front of string items.
// ----------------------------------------------------------------------------

class wxBitmapComboBox : public wxComboBox,
                                          public wxBitmapComboBoxBase
{
public:
    
    wxBitmapComboBox() = default;

    wxBitmapComboBox(wxWindow *parent,
                     wxWindowID id = wxID_ANY,
                     const std::string& value = {},
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     const std::vector<std::string>& choices = {},
                     unsigned int style = 0,
                     const wxValidator& validator = wxDefaultValidator,
                     const std::string& name = wxBitmapComboBoxNameStr)
        
          
    {
        Create(parent, id, value, pos, size,
                     choices, style, validator, name);
    }

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& value,
                const wxPoint& pos,
                const wxSize& size,
                const std::vector<std::string>& choices,
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxBitmapComboBoxNameStr);

    ~wxBitmapComboBox();

    // Sets the image for the given item.
    void SetItemBitmap(unsigned int n, const wxBitmap& bitmap) override;

    bool SetFont(const wxFont& font) override;

    // Adds item with image to the end of the combo box.
    int Append(const std::string& item, const wxBitmap& bitmap = wxNullBitmap);
    int Append(const std::string& item, const wxBitmap& bitmap, void *clientData);
    int Append(const std::string& item, const wxBitmap& bitmap, wxClientData *clientData);

    // Inserts item with image into the list before pos. Not valid for wxCB_SORT
    // styles, use Append instead.
    int Insert(const std::string& item, const wxBitmap& bitmap, unsigned int pos);
    int Insert(const std::string& item, const wxBitmap& bitmap,
               unsigned int pos, void *clientData);
    int Insert(const std::string& item, const wxBitmap& bitmap,
               unsigned int pos, wxClientData *clientData);

protected:

    DWORD MSWGetStyle(unsigned int style, DWORD *exstyle) const override;
    bool MSWOnDraw(WXDRAWITEMSTRUCT *item) override;
    bool MSWOnMeasure(WXMEASUREITEMSTRUCT *item) override;
    void MSWUpdateFontOnDPIChange(const wxSize& newDPI) override;

    // Event handlers
    void OnSize(wxSizeEvent& event);

    wxItemContainer* GetItemContainer() override { return this; }
    wxWindow* GetControl() override { return this; }

    // wxItemContainer implementation
    int DoInsertItems(const std::vector<std::string>& items,
                              unsigned int pos,
                              void **clientData, wxClientDataType type) override;
    void DoClear() override;
    void DoDeleteOneItem(unsigned int n) override;

    bool OnAddBitmap(const wxBitmap& bitmap) override;
    wxSize DoGetBestSize() const override;
    void RecreateControl();

private:
    bool m_inResize {false};

    wxDECLARE_EVENT_TABLE();

    wxDECLARE_DYNAMIC_CLASS(wxBitmapComboBox);
};

#endif // _WX_MSW_BMPCBOX_H_
