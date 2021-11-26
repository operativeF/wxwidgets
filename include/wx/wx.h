/////////////////////////////////////////////////////////////////////////////
// Name:        wx/wx.h
// Purpose:     wxWidgets central header including the most often used ones
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WX_H_
#define _WX_WX_H_

#include "wx/app.h"
#include "wx/dynarray.h"
#include "wx/event.h"
#include "wx/hash.h"
#include "wx/hashmap.h"
#include "wx/intl.h"
#include "wx/list.h"
#include "wx/log.h"
#include "wx/memory.h"
#include "wx/module.h"
#include "wx/object.h"
#include "wx/stopwatch.h"
#include "wx/stream.h"
#include "wx/string.h"
#include "wx/timer.h"
#include "wx/utils.h"
#include "wx/wxcrt.h"
#include "wx/wxcrtvararg.h"


#if wxUSE_GUI

#include "wx/bitmap.h"
#include "wx/brush.h"
#include "wx/button.h"
#include "wx/colour.h"
#include "wx/containr.h"
#include "wx/cursor.h"
#include "wx/dataobj.h"
#include "wx/dc.h"
#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/dcprint.h"
#include "wx/dcscreen.h"
#include "wx/dialog.h"
#include "wx/font.h"
#include "wx/frame.h"
#include "wx/gdicmn.h"
#include "wx/gdiobj.h"
#include "wx/icon.h"
#include "wx/menu.h"
#include "wx/menuitem.h"
#include "wx/msgdlg.h"
#include "wx/palette.h"
#include "wx/panel.h"
#include "wx/pen.h"
#include "wx/region.h"
#include "wx/settings.h"
#include "wx/toplevel.h"
#include "wx/window.h"

#include "wx/bmpbuttn.h"
#include "wx/checkbox.h"
#include "wx/checklst.h"
#include "wx/choicdlg.h"
#include "wx/choice.h"
#include "wx/combobox.h"
#include "wx/control.h"
#include "wx/ctrlsub.h"
#include "wx/dirdlg.h"
#include "wx/filedlg.h"
#include "wx/gauge.h"
#include "wx/layout.h"
#include "wx/listbox.h"
#include "wx/radiobox.h"
#include "wx/radiobut.h"
#include "wx/scrolbar.h"
#include "wx/scrolwin.h"
#include "wx/sizer.h"
#include "wx/slider.h"
#include "wx/statbmp.h"
#include "wx/statbox.h"
#include "wx/stattext.h"
#include "wx/statusbr.h"
#include "wx/textctrl.h"
#include "wx/textdlg.h"
#include "wx/toolbar.h"

// this one is included by exactly one file (mdi.cpp) during wx build so even
// although we keep it here for the library users, don't include it to avoid
// bloating the PCH and (worse) rebuilding the entire library when it changes
// when building the library itself
#ifndef WXBUILDING
#include "wx/mdi.h"
#endif

// always include, even if !wxUSE_VALIDATORS because we need wxDefaultValidator
#include "wx/validate.h"

#if wxUSE_VALIDATORS
#include "wx/valtext.h"
#endif // wxUSE_VALIDATORS

#endif // wxUSE_GUI

#endif // _WX_WX_H_
