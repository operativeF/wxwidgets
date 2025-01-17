///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/private/textmeasure.h
// Purpose:     Generic wxTextMeasure declaration.
// Author:      Vadim Zeitlin
// Created:     2012-10-17
// Copyright:   (c) 1997-2012 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_PRIVATE_TEXTMEASURE_H_
#define _WX_GENERIC_PRIVATE_TEXTMEASURE_H_

// ----------------------------------------------------------------------------
// wxTextMeasure for the platforms without native support.
// ----------------------------------------------------------------------------

class wxTextMeasure : public wxTextMeasureBase
{
public:
    explicit wxTextMeasure(const wxDC *dc, const wxFont *font = NULL)
        : wxTextMeasureBase(dc, font) {}
    explicit wxTextMeasure(const wxWindow *win, const wxFont *font = NULL)
        : wxTextMeasureBase(win, font) {}

protected:
    virtual void DoGetTextExtent(const std::string& string,
                               wxCoord *width,
                               wxCoord *height,
                               wxCoord *descent = NULL,
                               wxCoord *externalLeading = NULL) override;

    virtual std::vector<int> DoGetPartialTextExtents(std::string_view text, double scaleX) override;

    wxTextMeasure(const wxTextMeasure&) = delete;
	wxTextMeasure& operator=(const wxTextMeasure&) = delete;
};

#endif // _WX_GENERIC_PRIVATE_TEXTMEASURE_H_
