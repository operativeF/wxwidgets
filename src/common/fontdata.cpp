/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fontdata.cpp
// Author:      Julian Smart
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"


#if wxUSE_FONTDLG || wxUSE_FONTPICKERCTRL

#include "wx/fontdata.h"

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
