/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/nonownedwnd.h
// Author:      Sean D'Epagnier
// Copyright:   (c) 2016 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_NONOWNEDWND_H_
#define _WX_QT_NONOWNEDWND_H_

// ----------------------------------------------------------------------------
// wxNonOwnedWindow contains code common to wx{Popup,TopLevel}Window in wxQT.
// ----------------------------------------------------------------------------

class wxNonOwnedWindow : public wxNonOwnedWindowBase
{
public:
    wxNonOwnedWindow();

protected:
    bool DoClearShape() override;
    bool DoSetRegionShape(const wxRegion& region) override;
#if wxUSE_GRAPHICS_CONTEXT
    bool DoSetPathShape(const wxGraphicsPath& path) override;
#endif // wxUSE_GRAPHICS_CONTEXT

    wxNonOwnedWindow(const wxNonOwnedWindow&) = delete;
	wxNonOwnedWindow& operator=(const wxNonOwnedWindow&) = delete;
};

#endif // _WX_QT_NONOWNEDWND_H_
