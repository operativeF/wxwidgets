/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/validate.cpp
// Purpose:     wxValidator
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_VALIDATORS

#include "wx/validate.h"
#include "wx/window.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxValidator, wxEvtHandler);

const wxValidator wxDefaultValidator;

#endif // wxUSE_VALIDATORS
