/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/accel.h
// Purpose:     wxAcceleratorTable class
// Author:      Julian Smart
// Modified by:
// Created:     31/7/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ACCEL_H_
#define _WX_ACCEL_H_

#include <span>

class wxWindow;

// ----------------------------------------------------------------------------
// the accel table has all accelerators for a given window or menu
// ----------------------------------------------------------------------------

class wxAcceleratorTable : public wxObject
{
public:
    // default ctor
    wxAcceleratorTable() = default;

    // load from .rc resource (Windows specific)
    explicit wxAcceleratorTable(const std::string& resource);

    // initialize from array
    wxAcceleratorTable(std::span<wxAcceleratorEntry> entries);

    bool IsOk() const;
    void SetHACCEL(WXHACCEL hAccel);
    WXHACCEL GetHACCEL() const;

    // translate the accelerator, return true if done
    bool Translate(wxWindow *window, WXMSG *msg) const;

private:
    wxDECLARE_DYNAMIC_CLASS(wxAcceleratorTable);
};

#endif
    // _WX_ACCEL_H_
