/////////////////////////////////////////////////////////////////////////////
// Name:        src/gtk1/gauge.cpp
// Purpose:
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////



#if wxUSE_GAUGE

#include "wx/gauge.h"

#include <gtk/gtk.h>

//-----------------------------------------------------------------------------
// wxGauge
//-----------------------------------------------------------------------------

bool wxGauge::Create( wxWindow *parent,
                      wxWindowID id,
                      int range,
                      const wxPoint& pos,
                      const wxSize& size,
                      long style,
                      const wxValidator& validator,
                      const wxString& name )
{
    m_needParent = true;

    if (!PreCreation( parent, pos, size ) ||
        !CreateBase( parent, id, pos, size, style, validator, name ))
    {
        wxFAIL_MSG( wxT("wxGauge creation failed") );
        return false;
    }

    m_rangeMax = range;

    m_widget = gtk_progress_bar_new();
    if ( style & wxGA_VERTICAL )
    {
        gtk_progress_bar_set_orientation( GTK_PROGRESS_BAR(m_widget),
                                          GTK_PROGRESS_BOTTOM_TO_TOP );
    }

    m_parent->DoAddChild( this );

    PostCreation(size);
    SetInitialSize(size);

    return true;
}

void wxGauge::DoSetGauge()
{
    wxASSERT_MSG( 0 <= m_gaugePos && m_gaugePos <= m_rangeMax,
                  wxT("invalid gauge position in DoSetGauge()") );

    gtk_progress_bar_update( GTK_PROGRESS_BAR(m_widget),
                             m_rangeMax ? ((float)m_gaugePos)/m_rangeMax : 0.);
}

wxSize wxGauge::DoGetBestSize() const
{
    wxSize best;
    if (HasFlag(wxGA_VERTICAL))
        best = wxSize(28, 100);
    else
        best = wxSize(100, 28);
    return best;
}

void wxGauge::SetRange( int range )
{
    m_rangeMax = range;
    if (m_gaugePos > m_rangeMax)
        m_gaugePos = m_rangeMax;

    DoSetGauge();
}

void wxGauge::SetValue( int pos )
{
    wxCHECK_RET( pos <= m_rangeMax, wxT("invalid value in wxGauge::SetValue()") );

    m_gaugePos = pos;

    DoSetGauge();
}

int wxGauge::GetRange() const
{
    return m_rangeMax;
}

int wxGauge::GetValue() const
{
    return m_gaugePos;
}

wxVisualAttributes wxGauge::GetDefaultAttributes() const
{
    // Visible gauge colours use a different colour state
    return GetDefaultAttributesFromGTKWidget(m_widget,
                                             UseGTKStyleBase(),
                                             GTK_STATE_ACTIVE);

}

// static
wxVisualAttributes
wxGauge::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    return GetDefaultAttributesFromGTKWidget(gtk_progress_bar_new,
                                             false, GTK_STATE_ACTIVE);
}

#endif // wxUSE_GAUGE
