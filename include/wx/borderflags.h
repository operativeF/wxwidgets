#ifndef _WX_BORDERFLAGS_
#define _WX_BORDERFLAGS_

enum class wxBorder
{
    /*  this is different from wxBorder::None as by default the controls do have */
    /*  border */
    Default,
    None,
    Static,
    Simple,
    Raised,
    Sunken,
    Theme,
};

#endif // _WX_BORDERFLAGS_