/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/grideditors.h
// Purpose:     wxGridCellEditorEvtHandler and wxGrid editors
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: Santiago Palacios
// Created:     1/08/1999
// Copyright:   (c) Michael Bedward
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/event.h"

#include <fmt/core.h>

#include <memory>

export module WX.Grid.Editors;

import WX.Generic.Grid;

import <vector>;

export
{

class wxGridCellEditorEvtHandler : public wxEvtHandler
{
public:
    wxGridCellEditorEvtHandler(wxGrid* grid, wxGridCellEditor* editor)
        : m_grid(grid),
          m_editor(editor),
          m_inSetFocus(false)
    {
    }

    wxGridCellEditorEvtHandler& operator=(wxGridCellEditorEvtHandler&&) = delete;

    void DismissEditor();

    void OnKillFocus(wxFocusEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnChar(wxKeyEvent& event);

    void SetInSetFocus(bool inSetFocus) { m_inSetFocus = inSetFocus; }

private:
    wxGrid             *m_grid;
    wxGridCellEditor   *m_editor;

    // Work around the fact that a focus kill event can be sent to
    // a combobox within a set focus event.
    bool                m_inSetFocus;

    wxDECLARE_EVENT_TABLE();
};

#if wxUSE_TEXTCTRL

// the editor for string/text data
class wxGridCellTextEditor : public wxGridCellEditor
{
public:
    explicit wxGridCellTextEditor(size_t maxChars = 0);

    void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler) override;
    void SetSize(const wxRect& rect) override;

    bool IsAcceptedKey(wxKeyEvent& event) override;
    void BeginEdit(int row, int col, wxGrid* grid) override;
    bool EndEdit(int row, int col, const wxGrid* grid,
                         const std::string& oldval, std::string *newval) override;
    void ApplyEdit(int row, int col, wxGrid* grid) override;

    void Reset() override;
    void StartingKey(wxKeyEvent& event) override;
    void HandleReturn(wxKeyEvent& event) override;

    // parameters string format is "max_width"
    void SetParameters(const std::string& params) override;
#if wxUSE_VALIDATORS
    virtual void SetValidator(const wxValidator& validator);
#endif

    wxGridCellEditor *Clone() const override;

    // added GetValue so we can get the value which is in the control
    std::string GetValue() const override;

protected:
    wxTextCtrl *Text() const { return (wxTextCtrl *)m_control; }

    // parts of our virtual functions reused by the derived classes
    void DoCreate(wxWindow* parent, wxWindowID id, wxEvtHandler* evtHandler,
                  unsigned int style = 0);
    void DoBeginEdit(const std::string& startValue);
    void DoReset(const std::string& startValue);

private:
    size_t                   m_maxChars;        // max number of chars allowed
#if wxUSE_VALIDATORS
    std::unique_ptr<wxValidator> m_validator;
#endif
    std::string                 m_value;
};

// the editor for numeric (long) data
class wxGridCellNumberEditor : public wxGridCellTextEditor
{
public:
    // allows to specify the range - if min == max == -1, no range checking is
    // done
    wxGridCellNumberEditor(int min = -1, int max = -1);

    void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler) override;

    void SetSize(const wxRect& rect) override;

    bool IsAcceptedKey(wxKeyEvent& event) override;
    void BeginEdit(int row, int col, wxGrid* grid) override;
    bool EndEdit(int row, int col, const wxGrid* grid,
                         const std::string& oldval, std::string *newval) override;
    void ApplyEdit(int row, int col, wxGrid* grid) override;

    void Reset() override;
    void StartingKey(wxKeyEvent& event) override;

    // parameters string format is "min,max"
    void SetParameters(const std::string& params) override;

    wxGridCellEditor *Clone() const override
        { return new wxGridCellNumberEditor(m_min, m_max); }

    // added GetValue so we can get the value which is in the control
    std::string GetValue() const override;

protected:
#if wxUSE_SPINCTRL
    wxSpinCtrl *Spin() const { return (wxSpinCtrl *)m_control; }
#endif

    // if HasRange(), we use wxSpinCtrl - otherwise wxTextCtrl
    bool HasRange() const
    {
#if wxUSE_SPINCTRL
        return m_min != m_max;
#else
        return false;
#endif
    }

    // string representation of our value
    std::string GetString() const
        { return fmt::format("{:ld}", m_value); }

private:
    int m_min,
        m_max;

    long m_value;

    wxGridCellNumberEditor(const wxGridCellNumberEditor&) = delete;
	wxGridCellNumberEditor& operator=(const wxGridCellNumberEditor&) = delete;
};


enum wxGridCellFloatFormat
{
    // Decimal floating point (%f)
    wxGRID_FLOAT_FORMAT_FIXED      = 0x0010,

    // Scientific notation (mantise/exponent) using e character (%e)
    wxGRID_FLOAT_FORMAT_SCIENTIFIC = 0x0020,

    // Use the shorter of %e or %f (%g)
    wxGRID_FLOAT_FORMAT_COMPACT    = 0x0040,

    // To use in combination with one of the above formats (%F/%E/%G)
    wxGRID_FLOAT_FORMAT_UPPER      = 0x0080,

    // Format used by default.
    wxGRID_FLOAT_FORMAT_DEFAULT    = wxGRID_FLOAT_FORMAT_FIXED,

    // A mask to extract format from the combination of flags.
    wxGRID_FLOAT_FORMAT_MASK       = wxGRID_FLOAT_FORMAT_FIXED |
                                     wxGRID_FLOAT_FORMAT_SCIENTIFIC |
                                     wxGRID_FLOAT_FORMAT_COMPACT |
                                     wxGRID_FLOAT_FORMAT_UPPER
};

// the editor for floating point numbers (double) data
class wxGridCellFloatEditor : public wxGridCellTextEditor
{
public:
    wxGridCellFloatEditor(int width = -1,
                          int precision = -1,
                          unsigned int format = wxGRID_FLOAT_FORMAT_DEFAULT);

    void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler) override;

    bool IsAcceptedKey(wxKeyEvent& event) override;
    void BeginEdit(int row, int col, wxGrid* grid) override;
    bool EndEdit(int row, int col, const wxGrid* grid,
                         const std::string& oldval, std::string *newval) override;
    void ApplyEdit(int row, int col, wxGrid* grid) override;

    void Reset() override;
    void StartingKey(wxKeyEvent& event) override;

    wxGridCellEditor *Clone() const override
        { return new wxGridCellFloatEditor(m_width, m_precision); }

    // parameters string format is "width[,precision[,format]]"
    // format to choose between f|e|g|E|G (f is used by default)
    void SetParameters(const std::string& params) override;

protected:
    // string representation of our value
    std::string GetString();

private:
    int m_width,
        m_precision;
    double m_value;

    unsigned int m_style;
    std::string m_format;

    wxGridCellFloatEditor(const wxGridCellFloatEditor&) = delete;
	wxGridCellFloatEditor& operator=(const wxGridCellFloatEditor&) = delete;
};

#endif // wxUSE_TEXTCTRL

#if wxUSE_CHECKBOX

// the editor for boolean data
class wxGridCellBoolEditor : public wxGridCellEditor
{
public:
	wxGridCellBoolEditor& operator=(wxGridCellBoolEditor&&) = delete;

    wxGridActivationResult
    TryActivate(int row, int col, wxGrid* grid,
                const wxGridActivationSource& actSource) override;
    void DoActivate(int row, int col, wxGrid* grid) override;

    void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler) override;

    void SetSize(const wxRect& rect) override;
    void Show(bool show, wxGridCellAttr *attr = nullptr) override;

    bool IsAcceptedKey(wxKeyEvent& event) override;
    void BeginEdit(int row, int col, wxGrid* grid) override;
    bool EndEdit(int row, int col, const wxGrid* grid,
                         const std::string& oldval, std::string *newval) override;
    void ApplyEdit(int row, int col, wxGrid* grid) override;

    void Reset() override;
    void StartingClick() override;
    void StartingKey(wxKeyEvent& event) override;

    wxGridCellEditor *Clone() const override
        { return new wxGridCellBoolEditor; }

    // added GetValue so we can get the value which is in the control, see
    // also UseStringValues()
    std::string GetValue() const override;

    // set the string values returned by GetValue() for the true and false
    // states, respectively
    static void UseStringValues(const std::string& valueTrue = "1",
                                const std::string& valueFalse = {});

    // return true if the given string is equal to the string representation of
    // true value which we currently use
    static bool IsTrueValue(const std::string& value);

protected:
    wxCheckBox *CBox() const { return (wxCheckBox *)m_control; }

private:
    // These functions modify or use m_value.
    void SetValueFromGrid(int row, int col, wxGrid* grid);
    void SetGridFromValue(int row, int col, wxGrid* grid) const;

    std::string GetStringValue() const { return GetStringValue(m_value); }

    static
    std::string GetStringValue(bool value) { return ms_stringValues[value]; }

    bool m_value{};

    static std::string ms_stringValues[2];
};

#endif // wxUSE_CHECKBOX

#if wxUSE_COMBOBOX

// the editor for string data allowing to choose from the list of strings
class wxGridCellChoiceEditor : public wxGridCellEditor
{
public:
    // if !allowOthers, user can't type a string not in choices array
    wxGridCellChoiceEditor(const std::vector<std::string>& choices = {},
                           bool allowOthers = false);

	wxGridCellChoiceEditor& operator=(wxGridCellChoiceEditor&&) = delete;

    void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler) override;

    void SetSize(const wxRect& rect) override;

    void BeginEdit(int row, int col, wxGrid* grid) override;
    bool EndEdit(int row, int col, const wxGrid* grid,
                         const std::string& oldval, std::string *newval) override;
    void ApplyEdit(int row, int col, wxGrid* grid) override;

    void Reset() override;

    // parameters string format is "item1[,item2[...,itemN]]"
    void SetParameters(const std::string& params) override;

    wxGridCellEditor *Clone() const override;

    // added GetValue so we can get the value which is in the control
    std::string GetValue() const override;

protected:
    wxComboBox *Combo() const { return (wxComboBox *)m_control; }

    void OnComboCloseUp(wxCommandEvent& evt);

    std::string             m_value;
    std::vector<std::string>   m_choices;
    bool                    m_allowOthers;
};

#endif // wxUSE_COMBOBOX

#if wxUSE_COMBOBOX

class wxGridCellEnumEditor : public wxGridCellChoiceEditor
{
public:
    wxGridCellEnumEditor( const std::string& choices = {} );

	wxGridCellEnumEditor& operator=(wxGridCellEnumEditor&&) = delete;

    wxGridCellEditor*  Clone() const override;

    void BeginEdit(int row, int col, wxGrid* grid) override;
    bool EndEdit(int row, int col, const wxGrid* grid,
                         const std::string& oldval, std::string *newval) override;
    void ApplyEdit(int row, int col, wxGrid* grid) override;

private:
    long m_index{-1};
};

#endif // wxUSE_COMBOBOX

class wxGridCellAutoWrapStringEditor : public wxGridCellTextEditor
{
public:
    void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler) override;

	wxGridCellAutoWrapStringEditor& operator=(wxGridCellAutoWrapStringEditor&&) = delete;

    wxGridCellEditor *Clone() const override
        { return new wxGridCellAutoWrapStringEditor; }
};

#if wxUSE_DATEPICKCTRL

class wxGridCellDateEditor : public wxGridCellEditor
{
public:
    explicit wxGridCellDateEditor(const std::string& format = {});

	wxGridCellDateEditor& operator=(wxGridCellDateEditor&&) = delete;

    void SetParameters(const std::string& params) override;

    void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler) override;

    void SetSize(const wxRect& rect) override;

    void BeginEdit(int row, int col, wxGrid* grid) override;
    bool EndEdit(int row, int col, const wxGrid* grid,
                         const std::string& oldval, std::string *newval) override;
    void ApplyEdit(int row, int col, wxGrid* grid) override;

    void Reset() override;

    wxGridCellEditor *Clone() const override;

    std::string GetValue() const override;

protected:
    wxDatePickerCtrl* DatePicker() const;

private:
    wxDateTime m_value;
    std::string m_format;
};

#endif // wxUSE_DATEPICKCTRL

} // export
