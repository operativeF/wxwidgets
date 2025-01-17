///////////////////////////////////////////////////////////////////////////////
// Name:        src/ribbon/control.cpp
// Purpose:     Extension of wxControl with common ribbon methods
// Author:      Peter Cawley
// Modified by:
// Created:     2009-06-05
// Copyright:   (C) Peter Cawley
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_RIBBON

#include "wx/ribbon/control.h"
#include "wx/ribbon/bar.h"

#ifdef __WXMSW__
    #include "wx/msw/private.h"
#endif

wxIMPLEMENT_CLASS(wxRibbonControl, wxControl);

bool wxRibbonControl::Create(wxWindow *parent, wxWindowID id,
                    const wxPoint& pos,
                    const wxSize& size, unsigned int style,
                    const wxValidator& validator,
                    const wxString& name)
{
    if ( !wxControl::Create(parent, id, pos, size, style, validator, name) )
        return false;

    wxRibbonControl *ribbon_parent = dynamic_cast<wxRibbonControl*>(parent);
    if(ribbon_parent)
    {
        m_art = ribbon_parent->GetArtProvider();
    }

    return true;
}

void wxRibbonControl::SetArtProvider(wxRibbonArtProvider* art)
{
    m_art = art;
}

wxSize wxRibbonControl::DoGetNextSmallerSize(wxOrientation direction,
                                           wxSize size) const
{
    // Dummy implementation for code which doesn't check for IsSizingContinuous() == true
    wxSize minimum(GetMinSize());
    if((direction & wxHORIZONTAL) && size.x > minimum.x)
    {
        size.x--;
    }
    if((direction & wxVERTICAL) && size.y > minimum.y)
    {
        size.y--;
    }
    return size;
}

wxSize wxRibbonControl::DoGetNextLargerSize(wxOrientation direction,
                                          wxSize size) const
{
    // Dummy implementation for code which doesn't check for IsSizingContinuous() == true
    if(direction & wxHORIZONTAL)
    {
        size.x++;
    }
    if(direction & wxVERTICAL)
    {
        size.y++;
    }
    return size;
}

wxSize wxRibbonControl::GetNextSmallerSize(wxOrientation direction,
                                           wxSize relative_to) const
{
    return DoGetNextSmallerSize(direction, relative_to);
}

wxSize wxRibbonControl::GetNextLargerSize(wxOrientation direction,
                                          wxSize relative_to) const
{
    return DoGetNextLargerSize(direction, relative_to);
}

wxSize wxRibbonControl::GetNextSmallerSize(wxOrientation direction) const
{
    return DoGetNextSmallerSize(direction, GetSize());
}

wxSize wxRibbonControl::GetNextLargerSize(wxOrientation direction) const
{
    return DoGetNextLargerSize(direction, GetSize());
}

bool wxRibbonControl::Realize()
{
    return true;
}

wxRibbonBar* wxRibbonControl::GetAncestorRibbonBar()const
{
    for ( wxWindow* win = GetParent(); win; win = win->GetParent() )
    {
        wxRibbonBar* bar = dynamic_cast<wxRibbonBar*>(win);
        if ( bar )
            return bar;
    }

    return nullptr;
}

#endif // wxUSE_RIBBON
