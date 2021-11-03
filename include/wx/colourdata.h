/////////////////////////////////////////////////////////////////////////////
// Name:        wx/colourdata.h
// Author:      Julian Smart
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLOURDATA_H_
#define _WX_COLOURDATA_H_

#include "wx/colour.h"

#include <array>

class wxColourData
{
public:
    // number of custom colours we store
    static constexpr int NUM_CUSTOM = 16;

    wxColourData();

    void SetChooseFull(bool flag) { m_chooseFull = flag; }
    bool GetChooseFull() const { return m_chooseFull; }
    void SetChooseAlpha(bool flag) { m_chooseAlpha = flag; }
    bool GetChooseAlpha() const { return m_chooseAlpha; }
    void SetColour(const wxColour& colour) { m_dataColour = colour; }
    const wxColour& GetColour() const { return m_dataColour; }
    wxColour& GetColour() { return m_dataColour; }

    // SetCustomColour() modifies colours in an internal array of NUM_CUSTOM
    // custom colours;
    void SetCustomColour(int i, const wxColour& colour);
    wxColour GetCustomColour(int i) const;

    // Serialize the object to a string and restore it from it
    wxString ToString() const;
    bool FromString(const wxString& str);


    // public for backwards compatibility only: don't use directly
    std::array<wxColour, NUM_CUSTOM> m_custColours;

    wxColour        m_dataColour;
    bool            m_chooseFull{false};

protected:

#ifdef __WXOSX__
    // Under OSX, legacy wxColourDialog had opacity selector
    // (slider) always enabled, so for backward compatibilty
    // we should tell the dialog to enable it by default.
    bool m_chooseAlpha{true};
#else
    bool m_chooseAlpha{false};
#endif // __WXOSX__ / !__WXOSX__
};

#endif // _WX_COLOURDATA_H_
