///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/checklst.cpp
// Purpose:     implementation of wxCheckListBox class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     16.11.97
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_CHECKLISTBOX && wxUSE_OWNER_DRAWN

#include "wx/checklst.h"
#include "wx/window.h"
#include "wx/listbox.h"
#include "wx/log.h"
#include "wx/ownerdrw.h"

#include "wx/renderer.h"
#include "wx/msw/dc.h"
#include "wx/msw/private/dcdynwrap.h"

#include "wx/msw/private.h"
#include "wx/msw/wrapcctl.h"

#include <memory>

import WX.WinDef;
import WX.Win.UniqueHnd;

import WX.Utils.Settings;

import <string>;
import <string_view>;
import <vector>;

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

// get item (converted to right type)
#define GetItem(n)    ((wxCheckListBoxItem *)(GetItem(n)))


// space around check mark bitmap in pixels
constexpr int CHECKMARK_EXTRA_SPACE = 1;

// space between check bitmap and text label
constexpr int CHECKMARK_LABEL_SPACE = 2;

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// declaration and implementation of wxCheckListBoxItem class
// ----------------------------------------------------------------------------

class wxCheckListBoxItem : public wxOwnerDrawn
{
public:
    explicit wxCheckListBoxItem(wxCheckListBox *parent);

    wxCheckListBoxItem(const wxCheckListBoxItem&) = delete;
	wxCheckListBoxItem& operator=(const wxCheckListBoxItem&) = delete;

    // drawing functions
    bool OnDrawItem(wxDC& dc, const wxRect& rc, wxODAction act, wxODStatus stat) override;

    // simple accessors and operations
    wxCheckListBox *GetParent() const
        { return m_parent; }

    int GetIndex() const
        { return m_parent->GetItemIndex(const_cast<wxCheckListBoxItem*>(this)); }

    std::string GetName() const override
        { return m_parent->GetString(GetIndex()); }


    bool IsChecked() const
        { return m_checked; }

    void Check(bool bCheck)
        { m_checked = bCheck; }

    void Toggle()
        { Check(!IsChecked()); }

private:
    wxCheckListBox *m_parent;
    bool m_checked;
};

wxCheckListBoxItem::wxCheckListBoxItem(wxCheckListBox *parent)
{
    m_parent = parent;
    m_checked = false;

    wxSize size = wxRendererNative::Get().GetCheckBoxSize(parent);
    size.x += 2 * CHECKMARK_EXTRA_SPACE + CHECKMARK_LABEL_SPACE;

    SetMarginWidth(size.x);
    SetBackgroundColour(parent->GetBackgroundColour());
}

bool wxCheckListBoxItem::OnDrawItem(wxDC& dc, const wxRect& rc,
                                    wxODAction act, wxODStatus stat)
{
    // first draw the label
    if ( !wxOwnerDrawn::OnDrawItem(dc, rc, act, stat) )
        return false;

    // now draw the check mark part
    wxMSWDCImpl *impl = (wxMSWDCImpl*) dc.GetImpl();
    WXHDC hdc = GetHdcOf(*impl);

    wxSize size = wxRendererNative::Get().GetCheckBoxSize(GetParent());

    // first create bitmap in a memory DC
    MemoryHDC hdcMem(hdc);

    using msw::utils::unique_bitmap;

    auto hBmpCheck = unique_bitmap(::CreateCompatibleBitmap(hdc, size.x, size.y));

    // then draw a check mark into it
    {
        SelectInHDC selBmp(hdcMem, hBmpCheck.get());

        unsigned int flags = wxCONTROL_FLAT;
        if ( IsChecked() )
            flags |= wxCONTROL_CHECKED;

        wxDCTemp dcMem(hdcMem);
        wxRendererNative::Get().DrawCheckBox(GetParent(), dcMem, wxRect(size), flags);
    } // select hBmpCheck out of hdcMem

    // finally draw bitmap to screen

    // position of check mark bitmap
    int x = rc.GetX() + CHECKMARK_EXTRA_SPACE;
    int y = rc.GetY() + (rc.GetHeight() - size.y) / 2;

    WXUINT uState = stat & wxOwnerDrawn::wxODSelected ? wxDSB_SELECTED : wxDSB_NORMAL;

    // checkmarks should not be mirrored in RTL layout
    WXDWORD oldLayout = wxDynLoadWrappers::GetLayout(hdc);
    if ( oldLayout & LAYOUT_RTL )
        ::SetLayout(hdc, oldLayout | LAYOUT_BITMAPORIENTATIONPRESERVED);
    wxDrawStateBitmap(hdc, hBmpCheck.get(), x, y, uState);
    if ( oldLayout & LAYOUT_RTL )
        ::SetLayout(hdc, oldLayout);

    return true;
}

// ----------------------------------------------------------------------------
// implementation of wxCheckListBox class
// ----------------------------------------------------------------------------

// define event table
// ------------------
wxBEGIN_EVENT_TABLE(wxCheckListBox, wxListBox)
  EVT_KEY_DOWN(wxCheckListBox::OnKeyDown)
  EVT_LEFT_DOWN(wxCheckListBox::OnLeftClick)
wxEND_EVENT_TABLE()

// control creation
// ----------------

// TODO: Use std::span
// ctor which creates the associated control
wxCheckListBox::wxCheckListBox(wxWindow *parent, wxWindowID id,
                               const wxPoint& pos, const wxSize& size,
                               const std::vector<std::string>& choices,
                               unsigned int style, const wxValidator& val,
                               std::string_view name)
{
    Create(parent, id, pos, size, choices, style, val, name);
}

bool wxCheckListBox::Create(wxWindow *parent,
                            wxWindowID id,
                            const wxPoint& pos,
                            const wxSize& size,
                            const std::vector<std::string>& choices,
                            unsigned int style,
                            const wxValidator& validator,
                            std::string_view name)
{
    return wxListBox::Create(parent, id, pos, size, choices,
                             style | wxLB_OWNERDRAW, validator, name);
}

// create/retrieve item
// --------------------

// create a check list box item
std::unique_ptr<wxOwnerDrawn> wxCheckListBox::CreateLboxItem([[maybe_unused]] size_t n)
{
    return std::make_unique<wxCheckListBoxItem>(this);
}

// return item size
// ----------------
bool wxCheckListBox::MSWOnMeasure(WXMEASUREITEMSTRUCT *item)
{
    if ( wxListBox::MSWOnMeasure(item) )
    {
        MEASUREITEMSTRUCT *pStruct = (MEASUREITEMSTRUCT *)item;

        const wxSize size = MSWGetFullItemSize(pStruct->itemWidth,
                                                 pStruct->itemHeight);
        pStruct->itemWidth = size.x;
        pStruct->itemHeight = size.y;

        return true;
    }

    return false;
}

void wxCheckListBox::MSWUpdateFontOnDPIChange(const wxSize& newDPI)
{
    wxCheckListBoxBase::MSWUpdateFontOnDPIChange(newDPI);

    wxSize size = wxRendererNative::Get().GetCheckBoxSize(this);
    size.x += 2 * CHECKMARK_EXTRA_SPACE + CHECKMARK_LABEL_SPACE;

    for ( unsigned int i = 0; i < GetCount(); ++i )
    {
        GetItem(i)->SetMarginWidth(size.x);
    }
}

// check items
// -----------

bool wxCheckListBox::IsChecked(unsigned int uiIndex) const
{
    wxCHECK_MSG( IsValid(uiIndex), false, "bad wxCheckListBox index" );

    return GetItem(uiIndex)->IsChecked();
}

void wxCheckListBox::Check(unsigned int uiIndex, bool bCheck)
{
    wxCHECK_RET( IsValid(uiIndex), "bad wxCheckListBox index" );

    GetItem(uiIndex)->Check(bCheck);
    RefreshItem(uiIndex);
}

void wxCheckListBox::Toggle(unsigned int uiIndex)
{
    wxCHECK_RET( IsValid(uiIndex), "bad wxCheckListBox index" );

    GetItem(uiIndex)->Toggle();
    RefreshItem(uiIndex);
}

// process events
// --------------

void wxCheckListBox::OnKeyDown(wxKeyEvent& event)
{
    // what do we do?
    enum
    {
        NONE,
        TOGGLE,
        SET,
        CLEAR
    } oper;

    switch ( event.GetKeyCode() )
    {
        case WXK_SPACE:
            oper = TOGGLE;
            break;

        case WXK_NUMPAD_ADD:
        case '+':
            oper = SET;
            break;

        case WXK_NUMPAD_SUBTRACT:
        case '-':
            oper = CLEAR;
            break;

        default:
            oper = NONE;
    }

    if ( oper != NONE )
    {
        std::vector<int> selections;
        int count = 0;
        if ( HasMultipleSelection() )
        {
            count = GetSelections(selections);
        }
        else
        {
            int sel = GetSelection();
            if (sel != -1)
            {
                count = 1;
                selections.push_back(sel);
            }
        }

        for ( const auto selection : selections )
        {
            switch ( oper )
            {
                case TOGGLE:
                    Toggle(selection);
                    break;

                case SET:
                case CLEAR:
                    Check(selection, oper == SET);
                    break;

                default:
                    wxFAIL_MSG( "what should this key do?" );
            }

            // we should send an event as this has been done by the user and
            // not by the program
            SendEvent(selection);
        }
    }
    else // nothing to do
    {
        event.Skip();
    }
}

void wxCheckListBox::OnLeftClick(wxMouseEvent& event)
{
    // clicking on the item selects it, clicking on the checkmark toggles

    int nItem = HitTest(event.GetX(), event.GetY());

    if ( nItem != wxNOT_FOUND )
    {
        wxRect rect;
        GetItemRect(nItem, rect);

        // convert item rect to check mark rect
        wxSize size = wxRendererNative::Get().GetCheckBoxSize(this);
        rect.x += CHECKMARK_EXTRA_SPACE;
        rect.y += (rect.GetHeight() - size.y) / 2;
        rect.SetSize(size);

        if ( rect.Contains(event.GetX(), event.GetY()) )
        {
            // people expect to get "kill focus" event for the currently
            // focused control before getting events from the other controls
            // and, equally importantly, they may prevent the focus change from
            // taking place at all (e.g. because the old control contents is
            // invalid and needs to be corrected) in which case we shouldn't
            // generate this event at all
            SetFocus();
            if ( FindFocus() == this )
            {
                Toggle(nItem);
                SendEvent(nItem);

                // scroll one item down if the item is the last one
                // and isn't visible at all
                int h = GetClientSize().y;

                if ( rect.GetBottom() > h )
                    ScrollLines(1);
            }
        }
        else
        {
            // implement default behaviour: clicking on the item selects it
            event.Skip();
        }
    }
    else
    {
        // implement default behaviour on click outside of client zone
        event.Skip();
    }
}

wxSize wxCheckListBox::MSWGetFullItemSize(int w, int h) const
{
    wxSize size = wxRendererNative::Get().GetCheckBoxSize(const_cast<wxCheckListBox*>(this));
    size.x += 2 * CHECKMARK_EXTRA_SPACE;
    size.y += 2 * CHECKMARK_EXTRA_SPACE;

    w += size.x;
    if ( h < size.y )
        h = size.y;

    return {w, h};
}

wxSize wxCheckListBox::DoGetBestClientSize() const
{
    wxSize best = wxListBox::DoGetBestClientSize();

    // add room for the checkbox
    return MSWGetFullItemSize(best.x, best.y);
}

#endif // wxUSE_CHECKLISTBOX
