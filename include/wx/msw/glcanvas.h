/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/glcanvas.h
// Purpose:     wxGLCanvas, for using OpenGL with wxWidgets under Windows
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GLCANVAS_H_
#define _WX_GLCANVAS_H_

#include <GL/gl.h>

import WX.Win.UniqueHnd;
import <string>;

class wxPalette;

// ----------------------------------------------------------------------------
// wxGLContext: OpenGL rendering context
// ----------------------------------------------------------------------------

using msw::utils::unique_glrc;

class wxGLContext : public wxGLContextBase
{
public:
    wxGLContext(wxGLCanvas *win,
                const wxGLContext *other = nullptr,
                const wxGLContextAttrs *ctxAttrs = nullptr);

    bool SetCurrent(const wxGLCanvas& win) const override;

    HGLRC GetGLRC() const { return m_glContext.get(); }

protected:
    unique_glrc m_glContext;

private:
    wxDECLARE_CLASS(wxGLContext);
};

// ----------------------------------------------------------------------------
// wxGLCanvas: OpenGL output window
// ----------------------------------------------------------------------------

class wxGLCanvas : public wxGLCanvasBase
{
public:
    explicit // avoid implicitly converting a wxWindow* to wxGLCanvas
    wxGLCanvas(wxWindow *parent,
               const wxGLAttributes& dispAttrs,
               wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = 0,
               std::string_view name = wxGLCanvasName,
               const wxPalette& palette = wxNullPalette);

    explicit
    wxGLCanvas(wxWindow *parent,
               wxWindowID id = wxID_ANY,
               const int *attribList = nullptr,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = 0,
               std::string_view name = wxGLCanvasName,
               const wxPalette& palette = wxNullPalette);

    [[maybe_unused]] bool Create(wxWindow *parent,
                const wxGLAttributes& dispAttrs,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                std::string_view name = wxGLCanvasName,
                const wxPalette& palette = wxNullPalette);

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                std::string_view name = wxGLCanvasName,
                const int *attribList = nullptr,
                const wxPalette& palette = wxNullPalette);

    ~wxGLCanvas();

    // implement wxGLCanvasBase methods
    bool SwapBuffers() override;


    // MSW-specific helpers
    // --------------------

    // get the WXHDC used for OpenGL rendering
    WXHDC GetHDC() const { return m_hDC; }

    // Try to find pixel format matching the given attributes list for the
    // specified WXHDC, return 0 on error, otherwise ppfd is filled in with the
    // information from dispAttrs
    static int FindMatchingPixelFormat(const wxGLAttributes& dispAttrs,
                                       PIXELFORMATDESCRIPTOR* ppfd = nullptr);
    // Same as FindMatchingPixelFormat
    static int ChooseMatchingPixelFormat(WXHDC hdc, const int *attribList,
                                         PIXELFORMATDESCRIPTOR *pfd = nullptr);

#if wxUSE_PALETTE
    // palette stuff
    bool SetupPalette(const wxPalette& palette);
    wxPalette CreateDefaultPalette() override;
    void OnQueryNewPalette(wxQueryNewPaletteEvent& event);
    void OnPaletteChanged(wxPaletteChangedEvent& event);
#endif // wxUSE_PALETTE

protected:
    // the real window creation function, Create() may reuse it twice as we may
    // need to create an OpenGL window to query the available extensions and
    // then potentially delete and recreate it with another pixel format
    bool wxCreateWindow(wxWindow *parent,
                      wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      unsigned int style = 0,
                      const std::string& name = wxGLCanvasName);

    // set up the pixel format using the given attributes and palette
    int DoSetup(PIXELFORMATDESCRIPTOR &pfd, const int *attribList);


    // WXHDC for this window, we keep it all the time
    WXHDC m_hDC {nullptr};

private:
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_CLASS(wxGLCanvas);
};

#endif // _WX_GLCANVAS_H_
