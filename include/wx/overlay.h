/////////////////////////////////////////////////////////////////////////////
// Name:        wx/overlay.h
// Purpose:     wxOverlay class
// Author:      Stefan Csomor
// Modified by:
// Created:     2006-10-20
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OVERLAY_H_
#define _WX_OVERLAY_H_

#if defined(__WXDFB__)
    #define wxHAS_NATIVE_OVERLAY 1
#elif defined(__WXOSX__) && wxOSX_USE_COCOA
    #define wxHAS_NATIVE_OVERLAY 1
#else
    // don't define wxHAS_NATIVE_OVERLAY
#endif

#include "wx/private/overlay.h"

import Utils.Geometry;

#include <memory>

// ----------------------------------------------------------------------------
// creates an overlay over an existing window, allowing for manipulations like
// rubberbanding etc. This API is not stable yet, not to be used outside wx
// internal code
// ----------------------------------------------------------------------------

class wxDC;

class wxOverlay
{
public:
    wxOverlay();

    // clears the overlay without restoring the former state
    // to be done eg when the window content has been changed and repainted
    void Reset();

    // returns (port-specific) implementation of the overlay
    wxOverlayImpl* GetImpl() { return m_impl.get(); }

private:
    friend class wxDCOverlay;

    // returns true if it has been setup
    bool IsOk();

    void Init(wxDC* dc, wxRect boundary);

    void BeginDrawing(wxDC* dc);

    void EndDrawing(wxDC* dc);

    void Clear(wxDC* dc);

    std::unique_ptr<wxOverlayImpl> m_impl;

    bool m_inDrawing{false};
};


class wxDCOverlay
{
public:
    // connects this overlay to the corresponding drawing dc, if the overlay is
    // not initialized yet this call will do so
    wxDCOverlay(wxOverlay &overlay, wxDC *dc, wxRect boundary);

    // convenience wrapper that behaves the same using the entire area of the dc
    wxDCOverlay(wxOverlay &overlay, wxDC *dc);

    // removes the connection between the overlay and the dc
    virtual ~wxDCOverlay();

    // clears the layer, restoring the state at the last init
    void Clear();

private:
    void Init(wxDC *dc, wxRect boundary);

    wxOverlay& m_overlay;

    wxDC* m_dc;
};

#endif // _WX_OVERLAY_H_
