/////////////////////////////////////////////////////////////////////////////
// Name:        src/richtext/richtextimagedlg.cpp
// Purpose:
// Author:      Mingquan Yang
// Modified by: Julian Smart
// Created:     Wed 02 Jun 2010 11:27:23 CST
// RCS-ID:
// Copyright:   (c) Mingquan Yang, Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_RICHTEXT

#include "wx/button.h"
#include "wx/combobox.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"

#include "wx/statline.h"

#include "wx/richtext/richtextimagedlg.h"
#include "wx/richtext/richtextctrl.h"

import WX.Core.Sizer;

////@begin XPM images
////@end XPM images


/*!
 * wxRichTextObjectPropertiesDialog type definition
 */

wxIMPLEMENT_DYNAMIC_CLASS(wxRichTextObjectPropertiesDialog, wxRichTextFormattingDialog);


/*!
 * wxRichTextObjectPropertiesDialog event table definition
 */

wxBEGIN_EVENT_TABLE(wxRichTextObjectPropertiesDialog, wxRichTextFormattingDialog)

////@begin wxRichTextObjectPropertiesDialog event table entries
////@end wxRichTextObjectPropertiesDialog event table entries

wxEND_EVENT_TABLE()


/*!
 * wxRichTextObjectPropertiesDialog constructors
 */

wxRichTextObjectPropertiesDialog::wxRichTextObjectPropertiesDialog( wxRichTextObject* obj, wxWindow* parent, wxWindowID id, const std::string& caption, const wxPoint& pos, const wxSize& size, unsigned int style )
{
    Create(obj, parent, id, caption, pos, size, style);
}


/*!
 * wxRichTextImageDlg creator
 */

bool wxRichTextObjectPropertiesDialog::Create( wxRichTextObject* obj, wxWindow* parent, wxWindowID id, const std::string& caption, const wxPoint& pos, const wxSize& size, unsigned int style )
{
    SetObject(obj);
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);
    long flags = wxRICHTEXT_FORMAT_SIZE|wxRICHTEXT_FORMAT_MARGINS|wxRICHTEXT_FORMAT_BORDERS|wxRICHTEXT_FORMAT_BACKGROUND;
    wxRichTextFormattingDialog::Create( flags, parent, caption, id, pos, size, style );

    CreateControls();

    return true;
}

/*!
 * Member initialisation
 */

/*!
 * Control creation for wxRichTextImageDlg
 */

void wxRichTextObjectPropertiesDialog::CreateControls()
{
}


/*!
 * Should we show tooltips?
 */

bool wxRichTextObjectPropertiesDialog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap wxRichTextObjectPropertiesDialog::GetBitmapResource( const std::string& name )
{
    // Bitmap retrieval
////@begin wxRichTextObjectPropertiesDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end wxRichTextObjectPropertiesDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon wxRichTextObjectPropertiesDialog::GetIconResource( const std::string& name )
{
    // Icon retrieval
////@begin wxRichTextObjectPropertiesDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end wxRichTextObjectPropertiesDialog icon retrieval
}

#if 0
/*!
 * wxEVT_BUTTON event handler for ID_BUTTON_PARA_UP
 */
void wxRichTextObjectPropertiesDialog::OnRichtextParaUpClick( [[maybe_unused]] wxCommandEvent& event)
{
    // Before editing this code, remove the block markers.
    wxRichTextRange range = m_object->GetRange();
    wxRichTextObjectList::compatibility_iterator iter = m_buffer->GetChildren().GetFirst();
    if (!iter)
        return;

    while (iter)
    {
        if (iter->GetData() == m_parent)
            break;
        iter = iter->GetNext();
    }

    iter = iter->GetPrevious();
    if (!iter)
        return;

    wxRichTextObject *obj = iter->GetData();
    wxRichTextRange rg = obj->GetRange();
    m_object = m_object->Clone();

    m_buffer->DeleteRangeWithUndo(range, m_buffer->GetRichTextCtrl());
    m_buffer->InsertObjectWithUndo(rg.GetEnd(), m_object, m_buffer->GetRichTextCtrl(), 0);
    m_parent = obj;
    m_object->SetRange(wxRichTextRange(rg.GetEnd(), rg.GetEnd()));
}


/*!
 * wxEVT_BUTTON event handler for ID_BUTTON_PARA_DOWN
 */

void wxRichTextObjectPropertiesDialog::OnRichtextDownClick( [[maybe_unused]] wxCommandEvent& event)
{
    // Before editing this code, remove the block markers.
    wxRichTextRange range = m_object->GetRange();
    wxRichTextObjectList::compatibility_iterator iter = m_buffer->GetChildren().GetFirst();
    if (!iter)
        return;

    while (iter)
    {
        if (iter->GetData() == m_parent)
            break;
        iter = iter->GetNext();
    }

    iter = iter->GetNext();
    if (!iter)
        return;

    wxRichTextObject *obj = iter->GetData();
    wxRichTextRange rg = obj->GetRange();
    m_object = m_object->Clone();

    m_buffer->DeleteRangeWithUndo(range, m_buffer->GetRichTextCtrl());
    m_buffer->InsertObjectWithUndo(rg.GetEnd(), m_object, m_buffer->GetRichTextCtrl(), 0);
    m_parent = obj;
    m_object->SetRange(wxRichTextRange(rg.GetEnd(), rg.GetEnd()));
}

#endif

#endif
    // wxUSE_RICHTEXT
