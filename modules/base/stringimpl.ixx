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

module;

#include "wx/chartype.h"    // for wxChar
#include "wx/wxcrtbase.h"   // for wxStrlen() etc.

export module WX.Cmn.StringImpl;

import <string>;

export
{

// ----------------------------------------------------------------------------
// deal with various build options
// ----------------------------------------------------------------------------

// we use STL-based string internally

using wxStdWideString = std::wstring;

#if wxUSE_UNICODE_WCHAR
    using wxStdString = wxStdWideString;
#else
    using wxStdString = std::string;
#endif

using wxStringImpl = wxStdString;

} // export