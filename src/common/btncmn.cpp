///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/btncmn.cpp
// Purpose:     implementation of wxButtonBase
// Author:      Vadim Zeitlin
// Created:     2007-04-08
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_BUTTON

#include "wx/button.h"
#include "wx/directionflags.h"
#include "wx/toplevel.h"

// ----------------------------------------------------------------------------
// XTI
// ----------------------------------------------------------------------------

wxDEFINE_FLAGS( wxButtonStyle )
wxBEGIN_FLAGS( wxButtonStyle )
// new style border flags, we put them first to
// use them for streaming out
wxFLAGS_MEMBER(wxBorder::Simple)
wxFLAGS_MEMBER(wxBorder::Sunken)
wxFLAGS_MEMBER(wxBORDER_DOUBLE)
wxFLAGS_MEMBER(wxBorder::Raised)
wxFLAGS_MEMBER(wxBorder::Static)
wxFLAGS_MEMBER(wxBorder::None)

// old style border flags
wxFLAGS_MEMBER(wxBorder::Simple)
wxFLAGS_MEMBER(wxBorder::Sunken)
wxFLAGS_MEMBER(wxDOUBLE_BORDER)
wxFLAGS_MEMBER(wxBorder::Raised)
wxFLAGS_MEMBER(wxBorder::Static)
wxFLAGS_MEMBER(wxBORDER)

// standard window styles
wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
wxFLAGS_MEMBER(wxCLIP_CHILDREN)
wxFLAGS_MEMBER(wxTRANSPARENT_WINDOW)
wxFLAGS_MEMBER(wxWANTS_CHARS)
wxFLAGS_MEMBER(wxFULL_REPAINT_ON_RESIZE)
wxFLAGS_MEMBER(wxALWAYS_SHOW_SB )
wxFLAGS_MEMBER(wxVSCROLL)
wxFLAGS_MEMBER(wxHSCROLL)

wxFLAGS_MEMBER(wxBU_LEFT)
wxFLAGS_MEMBER(wxBU_RIGHT)
wxFLAGS_MEMBER(wxBU_TOP)
wxFLAGS_MEMBER(wxBU_BOTTOM)
wxFLAGS_MEMBER(wxBU_EXACTFIT)
wxEND_FLAGS( wxButtonStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxButton, wxControl, "wx/button.h");

wxBEGIN_PROPERTIES_TABLE(wxButton)
wxEVENT_PROPERTY( Click, wxEVT_BUTTON, wxCommandEvent )

wxPROPERTY( Font, wxFont, SetFont, GetFont, wxEMPTY_PARAMETER_VALUE, \
           0 /*flags*/, "The font associated with the button label", "group")
wxPROPERTY( Label, std::string, SetLabel, GetLabel, "", \
           0 /*flags*/, "The button label", "group")

wxPROPERTY_FLAGS( WindowStyle, wxButtonStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 /*flags*/,     \
                 "The button style", "group") // style
wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxButton)

wxCONSTRUCTOR_6( wxButton, wxWindow*, Parent, wxWindowID, Id, std::string, \
                Label, wxPoint, Position, wxSize, Size, long, WindowStyle )


// ============================================================================
// implementation
// ============================================================================

wxWindow *wxButtonBase::SetDefault()
{
    wxTopLevelWindow * const
        tlw = wxDynamicCast(wxGetTopLevelParent(this), wxTopLevelWindow);

    wxCHECK_MSG( tlw, nullptr, "button without top level window?");

    return tlw->SetDefaultItem(this);
}

void wxAnyButtonBase::SetBitmapPosition(wxDirection dir)
{
    DoSetBitmapPosition(dir);
}
#endif // wxUSE_BUTTON
