/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/dcprint.cpp
// Purpose:     wxPrinterDC class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_PRINTING_ARCHITECTURE

#include "wx/dcprint.h"
#include "wx/msw/dcprint.h"
#include "wx/log.h"
#include "wx/dcmemory.h"

#if wxUSE_WXDIB
    #include "wx/msw/dib.h"
#endif

#include "wx/printdlg.h"
#include "wx/msw/printdlg.h"

#include "wx/msw/private.h"
#include "wx/msw/wrapcdlg.h"

#include <boost/nowide/stackstring.hpp>

import WX.Win.UniqueHnd;

import <string>;

// mingw32 defines GDI_ERROR incorrectly
#if defined(__GNUWIN32__) || !defined(GDI_ERROR)
    #undef GDI_ERROR
    #define GDI_ERROR ((int)-1)
#endif

#if defined(__WXUNIVERSAL__) && wxUSE_POSTSCRIPT_ARCHITECTURE_IN_MSW
    #define wxUSE_PS_PRINTING 1
#else
    #define wxUSE_PS_PRINTING 0
#endif

// ----------------------------------------------------------------------------
// wxWin macros
// ----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxPrinterDCImpl, wxMSWDCImpl);

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxPrinterDC construction
// ----------------------------------------------------------------------------

wxPrinterDCImpl::wxPrinterDCImpl( wxPrinterDC *owner, const wxPrintData& printData ) :
    wxMSWDCImpl( owner )
    , m_printData(printData)
{
    m_hDC = wxGetPrinterDC(printData);
    m_ok = m_hDC != nullptr;
    m_bOwnsDC = true;

    if ( m_hDC )
    {
        //     int width = GetDeviceCaps(m_hDC, VERTRES);
        //     int height = GetDeviceCaps(m_hDC, HORZRES);
        SetMapMode(wxMappingMode::Text);

        SetBrush(*wxBLACK_BRUSH);
        SetPen(*wxBLACK_PEN);
    }
}


wxPrinterDCImpl::wxPrinterDCImpl( wxPrinterDC *owner, WXHDC dc ) :
    wxMSWDCImpl( owner )
{
    m_hDC = dc;
    m_bOwnsDC = true;
    m_ok = true;
}

// ----------------------------------------------------------------------------
// wxPrinterDCImpl {Start/End}{Page/Doc} methods
// ----------------------------------------------------------------------------

bool wxPrinterDCImpl::wxStartDoc(const std::string& message)
{
    DOCINFOW docinfo;
    docinfo.cbSize = sizeof(DOCINFO);
    boost::nowide::wstackstring stackMessage(message.c_str());
    docinfo.lpszDocName = stackMessage.get();

    std::string filename{m_printData.GetFilename()};
    boost::nowide::wstackstring stackFilename(filename.c_str());

    if (filename.empty())
        docinfo.lpszOutput = nullptr;
    else
    {
        docinfo.lpszOutput = stackFilename.get();
    }

    docinfo.lpszDatatype = nullptr;
    docinfo.fwType = 0;

    if (!m_hDC)
        return false;

    if ( ::StartDocW(GetHdc(), &docinfo) <= 0 )
    {
        wxLogLastError("StartDoc");
        return false;
    }

    return true;
}

void wxPrinterDCImpl::EndDoc()
{
    if (m_hDC) ::EndDoc((WXHDC) m_hDC);
}

void wxPrinterDCImpl::StartPage()
{
    if (m_hDC)
        ::StartPage((WXHDC) m_hDC);
}

void wxPrinterDCImpl::EndPage()
{
    if (m_hDC)
        ::EndPage((WXHDC) m_hDC);
}


wxRect wxPrinterDCImpl::GetPaperRect() const
{
    if (!IsOk()) return wxRect(0, 0, 0, 0);
    int w = ::GetDeviceCaps((WXHDC) m_hDC, PHYSICALWIDTH);
    int h = ::GetDeviceCaps((WXHDC) m_hDC, PHYSICALHEIGHT);
    int x = -::GetDeviceCaps((WXHDC) m_hDC, PHYSICALOFFSETX);
    int y = -::GetDeviceCaps((WXHDC) m_hDC, PHYSICALOFFSETY);
    return {x, y, w, h};
}


#if !wxUSE_PS_PRINTING

// Returns default device and port names
static bool wxGetDefaultDeviceName(std::string& deviceName, std::string& portName)
{
    deviceName.clear();

    LPDEVNAMES  lpDevNames;
    LPWSTR      lpszDeviceName;
    LPWSTR      lpszPortName;

    PRINTDLG    pd;
    memset(&pd, 0, sizeof(PRINTDLG));
    pd.lStructSize    = sizeof(PRINTDLG);
    pd.hwndOwner      = (WXHWND)nullptr;
    pd.hDevMode       = nullptr; // Will be created by PrintDlg
    pd.hDevNames      = nullptr; // Ditto
    pd.Flags          = PD_RETURNDEFAULT;
    pd.nCopies        = 1;

    if (!PrintDlg((LPPRINTDLG)&pd))
    {
        if ( pd.hDevMode )
            GlobalFree(pd.hDevMode);
        if (pd.hDevNames)
            GlobalFree(pd.hDevNames);

        return false;
    }

    if (pd.hDevNames)
    {
        {
            GlobalPtrLock ptr(pd.hDevNames);

            lpDevNames = (LPDEVNAMES)ptr.Get();
            lpszDeviceName = (LPWSTR)lpDevNames + lpDevNames->wDeviceOffset;
            lpszPortName   = (LPWSTR)lpDevNames + lpDevNames->wOutputOffset;

            deviceName = boost::nowide::narrow(lpszDeviceName);
            portName = boost::nowide::narrow(lpszPortName);
        } // unlock pd.hDevNames

        GlobalFree(pd.hDevNames);
        pd.hDevNames=nullptr;
    }

    if (pd.hDevMode)
    {
        GlobalFree(pd.hDevMode);
        pd.hDevMode=nullptr;
    }
    return ( !deviceName.empty() );
}

#endif // !wxUSE_PS_PRINTING

// Gets an WXHDC for the specified printer configuration
WXHDC wxGetPrinterDC(const wxPrintData& printDataConst)
{
#if wxUSE_PS_PRINTING
    // TODO
    wxUnusedVar(printDataConst);
    return 0;
#else // native Windows printing
    wxWindowsPrintNativeData *data =
        (wxWindowsPrintNativeData *) printDataConst.GetNativeData();

    data->TransferFrom( printDataConst );

    std::string deviceName = printDataConst.GetPrinterName();
    if ( deviceName.empty() )
    {
        // Retrieve the default device name
        std::string portName;
        if ( !wxGetDefaultDeviceName(deviceName, portName) )
        {
            return nullptr; // Could not get default device name
        }
    }


    GlobalPtrLock lockDevMode;
    const HGLOBAL devMode = data->GetDevMode();
    if ( devMode )
        lockDevMode.Init(devMode);

    WXHDC hDC = ::CreateDCW
                (
                    nullptr,               // no driver name as we use device name
                    boost::nowide::widen(deviceName).c_str(),
                    nullptr,               // unused
                    static_cast<DEVMODEW *>(lockDevMode.Get())
                );
    if ( !hDC )
    {
        wxLogLastError("CreateDC(printer)");
    }

    return (WXHDC) hDC;
#endif // PostScript/Windows printing
}

// ----------------------------------------------------------------------------
// wxPrinterDCImpl bit blitting/bitmap drawing
// ----------------------------------------------------------------------------

// helper of DoDrawBitmap() and DoBlit()
static
bool DrawBitmapUsingStretchDIBits(WXHDC hdc,
                                  const wxBitmap& bmp,
                                  wxCoord x, wxCoord y)
{
#if wxUSE_WXDIB
    wxDIB dib(bmp);
    bool ok = dib.IsOk();
    if ( !ok )
        return false;

    DIBSECTION ds;
    if ( !::GetObjectW(dib.GetHandle(), sizeof(ds), &ds) )
    {
        wxLogLastError("GetObject(DIBSECTION)");

        return false;
    }

    // ok, we've got all data we need, do blit it
    if ( ::StretchDIBits
            (
                hdc,
                x, y,
                ds.dsBmih.biWidth, ds.dsBmih.biHeight,
                0, 0,
                ds.dsBmih.biWidth, ds.dsBmih.biHeight,
                ds.dsBm.bmBits,
                (LPBITMAPINFO)&ds.dsBmih,
                DIB_RGB_COLORS,
                SRCCOPY
            ) == GDI_ERROR )
    {
        wxLogLastError("StretchDIBits");

        return false;
    }

    return true;
#else
    return false;
#endif
}

void wxPrinterDCImpl::DoDrawBitmap(const wxBitmap& bmp,
                               wxCoord x, wxCoord y,
                               bool useMask)
{
    wxCHECK_RET( bmp.IsOk(), "invalid bitmap in wxPrinterDC::DrawBitmap" );

    int width = bmp.GetWidth(),
        height = bmp.GetHeight();

    if ( !(::GetDeviceCaps(GetHdc(), RASTERCAPS) & RC_STRETCHDIB) ||
            !DrawBitmapUsingStretchDIBits(GetHdc(), bmp, x, y) )
    {
        // no support for StretchDIBits() or an error occurred if we got here
        wxMemoryDC memDC;

        memDC.SelectObjectAsSource(bmp);

        GetOwner()->Blit(wxPoint{x, y}, wxSize{width, height}, &memDC, wxPoint{0, 0}, wxRasterOperationMode::Copy, useMask);

        memDC.SelectObject(wxNullBitmap);
    }
}

bool wxPrinterDCImpl::DoBlit(wxCoord xdest, wxCoord ydest,
                         wxCoord width, wxCoord height,
                         wxDC *source,
                         [[maybe_unused]] wxPoint src,
                         [[maybe_unused]] wxRasterOperationMode rop, bool useMask,
                         [[maybe_unused]] wxPoint srcMask)
{
    wxDCImpl *impl = source->GetImpl();
    wxMSWDCImpl *msw_impl = dynamic_cast<wxMSWDCImpl*>(impl);
    if (!msw_impl)
        return false;

    wxBitmap& bmp = msw_impl->GetSelectedBitmap();
    wxMask *mask = useMask ? bmp.GetMask() : nullptr;

    using msw::utils::unique_brush;

    if ( mask )
    {
        // If we are printing source colours are screen colours not printer
        // colours and so we need copy the bitmap pixel by pixel.
        RECT rect;
        WXHDC dcSrc = GetHdcOf(*msw_impl);
        MemoryHDC dcMask(dcSrc);
        SelectInHDC selectMask(dcMask, (WXHBITMAP)mask->GetMaskBitmap());

        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                COLORREF cref = ::GetPixel(dcMask, x, y);
                if (cref)
                {
                    unique_brush brush{::CreateSolidBrush(::GetPixel(dcSrc, x, y))};
                    rect.left = xdest + x;
                    rect.right = rect.left + 1;
                    rect.top = ydest + y;
                    rect.bottom = rect.top + 1;
                    ::FillRect(GetHdc(), &rect, brush.get());
                }
            }
        }
    }
    else // no mask
    {
        if ( !(::GetDeviceCaps(GetHdc(), RASTERCAPS) & RC_STRETCHDIB) ||
                !DrawBitmapUsingStretchDIBits(GetHdc(), bmp, xdest, ydest) )
        {
            // no support for StretchDIBits

            // as we are printing, source colours are screen colours not
            // printer colours and so we need copy the bitmap pixel by pixel.
            WXHDC dcSrc = GetHdcOf(*msw_impl);
            RECT rect;
            for (int y = 0; y < height; y++)
            {
                // optimization: draw identical adjacent pixels together.
                for (int x = 0; x < width; x++)
                {
                    COLORREF col = ::GetPixel(dcSrc, x, y);
                    unique_brush brush{::CreateSolidBrush( col )};

                    rect.left = xdest + x;
                    rect.top = ydest + y;
                    while( (x + 1 < width) &&
                                (::GetPixel(dcSrc, x + 1, y) == col ) )
                    {
                        ++x;
                    }

                    rect.right = xdest + x + 1;
                    rect.bottom = rect.top + 1;
                    ::FillRect((WXHDC) m_hDC, &rect, brush.get());
                }
            }
        }
    }

    return true;
}

#endif
    // wxUSE_PRINTING_ARCHITECTURE
