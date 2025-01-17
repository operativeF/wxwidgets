///////////////////////////////////////////////////////////////////////////////
// Name:        src/ribbon/gallery.cpp
// Purpose:     Ribbon control which displays a gallery of items to choose from
// Author:      Peter Cawley
// Modified by:
// Created:     2009-07-22
// Copyright:   (C) Peter Cawley
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_RIBBON

#include "wx/ribbon/gallery.h"
#include "wx/ribbon/art.h"
#include "wx/ribbon/bar.h"
#include "wx/dcbuffer.h"

#ifdef __WXMSW__
    #include "wx/msw/private.h"
#endif

import WX.Cmn.ClntData;

wxDEFINE_EVENT(wxEVT_RIBBONGALLERY_HOVER_CHANGED, wxRibbonGalleryEvent);
wxDEFINE_EVENT(wxEVT_RIBBONGALLERY_SELECTED, wxRibbonGalleryEvent);
wxDEFINE_EVENT(wxEVT_RIBBONGALLERY_CLICKED, wxRibbonGalleryEvent);

wxIMPLEMENT_DYNAMIC_CLASS(wxRibbonGalleryEvent, wxCommandEvent);
wxIMPLEMENT_CLASS(wxRibbonGallery, wxRibbonControl);

class wxRibbonGalleryItem
{
public:
    void SetId(int id) {m_id = id;}
    void SetBitmap(const wxBitmap& bitmap) {m_bitmap = bitmap;}
    const wxBitmap& GetBitmap() const {return m_bitmap;}
    void SetIsVisible(bool visible) {m_is_visible = visible;}
    void SetPosition(int x, int y, const wxSize& size)
    {
        m_position = wxRect(wxPoint(x, y), size);
    }
    bool IsVisible() const {return m_is_visible;}
    const wxRect& GetPosition() const {return m_position;}

    void SetClientObject(wxClientData *data) {m_client_data.SetClientObject(data);}
    wxClientData *GetClientObject() const {return m_client_data.GetClientObject();}
    void SetClientData(void *data) {m_client_data.SetClientData(data);}
    void *GetClientData() const {return m_client_data.GetClientData();}

protected:
    wxBitmap m_bitmap;
    wxClientDataContainer m_client_data;
    wxRect m_position;
    int m_id{0};
    bool m_is_visible{false};
};

wxBEGIN_EVENT_TABLE(wxRibbonGallery, wxRibbonControl)
    EVT_ENTER_WINDOW(wxRibbonGallery::OnMouseEnter)
    EVT_ERASE_BACKGROUND(wxRibbonGallery::OnEraseBackground)
    EVT_LEAVE_WINDOW(wxRibbonGallery::OnMouseLeave)
    EVT_LEFT_DOWN(wxRibbonGallery::OnMouseDown)
    EVT_LEFT_UP(wxRibbonGallery::OnMouseUp)
    EVT_LEFT_DCLICK(wxRibbonGallery::OnMouseDClick)
    EVT_MOTION(wxRibbonGallery::OnMouseMove)
    EVT_PAINT(wxRibbonGallery::OnPaint)
    EVT_SIZE(wxRibbonGallery::OnSize)
wxEND_EVENT_TABLE()

wxRibbonGallery::wxRibbonGallery(wxWindow* parent,
                  wxWindowID id,
                  const wxPoint& pos,
                  const wxSize& size,
                  unsigned int style)
    : wxRibbonControl(parent, id, pos, size, wxBORDER_NONE)
{
    SetBackgroundStyle(wxBackgroundStyle::Paint);
}

wxRibbonGallery::~wxRibbonGallery()
{
    Clear();
}

bool wxRibbonGallery::Create(wxWindow* parent,
                wxWindowID id,
                const wxPoint& pos,
                const wxSize& size,
                unsigned int style)
{
    if(!wxRibbonControl::Create(parent, id, pos, size, wxBORDER_NONE))
    {
        return false;
    }

    SetBackgroundStyle(wxBackgroundStyle::Paint);
    
    return true;
}

void wxRibbonGallery::OnMouseEnter(wxMouseEvent& evt)
{
    m_hovered = true;
    if(m_mouse_active_rect != nullptr && !evt.LeftIsDown())
    {
        m_mouse_active_rect = nullptr;
        m_active_item = nullptr;
    }
    Refresh(false);
}

void wxRibbonGallery::OnMouseMove(wxMouseEvent& evt)
{
    bool refresh = false;
    wxPoint pos = evt.GetPosition();

    if(TestButtonHover(m_scroll_up_button_rect, pos, &m_up_button_state))
        refresh = true;
    if(TestButtonHover(m_scroll_down_button_rect, pos, &m_down_button_state))
        refresh = true;
    if(TestButtonHover(m_extension_button_rect, pos, &m_extension_button_state))
        refresh = true;

    wxRibbonGalleryItem *hovered_item = nullptr;
    wxRibbonGalleryItem *active_item = nullptr;
    if(m_client_rect.Contains(pos))
    {
        if(m_art && m_art->GetFlags() & wxRIBBON_BAR_FLOW_VERTICAL)
            pos.x += m_scroll_amount;
        else
            pos.y += m_scroll_amount;

        size_t item_count = m_items.Count();

        for(size_t item_i = 0; item_i < item_count; ++item_i)
        {
            wxRibbonGalleryItem *item = m_items.Item(item_i);
            if(!item->IsVisible())
                continue;

            if(item->GetPosition().Contains(pos))
            {
                if(m_mouse_active_rect == &item->GetPosition())
                    active_item = item;
                hovered_item = item;
                break;
            }
        }
    }
    if(active_item != m_active_item)
    {
        m_active_item = active_item;
        refresh = true;
    }
    if(hovered_item != m_hovered_item)
    {
        m_hovered_item = hovered_item;
        wxRibbonGalleryEvent notification(
            wxEVT_RIBBONGALLERY_HOVER_CHANGED, GetId());
        notification.SetEventObject(this);
        notification.SetGallery(this);
        notification.SetGalleryItem(hovered_item);
        ProcessWindowEvent(notification);
        refresh = true;
    }

    if(refresh)
        Refresh(false);
}

bool wxRibbonGallery::TestButtonHover(const wxRect& rect, wxPoint pos,
        wxRibbonGalleryButtonState* state)
{
    if(*state == wxRibbonGalleryButtonState::Disabled)
        return false;

    wxRibbonGalleryButtonState new_state;
    if(rect.Contains(pos))
    {
        if(m_mouse_active_rect == &rect)
            new_state = wxRibbonGalleryButtonState::Active;
        else
            new_state = wxRibbonGalleryButtonState::Hovered;
    }
    else
        new_state = wxRibbonGalleryButtonState::Normal;

    if(new_state != *state)
    {
        *state = new_state;
        return true;
    }
    else
    {
        return false;
    }
}

void wxRibbonGallery::OnMouseLeave([[maybe_unused]] wxMouseEvent& evt)
{
    m_hovered = false;
    m_active_item = nullptr;
    if(m_up_button_state != wxRibbonGalleryButtonState::Disabled)
        m_up_button_state = wxRibbonGalleryButtonState::Normal;
    if(m_down_button_state != wxRibbonGalleryButtonState::Disabled)
        m_down_button_state = wxRibbonGalleryButtonState::Normal;
    if(m_extension_button_state != wxRibbonGalleryButtonState::Disabled)
        m_extension_button_state = wxRibbonGalleryButtonState::Normal;
    if(m_hovered_item != nullptr)
    {
        m_hovered_item = nullptr;
        wxRibbonGalleryEvent notification(
            wxEVT_RIBBONGALLERY_HOVER_CHANGED, GetId());
        notification.SetEventObject(this);
        notification.SetGallery(this);
        ProcessWindowEvent(notification);
    }
    Refresh(false);
}

void wxRibbonGallery::OnMouseDown(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();
    m_mouse_active_rect = nullptr;
    if(m_client_rect.Contains(pos))
    {
        if(m_art && m_art->GetFlags() & wxRIBBON_BAR_FLOW_VERTICAL)
            pos.x += m_scroll_amount;
        else
            pos.y += m_scroll_amount;
        size_t item_count = m_items.Count();

        for(size_t item_i = 0; item_i < item_count; ++item_i)
        {
            wxRibbonGalleryItem *item = m_items.Item(item_i);
            if(!item->IsVisible())
                continue;

            const wxRect& rect = item->GetPosition();
            if(rect.Contains(pos))
            {
                m_active_item = item;
                m_mouse_active_rect = &rect;
                break;
            }
        }
    }
    else if(m_scroll_up_button_rect.Contains(pos))
    {
        if(m_up_button_state != wxRibbonGalleryButtonState::Disabled)
        {
            m_mouse_active_rect = &m_scroll_up_button_rect;
            m_up_button_state = wxRibbonGalleryButtonState::Active;
        }
    }
    else if(m_scroll_down_button_rect.Contains(pos))
    {
        if(m_down_button_state != wxRibbonGalleryButtonState::Disabled)
        {
            m_mouse_active_rect = &m_scroll_down_button_rect;
            m_down_button_state = wxRibbonGalleryButtonState::Active;
        }
    }
    else if(m_extension_button_rect.Contains(pos))
    {
        if(m_extension_button_state != wxRibbonGalleryButtonState::Disabled)
        {
            m_mouse_active_rect = &m_extension_button_rect;
            m_extension_button_state = wxRibbonGalleryButtonState::Active;
        }
    }
    if(m_mouse_active_rect != nullptr)
        Refresh(false);
}

void wxRibbonGallery::OnMouseUp(wxMouseEvent& evt)
{
    if(m_mouse_active_rect != nullptr)
    {
        wxPoint pos = evt.GetPosition();
        if(m_active_item)
        {
            if(m_art && m_art->GetFlags() & wxRIBBON_BAR_FLOW_VERTICAL)
                pos.x += m_scroll_amount;
            else
                pos.y += m_scroll_amount;
        }
        if(m_mouse_active_rect->Contains(pos))
        {
            if(m_mouse_active_rect == &m_scroll_up_button_rect)
            {
                m_up_button_state = wxRibbonGalleryButtonState::Hovered;
                ScrollLines(-1);
            }
            else if(m_mouse_active_rect == &m_scroll_down_button_rect)
            {
                m_down_button_state = wxRibbonGalleryButtonState::Hovered;
                ScrollLines(1);
            }
            else if(m_mouse_active_rect == &m_extension_button_rect)
            {
                m_extension_button_state = wxRibbonGalleryButtonState::Hovered;
                wxCommandEvent notification(wxEVT_BUTTON,
                    GetId());
                notification.SetEventObject(this);
                ProcessWindowEvent(notification);
            }
            else if(m_active_item != nullptr)
            {
                if(m_selected_item != m_active_item)
                {
                    m_selected_item = m_active_item;
                    wxRibbonGalleryEvent notification(
                        wxEVT_RIBBONGALLERY_SELECTED, GetId());
                    notification.SetEventObject(this);
                    notification.SetGallery(this);
                    notification.SetGalleryItem(m_selected_item);
                    ProcessWindowEvent(notification);
                }

                wxRibbonGalleryEvent notification(
                    wxEVT_RIBBONGALLERY_CLICKED, GetId());
                notification.SetEventObject(this);
                notification.SetGallery(this);
                notification.SetGalleryItem(m_selected_item);
                ProcessWindowEvent(notification);
            }
        }
        m_mouse_active_rect = nullptr;
        m_active_item = nullptr;
        Refresh(false);
    }
}

void wxRibbonGallery::OnMouseDClick(wxMouseEvent& evt)
{
    // The 2nd click of a double-click should be handled as a click in the
    // same way as the 1st click of the double-click. This is useful for
    // scrolling through the gallery.
    OnMouseDown(evt);
    OnMouseUp(evt);
}

void wxRibbonGallery::SetItemClientObject(wxRibbonGalleryItem* itm,
                                          wxClientData* data)
{
    itm->SetClientObject(data);
}

wxClientData* wxRibbonGallery::GetItemClientObject(const wxRibbonGalleryItem* itm) const
{
    return itm->GetClientObject();
}

void wxRibbonGallery::SetItemClientData(wxRibbonGalleryItem* itm, void* data)
{
    itm->SetClientData(data);
}

void* wxRibbonGallery::GetItemClientData(const wxRibbonGalleryItem* itm) const
{
    return itm->GetClientData();
}

bool wxRibbonGallery::ScrollLines(int lines)
{
    if(m_scroll_limit == 0 || m_art == nullptr)
        return false;

    return ScrollPixels(lines * GetScrollLineSize());
}

int wxRibbonGallery::GetScrollLineSize() const
{
    if(m_art == nullptr)
        return 32;

    int line_size = m_bitmap_padded_size.y;
    if(m_art->GetFlags() & wxRIBBON_BAR_FLOW_VERTICAL)
        line_size = m_bitmap_padded_size.x;

    return line_size;
}

bool wxRibbonGallery::ScrollPixels(int pixels)
{
    if(m_scroll_limit == 0 || m_art == nullptr)
        return false;

    if(pixels < 0)
    {
        if(m_scroll_amount > 0)
        {
            m_scroll_amount += pixels;
            if(m_scroll_amount <= 0)
            {
                m_scroll_amount = 0;
                m_up_button_state = wxRibbonGalleryButtonState::Disabled;
            }
            else if(m_up_button_state == wxRibbonGalleryButtonState::Disabled)
                m_up_button_state = wxRibbonGalleryButtonState::Normal;
            if(m_down_button_state == wxRibbonGalleryButtonState::Disabled)
                m_down_button_state = wxRibbonGalleryButtonState::Normal;
            return true;
        }
    }
    else if(pixels > 0)
    {
        if(m_scroll_amount < m_scroll_limit)
        {
            m_scroll_amount += pixels;
            if(m_scroll_amount >= m_scroll_limit)
            {
                m_scroll_amount = m_scroll_limit;
                m_down_button_state = wxRibbonGalleryButtonState::Disabled;
            }
            else if(m_down_button_state == wxRibbonGalleryButtonState::Disabled)
                m_down_button_state = wxRibbonGalleryButtonState::Normal;
            if(m_up_button_state == wxRibbonGalleryButtonState::Disabled)
                m_up_button_state = wxRibbonGalleryButtonState::Normal;
            return true;
        }
    }
    return false;
}

void wxRibbonGallery::EnsureVisible(const wxRibbonGalleryItem* item)
{
    if(item == nullptr || !item->IsVisible() || IsEmpty())
        return;

    if(m_art->GetFlags() & wxRIBBON_BAR_FLOW_VERTICAL)
    {
        int x = item->GetPosition().GetLeft();
        int base_x = m_items.Item(0)->GetPosition().GetLeft();
        int delta = x - base_x - m_scroll_amount;
        ScrollLines(delta / m_bitmap_padded_size.x);
    }
    else
    {
        int y = item->GetPosition().GetTop();
        int base_y = m_items.Item(0)->GetPosition().GetTop();
        int delta = y - base_y - m_scroll_amount;
        ScrollLines(delta / m_bitmap_padded_size.y);
    }
}

bool wxRibbonGallery::IsHovered() const
{
    return m_hovered;
}

void wxRibbonGallery::OnEraseBackground([[maybe_unused]] wxEraseEvent& evt)
{
    // All painting done in main paint handler to minimise flicker
}

void wxRibbonGallery::OnPaint([[maybe_unused]] wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    if(m_art == nullptr)
        return;

    m_art->DrawGalleryBackground(dc, this, GetSize());

    int padding_top = m_art->GetMetric(wxRIBBON_ART_GALLERY_BITMAP_PADDING_TOP_SIZE);
    int padding_left = m_art->GetMetric(wxRIBBON_ART_GALLERY_BITMAP_PADDING_LEFT_SIZE);

    dc.SetClippingRegion(m_client_rect);

    bool offset_vertical = true;
    if(m_art->GetFlags() & wxRIBBON_BAR_FLOW_VERTICAL)
        offset_vertical = false;
    size_t item_count = m_items.Count();

    for(size_t item_i = 0; item_i < item_count; ++item_i)
    {
        wxRibbonGalleryItem *item = m_items.Item(item_i);
        if(!item->IsVisible())
            continue;

        const wxRect& pos = item->GetPosition();
        wxRect offset_pos(pos);
        if(offset_vertical)
            offset_pos.SetTop(offset_pos.GetTop() - m_scroll_amount);
        else
            offset_pos.SetLeft(offset_pos.GetLeft() - m_scroll_amount);
        m_art->DrawGalleryItemBackground(dc, this, offset_pos, item);
        dc.DrawBitmap(item->GetBitmap(), offset_pos.GetLeft() + padding_left,
            offset_pos.GetTop() + padding_top);
    }
}

void wxRibbonGallery::OnSize([[maybe_unused]] wxSizeEvent& evt)
{
    Layout();
}

wxRibbonGalleryItem* wxRibbonGallery::Append(const wxBitmap& bitmap, int id)
{
    wxASSERT(bitmap.IsOk());
    if(m_items.IsEmpty())
    {
        m_bitmap_size = bitmap.GetScaledSize();
        CalculateMinSize();
    }
    else
    {
        wxASSERT(bitmap.GetScaledSize() == m_bitmap_size);
    }

    wxRibbonGalleryItem *item = new wxRibbonGalleryItem;
    item->SetId(id);
    item->SetBitmap(bitmap);
    m_items.Add(item);
    return item;
}

wxRibbonGalleryItem* wxRibbonGallery::Append(const wxBitmap& bitmap, int id,
                                             void* clientData)
{
    wxRibbonGalleryItem *item = Append(bitmap, id);
    item->SetClientData(clientData);
    return item;
}

wxRibbonGalleryItem* wxRibbonGallery::Append(const wxBitmap& bitmap, int id,
                                             wxClientData* clientData)
{
    wxRibbonGalleryItem *item = Append(bitmap, id);
    item->SetClientObject(clientData);
    return item;
}

void wxRibbonGallery::Clear()
{
    size_t item_count = m_items.Count();

    for(size_t item_i = 0; item_i < item_count; ++item_i)
    {
        wxRibbonGalleryItem *item = m_items.Item(item_i);
        delete item;
    }
    m_items.Clear();
}

bool wxRibbonGallery::IsSizingContinuous() const
{
    return false;
}

void wxRibbonGallery::CalculateMinSize()
{
    if(m_art == nullptr || !m_bitmap_size.IsFullySpecified())
    {
        SetMinSize(wxSize(20, 20));
    }
    else
    {
        m_bitmap_padded_size = m_bitmap_size;
        m_bitmap_padded_size.IncBy(
            m_art->GetMetric(wxRIBBON_ART_GALLERY_BITMAP_PADDING_LEFT_SIZE) +
            m_art->GetMetric(wxRIBBON_ART_GALLERY_BITMAP_PADDING_RIGHT_SIZE),
            m_art->GetMetric(wxRIBBON_ART_GALLERY_BITMAP_PADDING_TOP_SIZE) +
            m_art->GetMetric(wxRIBBON_ART_GALLERY_BITMAP_PADDING_BOTTOM_SIZE));

        wxMemoryDC dc;
        SetMinSize(m_art->GetGallerySize(dc, this, m_bitmap_padded_size));

        // The best size is displaying several items
        m_best_size = m_bitmap_padded_size;
        m_best_size.x *= 3;
        m_best_size = m_art->GetGallerySize(dc, this, m_best_size);
    }
}

bool wxRibbonGallery::Realize()
{
    CalculateMinSize();
    return Layout();
}

bool wxRibbonGallery::Layout()
{
    if(m_art == nullptr)
        return false;

    wxMemoryDC dc;
    wxPoint origin;
    wxSize client_size = m_art->GetGalleryClientSize(dc, this, GetSize(),
        &origin, &m_scroll_up_button_rect, &m_scroll_down_button_rect,
        &m_extension_button_rect);
    m_client_rect = wxRect(origin, client_size);

    int x_cursor = 0;
    int y_cursor = 0;

    size_t item_count = m_items.Count();

    long art_flags = m_art->GetFlags();

    size_t item_i;

    for(item_i = 0; item_i < item_count; ++item_i)
    {
        wxRibbonGalleryItem *item = m_items.Item(item_i);
        item->SetIsVisible(true);
        if(art_flags & wxRIBBON_BAR_FLOW_VERTICAL)
        {
            if(y_cursor + m_bitmap_padded_size.y > client_size.y)
            {
                if(y_cursor == 0)
                    break;
                y_cursor = 0;
                x_cursor += m_bitmap_padded_size.x;
            }
            item->SetPosition(origin.x + x_cursor, origin.y + y_cursor,
                m_bitmap_padded_size);
            y_cursor += m_bitmap_padded_size.y;
        }
        else
        {
            if(x_cursor + m_bitmap_padded_size.x > client_size.x)
            {
                if(x_cursor == 0)
                    break;
                x_cursor = 0;
                y_cursor += m_bitmap_padded_size.y;
            }
            item->SetPosition(origin.x + x_cursor, origin.y + y_cursor,
                m_bitmap_padded_size);
            x_cursor += m_bitmap_padded_size.x;
        }
    }
    for(; item_i < item_count; ++item_i)
    {
        wxRibbonGalleryItem *item = m_items.Item(item_i);
        item->SetIsVisible(false);
    }
    if(art_flags & wxRIBBON_BAR_FLOW_VERTICAL)
        m_scroll_limit = x_cursor;
    else
        m_scroll_limit = y_cursor;
    if(m_scroll_amount >= m_scroll_limit)
    {
        m_scroll_amount = m_scroll_limit;
        m_down_button_state = wxRibbonGalleryButtonState::Disabled;
    }
    else if(m_down_button_state == wxRibbonGalleryButtonState::Disabled)
        m_down_button_state = wxRibbonGalleryButtonState::Normal;

    if(m_scroll_amount <= 0)
    {
        m_scroll_amount = 0;
        m_up_button_state = wxRibbonGalleryButtonState::Disabled;
    }
    else if(m_up_button_state == wxRibbonGalleryButtonState::Disabled)
        m_up_button_state = wxRibbonGalleryButtonState::Normal;

    return true;
}

wxSize wxRibbonGallery::DoGetBestSize() const
{
    return m_best_size;
}

wxSize wxRibbonGallery::DoGetNextSmallerSize(wxOrientation direction,
                                        wxSize relative_to) const
{
    if(m_art == nullptr)
        return relative_to;

    wxMemoryDC dc;

    wxSize client = m_art->GetGalleryClientSize(dc, this, relative_to, nullptr,
        nullptr, nullptr, nullptr);
    switch(direction)
    {
    case wxHORIZONTAL:
        client.DecBy(1, 0);
        break;
    case wxVERTICAL:
        client.DecBy(0, 1);
        break;
    case wxBOTH:
        client.DecBy(1, 1);
        break;
    }
    if(client.x < 0 || client.y < 0)
        return relative_to;

    client.x = (client.x / m_bitmap_padded_size.x) * m_bitmap_padded_size.x;
    client.y = (client.y / m_bitmap_padded_size.y) * m_bitmap_padded_size.y;

    wxSize size = m_art->GetGallerySize(dc, this, client);
    wxSize minimum = GetMinSize();

    if(size.x < minimum.x ||
        size.y < minimum.y)
    {
        return relative_to;
    }

    switch(direction)
    {
    case wxHORIZONTAL:
        size.y = relative_to.y;
        break;
    case wxVERTICAL:
        size.x = relative_to.x;
        break;
    default:
        break;
    }

    return size;
}

wxSize wxRibbonGallery::DoGetNextLargerSize(wxOrientation direction,
                                       wxSize relative_to) const
{
    if(m_art == nullptr)
        return relative_to;

    wxMemoryDC dc;

    wxSize client = m_art->GetGalleryClientSize(dc, this, relative_to, nullptr,
        nullptr, nullptr, nullptr);

    // No need to grow if the given size can already display every item
    int nitems = (client.x / m_bitmap_padded_size.x) *
        (client.y / m_bitmap_padded_size.y);
    if(nitems >= (int)m_items.GetCount())
        return relative_to;

    switch(direction)
    {
    case wxHORIZONTAL:
        client.IncBy(m_bitmap_padded_size.x, 0);
        break;
    case wxVERTICAL:
        client.IncBy(0, m_bitmap_padded_size.y);
        break;
    case wxBOTH:
        client.IncBy(m_bitmap_padded_size);
        break;
    }

    client.x = (client.x / m_bitmap_padded_size.x) * m_bitmap_padded_size.x;
    client.y = (client.y / m_bitmap_padded_size.y) * m_bitmap_padded_size.y;

    wxSize size = m_art->GetGallerySize(dc, this, client);
    wxSize minimum = GetMinSize();

    if(size.x < minimum.x ||
        size.y < minimum.y)
    {
        return relative_to;
    }

    switch(direction)
    {
    case wxHORIZONTAL:
        size.y = relative_to.y;
        break;
    case wxVERTICAL:
        size.x = relative_to.x;
        break;
    default:
        break;
    }

    return size;
}

bool wxRibbonGallery::IsEmpty() const
{
    return m_items.IsEmpty();
}

unsigned int wxRibbonGallery::GetCount() const
{
    return (unsigned int)m_items.GetCount();
}

wxRibbonGalleryItem* wxRibbonGallery::GetItem(unsigned int n)
{
    if(n >= GetCount())
        return nullptr;
    return m_items.Item(n);
}

void wxRibbonGallery::SetSelection(wxRibbonGalleryItem* item)
{
    if(item != m_selected_item)
    {
        m_selected_item = item;
        Refresh(false);
    }
}

wxRibbonGalleryItem* wxRibbonGallery::GetSelection() const
{
    return m_selected_item;
}

wxRibbonGalleryItem* wxRibbonGallery::GetHoveredItem() const
{
    return m_hovered_item;
}

wxRibbonGalleryItem* wxRibbonGallery::GetActiveItem() const
{
    return m_active_item;
}

wxRibbonGalleryButtonState wxRibbonGallery::GetUpButtonState() const
{
    return m_up_button_state;
}

wxRibbonGalleryButtonState wxRibbonGallery::GetDownButtonState() const
{
    return m_down_button_state;
}

wxRibbonGalleryButtonState wxRibbonGallery::GetExtensionButtonState() const
{
    return m_extension_button_state;
}

#endif // wxUSE_RIBBON
