/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/stattextcmn.cpp
// Purpose:     common (to all ports) wxStaticText functions
// Author:      Vadim Zeitlin, Francesco Montorsi
// Created:     2007-01-07 (extracted from dlgcmn.cpp)
// Copyright:   (c) 1999-2006 Vadim Zeitlin
//              (c) 2007 Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_STATTEXT

#include "wx/stattext.h"
#include "wx/dcclient.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/settings.h"
#include "wx/containr.h"
#include "wx/textwrapper.h"

#include "wx/private/markupparser.h"

import Utils.Strings;

import <algorithm>;

// ----------------------------------------------------------------------------
// wxTextWrapper
// ----------------------------------------------------------------------------

void wxTextWrapper::Wrap(wxWindow *win, std::string_view text, int widthMax)
{
    const wxClientDC dc(win);

    const auto full_text = wx::utils::StrSplit(text, '\n');

    for ( auto&& tline : full_text )
    {
        std::string line = tline;

        if ( tline != *full_text.begin() )
        {
            // Do this even if the line is empty, except if it's the first one.
            OnNewLine();
        }

        // Is this a special case when wrapping is disabled?
        if ( widthMax < 0 )
        {
            DoOutputLine(line);
            continue;
        }

        for ( bool newLine = false; !line.empty(); newLine = true )
        {
            if ( newLine )
                OnNewLine();

            std::vector<int> widths = dc.GetPartialTextExtents(line);

            const size_t posEnd = std::ranges::lower_bound(widths, widthMax) - widths.begin();

            // Does the entire remaining line fit?
            if ( posEnd == line.length() )
            {
                DoOutputLine(line);
                break;
            }

            // Find the last word to chop off.
            const size_t lastSpace = line.rfind(' ', posEnd);
            if ( lastSpace == std::string::npos )
            {
                // No spaces, so can't wrap.
                DoOutputLine(line);
                break;
            }

            // Output the part that fits.
            DoOutputLine(line.substr(0, lastSpace));

            // And redo the layout with the rest.
            line = line.substr(lastSpace + 1);
        }
    }
}


// ----------------------------------------------------------------------------
// wxLabelWrapper: helper class for wxStaticTextBase::Wrap()
// ----------------------------------------------------------------------------

class wxLabelWrapper : public wxTextWrapper
{
public:
    void WrapLabel(wxWindow *text, int widthMax)
    {
        m_text.clear();
        Wrap(text, text->GetLabel(), widthMax);
        text->SetLabel(m_text);
    }

protected:
    void OnOutputLine(const std::string& line) override
    {
        m_text += line;
    }

    void OnNewLine() override
    {
        m_text += '\n';
    }

private:
    std::string m_text;
};


// ----------------------------------------------------------------------------
// wxStaticTextBase
// ----------------------------------------------------------------------------

void wxStaticTextBase::Wrap(int width)
{
    wxLabelWrapper wrapper;
    wrapper.WrapLabel(this, width);
}

void wxStaticTextBase::AutoResizeIfNecessary()
{
    // This flag is specifically used to prevent the control from resizing even
    // when its label changes.
    if ( HasFlag(wxST_NO_AUTORESIZE) )
        return;

    // This method is only called if either the label or the font changed, i.e.
    // if the label extent changed, so the best size is not the same neither
    // any more.
    //
    // Note that we don't invalidate it when wxST_NO_AUTORESIZE is on because
    // this would result in the control being effectively resized during the
    // next Layout() and this style is used expressly to prevent this from
    // happening.
    InvalidateBestSize();

    SetSize(GetBestSize());
}

// ----------------------------------------------------------------------------
// wxStaticTextBase - generic implementation for wxST_ELLIPSIZE_* support
// ----------------------------------------------------------------------------

void wxStaticTextBase::UpdateLabel()
{
    if (!IsEllipsized())
        return;

    const std::string& newlabel = GetEllipsizedLabel();

    // we need to touch the "real" label (i.e. the text set inside the control,
    // using port-specific functions) instead of the string returned by GetLabel().
    //
    // In fact, we must be careful not to touch the original label passed to
    // SetLabel() otherwise GetLabel() will behave in a strange way to the user
    // (e.g. returning a "Ver...ing" instead of "Very long string") !
    if (newlabel == WXGetVisibleLabel())
        return;
    WXSetVisibleLabel(newlabel);
}

std::string wxStaticTextBase::GetEllipsizedLabel() const
{
    // this function should be used only by ports which do not support
    // ellipsis in static texts: we first remove markup (which cannot
    // be handled safely by Ellipsize()) and then ellipsize the result.

    std::string ret(m_labelOrig);

    if (IsEllipsized())
        ret = Ellipsize(ret);

    return ret;
}

std::string wxStaticTextBase::Ellipsize(const std::string& label) const
{
    const wxSize sz(GetClientSize());
    if (sz.x < 2 || sz.y < 2)
    {
        // the size of this window is not valid (yet)
        return label;
    }

    wxClientDC dc(const_cast<wxStaticTextBase*>(this));

    wxEllipsizeMode mode;
    if ( HasFlag(wxST_ELLIPSIZE_START) )
        mode = wxEllipsizeMode::Start;
    else if ( HasFlag(wxST_ELLIPSIZE_MIDDLE) )
        mode = wxEllipsizeMode::Middle;
    else if ( HasFlag(wxST_ELLIPSIZE_END) )
        mode = wxEllipsizeMode::End;
    else
    {
        wxFAIL_MSG( "should only be called if have one of wxST_ELLIPSIZE_XXX" );

        return label;
    }

    return wxControl::Ellipsize(label, dc, mode, sz.x);
}

#endif // wxUSE_STATTEXT
