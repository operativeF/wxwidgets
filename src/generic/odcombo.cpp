/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/odcombo.cpp
// Purpose:     wxOwnerDrawnComboBox, wxVListBoxComboPopup
// Author:      Jaakko Salli
// Modified by:
// Created:     Apr-30-2006
// Copyright:   (c) 2005 Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_ODCOMBOBOX

#include "wx/odcombo.h"

#include "wx/combobox.h"
#include "wx/dcclient.h"
#include "wx/dialog.h"
#include "wx/textctrl.h"
#include "wx/utils.h"

#include "wx/combo.h"

#include <chrono>

import WX.Utils.Settings;

import <utility>;

// ============================================================================
// implementation
// ============================================================================

using namespace std::chrono_literals;

// time in milliseconds before partial completion buffer drops
constexpr auto wxODCB_PARTIAL_COMPLETION_TIME = 1000ms;

// ----------------------------------------------------------------------------
// wxVListBoxComboPopup is a wxVListBox customized to act as a popup control
//
// ----------------------------------------------------------------------------


wxBEGIN_EVENT_TABLE(wxVListBoxComboPopup, wxVListBox)
    EVT_MOTION(wxVListBoxComboPopup::OnMouseMove)
    EVT_KEY_DOWN(wxVListBoxComboPopup::OnKey)
    EVT_CHAR(wxVListBoxComboPopup::OnChar)
    EVT_LEFT_UP(wxVListBoxComboPopup::OnLeftClick)
wxEND_EVENT_TABLE()

bool wxVListBoxComboPopup::Create(wxWindow* parent)
{
    if ( !wxVListBox::Create(parent,
                             wxID_ANY,
                             wxDefaultPosition,
                             wxDefaultSize,
                             wxBORDER_SIMPLE | wxLB_INT_HEIGHT | wxWANTS_CHARS) )
        return false;

    m_useFont = m_combo->GetFont();

    wxVListBox::SetItemCount(m_strings.size());

    // TODO: Move this to SetFont
    m_itemHeight = m_combo->GetCharHeight();

    // Bind to the DPI event of the combobox. We get our own once the popup
    // is shown, but this is too late, m_itemHeight is already being used.
    m_combo->Bind(wxEVT_DPI_CHANGED, &wxVListBoxComboPopup::OnDPIChanged, this);

    return true;
}

wxVListBoxComboPopup::~wxVListBoxComboPopup()
{
    Clear();
}

void wxVListBoxComboPopup::SetFocus()
{
    // Suppress SetFocus() warning by simply not calling it. This combo popup
    // has already been designed with the assumption that SetFocus() may not
    // do anything useful, so it really doesn't need to be called.
#ifdef __WXMSW__
    //
#else
    wxVListBox::SetFocus();
#endif
}

void wxVListBoxComboPopup::OnDPIChanged([[maybe_unused]] wxDPIChangedEvent& event)
{
    m_itemHeight = m_combo->GetCharHeight();
}

bool wxVListBoxComboPopup::LazyCreate()
{
    // NB: There is a bug with wxVListBox that can be avoided by creating
    //     it later (bug causes empty space to be shown if initial selection
    //     is at the end of a list longer than the control can show at once).
    return true;
}

// paint the control itself
void wxVListBoxComboPopup::PaintComboControl( wxDC& dc, const wxRect& rect )
{
    if ( !(m_combo->wxGetWindowStyle() & wxODCB_STD_CONTROL_PAINT) )
    {
        unsigned int flags = wxODCB_PAINTING_CONTROL;

        if ( m_combo->ShouldDrawFocus() )
            flags |= wxODCB_PAINTING_SELECTED;

        OnDrawBg(dc, rect, m_value, flags);

        if ( m_value >= 0 )
        {
            OnDrawItem(dc,rect,m_value,flags);
            return;
        }
    }

    wxComboPopup::PaintComboControl(dc,rect);
}

void wxVListBoxComboPopup::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
    // TODO: Maybe this code could be moved to wxVListBox::OnPaint?
    dc.SetFont(m_useFont);

    unsigned int flags{};

    // Set correct text colour for selected items
    if ( wxVListBox::GetSelection() == (int) n )
    {
        dc.SetTextForeground( wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );
        flags |= wxODCB_PAINTING_SELECTED;
    }
    else
    {
        dc.SetTextForeground( wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT) );
    }

    OnDrawItem(dc, rect, (int)n, flags);
}

wxCoord wxVListBoxComboPopup::OnMeasureItem(size_t n) const
{
    wxOwnerDrawnComboBox* combo = (wxOwnerDrawnComboBox*) m_combo;

    wxASSERT_MSG( dynamic_cast<wxOwnerDrawnComboBox*>(combo),
                  "you must subclass wxVListBoxComboPopup for drawing and measuring methods" );

    wxCoord h = combo->OnMeasureItem(n);
    if ( h < 0 )
        h = m_itemHeight;
    return h;
}

wxCoord wxVListBoxComboPopup::OnMeasureItemWidth(size_t n) const
{
    wxOwnerDrawnComboBox* combo = (wxOwnerDrawnComboBox*) m_combo;

    wxASSERT_MSG( dynamic_cast<wxOwnerDrawnComboBox*>(combo),
                  "you must subclass wxVListBoxComboPopup for drawing and measuring methods" );

    return combo->OnMeasureItemWidth(n);
}

void wxVListBoxComboPopup::OnDrawBg( wxDC& dc,
                                     const wxRect& rect,
                                     int item,
                                     unsigned int flags ) const
{
    wxOwnerDrawnComboBox* combo = (wxOwnerDrawnComboBox*) m_combo;

    wxASSERT_MSG( dynamic_cast<wxOwnerDrawnComboBox*>(combo),
                  "you must subclass wxVListBoxComboPopup for drawing and measuring methods" );

    if ( IsCurrent((size_t)item) && !(flags & wxODCB_PAINTING_CONTROL) )
        flags |= wxODCB_PAINTING_SELECTED;

    combo->OnDrawBackground(dc,rect,item,flags);
}

void wxVListBoxComboPopup::OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const
{
    OnDrawBg(dc,rect,(int)n,0);
}

// This is called from wxVListBoxComboPopup::OnDrawItem, with text colour and font prepared
void wxVListBoxComboPopup::OnDrawItem( wxDC& dc, const wxRect& rect, int item, unsigned int flags ) const
{
    wxOwnerDrawnComboBox* combo = (wxOwnerDrawnComboBox*) m_combo;

    wxASSERT_MSG( dynamic_cast<wxOwnerDrawnComboBox*>(combo),
                  "you must subclass wxVListBoxComboPopup for drawing and measuring methods" );

    combo->OnDrawItem(dc,rect,item,flags);
}

void wxVListBoxComboPopup::DismissWithEvent()
{
    StopPartialCompletion();

    int selection = wxVListBox::GetSelection();

    if ( selection != wxNOT_FOUND )
        m_stringValue = m_strings[selection];
    else
        m_stringValue.clear();

    m_value = selection;

    Dismiss();

    SendComboBoxEvent(selection);
}

void wxVListBoxComboPopup::SendComboBoxEvent( int selection )
{
    // TODO: wxVListBox should be refactored to inherit from wxItemContainer
    //       and then we would be able to just call SendSelectionChangedEvent()
    //       (which, itself, should be moved down to wxItemContainer from
    //       wxControlWithItemsBase) instead of duplicating its code.

    wxCommandEvent evt(wxEVT_COMBOBOX,m_combo->GetId());

    evt.SetEventObject(m_combo);

    evt.SetInt(selection);
    if ( selection != wxNOT_FOUND )
        evt.SetString(m_strings[selection]);

    // Set client data, if any
    if ( selection >= 0 && (int)m_clientDatas.size() > selection )
    {
        void* clientData = m_clientDatas[selection];
        if ( m_clientDataItemsType == wxClientDataType::Object )
            evt.SetClientObject((wxClientData*)clientData);
        else
            evt.SetClientData(clientData);
    }

    m_combo->GetEventHandler()->AddPendingEvent(evt);
}

// returns true if key was consumed
bool wxVListBoxComboPopup::HandleKey( int keycode, bool saturate, char keychar )
{
    const int itemCount = GetCount();

    // keys do nothing in the empty control and returning immediately avoids
    // using invalid indices below
    if ( !itemCount )
        return false;

    int value = m_value;
    int comboStyle = m_combo->wxGetWindowStyle();

    if ( keychar > 0 )
    {
        // we have character equivalent of the keycode; filter out these that
        // are not printable characters
        if ( !wxIsprint(keychar) )
            keychar = 0;
    }

    const bool readOnly = (comboStyle & wxCB_READONLY) != 0;

    if ( keycode == WXK_DOWN || keycode == WXK_NUMPAD_DOWN || ( keycode == WXK_RIGHT && readOnly ) )
    {
        value++;
        StopPartialCompletion();
    }
    else if ( keycode == WXK_UP || keycode == WXK_NUMPAD_UP || ( keycode == WXK_LEFT && readOnly ) )
    {
        value--;
        StopPartialCompletion();
    }
    else if ( keycode == WXK_PAGEDOWN || keycode == WXK_NUMPAD_PAGEDOWN )
    {
        value+=10;
        StopPartialCompletion();
    }
    else if ( keycode == WXK_PAGEUP || keycode == WXK_NUMPAD_PAGEUP )
    {
        value-=10;
        StopPartialCompletion();
    }
    else if ( ( keycode == WXK_HOME || keycode == WXK_NUMPAD_HOME ) && readOnly )
    {
        value=0;
        StopPartialCompletion();
    }
    else if ( ( keycode == WXK_END || keycode == WXK_NUMPAD_END ) && readOnly )
    {
        value=itemCount-1;
        StopPartialCompletion();
    }
    else if ( keychar && readOnly )
    {
        // Try partial completion

        // find the new partial completion string
#if wxUSE_TIMER
        if (m_partialCompletionTimer.IsRunning())
            m_partialCompletionString += fmt::format("{}", keychar);
        else
#endif // wxUSE_TIMER
            m_partialCompletionString = fmt::format("{}", keychar);

        // now search through the values to see if this is found
        int found = -1;
        unsigned int length=m_partialCompletionString.length();
        int i;
        for (i=0; i<itemCount; i++)
        {
            std::string item = GetString(i);
            if (( item.length() >= length) && (!wx::utils::CmpNoCase(m_partialCompletionString, item.substr(0, length))))
            {
                found = i;
                break;
            }
        }

        if (found<0)
        {
            StopPartialCompletion();
            ::wxBell();
            return true; // to stop the first value being set
        }
        else
        {
            value=i;
#if wxUSE_TIMER
            m_partialCompletionTimer.Start(wxODCB_PARTIAL_COMPLETION_TIME, true);
#endif // wxUSE_TIMER
        }
    }
    else
        return false;

    if ( saturate )
    {
        if ( value >= itemCount )
            value = itemCount - 1;
        else if ( value < 0 )
            value = 0;
    }
    else
    {
        if ( value >= itemCount )
            value -= itemCount;
        else if ( value < 0 )
            value += itemCount;
    }

    if ( value == m_value )
        // Even if value was same, don't skip the event
        // (good for consistency)
        return true;

    if ( value >= 0 )
        m_combo->ChangeValue(m_strings[value]);

    // The m_combo->SetValue() call above sets m_value to the index of this
    // string. But if there are more identical string, the index is of the
    // first occurrence, which may be wrong, so set the index explicitly here,
    // _after_ the SetValue() call.
    m_value = value;

    SendComboBoxEvent(m_value);

    return true;
}

// stop partial completion
void wxVListBoxComboPopup::StopPartialCompletion()
{
    m_partialCompletionString.clear();
#if wxUSE_TIMER
    m_partialCompletionTimer.Stop();
#endif // wxUSE_TIMER
}

void wxVListBoxComboPopup::OnComboDoubleClick()
{
    // Cycle on dclick (disable saturation to allow true cycling).
    if ( !::wxGetKeyState(WXK_SHIFT) )
        HandleKey(WXK_DOWN,false);
    else
        HandleKey(WXK_UP,false);
}

void wxVListBoxComboPopup::OnComboKeyEvent( wxKeyEvent& event )
{
    // Saturated key movement on
    if ( !HandleKey(event.GetKeyCode(), true) )
        event.Skip();
}

void wxVListBoxComboPopup::OnComboCharEvent( wxKeyEvent& event )
{
    // unlike in OnComboKeyEvent, wxEVT_CHAR contains meaningful
    // printable character information, so pass it
    const wxChar charcode = event.GetUnicodeKey();

    if ( !HandleKey(event.GetKeyCode(), true, charcode) )
        event.Skip();
}

void wxVListBoxComboPopup::OnPopup()
{
    // *must* set value after size is set (this is because of a vlbox bug)
    wxVListBox::SetSelection(m_value);
}

void wxVListBoxComboPopup::OnMouseMove(wxMouseEvent& event)
{
    event.Skip();

    // Move selection to cursor if it is inside the popup

    int y = event.GetPosition().y;
    int fromBottom = GetClientSize().y - y;

    // Since in any case we need to find out if the last item is only
    // partially visible, we might just as well replicate the HitTest
    // loop here.
    const size_t lineMax = GetVisibleEnd();
    for ( size_t line = GetVisibleBegin(); line < lineMax; line++ )
    {
        y -= OnGetRowHeight(line);
        if ( y < 0 )
        {
            // Only change selection if item is fully visible
            if ( (y + fromBottom) >= 0 )
            {
                wxVListBox::SetSelection((int)line);
                return;
            }
        }
    }
}

void wxVListBoxComboPopup::OnLeftClick([[maybe_unused]] wxMouseEvent& event)
{
    DismissWithEvent();
}

void wxVListBoxComboPopup::OnKey(wxKeyEvent& event)
{
    // Hide popup if certain key or key combination was pressed
    if ( m_combo->IsKeyPopupToggle(event) )
    {
        StopPartialCompletion();
        Dismiss();
    }
    else if ( event.AltDown() )
    {
        // On both wxGTK and wxMSW, pressing Alt down seems to
        // completely freeze things in popup (ie. arrow keys and
        // enter won't work).
        return;
    }
    // Select item if ENTER is pressed
    else if ( event.GetKeyCode() == WXK_RETURN || event.GetKeyCode() == WXK_NUMPAD_ENTER )
    {
        DismissWithEvent();
    }
    else
    {
        // completion is handled in OnChar() below
        event.Skip();
    }
}

void wxVListBoxComboPopup::OnChar(wxKeyEvent& event)
{
    if ( m_combo->wxGetWindowStyle() & wxCB_READONLY )
    {
        // Process partial completion key codes here, but not the arrow keys as
        // the base class will do that for us
        const wxChar charcode = event.GetUnicodeKey();

        if ( wxIsprint(charcode) )
        {
            OnComboCharEvent(event);
            SetSelection(m_value); // ensure the highlight bar moves
            return; // don't skip the event
        }
    }

    event.Skip();
}

void wxVListBoxComboPopup::Insert( const std::string& item, int pos )
{
    // Need to change selection?
    if ( m_combo->GetValue() == item )
    {
        m_value = std::min(m_value, pos);
    }
    else if ( pos <= m_value )
    {
        m_value++;
    }

    m_strings.insert(std::cbegin(m_strings) + pos, item);
    if ( (int)m_clientDatas.size() >= pos )
        m_clientDatas.insert(m_clientDatas.begin()+pos, NULL);

    m_widths.insert(m_widths.begin()+pos, -1);
    m_widthsDirty = true;

    if ( IsCreated() )
        wxVListBox::SetItemCount( wxVListBox::GetItemCount()+1 );
}

int wxVListBoxComboPopup::Append(const std::string& item)
{
    int pos = (int)m_strings.size();

    if ( m_combo->wxGetWindowStyle() & wxCB_SORT )
    {
        // Find position
        // TODO: Could be optimized with binary search
        const std::vector<std::string>& strings = m_strings;
        for ( size_t i=0; i < strings.size(); i++ )
        {
            if ( wx::utils::CmpNoCase(item, strings[i]) <= 0 )
            {
                pos = (int)i;
                break;
            }
        }
    }

    Insert(item,pos);

    return pos;
}

void wxVListBoxComboPopup::Clear()
{
    wxASSERT(m_combo);

    m_strings.clear();
    m_widths.clear();

    m_widestWidth = 0;
    m_widestItem = -1;

    ClearClientDatas();

    m_value = wxNOT_FOUND;

    if ( IsCreated() )
        wxVListBox::SetItemCount(0);
}

void wxVListBoxComboPopup::ClearClientDatas()
{
    if ( m_clientDataItemsType == wxClientDataType::Object )
    {
        for ( std::vector<void*>::iterator it = m_clientDatas.begin(); it != m_clientDatas.end(); ++it )
            delete (wxClientData*) *it;
    }

    m_clientDatas.clear();
    m_clientDataItemsType = wxClientDataType::None;
}

void wxVListBoxComboPopup::SetItemClientData( unsigned int n,
                                              void* clientData,
                                              wxClientDataType clientDataItemsType )
{
    // It should be sufficient to update this variable only here
    m_clientDataItemsType = clientDataItemsType;

    m_clientDatas[n] = clientData;

    ItemWidthChanged(n);
}

void* wxVListBoxComboPopup::GetItemClientData(unsigned int n) const
{
    return n < m_clientDatas.size() ? m_clientDatas[n] : NULL;
}

void wxVListBoxComboPopup::Delete( unsigned int item )
{
    // Remove client data, if set
    if ( !m_clientDatas.empty() )
    {
        if ( m_clientDataItemsType == wxClientDataType::Object )
            delete (wxClientData*) m_clientDatas[item];

        m_clientDatas.erase(m_clientDatas.begin()+item);
    }

    m_strings.erase(m_strings.begin() + item);
    m_widths.erase(m_widths.begin() + item);

    if ( (int)item == m_widestItem )
        m_findWidest = true;

    int sel = GetSelection();

    if ( IsCreated() )
        wxVListBox::SetItemCount( wxVListBox::GetItemCount()-1 );

    // Fix selection
    if ( (int)item < sel )
        SetSelection(sel-1);
    else if ( (int)item == sel )
        SetSelection(wxNOT_FOUND);
}

int wxVListBoxComboPopup::FindString(std::string_view s, bool bCase) const
{
    // FIXME: Stupid.
    const auto possibleMatch = std::ranges::find_if(m_strings.cbegin(), m_strings.cend(),
        [s, bCase](const auto& str){
            if(!bCase)
            {
                return wx::utils::IsSameAsNoCase(s, str);
            }
            else
            {
                return wx::utils::IsSameAsCase(s, str);
            }
    });

    if(possibleMatch == m_strings.cend())
    {
        return wxNOT_FOUND;
    }
    else
    {
        return std::distance(std::cbegin(m_strings), possibleMatch);
    }
}

// TODO: Make test explicitly for this.
// item.IsSameAs(str, false)
bool wxVListBoxComboPopup::FindItem(std::string_view item, std::string* trueItem)
{
    int idx = std::ranges::find_if(m_strings,
        [item](const auto& str){
            return wx::utils::IsSameAsNoCase(item, str);
    }) - m_strings.cbegin();

    // TODO: Verify this.
    if (idx == m_strings.size())
        return false;
    if ( trueItem != nullptr )
        *trueItem = m_strings[idx];
    return true;
}

// TODO: Mismatch type return
unsigned int wxVListBoxComboPopup::GetCount() const
{
    return m_strings.size();
}

std::string wxVListBoxComboPopup::GetString( int item ) const
{
    return m_strings[item];
}

void wxVListBoxComboPopup::SetString( int item, const std::string& str )
{
    m_strings[item] = str;
    ItemWidthChanged(item);
}

std::string wxVListBoxComboPopup::GetStringValue() const
{
    return m_stringValue;
}

void wxVListBoxComboPopup::SetSelection( int item )
{
    wxCHECK_RET( item == wxNOT_FOUND || ((unsigned int)item < GetCount()),
                 "invalid index in wxVListBoxComboPopup::SetSelection" );

    m_value = item;

    if ( item >= 0 )
        m_stringValue = m_strings[item];
    else
        m_stringValue.clear();

    if ( IsCreated() )
        wxVListBox::SetSelection(item);
}

int wxVListBoxComboPopup::GetSelection() const
{
    return m_value;
}

void wxVListBoxComboPopup::SetStringValue( const std::string& value )
{
    const auto index = std::ranges::find(m_strings, value);

    m_stringValue = value;

    if ( index != std::cend(m_strings) &&
       ( std::cmp_less((index - std::cbegin(m_strings)), wxVListBox::GetItemCount())))
    {
        wxVListBox::SetSelection(index - std::cbegin(m_strings));
        m_value = index - m_strings.cbegin();
    }
}

void wxVListBoxComboPopup::CalcWidths()
{
    bool doFindWidest = m_findWidest;

    // Measure items with dirty width.
    if ( m_widthsDirty )
    {
        unsigned int n = m_widths.size();
        int dirtyHandled = 0;
        std::vector<int>& widths = m_widths;

        // I think using wxDC::GetTextExtent is faster than
        // wxWindow::GetTextExtent (assuming same dc is used
        // for all calls, as we do here).
        wxClientDC dc(m_combo);
        if ( !m_useFont.IsOk() )
            m_useFont = m_combo->GetFont();
        dc.SetFont(m_useFont);

        for ( unsigned int i=0; i<n; i++ )
        {
            if ( widths[i] < 0 )
            {
                wxCoord x = OnMeasureItemWidth(i);

                if ( x < 0 )
                {
                    const std::string& text = m_strings[i];

                    // To make sure performance won't suck in extreme scenarios,
                    // we'll estimate length after some arbitrary number of items
                    // have been checked precily.
                    if ( dirtyHandled < 1024 )
                    {
                        x = dc.GetTextExtent(text, nullptr, nullptr).x + 4;
                    }
                    else
                    {
                        x = text.length() * (dc.wxGetCharWidth() + 1);
                    }
                }

                widths[i] = x;

                if ( x >= m_widestWidth )
                {
                    m_widestWidth = x;
                    m_widestItem = (int)i;
                }
                else if ( (int)i == m_widestItem )
                {
                    // Width of previously widest item has been decreased, so
                    // we'll have to check all to find current widest item.
                    doFindWidest = true;
                }

                dirtyHandled++;
            }
        }

        m_widthsDirty = false;
    }

    if ( doFindWidest )
    {
        unsigned int n = m_widths.size();

        int bestWidth = -1;
        int bestIndex = -1;

        for ( unsigned int i=0; i<n; i++ )
        {
            int w = m_widths[i];
            if ( w > bestWidth )
            {
                bestIndex = (int)i;
                bestWidth = w;
            }
        }

        m_widestWidth = bestWidth;
        m_widestItem = bestIndex;

        m_findWidest = false;
    }
}

wxSize wxVListBoxComboPopup::GetAdjustedSize( int minWidth, int prefHeight, int maxHeight )
{
    int height = 250;

    maxHeight -= 2;  // Must take borders into account

    if ( !m_strings.empty() )
    {
        if ( prefHeight > 0 )
            height = prefHeight;

        if ( height > maxHeight )
            height = maxHeight;

        int totalHeight = GetTotalHeight(); // + 3;

        // Take borders into account on Mac or scrollbars always appear
#if defined(__WXMAC__)
        totalHeight += 2;
#endif
        if ( height >= totalHeight )
        {
            height = totalHeight;
        }
        else
        {
            // Adjust height to a multiple of the height of the first item
            // NB: Calculations that take variable height into account
            //     are unnecessary.
            int fih = GetLineHeight(0);
            height -= height % fih;
        }
    }
    else
        height = 50;

    CalcWidths();

    // Take scrollbar into account in width calculations
    int widestWidth = m_widestWidth + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this);
    return {minWidth > widestWidth ? minWidth : widestWidth, height + 2};
}

void wxVListBoxComboPopup::Populate( const std::vector<std::string>& choices )
{
    int n = choices.size();

    for ( int i=0; i<n; i++ )
    {
        const std::string& item = choices[i];
        m_strings.push_back(item);
    }

    m_widths.resize(n,-1);
    m_widthsDirty = true;

    if ( IsCreated() )
        wxVListBox::SetItemCount(n);

    // Sort the initial choices
    if ( m_combo->wxGetWindowStyle() & wxCB_SORT )
        std::ranges::sort(m_strings);
    // Find initial selection
    auto strValue = m_combo->GetValue();

    if ( !strValue.empty() )
        m_value = std::ranges::find(m_strings, strValue) - m_strings.cbegin();
}

// ----------------------------------------------------------------------------
// wxOwnerDrawnComboBox
// ----------------------------------------------------------------------------


wxBEGIN_EVENT_TABLE(wxOwnerDrawnComboBox, wxComboCtrl)
wxEND_EVENT_TABLE()



bool wxOwnerDrawnComboBox::Create(wxWindow *parent,
                                  wxWindowID id,
                                  const std::string& value,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  unsigned int style,
                                  const wxValidator& validator,
                                  std::string_view name)
{
    return wxComboCtrl::Create(parent,id,value,pos,size,style,validator,name);
}

wxOwnerDrawnComboBox::wxOwnerDrawnComboBox(wxWindow *parent,
                                           wxWindowID id,
                                           const std::string& value,
                                           const wxPoint& pos,
                                           const wxSize& size,
                                           const std::vector<std::string>& choices,
                                           unsigned int style,
                                           const wxValidator& validator,
                                           std::string_view name)
{
    


    Create(parent,id,value,pos,size,choices,style, validator, name);
}

bool wxOwnerDrawnComboBox::Create(wxWindow *parent,
                                  wxWindowID id,
                                  const std::string& value,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  const std::vector<std::string>& choices,
                                  unsigned int style,
                                  const wxValidator& validator,
                                  std::string_view name)
{
    m_initChs = choices;
    //wxCArrayString chs(choices);

    //return Create(parent, id, value, pos, size, chs.GetCount(),
    //              chs.GetStrings(), style, validator, name);
    return Create(parent, id, value, pos, size, 0,
                  nullptr, style, validator, name);
}

bool wxOwnerDrawnComboBox::Create(wxWindow *parent,
                                  wxWindowID id,
                                  const std::string& value,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  int n,
                                  const std::string choices[],
                                  unsigned int style,
                                  const wxValidator& validator,
                                  std::string_view name)
{

    if ( !Create(parent, id, value, pos, size, style,
                 validator, name) )
    {
        return false;
    }

    for ( int i=0; i<n; i++ )
        m_initChs.push_back(choices[i]);

    return true;
}

wxOwnerDrawnComboBox::~wxOwnerDrawnComboBox()
{
    if ( m_popupInterface )
        GetVListBoxComboPopup()->ClearClientDatas();
}

void wxOwnerDrawnComboBox::DoSetPopupControl(wxComboPopup* popup)
{
    if ( !popup )
    {
        popup = new wxVListBoxComboPopup();
    }

    wxComboCtrl::DoSetPopupControl(popup);

    wxASSERT(popup);

    // Add initial choices to the wxVListBox
    if ( !GetVListBoxComboPopup()->GetCount() )
    {
        GetVListBoxComboPopup()->Populate(m_initChs);
        m_initChs.clear();
    }
}

// ----------------------------------------------------------------------------
// wxOwnerDrawnComboBox item manipulation methods
// ----------------------------------------------------------------------------

void wxOwnerDrawnComboBox::DoClear()
{
    EnsurePopupControl();

    GetVListBoxComboPopup()->Clear();

    // There is no text entry when using wxCB_READONLY style, so test for it.
    if ( GetTextCtrl() )
        wxTextEntry::Clear();
}

void wxOwnerDrawnComboBox::Clear()
{
    DoClear();
    SetClientDataType(wxClientDataType::None);
}

void wxOwnerDrawnComboBox::DoDeleteOneItem(unsigned int n)
{
    wxCHECK_RET( IsValid(n), "invalid index in wxOwnerDrawnComboBox::Delete" );

    if ( GetSelection() == (int) n )
        ChangeValue("");

    GetVListBoxComboPopup()->Delete(n);
}

// TODO: Type mismatch
size_t wxOwnerDrawnComboBox::GetCount() const
{
    if ( !m_popupInterface )
        return m_initChs.size();

    return GetVListBoxComboPopup()->GetCount();
}

std::string wxOwnerDrawnComboBox::GetString(unsigned int n) const
{
    wxCHECK_MSG( IsValid(n), "", "invalid index in wxOwnerDrawnComboBox::GetString" );

    if ( !m_popupInterface )
        return m_initChs[n];

    return GetVListBoxComboPopup()->GetString(n);
}

void wxOwnerDrawnComboBox::SetString(unsigned int n, const std::string& s)
{
    EnsurePopupControl();

    wxCHECK_RET( IsValid(n), "invalid index in wxOwnerDrawnComboBox::SetString" );

    GetVListBoxComboPopup()->SetString(n,s);
}

int wxOwnerDrawnComboBox::FindString(std::string_view s, bool bCase) const
{
    // TODO: OOB?
    if ( !m_popupInterface )
        return std::ranges::find_if(m_initChs,
            [=](std::string_view str){
                return wxItemContainerImmutable::FindString(str, bCase);
        }) - m_initChs.cbegin();

    return GetVListBoxComboPopup()->FindString(s, bCase);
}

void wxOwnerDrawnComboBox::Select(int n)
{
    EnsurePopupControl();

    wxCHECK_RET( (n == wxNOT_FOUND) || IsValid(n), "invalid index in wxOwnerDrawnComboBox::Select" );

    GetVListBoxComboPopup()->SetSelection(n);

    std::string str;
    if ( n >= 0 )
        str = GetVListBoxComboPopup()->GetString(n);

    // Refresh text portion in control
    if ( m_text )
        m_text->ChangeValue( str );
    else
        m_valueString = str;

    Refresh();
}

int wxOwnerDrawnComboBox::GetSelection() const
{
    // TODO: OOB?
    if ( !m_popupInterface )
        return std::ranges::find(m_initChs, m_valueString) - m_initChs.cbegin();

    return GetVListBoxComboPopup()->GetSelection();
}

void wxOwnerDrawnComboBox::GetSelection(long *from, long *to) const
{
    wxComboCtrl::GetSelection(from, to);
}

int wxOwnerDrawnComboBox::DoInsertItems(const std::vector<std::string>& items,
                                        unsigned int pos,
                                        void **clientData,
                                        wxClientDataType type)
{
    EnsurePopupControl();

    const unsigned int count = items.size();

    if ( HasFlag(wxCB_SORT) )
    {
        int n = pos;

        for ( unsigned int i = 0; i < count; ++i )
        {
            n = GetVListBoxComboPopup()->Append(items[i]);
            AssignNewItemClientData(n, clientData, i, type);
        }

        return n;
    }
    else
    {
        for ( unsigned int i = 0; i < count; ++i, ++pos )
        {
            GetVListBoxComboPopup()->Insert(items[i], pos);
            AssignNewItemClientData(pos, clientData, i, type);
        }

        return pos - 1;
    }
}

void wxOwnerDrawnComboBox::DoSetItemClientData(unsigned int n, void* clientData)
{
    EnsurePopupControl();

    GetVListBoxComboPopup()->SetItemClientData(n, clientData,
                                               GetClientDataType());
}

void* wxOwnerDrawnComboBox::DoGetItemClientData(unsigned int n) const
{
    if ( !m_popupInterface )
        return nullptr;

    return GetVListBoxComboPopup()->GetItemClientData(n);
}

// ----------------------------------------------------------------------------
// wxOwnerDrawnComboBox item drawing and measuring default implementations
// ----------------------------------------------------------------------------

void wxOwnerDrawnComboBox::OnDrawItem( wxDC& dc,
                                       const wxRect& rect,
                                       int item,
                                       unsigned int flags ) const
{
    if ( flags & wxODCB_PAINTING_CONTROL )
    {
        std::string text;

        if ( !ShouldUseHintText() )
        {
            text = GetValue();
        }
        else
        {
            text = GetHint();
            wxColour col = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
            dc.SetTextForeground(col);
        }

        dc.wxDrawText( text,
                       wxPoint{rect.x + GetMargins().x,
                               (rect.height-dc.GetCharHeight())/2 + rect.y} );
    }
    else
    {
        dc.wxDrawText( GetVListBoxComboPopup()->GetString(item), wxPoint{rect.x + 2, rect.y} );
    }
}

wxCoord wxOwnerDrawnComboBox::OnMeasureItem( [[maybe_unused]] size_t item ) const
{
    return -1;
}

wxCoord wxOwnerDrawnComboBox::OnMeasureItemWidth( [[maybe_unused]] size_t item ) const
{
    return -1;
}

wxSize wxOwnerDrawnComboBox::DoGetBestSize() const
{
    if ( GetCount() == 0 )
        return wxComboCtrlBase::DoGetBestSize();

    wxOwnerDrawnComboBox* odc = const_cast<wxOwnerDrawnComboBox*>(this);
    // TODO: this class may also have GetHightestItemHeight() and
    // GetHightestItem() methods, and so set the whole (edit part + arrow)
    // control's height according with this max height, not only max width.
    return GetSizeFromTextSize(odc->GetWidestItemWidth());
}

void wxOwnerDrawnComboBox::OnDrawBackground(wxDC& dc,
                                            const wxRect& rect,
                                            [[maybe_unused]] int item,
                                            unsigned int flags) const
{
    // We need only to explicitly draw background for items
    // that should have selected background. Also, call PrepareBackground
    // always when painting the control so that clipping is done properly.

    if ( (flags & wxODCB_PAINTING_SELECTED) ||
         ((flags & wxODCB_PAINTING_CONTROL) && HasFlag(wxCB_READONLY)) )
    {
        int bgFlags = wxCONTROL_SELECTED;

        if ( !(flags & wxODCB_PAINTING_CONTROL) )
            bgFlags |= wxCONTROL_ISSUBMENU;

        PrepareBackground(dc, rect, bgFlags);
    }
}

#endif // wxUSE_ODCOMBOBOX
