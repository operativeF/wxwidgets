#ifndef _WX_ALIGNMENT_FLAGS_
#define _WX_ALIGNMENT_FLAGS_

#include "wx/bitflags.h"

enum class wxAlignment
{
    Invalid,
    None,
    Center,
    CenterHorizontal,
    Left = None,
    Top = None,
    Right,
    Bottom,
    CenterVertical,
    _max_size
};

using AlignmentFlags = InclBitfield<wxAlignment>;

#endif // _WX_ALIGNMENT_FLAGS_
