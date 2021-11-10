/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fontdata.h
// Author:      Julian Smart
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONTDATA_H_
#define _WX_FONTDATA_H_

#include "wx/font.h"
#include "wx/colour.h"
#include "wx/encinfo.h"

import Utils.Bitfield;

// Possible values for RestrictSelection() flags.
enum
{
    wxFONTRESTRICT_NONE         = 0,
    wxFONTRESTRICT_SCALABLE     = 1 << 0,
    wxFONTRESTRICT_FIXEDPITCH   = 1 << 1
};

class wxFontData
{
public:
    void SetAllowSymbols(bool flag) { m_allowSymbols = flag; }
    bool GetAllowSymbols() const { return m_allowSymbols; }

    void SetColour(const wxColour& colour) { m_fontColour = colour; }
    const wxColour& GetColour() const { return m_fontColour; }

    void SetShowHelp(bool flag) { m_showHelp = flag; }
    bool GetShowHelp() const { return m_showHelp; }

    void EnableEffects(bool flag) { m_enableEffects = flag; }
    bool GetEnableEffects() const { return m_enableEffects; }

    void RestrictSelection(unsigned int flags) { m_restrictSelection = flags; }
    int  GetRestrictSelection() const { return m_restrictSelection; }

    void SetInitialFont(const wxFont& font) { m_initialFont = font; }
    wxFont GetInitialFont() const { return m_initialFont; }

    void SetChosenFont(const wxFont& font) { m_chosenFont = font; }
    wxFont GetChosenFont() const { return m_chosenFont; }

    void SetRange(int minRange, int maxRange) { m_minSize = minRange; m_maxSize = maxRange; }

    // encoding info is split into 2 parts: the logical wxWin encoding
    // (wxFontEncoding) and a structure containing the native parameters for
    // it (wxNativeEncodingInfo)
    wxFontEncoding GetEncoding() const { return m_encoding; }
    void SetEncoding(wxFontEncoding encoding) { m_encoding = encoding; }

    wxNativeEncodingInfo& EncodingInfo() { return m_encodingInfo; }

private:
    wxNativeEncodingInfo m_encodingInfo;

public:
    // public for backwards compatibility only: don't use directly
    wxColour        m_fontColour;
    wxFont          m_initialFont;
    wxFont          m_chosenFont;

    int             m_minSize{0};
    int             m_maxSize{0};

private:
    int                  m_restrictSelection{wxFONTRESTRICT_NONE};
    wxFontEncoding       m_encoding{wxFONTENCODING_SYSTEM};

public:
    bool            m_showHelp{false};
    bool            m_allowSymbols{true};
    bool            m_enableEffects{true};
};

#endif // _WX_FONTDATA_H_
