/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/toolbar.h
// Purpose:     GTK toolbar
// Author:      Robert Roebling
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_TOOLBAR_H_
#define _WX_GTK_TOOLBAR_H_

typedef struct _GtkTooltips GtkTooltips;

// ----------------------------------------------------------------------------
// wxToolBar
// ----------------------------------------------------------------------------

class wxToolBar : public wxToolBarBase
{
public:
    // construction/destruction
    wxToolBar() { Init(); }
    wxToolBar( wxWindow *parent,
               wxWindowID id,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxTB_DEFAULT_STYLE,
               const wxString& name = wxASCII_STR(wxToolBarNameStr) )
    {
        Init();

        Create(parent, id, pos, size, style, name);
    }

    bool Create( wxWindow *parent,
                 wxWindowID id,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTB_DEFAULT_STYLE,
                 const wxString& name = wxASCII_STR(wxToolBarNameStr) );

    virtual ~wxToolBar();

    wxToolBarToolBase *FindToolForPosition(wxCoord x, wxCoord y) const override;

    void SetToolShortHelp(int id, const wxString& helpString) override;

    void SetWindowStyleFlag( long style ) override;

    void SetToolNormalBitmap(int id, const wxBitmap& bitmap) override;
    void SetToolDisabledBitmap(int id, const wxBitmap& bitmap) override;

    bool Realize() override;

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWINDOW_VARIANT_NORMAL);

    virtual wxToolBarToolBase *CreateTool(int id,
                                          const wxString& label,
                                          const wxBitmap& bitmap1,
                                          const wxBitmap& bitmap2 = wxNullBitmap,
                                          wxItemKind kind = wxITEM_NORMAL,
                                          wxObject *clientData = NULL,
                                          const wxString& shortHelpString = wxEmptyString,
                                          const wxString& longHelpString = wxEmptyString) override;
    virtual wxToolBarToolBase *CreateTool(wxControl *control,
                                          const wxString& label) override;

    // implementation from now on
    // --------------------------

    GtkToolbar* GTKGetToolbar() const { return m_toolbar; }

protected:
    // choose the default border for this window
    wxBorder GetDefaultBorder() const override { return wxBORDER_DEFAULT; }

    wxSize DoGetBestSize() const override;
    GdkWindow *GTKGetWindow(wxArrayGdkWindows& windows) const override;

    
    bool DoInsertTool(size_t pos, wxToolBarToolBase *tool) override;
    bool DoDeleteTool(size_t pos, wxToolBarToolBase *tool) override;

    void DoEnableTool(wxToolBarToolBase *tool, bool enable) override;
    void DoToggleTool(wxToolBarToolBase *tool, bool toggle) override;
    void DoSetToggle(wxToolBarToolBase *tool, bool toggle) override;

private:
    void Init();
    void GtkSetStyle();
    GSList* GetRadioGroup(size_t pos);
    void AddChildGTK(wxWindowGTK* child) override;

    GtkToolbar* m_toolbar;
    GtkTooltips* m_tooltips;

    wxDECLARE_DYNAMIC_CLASS(wxToolBar);
};

#endif
    // _WX_GTK_TOOLBAR_H_
