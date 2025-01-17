/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/imagfill.cpp
// Purpose:     FloodFill for wxImage
// Author:      Julian Smart
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_IMAGE && !defined(__WXMSW__)
// we have no use for this code in wxMSW...

#include "wx/brush.h"
#include "wx/dc.h"
#include "wx/dcmemory.h"

import WX.Image;

// DoFloodFill
// Fills with the colour extracted from fillBrush, starting at x,y until either
// a color different from the start pixel is reached (wxFloodFillStyle::Surface)
// or fill color is reached (wxFloodFillStyle::Border)

namespace
{

bool MatchPixel(wxImage *img, int x, int y, int w, int h, const wxColour& c)
{
    if ((x<0)||(x>=w)||(y<0)||(y>=h)) return false;

    unsigned char r = img->GetRed(x,y);
    unsigned char g = img->GetGreen(x,y);
    unsigned char b = img->GetBlue(x,y);
    return c.Red() == r && c.Green() == g && c.Blue() == b ;
}

bool MatchBoundaryPixel(wxImage *img, int x, int y, int w, int h, const wxColour & fill, const wxColour& bound)
{
    if ((x<0)||(x>=w)||(y<0)||(y>=h)) return true;

    unsigned char r = img->GetRed(x,y);
    unsigned char g = img->GetGreen(x,y);
    unsigned char b = img->GetBlue(x,y);
    if ( fill.Red() == r && fill.Green() == g && fill.Blue() == b )
        return true;
    if ( bound.Red() == r && bound.Green() == g && bound.Blue() == b )
        return true;
    return false;
}


void
wxImageFloodFill(wxImage *image,
                 wxCoord x, wxCoord y, const wxBrush & fillBrush,
                 const wxColour& testColour, wxFloodFillStyle style)
{
    /* A diamond flood-fill using a circular queue system.
    Each pixel surrounding the current pixel is added to
    the queue if it meets the criteria, then is retrieved in
    its turn.  Code originally based on http://www.drawit.co.nz/Developers.htm,
    with explicit permission to use this for wxWidgets granted by Andrew Empson
    (no copyright claimed)
     */

    int width = image->GetWidth();
    int height = image->GetHeight();

    //Draw using a pen made from the current brush colour
    //Potentially allows us to use patterned flood fills in future code
    wxColour fillColour = fillBrush.GetColour();
    unsigned char r = fillColour.Red();
    unsigned char g = fillColour.Green();
    unsigned char b = fillColour.Blue();

    //initial test :
    if (style == wxFloodFillStyle::Surface)
    {
       //if wxFloodFillStyle::Surface, if fill colour is same as required, we don't do anything
       if (     image->GetRed(x,y)   != r
             || image->GetGreen(x,y) != g
             || image->GetBlue (x,y) != b   )
        {
        //prepare memory for queue
        //queue save, start, read
        size_t *qs, *qst, *qr;

        //queue size (physical)
        long qSz= height * width * 2;
        qst = new size_t [qSz];

        //temporary x and y locations
        int xt, yt;

        for (int i=0; i < qSz; i++)
            qst[i] = 0;

        // start queue
        qs=qr=qst;
        *qs=xt=x;
        qs++;
        *qs=yt=y;
        qs++;

        image->SetRGB(xt,yt,r,g,b);

        //Main queue loop
        while(qr!=qs)
        {
            //Add new members to queue
            //Above current pixel
            if(MatchPixel(image,xt,yt-1,width,height,testColour))
            {
                *qs=xt;
                qs++;
                *qs=yt-1;
                qs++;
                image->SetRGB(xt,yt-1,r,g,b);

                //Loop back to beginning of queue
                if(qs>=(qst+qSz)) qs=qst;
            }

            //Below current pixel
            if(MatchPixel(image,xt,yt+1,width,height,testColour))
            {
                *qs=xt;
                qs++;
                *qs=yt+1;
                qs++;
                image->SetRGB(xt,yt+1,r,g,b);
                if(qs>=(qst+qSz)) qs=qst;
            }

            //Left of current pixel
            if(MatchPixel(image,xt-1,yt,width,height,testColour))
            {
                *qs=xt-1;
                qs++;
                *qs=yt;
                qs++;
                image->SetRGB(xt-1,yt,r,g,b);
                if(qs>=(qst+qSz)) qs=qst;
            }

            //Right of current pixel
            if(MatchPixel(image,xt+1,yt,width,height,testColour))
            {
                *qs=xt+1;
                qs++;
                *qs=yt;
                qs++;
                image->SetRGB(xt+1,yt,r,g,b);
                if(qs>=(qst+qSz)) qs=qst;
            }

            //Retrieve current queue member
            qr+=2;

            //Loop back to the beginning
            if(qr>=(qst+qSz)) qr=qst;
            xt=*qr;
            yt=*(qr+1);

        //Go Back to beginning of loop
        }

        delete[] qst;
        }
    }
    else
    {
    //style is wxFloodFillStyle::Border
    // fill up to testColor border - if already testColour don't do anything
    if (  image->GetRed(x,y)   != testColour.Red()
          || image->GetGreen(x,y) != testColour.Green()
          || image->GetBlue(x,y)  != testColour.Blue() )
    {
        //prepare memory for queue
        //queue save, start, read
        size_t *qs, *qst, *qr;

        //queue size (physical)
        long qSz= height * width * 2;
        qst = new size_t [qSz];

        //temporary x and y locations
        int xt, yt;

        for (int i=0; i < qSz; i++)
            qst[i] = 0;

        // start queue
        qs=qr=qst;
        *qs=xt=x;
        qs++;
        *qs=yt=y;
        qs++;

        image->SetRGB(xt,yt,r,g,b);

        //Main queue loop
        while (qr!=qs)
        {
            //Add new members to queue
            //Above current pixel
            if(!MatchBoundaryPixel(image,xt,yt-1,width,height,fillColour,testColour))
            {
                *qs=xt;
                qs++;
                *qs=yt-1;
                qs++;
                image->SetRGB(xt,yt-1,r,g,b);

                //Loop back to beginning of queue
                if(qs>=(qst+qSz)) qs=qst;
            }

            //Below current pixel
            if(!MatchBoundaryPixel(image,xt,yt+1,width,height,fillColour,testColour))
            {
                *qs=xt;
                qs++;
                *qs=yt+1;
                qs++;
                image->SetRGB(xt,yt+1,r,g,b);
                if(qs>=(qst+qSz)) qs=qst;
            }

            //Left of current pixel
            if(!MatchBoundaryPixel(image,xt-1,yt,width,height,fillColour,testColour))
            {
                *qs=xt-1;
                qs++;
                *qs=yt;
                qs++;
                image->SetRGB(xt-1,yt,r,g,b);
                if(qs>=(qst+qSz)) qs=qst;
            }

            //Right of current pixel
            if(!MatchBoundaryPixel(image,xt+1,yt,width,height,fillColour,testColour))
            {
                *qs=xt+1;
                qs++;
                *qs=yt;
                qs++;
                image->SetRGB(xt+1,yt,r,g,b);
                if(qs>=(qst+qSz)) qs=qst;
            }

            //Retrieve current queue member
            qr+=2;

            //Loop back to the beginning
            if(qr>=(qst+qSz)) qr=qst;
            xt=*qr;
            yt=*(qr+1);

        //Go Back to beginning of loop
        }

        delete[] qst;
        }
    }
    //all done,
}


bool wxDoFloodFill(wxDC *dc, wxCoord x, wxCoord y,
                   const wxColour& col, wxFloodFillStyle style)
{
    if (dc->GetBrush().IsTransparent())
        return true;

    wxSize dcSize = dc->GetSize();

    //it would be nice to fail if we don't get a sensible size...
    wxCHECK_MSG(dcSize.x >= 1 && dcSize.y >= 1, false,
                "In FloodFill, dc.GetSize routine failed, method not supported by this DC");

    const int x_dev = dc->LogicalToDeviceX(x);
    const int y_dev = dc->LogicalToDeviceY(y);

    // if start point is outside dc, can't do anything
    if (!wxRect(0, 0, dcSize.x, dcSize.y).Contains(x_dev, y_dev))
        return false;

    wxBitmap bitmap(dcSize);
    wxMemoryDC memdc(bitmap);
    // match dc scales
    auto userScale = dc->GetUserScale();
    memdc.SetUserScale(userScale);
    auto logicScale = dc->GetLogicalScale();
    memdc.SetLogicalScale(logicScale);

    // get logical size and origin
    const int w_log = dc->DeviceToLogicalXRel(dcSize.x);
    const int h_log = dc->DeviceToLogicalYRel(dcSize.y);
    const int x0_log = dc->DeviceToLogicalX(0);
    const int y0_log = dc->DeviceToLogicalY(0);

    memdc.Blit({0, 0}, {w_log, h_log}, dc, {x0_log, y0_log});
    memdc.SelectObject(wxNullBitmap);

    wxImage image = bitmap.ConvertToImage();
    wxImageFloodFill(&image, x_dev, y_dev, dc->GetBrush(), col, style);
    bitmap = wxBitmap(image);
    memdc.SelectObject(bitmap);
    dc->Blit({x0_log, y0_log}, {w_log, h_log}, &memdc, {0, 0});

    return true;
}

} // namespace anonymous

#endif // wxUSE_IMAGE
