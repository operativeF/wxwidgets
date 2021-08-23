///////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/icondir.h
// Purpose:     Declarations of structs used for loading MS icons
// Author:      wxWidgets team
// Created:     2017-05-19
// Copyright:   (c) 2017 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRIVATE_ICONDIR_H_
#define _WX_PRIVATE_ICONDIR_H_

#include "wx/defs.h"          // wxUint* declarations


// Structs declared here are used for loading group icons from
// .ICO files or MS Windows resources.
// Icon entry and directory structs for .ICO files and
// MS Windows resources are very similar but not identical.

#if wxUSE_ICO_CUR

#if wxUSE_STREAMS

// icon entry in .ICO files
struct ICONDIRENTRY
{
    std::uint8_t         bWidth;               // Width of the image
    std::uint8_t         bHeight;              // Height of the image (times 2)
    std::uint8_t         bColorCount;          // Number of colors in image (0 if >=8bpp)
    std::uint8_t         bReserved;            // Reserved

    // these two are different in icons and cursors:
                                          // icon           or  cursor
    std::uint16_t        wPlanes;              // Color Planes   or  XHotSpot
    std::uint16_t        wBitCount;            // Bits per pixel or  YHotSpot

    std::uint32_t        dwBytesInRes;         // how many bytes in this resource?
    std::uint32_t        dwImageOffset;        // where in the file is this image
};

// icon directory in .ICO files
struct ICONDIR
{
    std::uint16_t     idReserved;   // Reserved
    std::uint16_t     idType;       // resource type (1 for icons, 2 for cursors)
    std::uint16_t     idCount;      // how many images?
};

#endif // wxUSE_STREAMS


#ifdef __WINDOWS__

#pragma pack(push)
#pragma pack(2)

// icon entry in MS Windows resources
struct GRPICONDIRENTRY
{
    std::uint8_t         bWidth;               // Width of the image
    std::uint8_t         bHeight;              // Height of the image (times 2)
    std::uint8_t         bColorCount;          // Number of colors in image (0 if >=8bpp)
    std::uint8_t         bReserved;            // Reserved

    // these two are different in icons and cursors:
                                          // icon           or  cursor
    std::uint16_t        wPlanes;              // Color Planes   or  XHotSpot
    std::uint16_t        wBitCount;            // Bits per pixel or  YHotSpot

    std::uint32_t        dwBytesInRes;         // how many bytes in this resource?

    std::uint16_t        nID;                  // actual icon resource ID
};

// icon directory in MS Windows resources
struct GRPICONDIR
{
    std::uint16_t        idReserved;   // Reserved
    std::uint16_t        idType;       // resource type (1 for icons, 2 for cursors)
    std::uint16_t        idCount;      // how many images?
    GRPICONDIRENTRY idEntries[1]; // The entries for each image
};

#pragma pack(pop)

#endif // __WINDOWS__

#endif // wxUSE_ICO_CUR

#endif // _WX_PRIVATE_ICONDIR_H_
