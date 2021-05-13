///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/dvrenderers.h
// Purpose:     All generic wxDataViewCtrl renderer classes
// Author:      Robert Roebling, Vadim Zeitlin
// Created:     2009-11-07 (extracted from wx/generic/dataview.h)
// Copyright:   (c) 2006 Robert Roebling
//              (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_DVRENDERERS_H_
#define _WX_GENERIC_DVRENDERERS_H_

// ---------------------------------------------------------
// wxDataViewCustomRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewCustomRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("string"); }

    wxDataViewCustomRenderer( const wxString &varianttype = GetDefaultType(),
                              wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                              int align = wxDVR_DEFAULT_ALIGNMENT );


	wxDataViewCustomRenderer(const wxDataViewCustomRenderer&) = delete;
	wxDataViewCustomRenderer& operator=(const wxDataViewCustomRenderer&) = delete;

    // see the explanation of the following WXOnXXX() methods in wx/generic/dvrenderer.h

    virtual bool WXActivateCell(const wxRect& cell,
                                wxDataViewModel *model,
                                const wxDataViewItem& item,
                                unsigned int col,
                                const wxMouseEvent *mouseEvent) override
    {
        return ActivateCell(cell, model, item, col, mouseEvent);
    }

#if wxUSE_ACCESSIBILITY
    wxString GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};


// ---------------------------------------------------------
// wxDataViewTextRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewTextRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("string"); }

    wxDataViewTextRenderer( const wxString &varianttype = GetDefaultType(),
                            wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                            int align = wxDVR_DEFAULT_ALIGNMENT );
    virtual ~wxDataViewTextRenderer();

	wxDataViewTextRenderer(const wxDataViewTextRenderer&) = delete;
	wxDataViewTextRenderer& operator=(const wxDataViewTextRenderer&) = delete;

#if wxUSE_MARKUP
    void EnableMarkup(bool enable = true);
#endif // wxUSE_MARKUP

    bool SetValue( const wxVariant &value ) override;
    bool GetValue( wxVariant &value ) const override;
#if wxUSE_ACCESSIBILITY
    wxString GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

    bool Render(wxRect cell, wxDC *dc, int state) override;
    wxSize GetSize() const override;

    // in-place editing
    bool HasEditorCtrl() const override;
    virtual wxWindow* CreateEditorCtrl( wxWindow *parent, wxRect labelRect,
                                        const wxVariant &value ) override;
    bool GetValueFromEditorCtrl( wxWindow* editor, wxVariant &value ) override;

protected:
    wxString   m_text;

private:
#if wxUSE_MARKUP
    class wxItemMarkupText *m_markupText;
#endif // wxUSE_MARKUP

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------
// wxDataViewBitmapRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewBitmapRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("wxBitmap"); }

    wxDataViewBitmapRenderer( const wxString &varianttype = GetDefaultType(),
                              wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                              int align = wxDVR_DEFAULT_ALIGNMENT );

	wxDataViewBitmapRenderer(const wxDataViewBitmapRenderer&) = delete;
	wxDataViewBitmapRenderer& operator=(const wxDataViewBitmapRenderer&) = delete;

    bool SetValue( const wxVariant &value ) override;
    bool GetValue( wxVariant &value ) const override;
#if wxUSE_ACCESSIBILITY
    wxString GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

    bool Render( wxRect cell, wxDC *dc, int state ) override;
    wxSize GetSize() const override;

private:
    wxIcon m_icon;
    wxBitmap m_bitmap;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------
// wxDataViewToggleRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewToggleRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("bool"); }

    wxDataViewToggleRenderer( const wxString &varianttype = GetDefaultType(),
                              wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                              int align = wxDVR_DEFAULT_ALIGNMENT );

	wxDataViewToggleRenderer(const wxDataViewToggleRenderer&) = delete;
	wxDataViewToggleRenderer& operator=(const wxDataViewToggleRenderer&) = delete;

    void ShowAsRadio() { m_radio = true; }

    bool SetValue( const wxVariant &value ) override;
    bool GetValue( wxVariant &value ) const override;
#if wxUSE_ACCESSIBILITY
    wxString GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

    bool Render( wxRect cell, wxDC *dc, int state ) override;
    wxSize GetSize() const override;

    // Implementation only, don't use nor override
    virtual bool WXActivateCell(const wxRect& cell,
                                wxDataViewModel *model,
                                const wxDataViewItem& item,
                                unsigned int col,
                                const wxMouseEvent *mouseEvent) override;
private:
    bool    m_toggle;
    bool    m_radio;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------
// wxDataViewProgressRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewProgressRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("long"); }

    wxDataViewProgressRenderer( const wxString &label = wxEmptyString,
                                const wxString &varianttype = GetDefaultType(),
                                wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                                int align = wxDVR_DEFAULT_ALIGNMENT );

	wxDataViewProgressRenderer(const wxDataViewProgressRenderer&) = delete;
	wxDataViewProgressRenderer& operator=(const wxDataViewProgressRenderer&) = delete;

    bool SetValue( const wxVariant &value ) override;
    bool GetValue( wxVariant& value ) const override;
#if wxUSE_ACCESSIBILITY
    wxString GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

    bool Render(wxRect cell, wxDC *dc, int state) override;
    wxSize GetSize() const override;

private:
    wxString    m_label;
    int         m_value;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------
// wxDataViewIconTextRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_ADV wxDataViewIconTextRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("wxDataViewIconText"); }

    wxDataViewIconTextRenderer( const wxString &varianttype = GetDefaultType(),
                                wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                                int align = wxDVR_DEFAULT_ALIGNMENT );

	wxDataViewIconTextRenderer(const wxDataViewIconTextRenderer&) = delete;
	wxDataViewIconTextRenderer& operator=(const wxDataViewIconTextRenderer&) = delete;

    bool SetValue( const wxVariant &value ) override;
    bool GetValue( wxVariant &value ) const override;
#if wxUSE_ACCESSIBILITY
    wxString GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

    bool Render(wxRect cell, wxDC *dc, int state) override;
    wxSize GetSize() const override;

    bool HasEditorCtrl() const override { return true; }
    virtual wxWindow* CreateEditorCtrl( wxWindow *parent, wxRect labelRect,
                                        const wxVariant &value ) override;
    bool GetValueFromEditorCtrl( wxWindow* editor, wxVariant &value ) override;

private:
    wxDataViewIconText   m_value;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_GENERIC_DVRENDERERS_H_

