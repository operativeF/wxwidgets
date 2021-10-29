///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/safearrayconverttest.cpp
// Purpose:     Test conversions between wxVariant and OLE VARIANT using SAFEARRAYs
// Author:      PB
// Copyright:   (c) the wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#ifdef WX_WINDOWS

#if wxUSE_OLE && wxUSE_VARIANT

#include "wx/msw/ole/oleutils.h"
#include "wx/msw/ole/safearray.h"

// need this to be able to use CHECK_EQ with wxVariant objects
inline std::ostream& operator<<(std::ostream& ostr, const wxVariant& v)
{
    ostr << v.GetString();
    return ostr;
}

// test converting a wxVariant with the list type to an OLE VARIANT
// and back to wxVariant the list type
TEST_CASE("VariantListDefault")
{
    wxVariant variant;
    VARIANT oleVariant;

    variant.NullList();
    variant.Append(true);
    variant.Append(12.34);
    variant.Append(42L);
    variant.Append("ABC");
    CHECK( wxConvertVariantToOle(variant, oleVariant) );

    wxVariant variantCopy;

    CHECK( wxConvertOleToVariant(oleVariant, variantCopy) );
    CHECK( variant == variantCopy );
}

// test converting a wxVariant with the arrstring type to an OLE VARIANT
// and back to a wxVariant with the arrstring type
TEST_CASE("VariantStringsDefault")
{
    wxVariant variant;
    std::vector<wxString> as;
    VARIANT oleVariant;

    as.push_back("abc");
    as.push_back("def");
    as.push_back("ghi");
    variant = as;
    CHECK( wxConvertVariantToOle(variant, oleVariant) );

    wxVariant variantCopy;

    CHECK( wxConvertOleToVariant(oleVariant, variantCopy) );
    CHECK( variant == variantCopy );
}

// test converting a wxVariant with the list type to an OLE VARIANT
// and then to a wxVariant with the safearray type
TEST_CASE("VariantListReturnSafeArray")
{
    wxVariant variant;
    VARIANT oleVariant;

    variant.NullList();
    variant.Append(true);
    variant.Append(12.34);
    variant.Append(42L);
    variant.Append("test");
    CHECK( wxConvertVariantToOle(variant, oleVariant) );

    wxVariant variantCopy;

    CHECK(
        wxConvertOleToVariant(oleVariant, variantCopy,
                              wxOleConvertVariant_ReturnSafeArrays)
    );
    CHECK( variantCopy.GetType() == wxT("safearray") );

    wxSafeArray<VT_VARIANT> safeArray;
    wxVariantDataSafeArray*
        vsa = wxStaticCastVariantData(variantCopy.GetData(),
                                      wxVariantDataSafeArray);
    long bound wxDUMMY_INITIALIZE(0);

    CHECK( vsa );
    CHECK( safeArray.Attach(vsa->GetValue()) );
    CHECK_EQ( 1, safeArray.GetDim() );
    CHECK( safeArray.GetLBound(1, bound) );
    CHECK_EQ( 0, bound );
    CHECK( safeArray.GetUBound(1, bound) );

    const std::size_t count = variant.GetCount();

    // bound + 1 because safearray elements are accessed by index ranging from
    // LBound to UBound inclusive
    CHECK_EQ( bound + 1, count );

    wxVariant variantItem;

    for ( long i = 0; i < count; i++ )
    {
        CHECK( safeArray.GetElement(&i, variantItem) );
        CHECK_EQ( variantItem, variant[i] );
    }
}

// test converting a wxArrayString to an OLE VARIANT
// and then to a wxVariant with the safearray type
TEST_CASE("StringsReturnSafeArray")
{
    std::vector<wxString> as;
    wxSafeArray<VT_BSTR> safeArray;

    as.push_back("abc");
    as.push_back("def");
    as.push_back("ghi");
    CHECK( safeArray.CreateFromArrayString(as) );

    VARIANT oleVariant;
    wxVariant variant;

    oleVariant.vt = VT_BSTR | VT_ARRAY;
    oleVariant.parray = safeArray.Detach();
    CHECK( oleVariant.parray );
    CHECK(
        wxConvertOleToVariant(oleVariant, variant,
                              wxOleConvertVariant_ReturnSafeArrays)
    );
    CHECK( variant.GetType() == wxT("safearray") );

    wxVariantDataSafeArray*
        vsa = wxStaticCastVariantData(variant.GetData(),
                                      wxVariantDataSafeArray);
    long bound wxDUMMY_INITIALIZE(0);

    CHECK( vsa );
    CHECK( safeArray.Attach(vsa->GetValue()) );
    CHECK_EQ( 1, safeArray.GetDim() );
    CHECK( safeArray.GetLBound(1, bound) );
    CHECK_EQ( 0, bound );
    CHECK( safeArray.GetUBound(1, bound) );

    const long count = as.size();
    CHECK_EQ( bound + 1, count );

    wxString str;

    for ( long i = 0; i < count; i++ )
    {
        CHECK( safeArray.GetElement(&i, str) );
        CHECK( str == as[i] );
    }
}

#endif // WX_WINDOWS

#endif // wxUSE_OLE && wxUSE_VARIANT
