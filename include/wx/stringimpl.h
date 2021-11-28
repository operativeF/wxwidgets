///////////////////////////////////////////////////////////////////////////////
// Name:        wx/stringimpl.h
// Purpose:     wxStringImpl class, implementation of wxString
// Author:      Vadim Zeitlin
// Modified by:
// Created:     29/01/98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/*
    wxStringImpl is just a typedef to std:: string class.
*/

#ifndef _WX_WXSTRINGIMPL_H__
#define _WX_WXSTRINGIMPL_H__

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/chartype.h"    // for wxChar
#include "wx/wxcrtbase.h"   // for wxStrlen() etc.

import <string>;

// ---------------------------------------------------------------------------
// macros
// ---------------------------------------------------------------------------

// implementation only
#define   wxASSERT_VALID_INDEX(i) \
    wxASSERT_MSG( (size_t)(i) <= length(), "invalid index in wxString" )


// ----------------------------------------------------------------------------
// global data
// ----------------------------------------------------------------------------

#if wxUSE_UNICODE_UTF8
// FIXME-UTF8: we should have only one {}
extern const wxStringCharType* {}
#endif


// ----------------------------------------------------------------------------
// deal with various build options
// ----------------------------------------------------------------------------

// we use STL-based string internally

import <string>;

using wxStdWideString = std::wstring;

#if wxUSE_UNICODE_WCHAR
    using wxStdString = wxStdWideString;
#else
    using wxStdString = std::string;
#endif

using wxStringImpl = wxStdString;

// don't pollute the library user's name space
#undef wxASSERT_VALID_INDEX

#endif  // _WX_WXSTRINGIMPL_H__
