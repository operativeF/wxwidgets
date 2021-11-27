module;

#include "wx/private/icondir.h"
#include "wx/log.h"
#include "wx/stream.h"
#include "wx/translation.h"

module WX.Image.ICO;

import WX.Image.Base;
import WX.Image.PNG;

wxIMPLEMENT_DYNAMIC_CLASS(wxICOHandler, wxBMPHandler);

#if wxUSE_STREAMS

bool wxICOHandler::SaveFile(wxImage *image,
                            wxOutputStream& stream,
                            bool verbose)

{
    // sanity check; icon must be no larger than 256x256
    if ( image->GetHeight () > 256 )
    {
        if ( verbose )
        {
            wxLogError(_("ICO: Image too tall for an icon."));
        }
        return false;
    }
    if ( image->GetWidth () > 256 )
    {
        if ( verbose )
        {
            wxLogError(_("ICO: Image too wide for an icon."));
        }
        return false;
    }

    static constexpr int images = 1; // only generate one image

    // VS: This is a hack of sort - since ICO and CUR files are almost
    //     identical, we have all the meat in wxICOHandler and check for
    //     the actual (handler) type when the code has to distinguish between
    //     the two formats
    const int type = (this->GetType() == wxBitmapType::CUR) ? 2 : 1;

    // write a header, (ICONDIR)
    // Calculate the header size
    std::uint32_t offset = 3 * sizeof(std::uint16_t);

    ICONDIR IconDir;
    IconDir.idReserved = 0;
    IconDir.idType = wxUINT16_SWAP_ON_BE((std::uint16_t)type);
    IconDir.idCount = wxUINT16_SWAP_ON_BE((std::uint16_t)images);
    if ( !stream.WriteAll(&IconDir.idReserved, sizeof(IconDir.idReserved)) ||
         !stream.WriteAll(&IconDir.idType, sizeof(IconDir.idType)) ||
         !stream.WriteAll(&IconDir.idCount, sizeof(IconDir.idCount)) )
    {
        if ( verbose )
        {
            wxLogError(_("ICO: Error writing the image file!"));
        }
        return false;
    }

    // for each iamage write a description ICONDIRENTRY:
    ICONDIRENTRY icondirentry;
    for (int img = 0; img < images; img++)
    {
        wxImage mask;

        if ( image->HasMask() )
        {
            // make another image with black/white:
            mask = image->ConvertToMono (image->GetMaskRed(), image->GetMaskGreen(), image->GetMaskBlue() );

            // now we need to change the masked regions to black:
            unsigned char r = image->GetMaskRed();
            unsigned char g = image->GetMaskGreen();
            unsigned char b = image->GetMaskBlue();
            if ( (r != 0) || (g != 0) || (b != 0) )
            {
                // Go round and apply black to the masked bits:
                for (int i = 0; i < mask.GetWidth(); i++)
                {
                    for (int j = 0; j < mask.GetHeight(); j++)
                    {
                        if ((r == mask.GetRed(i, j)) &&
                            (g == mask.GetGreen(i, j))&&
                            (b == mask.GetBlue(i, j)) )
                                image->SetRGB(i, j, 0, 0, 0 );
                    }
                }
            }
        }
        else
        {
            // just make a black mask all over:
            mask = image->Copy();

            for (int i = 0; i < mask.GetWidth(); i++)
                for (int j = 0; j < mask.GetHeight(); j++)
                    mask.SetRGB(i, j, 0, 0, 0 );
        }
        // Set the formats for image and mask

        // The format depends on the number of the colours used, so count them,
        // but stop at 257 because we have to use 24 bpp anyhow if we have that
        // many of them.
        const int colours = image->CountColours(257);
        int bppFormat;
        int bpp;
        if ( image->HasAlpha() )
        {
            // Icons with alpha channel are always stored in ARGB format.
            bppFormat = wxBMP_24BPP;
            bpp = 32;
        }
        else if ( colours > 256 )
        {
            bppFormat = wxBMP_24BPP;
            bpp = 24;
        }
        else if ( colours > 16 )
        {
            bppFormat = wxBMP_8BPP;
            bpp = 8;
        }
        else if ( colours > 2 )
        {
            bppFormat = wxBMP_4BPP;
            bpp = 4;
        }
        else
        {
            bppFormat = wxBMP_1BPP;
            bpp = 1;
        }
        image->SetOption(wxIMAGE_OPTION_BMP_FORMAT, bppFormat);

        // monochome bitmap:
        mask.SetOption(wxIMAGE_OPTION_BMP_FORMAT, wxBMP_1BPP_BW);
        bool IsBmp = false;
        bool IsMask = false;

        //calculate size and offset of image and mask
        wxCountingOutputStream cStream;
        bool bResult;
#if wxUSE_LIBPNG
        // Typically, icons larger then 128x128 are saved as PNG images.
        bool saveAsPNG = false;
        if ( image->GetHeight() > 128 || image->GetWidth() > 128 )
        {
            wxPNGHandler handlerPNG;
            bResult = handlerPNG.SaveFile(image, cStream, verbose);
            if ( !bResult )
            {
                if ( verbose )
                {
                    wxLogError(_("ICO: Error writing the image file!"));
                }
                return false;
            }

            saveAsPNG = true;
        }
        if ( !saveAsPNG )
#endif // wxUSE_LIBPNG
        {
            bResult = SaveDib(image, cStream, verbose, IsBmp, IsMask);
            if ( !bResult )
            {
                if ( verbose )
                {
                    wxLogError(_("ICO: Error writing the image file!"));
                }
                return false;
            }
            IsMask = true;

            bResult = SaveDib(&mask, cStream, verbose, IsBmp, IsMask);
            if ( !bResult )
            {
                if ( verbose )
                {
                    wxLogError(_("ICO: Error writing the image file!"));
                }
                return false;
            }
        }
        std::uint32_t Size = cStream.GetSize();

        // wxCountingOutputStream::IsOk() always returns true for now and this
        // "if" provokes VC++ warnings in optimized build
#if 0
        if ( !cStream.IsOk() )
        {
            if ( verbose )
            {
                wxLogError(_("ICO: Error writing the image file!"));
            }
            return false;
        }
#endif // 0

        offset = offset + sizeof(ICONDIRENTRY);

        // Notice that the casts work correctly for width/height of 256 as it's
        // represented by 0 in ICO file format -- and larger values are not
        // allowed at all.
        icondirentry.bWidth = (std::uint8_t)image->GetWidth();
        icondirentry.bHeight = (std::uint8_t)image->GetHeight();
        icondirentry.bColorCount = 0;
        icondirentry.bReserved = 0;
        icondirentry.wPlanes = wxUINT16_SWAP_ON_BE(1);
        icondirentry.wBitCount = wxUINT16_SWAP_ON_BE(bpp);
        if ( type == 2 /*CUR*/)
        {
            int hx = image->HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_X) ?
                         image->GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_X) :
                         image->GetWidth() / 2;
            int hy = image->HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y) ?
                         image->GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_Y) :
                         image->GetHeight() / 2;

            // actually write the values of the hot spot here:
            icondirentry.wPlanes = wxUINT16_SWAP_ON_BE((std::uint16_t)hx);
            icondirentry.wBitCount = wxUINT16_SWAP_ON_BE((std::uint16_t)hy);
        }
        icondirentry.dwBytesInRes = wxUINT32_SWAP_ON_BE(Size);
        icondirentry.dwImageOffset = wxUINT32_SWAP_ON_BE(offset);

        // increase size to allow for the data written:
        offset += Size;

        // write to stream:
        if ( !stream.WriteAll(&icondirentry.bWidth, sizeof(icondirentry.bWidth)) ||
             !stream.WriteAll(&icondirentry.bHeight, sizeof(icondirentry.bHeight)) ||
             !stream.WriteAll(&icondirentry.bColorCount, sizeof(icondirentry.bColorCount)) ||
             !stream.WriteAll(&icondirentry.bReserved, sizeof(icondirentry.bReserved)) ||
             !stream.WriteAll(&icondirentry.wPlanes, sizeof(icondirentry.wPlanes)) ||
             !stream.WriteAll(&icondirentry.wBitCount, sizeof(icondirentry.wBitCount)) ||
             !stream.WriteAll(&icondirentry.dwBytesInRes, sizeof(icondirentry.dwBytesInRes)) ||
             !stream.WriteAll(&icondirentry.dwImageOffset, sizeof(icondirentry.dwImageOffset)) )
        {
            if ( verbose )
            {
                wxLogError(_("ICO: Error writing the image file!"));
            }
            return false;
        }

        // actually save it:
#if wxUSE_LIBPNG
        if ( saveAsPNG )
        {
            wxPNGHandler handlerPNG;
            bResult = handlerPNG.SaveFile(image, stream, verbose);
            if ( !bResult )
            {
                if ( verbose )
                {
                    wxLogError(_("ICO: Error writing the image file!"));
                }
                return false;
            }
        }
        else
#endif // wxUSE_LIBPNG
        {
            IsMask = false;
            bResult = SaveDib(image, stream, verbose, IsBmp, IsMask);
            if ( !bResult )
            {
                if ( verbose )
                {
                    wxLogError(_("ICO: Error writing the image file!"));
                }
                return false;
            }
            IsMask = true;

            bResult = SaveDib(&mask, stream, verbose, IsBmp, IsMask);
            if ( !bResult )
            {
                if ( verbose )
                {
                    wxLogError(_("ICO: Error writing the image file!"));
                }
                return false;
            }
        }

    } // end of for loop

    return true;
}

bool wxICOHandler::LoadFile(wxImage *image, wxInputStream& stream,
                            bool verbose, int index)
{
    if ( stream.IsSeekable() && stream.SeekI(0) == wxInvalidOffset )
    {
        return false;
    }

    return DoLoadFile(image, stream, verbose, index);
}

bool wxICOHandler::DoLoadFile(wxImage *image, wxInputStream& stream,
                            bool verbose, int index)
{
    bool bResult wxDUMMY_INITIALIZE(false);

    ICONDIR IconDir;

    if ( !stream.ReadAll(&IconDir, sizeof(IconDir)) )
        return false;

    std::uint16_t nIcons = wxUINT16_SWAP_ON_BE(IconDir.idCount);

    // nType is 1 for Icons, 2 for Cursors:
    std::uint16_t nType = wxUINT16_SWAP_ON_BE(IconDir.idType);

    // loop round the icons and choose the best one:
    auto pIconDirEntry = std::make_unique<ICONDIRENTRY[]>(nIcons);
    ICONDIRENTRY *pCurrentEntry = pIconDirEntry.get();
    int wMax = 0;
    int colmax = 0;
    int iSel = wxNOT_FOUND;

    // remember how many bytes we read from the stream:
    wxFileOffset alreadySeeked = sizeof(IconDir);

    for (unsigned int i = 0; i < nIcons; i++ )
    {
        if ( !stream.ReadAll(pCurrentEntry, sizeof(ICONDIRENTRY)) )
            return false;

        alreadySeeked += stream.LastRead();

        // ICO file format uses only a single byte for width and if it is 0, it
        // means that the width is actually 256 pixels.
        const std::uint16_t
            widthReal = pCurrentEntry->bWidth ? pCurrentEntry->bWidth : 256;

        // bHeight and bColorCount are std::uint8_t
        if ( widthReal >= wMax )
        {
            // see if we have more colors, ==0 indicates > 8bpp:
            if ( pCurrentEntry->bColorCount == 0 )
                pCurrentEntry->bColorCount = 255;
            if ( pCurrentEntry->bColorCount >= colmax )
            {
                iSel = i;
                wMax = widthReal;
                colmax = pCurrentEntry->bColorCount;
            }
        }

        pCurrentEntry++;
    }

    if ( index != -1 )
    {
        // VS: Note that we *have* to run the loop above even if index != -1, because
        //     it reads ICONDIRENTRies.
        iSel = index;
    }

    if ( iSel == wxNOT_FOUND || iSel < 0 || iSel >= nIcons )
    {
        wxLogError(_("ICO: Invalid icon index."));
        bResult = false;
    }
    else
    {
        // seek to selected icon:
        pCurrentEntry = pIconDirEntry.get() + iSel;

        // NOTE: seeking a positive amount in wxSeekMode::FromCurrent mode allows us to
        //       load even non-seekable streams (see wxInputStream::SeekI docs)!
        wxFileOffset offset = wxUINT32_SWAP_ON_BE(pCurrentEntry->dwImageOffset) - alreadySeeked;
        if (offset != 0 && stream.SeekI(offset, wxSeekMode::FromCurrent) == wxInvalidOffset)
            return false;

#if wxUSE_LIBPNG
        // We can't fall back to loading an icon in the usual BMP format after
        // trying to load it as PNG if we have an unseekable stream, so to
        // avoid breaking the existing code which does successfully load icons
        // from such streams, we only try to load them as PNGs if we can unwind
        // back later.
        //
        // Ideal would be to modify LoadDib() to accept the first 8 bytes not
        // coming from the stream but from the signature buffer below, as then
        // we'd be able to load PNG icons from any kind of streams.
        bool isPNG;
        if ( stream.IsSeekable() )
        {
            // Check for the PNG signature first to avoid wasting time on
            // trying to load typical ICO files which are not PNGs at all.
            static constexpr unsigned char signaturePNG[] =
            {
                0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
            };
            static constexpr int signatureLen = WXSIZEOF(signaturePNG);

            unsigned char signature[signatureLen];
            if ( !stream.ReadAll(signature, signatureLen) )
                return false;

            isPNG = memcmp(signature, signaturePNG, signatureLen) == 0;

            // Rewind to the beginning of the image in any case.
            if ( stream.SeekI(-signatureLen, wxSeekMode::FromCurrent) == wxInvalidOffset )
                return false;
        }
        else // Not seekable stream
        {
            isPNG = false;
        }

        if ( isPNG )
        {
            wxPNGHandler handlerPNG;
            bResult = handlerPNG.LoadFile(image, stream, verbose);
        }
        else
#endif // wxUSE_LIBPNG
        {
            bResult = LoadDib(image, stream, verbose, false /* not BMP */);
        }
        bool bIsCursorType = (this->GetType() == wxBitmapType::CUR) || (this->GetType() == wxBitmapType::ANI);
        if ( bResult && bIsCursorType && nType == 2 )
        {
            // it is a cursor, so let's set the hotspot:
            image->SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, wxUINT16_SWAP_ON_BE(pCurrentEntry->wPlanes));
            image->SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, wxUINT16_SWAP_ON_BE(pCurrentEntry->wBitCount));
        }
    }

    return bResult;
}

int wxICOHandler::DoGetImageCount(wxInputStream& stream)
{
    // It's ok to modify the stream position in this function.

    if ( stream.IsSeekable() && stream.SeekI(0) == wxInvalidOffset )
    {
        return 0;
    }

    ICONDIR IconDir;

    if ( !stream.ReadAll(&IconDir, sizeof(IconDir)) )
        return 0;

    return (int)wxUINT16_SWAP_ON_BE(IconDir.idCount);
}

bool wxICOHandler::DoCanRead(wxInputStream& stream)
{
    return CanReadICOOrCUR(&stream, 1 /*for identifying an icon*/);

}

#endif // wxUSE_STREAMS
