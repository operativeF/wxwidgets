module;

#include "wx/stream.h"

module WX.Image.CUR;

import WX.Image.ICO;

//-----------------------------------------------------------------------------
// wxCURHandler
//-----------------------------------------------------------------------------

#if wxUSE_STREAMS

bool wxCURHandler::DoCanRead(wxInputStream& stream)
{
    return CanReadICOOrCUR(&stream, 2 /*for identifying a cursor*/);
}

#endif // wxUSE_STREAMS