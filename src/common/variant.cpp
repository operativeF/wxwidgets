/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/variant.cpp
// Purpose:     wxVariant class, container for any type
// Author:      Julian Smart
// Modified by:
// Created:     10/09/98
// Copyright:   (c)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_VARIANT

#include "wx/variant.h"

#include "wx/string.h"
#include "wx/crt.h"

#if wxUSE_STREAMS
    #include "wx/stream.h"
#endif

import Utils.Strings;

import <fstream>;

#if wxUSE_STREAMS
import WX.Cmn.TextStream;
#endif

import Utils.Strings;

wxVariant wxNullVariant;


#include "wx/listimpl.cpp"
WX_DEFINE_LIST(wxVariantList)

/*
 * wxVariant
 */

wxIMPLEMENT_DYNAMIC_CLASS(wxVariant, wxObject);

bool wxVariant::IsNull() const
{
     return (m_refData == nullptr);
}

void wxVariant::MakeNull()
{
    UnRef();
}

void wxVariant::Clear()
{
    m_name.clear();
}

wxVariant::wxVariant(const wxVariant& variant)
    : 
     m_name(variant.m_name)
{
    if (!variant.IsNull())
        Ref(variant);
}

wxVariant::wxVariant(wxVariantData* data, const wxString& name) // User-defined data
    : 
     m_name(name)
{
    m_refData = data;
}

wxObjectRefData *wxVariant::CreateRefData() const
{
    // We cannot create any particular wxVariantData.
    wxFAIL_MSG("wxVariant::CreateRefData() cannot be implemented");
    return nullptr;
}

wxObjectRefData *wxVariant::CloneRefData(const wxObjectRefData *data) const
{
    return dynamic_cast<const wxVariantData*>(data)->Clone();
}

// Assignment
void wxVariant::operator= (const wxVariant& variant)
{
    Ref(variant);
    m_name = variant.m_name;
}

// myVariant = new wxStringVariantData("hello")
void wxVariant::operator= (wxVariantData* variantData)
{
    UnRef();
    m_refData = variantData;
}

bool wxVariant::operator== (const wxVariant& variant) const
{
    if (IsNull() || variant.IsNull())
        return (IsNull() == variant.IsNull());

    if (GetType() != variant.GetType())
        return false;

    return (GetData()->Eq(* variant.GetData()));
}

bool wxVariant::operator!= (const wxVariant& variant) const
{
    return (!(*this == variant));
}

wxString wxVariant::MakeString() const
{
    wxString str;
    if (!IsNull())
        GetData()->Write(str);
    return str;
}

void wxVariant::SetData(wxVariantData* data)
{
    UnRef();
    m_refData = data;
}

bool wxVariant::Unshare()
{
    if ( !m_refData || m_refData->GetRefCount() == 1 )
        return true;

    wxObject::UnShare();

    return (m_refData && m_refData->GetRefCount() == 1);
}


// Returns a string representing the type of the variant,
// e.g. "string", "bool", "list", "double", "long"
wxString wxVariant::GetType() const
{
    if (IsNull())
        return wxString("null");
    else
        return GetData()->GetType();
}


bool wxVariant::IsType(const wxString& type) const
{
    return (GetType() == type);
}

bool wxVariant::IsValueKindOf(const wxClassInfo* type) const
{
    wxClassInfo* info=GetData()->GetValueClassInfo();
    return info ? info->IsKindOf(type) : false ;
}

// -----------------------------------------------------------------
// wxVariant <-> wxAny conversion code
// -----------------------------------------------------------------

#if wxUSE_ANY

wxAnyToVariantRegistration::
    wxAnyToVariantRegistration(wxVariantDataFactory factory)
        : m_factory(factory)
{
    wxPreRegisterAnyToVariant(this);
}

wxVariant::wxVariant(const wxAny& any)
     
{
    wxVariant variant;
    if ( !any.GetAs(&variant) )
    {
        wxFAIL_MSG("wxAny of this type cannot be converted to wxVariant");
        return;
    }

    *this = variant;
}

wxAny wxVariant::GetAny() const
{
    if ( IsNull() )
        return wxAny();

    wxAny any;
    wxVariantData* data = GetData();

    if ( data->GetAsAny(&any) )
        return any;

    // If everything else fails, wrap the whole wxVariantData
    return wxAny(data);
}

#endif // wxUSE_ANY

// -----------------------------------------------------------------
// wxVariantDataLong
// -----------------------------------------------------------------

class wxVariantDataLong: public wxVariantData
{
public:
    wxVariantDataLong() = default;
    explicit wxVariantDataLong(long value) : m_value(value) {}

    inline long GetValue() const { return m_value; }
    inline void SetValue(long value) {m_value = value;}

    bool Eq(wxVariantData& data) const override;

    bool Read(wxString& str) override;
    bool Write(wxString& str) const override;
    bool Read(std::istream& str) override;
    bool Write(std::ostream& str) const override;
#if wxUSE_STREAMS
    virtual bool Read(wxInputStream& str);
    virtual bool Write(wxOutputStream &str) const;
#endif // wxUSE_STREAMS

    wxVariantData* Clone() const override { return new wxVariantDataLong(m_value); }

    wxString GetType() const override { return "long"; }

#if wxUSE_ANY
    // Since wxAny does not have separate type for integers shorter than
    // longlong, we do not usually implement wxVariant->wxAny conversion
    // here (but in wxVariantDataLongLong instead).
  #ifndef wxLongLong_t
    DECLARE_WXANY_CONVERSION()
  #else
    bool GetAsAny(wxAny* any) const override
    {
        *any = m_value;
        return true;
    }
  #endif
#endif // wxUSE_ANY

protected:
    long m_value{0};
};

#ifndef wxLongLong_t
IMPLEMENT_TRIVIAL_WXANY_CONVERSION(long, wxVariantDataLong)
#endif

bool wxVariantDataLong::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "long"), "wxVariantDataLong::Eq: argument mismatch" );

    wxVariantDataLong& otherData = (wxVariantDataLong&) data;

    return (otherData.m_value == m_value);
}

bool wxVariantDataLong::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << (const char*) s.mb_str();
    return true;
}

bool wxVariantDataLong::Write(wxString& str) const
{
    str.Printf("%ld", m_value);
    return true;
}

bool wxVariantDataLong::Read(std::istream& str)
{
    str >> m_value;
    return true;
}

#if wxUSE_STREAMS
bool wxVariantDataLong::Write(wxOutputStream& str) const
{
    wxTextOutputStream s(str);

    s.Write32((size_t)m_value);
    return true;
}

bool wxVariantDataLong::Read(wxInputStream& str)
{
   wxTextInputStream s(str);
   m_value = s.Read32();
   return true;
}
#endif // wxUSE_STREAMS

bool wxVariantDataLong::Read(wxString& str)
{
    m_value = wxAtol(str);
    return true;
}

// wxVariant

wxVariant::wxVariant(long val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataLong(val);
}

wxVariant::wxVariant(int val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataLong((long)val);
}

wxVariant::wxVariant(short val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataLong((long)val);
}

bool wxVariant::operator== (long value) const
{
    long thisValue;
    if (!Convert(&thisValue))
        return false;
    else
        return (value == thisValue);
}

bool wxVariant::operator!= (long value) const
{
    return (!((*this) == value));
}

void wxVariant::operator= (long value)
{
    if (GetType() == "long" &&
        m_refData->GetRefCount() == 1)
    {
        ((wxVariantDataLong*)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDataLong(value);
    }
}

long wxVariant::GetLong() const
{
    long value;
    if (Convert(& value))
        return value;
    else
    {
        wxFAIL_MSG("Could not convert to a long");
        return 0;
    }
}

// -----------------------------------------------------------------
// wxVariantDoubleData
// -----------------------------------------------------------------

class wxVariantDoubleData: public wxVariantData
{
public:
    wxVariantDoubleData() = default;
    explicit wxVariantDoubleData(double value) : m_value(value) {}

    inline double GetValue() const { return m_value; }
    inline void SetValue(double value) { m_value = value; }

    bool Eq(wxVariantData& data) const override;
    bool Read(wxString& str) override;
    bool Write(std::ostream& str) const override;
    bool Write(wxString& str) const override;
    bool Read(std::istream& str) override;
#if wxUSE_STREAMS
    virtual bool Read(wxInputStream& str);
    virtual bool Write(wxOutputStream &str) const;
#endif // wxUSE_STREAMS
    wxString GetType() const override { return "double"; }

    wxVariantData* Clone() const override { return new wxVariantDoubleData(m_value); }

    DECLARE_WXANY_CONVERSION()
protected:
    double m_value{0.0};
};

IMPLEMENT_TRIVIAL_WXANY_CONVERSION(double, wxVariantDoubleData)

bool wxVariantDoubleData::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "double"), "wxVariantDoubleData::Eq: argument mismatch" );

    wxVariantDoubleData& otherData = (wxVariantDoubleData&) data;

    // FIXME: Don't do this, it's wrong.
    return otherData.m_value == m_value;
}

bool wxVariantDoubleData::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << (const char*) s.mb_str();
    return true;
}

bool wxVariantDoubleData::Write(wxString& str) const
{
    str.Printf("%.14g", m_value);
    return true;
}

bool wxVariantDoubleData::Read(std::istream& str)
{
    str >> m_value;
    return true;
}

#if wxUSE_STREAMS
bool wxVariantDoubleData::Write(wxOutputStream& str) const
{
    wxTextOutputStream s(str);
    s.WriteDouble((double)m_value);
    return true;
}

bool wxVariantDoubleData::Read(wxInputStream& str)
{
    wxTextInputStream s(str);
    m_value = s.ReadDouble();
    return true;
}
#endif // wxUSE_STREAMS

bool wxVariantDoubleData::Read(wxString& str)
{
    m_value = wxAtof(str);
    return true;
}

//  wxVariant double code

wxVariant::wxVariant(double val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDoubleData(val);
}

bool wxVariant::operator== (double value) const
{
    double thisValue;
    if (!Convert(&thisValue))
        return false;
    // FIXME: Double equality
    return value == thisValue;
}

bool wxVariant::operator!= (double value) const
{
    return (!((*this) == value));
}

void wxVariant::operator= (double value)
{
    if (GetType() == "double" &&
        m_refData->GetRefCount() == 1)
    {
        ((wxVariantDoubleData*)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDoubleData(value);
    }
}

double wxVariant::GetDouble() const
{
    double value;
    if (Convert(& value))
        return value;
    else
    {
        wxFAIL_MSG("Could not convert to a double number");
        return 0.0;
    }
}

// -----------------------------------------------------------------
// wxVariantBoolData
// -----------------------------------------------------------------

class wxVariantDataBool: public wxVariantData
{
public:
    wxVariantDataBool() = default;
    explicit wxVariantDataBool(bool value) : m_value(value) {}

    inline bool GetValue() const { return m_value; }
    inline void SetValue(bool value) {m_value = value;}

    bool Eq(wxVariantData& data) const override;
    bool Write(std::ostream& str) const override;
    bool Write(wxString& str) const override;
    bool Read(wxString& str) override;
    bool Read(std::istream& str) override;
#if wxUSE_STREAMS
    virtual bool Read(wxInputStream& str);
    virtual bool Write(wxOutputStream& str) const;
#endif // wxUSE_STREAMS
    wxString GetType() const override { return "bool"; }

    wxVariantData* Clone() const override { return new wxVariantDataBool(m_value); }

    DECLARE_WXANY_CONVERSION()
protected:
    bool m_value{false};
};

IMPLEMENT_TRIVIAL_WXANY_CONVERSION(bool, wxVariantDataBool)

bool wxVariantDataBool::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "bool"), "wxVariantDataBool::Eq: argument mismatch" );

    const wxVariantDataBool& otherData = (wxVariantDataBool&) data;

    return (otherData.m_value == m_value);
}

bool wxVariantDataBool::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << (const char*) s.mb_str();
    return true;
}

bool wxVariantDataBool::Write(wxString& str) const
{
    str.Printf("%d", (int) m_value);
    return true;
}

bool wxVariantDataBool::Read([[maybe_unused]] std::istream& str)
{
    wxFAIL_MSG("Unimplemented");
//    str >> (long) m_value;
    return false;
}

#if wxUSE_STREAMS
bool wxVariantDataBool::Write(wxOutputStream& str) const
{
    wxTextOutputStream s(str);

    s.Write8(m_value);
    return true;
}

bool wxVariantDataBool::Read(wxInputStream& str)
{
    wxTextInputStream s(str);

    m_value = s.Read8() != 0;
    return true;
}
#endif // wxUSE_STREAMS

bool wxVariantDataBool::Read(wxString& str)
{
    m_value = (wxAtol(str) != 0);
    return true;
}

// wxVariant ****

wxVariant::wxVariant(bool val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataBool(val);
}

bool wxVariant::operator== (bool value) const
{
    bool thisValue;
    if (!Convert(&thisValue))
        return false;
    else
        return (value == thisValue);
}

bool wxVariant::operator!= (bool value) const
{
    return (!((*this) == value));
}

void wxVariant::operator= (bool value)
{
    if (GetType() == "bool" &&
        m_refData->GetRefCount() == 1)
    {
        ((wxVariantDataBool*)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDataBool(value);
    }
}

bool wxVariant::GetBool() const
{
    bool value;
    if (Convert(& value))
        return value;
    else
    {
        wxFAIL_MSG("Could not convert to a bool");
        return false;
    }
}

// -----------------------------------------------------------------
// wxVariantDataChar
// -----------------------------------------------------------------

class wxVariantDataChar: public wxVariantData
{
public:
    wxVariantDataChar() : m_value(0) { }
    explicit wxVariantDataChar(const wxUniChar& value) : m_value(value) { }

    inline wxUniChar GetValue() const { return m_value; }
    inline void SetValue(const wxUniChar& value) { m_value = value; }

    bool Eq(wxVariantData& data) const override;
    bool Read(std::istream& str) override;
    bool Write(std::ostream& str) const override;
    bool Read(wxString& str) override;
    bool Write(wxString& str) const override;
#if wxUSE_STREAMS
    virtual bool Read(wxInputStream& str);
    virtual bool Write(wxOutputStream& str) const;
#endif // wxUSE_STREAMS
    wxString GetType() const override { return "char"; }
    wxVariantData* Clone() const override { return new wxVariantDataChar(m_value); }

    DECLARE_WXANY_CONVERSION()
protected:
    wxUniChar m_value;
};

IMPLEMENT_TRIVIAL_WXANY_CONVERSION(wxUniChar, wxVariantDataChar)

bool wxVariantDataChar::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "char"), "wxVariantDataChar::Eq: argument mismatch" );

    wxVariantDataChar& otherData = (wxVariantDataChar&) data;

    return (otherData.m_value == m_value);
}

bool wxVariantDataChar::Write(std::ostream& str) const
{
    str << wxString(m_value);
    return true;
}

bool wxVariantDataChar::Write(wxString& str) const
{
    str = m_value;
    return true;
}

bool wxVariantDataChar::Read([[maybe_unused]] std::istream& str)
{
    wxFAIL_MSG("Unimplemented");

    return false;
}

#if wxUSE_STREAMS
bool wxVariantDataChar::Write(wxOutputStream& str) const
{
    wxTextOutputStream s(str);

    // FIXME-UTF8: this should be just "s << m_value;" after removal of
    //             ANSI build and addition of wxUniChar to wxTextOutputStream:
    s << (wxChar)m_value;

    return true;
}

bool wxVariantDataChar::Read(wxInputStream& str)
{
    wxTextInputStream s(str);

    // FIXME-UTF8: this should be just "s >> m_value;" after removal of
    //             ANSI build and addition of wxUniChar to wxTextInputStream:
    wxChar ch;
    s >> ch;
    m_value = ch;

    return true;
}
#endif // wxUSE_STREAMS

bool wxVariantDataChar::Read(wxString& str)
{
    m_value = str[0u];
    return true;
}

wxVariant::wxVariant(const wxUniChar& val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataChar(val);
}

wxVariant::wxVariant(char val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataChar(val);
}

wxVariant::wxVariant(wchar_t val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataChar(val);
}

bool wxVariant::operator==(const wxUniChar& value) const
{
    wxUniChar thisValue;
    if (!Convert(&thisValue))
        return false;
    else
        return (value == thisValue);
}

wxVariant& wxVariant::operator=(const wxUniChar& value)
{
    if (GetType() == "char" &&
        m_refData->GetRefCount() == 1)
    {
        ((wxVariantDataChar*)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDataChar(value);
    }

    return *this;
}

wxUniChar wxVariant::GetChar() const
{
    wxUniChar value;
    if (Convert(& value))
        return value;
    else
    {
        wxFAIL_MSG("Could not convert to a char");
        return wxUniChar(0);
    }
}

// ----------------------------------------------------------------------------
// wxVariantDataString
// ----------------------------------------------------------------------------

class wxVariantDataString: public wxVariantData
{
public:
    wxVariantDataString() = default;
    explicit wxVariantDataString(const wxString& value) : m_value(value) { }

    inline wxString GetValue() const { return m_value; }
    inline void SetValue(const wxString& value) { m_value = value; }

    bool Eq(wxVariantData& data) const override;
    bool Write(std::ostream& str) const override;
    bool Read(wxString& str) override;
    bool Write(wxString& str) const override;
    bool Read([[maybe_unused]] std::istream& str) override { return false; }
#if wxUSE_STREAMS
    virtual bool Read(wxInputStream& str);
    virtual bool Write(wxOutputStream& str) const;
#endif // wxUSE_STREAMS
    wxString GetType() const override { return "string"; }
    wxVariantData* Clone() const override { return new wxVariantDataString(m_value); }

    DECLARE_WXANY_CONVERSION()
protected:
    wxString m_value;
};

IMPLEMENT_TRIVIAL_WXANY_CONVERSION(wxString, wxVariantDataString)

#if wxUSE_ANY
// This allows converting string literal wxAnys to string variants
wxVariantData* wxVariantDataFromConstCharPAny(const wxAny& any)
{
    return new wxVariantDataString(any.As<const char*>());
}

wxVariantData* wxVariantDataFromConstWchar_tPAny(const wxAny& any)
{
    return new wxVariantDataString(any.As<const wchar_t*>());
}

_REGISTER_WXANY_CONVERSION(const char*,
                           ConstCharP,
                           wxVariantDataFromConstCharPAny)
_REGISTER_WXANY_CONVERSION(const wchar_t*,
                           ConstWchar_tP,
                           wxVariantDataFromConstWchar_tPAny)
#endif

bool wxVariantDataString::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "string"), "wxVariantDataString::Eq: argument mismatch" );

    const wxVariantDataString& otherData = (wxVariantDataString&) data;

    return (otherData.m_value == m_value);
}

bool wxVariantDataString::Write(std::ostream& str) const
{
    str << (const char*) m_value.mb_str();
    return true;
}

bool wxVariantDataString::Write(wxString& str) const
{
    str = m_value;
    return true;
}

#if wxUSE_STREAMS
bool wxVariantDataString::Write(wxOutputStream& str) const
{
  // why doesn't wxOutputStream::operator<< take "const wxString&"
    wxTextOutputStream s(str);
    s.WriteString(m_value);
    return true;
}

bool wxVariantDataString::Read(wxInputStream& str)
{
    wxTextInputStream s(str);

    m_value = s.ReadLine();
    return true;
}
#endif // wxUSE_STREAMS

bool wxVariantDataString::Read(wxString& str)
{
    m_value = str;
    return true;
}

// wxVariant ****

wxVariant::wxVariant(const wxString& val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataString(val);
}

wxVariant::wxVariant(const char* val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataString(wxString(val));
}

wxVariant::wxVariant(const wchar_t* val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataString(wxString(val));
}

wxVariant::wxVariant(const wxCStrData& val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataString(val.AsString());
}

wxVariant::wxVariant(const wxScopedCharBuffer& val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataString(wxString(val));
}

wxVariant::wxVariant(const wxScopedWCharBuffer& val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataString(wxString(val));
}

wxVariant::wxVariant(const std::string& val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataString(wxString(val));
}

wxVariant::wxVariant(const wxStdWideString& val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataString(wxString(val));
}

bool wxVariant::operator== (const wxString& value) const
{
    wxString thisValue;
    if (!Convert(&thisValue))
        return false;

    return value == thisValue;
}

bool wxVariant::operator!= (const wxString& value) const
{
    return (!((*this) == value));
}

wxVariant& wxVariant::operator= (const wxString& value)
{
    if (GetType() == "string" &&
        m_refData->GetRefCount() == 1)
    {
        ((wxVariantDataString*)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDataString(value);
    }
    return *this;
}

wxString wxVariant::GetString() const
{
    wxString value;
    if (!Convert(& value))
    {
        wxFAIL_MSG("Could not convert to a string");
    }

    return value;
}

// ----------------------------------------------------------------------------
// wxVariantDataWxObjectPtr
// ----------------------------------------------------------------------------

class wxVariantDataWxObjectPtr: public wxVariantData
{
public:
    wxVariantDataWxObjectPtr() = default;
    explicit wxVariantDataWxObjectPtr(wxObject* value) : m_value(value) {}

    inline wxObject* GetValue() const { return m_value; }
    inline void SetValue(wxObject* value) {m_value = value;}

    bool Eq(wxVariantData& data) const override;
    bool Write(std::ostream& str) const override;
    bool Write(wxString& str) const override;
    bool Read(std::istream& str) override;
    bool Read(wxString& str) override;
    wxString GetType() const override ;
    wxVariantData* Clone() const override { return new wxVariantDataWxObjectPtr(m_value); }

    wxClassInfo* GetValueClassInfo() override;

    DECLARE_WXANY_CONVERSION()
protected:
    wxObject* m_value;
};

IMPLEMENT_TRIVIAL_WXANY_CONVERSION(wxObject*, wxVariantDataWxObjectPtr)

bool wxVariantDataWxObjectPtr::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( data.GetType() == GetType(), "wxVariantDataWxObjectPtr::Eq: argument mismatch" );

    const wxVariantDataWxObjectPtr& otherData = (wxVariantDataWxObjectPtr&) data;

    return (otherData.m_value == m_value);
}

wxString wxVariantDataWxObjectPtr::GetType() const
{
    wxString returnVal("wxObject*");

    if (m_value)
    {
        returnVal = m_value->wxGetClassInfo()->wxGetClassName();
        returnVal += "*";
    }

    return returnVal;
}

wxClassInfo* wxVariantDataWxObjectPtr::GetValueClassInfo()
{
    wxClassInfo* returnVal=nullptr;

    if (m_value) returnVal = m_value->wxGetClassInfo();

    return returnVal;
}

bool wxVariantDataWxObjectPtr::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << (const char*) s.mb_str();
    return true;
}

bool wxVariantDataWxObjectPtr::Write(wxString& str) const
{
    str.Printf("%s(%p)", GetType().c_str(), m_value);
    return true;
}

bool wxVariantDataWxObjectPtr::Read([[maybe_unused]] std::istream& str)
{
    // Not implemented
    return false;
}

bool wxVariantDataWxObjectPtr::Read([[maybe_unused]] wxString& str)
{
    // Not implemented
    return false;
}

// wxVariant

wxVariant::wxVariant( wxObject* val, const wxString& name)
    : m_name(name)
{
    m_refData = new wxVariantDataWxObjectPtr(val);
}

bool wxVariant::operator== (wxObject* value) const
{
    return (value == ((wxVariantDataWxObjectPtr*)GetData())->GetValue());
}

bool wxVariant::operator!= (wxObject* value) const
{
    return (!((*this) == (wxObject*) value));
}

void wxVariant::operator= (wxObject* value)
{
    UnRef();
    m_refData = new wxVariantDataWxObjectPtr(value);
}

wxObject* wxVariant::GetWxObjectPtr() const
{
    return (wxObject*) ((wxVariantDataWxObjectPtr*) m_refData)->GetValue();
}

// ----------------------------------------------------------------------------
// wxVariantDataVoidPtr
// ----------------------------------------------------------------------------

class wxVariantDataVoidPtr: public wxVariantData
{
public:
    wxVariantDataVoidPtr() = default;
    explicit wxVariantDataVoidPtr(void* value) : m_value(value) {}

    inline void* GetValue() const { return m_value; }
    inline void SetValue(void* value) { m_value = value; }

    bool Eq(wxVariantData& data) const override;
    bool Write(std::ostream& str) const override;
    bool Write(wxString& str) const override;
    bool Read(std::istream& str) override;
    bool Read(wxString& str) override;
    wxString GetType() const override { return "void*"; }
    wxVariantData* Clone() const override { return new wxVariantDataVoidPtr(m_value); }

    DECLARE_WXANY_CONVERSION()
protected:
    void* m_value;
};

IMPLEMENT_TRIVIAL_WXANY_CONVERSION(void*, wxVariantDataVoidPtr)

bool wxVariantDataVoidPtr::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( data.GetType() == "void*", "wxVariantDataVoidPtr::Eq: argument mismatch" );

    wxVariantDataVoidPtr& otherData = (wxVariantDataVoidPtr&) data;

    return (otherData.m_value == m_value);
}

bool wxVariantDataVoidPtr::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << (const char*) s.mb_str();
    return true;
}

bool wxVariantDataVoidPtr::Write(wxString& str) const
{
    str.Printf("%p", m_value);
    return true;
}

bool wxVariantDataVoidPtr::Read([[maybe_unused]] std::istream& str)
{
    // Not implemented
    return false;
}

bool wxVariantDataVoidPtr::Read([[maybe_unused]] wxString& str)
{
    // Not implemented
    return false;
}

// wxVariant

wxVariant::wxVariant( void* val, const wxString& name)
{
    m_refData = new wxVariantDataVoidPtr(val);
    m_name = name;
}

bool wxVariant::operator== (void* value) const
{
    return (value == ((wxVariantDataVoidPtr*)GetData())->GetValue());
}

bool wxVariant::operator!= (void* value) const
{
    return (!((*this) == (void*) value));
}

void wxVariant::operator= (void* value)
{
    if (GetType() == "void*" && (m_refData->GetRefCount() == 1))
    {
        ((wxVariantDataVoidPtr*)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDataVoidPtr(value);
    }
}

void* wxVariant::GetVoidPtr() const
{
    // handling this specially is convenient when working with COM, see #9873
    if ( IsNull() )
        return nullptr;

    wxASSERT( GetType() == "void*" );

    return (void*) ((wxVariantDataVoidPtr*) m_refData)->GetValue();
}

// ----------------------------------------------------------------------------
// wxVariantDataDateTime
// ----------------------------------------------------------------------------

#if wxUSE_DATETIME

class wxVariantDataDateTime: public wxVariantData
{
public:
    wxVariantDataDateTime() = default;
    explicit wxVariantDataDateTime(const wxDateTime& value) : m_value(value) { }

    inline wxDateTime GetValue() const { return m_value; }
    inline void SetValue(const wxDateTime& value) { m_value = value; }

    bool Eq(wxVariantData& data) const override;
    bool Write(std::ostream& str) const override;
    bool Write(wxString& str) const override;
    bool Read(std::istream& str) override;
    bool Read(wxString& str) override;
    wxString GetType() const override { return "datetime"; }
    wxVariantData* Clone() const override { return new wxVariantDataDateTime(m_value); }

    DECLARE_WXANY_CONVERSION()
protected:
    wxDateTime m_value;
};

IMPLEMENT_TRIVIAL_WXANY_CONVERSION(wxDateTime, wxVariantDataDateTime)

bool wxVariantDataDateTime::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "datetime"), "wxVariantDataDateTime::Eq: argument mismatch" );

    wxVariantDataDateTime& otherData = (wxVariantDataDateTime&) data;

    return (otherData.m_value == m_value);
}


bool wxVariantDataDateTime::Write(std::ostream& str) const
{
    wxString value;
    Write( value );
    str << value.c_str();
    return true;
}


bool wxVariantDataDateTime::Write(wxString& str) const
{
    if ( m_value.IsValid() )
        str = m_value.Format();
    else
        str = "Invalid";
    return true;
}


bool wxVariantDataDateTime::Read([[maybe_unused]] std::istream& str)
{
    // Not implemented
    return false;
}


bool wxVariantDataDateTime::Read(wxString& str)
{
    if ( str == "Invalid" )
    {
        m_value = wxInvalidDateTime;
        return true;
    }

    wxString::const_iterator end;
    return m_value.ParseDateTime(str, &end) && end == str.end();
}

// wxVariant

wxVariant::wxVariant(const wxDateTime& val, const wxString& name) // Date
{
    m_refData = new wxVariantDataDateTime(val);
    m_name = name;
}

bool wxVariant::operator== (const wxDateTime& value) const
{
    wxDateTime thisValue;
    if (!Convert(&thisValue))
        return false;

    return value.IsEqualTo(thisValue);
}

bool wxVariant::operator!= (const wxDateTime& value) const
{
    return (!((*this) == value));
}

void wxVariant::operator= (const wxDateTime& value)
{
    if (GetType() == "datetime" &&
        m_refData->GetRefCount() == 1)
    {
        ((wxVariantDataDateTime*)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDataDateTime(value);
    }
}

wxDateTime wxVariant::GetDateTime() const
{
    wxDateTime value;
    if (!Convert(& value))
    {
        wxFAIL_MSG("Could not convert to a datetime");
    }

    return value;
}

#endif // wxUSE_DATETIME

// ----------------------------------------------------------------------------
// wxVariantDataArrayString
// ----------------------------------------------------------------------------

class wxVariantDataArrayString: public wxVariantData
{
public:
    wxVariantDataArrayString() = default;
    explicit wxVariantDataArrayString(const std::vector<wxString>& value) : m_value(value) { }

    std::vector<wxString> GetValue() const { return m_value; }
    void SetValue(const std::vector<wxString>& value) { m_value = value; }

    bool Eq(wxVariantData& data) const override;
    bool Write(std::ostream& str) const override;
    bool Write(wxString& str) const override;
    bool Read(std::istream& str) override;
    bool Read(wxString& str) override;
    wxString GetType() const override { return "arrstring"; }
    wxVariantData* Clone() const override { return new wxVariantDataArrayString(m_value); }

    DECLARE_WXANY_CONVERSION()
protected:
    std::vector<wxString> m_value;
};

IMPLEMENT_TRIVIAL_WXANY_CONVERSION(std::vector<wxString>, wxVariantDataArrayString)

bool wxVariantDataArrayString::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( data.GetType() == GetType(), "wxVariantDataArrayString::Eq: argument mismatch" );

    wxVariantDataArrayString& otherData = (wxVariantDataArrayString&) data;

    return otherData.m_value == m_value;
}

bool wxVariantDataArrayString::Write([[maybe_unused]] std::ostream& str) const
{
    // Not implemented
    return false;
}

bool wxVariantDataArrayString::Write(wxString& str) const
{
    const size_t count = m_value.size();
    for ( size_t n = 0; n < count; n++ )
    {
        if ( n )
            str += wxT(';');

        str += m_value[n];
    }

    return true;
}


bool wxVariantDataArrayString::Read([[maybe_unused]] std::istream& str)
{
    // Not implemented
    return false;
}


bool wxVariantDataArrayString::Read(wxString& str)
{
    wxStringTokenizer tk(str, ";");
    while ( tk.HasMoreTokens() )
    {
        m_value.push_back(tk.GetNextToken());
    }

    return true;
}

// wxVariant

wxVariant::wxVariant(const std::vector<wxString>& val, const wxString& name) // Strings
{
    m_refData = new wxVariantDataArrayString(val);
    m_name = name;
}

bool wxVariant::operator==([[maybe_unused]] const std::vector<wxString>& value) const
{
    wxFAIL_MSG( "TODO" );

    return false;
}

bool wxVariant::operator!=(const std::vector<wxString>& value) const
{
    return !(*this == value);
}

void wxVariant::operator=(const std::vector<wxString>& value)
{
    if (GetType() == "arrstring" &&
        m_refData->GetRefCount() == 1)
    {
        ((wxVariantDataArrayString *)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDataArrayString(value);
    }
}

std::vector<wxString> wxVariant::GetArrayString() const
{
    if ( GetType() == "arrstring" )
        return ((wxVariantDataArrayString *)GetData())->GetValue();

    return std::vector<wxString>();
}

// ----------------------------------------------------------------------------
// wxVariantDataLongLong
// ----------------------------------------------------------------------------

#if wxUSE_LONGLONG

class wxVariantDataLongLong : public wxVariantData
{
public:
    wxVariantDataLongLong() : m_value(0) { }
    explicit wxVariantDataLongLong(wxLongLong value) : m_value(value) { }

    wxLongLong GetValue() const { return m_value; }
    void SetValue(wxLongLong value) { m_value = value; }

    bool Eq(wxVariantData& data) const override;

    bool Read(wxString& str) override;
    bool Write(wxString& str) const override;
    bool Read(std::istream& str) override;
    bool Write(std::ostream& str) const override;
#if wxUSE_STREAMS
    virtual bool Read(wxInputStream& str);
    virtual bool Write(wxOutputStream &str) const;
#endif // wxUSE_STREAMS

    wxVariantData* Clone() const override
    {
        return new wxVariantDataLongLong(m_value);
    }

    wxString GetType() const override { return "longlong"; }

    DECLARE_WXANY_CONVERSION()
protected:
    wxLongLong m_value;
};

//
// wxLongLong type requires customized wxAny conversion code
//
#if wxUSE_ANY
#ifdef wxLongLong_t

bool wxVariantDataLongLong::GetAsAny(wxAny* any) const
{
    *any = m_value.GetValue();
    return true;
}

wxVariantData* wxVariantDataLongLong::VariantDataFactory(const wxAny& any)
{
    return new wxVariantDataLongLong(any.As<wxLongLong_t>());
}

REGISTER_WXANY_CONVERSION(wxLongLong_t, wxVariantDataLongLong)

#else // if !defined(wxLongLong_t)

bool wxVariantDataLongLong::GetAsAny(wxAny* any) const
{
    *any = m_value;
    return true;
}

wxVariantData* wxVariantDataLongLong::VariantDataFactory(const wxAny& any)
{
    return new wxVariantDataLongLong(any.As<wxLongLong>());
}

REGISTER_WXANY_CONVERSION(wxLongLong, wxVariantDataLongLong)

#endif // defined(wxLongLong_t)/!defined(wxLongLong_t)
#endif // wxUSE_ANY

bool wxVariantDataLongLong::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "longlong"),
                  "wxVariantDataLongLong::Eq: argument mismatch" );

    wxVariantDataLongLong& otherData = (wxVariantDataLongLong&) data;

    return (otherData.m_value == m_value);
}

bool wxVariantDataLongLong::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << (const char*) s.mb_str();
    return true;
}

bool wxVariantDataLongLong::Write(wxString& str) const
{
#ifdef wxLongLong_t
    str.Printf("%lld", m_value.GetValue());
    return true;
#else
    return false;
#endif
}

bool wxVariantDataLongLong::Read([[maybe_unused]] std::istream& str)
{
    wxFAIL_MSG("Unimplemented");
    return false;
}

#if wxUSE_STREAMS
bool wxVariantDataLongLong::Write(wxOutputStream& str) const
{
    wxTextOutputStream s(str);
    s.Write32(m_value.GetLo());
    s.Write32(m_value.GetHi());
    return true;
}

bool wxVariantDataLongLong::Read(wxInputStream& str)
{
   wxTextInputStream s(str);
   unsigned long lo = s.Read32();
   long hi = s.Read32();
   m_value = wxLongLong(hi, lo);
   return true;
}
#endif // wxUSE_STREAMS

bool wxVariantDataLongLong::Read(wxString& str)
{
#ifdef wxLongLong_t
    wxLongLong_t value_t;
    if ( !str.ToLongLong(&value_t) )
        return false;
    m_value = value_t;
    return true;
#else
    return false;
#endif
}

// wxVariant

wxVariant::wxVariant(wxLongLong val, const wxString& name)
{
    m_refData = new wxVariantDataLongLong(val);
    m_name = name;
}

bool wxVariant::operator==(wxLongLong value) const
{
    wxLongLong thisValue;
    if ( !Convert(&thisValue) )
        return false;
    else
        return (value == thisValue);
}

bool wxVariant::operator!=(wxLongLong value) const
{
    return (!((*this) == value));
}

void wxVariant::operator=(wxLongLong value)
{
    if ( GetType() == "longlong" &&
         m_refData->GetRefCount() == 1 )
    {
        ((wxVariantDataLongLong*)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDataLongLong(value);
    }
}

wxLongLong wxVariant::GetLongLong() const
{
    wxLongLong value;
    if ( Convert(&value) )
    {
        return value;
    }
    else
    {
        wxFAIL_MSG("Could not convert to a long long");
        return 0;
    }
}

#endif // wxUSE_LONGLONG

// ----------------------------------------------------------------------------
// wxVariantDataULongLong
// ----------------------------------------------------------------------------

#if wxUSE_LONGLONG

class wxVariantDataULongLong : public wxVariantData
{
public:
    wxVariantDataULongLong() : m_value(0) { }
    explicit wxVariantDataULongLong(wxULongLong value) : m_value(value) { }

    wxULongLong GetValue() const { return m_value; }
    void SetValue(wxULongLong value) { m_value = value; }

    bool Eq(wxVariantData& data) const override;

    bool Read(wxString& str) override;
    bool Write(wxString& str) const override;
    bool Read(std::istream& str) override;
    bool Write(std::ostream& str) const override;
#if wxUSE_STREAMS
    virtual bool Read(wxInputStream& str);
    virtual bool Write(wxOutputStream &str) const;
#endif // wxUSE_STREAMS

    wxVariantData* Clone() const override
    {
        return new wxVariantDataULongLong(m_value);
    }

    wxString GetType() const override { return "ulonglong"; }

    DECLARE_WXANY_CONVERSION()
protected:
    wxULongLong m_value;
};

//
// wxULongLong type requires customized wxAny conversion code
//
#if wxUSE_ANY
#ifdef wxLongLong_t

bool wxVariantDataULongLong::GetAsAny(wxAny* any) const
{
    *any = m_value.GetValue();
    return true;
}

wxVariantData* wxVariantDataULongLong::VariantDataFactory(const wxAny& any)
{
    return new wxVariantDataULongLong(any.As<wxULongLong_t>());
}

REGISTER_WXANY_CONVERSION(wxULongLong_t, wxVariantDataULongLong)

#else // if !defined(wxLongLong_t)

bool wxVariantDataULongLong::GetAsAny(wxAny* any) const
{
    *any = m_value;
    return true;
}

wxVariantData* wxVariantDataULongLong::VariantDataFactory(const wxAny& any)
{
    return new wxVariantDataULongLong(any.As<wxULongLong>());
}

REGISTER_WXANY_CONVERSION(wxULongLong, wxVariantDataULongLong)

#endif // defined(wxLongLong_t)/!defined(wxLongLong_t)
#endif // wxUSE_ANY


bool wxVariantDataULongLong::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "ulonglong"),
                  "wxVariantDataULongLong::Eq: argument mismatch" );

    wxVariantDataULongLong& otherData = (wxVariantDataULongLong&) data;

    return (otherData.m_value == m_value);
}

bool wxVariantDataULongLong::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << (const char*) s.mb_str();
    return true;
}

bool wxVariantDataULongLong::Write(wxString& str) const
{
#ifdef wxLongLong_t
    str.Printf("%llu", m_value.GetValue());
    return true;
#else
    return false;
#endif
}

bool wxVariantDataULongLong::Read([[maybe_unused]] std::istream& str)
{
    wxFAIL_MSG("Unimplemented");
    return false;
}

#if wxUSE_STREAMS
bool wxVariantDataULongLong::Write(wxOutputStream& str) const
{
    wxTextOutputStream s(str);
    s.Write32(m_value.GetLo());
    s.Write32(m_value.GetHi());
    return true;
}

bool wxVariantDataULongLong::Read(wxInputStream& str)
{
   wxTextInputStream s(str);
   unsigned long lo = s.Read32();
   long hi = s.Read32();
   m_value = wxULongLong(hi, lo);
   return true;
}
#endif // wxUSE_STREAMS

bool wxVariantDataULongLong::Read(wxString& str)
{
#ifdef wxLongLong_t
    wxULongLong_t value_t;
    if ( !str.ToULongLong(&value_t) )
        return false;
    m_value = value_t;
    return true;
#else
    return false;
#endif
}

// wxVariant

wxVariant::wxVariant(wxULongLong val, const wxString& name)
{
    m_refData = new wxVariantDataULongLong(val);
    m_name = name;
}

bool wxVariant::operator==(wxULongLong value) const
{
    wxULongLong thisValue;
    if ( !Convert(&thisValue) )
        return false;
    else
        return (value == thisValue);
}

bool wxVariant::operator!=(wxULongLong value) const
{
    return (!((*this) == value));
}

void wxVariant::operator=(wxULongLong value)
{
    if ( GetType() == "ulonglong" &&
         m_refData->GetRefCount() == 1 )
    {
        ((wxVariantDataULongLong*)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDataULongLong(value);
    }
}

wxULongLong wxVariant::GetULongLong() const
{
    wxULongLong value;
    if ( Convert(&value) )
    {
        return value;
    }
    else
    {
        wxFAIL_MSG("Could not convert to a long long");
        return 0;
    }
}

#endif // wxUSE_LONGLONG

// ----------------------------------------------------------------------------
// wxVariantDataList
// ----------------------------------------------------------------------------

class wxVariantDataList: public wxVariantData
{
public:
    wxVariantDataList() = default;
    explicit wxVariantDataList(const wxVariantList& list);
    ~wxVariantDataList();

    wxVariantList& GetValue() { return m_value; }
    void SetValue(const wxVariantList& value) ;

    bool Eq(wxVariantData& data) const override;
    bool Write(std::ostream& str) const override;
    bool Write(wxString& str) const override;
    bool Read(std::istream& str) override;
    bool Read(wxString& str) override;
    wxString GetType() const override { return "list"; }

    void Clear();

    wxVariantData* Clone() const override { return new wxVariantDataList(m_value); }

    DECLARE_WXANY_CONVERSION()
protected:
    wxVariantList  m_value;
};

#if wxUSE_ANY

//
// Convert to/from list of wxAnys
//

bool wxVariantDataList::GetAsAny(wxAny* any) const
{
    wxAnyList dst;
    wxVariantList::compatibility_iterator node = m_value.GetFirst();
    while (node)
    {
        wxVariant* pVar = node->GetData();
        dst.push_back(new wxAny(((const wxVariant&)*pVar)));
        node = node->GetNext();
    }

    *any = dst;
    return true;
}

wxVariantData* wxVariantDataList::VariantDataFactory(const wxAny& any)
{
    wxAnyList src = any.As<wxAnyList>();
    wxVariantList dst;
    dst.DeleteContents(true);
    wxAnyList::compatibility_iterator node = src.GetFirst();
    while (node)
    {
        wxAny* pAny = node->GetData();
        dst.push_back(new wxVariant(*pAny));
        node = node->GetNext();
    }

    return new wxVariantDataList(dst);
}

REGISTER_WXANY_CONVERSION(wxAnyList, wxVariantDataList)

#endif // wxUSE_ANY

wxVariantDataList::wxVariantDataList(const wxVariantList& list)
{
    SetValue(list);
}

wxVariantDataList::~wxVariantDataList()
{
    Clear();
}

void wxVariantDataList::SetValue(const wxVariantList& value)
{
    Clear();
    wxVariantList::compatibility_iterator node = value.GetFirst();
    while (node)
    {
        wxVariant* var = node->GetData();
        m_value.Append(new wxVariant(*var));
        node = node->GetNext();
    }
}

void wxVariantDataList::Clear()
{
    wxVariantList::compatibility_iterator node = m_value.GetFirst();
    while (node)
    {
        wxVariant* var = node->GetData();
        delete var;
        node = node->GetNext();
    }
    m_value.Clear();
}

bool wxVariantDataList::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "list"), "wxVariantDataList::Eq: argument mismatch" );

    wxVariantDataList& listData = (wxVariantDataList&) data;
    wxVariantList::compatibility_iterator node1 = m_value.GetFirst();
    wxVariantList::compatibility_iterator node2 = listData.GetValue().GetFirst();
    while (node1 && node2)
    {
        wxVariant* var1 = node1->GetData();
        wxVariant* var2 = node2->GetData();
        if ((*var1) != (*var2))
            return false;
        node1 = node1->GetNext();
        node2 = node2->GetNext();
    }
    return !(node1 || node2);
}

bool wxVariantDataList::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << (const char*) s.mb_str();
    return true;
}

bool wxVariantDataList::Write(wxString& str) const
{
    str.clear();
    wxVariantList::compatibility_iterator node = m_value.GetFirst();
    while (node)
    {
        wxVariant* var = node->GetData();
        if (node != m_value.GetFirst())
          str += " ";
        wxString str1;
        str += var->MakeString();
        node = node->GetNext();
    }

    return true;
}

bool wxVariantDataList::Read([[maybe_unused]] std::istream& str)
{
    wxFAIL_MSG("Unimplemented");
    // TODO
    return false;
}

bool wxVariantDataList::Read([[maybe_unused]] wxString& str)
{
    wxFAIL_MSG("Unimplemented");
    // TODO
    return false;
}

// wxVariant

wxVariant::wxVariant(const wxVariantList& val, const wxString& name) // List of variants
{
    m_refData = new wxVariantDataList(val);
    m_name = name;
}

bool wxVariant::operator== (const wxVariantList& value) const
{
    wxASSERT_MSG( (GetType() == "list"), "Invalid type for == operator" );

    wxVariantDataList other(value);
    return (GetData()->Eq(other));
}

bool wxVariant::operator!= (const wxVariantList& value) const
{
    return (!((*this) == value));
}

void wxVariant::operator= (const wxVariantList& value)
{
    if (GetType() == "list" &&
        m_refData->GetRefCount() == 1)
    {
        ((wxVariantDataList*)GetData())->SetValue(value);
    }
    else
    {
        UnRef();
        m_refData = new wxVariantDataList(value);
    }
}

wxVariantList& wxVariant::GetList() const
{
    wxASSERT( (GetType() == "list") );

    return (wxVariantList&) ((wxVariantDataList*) m_refData)->GetValue();
}

// Make empty list
void wxVariant::NullList()
{
    SetData(new wxVariantDataList());
}

// Append to list
void wxVariant::Append(const wxVariant& value)
{
    wxVariantList& list = GetList();

    list.Append(new wxVariant(value));
}

// Insert at front of list
void wxVariant::Insert(const wxVariant& value)
{
    wxVariantList& list = GetList();

    list.Insert(new wxVariant(value));
}

// Returns true if the variant is a member of the list
bool wxVariant::Member(const wxVariant& value) const
{
    wxVariantList& list = GetList();

    wxVariantList::compatibility_iterator node = list.GetFirst();
    while (node)
    {
        wxVariant* other = node->GetData();
        if (value == *other)
            return true;
        node = node->GetNext();
    }
    return false;
}

// Deletes the nth element of the list
bool wxVariant::Delete(size_t item)
{
    wxVariantList& list = GetList();

    wxASSERT_MSG( (item < list.GetCount()), "Invalid index to Delete" );
    wxVariantList::compatibility_iterator node = list.Item(item);
    wxVariant* variant = node->GetData();
    delete variant;
    list.Erase(node);
    return true;
}

// Clear list
void wxVariant::ClearList()
{
    if (!IsNull() && (GetType() == "list"))
    {
        ((wxVariantDataList*) m_refData)->Clear();
    }
    else
    {
        if (!GetType().IsSameAs("list"))
            UnRef();

        m_refData = new wxVariantDataList;
    }
}

// Treat a list variant as an array
wxVariant wxVariant::operator[] (size_t idx) const
{
    wxASSERT_MSG( GetType() == "list", "Invalid type for array operator" );

    if (GetType() == "list")
    {
        wxVariantDataList* data = (wxVariantDataList*) m_refData;
        wxASSERT_MSG( (idx < data->GetValue().GetCount()), "Invalid index for array" );
        return *(data->GetValue().Item(idx)->GetData());
    }
    return wxNullVariant;
}

wxVariant& wxVariant::operator[] (size_t idx)
{
    // We can't return a reference to a variant for a string list, since the string
    // is actually stored as a char*, not a variant.

    wxASSERT_MSG( (GetType() == "list"), "Invalid type for array operator" );

    wxVariantDataList* data = (wxVariantDataList*) m_refData;
    wxASSERT_MSG( (idx < data->GetValue().GetCount()), "Invalid index for array" );

    return * (data->GetValue().Item(idx)->GetData());
}

// Return the number of elements in a list
size_t wxVariant::GetCount() const
{
    wxASSERT_MSG( GetType() == "list", "Invalid type for GetCount()" );

    if (GetType() == "list")
    {
        wxVariantDataList* data = (wxVariantDataList*) m_refData;
        return data->GetValue().GetCount();
    }
    return 0;
}

// ----------------------------------------------------------------------------
// Type conversion
// ----------------------------------------------------------------------------

bool wxVariant::Convert(long* value) const
{
    wxString type(GetType());
    if (type == "double")
        *value = (long) (((wxVariantDoubleData*)GetData())->GetValue());
    else if (type == "long")
        *value = ((wxVariantDataLong*)GetData())->GetValue();
    else if (type == "bool")
        *value = (long) (((wxVariantDataBool*)GetData())->GetValue());
    else if (type == "string")
        *value = wxAtol(((wxVariantDataString*)GetData())->GetValue());
#if wxUSE_LONGLONG
    else if (type == "longlong")
    {
        wxLongLong v = ((wxVariantDataLongLong*)GetData())->GetValue();
        // Don't convert if return value would be vague
        if ( v < LONG_MIN || v > LONG_MAX )
            return false;
        *value = v.ToLong();
    }
    else if (type == "ulonglong")
    {
        wxULongLong v = ((wxVariantDataULongLong*)GetData())->GetValue();
        // Don't convert if return value would be vague
        if ( v.GetHi() )
            return false;
        *value = (long) v.ToULong();
    }
#endif
    else
        return false;

    return true;
}

bool wxVariant::Convert(bool* value) const
{
    wxString type(GetType());
    if (type == "double")
        *value = ((int) (((wxVariantDoubleData*)GetData())->GetValue()) != 0);
    else if (type == "long")
        *value = (((wxVariantDataLong*)GetData())->GetValue() != 0);
    else if (type == "bool")
        *value = ((wxVariantDataBool*)GetData())->GetValue();
    else if (type == "string")
    {
        std::string val(((wxVariantDataString*)GetData())->GetValue());
        wx::utils::ToLower(val);

        if (val == "true" || val == "yes" || val == '1' )
            *value = true;
        else if (val == "false" || val == "no" || val == '0' )
            *value = false;
        else
            return false;
    }
    else
        return false;

    return true;
}

bool wxVariant::Convert(double* value) const
{
    wxString type(GetType());
    if (type == "double")
        *value = ((wxVariantDoubleData*)GetData())->GetValue();
    else if (type == "long")
        *value = (double) (((wxVariantDataLong*)GetData())->GetValue());
    else if (type == "bool")
        *value = (double) (((wxVariantDataBool*)GetData())->GetValue());
    else if (type == "string")
        *value = (double) wxAtof(((wxVariantDataString*)GetData())->GetValue());
#if wxUSE_LONGLONG
    else if (type == "longlong")
    {
        *value = ((wxVariantDataLongLong*)GetData())->GetValue().ToDouble();
    }
    else if (type == "ulonglong")
    {
        *value = ((wxVariantDataULongLong*)GetData())->GetValue().ToDouble();
    }
#endif
    else
        return false;

    return true;
}

bool wxVariant::Convert(wxUniChar* value) const
{
    wxString type(GetType());
    if (type == "char")
        *value = ((wxVariantDataChar*)GetData())->GetValue();
    else if (type == "long")
        *value = (char) (((wxVariantDataLong*)GetData())->GetValue());
    else if (type == "bool")
        *value = (char) (((wxVariantDataBool*)GetData())->GetValue());
    else if (type == "string")
    {
        // Also accept strings of length 1
        const wxString& str = (((wxVariantDataString*)GetData())->GetValue());
        if ( str.length() == 1 )
            *value = str[0];
        else
            return false;
    }
    else
        return false;

    return true;
}

bool wxVariant::Convert(char* value) const
{
    wxUniChar ch;
    if ( !Convert(&ch) )
        return false;
    *value = ch;
    return true;
}

bool wxVariant::Convert(wchar_t* value) const
{
    wxUniChar ch;
    if ( !Convert(&ch) )
        return false;
    *value = ch;
    return true;
}

bool wxVariant::Convert(wxString* value) const
{
    *value = MakeString();
    return true;
}

#if wxUSE_LONGLONG
bool wxVariant::Convert(wxLongLong* value) const
{
    wxString type(GetType());
    if (type == "longlong")
        *value = ((wxVariantDataLongLong*)GetData())->GetValue();
    else if (type == "long")
        *value = ((wxVariantDataLong*)GetData())->GetValue();
    else if (type == "string")
    {
        wxString s = ((wxVariantDataString*)GetData())->GetValue();
#ifdef wxLongLong_t
        wxLongLong_t value_t;
        if ( !s.ToLongLong(&value_t) )
            return false;
        *value = value_t;
#else
        long l_value;
        if ( !s.ToLong(&l_value) )
            return false;
        *value = l_value;
#endif
    }
    else if (type == "bool")
        *value = (long) (((wxVariantDataBool*)GetData())->GetValue());
    else if (type == "double")
    {
        value->Assign(((wxVariantDoubleData*)GetData())->GetValue());
    }
    else if (type == "ulonglong")
        *value = ((wxVariantDataULongLong*)GetData())->GetValue();
    else
        return false;

    return true;
}

bool wxVariant::Convert(wxULongLong* value) const
{
    wxString type(GetType());
    if (type == "ulonglong")
        *value = ((wxVariantDataULongLong*)GetData())->GetValue();
    else if (type == "long")
        *value = ((wxVariantDataLong*)GetData())->GetValue();
    else if (type == "string")
    {
        wxString s = ((wxVariantDataString*)GetData())->GetValue();
#ifdef wxLongLong_t
        wxULongLong_t value_t;
        if ( !s.ToULongLong(&value_t) )
            return false;
        *value = value_t;
#else
        unsigned long l_value;
        if ( !s.ToULong(&l_value) )
            return false;
        *value = l_value;
#endif
    }
    else if (type == "bool")
        *value = (long) (((wxVariantDataBool*)GetData())->GetValue());
    else if (type == "double")
    {
        double value_d = ((wxVariantDoubleData*)GetData())->GetValue();

        if ( value_d < 0.0 )
            return false;

#ifdef wxLongLong_t
        *value = (wxULongLong_t) value_d;
#else
        wxLongLong temp;
        temp.Assign(value_d);
        *value = temp;
#endif
    }
    else if (type == "longlong")
        *value = ((wxVariantDataLongLong*)GetData())->GetValue();
    else
        return false;

    return true;
}
#endif // wxUSE_LONGLONG

#if wxUSE_DATETIME
bool wxVariant::Convert(wxDateTime* value) const
{
    wxString type(GetType());
    if (type == "datetime")
    {
        *value = ((wxVariantDataDateTime*)GetData())->GetValue();
        return true;
    }

    // Fallback to string conversion
    wxString val;
    if ( !Convert(&val) )
        return false;

    // Try to parse this as either date and time, only date or only time
    // checking that the entire string was parsed
    wxString::const_iterator end;
    if ( value->ParseDateTime(val, &end) && end == val.end() )
        return true;

    if ( value->ParseDate(val, &end) && end == val.end() )
        return true;

    if ( value->ParseTime(val, &end) && end == val.end() )
        return true;

    return false;
}
#endif // wxUSE_DATETIME

#endif // wxUSE_VARIANT
