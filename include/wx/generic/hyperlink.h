/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/hyperlink.h
// Purpose:     Hyperlink control
// Author:      David Norris <danorris@gmail.com>, Otto Wyss
// Modified by: Ryan Norton, Francesco Montorsi
// Created:     04/02/2005
// Copyright:   (c) 2005 David Norris
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERICHYPERLINKCTRL_H_
#define _WX_GENERICHYPERLINKCTRL_H_

// ----------------------------------------------------------------------------
// wxGenericHyperlinkCtrl
// ----------------------------------------------------------------------------

class wxGenericHyperlinkCtrl : public wxHyperlinkCtrlBase
{
public:
    // Default constructor (for two-step construction).
    wxGenericHyperlinkCtrl() = default;

    // Constructor.
    wxGenericHyperlinkCtrl(wxWindow *parent,
                            wxWindowID id,
                            const std::string& label,
                            const std::string& url,
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxDefaultSize,
                            unsigned int style = wxHL_DEFAULT_STYLE,
                            std::string_view name = wxHyperlinkCtrlNameStr)
    {
        Create(parent, id, label, url, pos, size, style, name);
    }

    // Creation function (for two-step construction).
    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& label,
                const std::string& url,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxHL_DEFAULT_STYLE,
                std::string_view name = wxHyperlinkCtrlNameStr);


    // get/set
    wxColour GetHoverColour() const override { return m_hoverColour; }
    void SetHoverColour(const wxColour &colour) override { m_hoverColour = colour; }

    wxColour GetNormalColour() const override { return m_normalColour; }
    void SetNormalColour(const wxColour &colour) override;

    wxColour GetVisitedColour() const override { return m_visitedColour; }
    void SetVisitedColour(const wxColour &colour) override;

    const std::string& GetURL() const override { return m_url; }
    void SetURL (const std::string& url) override { m_url = url; }

    void SetVisited(bool visited = true) override { m_visited=visited; }
    bool GetVisited() const override { return m_visited; }

    // NOTE: also wxWindow::Set/GetLabel, wxWindow::Set/GetBackgroundColour,
    //       wxWindow::Get/SetFont, wxWindow::Get/SetCursor are important !


protected:
    // Helper used by this class itself and native MSW implementation that
    // connects OnRightUp() and OnPopUpCopy() handlers.
    void ConnectMenuHandlers();

    // event handlers

    // Renders the hyperlink.
    void OnPaint(wxPaintEvent& event);

    // Handle set/kill focus events (invalidate for painting focus rect)
    void OnFocus(wxFocusEvent& event);

    // Fire a HyperlinkEvent on space
    void OnChar(wxKeyEvent& event);

    // Returns the wxRect of the label of this hyperlink.
    // This is different from the clientsize's rectangle when
    // clientsize != bestsize and this rectangle is influenced
    // by the alignment of the label (wxHL_ALIGN_*).
    wxRect GetLabelRect() const;

    // If the click originates inside the bounding box of the label,
    // a flag is set so that an event will be fired when the left
    // button is released.
    void OnLeftDown(wxMouseEvent& event);

    // If the click both originated and finished inside the bounding box
    // of the label, a HyperlinkEvent is fired.
    void OnLeftUp(wxMouseEvent& event);
    void OnRightUp(wxMouseEvent& event);

    // Changes the cursor to a hand, if the mouse is inside the label's
    // bounding box.
    void OnMotion(wxMouseEvent& event);

    // Changes the cursor back to the default, if necessary.
    void OnLeaveWindow(wxMouseEvent& event);

    // handles "Copy URL" menuitem
    void OnPopUpCopy(wxCommandEvent& event);

    // overridden base class virtuals

    // Returns the best size for the window, which is the size needed
    // to display the text label.
    wxSize DoGetBestClientSize() const override;

    // creates a context menu with "Copy URL" menuitem
    virtual void DoContextMenu(const wxPoint &);

private:
    // Common part of all ctors.
    

    // URL associated with the link. This is transmitted inside
    // the HyperlinkEvent fired when the user clicks on the label.
    std::string m_url;

    // Foreground colours for various link types.
    // NOTE: wxWindow::m_backgroundColour is used for background,
    //       wxWindow::m_foregroundColour is used to render non-visited links
    wxColour m_hoverColour{*wxRED};
    wxColour m_normalColour{*wxBLUE};
    wxColour m_visitedColour{wxColour("#551a8b")};

    // True if the mouse cursor is inside the label's bounding box.
    bool m_rollover{false};

    // True if the link has been clicked before.
    bool m_visited{false};

    // True if a click is in progress (left button down) and the click
    // originated inside the label's bounding box.
    bool m_clicking{false};
};

#endif // _WX_GENERICHYPERLINKCTRL_H_
