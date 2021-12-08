/////////////////////////////////////////////////////////////////////////////
// Name:        src/html/m_image.cpp
// Purpose:     wxHtml module for displaying images
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik, Joel Lucsy
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_HTML && wxUSE_STREAMS

#include "wx/app.h"
#include "wx/dc.h"
#include "wx/dcprint.h"
#include "wx/scrolwin.h"
#include "wx/timer.h"
#include "wx/dcmemory.h"
#include "wx/log.h"
#include "wx/wxcrtvararg.h"

#include "wx/html/forcelnk.h"
#include "wx/html/m_templ.h"
#include "wx/html/htmlwin.h"

#include "wx/gifdecod.h"
#include "wx/artprov.h"

import WX.Image;

import <cmath>;
import <vector>;

FORCE_LINK_ME(m_image)

WX_DECLARE_OBJARRAY(int, CoordArray);
WX_DEFINE_OBJARRAY(CoordArray)

// ---------------------------------------------------------------------------
// wxHtmlImageMapAreaCell
//                  0-width, 0-height cell that represents single area in
//                  imagemap (it's GetLink is called from wxHtmlImageCell's)
// ---------------------------------------------------------------------------

class wxHtmlImageMapAreaCell : public wxHtmlCell
{
    public:
        enum celltype { CIRCLE, RECT, POLY };
    protected:
        std::vector<int> coords;
        celltype type;
        int radius;
    public:
        wxHtmlImageMapAreaCell( celltype t, wxString &coords, double pixel_scale = 1.0);
        wxHtmlLinkInfo *GetLink( int x = 0, int y = 0 ) const override;
        void Draw([[maybe_unused]] wxDC& dc,
                  [[maybe_unused]] int x, [[maybe_unused]] int y,
                  [[maybe_unused]] int view_y1, [[maybe_unused]] int view_y2,
                  [[maybe_unused]] wxHtmlRenderingInfo& info) override {}


    wxHtmlImageMapAreaCell(const wxHtmlImageMapAreaCell&) = delete;
	wxHtmlImageMapAreaCell& operator=(const wxHtmlImageMapAreaCell&) = delete;
};

wxHtmlImageMapAreaCell::wxHtmlImageMapAreaCell( wxHtmlImageMapAreaCell::celltype t, wxString &incoords, double pixel_scale )
    : type(t)
{
    int i;
    wxString x = incoords, y;

    while ((i = x.Find( ',' )) != wxNOT_FOUND)
    {
        coords.push_back( (int)(pixel_scale * (double)wxAtoi( x.Left( i ).c_str())) );
        x = x.Mid( i + 1 );
    }
    coords.push_back( (int)(pixel_scale * (double)wxAtoi( x.c_str())) );
}

wxHtmlLinkInfo *wxHtmlImageMapAreaCell::GetLink( int x, int y ) const
{
    switch (type)
    {
        case RECT:
            if ( coords.size() == 4 )
            {
                int l = coords[ 0 ];
                int t = coords[ 1 ];
                int r = coords[ 2 ];
                int b = coords[ 3 ];
                if (x >= l && x <= r && y >= t && y <= b)
                {
                    return m_Link;
                }
            }
            break;
        case CIRCLE:
            if ( coords.size() == 3 )
            {
                int l = coords[ 0 ];
                int t = coords[ 1 ];
                int r = coords[ 2 ];
                double d = std::hypot(x - l, y - t);
                if (d < (double)r)
                {
                    return m_Link;
                }
            }
            break;
        case POLY:
             if (coords.size() >= 6)
             {
                 int intersects = 0;
                 int wherex = x;
                 int wherey = y;
                 int totalv = coords.size() / 2;
                 int totalc = totalv * 2;
                 int xval = coords[totalc - 2];
                 int yval = coords[totalc - 1];
                 int end = totalc;
                 int pointer = 1;

                 if ((yval >= wherey) != (coords[pointer] >= wherey))
                 {
                     if ((xval >= wherex) == (coords[0] >= wherex))
                     {
                         intersects += (xval >= wherex) ? 1 : 0;
                     }
                     else
                     {
                         intersects += ((xval - (yval - wherey) *
                                         (coords[0] - xval) /
                                         (coords[pointer] - yval)) >= wherex) ? 1 : 0;
                     }
                 }

                 while (pointer < end)
                 {
                     yval = coords[pointer];
                     pointer += 2;
                     if (yval >= wherey)
                     {
                         while ((pointer < end) && (coords[pointer] >= wherey))
                         {
                             pointer += 2;
                         }
                         if (pointer >= end)
                         {
                             break;
                         }
                         if ((coords[pointer - 3] >= wherex) ==
                                 (coords[pointer - 1] >= wherex)) {
                             intersects += (coords[pointer - 3] >= wherex) ? 1 : 0;
                         }
                         else
                         {
                             intersects +=
                                 ((coords[pointer - 3] - (coords[pointer - 2] - wherey) *
                                   (coords[pointer - 1] - coords[pointer - 3]) /
                                   (coords[pointer] - coords[pointer - 2])) >= wherex) ? 1 : 0;
                         }
                     }
                     else
                     {
                         while ((pointer < end) && (coords[pointer] < wherey))
                         {
                             pointer += 2;
                         }
                         if (pointer >= end)
                         {
                             break;
                         }
                         if ((coords[pointer - 3] >= wherex) ==
                                 (coords[pointer - 1] >= wherex))
                         {
                             intersects += (coords[pointer - 3] >= wherex) ? 1 : 0;
                         }
                         else
                         {
                             intersects +=
                                 ((coords[pointer - 3] - (coords[pointer - 2] - wherey) *
                                   (coords[pointer - 1] - coords[pointer - 3]) /
                                   (coords[pointer] - coords[pointer - 2])) >= wherex) ? 1 : 0;
                         }
                     }
                 }
                 if ((intersects & 1) != 0)
                 {
                     return m_Link;
                 }
            }
            break;
    }

    if (m_Next)
    {
        wxHtmlImageMapAreaCell  *a = (wxHtmlImageMapAreaCell*)m_Next;
        return a->GetLink( x, y );
    }
    return nullptr;
}








//--------------------------------------------------------------------------------
// wxHtmlImageMapCell
//                  0-width, 0-height cell that represents map from imagemaps
//                  it is always placed before wxHtmlImageMapAreaCells
//                  It responds to Find(wxHTML_COND_ISIMAGEMAP)
//--------------------------------------------------------------------------------


class wxHtmlImageMapCell : public wxHtmlCell
{
    public:
        explicit wxHtmlImageMapCell( wxString &name );
    protected:
        wxString m_Name;
    public:
        wxHtmlLinkInfo *GetLink( int x = 0, int y = 0 ) const override;
        const wxHtmlCell *Find( int cond, const void *param ) const override;
        void Draw([[maybe_unused]] wxDC& dc,
                  [[maybe_unused]] int x, [[maybe_unused]] int y,
                  [[maybe_unused]] int view_y1, [[maybe_unused]] int view_y2,
                  [[maybe_unused]] wxHtmlRenderingInfo& info) override {}

    wxHtmlImageMapCell(const wxHtmlImageMapCell&) = delete;
	wxHtmlImageMapCell& operator=(const wxHtmlImageMapCell&) = delete;
};


wxHtmlImageMapCell::wxHtmlImageMapCell( wxString &name )
    : m_Name(name)
{
}

wxHtmlLinkInfo *wxHtmlImageMapCell::GetLink( int x, int y ) const
{
    wxHtmlImageMapAreaCell  *a = (wxHtmlImageMapAreaCell*)m_Next;
    if (a)
        return a->GetLink( x, y );
    return wxHtmlCell::GetLink( x, y );
}

const wxHtmlCell *wxHtmlImageMapCell::Find( int cond, const void *param ) const
{
    if (cond == wxHTML_COND_ISIMAGEMAP)
    {
        if (m_Name == *static_cast<const wxString*>(param))
            return this;
    }
    return wxHtmlCell::Find(cond, param);
}





//--------------------------------------------------------------------------------
// wxHtmlImageCell
//                  Image/bitmap
//--------------------------------------------------------------------------------

class wxHtmlImageCell : public wxHtmlCell
{
public:
    wxHtmlImageCell(wxHtmlWindowInterface *windowIface,
                    wxFSFile *input, double scaleHDPI = 1.0,
                    int w = wxDefaultCoord, bool wpercent = false,
                    int h = wxDefaultCoord, bool hpresent = false,
                    double scale = 1.0, int align = wxHTML_ALIGN_BOTTOM,
                    const wxString& mapname = {});
    ~wxHtmlImageCell();

    wxHtmlImageCell(const wxHtmlImageCell&) = delete;
	wxHtmlImageCell& operator=(const wxHtmlImageCell&) = delete;

    void Draw(wxDC& dc, int x, int y, int view_y1, int view_y2,
              wxHtmlRenderingInfo& info) override;
    wxHtmlLinkInfo *GetLink(int x = 0, int y = 0) const override;

    void SetImage(const wxImage& img, double scaleHDPI = 1.0);

    // If "alt" text is set, it will be used when converting this cell to text.
    void SetAlt(const wxString& alt);
    std::string ConvertToText(wxHtmlSelection *sel) const override;

#if wxUSE_GIF && wxUSE_TIMER
    void AdvanceAnimation(wxTimer *timer);
#endif

    void Layout(int w) override;

    wxString GetDescription() const override
    {
        return wxString::Format("wxHtmlImageCell with bitmap of size %d*%d",
                                m_bmpW, m_bmpH);
    }

private:
    wxBitmap           *m_bitmap;
    int                 m_align;
    int                 m_bmpW, m_bmpH;
    bool                m_bmpWpercent:1;
    bool                m_bmpHpresent:1;
    bool                m_showFrame:1;
    wxHtmlWindowInterface *m_windowIface;
#if wxUSE_GIF && wxUSE_TIMER
    wxGIFDecoder       *m_gifDecoder;
    wxTimer            *m_gifTimer;
    int                 m_physX, m_physY;
    size_t              m_nCurrFrame;
#endif
    double              m_scale;
    mutable const wxHtmlImageMapCell* m_imageMap;
    mutable wxString    m_mapName;
    wxString            m_alt;
};

#if wxUSE_GIF && wxUSE_TIMER
class wxGIFTimer : public wxTimer
{
    public:
        explicit wxGIFTimer(wxHtmlImageCell *cell) : m_cell(cell) {}
        void Notify() override
        {
            m_cell->AdvanceAnimation(this);
        }

        wxGIFTimer(const wxGIFTimer&) = delete;
        wxGIFTimer& operator=(const wxGIFTimer&) = delete;

    private:
        wxHtmlImageCell *m_cell;
};
#endif


//----------------------------------------------------------------------------
// wxHtmlImageCell
//----------------------------------------------------------------------------


wxHtmlImageCell::wxHtmlImageCell(wxHtmlWindowInterface *windowIface,
                                 wxFSFile *input, double scaleHDPI,
                                 int w, bool wpercent, int h, bool hpresent, double scale, int align,
                                 const wxString& mapname) : 
     m_mapName(mapname)
{
    m_windowIface = windowIface;
    m_scale = scale;
    m_showFrame = false;
    m_bitmap = nullptr;
    m_bmpW   = w;
    m_bmpH   = h;
    m_align  = align;
    m_bmpWpercent = wpercent;
    m_bmpHpresent = hpresent;
    m_imageMap = nullptr;
    SetCanLiveOnPagebreak(false);
#if wxUSE_GIF && wxUSE_TIMER
    m_gifDecoder = nullptr;
    m_gifTimer = nullptr;
    m_physX = m_physY = wxDefaultCoord;
    m_nCurrFrame = 0;
#endif

    if ( m_bmpW && m_bmpH )
    {
        if ( input )
        {
            wxInputStream *s = input->GetStream();

            if ( s )
            {
#if wxUSE_GIF && wxUSE_TIMER
                bool readImg = true;
                if ( m_windowIface &&
                     (input->GetLocation().Matches("*.gif") ||
                      input->GetLocation().Matches("*.GIF")) )
                {
                    m_gifDecoder = new wxGIFDecoder();
                    if ( m_gifDecoder->LoadGIF(*s) == wxGIFErrorCode::OK )
                    {
                        wxImage img;
                        if ( m_gifDecoder->ConvertToImage(0, &img) )
                            SetImage(img);

                        readImg = false;

                        if ( m_gifDecoder->IsAnimation() )
                        {
                            m_gifTimer = new wxGIFTimer(this);
                            auto delay = m_gifDecoder->GetDelay(0);
                            if ( delay == 0ms )
                                delay = 1ms;
                            m_gifTimer->Start(delay, true);
                        }
                        else
                        {
                            wxDELETE(m_gifDecoder);
                        }
                    }
                    else
                    {
                        wxDELETE(m_gifDecoder);
                    }
                }

                if ( readImg )
#endif // wxUSE_GIF && wxUSE_TIMER
                {
                    wxImage image(*s, wxBitmapType::Any);
                    if ( image.IsOk() )
                        SetImage(image, scaleHDPI);
                }
            }
        }
        else // input==NULL, use "broken image" bitmap
        {
            if ( m_bmpW == wxDefaultCoord && m_bmpH == wxDefaultCoord )
            {
                m_bmpW = 29;
                m_bmpH = 31;
            }
            else
            {
                m_showFrame = true;
                if ( m_bmpW == wxDefaultCoord ) m_bmpW = 31;
                if ( m_bmpH == wxDefaultCoord ) m_bmpH = 33;
            }
            m_bitmap =
                new wxBitmap(wxArtProvider::GetBitmap(wxART_MISSING_IMAGE));
        }
    }
    //else: ignore the 0-sized images used sometimes on the Web pages

 }

void wxHtmlImageCell::SetImage(const wxImage& img, double scaleHDPI)
{
#if !defined(__WXMSW__) || wxUSE_WXDIB
    if ( img.IsOk() )
    {
        delete m_bitmap;

        const int ww = img.GetWidth();
        const int hh = img.GetHeight();

        if ( m_bmpW == wxDefaultCoord)
            m_bmpW = std::lround(ww / scaleHDPI);
        if ( m_bmpH == wxDefaultCoord)
            m_bmpH = std::lround(hh / scaleHDPI);

        // On a Mac retina screen, we might have found a @2x version of the image,
        // so specify this scale factor.
        m_bitmap = new wxBitmap(img, -1, scaleHDPI);
    }
#endif
}

void wxHtmlImageCell::SetAlt(const wxString& alt)
{
    m_alt = alt;
}

std::string wxHtmlImageCell::ConvertToText([[maybe_unused]] wxHtmlSelection* sel) const
{
    return m_alt;
}

#if wxUSE_GIF && wxUSE_TIMER
void wxHtmlImageCell::AdvanceAnimation(wxTimer *timer)
{
    wxImage img;

    // advance current frame
    m_nCurrFrame++;
    if (m_nCurrFrame == m_gifDecoder->GetFrameCount())
        m_nCurrFrame = 0;

    if ( m_physX == wxDefaultCoord )
    {
        m_physX = m_physY = 0;
        for (wxHtmlCell *cell = this; cell; cell = cell->GetParent())
        {
            m_physX += cell->GetPosX();
            m_physY += cell->GetPosY();
        }
    }

    wxWindow *win = m_windowIface->GetHTMLWindow();
    wxPoint pos =
        m_windowIface->HTMLCoordsToWindow(this, wxPoint(m_physX, m_physY));
    wxRect rect(pos, wxSize(m_Width, m_Height));

    if ( win->GetClientRect().Intersects(rect) &&
         m_gifDecoder->ConvertToImage(m_nCurrFrame, &img) )
    {
#if !defined(__WXMSW__) || wxUSE_WXDIB
        if ( m_gifDecoder->GetFrameSize(m_nCurrFrame) != wxSize(m_Width, m_Height) ||
             m_gifDecoder->GetFramePosition(m_nCurrFrame) != wxPoint(0, 0) )
        {
            wxBitmap bmp(img);
            wxMemoryDC dc;
            dc.SelectObject(*m_bitmap);
            dc.DrawBitmap(bmp, m_gifDecoder->GetFramePosition(m_nCurrFrame),
                          true /* use mask */);
        }
        else
#endif
            SetImage(img);
        win->Refresh(img.HasMask(), &rect);
    }

    auto delay = m_gifDecoder->GetDelay(m_nCurrFrame);
    if ( delay == 0ms )
        delay = 1ms;
    timer->Start(delay, true);
}
#endif

void wxHtmlImageCell::Layout(int w)
{
    if (m_bmpWpercent)
    {

        m_Width = w*m_bmpW/100;

        if (!m_bmpHpresent && m_bitmap != nullptr)
            m_Height = m_bitmap->GetScaledHeight()*m_Width/m_bitmap->GetScaledWidth();
        else
            m_Height = static_cast<int>(m_scale*m_bmpH);
    } else
    {
        m_Width  = static_cast<int>(m_scale*m_bmpW);
        m_Height = static_cast<int>(m_scale*m_bmpH);
    }

    switch (m_align)
    {
        case wxHTML_ALIGN_TOP :
            m_Descent = m_Height;
            break;
        case wxHTML_ALIGN_CENTER :
            m_Descent = m_Height / 2;
            break;
        case wxHTML_ALIGN_BOTTOM :
        default :
            m_Descent = 0;
            break;
    }

    wxHtmlCell::Layout(w);
#if wxUSE_GIF && wxUSE_TIMER
    m_physX = m_physY = wxDefaultCoord;
#endif
}

wxHtmlImageCell::~wxHtmlImageCell()
{
    delete m_bitmap;
#if wxUSE_GIF && wxUSE_TIMER
    delete m_gifTimer;
    delete m_gifDecoder;
#endif
}


void wxHtmlImageCell::Draw(wxDC& dc, int x, int y,
                           [[maybe_unused]] int view_y1, [[maybe_unused]] int view_y2,
                           [[maybe_unused]] wxHtmlRenderingInfo& info)
{
    if ( m_showFrame )
    {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawRectangle(x + m_PosX, y + m_PosY, m_Width, m_Height);
        x++, y++;
    }
    if ( m_bitmap && m_Width && m_Height )
    {
        // We add in the scaling from the desired bitmap width
        // and height, so we only do the scaling once.
        double imageScaleX = 1.0;
        double imageScaleY = 1.0;

        // Optimisation for Windows: WIN32 scaling for window DCs is very poor,
        // so unless we're using a printer DC, do the scaling ourselves.
#if defined(__WXMSW__) && wxUSE_IMAGE
        if (m_Width != m_bitmap->GetWidth()
    #if wxUSE_PRINTING_ARCHITECTURE
            && !dc.IsKindOf(CLASSINFO(wxPrinterDC))
    #endif
           )
        {
            wxImage image(m_bitmap->ConvertToImage());
            if (image.HasMask())
            {
                // Convert the mask to an alpha channel or scaling won't work correctly
                image.InitAlpha();
            }
            image.Rescale(m_Width, m_Height, wxImageResizeQuality::High);
            (*m_bitmap) = wxBitmap(image);
        }
#endif 

        if (m_Width != m_bitmap->GetScaledWidth())
            imageScaleX = (double) m_Width / (double) m_bitmap->GetScaledWidth();
        if (m_Height != m_bitmap->GetScaledHeight())
            imageScaleY = (double) m_Height / (double) m_bitmap->GetScaledHeight();

        wxScale userScale = dc.GetUserScale();
        dc.SetUserScale({ userScale.x * imageScaleX, userScale.y * imageScaleY });

        dc.DrawBitmap(*m_bitmap, (int) ((x + m_PosX) / (imageScaleX)),
                                 (int) ((y + m_PosY) / (imageScaleY)), true);
        dc.SetUserScale(userScale);
    }
}

wxHtmlLinkInfo *wxHtmlImageCell::GetLink( int x, int y ) const
{
    if (m_mapName.empty())
        return wxHtmlCell::GetLink( x, y );
    if (!m_imageMap)
    {
        wxHtmlContainerCell *p, *op;
        op = p = GetParent();
        while (p)
        {
            op = p;
            p = p->GetParent();
        }
        p = op;
        const wxHtmlCell* cell = p->Find(wxHTML_COND_ISIMAGEMAP,
                                                (const void*)(&m_mapName));
        if (!cell)
        {
            m_mapName.Clear();
            return wxHtmlCell::GetLink( x, y );
        }
        m_imageMap = dynamic_cast<const wxHtmlImageMapCell*>(cell);
    }
    return m_imageMap->GetLink(x, y);
}



//--------------------------------------------------------------------------------
// tag handler
//--------------------------------------------------------------------------------

TAG_HANDLER_BEGIN(IMG, "IMG,MAP,AREA")
    TAG_HANDLER_CONSTR(IMG) { }

    TAG_HANDLER_PROC(tag)
    {
        if (tag.GetName() == "IMG")
        {
            wxString tmp;
            if (tag.GetParamAsString("SRC", &tmp))
            {
                int w = wxDefaultCoord, h = wxDefaultCoord;
                bool wpercent = false;
                bool hpresent = false;
                int al;
                wxFSFile *str = nullptr;
                wxString mn;
                double scaleHDPI = 1.0;

#if defined(__WXOSX_COCOA__)
                // Try to find a 2x resolution image with @2x appended before the file extension.
                wxWindow* win = m_WParser->GetWindowInterface() ? m_WParser->GetWindowInterface()->GetHTMLWindow() : NULL;
                if (!win)
                    win = wxApp::GetMainTopWindow();
                if (win && win->GetContentScaleFactor() > 1.0)
                {
                    if (tmp.Find('.') != wxNOT_FOUND)
                    {
                        wxString ext = tmp.AfterLast('.');
                        wxString rest = tmp.BeforeLast('.');
                        wxString hiDPIFilename = rest + "@2x." + ext;
                        str = m_WParser->OpenURL(wxHtmlURLType::Image, hiDPIFilename);
                        if (str)
                        {
                            scaleHDPI = 2.0;
                        }
                    }
                }                    
#endif
                if (!str)
                    str = m_WParser->OpenURL(wxHtmlURLType::Image, tmp);

                if (tag.GetParamAsIntOrPercent("WIDTH", &w, wpercent))
                {
                    if (wpercent)
                    {
                        if (w < 0)
                            w = 0;
                        else if (w > 100)
                            w = 100;
                    }
                }

                if (tag.GetParamAsInt("HEIGHT", &h))
                {
                    hpresent = true;
                }

                al = wxHTML_ALIGN_BOTTOM;
                wxString alstr;
                if (tag.GetParamAsString("ALIGN", &alstr))
                {
                    alstr.MakeUpper();  // for the case alignment was in ".."
                    if (alstr == "TEXTTOP")
                        al = wxHTML_ALIGN_TOP;
                    else if ((alstr == "CENTER") || (alstr == "ABSCENTER"))
                        al = wxHTML_ALIGN_CENTER;
                }
                if (tag.GetParamAsString("USEMAP", &mn))
                {
                    if ( !mn.empty() && *mn.begin() == '#' )
                    {
                        mn = mn.Mid( 1 );
                    }
                }
                wxHtmlImageCell *cel = new wxHtmlImageCell(
                                          m_WParser->GetWindowInterface(),
                                          str, scaleHDPI, w, wpercent, h, hpresent,
                                          m_WParser->GetPixelScale(),
                                          al, mn);
                m_WParser->ApplyStateToCell(cel);
                m_WParser->StopCollapsingSpaces();
                cel->SetId(tag.GetParam("id")); // may be empty
                cel->SetAlt(tag.GetParam("alt"));
                m_WParser->GetContainer()->InsertCell(cel);
                delete str;
            }
        }
        if (tag.GetName() == "MAP")
        {
            m_WParser->CloseContainer();
            m_WParser->OpenContainer();
            wxString tmp;
            if (tag.GetParamAsString("NAME", &tmp))
            {
                wxHtmlImageMapCell *cel = new wxHtmlImageMapCell( tmp );
                m_WParser->GetContainer()->InsertCell( cel );
            }
            ParseInner( tag );
            m_WParser->CloseContainer();
            m_WParser->OpenContainer();
        }
        if (tag.GetName() == "AREA")
        {
            wxString tmp;
            if (tag.GetParamAsString("SHAPE", &tmp))
            {
                wxString coords = tag.GetParam("COORDS");
                tmp.MakeUpper();
                wxHtmlImageMapAreaCell *cel = nullptr;
                if (tmp == "POLY")
                {
                    cel = new wxHtmlImageMapAreaCell( wxHtmlImageMapAreaCell::POLY, coords, m_WParser->GetPixelScale() );
                }
                else if (tmp == "CIRCLE")
                {
                    cel = new wxHtmlImageMapAreaCell( wxHtmlImageMapAreaCell::CIRCLE, coords, m_WParser->GetPixelScale() );
                }
                else if (tmp == "RECT")
                {
                    cel = new wxHtmlImageMapAreaCell( wxHtmlImageMapAreaCell::RECT, coords, m_WParser->GetPixelScale() );
                }
                wxString href;
                if (cel != nullptr && tag.GetParamAsString("HREF", &href))
                    cel->SetLink(wxHtmlLinkInfo(href, tag.GetParam("TARGET")));
                if (cel != nullptr)
                    m_WParser->GetContainer()->InsertCell( cel );
            }
        }

        return false;
    }

TAG_HANDLER_END(IMG)



TAGS_MODULE_BEGIN(Image)

    TAGS_MODULE_ADD(IMG)

TAGS_MODULE_END(Image)


#endif
