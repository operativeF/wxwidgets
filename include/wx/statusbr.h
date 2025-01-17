/////////////////////////////////////////////////////////////////////////////
// Name:        wx/statusbr.h
// Purpose:     wxStatusBar class interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     05.02.00
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_STATUSBR_H_BASE_
#define _WX_STATUSBR_H_BASE_

#if wxUSE_STATUSBAR

#include "wx/control.h"

import WX.Cfg.Flags;

import Utils.Geometry;

import <string>;
import <vector>;

inline constexpr std::string_view wxStatusBarNameStr = "statusBar";

// ----------------------------------------------------------------------------
// wxStatusBar constants
// ----------------------------------------------------------------------------

// wxStatusBar styles
inline constexpr unsigned int wxSTB_SIZEGRIP          = 0x0010;
inline constexpr unsigned int wxSTB_SHOW_TIPS         = 0x0020;

inline constexpr unsigned int wxSTB_ELLIPSIZE_START   = 0x0040;
inline constexpr unsigned int wxSTB_ELLIPSIZE_MIDDLE  = 0x0080;
inline constexpr unsigned int wxSTB_ELLIPSIZE_END     = 0x0100;

inline constexpr unsigned int wxSTB_DEFAULT_STYLE     = wxSTB_SIZEGRIP|wxSTB_ELLIPSIZE_END|wxSTB_SHOW_TIPS|wxFULL_REPAINT_ON_RESIZE;

// style flags for wxStatusBar fields
inline constexpr unsigned int wxSB_NORMAL    = 0x0000;
inline constexpr unsigned int wxSB_FLAT      = 0x0001;
inline constexpr unsigned int wxSB_RAISED    = 0x0002;
inline constexpr unsigned int wxSB_SUNKEN    = 0x0003;

// ----------------------------------------------------------------------------
// wxStatusBarPane: an helper for wxStatusBar
// ----------------------------------------------------------------------------

class wxStatusBarPane
{
public:
    wxStatusBarPane(unsigned int style = wxSB_NORMAL, int width = 0)
        : m_nStyle(style), m_nWidth(width)
    {
    }

    int GetWidth() const { return m_nWidth; }
    unsigned int GetStyle() const { return m_nStyle; }
    std::string GetText() const { return m_text; }


    // implementation-only from now on
    // -------------------------------

    bool IsEllipsized() const
        { return m_bEllipsized; }
    void SetIsEllipsized(bool isEllipsized) { m_bEllipsized = isEllipsized; }

    void SetWidth(int width) { m_nWidth = width; }
    void SetStyle(unsigned int style) { m_nStyle = style; }

    // set text, return true if it changed or false if it was already set to
    // this value
    bool SetText(std::string_view text);

    // save the existing text on top of our stack and make the new text
    // current; return true if the text really changed
    bool PushText(const std::string& text);

    // restore the message saved by the last call to Push() (unless it was
    // changed by an intervening call to SetText()) and return true if we
    // really restored anything
    bool PopText();

private:
    std::string m_text;

    std::vector<std::string> m_arrStack;

    unsigned int m_nStyle;
    int m_nWidth;     // may be negative, indicating a variable-width field

    // the array used to keep the previous values of this pane after a
    // PushStatusText() call, its top element is the value to restore after the
    // next PopStatusText() call while the currently shown value is always in
    // m_text

    // is the currently shown value shown with ellipsis in the status bar?
    bool m_bEllipsized{false};
};

using wxStatusBarPaneArray = std::vector<wxStatusBarPane>;

// ----------------------------------------------------------------------------
// wxStatusBar: a window near the bottom of the frame used for status info
// ----------------------------------------------------------------------------

class wxStatusBarBase : public wxControl
{
public:
    ~wxStatusBarBase();

    wxStatusBarBase& operator=(wxStatusBarBase&&) = delete;

    // field count
    // -----------

    // set the number of fields and call SetStatusWidths(widths) if widths are
    // given
    virtual void SetFieldsCount(int number = 1, const int *widths = nullptr);
    int GetFieldsCount() const { return (int)m_panes.size(); }

    // field text
    // ----------

    // just change or get the currently shown text
    void SetStatusText(std::string_view text, int number = 0);
    std::string GetStatusText(int number = 0) const;

    // change the currently shown text to the new one and save the current
    // value to be restored by the next call to PopStatusText()
    void PushStatusText(const std::string& text, int number = 0);
    void PopStatusText(int number = 0);

    // fields widths
    // -------------

    // set status field widths as absolute numbers: positive widths mean that
    // the field has the specified absolute width, negative widths are
    // interpreted as the sizer options, i.e. the extra space (total space
    // minus the sum of fixed width fields) is divided between the fields with
    // negative width according to the abs value of the width (field with width
    // -2 grows twice as much as one with width -1 &c)
    virtual void SetStatusWidths(int n, const int widths[]);

    int GetStatusWidth(int n) const
        { return m_panes[n].GetWidth(); }

    // field styles
    // ------------

    // Set the field border style to one of wxSB_XXX values.
    virtual void SetStatusStyles(int n, const int styles[]);

    int GetStatusStyle(int n) const
        { return m_panes[n].GetStyle(); }

    // geometry
    // --------

    // Get the position and size of the field's internal bounding rectangle
    virtual bool GetFieldRect(int i, wxRect& rect) const = 0;

    // sets the minimal vertical size of the status bar
    virtual void SetMinHeight(int height) = 0;

    // get the dimensions of the horizontal and vertical borders
    virtual int GetBorderX() const = 0;
    virtual int GetBorderY() const = 0;

    wxSize GetBorders() const
        { return wxSize(GetBorderX(), GetBorderY()); }

    // miscellaneous
    // -------------

    const wxStatusBarPane& GetField(int n) const
        { return m_panes[n]; }

    // wxWindow overrides:

    // don't want status bars to accept the focus at all
    bool AcceptsFocus() const override { return false; }

    // the client size of a toplevel window doesn't include the status bar
    bool CanBeOutsideClientArea() const override { return true; }

protected:
    // called after the status bar pane text changed and should update its
    // display
    virtual void DoUpdateStatusText(int number) = 0;


    // wxWindow overrides:

#if wxUSE_TOOLTIPS
   void DoSetToolTip( wxToolTip *tip ) override
        {
            wxASSERT_MSG(!HasFlag(wxSTB_SHOW_TIPS),
                         "Do not set tooltip(s) manually when using wxSTB_SHOW_TIPS!");
            wxWindow::DoSetToolTip(tip);
        }
#endif // wxUSE_TOOLTIPS
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }


    // internal helpers & data:

    // calculate the real field widths for the given total available size
    std::vector<int> CalculateAbsWidths(wxCoord widthTotal) const;

    // should be called to remember if the pane text is currently being show
    // ellipsized or not
    void SetEllipsizedFlag(int n, bool isEllipsized);


    // the array with the pane infos:
    wxStatusBarPaneArray m_panes;

    // if true overrides the width info of the wxStatusBarPanes
    bool m_bSameWidthForAllPanes{true};
};

// ----------------------------------------------------------------------------
// include the actual wxStatusBar class declaration
// ----------------------------------------------------------------------------

#if defined(__WXUNIVERSAL__)
    #define wxStatusBarUniv wxStatusBar
    #include "wx/univ/statusbr.h"
#elif defined(__WXMSW__) && wxUSE_NATIVE_STATUSBAR
    #include "wx/msw/statusbar.h"
#elif defined(__WXMAC__)
    #define wxStatusBarMac wxStatusBar
    #include "wx/generic/statusbr.h"
    #include "wx/osx/statusbr.h"
#elif defined(__WXQT__)
    #include "wx/qt/statusbar.h"
#else
    #define wxStatusBarGeneric wxStatusBar
    #include "wx/generic/statusbr.h"
#endif

#endif // wxUSE_STATUSBAR

#endif
    // _WX_STATUSBR_H_BASE_
