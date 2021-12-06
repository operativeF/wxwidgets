///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/wizard.h
// Purpose:     declaration of generic wxWizard class
// Author:      Vadim Zeitlin
// Modified by: Robert Vazan (sizers)
// Created:     28.09.99
// Copyright:   (c) 1999 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_WIZARD_H_
#define _WX_GENERIC_WIZARD_H_

// ----------------------------------------------------------------------------
// wxWizard
// ----------------------------------------------------------------------------

class wxButton;
class wxStaticBitmap;
class wxWizardEvent;
class wxBoxSizer;
class wxWizardSizer;

class wxWizard : public wxWizardBase
{
public:
    wxWizard() = default;
    wxWizard(wxWindow *parent,
             int id = wxID_ANY,
             const std::string& title = {},
             const wxBitmap& bitmap = wxNullBitmap,
             const wxPoint& pos = wxDefaultPosition,
             unsigned int style = wxDEFAULT_DIALOG_STYLE)
    {
        Create(parent, id, title, bitmap, pos, style);
    }

	wxWizard& operator=(wxWizard&&) = delete;

    bool Create(wxWindow *parent,
             int id = wxID_ANY,
             const std::string& title = {},
             const wxBitmap& bitmap = wxNullBitmap,
             const wxPoint& pos = wxDefaultPosition,
             unsigned int style = wxDEFAULT_DIALOG_STYLE);

    ~wxWizard();

    bool RunWizard(wxWizardPage *firstPage) override;
    wxWizardPage *GetCurrentPage() const override;
    void SetPageSize(const wxSize& size) override;
    wxSize GetPageSize() const override;
    void FitToPage(const wxWizardPage *firstPage) override;
    wxSizer *GetPageAreaSizer() const override;
    void SetBorder(int border) override;

    const wxBitmap& GetBitmap() const { return m_bitmap; }
    void SetBitmap(const wxBitmap& bitmap);

    // implementation only from now on
    // -------------------------------

    // is the wizard running?
    bool IsRunning() const { return m_page != nullptr; }

    // show the prev/next page, but call TransferDataFromWindow on the current
    // page first and return false without changing the page if
    // TransferDataFromWindow() returns false - otherwise, returns true
    [[maybe_unused]] virtual bool ShowPage(wxWizardPage *page, bool goingForward = true);

    // do fill the dialog with controls
    // this is app-overridable to, for example, set help and tooltip text
    virtual void DoCreateControls();

    // Do the adaptation
    bool DoLayoutAdaptation() override;

    // Set/get bitmap background colour
    void SetBitmapBackgroundColour(const wxColour& colour) { m_bitmapBackgroundColour = colour; }
    const wxColour& GetBitmapBackgroundColour() const { return m_bitmapBackgroundColour; }

    // Set/get bitmap placement (centred, tiled etc.)
    void SetBitmapPlacement(int placement) { m_bitmapPlacement = placement; }
    int GetBitmapPlacement() const { return m_bitmapPlacement; }

    // Set/get minimum bitmap width
    void SetMinimumBitmapWidth(int w) { m_bitmapMinimumWidth = w; }
    int GetMinimumBitmapWidth() const { return m_bitmapMinimumWidth; }

    // Tile bitmap
    static bool TileBitmap(const wxRect& rect, wxDC& dc, const wxBitmap& bitmap);

protected:
    // for compatibility only, doesn't do anything any more
    void FinishLayout() { }

    // Do fit, and adjust to screen size if necessary
    virtual void DoWizardLayout();

    // Resize bitmap if necessary
    virtual bool ResizeBitmap(wxBitmap& bmp);

    // was the dialog really created?
    bool WasCreated() const { return m_btnPrev != nullptr; }

    // event handlers
    void OnCancel(wxCommandEvent& event);
    void OnBackOrNext(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);

    void OnWizEvent(wxWizardEvent& event);

    void AddBitmapRow(wxBoxSizer *mainColumn);
    void AddStaticLine(wxBoxSizer *mainColumn);
    void AddBackNextPair(wxBoxSizer *buttonRow);
    void AddButtonRow(wxBoxSizer *mainColumn);

    wxBitmap      m_bitmap;     // the default bitmap to show

    // cached labels so their translations stay consistent
    std::string    m_nextLabel;
    std::string    m_finishLabel;

    // Bitmap background colour if resizing bitmap
    wxColour    m_bitmapBackgroundColour{*wxWHITE};

    // the page size requested by user
    wxSize m_sizePage;

    // the dialog position from the ctor
    wxPoint m_posWizard{wxDefaultPosition};

    // Actual position and size of pages
    wxWizardSizer *m_sizerPage{nullptr};

    // wizard state
    wxWizardPage *m_page{nullptr};       // the current page or NULL
    wxWizardPage *m_firstpage{nullptr};  // the page RunWizard started on or NULL

    // Page area sizer will be inserted here with padding
    wxBoxSizer *m_sizerBmpAndPage{nullptr};

    // wizard controls
    wxButton       *m_btnPrev{nullptr};  // the "<Back" button
    wxButton       *m_btnNext{nullptr};  // the "Next>" or "Finish" button
    wxStaticBitmap *m_statbmp{nullptr};  // the control for the bitmap

    // Border around page area sizer requested using SetBorder()
    int m_border{5};

    // Bitmap placement flags
    int         m_bitmapPlacement{0};

    // Minimum bitmap width
    int         m_bitmapMinimumWidth{115};

    // Whether RunWizard() was called
    bool m_started{false};

    // Whether was modal (modeless has to be destroyed on finish or cancel)
    bool m_wasModal{false};

    // True if pages are laid out using the sizer
    bool m_usingSizer{false};

    friend class wxWizardSizer;

    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_GENERIC_WIZARD_H_
