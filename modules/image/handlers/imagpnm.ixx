/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagpnm.h
// Purpose:     wxImage PNM handler
// Author:      Sylvain Bougnoux
// Copyright:   (c) Sylvain Bougnoux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/gdicmn.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/txtstrm.h"
#include "wx/stream.h"

export module WX.Image.PNM;

import WX.Image.Base;

//-----------------------------------------------------------------------------
// wxPNMHandler
//-----------------------------------------------------------------------------

#if wxUSE_PNM

namespace
{

void Skip_Comment(wxInputStream &stream)
{
    wxTextInputStream text_stream(stream);

    if (stream.Peek()==wxT('#'))
    {
        text_stream.ReadLine();
        Skip_Comment(stream);
    }
}

} // namespace anonymous

export
{

class wxPNMHandler : public wxImageHandler
{
public:
    wxPNMHandler()
    {
        m_name = "PNM file";
        m_extension = "pnm";
        m_altExtensions.push_back("ppm");
        m_altExtensions.push_back("pgm");
        m_altExtensions.push_back("pbm");
        m_type = wxBitmapType::PNM;
        m_mime = "image/pnm";
    }

#if wxUSE_STREAMS
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
protected:
    bool DoCanRead( wxInputStream& stream ) override;
#endif
};


#if wxUSE_STREAMS

bool wxPNMHandler::LoadFile( wxImage *image, wxInputStream& stream, bool verbose, [[maybe_unused]] int index )
{
    std::uint32_t  width, height;
    std::uint16_t  maxval;
    char      c(0);

    image->Destroy();

    /*
     * Read the PNM header
     */

    wxBufferedInputStream buf_stream(stream);
    wxTextInputStream text_stream(buf_stream);

    Skip_Comment(buf_stream);
    if (buf_stream.GetC()==wxT('P')) c=buf_stream.GetC();

    switch (c)
    {
        case wxT('2'): // ASCII Grey
        case wxT('3'): // ASCII RGB
        case wxT('5'): // RAW Grey
        case wxT('6'): break;
        default:
            if (verbose)
            {
                wxLogError(_("PNM: File format is not recognized."));
            }
            return false;
    }

    text_stream.ReadLine(); // for the \n
    Skip_Comment(buf_stream);
    text_stream >> width >> height ;
    Skip_Comment(buf_stream);
    text_stream >> maxval;

    //cout << line << " " << width << " " << height << " " << maxval << endl;
    image->Create( width, height );
    unsigned char *ptr = image->GetData();
    if (!ptr)
    {
        if (verbose)
        {
           wxLogError( _("PNM: Couldn't allocate memory.") );
        }
        return false;
    }


    if (c=='2') // Ascii GREY
    {
        std::uint32_t size=width*height;
        for (std::uint32_t i=0; i<size; ++i)
        {
            std::uint32_t value;
            value=text_stream.Read32();
            if ( maxval != 255 )
                value = (255 * value)/maxval;
            *ptr++=(unsigned char)value; // R
            *ptr++=(unsigned char)value; // G
            *ptr++=(unsigned char)value; // B
            if ( !buf_stream )
            {
                if (verbose)
                {
                    wxLogError(_("PNM: File seems truncated."));
                }
                return false;
            }
        }
    }
    if (c=='3') // Ascii RBG
    {
        std::uint32_t size=3*width*height;
        for (std::uint32_t i=0; i<size; ++i)
          {
            //this is very slow !!!
            //I wonder how we can make any better ?
            std::uint32_t value;
            value=text_stream.Read32();
            if ( maxval != 255 )
                value = (255 * value)/maxval;
            *ptr++=(unsigned char)value;

            if ( !buf_stream )
              {
                if (verbose)
                {
                    wxLogError(_("PNM: File seems truncated."));
                }
                return false;
              }
          }
    }
    if (c=='5') // Raw GREY
    {
        std::uint32_t size=width*height;
        for (std::uint32_t i=0; i<size; ++i)
        {
            unsigned char value;
            buf_stream.Read(&value,1);
            if ( maxval != 255 )
                value = (255 * value)/maxval;
            *ptr++=value; // R
            *ptr++=value; // G
            *ptr++=value; // B
            if ( !buf_stream )
            {
                if (verbose)
                {
                    wxLogError(_("PNM: File seems truncated."));
                }
                return false;
            }
        }
    }

    if ( c=='6' ) // Raw RGB
    {
        buf_stream.Read(ptr, 3*width*height);
        if ( maxval != 255 )
        {
            for ( unsigned i = 0; i < 3*width*height; i++ )
                ptr[i] = (255 * ptr[i])/maxval;
        }
    }

    image->SetMask( false );

    const wxStreamError err = buf_stream.GetLastError();
    return err == wxSTREAM_NO_ERROR || err == wxSTREAM_EOF;
}

bool wxPNMHandler::SaveFile( wxImage *image, wxOutputStream& stream, [[maybe_unused]] bool verbose )
{
    wxTextOutputStream text_stream(stream);

    //text_stream << "P6" << endl
    //<< image->GetWidth() << " " << image->GetHeight() << endl
    //<< "255" << endl;
    text_stream << "P6\n" << image->GetWidth() << " " << image->GetHeight() << "\n255\n";
    stream.Write(image->GetData(),3*image->GetWidth()*image->GetHeight());

    return stream.IsOk();
}

bool wxPNMHandler::DoCanRead( wxInputStream& stream )
{
    Skip_Comment(stream);

    // it's ok to modify the stream position here
    if ( stream.GetC() == 'P' )
    {
        switch ( stream.GetC() )
        {
            case '2': // ASCII Grey
            case '3': // ASCII RGB
            case '5': // RAW Grey
            case '6': // RAW RGB
                return true;
        }
    }

    return false;
}


#endif // wxUSE_STREAMS

} // export

#endif // wxUSE_PNM
