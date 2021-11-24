/////////////////////////////////////////////////////////////////////////////
// Name:        wx/help.h
// Purpose:     wxHelpController base header
// Author:      wxWidgets Team
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HELP_H_BASE_
#define _WX_HELP_H_BASE_

#if wxUSE_HELP

#include "wx/helpbase.h"

#if defined(__WXMSW__)
    #include "wx/msw/helpchm.h"

    using wxHelpController = wxCHMHelpController;
#else // !MSW

#if wxUSE_WXHTML_HELP
    #include "wx/html/helpctrl.h"
    using wxHelpController = wxHtmlHelpController;
#else
    #include "wx/generic/helpext.h"
    using wxHelpController = wxExtHelpController;
#endif

#endif // MSW/!MSW

#endif // wxUSE_HELP

#endif
    // _WX_HELP_H_BASE_
