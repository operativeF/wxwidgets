/////////////////////////////////////////////////////////////////////////////
// Name:        wx/tbarbase.h
// Purpose:     Base class for toolbar classes
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TBARBASE_H_
#define _WX_TBARBASE_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/defs.h"

#if wxUSE_TOOLBAR

#include "wx/bitmap.h"
#include "wx/list.h"
#include "wx/control.h"

class WXDLLIMPEXP_FWD_CORE wxToolBarBase;
class WXDLLIMPEXP_FWD_CORE wxToolBarToolBase;
class WXDLLIMPEXP_FWD_CORE wxImage;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

inline constexpr char wxToolBarNameStr[] = "toolbar";

enum wxToolBarToolStyle
{
    wxTOOL_STYLE_BUTTON    = 1,
    wxTOOL_STYLE_SEPARATOR = 2,
    wxTOOL_STYLE_CONTROL
};

// ----------------------------------------------------------------------------
// wxToolBarTool is a toolbar element.
//
// It has a unique id (except for the separators which always have id wxID_ANY), the
// style (telling whether it is a normal button, separator or a control), the
// state (toggled or not, enabled or not) and short and long help strings. The
// default implementations use the short help string for the tooltip text which
// is popped up when the mouse pointer enters the tool and the long help string
// for the applications status bar.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxToolBarToolBase : public wxObject
{
public:

    // generic ctor for any kind of tool
    wxToolBarToolBase(wxToolBarBase *tbar = nullptr,
                      int toolid = wxID_SEPARATOR,
                      const std::string& label = {},
                      const wxBitmap& bmpNormal = wxNullBitmap,
                      const wxBitmap& bmpDisabled = wxNullBitmap,
                      wxItemKind kind = wxITEM_NORMAL,
                      wxObject *clientData = nullptr,
                      const std::string& shortHelpString = {},
                      const std::string& longHelpString = "")
        : m_tbar(tbar),
          m_label(label),
          m_shortHelpString(shortHelpString),
          m_longHelpString(longHelpString),
          m_bmpNormal(bmpNormal),
          m_bmpDisabled(bmpDisabled),
          m_clientData(clientData),
          m_kind(kind)
    {
        m_toolStyle = toolid == wxID_SEPARATOR ? wxTOOL_STYLE_SEPARATOR : wxTOOL_STYLE_BUTTON;
        m_id = toolid == wxID_ANY ? wxWindow::NewControlId() : toolid;
    }

    // ctor for controls only
    wxToolBarToolBase(wxToolBarBase *tbar,
                      wxControl *control,
                      const std::string& label)
        : m_label(label),
          m_control(control),
          m_tbar(tbar),
          m_toolStyle(wxTOOL_STYLE_CONTROL),
          m_kind(wxITEM_MAX),
          m_id(control->GetId())
    {
    }

    ~wxToolBarToolBase();

    wxToolBarToolBase(const wxToolBarToolBase&) = delete;
    wxToolBarToolBase& operator=(const wxToolBarToolBase&) = delete;
    wxToolBarToolBase(wxToolBarToolBase&&) = default;
    wxToolBarToolBase& operator=(wxToolBarToolBase&&) = default;

    // general
    int GetId() const { return m_id; }

    wxControl *GetControl() const
    {
        wxASSERT_MSG( IsControl(), wxT("this toolbar tool is not a control") );

        return m_control;
    }

    wxToolBarBase *GetToolBar() const { return m_tbar; }

    // style/kind
    bool IsStretchable() const { return m_stretchable; }
    bool IsButton() const { return m_toolStyle == wxTOOL_STYLE_BUTTON; }
    bool IsControl() const { return m_toolStyle == wxTOOL_STYLE_CONTROL; }
    bool IsSeparator() const { return m_toolStyle == wxTOOL_STYLE_SEPARATOR; }
    bool IsStretchableSpace() const { return IsSeparator() && IsStretchable(); }
    int GetStyle() const { return m_toolStyle; }
    wxItemKind GetKind() const
    {
        wxASSERT_MSG( IsButton(), wxT("only makes sense for buttons") );

        return m_kind;
    }

    void MakeStretchable()
    {
        wxASSERT_MSG( IsSeparator(), "only separators can be stretchable" );

        m_stretchable = true;
    }

    // state
    bool IsEnabled() const { return m_enabled; }
    bool IsToggled() const { return m_toggled; }
    bool CanBeToggled() const
        { return m_kind == wxITEM_CHECK || m_kind == wxITEM_RADIO; }

    // attributes
    const wxBitmap& GetNormalBitmap() const { return m_bmpNormal; }
    const wxBitmap& GetDisabledBitmap() const { return m_bmpDisabled; }

    const wxBitmap& GetBitmap() const
        { return IsEnabled() ? GetNormalBitmap() : GetDisabledBitmap(); }

    const std::string& GetLabel() const { return m_label; }

    const std::string& GetShortHelp() const { return m_shortHelpString; }
    const std::string& GetLongHelp() const { return m_longHelpString; }

    wxObject *GetClientData() const
    {
        if ( m_toolStyle == wxTOOL_STYLE_CONTROL )
        {
            return (wxObject*)m_control->GetClientData();
        }
        else
        {
            return m_clientData;
        }
    }

    // modifiers: return true if the state really changed
    virtual bool Enable(bool enable);
    virtual bool Toggle(bool toggle);
    virtual bool SetToggle(bool toggle);
    virtual bool SetShortHelp(const std::string& help);
    virtual bool SetLongHelp(const std::string& help);

    void Toggle() { Toggle(!IsToggled()); }

    void SetNormalBitmap(const wxBitmap& bmp) { m_bmpNormal = bmp; }
    void SetDisabledBitmap(const wxBitmap& bmp) { m_bmpDisabled = bmp; }

    virtual void SetLabel(const std::string& label) { m_label = label; }

    void SetClientData(wxObject *clientData)
    {
        if ( m_toolStyle == wxTOOL_STYLE_CONTROL )
        {
            m_control->SetClientData(clientData);
        }
        else
        {
            m_clientData = clientData;
        }
    }

    // add tool to/remove it from a toolbar
    virtual void Detach() { m_tbar = nullptr; }
    virtual void Attach(wxToolBarBase *tbar) { m_tbar = tbar; }

#if wxUSE_MENUS
    // these methods are only for tools of wxITEM_DROPDOWN kind (but even such
    // tools can have a NULL associated menu)
    virtual void SetDropdownMenu(wxMenu *menu);
    wxMenu *GetDropdownMenu() const { return m_dropdownMenu; }
#endif

protected:
    wxToolBarBase *m_tbar;  // the toolbar to which we belong (may be NULL)

    // tool parameters
    wxToolBarToolStyle m_toolStyle;
    wxWindowIDRef m_id; // the tool id, wxID_SEPARATOR for separator
    wxItemKind m_kind;  // for normal buttons may be wxITEM_NORMAL/CHECK/RADIO

    // as controls have their own client data, no need to waste memory
    union
    {
        wxObject         *m_clientData;
        wxControl        *m_control;
    };

    // true if this tool is stretchable: currently is only value for separators
    bool m_stretchable{false};

    // tool state
    bool m_toggled{false};
    bool m_enabled{true};

    // normal and disabled bitmaps for the tool, both can be invalid
    wxBitmap m_bmpNormal;
    wxBitmap m_bmpDisabled;

    // the button label
    std::string m_label;

    // short and long help strings
    std::string m_shortHelpString;
    std::string m_longHelpString;

#if wxUSE_MENUS
    wxMenu *m_dropdownMenu{nullptr};
#endif

public:
	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// a list of toolbar tools
WX_DECLARE_EXPORTED_LIST(wxToolBarToolBase, wxToolBarToolsList);

// ----------------------------------------------------------------------------
// the base class for all toolbars
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxToolBarBase : public wxControl
{
public:
    wxToolBarBase();
    ~wxToolBarBase();

    wxToolBarBase(const wxToolBarBase&) = delete;
    wxToolBarBase& operator=(const wxToolBarBase&) = delete;
    wxToolBarBase(wxToolBarBase&&) = default;
    wxToolBarBase& operator=(wxToolBarBase&&) = default;

    // the full AddTool() function
    //
    // If bmpDisabled is wxNullBitmap, a shadowed version of the normal bitmap
    // is created and used as the disabled image.
    wxToolBarToolBase *AddTool(int toolid,
                               const std::string& label,
                               const wxBitmap& bitmap,
                               const wxBitmap& bmpDisabled,
                               wxItemKind kind = wxITEM_NORMAL,
                               const std::string& shortHelp = {},
                               const std::string& longHelp = {},
                               wxObject *clientData = nullptr)
    {
        return DoAddTool(toolid, label, bitmap, bmpDisabled, kind,
                         shortHelp, longHelp, clientData);
    }

    // the most common AddTool() version
    wxToolBarToolBase *AddTool(int toolid,
                               const std::string& label,
                               const wxBitmap& bitmap,
                               const std::string& shortHelp = {},
                               wxItemKind kind = wxITEM_NORMAL)
    {
        return AddTool(toolid, label, bitmap, wxNullBitmap, kind, shortHelp);
    }

    // add a check tool, i.e. a tool which can be toggled
    wxToolBarToolBase *AddCheckTool(int toolid,
                                    const std::string& label,
                                    const wxBitmap& bitmap,
                                    const wxBitmap& bmpDisabled = wxNullBitmap,
                                    const std::string& shortHelp = {},
                                    const std::string& longHelp = {},
                                    wxObject *clientData = nullptr)
    {
        return AddTool(toolid, label, bitmap, bmpDisabled, wxITEM_CHECK,
                       shortHelp, longHelp, clientData);
    }

    // add a radio tool, i.e. a tool which can be toggled and releases any
    // other toggled radio tools in the same group when it happens
    wxToolBarToolBase *AddRadioTool(int toolid,
                                    const std::string& label,
                                    const wxBitmap& bitmap,
                                    const wxBitmap& bmpDisabled = wxNullBitmap,
                                    const std::string& shortHelp = {},
                                    const std::string& longHelp = {},
                                    wxObject *clientData = nullptr)
    {
        return AddTool(toolid, label, bitmap, bmpDisabled, wxITEM_RADIO,
                       shortHelp, longHelp, clientData);
    }


    // insert the new tool at the given position, if pos == GetToolsCount(), it
    // is equivalent to AddTool()
    virtual wxToolBarToolBase *InsertTool
                               (
                                    size_t pos,
                                    int toolid,
                                    const std::string& label,
                                    const wxBitmap& bitmap,
                                    const wxBitmap& bmpDisabled = wxNullBitmap,
                                    wxItemKind kind = wxITEM_NORMAL,
                                    const std::string& shortHelp = {},
                                    const std::string& longHelp = {},
                                    wxObject *clientData = nullptr
                               );

    virtual wxToolBarToolBase *AddTool (wxToolBarToolBase *tool);
    virtual wxToolBarToolBase *InsertTool (size_t pos, wxToolBarToolBase *tool);

    // add an arbitrary control to the toolbar (notice that the control will be
    // deleted by the toolbar and that it will also adjust its position/size)
    //
    // the label is optional and, if specified, will be shown near the control
    // NB: the control should have toolbar as its parent
    virtual wxToolBarToolBase *
    AddControl(wxControl *control, const std::string& label = "");

    virtual wxToolBarToolBase *
    InsertControl(size_t pos, wxControl *control,
                  const std::string& label = "");

    // get the control with the given id or return NULL
    virtual wxControl *FindControl( int toolid );

    // add a separator to the toolbar
    virtual wxToolBarToolBase *AddSeparator();
    virtual wxToolBarToolBase *InsertSeparator(size_t pos);

    // add a stretchable space to the toolbar: this is similar to a separator
    // except that it's always blank and that all the extra space the toolbar
    // has is [equally] distributed among the stretchable spaces in it
    virtual wxToolBarToolBase *AddStretchableSpace();
    virtual wxToolBarToolBase *InsertStretchableSpace(size_t pos);

    // remove the tool from the toolbar: the caller is responsible for actually
    // deleting the pointer
    virtual wxToolBarToolBase *RemoveTool(int toolid);

    // delete tool either by index or by position
    virtual bool DeleteToolByPos(size_t pos);
    virtual bool DeleteTool(int toolid);

    // delete all tools
    virtual void ClearTools();

    // must be called after all buttons have been created to finish toolbar
    // initialisation
    //
    // derived class versions should call the base one first, before doing
    // platform-specific stuff
    virtual bool Realize();

    // tools state
    // -----------

    virtual void EnableTool(int toolid, bool enable);
    virtual void ToggleTool(int toolid, bool toggle);

    // Set this to be togglable (or not)
    virtual void SetToggle(int toolid, bool toggle);

    // set/get tools client data (not for controls)
    virtual wxObject *GetToolClientData(int toolid) const;
    virtual void SetToolClientData(int toolid, wxObject *clientData);

    // returns tool pos, or wxNOT_FOUND if tool isn't found
    virtual int GetToolPos(int id) const;

    // return true if the tool is toggled
    virtual bool GetToolState(int toolid) const;

    virtual bool GetToolEnabled(int toolid) const;

    virtual void SetToolShortHelp(int toolid, const std::string& helpString);
    virtual std::string GetToolShortHelp(int toolid) const;
    virtual void SetToolLongHelp(int toolid, const std::string& helpString);
    virtual std::string GetToolLongHelp(int toolid) const;

    virtual void SetToolNormalBitmap(int WXUNUSED(id),
                                     const wxBitmap& WXUNUSED(bitmap)) {}
    virtual void SetToolDisabledBitmap(int WXUNUSED(id),
                                       const wxBitmap& WXUNUSED(bitmap)) {}


    // margins/packing/separation
    // --------------------------

    virtual void SetMargins(int x, int y);
    void SetMargins(const wxSize& size)
        { SetMargins((int) size.x, (int) size.y); }
    virtual void SetToolPacking(int packing)
        { m_toolPacking = packing; }
    virtual void SetToolSeparation(int separation)
        { m_toolSeparation = separation; }

    virtual wxSize GetToolMargins() const { return wxSize(m_xMargin, m_yMargin); }
    virtual int GetToolPacking() const { return m_toolPacking; }
    virtual int GetToolSeparation() const { return m_toolSeparation; }

    // toolbar geometry
    // ----------------

    // set the number of toolbar rows
    virtual void SetRows(int nRows);

    // the toolbar can wrap - limit the number of columns or rows it may take
    void SetMaxRowsCols(int rows, int cols)
        { m_maxRows = rows; m_maxCols = cols; }
    int GetMaxRows() const { return m_maxRows; }
    int GetMaxCols() const { return m_maxCols; }

    // get/set the size of the bitmaps used by the toolbar: should be called
    // before adding any tools to the toolbar
    virtual void SetToolBitmapSize(const wxSize& size)
        { m_defaultWidth = size.x; m_defaultHeight = size.y; }
    virtual wxSize GetToolBitmapSize() const
        { return wxSize(m_defaultWidth, m_defaultHeight); }

    // the button size in some implementations is bigger than the bitmap size:
    // get the total button size (by default the same as bitmap size)
    virtual wxSize GetToolSize() const
        { return GetToolBitmapSize(); }

    // returns a (non separator) tool containing the point (x, y) or NULL if
    // there is no tool at this point (coordinates are client)
    virtual wxToolBarToolBase *FindToolForPosition(wxCoord x,
                                                   wxCoord y) const = 0;

    // find the tool by id
    wxToolBarToolBase *FindById(int toolid) const;

    // return true if this is a vertical toolbar, otherwise false
    bool IsVertical() const;

    // returns one of wxTB_TOP, wxTB_BOTTOM, wxTB_LEFT, wxTB_RIGHT
    // indicating where the toolbar is placed in the associated frame
    int GetDirection() const;

    // these methods allow to access tools by their index in the toolbar
    size_t GetToolsCount() const { return m_tools.GetCount(); }
    wxToolBarToolBase *GetToolByPos(int pos) { return m_tools[pos]; }
    const wxToolBarToolBase *GetToolByPos(int pos) const { return m_tools[pos]; }

    // event handlers
    // --------------

    // NB: these functions are deprecated, use EVT_TOOL_XXX() instead!

    // Only allow toggle if returns true. Call when left button up.
    virtual bool OnLeftClick(int toolid, bool toggleDown);

    // Call when right button down.
    virtual void OnRightClick(int toolid, long x, long y);

    // Called when the mouse cursor enters a tool bitmap.
    // Argument is wxID_ANY if mouse is exiting the toolbar.
    virtual void OnMouseEnter(int toolid);

    // more deprecated functions
    // -------------------------

    // use GetToolMargins() instead
    wxSize GetMargins() const { return GetToolMargins(); }

    // Tool factories,
    // helper functions to create toolbar tools
    // -------------------------
    virtual wxToolBarToolBase *CreateTool(int toolid,
                                          const std::string& label,
                                          const wxBitmap& bmpNormal,
                                          const wxBitmap& bmpDisabled = wxNullBitmap,
                                          wxItemKind kind = wxITEM_NORMAL,
                                          wxObject *clientData = nullptr,
                                          const std::string& shortHelp = {},
                                          const std::string& longHelp = "") = 0;

    virtual wxToolBarToolBase *CreateTool(wxControl *control,
                                          const std::string& label) = 0;

    // this one is not virtual but just a simple helper/wrapper around
    // CreateTool() for separators
    wxToolBarToolBase *CreateSeparator()
    {
        return CreateTool(wxID_SEPARATOR,
                          "",
                          wxNullBitmap, wxNullBitmap,
                          wxITEM_SEPARATOR, nullptr,
                          "", "");
    }


    // implementation only from now on
    // -------------------------------

    // Do the toolbar button updates (check for EVT_UPDATE_UI handlers)
    void UpdateWindowUI(long flags = wxUPDATE_UI_NONE) override ;

    // don't want toolbars to accept the focus
    bool AcceptsFocus() const override { return false; }

#if wxUSE_MENUS
    // Set dropdown menu
    bool SetDropdownMenu(int toolid, wxMenu *menu);
#endif

protected:
    // choose the default border for this window
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    // to implement in derived classes
    // -------------------------------

    // create a new toolbar tool and add it to the toolbar, this is typically
    // implemented by just calling InsertTool()
    virtual wxToolBarToolBase *DoAddTool
                               (
                                   int toolid,
                                   const std::string& label,
                                   const wxBitmap& bitmap,
                                   const wxBitmap& bmpDisabled,
                                   wxItemKind kind,
                                   const std::string& shortHelp = {},
                                   const std::string& longHelp = {},
                                   wxObject *clientData = nullptr,
                                   wxCoord xPos = wxDefaultCoord,
                                   wxCoord yPos = wxDefaultCoord
                               );

    // the tool is not yet inserted into m_tools list when this function is
    // called and will only be added to it if this function succeeds
    virtual bool DoInsertTool(size_t pos, wxToolBarToolBase *tool) = 0;

    // the tool is still in m_tools list when this function is called, it will
    // only be deleted from it if it succeeds
    virtual bool DoDeleteTool(size_t pos, wxToolBarToolBase *tool) = 0;

    // called when the tools enabled flag changes
    virtual void DoEnableTool(wxToolBarToolBase *tool, bool enable) = 0;

    // called when the tool is toggled
    virtual void DoToggleTool(wxToolBarToolBase *tool, bool toggle) = 0;

    // called when the tools "can be toggled" flag changes
    virtual void DoSetToggle(wxToolBarToolBase *tool, bool toggle) = 0;


    // helper functions
    // ----------------

    // call this from derived class ctor/Create() to ensure that we have either
    // wxTB_HORIZONTAL or wxTB_VERTICAL style, there is a lot of existing code
    // which randomly checks either one or the other of them and gets confused
    // if neither is set (and making one of them 0 is not an option neither as
    // then the existing tests would break down)
    void FixupStyle();

    // un-toggle all buttons in the same radio group
    void UnToggleRadioGroup(wxToolBarToolBase *tool);

    // make the size of the buttons big enough to fit the largest bitmap size
    void AdjustToolBitmapSize();

    // calls InsertTool() and deletes the tool if inserting it failed
    wxToolBarToolBase *DoInsertNewTool(size_t pos, wxToolBarToolBase *tool)
    {
        if ( !InsertTool(pos, tool) )
        {
            delete tool;
            return nullptr;
        }

        return tool;
    }

    // the list of all our tools
    wxToolBarToolsList m_tools;

    // the offset of the first tool
    int m_xMargin{0};
    int m_yMargin{0};

    // the maximum number of toolbar rows/columns
    int m_maxRows{0};
    int m_maxCols{0};

    // the tool packing and separation
    int m_toolPacking{0};
    int m_toolSeparation{0};

    // FIXME: Should we still do it like this?
    // even though modern Windows applications typically use 24*24 (or even
    // 32*32) size for their bitmaps, the native control itself still uses the
    // old 16*15 default size (see TB_SETBITMAPSIZE documentation in MSDN), so
    // default to it so that we don't call SetToolBitmapSize() unnecessarily in
    // wxToolBarBase::AdjustToolBitmapSize()
    wxCoord m_defaultWidth{16};
    wxCoord m_defaultHeight{15};

private:
    wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_TOOLBAR

#endif
    // _WX_TBARBASE_H_

