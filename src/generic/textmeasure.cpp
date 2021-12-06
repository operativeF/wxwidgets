///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/textmeasure.cpp
// Purpose:
// Author:      Vadim Zeitlin
// Created:     2012-10-17
// Copyright:   (c) 2012 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/window.h"
#include "wx/dc.h"

#include "wx/private/textmeasure.h"

#if wxUSE_GENERIC_TEXTMEASURE

// ============================================================================
// wxTextMeasure generic implementation
// ============================================================================

// We assume that the ports not providing platform-specific wxTextMeasure
// implementation implement the corresponding functions in their wxDC and
// wxWindow classes, so forward back to them instead of using wxTextMeasure
// from there, as usual.
void wxTextMeasure::DoGetTextExtent(std::string_view string,
                                    wxCoord *width,
                                    wxCoord *height,
                                    wxCoord *descent,
                                    wxCoord *externalLeading)
{
    if ( m_dc )
    {
        m_dc->GetTextExtent(string, width, height,
                            descent, externalLeading, m_font);
    }
    else if ( m_win )
    {
        m_win->GetTextExtent(string, width, height,
                             descent, externalLeading, m_font);
    }
    //else: we already asserted in the ctor, don't do it any more
}

std::vector<int> wxTextMeasure::DoGetPartialTextExtents(std::string_view text, double scaleX)
{
    return wxTextMeasureBase::DoGetPartialTextExtents(text, scaleX);
}

#endif // wxUSE_GENERIC_TEXTMEASURE
