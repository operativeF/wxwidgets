/////////////////////////////////////////////////////////////////////////////
// Name:        wx/toolbar.h
// Purpose:     wxToolBar interface declaration
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.11.99
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TOOLBAR_H_BASE_
#define _WX_TOOLBAR_H_BASE_

import WX.Cfg.Flags;

#if wxUSE_TOOLBAR
    #include "wx/tbarbase.h"     // the base class for all toolbars

    #if defined(__WXUNIVERSAL__)
       #include "wx/univ/toolbar.h"
    #elif defined(__WXMSW__)
       #include "wx/msw/toolbar.h"
    #elif defined(__WXMOTIF__)
       #include "wx/motif/toolbar.h"
    #elif defined(__WXGTK20__)
        #include "wx/gtk/toolbar.h"
    #elif defined(__WXGTK__)
        #include "wx/gtk1/toolbar.h"
    #elif defined(__WXMAC__)
       #include "wx/osx/toolbar.h"
    #elif defined(__WXQT__)
        #include "wx/qt/toolbar.h"
    #endif
#endif // wxUSE_TOOLBAR

#endif
    // _WX_TOOLBAR_H_BASE_
