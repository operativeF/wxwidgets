/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/cursor.cpp
// Purpose:     wxCursor class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) 1997-2003 Julian Smart and Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/cursor.h"
#include "wx/utils.h"
#include "wx/app.h"
#include "wx/bitmap.h"
#include "wx/icon.h"
#include "wx/settings.h"
#include "wx/intl.h"
#include "wx/module.h"
#include "wx/display.h"

#include "wx/msw/private.h"

#include <boost/nowide/convert.hpp>

#include <cassert>

import WX.Image;
import WX.WinDef;

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

class wxCursorRefData : public wxGDIImageRefData
{
public:
    // the second parameter is used to tell us to delete the cursor when we're
    // done with it (normally we shouldn't call DestroyCursor() this is why it
    // doesn't happen by default)
    explicit wxCursorRefData(WXHCURSOR hcursor = nullptr, bool takeOwnership = false);

    ~wxCursorRefData() { Free(); }

    void Free() override;


    // return the size of the standard cursor: notice that the system only
    // supports the cursors of this size
    static wxCoord GetStandardWidth();
    static wxCoord GetStandardHeight();

private:
    bool m_destroyCursor;
};

// ----------------------------------------------------------------------------
// wxWin macros
// ----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxCursor, wxGDIObject);

// ----------------------------------------------------------------------------
// Global cursor setting
// ----------------------------------------------------------------------------

namespace
{

// Current cursor, in order to hang on to cursor handle when setting the cursor
// globally
wxCursor* gs_globalCursor = nullptr;

class wxCursorModule : public wxModule
{
public:
    bool OnInit() override
    {
        gs_globalCursor = new wxCursor;

        return true;
    }

    void OnExit() override
    {
        wxDELETE(gs_globalCursor);
    }
};

} // namespace anonymous

void wxSetCursor(const wxCursor& cursor)
{
    if ( cursor.IsOk() )
    {
        ::SetCursor(GetHcursorOf(cursor));

        if ( gs_globalCursor )
            *gs_globalCursor = cursor;
    }
}

const wxCursor *wxGetGlobalCursor()
{
    return gs_globalCursor;
}

wxCoord wxCursorRefData::GetStandardWidth()
{
    const wxWindow* win = wxApp::GetMainTopWindow();
    return wxSystemSettings::GetMetric(wxSYS_CURSOR_X, win);
}

wxCoord wxCursorRefData::GetStandardHeight()
{
    const wxWindow* win = wxApp::GetMainTopWindow();
    return wxSystemSettings::GetMetric(wxSYS_CURSOR_Y, win);
}

wxCursorRefData::wxCursorRefData(WXHCURSOR hcursor, bool destroy)
{
    m_hCursor = (WXHCURSOR)hcursor;

    if ( m_hCursor )
    {
        m_size = {GetStandardWidth(), GetStandardHeight()};
    }

    m_destroyCursor = destroy;
}

void wxCursorRefData::Free()
{
    if ( m_hCursor )
    {
        if ( m_destroyCursor )
            ::DestroyCursor((WXHCURSOR)m_hCursor);

        m_hCursor = nullptr;
    }
}

// ----------------------------------------------------------------------------
// Cursors
// ----------------------------------------------------------------------------

#if wxUSE_IMAGE
wxCursor::wxCursor(const wxImage& image)
{
    InitFromImage(image);
}

wxCursor::wxCursor(const char* const* xpmData)
{
    InitFromImage(wxImage(xpmData));
}

void wxCursor::InitFromImage(const wxImage& image)
{
    // image has to be of the standard cursor size, otherwise we won't be able
    // to create it
    const int w = wxCursorRefData::GetStandardWidth();
    const int h = wxCursorRefData::GetStandardHeight();

    int hotSpotX = image.GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_X);
    int hotSpotY = image.GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_Y);
    int image_w = image.GetWidth();
    int image_h = image.GetHeight();

    wxASSERT_MSG( hotSpotX >= 0 && hotSpotX < image_w &&
                  hotSpotY >= 0 && hotSpotY < image_h,
                  "invalid cursor hot spot coordinates" );

    wxImage imageSized(image); // final image of correct size

    // if image is too small then place it in the center, resize it if too big
    if ((w > image_w) && (h > image_h))
    {
        wxPoint offset((w - image_w)/2, (h - image_h)/2);
        hotSpotX = hotSpotX + offset.x;
        hotSpotY = hotSpotY + offset.y;

        imageSized = image.Size(wxSize(w, h), offset);
    }
    else if ((w != image_w) || (h != image_h))
    {
        hotSpotX = int(hotSpotX * double(w) / double(image_w));
        hotSpotY = int(hotSpotY * double(h) / double(image_h));

        imageSized = image.Scale(w, h);
    }

    WXHCURSOR hcursor = wxBitmapToHCURSOR( wxBitmap(imageSized),
                                         hotSpotX, hotSpotY );

    if ( !hcursor )
    {
        wxLogWarning(_("Failed to create cursor."));
        return;
    }

    m_refData = new wxCursorRefData(hcursor, true /* delete it later */);
}
#endif // wxUSE_IMAGE

wxCursor::wxCursor(const std::string& filename,
                   wxBitmapType kind,
                   int hotSpotX,
                   int hotSpotY)
{
    WXHCURSOR hcursor = [=]() {
        switch ( kind )
        {
            case wxBitmapType::CUR_Resource:
                return ::LoadCursorW(wxGetInstance(), boost::nowide::widen(filename).c_str());

            case wxBitmapType::ANI:
            case wxBitmapType::CUR:
                return ::LoadCursorFromFileW(boost::nowide::widen(filename).c_str());

            case wxBitmapType::ICO:
                return wxBitmapToHCURSOR
                        (
                        wxIcon(filename, wxBitmapType::ICO),
                        hotSpotX,
                        hotSpotY
                        );

            case wxBitmapType::BMP:
                return wxBitmapToHCURSOR
                        (
                        wxBitmap(filename, wxBitmapType::BMP),
                        hotSpotX,
                        hotSpotY
                        );

            default:
                wxLogError( "unknown cursor resource type '%d'", kind );

                return static_cast<WXHCURSOR>(nullptr);
        }
    }();

    if ( hcursor )
    {
        m_refData = new wxCursorRefData(hcursor, true /* delete it later */);
    }
}

wxPoint wxCursor::GetHotSpot() const
{
    if ( !GetGDIImageData() )
        return wxDefaultPosition;

    AutoIconInfo ii;
    if ( !ii.GetFrom((WXHICON)GetGDIImageData()->m_hCursor) )
        return wxDefaultPosition;

    return {gsl::narrow_cast<int>(ii.xHotspot), gsl::narrow_cast<int>(ii.yHotspot)};
}

namespace
{

wxSize ScaleAndReverseBitmap(WXHBITMAP& bitmap, float scale)
{
    BITMAP bmp;
    if ( !::GetObjectW(bitmap, sizeof(bmp), &bmp) )
        return {};
    wxSize cs(gsl::narrow_cast<int>(bmp.bmWidth * scale), gsl::narrow_cast<int>(bmp.bmHeight * scale));

    MemoryHDC hdc;
    SelectInHDC selBitmap(hdc, bitmap);
    if ( scale != 1 )
        ::SetStretchBltMode(hdc, HALFTONE);
    ::StretchBlt(hdc, cs.x - 1, 0, -cs.x, cs.y, hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

    return cs;
}

WXHCURSOR CreateReverseCursor(WXHCURSOR cursor)
{
    AutoIconInfo info;
    if ( !info.GetFrom(cursor) )
        return nullptr;

    const unsigned displayID = (unsigned)wxDisplay::GetFromPoint(wxGetMousePosition());
    wxDisplay disp(displayID == 0u || displayID < wxDisplay::GetCount() ? displayID : 0u);
    const float scale = (float)disp.GetPPI().y / wxGetDisplayPPI().y;

    wxSize cursorSize = ScaleAndReverseBitmap(info.hbmMask, scale);
    if ( info.hbmColor )
        ScaleAndReverseBitmap(info.hbmColor, scale);
    info.xHotspot = (DWORD)(cursorSize.x - 1 - info.xHotspot * scale);
    info.yHotspot = (DWORD)(info.yHotspot * scale);

    return ::CreateIconIndirect(&info);
}

} // anonymous namespace

// Cursors by stock number
void wxCursor::InitFromStock(wxStockCursor idCursor)
{
    // all wxWidgets standard cursors
    static const struct StdCursor
    {
        // is this a standard Windows cursor?
        bool isStd;

        // the cursor name or id
        LPCWSTR name;
    } stdCursors[] =
    {
        {  true, nullptr                        }, // wxCURSOR_NONE
        {  true, IDC_ARROW                   }, // wxCURSOR_ARROW
        { false, L"WXCURSOR_RIGHT_ARROW"  }, // wxCURSOR_RIGHT_ARROW
        { false, L"WXCURSOR_BULLSEYE"     }, // wxCURSOR_BULLSEYE
        {  true, IDC_ARROW                   }, // WXCURSOR_CHAR
        {  true, IDC_CROSS                   }, // WXCURSOR_CROSS
        {  true, IDC_HAND                    }, // wxCURSOR_HAND
        {  true, IDC_IBEAM                   }, // WXCURSOR_IBEAM
        {  true, IDC_ARROW                   }, // WXCURSOR_LEFT_BUTTON
        { false, L"WXCURSOR_MAGNIFIER"    }, // wxCURSOR_MAGNIFIER
        {  true, IDC_ARROW                   }, // WXCURSOR_MIDDLE_BUTTON
        {  true, IDC_NO                      }, // WXCURSOR_NO_ENTRY
        { false, L"WXCURSOR_PBRUSH"       }, // wxCURSOR_PAINT_BRUSH
        { false, L"WXCURSOR_PENCIL"       }, // wxCURSOR_PENCIL
        { false, L"WXCURSOR_PLEFT"        }, // wxCURSOR_POINT_LEFT
        { false, L"WXCURSOR_PRIGHT"       }, // wxCURSOR_POINT_RIGHT
        {  true, IDC_HELP                    }, // WXCURSOR_QUESTION_ARROW
        {  true, IDC_ARROW                   }, // WXCURSOR_RIGHT_BUTTON
        {  true, IDC_SIZENESW                }, // WXCURSOR_SIZENESW
        {  true, IDC_SIZENS                  }, // WXCURSOR_SIZENS
        {  true, IDC_SIZENWSE                }, // WXCURSOR_SIZENWSE
        {  true, IDC_SIZEWE                  }, // WXCURSOR_SIZEWE
        {  true, IDC_SIZEALL                 }, // WXCURSOR_SIZING
        { false, L"WXCURSOR_PBRUSH"       }, // wxCURSOR_SPRAYCAN
        {  true, IDC_WAIT                    }, // WXCURSOR_WAIT
        {  true, IDC_WAIT                    }, // WXCURSOR_WATCH
        { false, L"WXCURSOR_BLANK"        }, // wxCURSOR_BLANK
        {  true, IDC_APPSTARTING             }, // wxCURSOR_ARROWWAIT

        // no entry for wxCURSOR_MAX
    };

    static_assert(WXSIZEOF(stdCursors) == wxCURSOR_MAX);

    wxCHECK_RET( idCursor > 0 && (size_t)idCursor < WXSIZEOF(stdCursors),
                 "invalid cursor id in wxCursor() ctor" );

    const StdCursor& stdCursor = stdCursors[idCursor];
    bool deleteLater = !stdCursor.isStd;

    WXHCURSOR hcursor = ::LoadCursorW(stdCursor.isStd ? nullptr : wxGetInstance(),
                                   stdCursor.name);

    // IDC_HAND may not be available on some versions of Windows.
    if ( !hcursor && idCursor == wxCURSOR_HAND)
    {
        hcursor = ::LoadCursorW(wxGetInstance(), L"WXCURSOR_HAND");
        deleteLater = true;
    }

    if ( !hcursor && idCursor == wxCURSOR_RIGHT_ARROW)
    {
        hcursor = ::LoadCursorW(nullptr, IDC_ARROW);
        if ( hcursor )
        {
            hcursor = CreateReverseCursor(hcursor);
            deleteLater = true;
        }
    }

    if ( !hcursor )
    {
        if ( !stdCursor.isStd )
        {
            // it may be not obvious to the programmer why did loading fail,
            // try to help by pointing to the by far the most probable reason
            wxFAIL_MSG("Loading a cursor defined by wxWidgets failed, "
                       "did you include include/wx/msw/wx.rc file from "
                       "your resource file?");
        }

        wxLogLastError("LoadCursor");
    }
    else
    {
        m_refData = new wxCursorRefData(hcursor, deleteLater);
    }
}

// ----------------------------------------------------------------------------
// other wxCursor functions
// ----------------------------------------------------------------------------

wxGDIImageRefData *wxCursor::CreateData() const
{
    return new wxCursorRefData;
}
