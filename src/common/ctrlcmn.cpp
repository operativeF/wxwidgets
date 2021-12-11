/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/ctrlcmn.cpp
// Purpose:     wxControl common interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     26.07.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_CONTROLS

#include "wx/control.h"
#include "wx/dc.h"
#include "wx/log.h"
#include "wx/radiobut.h"
#include "wx/statbmp.h"
#include "wx/bitmap.h"
#include "wx/utils.h"       // for wxStripMenuCodes()

#include "wx/private/markupparser.h"

import WX.Cfg.Flags;

import WX.Utils.Settings;
import Utils.Strings;

// ============================================================================
// implementation
// ============================================================================

bool wxControlBase::Create(wxWindow *parent,
                           wxWindowID id,
                           const wxPoint &pos,
                           const wxSize &size,
                           unsigned int style,
                           const wxValidator& wxVALIDATOR_PARAM(validator),
                           std::string_view name)
{
    const bool ret = wxWindow::Create(parent, id, pos, size, style, name);

#if wxUSE_VALIDATORS
    if ( ret )
        SetValidator(validator);
#endif // wxUSE_VALIDATORS

    return ret;
}

bool wxControlBase::CreateControl(wxWindowBase *parent,
                                  wxWindowID id,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  unsigned int style,
                                  const wxValidator& validator,
                                  std::string_view name)
{
    // even if it's possible to create controls without parents in some port,
    // it should surely be discouraged because it doesn't work at all under
    // Windows
    wxCHECK_MSG( parent, false, "all controls must have parents" );

    if ( !CreateBase(parent, id, pos, size, style, validator, name) )
        return false;

    parent->AddChild(this);

    return true;
}

void wxControlBase::Command(wxCommandEvent& event)
{
    GetEventHandler()->ProcessEvent(event);
}

void wxControlBase::InitCommandEvent(wxCommandEvent& event) const
{
    event.SetEventObject(const_cast<wxControlBase *>(this));

    // event.SetId(GetId()); -- this is usuall done in the event ctor

    switch ( m_clientDataType )
    {
        case wxClientDataType::Void:
            event.SetClientData(GetClientData());
            break;

        case wxClientDataType::Object:
            event.SetClientObject(GetClientObject());
            break;

        case wxClientDataType::None:
            // nothing to do
            [[fallthrough]];
        default:
            break;
    }
}

bool wxControlBase::SetFont(const wxFont& font)
{
    if ( !wxWindow::SetFont(font) )
        return false;

    InvalidateBestSize();

    return true;
}

// wxControl-specific processing after processing the update event
void wxControlBase::DoUpdateWindowUI(wxUpdateUIEvent& event)
{
    // call inherited
    wxWindowBase::DoUpdateWindowUI(event);

    // update label
    if ( event.GetSetText() )
    {
        if ( event.GetText() != GetLabel() )
            SetLabel(event.GetText());
    }

    // Unfortunately we don't yet have common base class for
    // wxRadioButton, so we handle updates of radiobuttons here.
    // TODO: If once wxRadioButtonBase will exist, move this code there.
#if wxUSE_RADIOBTN
    if ( event.GetSetChecked() )
    {
        wxRadioButton *radiobtn = wxDynamicCastThis(wxRadioButton);
        if ( radiobtn )
            radiobtn->SetValue(event.GetChecked());
    }
#endif // wxUSE_RADIOBTN
}

wxSize wxControlBase::DoGetSizeFromTextSize([[maybe_unused]] int xlen,
                                            [[maybe_unused]] int ylen) const
{
    return {-1, -1};
}

/* static */
std::string wxControlBase::GetLabelText(const std::string& label)
{
    // we don't want strip the TABs here, just the mnemonics
    return wxStripMenuCodes(label, wxStrip_Mnemonics);
}

/* static */
std::string wxControlBase::RemoveMnemonics(const std::string& str)
{
    // we don't want strip the TABs here, just the mnemonics
    return wxStripMenuCodes(str, wxStrip_Mnemonics);
}

/* static */
std::string wxControlBase::EscapeMnemonics(std::string_view text)
{
    std::string label{text.begin(), text.end()};
    wx::utils::ReplaceAll(label, "&", "&&");

    return label;
}

/* static */
int wxControlBase::FindAccelIndex(const std::string& label, std::string* labelOnly)
{
    // the character following MNEMONIC_PREFIX is the accelerator for this
    // control unless it is MNEMONIC_PREFIX too - this allows to insert
    // literal MNEMONIC_PREFIX chars into the label
    static constexpr char MNEMONIC_PREFIX = '&';

    if ( labelOnly )
    {
        labelOnly->clear();
        labelOnly->resize(label.length());
    }

    // When computing the offset below, we need to ignore the characters that
    // are not actually displayed, i.e. the ampersands themselves.
    int numSkipped = 0;
    int indexAccel = -1;
    for ( std::string::const_iterator pc = label.begin(); pc != label.end(); ++pc )
    {
        if ( *pc == MNEMONIC_PREFIX )
        {
            ++pc; // skip it
            ++numSkipped;

            if ( pc == label.end() )
                break;
            else if ( *pc != MNEMONIC_PREFIX )
            {
                if ( indexAccel == -1 )
                {
                    indexAccel = pc - label.begin() - numSkipped;
                }
                else
                {
                    wxFAIL_MSG("duplicate accel char in control label");
                }
            }
        }

        if ( labelOnly )
        {
            *labelOnly += *pc;
        }
    }

    return indexAccel;
}

wxBorder wxControlBase::GetDefaultBorder() const
{
    return wxBORDER_THEME;
}

/* static */ wxVisualAttributes
wxControlBase::GetCompositeControlsDefaultAttributes([[maybe_unused]] wxWindowVariant variant)
{
    wxVisualAttributes attrs;
    attrs.font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    attrs.colFg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    attrs.colBg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);

    return attrs;
}

// ----------------------------------------------------------------------------
// wxControl markup support
// ----------------------------------------------------------------------------

#if wxUSE_MARKUP

/* static */
std::string wxControlBase::RemoveMarkup(const std::string& markup)
{
    return wxMarkupParser::Strip(markup);
}

bool wxControlBase::DoSetLabelMarkup(const std::string& markup)
{
    const std::string label = RemoveMarkup(markup);
    if ( label.empty() && !markup.empty() )
        return false;

    SetLabel(label);

    return true;
}

#endif // wxUSE_MARKUP

// ----------------------------------------------------------------------------
// wxControlBase - ellipsization code
// ----------------------------------------------------------------------------

constexpr char wxELLIPSE_REPLACEMENT[] = "...";

namespace
{

struct EllipsizeCalculator
{
    EllipsizeCalculator(const std::string& s,
                        const wxDC& dc,
                        int maxFinalWidthPx,
                        int replacementWidthPx,
                        EllipsizeFlags flags)
        :
          m_initialCharToRemove(0),
          m_nCharsToRemove(0),
          m_outputNeedsUpdate(true),
          m_str(s),
          m_dc(dc),
          m_maxFinalWidthPx(maxFinalWidthPx),
          m_replacementWidthPx(replacementWidthPx)
    {
        size_t expectedOffsetsCount = s.length();

        // Where ampersands are used as mnemonic indicator they should not
        // affect the overall width of the string and must be removed from the
        // measurement. Nonetheless, we need to keep them in the string and
        // have a corresponding entry in m_charOffsetsPx.
        if ( flags & wxEllipsizeFlags::ProcessMnemonics )
        {
            // Create a copy of the string with the ampersands removed to get
            // the correct widths.
            const std::string cpy = wxControl::RemoveMnemonics(s);

            m_charOffsetsPx = dc.GetPartialTextExtents(cpy);
            m_isOk = true;
            // Iterate through the original string inserting a cumulative width
            // value for each ampersand that is the same as the following
            // character's cumulative width value. Except this is only done
            // for the first ampersand in a pair (see RemoveMnemonics).
            size_t n = 0;
            bool lastWasMnemonic = false;
            for ( std::string::const_iterator it = s.begin();
                  it != s.end();
                  ++it, ++n )
            {
                if ( *it == '&' && !lastWasMnemonic )
                {
                    if ( (it + 1) != s.end() )
                    {
                        const int w = m_charOffsetsPx[n];
                        m_charOffsetsPx.insert(std::begin(m_charOffsetsPx) + n, w);
                        lastWasMnemonic = true;
                    }
                    else // Last character is an ampersand.
                    {
                        // This ampersand is removed by RemoveMnemonics() and
                        // won't be displayed when this string is drawn
                        // neither, so we intentionally don't use it for our
                        // calculations neither -- just account for this in the
                        // assert below.
                        expectedOffsetsCount--;
                    }
                }
                else // Not an ampersand used to introduce a mnemonic.
                {
                    lastWasMnemonic = false;
                }
            }
        }
        else
        {
            m_charOffsetsPx = dc.GetPartialTextExtents(s);
            m_isOk = true;
        }

        // Either way, we should end up with the same number of offsets as
        // characters in the original string.
        wxASSERT( m_charOffsetsPx.size() == expectedOffsetsCount );
    }

    bool IsOk() const { return m_isOk; }

    bool EllipsizationNotNeeded() const
    {
        // NOTE: charOffsetsPx[n] is the width in pixels of the first n characters (with the last one INCLUDED)
        //       thus charOffsetsPx[len-1] is the total width of the string
        return m_charOffsetsPx.back() <= m_maxFinalWidthPx;
    }

    void Init(size_t initialCharToRemove, size_t nCharsToRemove)
    {
        m_initialCharToRemove = initialCharToRemove;
        m_nCharsToRemove = nCharsToRemove;
    }

    void RemoveFromEnd()
    {
        m_nCharsToRemove++;
    }

    void RemoveFromStart()
    {
        m_initialCharToRemove--;
        m_nCharsToRemove++;
    }

    size_t GetFirstRemoved() const { return m_initialCharToRemove; }
    size_t GetLastRemoved() const { return m_initialCharToRemove + m_nCharsToRemove - 1; }

    const std::string& GetEllipsizedText()
    {
        if ( m_outputNeedsUpdate )
        {
            wxASSERT(m_initialCharToRemove <= m_str.length() - 1);  // see valid range for initialCharToRemove above
            wxASSERT(m_nCharsToRemove >= 1 && m_nCharsToRemove <= m_str.length() - m_initialCharToRemove);  // see valid range for nCharsToRemove above

            // erase m_nCharsToRemove characters after m_initialCharToRemove (included);
            // e.g. if we have the string "foobar" (len = 6)
            //                               ^
            //                               \--- m_initialCharToRemove = 2
            //      and m_nCharsToRemove = 2, then we get "foar"
            m_output = m_str;
            m_output.replace(m_initialCharToRemove, m_nCharsToRemove, wxELLIPSE_REPLACEMENT);
        }

        return m_output;
    }

    bool IsShortEnough()
    {
        if ( m_nCharsToRemove == m_str.length() )
            return true; // that's the best we could do

        // Width calculation using partial extents is just an inaccurate
        // estimate: partial extents have sub-pixel precision and are rounded
        // by GetPartialTextExtents(); replacing part of the string with "..."
        // may change them too thanks to changes in ligatures, kerning etc.
        //
        // The correct algorithm would be to call GetTextExtent() in every step
        // of ellipsization, but that would be too expensive, especially when
        // the difference is just a few pixels. So we use partial extents to
        // estimate string width and only verify it with GetTextExtent() when
        // it looks good.

        int estimatedWidth = m_replacementWidthPx; // length of "..."

        // length of text before the removed part:
        if ( m_initialCharToRemove > 0 )
            estimatedWidth += m_charOffsetsPx[m_initialCharToRemove - 1];

        // length of text after the removed part:

        if ( GetLastRemoved() < m_str.length() )
           estimatedWidth += m_charOffsetsPx.back() - m_charOffsetsPx[GetLastRemoved()];

        if ( estimatedWidth > m_maxFinalWidthPx )
            return false;

        return m_dc.GetTextExtent(GetEllipsizedText()).x <= m_maxFinalWidthPx;
    }

    std::string m_output;
    std::string m_str;

    std::vector<int> m_charOffsetsPx;

    const wxDC& m_dc;

    // calculation state:

    // REMEMBER: indexes inside the string have a valid range of [0;len-1] if not otherwise constrained
    //           lengths/counts of characters (e.g. nCharsToRemove) have a
    //           valid range of [0;len] if not otherwise constrained
    // NOTE: since this point we know we have for sure a non-empty string from which we need
    //       to remove _at least_ one character (thus nCharsToRemove below is constrained to be >= 1)

    // index of first character to erase, valid range is [0;len-1]:
    size_t m_initialCharToRemove;
    // how many chars do we need to erase? valid range is [0;len-m_initialCharToRemove]
    size_t m_nCharsToRemove;

    int m_maxFinalWidthPx;
    int m_replacementWidthPx;

    bool m_outputNeedsUpdate;
    bool m_isOk;
};

std::string DoEllipsizeSingleLine(const std::string& curLine, const wxDC& dc,
                               wxEllipsizeMode mode, int maxFinalWidthPx,
                               int replacementWidthPx, EllipsizeFlags flags)
{
    wxASSERT_MSG(replacementWidthPx > 0, "Invalid parameters");
    wxASSERT_LEVEL_2_MSG(!curLine.Contains('\n'),
                         "Use Ellipsize() instead!");

    wxASSERT_MSG( mode != wxEllipsizeMode::None, "shouldn't be called at all then" );

    if (maxFinalWidthPx <= 0)
        return {};

    const size_t len = curLine.length();
    if (len <= 1 )
        return curLine;

    EllipsizeCalculator calc(curLine, dc, maxFinalWidthPx, replacementWidthPx, flags);

    if ( !calc.IsOk() )
        return curLine;

    if ( calc.EllipsizationNotNeeded() )
        return curLine;

    // let's compute the range of characters to remove depending on the ellipsization mode:
    switch (mode)
    {
        case wxEllipsizeMode::Start:
            {
                calc.Init(0, 1);
                while ( !calc.IsShortEnough() )
                    calc.RemoveFromEnd();

                // always show at least one character of the string:
                if ( calc.m_nCharsToRemove == len )
                    return fmt::format("{}{}", wxELLIPSE_REPLACEMENT, curLine[len - 1]);

                break;
            }

        case wxEllipsizeMode::Middle:
            {
                // NOTE: the following piece of code works also when len == 1

                // start the removal process from the middle of the string
                // i.e. separe the string in three parts:
                // - the first one to preserve, valid range [0;initialCharToRemove-1] or the empty range if initialCharToRemove==0
                // - the second one to remove, valid range [initialCharToRemove;endCharToRemove]
                // - the third one to preserve, valid range [endCharToRemove+1;len-1] or the empty range if endCharToRemove==len-1
                // NOTE: empty range != range [0;0] since the range [0;0] contains 1 character (the zero-th one)!

                calc.Init(len/2, 0);

                bool removeFromStart = true;

                while ( !calc.IsShortEnough() )
                {
                    const bool canRemoveFromStart = calc.GetFirstRemoved() > 0;
                    const bool canRemoveFromEnd = calc.GetLastRemoved() < len - 1;

                    if ( !canRemoveFromStart && !canRemoveFromEnd )
                    {
                        // we need to remove all the characters of the string!
                        break;
                    }

                    // Remove from the beginning in even steps and from the end
                    // in odd steps, unless we exhausted one side already:
                    removeFromStart = !removeFromStart;
                    if ( removeFromStart && !canRemoveFromStart )
                        removeFromStart = false;
                    else if ( !removeFromStart && !canRemoveFromEnd )
                        removeFromStart = true;

                    if ( removeFromStart )
                        calc.RemoveFromStart();
                    else
                        calc.RemoveFromEnd();
                }

                // Always show at least one character of the string.
                // Additionally, if there's only one character left, prefer
                // "a..." to "...a":
                if ( calc.m_nCharsToRemove == len ||
                     calc.m_nCharsToRemove == len - 1 )
                {
                    return fmt::format("{}{}", curLine[0], wxELLIPSE_REPLACEMENT);
                }
            }
            break;

        case wxEllipsizeMode::End:
            {
                calc.Init(len - 1, 1);
                while ( !calc.IsShortEnough() )
                    calc.RemoveFromStart();

                // always show at least one character of the string:
                if ( calc.m_nCharsToRemove == len )
                    return fmt::format("{}{}", curLine[0], wxELLIPSE_REPLACEMENT);

                break;
            }

        case wxEllipsizeMode::None:
        default:
            wxFAIL_MSG("invalid ellipsize mode");
            return curLine;
    }

    return calc.GetEllipsizedText();
}

} // anonymous namespace


/* static */
std::string wxControlBase::Ellipsize(std::string_view label, const wxDC& dc,
                                     wxEllipsizeMode mode, int maxFinalWidth,
                                     EllipsizeFlags flags)
{
    if (mode == wxEllipsizeMode::None)
        return std::string(label);

    std::string ret;

    // these cannot be cached between different Ellipsize() calls as they can
    // change because of e.g. a font change; however we calculate them only once
    // when ellipsizing multiline labels:
    int replacementWidth = dc.GetTextExtent(wxELLIPSE_REPLACEMENT).x;

    // NB: we must handle correctly labels with newlines:
    std::string curLine;

    for ( std::string_view::const_iterator pc = label.begin(); ; ++pc )
    {
        char ch{};

        if(pc != label.end())
            ch = *pc;

        if ( pc == label.end() || ch == '\n' )
        {
            wx::utils::TrimTrailingSpace(curLine);

            curLine = DoEllipsizeSingleLine(curLine, dc, mode, maxFinalWidth,
                                            replacementWidth, flags);

            // add this (ellipsized) row to the rest of the label
            ret += curLine;
            if ( pc == label.end() )
                break;

            ret += ch;
            curLine.clear();
        }
        // we need also to expand tabs to properly calc their size
        else if ( ch == '\t' && (flags & wxEllipsizeFlags::ExpandTabs) )
        {
            // Windows natively expands the TABs to 6 spaces. Do the same:
            curLine += "      ";
        }
        else
        {
            curLine += ch;
        }
    }

    return ret;
}

// ----------------------------------------------------------------------------
// wxStaticBitmap
// ----------------------------------------------------------------------------

#if wxUSE_STATBMP

wxSize wxStaticBitmapBase::DoGetBestSize() const
{
    // the fall back size is completely arbitrary
    const wxBitmap bmp = GetBitmap();
    return bmp.IsOk() ? bmp.GetScaledSize() : wxSize(16, 16);
}

#endif // wxUSE_STATBMP

#endif // wxUSE_CONTROLS
