/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtext/richtextprint.h
// Purpose:     Rich text printing classes
// Author:      Julian Smart
// Created:     2006-10-23
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RICHTEXTPRINT_H_
#define _WX_RICHTEXTPRINT_H_

#include "wx/defs.h"

#if wxUSE_RICHTEXT & wxUSE_PRINTING_ARCHITECTURE

#include "wx/richtext/richtextbuffer.h"

#include "wx/print.h"
#include "wx/printdlg.h"

#define wxRICHTEXT_PRINT_MAX_PAGES 99999

// Header/footer page identifiers
enum wxRichTextOddEvenPage {
    wxRICHTEXT_PAGE_ODD,
    wxRICHTEXT_PAGE_EVEN,
    wxRICHTEXT_PAGE_ALL
};

// Header/footer text locations
enum class wxRichTextPageLocation {
    Left,
    Centre,
    Center = Centre,
    Right
};

/*!
 * Header/footer data
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextHeaderFooterData: public wxObject
{
public:
    wxRichTextHeaderFooterData() { Init(); }
    wxRichTextHeaderFooterData(const wxRichTextHeaderFooterData& data) { Copy(data); }

    /// Initialise
    void Init() { m_headerMargin = 20; m_footerMargin = 20; m_showOnFirstPage = true; }

    /// Copy
    void Copy(const wxRichTextHeaderFooterData& data);

    /// Assignment
    void operator= (const wxRichTextHeaderFooterData& data) { Copy(data); }

    /// Set/get header text, e.g. wxRICHTEXT_PAGE_ODD, wxRichTextPageLocation::Left
    void SetHeaderText(const wxString& text, wxRichTextOddEvenPage page = wxRICHTEXT_PAGE_ALL, wxRichTextPageLocation location = wxRichTextPageLocation::Centre);
    wxString GetHeaderText(wxRichTextOddEvenPage page = wxRICHTEXT_PAGE_EVEN, wxRichTextPageLocation location = wxRichTextPageLocation::Centre) const;

    /// Set/get footer text, e.g. wxRICHTEXT_PAGE_ODD, wxRichTextPageLocation::Left
    void SetFooterText(const wxString& text, wxRichTextOddEvenPage page = wxRICHTEXT_PAGE_ALL, wxRichTextPageLocation location = wxRichTextPageLocation::Centre);
    wxString GetFooterText(wxRichTextOddEvenPage page = wxRICHTEXT_PAGE_EVEN, wxRichTextPageLocation location = wxRichTextPageLocation::Centre) const;

    /// Set/get text
    void SetText(const wxString& text, int headerFooter, wxRichTextOddEvenPage page, wxRichTextPageLocation location);
    wxString GetText(int headerFooter, wxRichTextOddEvenPage page, wxRichTextPageLocation location) const;

    /// Set/get margins between text and header or footer, in tenths of a millimeter
    void SetMargins(int headerMargin, int footerMargin) { m_headerMargin = headerMargin; m_footerMargin = footerMargin; }
    int GetHeaderMargin() const { return m_headerMargin; }
    int GetFooterMargin() const { return m_footerMargin; }

    /// Set/get whether to show header or footer on first page
    void SetShowOnFirstPage(bool showOnFirstPage) { m_showOnFirstPage = showOnFirstPage; }
    bool GetShowOnFirstPage() const { return m_showOnFirstPage; }

    /// Clear all text
    void Clear();

    /// Set/get font
    void SetFont(const wxFont& font) { m_font = font; }
    const wxFont& GetFont() const { return m_font; }

    /// Set/get colour
    void SetTextColour(const wxColour& col) { m_colour = col; }
    const wxColour& GetTextColour() const { return m_colour; }

    wxDECLARE_CLASS(wxRichTextHeaderFooterData);

private:

    // Strings for left, centre, right, top, bottom, odd, even
    wxString    m_text[12];
    wxFont      m_font;
    wxColour    m_colour;
    int         m_headerMargin;
    int         m_footerMargin;
    bool        m_showOnFirstPage;
};

/*!
 * wxRichTextPrintout
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextPrintout : public wxPrintout
{
public:
    wxRichTextPrintout(const wxString& title = wxGetTranslation("Printout"));
    ~wxRichTextPrintout() override = default;

    wxRichTextPrintout(const wxRichTextPrintout&) = delete;
	wxRichTextPrintout& operator=(const wxRichTextPrintout&) = delete;

    /// The buffer to print
    void SetRichTextBuffer(wxRichTextBuffer* buffer) { m_richTextBuffer = buffer; }
    wxRichTextBuffer* GetRichTextBuffer() const { return m_richTextBuffer; }

    /// Set/get header/footer data
    void SetHeaderFooterData(const wxRichTextHeaderFooterData& data) { m_headerFooterData = data; }
    const wxRichTextHeaderFooterData& GetHeaderFooterData() const { return m_headerFooterData; }

    /// Sets margins in 10ths of millimetre. Defaults to 1 inch for margins.
    void SetMargins(int top = 254, int bottom = 254, int left = 254, int right = 254);

    /// Calculate scaling and rectangles, setting the device context scaling
    void CalculateScaling(wxDC* dc, wxRect& textRect, wxRect& headerRect, wxRect& footerRect);

    // wxPrintout virtual functions
    bool OnPrintPage(int page) override;
    bool HasPage(int page) override;
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo) override;
    bool OnBeginDocument(int startPage, int endPage) override;
    void OnPreparePrinting() override;

private:

    /// Renders one page into dc
    void RenderPage(wxDC *dc, int page);

    /// Substitute keywords
    static bool SubstituteKeywords(wxString& str, const wxString& title, int pageNum, int pageCount);

private:

    wxRichTextBuffer*           m_richTextBuffer;
    int                         m_numPages{wxRICHTEXT_PRINT_MAX_PAGES};
    std::vector<int>                  m_pageBreaksStart;
    std::vector<int>                  m_pageBreaksEnd;
    std::vector<int>                  m_pageYOffsets;
    int                         m_marginLeft, m_marginTop, m_marginRight, m_marginBottom;

    wxRichTextHeaderFooterData  m_headerFooterData;
};

/*
 *! wxRichTextPrinting
 * A simple interface to perform wxRichTextBuffer printing.
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextPrinting : public wxObject
{
public:
    wxRichTextPrinting(const wxString& name = wxGetTranslation("Printing"), wxWindow *parentWindow = nullptr);
    ~wxRichTextPrinting() override;

    /// Preview the file or buffer
#if wxUSE_FFILE && wxUSE_STREAMS
    bool PreviewFile(const wxString& richTextFile);
#endif
    bool PreviewBuffer(const wxRichTextBuffer& buffer);

    /// Print the file or buffer
#if wxUSE_FFILE && wxUSE_STREAMS
    bool PrintFile(const wxString& richTextFile, bool showPrintDialog = true);
#endif
    bool PrintBuffer(const wxRichTextBuffer& buffer, bool showPrintDialog = true);

    /// Shows page setup dialog
    void PageSetup();

    /// Set/get header/footer data
    void SetHeaderFooterData(const wxRichTextHeaderFooterData& data) { m_headerFooterData = data; }
    const wxRichTextHeaderFooterData& GetHeaderFooterData() const { return m_headerFooterData; }

    /// Set/get header text, e.g. wxRICHTEXT_PAGE_ODD, wxRichTextPageLocation::Left
    void SetHeaderText(const wxString& text, wxRichTextOddEvenPage page = wxRICHTEXT_PAGE_ALL, wxRichTextPageLocation location = wxRichTextPageLocation::Centre);
    wxString GetHeaderText(wxRichTextOddEvenPage page = wxRICHTEXT_PAGE_EVEN, wxRichTextPageLocation location = wxRichTextPageLocation::Centre) const;

    /// Set/get footer text, e.g. wxRICHTEXT_PAGE_ODD, wxRichTextPageLocation::Left
    void SetFooterText(const wxString& text, wxRichTextOddEvenPage page = wxRICHTEXT_PAGE_ALL, wxRichTextPageLocation location = wxRichTextPageLocation::Centre);
    wxString GetFooterText(wxRichTextOddEvenPage page = wxRICHTEXT_PAGE_EVEN, wxRichTextPageLocation location = wxRichTextPageLocation::Centre) const;

    /// Show header/footer on first page, or not
    void SetShowOnFirstPage(bool show) { m_headerFooterData.SetShowOnFirstPage(show); }

    /// Set the font
    void SetHeaderFooterFont(const wxFont& font) { m_headerFooterData.SetFont(font); }

    /// Set the colour
    void SetHeaderFooterTextColour(const wxColour& font) { m_headerFooterData.SetTextColour(font); }

    /// Get print and page setup data
    wxPrintData *GetPrintData();
    wxPageSetupDialogData *GetPageSetupData() { return m_pageSetupData; }

    /// Set print and page setup data
    void SetPrintData(const wxPrintData& printData);
    void SetPageSetupData(const wxPageSetupDialogData& pageSetupData);

    /// Set the rich text buffer pointer, deleting the existing object if present
    void SetRichTextBufferPreview(wxRichTextBuffer* buf);
    wxRichTextBuffer* GetRichTextBufferPreview() const { return m_richTextBufferPreview; }

    void SetRichTextBufferPrinting(wxRichTextBuffer* buf);
    wxRichTextBuffer* GetRichTextBufferPrinting() const { return m_richTextBufferPrinting; }

    /// Set/get the parent window
    void SetParentWindow(wxWindow* parent) { m_parentWindow = parent; }
    wxWindow* GetParentWindow() const { return m_parentWindow; }

    /// Set/get the title
    void SetTitle(const wxString& title) { m_title = title; }
    const wxString& GetTitle() const { return m_title; }

    /// Set/get the preview rect
    void SetPreviewRect(const wxRect& rect) { m_previewRect = rect; }
    const wxRect& GetPreviewRect() const { return m_previewRect; }

protected:
    virtual wxRichTextPrintout *CreatePrintout();
    virtual bool DoPreview(wxRichTextPrintout *printout1, wxRichTextPrintout *printout2);
    virtual bool DoPrint(wxRichTextPrintout *printout, bool showPrintDialog);

private:
    wxPrintData*                m_printData{nullptr};
    wxPageSetupDialogData*      m_pageSetupData;

    wxRichTextHeaderFooterData  m_headerFooterData;
    wxString                    m_title;
    wxWindow*                   m_parentWindow;
    wxRichTextBuffer*           m_richTextBufferPreview{nullptr};
    wxRichTextBuffer*           m_richTextBufferPrinting{nullptr};
    wxRect                      m_previewRect;

    wxRichTextPrinting(const wxRichTextPrinting&) = delete;
	wxRichTextPrinting& operator=(const wxRichTextPrinting&) = delete;
};

#endif  // wxUSE_RICHTEXT & wxUSE_PRINTING_ARCHITECTURE

#endif // _WX_RICHTEXTPRINT_H_

