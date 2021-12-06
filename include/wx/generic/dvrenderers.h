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

class wxDataViewCustomRenderer: public wxDataViewRenderer
{
public:
    inline static constexpr char DefaultType[] = "string";

    wxDataViewCustomRenderer( const std::string& varianttype = DefaultType,
                              wxDataViewCellMode mode = wxDataViewCellMode::Inert,
                              int align = wxDVR_DEFAULT_ALIGNMENT );


	wxDataViewCustomRenderer& operator=(wxDataViewCustomRenderer&&) = delete;

    // see the explanation of the following WXOnXXX() methods in wx/generic/dvrenderer.h

    bool WXActivateCell(const wxRect& cell,
                                wxDataViewModel *model,
                                const wxDataViewItem& item,
                                unsigned int col,
                                const wxMouseEvent *mouseEvent) override
    {
        return ActivateCell(cell, model, item, col, mouseEvent);
    }

#if wxUSE_ACCESSIBILITY
    std::string GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY
};


// ---------------------------------------------------------
// wxDataViewTextRenderer
// ---------------------------------------------------------

class wxDataViewTextRenderer: public wxDataViewRenderer
{
public:
    inline static constexpr char DefaultType[] = "string";

    wxDataViewTextRenderer( const std::string& varianttype = DefaultType,
                            wxDataViewCellMode mode = wxDataViewCellMode::Inert,
                            int align = wxDVR_DEFAULT_ALIGNMENT );
    ~wxDataViewTextRenderer();

	wxDataViewTextRenderer& operator=(wxDataViewTextRenderer&&) = delete;

#if wxUSE_MARKUP
    void EnableMarkup(bool enable = true);
#endif // wxUSE_MARKUP

    bool SetValue( const wxVariant &value ) override;
    bool GetValue( wxVariant &value ) const override;
#if wxUSE_ACCESSIBILITY
    std::string GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

    bool Render(wxRect cell, wxDC *dc, int state) override;
    wxSize GetSize() const override;

    // in-place editing
    bool HasEditorCtrl() const override;
    wxWindow* CreateEditorCtrl( wxWindow *parent, wxRect labelRect,
                                        const wxVariant &value ) override;
    bool GetValueFromEditorCtrl( wxWindow* editor, wxVariant &value ) override;

protected:
    wxString   m_text;

private:
#if wxUSE_MARKUP
    class wxItemMarkupText *m_markupText;
#endif // wxUSE_MARKUP
};

// ---------------------------------------------------------
// wxDataViewBitmapRenderer
// ---------------------------------------------------------

class wxDataViewBitmapRenderer: public wxDataViewRenderer
{
public:
    inline static constexpr char DefaultType[] = "wxBitmap";

    wxDataViewBitmapRenderer( const wxString &varianttype = DefaultType,
                              wxDataViewCellMode mode = wxDataViewCellMode::Inert,
                              int align = wxDVR_DEFAULT_ALIGNMENT );

	wxDataViewBitmapRenderer& operator=(wxDataViewBitmapRenderer&&) = delete;

    bool SetValue( const wxVariant &value ) override;
    bool GetValue( wxVariant &value ) const override;
#if wxUSE_ACCESSIBILITY
    std::string GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

    bool Render( wxRect cell, wxDC *dc, int state ) override;
    wxSize GetSize() const override;

private:
    wxIcon m_icon;
    wxBitmap m_bitmap;
};

// ---------------------------------------------------------
// wxDataViewToggleRenderer
// ---------------------------------------------------------

class wxDataViewToggleRenderer: public wxDataViewRenderer
{
public:
    inline static constexpr char DefaultType[] = "bool";

    wxDataViewToggleRenderer( const std::string& varianttype = DefaultType,
                              wxDataViewCellMode mode = wxDataViewCellMode::Inert,
                              int align = wxDVR_DEFAULT_ALIGNMENT );

	wxDataViewToggleRenderer& operator=(wxDataViewToggleRenderer&&) = delete;

    void ShowAsRadio() { m_radio = true; }

    bool SetValue( const wxVariant &value ) override;
    bool GetValue( wxVariant &value ) const override;
#if wxUSE_ACCESSIBILITY
    std::string GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

    bool Render( wxRect cell, wxDC *dc, int state ) override;
    wxSize GetSize() const override;

    // Implementation only, don't use nor override
    bool WXActivateCell(const wxRect& cell,
                                wxDataViewModel *model,
                                const wxDataViewItem& item,
                                unsigned int col,
                                const wxMouseEvent *mouseEvent) override;
private:
    bool    m_toggle;
    bool    m_radio;
};

// ---------------------------------------------------------
// wxDataViewProgressRenderer
// ---------------------------------------------------------

class wxDataViewProgressRenderer: public wxDataViewRenderer
{
public:
    inline static constexpr char DefaultType[] = "long";

    wxDataViewProgressRenderer( const std::string& label = {},
                                const std::string& varianttype = DefaultType,
                                wxDataViewCellMode mode = wxDataViewCellMode::Inert,
                                int align = wxDVR_DEFAULT_ALIGNMENT );

	wxDataViewProgressRenderer& operator=(wxDataViewProgressRenderer&&) = delete;

    bool SetValue( const wxVariant &value ) override;
    bool GetValue( wxVariant& value ) const override;
#if wxUSE_ACCESSIBILITY
    std::string GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

    bool Render(wxRect cell, wxDC *dc, int state) override;
    wxSize GetSize() const override;

private:
    std::string m_label;
    int         m_value;
};

// ---------------------------------------------------------
// wxDataViewIconTextRenderer
// ---------------------------------------------------------

class wxDataViewIconTextRenderer: public wxDataViewRenderer
{
public:
    inline static constexpr char DefaultType[] = "wxDataViewIconText";

    wxDataViewIconTextRenderer( const std::string& varianttype = DefaultType,
                                wxDataViewCellMode mode = wxDataViewCellMode::Inert,
                                int align = wxDVR_DEFAULT_ALIGNMENT );

	wxDataViewIconTextRenderer& operator=(wxDataViewIconTextRenderer&&) = delete;

    bool SetValue( const wxVariant &value ) override;
    bool GetValue( wxVariant &value ) const override;
#if wxUSE_ACCESSIBILITY
    std::string GetAccessibleDescription() const override;
#endif // wxUSE_ACCESSIBILITY

    bool Render(wxRect cell, wxDC *dc, int state) override;
    wxSize GetSize() const override;

    bool HasEditorCtrl() const override { return true; }
    wxWindow* CreateEditorCtrl( wxWindow *parent, wxRect labelRect,
                                        const wxVariant &value ) override;
    bool GetValueFromEditorCtrl( wxWindow* editor, wxVariant &value ) override;

private:
    wxDataViewIconText   m_value;
};

#endif // _WX_GENERIC_DVRENDERERS_H_

