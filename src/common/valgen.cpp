/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/valgen.cpp
// Purpose:     wxGenericValidator class
// Author:      Kevin Smith
// Modified by:
// Created:     Jan 22 1999
// Copyright:   (c) 1999 Kevin Smith
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_VALIDATORS

#include "wx/utils.h"
#include "wx/intl.h"
#include "wx/choice.h"
#include "wx/combobox.h"
#include "wx/radiobox.h"
#include "wx/radiobut.h"
#include "wx/checkbox.h"
#include "wx/scrolbar.h"
#include "wx/gauge.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"
#include "wx/button.h"
#include "wx/listbox.h"
#include "wx/slider.h"
#include "wx/checklst.h"
#include "wx/spinctrl.h"
// #include "wx/datectrl.h" -- can't use it in this (core) file for now

#if wxUSE_SPINBTN
    #include "wx/spinbutt.h"
#endif
#if wxUSE_TOGGLEBTN
    #include "wx/tglbtn.h"
#endif

#include "wx/valgen.h"

#include <fmt/core.h>

import WX.File.Filename;

import <charconv>;

wxIMPLEMENT_CLASS(wxGenericValidator, wxValidator);

wxGenericValidator::wxGenericValidator(bool *val)
    : m_pBool(val)
{
    Initialize();
}

wxGenericValidator::wxGenericValidator(int *val)
    : m_pInt(val)
{
    Initialize();
}

wxGenericValidator::wxGenericValidator(std::string* val)
    : m_pString(val)
{
    Initialize();
}

wxGenericValidator::wxGenericValidator(std::vector<int> *val)
    : m_pArrayInt(val)
{
    Initialize();
}

#if wxUSE_DATETIME

wxGenericValidator::wxGenericValidator(wxDateTime *val)
    : m_pDateTime(val)
{
    Initialize();
}

#endif // wxUSE_DATETIME

wxGenericValidator::wxGenericValidator(wxFileName *val)
    : m_pFileName(val)
{
    Initialize();
}

wxGenericValidator::wxGenericValidator(float *val)
    : m_pFloat(val)
{
    Initialize();
}

wxGenericValidator::wxGenericValidator(double *val)
    : m_pDouble(val)
{
    Initialize();
}

wxGenericValidator::wxGenericValidator(const wxGenericValidator& val)
     
{
    Copy(val);
}

bool wxGenericValidator::Copy(const wxGenericValidator& val)
{
    wxValidator::Copy(val);

    m_pBool = val.m_pBool;
    m_pInt = val.m_pInt;
    m_pString = val.m_pString;
    m_pArrayInt = val.m_pArrayInt;
#if wxUSE_DATETIME
    m_pDateTime = val.m_pDateTime;
#endif // wxUSE_DATETIME
    m_pFileName = val.m_pFileName;
    m_pFloat = val.m_pFloat;
    m_pDouble = val.m_pDouble;

    return true;
}

// Called to transfer data to the window
bool wxGenericValidator::TransferToWindow()
{
    if ( !m_validatorWindow )
        return false;

    // bool controls
#if wxUSE_CHECKBOX
    if (dynamic_cast<wxCheckBox*>(m_validatorWindow))
    {
        wxCheckBox* pControl = (wxCheckBox*) m_validatorWindow;
        if (m_pBool)
        {
            pControl->SetValue(*m_pBool);
            return true;
        }
    } else
#endif
#if wxUSE_RADIOBTN
    if (dynamic_cast<wxRadioButton*>(m_validatorWindow))
    {
        wxRadioButton* pControl = (wxRadioButton*) m_validatorWindow;
        if (m_pBool)
        {
            pControl->SetValue(*m_pBool) ;
            return true;
        }
    } else
#endif

#if wxUSE_TOGGLEBTN
    if (dynamic_cast<wxToggleButton*>(m_validatorWindow))
    {
        wxToggleButton * pControl = (wxToggleButton *) m_validatorWindow;
        if (m_pBool)
        {
            pControl->SetValue(*m_pBool);
            return true;
        }
    } else
#if (defined(__WXMAC__) || defined(__WXMSW__) || defined(__WXGTK20__)) && !defined(__WXUNIVERSAL__)
    if (dynamic_cast<wxBitmapToggleButton*>(m_validatorWindow))
    {
        wxBitmapToggleButton * pControl = (wxBitmapToggleButton *) m_validatorWindow;
        if (m_pBool)
        {
            pControl->SetValue(*m_pBool);
            return true;
        }
    } else
#endif
#endif

    // int controls
#if wxUSE_GAUGE
    if (dynamic_cast<wxGauge*>(m_validatorWindow))
    {
        wxGauge* pControl = (wxGauge*) m_validatorWindow;
        if (m_pInt)
        {
            pControl->SetValue(*m_pInt);
            return true;
        }
    } else
#endif
#if wxUSE_RADIOBOX
    if (dynamic_cast<wxRadioBox*>(m_validatorWindow))
    {
        wxRadioBox* pControl = (wxRadioBox*) m_validatorWindow;
        if (m_pInt)
        {
            pControl->SetSelection(*m_pInt) ;
            return true;
        }
    } else
#endif
#if wxUSE_SCROLLBAR
    if (dynamic_cast<wxScrollBar*>(m_validatorWindow))
    {
        wxScrollBar* pControl = (wxScrollBar*) m_validatorWindow;
        if (m_pInt)
        {
            pControl->SetThumbPosition(*m_pInt) ;
            return true;
        }
    } else
#endif
#if wxUSE_SPINCTRL && !defined(__WXMOTIF__)
    if (dynamic_cast<wxSpinCtrl*>(m_validatorWindow))
    {
        wxSpinCtrl* pControl = (wxSpinCtrl*) m_validatorWindow;
        if (m_pInt)
        {
            pControl->SetValue(*m_pInt);
            return true;
        }
    } else
#endif
#if wxUSE_SPINBTN
    if (dynamic_cast<wxSpinButton*>(m_validatorWindow))
    {
        wxSpinButton* pControl = (wxSpinButton*) m_validatorWindow;
        if (m_pInt)
        {
            pControl->SetValue(*m_pInt) ;
            return true;
        }
    } else
#endif
#if wxUSE_SLIDER
    if (dynamic_cast<wxSlider*>(m_validatorWindow))
    {
        wxSlider* pControl = (wxSlider*) m_validatorWindow;
        if (m_pInt)
        {
            pControl->SetValue(*m_pInt) ;
            return true;
        }
    } else
#endif

    // date time controls
#if 0 // wxUSE_DATEPICKCTRL -- temporary fix for shared build linking
    if (dynamic_cast<wxDatePickerCtrl*>(m_validatorWindow))
    {
        wxDatePickerCtrl* pControl = (wxDatePickerCtrl*) m_validatorWindow;
        if (m_pDateTime)
        {
            pControl->SetValue(*m_pDateTime) ;
            return true;
        }
    } else
#endif

    // string controls
#if wxUSE_BUTTON
    if (dynamic_cast<wxButton*>(m_validatorWindow))
    {
        wxButton* pControl = (wxButton*) m_validatorWindow;
        if (m_pString)
        {
            pControl->SetLabel(*m_pString) ;
            return true;
        }
    } else
#endif
#if wxUSE_COMBOBOX
    if (dynamic_cast<wxComboBox*>(m_validatorWindow))
    {
        wxComboBox* pControl = (wxComboBox*) m_validatorWindow;
        if (m_pInt)
        {
            pControl->SetSelection(*m_pInt) ;
            return true;
        }
        else if (m_pString)
        {
            if (pControl->FindString(*m_pString) != wxNOT_FOUND)
            {
                pControl->SetStringSelection(*m_pString);
            }
            if ((m_validatorWindow->wxGetWindowStyle() & wxCB_READONLY) == 0)
            {
                pControl->SetValue(*m_pString);
            }
            return true;
        }
    } else
#endif
#if wxUSE_CHOICE
    if (dynamic_cast<wxChoice*>(m_validatorWindow))
    {
        wxChoice* pControl = (wxChoice*) m_validatorWindow;
        if (m_pInt)
        {
            pControl->SetSelection(*m_pInt) ;
            return true;
        }
        else if (m_pString)
        {
            if (pControl->FindString(* m_pString) != wxNOT_FOUND)
            {
                pControl->SetStringSelection(* m_pString);
            }
            return true;
        }
    } else
#endif
#if wxUSE_STATTEXT
    if (dynamic_cast<wxStaticText*>(m_validatorWindow))
    {
        wxStaticText* pControl = (wxStaticText*) m_validatorWindow;
        if (m_pString)
        {
            pControl->SetLabel(*m_pString) ;
            return true;
        }
    } else
#endif
#if wxUSE_TEXTCTRL
    if (dynamic_cast<wxTextCtrl*>(m_validatorWindow))
    {
        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
        if (m_pString)
        {
            pControl->SetValue(*m_pString) ;
            return true;
        }
        else if (m_pInt)
        {
            std::string str = fmt::format("{:d}", *m_pInt);
            pControl->SetValue(str);

            return true;
        }
        else if (m_pFileName)
        {
            pControl->SetValue(m_pFileName->GetFullPath());
            return true;
        }
        else if (m_pFloat)
        {
            pControl->SetValue(fmt::format("{:g}", double(*m_pFloat)));
            return true;
        }
        else if (m_pDouble)
        {
            pControl->SetValue(fmt::format("{:g}", *m_pDouble));
            return true;
        }
    } else
#endif

    // array controls
#if wxUSE_CHECKLISTBOX
    // NOTE: wxCheckListBox is a wxListBox, so wxCheckListBox MUST come first:
    if (dynamic_cast<wxCheckListBox*>(m_validatorWindow))
    {
        wxCheckListBox* pControl = (wxCheckListBox*) m_validatorWindow;
        if (m_pArrayInt)
        {
            // clear all selections
            for ( size_t i = 0 ; i != pControl->GetCount(); i++ )
                pControl->Check(i, false);

            // select each item in our array
            std::ranges::for_each(*m_pArrayInt,
                [pControl](const auto& selection){
                    pControl->Check(selection);
                });

            return true;
        }
        else
            return false;
    } else
#endif
#if wxUSE_LISTBOX
    if (dynamic_cast<wxListBox*>(m_validatorWindow))
    {
        wxListBox* pControl = (wxListBox*) m_validatorWindow;
        if (m_pArrayInt)
        {
            // clear all selections
            for ( size_t i = 0 ; i != pControl->GetCount(); i++ )
                pControl->Deselect(i);

            // select each item in our array
            std::ranges::for_each(*m_pArrayInt,
                [pControl](const auto& selection){
                    pControl->SetSelection(selection);
                });

            return true;
        }
    } else
#endif
    {   // to match the last 'else' above
    }

  // unrecognized control, or bad pointer
  return false;
}

// Called to transfer data from the window
bool wxGenericValidator::TransferFromWindow()
{
    if ( !m_validatorWindow )
        return false;

    // BOOL CONTROLS **************************************
#if wxUSE_CHECKBOX
    if (dynamic_cast<wxCheckBox*>(m_validatorWindow))
    {
        wxCheckBox* pControl = (wxCheckBox*) m_validatorWindow;
        if (m_pBool)
        {
            *m_pBool = pControl->GetValue() ;
            return true;
        }
    } else
#endif
#if wxUSE_RADIOBTN
    if (dynamic_cast<wxRadioButton*>(m_validatorWindow))
    {
        wxRadioButton* pControl = (wxRadioButton*) m_validatorWindow;
        if (m_pBool)
        {
            *m_pBool = pControl->GetValue() ;
            return true;
        }
    } else
#endif
#if wxUSE_TOGGLEBTN
    if (dynamic_cast<wxToggleButton*>(m_validatorWindow))
    {
        wxToggleButton *pControl = (wxToggleButton *) m_validatorWindow;
        if (m_pBool)
        {
            *m_pBool = pControl->GetValue() ;
            return true;
        }
    } else
#if (defined(__WXMAC__) || defined(__WXMSW__) || defined(__WXGTK20__)) && !defined(__WXUNIVERSAL__)
    if (dynamic_cast<wxBitmapToggleButton*>(m_validatorWindow))
    {
        wxBitmapToggleButton *pControl = (wxBitmapToggleButton *) m_validatorWindow;
        if (m_pBool)
        {
            *m_pBool = pControl->GetValue() ;
            return true;
        }
    } else
#endif
#endif

    // INT CONTROLS ***************************************
#if wxUSE_GAUGE
    if (dynamic_cast<wxGauge*>(m_validatorWindow))
    {
        wxGauge* pControl = (wxGauge*) m_validatorWindow;
        if (m_pInt)
        {
            *m_pInt = pControl->GetValue() ;
            return true;
        }
    } else
#endif
#if wxUSE_RADIOBOX
    if (dynamic_cast<wxRadioBox*>(m_validatorWindow))
    {
        wxRadioBox* pControl = (wxRadioBox*) m_validatorWindow;
        if (m_pInt)
        {
            *m_pInt = pControl->GetSelection() ;
            return true;
        }
    } else
#endif
#if wxUSE_SCROLLBAR
    if (dynamic_cast<wxScrollBar*>(m_validatorWindow))
    {
        wxScrollBar* pControl = (wxScrollBar*) m_validatorWindow;
        if (m_pInt)
        {
            *m_pInt = pControl->GetThumbPosition() ;
            return true;
        }
    } else
#endif
#if wxUSE_SPINCTRL && !defined(__WXMOTIF__)
    if (dynamic_cast<wxSpinCtrl*>(m_validatorWindow))
    {
        wxSpinCtrl* pControl = (wxSpinCtrl*) m_validatorWindow;
        if (m_pInt)
        {
            *m_pInt=pControl->GetValue();
            return true;
        }
    } else
#endif
#if wxUSE_SPINBTN
    if (dynamic_cast<wxSpinButton*>(m_validatorWindow))
    {
        wxSpinButton* pControl = (wxSpinButton*) m_validatorWindow;
        if (m_pInt)
        {
            *m_pInt = pControl->GetValue() ;
            return true;
        }
    } else
#endif
#if wxUSE_SLIDER
    if (dynamic_cast<wxSlider*>(m_validatorWindow))
    {
        wxSlider* pControl = (wxSlider*) m_validatorWindow;
        if (m_pInt)
        {
            *m_pInt = pControl->GetValue() ;
            return true;
        }
    } else
#endif

    // DATE TIME CONTROLS ************************************
#if 0 // wxUSE_DATEPICKCTRL -- temporary fix for shared build linking
    if (dynamic_cast<wxDatePickerCtrl*>(m_validatorWindow))
    {
        wxDatePickerCtrl* pControl = (wxDatePickerCtrl*) m_validatorWindow;
        if (m_pDateTime)
        {
            *m_pDateTime = pControl->GetValue() ;
            return true;
        }
    } else
#endif

    // STRING CONTROLS ************************************
#if wxUSE_BUTTON
    if (dynamic_cast<wxButton*>(m_validatorWindow))
    {
        wxButton* pControl = (wxButton*) m_validatorWindow;
        if (m_pString)
        {
            *m_pString = pControl->GetLabel() ;
            return true;
        }
    } else
#endif
#if wxUSE_COMBOBOX
    if (dynamic_cast<wxComboBox*>(m_validatorWindow))
    {
        wxComboBox* pControl = (wxComboBox*) m_validatorWindow;
        if (m_pInt)
        {
            *m_pInt = pControl->GetSelection() ;
            return true;
        }
        else if (m_pString)
        {
            if (m_validatorWindow->wxGetWindowStyle() & wxCB_READONLY)
                *m_pString = pControl->GetStringSelection();
            else
                *m_pString = pControl->GetValue();
            return true;
        }
    } else
#endif
#if wxUSE_CHOICE
    if (dynamic_cast<wxChoice*>(m_validatorWindow))
    {
        wxChoice* pControl = (wxChoice*) m_validatorWindow;
        if (m_pInt)
        {
            *m_pInt = pControl->GetSelection() ;
            return true;
        }
        else if (m_pString)
        {
            *m_pString = pControl->GetStringSelection();
            return true;
        }
    } else
#endif
#if wxUSE_STATTEXT
    if (dynamic_cast<wxStaticText*>(m_validatorWindow))
    {
        wxStaticText* pControl = (wxStaticText*) m_validatorWindow;
        if (m_pString)
        {
            *m_pString = pControl->GetLabel() ;
            return true;
        }
    } else
#endif
#if wxUSE_TEXTCTRL
    if (dynamic_cast<wxTextCtrl*>(m_validatorWindow))
    {
        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
        if (m_pString)
        {
            *m_pString = pControl->GetValue() ;
            return true;
        }
        else if (m_pInt)
        {
            auto [p, ec] = std::from_chars(pControl->GetValue().data(),
                                pControl->GetValue().data() + pControl->GetValue().size(),
                                *m_pInt );
            return ec == std::errc();
        }
        else if (m_pFileName)
        {
            m_pFileName->Assign(pControl->GetValue());
            return true;
        }
        else if (m_pFloat)
        {
            auto [p, ec] = std::from_chars(pControl->GetValue().data(),
                                           pControl->GetValue().data() + pControl->GetValue().size(),
                                           *m_pFloat );
            return ec == std::errc(); // FIXME: Return actual conversion success instead of just true?
        }
        else if (m_pDouble)
        {
            auto [p, ec] = std::from_chars(pControl->GetValue().data(),
                                pControl->GetValue().data() + pControl->GetValue().size(),
                                *m_pDouble );

            return ec == std::errc();
        }
    } else
#endif

    // ARRAY CONTROLS *************************************
#if wxUSE_CHECKLISTBOX
    // NOTE: wxCheckListBox isa wxListBox, so wxCheckListBox MUST come first:
    if (dynamic_cast<wxCheckListBox*>(m_validatorWindow))
    {
        wxCheckListBox* pControl = (wxCheckListBox*) m_validatorWindow;
        if (m_pArrayInt)
        {
            // clear our array
            m_pArrayInt->clear();

            // add each selected item to our array
            for ( size_t i = 0; i != pControl->GetCount(); i++ )
            {
                if (pControl->IsChecked(i))
                    m_pArrayInt->push_back(i);
            }

            return true;
        }
        else
            return false;
    } else
#endif
#if wxUSE_LISTBOX
    if (dynamic_cast<wxListBox*>(m_validatorWindow))
    {
        wxListBox* pControl = (wxListBox*) m_validatorWindow;
        if (m_pArrayInt)
        {
            // clear our array
            m_pArrayInt->clear();

            // add each selected item to our array
            for ( size_t i = 0; i != pControl->GetCount(); i++ )
            {
                if (pControl->IsSelected(i))
                    m_pArrayInt->push_back(i);
            }

            return true;
        }
    } else
#endif

    // unrecognized control, or bad pointer
        return false;

    return false;
}

/*
  Called by constructors to initialize ALL data members
*/
void wxGenericValidator::Initialize()
{
    m_pBool = nullptr;
    m_pInt = nullptr;
    m_pString = nullptr;
    m_pArrayInt = nullptr;
#if wxUSE_DATETIME
    m_pDateTime = nullptr;
#endif // wxUSE_DATETIME
    m_pFileName = nullptr;
    m_pFloat = nullptr;
    m_pDouble = nullptr;
}

#endif // wxUSE_VALIDATORS
