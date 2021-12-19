/////////////////////////////////////////////////////////////////////////////
// Name:        wx/propgrid/propgriddefs.h
// Purpose:     wxPropertyGrid miscellaneous definitions
// Author:      Jaakko Salli
// Modified by:
// Created:     2008-08-31
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROPGRID_PROPGRIDDEFS_H_
#define _WX_PROPGRID_PROPGRIDDEFS_H_

#if wxUSE_PROPGRID

#include "wx/colour.h"
#include "wx/hashset.h"

class  wxFont;

import Utils.Geometry;
import Utils.Strings;

import <numeric>;

// -----------------------------------------------------------------------

//
// Here are some platform dependent defines
// NOTE: More in propertygrid.cpp
//
// NB: Only define wxPG_TEXTCTRLXADJUST for platforms that do not
//     (yet) support wxTextEntry::SetMargins() for the left margin.

#if defined(__WXMSW__)

    // space between vertical line and value text
    #define wxPG_XBEFORETEXT            4
    // space between vertical line and value editor control
    #define wxPG_XBEFOREWIDGET          1

    // left margin can be set with wxTextEntry::SetMargins()
    #undef wxPG_TEXTCTRLXADJUST

    // comment to use bitmap buttons
    #define wxPG_ICON_WIDTH             9
    // 1 if wxRendererNative should be employed
    #define wxPG_USE_RENDERER_NATIVE    1

    // width of optional bitmap/image in front of property
    #define wxPG_CUSTOM_IMAGE_WIDTH     20

    // 1 if splitter drag detect margin and control cannot overlap
    #define wxPG_NO_CHILD_EVT_MOTION    0

    #define wxPG_NAT_BUTTON_BORDER_ANY          1
    #define wxPG_NAT_BUTTON_BORDER_X            1
    #define wxPG_NAT_BUTTON_BORDER_Y            1

    // If 1 then controls are refreshed explicitly in a few places
    #define wxPG_REFRESH_CONTROLS 0

#elif defined(__WXGTK__)

    // space between vertical line and value text
    #define wxPG_XBEFORETEXT            5
    // space between vertical line and value editor control
    #define wxPG_XBEFOREWIDGET          1

    // x position adjustment for wxTextCtrl (and like)
    // left margin can be set with wxTextEntry::SetMargins()
    #undef wxPG_TEXTCTRLXADJUST

    // comment to use bitmap buttons
    #define wxPG_ICON_WIDTH             9
    // 1 if wxRendererNative should be employed
    #define wxPG_USE_RENDERER_NATIVE    1

    // width of optional bitmap/image in front of property
    #define wxPG_CUSTOM_IMAGE_WIDTH     20

    // 1 if splitter drag detect margin and control cannot overlap
    #define wxPG_NO_CHILD_EVT_MOTION    1

    #define wxPG_NAT_BUTTON_BORDER_ANY      1
    #define wxPG_NAT_BUTTON_BORDER_X        1
    #define wxPG_NAT_BUTTON_BORDER_Y        1

    // If 1 then controls are refreshed after selected was drawn.
    #define wxPG_REFRESH_CONTROLS 1

#elif defined(__WXMAC__)

    // space between vertical line and value text
    #define wxPG_XBEFORETEXT            4
    // space between vertical line and value editor widget
    #define wxPG_XBEFOREWIDGET          1

    // x position adjustment for wxTextCtrl (and like)
    // left margin cannot be set with wxTextEntry::SetMargins()
    #define wxPG_TEXTCTRLXADJUST        1

    // comment to use bitmap buttons
    #define wxPG_ICON_WIDTH             11
    // 1 if wxRendererNative should be employed
    #define wxPG_USE_RENDERER_NATIVE    1

    // width of optional bitmap/image in front of property
    #define wxPG_CUSTOM_IMAGE_WIDTH     20

    // 1 if splitter drag detect margin and control cannot overlap
    #define wxPG_NO_CHILD_EVT_MOTION    0

    #define wxPG_NAT_BUTTON_BORDER_ANY      0
    #define wxPG_NAT_BUTTON_BORDER_X        0
    #define wxPG_NAT_BUTTON_BORDER_Y        0

    // If 1 then controls are refreshed after selected was drawn.
    #define wxPG_REFRESH_CONTROLS 0

#else // defaults

    // space between vertical line and value text
    #define wxPG_XBEFORETEXT            5
    // space between vertical line and value editor widget
    #define wxPG_XBEFOREWIDGET          1

    // x position adjustment for wxTextCtrl (and like)
    // left margin cannot be set with wxTextEntry::SetMargins()
    #define wxPG_TEXTCTRLXADJUST        3

    // comment to use bitmap buttons
    #define wxPG_ICON_WIDTH             9
    // 1 if wxRendererNative should be employed
    #define wxPG_USE_RENDERER_NATIVE    0

    // width of optional bitmap/image in front of property
    #define wxPG_CUSTOM_IMAGE_WIDTH     20

    // 1 if splitter drag detect margin and control cannot overlap
    #define wxPG_NO_CHILD_EVT_MOTION    1

    #define wxPG_NAT_BUTTON_BORDER_ANY      0
    #define wxPG_NAT_BUTTON_BORDER_X        0
    #define wxPG_NAT_BUTTON_BORDER_Y        0

    // If 1 then controls are refreshed after selected was drawn.
    #define wxPG_REFRESH_CONTROLS 0
#endif // platform


#define wxPG_CONTROL_MARGIN             0 // space between splitter and control

#define wxCC_CUSTOM_IMAGE_MARGIN1       4  // before image
#define wxCC_CUSTOM_IMAGE_MARGIN2       5  // after image

#define DEFAULT_IMAGE_OFFSET_INCREMENT \
    (wxCC_CUSTOM_IMAGE_MARGIN1 + wxCC_CUSTOM_IMAGE_MARGIN2)

#define wxPG_DRAG_MARGIN                30

#if wxPG_NO_CHILD_EVT_MOTION
    #define wxPG_SPLITTERX_DETECTMARGIN1    3 // this much on left
    #define wxPG_SPLITTERX_DETECTMARGIN2    2 // this much on right
#else
    #define wxPG_SPLITTERX_DETECTMARGIN1    3 // this much on left
    #define wxPG_SPLITTERX_DETECTMARGIN2    2 // this much on right
#endif

// Use this macro to generate standard custom image height from
#define wxPG_STD_CUST_IMAGE_HEIGHT(LINEHEIGHT)  ((LINEHEIGHT)-3)

// Undefine wxPG_ICON_WIDTH to use supplied xpm bitmaps instead
// (for tree buttons)
//#undef wxPG_ICON_WIDTH

// Set 1 to include advanced properties (wxFontProperty, wxColourProperty, etc.)
#ifndef wxPG_INCLUDE_ADVPROPS
    #define wxPG_INCLUDE_ADVPROPS           1
#endif

// Set 1 to include checkbox editor class
#define wxPG_INCLUDE_CHECKBOX           1

// -----------------------------------------------------------------------


class wxPGEditor;
class wxPGProperty;
class wxPropertyCategory;
class wxPGChoices;
class wxPropertyGridPageState;
class wxPGCell;
class wxPGCellRenderer;
class wxPGChoiceEntry;
class wxPGPropArgCls;
class wxPropertyGridInterface;
class wxPropertyGrid;
class wxPropertyGridEvent;
class wxPropertyGridManager;
class wxPGEditorDialogAdapter;
class wxPGValidationInfo;


// -----------------------------------------------------------------------

// Some miscellaneous values, types and macros.

// Used to tell wxPGProperty to use label as name as well
#define wxPG_LABEL              (*wxPGProperty::sm_wxPG_LABEL)

// This is the value placed in wxPGProperty::sm_wxPG_LABEL
#define wxPG_LABEL_STRING       "@!"

#define wxPG_COLOUR_BLACK       (*wxBLACK)

// Convert Red, Green and Blue to a single 32-bit value.
#define wxPG_COLOUR(R,G,B) ((std::uint32_t)((R)+((G)<<8)+((B)<<16)))


// If property is supposed to have custom-painted image, then returning
// this in OnMeasureImage() will usually be enough.
#define wxPG_DEFAULT_IMAGE_SIZE  wxDefaultSize


// This callback function is used for sorting properties.
// Call wxPropertyGrid::SetSortFunction() to set it.
// Sort function should return a value greater than 0 if position of p1 is
// after p2. So, for instance, when comparing property names, you can use
// following implementation:
//   int MyPropertySortFunction(wxPropertyGrid* propGrid,
//                              wxPGProperty* p1,
//                              wxPGProperty* p2)
//   {
//      return p1->GetBaseName().compare( p2->GetBaseName() );
//   }
typedef int (*wxPGSortCallback)(wxPropertyGrid* propGrid,
                                wxPGProperty* p1,
                                wxPGProperty* p2);

// -----------------------------------------------------------------------

// Used to indicate wxPGChoices::Add etc. that the value is actually not given
// by the caller.
#define wxPG_INVALID_VALUE      INT_MAX

// -----------------------------------------------------------------------

WX_DEFINE_TYPEARRAY_WITH_DECL_PTR(wxPGProperty*, wxArrayPGProperty,
                                  wxBaseArrayPtrVoid,
                                  class);

WX_DECLARE_STRING_HASH_MAP_WITH_DECL(void*,
                                     wxPGHashMapS2P,
                                     class);

WX_DECLARE_STRING_HASH_MAP_WITH_DECL(wxString,
                                     wxPGHashMapS2S,
                                     class);

using wxPGHashMapP2P = std::unordered_map< void*, void*, wxPointerHash, wxPointerEqual >;

using wxPGHashMapI2I = std::unordered_map< std::int32_t, std::int32_t, wxIntegerHash, wxIntegerEqual >;

class wxPGHashSetInt : public std::unordered_set<int, wxIntegerHash, wxIntegerEqual> {
public:
    explicit wxPGHashSetInt(size_type n = 3, const hasher& h = hasher(), const key_equal& ke = key_equal(),
        const allocator_type& a = allocator_type())
        : std::unordered_set<int, wxIntegerHash, wxIntegerEqual>(n, h, ke, a) {}

    template <class InputIterator>
    wxPGHashSetInt(InputIterator f, InputIterator l, const hasher& h = hasher(), const key_equal& ke = key_equal(),
        const allocator_type& a = allocator_type())
        : std::unordered_set<int, wxIntegerHash, wxIntegerEqual>(f, l, h, ke, a) {}

    wxPGHashSetInt(const std::unordered_set<int, wxIntegerHash, wxIntegerEqual>& s)
        : std::unordered_set<int, wxIntegerHash, wxIntegerEqual>(s) {}
};

// -----------------------------------------------------------------------

enum wxPG_PROPERTYVALUES_FLAGS {
    // Flag for wxPropertyGridInterface::SetProperty* functions,
    // wxPropertyGridInterface::HideProperty(), etc.
    // Apply changes only for the property in question.
    wxPG_DONT_RECURSE = 0x00000000,

    // Flag for wxPropertyGridInterface::GetPropertyValues().
    // Use this flag to retain category structure; each sub-category
    // will be its own wxVariantList of wxVariant.
    wxPG_KEEP_STRUCTURE = 0x00000010,

    // Flag for wxPropertyGridInterface::SetProperty* functions,
    // wxPropertyGridInterface::HideProperty(), etc.
    // Apply changes recursively for the property and all its children.
    wxPG_RECURSE = 0x00000020,

    // Flag for wxPropertyGridInterface::GetPropertyValues().
    // Use this flag to include property attributes as well.
    wxPG_INC_ATTRIBUTES = 0x00000040,

    // Used when first starting recursion.
    wxPG_RECURSE_STARTS = 0x00000080,

    // Force value change.
    wxPG_FORCE = 0x00000100,

    // Only sort categories and their immediate children.
    // Sorting done by wxPG_AUTO_SORT option uses this.
    wxPG_SORT_TOP_LEVEL_ONLY = 0x00000200
};

// -----------------------------------------------------------------------

// Misc. argument flags.
enum wxPG_MISC_ARG_FLAGS
{
    // Get/Store full value instead of displayed value.
    wxPG_FULL_VALUE                     = 0x00000001,

    // Perform special action in case of unsuccessful conversion.
    wxPG_REPORT_ERROR                   = 0x00000002,

    wxPG_PROPERTY_SPECIFIC              = 0x00000004,

    // Get/Store editable value instead of displayed one (should only be
    // different in the case of common values)
    wxPG_EDITABLE_VALUE                 = 0x00000008,

    // Used when dealing with fragments of composite string value
    wxPG_COMPOSITE_FRAGMENT             = 0x00000010,

    // Means property for which final string value is for cannot really be
    // edited.
    wxPG_UNEDITABLE_COMPOSITE_FRAGMENT  = 0x00000020,

    // ValueToString() called from GetValueAsString()
    // (guarantees that input wxVariant value is current own value)
    wxPG_VALUE_IS_CURRENT               = 0x00000040,

    // Value is being set programmatically (i.e. not by user)
    wxPG_PROGRAMMATIC_VALUE             = 0x00000080
};

// -----------------------------------------------------------------------

// wxPGProperty::SetValue() flags
enum wxPG_SETVALUE_FLAGS
{
    wxPG_SETVAL_REFRESH_EDITOR      = 0x0001,
    wxPG_SETVAL_AGGREGATED          = 0x0002,
    wxPG_SETVAL_FROM_PARENT         = 0x0004,
    wxPG_SETVAL_BY_USER             = 0x0008  // Set if value changed by user
};

// -----------------------------------------------------------------------

//
// Valid constants for wxPG_UINT_BASE attribute
// (long because of wxVariant constructor)
#define wxPG_BASE_OCT                       8L
#define wxPG_BASE_DEC                       10L
#define wxPG_BASE_HEX                       16L
#define wxPG_BASE_HEXL                      32L

//
// Valid constants for wxPG_UINT_PREFIX attribute
#define wxPG_PREFIX_NONE                    0L
#define wxPG_PREFIX_0x                      1L
#define wxPG_PREFIX_DOLLAR_SIGN             2L

// -----------------------------------------------------------------------
// Editor class.

// Editor accessor (for backwards compatibility use only).
#define wxPG_EDITOR(T)          wxPGEditor_##T

// Macro for declaring editor class, with optional impexpdecl part.
#ifndef WX_PG_DECLARE_EDITOR_WITH_DECL

    #define WX_PG_DECLARE_EDITOR_WITH_DECL(EDITOR) \
    extern wxPGEditor* wxPGEditor_##EDITOR; \
    extern wxPGEditor* wxPGConstruct##EDITOR##EditorClass();

#endif

// Declare editor class.
#define WX_PG_DECLARE_EDITOR(EDITOR) \
extern wxPGEditor* wxPGEditor_##EDITOR; \
extern wxPGEditor* wxPGConstruct##EDITOR##EditorClass();

// Declare built-in editor classes.
WX_PG_DECLARE_EDITOR_WITH_DECL(TextCtrl)
WX_PG_DECLARE_EDITOR_WITH_DECL(Choice)
WX_PG_DECLARE_EDITOR_WITH_DECL(ComboBox)
WX_PG_DECLARE_EDITOR_WITH_DECL(TextCtrlAndButton)
#if wxPG_INCLUDE_CHECKBOX
WX_PG_DECLARE_EDITOR_WITH_DECL(CheckBox)
#endif
WX_PG_DECLARE_EDITOR_WITH_DECL(ChoiceAndButton)

// -----------------------------------------------------------------------

#ifndef SWIG

//
// Macro WXVARIANT allows creation of wxVariant from any type supported by
// wxWidgets internally, and of all types created using
// WX_PG_DECLARE_VARIANT_DATA.
template<class T>
wxVariant WXVARIANT( [[maybe_unused]] const T& value )
{
    wxFAIL_MSG("Code should always call specializations of this template");
    return wxVariant();
}

template<> inline wxVariant WXVARIANT( const int& value )
    { return wxVariant((long)value); }
template<> inline wxVariant WXVARIANT( const long& value )
    { return wxVariant(value); }
template<> inline wxVariant WXVARIANT( const bool& value )
    { return wxVariant(value); }
template<> inline wxVariant WXVARIANT( const double& value )
    { return wxVariant(value); }
template<> inline wxVariant WXVARIANT( const std::vector<wxString>& value )
    { return wxVariant(value); }
template<> inline wxVariant WXVARIANT( const wxString& value )
    { return wxVariant(value); }
#if wxUSE_LONGLONG
template<> inline wxVariant WXVARIANT( const wxLongLong& value )
    { return wxVariant(value); }
template<> inline wxVariant WXVARIANT( const wxULongLong& value )
    { return wxVariant(value); }
#endif
#if wxUSE_DATETIME
template<> inline wxVariant WXVARIANT( const wxDateTime& value )
    { return wxVariant(value); }
#endif


//
// These are modified versions of DECLARE/WX_PG_IMPLEMENT_VARIANT_DATA
// macros found in variant.h. Differences are as follows:
//   * These support non-wxObject data
//   * These implement classname##RefFromVariant function which returns
//     reference to data within.
//   * const char* classname##_VariantType which equals classname.
//   * WXVARIANT
//
#define WX_PG_DECLARE_VARIANT_DATA(classname) \
    WX_PG_DECLARE_VARIANT_DATA_EXPORTED(classname, wxEMPTY_PARAMETER_VALUE)

#define WX_PG_DECLARE_VARIANT_DATA_EXPORTED(classname) \
classname& operator << ( classname &object, const wxVariant &variant ); \
wxVariant& operator << ( wxVariant &variant, const classname &object ); \
const classname& classname##RefFromVariant( const wxVariant& variant ); \
classname& classname##RefFromVariant( wxVariant& variant ); \
template<> inline wxVariant WXVARIANT( const classname& value ) \
{ \
    wxVariant variant; \
    variant << value; \
    return variant; \
} \
extern const char* classname##_VariantType;

// Add getter (i.e. classname << variant) separately to allow
// custom implementations.
#define WX_PG_IMPLEMENT_VARIANT_DATA_EXPORTED_NO_EQ_NO_GETTER(classname,expdecl) \
const char* classname##_VariantType = #classname; \
class classname##VariantData: public wxVariantData \
{ \
public:\
    classname##VariantData() {} \
    classname##VariantData( const classname &value ) : m_value(value) { } \
\
    classname &GetValue() { return m_value; } \
\
    const classname &GetValue() const { return m_value; } \
\
    bool Eq(wxVariantData& data) const override; \
\
    wxString GetType() const override; \
\
    wxVariantData* Clone() const override { return new classname##VariantData(m_value); } \
\
    DECLARE_WXANY_CONVERSION() \
protected:\
    classname m_value; \
};\
\
IMPLEMENT_TRIVIAL_WXANY_CONVERSION(classname, classname##VariantData) \
\
wxString classname##VariantData::GetType() const\
{\
    return wxS(#classname);\
}\
\
expdecl wxVariant& operator << ( wxVariant &variant, const classname &value )\
{\
    classname##VariantData *data = new classname##VariantData( value );\
    variant.SetData( data );\
    return variant;\
} \
expdecl classname& classname##RefFromVariant( wxVariant& variant ) \
{ \
    wxASSERT_MSG( variant.GetType() == wxS(#classname), \
                  wxString::Format("Variant type should have been '%s'" \
                                   "instead of '%s'", \
                                   wxS(#classname), \
                                   variant.GetType())); \
    classname##VariantData *data = \
        (classname##VariantData*) variant.GetData(); \
    return data->GetValue();\
} \
expdecl const classname& classname##RefFromVariant( const wxVariant& variant ) \
{ \
    wxASSERT_MSG( variant.GetType() == wxS(#classname), \
                  wxString::Format("Variant type should have been '%s'" \
                                   "instead of '%s'", \
                                   wxS(#classname), \
                                   variant.GetType())); \
    classname##VariantData *data = \
        (classname##VariantData*) variant.GetData(); \
    return data->GetValue();\
}

#define WX_PG_IMPLEMENT_VARIANT_DATA_GETTER(classname, expdecl) \
expdecl classname& operator << ( classname &value, const wxVariant &variant )\
{\
    wxASSERT( variant.GetType() == #classname );\
    \
    classname##VariantData *data = (classname##VariantData*) variant.GetData();\
    value = data->GetValue();\
    return value;\
}

#define WX_PG_IMPLEMENT_VARIANT_DATA_EQ(classname, expdecl) \
bool classname##VariantData::Eq(wxVariantData& data) const \
{\
    wxASSERT( GetType() == data.GetType() );\
\
    classname##VariantData & otherData = (classname##VariantData &) data;\
\
    return otherData.m_value == m_value;\
}

// implements a wxVariantData-derived class using for the Eq() method the
// operator== which must have been provided by "classname"
#define WX_PG_IMPLEMENT_VARIANT_DATA_EXPORTED(classname,expdecl) \
WX_PG_IMPLEMENT_VARIANT_DATA_EXPORTED_NO_EQ_NO_GETTER(classname,wxEMPTY_PARAMETER_VALUE expdecl) \
WX_PG_IMPLEMENT_VARIANT_DATA_GETTER(classname,wxEMPTY_PARAMETER_VALUE expdecl) \
WX_PG_IMPLEMENT_VARIANT_DATA_EQ(classname,wxEMPTY_PARAMETER_VALUE expdecl)

#define WX_PG_IMPLEMENT_VARIANT_DATA(classname) \
WX_PG_IMPLEMENT_VARIANT_DATA_EXPORTED(classname, wxEMPTY_PARAMETER_VALUE)

// with Eq() implementation that always returns false
#define WX_PG_IMPLEMENT_VARIANT_DATA_EXPORTED_DUMMY_EQ(classname,expdecl) \
WX_PG_IMPLEMENT_VARIANT_DATA_EXPORTED_NO_EQ_NO_GETTER(classname,wxEMPTY_PARAMETER_VALUE expdecl) \
WX_PG_IMPLEMENT_VARIANT_DATA_GETTER(classname,wxEMPTY_PARAMETER_VALUE expdecl) \
\
bool classname##VariantData::Eq([[maybe_unused]] wxVariantData& data) const \
{\
    return false; \
}

#define WX_PG_IMPLEMENT_VARIANT_DATA_DUMMY_EQ(classname) \
WX_PG_IMPLEMENT_VARIANT_DATA_EXPORTED_DUMMY_EQ(classname, wxEMPTY_PARAMETER_VALUE)

WX_PG_DECLARE_VARIANT_DATA_EXPORTED(wxPoint)
WX_PG_DECLARE_VARIANT_DATA_EXPORTED(wxSize)
WX_PG_DECLARE_VARIANT_DATA_EXPORTED(wxArrayInt)
DECLARE_VARIANT_OBJECT_EXPORTED(wxFont)
template<> inline wxVariant WXVARIANT( const wxFont& value )
{
    wxVariant variant;
    variant << value;
    return variant;
}

template<> inline wxVariant WXVARIANT( const wxColour& value )
{
    wxVariant variant;
    variant << value;
    return variant;
}

// Define constants for common wxVariant type strings

#define wxPG_VARIANT_TYPE_STRING        wxPGGlobalVars->m_strstring
#define wxPG_VARIANT_TYPE_LONG          wxPGGlobalVars->m_strlong
#define wxPG_VARIANT_TYPE_BOOL          wxPGGlobalVars->m_strbool
#define wxPG_VARIANT_TYPE_LIST          wxPGGlobalVars->m_strlist
#define wxPG_VARIANT_TYPE_DOUBLE        "double"
#define wxPG_VARIANT_TYPE_ARRSTRING     "arrstring"
#if wxUSE_DATETIME
#define wxPG_VARIANT_TYPE_DATETIME      "datetime"
#endif
#if wxUSE_LONGLONG
#define wxPG_VARIANT_TYPE_LONGLONG      "longlong"
#define wxPG_VARIANT_TYPE_ULONGLONG     "ulonglong"
#endif
#endif // !SWIG

// -----------------------------------------------------------------------

//
// Tokenizer macros.
// NOTE: I have made two versions - worse ones (performance and consistency
//   wise) use wxStringTokenizer and better ones (may have unfound bugs)
//   use custom code.
//

// TOKENIZER1 can be done with wxStringTokenizer
#define WX_PG_TOKENIZER1_BEGIN(WXSTRING,DELIMITER) \
    wxStringTokenizer tkz(WXSTRING,DELIMITER,wxStringTokenizerMode::RetEmpty); \
    while ( tkz.HasMoreTokens() ) \
    { \
        wxString token = tkz.GetNextToken(); \
        token.Trim(true); \
        token.Trim(false);

#define WX_PG_TOKENIZER1_END() \
    }


//
// 2nd version: tokens are surrounded by DELIMITERs (for example, C-style
// strings). TOKENIZER2 must use custom code (a class) for full compliance with
// " surrounded strings with \" inside.
//
// class implementation is in propgrid.cpp
//

class wxPGStringTokenizer
{
public:
    wxPGStringTokenizer( const wxString& str, wxChar delimiter );

    bool HasMoreTokens(); // not const so we can do some stuff in it
    wxString GetNextToken();

protected:
    const wxString*             m_str;
    wxString::const_iterator    m_curPos;
    wxString                    m_readyToken;
    wxUniChar                   m_delimiter;
};

#define WX_PG_TOKENIZER2_BEGIN(WXSTRING,DELIMITER) \
    wxPGStringTokenizer tkz(WXSTRING,DELIMITER); \
    while ( tkz.HasMoreTokens() ) \
    { \
        wxString token = tkz.GetNextToken();

#define WX_PG_TOKENIZER2_END() \
    }

// -----------------------------------------------------------------------
// wxVector utilities

// Utility to check if specific item is in a vector.
template<typename T>
inline bool wxPGItemExistsInVector(const std::vector<T>& vector, const T& item)
{
    return std::ranges::find(vector, item) != vector.end();
}

// Utility to determine the index of the item in the vector.
template<typename T>
inline int wxPGItemIndexInVector(const std::vector<T>& vector, const T& item)
{
    typename std::vector<T>::const_iterator it = std::ranges::find(vector, item);
    if ( it != vector.end() )
        return (int)(it - vector.begin());

    return wxNOT_FOUND;
}

// Utility to remove given item from the vector.
template<typename T>
inline void wxPGRemoveItemFromVector(std::vector<T>& vector, const T& item)
{
    typename std::vector<T>::iterator it = std::ranges::find(vector, item);
    if ( it != vector.end() )
    {
        vector.erase(it);
    }
}

// Utility to calaculate sum of all elements of the vector.
template<typename T>
inline T wxPGGetSumVectorItems(const std::vector<T>& vector, T init)
{
    return std::accumulate(vector.begin(), vector.end(), init);
}

// -----------------------------------------------------------------------

#endif // wxUSE_PROPGRID

#endif // _WX_PROPGRID_PROPGRIDDEFS_H_
