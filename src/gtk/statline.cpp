/////////////////////////////////////////////////////////////////////////////
// Name:        src/gtk/statline.cpp
// Purpose:
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////



#include "wx/statline.h"

#if wxUSE_STATLINE

#include "wx/gtk/private/wrapgtk.h"

//-----------------------------------------------------------------------------
// wxStaticLine
//-----------------------------------------------------------------------------

wxStaticLine::wxStaticLine()
{
}

wxStaticLine::wxStaticLine( wxWindow *parent, wxWindowID id,
                            const wxPoint &pos, const wxSize &size,
                            long style, const wxString &name )
{
    Create( parent, id, pos, size, style, name );
}

bool wxStaticLine::Create( wxWindow *parent, wxWindowID id,
                           const wxPoint &pos, const wxSize &size,
                           long style, const wxString &name )
{
    if (!PreCreation( parent, pos, size ) ||
        !CreateBase( parent, id, pos, size, style, wxDefaultValidator, name ))
    {
        wxFAIL_MSG( wxT("wxStaticLine creation failed") );
        return FALSE;
    }

    const bool isVertical = IsVertical();
    m_widget = gtk_separator_new(GtkOrientation(isVertical));
    g_object_ref(m_widget);
    if (isVertical)
    {
        if (size.x == -1)
        {
            wxSize new_size( size );
            new_size.x = 4;
            SetSize( new_size );
        }
    }
    else
    {
        if (size.y == -1)
        {
            wxSize new_size( size );
            new_size.y = 4;
            SetSize( new_size );
        }
    }

    m_parent->DoAddChild( this );

    PostCreation(size);

    return TRUE;
}

// static
wxVisualAttributes
wxStaticLine::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    return GetDefaultAttributesFromGTKWidget(gtk_separator_new(GTK_ORIENTATION_VERTICAL));
}

#endif
