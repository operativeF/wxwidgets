/////////////////////////////////////////////////////////////////////////////
// Name:        wx/univ/textctrl.h
// Purpose:     wxTextCtrl class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     15.09.00
// Copyright:   (c) 2000 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIV_TEXTCTRL_H_
#define _WX_UNIV_TEXTCTRL_H_

class WXDLLIMPEXP_FWD_CORE wxCaret;
class WXDLLIMPEXP_FWD_CORE wxTextCtrlCommandProcessor;

#include "wx/scrolwin.h"    // for wxScrollHelper

#include "wx/univ/inphand.h"

// ----------------------------------------------------------------------------
// wxTextCtrl actions
// ----------------------------------------------------------------------------

// cursor movement and also selection and delete operations
constexpr wxChar wxACTION_TEXT_GOTO[]          = wxT("goto");  // to pos in numArg
constexpr wxChar wxACTION_TEXT_FIRST[]         = wxT("first"); // go to pos 0
constexpr wxChar wxACTION_TEXT_LAST[]          = wxT("last");  // go to last pos
constexpr wxChar wxACTION_TEXT_HOME[]          = wxT("home");
constexpr wxChar wxACTION_TEXT_END[]           = wxT("end");
constexpr wxChar wxACTION_TEXT_LEFT[]          = wxT("left");
constexpr wxChar wxACTION_TEXT_RIGHT[]         = wxT("right");
constexpr wxChar wxACTION_TEXT_UP[]            = wxT("up");
constexpr wxChar wxACTION_TEXT_DOWN[]          = wxT("down");
constexpr wxChar wxACTION_TEXT_WORD_LEFT[]     = wxT("wordleft");
constexpr wxChar wxACTION_TEXT_WORD_RIGHT[]    = wxT("wordright");
constexpr wxChar wxACTION_TEXT_PAGE_UP[]       = wxT("pageup");
constexpr wxChar wxACTION_TEXT_PAGE_DOWN[]     = wxT("pagedown");

// clipboard operations
constexpr wxChar wxACTION_TEXT_COPY[]          = wxT("copy");
constexpr wxChar wxACTION_TEXT_CUT[]           = wxT("cut");
constexpr wxChar wxACTION_TEXT_PASTE[]         = wxT("paste");

// insert text at the cursor position: the text is in strArg of PerformAction
constexpr wxChar wxACTION_TEXT_INSERT[]        = wxT("insert");

// if the action starts with either of these prefixes and the rest of the
// string is one of the movement commands, it means to select/delete text from
// the current cursor position to the new one
constexpr wxChar wxACTION_TEXT_PREFIX_SEL[]    = wxT("sel");
constexpr wxChar wxACTION_TEXT_PREFIX_DEL[]    = wxT("del");

// mouse selection
constexpr wxChar wxACTION_TEXT_ANCHOR_SEL[]    = wxT("anchorsel");
constexpr wxChar wxACTION_TEXT_EXTEND_SEL[]    = wxT("extendsel");
constexpr wxChar wxACTION_TEXT_SEL_WORD[]      = wxT("wordsel");
constexpr wxChar wxACTION_TEXT_SEL_LINE[]      = wxT("linesel");

// undo or redo
constexpr wxChar wxACTION_TEXT_UNDO[]          = wxT("undo");
constexpr wxChar wxACTION_TEXT_REDO[]          = wxT("redo");

// ----------------------------------------------------------------------------
// wxTextCtrl
// ----------------------------------------------------------------------------

class wxTextCtrl : public wxTextCtrlBase,
                               public wxScrollHelper
{
public:
    // creation
    // --------

    wxTextCtrl() : wxScrollHelper(this) { Init(); }

    wxTextCtrl(wxWindow *parent,
               wxWindowID id,
               const wxString& value = wxEmptyString,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxASCII_STR(wxTextCtrlNameStr))
        : wxScrollHelper(this)
    {
        Init();

        Create(parent, id, value, pos, size, style, validator, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& value = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxTextCtrlNameStr));

    ~wxTextCtrl();

    
    // ----------------------------------

    int GetLineLength(wxTextCoord lineNo) const override;
    wxString GetLineText(wxTextCoord lineNo) const override;
    int GetNumberOfLines() const override;

    bool IsModified() const override;
    bool IsEditable() const override;

    void SetMaxLength(unsigned long len) override;
    // If the return values from and to are the same, there is no selection.
    void GetSelection(wxTextPos* from, wxTextPos* to) const override;

    // operations
    // ----------

    // editing
    void Clear() override;
    void Replace(wxTextPos from, wxTextPos to, const wxString& value) override;
    void Remove(wxTextPos from, wxTextPos to) override;

    // sets/clears the dirty flag
    void MarkDirty() override;
    void DiscardEdits() override;

    // writing text inserts it at the current position, appending always
    // inserts it at the end
    void WriteText(const wxString& text) override;
    void AppendText(const wxString& text) override;

    // translate between the position (which is just an index in the text ctrl
    // considering all its contents as a single strings) and (x, y) coordinates
    // which represent (logical, i.e. unwrapped) column and line.
    wxTextPos XYToPosition(wxTextCoord x, wxTextCoord y) const override;
    virtual bool PositionToXY(wxTextPos pos,
                              wxTextCoord *x, wxTextCoord *y) const override;

    // wxUniv-specific: find a screen position (in client coordinates) of the
    // given text position or of the caret
    bool PositionToLogicalXY(wxTextPos pos, wxCoord *x, wxCoord *y) const;
    bool PositionToDeviceXY(wxTextPos pos, wxCoord *x, wxCoord *y) const;
    wxPoint GetCaretPosition() const;

    void ShowPosition(wxTextPos pos) override;

    // Clipboard operations
    void Copy() override;
    void Cut() override;
    void Paste() override;

    // Undo/redo
    void Undo() override;
    void Redo() override;

    bool CanUndo() const override;
    bool CanRedo() const override;

    // Insertion point
    void SetInsertionPoint(wxTextPos pos) override;
    void SetInsertionPointEnd() override;
    wxTextPos GetInsertionPoint() const override;
    wxTextPos GetLastPosition() const override;

    void SetSelection(wxTextPos from, wxTextPos to) override;
    void SetEditable(bool editable) override;

    // wxUniv-specific methods
    // -----------------------

    // caret stuff
    virtual void ShowCaret(bool show = true);
    void HideCaret() { ShowCaret(false); }
    void CreateCaret(); // for the current font size

    // helpers for cursor movement
    wxTextPos GetWordStart() const;
    wxTextPos GetWordEnd() const;

    // selection helpers
    bool HasSelection() const
        { return m_selStart != -1 && m_selEnd > m_selStart; }
    void ClearSelection();
    void RemoveSelection();
    wxString GetSelectionText() const;

    wxTextCtrlHitTestResult HitTest(const wxPoint& pt, long *pos) const override;
    virtual wxTextCtrlHitTestResult HitTest(const wxPoint& pt,
                                            wxTextCoord *col,
                                            wxTextCoord *row) const override;

    // find the character at this position in the given line, return value as
    // for HitTest()
    //
    // NB: x is the logical coord (client and unscrolled)
    wxTextCtrlHitTestResult HitTestLine(const wxString& line,
                                        wxCoord x,
                                        wxTextCoord *colOut) const;

    // bring the given position into view
    void ShowHorzPosition(wxCoord pos);

    // scroll the window horizontally so that the first character shown is in
    // position pos
    void ScrollText(wxTextCoord col);

    // adjust the DC for horz text control scrolling too
    void DoPrepareDC(wxDC& dc) override;

    // implementation only from now on
    // -------------------------------

    // override this to take into account our scrollbar-less scrolling
    virtual void CalcUnscrolledPosition(int x, int y, int *xx, int *yy) const;
    virtual void CalcScrolledPosition(int x, int y, int *xx, int *yy) const;

    // perform an action
    virtual bool PerformAction(const wxControlAction& action,
                               long numArg = -1,
                               const wxString& strArg = wxEmptyString) override;

    static wxInputHandler *GetStdInputHandler(wxInputHandler *handlerDef);
    wxInputHandler *DoGetStdInputHandler(wxInputHandler *handlerDef) override
    {
        return GetStdInputHandler(handlerDef);
    }

    // override these methods to handle the caret
    bool SetFont(const wxFont &font) override;
    bool Enable(bool enable = true) override;

    // more readable flag testing methods
    bool IsPassword() const { return HasFlag(wxTE_PASSWORD); }
    bool WrapLines() const { return m_wrapLines; }

    // only for wxStdTextCtrlInputHandler
    void RefreshSelection();

    // override wxScrollHelper method to prevent (auto)scrolling beyond the end
    // of line
    bool SendAutoScrollEvents(wxScrollWinEvent& event) const override;

    // idle processing
    void OnInternalIdle() override;

protected:
    // ensure we have correct default border
    wxBorder GetDefaultBorder() const override { return wxBORDER_SUNKEN; }

    // override base class methods
    void DoDrawBorder(wxDC& dc, const wxRect& rect) override;
    void DoDraw(wxControlRenderer *renderer) override;

    // calc the size from the text extent
    wxSize DoGetBestClientSize() const override;

    // implements Set/ChangeValue()
    void DoSetValue(const wxString& value, int flags = 0) override;
    wxString DoGetValue() const override;

    // common part of all ctors
    void Init();

    // drawing
    // -------

    // draw the text in the given rectangle
    void DoDrawTextInRect(wxDC& dc, const wxRect& rectUpdate);

    // draw the line wrap marks in this rect
    void DoDrawLineWrapMarks(wxDC& dc, const wxRect& rectUpdate);

    // line/row geometry calculations
    // ------------------------------

    // get the extent (width) of the text
    wxCoord GetTextWidth(const wxString& text) const;

    // get the logical text width (accounting for scrolling)
    wxCoord GetTotalWidth() const;

    // get total number of rows (different from number of lines if the lines
    // can be wrapped)
    wxTextCoord GetRowCount() const;

    // find the number of rows in this line (only if WrapLines())
    wxTextCoord GetRowsPerLine(wxTextCoord line) const;

    // get the starting row of the given line
    wxTextCoord GetFirstRowOfLine(wxTextCoord line) const;

    // get the row following this line
    wxTextCoord GetRowAfterLine(wxTextCoord line) const;

    // refresh functions
    // -----------------

    // the text area is the part of the window in which the text can be
    // displayed, i.e. part of it inside the margins and the real text area is
    // the area in which the text *is* currently displayed: for example, in the
    // multiline control case the text area can have extra space at the bottom
    // which is not tall enough for another line and which is then not included
    // into the real text area
    wxRect GetRealTextArea() const;

    // refresh the text in the given (in logical coords) rect
    void RefreshTextRect(const wxRect& rect, bool textOnly = true);

    // refresh the line wrap marks for the given range of lines (inclusive)
    void RefreshLineWrapMarks(wxTextCoord rowFirst, wxTextCoord rowLast);

    // refresh the text in the given range (in logical coords) of this line, if
    // width is 0, refresh to the end of line
    void RefreshPixelRange(wxTextCoord line, wxCoord start, wxCoord width);

    // refresh the text in the given range (in text coords) in this line
    void RefreshColRange(wxTextCoord line, wxTextPos start, size_t count);

    // refresh the text from in the given line range (inclusive)
    void RefreshLineRange(wxTextCoord lineFirst, wxTextCoord lineLast);

    // refresh the text in the given range which can span multiple lines
    // (this method accepts arguments in any order)
    void RefreshTextRange(wxTextPos start, wxTextPos end);

    // get the text to show: either the text itself or the text replaced with
    // starts for wxTE_PASSWORD control
    wxString GetTextToShow(const wxString& text) const;

    // find the row in this line where the given position (counted from the
    // start of line) is
    wxTextCoord GetRowInLine(wxTextCoord line,
                             wxTextCoord col,
                             wxTextCoord *colRowStart = NULL) const;

    // find the number of characters of a line before it wraps
    // (and optionally also the real width of the line)
    size_t GetPartOfWrappedLine(const wxChar* text,
                                wxCoord *widthReal = NULL) const;

    // get the start and end of the selection for this line: if the line is
    // outside the selection, both will be -1 and false will be returned
    bool GetSelectedPartOfLine(wxTextCoord line,
                               wxTextPos *start, wxTextPos *end) const;

    // update the text rect: the zone inside our client rect (its coords are
    // client coords) which contains the text
    void UpdateTextRect();

    // calculate the last visible position
    void UpdateLastVisible();

    // move caret to the given position unconditionally
    // (SetInsertionPoint() does nothing if the position didn't change)
    void DoSetInsertionPoint(wxTextPos pos);

    // move caret to the new position without updating the display (for
    // internal use only)
    void MoveInsertionPoint(wxTextPos pos);

    // set the caret to its initial (default) position
    void InitInsertionPoint();

    // get the width of the longest line in pixels
    wxCoord GetMaxWidth() const;

    // force recalculation of the max line width
    void RecalcMaxWidth();

    // update the max width after the given line was modified
    void UpdateMaxWidth(wxTextCoord line);

    // hit testing
    // -----------

    // HitTest2() is more efficient than 2 consecutive HitTest()s with the same
    // line (i.e. y) and it also returns the offset of the starting position in
    // pixels
    //
    // as the last hack, this function accepts either logical or device (by
    // default) coords depending on devCoords flag
    wxTextCtrlHitTestResult HitTest2(wxCoord y,
                                     wxCoord x1,
                                     wxCoord x2,
                                     wxTextCoord *row,
                                     wxTextCoord *colStart,
                                     wxTextCoord *colEnd,
                                     wxTextCoord *colRowStart,
                                     bool devCoords = true) const;

    // HitTest() version which takes the logical text coordinates and not the
    // device ones
    wxTextCtrlHitTestResult HitTestLogical(const wxPoint& pos,
                                           wxTextCoord *col,
                                           wxTextCoord *row) const;

    // get the line and the row in this line corresponding to the given row,
    // return true if ok and false if row is out of range
    //
    // NB: this function can only be called for controls which wrap lines
    bool GetLineAndRow(wxTextCoord row,
                       wxTextCoord *line,
                       wxTextCoord *rowInLine) const;

    // get the height of one line (the same for all lines)
    wxCoord GetLineHeight() const
    {
        // this one should be already precalculated
        wxASSERT_MSG( m_heightLine != -1, wxT("should have line height") );

        return m_heightLine;
    }

    // get the average char width
    wxCoord GetAverageWidth() const { return m_widthAvg; }

    // recalc the line height and char width (to call when the font changes)
    void RecalcFontMetrics();

    // vertical scrolling helpers
    // --------------------------

    // all these functions are for multi line controls only

    // get the number of visible lines
    size_t GetLinesPerPage() const;

    // return the position above the cursor or INVALID_POS_VALUE
    wxTextPos GetPositionAbove();

    // return the position below the cursor or INVALID_POS_VALUE
    wxTextPos GetPositionBelow();

    // event handlers
    // --------------
    void OnChar(wxKeyEvent& event);
    void OnSize(wxSizeEvent& event);

    // return the struct containing control-type dependent data
    struct wxTextSingleLineData& SData() { return *m_data.sdata; }
    struct wxTextMultiLineData& MData() { return *m_data.mdata; }
    struct wxTextWrappedData& WData() { return *m_data.wdata; }
    const wxTextSingleLineData& SData() const { return *m_data.sdata; }
    const wxTextMultiLineData& MData() const { return *m_data.mdata; }
    const wxTextWrappedData& WData() const { return *m_data.wdata; }

    // clipboard operations (unlike the versions without Do prefix, they have a
    // return code)
    bool DoCut();
    bool DoPaste();

private:
    // all these methods are for multiline text controls only

    // update the scrollbars (only called from OnIdle)
    void UpdateScrollbars();

    // get read only access to the lines of multiline control
    inline const wxArrayString& GetLines() const;
    inline size_t GetLineCount() const;

    // replace a line (returns true if the number of rows in thel ine changed)
    bool ReplaceLine(wxTextCoord line, const wxString& text);

    // remove a line
    void RemoveLine(wxTextCoord line);

    // insert a line at this position
    void InsertLine(wxTextCoord line, const wxString& text);

    // calculate geometry of this line
    void LayoutLine(wxTextCoord line, class wxWrappedLineData& lineData) const;

    // calculate geometry of all lines until the given one
    void LayoutLines(wxTextCoord lineLast) const;

    // the initially specified control size
    wxSize m_sizeInitial;

    // the global control text
    wxString m_value;

    // current position
    wxTextPos m_curPos;
    wxTextCoord m_curCol,
                m_curRow;

    // last position (only used by GetLastPosition())
    wxTextPos m_posLast;

    // max text line length
    unsigned long m_maxLength;

    // selection
    wxTextPos m_selAnchor,
              m_selStart,
              m_selEnd;

    // flags
    bool m_isModified;
    bool m_isEditable;
    bool m_hasCaret;
    bool m_wrapLines; // can't be changed after creation

    // the rectangle (in client coordinates) to draw text inside
    wxRect m_rectText;

    // the height of one line (cached value of GetCharHeight)
    wxCoord m_heightLine;

    // and the average char width (cached value of GetCharWidth)
    wxCoord m_widthAvg;

    // we have some data which depends on the kind of control (single or multi
    // line)
    union
    {
        wxTextSingleLineData *sdata;
        wxTextMultiLineData *mdata;
        wxTextWrappedData *wdata;
        void *data;
    } m_data;

    // the object to which we delegate our undo/redo implementation
    wxTextCtrlCommandProcessor *m_cmdProcessor;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxTextCtrl);

    friend class wxWrappedLineData;
};

#endif // _WX_UNIV_TEXTCTRL_H_

