#ifndef _WX_DIRECTION_FLAGS_
#define _WX_DIRECTION_FLAGS_

#include "wx/bitflags.h"

enum class wxDirection
{
    Left,
    Right,
    Up,
    Down,
    Center,
    All,
    Top    = Up,
    Bottom = Down,
    North  = Up,
    South  = Down,
    West   = Left,
    East   = Right,
    _max_size
};

using DirectionFlags = InclBitfield<wxDirection>;

#endif // _WX_DIRECTION_FLAGS_
