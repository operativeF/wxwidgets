///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/ole/oleutils.cpp
// Purpose:     implementation of OLE helper functions
// Author:      Vadim Zeitlin
// Modified by:
// Created:     19.02.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_OLE

#ifndef __CYGWIN10__

#include "wx/msw/private.h"

#include  "wx/msw/ole/oleutils.h"
#include "wx/msw/ole/safearray.h"

// ============================================================================
// Implementation
// ============================================================================

BSTR wxConvertStringToOle(const wxString& str)
{
    return wxBasicString(str).Detach();
}

wxString wxConvertStringFromOle(BSTR bStr)
{
    // NULL BSTR is equivalent to an empty string (this is the convention used
    // by VB and hence we must follow it)
    if ( !bStr )
        return {};

    const int len = SysStringLen(bStr);

    wxString str(bStr, len);

    return str;
}

// ----------------------------------------------------------------------------
// wxBasicString
// ----------------------------------------------------------------------------
void wxBasicString::AssignFromString(const wxString& str)
{
    SysFreeString(m_bstrBuf);
    m_bstrBuf = SysAllocString(str.wc_str(*wxConvCurrent));
}

BSTR wxBasicString::Detach()
{
    BSTR bstr = m_bstrBuf;

    m_bstrBuf = nullptr;

    return bstr;
}

BSTR* wxBasicString::ByRef()
{
    wxASSERT_MSG(!m_bstrBuf, "Can't get direct access to initialized BSTR");
    return &m_bstrBuf;
}

wxBasicString& wxBasicString::operator=(const wxBasicString& src)
{
    if ( this != &src )
    {
        wxCHECK_MSG(m_bstrBuf == nullptr || m_bstrBuf != src.m_bstrBuf,
            *this, "Attempting to assign already owned BSTR");
        SysFreeString(m_bstrBuf);
        m_bstrBuf = src.Copy();
    }

    return *this;
}

// ----------------------------------------------------------------------------
// Convert variants
// ----------------------------------------------------------------------------

#if wxUSE_VARIANT

// ----------------------------------------------------------------------------
// wxVariantDataCurrency
// ----------------------------------------------------------------------------


#if wxUSE_ANY

bool wxVariantDataCurrency::GetAsAny(wxAny* any) const
{
    *any = m_value;
    return true;
}

wxVariantData* wxVariantDataCurrency::VariantDataFactory(const wxAny& any)
{
    return new wxVariantDataCurrency(any.As<CURRENCY>());
}

REGISTER_WXANY_CONVERSION(CURRENCY, wxVariantDataCurrency)

#endif // wxUSE_ANY

bool wxVariantDataCurrency::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "currency"),
                  "wxVariantDataCurrency::Eq: argument mismatch" );

    wxVariantDataCurrency& otherData = (wxVariantDataCurrency&) data;

    return otherData.m_value.int64 == m_value.int64;
}

bool wxVariantDataCurrency::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << s;
    return true;
}

bool wxVariantDataCurrency::Write(wxString& str) const
{
    BSTR bStr = nullptr;
    if ( SUCCEEDED(VarBstrFromCy(m_value, LOCALE_USER_DEFAULT, 0, &bStr)) )
    {
        str = wxConvertStringFromOle(bStr);
        SysFreeString(bStr);
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
// wxVariantDataErrorCode
// ----------------------------------------------------------------------------

#if wxUSE_ANY

bool wxVariantDataErrorCode::GetAsAny(wxAny* any) const
{
    *any = m_value;
    return true;
}

wxVariantData* wxVariantDataErrorCode::VariantDataFactory(const wxAny& any)
{
    return new wxVariantDataErrorCode(any.As<SCODE>());
}

REGISTER_WXANY_CONVERSION(SCODE, wxVariantDataErrorCode)

#endif // wxUSE_ANY

bool wxVariantDataErrorCode::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "errorcode"),
                  "wxVariantDataErrorCode::Eq: argument mismatch" );

    wxVariantDataErrorCode& otherData = (wxVariantDataErrorCode&) data;

    return otherData.m_value == m_value;
}

bool wxVariantDataErrorCode::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << s;
    return true;
}

bool wxVariantDataErrorCode::Write(wxString& str) const
{
    str << m_value;
    return true;
}


// ----------------------------------------------------------------------------
// wxVariantDataSafeArray
// ----------------------------------------------------------------------------

#if wxUSE_ANY

bool wxVariantDataSafeArray::GetAsAny(wxAny* any) const
{
    *any = m_value;
    return true;
}

wxVariantData* wxVariantDataSafeArray::VariantDataFactory(const wxAny& any)
{
    return new wxVariantDataSafeArray(any.As<SAFEARRAY*>());
}

REGISTER_WXANY_CONVERSION(SAFEARRAY*, wxVariantDataSafeArray)

#endif // wxUSE_ANY

bool wxVariantDataSafeArray::Eq(wxVariantData& data) const
{
    wxASSERT_MSG( (data.GetType() == "safearray"),
                  "wxVariantDataSafeArray::Eq: argument mismatch" );

    wxVariantDataSafeArray& otherData = (wxVariantDataSafeArray&) data;

    return otherData.m_value == m_value;
}

bool wxVariantDataSafeArray::Write(std::ostream& str) const
{
    wxString s;
    Write(s);
    str << s;
    return true;
}

bool wxVariantDataSafeArray::Write(wxString& str) const
{
    str.Printf("SAFEARRAY: %p", (void*)m_value);
    return true;
}

bool wxConvertVariantToOle(const wxVariant& variant, VARIANTARG& oleVariant)
{
    VariantInit(&oleVariant);
    if (variant.IsNull())
    {
        oleVariant.vt = VT_NULL;
        return true;
    }

    wxString type(variant.GetType());

    if (type == "errorcode")
    {
        wxVariantDataErrorCode* const
            ec = wxStaticCastVariantData(variant.GetData(),
                                         wxVariantDataErrorCode);
        oleVariant.vt = VT_ERROR;
        oleVariant.scode = ec->GetValue();
    }
    else if (type == "currency")
    {
        wxVariantDataCurrency* const
            c = wxStaticCastVariantData(variant.GetData(),
                                        wxVariantDataCurrency);
        oleVariant.vt = VT_CY;
        oleVariant.cyVal = c->GetValue();
    }
    else if (type == "safearray")
    {
        wxVariantDataSafeArray* const
            vsa = wxStaticCastVariantData(variant.GetData(),
                                          wxVariantDataSafeArray);
        SAFEARRAY* psa = vsa->GetValue();
        VARTYPE vt;

        wxCHECK(psa, false);
        HRESULT hr = SafeArrayGetVartype(psa, &vt);
        if ( FAILED(hr) )
        {
            wxLogApiError("SafeArrayGetVartype()", hr);
            SafeArrayDestroy(psa);
            return false;
        }
        oleVariant.vt = vt | VT_ARRAY;
        oleVariant.parray = psa;
    }
    else if (type == "long")
    {
        oleVariant.vt = VT_I4;
        oleVariant.lVal = variant.GetLong() ;
    }
#if wxUSE_LONGLONG
    else if (type == "longlong")
    {
        oleVariant.vt = VT_I8;
        oleVariant.llVal = variant.GetLongLong().GetValue();
    }
#endif
    else if (type == "char")
    {
        oleVariant.vt=VT_I1;            // Signed Char
        oleVariant.cVal=variant.GetChar();
    }
    else if (type == "double")
    {
        oleVariant.vt = VT_R8;
        oleVariant.dblVal = variant.GetDouble();
    }
    else if (type == "bool")
    {
        oleVariant.vt = VT_BOOL;
        oleVariant.boolVal = variant.GetBool() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (type == "string")
    {
        wxString str( variant.GetString() );
        oleVariant.vt = VT_BSTR;
        oleVariant.bstrVal = wxConvertStringToOle(str);
    }
#if wxUSE_DATETIME
    else if (type == "datetime")
    {
        wxDateTime date( variant.GetDateTime() );
        oleVariant.vt = VT_DATE;

        SYSTEMTIME st;
        date.GetAsMSWSysTime(&st);

        SystemTimeToVariantTime(&st, &oleVariant.date);
    }
#endif
    else if (type == "void*")
    {
        oleVariant.vt = VT_DISPATCH;
        oleVariant.pdispVal = (IDispatch*) variant.GetVoidPtr();
    }
    else if (type == "list")
    {
        wxSafeArray<VT_VARIANT> safeArray;
        if (!safeArray.CreateFromListVariant(variant))
            return false;

        oleVariant.vt = VT_VARIANT | VT_ARRAY;
        oleVariant.parray = safeArray.Detach();
    }
    else if (type == "arrstring")
    {
        wxSafeArray<VT_BSTR> safeArray;

        if (!safeArray.CreateFromArrayString(variant.GetArrayString()))
            return false;

        oleVariant.vt = VT_BSTR | VT_ARRAY;
        oleVariant.parray = safeArray.Detach();
    }
    else
    {
        oleVariant.vt = VT_NULL;
        return false;
    }
    return true;
}

bool
wxConvertOleToVariant(const VARIANTARG& oleVariant, wxVariant& variant, long flags)
{
    bool ok = true;
    if ( oleVariant.vt & VT_ARRAY )
    {
        if ( flags & wxOleConvertVariant_ReturnSafeArrays  )
        {
            variant.SetData(new wxVariantDataSafeArray(oleVariant.parray));
        }
        else
        {
            switch (oleVariant.vt & VT_TYPEMASK)
            {
                case VT_I2:
                    ok = wxSafeArray<VT_I2>::ConvertToVariant(oleVariant.parray, variant);
                    break;
                case VT_I4:
                    ok = wxSafeArray<VT_I4>::ConvertToVariant(oleVariant.parray, variant);
                    break;
                case VT_R4:
                    ok = wxSafeArray<VT_R4>::ConvertToVariant(oleVariant.parray, variant);
                    break;
                case VT_R8:
                    ok = wxSafeArray<VT_R8>::ConvertToVariant(oleVariant.parray, variant);
                    break;
                case VT_VARIANT:
                    ok = wxSafeArray<VT_VARIANT>::ConvertToVariant(oleVariant.parray, variant);
                    break;
                case VT_BSTR:
                    {
                        std::vector<wxString> strings;
                        if ( wxSafeArray<VT_BSTR>::ConvertToArrayString(oleVariant.parray, strings) )
                            variant = strings;
                        else
                            ok = false;
                    }
                    break;
                default:
                    ok = false;
                    break;
            }
            if ( !ok )
            {
                wxLogDebug("unhandled VT_ARRAY type %x in wxConvertOleToVariant",
                           oleVariant.vt & VT_TYPEMASK);
                variant = wxVariant();
            }
        }
    }
    else if ( oleVariant.vt & VT_BYREF )
    {
        switch ( oleVariant.vt & VT_TYPEMASK )
        {
            case VT_VARIANT:
                {
                    VARIANTARG& oleReference = *((LPVARIANT)oleVariant.byref);
                    if (!wxConvertOleToVariant(oleReference,variant))
                        return false;
                    break;
                }

            default:
                wxLogError("wxAutomationObject::ConvertOleToVariant: [as yet] unhandled reference %X",
                            oleVariant.vt);
                return false;
        }
    }
    else // simply type (not array or reference)
    {
        switch ( oleVariant.vt & VT_TYPEMASK )
        {
            case VT_ERROR:
                variant.SetData(new wxVariantDataErrorCode(oleVariant.scode));
                break;

            case VT_CY:
                variant.SetData(new wxVariantDataCurrency(oleVariant.cyVal));
                break;

            case VT_BSTR:
                {
                    wxString str(wxConvertStringFromOle(oleVariant.bstrVal));
                    variant = str;
                }
                break;

            case VT_DATE:
#if wxUSE_DATETIME
                {
                    SYSTEMTIME st;
                    VariantTimeToSystemTime(oleVariant.date, &st);

                    wxDateTime date;
                    date.SetFromMSWSysTime(st);
                    variant = date;
                }
#endif // wxUSE_DATETIME
                break;

#if wxUSE_LONGLONG
            case VT_I8:
                variant = std::int64_t(oleVariant.llVal);
                break;
#endif // wxUSE_LONGLONG

            case VT_I4:
            case VT_UI4:
                variant = (long) oleVariant.lVal;
                break;

            case VT_I2:
            case VT_UI2:
                variant = (long) oleVariant.iVal;
                break;

            case VT_BOOL:
                variant = oleVariant.boolVal != 0;
                break;

            case VT_R4:
                variant = oleVariant.fltVal;
                break;

            case VT_R8:
                variant = oleVariant.dblVal;
                break;

            case VT_DISPATCH:
                variant = (void*) oleVariant.pdispVal;
                break;

            case VT_NULL:
            case VT_EMPTY:
                variant.MakeNull();
                break;

            default:
                wxLogError("wxAutomationObject::ConvertOleToVariant: Unknown variant value type %X -> %X",
                           oleVariant.vt,oleVariant.vt&VT_TYPEMASK);
                return false;
        }
    }

    return ok;
}

#endif // wxUSE_VARIANT

#endif // __CYGWIN10__

#endif // wxUSE_OLE
