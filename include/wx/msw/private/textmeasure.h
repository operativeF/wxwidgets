///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/textmeasure.h
// Purpose:     wxMSW-specific declaration of wxTextMeasure class
// Author:      Manuel Martin
// Created:     2012-10-05
// Copyright:   (c) 1997-2012 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PRIVATE_TEXTMEASURE_H_
#define _WX_MSW_PRIVATE_TEXTMEASURE_H_

import WX.WinDef;

// ----------------------------------------------------------------------------
// wxTextMeasure for MSW.
// ----------------------------------------------------------------------------

class wxTextMeasure : public wxTextMeasureBase
{
public:
    explicit wxTextMeasure(const wxDC *dc, const wxFont *font = nullptr)
        : wxTextMeasureBase(dc, font)
    {
        Init();
    }

    explicit wxTextMeasure(const wxWindow *win, const wxFont *font = nullptr)
        : wxTextMeasureBase(win, font)
    {
        Init();
    }

    wxTextMeasure(const wxTextMeasure&) = delete;
	wxTextMeasure& operator=(const wxTextMeasure&) = delete;

protected:
    void Init();

    void BeginMeasuring() override;
    void EndMeasuring() override;

    wxSize DoGetTextExtent(std::string_view string,
                               wxCoord *descent = nullptr,
                               wxCoord *externalLeading = nullptr) override;

    std::vector<int> DoGetPartialTextExtents(std::string_view text, double scaleX) override;



    // We use either the HDC of the provided wxDC or an HDC created for our
    // window.
    HDC m_hdc{nullptr};

    // If we change the font in BeginMeasuring(), we restore it to the old one
    // in EndMeasuring().
    WXHFONT m_hfontOld{nullptr};
};

#endif // _WX_MSW_PRIVATE_TEXTMEASURE_H_
