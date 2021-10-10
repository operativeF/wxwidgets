#ifndef _WX_DIALOGFLAGS_
#define _WX_DIALOGFLAGS_

#include "wx/bitflags.h"

enum class wxDialogFlags
{
    Centered,
    Yes,
    OK,
    No,
    Yes_No,
    Cancel,
    Apply,
    Close,
    Help,
    Forward,
    Backward,
    Reset,
    More,
    Setup,
    _max_size
};

enum class wxDialogDefaultFlags
{
    Yes,
    OK = Yes,
    Cancel,
    No,
    _max_size
};

enum class wxDialogIconFlags
{
    None,
    Exclamation,
    Hand,
    Warning = Exclamation,
    Error = Hand,
    Question,
    Information,
    Stop = Hand,
    Asterisk = Information,
    AuthNeeded,
    _max_size
};

using DialogFlags = CombineBitfield<wxDialogFlags, wxDialogDefaultFlags, wxDialogIconFlags>;

// FIXME: Need to extract WindowFlags
constexpr DialogFlags wxDEFAULT_DIALOG_STYLE = {};

#endif // _WX_DIALOGFLAGS_
