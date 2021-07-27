/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fontdata.cpp
// Author:      Julian Smart
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"


#if wxUSE_FONTDLG || wxUSE_FONTPICKERCTRL

#include "wx/fontdata.h"

wxFontData::wxFontData(const wxFontData& data)
    : 
      m_fontColour(data.m_fontColour),
      m_showHelp(data.m_showHelp),
      m_allowSymbols(data.m_allowSymbols),
      m_enableEffects(data.m_enableEffects),
      m_initialFont(data.m_initialFont),
      m_chosenFont(data.m_chosenFont),
      m_minSize(data.m_minSize),
      m_maxSize(data.m_maxSize),
      m_encoding(data.m_encoding),
      m_encodingInfo(data.m_encodingInfo),
      m_restrictSelection(data.m_restrictSelection)
{
}

wxFontData& wxFontData::operator=(const wxFontData& data)
{
    if (&data != this)
    {
        m_fontColour        = data.m_fontColour;
        m_showHelp          = data.m_showHelp;
        m_allowSymbols      = data.m_allowSymbols;
        m_enableEffects     = data.m_enableEffects;
        m_initialFont       = data.m_initialFont;
        m_chosenFont        = data.m_chosenFont;
        m_minSize           = data.m_minSize;
        m_maxSize           = data.m_maxSize;
        m_encoding          = data.m_encoding;
        m_encodingInfo      = data.m_encodingInfo;
        m_restrictSelection = data.m_restrictSelection;
    }
    return *this;
}
#endif // wxUSE_FONTDLG || wxUSE_FONTPICKERCTRL

#if wxUSE_FONTDLG

#include "wx/fontdlg.h"

wxFont wxGetFontFromUser(wxWindow *parent, const wxFont& fontInit, const std::string& caption)
{
    wxFontData data;
    if ( fontInit.IsOk() )
    {
        data.SetInitialFont(fontInit);
    }

    wxFont fontRet;
    wxFontDialog dialog(parent, data);
    if (!caption.empty())
        dialog.SetTitle(caption);
    if ( dialog.ShowModal() == wxID_OK )
    {
        fontRet = dialog.GetFontData().GetChosenFont();
    }
    //else: leave it invalid

    return fontRet;
}
#endif // wxUSE_FONTDLG
