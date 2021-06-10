///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/dvrenderers.h
// Purpose:     All OS X wxDataViewCtrl renderer classes
// Author:      Vadim Zeitlin
// Created:     2009-11-07 (extracted from wx/osx/dataview.h)
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OSX_DVRENDERERS_H_
#define _WX_OSX_DVRENDERERS_H_

// ---------------------------------------------------------
// wxDataViewCustomRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_CORE wxDataViewCustomRenderer : public wxDataViewCustomRendererBase
{
public:
    static wxString GetDefaultType() { return wxS("string"); }

    wxDataViewCustomRenderer(const wxString& varianttype = GetDefaultType(),
                             wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                             int align = wxDVR_DEFAULT_ALIGNMENT);

    virtual ~wxDataViewCustomRenderer();

	wxDataViewCustomRenderer(const wxDataViewCustomRenderer&) = delete;
	wxDataViewCustomRenderer& operator=(const wxDataViewCustomRenderer&) = delete;

    // implementation only
    // -------------------

    bool MacRender() override;

    wxDC* GetDC() override; // creates a device context and keeps it
    void SetDC(wxDC* newDCPtr); // this method takes ownership of the pointer

private:
    wxControl* m_editorCtrlPtr; // pointer to an in-place editor control

    wxDC* m_DCPtr;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------------------------
// This is a Mac-specific class that should be used as the base class for the
// renderers that should be disabled when they're inert, to prevent the user
// from editing them.
// ---------------------------------------------------------------------------

class wxOSXDataViewDisabledInertRenderer : public wxDataViewRenderer
{
protected:
    wxOSXDataViewDisabledInertRenderer(const wxString& varianttype,
                                       wxDataViewCellMode mode,
                                       int alignment)
        : wxDataViewRenderer(varianttype, mode, alignment)
    {
    }

    void SetEnabled(bool enabled) override
    {
        wxDataViewRenderer::SetEnabled(enabled &&
                                        GetMode() != wxDATAVIEW_CELL_INERT);
    }
};

// ---------------------------------------------------------
// wxDataViewTextRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_CORE wxDataViewTextRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("string"); }

    wxDataViewTextRenderer(const wxString& varianttype = GetDefaultType(),
                           wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                           int align = wxDVR_DEFAULT_ALIGNMENT);

	wxDataViewTextRenderer(const wxDataViewTextRenderer&) = delete;
	wxDataViewTextRenderer& operator=(const wxDataViewTextRenderer&) = delete;

#if wxUSE_MARKUP && wxOSX_USE_COCOA
    void EnableMarkup(bool enable = true);
#endif // wxUSE_MARKUP && Cocoa

    bool MacRender() override;

    virtual void OSXOnCellChanged(NSObject *value,
                                  const wxDataViewItem& item,
                                  unsigned col) override;

private:
#if wxUSE_MARKUP && wxOSX_USE_COCOA
    // True if we should interpret markup in our text.
    bool m_useMarkup;
#endif // wxUSE_MARKUP && Cocoa

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------
// wxDataViewBitmapRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_CORE wxDataViewBitmapRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("wxBitmap"); }

    wxDataViewBitmapRenderer(const wxString& varianttype = GetDefaultType(),
                             wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                             int align = wxDVR_DEFAULT_ALIGNMENT);

	wxDataViewBitmapRenderer(const wxDataViewBitmapRenderer&) = delete;
	wxDataViewBitmapRenderer& operator=(const wxDataViewBitmapRenderer&) = delete;

    bool MacRender() override;

	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// -------------------------------------
// wxDataViewChoiceRenderer
// -------------------------------------

class WXDLLIMPEXP_CORE wxDataViewChoiceRenderer
    : public wxOSXDataViewDisabledInertRenderer
{
public:
    wxDataViewChoiceRenderer(const wxArrayString& choices,
                             wxDataViewCellMode mode = wxDATAVIEW_CELL_EDITABLE,
                             int alignment = wxDVR_DEFAULT_ALIGNMENT );

	wxDataViewChoiceRenderer(const wxDataViewChoiceRenderer&) = delete;
	wxDataViewChoiceRenderer& operator=(const wxDataViewChoiceRenderer&) = delete;

    bool MacRender() override;

    wxString GetChoice(size_t index) const { return m_choices[index]; }
    const wxArrayString& GetChoices() const { return m_choices; }

    virtual void OSXOnCellChanged(NSObject *value,
                                  const wxDataViewItem& item,
                                  unsigned col) override;

private:
    wxArrayString m_choices;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ----------------------------------------------------------------------------
// wxDataViewChoiceByIndexRenderer
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxDataViewChoiceByIndexRenderer: public wxDataViewChoiceRenderer
{
public:
    wxDataViewChoiceByIndexRenderer(const wxArrayString& choices,
                                    wxDataViewCellMode mode = wxDATAVIEW_CELL_EDITABLE,
                                    int alignment = wxDVR_DEFAULT_ALIGNMENT);

    bool SetValue(const wxVariant& value) override;
    bool GetValue(wxVariant& value) const override;

    virtual void OSXOnCellChanged(NSObject *value,
                                  const wxDataViewItem& item,
                                  unsigned col) override;
};

// ---------------------------------------------------------
// wxDataViewIconTextRenderer
// ---------------------------------------------------------
class WXDLLIMPEXP_CORE wxDataViewIconTextRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("wxDataViewIconText"); }

    wxDataViewIconTextRenderer(const wxString& varianttype = GetDefaultType(),
                               wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                               int align = wxDVR_DEFAULT_ALIGNMENT);

	wxDataViewIconTextRenderer(const wxDataViewIconTextRenderer&) = delete;
	wxDataViewIconTextRenderer& operator=(const wxDataViewIconTextRenderer&) = delete;

    bool MacRender() override;

    virtual void OSXOnCellChanged(NSObject *value,
                                  const wxDataViewItem& item,
                                  unsigned col) override;

	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------
// wxDataViewIconTextRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_CORE wxDataViewCheckIconTextRenderer
    : public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("wxDataViewCheckIconText"); }

    explicit wxDataViewCheckIconTextRenderer
        (
         wxDataViewCellMode mode = wxDATAVIEW_CELL_ACTIVATABLE,
         int align = wxDVR_DEFAULT_ALIGNMENT
        );

	wxDataViewCheckIconTextRenderer(const wxDataViewCheckIconTextRenderer&) = delete;
	wxDataViewCheckIconTextRenderer& operator=(const wxDataViewCheckIconTextRenderer&) = delete;

    // This renderer can always display the 3rd ("indeterminate") checkbox
    // state if the model contains cells with wxCHK_UNDETERMINED value, but it
    // doesn't allow the user to set it by default. Call this method to allow
    // this to happen.
    void Allow3rdStateForUser(bool allow = true);

    bool MacRender() override;

    virtual void OSXOnCellChanged(NSObject *value,
                                  const wxDataViewItem& item,
                                  unsigned col) override;

private:
    bool m_allow3rdStateForUser;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------
// wxDataViewToggleRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_CORE wxDataViewToggleRenderer
    : public wxOSXDataViewDisabledInertRenderer
{
public:
    static wxString GetDefaultType() { return wxS("bool"); }

    wxDataViewToggleRenderer(const wxString& varianttype = GetDefaultType(),
                             wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                             int align = wxDVR_DEFAULT_ALIGNMENT);

	wxDataViewToggleRenderer(const wxDataViewToggleRenderer&) = delete;
	wxDataViewToggleRenderer& operator=(const wxDataViewToggleRenderer&) = delete;

    void ShowAsRadio();

    bool MacRender() override;

    virtual void OSXOnCellChanged(NSObject *value,
                                  const wxDataViewItem& item,
                                  unsigned col) override;

private:
    void DoInitButtonCell(int buttonType);

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------
// wxDataViewProgressRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_CORE wxDataViewProgressRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("long"); }

    wxDataViewProgressRenderer(const wxString& label = wxEmptyString,
                               const wxString& varianttype = GetDefaultType(),
                               wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                               int align = wxDVR_DEFAULT_ALIGNMENT);

	wxDataViewProgressRenderer(const wxDataViewProgressRenderer&) = delete;
	wxDataViewProgressRenderer& operator=(const wxDataViewProgressRenderer&) = delete;

    bool MacRender() override;

    virtual void OSXOnCellChanged(NSObject *value,
                                  const wxDataViewItem& item,
                                  unsigned col) override;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------
// wxDataViewDateRenderer
// ---------------------------------------------------------

class WXDLLIMPEXP_CORE wxDataViewDateRenderer: public wxDataViewRenderer
{
public:
    static wxString GetDefaultType() { return wxS("datetime"); }

    wxDataViewDateRenderer(const wxString& varianttype = GetDefaultType(),
                           wxDataViewCellMode mode = wxDATAVIEW_CELL_ACTIVATABLE,
                           int align = wxDVR_DEFAULT_ALIGNMENT);

	wxDataViewDateRenderer(const wxDataViewDateRenderer&) = delete;
	wxDataViewDateRenderer& operator=(const wxDataViewDateRenderer&) = delete;

    bool MacRender() override;

    virtual void OSXOnCellChanged(NSObject *value,
                                  const wxDataViewItem& item,
                                  unsigned col) override;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_OSX_DVRENDERERS_H_

