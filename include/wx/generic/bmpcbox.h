/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/bmpcbox.h
// Purpose:     wxBitmapComboBox
// Author:      Jaakko Salli
// Modified by:
// Created:     Aug-30-2006
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_BMPCBOX_H_
#define _WX_GENERIC_BMPCBOX_H_


#define wxGENERIC_BITMAPCOMBOBOX     1

#include "wx/odcombo.h"

// ----------------------------------------------------------------------------
// wxBitmapComboBox: a wxComboBox that allows images to be shown
// in front of string items.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxBitmapComboBox : public wxOwnerDrawnComboBox,
                                         public wxBitmapComboBoxBase
{
public:

    
    wxBitmapComboBox()
    {
        Init();
    }

    wxBitmapComboBox(wxWindow *parent,
                     wxWindowID id = wxID_ANY,
                     const std::string& value = {},
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     int n = 0,
                     const wxString choices[] = NULL,
                     long style = 0,
                     const wxValidator& validator = wxDefaultValidator,
                     const std::string& name = wxBitmapComboBoxNameStr)
        : wxOwnerDrawnComboBox(),
          wxBitmapComboBoxBase()
    {
        Init();

        (void)Create(parent, id, value, pos, size, n,
                     choices, style, validator, name);
    }

    wxBitmapComboBox(wxWindow *parent,
                     wxWindowID id,
                     const std::string& value,
                     const wxPoint& pos,
                     const wxSize& size,
                     const wxArrayString& choices,
                     long style,
                     const wxValidator& validator = wxDefaultValidator,
                     const std::string& name = wxBitmapComboBoxNameStr);

    bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& value,
                const wxPoint& pos,
                const wxSize& size,
                int n,
                const wxString choices[],
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxBitmapComboBoxNameStr);

    bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& value,
                const wxPoint& pos,
                const wxSize& size,
                const wxArrayString& choices,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxASCII_STR(wxBitmapComboBoxNameStr));

    virtual ~wxBitmapComboBox();

    std::string GetStringSelection() const override;

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

    // Sets the image for the given item.
    void SetItemBitmap(unsigned int n, const wxBitmap& bitmap) override;
    bool SetFont(const wxFont& font) override;

protected:

    void OnDrawBackground(wxDC& dc, const wxRect& rect, int item, int flags) const override;
    void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const override;
    wxCoord OnMeasureItem(size_t item) const override;
    wxCoord OnMeasureItemWidth(size_t item) const override;

    // Event handlers
    void OnSize(wxSizeEvent& event);

    wxSize DoGetBestSize() const override;

    wxItemContainer* GetItemContainer() override { return this; }
    wxWindow* GetControl() override { return this; }

    // wxItemContainer implementation
    virtual int DoInsertItems(const wxArrayStringsAdapter & items,
                              unsigned int pos,
                              void **clientData, wxClientDataType type) override;
    void DoClear() override;
    void DoDeleteOneItem(unsigned int n) override;

private:
    bool                m_inResize;

    void Init();

    wxDECLARE_EVENT_TABLE();

    wxDECLARE_DYNAMIC_CLASS(wxBitmapComboBox);
};

#endif // _WX_GENERIC_BMPCBOX_H_
