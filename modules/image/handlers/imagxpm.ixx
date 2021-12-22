/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagxpm.h
// Purpose:     wxImage XPM handler
// Author:      Vaclav Slavik
// Copyright:   (c) 2001 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/gdicmn.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/utils.h"
#include "wx/xpmdecod.h"
#include "wx/filename.h"

export module WX.Image.XPM;

import WX.Cmn.Stream;

import WX.Image.Base;
import Utils.Chars;
import Utils.Strings;
import WX.Cmn.WFStream;

#if wxUSE_XPM


namespace
{

// Make the given string a valid C identifier.
//
// All invalid characters are simply replaced by underscores and underscore is
// also prepended in the beginning if the initial character is not alphabetic.
void MakeValidCIdent(std::string* str)
{
    for ( auto it = str->begin(); it != str->end(); ++it )
    {
        const char ch = *it;
        if ( wx::utils::isDigit(ch) )
        {
            if ( it == str->begin() )
            {
                // Identifiers can't start with a digit.
                str->insert(0, "_"); // prepend underscore
                it = str->begin(); // restart as string changed
                continue;
            }
        }
        else if ( !wx::utils::isAlpha(ch) && ch != '_' )
        {
            // Not a valid character in C identifiers.
            *it = '_';
        }
    }

    // Double underscores are not allowed in normal C identifiers and are
    // useless anyhow.
    wx::utils::ReplaceAll(*str, "__", "_");
}

} // anonymous namespace

export
{

//-----------------------------------------------------------------------------
// wxXPMHandler
//-----------------------------------------------------------------------------

class wxXPMHandler : public wxImageHandler
{
public:
    wxXPMHandler()
    {
        m_name = "XPM file";
        m_extension = "xpm";
        m_type = wxBitmapType::XPM;
        m_mime = "image/xpm";
    }

#if wxUSE_STREAMS
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
protected:
    bool DoCanRead( wxInputStream& stream ) override;
#endif
};

} // export


#if wxUSE_STREAMS

bool wxXPMHandler::LoadFile(wxImage *image,
                            wxInputStream& stream,
                            [[maybe_unused]] bool verbose, [[maybe_unused]] int index)
{
    wxXPMDecoder decoder;

    wxImage img = decoder.ReadFile(stream);
    if ( !img.IsOk() )
        return false;
    *image = img;
    return true;
}

bool wxXPMHandler::SaveFile(wxImage * image,
                            wxOutputStream& stream, [[maybe_unused]] bool verbose)
{
    // 1. count colours:
    #define MaxCixels  92
    static constexpr char Cixel[MaxCixels+1] =
                         " .XoO+@#$%&*=-;:>,<1234567890qwertyuipasdfghjk"
                         "lzxcvbnmMNBVCZASDFGHJKLPIUYTREWQ!~^/()_`'][{}|";
    int i, j, k;

    wxImageHistogram histogram;
    int cols = int(image->ComputeHistogram(histogram));

    int chars_per_pixel = 1;
    for ( k = MaxCixels; cols > k; k *= MaxCixels)
        chars_per_pixel++;

    // 2. write the header:
    std::string sName;
    if ( image->HasOption(wxIMAGE_OPTION_FILENAME) )
    {
        sName = wxFileName(image->GetOption(wxIMAGE_OPTION_FILENAME)).GetName();
        MakeValidCIdent(&sName);
        sName += "_xpm";
    }

    if ( !sName.empty() )
        sName = fmt::format("/* XPM */\nstatic const char* {}", sName);
    else
        sName = "/* XPM */\nstatic const char *xpm_data";
    stream.Write( sName.data(), sName.length() );

    char tmpbuf[200];
    // VS: 200b is safe upper bound for anything produced by sprintf below
    //     (<101 bytes the string, neither %i can expand into more than 10 chars)
    sprintf(tmpbuf,
               "[] = {\n"
               "/* columns rows colors chars-per-pixel */\n"
               "\"%i %i %i %i\",\n",
               image->GetWidth(), image->GetHeight(), cols, chars_per_pixel);
    stream.Write(tmpbuf, strlen(tmpbuf));

    // 3. create color symbols table:
    char *symbols_data = new char[cols * (chars_per_pixel+1)];
    char **symbols = new char*[cols];

    // 2a. find mask colour:
    unsigned long mask_key = 0x1000000 /*invalid RGB value*/;
    if (image->HasMask())
        mask_key = (image->GetMaskRed() << 16) |
                   (image->GetMaskGreen() << 8) | image->GetMaskBlue();

    // 2b. generate colour table:
    for (wxImageHistogram::iterator entry = histogram.begin();
         entry != histogram.end(); ++entry )
    {
        unsigned long index = entry->second.index;
        symbols[index] = symbols_data + index * (chars_per_pixel+1);
        char *sym = symbols[index];

        for (j = 0; j < chars_per_pixel; j++)
        {
            sym[j] = Cixel[index % MaxCixels];
            index /= MaxCixels;
        }
        sym[j] = '\0';

        unsigned long key = entry->first;

        if (key == 0)
            sprintf( tmpbuf, "\"%s c Black\",\n", sym);
        else if (key == mask_key)
            sprintf( tmpbuf, "\"%s c None\",\n", sym);
        else
        {
            wxByte r = wxByte(key >> 16);
            wxByte g = wxByte(key >> 8);
            wxByte b = wxByte(key);
            sprintf(tmpbuf, "\"%s c #%02X%02X%02X\",\n", sym, r, g, b);
        }
        stream.Write( tmpbuf, strlen(tmpbuf) );
    }

    stream.Write("/* pixels */\n", 13);

    unsigned char *data = image->GetData();
    for (j = 0; j < image->GetHeight(); j++)
    {
        char tmp_c;
        tmp_c = '\"'; stream.Write(&tmp_c, 1);
        for (i = 0; i < image->GetWidth(); i++, data += 3)
        {
            unsigned long key = (data[0] << 16) | (data[1] << 8) | (data[2]);
            stream.Write(symbols[histogram[key].index], chars_per_pixel);
        }
        tmp_c = '\"'; stream.Write(&tmp_c, 1);
        if ( j + 1 < image->GetHeight() )
        {
            tmp_c = ','; stream.Write(&tmp_c, 1);
        }
        tmp_c = '\n'; stream.Write(&tmp_c, 1);
    }
    stream.Write("};\n", 3 );

    // Clean up:
    delete[] symbols;
    delete[] symbols_data;

    return true;
}

bool wxXPMHandler::DoCanRead(wxInputStream& stream)
{
    wxXPMDecoder decoder;
    return decoder.CanRead(stream);
         // it's ok to modify the stream position here
}

#endif  // wxUSE_STREAMS

#endif // wxUSE_XPM
