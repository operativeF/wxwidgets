/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/cmndata.cpp
// Purpose:     Common GDI data
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_PRINTING_ARCHITECTURE

#include "wx/cmndata.h"

#ifndef WX_PRECOMP
    #if defined(__WXMSW__)
        #include "wx/msw/wrapcdlg.h"
    #endif // MSW
    #include "wx/app.h"
    #include "wx/gdicmn.h"
    #include "wx/log.h"
    #include "wx/utils.h"
#endif

#include "wx/prntbase.h"
#include "wx/printdlg.h"
#include "wx/paper.h"

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// Print data
// ----------------------------------------------------------------------------

wxPrintData::wxPrintData()
    : m_paperSize(wxDefaultSize),
      m_nativeData(wxPrintFactory::GetFactory()->CreatePrintNativeData())
{
    // we intentionally don't initialize paper id and size at all, like this
    // the default system settings will be used for them
}

wxPrintData::wxPrintData(const wxPrintData& printData)
{
    (*this) = printData;
}

void wxPrintData::SetPrivData( char *privData, int len )
{
    wxDELETEA(m_privData);
    m_privDataLen = len;
    if (m_privDataLen > 0)
    {
        m_privData = new char[m_privDataLen];
        memcpy( m_privData, privData, m_privDataLen );
    }
}

wxPrintData::~wxPrintData()
{
    m_nativeData->m_ref--;
    if (m_nativeData->m_ref == 0)
        delete m_nativeData;

    delete[] m_privData;
}

void wxPrintData::ConvertToNative()
{
    m_nativeData->TransferFrom( *this ) ;
}

void wxPrintData::ConvertFromNative()
{
    m_nativeData->TransferTo( *this ) ;
}

wxPrintData& wxPrintData::operator=(const wxPrintData& data)
{
    if ( &data == this )
        return *this;

    m_printNoCopies = data.m_printNoCopies;
    m_printCollate = data.m_printCollate;
    m_printOrientation = data.m_printOrientation;
    m_printOrientationReversed = data.m_printOrientationReversed;
    m_printerName = data.m_printerName;
    m_colour = data.m_colour;
    m_duplexMode = data.m_duplexMode;
    m_printQuality = data.m_printQuality;
    m_paperId = data.m_paperId;
    m_paperSize = data.m_paperSize;
    m_bin = data.m_bin;
    m_media = data.m_media;
    m_printMode = data.m_printMode;
    m_filename = data.m_filename;

    // UnRef old m_nativeData
    if (m_nativeData)
    {
        m_nativeData->m_ref--;
        if (m_nativeData->m_ref == 0)
            delete m_nativeData;
    }
    // Set Ref new one
    m_nativeData = data.GetNativeData();
    m_nativeData->m_ref++;

    wxDELETEA(m_privData);
    m_privDataLen = data.GetPrivDataLen();
    if (m_privDataLen > 0)
    {
        m_privData = new char[m_privDataLen];
        memcpy( m_privData, data.GetPrivData(), m_privDataLen );
    }

    return *this;
}

// Is this data OK for showing the print dialog?
bool wxPrintData::IsOk() const
{
    m_nativeData->TransferFrom( *this );

    return m_nativeData->IsOk();
}

// ----------------------------------------------------------------------------
// Print dialog data
// ----------------------------------------------------------------------------

wxPrintDialogData::wxPrintDialogData()
{
    wxPrintFactory* factory = wxPrintFactory::GetFactory();
    m_printEnablePrintToFile = ! factory->HasOwnPrintToFile();
}

wxPrintDialogData::wxPrintDialogData(const wxPrintDialogData& dialogData)
     
{
    (*this) = dialogData;
}

wxPrintDialogData::wxPrintDialogData(const wxPrintData& printData)
    : m_printData(printData),
      m_printFromPage(1),
      m_printMinPage(1),
      m_printMaxPage(9999)
          // On Mac the Print dialog always defaults to "All Pages"
#ifdef __WXMAC__
      , m_printAllPages{true}
#endif
{
}

// FIXME: Not needed?
void wxPrintDialogData::operator=(const wxPrintDialogData& data)
{
    m_printFromPage = data.m_printFromPage;
    m_printToPage = data.m_printToPage;
    m_printMinPage = data.m_printMinPage;
    m_printMaxPage = data.m_printMaxPage;
    m_printNoCopies = data.m_printNoCopies;
    m_printAllPages = data.m_printAllPages;
    m_printCollate = data.m_printCollate;
    m_printToFile = data.m_printToFile;
    m_printSelection = data.m_printSelection;
    m_printEnableSelection = data.m_printEnableSelection;
    m_printEnablePageNumbers = data.m_printEnablePageNumbers;
    m_printEnableHelp = data.m_printEnableHelp;
    m_printEnablePrintToFile = data.m_printEnablePrintToFile;
    m_printData = data.m_printData;
}

void wxPrintDialogData::operator=(const wxPrintData& data)
{
    m_printData = data;
}

// ----------------------------------------------------------------------------
// wxPageSetupDialogData
// ----------------------------------------------------------------------------

wxPageSetupDialogData::wxPageSetupDialogData()
{
    CalculatePaperSizeFromId();
}

wxPageSetupDialogData::wxPageSetupDialogData(const wxPrintData& printData)
    : m_printData(printData)
{
    // The wxPrintData paper size overrides these values, unless the size cannot
    // be found.
    CalculatePaperSizeFromId();
}

wxPageSetupDialogData& wxPageSetupDialogData::operator=(const wxPrintData& data)
{
    m_printData = data;
    CalculatePaperSizeFromId();

    return *this;
}

// If a corresponding paper type is found in the paper database, will set the m_printData
// paper size id member as well.
void wxPageSetupDialogData::SetPaperSize(const wxSize& sz)
{
    m_paperSize = sz;

    CalculateIdFromPaperSize();
}

// Sets the wxPrintData id, plus the paper width/height if found in the paper database.
void wxPageSetupDialogData::SetPaperSize(wxPaperSize id)
{
    m_printData.SetPaperId(id);

    CalculatePaperSizeFromId();
}

void wxPageSetupDialogData::SetPrintData(const wxPrintData& printData)
{
    m_printData = printData;
    CalculatePaperSizeFromId();
}

// Use paper size defined in this object to set the wxPrintData
// paper id
void wxPageSetupDialogData::CalculateIdFromPaperSize()
{
    wxASSERT_MSG( (wxThePrintPaperDatabase != nullptr),
                  wxT("wxThePrintPaperDatabase should not be NULL. Do not create global print dialog data objects.") );

    const wxSize sz = GetPaperSize();

    const wxPaperSize id = wxThePrintPaperDatabase->GetSize(wxSize(sz.x* 10, sz.y * 10));
    if (id != wxPaperSize::None)
    {
        m_printData.SetPaperId(id);
    }
}

// FIXME: Make constexpr
// Use paper id in wxPrintData to set this object's paper size
void wxPageSetupDialogData::CalculatePaperSizeFromId()
{
    wxASSERT_MSG( (wxThePrintPaperDatabase != nullptr),
                  wxT("wxThePrintPaperDatabase should not be NULL. Do not create global print dialog data objects.") );

    const wxSize sz = wxThePrintPaperDatabase->GetSize(m_printData.GetPaperId());

    if (sz != wxSize(0, 0))
    {
        // sz is in 10ths of a mm, while paper size is in mm
        m_paperSize.x = sz.x / 10;
        m_paperSize.y = sz.y / 10;
    }
}

#endif // wxUSE_PRINTING_ARCHITECTURE
