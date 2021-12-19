/////////////////////////////////////////////////////////////////////////////
// Name:        wx/cmndata.h
// Purpose:     Common GDI data classes
// Author:      Julian Smart and others
// Modified by:
// Created:     01/02/97
// Copyright:   (c)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

export module WX.Core.Cmndata;

import WX.Print;
import Utils.Geometry;

import <string>;

#if wxUSE_PRINTING_ARCHITECTURE

export
{

class wxPrintNativeDataBase;

inline constexpr unsigned int wxPRINTMEDIA_DEFAULT = 0;

class wxPrintData
{
public:
    wxPrintData();
    wxPrintData(const wxPrintData& printData);
    ~wxPrintData();

    int GetNoCopies() const { return m_printNoCopies; }
    bool GetCollate() const { return m_printCollate; }
    wxPrintOrientation GetOrientation() const { return m_printOrientation; }
    bool IsOrientationReversed() const { return m_printOrientationReversed; }

    // Is this data OK for showing the print dialog?
    bool IsOk() const ;

    const std::string& GetPrinterName() const { return m_printerName; }
    bool GetColour() const { return m_colour; }
    wxDuplexMode GetDuplex() const { return m_duplexMode; }
    wxPaperSize GetPaperId() const { return m_paperId; }
    const wxSize& GetPaperSize() const { return m_paperSize; }
    wxPrintQuality GetQuality() const { return m_printQuality; }
    wxPrintBin GetBin() const { return m_bin; }
    wxPrintMode GetPrintMode() const { return m_printMode; }
    int GetMedia() const { return m_media; }

    void SetNoCopies(int v) { m_printNoCopies = v; }
    void SetCollate(bool flag) { m_printCollate = flag; }

    void SetOrientation(wxPrintOrientation orient) { m_printOrientation = orient; }
    void SetOrientationReversed(bool reversed) { m_printOrientationReversed = reversed; }

    void SetPrinterName(const std::string& name) { m_printerName = name; }
    void SetColour(bool colour) { m_colour = colour; }
    void SetDuplex(wxDuplexMode duplex) { m_duplexMode = duplex; }
    void SetPaperId(wxPaperSize sizeId) { m_paperId = sizeId; }
    void SetPaperSize(const wxSize& sz) { m_paperSize = sz; }
    void SetQuality(wxPrintQuality quality) { m_printQuality = quality; }
    void SetBin(wxPrintBin bin) { m_bin = bin; }
    void SetMedia(int media) { m_media = media; }
    void SetPrintMode(wxPrintMode printMode) { m_printMode = printMode; }

    std::string GetFilename() const { return m_filename; }
    void SetFilename( const std::string& filename ) { m_filename = filename; }

    wxPrintData& operator=(const wxPrintData& data);

    char* GetPrivData() const { return m_privData; }
    int GetPrivDataLen() const { return m_privDataLen; }
    void SetPrivData( char *privData, int len );


    // Convert between wxPrintData and native data
    void ConvertToNative();
    void ConvertFromNative();
    // Holds the native print data
    wxPrintNativeDataBase *GetNativeData() const { return m_nativeData; }

private:
    std::string     m_printerName;
    std::string     m_filename;

    wxSize          m_paperSize;

    wxPrintNativeDataBase  *m_nativeData;
    char* m_privData{nullptr};

    int   m_media{wxPRINTMEDIA_DEFAULT};
    int   m_printNoCopies{1};
    int   m_privDataLen{0};

    wxPrintOrientation m_printOrientation{wxPrintOrientation::Portrait};
    wxPrintBin      m_bin{wxPrintBin::Default};
    wxPrintMode     m_printMode{wxPrintMode::Printer};
    wxDuplexMode    m_duplexMode{wxDuplexMode::Simplex};
    wxPrintQuality  m_printQuality{wxPRINT_QUALITY_HIGH};
    wxPaperSize     m_paperId{wxPaperSize::None};
    
    bool            m_colour{true};
    bool            m_printOrientationReversed{false};
    bool            m_printCollate{false};
};

/*
 * wxPrintDialogData
 * Encapsulates information displayed and edited in the printer dialog box.
 * Contains a wxPrintData object which is filled in according to the values retrieved
 * from the dialog.
 */

class wxPrintDialogData
{
public:
    wxPrintDialogData();
    wxPrintDialogData(const wxPrintDialogData& dialogData);
    wxPrintDialogData(const wxPrintData& printData);

    int GetFromPage() const { return m_printFromPage; }
    int GetToPage() const { return m_printToPage; }
    int GetMinPage() const { return m_printMinPage; }
    int GetMaxPage() const { return m_printMaxPage; }
    int GetNoCopies() const { return m_printNoCopies; }
    bool GetAllPages() const { return m_printAllPages; }
    bool GetSelection() const { return m_printSelection; }
    bool GetCollate() const { return m_printCollate; }
    bool GetPrintToFile() const { return m_printToFile; }

    void SetFromPage(int v) { m_printFromPage = v; }
    void SetToPage(int v) { m_printToPage = v; }
    void SetMinPage(int v) { m_printMinPage = v; }
    void SetMaxPage(int v) { m_printMaxPage = v; }
    void SetNoCopies(int v) { m_printNoCopies = v; }
    void SetAllPages(bool flag) { m_printAllPages = flag; }
    void SetSelection(bool flag) { m_printSelection = flag; }
    void SetCollate(bool flag) { m_printCollate = flag; }
    void SetPrintToFile(bool flag) { m_printToFile = flag; }

    void EnablePrintToFile(bool flag) { m_printEnablePrintToFile = flag; }
    void EnableSelection(bool flag) { m_printEnableSelection = flag; }
    void EnablePageNumbers(bool flag) { m_printEnablePageNumbers = flag; }
    void EnableHelp(bool flag) { m_printEnableHelp = flag; }

    bool GetEnablePrintToFile() const { return m_printEnablePrintToFile; }
    bool GetEnableSelection() const { return m_printEnableSelection; }
    bool GetEnablePageNumbers() const { return m_printEnablePageNumbers; }
    bool GetEnableHelp() const { return m_printEnableHelp; }

    // Is this data OK for showing the print dialog?
    bool IsOk() const { return m_printData.IsOk() ; }

    wxPrintData& GetPrintData() { return m_printData; }
    void SetPrintData(const wxPrintData& printData) { m_printData = printData; }

    void operator=(const wxPrintDialogData& data);
    void operator=(const wxPrintData& data); // Sets internal m_printData member

private:
    wxPrintData     m_printData;

    int             m_printFromPage{0};
    int             m_printToPage{0};
    int             m_printMinPage{0};
    int             m_printMaxPage{0};
    int             m_printNoCopies{1};
    
    bool            m_printAllPages{false};
    bool            m_printCollate{false};
    bool            m_printToFile{false};
    bool            m_printSelection{false};
    bool            m_printEnableSelection{false};
    bool            m_printEnablePageNumbers{true};
    bool            m_printEnableHelp{false};
    bool            m_printEnablePrintToFile{true}; // TODO: correct default behavior?
};

/*
* This is the data used (and returned) by the wxPageSetupDialog.
*/

class wxPageSetupDialogData
{
public:
    wxPageSetupDialogData();
    wxPageSetupDialogData(const wxPrintData& printData);

    wxSize GetPaperSize() const { return m_paperSize; }
    wxPaperSize GetPaperId() const { return m_printData.GetPaperId(); }
    wxPoint GetMinMarginTopLeft() const { return m_minMarginTopLeft; }
    wxPoint GetMinMarginBottomRight() const { return m_minMarginBottomRight; }
    wxPoint GetMarginTopLeft() const { return m_marginTopLeft; }
    wxPoint GetMarginBottomRight() const { return m_marginBottomRight; }

    bool GetDefaultMinMargins() const { return m_defaultMinMargins; }
    bool GetEnableMargins() const { return m_enableMargins; }
    bool GetEnableOrientation() const { return m_enableOrientation; }
    bool GetEnablePaper() const { return m_enablePaper; }
    bool GetEnablePrinter() const { return m_enablePrinter; }
    bool GetDefaultInfo() const { return m_getDefaultInfo; }
    bool GetEnableHelp() const { return m_enableHelp; }

    // Is this data OK for showing the page setup dialog?
    bool IsOk() const { return m_printData.IsOk() ; }

    // If a corresponding paper type is found in the paper database, will set the m_printData
    // paper size id member as well.
    void SetPaperSize(const wxSize& sz);

    void SetPaperId(wxPaperSize id) { m_printData.SetPaperId(id); }

    // Sets the wxPrintData id, plus the paper width/height if found in the paper database.
    void SetPaperSize(wxPaperSize id);

    void SetMinMarginTopLeft(const wxPoint& pt) { m_minMarginTopLeft = pt; }
    void SetMinMarginBottomRight(const wxPoint& pt) { m_minMarginBottomRight = pt; }
    void SetMarginTopLeft(const wxPoint& pt) { m_marginTopLeft = pt; }
    void SetMarginBottomRight(const wxPoint& pt) { m_marginBottomRight = pt; }
    void SetDefaultMinMargins(bool flag) { m_defaultMinMargins = flag; }
    void SetDefaultInfo(bool flag) { m_getDefaultInfo = flag; }

    void EnableMargins(bool flag) { m_enableMargins = flag; }
    void EnableOrientation(bool flag) { m_enableOrientation = flag; }
    void EnablePaper(bool flag) { m_enablePaper = flag; }
    void EnablePrinter(bool flag) { m_enablePrinter = flag; }
    void EnableHelp(bool flag) { m_enableHelp = flag; }

    // Use paper size defined in this object to set the wxPrintData
    // paper id
    void CalculateIdFromPaperSize();

    // Use paper id in wxPrintData to set this object's paper size
    void CalculatePaperSizeFromId();

    wxPageSetupDialogData& operator=(const wxPrintData& data);

    wxPrintData& GetPrintData() { return m_printData; }
    const wxPrintData& GetPrintData() const { return m_printData; }
    void SetPrintData(const wxPrintData& printData);

private:
    wxPrintData     m_printData;

    wxSize          m_paperSize; // The dimensions selected by the user (on return, same as in wxPrintData?)

    wxPoint         m_minMarginTopLeft{0, 0};
    wxPoint         m_minMarginBottomRight{0, 0};
    wxPoint         m_marginTopLeft{0, 0};
    wxPoint         m_marginBottomRight{0, 0};

    bool            m_defaultMinMargins{false};
    bool            m_enableMargins{true};
    bool            m_enableOrientation{true};
    bool            m_enablePaper{true};
    bool            m_enablePrinter{true};
    bool            m_getDefaultInfo{false}; // Equiv. to PSD_RETURNDEFAULT
    bool            m_enableHelp{false};
};

} // export

#endif // wxUSE_PRINTING_ARCHITECTURE
