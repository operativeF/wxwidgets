/////////////////////////////////////////////////////////////////////////////
// Name:        wx/printercfg.h
// Purpose:     Configuration data for printing
// Author:      Thomas Figueroa
// Modified by:
// Created:     2021-05-11
// Copyright:   (c) Thomas Figueroa
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRINTERCFG_H_BASE_
#define _WX_PRINTERCFG_H_BASE_

/*
 * wxPrintData
 * Encapsulates printer information (not printer dialog information)
 */

enum class wxPrintBin
{
    Default,

    OnlyOne,
    Lower,
    Middle,
    Manual,
    Envelope,
    EnvManual,
    Auto,
    Tractor,
    SmallFmt,
    LargeFmt,
    LargeCapacity,
    Cassette,
    FormSource,

    User
};

/* Print mode (currently PostScript only)
 */

enum class wxPrintMode
{
    None,
    Preview,   /*  Preview in external application */
    File,      /*  Print to file */
    Printer,   /*  Send to printer */
    Stream     /*  Send postscript data into a stream */
};

/* Paper types */
enum class wxPaperSize
{
    None,               /*  Use specific dimensions */
    Letter,             /*  Letter, 8 1/2 by 11 inches */
    Legal,              /*  Legal, 8 1/2 by 14 inches */
    A4,                 /*  A4 Sheet, 210 by 297 millimeters */
    Csheet,             /*  C Sheet, 17 by 22 inches */
    Dsheet,             /*  D Sheet, 22 by 34 inches */
    Esheet,             /*  E Sheet, 34 by 44 inches */
    Lettersmall,        /*  Letter Small, 8 1/2 by 11 inches */
    Tabloid,            /*  Tabloid, 11 by 17 inches */
    Ledger,             /*  Ledger, 17 by 11 inches */
    Statement,          /*  Statement, 5 1/2 by 8 1/2 inches */
    Executive,          /*  Executive, 7 1/4 by 10 1/2 inches */
    A3,                 /*  A3 sheet, 297 by 420 millimeters */
    A4small,            /*  A4 small sheet, 210 by 297 millimeters */
    A5,                 /*  A5 sheet, 148 by 210 millimeters */
    B4,                 /*  B4 sheet, 250 by 354 millimeters */
    B5,                 /*  B5 sheet, 182-by-257-millimeter paper */
    Folio,              /*  Folio, 8-1/2-by-13-inch paper */
    Quarto,             /*  Quarto, 215-by-275-millimeter paper */
    _10X14,              /*  10-by-14-inch sheet */
    _11X17,              /*  11-by-17-inch sheet */
    Note,               /*  Note, 8 1/2 by 11 inches */
    Env_9,              /*  #9 Envelope, 3 7/8 by 8 7/8 inches */
    Env_10,             /*  #10 Envelope, 4 1/8 by 9 1/2 inches */
    Env_11,             /*  #11 Envelope, 4 1/2 by 10 3/8 inches */
    Env_12,             /*  #12 Envelope, 4 3/4 by 11 inches */
    Env_14,             /*  #14 Envelope, 5 by 11 1/2 inches */
    Env_dl,             /*  DL Envelope, 110 by 220 millimeters */
    Env_c5,             /*  C5 Envelope, 162 by 229 millimeters */
    Env_c3,             /*  C3 Envelope, 324 by 458 millimeters */
    Env_c4,             /*  C4 Envelope, 229 by 324 millimeters */
    Env_c6,             /*  C6 Envelope, 114 by 162 millimeters */
    Env_c65,            /*  C65 Envelope, 114 by 229 millimeters */
    Env_b4,             /*  B4 Envelope, 250 by 353 millimeters */
    Env_b5,             /*  B5 Envelope, 176 by 250 millimeters */
    Env_b6,             /*  B6 Envelope, 176 by 125 millimeters */
    Env_italy,          /*  Italy Envelope, 110 by 230 millimeters */
    Env_monarch,        /*  Monarch Envelope, 3 7/8 by 7 1/2 inches */
    Env_personal,       /*  6 3/4 Envelope, 3 5/8 by 6 1/2 inches */
    Fanfold_us,         /*  US Std Fanfold, 14 7/8 by 11 inches */
    Fanfold_std_german, /*  German Std Fanfold, 8 1/2 by 12 inches */
    Fanfold_lgl_german, /*  German Legal Fanfold, 8 1/2 by 13 inches */

    Iso_b4,             /*  B4 (ISO) 250 x 353 mm */
    Japanese_postcard,  /*  Japanese Postcard 100 x 148 mm */
    _9X11,               /*  9 x 11 in */
    _10X11,              /*  10 x 11 in */
    _15X11,              /*  15 x 11 in */
    Env_invite,         /*  Envelope Invite 220 x 220 mm */
    Letter_extra,       /*  Letter Extra 9 \275 x 12 in */
    Legal_extra,        /*  Legal Extra 9 \275 x 15 in */
    Tabloid_extra,      /*  Tabloid Extra 11.69 x 18 in */
    A4_extra,           /*  A4 Extra 9.27 x 12.69 in */
    Letter_transverse,  /*  Letter Transverse 8 \275 x 11 in */
    A4_transverse,      /*  A4 Transverse 210 x 297 mm */
    Letter_extra_transverse, /*  Letter Extra Transverse 9\275 x 12 in */
    A_plus,             /*  SuperA/SuperA/A4 227 x 356 mm */
    B_plus,             /*  SuperB/SuperB/A3 305 x 487 mm */
    Letter_plus,        /*  Letter Plus 8.5 x 12.69 in */
    A4_plus,            /*  A4 Plus 210 x 330 mm */
    A5_transverse,      /*  A5 Transverse 148 x 210 mm */
    B5_transverse,      /*  B5 (JIS) Transverse 182 x 257 mm */
    A3_extra,           /*  A3 Extra 322 x 445 mm */
    A5_extra,           /*  A5 Extra 174 x 235 mm */
    B5_extra,           /*  B5 (ISO) Extra 201 x 276 mm */
    A2,                 /*  A2 420 x 594 mm */
    A3_transverse,      /*  A3 Transverse 297 x 420 mm */
    A3_extra_transverse, /*  A3 Extra Transverse 322 x 445 mm */

    Dbl_japanese_postcard,/* Japanese Double Postcard 200 x 148 mm */
    A6,                 /* A6 105 x 148 mm */
    Jenv_kaku2,         /* Japanese Envelope Kaku #2 */
    Jenv_kaku3,         /* Japanese Envelope Kaku #3 */
    Jenv_chou3,         /* Japanese Envelope Chou #3 */
    Jenv_chou4,         /* Japanese Envelope Chou #4 */
    Letter_rotated,     /* Letter Rotated 11 x 8 1/2 in */
    A3_rotated,         /* A3 Rotated 420 x 297 mm */
    A4_rotated,         /* A4 Rotated 297 x 210 mm */
    A5_rotated,         /* A5 Rotated 210 x 148 mm */
    B4_jis_rotated,     /* B4 (JIS) Rotated 364 x 257 mm */
    B5_jis_rotated,     /* B5 (JIS) Rotated 257 x 182 mm */
    Japanese_postcard_rotated,/* Japanese Postcard Rotated 148 x 100 mm */
    Dbl_japanese_postcard_rotated,/* Double Japanese Postcard Rotated 148 x 200 mm */
    A6_rotated,         /* A6 Rotated 148 x 105 mm */
    Jenv_kaku2_rotated, /* Japanese Envelope Kaku #2 Rotated */
    Jenv_kaku3_rotated, /* Japanese Envelope Kaku #3 Rotated */
    Jenv_chou3_rotated, /* Japanese Envelope Chou #3 Rotated */
    Jenv_chou4_rotated, /* Japanese Envelope Chou #4 Rotated */
    B6_jis,             /* B6 (JIS) 128 x 182 mm */
    B6_jis_rotated,     /* B6 (JIS) Rotated 182 x 128 mm */
    _12X11,              /* 12 x 11 in */
    Jenv_you4,          /* Japanese Envelope You #4 */
    Jenv_you4_rotated,  /* Japanese Envelope You #4 Rotated */
    P16k,               /* PRC 16K 146 x 215 mm */
    P32k,               /* PRC 32K 97 x 151 mm */
    P32kbig,            /* PRC 32K(Big) 97 x 151 mm */
    Penv_1,             /* PRC Envelope #1 102 x 165 mm */
    Penv_2,             /* PRC Envelope #2 102 x 176 mm */
    Penv_3,             /* PRC Envelope #3 125 x 176 mm */
    Penv_4,             /* PRC Envelope #4 110 x 208 mm */
    Penv_5,             /* PRC Envelope #5 110 x 220 mm */
    Penv_6,             /* PRC Envelope #6 120 x 230 mm */
    Penv_7,             /* PRC Envelope #7 160 x 230 mm */
    Penv_8,             /* PRC Envelope #8 120 x 309 mm */
    Penv_9,             /* PRC Envelope #9 229 x 324 mm */
    Penv_10,            /* PRC Envelope #10 324 x 458 mm */
    P16k_rotated,       /* PRC 16K Rotated */
    P32k_rotated,       /* PRC 32K Rotated */
    P32kbig_rotated,    /* PRC 32K(Big) Rotated */
    Penv_1_rotated,     /* PRC Envelope #1 Rotated 165 x 102 mm */
    Penv_2_rotated,     /* PRC Envelope #2 Rotated 176 x 102 mm */
    Penv_3_rotated,     /* PRC Envelope #3 Rotated 176 x 125 mm */
    Penv_4_rotated,     /* PRC Envelope #4 Rotated 208 x 110 mm */
    Penv_5_rotated,     /* PRC Envelope #5 Rotated 220 x 110 mm */
    Penv_6_rotated,     /* PRC Envelope #6 Rotated 230 x 120 mm */
    Penv_7_rotated,     /* PRC Envelope #7 Rotated 230 x 160 mm */
    Penv_8_rotated,     /* PRC Envelope #8 Rotated 309 x 120 mm */
    Penv_9_rotated,     /* PRC Envelope #9 Rotated 324 x 229 mm */
    Penv_10_rotated,    /* PRC Envelope #10 Rotated 458 x 324 m */
    A0,                 /* A0 Sheet 841 x 1189 mm */
    A1                  /* A1 Sheet 594 x 841 mm */
};

/* Printing orientation */
enum class wxPrintOrientation
{
   Portrait = 1,
   Landscape
};

/* Duplex printing modes
 */

enum class wxDuplexMode
{
    Simplex, /*  Non-duplex */
    Horizontal,
    Vertical
};

/* Print quality.
 */

inline constexpr int wxPRINT_QUALITY_HIGH    = -1;
inline constexpr int wxPRINT_QUALITY_MEDIUM  = -2;
inline constexpr int wxPRINT_QUALITY_LOW     = -3;
inline constexpr int wxPRINT_QUALITY_DRAFT   = -4;

using wxPrintQuality = int;

#endif
