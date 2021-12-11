/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/odcombocmn.cpp
// Purpose:     wxOwnerDrawnComboBox common code
// Author:      Jaakko Salli
// Modified by:
// Created:     Apr-30-2006
// Copyright:   (c) 2005 Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_ODCOMBOBOX

#include "wx/odcombo.h"
#include "wx/log.h"
#include "wx/combobox.h"
#include "wx/dcclient.h"
#include "wx/combo.h"

import WX.Utils.Settings;

// ----------------------------------------------------------------------------
// XTI
// ----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS2_XTI(wxOwnerDrawnComboBox, wxComboCtrl, \
                               wxControlWithItems, "wx/odcombo.h")

wxBEGIN_PROPERTIES_TABLE(wxOwnerDrawnComboBox)
wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxOwnerDrawnComboBox)

wxCONSTRUCTOR_5( wxOwnerDrawnComboBox , wxWindow* , Parent , wxWindowID , \
                 Id , std::string , Value , wxPoint , Position , wxSize , Size )

#endif // wxUSE_ODCOMBOBOX
