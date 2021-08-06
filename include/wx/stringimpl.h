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

#include "wx/defs.h"        // everybody should include this
#include "wx/chartype.h"    // for wxChar
#include "wx/wxcrtbase.h"   // for wxStrlen() etc.

#include <cstdlib>

// ---------------------------------------------------------------------------
// macros
// ---------------------------------------------------------------------------

// implementation only
#define   wxASSERT_VALID_INDEX(i) \
    wxASSERT_MSG( (size_t)(i) <= length(), wxT("invalid index in wxString") )


// ----------------------------------------------------------------------------
// global data
// ----------------------------------------------------------------------------

// global pointer to empty string
extern WXDLLIMPEXP_DATA_BASE(const wxChar*) wxEmptyString;
#if wxUSE_UNICODE_UTF8
// FIXME-UTF8: we should have only one wxEmptyString
extern WXDLLIMPEXP_DATA_BASE(const wxStringCharType*) wxEmptyStringImpl;
#endif


// ----------------------------------------------------------------------------
// deal with various build options
// ----------------------------------------------------------------------------

// we use STL-based string internally

#include <string>

using wxStdWideString = std::wstring;

#if wxUSE_UNICODE_WCHAR
    using wxStdString = wxStdWideString;
#else
    typedef std::string wxStdString;
#endif

using wxStringImpl = wxStdString;

// don't pollute the library user's name space
#undef wxASSERT_VALID_INDEX

#endif  // _WX_WXSTRINGIMPL_H__
