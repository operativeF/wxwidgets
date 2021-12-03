///////////////////////////////////////////////////////////////////////////
// Name:        src/generic/grideditors.cpp
// Purpose:     wxGridCellEditorEvtHandler and wxGrid editors
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: Robin Dunn, Vadim Zeitlin, Santiago Palacios
// Created:     1/08/1999
// Copyright:   (c) Michael Bedward (mbedward@ozemail.com.au)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_GRID

#include "wx/grid.h"

#include "wx/utils.h"
#include "wx/dc.h"
#include "wx/settings.h"
#include "wx/log.h"
#include "wx/textctrl.h"
#include "wx/checkbox.h"
#include "wx/combobox.h"
#include "wx/intl.h"

#include "wx/valnum.h"
#include "wx/spinctrl.h"
#include "wx/tokenzr.h"
#include "wx/datectrl.h"

#include "wx/generic/gridsel.h"
#include "wx/generic/grideditors.h"
#include "wx/generic/private/grid.h"

#if defined(__WXMOTIF__)
    #define WXUNUSED_MOTIF(identifier)  WXUNUSED(identifier)
#else
    #define WXUNUSED_MOTIF(identifier)  identifier
#endif

#if defined(__WXGTK__)
    #define WXUNUSED_GTK(identifier)    WXUNUSED(identifier)
#else
    #define WXUNUSED_GTK(identifier)    identifier
#endif

#ifdef __WXOSX__
#include "wx/osx/private.h"
#endif

// Required for wxIs... functions
import <cctype>;
import <charconv>;

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxGridCellEditorEvtHandler
// ----------------------------------------------------------------------------

void wxGridCellEditorEvtHandler::DismissEditor()
{
    // Tell the grid to dismiss the control but don't do it immediately as it
    // could result in the editor being destroyed right now and a crash in the
    // code searching for the next event handler, so tell the grid to close it
    // after this event is processed.
    m_grid->CallAfter(&wxGrid::DisableCellEditControl);
}

void wxGridCellEditorEvtHandler::OnKillFocus(wxFocusEvent& event)
{
    // We must let the native control have this event so in any case don't mark
    // it as handled, otherwise various weird problems can happen (see #11681).
    event.Skip();

    // Don't disable the cell if we're just starting to edit it
    if (m_inSetFocus)
        return;

    DismissEditor();
}

void wxGridCellEditorEvtHandler::OnKeyDown(wxKeyEvent& event)
{
    switch ( event.GetKeyCode() )
    {
        case WXK_ESCAPE:
            m_editor->Reset();
            DismissEditor();
            break;

        case WXK_TAB:
            m_grid->GetEventHandler()->ProcessEvent( event );
            break;

        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            if (!m_grid->GetEventHandler()->ProcessEvent(event))
                m_editor->HandleReturn(event);
            break;

        default:
            event.Skip();
            break;
    }
}

void wxGridCellEditorEvtHandler::OnChar(wxKeyEvent& event)
{
    const int row = m_grid->GetGridCursorRow();
    const int col = m_grid->GetGridCursorCol();
    const wxRect rect = m_grid->CellToRect( row, col );

    const wxSize client_size = m_grid->GetGridWindow()->GetClientSize();

    // if cell width is smaller than grid client area, cell is wholly visible
    const bool wholeCellVisible = (rect.GetWidth() < client_size.x);

    switch ( event.GetKeyCode() )
    {
        case WXK_ESCAPE:
        case WXK_TAB:
        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            break;

        case WXK_HOME:
        {
            if ( wholeCellVisible )
            {
                // no special processing needed...
                event.Skip();
                break;
            }

            // do special processing for partly visible cell...

            // get the widths of all cells previous to this one
            int colXPos = 0;
            for ( int i = 0; i < col; i++ )
            {
                colXPos += m_grid->GetColSize(i);
            }

            int xUnit = 1, yUnit = 1;
            m_grid->GetScrollPixelsPerUnit(&xUnit, &yUnit);
            if (col != 0)
            {
                m_grid->Scroll(colXPos / xUnit - 1, m_grid->GetScrollPos(wxVERTICAL));
            }
            else
            {
                m_grid->Scroll(colXPos / xUnit, m_grid->GetScrollPos(wxVERTICAL));
            }
            event.Skip();
            break;
        }

        case WXK_END:
        {
            if ( wholeCellVisible )
            {
                // no special processing needed...
                event.Skip();
                break;
            }

            // do special processing for partly visible cell...

            int textWidth = 0;
            std::string value = m_grid->GetCellValue(row, col);
            if (!value.empty())
            {
                // get width of cell CONTENTS (text)
                wxFont font = m_grid->GetCellFont(row, col);
                textWidth = m_grid->GetTextExtent(value, nullptr, nullptr, &font).x;

                // try to RIGHT align the text by scrolling
                const int client_right = m_grid->GetGridWindow()->GetClientSize().x;

                // (m_grid->GetScrollLineX()*2) is a factor for not scrolling to far,
                // otherwise the last part of the cell content might be hidden below the scroll bar
                // FIXME: maybe there is a more suitable correction?
                textWidth -= (client_right - (m_grid->GetScrollLineX() * 2));
                if ( textWidth < 0 )
                {
                    textWidth = 0;
                }
            }

            // get the widths of all cells previous to this one
            int colXPos = 0;
            for ( int i = 0; i < col; i++ )
            {
                colXPos += m_grid->GetColSize(i);
            }

            // and add the (modified) text width of the cell contents
            // as we'd like to see the last part of the cell contents
            colXPos += textWidth;

            int xUnit = 1, yUnit = 1;
            m_grid->GetScrollPixelsPerUnit(&xUnit, &yUnit);
            m_grid->Scroll(colXPos / xUnit - 1, m_grid->GetScrollPos(wxVERTICAL));
            event.Skip();
            break;
        }

        default:
            event.Skip();
            break;
    }
}

// ----------------------------------------------------------------------------
// wxGridCellEditor
// ----------------------------------------------------------------------------

wxGridCellEditor::~wxGridCellEditor()
{
    Destroy();
}

void wxGridCellEditor::Create([[maybe_unused]] wxWindow* parent,
                              [[maybe_unused]] wxWindowID id,
                              wxEvtHandler* evtHandler)
{
    if ( evtHandler )
        m_control->PushEventHandler(evtHandler);
}

void wxGridCellEditor::PaintBackground(wxDC& dc,
                                       const wxRect& rectCell,
                                       const wxGridCellAttr& attr)
{
    // erase the background because we might not fill the cell
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(attr.GetBackgroundColour()));
    dc.DrawRectangle(rectCell);
}

void wxGridCellEditor::Destroy()
{
    if (m_control)
    {
        m_control->PopEventHandler( true /* delete it*/ );

        m_control->Destroy();
        m_control = nullptr;
    }
}

void wxGridCellEditor::Show(bool show, wxGridCellAttr *attr)
{
    wxASSERT_MSG(m_control, "The wxGridCellEditor must be created first!");

    m_control->Show(show);

    if ( show )
    {
        // set the colours/fonts if we have any
        if ( attr )
        {
            m_colFgOld = m_control->GetForegroundColour();
            m_control->SetForegroundColour(attr->GetTextColour());

            m_colBgOld = m_control->GetBackgroundColour();
            m_control->SetBackgroundColour(attr->GetBackgroundColour());

// Workaround for GTK+1 font setting problem on some platforms
#if !defined(__WXGTK__) || defined(__WXGTK20__)
            m_fontOld = m_control->GetFont();
            m_control->SetFont(attr->GetFont());
#endif

            // can't do anything more in the base class version, the other
            // attributes may only be used by the derived classes
        }
    }
    else
    {
        // restore the standard colours fonts
        if ( m_colFgOld.IsOk() )
        {
            m_control->SetForegroundColour(m_colFgOld);
            m_colFgOld = wxNullColour;
        }

        if ( m_colBgOld.IsOk() )
        {
            m_control->SetBackgroundColour(m_colBgOld);
            m_colBgOld = wxNullColour;
        }

// Workaround for GTK+1 font setting problem on some platforms
#if !defined(__WXGTK__) || defined(__WXGTK20__)
        if ( m_fontOld.IsOk() )
        {
            m_control->SetFont(m_fontOld);
            m_fontOld = wxNullFont;
        }
#endif
    }
}

void wxGridCellEditor::SetSize(const wxRect& rect)
{
    wxASSERT_MSG(m_control, "The wxGridCellEditor must be created first!");

    m_control->SetSize(rect, wxSIZE_ALLOW_MINUS_ONE);
}

void wxGridCellEditor::DoPositionEditor(const wxSize& size,
                                        const wxRect& rectCell,
                                        int hAlign,
                                        int vAlign)
{
    wxRect rect(rectCell.GetPosition(), size);

    // We center the control around the cell if it doesn't fit into it in one
    // or both of directions, as this seems to look the best (difference is
    // typically relatively small and by centering it, we divide it by two on
    // each side, making it even smaller).
    //
    // For now just remember in which direction to do it in this variable and
    // then do it at the end.
    int centerDir = 0;

    // We're only going to need the alignment if the control is smaller than
    // the cell in at least one direction.
    if ( size.x < rectCell.width || size.y < rectCell.height )
    {
        if ( GetCellAttr() )
            GetCellAttr()->GetNonDefaultAlignment(&hAlign, &vAlign);
    }

    if ( size.x < rectCell.width )
    {
        if ( hAlign == wxALIGN_CENTER_HORIZONTAL )
            centerDir |= wxHORIZONTAL;
        else if ( hAlign == wxALIGN_RIGHT )
            rect.x = rectCell.x + rectCell.width - rect.width;
        //else: nothing to do for the left alignment
    }
    else
    {
        centerDir |= wxHORIZONTAL;
    }

    if ( size.y < rectCell.height )
    {
        if ( vAlign == wxALIGN_CENTRE_VERTICAL )
            centerDir |= wxVERTICAL;
        else if ( vAlign == wxALIGN_BOTTOM )
            rect.y = rectCell.y + rectCell.height - rect.height;
        //else: nothing to do for the top alignment
    }
    else
    {
        centerDir |= wxVERTICAL;
    }

    if ( centerDir )
        rect = rect.CenterIn(rectCell, centerDir);

    wxGridCellEditor::SetSize(rect);
}

void wxGridCellEditor::HandleReturn(wxKeyEvent& event)
{
    event.Skip();
}

bool wxGridCellEditor::IsAcceptedKey(wxKeyEvent& event)
{
    const bool ctrl = event.ControlDown();
    bool alt;

#ifdef __WXMAC__
    // On the Mac the Alt key is more like shift and is used for entry of
    // valid characters, so check for Ctrl and Meta instead.
    alt = event.MetaDown();
#else // !__WXMAC__
    alt = event.AltDown();
#endif // __WXMAC__/!__WXMAC__

    // Assume it's not a valid char if ctrl or alt is down, but if both are
    // down then it may be because of an AltGr key combination, so let them
    // through in that case.
    if ((ctrl || alt) && !(ctrl && alt))
        return false;

    if ( static_cast<int>(event.GetUnicodeKey()) == WXK_NONE )
        return false;

    return true;
}

void wxGridCellEditor::StartingKey(wxKeyEvent& event)
{
    event.Skip();
}

void wxGridCellEditor::StartingClick()
{
}

#if wxUSE_TEXTCTRL

// ----------------------------------------------------------------------------
// wxGridCellTextEditor
// ----------------------------------------------------------------------------

wxGridCellTextEditor::wxGridCellTextEditor(size_t maxChars)
    : m_maxChars(maxChars)
{
}

void wxGridCellTextEditor::Create(wxWindow* parent,
                                  wxWindowID id,
                                  wxEvtHandler* evtHandler)
{
    DoCreate(parent, id, evtHandler);
}

void wxGridCellTextEditor::DoCreate(wxWindow* parent,
                                    wxWindowID id,
                                    wxEvtHandler* evtHandler,
                                    unsigned int style)
{
    style |= wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB | wxNO_BORDER;

    wxTextCtrl* const text = new wxTextCtrl(parent, id, "",
                                            wxDefaultPosition, wxDefaultSize,
                                            style);
    text->SetMargins(0, 0);
    m_control = text;

    // set max length allowed in the textctrl, if the parameter was set
    if ( m_maxChars != 0 )
    {
        Text()->SetMaxLength(m_maxChars);
    }
#if wxUSE_VALIDATORS
    // validate text in textctrl, if validator is set
    if ( m_validator )
    {
        Text()->SetValidator(*m_validator);
    }
#endif

    wxGridCellEditor::Create(parent, id, evtHandler);
}

void wxGridCellTextEditor::SetSize(const wxRect& rectOrig)
{
    wxRect rect(rectOrig);

    // Make the edit control large enough to allow for internal margins
    //
    // TODO: remove this if the text ctrl sizing is improved
    //
#if defined(__WXMSW__)
    rect.x += 2;
    rect.y += 2;

    rect.width -= 2;
    rect.height -= 2;
#elif !defined(__WXGTK__)
    int extra_x = 2;
    int extra_y = 2;

    #if defined(__WXMOTIF__)
        extra_x *= 2;
        extra_y *= 2;
    #endif

    rect.SetLeft( std::max(0, rect.x - extra_x) );
    rect.SetTop( std::max(0, rect.y - extra_y) );
    rect.SetRight( rect.GetRight() + 2 * extra_x );
    rect.SetBottom( rect.GetBottom() + 2 * extra_y );
#endif

    wxGridCellEditor::SetSize(rect);
}

void wxGridCellTextEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control, "The wxGridCellEditor must be created first!");

    m_value = grid->GetTable()->GetValue(row, col);

    DoBeginEdit(m_value);
}

void wxGridCellTextEditor::DoBeginEdit(const std::string& startValue)
{
    Text()->SetValue(startValue);
    Text()->SetInsertionPointEnd();
    Text()->SelectAll();
    Text()->SetFocus();
}

bool wxGridCellTextEditor::EndEdit([[maybe_unused]] int row,
                                   [[maybe_unused]] int col,
                                   [[maybe_unused]] const wxGrid* grid,
                                   [[maybe_unused]] const std::string& oldval,
                                   std::string *newval)
{
    wxCHECK_MSG( m_control, false,
                 "wxGridCellTextEditor must be created first!" );

    const std::string value = Text()->GetValue();
    if ( value == m_value )
        return false;

    m_value = value;

    if ( newval )
        *newval = m_value;

    return true;
}

void wxGridCellTextEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    grid->GetTable()->SetValue(row, col, m_value);
    m_value.clear();
}

void wxGridCellTextEditor::Reset()
{
    wxASSERT_MSG( m_control, "wxGridCellTextEditor must be created first!" );

    DoReset(m_value);
}

void wxGridCellTextEditor::DoReset(const std::string& startValue)
{
    Text()->SetValue(startValue);
    Text()->SetInsertionPointEnd();
}

bool wxGridCellTextEditor::IsAcceptedKey(wxKeyEvent& event)
{
    switch ( event.GetKeyCode() )
    {
        case WXK_DELETE:
        case WXK_BACK:
            return true;

        default:
            return wxGridCellEditor::IsAcceptedKey(event);
    }
}

void wxGridCellTextEditor::StartingKey(wxKeyEvent& event)
{
    // Since this is now happening in the EVT_CHAR event EmulateKeyPress is no
    // longer an appropriate way to get the character into the text control.
    // Do it ourselves instead.  We know that if we get this far that we have
    // a valid character, so not a whole lot of testing needs to be done.

    wxTextCtrl* tc = Text();
    int ch;

    bool isPrintable;

    ch = event.GetUnicodeKey();
    if ( ch != WXK_NONE )
        isPrintable = true;
    else
    {
        ch = event.GetKeyCode();
        isPrintable = ch >= WXK_SPACE && ch < WXK_START;
    }

    switch (ch)
    {
        case WXK_DELETE:
            // Delete the initial character when starting to edit with DELETE.
            tc->Remove(0, 1);
            break;

        case WXK_BACK:
            // Delete the last character when starting to edit with BACKSPACE.
            {
                const long pos = tc->GetLastPosition();
                tc->Remove(pos - 1, pos);
            }
            break;

        default:
            if ( isPrintable ) // TODO: Possible to just to_string from int char?
                tc->WriteText(std::to_string(gsl::narrow_cast<char>(ch)));
            break;
    }
}

void wxGridCellTextEditor::HandleReturn( wxKeyEvent&
                                         WXUNUSED_GTK(WXUNUSED_MOTIF(event)) )
{
#if defined(__WXMOTIF__) || defined(__WXGTK__)
    // wxMotif needs a little extra help...
    size_t pos = (size_t)( Text()->GetInsertionPoint() );
    std::string s = Text()->GetValue();
    s = s.Left(pos) + "\n" + s.Mid(pos);
    Text()->SetValue(s);
    Text()->SetInsertionPoint( pos );
#else
    // the other ports can handle a Return key press
    //
    event.Skip();
#endif
}

void wxGridCellTextEditor::SetParameters(const std::string& params)
{
    if ( params.empty() )
    {
        // reset to default
        m_maxChars = 0;
    }
    else
    {
        size_t tmp;
        auto [p, ec] = std::from_chars(params.data(), params.data() + params.size(), tmp);

        if ( ec == std::errc() )
        {
            m_maxChars = tmp;
        }
        else
        {
            wxLogDebug( "Invalid wxGridCellTextEditor parameter string '%s' ignored", params.c_str() );
        }
    }
}

#if wxUSE_VALIDATORS
void wxGridCellTextEditor::SetValidator(const wxValidator& validator)
{
    m_validator.reset(dynamic_cast<wxValidator*>(validator.Clone()));
    if ( m_validator && IsCreated() )
        Text()->SetValidator(*m_validator);
}
#endif

wxGridCellEditor *wxGridCellTextEditor::Clone() const
{
    wxGridCellTextEditor* editor = new wxGridCellTextEditor(m_maxChars);
#if wxUSE_VALIDATORS
    if ( m_validator )
    {
        editor->SetValidator(*m_validator);
    }
#endif
    return editor;
}

// return the value in the text control
std::string wxGridCellTextEditor::GetValue() const
{
    return Text()->GetValue();
}

// ----------------------------------------------------------------------------
// wxGridCellNumberEditor
// ----------------------------------------------------------------------------

wxGridCellNumberEditor::wxGridCellNumberEditor(int min, int max)
    : m_min(min),
      m_max(max)
{
}

void wxGridCellNumberEditor::Create(wxWindow* parent,
                                    wxWindowID id,
                                    wxEvtHandler* evtHandler)
{
#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
        static constexpr unsigned int style = wxSP_ARROW_KEYS |
                                      wxTE_PROCESS_ENTER |
                                      wxTE_PROCESS_TAB;

        // create a spin ctrl
        m_control = new wxSpinCtrl(parent, wxID_ANY, "",
                                   wxDefaultPosition, wxDefaultSize,
                                   style,
                                   m_min, m_max);

        wxGridCellEditor::Create(parent, id, evtHandler);
    }
    else
#endif
    {
        // just a text control
        wxGridCellTextEditor::Create(parent, id, evtHandler);

#if wxUSE_VALIDATORS
        Text()->SetValidator(wxIntegerValidator<int>());
#endif
    }
}

void wxGridCellNumberEditor::SetSize(const wxRect& rectCell)
{
#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
        wxASSERT_MSG(m_control, "The wxSpinCtrl must be created first!");

        wxSize size = Spin()->GetBestSize();

        // Extend the control to fill the entire cell horizontally.
        if ( size.x < rectCell.GetWidth() )
            size.x = rectCell.GetWidth();

        // Ensure it uses a reasonable height even if wxSpinCtrl::GetBestSize()
        // didn't return anything useful.
        if ( size.y <= 0 )
            size.y = rectCell.GetHeight();

        DoPositionEditor(size, rectCell);
    }
    else
#endif // wxUSE_SPINCTRL
    {
        wxGridCellTextEditor::SetSize(rectCell);
    }
}

void wxGridCellNumberEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    // first get the value
    wxGridTableBase *table = grid->GetTable();
    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER) )
    {
        m_value = table->GetValueAsLong(row, col);
    }
    else
    {
        m_value = 0;
        std::string sValue = table->GetValue(row, col);

        auto [p, ec] = std::from_chars(sValue.data(), sValue.data() + sValue.size(), m_value);

        if ( ec != std::errc() )
        {
            wxFAIL_MSG( "this cell doesn't have numeric value" );
            return;
        }
    }

#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
        Spin()->SetValue((int)m_value);
        // Select all the text for convenience of editing and for compatibility
        // with the plain text editor.
        Spin()->SetSelection(-1, -1);
        Spin()->SetFocus();
    }
    else
#endif
    {
        DoBeginEdit(GetString());
    }
}

bool wxGridCellNumberEditor::EndEdit([[maybe_unused]] int row,
                                     [[maybe_unused]] int col,
                                     [[maybe_unused]] const wxGrid* grid,
                                     const std::string& oldval,
                                     std::string *newval)
{
    long value = 0;
    std::string text;

#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
        value = Spin()->GetValue();
        if ( value == m_value )
            return false;

        text = fmt::format("{:ld}", value);
    }
    else // using unconstrained input
#endif // wxUSE_SPINCTRL
    {
        text = Text()->GetValue();
        if ( text.empty() )
        {
            if ( oldval.empty() )
                return false;
        }
        else // non-empty text now (maybe 0)
        {
            auto [p, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

            if ( ec != std::errc() )
                return false;

            // if value == m_value == 0 but old text was "" and new one is
            // "0" something still did change
            if ( value == m_value && (value || !oldval.empty()) )
                return false;
        }
    }

    m_value = value;

    if ( newval )
        *newval = text;

    return true;
}

void wxGridCellNumberEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    wxGridTableBase * const table = grid->GetTable();
    if ( table->CanSetValueAs(row, col, wxGRID_VALUE_NUMBER) )
        table->SetValueAsLong(row, col, m_value);
    else
        table->SetValue(row, col, fmt::format("{:ld}", m_value));
}

void wxGridCellNumberEditor::Reset()
{
#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
        Spin()->SetValue((int)m_value);
    }
    else
#endif
    {
        DoReset(GetString());
    }
}

bool wxGridCellNumberEditor::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxGridCellEditor::IsAcceptedKey(event) )
    {
        const int keycode = event.GetKeyCode();
        switch ( keycode )
        {
            // Accept +/- because they can be part of the number and space just
            // because it's a convenient key to start editing with and is also
            // consistent with many (all?) other editors, which allow starting
            // editing using it.
            case '+':
            case '-':
            case ' ':
                return true;

            default:
                return (keycode < 128) && wxIsdigit(keycode);
        }
    }

    return false;
}

void wxGridCellNumberEditor::StartingKey(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    if ( !HasRange() )
    {
        if ( wxIsdigit(keycode) || keycode == '+' || keycode == '-')
        {
            wxGridCellTextEditor::StartingKey(event);

            // skip Skip() below
            return;
        }
    }
#if wxUSE_SPINCTRL
    else
    {
        if ( wxIsdigit(keycode) )
        {
            wxSpinCtrl* spin = (wxSpinCtrl*)m_control;
            spin->SetValue(keycode - '0');
            spin->SetSelection(1,1);
            return;
        }
    }
#endif

    event.Skip();
}

void wxGridCellNumberEditor::SetParameters(const std::string& params)
{
    if ( params.empty() )
    {
        // reset to default
        m_min = -1;
        m_max = -1;
    }
    else
    {
        int tmpBefore{ 0 };

        auto beforeParam = wx::utils::BeforeFirst(params, ',');
        auto [bp, bc] = std::from_chars(beforeParam.data(), beforeParam.data() + beforeParam.size(), tmpBefore);

        if ( bc == std::errc() )
        {
            m_min = tmpBefore;

            int tmpAfter{0};

            auto afterParam = wx::utils::AfterFirst(params, ',');
            auto [ap, ac] = std::from_chars(afterParam.data(), afterParam.data() + afterParam.size(), tmpAfter);

            if ( ac == std::errc() )
            {
                m_max = tmpAfter;

                // skip the error message below
                return;
            }
        }

        wxLogDebug("Invalid wxGridCellNumberEditor parameter string '%s' ignored", params.c_str());
    }
}

// return the value in the spin control if it is there (the text control otherwise)
std::string wxGridCellNumberEditor::GetValue() const
{
#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
        long value = Spin()->GetValue();
        return fmt::format("{:ld}", value);
    }
    else
#endif
    {
        return Text()->GetValue();
    }
}

// ----------------------------------------------------------------------------
// wxGridCellFloatEditor
// ----------------------------------------------------------------------------

wxGridCellFloatEditor::wxGridCellFloatEditor(int width,
                                             int precision,
                                             unsigned int format)
    : m_width(width),
      m_precision(precision),
      m_style(format)
{
}

void wxGridCellFloatEditor::Create(wxWindow* parent,
                                   wxWindowID id,
                                   wxEvtHandler* evtHandler)
{
    wxGridCellTextEditor::Create(parent, id, evtHandler);

#if wxUSE_VALIDATORS
    Text()->SetValidator(wxFloatingPointValidator<double>(m_precision));
#endif
}

void wxGridCellFloatEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    // first get the value
    wxGridTableBase * const table = grid->GetTable();
    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_FLOAT) )
    {
        m_value = table->GetValueAsDouble(row, col);
    }
    else
    {
        m_value = 0.0;

        const std::string value = table->GetValue(row, col);
        if ( !value.empty() )
        {
            auto [p, ec] = std::from_chars(value.data(), value.data() + value.size(), m_value);

            if ( ec != std::errc() )
            {
                wxFAIL_MSG( "this cell doesn't have float value" );
                return;
            }
        }
    }

    DoBeginEdit(GetString());
}

bool wxGridCellFloatEditor::EndEdit([[maybe_unused]] int row,
                                    [[maybe_unused]] int col,
                                    [[maybe_unused]] const wxGrid* grid,
                                    const std::string& oldval, std::string *newval)
{
    const std::string text = Text()->GetValue();

    double value{0.0};

    if ( !text.empty() )
    {
        auto [p, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

        if ( ec != std::errc() )
            return false;
    }
    else // new value is empty string
    {
        if ( oldval.empty() )
            return false;           // nothing changed
    }

    // the test for empty strings ensures that we don't skip the value setting
    // when "" is replaced by "0" or vice versa as "" numeric value is also 0.
    // FIXME: Double equality
    if ( (value == m_value) && !text.empty() && !oldval.empty() )
        return false;           // nothing changed

    m_value = value;

    if ( newval )
        *newval = text;

    return true;
}

void wxGridCellFloatEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    wxGridTableBase * const table = grid->GetTable();

    if ( table->CanSetValueAs(row, col, wxGRID_VALUE_FLOAT) )
        table->SetValueAsDouble(row, col, m_value);
    else
        table->SetValue(row, col, Text()->GetValue());
}

void wxGridCellFloatEditor::Reset()
{
    DoReset(GetString());
}

void wxGridCellFloatEditor::StartingKey(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    char tmpbuf[2];
    tmpbuf[0] = (char) keycode;
    tmpbuf[1] = '\0';
    wxString strbuf(tmpbuf, *wxConvCurrent);

#if wxUSE_INTL
    bool is_decimal_point = ( strbuf ==
       wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER) );
#else
    bool is_decimal_point = ( strbuf == "." );
#endif

    if ( wxIsdigit(keycode) || keycode == '+' || keycode == '-'
         || is_decimal_point )
    {
        wxGridCellTextEditor::StartingKey(event);

        // skip Skip() below
        return;
    }

    event.Skip();
}

// FIXME: Temporary, make a struct of the params.
void wxGridCellFloatEditor::SetParameters(const std::string& params)
{
    if ( params.empty() )
    {
        // reset to default
        m_width = - 1;
        m_precision = -1;
        m_style = wxGRID_FLOAT_FORMAT_DEFAULT;
        m_format.clear();
    }
    else
    {
        auto paramSplit = wx::utils::StrSplit(params, ',');

        auto [pWidth, errW] = std::from_chars(paramSplit[0].data(), paramSplit[0].data() + paramSplit[0].size(), m_width);
        auto [pPrec, errP] = std::from_chars(paramSplit[1].data(), paramSplit[1].data() + paramSplit[1].size(), m_precision);

        if ( !paramSplit[0].empty() && errW != std::errc() )
        {
            wxLogDebug("Invalid wxGridCellFloatRenderer width parameter string '%s ignored", params.c_str());
        }

        if ( !paramSplit[1].empty() && errP != std::errc() )
        {
            wxLogDebug("Invalid wxGridCellFloatRenderer precision parameter string '%s ignored", params.c_str());
        }

        if ( !paramSplit[2].empty() )
        {
            if ( paramSplit[2] == 'f' )
            {
                m_style = wxGRID_FLOAT_FORMAT_FIXED;
            }
            else if ( paramSplit[2] == 'e' )
            {
                m_style = wxGRID_FLOAT_FORMAT_SCIENTIFIC;
            }
            else if ( paramSplit[2] == 'g' )
            {
                m_style = wxGRID_FLOAT_FORMAT_COMPACT;
            }
            else if ( paramSplit[2] == 'E' )
            {
                m_style = wxGRID_FLOAT_FORMAT_SCIENTIFIC |
                          wxGRID_FLOAT_FORMAT_UPPER;
            }
            else if ( paramSplit[2] == 'F' )
            {
                m_style = wxGRID_FLOAT_FORMAT_FIXED |
                          wxGRID_FLOAT_FORMAT_UPPER;
            }
            else if ( paramSplit[2] == 'G' )
            {
                m_style = wxGRID_FLOAT_FORMAT_COMPACT |
                          wxGRID_FLOAT_FORMAT_UPPER;
            }
            else
            {
                wxLogDebug("Invalid wxGridCellFloatRenderer format "
                           "parameter string '%s ignored", params);
            }
        }
    }
}

std::string wxGridCellFloatEditor::GetString()
{
    if ( m_format.empty() )
    {
        if ( m_precision == -1 && m_width != -1)
        {
            // default precision
            m_format = fmt::format("%%%d.", m_width);
        }
        else if ( m_precision != -1 && m_width == -1)
        {
            // default width
            m_format = fmt::format("%%.%d", m_precision);
        }
        else if ( m_precision != -1 && m_width != -1 )
        {
            m_format = fmt::format("%%%d.%d", m_width, m_precision);
        }
        else
        {
            // default width/precision
            m_format = "%";
        }

        bool isUpper = (m_style & wxGRID_FLOAT_FORMAT_UPPER) != 0;
        if ( m_style & wxGRID_FLOAT_FORMAT_SCIENTIFIC )
            m_format += isUpper ? 'E' : 'e';
        else if ( m_style & wxGRID_FLOAT_FORMAT_COMPACT )
            m_format += isUpper ? 'G' : 'g';
        else
            m_format += 'f';
    }

    return fmt::format(m_format, m_value);
}

bool wxGridCellFloatEditor::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxGridCellEditor::IsAcceptedKey(event) )
    {
        const int keycode = event.GetKeyCode();
        if ( wxIsascii(keycode) )
        {
#if wxUSE_INTL
            const wxString decimalPoint =
                wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER);
#else
            const wxString decimalPoint(wxT('.'));
#endif

            // accept digits, 'e' as in '1e+6', also '-', '+', and '.'
            if ( wxIsdigit(keycode) ||
                    tolower(keycode) == 'e' ||
                        keycode == decimalPoint ||
                            keycode == '+' ||
                                keycode == '-' )
            {
                return true;
            }
        }
    }

    return false;
}

#endif // wxUSE_TEXTCTRL

#if wxUSE_CHECKBOX

// ----------------------------------------------------------------------------
// wxGridCellBoolEditor
// ----------------------------------------------------------------------------

// the default values for GetValue()
std::string wxGridCellBoolEditor::ms_stringValues[2] = { "", "1" };

wxGridActivationResult
wxGridCellBoolEditor::TryActivate(int row, int col, wxGrid* grid,
                                  const wxGridActivationSource& actSource)
{
    SetValueFromGrid(row, col, grid);

    switch ( actSource.GetOrigin() )
    {
        case wxGridActivationSource::Origin::Program:
            // It's not really clear what should happen in this case, so for
            // now show the editor interactively to avoid making the choice.
            return wxGridActivationResult::DoEdit();

        case wxGridActivationSource::Origin::Mouse:
            m_value = !m_value;
            return wxGridActivationResult::DoChange(GetStringValue());

        case wxGridActivationSource::Origin::Key:
            switch ( actSource.GetKeyEvent().GetKeyCode() )
            {
                // Handle F2 as space here because we must handle it somehow,
                // because pressing it always starts editing in wxGrid, and
                // it's not really clear what else could it do.
                case WXK_F2:
                case WXK_SPACE:
                    m_value = !m_value;
                    break;

                case '+':
                    if ( m_value )
                        return wxGridActivationResult::DoNothing();

                    m_value = true;
                    break;

                case '-':
                    if ( !m_value )
                        return wxGridActivationResult::DoNothing();

                    m_value = false;
                    break;
            }

            return wxGridActivationResult::DoChange(GetStringValue());
    }

    wxFAIL_MSG( "unknown activation source origin" );
    return wxGridActivationResult::DoNothing();
}

void wxGridCellBoolEditor::DoActivate(int row, int col, wxGrid* grid)
{
    SetGridFromValue(row, col, grid);
}

void wxGridCellBoolEditor::Create(wxWindow* parent,
                                  wxWindowID id,
                                  wxEvtHandler* evtHandler)
{
    m_control = new wxCheckBox(parent, id, "",
                               wxDefaultPosition, wxDefaultSize,
                               wxNO_BORDER);

    wxGridCellEditor::Create(parent, id, evtHandler);
}

void wxGridCellBoolEditor::SetSize(const wxRect& r)
{
    int hAlign = wxALIGN_LEFT;
    int vAlign = wxALIGN_CENTRE_VERTICAL;
    if (GetCellAttr()) // FIXME: Return align pair
        GetCellAttr()->GetNonDefaultAlignment(&hAlign, &vAlign);

    const wxRect checkBoxRect =
        wxGetContentRect(m_control->GetSize(), r, hAlign, vAlign);

    // Don't resize the checkbox, it should have its default (and fitting)
    // size, but do move it to the right position.
    m_control->Move(checkBoxRect.GetPosition());
}

void wxGridCellBoolEditor::Show(bool show, wxGridCellAttr *attr)
{
    m_control->Show(show);

    // Under MSW we need to set the checkbox background colour to the same
    // colour as is used by the cell in order for it to blend in, but using
    // just SetBackgroundColour() would be wrong as this would actually change
    // the background of the checkbox with e.g. GTK 3, making it unusable in
    // any theme where the check mark colour is the same, or close to, our
    // background colour -- which happens to be the case for the default GTK 3
    // theme, making this a rather serious problem.
    //
    // One possible workaround would be to set the foreground colour too, but
    // wxRendererNative methods used in wxGridCellBoolRenderer don't currently
    // take the colours into account, so this would mean that starting to edit
    // a boolean field would change its colours, which would be jarring (and
    // especially so as we currently set custom colours for all cells, not just
    // those that really need them).
    //
    // A more portable solution could be to create a parent window using the
    // background colour if it's different from the default and reparent the
    // checkbox under it, so that the parent window colour showed through the
    // transparent parts of the checkbox, but this would be more complicated
    // for no real gain in practice.
    //
    // So, finally, just use the bespoke function that will change the
    // background under MSW, but doesn't do anything elsewhere.
    if ( show )
    {
        wxColour colBg = attr ? attr->GetBackgroundColour() : *wxLIGHT_GREY;
        CBox()->SetTransparentPartColour(colBg);
    }
}

void wxGridCellBoolEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control,
                 "The wxGridCellEditor must be created first!");

    SetValueFromGrid(row, col, grid);

    CBox()->SetValue(m_value);
    CBox()->SetFocus();
}

bool wxGridCellBoolEditor::EndEdit([[maybe_unused]] int row,
                                   [[maybe_unused]] int col,
                                   [[maybe_unused]] const wxGrid* grid,
                                   [[maybe_unused]] const std::string& oldval,
                                   std::string* newval)
{
    bool value = CBox()->GetValue();
    if ( value == m_value )
        return false;

    m_value = value;

    if ( newval )
        *newval = GetValue();

    return true;
}

void wxGridCellBoolEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    SetGridFromValue(row, col, grid);
}

void wxGridCellBoolEditor::Reset()
{
    wxASSERT_MSG(m_control,
                 "The wxGridCellEditor must be created first!");

    CBox()->SetValue(m_value);
}

void wxGridCellBoolEditor::StartingClick()
{
    CBox()->SetValue(!CBox()->GetValue());
}

bool wxGridCellBoolEditor::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxGridCellEditor::IsAcceptedKey(event) )
    {
        switch ( event.GetKeyCode() )
        {
            case WXK_SPACE:
            case '+':
            case '-':
                return true;

            default:
                return false;
        }
    }

    return false;
}

void wxGridCellBoolEditor::StartingKey(wxKeyEvent& event)
{
    switch ( event.GetKeyCode() )
    {
        case WXK_SPACE:
            CBox()->SetValue(!CBox()->GetValue());
            break;

        case '+':
            CBox()->SetValue(true);
            break;

        case '-':
            CBox()->SetValue(false);
            break;
        
        default:
            return;
    }
}

std::string wxGridCellBoolEditor::GetValue() const
{
  return GetStringValue(CBox()->GetValue());
}

/* static */ void
wxGridCellBoolEditor::UseStringValues(const std::string& valueTrue,
                                      const std::string& valueFalse)
{
    // FIXME: Indexing with bool
    ms_stringValues[false] = valueFalse;
    ms_stringValues[true] = valueTrue;
}

/* static */ bool
wxGridCellBoolEditor::IsTrueValue(const std::string& value)
{
    // FIXME: Indexing with bool
    return value == ms_stringValues[true];
}

void wxGridCellBoolEditor::SetValueFromGrid(int row, int col, wxGrid* grid)
{
    if (grid->GetTable()->CanGetValueAs(row, col, wxGRID_VALUE_BOOL))
    {
        m_value = grid->GetTable()->GetValueAsBool(row, col);
    }
    else
    {
        std::string cellval = grid->GetTable()->GetValue(row, col);

        // FIXME: Indexing with bool
        if ( cellval == ms_stringValues[false] )
            m_value = false;
        else if ( cellval == ms_stringValues[true] )
            m_value = true;
        else
        {
            // do not try to be smart here and convert it to true or false
            // because we'll still overwrite it with something different and
            // this risks to be very surprising for the user code, let them
            // know about it
            wxFAIL_MSG( "invalid value for a cell with bool editor!" );

            // Still need to initialize it to something.
            m_value = false;
        }
    }
}

void wxGridCellBoolEditor::SetGridFromValue(int row, int col, wxGrid* grid) const
{
    wxGridTableBase * const table = grid->GetTable();

    if ( table->CanSetValueAs(row, col, wxGRID_VALUE_BOOL) )
        table->SetValueAsBool(row, col, m_value);
    else
        table->SetValue(row, col, GetStringValue());
}

#endif // wxUSE_CHECKBOX

#if wxUSE_COMBOBOX

// ----------------------------------------------------------------------------
// wxGridCellChoiceEditor
// ----------------------------------------------------------------------------

wxGridCellChoiceEditor::wxGridCellChoiceEditor(const std::vector<std::string>& choices,
                                               bool allowOthers)
    : m_choices(choices),
      m_allowOthers(allowOthers) { }

wxGridCellEditor *wxGridCellChoiceEditor::Clone() const
{
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor;
    editor->m_allowOthers = m_allowOthers;
    editor->m_choices = m_choices;

    return editor;
}

void wxGridCellChoiceEditor::Create(wxWindow* parent,
                                    wxWindowID id,
                                    wxEvtHandler* evtHandler)
{
    int style = wxTE_PROCESS_ENTER |
                wxTE_PROCESS_TAB |
                wxBORDER_NONE;

    if ( !m_allowOthers )
        style |= wxCB_READONLY;
    m_control = new wxComboBox(parent, id, "",
                               wxDefaultPosition, wxDefaultSize,
                               m_choices,
                               style);

    wxGridCellEditor::Create(parent, id, evtHandler);
}

void wxGridCellChoiceEditor::SetSize(const wxRect& rect)
{
    wxASSERT_MSG(m_control,
                 "The wxGridCellChoiceEditor must be created first!");

    // Use normal wxChoice size, except for extending it to fill the cell
    // width: we can't be smaller because this could make the control unusable
    // and we don't want to be taller because this looks unusual and weird.
    wxSize size = m_control->GetBestSize();
    if ( size.x < rect.width )
        size.x = rect.width;

    DoPositionEditor(size, rect);
}

void wxGridCellChoiceEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control,
                 "The wxGridCellEditor must be created first!");

    wxGridCellEditorEvtHandler* evtHandler = nullptr;
    if (m_control)
    {
        // This event handler is needed to properly dismiss the editor when the popup is closed
        m_control->Bind(wxEVT_COMBOBOX_CLOSEUP, &wxGridCellChoiceEditor::OnComboCloseUp, this);
        evtHandler = wxDynamicCast(m_control->GetEventHandler(), wxGridCellEditorEvtHandler);
    }

    // Don't immediately end if we get a kill focus event within BeginEdit
    if (evtHandler)
        evtHandler->SetInSetFocus(true);

    m_value = grid->GetTable()->GetValue(row, col);

    Reset(); // this updates combo box to correspond to m_value

    Combo()->SetFocus();

#ifdef __WXOSX_COCOA__
    // This is a work around for the combobox being simply dismissed when a
    // choice is made in it under OS X. The bug is almost certainly due to a
    // problem in focus events generation logic but it's not obvious to fix and
    // for now this at least allows to use wxGrid.
    Combo()->Popup();
#endif

    if (evtHandler)
    {
        // When dropping down the menu, a kill focus event
        // happens after this point, so we can't reset the flag yet.
#if !defined(__WXGTK20__)
        evtHandler->SetInSetFocus(false);
#endif
    }
}

// FIXME: Return string
bool wxGridCellChoiceEditor::EndEdit([[maybe_unused]] int row,
                                     [[maybe_unused]] int col,
                                     [[maybe_unused]] const wxGrid* grid,
                                     [[maybe_unused]] const std::string& oldval,
                                     std::string *newval)
{
    const std::string value = Combo()->GetValue();
    if ( value == m_value )
        return false;

    m_value = value;

    if ( newval )
        *newval = value;

    return true;
}

void wxGridCellChoiceEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    grid->GetTable()->SetValue(row, col, m_value);
}

void wxGridCellChoiceEditor::Reset()
{
    if (m_allowOthers)
    {
        Combo()->SetValue(m_value);
        Combo()->SetInsertionPointEnd();
    }
    else // the combobox is read-only
    {
        // find the right position, or default to the first if not found
        int pos = Combo()->FindString(m_value);
        if (pos == wxNOT_FOUND)
            pos = 0;
        Combo()->SetSelection(pos);
    }
}

void wxGridCellChoiceEditor::SetParameters(const std::string& params)
{
    if ( params.empty() )
    {
        // what can we do?
        return;
    }

    m_choices.clear();

    wxStringTokenizer tk(params, ",");
    while ( tk.HasMoreTokens() )
    {
        m_choices.emplace_back(tk.GetNextToken());
    }

    if ( m_control )
    {
        Combo()->Set(m_choices);
    }
    //else: m_choices will be used when creating it
}

// return the value in the text control
std::string wxGridCellChoiceEditor::GetValue() const
{
  return Combo()->GetValue();
}

void wxGridCellChoiceEditor::OnComboCloseUp([[maybe_unused]] wxCommandEvent& evt)
{
    wxGridCellEditorEvtHandler* evtHandler = wxDynamicCast(m_control->GetEventHandler(),
                                                           wxGridCellEditorEvtHandler);

    if ( !evtHandler )
        return;

    // Close the grid editor when the combobox closes, otherwise it leaves the
    // dropdown arrow visible in the cell.
    evtHandler->DismissEditor();
}

#endif // wxUSE_COMBOBOX

#if wxUSE_COMBOBOX

// ----------------------------------------------------------------------------
// wxGridCellEnumEditor
// ----------------------------------------------------------------------------

// A cell editor which displays an enum number as a textual equivalent. eg
// data in cell is 0,1,2 ... n the cell could be displayed as
// "John","Fred"..."Bob" in the combo choice box

wxGridCellEnumEditor::wxGridCellEnumEditor(const std::string& choices)
{
    if (!choices.empty())
        SetParameters(choices);
}

wxGridCellEditor *wxGridCellEnumEditor::Clone() const
{
    wxGridCellEnumEditor *editor = new wxGridCellEnumEditor();
    editor->m_index = m_index;
    return editor;
}

void wxGridCellEnumEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control,
                 "The wxGridCellEnumEditor must be Created first!");

    wxGridCellEditorEvtHandler* evtHandler = nullptr;
    if (m_control)
        evtHandler = wxDynamicCast(m_control->GetEventHandler(), wxGridCellEditorEvtHandler);

    // Don't immediately end if we get a kill focus event within BeginEdit
    if (evtHandler)
        evtHandler->SetInSetFocus(true);

    wxGridTableBase *table = grid->GetTable();

    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER) )
    {
        m_index = table->GetValueAsLong(row, col);
    }
    else
    {
        std::string startValue = table->GetValue(row, col);

        auto [p, ec] = std::from_chars(startValue.data(), startValue.data() + startValue.size(), m_index);

        if( ec != std::errc() )
        {
            m_index = -1;
        }
    }

    Combo()->SetSelection(m_index);
    Combo()->SetFocus();

#ifdef __WXOSX_COCOA__
    // This is a work around for the combobox being simply dismissed when a
    // choice is made in it under OS X. The bug is almost certainly due to a
    // problem in focus events generation logic but it's not obvious to fix and
    // for now this at least allows to use wxGrid.
    Combo()->Popup();
#endif

    if (evtHandler)
    {
        // When dropping down the menu, a kill focus event
        // happens after this point, so we can't reset the flag yet.
#if !defined(__WXGTK20__)
        evtHandler->SetInSetFocus(false);
#endif
    }
}

// FIXME: Return string
bool wxGridCellEnumEditor::EndEdit([[maybe_unused]] int row,
                                   [[maybe_unused]] int col,
                                   [[maybe_unused]] const wxGrid* grid,
                                   [[maybe_unused]] const std::string& oldval,
                                   std::string *newval)
{
    long idx = Combo()->GetSelection();
    if ( idx == m_index )
        return false;

    m_index = idx;

    if ( newval )
        *newval = fmt::format("{:ld}", m_index);

    return true;
}

void wxGridCellEnumEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    wxGridTableBase * const table = grid->GetTable();
    if ( table->CanSetValueAs(row, col, wxGRID_VALUE_NUMBER) )
        table->SetValueAsLong(row, col, m_index);
    else
        table->SetValue(row, col, fmt::format("{:ld}", m_index));
}

#endif // wxUSE_COMBOBOX

// ----------------------------------------------------------------------------
// wxGridCellAutoWrapStringEditor
// ----------------------------------------------------------------------------

void
wxGridCellAutoWrapStringEditor::Create(wxWindow* parent,
                                       wxWindowID id,
                                       wxEvtHandler* evtHandler)
{
    wxGridCellTextEditor::DoCreate(parent, id, evtHandler,
                                    wxTE_MULTILINE | wxTE_RICH);
}

#if wxUSE_DATEPICKCTRL

// ----------------------------------------------------------------------------
// wxGridCellDateEditor
// ----------------------------------------------------------------------------

#if defined ( __WXGTK__ )
// Desired behavior is to close the editor control on ESC without updating the
// table, and to close with update on ENTER. On wxMSW wxWANTS_CHARS is enough
// for that, but on wxGTK a bit of special processing is required to forward
// some of the key events from wxDatePickerCtrl to the generic cell editor
// event handler.
struct wxGridCellDateEditorKeyHandler
{
    explicit wxGridCellDateEditorKeyHandler(wxGridCellEditorEvtHandler* handler)
        : m_handler(handler)
    {}

    void operator()(wxKeyEvent& event) const
    {
        switch ( event.GetKeyCode() )
        {
            case WXK_ESCAPE:
                m_handler->OnKeyDown(event);
                break;

            case WXK_RETURN:
            case WXK_NUMPAD_ENTER:
                m_handler->DismissEditor();
                event.Skip();
                break;

            default:
                event.Skip();
                break;
        }
    }

    wxGridCellEditorEvtHandler* m_handler;

#ifdef wxNO_RTTI
    // wxEventFunctorFunction used when an object of this class is passed to
    // Bind() must have a default ctor when using wx RTTI implementation (see
    // see the comment before WX_DECLARE_TYPEINFO_INLINE() in wx/typeinfo.h)
    // and this, in turn, requires a default ctor of this class -- which will
    // never be actually used, but must nevertheless exist.
    wxGridCellDateEditorKeyHandler() : m_handler(NULL) { }
#endif // wxNO_RTTI
};
#endif // __WXGTK__

wxGridCellDateEditor::wxGridCellDateEditor(const std::string& format)
{
    SetParameters(format);
}

void wxGridCellDateEditor::SetParameters(const std::string& params)
{
    if ( params.empty() )
        m_format = "%x";
    else
        m_format = params;
}

void wxGridCellDateEditor::Create(wxWindow* parent, wxWindowID id,
                                  wxEvtHandler* evtHandler)
{
    m_control = new wxDatePickerCtrl(parent, id,
                                     wxDefaultDateTime,
                                     wxDefaultPosition,
                                     wxDefaultSize,
                                     wxDP_DEFAULT |
                                     wxDP_SHOWCENTURY |
                                     wxWANTS_CHARS);

    wxGridCellEditor::Create(parent, id, evtHandler);

#if defined ( __WXGTK__ )
    // Install a handler for ESC and ENTER keys.
    wxGridCellEditorEvtHandler* handler =
        wxDynamicCast(evtHandler, wxGridCellEditorEvtHandler);
    if ( handler )
    {
        handler->Bind(wxEVT_CHAR, wxGridCellDateEditorKeyHandler(handler));
    }
#endif // __WXGTK__
}

void wxGridCellDateEditor::SetSize(const wxRect& r)
{
    wxASSERT_MSG(m_control, "The wxGridCellDateEditor must be created first!");

    wxSize size = DatePicker()->GetBestSize();

    // Allow date picker to become a bit wider, if necessary, but not too wide,
    // otherwise it just looks ugly.
    if ( r.GetWidth() > size.x
            && r.GetWidth() < 3*size.x / 2 )
    {
        size.x = r.GetWidth();
    }

    // Use right alignment by default for consistency with the date renderer.
    DoPositionEditor(size, r, wxALIGN_RIGHT);
}

void wxGridCellDateEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control, "The wxGridCellDateEditor must be created first!");

    using namespace wxGridPrivate;

    if ( !TryGetValueAsDate(m_value, DateParseParams::WithFallback(m_format),
                            *grid, row, col) )
    {
        // Invalidate m_value, so that it always compares different
        // to any value returned from DatePicker()->GetValue().
        m_value = wxDefaultDateTime;
    }
    else
    {
        DatePicker()->SetValue(m_value);
    }

    DatePicker()->SetFocus();
}

// FIXME: Return string
bool wxGridCellDateEditor::EndEdit([[maybe_unused]] int row, [[maybe_unused]] int col,
                                   [[maybe_unused]] const wxGrid* grid,
                                   [[maybe_unused]] const std::string& oldval,
                                   std::string *newval)
{
    wxASSERT_MSG(m_control, "The wxGridCellDateEditor must be created first!");

    const wxDateTime date = DatePicker()->GetValue();

    if ( m_value == date )
    {
        return false;
    }

    m_value = date;

    if ( newval )
    {
        *newval = m_value.FormatISODate();
    }

    return true;
}

void wxGridCellDateEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    grid->GetTable()->SetValue(row, col, m_value.FormatISODate());
}

void wxGridCellDateEditor::Reset()
{
    wxASSERT_MSG(m_control, "The wxGridCellDateEditor must be created first!");

    m_value = DatePicker()->GetValue();
}

wxGridCellEditor *wxGridCellDateEditor::Clone() const
{
    return new wxGridCellDateEditor(m_format);
}

std::string wxGridCellDateEditor::GetValue() const
{
    wxASSERT_MSG(m_control, "The wxGridCellDateEditor must be created first!");

    return DatePicker()->GetValue().FormatISODate();
}

wxDatePickerCtrl* wxGridCellDateEditor::DatePicker() const
{
    return dynamic_cast<wxDatePickerCtrl*>(m_control);
}

#endif // wxUSE_DATEPICKCTRL

#endif // wxUSE_GRID
