/////////////////////////////////////////////////////////////////////////////
// Name:        wx/html/htmlwin.h
// Purpose:     wxHtmlWindow class for parsing & displaying HTML
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HTMLWIN_H_
#define _WX_HTMLWIN_H_

#if wxUSE_HTML

#include "wx/scrolwin.h"
#include "wx/config.h"
#include "wx/html/winpars.h"
#include "wx/html/htmlcell.h"
#include "wx/html/htmlfilt.h"
#include "wx/bitmap.h"

import Utils.Geometry;
import WX.File.Filename;

import WX.Cmn.Stopwatch;

class wxHtmlProcessor;
class wxHtmlWinModule;
class wxHtmlProcessorList;

class wxHtmlWinAutoScrollTimer;
class wxHtmlCellEvent;
class wxHtmlLinkEvent;
class wxStatusBar;
class wxFileSystem;
class wxWindow;

// wxHtmlWindow flags:
// FIXME: Bitfield
#define wxHW_SCROLLBAR_NEVER    0x0002
#define wxHW_SCROLLBAR_AUTO     0x0004
#define wxHW_NO_SELECTION       0x0008

#define wxHW_DEFAULT_STYLE      wxHW_SCROLLBAR_AUTO

/// Enum for wxHtmlWindow::OnOpeningURL and wxHtmlWindowInterface::OnOpeningURL
enum class wxHtmlOpeningStatus
{
    Open,     /// Open the requested URL
    Block,    /// Do not open the URL
    Redirect  /// Redirect to another URL (returned from OnOpeningURL)
};

//-----------------------------------------------------------------------------
// wxHtmlHistoryItem
//-----------------------------------------------------------------------------

// item of history list
class wxHtmlHistoryItem
{
public:
    wxHtmlHistoryItem(const std::string& p, const std::string& a) : m_Page(p), m_Anchor(a), m_Pos(0) { }
    int GetPos() const {return m_Pos;}
    void SetPos(int p) {m_Pos = p;}
    const std::string& GetPage() const {return m_Page;}
    const std::string& GetAnchor() const {return m_Anchor;}

private:
    std::string m_Page;
    std::string m_Anchor;
    int m_Pos;
};

/**
    Abstract interface to a HTML rendering window (such as wxHtmlWindow or
    wxHtmlListBox) that is passed to wxHtmlWinParser. It encapsulates all
    communication from the parser to the window.
 */
class wxHtmlWindowInterface
{
public:
    virtual ~wxHtmlWindowInterface() = default;

    /**
        Called by the parser to set window's title to given text.
     */
    virtual void SetHTMLWindowTitle(const std::string& title) = 0;

    /**
        Called when a link is clicked.

        @param link information about the clicked link
     */
    virtual void OnHTMLLinkClicked(const wxHtmlLinkInfo& link) = 0;

    /**
        Called when the parser needs to open another URL (e.g. an image).

        @param type     Type of the URL request (e.g. image)
        @param url      URL the parser wants to open
        @param redirect If the return value is wxHtmlOpeningStatus::Redirect, then the
                        URL to redirect to will be stored in this variable
                        (the pointer must never be NULL)

        @return indicator of how to treat the request
     */
    virtual wxHtmlOpeningStatus OnHTMLOpeningURL(wxHtmlURLType type,
                                                 const std::string& url,
                                                 std::string *redirect) const = 0;

    /**
        Converts coordinates @a pos relative to given @a cell to
        physical coordinates in the window.
     */
    virtual wxPoint HTMLCoordsToWindow(wxHtmlCell *cell,
                                       const wxPoint& pos) const = 0;

    /// Returns the window used for rendering (may be NULL).
    virtual wxWindow* GetHTMLWindow() = 0;

    /// Returns background colour to use by default.
    virtual wxColour GetHTMLBackgroundColour() const = 0;

    /// Sets window's background to colour @a clr.
    virtual void SetHTMLBackgroundColour(const wxColour& clr) = 0;

    /// Sets window's background to given bitmap.
    virtual void SetHTMLBackgroundImage(const wxBitmap& bmpBg) = 0;

    /// Sets status bar text.
    virtual void SetHTMLStatusText(const std::string& text) = 0;

    /// Type of mouse cursor
    enum class HTMLCursor
    {
        /// Standard mouse cursor (typically an arrow)
        Default,
        /// Cursor shown over links
        Link,
        /// Cursor shown over selectable text
        Text
    };

    /**
        Returns mouse cursor of given @a type.
     */
    virtual wxCursor GetHTMLCursor(HTMLCursor type) const = 0;
};

/**
    Helper class that implements part of mouse handling for wxHtmlWindow and
    wxHtmlListBox. Cursor changes and clicking on links are handled, text
    selection is not.
 */
class wxHtmlWindowMouseHelper
{
protected:
    /**
        Ctor.

        @param iface Interface to the owner window.
     */
    wxHtmlWindowMouseHelper(wxHtmlWindowInterface *iface);

    /**
        Virtual dtor.

        It is not really needed in this case, but at least it prevents gcc from
        complaining about its absence.
     */
    virtual ~wxHtmlWindowMouseHelper() = default;

    /// Returns true if the mouse moved since the last call to HandleIdle
    bool DidMouseMove() const { return m_tmpMouseMoved; }

    /// Call this from EVT_MOTION event handler
    void HandleMouseMoved();

    /**
        Call this from EVT_LEFT_UP handler (or, alternatively, EVT_LEFT_DOWN).

        @param rootCell HTML cell inside which the click occurred. This doesn't
                        have to be the leaf cell, it can be e.g. toplevel
                        container, but the mouse must be inside the container's
                        area, otherwise the event would be ignored.
        @param pos      Mouse position in coordinates relative to @a cell
        @param event    The event that triggered the call
     */
    bool HandleMouseClick(wxHtmlCell *rootCell,
                          const wxPoint& pos, const wxMouseEvent& event);

    /**
        Call this from OnInternalIdle of the HTML displaying window. Handles
        mouse movements and must be used together with HandleMouseMoved.

        @param rootCell HTML cell inside which the click occurred. This doesn't
                        have to be the leaf cell, it can be e.g. toplevel
                        container, but the mouse must be inside the container's
                        area, otherwise the event would be ignored.
        @param pos      Current mouse position in coordinates relative to
                        @a cell
     */
    void HandleIdle(wxHtmlCell *rootCell, const wxPoint& pos);

    /**
        Called by HandleIdle when the mouse hovers over a cell. Default
        behaviour is to do nothing.

        @param cell   the cell the mouse is over
        @param x, y   coordinates of mouse relative to the cell
     */
    virtual void OnCellMouseHover(wxHtmlCell *cell, wxCoord x, wxCoord y);

    /**
        Called by HandleMouseClick when the user clicks on a cell.
        Default behaviour is to call wxHtmlWindowInterface::OnLinkClicked()
        if this cell corresponds to a hypertext link.

        @param cell   the cell the mouse is over
        @param x, y   coordinates of mouse relative to the cell
        @param event    The event that triggered the call


        @return true if a link was clicked, false otherwise.
     */
    virtual bool OnCellClicked(wxHtmlCell *cell,
                               wxCoord x, wxCoord y,
                               const wxMouseEvent& event);

protected:
    // this flag indicates if the mouse moved (used by HandleIdle)
    bool m_tmpMouseMoved;
    // contains last link name
    wxHtmlLinkInfo *m_tmpLastLink;
    // contains the last (terminal) cell which contained the mouse
    wxHtmlCell *m_tmpLastCell;

private:
    wxHtmlWindowInterface *m_interface;
};

using wxHtmlHistoryArray = std::vector<std::unique_ptr<wxHtmlHistoryItem>>;

// ----------------------------------------------------------------------------
// wxHtmlWindow
//                  (This is probably the only class you will directly use.)
//                  Purpose of this class is to display HTML page (either local
//                  file or downloaded via HTTP protocol) in a window. Width of
//                  window is constant - given in constructor - virtual height
//                  is changed dynamically depending on page size.  Once the
//                  window is created you can set its content by calling
//                  SetPage(text) or LoadPage(filename).
// ----------------------------------------------------------------------------

class wxHtmlWindow : public wxScrolledWindow,
                                      public wxHtmlWindowInterface,
                                      public wxHtmlWindowMouseHelper
{
    wxDECLARE_DYNAMIC_CLASS(wxHtmlWindow);
    friend class wxHtmlWinModule;

public:
    wxHtmlWindow() : wxHtmlWindowMouseHelper(this) {}
    wxHtmlWindow(wxWindow *parent, wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = wxHW_DEFAULT_STYLE,
                 const std::string& name = "htmlWindow")
        : wxHtmlWindowMouseHelper(this)
    {
        Create(parent, id, pos, size, style, name);
        m_Parser->SetFS(m_FS);
        SetBorders(10);
    }

    ~wxHtmlWindow();

    wxHtmlWindow(const wxHtmlWindow&) = delete;
	wxHtmlWindow& operator=(const wxHtmlWindow&) = delete;

    bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxHW_SCROLLBAR_AUTO,
                const std::string& name = "htmlWindow");

    // Set HTML page and display it. !! source is HTML document itself,
    // it is NOT address/filename of HTML document. If you want to
    // specify document location, use LoadPage() instead
    // Return value : false if an error occurred, true otherwise
    virtual bool SetPage(const std::string& source);

    // Append to current page
    bool AppendToPage(const std::string& source);

    // Load HTML page from given location. Location can be either
    // a) /usr/wxGTK2/docs/html/wx.htm
    // b) http://www.somewhere.uk/document.htm
    // c) ftp://ftp.somesite.cz/pub/something.htm
    // In case there is no prefix (http:,ftp:), the method
    // will try to find it itself (1. local file, then http or ftp)
    // After the page is loaded, the method calls SetPage() to display it.
    // Note : you can also use path relative to previously loaded page
    // Return value : same as SetPage
    virtual bool LoadPage(const std::string& location);

    // Loads HTML page from file
    bool LoadFile(const wxFileName& filename);

    // Returns full location of opened page
    std::string GetOpenedPage() const {return m_OpenedPage;}
    // Returns anchor within opened page
    std::string GetOpenedAnchor() const {return m_OpenedAnchor;}
    // Returns <TITLE> of opened page or empty string otherwise
    std::string GetOpenedPageTitle() const {return m_OpenedPageTitle;}

    // Sets frame in which page title will  be displayed. Format is format of
    // frame title, e.g. "HtmlHelp : %s". It must contain exactly one %s
    void SetRelatedFrame(wxFrame* frame, const std::string& format);
    wxFrame* GetRelatedFrame() const {return m_RelatedFrame;}

#if wxUSE_STATUSBAR
    // After(!) calling SetRelatedFrame, this sets statusbar slot where messages
    // will be displayed. Default is -1 = no messages.
    void SetRelatedStatusBar(int index);
    void SetRelatedStatusBar(wxStatusBar*, int index = 0);
#endif // wxUSE_STATUSBAR

    // Sets fonts to be used when displaying HTML page.
    void SetFonts(const std::string& normal_face, const std::string& fixed_face,
                  const int *sizes = nullptr);

    // Sets font sizes to be relative to the given size or the system
    // default size; use either specified or default font
    void SetStandardFonts(int size = -1,
                          const std::string& normal_face = {},
                          const std::string& fixed_face = {});

    // Sets space between text and window borders.
    void SetBorders(int b) {m_Borders = b;}

    // Sets the bitmap to use for background (currnetly it will be tiled,
    // when/if we have CSS support we could add other possibilities...)
    void SetBackgroundImage(const wxBitmap& bmpBg) { m_bmpBg = bmpBg; }

#if wxUSE_CONFIG
    // Saves custom settings into cfg config. it will use the path 'path'
    // if given, otherwise it will save info into currently selected path.
    // saved values : things set by SetFonts, SetBorders.
    virtual void ReadCustomization(wxConfigBase *cfg, std::string path = {});
    // ...
    virtual void WriteCustomization(wxConfigBase *cfg, std::string path = {});
#endif // wxUSE_CONFIG

    // Goes to previous/next page (in browsing history)
    // Returns true if successful, false otherwise
    bool HistoryBack();
    bool HistoryForward();
    bool HistoryCanBack();
    bool HistoryCanForward();
    // Resets history
    void HistoryClear();

    // Returns pointer to conteiners/cells structure.
    // It should be used ONLY when printing
    wxHtmlContainerCell* GetInternalRepresentation() const {return m_Cell;}

    // Adds input filter
    static void AddFilter(wxHtmlFilter *filter);

    // Returns a pointer to the parser.
    wxHtmlWinParser *GetParser() const { return m_Parser; }

    // Adds HTML processor to this instance of wxHtmlWindow:
    void AddProcessor(wxHtmlProcessor *processor);
    // Adds HTML processor to wxHtmlWindow class as whole:
    static void AddGlobalProcessor(wxHtmlProcessor *processor);


    // -- Callbacks --

    // Sets the title of the window
    // (depending on the information passed to SetRelatedFrame() method)
    virtual void OnSetTitle(const std::string& title);

    // Called when user clicked on hypertext link. Default behaviour is to
    // call LoadPage(loc)
    virtual void OnLinkClicked(const wxHtmlLinkInfo& link);

    // Called when wxHtmlWindow wants to fetch data from an URL (e.g. when
    // loading a page or loading an image). The data are downloaded if and only if
    // OnOpeningURL returns true. If OnOpeningURL returns wxHtmlOpeningStatus::Redirect,
    // it must set *redirect to the new URL
    virtual wxHtmlOpeningStatus OnOpeningURL([[maybe_unused]] wxHtmlURLType type,
                                             [[maybe_unused]] const std::string& url,
                                             [[maybe_unused]] std::string *redirect) const
        { return wxHtmlOpeningStatus::Open; }

#if wxUSE_CLIPBOARD
    // Helper functions to select parts of page:
    void SelectWord(const wxPoint& pos);
    void SelectLine(const wxPoint& pos);
    void SelectAll();

    // Convert selection to text:
    std::string SelectionToText() { return DoSelectionToText(m_selection); }

    // Converts current page to text:
    std::string ToText();
#endif // wxUSE_CLIPBOARD

    void OnInternalIdle() override;

    /// Returns standard HTML cursor as used by wxHtmlWindow
    static wxCursor GetDefaultHTMLCursor(HTMLCursor type);
    static void SetDefaultHTMLCursor(HTMLCursor type, const wxCursor& cursor);

protected:
    // Scrolls to anchor of this name. (Anchor is #news
    // or #features etc. it is part of address sometimes:
    // http://www.ms.mff.cuni.cz/~vsla8348/wxhtml/index.html#news)
    // Return value : true if anchor exists, false otherwise
    bool ScrollToAnchor(const std::string& anchor);

    // Prepares layout (= fill m_PosX, m_PosY for fragments) based on
    // actual size of window. This method also setup scrollbars
    void CreateLayout();

    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnDPIChanged(wxDPIChangedEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnFocusEvent(wxFocusEvent& event);
#if wxUSE_CLIPBOARD
    void OnKeyUp(wxKeyEvent& event);
    void OnDoubleClick(wxMouseEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnClipboardEvent(wxClipboardTextEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
#endif // wxUSE_CLIPBOARD

    // Returns new filter (will be stored into m_DefaultFilter variable)
    virtual wxHtmlFilter *GetDefaultFilter() {return new wxHtmlFilterPlainText;}

    // cleans static variables
    static void CleanUpStatics();

    // Returns true if text selection is enabled (wxClipboard must be available
    // and wxHW_NO_SELECTION not used)
    bool IsSelectionEnabled() const;

    enum class ClipboardType
    {
        Primary,
        Secondary
    };

    // Copies selection to clipboard if the clipboard support is available
    //
    // returns true if anything was copied to clipboard, false otherwise
    bool CopySelection(ClipboardType t = ClipboardType::Secondary);

#if wxUSE_CLIPBOARD
    // Automatic scrolling during selection:
    void StopAutoScrolling();
#endif // wxUSE_CLIPBOARD

    std::string DoSelectionToText(wxHtmlSelection *sel);

public:
    // wxHtmlWindowInterface methods:
    void SetHTMLWindowTitle(const std::string& title) override;
    void OnHTMLLinkClicked(const wxHtmlLinkInfo& link) override;
    wxHtmlOpeningStatus OnHTMLOpeningURL(wxHtmlURLType type,
                                                 const std::string& url,
                                                 std::string *redirect) const override;
    wxPoint HTMLCoordsToWindow(wxHtmlCell *cell,
                                       const wxPoint& pos) const override;
    wxWindow* GetHTMLWindow() override;
    wxColour GetHTMLBackgroundColour() const override;
    void SetHTMLBackgroundColour(const wxColour& clr) override;
    void SetHTMLBackgroundImage(const wxBitmap& bmpBg) override;
    void SetHTMLStatusText(const std::string& text) override;
    wxCursor GetHTMLCursor(HTMLCursor type) const override;

    // implementation of SetPage()
    bool DoSetPage(const std::string& source);

protected:
    // This is pointer to the first cell in parsed data.  (Note: the first cell
    // is usually top one = all other cells are sub-cells of this one)
    wxHtmlContainerCell *m_Cell{nullptr};
    // parser which is used to parse HTML input.
    // Each wxHtmlWindow has its own parser because sharing one global
    // parser would be problematic (because of reentrancy)
    wxHtmlWinParser *m_Parser{new wxHtmlWinParser(this)};
    // contains name of actually opened page or empty string if no page opened
    std::string m_OpenedPage;
    // contains name of current anchor within m_OpenedPage
    std::string m_OpenedAnchor;
    // contains title of actually opened page or empty string if no <TITLE> tag
    std::string m_OpenedPageTitle;
    // class for opening files (file system)
    wxFileSystem* m_FS{new wxFileSystem()};

    // frame in which page title should be displayed & number of its statusbar
    // reserved for usage with this html window
    wxFrame *m_RelatedFrame{nullptr};
#if wxUSE_STATUSBAR
    int m_RelatedStatusBarIndex{-1};

    wxStatusBar* m_RelatedStatusBar{nullptr};
#endif // wxUSE_STATUSBAR
    std::string m_TitleFormat{"%s"};

    // borders (free space between text and window borders)
    // defaults to 10 pixels.
    int m_Borders;

    // current text selection or NULL
    wxHtmlSelection *m_selection{nullptr};

    // true if the user is dragging mouse to select text
    bool m_makingSelection{false};

#if wxUSE_CLIPBOARD
    // time of the last double-click event, used to detect triple clicks
    // (triple clicks are used to select whole line):
    wxMilliClock_t m_lastDoubleClick{0};

    // helper class to automatically scroll the window if the user is selecting
    // text and the mouse leaves wxHtmlWindow:
    wxHtmlWinAutoScrollTimer *m_timerAutoScroll{nullptr};
#endif // wxUSE_CLIPBOARD

private:
    // erase the window background using m_bmpBg or just solid colour if we
    // don't have any background image
    void DoEraseBackground(wxDC& dc);

    // window content for double buffered rendering, may be invalid until it is
    // really initialized in OnPaint()
    wxBitmap m_backBuffer;

    // background image, may be invalid
    wxBitmap m_bmpBg;

    // variables used when user is selecting text
    wxPoint     m_tmpSelFromPos;
    wxHtmlCell *m_tmpSelFromCell{nullptr};

    // if >0 contents of the window is not redrawn
    // (in order to avoid ugly blinking)
    int m_tmpCanDrawLocks{};

    // list of HTML filters
    static wxList m_Filters;
    // this filter is used when no filter is able to read some file
    static wxHtmlFilter *m_DefaultFilter;

    // html processors array:
    wxHtmlProcessorList *m_Processors{nullptr};
    static wxHtmlProcessorList *m_GlobalProcessors;

    // browser history
    wxHtmlHistoryArray m_History;
    int m_HistoryPos{-1};
    // if this FLAG is false, items are not added to history
    bool m_HistoryOn{true};

    // Flag used to communicate between OnPaint() and OnEraseBackground(), see
    // the comments near its use.
    bool m_isBgReallyErased;

    // standard mouse cursors
    static wxCursor *ms_cursorLink;
    static wxCursor *ms_cursorText;
    static wxCursor *ms_cursorDefault;

    wxDECLARE_EVENT_TABLE();
};

class wxHtmlCellEvent;

wxDECLARE_EVENT( wxEVT_HTML_CELL_CLICKED, wxHtmlCellEvent );
wxDECLARE_EVENT( wxEVT_HTML_CELL_HOVER, wxHtmlCellEvent );
wxDECLARE_EVENT( wxEVT_HTML_LINK_CLICKED, wxHtmlLinkEvent );


/*!
 * Html cell window event
 */

class wxHtmlCellEvent : public wxCommandEvent
{
public:
    wxHtmlCellEvent() = default;
    wxHtmlCellEvent(wxEventType commandType, int id,
                    wxHtmlCell *cell, const wxPoint &pt,
                    const wxMouseEvent &ev)
        : wxCommandEvent(commandType, id)
        , m_mouseEvent(ev)
        , m_pt(pt)
    {
        m_cell = cell;
        m_bLinkWasClicked = false;
    }

	wxHtmlCellEvent& operator=(const wxHtmlCellEvent&) = delete;

    wxHtmlCell* GetCell() const { return m_cell; }
    wxPoint GetPoint() const { return m_pt; }
    wxMouseEvent GetMouseEvent() const { return m_mouseEvent; }

    void SetLinkClicked(bool linkclicked) { m_bLinkWasClicked=linkclicked; }
    bool GetLinkClicked() const { return m_bLinkWasClicked; }

    // default copy ctor, assignment operator and dtor are ok
    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxHtmlCellEvent>(*this); }

private:
    wxHtmlCell *m_cell;
    wxMouseEvent m_mouseEvent;
    wxPoint m_pt;

    bool m_bLinkWasClicked;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};



/*!
 * Html link event
 */

class wxHtmlLinkEvent : public wxCommandEvent
{
public:
    wxHtmlLinkEvent() = default;
    wxHtmlLinkEvent(int id, const wxHtmlLinkInfo &linkinfo)
        : wxCommandEvent(wxEVT_HTML_LINK_CLICKED, id)
        , m_linkInfo(linkinfo)
    {
    }

    const wxHtmlLinkInfo &GetLinkInfo() const { return m_linkInfo; }

    // default copy ctor, assignment operator and dtor are ok
    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxHtmlLinkEvent>(*this); }

private:
    wxHtmlLinkInfo m_linkInfo;

    public:
	wxHtmlLinkEvent& operator=(const wxHtmlLinkEvent&) = delete;
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};


typedef void (wxEvtHandler::*wxHtmlCellEventFunction)(wxHtmlCellEvent&);
typedef void (wxEvtHandler::*wxHtmlLinkEventFunction)(wxHtmlLinkEvent&);

#define wxHtmlCellEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxHtmlCellEventFunction, func)
#define wxHtmlLinkEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxHtmlLinkEventFunction, func)

#define EVT_HTML_CELL_CLICKED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_HTML_CELL_CLICKED, id, wxHtmlCellEventHandler(fn))
#define EVT_HTML_CELL_HOVER(id, fn) \
    wx__DECLARE_EVT1(wxEVT_HTML_CELL_HOVER, id, wxHtmlCellEventHandler(fn))
#define EVT_HTML_LINK_CLICKED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_HTML_LINK_CLICKED, id, wxHtmlLinkEventHandler(fn))

#endif // wxUSE_HTML

#endif // _WX_HTMLWIN_H_

