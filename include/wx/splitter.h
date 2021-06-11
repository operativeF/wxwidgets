/////////////////////////////////////////////////////////////////////////////
// Name:        wx/splitter.h
// Purpose:     Base header for wxSplitterWindow
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SPLITTER_H_BASE_
#define _WX_SPLITTER_H_BASE_

#include "wx/event.h"

// ----------------------------------------------------------------------------
// wxSplitterWindow flags
// ----------------------------------------------------------------------------

inline constexpr int wxSP_NOBORDER         = 0x0000;
inline constexpr int wxSP_THIN_SASH        = 0x0000;    // NB: the default is 3D sash
inline constexpr int wxSP_NOSASH           = 0x0010;
inline constexpr int wxSP_PERMIT_UNSPLIT   = 0x0040;
inline constexpr int wxSP_LIVE_UPDATE      = 0x0080;
inline constexpr int wxSP_3DSASH           = 0x0100;
inline constexpr int wxSP_3DBORDER         = 0x0200;
inline constexpr int wxSP_NO_XP_THEME      = 0x0400;
inline constexpr int wxSP_BORDER           = wxSP_3DBORDER;
inline constexpr int wxSP_3D               = wxSP_3DBORDER | wxSP_3DSASH;

class WXDLLIMPEXP_FWD_CORE wxSplitterEvent;

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_SPLITTER_SASH_POS_CHANGED, wxSplitterEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_SPLITTER_SASH_POS_CHANGING, wxSplitterEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_SPLITTER_DOUBLECLICKED, wxSplitterEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_SPLITTER_UNSPLIT, wxSplitterEvent );

#include "wx/generic/splitter.h"

#endif // _WX_SPLITTER_H_BASE_
