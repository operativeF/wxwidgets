/////////////////////////////////////////////////////////////////////////////
// Name:        wx/propgrid/advprops.h
// Purpose:     wxPropertyGrid Advanced Properties (font, colour, etc.)
// Author:      Jaakko Salli
// Modified by:
// Created:     2004-09-25
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROPGRID_ADVPROPS_H_
#define _WX_PROPGRID_ADVPROPS_H_

#if wxUSE_PROPGRID

#include "wx/propgrid/props.h"

// -----------------------------------------------------------------------

//
// Additional Value Type Handlers
//
bool operator==(const wxArrayInt& array1, const wxArrayInt& array2);

//
// Additional Property Editors
//
#if wxUSE_SPINBTN
WX_PG_DECLARE_EDITOR_WITH_DECL(SpinCtrl, )
#endif

#if wxUSE_DATEPICKCTRL
WX_PG_DECLARE_EDITOR_WITH_DECL(DatePickerCtrl, )
#endif

// -----------------------------------------------------------------------


// Web colour is currently unsupported
#define wxPG_COLOUR_WEB_BASE        0x10000
//#define wxPG_TO_WEB_COLOUR(A)   ((std::uint32_t)(A+wxPG_COLOUR_WEB_BASE))


#define wxPG_COLOUR_CUSTOM      0xFFFFFF
#define wxPG_COLOUR_UNSPECIFIED (wxPG_COLOUR_CUSTOM+1)

// Because text, background and other colours tend to differ between
// platforms, wxSystemColourProperty must be able to select between system
// colour and, when necessary, to pick a custom one. wxSystemColourProperty
// value makes this possible.
class wxColourPropertyValue : public wxObject
{
public:
    // An integer value relating to the colour, and which exact
    // meaning depends on the property with which it is used.
    // For wxSystemColourProperty:
    // Any of wxSYS_COLOUR_XXX, or any web-colour ( use wxPG_TO_WEB_COLOUR
    // macro - (currently unsupported) ), or wxPG_COLOUR_CUSTOM.
    //
    // For custom colour properties without values array specified:
    // index or wxPG_COLOUR_CUSTOM
    // For custom colour properties with values array specified:
    // m_arrValues[index] or wxPG_COLOUR_CUSTOM
    std::uint32_t    m_type;

    // Resulting colour. Should be correct regardless of type.
    wxColour    m_colour;

    wxColourPropertyValue()
         
    {
        m_type = 0;
    }

    ~wxColourPropertyValue()
    override = default;

    wxColourPropertyValue( const wxColourPropertyValue& v )
        : 
         m_colour(v.m_colour)
    {
        m_type = v.m_type;
    }

    void Init( std::uint32_t type, const wxColour& colour )
    {
        m_type = type;
        m_colour = colour;
    }

    wxColourPropertyValue( const wxColour& colour )
        : 
         m_colour(colour)
    {
        m_type = wxPG_COLOUR_CUSTOM;
    }

    wxColourPropertyValue( std::uint32_t type )
         
    {
        m_type = type;
    }

    wxColourPropertyValue( std::uint32_t type, const wxColour& colour )
         
    {
        Init( type, colour );
    }

    void operator=(const wxColourPropertyValue& cpv)
    {
        if (this != &cpv)
            Init( cpv.m_type, cpv.m_colour );
    }

private:
    wxDECLARE_DYNAMIC_CLASS(wxColourPropertyValue);
};


bool operator==(const wxColourPropertyValue&, const wxColourPropertyValue&);

DECLARE_VARIANT_OBJECT(wxColourPropertyValue)

// -----------------------------------------------------------------------

// Property representing wxFont.
class wxFontProperty : public wxEditorDialogProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxFontProperty)
public:

    wxFontProperty(const wxString& label = wxPG_LABEL,
                   const wxString& name = wxPG_LABEL,
                   const wxFont& value = wxFont());

    void OnSetValue() override;
    wxString ValueToString( wxVariant& value, int argFlags = 0 ) const override;
    wxVariant ChildChanged( wxVariant& thisValue,
                                    int childIndex,
                                    wxVariant& childValue ) const override;
    void RefreshChildren() override;

protected:
    bool DisplayEditorDialog(wxPropertyGrid* pg, wxVariant& value) override;
};

// -----------------------------------------------------------------------


// If set, then match from list is searched for a custom colour.
#define wxPG_PROP_TRANSLATE_CUSTOM      wxPG_PROP_CLASS_SPECIFIC_1


// Has dropdown list of wxWidgets system colours. Value used is
// of wxColourPropertyValue type.
class wxSystemColourProperty : public wxEnumProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxSystemColourProperty)
public:

    wxSystemColourProperty( const wxString& label = wxPG_LABEL,
                            const wxString& name = wxPG_LABEL,
                            const wxColourPropertyValue&
                                value = wxColourPropertyValue() );

    void OnSetValue() override;
    bool IntToValue(wxVariant& variant,
                            int number,
                            int argFlags = 0) const override;

    // Override in derived class to customize how colours are printed as
    // strings.
    virtual wxString ColourToString( const wxColour& col, int index,
                                     int argFlags = 0 ) const;

    // Returns index of entry that triggers colour picker dialog
    // (default is last).
    virtual int GetCustomColourIndex() const;

    wxString ValueToString( wxVariant& value, int argFlags = 0 ) const override;
    bool StringToValue( wxVariant& variant,
                                const wxString& text,
                                int argFlags = 0 ) const override;
    bool OnEvent( wxPropertyGrid* propgrid,
                          wxWindow* primary, wxEvent& event ) override;
    bool DoSetAttribute( const wxString& name, wxVariant& value ) override;
    wxSize OnMeasureImage( int item ) const override;
    void OnCustomPaint( wxDC& dc,
                                const wxRect& rect, wxPGPaintData& paintdata ) override;

    // Helper function to show the colour dialog
    bool QueryColourFromUser( wxVariant& variant ) const;

    // Default is to use wxSystemSettings::GetColour(index). Override to use
    // custom colour tables etc.
    virtual wxColour GetColour( int index ) const;

    wxColourPropertyValue GetVal( const wxVariant* pVariant = nullptr ) const;

protected:

    // Special constructors to be used by derived classes.
    wxSystemColourProperty( const wxString& label, const wxString& name,
        const char* const* labels, const long* values, wxPGChoices* choicesCache,
        const wxColourPropertyValue& value );
    wxSystemColourProperty( const wxString& label, const wxString& name,
        const char* const* labels, const long* values, wxPGChoices* choicesCache,
        const wxColour& value );

    void Init( int type, const wxColour& colour );

    // Utility functions for internal use
    virtual wxVariant DoTranslateVal( wxColourPropertyValue& v ) const;
    wxVariant TranslateVal( wxColourPropertyValue& v ) const
    {
        return DoTranslateVal( v );
    }
    wxVariant TranslateVal( int type, const wxColour& colour ) const
    {
        wxColourPropertyValue v(type, colour);
        return DoTranslateVal( v );
    }

    // Translates colour to a int value, return wxNOT_FOUND if no match.
    int ColToInd( const wxColour& colour ) const;
};

// -----------------------------------------------------------------------

class wxColourProperty : public wxSystemColourProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxColourProperty)
public:
    wxColourProperty( const wxString& label = wxPG_LABEL,
                      const wxString& name = wxPG_LABEL,
                      const wxColour& value = *wxWHITE );

    wxString ValueToString( wxVariant& value, int argFlags = 0 ) const override;
    wxColour GetColour( int index ) const override;

protected:
    wxVariant DoTranslateVal( wxColourPropertyValue& v ) const override;

private:
    void Init( wxColour colour );
};

// -----------------------------------------------------------------------

// Property representing wxCursor.
class wxCursorProperty : public wxEnumProperty
{
    wxDECLARE_DYNAMIC_CLASS(wxCursorProperty);

    wxCursorProperty( const wxString& label= wxPG_LABEL,
                      const wxString& name= wxPG_LABEL,
                      int value = 0 );

    wxSize OnMeasureImage( int item ) const override;
    void OnCustomPaint( wxDC& dc,
                                const wxRect& rect, wxPGPaintData& paintdata ) override;
};

// -----------------------------------------------------------------------

#if wxUSE_IMAGE

const wxString& wxPGGetDefaultImageWildcard();
class wxBitmap;
class wxImage;

// Property representing image file(name).
class wxImageFileProperty : public wxFileProperty
{
    wxDECLARE_DYNAMIC_CLASS(wxImageFileProperty);
public:

    wxImageFileProperty( const wxString& label= wxPG_LABEL,
                         const wxString& name = wxPG_LABEL,
                         const wxString& value = {});
    ~wxImageFileProperty();

    void OnSetValue() override;

    wxSize OnMeasureImage( int item ) const override;
    void OnCustomPaint( wxDC& dc,
                                const wxRect& rect, wxPGPaintData& paintdata ) override;

protected:
    wxBitmap*   m_pBitmap{nullptr}; // final thumbnail area
    wxImage*    m_pImage{nullptr}; // intermediate thumbnail area

private:
    // Initialize m_pImage using the current file name.
    void LoadImageFromFile();
};

#endif

#if wxUSE_CHOICEDLG

// Property that manages a value resulting from wxMultiChoiceDialog. Value is
// array of strings. You can get value as array of choice values/indices by
// calling wxMultiChoiceProperty::GetValueAsArrayInt().
class wxMultiChoiceProperty : public wxEditorDialogProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxMultiChoiceProperty)
public:

    wxMultiChoiceProperty( const wxString& label,
                           const wxString& name,
                           const std::vector<std::string>& strings,
                           const std::vector<wxString>& value );
    wxMultiChoiceProperty( const wxString& label,
                           const wxString& name,
                           const wxPGChoices& choices,
                           const std::vector<wxString>& value = {} );

    wxMultiChoiceProperty( const wxString& label = wxPG_LABEL,
                           const wxString& name = wxPG_LABEL,
                           const std::vector<wxString>& value = {} );

    void OnSetValue() override;
    wxString ValueToString( wxVariant& value, int argFlags = 0 ) const override;
    bool StringToValue(wxVariant& variant,
                               const wxString& text,
                               int argFlags = 0) const override;
    bool DoSetAttribute( const wxString& name, wxVariant& value ) override;

    std::vector<int> GetValueAsArrayInt() const
    {
        return m_choices.GetValuesForStrings(m_value.GetArrayString());
    }

protected:
    bool DisplayEditorDialog(wxPropertyGrid* pg, wxVariant& value) override;

    void GenerateValueAsString( wxVariant& value, wxString* target ) const;

    // Returns translation of values into string indices.
    std::vector<int> GetValueAsIndices() const;

    std::vector<wxString>       m_valueAsStrings;  // Value as array of strings

    // Cache displayed text since generating it is relatively complicated.
    wxString            m_display;
    // How to handle user strings
    int                 m_userStringMode{0};
};

#endif // wxUSE_CHOICEDLG

// -----------------------------------------------------------------------

#if wxUSE_DATETIME

// Property representing wxDateTime.
class wxDateProperty : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(wxDateProperty)
public:

    wxDateProperty( const wxString& label = wxPG_LABEL,
                    const wxString& name = wxPG_LABEL,
                    const wxDateTime& value = wxDateTime() );

    void OnSetValue() override;
    wxString ValueToString( wxVariant& value, int argFlags = 0 ) const override;
    bool StringToValue(wxVariant& variant,
                               const wxString& text,
                               int argFlags = 0) const override;

    bool DoSetAttribute( const wxString& name, wxVariant& value ) override;

    void SetFormat( const wxString& format )
    {
        m_format = format;
    }

    const wxString& GetFormat() const
    {
        return m_format;
    }

    void SetDateValue( const wxDateTime& dt )
    {
        //m_valueDateTime = dt;
        m_value = dt;
    }

    wxDateTime GetDateValue() const
    {
        //return m_valueDateTime;
        return m_value;
    }

    long GetDatePickerStyle() const
    {
        return m_dpStyle;
    }

protected:
    wxString        m_format;
    long            m_dpStyle;  // DatePicker style

    inline static wxString ms_defaultDateFormat{};
    static wxString DetermineDefaultDateFormat( bool showCentury );
};

#endif // wxUSE_DATETIME

// -----------------------------------------------------------------------

#if wxUSE_SPINBTN

//
// Implement an editor control that allows using wxSpinCtrl (actually, a
// combination of wxTextCtrl and wxSpinButton) to edit value of wxIntProperty
// and wxFloatProperty (and similar).
//
// Note that new editor classes needs to be registered before use. This can be
// accomplished using wxPGRegisterEditorClass macro, which is used for SpinCtrl
// in wxPropertyGridInterface::RegisterAdditionalEditors (see below).
// Registration can also be performed in a constructor of a property that is
// likely to require the editor in question.
//


#include "wx/spinbutt.h"
#include "wx/propgrid/editors.h"


// NOTE: Regardless that this class inherits from a working editor, it has
//   all necessary methods to work independently. wxTextCtrl stuff is only
//   used for event handling here.
class wxPGSpinCtrlEditor : public wxPGTextCtrlEditor
{
    wxDECLARE_DYNAMIC_CLASS(wxPGSpinCtrlEditor);
public:
    ~wxPGSpinCtrlEditor();

    wxString GetName() const override;
    wxPGWindowList CreateControls(wxPropertyGrid* propgrid,
                                          wxPGProperty* property,
                                          const wxPoint& pos,
                                          const wxSize& size) const override;
    bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
        wxWindow* wnd, wxEvent& event ) const override;

private:
    mutable wxString m_tempString;
};

#endif // wxUSE_SPINBTN

// -----------------------------------------------------------------------

#endif // wxUSE_PROPGRID

#endif // _WX_PROPGRID_ADVPROPS_H_
