///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/ole/dataform.h
// Purpose:     declaration of the wxDataFormat class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     19.10.99 (extracted from msw/ole/dataobj.h)
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef   _WX_MSW_OLE_DATAFORM_H
#define   _WX_MSW_OLE_DATAFORM_H

// ----------------------------------------------------------------------------
// wxDataFormat identifies the single format of data
// ----------------------------------------------------------------------------

class wxDataFormat
{
public:
    // the clipboard formats under Win32 are WXWORD's
    using NativeFormat = unsigned short;

    wxDataFormat(NativeFormat format = wxDF_INVALID) : m_format{format} {}

    // we need constructors from all string types as implicit conversions to
    // std::string don't apply when we already rely on implicit conversion of a,
    // for example, "char *" string to wxDataFormat, and existing code does it
    wxDataFormat(const std::string& format) { SetId(format); }
#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
    wxDataFormat(const char *format) { SetId(format); }
#endif // wxNO_IMPLICIT_WXSTRING_ENCODING

    wxDataFormat& operator=(NativeFormat format)
        { m_format = format; return *this; }

    // default copy ctor/assignment operators ok

    // comparison (must have both versions)
    bool operator==(wxDataFormatId format) const;
    bool operator!=(wxDataFormatId format) const;
    bool operator==(const wxDataFormat& format) const;
    bool operator!=(const wxDataFormat& format) const;
    bool operator==(NativeFormat format) const;
    bool operator!=(NativeFormat format) const;

    // explicit and implicit conversions to NativeFormat which is one of
    // standard data types (implicit conversion is useful for preserving the
    // compatibility with old code)
    NativeFormat GetFormatId() const { return m_format; }
    operator NativeFormat() const { return m_format; }

    // this works with standard as well as custom ids
    void SetType(NativeFormat format) { m_format = format; }
    NativeFormat GetType() const { return m_format; }

    // string ids are used for custom types - this SetId() must be used for
    // application-specific formats
    std::string GetId() const;
    void SetId(const std::string& format);

    // returns true if the format is one of those defined in wxDataFormatId
    bool IsStandard() const { return m_format > 0 && m_format < wxDF_PRIVATE; }

private:
    NativeFormat m_format;
};

#endif //  _WX_MSW_OLE_DATAFORM_H

