///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/private/markuptext.h
// Purpose:     Generic wx*MarkupText classes for managing text with markup.
// Author:      Vadim Zeitlin
// Created:     2011-02-21
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_PRIVATE_MARKUPTEXT_H_
#define _WX_GENERIC_PRIVATE_MARKUPTEXT_H_

import Utils.Geometry;

enum class wxEllipsizeMode;
class wxDC;

class wxMarkupParserOutput;

// ----------------------------------------------------------------------------
// wxMarkupText: allows to measure and draw the text containing markup.
// ----------------------------------------------------------------------------

class wxMarkupTextBase
{
public:
    virtual ~wxMarkupTextBase() = default;

    // Update the markup string.
    void SetMarkup(const std::string& markup) { m_markup = markup; }

    // Return the width and height required by the given string and optionally
    // the height of the visible part above the baseline (i.e. ascent minus
    // internal leading).
    //
    // The font currently selected into the DC is used for measuring (notice
    // that it is changed by this function but normally -- i.e. if markup is
    // valid -- restored to its original value when it returns).
    wxSize Measure(wxDC& dc, int *visibleHeight = nullptr) const;

protected:
    wxMarkupTextBase(const std::string& markup)
        : m_markup(markup)
    {
    }

    // Return m_markup suitable for measuring by Measure, i.e. stripped of
    // any mnenomics.
    virtual std::string GetMarkupForMeasuring() const = 0;

    std::string m_markup;
};


class wxMarkupText : public wxMarkupTextBase
{
public:
    // Constants for Render() flags.
    enum
    {
        Render_Default = 0,     // Don't show mnemonics visually.
        Render_ShowAccels = 1   // Underline mnemonics.
    };


    // Initialize with the given string containing markup (which is supposed to
    // be valid, the caller must check for it before constructing this object).
    //
    // Notice that the usual rules for mnemonics apply to the markup text: if
    // it contains any '&' characters they must be escaped by doubling them,
    // otherwise they indicate that the next character is the mnemonic for this
    // field.
    //
    // TODO-MULTILINE-MARKUP: Currently only single line labels are supported,
    // search for other occurrences of this comment to find the places which
    // need to be updated to support multiline labels with markup.
    wxMarkupText(const std::string& markup) : wxMarkupTextBase(markup)
    {
    }

    // Default copy ctor, assignment operator and dtor are ok.

    // Update the markup string.
    //
    // The same rules for mnemonics as in the ctor apply to this string.
    void SetMarkup(const std::string& markup) { m_markup = markup; }

    // Render the markup string into the given DC in the specified rectangle.
    //
    // Notice that while the function uses the provided rectangle for alignment
    // (it centers the text in it), no clipping is done by it so use Measure()
    // and set the clipping region before rendering if necessary.
    void Render(wxDC& dc, const wxRect& rect, int flags);

protected:
    std::string GetMarkupForMeasuring() const override;
};


// ----------------------------------------------------------------------------
// wxItemMarkupText: variant of wxMarkupText for items without mnemonics
// ----------------------------------------------------------------------------

// This class has similar interface to wxItemMarkup, but no strings contain
// mnemonics and no escaping is done.
class wxItemMarkupText : public wxMarkupTextBase
{
public:
    // Initialize with the given string containing markup (which is supposed to
    // be valid, the caller must check for it before constructing this object).
    // Notice that mnemonics are not interpreted at all by this class, so
    // literal ampersands shouldn't be escaped/doubled.
    wxItemMarkupText(const std::string& markup) : wxMarkupTextBase(markup)
    {
    }

    // Default copy ctor, assignment operator and dtor are ok.

    // Similar to wxMarkupText::Render(), but uses wxRendererNative::DrawItemText()
    // instead of generic wxDC::DrawLabel(), so is more suitable for use in
    // controls that already use DrawItemText() for its items.
    //
    // The meaning of the flags here is different than in wxMarkupText too:
    // they're passed to DrawItemText().
    //
    // Currently the only supported ellipsize modes are wxEllipsizeMode::None and
    // wxEllipsizeMode::End, the others are treated as wxEllipsizeMode::End.
    void Render(wxWindow *win,
                wxDC& dc,
                const wxRect& rect,
                int rendererFlags,
                wxEllipsizeMode ellipsizeMode);

protected:
    std::string GetMarkupForMeasuring() const override { return m_markup; }
};

#endif // _WX_GENERIC_PRIVATE_MARKUPTEXT_H_
