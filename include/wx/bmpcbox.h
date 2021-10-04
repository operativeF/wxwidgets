/////////////////////////////////////////////////////////////////////////////
// Name:        wx/bmpcbox.h
// Purpose:     wxBitmapComboBox base header
// Author:      Jaakko Salli
// Modified by:
// Created:     Aug-31-2006
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BMPCBOX_H_BASE_
#define _WX_BMPCBOX_H_BASE_


#include "wx/defs.h"

#if wxUSE_BITMAPCOMBOBOX

#include <string>
#include <vector>

class WXDLLIMPEXP_FWD_CORE wxBitmap;
class WXDLLIMPEXP_FWD_CORE wxRect;
class WXDLLIMPEXP_FWD_CORE wxWindow;
class WXDLLIMPEXP_FWD_CORE wxItemContainer;

// Define wxBITMAPCOMBOBOX_OWNERDRAWN_BASED for platforms which
// wxBitmapComboBox implementation utilizes ownerdrawn combobox
// (either native or generic).
#if !defined(__WXGTK20__) || defined(__WXUNIVERSAL__)
    #define wxBITMAPCOMBOBOX_OWNERDRAWN_BASED

class WXDLLIMPEXP_FWD_CORE wxDC;
#endif

constexpr char wxBitmapComboBoxNameStr[] = "bitmapComboBox";


class WXDLLIMPEXP_CORE wxBitmapComboBoxBase
{
public:
    wxBitmapComboBoxBase() = default;

    virtual ~wxBitmapComboBoxBase() = default;

    // Sets the image for the given item.
    virtual void SetItemBitmap(unsigned int n, const wxBitmap& bitmap) = 0;

#if !defined(wxBITMAPCOMBOBOX_OWNERDRAWN_BASED)

    // Returns the image of the item with the given index.
    virtual wxBitmap GetItemBitmap(unsigned int n) const = 0;

    // Returns size of the image used in list
    virtual wxSize GetBitmapSize() const = 0;

private:
#else // wxBITMAPCOMBOBOX_OWNERDRAWN_BASED

    // Returns the image of the item with the given index.
    virtual wxBitmap GetItemBitmap(unsigned int n) const;

    // Returns size of the image used in list
    virtual wxSize GetBitmapSize() const
    {
        return m_usedImgSize;
    }

protected:

    // Returns pointer to the combobox item container
    virtual wxItemContainer* GetItemContainer() = 0;

    // Return pointer to the owner-drawn combobox control
    virtual wxWindow* GetControl() = 0;

    // wxItemContainer functions
    void BCBDoClear();
    void BCBDoDeleteOneItem(unsigned int n);

    void DoSetItemBitmap(unsigned int n, const wxBitmap& bitmap);

    void DrawBackground(wxDC& dc, const wxRect& rect, int item, unsigned int flags) const;
    void DrawItem(wxDC& dc, const wxRect& rect, int item, const std::string& text,
                  unsigned int flags) const;
    wxCoord MeasureItem(size_t item) const;

    // Returns true if image size was affected
    virtual bool OnAddBitmap(const wxBitmap& bitmap);

    // Recalculates amount of empty space needed in front of text
    // in control itself. Returns number that can be passed to
    // wxOwnerDrawnComboBox::SetCustomPaintWidth() and similar
    // functions.
    virtual int DetermineIndent();

    void UpdateInternals();

    // FIXME: Remove protected data
    std::vector<void*>  m_bitmaps;  // Images associated with items
    wxSize              m_usedImgSize{-1, -1};  // Size of bitmaps

private:
    int                 m_imgAreaWidth{0};  // Width and height of area next to text field
    int                 m_fontHeight{0};
    int                 m_indent{0};

#endif // !wxBITMAPCOMBOBOX_OWNERDRAWN_BASED/wxBITMAPCOMBOBOX_OWNERDRAWN_BASED
};


#if defined(__WXUNIVERSAL__)
    #include "wx/generic/bmpcbox.h"
#elif defined(__WXMSW__)
    #include "wx/msw/bmpcbox.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/bmpcbox.h"
#else
    #include "wx/generic/bmpcbox.h"
#endif

#endif // wxUSE_BITMAPCOMBOBOX

#endif // _WX_BMPCBOX_H_BASE_
