#ifndef _WX_WINDOWFLAGS_
#define _WX_WINDOWFLAGS_

#include "wx/bitflags.h"

// style common to both wxFrame and wxDialog

/*  Clip children when painting, which reduces flicker in e.g. frames and */
/*  splitter windows, but can't be used in a panel where a static box must be */
/*  'transparent' (panel paints the background for it) */

enum class wxWindowFlags
{
    StayOnTop,
    Iconize,
    Minimize = Iconize,
    Maximize,
    CloseBox,
    SystemMenu,
    MinimizeBox,
    MaximizeBox,
    TinyCaption,
    ResizeBorder,
    VScroll,
    HScroll,
    Caption,
    ClipChildren,
    _max_size
};

using WindowFlags = InclBitfield<wxWindowFlags>;

constexpr WindowFlags wxDEFAULT_FRAME_STYLE = { wxWindowFlags::SystemMenu,
                                                wxWindowFlags::ResizeBorder,
                                                wxWindowFlags::MinimizeBox,
                                                wxWindowFlags::MaximizeBox,
                                                wxWindowFlags::CloseBox,
                                                wxWindowFlags::Caption,
                                                wxWindowFlags::ClipChildren };

#endif _WX_WINDOWFLAGS_