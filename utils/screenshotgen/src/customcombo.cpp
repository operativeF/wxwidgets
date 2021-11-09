/////////////////////////////////////////////////////////////////////////////
// Name:        customcombo.cpp
// Purpose:     Implement some custom wxComboCtrls
// Author:      Utensil Candel (UtensilCandel@@gmail.com)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#if !wxUSE_COMBOCTRL
    #error "Please set wxUSE_COMBOCTRL to 1 and rebuild the library."
#endif

#include "customcombo.h"


wxBEGIN_EVENT_TABLE(ListViewComboPopup, wxListView)
    EVT_MOTION(ListViewComboPopup::OnMouseMove)
    // NOTE: Left down event is used instead of left up right now
    //       since MSW wxListCtrl doesn't seem to emit left ups
    //       consistently.
    EVT_LEFT_DOWN(ListViewComboPopup::OnMouseClick)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(TreeCtrlComboPopup, wxTreeCtrl)
    EVT_MOTION(TreeCtrlComboPopup::OnMouseMove)
    // NOTE: Left down event is used instead of left up right now
    //       since MSW wxTreeCtrl doesn't seem to emit left ups
    //       consistently.
    EVT_LEFT_DOWN(TreeCtrlComboPopup::OnMouseClick)
wxEND_EVENT_TABLE()



// ----------------------------------------------------------------------------
// PenStyleComboBox
// ----------------------------------------------------------------------------

void PenStyleComboBox::OnDrawItem( wxDC& dc,
                                    const wxRect& rect,
                                    int item,
                                    int flags ) const
{
    if ( item == wxNOT_FOUND )
        return;

    wxRect r(rect);
    r.Deflate(3);
    r.height -= 2;

    wxPenStyle penStyle = wxPenStyle::Solid;
//    if ( item == 1 )
//        penStyle = wxPenStyle::Transparent;
//    else if ( item == 2 )
//        penStyle = wxPenStyle::Dot;
//    else if ( item == 3 )
//        penStyle = wxPenStyle::LongDash;
//    else if ( item == 4 )
//        penStyle = wxPenStyle::ShortDash;
    if ( item == 0 )
        penStyle = wxPenStyle::DotDash;
    else if ( item == 1 )
        penStyle = wxPenStyle::BDiagonalHatch;
    else if ( item == 2 )
        penStyle = wxPenStyle::CrossDiagHatch;
//    else if ( item == 8 )
//        penStyle = wxPenStyle::FDiagonalHatch;
//    else if ( item == 9 )
//        penStyle = wxPenStyle::CrossHatch;
//    else if ( item == 10 )
//        penStyle = wxPenStyle::HorizontalHatch;
//    else if ( item == 11 )
//        penStyle = wxPenStyle::VerticalHatch;

    wxPen pen( dc.GetTextForeground(), 3, penStyle );

    // Get text colour as pen colour
    dc.SetPen( pen );

    if ( !(flags & wxODCB_PAINTING_CONTROL) )
    {
        dc.wxDrawText(GetString( item ),
                    r.x + 3,
                    (r.y + 0) + ( (r.height/2) - dc.GetCharHeight() )/2
                   );

        dc.DrawLine( r.x+5, r.y+((r.height/4)*3), r.x+r.width - 5, r.y+((r.height/4)*3) );
    }
    else
    {
        dc.DrawLine( r.x+5, r.y+r.height/2, r.x+r.width - 5, r.y+r.height/2 );
    }
}

void PenStyleComboBox::OnDrawBackground( wxDC& dc, const wxRect& rect,
                                   int item, int flags ) const
{
    // If item is selected or even, or we are painting the
    // combo control itself, use the default rendering.
    if ( (flags & (wxODCB_PAINTING_CONTROL|wxODCB_PAINTING_SELECTED)) ||
         (item & 1) == 0 )
    {
        wxOwnerDrawnComboBox::OnDrawBackground(dc,rect,item,flags);
        return;
    }

    // Otherwise, draw every other background with different colour.
    wxColour bgCol(240,240,250);
    dc.SetBrush(wxBrush(bgCol));
    dc.SetPen(wxPen(bgCol));
    dc.DrawRectangle(rect);
}

inline wxCoord PenStyleComboBox::OnMeasureItem( size_t item ) const
{
    // Simply demonstrate the ability to have variable-height items
    if ( item & 1 )
        return 36;
    else
        return 24;
}

inline wxCoord PenStyleComboBox::OnMeasureItemWidth( size_t WXUNUSED(item) ) const
{
    return -1; // default - will be measured from text width
}

PenStyleComboBox * PenStyleComboBox::CreateSample(wxWindow* parent)
{
    PenStyleComboBox* odc;

    // Common list of items for all dialogs.
    std::vector<wxString>   arrItems;

    // Create common strings array
//    arrItems.Add( "Solid" );
//    arrItems.Add( "Transparent" );
//    arrItems.Add( "Dot" );
//    arrItems.Add( "Long Dash" );
//    arrItems.Add( "Short Dash" );
    //    Comment the following since we don't need too long a drop list
        arrItems.Add( "Dot Dash" );
        arrItems.Add( "Backward Diagonal Hatch" );
        arrItems.Add( "Cross-diagonal Hatch" );
//        arrItems.Add( "Forward Diagonal Hatch" );
//        arrItems.Add( "Cross Hatch" );
//        arrItems.Add( "Horizontal Hatch" );
//        arrItems.Add( "Vertical Hatch" );

    // When defining derivative class for callbacks, we need
    // to use two-stage creation (or redefine the common wx
    // constructor).
    odc = new PenStyleComboBox();
    odc->Create(parent,wxID_ANY,{},
                wxDefaultPosition, wxDefaultSize,
                arrItems,
                wxCB_READONLY //wxNO_BORDER | wxCB_READONLY
               );


    odc->SetSelection(0);

    // Load images from disk
    wxImage imgNormal("bitmaps/dropbutn.png");
    wxImage imgPressed("bitmaps/dropbutp.png");
    wxImage imgHover("bitmaps/dropbuth.png");

    if ( imgNormal.IsOk() && imgPressed.IsOk() && imgHover.IsOk() )
    {
        wxBitmap bmpNormal(imgNormal);
        wxBitmap bmpPressed(imgPressed);
        wxBitmap bmpHover(imgHover);
        odc->SetButtonBitmaps(bmpNormal,false,bmpPressed,bmpHover);
    }
    else
        wxLogError("Dropbutton images not found");

    return odc;
}

