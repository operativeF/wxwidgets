/////////////////////////////////////////////////////////////////////////////
// Name:        wx/prntbase.h
// Purpose:     Base classes for printing framework
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRNTBASEH__
#define _WX_PRNTBASEH__

#include "wx/defs.h"

#if wxUSE_PRINTING_ARCHITECTURE

#include "wx/event.h"
#include "wx/cmndata.h"
#include "wx/panel.h"
#include "wx/scrolwin.h"
#include "wx/dialog.h"
#include "wx/frame.h"
#include "wx/dc.h"
#include "wx/geometry/rect.h"


#include <string>

class WXDLLIMPEXP_FWD_CORE wxDC;
class WXDLLIMPEXP_FWD_CORE wxButton;
class WXDLLIMPEXP_FWD_CORE wxChoice;
class WXDLLIMPEXP_FWD_CORE wxPrintout;
class WXDLLIMPEXP_FWD_CORE wxPrinterBase;
class WXDLLIMPEXP_FWD_CORE wxPrintDialogBase;
class WXDLLIMPEXP_FWD_CORE wxPrintDialog;
class WXDLLIMPEXP_FWD_CORE wxPageSetupDialogBase;
class WXDLLIMPEXP_FWD_CORE wxPageSetupDialog;
class WXDLLIMPEXP_FWD_CORE wxPrintPreviewBase;
class WXDLLIMPEXP_FWD_CORE wxPreviewCanvas;
class WXDLLIMPEXP_FWD_CORE wxPreviewControlBar;
class WXDLLIMPEXP_FWD_CORE wxPreviewFrame;
class WXDLLIMPEXP_FWD_CORE wxPrintFactory;
class WXDLLIMPEXP_FWD_CORE wxPrintNativeDataBase;
class WXDLLIMPEXP_FWD_CORE wxPrintPreview;
class WXDLLIMPEXP_FWD_CORE wxPrintAbortDialog;
class WXDLLIMPEXP_FWD_CORE wxStaticText;
class wxPrintPageMaxCtrl;
class wxPrintPageTextCtrl;

//----------------------------------------------------------------------------
// error consts
//----------------------------------------------------------------------------

enum class wxPrinterError
{
    NoError,
    Cancelled,
    Error
};

// Preview frame modality kind used with wxPreviewFrame::Initialize()
enum class wxPreviewFrameModalityKind
{
    // Disable all the other top level windows while the preview is shown.
    AppModal,

    // Disable only the parent window while the preview is shown.
    WindowModal,

    // Don't disable any windows.
    NonModal
};

//----------------------------------------------------------------------------
// wxPrintFactory
//----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPrintFactory
{
public:
    wxPrintFactory() = default;
    virtual ~wxPrintFactory() = default;

    virtual wxPrinterBase *CreatePrinter( wxPrintDialogData* data ) = 0;

    virtual wxPrintPreviewBase *CreatePrintPreview( wxPrintout *preview,
                                                    wxPrintout *printout = nullptr,
                                                    wxPrintDialogData *data = nullptr ) = 0;
    virtual wxPrintPreviewBase *CreatePrintPreview( wxPrintout *preview,
                                                    wxPrintout *printout,
                                                    wxPrintData *data ) = 0;

    virtual wxPrintDialogBase *CreatePrintDialog( wxWindow *parent,
                                                  wxPrintDialogData *data = nullptr ) = 0;
    virtual wxPrintDialogBase *CreatePrintDialog( wxWindow *parent,
                                                  wxPrintData *data ) = 0;

    virtual wxPageSetupDialogBase *CreatePageSetupDialog( wxWindow *parent,
                                                          wxPageSetupDialogData * data = nullptr ) = 0;

    virtual std::unique_ptr<wxDCImpl> CreatePrinterDCImpl( wxPrinterDC *owner, const wxPrintData& data ) = 0;

    // What to do and what to show in the wxPrintDialog
    // a) Use the generic print setup dialog or a native one?
    virtual bool HasPrintSetupDialog() = 0;
    virtual wxDialog *CreatePrintSetupDialog( wxWindow *parent, wxPrintData *data ) = 0;
    // b) Provide the "print to file" option ourselves or via print setup?
    virtual bool HasOwnPrintToFile() = 0;
    // c) Show current printer
    virtual bool HasPrinterLine() = 0;
    virtual wxString CreatePrinterLine() = 0;
    // d) Show Status line for current printer?
    virtual bool HasStatusLine() = 0;
    virtual wxString CreateStatusLine() = 0;


    virtual wxPrintNativeDataBase *CreatePrintNativeData() = 0;

    static void SetPrintFactory( wxPrintFactory *factory );
    static wxPrintFactory *GetFactory();
private:
    inline static wxPrintFactory *m_factory{nullptr};
};

class WXDLLIMPEXP_CORE wxNativePrintFactory: public wxPrintFactory
{
public:
    wxPrinterBase *CreatePrinter( wxPrintDialogData *data ) override;

    wxPrintPreviewBase *CreatePrintPreview( wxPrintout *preview,
                                                    wxPrintout *printout = nullptr,
                                                    wxPrintDialogData *data = nullptr ) override;
    wxPrintPreviewBase *CreatePrintPreview( wxPrintout *preview,
                                                    wxPrintout *printout,
                                                    wxPrintData *data ) override;

    wxPrintDialogBase *CreatePrintDialog( wxWindow *parent,
                                                  wxPrintDialogData *data = nullptr ) override;
    wxPrintDialogBase *CreatePrintDialog( wxWindow *parent,
                                                  wxPrintData *data ) override;

    wxPageSetupDialogBase *CreatePageSetupDialog( wxWindow *parent,
                                                          wxPageSetupDialogData * data = nullptr ) override;

    std::unique_ptr<wxDCImpl> CreatePrinterDCImpl( wxPrinterDC *owner, const wxPrintData& data ) override;

    bool HasPrintSetupDialog() override;
    wxDialog *CreatePrintSetupDialog( wxWindow *parent, wxPrintData *data ) override;
    bool HasOwnPrintToFile() override;
    bool HasPrinterLine() override;
    wxString CreatePrinterLine() override;
    bool HasStatusLine() override;
    wxString CreateStatusLine() override;

    wxPrintNativeDataBase *CreatePrintNativeData() override;
};

//----------------------------------------------------------------------------
// wxPrintNativeDataBase
//----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPrintNativeDataBase
{
public:
    wxPrintNativeDataBase() = default;
    virtual ~wxPrintNativeDataBase() = default;

    wxPrintNativeDataBase(const wxPrintNativeDataBase&) = delete;
    wxPrintNativeDataBase& operator=(const wxPrintNativeDataBase&) = delete;
    wxPrintNativeDataBase(wxPrintNativeDataBase&&) = default;
    wxPrintNativeDataBase& operator=(wxPrintNativeDataBase&&) = default;

    virtual bool TransferTo( wxPrintData &data ) = 0;
    virtual bool TransferFrom( const wxPrintData &data ) = 0;
#ifdef __WXOSX__
    // in order to expose functionality already to the result type of the ..PrintData->GetNativeData()
    virtual void TransferFrom( const wxPageSetupDialogData * ) = 0;
    virtual void TransferTo( wxPageSetupDialogData * ) = 0;
#endif
    virtual bool IsOk() const = 0;

    int  m_ref{1};
};

//----------------------------------------------------------------------------
// wxPrinterBase
//----------------------------------------------------------------------------

/*
 * Represents the printer: manages printing a wxPrintout object
 */

class WXDLLIMPEXP_CORE wxPrinterBase
{
public:
    wxPrinterBase(wxPrintDialogData *data = nullptr);
    virtual ~wxPrinterBase() = default;

    wxPrinterBase(const wxPrinterBase&) = delete;
    wxPrinterBase& operator=(const wxPrinterBase&) = delete;
    wxPrinterBase(wxPrinterBase&&) = default;
    wxPrinterBase& operator=(wxPrinterBase&&) = default;

    virtual wxPrintAbortDialog *CreateAbortWindow(wxWindow *parent, wxPrintout *printout);
    virtual void ReportError(wxWindow *parent, wxPrintout *printout, const wxString& message);

    virtual wxPrintDialogData& GetPrintDialogData() const;
    bool GetAbort() const { return sm_abortIt; }

    static wxPrinterError GetLastError() { return sm_lastError; }

    ///////////////////////////////////////////////////////////////////////////
    // OVERRIDES

    virtual bool Setup(wxWindow *parent) = 0;
    virtual bool Print(wxWindow *parent, wxPrintout *printout, bool prompt = true) = 0;
    virtual wxDC* PrintDialog(wxWindow *parent) = 0;

protected:
    wxPrintDialogData     m_printDialogData;
    wxPrintout*           m_currentPrintout{nullptr};

    inline static wxPrinterError sm_lastError{wxPrinterError::NoError};

public:
    inline static wxWindow*      sm_abortWindow{nullptr};
    inline static bool           sm_abortIt{false};
};

//----------------------------------------------------------------------------
// wxPrinter
//----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPrinter: public wxPrinterBase
{
public:
    wxPrinter(wxPrintDialogData *data = nullptr);
    ~wxPrinter();

    wxPrinter(const wxPrinter&) = delete;
    wxPrinter& operator=(const wxPrinter&) = delete;
    wxPrinter(wxPrinter&&) = default;
    wxPrinter& operator=(wxPrinter&&) = default;

    wxPrintAbortDialog *CreateAbortWindow(wxWindow *parent, wxPrintout *printout) override;
    void ReportError(wxWindow *parent, wxPrintout *printout, const wxString& message) override;

    bool Setup(wxWindow *parent) override;
    bool Print(wxWindow *parent, wxPrintout *printout, bool prompt = true) override;
    wxDC* PrintDialog(wxWindow *parent) override;

    wxPrintDialogData& GetPrintDialogData() const override;

protected:
    wxPrinterBase    *m_pimpl;
};

//----------------------------------------------------------------------------
// wxPrintout
//----------------------------------------------------------------------------

/*
 * Represents an object via which a document may be printed.
 * The programmer derives from this, overrides (at least) OnPrintPage,
 * and passes it to a wxPrinter object for printing, or a wxPrintPreview
 * object for previewing.
 */

class WXDLLIMPEXP_CORE wxPrintout
{
public:
    wxPrintout(const wxString& title = wxGetTranslation("Printout")) :
        m_printoutTitle(title)    
    {
    }

    virtual ~wxPrintout() = default;

    wxPrintout(const wxPrintout&) = delete;
    wxPrintout& operator=(const wxPrintout&) = delete;
    wxPrintout(wxPrintout&&) = default;
    wxPrintout& operator=(wxPrintout&&) = default;

    virtual bool OnBeginDocument(int startPage, int endPage);
    virtual void OnEndDocument();
    virtual void OnBeginPrinting();
    virtual void OnEndPrinting();

    // Guaranteed to be before any other functions are called
    virtual void OnPreparePrinting() { }

    virtual bool HasPage(int page);
    virtual bool OnPrintPage(int page) = 0;
    virtual void GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo);

    virtual wxString GetTitle() const { return m_printoutTitle; }

    // Port-specific code should call this function to initialize this object
    // with everything it needs, instead of using individual accessors below.
    bool SetUp(wxDC& dc);

    wxDC *GetDC() const { return m_printoutDC; }
    void SetDC(wxDC *dc) { m_printoutDC = dc; }

    void FitThisSizeToPaper(const wxSize& imageSize);
    void FitThisSizeToPage(const wxSize& imageSize);
    void FitThisSizeToPageMargins(const wxSize& imageSize, const wxPageSetupDialogData& pageSetupData);
    void MapScreenSizeToPaper();
    void MapScreenSizeToPage();
    void MapScreenSizeToPageMargins(const wxPageSetupDialogData& pageSetupData);
    void MapScreenSizeToDevice();

    wxRect GetLogicalPaperRect() const;
    wxRect GetLogicalPageRect() const;
    wxRect GetLogicalPageMarginsRect(const wxPageSetupDialogData& pageSetupData) const;

    void SetLogicalOrigin(wxCoord x, wxCoord y);
    void OffsetLogicalOrigin(wxCoord xoff, wxCoord yoff);

    void SetPageSizePixels(int w, int  h) { m_pageWidthPixels = w; m_pageHeightPixels = h; }
    void GetPageSizePixels(int *w, int  *h) const { *w = m_pageWidthPixels; *h = m_pageHeightPixels; }
    void SetPageSizeMM(int w, int  h) { m_pageWidthMM = w; m_pageHeightMM = h; }
    void GetPageSizeMM(int *w, int  *h) const { *w = m_pageWidthMM; *h = m_pageHeightMM; }

    void SetPPIScreen(int x, int y) { m_PPIScreenX = x; m_PPIScreenY = y; }
    void SetPPIScreen(const wxSize& ppi) { SetPPIScreen(ppi.x, ppi.y); }
    void GetPPIScreen(int *x, int *y) const { *x = m_PPIScreenX; *y = m_PPIScreenY; }
    void SetPPIPrinter(int x, int y) { m_PPIPrinterX = x; m_PPIPrinterY = y; }
    void SetPPIPrinter(const wxSize& ppi) { SetPPIPrinter(ppi.x, ppi.y); }
    void GetPPIPrinter(int *x, int *y) const { *x = m_PPIPrinterX; *y = m_PPIPrinterY; }

    void SetPaperRectPixels(const wxRect& paperRectPixels) { m_paperRectPixels = paperRectPixels; }
    wxRect GetPaperRectPixels() const { return m_paperRectPixels; }

    // This must be called by wxPrintPreview to associate itself with the
    // printout it uses.
    virtual void SetPreview(wxPrintPreview *preview) { m_preview = preview; }

    wxPrintPreview *GetPreview() const { return m_preview; }
    virtual bool IsPreview() const { return GetPreview() != nullptr; }

private:
    wxString         m_printoutTitle;
    wxDC*            m_printoutDC{nullptr};
    wxPrintPreview  *m_preview{nullptr};

    int              m_pageWidthPixels{0};
    int              m_pageHeightPixels{0};

    int              m_pageWidthMM{0};
    int              m_pageHeightMM{0};

    int              m_PPIScreenX{0};
    int              m_PPIScreenY{0};
    int              m_PPIPrinterX{0};
    int              m_PPIPrinterY{0};

    wxRect           m_paperRectPixels;
};

//----------------------------------------------------------------------------
// wxPreviewCanvas
//----------------------------------------------------------------------------

/*
 * Canvas upon which a preview is drawn.
 */

class WXDLLIMPEXP_CORE wxPreviewCanvas: public wxScrolledWindow
{
public:
    wxPreviewCanvas(wxPrintPreviewBase *preview,
                    wxWindow *parent,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    unsigned int style = 0,
                    const wxString& name = wxT("canvas"));
    ~wxPreviewCanvas() = default;

    wxPreviewCanvas(const wxPreviewCanvas&) = delete;
    wxPreviewCanvas& operator=(const wxPreviewCanvas&) = delete;
    wxPreviewCanvas(wxPreviewCanvas&&) = default;
    wxPreviewCanvas& operator=(wxPreviewCanvas&&) = default;

    void SetPreview(wxPrintPreviewBase *preview) { m_printPreview = preview; }

    void OnPaint(wxPaintEvent& event);
    void OnChar(wxKeyEvent &event);
    // Responds to colour changes
    void OnSysColourChanged(wxSysColourChangedEvent& event);

private:
#if wxUSE_MOUSEWHEEL
    void OnMouseWheel(wxMouseEvent& event);
#endif // wxUSE_MOUSEWHEEL
    void OnIdle(wxIdleEvent& event);

    wxPrintPreviewBase* m_printPreview;

    wxDECLARE_CLASS(wxPreviewCanvas);
    wxDECLARE_EVENT_TABLE();
};

//----------------------------------------------------------------------------
// wxPreviewFrame
//----------------------------------------------------------------------------

/*
 * Default frame for showing preview.
 */

class WXDLLIMPEXP_CORE wxPreviewFrame: public wxFrame
{
public:
    wxPreviewFrame(wxPrintPreviewBase *preview,
                   wxWindow *parent,
                   const std::string& title = wxGetTranslation("Print Preview"),
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   unsigned int style = wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT,
                   const std::string& name = wxFrameNameStr);
    ~wxPreviewFrame();

    wxPreviewFrame(const wxPreviewFrame&) = delete;
    wxPreviewFrame& operator=(const wxPreviewFrame&) = delete;
    wxPreviewFrame(wxPreviewFrame&&) = default;
    wxPreviewFrame& operator=(wxPreviewFrame&&) = default;

    // Either Initialize() or InitializeWithModality() must be called before
    // showing the preview frame, the former being just a particular case of
    // the latter initializing the frame for being showing app-modally.

    // Notice that we must keep Initialize() with its existing signature to
    // avoid breaking the old code that overrides it and we can't reuse the
    // same name for the other functions to avoid virtual function hiding
    // problem and the associated warnings given by some compilers (e.g. from
    // g++ with -Woverloaded-virtual).
    virtual void Initialize()
    {
        InitializeWithModality(wxPreviewFrameModalityKind::AppModal);
    }

    // Also note that this method is not virtual as it doesn't need to be
    // overridden: it's never called by wxWidgets (of course, the same is true
    // for Initialize() but, again, it must remain virtual for compatibility).
    void InitializeWithModality(wxPreviewFrameModalityKind kind);

    void OnCloseWindow(wxCloseEvent& event);
    virtual void CreateCanvas();
    virtual void CreateControlBar();

    inline wxPreviewControlBar* GetControlBar() const { return m_controlBar; }

protected:
    wxPreviewCanvas*      m_previewCanvas{nullptr};
    wxPreviewControlBar*  m_controlBar{nullptr};
    wxPrintPreviewBase*   m_printPreview{nullptr};
    wxWindowDisabler*     m_windowDisabler{nullptr};

    wxPreviewFrameModalityKind m_modalityKind{wxPreviewFrameModalityKind::NonModal};


private:
    void OnChar(wxKeyEvent& event);

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_CLASS(wxPreviewFrame);
};

//----------------------------------------------------------------------------
// wxPreviewControlBar
//----------------------------------------------------------------------------

/*
 * A panel with buttons for controlling a print preview.
 * The programmer may wish to use other means for controlling
 * the print preview.
 */

#define wxPREVIEW_PRINT        1
#define wxPREVIEW_PREVIOUS     2
#define wxPREVIEW_NEXT         4
#define wxPREVIEW_ZOOM         8
#define wxPREVIEW_FIRST       16
#define wxPREVIEW_LAST        32
#define wxPREVIEW_GOTO        64

#define wxPREVIEW_DEFAULT  (wxPREVIEW_PREVIOUS|wxPREVIEW_NEXT|wxPREVIEW_ZOOM\
                            |wxPREVIEW_FIRST|wxPREVIEW_GOTO|wxPREVIEW_LAST)

// Ids for controls
#define wxID_PREVIEW_CLOSE      1
#define wxID_PREVIEW_NEXT       2
#define wxID_PREVIEW_PREVIOUS   3
#define wxID_PREVIEW_PRINT      4
#define wxID_PREVIEW_ZOOM       5
#define wxID_PREVIEW_FIRST      6
#define wxID_PREVIEW_LAST       7
#define wxID_PREVIEW_GOTO       8
#define wxID_PREVIEW_ZOOM_IN    9
#define wxID_PREVIEW_ZOOM_OUT   10

class WXDLLIMPEXP_CORE wxPreviewControlBar: public wxPanel
{
    wxDECLARE_CLASS(wxPreviewControlBar);

public:
    wxPreviewControlBar(wxPrintPreviewBase *preview,
                        unsigned int buttons,
                        wxWindow *parent,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        unsigned int style = wxTAB_TRAVERSAL,
                        const wxString& name = wxT("panel"));
    ~wxPreviewControlBar() = default;

    wxPreviewControlBar(const wxPreviewControlBar&) = delete;
    wxPreviewControlBar& operator=(const wxPreviewControlBar&) = delete;
    wxPreviewControlBar(wxPreviewControlBar&&) = default;
    wxPreviewControlBar& operator=(wxPreviewControlBar&&) = default;

    virtual void CreateButtons();
    virtual void SetPageInfo(int minPage, int maxPage);
    virtual void SetZoomControl(int zoom);
    virtual int GetZoomControl();
    virtual wxPrintPreviewBase *GetPrintPreview() const
        { return m_printPreview; }


    // Implementation only from now on.
    void OnWindowClose(wxCommandEvent& event);
    void OnNext();
    void OnPrevious();
    void OnFirst();
    void OnLast();
    void OnGotoPage();
    void OnPrint();

    void OnPrintButton(wxCommandEvent& WXUNUSED(event)) { OnPrint(); }
    void OnNextButton(wxCommandEvent & WXUNUSED(event)) { OnNext(); }
    void OnPreviousButton(wxCommandEvent & WXUNUSED(event)) { OnPrevious(); }
    void OnFirstButton(wxCommandEvent & WXUNUSED(event)) { OnFirst(); }
    void OnLastButton(wxCommandEvent & WXUNUSED(event)) { OnLast(); }
    void OnPaint(wxPaintEvent& event);

    void OnUpdateNextButton(wxUpdateUIEvent& event)
        { event.Enable(IsNextEnabled()); }
    void OnUpdatePreviousButton(wxUpdateUIEvent& event)
        { event.Enable(IsPreviousEnabled()); }
    void OnUpdateFirstButton(wxUpdateUIEvent& event)
        { event.Enable(IsFirstEnabled()); }
    void OnUpdateLastButton(wxUpdateUIEvent& event)
        { event.Enable(IsLastEnabled()); }
    void OnUpdateZoomInButton(wxUpdateUIEvent& event)
        { event.Enable(IsZoomInEnabled()); }
    void OnUpdateZoomOutButton(wxUpdateUIEvent& event)
        { event.Enable(IsZoomOutEnabled()); }

    // These methods are not private because they are called by wxPreviewCanvas.
    void DoZoomIn();
    void DoZoomOut();

protected:
    wxPrintPreviewBase*   m_printPreview;
    wxButton*             m_closeButton{nullptr};
    wxChoice*             m_zoomControl{nullptr};
    wxPrintPageTextCtrl*  m_currentPageText{nullptr};
    wxPrintPageMaxCtrl*   m_maxPageText{nullptr};

    unsigned int          m_buttonFlags;

private:
    void DoGotoPage(int page);

    void DoZoom();

    bool IsNextEnabled() const;
    bool IsPreviousEnabled() const;
    bool IsFirstEnabled() const;
    bool IsLastEnabled() const;
    bool IsZoomInEnabled() const;
    bool IsZoomOutEnabled() const;

    void OnZoomInButton(wxCommandEvent & WXUNUSED(event)) { DoZoomIn(); }
    void OnZoomOutButton(wxCommandEvent & WXUNUSED(event)) { DoZoomOut(); }
    void OnZoomChoice(wxCommandEvent& WXUNUSED(event)) { DoZoom(); }

    wxDECLARE_EVENT_TABLE();
};

//----------------------------------------------------------------------------
// wxPrintPreviewBase
//----------------------------------------------------------------------------

/*
 * Programmer creates an object of this class to preview a wxPrintout.
 */

class WXDLLIMPEXP_CORE wxPrintPreviewBase
{
public:
    wxPrintPreviewBase(wxPrintout *printout,
                       wxPrintout *printoutForPrinting = nullptr,
                       wxPrintDialogData *data = nullptr);
    wxPrintPreviewBase(wxPrintout *printout,
                       wxPrintout *printoutForPrinting,
                       wxPrintData *data);
    virtual ~wxPrintPreviewBase();

    wxPrintPreviewBase(const wxPrintPreviewBase&) = delete;
    wxPrintPreviewBase& operator=(const wxPrintPreviewBase&) = delete;
    wxPrintPreviewBase(wxPrintPreviewBase&&) = default;
    wxPrintPreviewBase& operator=(wxPrintPreviewBase&&) = default;

    virtual bool SetCurrentPage(int pageNum);
    virtual int GetCurrentPage() const;

    virtual void SetPrintout(wxPrintout *printout);
    virtual wxPrintout *GetPrintout() const;
    virtual wxPrintout *GetPrintoutForPrinting() const;

    virtual void SetFrame(wxFrame *frame);
    virtual void SetCanvas(wxPreviewCanvas *canvas);

    virtual wxFrame *GetFrame() const;
    virtual wxPreviewCanvas *GetCanvas() const;

    // This is a helper routine, used by the next 4 routines.

    virtual void CalcRects(wxPreviewCanvas *canvas, wxRect& printableAreaRect, wxRect& paperRect);

    // The preview canvas should call this from OnPaint
    virtual bool PaintPage(wxPreviewCanvas *canvas, wxDC& dc);

    // Updates rendered page by calling RenderPage() if needed, returns true
    // if there was some change. Preview canvas should call it at idle time
    virtual bool UpdatePageRendering();

    // This draws a blank page onto the preview canvas
    virtual bool DrawBlankPage(wxPreviewCanvas *canvas, wxDC& dc);

    // Adjusts the scrollbars for the current scale
    virtual void AdjustScrollbars(wxPreviewCanvas *canvas);

    // This is called by wxPrintPreview to render a page into a wxMemoryDC.
    virtual bool RenderPage(int pageNum);


    virtual void SetZoom(int percent);
    virtual int GetZoom() const;

    virtual wxPrintDialogData& GetPrintDialogData();

    virtual int GetMaxPage() const;
    virtual int GetMinPage() const;

    virtual bool IsOk() const;
    virtual void SetOk(bool ok);

    ///////////////////////////////////////////////////////////////////////////
    // OVERRIDES

    // If we own a wxPrintout that can be used for printing, this
    // will invoke the actual printing procedure. Called
    // by the wxPreviewControlBar.
    virtual bool Print(bool interactive) = 0;

    // Calculate scaling that needs to be done to get roughly
    // the right scaling for the screen pretending to be
    // the currently selected printer.
    virtual void DetermineScaling() = 0;

protected:
    // helpers for RenderPage():
    virtual bool RenderPageIntoDC(wxDC& dc, int pageNum);
    // renders preview into m_previewBitmap
    virtual bool RenderPageIntoBitmap(wxBitmap& bmp, int pageNum);

    void InvalidatePreviewBitmap();

protected:
    wxPrintDialogData m_printDialogData;
    
    wxPreviewCanvas*  m_previewCanvas;
    wxFrame*          m_previewFrame;
    wxBitmap*         m_previewBitmap;
    wxPrintout*       m_previewPrintout;
    wxPrintout*       m_printPrintout;
    
    int               m_currentPage;
    int               m_currentZoom;
    int               m_topMargin;
    int               m_leftMargin;
    int               m_pageWidth;
    int               m_pageHeight;
    int               m_minPage;
    int               m_maxPage;

    float             m_previewScaleX;
    float             m_previewScaleY;
    
    bool              m_previewFailed;
    bool              m_isOk;
    bool              m_printingPrepared; // Called OnPreparePrinting?

private:
    void Init(wxPrintout *printout, wxPrintout *printoutForPrinting);
};

//----------------------------------------------------------------------------
// wxPrintPreview
//----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPrintPreview: public wxPrintPreviewBase
{
public:
    wxPrintPreview(wxPrintout *printout,
                   wxPrintout *printoutForPrinting = nullptr,
                   wxPrintDialogData *data = nullptr);
    wxPrintPreview(wxPrintout *printout,
                   wxPrintout *printoutForPrinting,
                   wxPrintData *data);
    ~wxPrintPreview();

    wxPrintPreview(const wxPrintPreview&) = delete;
    wxPrintPreview& operator=(const wxPrintPreview&) = delete;
    wxPrintPreview(wxPrintPreview&&) = default;
    wxPrintPreview& operator=(wxPrintPreview&&) = default;

    bool SetCurrentPage(int pageNum) override;
    int GetCurrentPage() const override;
    void SetPrintout(wxPrintout *printout) override;
    wxPrintout *GetPrintout() const override;
    wxPrintout *GetPrintoutForPrinting() const override;
    void SetFrame(wxFrame *frame) override;
    void SetCanvas(wxPreviewCanvas *canvas) override;

    wxFrame *GetFrame() const override;
    wxPreviewCanvas *GetCanvas() const override;
    bool PaintPage(wxPreviewCanvas *canvas, wxDC& dc) override;
    bool UpdatePageRendering() override;
    bool DrawBlankPage(wxPreviewCanvas *canvas, wxDC& dc) override;
    void AdjustScrollbars(wxPreviewCanvas *canvas) override;
    bool RenderPage(int pageNum) override;
    void SetZoom(int percent) override;
    int GetZoom() const override;

    bool Print(bool interactive) override;
    void DetermineScaling() override;

    wxPrintDialogData& GetPrintDialogData() override;

    int GetMaxPage() const override;
    int GetMinPage() const override;

    bool IsOk() const override;
    void SetOk(bool ok) override;

private:
    wxPrintPreviewBase *m_pimpl;

private:
    wxDECLARE_CLASS(wxPrintPreview);
};

//----------------------------------------------------------------------------
// wxPrintAbortDialog
//----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPrintAbortDialog: public wxDialog
{
public:
    wxPrintAbortDialog(wxWindow *parent,
                       const wxString& documentTitle,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       unsigned int style = wxDEFAULT_DIALOG_STYLE,
                       const wxString& name = wxT("dialog"));

    wxPrintAbortDialog(const wxPrintAbortDialog&) = delete;
    wxPrintAbortDialog& operator=(const wxPrintAbortDialog&) = delete;
    wxPrintAbortDialog(wxPrintAbortDialog&&) = default;
    wxPrintAbortDialog& operator=(wxPrintAbortDialog&&) = default;

    void SetProgress(int currentPage, int totalPages,
                     int currentCopy, int totalCopies);

    void OnCancel(wxCommandEvent& event);

private:
    wxStaticText *m_progress;

    wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_PRINTING_ARCHITECTURE

#endif
    // _WX_PRNTBASEH__
