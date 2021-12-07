///////////////////////////////////////////////////////////////////////////////
// Name:        wx/ribbon/control.h
// Purpose:     Extension of wxControl with common ribbon methods
// Author:      Peter Cawley
// Modified by:
// Created:     2009-06-05
// Copyright:   (C) Peter Cawley
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RIBBON_CONTROL_H_
#define _WX_RIBBON_CONTROL_H_

#if wxUSE_RIBBON

#include "wx/control.h"

class wxRibbonBar;
class wxRibbonArtProvider;

class wxRibbonControl : public wxControl
{
public:
    wxRibbonControl() {  m_art = nullptr;  }

    wxRibbonControl(wxWindow *parent, wxWindowID id,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize, unsigned int style = 0,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = wxASCII_STR(wxControlNameStr))
    {
         m_art = nullptr; 

        Create(parent, id, pos, size, style, validator, name);
    }

    bool Create(wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, unsigned int style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxControlNameStr));

    virtual void SetArtProvider(wxRibbonArtProvider* art);
    wxRibbonArtProvider* GetArtProvider() const {return m_art;}

    virtual bool IsSizingContinuous() const {return true;}
    wxSize GetNextSmallerSize(wxOrientation direction, wxSize relative_to) const;
    wxSize GetNextLargerSize(wxOrientation direction, wxSize relative_to) const;
    wxSize GetNextSmallerSize(wxOrientation direction) const;
    wxSize GetNextLargerSize(wxOrientation direction) const;

    virtual bool Realize();
    bool Realise() {return Realize();}

    virtual wxRibbonBar* GetAncestorRibbonBar()const;

    // Finds the best width and height given the parent's width and height
    virtual wxSize GetBestSizeForParentSize([[maybe_unused]] const wxSize& parentSize) const { return GetBestSize(); }

protected:
    wxRibbonArtProvider* m_art;

    virtual wxSize DoGetNextSmallerSize(wxOrientation direction,
                                        wxSize relative_to) const;
    virtual wxSize DoGetNextLargerSize(wxOrientation direction,
                                       wxSize relative_to) const;

private:
    

#ifndef SWIG
    wxDECLARE_CLASS(wxRibbonControl);
#endif
};

using wxArrayRibbonControl = std::vector<wxRibbonControl*>;

#endif // wxUSE_RIBBON

#endif // _WX_RIBBON_CONTROL_H_
