/////////////////////////////////////////////////////////////////////////////
// Name:        wx/icon.h
// Purpose:     wxIcon base header
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ICON_H_BASE_
#define _WX_ICON_H_BASE_

#include "wx/iconloc.h"

import WX.Utils.Cast;

import WX.Image;

// a more readable way to tell
inline constexpr auto wxICON_SCREEN_DEPTH = wx::narrow_cast<unsigned int>(-1);

// the wxICON_DEFAULT_TYPE (the wxIcon equivalent of wxBITMAP_DEFAULT_TYPE)
// constant defines the default argument value for wxIcon ctor and wxIcon::LoadFile()
// functions.

#ifdef WX_WINDOWS
  inline constexpr auto wxICON_DEFAULT_TYPE = wxBitmapType::ICO_Resource;
  #include "wx/msw/icon.h"
#elif defined(__WXMOTIF__)
  inline constexpr auto wxICON_DEFAULT_TYPE = wxBitmapType::XPM;
  #include "wx/motif/icon.h"
#elif defined(__WXGTK20__)
  #ifdef WX_WINDOWS
    inline constexpr auto wxICON_DEFAULT_TYPE = wxBitmapType::ICO_Resource;
  #else
    inline constexpr auto wxICON_DEFAULT_TYPE = wxBitmapType::XPM;
  #endif
  #include "wx/generic/icon.h"
#elif defined(__WXGTK__)
  inline constexpr auto wxICON_DEFAULT_TYPE = wxBitmapType::XPM;
  #include "wx/generic/icon.h"
#elif defined(__WXX11__)
  inline constexpr auto wxICON_DEFAULT_TYPE = wxBitmapType::XPM;
  #include "wx/generic/icon.h"
#elif defined(__WXDFB__)
  inline constexpr auto wxICON_DEFAULT_TYPE = wxBitmapType::XPM;
  #include "wx/generic/icon.h"
#elif defined(__WXMAC__)
#if wxOSX_USE_COCOA_OR_CARBON
  inline constexpr wxICON_DEFAULT_TYPE = wxBITMAP_TYPE_ICON_RESOURCE;
  #include "wx/generic/icon.h"
#else
  // iOS and others
  inline constexpr auto wxICON_DEFAULT_TYPE = wxBITMAP_TYPE_PNG_RESOURCE;
  #include "wx/generic/icon.h"
#endif
#elif defined(__WXQT__)
  inline constexpr auto wxICON_DEFAULT_TYPE = wxBitmapType::XPM;
  #include "wx/generic/icon.h"
#endif

//-----------------------------------------------------------------------------
// wxVariant support
//-----------------------------------------------------------------------------

#if wxUSE_VARIANT
#include "wx/variant.h"
DECLARE_VARIANT_OBJECT(wxIcon)
#endif


#endif
    // _WX_ICON_H_BASE_
