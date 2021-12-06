/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/statline.h
// Purpose:     a generic wxStaticLine class
// Author:      Vadim Zeitlin
// Created:     28.06.99
// Copyright:   (c) 1998 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_STATLINE_H_
#define _WX_GENERIC_STATLINE_H_

class wxStaticBox;

// ----------------------------------------------------------------------------
// wxStaticLine
// ----------------------------------------------------------------------------

class wxStaticLine : public wxStaticLineBase
{
public:
    // constructors and pseudo-constructors
    wxStaticLine() = default;

    wxStaticLine( wxWindow *parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint &pos = wxDefaultPosition,
                  const wxSize &size = wxDefaultSize,
                  unsigned int style = wxLI_HORIZONTAL,
                  std::string_view name = wxStaticLineNameStr)
    {
        Create(parent, id, pos, size, style, name);
    }

    ~wxStaticLine();

    bool Create( wxWindow *parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize,
                 unsigned int style = wxLI_HORIZONTAL,
                 std::string_view name = wxStaticLineNameStr);

    // it's necessary to override this wxWindow function because we
    // will want to return the main widget for m_statbox
    //
    WXWidget GetMainWidget() const;

    // override wxWindow methods to make things work
    virtual void DoSetSize(int x, int y, int width, int height,
                           unsigned int sizeFlags = wxSIZE_AUTO);
    virtual void DoMoveWindow(int x, int y, int width, int height);
protected:
    // we implement the static line using a static box
    wxStaticBox *m_statbox{nullptr};
};

#endif // _WX_GENERIC_STATLINE_H_

