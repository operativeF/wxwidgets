
module;

export module WX.GDI.Flags;

import Utils.Bitfield;

export
{

//  Polygon filling mode
enum class wxPolygonFillMode
{
    OddEven,
    WindingRule
};


enum class wxEllipsizeFlags
{
    None,
    ProcessMnemonics,
    ExpandTabs,
    Default,
    _max_size
};

using EllipsizeFlags = InclBitfield<wxEllipsizeFlags>;

// NB: Don't change the order of these values, they're the same as in
//     PangoEllipsizeMode enum.
enum class wxEllipsizeMode
{
    None,
    Start,
    Middle,
    End
};

} // export
