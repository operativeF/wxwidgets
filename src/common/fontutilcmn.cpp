/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fontutilcmn.cpp
// Purpose:     Font helper functions common for all ports
// Author:      Vaclav Slavik
// Modified by:
// Created:     2006-12-20
// Copyright:   (c) Vadim Zeitlin, Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// implementation
// ============================================================================

#ifdef wxHAS_UTF8_FONTS

#include "wx/fontutil.h"
#include "wx/encinfo.h"

// ----------------------------------------------------------------------------
// wxNativeEncodingInfo
// ----------------------------------------------------------------------------

bool wxNativeEncodingInfo::FromString(const wxString& WXUNUSED(s))
{
    return false;
}

wxString wxNativeEncodingInfo::ToString() const
{
    return {};
}

bool wxTestFontEncoding(const wxNativeEncodingInfo& WXUNUSED(info))
{
    return true;
}

bool wxGetNativeFontEncoding(wxFontEncoding encoding,
                             wxNativeEncodingInfo *info)
{
    // all encodings are available because we translate text in any encoding to
    // UTF-8 internally anyhow
    info->facename.clear();
    info->encoding = encoding;

    return true;
}

#endif // wxHAS_UTF8_FONTS
