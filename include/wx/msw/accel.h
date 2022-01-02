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

#include "wx/object.h"

#include <memory>

import WX.WinDef;
import WX.Win.UniqueHnd;

import <string>;
import <span>;

class wxWindow;

// ----------------------------------------------------------------------------
// the accel table has all accelerators for a given window or menu
// ----------------------------------------------------------------------------

class wxAcceleratorTable
{
public:
    // default ctor
    wxAcceleratorTable() = default;

    // load from .rc resource (Windows specific)
    explicit wxAcceleratorTable(const std::string& resource);

    // initialize from a contiguous container
    wxAcceleratorTable(std::span<wxAcceleratorEntry> entries);

    bool IsOk() const;
    void SetHACCEL(WXHACCEL hAccel);
    WXHACCEL GetHACCEL() const;

    // translate the accelerator, return true if done
    bool Translate(wxWindow *window, WXMSG *msg) const;

private:
    msw::utils::unique_accel m_hAccel;

    bool m_ok{false};
};

#endif
    // _WX_ACCEL_H_
