/////////////////////////////////////////////////////////////////////////////
// Name:        wx/propgrid/editors.h
// Purpose:     wxPropertyGrid editors
// Author:      Jaakko Salli
// Modified by:
// Created:     2007-04-14
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROPGRID_EDITORS_H_
#define _WX_PROPGRID_EDITORS_H_

#if wxUSE_PROPGRID

#include "wx/window.h"

class wxPGCell;
class wxPGProperty;
class wxPropertyGrid;

// -----------------------------------------------------------------------
// wxPGWindowList contains list of editor windows returned by CreateControls.

class wxPGWindowList
{
public:
    wxPGWindowList(wxWindow* primary, wxWindow* secondary = nullptr)
        : m_primary(primary)
        , m_secondary(secondary)
    {
    }

    void SetSecondary(wxWindow* secondary) { m_secondary = secondary; }

    wxWindow* GetPrimary() const { return m_primary; }
    wxWindow* GetSecondary() const { return m_secondary; }

    wxWindow*   m_primary;
    wxWindow*   m_secondary;
};

// -----------------------------------------------------------------------

// Base class for custom wxPropertyGrid editors.
// - Names of builtin property editors are: TextCtrl, Choice,
//   ComboBox, CheckBox, TextCtrlAndButton, and ChoiceAndButton. Additional
//   editors include SpinCtrl and DatePickerCtrl, but using them requires
//   calling wxPropertyGrid::RegisterAdditionalEditors() prior use.
// - Pointer to builtin editor is available as wxPGEditor_EditorName
//   (e.g. wxPGEditor_TextCtrl).
// - To add new editor you need to register it first using static function
//   wxPropertyGrid::RegisterEditorClass(), with code like this:
//      wxPGEditor *editorPointer = wxPropertyGrid::RegisterEditorClass(
//                                     new MyEditorClass(), "MyEditor");
//   After that, wxPropertyGrid will take ownership of the given object, but
//   you should still store editorPointer somewhere, so you can pass it to
//   wxPGProperty::SetEditor(), or return it from
//   wxPGEditor::DoGetEditorClass().
class wxPGEditor : public wxObject
{
    wxDECLARE_ABSTRACT_CLASS(wxPGEditor);
public:
    // Returns pointer to the name of the editor. For example,
    // wxPGEditor_TextCtrl has name "TextCtrl". If you don't need to access
    // your custom editor by string name, then you do not need to implement
    // this function.
    virtual wxString GetName() const;

    // Instantiates editor controls.
    //  propgrid- wxPropertyGrid to which the property belongs
    //    (use as parent for control).
    // property - Property for which this method is called.
    // pos - Position, inside wxPropertyGrid, to create control(s) to.
    // size - Initial size for control(s).
    // Unlike in previous version of wxPropertyGrid, it is no longer
    // necessary to call wxEvtHandler::Connect() for interesting editor
    // events. Instead, all events from control are now automatically
    // forwarded to wxPGEditor::OnEvent() and wxPGProperty::OnEvent().
    virtual wxPGWindowList CreateControls(wxPropertyGrid* propgrid,
                                          wxPGProperty* property,
                                          const wxPoint& pos,
                                          const wxSize& size) const = 0;

    // Loads value from property to the control.
    virtual void UpdateControl( wxPGProperty* property,
                                wxWindow* ctrl ) const = 0;

    // Used to get the renderer to draw the value with when the control is
    // hidden.
    // Default implementation returns g_wxPGDefaultRenderer.
    //virtual wxPGCellRenderer* GetCellRenderer() const;

    // Draws value for given property.
    virtual void DrawValue( wxDC& dc,
                            const wxRect& rect,
                            wxPGProperty* property,
                            const std::string& text ) const;

    // Handles events. Returns true if value in control was modified
    // (see wxPGProperty::OnEvent for more information).
    // wxPropertyGrid will automatically unfocus the editor when
    // wxEVT_TEXT_ENTER is received and when it results in
    // property value being modified. This happens regardless of
    // editor type (i.e. behaviour is same for any wxTextCtrl and
    // wxComboBox based editor).
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
        wxWindow* wnd_primary, wxEvent& event ) const = 0;

    // Returns value from control, via parameter 'variant'.
    // Usually ends up calling property's StringToValue or IntToValue.
    // Returns true if value was different.
    virtual bool GetValueFromControl( wxVariant& variant,
                                      wxPGProperty* property,
                                      wxWindow* ctrl ) const;

    // Sets new appearance for the control. Default implementation
    // sets foreground colour, background colour, font, plus text
    // for wxTextCtrl and wxComboCtrl.
    // appearance - New appearance to be applied.
    // oldAppearance - Previously applied appearance. Used to detect
    //   which control attributes need to be changed (e.g. so we only
    //   change background colour if really needed).
    // unspecified - true if the new appearance represents an unspecified
    // property value.
    virtual void SetControlAppearance( wxPropertyGrid* pg,
                                       wxPGProperty* property,
                                       wxWindow* ctrl,
                                       const wxPGCell& appearance,
                                       const wxPGCell& oldAppearance,
                                       bool unspecified ) const;

    // Sets value in control to unspecified.
    virtual void SetValueToUnspecified( wxPGProperty* property,
                                        wxWindow* ctrl ) const;

    // Sets control's value specifically from string.
    virtual void SetControlStringValue( wxPGProperty* property,
                                        wxWindow* ctrl,
                                        const wxString& txt ) const;

    // Sets control's value specifically from int (applies to choice etc.).
    virtual void SetControlIntValue( wxPGProperty* property,
                                     wxWindow* ctrl,
                                     int value ) const;

    // Inserts item to existing control. Index -1 means appending.
    // Default implementation does nothing. Returns index of item added.
    virtual int InsertItem( wxWindow* ctrl,
                            const wxString& label,
                            int index ) const;

    // Deletes item from existing control.
    // Default implementation does nothing.
    virtual void DeleteItem( wxWindow* ctrl, int index ) const;

    // Sets items of existing control.
    // Default implementation does nothing.
    virtual void SetItems(wxWindow* ctrl,  const std::vector<std::string>& labels) const;

    // Extra processing when control gains focus. For example, wxTextCtrl
    // based controls should select all text.
    virtual void OnFocus( wxPGProperty* property, wxWindow* wnd ) const;

    // Returns true if control itself can contain the custom image. Default is
    // to return false.
    virtual bool CanContainCustomImage() const;

    //
    // This member is public so scripting language bindings
    // wrapper code can access it freely.
    void*       m_clientData{nullptr};
};


#define WX_PG_IMPLEMENT_INTERNAL_EDITOR_CLASS(EDITOR,CLASSNAME,BASECLASS) \
wxIMPLEMENT_DYNAMIC_CLASS(CLASSNAME, BASECLASS); \
wxString CLASSNAME::GetName() const \
{ \
    return wxS(#EDITOR); \
} \
wxPGEditor* wxPGEditor_##EDITOR = NULL;


//
// Following are the built-in editor classes.
//

class wxPGTextCtrlEditor : public wxPGEditor
{
    wxDECLARE_DYNAMIC_CLASS(wxPGTextCtrlEditor);
public:
    wxPGTextCtrlEditor() = default;
    ~wxPGTextCtrlEditor();

    wxPGWindowList CreateControls(wxPropertyGrid* propgrid,
                                          wxPGProperty* property,
                                          const wxPoint& pos,
                                          const wxSize& size) const override;
    void UpdateControl( wxPGProperty* property,
                                wxWindow* ctrl ) const override;
    bool OnEvent( wxPropertyGrid* propgrid,
                          wxPGProperty* property,
                          wxWindow* primaryCtrl,
                          wxEvent& event ) const override;
    bool GetValueFromControl( wxVariant& variant,
                                      wxPGProperty* property,
                                      wxWindow* ctrl ) const override;

    wxString GetName() const override;

    //virtual wxPGCellRenderer* GetCellRenderer() const;
    void SetControlStringValue( wxPGProperty* property,
                                        wxWindow* ctrl,
                                        const wxString& txt ) const override;
    void OnFocus( wxPGProperty* property, wxWindow* wnd ) const override;

    // Provided so that, for example, ComboBox editor can use the same code
    // (multiple inheritance would get way too messy).
    static bool OnTextCtrlEvent( wxPropertyGrid* propgrid,
                                 wxPGProperty* property,
                                 wxWindow* ctrl,
                                 wxEvent& event );

    static bool GetTextCtrlValueFromControl( wxVariant& variant,
                                             wxPGProperty* property,
                                             wxWindow* ctrl );

};


class wxPGChoiceEditor : public wxPGEditor
{
    wxDECLARE_DYNAMIC_CLASS(wxPGChoiceEditor);
public:
    wxPGChoiceEditor() = default;
    ~wxPGChoiceEditor();

    wxPGWindowList CreateControls(wxPropertyGrid* propgrid,
                                          wxPGProperty* property,
                                          const wxPoint& pos,
                                          const wxSize& size) const override;
    void UpdateControl( wxPGProperty* property,
                                wxWindow* ctrl ) const override;
    bool OnEvent( wxPropertyGrid* propgrid,
                          wxPGProperty* property,
                          wxWindow* primaryCtrl,
                          wxEvent& event ) const override;
    bool GetValueFromControl( wxVariant& variant,
                                      wxPGProperty* property,
                                      wxWindow* ctrl ) const override;
    void SetValueToUnspecified( wxPGProperty* property,
                                        wxWindow* ctrl ) const override;
    wxString GetName() const override;

    void SetControlIntValue( wxPGProperty* property,
                                     wxWindow* ctrl,
                                     int value ) const override;
    void SetControlStringValue( wxPGProperty* property,
                                        wxWindow* ctrl,
                                        const wxString& txt ) const override;

    int InsertItem( wxWindow* ctrl,
                            const wxString& label,
                            int index ) const override;
    void DeleteItem( wxWindow* ctrl, int index ) const override;
    void SetItems(wxWindow* ctrl, const std::vector<std::string>& labels) const override;

    bool CanContainCustomImage() const override;

    // CreateControls calls this with CB_READONLY in extraStyle
    wxWindow* CreateControlsBase( wxPropertyGrid* propgrid,
                                  wxPGProperty* property,
                                  const wxPoint& pos,
                                  const wxSize& sz,
                                  long extraStyle ) const;

};


class wxPGComboBoxEditor : public wxPGChoiceEditor
{
    wxDECLARE_DYNAMIC_CLASS(wxPGComboBoxEditor);
public:
    wxPGComboBoxEditor() = default;
    ~wxPGComboBoxEditor();

    wxPGWindowList CreateControls(wxPropertyGrid* propgrid,
                                          wxPGProperty* property,
                                          const wxPoint& pos,
                                          const wxSize& size) const override;

    wxString GetName() const override;

    void UpdateControl( wxPGProperty* property, wxWindow* ctrl ) const override;

    bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
        wxWindow* ctrl, wxEvent& event ) const override;

    bool GetValueFromControl( wxVariant& variant,
                                      wxPGProperty* property,
                                      wxWindow* ctrl ) const override;

    void OnFocus( wxPGProperty* property, wxWindow* wnd ) const override;

};


class wxPGChoiceAndButtonEditor : public wxPGChoiceEditor
{
public:
    wxPGChoiceAndButtonEditor() = default;
    ~wxPGChoiceAndButtonEditor();
    wxString GetName() const override;

    wxPGWindowList CreateControls(wxPropertyGrid* propgrid,
                                          wxPGProperty* property,
                                          const wxPoint& pos,
                                          const wxSize& size) const override;

    wxDECLARE_DYNAMIC_CLASS(wxPGChoiceAndButtonEditor);
};

class wxPGTextCtrlAndButtonEditor : public wxPGTextCtrlEditor
{
public:
    wxPGTextCtrlAndButtonEditor() = default;
    ~wxPGTextCtrlAndButtonEditor();
    wxString GetName() const override;

    wxPGWindowList CreateControls(wxPropertyGrid* propgrid,
                                          wxPGProperty* property,
                                          const wxPoint& pos,
                                          const wxSize& size) const override;

    wxDECLARE_DYNAMIC_CLASS(wxPGTextCtrlAndButtonEditor);
};


#if wxPG_INCLUDE_CHECKBOX

//
// Use custom check box code instead of native control
// for cleaner (i.e. more integrated) look.
//
class wxPGCheckBoxEditor : public wxPGEditor
{
    wxDECLARE_DYNAMIC_CLASS(wxPGCheckBoxEditor);
public:
    wxPGCheckBoxEditor() = default;
    ~wxPGCheckBoxEditor();

    wxString GetName() const override;
    wxPGWindowList CreateControls(wxPropertyGrid* propgrid,
                                          wxPGProperty* property,
                                          const wxPoint& pos,
                                          const wxSize& size) const override;
    void UpdateControl( wxPGProperty* property,
                                wxWindow* ctrl ) const override;
    bool OnEvent( wxPropertyGrid* propgrid,
                          wxPGProperty* property,
                          wxWindow* primaryCtrl,
                          wxEvent& event ) const override;
    bool GetValueFromControl( wxVariant& variant,
                                      wxPGProperty* property,
                                      wxWindow* ctrl ) const override;
    void SetValueToUnspecified( wxPGProperty* property,
                                        wxWindow* ctrl ) const override;

    void DrawValue( wxDC& dc,
                            const wxRect& rect,
                            wxPGProperty* property,
                            const std::string& text ) const override;
    //virtual wxPGCellRenderer* GetCellRenderer() const;

    void SetControlIntValue( wxPGProperty* property,
                                     wxWindow* ctrl,
                                     int value ) const override;
};

#endif


// -----------------------------------------------------------------------
// Editor class registration macro (mostly for internal use)

#define wxPGRegisterEditorClass(EDITOR) \
    if ( wxPGEditor_##EDITOR == NULL ) \
    { \
        wxPGEditor_##EDITOR = wxPropertyGrid::RegisterEditorClass( \
                new wxPG##EDITOR##Editor ); \
    }

// -----------------------------------------------------------------------

// Derive a class from this to adapt an existing editor dialog or function to
// be used when editor button of a property is pushed.
// You only need to derive class and implement DoShowDialog() to create and
// show the dialog, and finally submit the value returned by the dialog
// via SetValue().
class wxPGEditorDialogAdapter : public wxObject
{
    wxDECLARE_ABSTRACT_CLASS(wxPGEditorDialogAdapter);
public:
    bool ShowDialog( wxPropertyGrid* propGrid, wxPGProperty* property );

    virtual bool DoShowDialog( wxPropertyGrid* propGrid,
                               wxPGProperty* property ) = 0;

    void SetValue( const wxVariant& value )
    {
        m_value = value;
    }

    // This method is typically only used if deriving class from existing
    // adapter with value conversion purposes.
    wxVariant& GetValue() { return m_value; }

    // This member is public so scripting language bindings
    // wrapper code can access it freely.
    void*               m_clientData{nullptr};

private:
    wxVariant           m_value;
};

// -----------------------------------------------------------------------


// This class can be used to have multiple buttons in a property editor.
// You will need to create a new property editor class, override
// CreateControls, and have it return wxPGMultiButton instance in
// wxPGWindowList::SetSecondary().
class wxPGMultiButton : public wxWindow
{
public:
    wxPGMultiButton( wxPropertyGrid* pg, const wxSize& sz );

    wxWindow* GetButton( unsigned int i ) { return m_buttons[i]; }
    const wxWindow* GetButton( unsigned int i ) const
        { return m_buttons[i]; }

    // Utility function to be used in event handlers.
    int GetButtonId( unsigned int i ) const { return GetButton(i)->GetId(); }

    // Returns number of buttons.
    unsigned int GetCount() const { return (unsigned int) m_buttons.size(); }

    void Add( const wxString& label, int id = -2 );
#if wxUSE_BMPBUTTON
    void Add( const wxBitmap& bitmap, int id = -2 );
#endif

    wxSize GetPrimarySize() const
    {
        return wxSize(m_fullEditorSize.x - m_buttonsWidth, m_fullEditorSize.y);
    }

    void Finalize( wxPropertyGrid* propGrid, const wxPoint& pos );

protected:

    void DoAddButton( wxWindow* button, const wxSize& sz );

    int GenId( int id ) const;

    std::vector<wxWindow*> m_buttons;
    wxSize          m_fullEditorSize;
    int             m_buttonsWidth;
};

// -----------------------------------------------------------------------

#endif // wxUSE_PROPGRID

#endif // _WX_PROPGRID_EDITORS_H_
