/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/dcbufcmn.cpp
// Purpose:     Buffered DC implementation
// Author:      Ron Lee, Jaakko Salli
// Modified by:
// Created:     Sep-20-2006
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/dcbuffer.h"
#include "wx/module.h"

// ----------------------------------------------------------------------------
// wxSharedDCBufferManager: helper class maintaining backing store bitmap
// ----------------------------------------------------------------------------

class wxSharedDCBufferManager : public wxModule
{
public:
    wxSharedDCBufferManager() = default;

    bool OnInit() override { return true; }
    void OnExit() override { wxDELETE(ms_buffer); }

    static wxBitmap* GetBuffer(wxDC* dc, int w, int h)
    {
        if ( ms_usingSharedBuffer )
            return DoCreateBuffer(dc, w, h);

        if ( !ms_buffer ||
                w > ms_buffer->GetScaledWidth() ||
                    h > ms_buffer->GetScaledHeight() )
        {
            delete ms_buffer;

            ms_buffer = DoCreateBuffer(dc, w, h);
        }

        ms_usingSharedBuffer = true;
        return ms_buffer;
    }

    static void ReleaseBuffer(wxBitmap* buffer)
    {
        if ( buffer == ms_buffer )
        {
            wxASSERT_MSG( ms_usingSharedBuffer, "shared buffer already released" );
            ms_usingSharedBuffer = false;
        }
        else
        {
            delete buffer;
        }
    }

private:
    static wxBitmap* DoCreateBuffer(wxDC* dc, int w, int h)
    {
        const double scale = dc ? dc->GetContentScaleFactor() : 1.0;
        wxBitmap* const buffer = new wxBitmap;

        // we must always return a valid bitmap but creating a bitmap of
        // size 0 would fail, so create a 1*1 bitmap in this case
        buffer->CreateScaled(std::max(w, 1), std::max(h, 1), -1, scale);

        return buffer;
    }

    static wxBitmap *ms_buffer;
    static bool ms_usingSharedBuffer;

    wxDECLARE_DYNAMIC_CLASS(wxSharedDCBufferManager);
};

wxBitmap* wxSharedDCBufferManager::ms_buffer = nullptr;
bool wxSharedDCBufferManager::ms_usingSharedBuffer = false;

wxIMPLEMENT_DYNAMIC_CLASS(wxSharedDCBufferManager, wxModule);

// ============================================================================
// wxBufferedDC
// ============================================================================

void wxBufferedDC::UseBuffer(wxCoord w, wxCoord h)
{
    wxCHECK_RET( w >= -1 && h >= -1, "Invalid buffer size" );

    if ( !m_buffer || !m_buffer->IsOk() )
    {
        if (w == -1 || h == -1)
        {
            // FIXME: Have this function just accept a size instead of two coords.
            w = m_dc->GetSize().x;
            h = m_dc->GetSize().y;
        }

        m_buffer = wxSharedDCBufferManager::GetBuffer(m_dc, w, h);
        m_style |= wxBUFFER_USES_SHARED_BUFFER;
        m_area.Set(w,h);
    }
    else
        m_area = m_buffer->GetSize();

    SelectObject(*m_buffer);

    // now that the DC is valid we can inherit the attributes (fonts, colours,
    // layout direction, ...) from the original DC
    if ( m_dc && m_dc->IsOk() )
        CopyAttributes(*m_dc);
}

void wxBufferedDC::UnMask()
{
    wxCHECK_RET( m_dc, "no underlying wxDC?" );
    wxASSERT_MSG( m_buffer && m_buffer->IsOk(), "invalid backing store" );

    wxPoint DOrigin;

    // Ensure the scale matches the device
    SetUserScale(1.0);

    if ( m_style & wxBUFFER_CLIENT_AREA )
        DOrigin = GetDeviceOrigin();

    // It's possible that the buffer may be bigger than the area that needs to
    // be drawn (the client size of the window is smaller than the bitmap, or
    // a shared bitmap has been reused for a smaller area, etc.) so avoid
    // blitting too much if possible, but only use the real DC size if the
    // wxBUFFER_VIRTUAL_AREA style is not set.
    int width = m_area.x,
        height = m_area.y;

    if (!(m_style & wxBUFFER_VIRTUAL_AREA))
    {
        wxSize dcSize = m_dc->GetSize();
        width = std::min(width, dcSize.x);
        height = std::min(height, dcSize.y);
    }

    const wxPoint origin = GetLogicalOrigin();
    m_dc->Blit(-origin, wxSize{width, height}, this, -DOrigin);
    m_dc = nullptr;

    if ( m_style & wxBUFFER_USES_SHARED_BUFFER )
        wxSharedDCBufferManager::ReleaseBuffer(m_buffer);
}
