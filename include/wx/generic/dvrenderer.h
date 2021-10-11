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

class WXDLLIMPEXP_FWD_CORE wxRect;

class WXDLLIMPEXP_CORE wxDataViewRenderer: public wxDataViewCustomRendererBase
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

    virtual bool WXActivateCell(const wxRect& WXUNUSED(cell),
                                wxDataViewModel *WXUNUSED(model),
                                const wxDataViewItem & WXUNUSED(item),
                                unsigned int WXUNUSED(col),
                                const wxMouseEvent* WXUNUSED(mouseEvent))
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

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_GENERIC_DVRENDERER_H_

