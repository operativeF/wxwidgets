/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/grid.h
// Purpose:     wxGrid and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: Santiago Palacios
// Created:     1/08/1999
// Copyright:   (c) Michael Bedward
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_GRID_H_
#define _WX_GENERIC_GRID_H_

#include "wx/defs.h"

#if wxUSE_GRID

#include "wx/generic/gridcoord.h"
#include "wx/generic/gridevt.h"

#include "wx/hashmap.h"
#include "wx/scrolwin.h"
#include "wx/settings.h"

import <iterator>;

#include <gsl/gsl>

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

inline constexpr char wxGridNameStr[] = "grid";

// Various constants used in wxGrid code.
//
// Note that all the values are in DIPs, not pixels, i.e. you must use
// FromDIP() when using them in your code.
inline constexpr int WXGRID_DEFAULT_COL_WIDTH              = 80;
inline constexpr int WXGRID_DEFAULT_COL_LABEL_HEIGHT       = 32;
inline constexpr int WXGRID_DEFAULT_ROW_LABEL_WIDTH        = 82;
inline constexpr int WXGRID_LABEL_EDGE_ZONE                = 2;
inline constexpr int WXGRID_MIN_ROW_HEIGHT                 = 15;
inline constexpr int WXGRID_MIN_COL_WIDTH                  = 15;

// scroll line size
inline constexpr int GRID_SCROLL_LINE_X = 15;
inline constexpr int GRID_SCROLL_LINE_Y = GRID_SCROLL_LINE_X;

// type names for grid table values
inline constexpr std::string_view wxGRID_VALUE_STRING     = "string";
inline constexpr std::string_view wxGRID_VALUE_BOOL       = "bool";
inline constexpr std::string_view wxGRID_VALUE_NUMBER     = "long";
inline constexpr std::string_view wxGRID_VALUE_FLOAT      = "double";
inline constexpr std::string_view wxGRID_VALUE_CHOICE     = "choice";
inline constexpr std::string_view wxGRID_VALUE_DATE       = "date";

inline constexpr std::string_view wxGRID_VALUE_TEXT       = wxGRID_VALUE_STRING;
inline constexpr std::string_view wxGRID_VALUE_LONG       = wxGRID_VALUE_NUMBER;

// magic constant which tells (to some functions) to automatically calculate
// the appropriate size
#define wxGRID_AUTOSIZE (-1)

// many wxGrid methods work either with columns or rows, this enum is used for
// the parameter indicating which one should it be
enum class wxGridDirection
{
    Column,
    Row
};

// Flags used with wxGrid::Render() to select parts of the grid to draw.
enum wxGridRenderStyle
{
    wxGRID_DRAW_ROWS_HEADER = 0x001,
    wxGRID_DRAW_COLS_HEADER = 0x002,
    wxGRID_DRAW_CELL_LINES = 0x004,
    wxGRID_DRAW_BOX_RECT = 0x008,
    wxGRID_DRAW_SELECTION = 0x010,
    wxGRID_DRAW_DEFAULT = wxGRID_DRAW_ROWS_HEADER |
                          wxGRID_DRAW_COLS_HEADER |
                          wxGRID_DRAW_CELL_LINES |
                          wxGRID_DRAW_BOX_RECT
};

// ----------------------------------------------------------------------------
// forward declarations
// ----------------------------------------------------------------------------

class wxGrid;
class wxGridCellAttr;
struct wxGridCellAttrProviderData;
class wxGridColLabelWindow;
class wxGridCornerLabelWindow;
class wxGridEvent;
class wxGridRowLabelWindow;
class wxGridWindow;
class wxGridTypeRegistry;
class wxGridSelection;

class wxHeaderCtrl;
class wxCheckBox;
class wxComboBox;
class wxTextCtrl;
#if wxUSE_SPINCTRL
class wxSpinCtrl;
#endif
#if wxUSE_DATEPICKCTRL
class wxDatePickerCtrl;
#endif

class wxGridFixedIndicesSet;

class wxGridOperations;
class wxGridRowOperations;
class wxGridColumnOperations;
class wxGridDirectionOperations;


// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

#define wxSafeIncRef(p) if ( p ) (p)->IncRef()
#define wxSafeDecRef(p) if ( p ) (p)->DecRef()

// ----------------------------------------------------------------------------
// wxGridCellWorker: common base class for wxGridCellRenderer and
// wxGridCellEditor
//
// NB: this is more an implementation convenience than a design issue, so this
//     class is not documented and is not public at all
// ----------------------------------------------------------------------------

class wxGridCellWorker : public wxClientDataContainer, public wxRefCounter
{
public:
    wxGridCellWorker() = default;

    // interpret renderer parameters: arbitrary string whose interpretation is
    // left to the derived classes
    virtual void SetParameters(const std::string& params);

protected:
    // virtual dtor for any base class - private because only DecRef() can
    // delete us
    ~wxGridCellWorker() = default;

private:
    // suppress the stupid gcc warning about the class having private dtor and
    // no friends
    friend class wxGridCellWorkerDummyFriend;
};

// ----------------------------------------------------------------------------
// wxGridCellRenderer: this class is responsible for actually drawing the cell
// in the grid. You may pass it to the wxGridCellAttr (below) to change the
// format of one given cell or to wxGrid::SetDefaultRenderer() to change the
// view of all cells. This is an ABC, you will normally use one of the
// predefined derived classes or derive your own class from it.
// ----------------------------------------------------------------------------

class wxGridCellRenderer : public wxGridCellWorker
{
public:
    // draw the given cell on the provided DC inside the given rectangle
    // using the style specified by the attribute and the default or selected
    // state corresponding to the isSelected value.
    //
    // this pure virtual function has a default implementation which will
    // prepare the DC using the given attribute: it will draw the rectangle
    // with the bg colour from attr and set the text colour and font
    virtual void Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rect,
                      int row, int col,
                      bool isSelected) = 0;

    // get the preferred size of the cell for its contents
    virtual wxSize GetBestSize(wxGrid& grid,
                               wxGridCellAttr& attr,
                               wxDC& dc,
                               int row, int col) = 0;

    // Get the preferred height for a given width. Override this method if the
    // renderer computes height as function of its width, as is the case of the
    // standard wxGridCellAutoWrapStringRenderer, for example.
    // and vice versa
    virtual int GetBestHeight(wxGrid& grid,
                              wxGridCellAttr& attr,
                              wxDC& dc,
                              int row, int col,
                              int WXUNUSED(width))
    {
        return GetBestSize(grid, attr, dc, row, col).y;
    }

    // Get the preferred width for a given height, this is the symmetric
    // version of GetBestHeight().
    virtual int GetBestWidth(wxGrid& grid,
                             wxGridCellAttr& attr,
                             wxDC& dc,
                             int row, int col,
                             int WXUNUSED(height))
    {
        return GetBestSize(grid, attr, dc, row, col).x;
    }


    // Unlike GetBestSize(), this functions is optional: it is used when
    // auto-sizing columns to determine the best width without iterating over
    // all cells in this column, if possible.
    //
    // If it isn't, return wxDefaultSize as the base class version does by
    // default.
    virtual wxSize GetMaxBestSize(wxGrid& WXUNUSED(grid),
                                  wxGridCellAttr& WXUNUSED(attr),
                                  wxDC& WXUNUSED(dc))
    {
        return wxDefaultSize;
    }

    // create a new object which is the copy of this one
    virtual wxGridCellRenderer *Clone() const = 0;

protected:
    // set the text colours before drawing
    void SetTextColoursAndFont(const wxGrid& grid,
                               const wxGridCellAttr& attr,
                               wxDC& dc,
                               bool isSelected);
};

// Smart pointer to wxGridCellRenderer, calling DecRef() on it automatically.
using wxGridCellRendererPtr = wxObjectDataPtr<wxGridCellRenderer>;


// ----------------------------------------------------------------------------
// Helper classes used by wxGridCellEditor::TryActivate() and DoActivate().
// ----------------------------------------------------------------------------

// This class represents a source of cell activation, which may be either a
// user event (mouse or keyboard) or the program itself.
//
// Note that objects of this class are supposed to be ephemeral and so store
// pointers to the events specified when creating them, which are supposed to
// have life-time greater than that of the objects of this class.
class wxGridActivationSource
{
public:
    enum class Origin
    {
        Program,
        Key,
        Mouse
    };

    // Factory functions, only used by the library itself.
    static wxGridActivationSource FromProgram()
    {
        return wxGridActivationSource(Origin::Program, nullptr);
    }

    static wxGridActivationSource From(const wxKeyEvent& event)
    {
        return wxGridActivationSource(Origin::Key, &event);
    }

    static wxGridActivationSource From(const wxMouseEvent& event)
    {
        return wxGridActivationSource(Origin::Mouse, &event);
    }

    // Accessors allowing to retrieve information about the source.

    // Can be called for any object.
    Origin GetOrigin() const { return m_origin; }

    // Can be called for objects with Key origin only.
    const wxKeyEvent& GetKeyEvent() const
    {
        wxASSERT( m_origin == Origin::Key );

        return *dynamic_cast<const wxKeyEvent*>(m_event);
    }

    // Can be called for objects with Mouse origin only.
    const wxMouseEvent& GetMouseEvent() const
    {
        wxASSERT( m_origin == Origin::Mouse );

        return *dynamic_cast<const wxMouseEvent*>(m_event);
    }

private:
    wxGridActivationSource(Origin origin, const wxEvent* event)
        : m_origin(origin),
          m_event(event)
    {
    }

    const Origin m_origin;
    const wxEvent* const m_event;
};

// This class represents the result of TryActivate(), which may be either
// absence of any action (if activating wouldn't change the value anyhow),
// attempt to change the value to the specified one or just start normal
// editing, which is the default for the editors not supporting activation.
class wxGridActivationResult
{
public:
    enum class Action
    {
        Ignore,
        Change,
        ShowEditor
    };

    // Factory functions, only used by the library itself.
    static wxGridActivationResult DoNothing()
    {
        return wxGridActivationResult(Action::Ignore);
    }

    static wxGridActivationResult DoChange(const std::string& newval)
    {
        return wxGridActivationResult(Action::Change, newval);
    }

    static wxGridActivationResult DoEdit()
    {
        return wxGridActivationResult(Action::ShowEditor);
    }

    // Accessors allowing to retrieve information about the result.

    // Can be called for any object.
    Action GetAction() const { return m_action; }

    // Can be called for objects with Change action type only.
    const std::string& GetNewValue() const
    {
        wxASSERT( m_action == Action::Change );

        return m_newval;
    }

private:
    explicit
    wxGridActivationResult(Action action, const std::string& newval = {})
        : m_action{action},
          m_newval{newval}
    {
    }

    const Action m_action;
    const std::string m_newval;
};

// ----------------------------------------------------------------------------
// wxGridCellEditor:  This class is responsible for providing and manipulating
// the in-place edit controls for the grid.  Instances of wxGridCellEditor
// (actually, instances of derived classes since it is an ABC) can be
// associated with the cell attributes for individual cells, rows, columns, or
// even for the entire grid.
// ----------------------------------------------------------------------------

class wxGridCellEditor : public wxGridCellWorker
{
public:
    wxGridCellEditor() = default;

    wxGridCellEditor(const wxGridCellEditor&) = delete;
    wxGridCellEditor& operator=(const wxGridCellEditor&) = delete;

    bool IsCreated() const { return m_control != nullptr; }

    wxWindow* GetWindow() const { return m_control; }
    void SetWindow(wxWindow* window) { m_control = window; }

    wxGridCellAttr* GetCellAttr() const { return m_attr; }
    void SetCellAttr(wxGridCellAttr* attr) { m_attr = attr; }

    // Creates the actual edit control
    virtual void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler) = 0;

    // Size and position the edit control
    virtual void SetSize(const wxRect& rect);

    // Show or hide the edit control, use the specified attributes to set
    // colours/fonts for it
    virtual void Show(bool show, wxGridCellAttr *attr = nullptr);

    // Draws the part of the cell not occupied by the control: the base class
    // version just fills it with background colour from the attribute
    virtual void PaintBackground(wxDC& dc,
                                 const wxRect& rectCell,
                                 const wxGridCellAttr& attr);


    // The methods called by wxGrid when a cell is edited: first BeginEdit() is
    // called, then EndEdit() is and if it returns true and if the change is
    // not vetoed by a user-defined event handler, finally ApplyEdit() is called

    // Fetch the value from the table and prepare the edit control
    // to begin editing.  Set the focus to the edit control.
    virtual void BeginEdit(int row, int col, wxGrid* grid) = 0;

    // Returns false if nothing changed, otherwise returns true and return the
    // new value in its string form in the newval output parameter.
    //
    // This should also store the new value in its real type internally so that
    // it could be used by ApplyEdit() but it must not modify the grid as the
    // change could still be vetoed.
    virtual bool EndEdit(int row, int col, const wxGrid *grid,
                         const std::string& oldval, std::string *newval) = 0;

    // Complete the editing of the current cell by storing the value saved by
    // the previous call to EndEdit() in the grid
    virtual void ApplyEdit(int row, int col, wxGrid* grid) = 0;


    // Reset the value in the control back to its starting value
    virtual void Reset() = 0;

    // return true to allow the given key to start editing: the base class
    // version only checks that the event has no modifiers. The derived
    // classes are supposed to do "if ( base::IsAcceptedKey() && ... )" in
    // their IsAcceptedKey() implementation, although, of course, it is not a
    // mandatory requirement.
    //
    // NB: if the key is F2 (special), editing will always start and this
    //     method will not be called at all (but StartingKey() will)
    virtual bool IsAcceptedKey(wxKeyEvent& event);

    // If the editor is enabled by pressing keys on the grid, this will be
    // called to let the editor do something about that first key if desired
    virtual void StartingKey(wxKeyEvent& event);

    // if the editor is enabled by clicking on the cell, this method will be
    // called
    virtual void StartingClick();

    // Some types of controls on some platforms may need some help
    // with the Return key.
    virtual void HandleReturn(wxKeyEvent& event);

    // Final cleanup
    virtual void Destroy();

    // create a new object which is the copy of this one
    virtual wxGridCellEditor *Clone() const = 0;

    // added GetValue so we can get the value which is in the control
    virtual std::string GetValue() const = 0;


    // These functions exist only for backward compatibility, use Get and
    // SetWindow() instead in the new code.
    wxControl* GetControl() { return wxDynamicCast(m_control, wxControl); }
    void SetControl(wxControl* control) { m_control = control; }


    // Support for "activatable" editors: those change the value of the cell
    // immediately, instead of creating an editor control and waiting for user
    // input.
    //
    // See wxGridCellBoolEditor for an example of such editor.

    // Override this function to return "Change" activation result from it to
    // show that the editor supports activation. DoActivate() will be called if
    // the cell changing event is not vetoed.
    virtual
    wxGridActivationResult
    TryActivate(int WXUNUSED(row), int WXUNUSED(col),
                wxGrid* WXUNUSED(grid),
                const wxGridActivationSource& WXUNUSED(actSource))
    {
        return wxGridActivationResult::DoEdit();
    }

    virtual
    void
    DoActivate(int WXUNUSED(row), int WXUNUSED(col), wxGrid* WXUNUSED(grid))
    {
        wxFAIL_MSG( "Must be overridden if TryActivate() is overridden" );
    }

protected:
    // the dtor is private because only DecRef() can delete us
    ~wxGridCellEditor();

    // Helper for the derived classes positioning the control according to the
    // attribute alignment if the desired control size is smaller than the cell
    // size, or centering it vertically if its size is bigger: this looks like
    // the best compromise when the editor control doesn't fit into the cell.
    void DoPositionEditor(const wxSize& size,
                          const wxRect& rectCell,
                          int hAlign = wxALIGN_LEFT,
                          int vAlign = wxALIGN_CENTRE_VERTICAL);


    // the actual window we show on screen (this variable should actually be
    // named m_window, but m_control is kept for backward compatibility)
    wxWindow*  m_control{nullptr};

    // a temporary pointer to the attribute being edited
    wxGridCellAttr* m_attr{nullptr};

    // if we change the colours/font of the control from the default ones, we
    // must restore the default later and we save them here between calls to
    // Show(true) and Show(false)
    wxColour m_colFgOld,
             m_colBgOld;
    wxFont m_fontOld;

    // suppress the stupid gcc warning about the class having private dtor and
    // no friends
    friend class wxGridCellEditorDummyFriend;
};

// Smart pointer to wxGridCellEditor, calling DecRef() on it automatically.
using wxGridCellEditorPtr = wxObjectDataPtr<wxGridCellEditor>;

// Base class for editors that can be only activated and not edited normally.
class wxGridCellActivatableEditor : public wxGridCellEditor
{
public:
    // In this class these methods must be overridden.
    wxGridActivationResult
    TryActivate(int row, int col, wxGrid* grid,
                const wxGridActivationSource& actSource) override = 0;
    void DoActivate(int row, int col, wxGrid* grid) override = 0;

    // All the other methods that normally must be implemented in an editor are
    // defined as just stubs below, as they should be never called.

    void Create(wxWindow*, wxWindowID, wxEvtHandler*) override
        { wxFAIL; }
    void BeginEdit(int, int, wxGrid*) override
        { wxFAIL; }
    bool EndEdit(int, int, const wxGrid*,
                         const std::string&, std::string*) override
        { wxFAIL; return false; }
    void ApplyEdit(int, int, wxGrid*) override
        { wxFAIL; }
    void Reset() override
        { wxFAIL; }
    std::string GetValue() const override
        { wxFAIL; return {}; }
};

// ----------------------------------------------------------------------------
// wxGridHeaderRenderer and company: like wxGridCellRenderer but for headers
// ----------------------------------------------------------------------------

// Base class for header cells renderers.
class wxGridHeaderLabelsRenderer
{
public:
    virtual ~wxGridHeaderLabelsRenderer() = default;

    // Draw the border around cell window.
    virtual void DrawBorder(const wxGrid& grid,
                            wxDC& dc,
                            wxRect& rect) const = 0;

    // Draw header cell label
    virtual void DrawLabel(const wxGrid& grid,
                           wxDC& dc,
                           std::string_view value,
                           const wxRect& rect,
                           int horizAlign,
                           int vertAlign,
                           int textOrientation) const;
};

// Currently the row/column/corner renders don't need any methods other than
// those already in wxGridHeaderLabelsRenderer but still define separate classes
// for them for future extensions and also for better type safety (i.e. to
// avoid inadvertently using a column header renderer for the row headers)
class wxGridRowHeaderRenderer
    : public wxGridHeaderLabelsRenderer
{
};

class wxGridColumnHeaderRenderer
    : public wxGridHeaderLabelsRenderer
{
};

class wxGridCornerHeaderRenderer
    : public wxGridHeaderLabelsRenderer
{
};

// Also define the default renderers which are used by wxGridCellAttrProvider
// by default
class wxGridRowHeaderRendererDefault
    : public wxGridRowHeaderRenderer
{
public:
    void DrawBorder(const wxGrid& grid,
                            wxDC& dc,
                            wxRect& rect) const override;
};

// Column header cells renderers
class wxGridColumnHeaderRendererDefault
    : public wxGridColumnHeaderRenderer
{
public:
    void DrawBorder(const wxGrid& grid,
                            wxDC& dc,
                            wxRect& rect) const override;
};

// Header corner renderer
class wxGridCornerHeaderRendererDefault
    : public wxGridCornerHeaderRenderer
{
public:
    void DrawBorder(const wxGrid& grid,
                            wxDC& dc,
                            wxRect& rect) const override;
};

// ----------------------------------------------------------------------------
// Helper class used to define What should happen if the cell contents doesn't
// fit into its allotted space.
// ----------------------------------------------------------------------------

class wxGridFitMode
{
public:
    // Default ctor creates an object not specifying any particular behaviour.
    wxGridFitMode()  = default;

    // Static methods allowing to create objects actually specifying behaviour.
    static wxGridFitMode Clip() { return wxGridFitMode(Mode_Clip); }
    static wxGridFitMode Overflow() { return wxGridFitMode(Mode_Overflow); }
    static wxGridFitMode Ellipsize(wxEllipsizeMode ellipsize = wxEllipsizeMode::End)
    {
        // This cast works because the enum elements are the same, see below.
        return wxGridFitMode(static_cast<Mode>(ellipsize));
    }

    // Accessors.
    bool IsSpecified() const { return m_mode != Mode_Unset; }
    bool IsClip() const { return m_mode == Mode_Clip; }
    bool IsOverflow() const { return m_mode == Mode_Overflow; }

    wxEllipsizeMode GetEllipsizeMode() const
    {
        switch ( m_mode )
        {
            case Mode_Unset:
            case Mode_EllipsizeStart:
            case Mode_EllipsizeMiddle:
            case Mode_EllipsizeEnd:
                return static_cast<wxEllipsizeMode>(m_mode);

            case Mode_Overflow:
            case Mode_Clip:
                break;
        }

        return wxEllipsizeMode::None;
    }

    // This one is used in the implementation only.
    static wxGridFitMode FromOverflowFlag(bool allow)
        { return allow ? Overflow() : Clip(); }

private:
    enum Mode
    {
        // This is a hack to save space: the first 4 elements of this enum are
        // the same as those of wxEllipsizeMode.
        Mode_Unset = wxEllipsizeMode::None,
        Mode_EllipsizeStart = wxEllipsizeMode::Start,
        Mode_EllipsizeMiddle = wxEllipsizeMode::Middle,
        Mode_EllipsizeEnd = wxEllipsizeMode::End,
        Mode_Overflow,
        Mode_Clip
    };

    explicit wxGridFitMode(Mode mode) : m_mode(mode) {}

    Mode m_mode{Mode_Unset};
};

// ----------------------------------------------------------------------------
// wxGridCellAttr: this class can be used to alter the cells appearance in
// the grid by changing their colour/font/... from default. An object of this
// class may be returned by wxGridTable::GetAttr().
// ----------------------------------------------------------------------------

class wxGridCellAttr : public wxClientDataContainer, public wxRefCounter
{
public:
    enum wxAttrKind
    {
        Any,
        Default,
        Cell,
        Row,
        Col,
        Merged
    };

    // default ctor
    explicit wxGridCellAttr(wxGridCellAttr *attrDefault = nullptr)
        : m_defGridAttr{attrDefault}
    {
    }

    // ctor setting the most common attributes
    wxGridCellAttr(const wxColour& colText,
                   const wxColour& colBack,
                   const wxFont& font,
                   int hAlign,
                   int vAlign)
        : m_colText(colText), m_colBack(colBack), m_font(font)
    {
        SetAlignment(hAlign, vAlign);
    }

    // use Clone() instead
    wxGridCellAttr(const wxGridCellAttr&) = delete;
    wxGridCellAttr& operator=(const wxGridCellAttr&) = delete;

    // creates a new copy of this object
    wxGridCellAttr *Clone() const;
    void MergeWith(wxGridCellAttr *mergefrom);

    // setters
    void SetTextColour(const wxColour& colText) { m_colText = colText; }
    void SetBackgroundColour(const wxColour& colBack) { m_colBack = colBack; }
    void SetFont(const wxFont& font) { m_font = font; }
    void SetAlignment(int hAlign, int vAlign)
    {
        m_hAlign = hAlign;
        m_vAlign = vAlign;
    }
    void SetSize(int num_rows, int num_cols);
    void SetFitMode(wxGridFitMode fitMode) { m_fitMode = fitMode; }
    void SetOverflow(bool allow = true)
        { SetFitMode(wxGridFitMode::FromOverflowFlag(allow)); }
    void SetReadOnly(bool isReadOnly = true)
        { m_isReadOnly = isReadOnly ? ReadOnly : ReadWrite; }

    // takes ownership of the pointer
    void SetRenderer(wxGridCellRenderer *renderer)
        { wxSafeDecRef(m_renderer); m_renderer = renderer; }
    void SetEditor(wxGridCellEditor* editor)
        { wxSafeDecRef(m_editor); m_editor = editor; }

    void SetKind(wxAttrKind kind) { m_attrkind = kind; }

    bool HasTextColour() const { return m_colText.IsOk(); }
    bool HasBackgroundColour() const { return m_colBack.IsOk(); }
    bool HasFont() const { return m_font.IsOk(); }
    bool HasAlignment() const
    {
        return m_hAlign != wxALIGN_INVALID || m_vAlign != wxALIGN_INVALID;
    }
    bool HasRenderer() const { return m_renderer != nullptr; }
    bool HasEditor() const { return m_editor != nullptr; }
    bool HasReadWriteMode() const { return m_isReadOnly != Unset; }
    bool HasOverflowMode() const { return m_fitMode.IsSpecified(); }
    bool HasSize() const { return m_sizeRows != 1 || m_sizeCols != 1; }

    const wxColour& GetTextColour() const;
    const wxColour& GetBackgroundColour() const;
    const wxFont& GetFont() const;
    void GetAlignment(int *hAlign, int *vAlign) const;

    // unlike GetAlignment() which always overwrites its output arguments with
    // the alignment values to use, falling back on default alignment if this
    // attribute doesn't have any, this function will preserve the values of
    // parameters on entry if the corresponding alignment is not set in this
    // attribute meaning that they can be initialized to default alignment (and
    // also that they must be initialized, unlike with GetAlignment())
    void GetNonDefaultAlignment(int *hAlign, int *vAlign) const;

    void GetSize(int *num_rows, int *num_cols) const;
    wxGridFitMode GetFitMode() const;
    bool GetOverflow() const { return GetFitMode().IsOverflow(); }
    // whether the cell will draw the overflowed text to neighbour cells
    // currently only left aligned cells can overflow
    bool CanOverflow() const;

    wxGridCellRenderer *GetRenderer(const wxGrid* grid, int row, int col) const;
    wxGridCellRendererPtr GetRendererPtr(const wxGrid* grid, int row, int col) const
    {
        return wxGridCellRendererPtr(GetRenderer(grid, row, col));
    }

    wxGridCellEditor *GetEditor(const wxGrid* grid, int row, int col) const;
    wxGridCellEditorPtr GetEditorPtr(const wxGrid* grid, int row, int col) const
    {
        return wxGridCellEditorPtr(GetEditor(grid, row, col));
    }

    bool IsReadOnly() const { return m_isReadOnly == wxGridCellAttr::ReadOnly; }

    wxAttrKind GetKind() { return m_attrkind; }

    void SetDefAttr(wxGridCellAttr* defAttr) { m_defGridAttr = defAttr; }

protected:
    // the dtor is private because only DecRef() can delete us
    ~wxGridCellAttr()
    {
        wxSafeDecRef(m_renderer);
        wxSafeDecRef(m_editor);
    }

private:
    enum wxAttrReadMode
    {
        Unset = -1,
        ReadWrite,
        ReadOnly
    };

    wxColour m_colText;
    wxColour m_colBack;

    wxFont        m_font;
    wxGridFitMode m_fitMode;

    wxGridCellRenderer* m_renderer{nullptr};
    wxGridCellEditor*   m_editor{nullptr};
    wxGridCellAttr*     m_defGridAttr;

    int      m_hAlign{wxALIGN_INVALID};
    int      m_vAlign{wxALIGN_INVALID};
    int      m_sizeRows{1};
    int      m_sizeCols{1};

    wxAttrReadMode m_isReadOnly{wxAttrReadMode::Unset};

    wxAttrKind m_attrkind{wxGridCellAttr::Cell};

    // suppress the stupid gcc warning about the class having private dtor and
    // no friends
    friend class wxGridCellAttrDummyFriend;
};

// Smart pointer to wxGridCellAttr, calling DecRef() on it automatically.
using wxGridCellAttrPtr = wxObjectDataPtr<wxGridCellAttr>;

// ----------------------------------------------------------------------------
// wxGridCellAttrProvider: class used by wxGridTableBase to retrieve/store the
// cell attributes.
// ----------------------------------------------------------------------------

// implementation note: we separate it from wxGridTableBase because we wish to
// avoid deriving a new table class if possible, and sometimes it will be
// enough to just derive another wxGridCellAttrProvider instead
//
// the default implementation is reasonably efficient for the generic case,
// but you might still wish to implement your own for some specific situations
// if you have performance problems with the stock one
class wxGridCellAttrProvider : public wxClientDataContainer
{
public:
    wxGridCellAttrProvider() = default;
    ~wxGridCellAttrProvider();

    wxGridCellAttrProvider(const wxGridCellAttrProvider&) = delete;
    wxGridCellAttrProvider& operator=(const wxGridCellAttrProvider&) = delete;

    // DecRef() must be called on the returned pointer
    virtual wxGridCellAttr *GetAttr(int row, int col,
                                    wxGridCellAttr::wxAttrKind  kind ) const;

    // Helper returning smart pointer calling DecRef() automatically.
    wxGridCellAttrPtr GetAttrPtr(int row, int col,
                                 wxGridCellAttr::wxAttrKind  kind ) const
    {
        return wxGridCellAttrPtr(GetAttr(row, col, kind));
    }

    // all these functions take ownership of the pointer, don't call DecRef()
    // on it
    virtual void SetAttr(wxGridCellAttr *attr, int row, int col);
    virtual void SetRowAttr(wxGridCellAttr *attr, int row);
    virtual void SetColAttr(wxGridCellAttr *attr, int col);

    // these functions must be called whenever some rows/cols are deleted
    // because the internal data must be updated then
    void UpdateAttrRows( size_t pos, int numRows );
    void UpdateAttrCols( size_t pos, int numCols );


    // get renderers for the given row/column header label and the corner
    // window: unlike cell renderers, these objects are not reference counted
    // and are never NULL so they are returned by reference
    virtual const wxGridColumnHeaderRenderer& GetColumnHeaderRenderer(int col);
    virtual const wxGridRowHeaderRenderer& GetRowHeaderRenderer(int row);
    virtual const wxGridCornerHeaderRenderer& GetCornerRenderer();

private:
    void InitData();

    wxGridCellAttrProviderData *m_data{nullptr};
};

// ----------------------------------------------------------------------------
// wxGridBlockCoords: location of a block of cells in the grid
// ----------------------------------------------------------------------------

struct wxGridBlockDiffResult;

class wxGridBlockCoords
{
public:
    wxGridBlockCoords() = default;

    wxGridBlockCoords(int topRow, int leftCol, int bottomRow, int rightCol) :
        m_topRow(topRow),
        m_leftCol(leftCol),
        m_bottomRow(bottomRow),
        m_rightCol(rightCol)
    {
    }

    int GetTopRow() const { return m_topRow; }
    void SetTopRow(int row) { m_topRow = row; }
    int GetLeftCol() const { return m_leftCol; }
    void SetLeftCol(int col) { m_leftCol = col; }
    int GetBottomRow() const { return m_bottomRow; }
    void SetBottomRow(int row) { m_bottomRow = row; }
    int GetRightCol() const { return m_rightCol; }
    void SetRightCol(int col) { m_rightCol = col; }

    wxGridCellCoords GetTopLeft() const
    {
        return wxGridCellCoords(m_topRow, m_leftCol);
    }

    wxGridCellCoords GetBottomRight() const
    {
        return wxGridCellCoords(m_bottomRow, m_rightCol);
    }

    wxGridBlockCoords Canonicalize() const
    {
        wxGridBlockCoords result = *this;

        if ( result.m_topRow > result.m_bottomRow )
            std::swap(result.m_topRow, result.m_bottomRow);

        if ( result.m_leftCol > result.m_rightCol )
            std::swap(result.m_leftCol, result.m_rightCol);

        return result;
    }

    bool Intersects(const wxGridBlockCoords& other) const
    {
        return m_topRow <= other.m_bottomRow && m_bottomRow >= other.m_topRow &&
               m_leftCol <= other.m_rightCol && m_rightCol >= other.m_leftCol;
    }

    // Return whether this block contains the given cell.
    bool Contains(const wxGridCellCoords& cell) const
    {
        return m_topRow <= cell.GetRow() && cell.GetRow() <= m_bottomRow &&
               m_leftCol <= cell.GetCol() && cell.GetCol() <= m_rightCol;
    }

    // Return whether this blocks fully contains another one.
    bool Contains(const wxGridBlockCoords& other) const
    {
        return m_topRow <= other.m_topRow && other.m_bottomRow <= m_bottomRow &&
               m_leftCol <= other.m_leftCol && other.m_rightCol <= m_rightCol;
    }

    // Calculates the result blocks by subtracting the other block from this
    // block. splitOrientation can be wxVERTICAL or wxHORIZONTAL.
    wxGridBlockDiffResult
    Difference(const wxGridBlockCoords& other, int splitOrientation) const;

    // Calculates the symmetric difference of the blocks.
    wxGridBlockDiffResult
    SymDifference(const wxGridBlockCoords& other) const;

    bool operator==(const wxGridBlockCoords& other) const
    {
        return m_topRow == other.m_topRow && m_leftCol == other.m_leftCol &&
               m_bottomRow == other.m_bottomRow && m_rightCol == other.m_rightCol;
    }

    bool operator!=(const wxGridBlockCoords& other) const
    {
        return !(*this == other);
    }

    bool operator!() const
    {
        return m_topRow == -1 && m_leftCol == -1 &&
               m_bottomRow == -1 && m_rightCol == -1;
    }

private:
    int m_topRow{-1};
    int m_leftCol{-1};
    int m_bottomRow{-1};
    int m_rightCol{-1};
};

using wxGridBlockCoordsVector = std::vector<wxGridBlockCoords>;

// ----------------------------------------------------------------------------
// wxGridBlockDiffResult: The helper struct uses as a result type for difference
// functions of wxGridBlockCoords class.
// Parts can be uninitialized (equals to wxGridNoBlockCoords), that means
// that the corresponding part doesn't exists in the result.
// ----------------------------------------------------------------------------

struct wxGridBlockDiffResult
{
    wxGridBlockCoords m_parts[4];
};

// ----------------------------------------------------------------------------
// wxGridBlocks: a range of grid blocks that can be iterated over
// ----------------------------------------------------------------------------

class wxGridBlocks
{
    using iterator_impl = wxGridBlockCoordsVector::const_iterator;

public:
    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = wxGridBlockCoords;
        using reference = const value_type&;
        using pointer = const value_type*;

        iterator()  = default;

        reference operator*() const { return *m_it; }
        pointer operator->() const { return &*m_it; }

        iterator& operator++()
            { ++m_it; return *this; }
        iterator operator++(int)
            { iterator tmp = *this; ++m_it; return tmp; }

        bool operator==(const iterator& it) const
            { return m_it == it.m_it; }
        bool operator!=(const iterator& it) const
            { return m_it != it.m_it; }

    private:
        explicit iterator(iterator_impl it) : m_it(it) { }

        iterator_impl m_it;

        friend class wxGridBlocks;
    };

    iterator begin() const
    {
        return m_begin;
    }

    iterator end() const
    {
        return m_end;
    }

private:
    wxGridBlocks() :
        m_begin(),
        m_end()
    {
    }

    wxGridBlocks(iterator_impl ibegin, iterator_impl iend) :
        m_begin(ibegin),
        m_end(iend)
    {
    }

    const iterator m_begin;
    const iterator m_end;

    friend class wxGrid;
};

// For comparisons...
//
extern wxGridBlockCoords wxGridNoBlockCoords;
extern wxRect            wxGridNoCellRect;

// An array of cell coords...
//
WX_DECLARE_OBJARRAY_WITH_DECL(wxGridCellCoords, wxGridCellCoordsArray,
                              class WXDLLIMPEXP_CORE);

// ----------------------------------------------------------------------------
// Grid table classes
// ----------------------------------------------------------------------------

// the abstract base class
class wxGridTableBase : public wxObject,
                                        public wxClientDataContainer
{
public:
    wxGridTableBase() = default;
    ~wxGridTableBase();

    wxGridTableBase(const wxGridTableBase&) = delete;
    wxGridTableBase& operator=(const wxGridTableBase&) = delete;

    // You must override these functions in a derived table class
    //

    // return the number of rows and columns in this table
    virtual int GetNumberRows() = 0;
    virtual int GetNumberCols() = 0;

    // the methods above are unfortunately non-const even though they should
    // have been const -- but changing it now is not possible any longer as it
    // would break the existing code overriding them, so instead we provide
    // these const synonyms which can be used from const-correct code
    int GetRowsCount() const
        { return const_cast<wxGridTableBase *>(this)->GetNumberRows(); }
    int GetColsCount() const
        { return const_cast<wxGridTableBase *>(this)->GetNumberCols(); }


    virtual bool IsEmptyCell( int row, int col )
    {
        return GetValue(row, col).empty();
    }

    bool IsEmpty(const wxGridCellCoords& coord)
    {
        return IsEmptyCell(coord.GetRow(), coord.GetCol());
    }

    virtual std::string GetValue( int row, int col ) = 0;
    virtual void SetValue( int row, int col, const std::string& value ) = 0;

    // Data type determination and value access
    virtual std::string GetTypeName( int row, int col );
    virtual bool CanGetValueAs( int row, int col, std::string_view typeName );
    virtual bool CanSetValueAs( int row, int col, std::string_view typeName );

    virtual long GetValueAsLong( int row, int col );
    virtual double GetValueAsDouble( int row, int col );
    virtual bool GetValueAsBool( int row, int col );

    virtual void SetValueAsLong( int row, int col, long value );
    virtual void SetValueAsDouble( int row, int col, double value );
    virtual void SetValueAsBool( int row, int col, bool value );

    // For user defined types
    virtual void* GetValueAsCustom( int row, int col, std::string_view typeName );
    virtual void  SetValueAsCustom( int row, int col, std::string_view typeName, void* value );

    // Overriding these is optional
    //
    virtual void SetView( wxGrid *grid ) { m_view = grid; }
    virtual wxGrid * GetView() const { return m_view; }

    virtual void Clear() {}
    virtual bool InsertRows( size_t pos = 0, size_t numRows = 1 );
    virtual bool AppendRows( size_t numRows = 1 );
    virtual bool DeleteRows( size_t pos = 0, size_t numRows = 1 );
    virtual bool InsertCols( size_t pos = 0, size_t numCols = 1 );
    virtual bool AppendCols( size_t numCols = 1 );
    virtual bool DeleteCols( size_t pos = 0, size_t numCols = 1 );

    virtual std::string GetRowLabelValue( int row );
    virtual std::string GetColLabelValue( int col );
    virtual std::string GetCornerLabelValue() const;
    virtual void SetRowLabelValue( int WXUNUSED(row), const std::string& ) = 0;
    virtual void SetColLabelValue( int WXUNUSED(col), const std::string& ) = 0;
    virtual void SetCornerLabelValue( const std::string& ) = 0;

    // Attribute handling
    //

    // give us the attr provider to use - we take ownership of the pointer
    void SetAttrProvider(wxGridCellAttrProvider *attrProvider);

    // get the currently used attr provider (may be NULL)
    wxGridCellAttrProvider *GetAttrProvider() const { return m_attrProvider; }

    // Does this table allow attributes?  Default implementation creates
    // a wxGridCellAttrProvider if necessary.
    virtual bool CanHaveAttributes();

    // by default forwarded to wxGridCellAttrProvider if any. May be
    // overridden to handle attributes directly in the table.
    virtual wxGridCellAttr *GetAttr( int row, int col,
                                     wxGridCellAttr::wxAttrKind  kind );

    wxGridCellAttrPtr GetAttrPtr(int row, int col,
                                 wxGridCellAttr::wxAttrKind  kind)
    {
        return wxGridCellAttrPtr(GetAttr(row, col, kind));
    }

    // This is an optimization for a common case when the entire column uses
    // roughly the same attribute, which can thus be reused for measuring all
    // the cells in this column. Override this to return true (possibly for
    // some columns only) to speed up AutoSizeColumns() for the grids using
    // this table.
    virtual bool CanMeasureColUsingSameAttr(int WXUNUSED(col)) const
    {
        return false;
    }

    // these functions take ownership of the pointer
    virtual void SetAttr(wxGridCellAttr* attr, int row, int col);
    virtual void SetRowAttr(wxGridCellAttr *attr, int row);
    virtual void SetColAttr(wxGridCellAttr *attr, int col);

private:
    wxGrid * m_view{nullptr};
    wxGridCellAttrProvider *m_attrProvider{nullptr};

    wxDECLARE_ABSTRACT_CLASS(wxGridTableBase);
};


// ----------------------------------------------------------------------------
// wxGridTableMessage
// ----------------------------------------------------------------------------

// IDs for messages sent from grid table to view
//
enum wxGridTableRequest
{
    // The first two requests never did anything, simply don't use them.
    wxGRIDTABLE_NOTIFY_ROWS_INSERTED = 2002,
    wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
    wxGRIDTABLE_NOTIFY_ROWS_DELETED,
    wxGRIDTABLE_NOTIFY_COLS_INSERTED,
    wxGRIDTABLE_NOTIFY_COLS_APPENDED,
    wxGRIDTABLE_NOTIFY_COLS_DELETED
};

class wxGridTableMessage
{
public:
    wxGridTableMessage() = default;
    wxGridTableMessage( wxGridTableBase *table, int id,
                        int comInt1 = -1,
                        int comInt2 = -1 );

    wxGridTableMessage(const wxGridTableMessage&) = delete;
    wxGridTableMessage& operator=(const wxGridTableMessage&) = delete;

    void SetTableObject( wxGridTableBase *table ) { m_table = table; }
    wxGridTableBase * GetTableObject() const { return m_table; }
    void SetId( int id ) { m_id = id; }
    int GetId() const { return m_id; }
    void SetCommandInt( int comInt1 ) { m_comInt1 = comInt1; }
    int GetCommandInt() const { return m_comInt1; }
    void SetCommandInt2( int comInt2 ) { m_comInt2 = comInt2; }
    int GetCommandInt2() const { return m_comInt2; }

private:
    wxGridTableBase *m_table{nullptr};
    int m_id{-1};
    int m_comInt1{-1};
    int m_comInt2{-1};
};



// ------ wxGridStringArray
// A 2-dimensional array of strings for data values
//

WX_DECLARE_OBJARRAY_WITH_DECL(std::vector<std::string>, wxGridStringArray, class);



// ------ wxGridStringTable
//
// Simplest type of data table for a grid for small tables of strings
// that are stored in memory
//

class wxGridStringTable : public wxGridTableBase
{
public:
    wxGridStringTable() = default;
    wxGridStringTable( int numRows, int numCols );
    
	wxGridStringTable(const wxGridStringTable&) = delete;
	wxGridStringTable& operator=(const wxGridStringTable&) = delete;

    // these are pure virtual in wxGridTableBase
    //
    int GetNumberRows() override { return gsl::narrow_cast<int>(m_data.size()); }
    int GetNumberCols() override { return m_numCols; }
    std::string GetValue( int row, int col ) override;
    void SetValue( int row, int col, const std::string& s ) override;

    // overridden functions from wxGridTableBase
    //
    void Clear() override;
    bool InsertRows( size_t pos = 0, size_t numRows = 1 ) override;
    bool AppendRows( size_t numRows = 1 ) override;
    bool DeleteRows( size_t pos = 0, size_t numRows = 1 ) override;
    bool InsertCols( size_t pos = 0, size_t numCols = 1 ) override;
    bool AppendCols( size_t numCols = 1 ) override;
    bool DeleteCols( size_t pos = 0, size_t numCols = 1 ) override;

    void SetRowLabelValue( int row, const std::string& ) override;
    void SetColLabelValue( int col, const std::string& ) override;
    void SetCornerLabelValue( const std::string& ) override;
    std::string GetRowLabelValue( int row ) override;
    std::string GetColLabelValue( int col ) override;
    std::string GetCornerLabelValue() const override;

private:
    std::vector<std::vector<std::string>> m_data;

    std::string m_cornerLabel;

    // These only get used if you set your own labels, otherwise the
    // GetRow/ColLabelValue functions return wxGridTableBase defaults
    //
    std::vector<std::string>     m_rowLabels;
    std::vector<std::string>     m_colLabels;

    // notice that while we don't need to store the number of our rows as it's
    // always equal to the size of m_data array, we do need to store the number
    // of our columns as we can't retrieve it from m_data when the number of
    // rows is 0 (see #10818)
    int m_numCols{};

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};



// ============================================================================
//  Grid view classes
// ============================================================================

// ----------------------------------------------------------------------------
// wxGridSizesInfo stores information about sizes of the rows or columns.
//
// It assumes that most of the columns or rows have default size and so stores
// the default size separately and uses a hash to map column or row numbers to
// their non default size for those which don't have the default size.
// ----------------------------------------------------------------------------

// hash map to store positions as the keys and sizes as the values
using wxUnsignedToIntHashMap = std::unordered_map< unsigned, int, wxIntegerHash, wxIntegerEqual >;


struct wxGridSizesInfo
{
    // default ctor, initialize m_sizeDefault and m_customSizes later
    wxGridSizesInfo() = default;

    // ctor used by wxGrid::Get{Col,Row}Sizes()
    wxGridSizesInfo(int defSize, const std::vector<int>& allSizes);

    // default copy ctor, assignment operator and dtor are ok

    // Get the size of the element with the given index
    int GetSize(unsigned pos) const;


    // default size
    int m_sizeDefault{};

    // position -> size map containing all elements with non-default size
    wxUnsignedToIntHashMap m_customSizes;
};

// ----------------------------------------------------------------------------
// wxGrid
// ----------------------------------------------------------------------------

class wxGrid : public wxScrolledCanvas
{
public:
    // possible selection modes
    enum wxGridSelectionModes
    {
        wxGridSelectCells         = 0,  // allow selecting anything
        wxGridSelectRows          = 1,  // allow selecting only entire rows
        wxGridSelectColumns       = 2,  // allow selecting only entire columns
        wxGridSelectRowsOrColumns = wxGridSelectRows | wxGridSelectColumns,
        wxGridSelectNone          = 4   // disallow selecting anything
    };

    // Different behaviour of the TAB key when the end (or the beginning, for
    // Shift-TAB) of the current row is reached:
    enum TabBehaviour
    {
        Tab_Stop,   // Do nothing, this is default.
        Tab_Wrap,   // Move to the next (or previous) row.
        Tab_Leave   // Move to the next (or previous) control.
    };

    // creation and destruction
    // ------------------------

    // ctor and Create() create the grid window, as with the other controls
    wxGrid()
    {
        m_labelFont = GetFont();
        m_labelFont.SetWeight( wxFONTWEIGHT_BOLD );

        if ( wxSystemSettings::GetAppearance().IsDark() )
            m_gridFrozenBorderColour = *wxWHITE;
        else
            m_gridFrozenBorderColour = *wxBLACK;

        // we can't call SetScrollRate() as the window isn't created yet but OTOH
        // we don't need to call it neither as the scroll position is (0, 0) right
        // now anyhow, so just set the parameters directly
        m_xScrollPixelsPerLine = GRID_SCROLL_LINE_X;
        m_yScrollPixelsPerLine = GRID_SCROLL_LINE_Y;
    }

    wxGrid(wxWindow *parent,
            wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            unsigned int style = wxWANTS_CHARS,
            const std::string& name = wxGridNameStr)
    {
        m_labelFont = GetFont();
        m_labelFont.SetWeight( wxFONTWEIGHT_BOLD );

        if ( wxSystemSettings::GetAppearance().IsDark() )
            m_gridFrozenBorderColour = *wxWHITE;
        else
            m_gridFrozenBorderColour = *wxBLACK;

        // we can't call SetScrollRate() as the window isn't created yet but OTOH
        // we don't need to call it neither as the scroll position is (0, 0) right
        // now anyhow, so just set the parameters directly
        m_xScrollPixelsPerLine = GRID_SCROLL_LINE_X;
        m_yScrollPixelsPerLine = GRID_SCROLL_LINE_Y;

        Create(parent, id, pos, size, style, name);
    }

    wxGrid(const wxGrid&) = delete;
    wxGrid& operator=(const wxGrid&) = delete;

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxWANTS_CHARS,
                const std::string& name = wxGridNameStr);

    ~wxGrid();

    // however to initialize grid data either CreateGrid() (to use a simple
    // default table class) or {Set,Assign}Table() (to use a custom table) must
    // be also called

    // this is basically equivalent to
    //
    //   AssignTable(new wxGridStringTable(numRows, numCols), selmode)
    //
    bool CreateGrid( int numRows, int numCols,
                     wxGridSelectionModes selmode = wxGridSelectCells );

    bool SetTable( wxGridTableBase *table,
                   bool takeOwnership = false,
                   wxGridSelectionModes selmode = wxGridSelectCells );

    void AssignTable( wxGridTableBase *table,
                      wxGridSelectionModes selmode = wxGridSelectCells );

    bool ProcessTableMessage(wxGridTableMessage&);

    wxGridTableBase *GetTable() const { return m_table; }


    void SetSelectionMode(wxGridSelectionModes selmode);
    wxGridSelectionModes GetSelectionMode() const;

    // ------ grid dimensions
    //
    int      GetNumberRows() const { return  m_numRows; }
    int      GetNumberCols() const { return  m_numCols; }

    int      GetNumberFrozenRows() const { return m_numFrozenRows; }
    int      GetNumberFrozenCols() const { return m_numFrozenCols; }

    // ------ display update functions
    //
    std::vector<int> CalcRowLabelsExposed( const wxRegion& reg,
                                     wxGridWindow *gridWindow = nullptr) const;
    std::vector<int> CalcColLabelsExposed( const wxRegion& reg,
                                     wxGridWindow *gridWindow = nullptr) const;
    wxGridCellCoordsArray CalcCellsExposed( const wxRegion& reg,
                                            wxGridWindow *gridWindow = nullptr) const;

    void PrepareDCFor(wxDC &dc, wxGridWindow *gridWindow);

    void ClearGrid();
    bool InsertRows(int pos = 0, int numRows = 1, bool updateLabels = true)
    {
        return DoModifyLines(&wxGridTableBase::InsertRows,
                             pos, numRows, updateLabels);
    }
    bool InsertCols(int pos = 0, int numCols = 1, bool updateLabels = true)
    {
        return DoModifyLines(&wxGridTableBase::InsertCols,
                             pos, numCols, updateLabels);
    }

    bool AppendRows(int numRows = 1, bool updateLabels = true)
    {
        return DoAppendLines(&wxGridTableBase::AppendRows, numRows, updateLabels);
    }
    bool AppendCols(int numCols = 1, bool updateLabels = true)
    {
        return DoAppendLines(&wxGridTableBase::AppendCols, numCols, updateLabels);
    }

    bool DeleteRows(int pos = 0, int numRows = 1, bool updateLabels = true)
    {
        return DoModifyLines(&wxGridTableBase::DeleteRows,
                             pos, numRows, updateLabels);
    }
    bool DeleteCols(int pos = 0, int numCols = 1, bool updateLabels = true)
    {
        return DoModifyLines(&wxGridTableBase::DeleteCols,
                             pos, numCols, updateLabels);
    }

    bool FreezeTo(int row, int col);
    bool FreezeTo(const wxGridCellCoords& coords)
    {
        return FreezeTo(coords.GetRow(), coords.GetCol());
    }

    bool IsFrozen() const;

    void DrawGridCellArea( wxDC& dc , const wxGridCellCoordsArray& cells );
    void DrawGridSpace( wxDC& dc, wxGridWindow *gridWindow );
    void DrawCellBorder( wxDC& dc, const wxGridCellCoords& );
    void DrawAllGridLines();
    void DrawAllGridWindowLines( wxDC& dc, const wxRegion & reg , wxGridWindow *gridWindow);
    void DrawCell( wxDC& dc, const wxGridCellCoords& );
    void DrawHighlight(wxDC& dc, const wxGridCellCoordsArray& cells);
    void DrawFrozenBorder( wxDC& dc, wxGridWindow *gridWindow );
    void DrawLabelFrozenBorder( wxDC& dc, wxWindow *window, bool isRow );

    void ScrollWindow( int dx, int dy, const wxRect *rect ) override;

    void UpdateGridWindows() const;

    // this function is called when the current cell highlight must be redrawn
    // and may be overridden by the user
    virtual void DrawCellHighlight( wxDC& dc, const wxGridCellAttr *attr );

    virtual void DrawRowLabels( wxDC& dc, const std::vector<int>& rows );
    virtual void DrawRowLabel( wxDC& dc, int row );

    virtual void DrawColLabels( wxDC& dc, const std::vector<int>& cols );
    virtual void DrawColLabel( wxDC& dc, int col );

    virtual void DrawCornerLabel(wxDC& dc);

    // ------ Cell text drawing functions
    //
    void DrawTextRectangle( wxDC& dc, std::string_view value, const wxRect&,
                            int horizontalAlignment = wxALIGN_LEFT,
                            int verticalAlignment = wxALIGN_TOP,
                            int textOrientation = wxHORIZONTAL ) const;

    void DrawTextRectangle( wxDC& dc, const std::vector<std::string>& lines, const wxRect&,
                            int horizontalAlignment = wxALIGN_LEFT,
                            int verticalAlignment = wxALIGN_TOP,
                            int textOrientation = wxHORIZONTAL ) const;

    void DrawTextRectangle(wxDC& dc,
                           std::string_view text,
                           const wxRect& rect,
                           const wxGridCellAttr& attr,
                           int defaultHAlign = wxALIGN_INVALID,
                           int defaultVAlign = wxALIGN_INVALID) const;

    // ------ grid render function for printing
    //
    void Render( wxDC& dc,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 const wxGridCellCoords& topLeft = wxGridCellCoords(-1, -1),
                 const wxGridCellCoords& bottomRight = wxGridCellCoords(-1, -1),
                 unsigned int style = wxGRID_DRAW_DEFAULT );

    // Split a string containing newline characters into an array of
    // strings and return the number of lines
    //
    void StringToLines( const std::string& value, std::vector<std::string>& lines ) const;

    wxSize GetTextBoxSize( const wxDC& dc,
                         const std::vector<std::string>& lines) const;

    // If bottomRight is invalid, i.e. == wxGridNoCellCoords, it defaults to
    // topLeft. If topLeft itself is invalid, the function simply returns.
    void RefreshBlock(const wxGridCellCoords& topLeft,
                      const wxGridCellCoords& bottomRight);

    void RefreshBlock(int topRow, int leftCol,
                      int bottomRow, int rightCol);

    // ------
    // Code that does a lot of grid modification can be enclosed
    // between BeginBatch() and EndBatch() calls to avoid screen
    // flicker
    //
    void     BeginBatch() { m_batchCount++; }
    void     EndBatch();

    int      GetBatchCount() const { return m_batchCount; }

    void Refresh(bool eraseb = true, const wxRect* rect = nullptr) override;

    // Use this, rather than wxWindow::Refresh(), to force an
    // immediate repainting of the grid. Has no effect if you are
    // already inside a BeginBatch / EndBatch block.
    //
    // This function is necessary because wxGrid has a minimal OnPaint()
    // handler to reduce screen flicker.
    //
    void     ForceRefresh();


    // ------ edit control functions
    //
    bool IsEditable() const { return m_editable; }
    void EnableEditing( bool edit );

    void EnableCellEditControl( bool enable = true );
    void DisableCellEditControl() { EnableCellEditControl(false); }
    bool CanEnableCellControl() const;
    bool IsCellEditControlEnabled() const { return m_cellEditCtrlEnabled; }
    bool IsCellEditControlShown() const;

    bool IsCurrentCellReadOnly() const;

    void ShowCellEditControl(); // Use EnableCellEditControl() instead.
    void HideCellEditControl();
    void SaveEditControlValue();


    // ------ grid location functions
    //  Note that all of these functions work with the logical coordinates of
    //  grid cells and labels so you will need to convert from device
    //  coordinates for mouse events etc.
    //
    wxGridCellCoords XYToCell(int x, int y, wxGridWindow *gridWindow = nullptr) const;
    void XYToCell(int x, int y,
                  wxGridCellCoords& coords,
                  wxGridWindow *gridWindow = nullptr) const
        { coords = XYToCell(x, y, gridWindow); }

    wxGridCellCoords XYToCell(const wxPoint& pos,
                              wxGridWindow *gridWindow = nullptr) const
        { return XYToCell(pos.x, pos.y, gridWindow); }

    // these functions return the index of the row/columns corresponding to the
    // given logical position in pixels
    //
    // if clipToMinMax is false (default, wxNOT_FOUND is returned if the
    // position is outside any row/column, otherwise the first/last element is
    // returned in this case
    int  YToRow( int y, bool clipToMinMax = false, wxGridWindow *gridWindow = nullptr ) const;
    int  XToCol( int x, bool clipToMinMax = false, wxGridWindow *gridWindow = nullptr ) const;

    int  YToEdgeOfRow( int y ) const;
    int  XToEdgeOfCol( int x ) const;

    wxRect CellToRect( int row, int col ) const;
    wxRect CellToRect( const wxGridCellCoords& coords ) const
        { return CellToRect( coords.GetRow(), coords.GetCol() ); }

    wxGridWindow* CellToGridWindow( int row, int col ) const;
    wxGridWindow* CellToGridWindow( const wxGridCellCoords& coords ) const
        { return CellToGridWindow( coords.GetRow(), coords.GetCol() ); }

    const wxGridCellCoords& GetGridCursorCoords() const
        { return m_currentCellCoords; }

    int  GetGridCursorRow() const { return m_currentCellCoords.GetRow(); }
    int  GetGridCursorCol() const { return m_currentCellCoords.GetCol(); }

    void    GetGridWindowOffset(const wxGridWindow *gridWindow, int &x, int &y) const;
    wxPoint GetGridWindowOffset(const wxGridWindow *gridWindow) const;

    wxGridWindow* DevicePosToGridWindow(wxPoint pos) const;
    wxGridWindow* DevicePosToGridWindow(int x, int y) const;

    void    CalcGridWindowUnscrolledPosition(int x, int y, int *xx, int *yy,
                                             const wxGridWindow *gridWindow) const;
    wxPoint CalcGridWindowUnscrolledPosition(const wxPoint& pt,
                                             const wxGridWindow *gridWindow) const;

    void    CalcGridWindowScrolledPosition(int x, int y, int *xx, int *yy,
                                           const wxGridWindow *gridWindow) const;
    wxPoint CalcGridWindowScrolledPosition(const wxPoint& pt,
                                           const wxGridWindow *gridWindow) const;

    // check to see if a cell is either wholly visible (the default arg) or
    // at least partially visible in the grid window
    //
    bool IsVisible( int row, int col, bool wholeCellVisible = true ) const;
    bool IsVisible( const wxGridCellCoords& coords, bool wholeCellVisible = true ) const
        { return IsVisible( coords.GetRow(), coords.GetCol(), wholeCellVisible ); }
    void MakeCellVisible( int row, int col );
    void MakeCellVisible( const wxGridCellCoords& coords )
        { MakeCellVisible( coords.GetRow(), coords.GetCol() ); }

    // Returns the topmost row of the current visible area.
    int GetFirstFullyVisibleRow() const;
    // Returns the leftmost column of the current visible area.
    int GetFirstFullyVisibleColumn() const;

    // ------ grid cursor movement functions
    //
    void SetGridCursor(int row, int col) { SetCurrentCell(row, col); }
    void SetGridCursor(const wxGridCellCoords& c) { SetCurrentCell(c); }

    void GoToCell(int row, int col)
    {
        if ( SetCurrentCell(row, col) )
            MakeCellVisible(row, col);
    }

    void GoToCell(const wxGridCellCoords& coords)
    {
        if ( SetCurrentCell(coords) )
            MakeCellVisible(coords);
    }

    bool MoveCursorUp( bool expandSelection );
    bool MoveCursorDown( bool expandSelection );
    bool MoveCursorLeft( bool expandSelection );
    bool MoveCursorRight( bool expandSelection );
    bool MovePageDown();
    bool MovePageUp();
    bool MoveCursorUpBlock( bool expandSelection );
    bool MoveCursorDownBlock( bool expandSelection );
    bool MoveCursorLeftBlock( bool expandSelection );
    bool MoveCursorRightBlock( bool expandSelection );

    void SetTabBehaviour(TabBehaviour behaviour) { m_tabBehaviour = behaviour; }


    // ------ label and gridline formatting
    //
    int      GetDefaultRowLabelSize() const { return FromDIP(WXGRID_DEFAULT_ROW_LABEL_WIDTH); }
    int      GetRowLabelSize() const { return m_rowLabelWidth; }
    int      GetDefaultColLabelSize() const { return FromDIP(WXGRID_DEFAULT_COL_LABEL_HEIGHT); }
    int      GetColLabelSize() const { return m_colLabelHeight; }
    wxColour GetLabelBackgroundColour() const { return m_labelBackgroundColour; }
    wxColour GetLabelTextColour() const { return m_labelTextColour; }
    wxFont   GetLabelFont() const { return m_labelFont; }
    void     GetRowLabelAlignment( int *horiz, int *vert ) const;
    void     GetColLabelAlignment( int *horiz, int *vert ) const;
    void     GetCornerLabelAlignment( int *horiz, int *vert ) const;
    int      GetColLabelTextOrientation() const;
    int      GetCornerLabelTextOrientation() const;
    std::string GetRowLabelValue( int row ) const;
    std::string GetColLabelValue( int col ) const;
    std::string GetCornerLabelValue() const;

    wxColour GetCellHighlightColour() const { return m_cellHighlightColour; }
    int      GetCellHighlightPenWidth() const { return m_cellHighlightPenWidth; }
    int      GetCellHighlightROPenWidth() const { return m_cellHighlightROPenWidth; }
    wxColor  GetGridFrozenBorderColour() const { return m_gridFrozenBorderColour; }
    int      GetGridFrozenBorderPenWidth() const { return m_gridFrozenBorderPenWidth; }

    // this one will use wxHeaderCtrl for the column labels
    bool UseNativeColHeader(bool native = true);

    // this one will still draw them manually but using the native renderer
    // instead of using the same appearance as for the row labels
    void SetUseNativeColLabels( bool native = true );

    void     SetRowLabelSize( int width );
    void     SetColLabelSize( int height );
    void     HideRowLabels() { SetRowLabelSize( 0 ); }
    void     HideColLabels() { SetColLabelSize( 0 ); }
    void     SetLabelBackgroundColour( const wxColour& );
    void     SetLabelTextColour( const wxColour& );
    void     SetLabelFont( const wxFont& );
    void     SetRowLabelAlignment( int horiz, int vert );
    void     SetColLabelAlignment( int horiz, int vert );
    void     SetCornerLabelAlignment( int horiz, int vert );
    void     SetColLabelTextOrientation( int textOrientation );
    void     SetCornerLabelTextOrientation( int textOrientation );
    void     SetRowLabelValue( int row, const std::string& );
    void     SetColLabelValue( int col, const std::string& );
    void     SetCornerLabelValue( const std::string& );
    void     SetCellHighlightColour( const wxColour& );
    void     SetCellHighlightPenWidth( int width );
    void     SetCellHighlightROPenWidth( int width );
    void     SetGridFrozenBorderColour( const wxColour& );
    void     SetGridFrozenBorderPenWidth( int width );

    // interactive grid mouse operations control
    // -----------------------------------------

    // functions globally enabling row/column interactive resizing (enabled by
    // default)
    void     EnableDragRowSize( bool enable = true );
    void     DisableDragRowSize() { EnableDragRowSize( false ); }

    void     EnableDragColSize( bool enable = true );
    void     DisableDragColSize() { EnableDragColSize( false ); }

        // if interactive resizing is enabled, some rows/columns can still have
        // fixed size
    void DisableRowResize(int row) { DoDisableLineResize(row, m_setFixedRows); }
    void DisableColResize(int col) { DoDisableLineResize(col, m_setFixedCols); }

        // These function return true if resizing rows/columns by dragging
        // their edges inside the grid is enabled. Note that this doesn't cover
        // dragging their separators in the label windows (which can be enabled
        // for the columns even if dragging inside the grid is not), nor checks
        // whether a particular row/column is resizeable or not, so you should
        // always check CanDrag{Row,Col}Size() below too.
    bool CanDragGridRowEdges() const
    {
        return m_canDragGridSize && m_canDragRowSize;
    }

    bool CanDragGridColEdges() const
    {
        // When using the native header window we can only resize the columns by
        // dragging the dividers in the header itself, but not by dragging them
        // in the grid because we can't make the native control enter into the
        // column resizing mode programmatically.
        return m_canDragGridSize && m_canDragColSize && !m_useNativeHeader;
    }

        // These functions return whether the given row/column can be
        // effectively resized: for this interactive resizing must be enabled
        // and this index must not have been passed to DisableRow/ColResize()
    bool CanDragRowSize(int row) const
        { return m_canDragRowSize && DoCanResizeLine(row, m_setFixedRows); }
    bool CanDragColSize(int col) const
        { return m_canDragColSize && DoCanResizeLine(col, m_setFixedCols); }

    // interactive column reordering (disabled by default)
    bool     EnableDragColMove( bool enable = true );
    void     DisableDragColMove() { EnableDragColMove( false ); }
    bool     CanDragColMove() const { return m_canDragColMove; }

    // interactive column hiding (enabled by default, works only for native header)
    bool     EnableHidingColumns( bool enable = true );
    void     DisableHidingColumns() { EnableHidingColumns(false); }
    bool     CanHideColumns() const { return m_canHideColumns; }

    // interactive resizing of grid cells (enabled by default)
    void     EnableDragGridSize(bool enable = true);
    void     DisableDragGridSize() { EnableDragGridSize(false); }
    bool     CanDragGridSize() const { return m_canDragGridSize; }

    // interactive dragging of cells (disabled by default)
    void     EnableDragCell( bool enable = true );
    void     DisableDragCell() { EnableDragCell( false ); }
    bool     CanDragCell() const { return m_canDragCell; }


    // grid lines
    // ----------

    // enable or disable drawing of the lines
    void EnableGridLines(bool enable = true);
    bool GridLinesEnabled() const { return m_gridLinesEnabled; }

    // by default grid lines stop at last column/row, but this may be changed
    void ClipHorzGridLines(bool clip)
        { DoClipGridLines(m_gridLinesClipHorz, clip); }
    void ClipVertGridLines(bool clip)
        { DoClipGridLines(m_gridLinesClipVert, clip); }
    bool AreHorzGridLinesClipped() const { return m_gridLinesClipHorz; }
    bool AreVertGridLinesClipped() const { return m_gridLinesClipVert; }

    // this can be used to change the global grid lines colour
    void SetGridLineColour(const wxColour& col);
    wxColour GetGridLineColour() const { return m_gridLineColour; }

    // these methods may be overridden to customize individual grid lines
    // appearance
    virtual wxPen GetDefaultGridLinePen();
    virtual wxPen GetRowGridLinePen(int row);
    virtual wxPen GetColGridLinePen(int col);


    // attributes
    // ----------

    // this sets the specified attribute for this cell or in this row/col
    void     SetAttr(int row, int col, wxGridCellAttr *attr);
    void     SetRowAttr(int row, wxGridCellAttr *attr);
    void     SetColAttr(int col, wxGridCellAttr *attr);

    // the grid can cache attributes for the recently used cells (currently it
    // only caches one attribute for the most recently used one) and might
    // notice that its value in the attribute provider has changed -- if this
    // happens, call this function to force it
    void RefreshAttr(int row, int col);

    // returns the attribute we may modify in place: a new one if this cell
    // doesn't have any yet or the existing one if it does
    //
    // DecRef() must be called on the returned pointer, as usual
    wxGridCellAttr *GetOrCreateCellAttr(int row, int col) const;

    wxGridCellAttrPtr GetOrCreateCellAttrPtr(int row, int col) const
    {
        return wxGridCellAttrPtr(GetOrCreateCellAttr(row, col));
    }


    // shortcuts for setting the column parameters

    // set the format for the data in the column: default is string
    void     SetColFormatBool(int col);
    void     SetColFormatNumber(int col);
    void     SetColFormatFloat(int col, int width = -1, int precision = -1);
    void     SetColFormatDate(int col, const std::string& format = {});
    void     SetColFormatCustom(int col, std::string_view typeName);

    // ------ row and col formatting
    //
    int      GetDefaultRowSize() const;
    int      GetRowSize( int row ) const;
    bool     IsRowShown(int row) const { return GetRowSize(row) != 0; }
    int      GetDefaultColSize() const;
    int      GetColSize( int col ) const;
    bool     IsColShown(int col) const { return GetColSize(col) != 0; }
    wxColour GetDefaultCellBackgroundColour() const;
    wxColour GetCellBackgroundColour( int row, int col ) const;
    wxColour GetDefaultCellTextColour() const;
    wxColour GetCellTextColour( int row, int col ) const;
    wxFont   GetDefaultCellFont() const;
    wxFont   GetCellFont( int row, int col ) const;
    void     GetDefaultCellAlignment( int *horiz, int *vert ) const;
    void     GetCellAlignment( int row, int col, int *horiz, int *vert ) const;

    wxGridFitMode GetDefaultCellFitMode() const;
    wxGridFitMode GetCellFitMode(int row, int col) const;

    bool     GetDefaultCellOverflow() const
        { return GetDefaultCellFitMode().IsOverflow(); }
    bool     GetCellOverflow( int row, int col ) const
        { return GetCellFitMode(row, col).IsOverflow(); }

    // this function returns 1 in num_rows and num_cols for normal cells,
    // positive numbers for a cell spanning multiple columns/rows (as set with
    // SetCellSize()) and _negative_ numbers corresponding to the offset of the
    // top left cell of the span from this one for the other cells covered by
    // this cell
    //
    // the return value is CellSpan_None, CellSpan_Main or CellSpan_Inside for
    // each of these cases respectively
    enum CellSpan
    {
        CellSpan_Inside = -1,
        CellSpan_None = 0,
        CellSpan_Main
    };

    CellSpan GetCellSize( int row, int col, int *num_rows, int *num_cols ) const;

    wxSize GetCellSize(const wxGridCellCoords& coords) const
    {
        wxSize s;
        GetCellSize(coords.GetRow(), coords.GetCol(), &s.x, &s.y);
        return s;
    }

    // ------ row and col sizes
    void     SetDefaultRowSize( int height, bool resizeExistingRows = false );
    void     SetRowSize( int row, int height );
    void     HideRow(int row) { DoSetRowSize(row, 0); }
    void     ShowRow(int row) { DoSetRowSize(row, -1); }

    void     SetDefaultColSize( int width, bool resizeExistingCols = false );
    void     SetColSize( int col, int width );
    void     HideCol(int col) { DoSetColSize(col, 0); }
    void     ShowCol(int col) { DoSetColSize(col, -1); }

    // the row and column sizes can be also set all at once using
    // wxGridSizesInfo which holds all of them at once

    wxGridSizesInfo GetColSizes() const
        { return wxGridSizesInfo(GetDefaultColSize(), m_colWidths); }
    wxGridSizesInfo GetRowSizes() const
        { return wxGridSizesInfo(GetDefaultRowSize(), m_rowHeights); }

    void SetColSizes(const wxGridSizesInfo& sizeInfo);
    void SetRowSizes(const wxGridSizesInfo& sizeInfo);


    // ------- columns (only, for now) reordering

    // columns index <-> positions mapping: by default, the position of the
    // column is the same as its index, but the columns can also be reordered
    // (either by calling SetColPos() explicitly or by the user dragging the
    // columns around) in which case their indices don't correspond to their
    // positions on display any longer
    //
    // internally we always work with indices except for the functions which
    // have "Pos" in their names (and which work with columns, not pixels) and
    // only the display and hit testing code really cares about display
    // positions at all

    // set the positions of all columns at once (this method uses the same
    // conventions as wxHeaderCtrl::SetColumnsOrder() for the order array)
    void SetColumnsOrder(const std::vector<int>& order);

    // return the column index corresponding to the given (valid) position
    int GetColAt(int pos) const
    {
        return m_colAt.empty() ? pos : m_colAt[pos];
    }

    // reorder the columns so that the column with the given index is now shown
    // as the position pos
    void SetColPos(int idx, int pos);

    // return the position at which the column with the given index is
    // displayed: notice that this is a slow operation as we don't maintain the
    // reverse mapping currently
    gsl::index GetColPos(int idx) const
    {
        wxASSERT_MSG( idx >= 0 && idx < m_numCols, "invalid column index" );

        if ( m_colAt.empty() )
            return idx;

        auto pos = std::find(m_colAt.begin(), m_colAt.end(), idx);
        wxASSERT_MSG( pos != std::end(m_colAt), "invalid column index" );

        return std::distance(std::begin(m_colAt), pos);
    }

    // reset the columns positions to the default order
    void ResetColPos();


    // automatically size the column or row to fit to its contents, if
    // setAsMin is true, this optimal width will also be set as minimal width
    // for this column
    void     AutoSizeColumn( int col, bool setAsMin = true )
        { AutoSizeColOrRow(col, setAsMin, wxGridDirection::Column); }
    void     AutoSizeRow( int row, bool setAsMin = true )
        { AutoSizeColOrRow(row, setAsMin, wxGridDirection::Row); }

    // auto size all columns (very ineffective for big grids!)
    void     AutoSizeColumns( bool setAsMin = true );
    void     AutoSizeRows( bool setAsMin = true );

    // auto size the grid, that is make the columns/rows of the "right" size
    // and also set the grid size to just fit its contents
    void     AutoSize();

    // Note for both AutoSizeRowLabelSize and AutoSizeColLabelSize:
    // If col equals to wxGRID_AUTOSIZE value then function autosizes labels column
    // instead of data column. Note that this operation may be slow for large
    // tables.
    // autosize row height depending on label text
    void     AutoSizeRowLabelSize( int row );

    // autosize column width depending on label text
    void     AutoSizeColLabelSize( int col );

    // column won't be resized to be lesser width - this must be called during
    // the grid creation because it won't resize the column if it's already
    // narrower than the minimal width
    void     SetColMinimalWidth( int col, int width );
    void     SetRowMinimalHeight( int row, int width );

    /*  These members can be used to query and modify the minimal
     *  acceptable size of grid rows and columns. Call this function in
     *  your code which creates the grid if you want to display cells
     *  with a size smaller than the default acceptable minimum size.
     *  Like the members SetColMinimalWidth and SetRowMinimalWidth,
     *  the existing rows or columns will not be checked/resized.
     */
    void     SetColMinimalAcceptableWidth( int width );
    void     SetRowMinimalAcceptableHeight( int width );
    int      GetColMinimalAcceptableWidth() const;
    int      GetRowMinimalAcceptableHeight() const;

    void     SetDefaultCellBackgroundColour( const wxColour& );
    void     SetCellBackgroundColour( int row, int col, const wxColour& );
    void     SetDefaultCellTextColour( const wxColour& );

    void     SetCellTextColour( int row, int col, const wxColour& );
    void     SetDefaultCellFont( const wxFont& );
    void     SetCellFont( int row, int col, const wxFont& );
    void     SetDefaultCellAlignment( int horiz, int vert );
    void     SetCellAlignment( int row, int col, int horiz, int vert );

    void     SetDefaultCellFitMode(wxGridFitMode fitMode);
    void     SetCellFitMode(int row, int col, wxGridFitMode fitMode);
    void     SetDefaultCellOverflow( bool allow )
        { SetDefaultCellFitMode(wxGridFitMode::FromOverflowFlag(allow)); }
    void     SetCellOverflow( int row, int col, bool allow )
        { SetCellFitMode(row, col, wxGridFitMode::FromOverflowFlag(allow)); }

    void     SetCellSize( int row, int col, int num_rows, int num_cols );

    // takes ownership of the pointer
    void SetDefaultRenderer(wxGridCellRenderer *renderer);
    void SetCellRenderer(int row, int col, wxGridCellRenderer *renderer);
    wxGridCellRenderer *GetDefaultRenderer() const;
    wxGridCellRenderer* GetCellRenderer(int row, int col) const;

    // takes ownership of the pointer
    void SetDefaultEditor(wxGridCellEditor *editor);
    void SetCellEditor(int row, int col, wxGridCellEditor *editor);
    wxGridCellEditor *GetDefaultEditor() const;
    wxGridCellEditor* GetCellEditor(int row, int col) const;



    // ------ cell value accessors
    //
    std::string GetCellValue( int row, int col ) const
    {
        if ( m_table )
        {
            return m_table->GetValue( row, col );
        }
        else
        {
            return "";
        }
    }

    std::string GetCellValue( const wxGridCellCoords& coords ) const
        { return GetCellValue( coords.GetRow(), coords.GetCol() ); }

    void SetCellValue( int row, int col, const std::string& s );
    void SetCellValue( const wxGridCellCoords& coords, const std::string& s )
        { SetCellValue( coords.GetRow(), coords.GetCol(), s ); }

    // returns true if the cell can't be edited
    bool IsReadOnly(int row, int col) const;

    // make the cell editable/readonly
    void SetReadOnly(int row, int col, bool isReadOnly = true);

    // ------ select blocks of cells
    //
    void SelectRow( int row, bool addToSelected = false );
    void SelectCol( int col, bool addToSelected = false );

    void SelectBlock( int topRow, int leftCol, int bottomRow, int rightCol,
                      bool addToSelected = false );

    void SelectBlock( const wxGridCellCoords& topLeft,
                      const wxGridCellCoords& bottomRight,
                      bool addToSelected = false )
        { SelectBlock( topLeft.GetRow(), topLeft.GetCol(),
                       bottomRight.GetRow(), bottomRight.GetCol(),
                       addToSelected ); }

    void SelectAll();

    bool IsSelection() const;

    // ------ deselect blocks or cells
    //
    void DeselectRow( int row );
    void DeselectCol( int col );
    void DeselectCell( int row, int col );

    void ClearSelection();

    bool IsInSelection( int row, int col ) const;

    bool IsInSelection( const wxGridCellCoords& coords ) const
        { return IsInSelection( coords.GetRow(), coords.GetCol() ); }

    // Efficient methods returning the selected blocks (there are few of those).
    wxGridBlocks GetSelectedBlocks() const;
    wxGridBlockCoordsVector GetSelectedRowBlocks() const;
    wxGridBlockCoordsVector GetSelectedColBlocks() const;

    // Less efficient (but maybe more convenient methods) returning all
    // selected cells, rows or columns -- there can be many and many of those.
    wxGridCellCoordsArray GetSelectedCells() const;
    wxGridCellCoordsArray GetSelectionBlockTopLeft() const;
    wxGridCellCoordsArray GetSelectionBlockBottomRight() const;
    std::vector<int> GetSelectedRows() const;
    std::vector<int> GetSelectedCols() const;

    // This function returns the rectangle that encloses the block of cells
    // limited by TopLeft and BottomRight cell in device coords and clipped
    //  to the client size of the grid window.
    //
    wxRect BlockToDeviceRect( const wxGridCellCoords & topLeft,
                              const wxGridCellCoords & bottomRight,
                              const wxGridWindow *gridWindow = nullptr) const;

    // Access or update the selection fore/back colours
    wxColour GetSelectionBackground() const
        { return m_selectionBackground; }
    wxColour GetSelectionForeground() const
        { return m_selectionForeground; }

    void SetSelectionBackground(const wxColour& c) { m_selectionBackground = c; }
    void SetSelectionForeground(const wxColour& c) { m_selectionForeground = c; }


    // Methods for a registry for mapping data types to Renderers/Editors
    void RegisterDataType(std::string_view typeName,
                          wxGridCellRenderer* renderer,
                          wxGridCellEditor* editor);
    // DJC MAPTEK
    virtual wxGridCellEditor* GetDefaultEditorForCell(int row, int col) const;
    wxGridCellEditor* GetDefaultEditorForCell(const wxGridCellCoords& c) const
        { return GetDefaultEditorForCell(c.GetRow(), c.GetCol()); }
    virtual wxGridCellRenderer* GetDefaultRendererForCell(int row, int col) const;
    virtual wxGridCellEditor* GetDefaultEditorForType(std::string_view typeName) const;
    virtual wxGridCellRenderer* GetDefaultRendererForType(std::string_view typeName) const;

    // grid may occupy more space than needed for its rows/columns, this
    // function allows to set how big this extra space is
    void SetMargins(int extraWidth, int extraHeight)
    {
        m_extraWidth = extraWidth;
        m_extraHeight = extraHeight;

        CalcDimensions();
    }

    // Accessors for component windows
    wxWindow* GetGridWindow() const            { return (wxWindow*)m_gridWin; }
    wxWindow* GetFrozenCornerGridWindow()const { return (wxWindow*)m_frozenCornerGridWin; }
    wxWindow* GetFrozenRowGridWindow() const   { return (wxWindow*)m_frozenRowGridWin; }
    wxWindow* GetFrozenColGridWindow() const   { return (wxWindow*)m_frozenColGridWin; }
    wxWindow* GetGridRowLabelWindow() const    { return (wxWindow*)m_rowLabelWin; }
    wxWindow* GetGridColLabelWindow() const    { return m_colLabelWin; }
    wxWindow* GetGridCornerLabelWindow() const { return (wxWindow*)m_cornerLabelWin; }

    // Return true if native header is used by the grid.
    bool IsUsingNativeHeader() const { return m_useNativeHeader; }

    // This one can only be called if we are using the native header window
    wxHeaderCtrl *GetGridColHeader() const
    {
        wxASSERT_MSG( m_useNativeHeader, "no column header window" );

        // static_cast<> doesn't work without the full class declaration in
        // view and we prefer to avoid adding more compile-time dependencies
        // even at the cost of using reinterpret_cast<>
        return reinterpret_cast<wxHeaderCtrl *>(m_colLabelWin);
    }

    // Allow adjustment of scroll increment. The default is (15, 15).
    void SetScrollLineX(int x) { m_xScrollPixelsPerLine = x; }
    void SetScrollLineY(int y) { m_yScrollPixelsPerLine = y; }
    int GetScrollLineX() const { return m_xScrollPixelsPerLine; }
    int GetScrollLineY() const { return m_yScrollPixelsPerLine; }

    // ------- drag and drop
#if wxUSE_DRAG_AND_DROP
    void SetDropTarget(std::unique_ptr<wxDropTarget> dropTarget) override;
#endif // wxUSE_DRAG_AND_DROP


    // ------- sorting support

    // wxGrid doesn't support sorting on its own but it can indicate the sort
    // order in the column header (currently only if native header control is
    // used though)

    // return the column currently displaying the sort indicator or wxNOT_FOUND
    // if none
    int GetSortingColumn() const { return m_sortCol; }

    // return true if this column is currently used for sorting
    bool IsSortingBy(int col) const { return GetSortingColumn() == col; }

    // return the current sorting order (on GetSortingColumn()): true for
    // ascending sort and false for descending; it doesn't make sense to call
    // it if GetSortingColumn() returns wxNOT_FOUND
    bool IsSortOrderAscending() const { return m_sortIsAscending; }

    // set the sorting column (or unsets any existing one if wxNOT_FOUND) and
    // the order in which to sort
    void SetSortingColumn(int col, bool ascending = true);

    // unset any existing sorting column
    void UnsetSortingColumn() { SetSortingColumn(wxNOT_FOUND); }

    // override some base class functions
    void Fit() override;
    void SetFocus() override;

    // implementation only
    void CancelMouseCapture();

protected:
    wxSize DoGetBestSize() const override;
    void DoEnable(bool enable) override;

    wxColour    m_selectionBackground{wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)};
    wxColour    m_selectionForeground{wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT)};
    wxColour    m_labelBackgroundColour;
    wxColour    m_labelTextColour;
    wxColour    m_gridLineColour{wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)};
    wxColour    m_cellHighlightColour{wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT)};
    wxColour    m_gridFrozenBorderColour;

    std::vector<int> m_rowHeights;
    std::vector<int> m_rowBottoms;
    std::vector<int> m_colWidths;
    std::vector<int> m_colRights;
    std::vector<int> m_colAt;     //Column positions

    // if a column has a minimal width, it will be the value for it in this
    // hash table
    wxLongToLongHashMap m_colMinWidths;
    wxLongToLongHashMap m_rowMinHeights;

    wxFont     m_labelFont;

    wxCursor m_rowResizeCursor{wxCURSOR_SIZENS};
    wxCursor m_colResizeCursor{wxCURSOR_SIZEWE};

    wxGridCellCoords m_currentCellCoords{wxGridNoCellCoords};

    // the corners of the block being currently selected or wxGridNoCellCoords
    wxGridCellCoords m_selectedBlockTopLeft;
    wxGridCellCoords m_selectedBlockBottomRight;

    // the position (in physical coordinates) where the user started dragging
    // the mouse or wxDefaultPosition if mouse isn't being dragged
    //
    // notice that this can be != wxDefaultPosition while m_isDragging is still
    // false because we wait until the mouse is moved some distance away before
    // setting m_isDragging to true
    wxPoint                   m_startDragPos{wxDefaultPosition};

    wxGridWindow             *m_gridWin{nullptr};
    wxGridWindow             *m_frozenColGridWin{nullptr};
    wxGridWindow             *m_frozenRowGridWin{nullptr};
    wxGridWindow             *m_frozenCornerGridWin{nullptr};
    wxGridCornerLabelWindow  *m_cornerLabelWin{nullptr};
    wxGridRowLabelWindow     *m_rowLabelWin{nullptr};
    wxGridRowLabelWindow     *m_rowFrozenLabelWin{nullptr};
    wxGridTableBase          *m_table{nullptr};

    wxGridSelection  *m_selection{nullptr};

    wxWindow *m_winCapture{nullptr};     // the window which captured the mouse

    // the default cell attr object for cells that don't have their own
    wxGridCellAttr*        m_defaultCellAttr{nullptr};
    wxGridTypeRegistry*    m_typeRegistry{nullptr};


    // the real type of the column window depends on m_useNativeHeader value:
    // if it is true, its dynamic type is wxHeaderCtrl, otherwise it is
    // wxGridColLabelWindow, use accessors below when the real type matters
    wxWindow                *m_colLabelWin{nullptr};
    wxWindow                *m_colFrozenLabelWin{nullptr};

    wxGridColLabelWindow *GetColLabelWindow() const
    {
        wxASSERT_MSG( !m_useNativeHeader, "no column label window" );
        // FIXME: Reinterpret cast.
        return reinterpret_cast<wxGridColLabelWindow *>(m_colLabelWin);
    }

    std::size_t m_numRows{};
    std::size_t m_numCols{};

    // Number of frozen rows/columns in the beginning of the grid, 0 if none.
    int        m_numFrozenRows{};
    int        m_numFrozenCols{};

    // NB: *never* access m_row/col arrays directly because they are created
    //     on demand, *always* use accessor functions instead!

    // init the m_rowHeights/Bottoms arrays with default values
    void InitRowHeights();

    int        m_defaultRowHeight{};
    int        m_minAcceptableRowHeight{};

    // init the m_colWidths/Rights arrays
    void InitColWidths();

    int        m_defaultColWidth{};
    int        m_minAcceptableColWidth{};
    
    int        m_rowLabelWidth{};
    int        m_colLabelHeight{};

    int        m_sortCol{wxNOT_FOUND};

    // the size of the margin left to the right and bottom of the cell area
    int        m_extraWidth{};
    int        m_extraHeight{};

    int        m_rowLabelHorizAlign{wxALIGN_CENTRE};
    int        m_rowLabelVertAlign{wxALIGN_CENTRE};
    int        m_colLabelHorizAlign{wxALIGN_CENTRE};
    int        m_colLabelVertAlign{wxALIGN_CENTRE};
    int        m_colLabelTextOrientation{wxHORIZONTAL};
    int        m_cornerLabelHorizAlign{wxALIGN_CENTRE};
    int        m_cornerLabelVertAlign{wxALIGN_CENTRE};
    int        m_cornerLabelTextOrientation{wxHORIZONTAL};
    int        m_cellHighlightPenWidth{2};
    int        m_cellHighlightROPenWidth{1};
    int        m_gridFrozenBorderPenWidth{2};

    int        m_batchCount{};

    // Index of the column being drag-moved or -1 if there is no move operation
    // in progress.
    int     m_dragMoveCol{-1};

    // Last horizontal mouse position while drag-moving a column.
    int     m_dragLastPos{-1};

    // Row or column (depending on m_cursorMode value) currently being resized
    // or -1 if there is no resize operation in progress.
    int     m_dragRowOrCol{-1};

    enum class CursorMode
    {
        SelectCell,
        ResizeRow,
        ResizeCol,
        SelectRow,
        SelectCol,
        MoveCol
    };

    // this variable is used not for finding the correct current cursor but
    // mainly for finding out what is going to happen if the mouse starts being
    // dragged right now
    //
    // by default it is CursorMode::SelectCell meaning that nothing else is
    // going on, and it is set to one of RESIZE/SELECT/MOVE values while the
    // corresponding operation will be started if the user starts dragging the
    // mouse from the current position
    CursorMode   m_cursorMode{CursorMode::SelectCell};

    TabBehaviour m_tabBehaviour{Tab_Stop};        // determines how the TAB key behaves

    bool       m_sortIsAscending{true};
    bool       m_useNativeHeader{false};
    bool       m_nativeColumnLabels{false};
    bool       m_ownTable{false};
    bool       m_created{false};
    bool       m_gridLinesEnabled{true};
    bool       m_gridLinesClipHorz{true};
    bool       m_gridLinesClipVert{true};

    bool       m_editable{true};              // applies to whole grid
    bool       m_cellEditCtrlEnabled{false};   // is in-place edit currently shown?

    bool       m_canDragRowSize{true};
    bool       m_canDragColSize{true};
    bool       m_canDragColMove{false};
    bool       m_canHideColumns{true};
    bool       m_canDragGridSize{true};
    bool       m_canDragCell{false};
    bool       m_waitForSlowClick{false};

    // true if a drag operation is in progress; when this is true,
    // m_startDragPos is valid, i.e. not wxDefaultPosition
    bool       m_isDragging{false};

    // get the col/row coords
    int GetColWidth(int col) const;
    int GetColLeft(int col) const;
    int GetColRight(int col) const;

    // this function must be public for compatibility...
public:
    int GetRowHeight(int row) const;
protected:

    int GetRowTop(int row) const;
    int GetRowBottom(int row) const;

    // common part of AutoSizeColumn/Row()
    void AutoSizeColOrRow(int n, bool setAsMin, wxGridDirection direction);

    // Calculate the minimum acceptable size for labels area
    wxCoord CalcColOrRowLabelAreaMinSize(wxGridDirection direction);

    // get the minimal width of the given column/row
    int GetColMinimalWidth(int col) const;
    int GetRowMinimalHeight(int col) const;

    // do we have some place to store attributes in?
    bool CanHaveAttributes() const;

    // cell attribute cache (currently we only cache 1, may be will do
    // more/better later)
    struct CachedAttr
    {
        wxGridCellAttr *attr{nullptr};
        int             row{-1};
        int             col{-1};
    } m_attrCache;

    // invalidates the attribute cache
    void ClearAttrCache();

    // adds an attribute to cache
    void CacheAttr(int row, int col, wxGridCellAttr *attr) const;

    // looks for an attr in cache, returns true if found
    bool LookupAttr(int row, int col, wxGridCellAttr **attr) const;

    // looks for the attr in cache, if not found asks the table and caches the
    // result
    wxGridCellAttr *GetCellAttr(int row, int col) const;
    wxGridCellAttr *GetCellAttr(const wxGridCellCoords& coords ) const
        { return GetCellAttr( coords.GetRow(), coords.GetCol() ); }

    wxGridCellAttrPtr GetCellAttrPtr(int row, int col) const
    {
        return wxGridCellAttrPtr(GetCellAttr(row, col));
    }
    wxGridCellAttrPtr GetCellAttrPtr(const wxGridCellCoords& coords) const
    {
        return wxGridCellAttrPtr(GetCellAttr(coords));
    }

    // this method not only sets m_cursorMode but also sets the correct cursor
    // for the given mode and, if captureMouse is not false releases the mouse
    // if it was captured and captures it if it must be captured
    //
    // for this to work, you should always use it and not set m_cursorMode
    // directly!
    void ChangeCursorMode(CursorMode mode,
                          wxWindow *win = nullptr,
                          bool captureMouse = true);

    void Create();
    void CreateColumnWindow();
    void CalcDimensions();
    void CalcWindowSizes();
    bool Redimension( wxGridTableMessage& );

    enum EventResult
    {
        Event_Vetoed = -1,
        Event_Unhandled,
        Event_Handled,
        Event_CellDeleted   // Event handler deleted the cell.
    };

    // Send the given grid event and returns one of the event handling results
    // defined above.
    EventResult DoSendEvent(wxGridEvent& gridEvt);

    // Generate an event of the given type and call DoSendEvent().
    EventResult SendEvent(wxEventType evtType,
                  int row, int col,
                  const wxMouseEvent& e);
    EventResult SendEvent(wxEventType evtType,
                  const wxGridCellCoords& coords,
                  const wxMouseEvent& e)
        { return SendEvent(evtType, coords.GetRow(), coords.GetCol(), e); }
    EventResult SendEvent(wxEventType evtType,
                  int row, int col,
                  const std::string& s = {});
    EventResult SendEvent(wxEventType evtType,
                  const wxGridCellCoords& coords,
                  const std::string& s = {})
        { return SendEvent(evtType, coords.GetRow(), coords.GetCol(), s); }
    EventResult SendEvent(wxEventType evtType, const std::string& s = {})
        { return SendEvent(evtType, m_currentCellCoords, s); }

    // send wxEVT_GRID_{ROW,COL}_SIZE or wxEVT_GRID_COL_AUTO_SIZE, return true
    // if the event was processed, false otherwise
    bool SendGridSizeEvent(wxEventType type,
                           int row, int col,
                           const wxMouseEvent& mouseEv);

    void OnSize( wxSizeEvent& );
    void OnKeyDown( wxKeyEvent& );
    void OnKeyUp( wxKeyEvent& );
    void OnChar( wxKeyEvent& );


    bool SetCurrentCell( const wxGridCellCoords& coords );
    bool SetCurrentCell( int row, int col )
        { return SetCurrentCell( wxGridCellCoords(row, col) ); }


    bool ShouldScrollToChildOnFocus(wxWindow* WXUNUSED(win)) override
        { return false; }

    friend class wxGridSelection;
    friend class wxGridRowOperations;
    friend class wxGridColumnOperations;

    // they call our private Process{{Corner,Col,Row}Label,GridCell}MouseEvent()
    friend class wxGridCornerLabelWindow;
    friend class wxGridColLabelWindow;
    friend class wxGridRowLabelWindow;
    friend class wxGridWindow;
    friend class wxGridHeaderRenderer;

    friend class wxGridHeaderColumn;
    friend class wxGridHeaderCtrl;

private:
    // This is called from both Create() and OnDPIChanged() to (re)initialize
    // the values in pixels, which depend on the current DPI.
    void InitPixelFields();

    // Event handler for DPI change event recomputes pixel values and relays
    // out the grid.
    void OnDPIChanged(wxDPIChangedEvent& event);

    // implement wxScrolledCanvas method to return m_gridWin size
    wxSize GetSizeAvailableForScrollTarget(const wxSize& size) override;

    // depending on the values of m_numFrozenRows and m_numFrozenCols, it will
    // create and initialize or delete the frozen windows
    void InitializeFrozenWindows();

    // redraw the grid lines, should be called after changing their attributes
    void RedrawGridLines();

    // draw all grid lines in the given cell region (unlike the public
    // DrawAllGridLines() which just draws all of them)
    void DrawRangeGridLines(wxDC& dc, const wxRegion& reg,
                            const wxGridCellCoords& topLeft,
                            const wxGridCellCoords& bottomRight);

    // draw all lines from top to bottom row and left to right column in the
    // rectangle determined by (top, left)-(bottom, right) -- but notice that
    // the caller must have set up the clipping correctly, this rectangle is
    // only used here for optimization
    void DoDrawGridLines(wxDC& dc,
                         int top, int left,
                         int bottom, int right,
                         int topRow, int leftCol,
                         int bottomRight, int rightCol);

    // common part of Clip{Horz,Vert}GridLines
    void DoClipGridLines(bool& var, bool clip);

    // Redimension() helper: update m_currentCellCoords if necessary after a
    // grid size change
    void UpdateCurrentCellOnRedim();

    // update the sorting indicator shown in the specified column (whose index
    // must be valid)
    //
    // this will use GetSortingColumn() and IsSortOrderAscending() to determine
    // the sorting indicator to effectively show
    void UpdateColumnSortingIndicator(int col);

    // update the grid after changing the columns order (common part of
    // SetColPos() and ResetColPos())
    void RefreshAfterColPosChange();

    // reset the variables used during dragging operations after it ended,
    // either because we called EndDraggingIfNecessary() ourselves or because
    // we lost mouse capture
    void DoAfterDraggingEnd();

    // release the mouse capture if it's currently captured
    void EndDraggingIfNecessary();

    // return true if the grid should be refreshed right now
    bool ShouldRefresh() const
    {
        return !GetBatchCount() && IsShownOnScreen();
    }


    // return the position (not index) of the column at the given logical pixel
    // position
    //
    // this always returns a valid position, even if the coordinate is out of
    // bounds (in which case first/last column is returned)
    int XToPos(int x, wxGridWindow *gridWindow) const;

    // event handlers and their helpers
    // --------------------------------

    // process mouse drag event in CursorMode::SelectCell mode
    bool DoGridCellDrag(wxMouseEvent& event,
                        const wxGridCellCoords& coords,
                        bool isFirstDrag);

    // process mouse drag event in the grid window, return false if starting
    // dragging was vetoed by the user-defined wxEVT_GRID_CELL_BEGIN_DRAG
    // handler
    bool DoGridDragEvent(wxMouseEvent& event,
                         const wxGridCellCoords& coords,
                         bool isFirstDrag,
                         wxGridWindow* gridWindow);

    // Update the width/height of the column/row being drag-resized.
    void DoGridDragResize(const wxPoint& position,
                          const wxGridOperations& oper,
                          wxGridWindow* gridWindow);

    // process different clicks on grid cells
    void DoGridCellLeftDown(wxMouseEvent& event,
                            const wxGridCellCoords& coords,
                            const wxPoint& pos);
    void DoGridCellLeftDClick(wxMouseEvent& event,
                             const wxGridCellCoords& coords,
                             const wxPoint& pos);
    void DoGridCellLeftUp(wxMouseEvent& event,
                          const wxGridCellCoords& coords,
                          wxGridWindow* gridWindow);

    // process movement (but not dragging) event in the grid cell area
    void DoGridMouseMoveEvent(wxMouseEvent& event,
                              const wxGridCellCoords& coords,
                              const wxPoint& pos,
                              wxGridWindow* gridWindow);

    // process mouse events in the grid window
    void ProcessGridCellMouseEvent(wxMouseEvent& event, wxGridWindow* gridWindow);

    // process mouse events in the row/column labels/corner windows
    void ProcessRowLabelMouseEvent(wxMouseEvent& event,
                                   wxGridRowLabelWindow* rowLabelWin);
    void ProcessColLabelMouseEvent(wxMouseEvent& event,
                                   wxGridColLabelWindow* colLabelWin);
    void ProcessCornerLabelMouseEvent(wxMouseEvent& event);

    void HandleColumnAutosize(int col, const wxMouseEvent& event);

    void DoColHeaderClick(int col);

    void DoStartResizeRowOrCol(int col);
    void DoStartMoveCol(int col);

    void DoEndDragResizeRow(const wxMouseEvent& event, wxGridWindow *gridWindow);
    void DoEndDragResizeCol(const wxMouseEvent& event, wxGridWindow *gridWindow);
    void DoEndMoveCol(int pos);

    // Helper function returning the position (only the horizontal component
    // really counts) corresponding to the given column drag-resize event.
    //
    // It's a bit ugly to create a phantom mouse position when we really only
    // need the column width anyhow, but wxGrid code was originally written to
    // expect the position and not the width and it's simpler to keep it happy
    // by giving it the position than to change it.
    wxPoint GetPositionForResizeEvent(int width) const;

    // functions called by wxGridHeaderCtrl while resizing m_dragRowOrCol
    void DoHeaderStartDragResizeCol(int col);
    void DoHeaderDragResizeCol(int width);
    void DoHeaderEndDragResizeCol(int width);

    // process a TAB keypress
    void DoGridProcessTab(wxKeyboardState& kbdState);

    // common implementations of methods defined for both rows and columns
    int PosToLinePos(int pos, bool clipToMinMax,
                     const wxGridOperations& oper,
                     wxGridWindow *gridWindow) const;
    int PosToLine(int pos, bool clipToMinMax,
                  const wxGridOperations& oper,
                  wxGridWindow *gridWindow) const;
    int PosToEdgeOfLine(int pos, const wxGridOperations& oper) const;

    void DoMoveCursorFromKeyboard(const wxKeyboardState& kbdState,
                                  const wxGridDirectionOperations& diroper);
    bool DoMoveCursor(const wxKeyboardState& kbdState,
                      const wxGridDirectionOperations& diroper);
    bool DoMoveCursorByPage(const wxKeyboardState& kbdState,
                            const wxGridDirectionOperations& diroper);
    bool AdvanceByPage(wxGridCellCoords& coords,
                       const wxGridDirectionOperations& diroper);
    bool DoMoveCursorByBlock(const wxKeyboardState& kbdState,
                             const wxGridDirectionOperations& diroper);
    void AdvanceToNextNonEmpty(wxGridCellCoords& coords,
                               const wxGridDirectionOperations& diroper);
    bool AdvanceByBlock(wxGridCellCoords& coords,
                        const wxGridDirectionOperations& diroper);

    // common part of {Insert,Delete}{Rows,Cols}
    bool DoModifyLines(bool (wxGridTableBase::*funcModify)(size_t, size_t),
                       int pos, int num, bool updateLabels);
    // Append{Rows,Cols} is a bit different because of one less parameter
    bool DoAppendLines(bool (wxGridTableBase::*funcAppend)(size_t),
                       int num, bool updateLabels);

    // common part of Set{Col,Row}Sizes
    void DoSetSizes(const wxGridSizesInfo& sizeInfo,
                    const wxGridOperations& oper);

    // common part of Disable{Row,Col}Resize and CanDrag{Row,Col}Size
    void DoDisableLineResize(int line, wxGridFixedIndicesSet *& setFixed);
    bool DoCanResizeLine(int line, const wxGridFixedIndicesSet *setFixed) const;

    // Helper of Render(): get grid size, origin offset and fill cell arrays
    void GetRenderSizes( const wxGridCellCoords& topLeft,
                         const wxGridCellCoords& bottomRight,
                         wxPoint& pointOffSet, wxSize& sizeGrid,
                         wxGridCellCoordsArray& renderCells,
                         std::vector<int>& arrayCols, std::vector<int>& arrayRows ) const;

    // Helper of Render(): set the scale to draw the cells at the right size.
    void SetRenderScale( wxDC& dc, const wxPoint& pos, const wxSize& size,
                         const wxSize& sizeGrid );

    // Helper of Render(): get render start position from passed parameter
    wxPoint GetRenderPosition( wxDC& dc, const wxPoint& position );

    // Helper of Render(): draws a box around the rendered area
    void DoRenderBox( wxDC& dc,
                      unsigned int style,
                      const wxPoint& pointOffSet,
                      const wxSize& sizeCellArea,
                      const wxGridCellCoords& topLeft,
                      const wxGridCellCoords& bottomRight );

    // Implementation of public Set{Row,Col}Size() and {Hide,Show}{Row,Col}().
    // They interpret their height or width parameter slightly different from
    // the public methods where -1 in it means "auto fit to the label" for the
    // compatibility reasons. Here it means "show a previously hidden row or
    // column" while 0 means "hide it" just as in the public methods. And any
    // positive values are handled naturally, i.e. they just specify the size.
    void DoSetRowSize( int row, int height );
    void DoSetColSize( int col, int width );

    // These methods can only be called when m_useNativeHeader is true and call
    // SetColumnCount() and Set- or ResetColumnsOrder() as necessary on the
    // native wxHeaderCtrl being used. Note that the first one already calls
    // the second one, so it's never necessary to call both of them.
    void SetNativeHeaderColCount();
    void SetNativeHeaderColOrder();

    // Return the editor which should be used for the current cell.
    wxGridCellEditorPtr GetCurrentCellEditorPtr() const
    {
        return GetCellAttrPtr(m_currentCellCoords)->GetEditorPtr
                (
                    this,
                    m_currentCellCoords.GetRow(),
                    m_currentCellCoords.GetCol()
                );
    }

    // Show/hide the cell editor for the current cell unconditionally.

    // Return false if the editor was activated instead of being shown and also
    // sets m_cellEditCtrlEnabled to true when it returns true as a side effect.
    bool DoShowCellEditControl(const wxGridActivationSource& actSource);
    void DoHideCellEditControl();

    // Unconditionally try showing the editor for the current cell.
    //
    // Returns false if the user code vetoed wxEVT_GRID_EDITOR_SHOWN or if the
    // editor was simply activated and won't be permanently shown.
    bool DoEnableCellEditControl(const wxGridActivationSource& actSource);

    // Unconditionally disable (accepting the changes) the editor.
    void DoDisableCellEditControl();

    // Accept the changes in the edit control, i.e. save them to the table and
    // dismiss the editor. Also reset m_cellEditCtrlEnabled.
    void DoAcceptCellEditControl();

    // As above, but do nothing if the control is not currently shown.
    void AcceptCellEditControlIfShown();

    // Unlike the public SaveEditControlValue(), this method doesn't check if
    // the edit control is shown, but just supposes that it is.
    void DoSaveEditControlValue();

    // these sets contain the indices of fixed, i.e. non-resizable
    // interactively, grid rows or columns and are NULL if there are no fixed
    // elements (which is the default)
    wxGridFixedIndicesSet *m_setFixedRows{nullptr};
    wxGridFixedIndicesSet *m_setFixedCols{nullptr};

    wxDECLARE_DYNAMIC_CLASS(wxGrid);
    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// wxGridUpdateLocker prevents updates to a grid during its lifetime
// ----------------------------------------------------------------------------

class wxGridUpdateLocker
{
public:
    // if the pointer is NULL, Create() can be called later
    explicit wxGridUpdateLocker(wxGrid *grid = nullptr)
        : m_grid{grid}
    {
        if (m_grid)
            m_grid->BeginBatch();
    }

    wxGridUpdateLocker(const wxGridUpdateLocker&) = delete;
    wxGridUpdateLocker& operator=(const wxGridUpdateLocker&) = delete;

    // can be called if ctor was used with a NULL pointer, must not be called
    // more than once
    // FIXME: Can be called with nullptr.
    // FIXME: This isn't a good way to lock a resource.
    void Create(wxGrid *grid)
    {
        wxASSERT_MSG( !m_grid, "shouldn't be called more than once" );

        m_grid = grid;

        if (m_grid)
            m_grid->BeginBatch();
    }

    ~wxGridUpdateLocker()
    {
        if ( m_grid )
            m_grid->EndBatch();
    }

private:
    wxGrid *m_grid;
};

#endif // wxUSE_GRID
#endif // _WX_GENERIC_GRID_H_
