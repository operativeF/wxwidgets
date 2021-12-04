///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/dvrenderer.h
// Purpose:     wxDataViewRenderer for generic wxDataViewCtrl implementation
// Author:      Robert Roebling, Vadim Zeitlin
// Created:     2009-11-07 (extracted from wx/generic/dataview.h)
// Copyright:   (c) 2006 Robert Roebling
//              (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_DVRENDERER_H_
#define _WX_GENERIC_DVRENDERER_H_

// ----------------------------------------------------------------------------
// wxDataViewRenderer
// ----------------------------------------------------------------------------

#include "wx/geometry/rect.h"

class wxDataViewRenderer: public wxDataViewCustomRendererBase
{
public:
    wxDataViewRenderer( const wxString &varianttype,
                        wxDataViewCellMode mode = wxDataViewCellMode::Inert,
                        int align = wxDVR_DEFAULT_ALIGNMENT );
    ~wxDataViewRenderer();

    wxDataViewRenderer(const wxDataViewRenderer&) = delete;
	wxDataViewRenderer& operator=(const wxDataViewRenderer&) = delete;

    wxDC *GetDC() override;

    void SetAlignment( int align ) override;
    int GetAlignment() const override;

    void EnableEllipsize(wxEllipsizeMode mode = wxEllipsizeMode::Middle) override
        { m_ellipsizeMode = mode; }
    wxEllipsizeMode GetEllipsizeMode() const override
        { return m_ellipsizeMode; }

    void SetMode( wxDataViewCellMode mode ) override
        { m_mode = mode; }
    wxDataViewCellMode GetMode() const override
        { return m_mode; }

    // implementation

    // This callback is used by generic implementation of wxDVC itself.  It's
    // different from the corresponding ActivateCell() method which should only
    // be overridable for the custom renderers while the generic implementation
    // uses this one for all of them, including the standard ones.

    virtual bool WXActivateCell([[maybe_unused]] const wxRect& cell,
                                [[maybe_unused]] wxDataViewModel *model,
                                const [[maybe_unused]] wxDataViewItem & item,
                                [[maybe_unused]] unsigned int col,
                                [[maybe_unused]] const wxMouseEvent* mouseEvent)
        { return false; }

    void SetState(int state) { m_state = state; }

protected:
    bool IsHighlighted() const override
        { return m_state & wxDATAVIEW_CELL_SELECTED; }

private:
    wxDC *m_dc;

    int                          m_state;
    int                          m_align;
    
    wxDataViewCellMode           m_mode;

    wxEllipsizeMode m_ellipsizeMode;
};

#endif // _WX_GENERIC_DVRENDERER_H_

