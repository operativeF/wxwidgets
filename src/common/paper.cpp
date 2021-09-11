/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/paper.cpp
// Purpose:     Paper size classes
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_PRINTING_ARCHITECTURE

#ifndef WX_PRECOMP
    #if defined(__WXMSW__)
        #include "wx/msw/wrapcdlg.h"
    #endif // MSW
#endif

#include "wx/utils.h"
#include "wx/settings.h"
#include "wx/intl.h"
#include "wx/module.h"
#include "wx/paper.h"

#ifdef __WXMSW__
    #ifndef __WIN32__
        #include <print.h>
    #endif
#endif
 // End __WXMSW__

/*
 * Paper size database for all platforms
 */

wxPrintPaperType::wxPrintPaperType(wxPaperSize paperId, int platformId, const wxString& name, int w, int h)
    : m_paperName(name),
      m_paperId(paperId),
      m_platformId(platformId),
      m_width(w),
      m_height(h)
{
}

// Get width and height in points (1/72th of an inch)
wxSize wxPrintPaperType::GetSizeDeviceUnits() const
{
    return { (int) ((m_width / 10.0) / (25.4 / 72.0)), (int) ((m_height / 10.0) / (25.4 / 72.0)) };
}

/*
 * Print paper database for PostScript
 */

WX_DECLARE_LIST(wxPrintPaperType, wxPrintPaperTypeList);
#include "wx/listimpl.cpp"
WX_DEFINE_LIST(wxPrintPaperTypeList)

wxPrintPaperDatabase* wxThePrintPaperDatabase = nullptr;

wxPrintPaperDatabase::wxPrintPaperDatabase()
    : m_map(new wxStringToPrintPaperTypeHashMap),
      m_list(new wxPrintPaperTypeList)
{
}

wxPrintPaperDatabase::~wxPrintPaperDatabase()
{
    ClearDatabase();
}

void wxPrintPaperDatabase::CreateDatabase()
{
    WXADDPAPER(wxPaperSize::Letter,             DMPAPER_LETTER,             wxTRANSLATE("Letter, 8 1/2 x 11 in"), 2159, 2794);
    WXADDPAPER(wxPaperSize::Legal,              DMPAPER_LEGAL,              wxTRANSLATE("Legal, 8 1/2 x 14 in"), 2159, 3556);
    WXADDPAPER(wxPaperSize::A4,                 DMPAPER_A4,                 wxTRANSLATE("A4 sheet, 210 x 297 mm"), 2100, 2970);
    WXADDPAPER(wxPaperSize::Csheet,             DMPAPER_CSHEET,             wxTRANSLATE("C sheet, 17 x 22 in"), 4318, 5588);
    WXADDPAPER(wxPaperSize::Dsheet,             DMPAPER_DSHEET,             wxTRANSLATE("D sheet, 22 x 34 in"), 5588, 8636);
    WXADDPAPER(wxPaperSize::Esheet,             DMPAPER_ESHEET,             wxTRANSLATE("E sheet, 34 x 44 in"), 8636, 11176);
    WXADDPAPER(wxPaperSize::Lettersmall,        DMPAPER_LETTERSMALL,        wxTRANSLATE("Letter Small, 8 1/2 x 11 in"), 2159, 2794);
    WXADDPAPER(wxPaperSize::Tabloid,            DMPAPER_TABLOID,            wxTRANSLATE("Tabloid, 11 x 17 in"), 2794, 4318);
    WXADDPAPER(wxPaperSize::Ledger,             DMPAPER_LEDGER,             wxTRANSLATE("Ledger, 17 x 11 in"), 4318, 2794);
    WXADDPAPER(wxPaperSize::Statement,          DMPAPER_STATEMENT,          wxTRANSLATE("Statement, 5 1/2 x 8 1/2 in"), 1397, 2159);
    WXADDPAPER(wxPaperSize::Executive,          DMPAPER_EXECUTIVE,          wxTRANSLATE("Executive, 7 1/4 x 10 1/2 in"), 1842, 2667);
    WXADDPAPER(wxPaperSize::A3,                 DMPAPER_A3,                 wxTRANSLATE("A3 sheet, 297 x 420 mm"), 2970, 4200);
    WXADDPAPER(wxPaperSize::A4small,            DMPAPER_A4SMALL,            wxTRANSLATE("A4 small sheet, 210 x 297 mm"), 2100, 2970);
    WXADDPAPER(wxPaperSize::A5,                 DMPAPER_A5,                 wxTRANSLATE("A5 sheet, 148 x 210 mm"), 1480, 2100);
    WXADDPAPER(wxPaperSize::B4,                 DMPAPER_B4,                 wxTRANSLATE("B4 sheet, 250 x 354 mm"), 2500, 3540);
    WXADDPAPER(wxPaperSize::B5,                 DMPAPER_B5,                 wxTRANSLATE("B5 sheet, 182 x 257 millimeter"), 1820, 2570);
    WXADDPAPER(wxPaperSize::Folio,              DMPAPER_FOLIO,              wxTRANSLATE("Folio, 8 1/2 x 13 in"), 2159, 3302);
    WXADDPAPER(wxPaperSize::Quarto,             DMPAPER_QUARTO,             wxTRANSLATE("Quarto, 215 x 275 mm"), 2150, 2750);
    WXADDPAPER(wxPaperSize::_10X14,              DMPAPER_10X14,              wxTRANSLATE("10 x 14 in"), 2540, 3556);
    WXADDPAPER(wxPaperSize::_11X17,              DMPAPER_11X17,              wxTRANSLATE("11 x 17 in"), 2794, 4318);
    WXADDPAPER(wxPaperSize::Note,               DMPAPER_NOTE,               wxTRANSLATE("Note, 8 1/2 x 11 in"), 2159, 2794);
    WXADDPAPER(wxPaperSize::Env_9,              DMPAPER_ENV_9,              wxTRANSLATE("#9 Envelope, 3 7/8 x 8 7/8 in"), 984, 2254);
    WXADDPAPER(wxPaperSize::Env_10,             DMPAPER_ENV_10,             wxTRANSLATE("#10 Envelope, 4 1/8 x 9 1/2 in"), 1048, 2413);
    WXADDPAPER(wxPaperSize::Env_11,             DMPAPER_ENV_11,             wxTRANSLATE("#11 Envelope, 4 1/2 x 10 3/8 in"), 1143, 2635);
    WXADDPAPER(wxPaperSize::Env_12,             DMPAPER_ENV_12,             wxTRANSLATE("#12 Envelope, 4 3/4 x 11 in"), 1206, 2794);
    WXADDPAPER(wxPaperSize::Env_14,             DMPAPER_ENV_14,             wxTRANSLATE("#14 Envelope, 5 x 11 1/2 in"), 1270, 2921);
    WXADDPAPER(wxPaperSize::Env_dl,             DMPAPER_ENV_DL,             wxTRANSLATE("DL Envelope, 110 x 220 mm"), 1100, 2200);
    WXADDPAPER(wxPaperSize::Env_c5,             DMPAPER_ENV_C5,             wxTRANSLATE("C5 Envelope, 162 x 229 mm"), 1620, 2290);
    WXADDPAPER(wxPaperSize::Env_c3,             DMPAPER_ENV_C3,             wxTRANSLATE("C3 Envelope, 324 x 458 mm"), 3240, 4580);
    WXADDPAPER(wxPaperSize::Env_c4,             DMPAPER_ENV_C4,             wxTRANSLATE("C4 Envelope, 229 x 324 mm"), 2290, 3240);
    WXADDPAPER(wxPaperSize::Env_c6,             DMPAPER_ENV_C6,             wxTRANSLATE("C6 Envelope, 114 x 162 mm"), 1140, 1620);
    WXADDPAPER(wxPaperSize::Env_c65,            DMPAPER_ENV_C65,            wxTRANSLATE("C65 Envelope, 114 x 229 mm"), 1140, 2290);
    WXADDPAPER(wxPaperSize::Env_b4,             DMPAPER_ENV_B4,             wxTRANSLATE("B4 Envelope, 250 x 353 mm"), 2500, 3530);
    WXADDPAPER(wxPaperSize::Env_b5,             DMPAPER_ENV_B5,             wxTRANSLATE("B5 Envelope, 176 x 250 mm"), 1760, 2500);
    WXADDPAPER(wxPaperSize::Env_b6,             DMPAPER_ENV_B6,             wxTRANSLATE("B6 Envelope, 176 x 125 mm"), 1760, 1250);
    WXADDPAPER(wxPaperSize::Env_italy,          DMPAPER_ENV_ITALY,          wxTRANSLATE("Italy Envelope, 110 x 230 mm"), 1100, 2300);
    WXADDPAPER(wxPaperSize::Env_monarch,        DMPAPER_ENV_MONARCH,        wxTRANSLATE("Monarch Envelope, 3 7/8 x 7 1/2 in"), 984, 1905);
    WXADDPAPER(wxPaperSize::Env_personal,       DMPAPER_ENV_PERSONAL,       wxTRANSLATE("6 3/4 Envelope, 3 5/8 x 6 1/2 in"), 921, 1651);
    WXADDPAPER(wxPaperSize::Fanfold_us,         DMPAPER_FANFOLD_US,         wxTRANSLATE("US Std Fanfold, 14 7/8 x 11 in"), 3778, 2794);
    WXADDPAPER(wxPaperSize::Fanfold_std_german, DMPAPER_FANFOLD_STD_GERMAN, wxTRANSLATE("German Std Fanfold, 8 1/2 x 12 in"), 2159, 3048);
    WXADDPAPER(wxPaperSize::Fanfold_lgl_german, DMPAPER_FANFOLD_LGL_GERMAN, wxTRANSLATE("German Legal Fanfold, 8 1/2 x 13 in"), 2159, 3302);

    WXADDPAPER(wxPaperSize::Iso_b4,             DMPAPER_ISO_B4,             wxTRANSLATE("B4 (ISO) 250 x 353 mm"), 2500, 3530);
    WXADDPAPER(wxPaperSize::Japanese_postcard,  DMPAPER_JAPANESE_POSTCARD,  wxTRANSLATE("Japanese Postcard 100 x 148 mm"), 1000, 1480);
    WXADDPAPER(wxPaperSize::_9X11,               DMPAPER_9X11,               wxTRANSLATE("9 x 11 in"), 2286, 2794);
    WXADDPAPER(wxPaperSize::_10X11,              DMPAPER_10X11,              wxTRANSLATE("10 x 11 in"), 2540, 2794);
    WXADDPAPER(wxPaperSize::_15X11,              DMPAPER_15X11,              wxTRANSLATE("15 x 11 in"), 3810, 2794);
    WXADDPAPER(wxPaperSize::Env_invite,         DMPAPER_ENV_INVITE,         wxTRANSLATE("Envelope Invite 220 x 220 mm"), 2200, 2200);
    WXADDPAPER(wxPaperSize::Letter_extra,       DMPAPER_LETTER_EXTRA,       wxTRANSLATE("Letter Extra 9 1/2 x 12 in"), 2413, 3048);
    WXADDPAPER(wxPaperSize::Legal_extra,        DMPAPER_LEGAL_EXTRA,        wxTRANSLATE("Legal Extra 9 1/2 x 15 in"), 2413, 3810);
    WXADDPAPER(wxPaperSize::Tabloid_extra,      DMPAPER_TABLOID_EXTRA,      wxTRANSLATE("Tabloid Extra 11.69 x 18 in"), 2969, 4572);
    WXADDPAPER(wxPaperSize::A4_extra,           DMPAPER_A4_EXTRA,           wxTRANSLATE("A4 Extra 9.27 x 12.69 in"), 2355, 3223);
    WXADDPAPER(wxPaperSize::Letter_transverse,  DMPAPER_LETTER_TRANSVERSE,  wxTRANSLATE("Letter Transverse 8 1/2 x 11 in"), 2159, 2794);
    WXADDPAPER(wxPaperSize::A4_transverse,      DMPAPER_A4_TRANSVERSE,      wxTRANSLATE("A4 Transverse 210 x 297 mm"), 2100, 2970);
    WXADDPAPER(wxPaperSize::Letter_extra_transverse, DMPAPER_LETTER_EXTRA_TRANSVERSE, wxTRANSLATE("Letter Extra Transverse 9.275 x 12 in"), 2355, 3048);
    WXADDPAPER(wxPaperSize::A_plus,             DMPAPER_A_PLUS,             wxTRANSLATE("SuperA/SuperA/A4 227 x 356 mm"), 2270, 3560);
    WXADDPAPER(wxPaperSize::B_plus,             DMPAPER_B_PLUS,             wxTRANSLATE("SuperB/SuperB/A3 305 x 487 mm"), 3050, 4870);
    WXADDPAPER(wxPaperSize::Letter_plus,        DMPAPER_LETTER_PLUS,        wxTRANSLATE("Letter Plus 8 1/2 x 12.69 in"), 2159, 3223);
    WXADDPAPER(wxPaperSize::A4_plus,            DMPAPER_A4_PLUS,            wxTRANSLATE("A4 Plus 210 x 330 mm"), 2100, 3300);
    WXADDPAPER(wxPaperSize::A5_transverse,      DMPAPER_A5_TRANSVERSE,      wxTRANSLATE("A5 Transverse 148 x 210 mm"), 1480, 2100);
    WXADDPAPER(wxPaperSize::B5_transverse,      DMPAPER_B5_TRANSVERSE,      wxTRANSLATE("B5 (JIS) Transverse 182 x 257 mm"), 1820, 2570);
    WXADDPAPER(wxPaperSize::A3_extra,           DMPAPER_A3_EXTRA,           wxTRANSLATE("A3 Extra 322 x 445 mm"), 3220, 4450);
    WXADDPAPER(wxPaperSize::A5_extra,           DMPAPER_A5_EXTRA,           wxTRANSLATE("A5 Extra 174 x 235 mm"), 1740, 2350);
    WXADDPAPER(wxPaperSize::B5_extra,           DMPAPER_B5_EXTRA,           wxTRANSLATE("B5 (ISO) Extra 201 x 276 mm"), 2010, 2760);
    WXADDPAPER(wxPaperSize::A2,                 DMPAPER_A2,                 wxTRANSLATE("A2 420 x 594 mm"), 4200, 5940);
    WXADDPAPER(wxPaperSize::A3_transverse,      DMPAPER_A3_TRANSVERSE,      wxTRANSLATE("A3 Transverse 297 x 420 mm"), 2970, 4200);
    WXADDPAPER(wxPaperSize::A3_extra_transverse,DMPAPER_A3_EXTRA_TRANSVERSE,wxTRANSLATE("A3 Extra Transverse 322 x 445 mm"), 3220, 4450);

    WXADDPAPER(wxPaperSize::Dbl_japanese_postcard, 69,                       wxTRANSLATE("Japanese Double Postcard 200 x 148 mm"), 2000, 1480);
    WXADDPAPER(wxPaperSize::A6,                  70,                         wxTRANSLATE("A6 105 x 148 mm"), 1050, 1480);
    WXADDPAPER(wxPaperSize::Jenv_kaku2,          71,                         wxTRANSLATE("Japanese Envelope Kaku #2"), 2400, 3320);
    WXADDPAPER(wxPaperSize::Jenv_kaku3,          72,                         wxTRANSLATE("Japanese Envelope Kaku #3"), 2160, 2770);
    WXADDPAPER(wxPaperSize::Jenv_chou3,          73,                         wxTRANSLATE("Japanese Envelope Chou #3"), 1200, 2350);
    WXADDPAPER(wxPaperSize::Jenv_chou4,          74,                         wxTRANSLATE("Japanese Envelope Chou #4"), 900, 2050);
    WXADDPAPER(wxPaperSize::Letter_rotated,      75,                         wxTRANSLATE("Letter Rotated 11 x 8 1/2 in"), 2794, 2159);
    WXADDPAPER(wxPaperSize::A3_rotated,          76,                         wxTRANSLATE("A3 Rotated 420 x 297 mm"), 4200, 2970);
    WXADDPAPER(wxPaperSize::A4_rotated,          77,                         wxTRANSLATE("A4 Rotated 297 x 210 mm"), 2970, 2100);
    WXADDPAPER(wxPaperSize::A5_rotated,          78,                         wxTRANSLATE("A5 Rotated 210 x 148 mm"), 2100, 1480);
    WXADDPAPER(wxPaperSize::B4_jis_rotated,      79,                         wxTRANSLATE("B4 (JIS) Rotated 364 x 257 mm"), 3640, 2570);
    WXADDPAPER(wxPaperSize::B5_jis_rotated,      80,                         wxTRANSLATE("B5 (JIS) Rotated 257 x 182 mm"), 2570, 1820);
    WXADDPAPER(wxPaperSize::Japanese_postcard_rotated, 81,                   wxTRANSLATE("Japanese Postcard Rotated 148 x 100 mm"), 1480, 1000);
    WXADDPAPER(wxPaperSize::Dbl_japanese_postcard_rotated, 82,               wxTRANSLATE("Double Japanese Postcard Rotated 148 x 200 mm"), 1480, 2000);
    WXADDPAPER(wxPaperSize::A6_rotated,          83,                         wxTRANSLATE("A6 Rotated 148 x 105 mm"), 1480, 1050);
    WXADDPAPER(wxPaperSize::Jenv_kaku2_rotated,  84,                         wxTRANSLATE("Japanese Envelope Kaku #2 Rotated"), 3320, 2400);
    WXADDPAPER(wxPaperSize::Jenv_kaku3_rotated,  85,                         wxTRANSLATE("Japanese Envelope Kaku #3 Rotated"), 2770, 2160);
    WXADDPAPER(wxPaperSize::Jenv_chou3_rotated,  86,                         wxTRANSLATE("Japanese Envelope Chou #3 Rotated"), 2350, 1200);
    WXADDPAPER(wxPaperSize::Jenv_chou4_rotated,  87,                         wxTRANSLATE("Japanese Envelope Chou #4 Rotated"), 2050, 900);
    WXADDPAPER(wxPaperSize::B6_jis,              88,                         wxTRANSLATE("B6 (JIS) 128 x 182 mm"), 1280, 1820);
    WXADDPAPER(wxPaperSize::B6_jis_rotated,      89,                         wxTRANSLATE("B6 (JIS) Rotated 182 x 128 mm"), 1920, 1280);
    WXADDPAPER(wxPaperSize::_12X11,               90,                         wxTRANSLATE("12 x 11 in"), 3048, 2794);
    WXADDPAPER(wxPaperSize::Jenv_you4,           91,                         wxTRANSLATE("Japanese Envelope You #4"), 2350, 1050);
    WXADDPAPER(wxPaperSize::Jenv_you4_rotated,   92,                         wxTRANSLATE("Japanese Envelope You #4 Rotated"), 1050, 2350);
    WXADDPAPER(wxPaperSize::P16k,                93,                         wxTRANSLATE("PRC 16K 146 x 215 mm"), 1460, 2150);
    WXADDPAPER(wxPaperSize::P32k,                94,                         wxTRANSLATE("PRC 32K 97 x 151 mm"), 970, 1510);
    WXADDPAPER(wxPaperSize::P32kbig,             95,                         wxTRANSLATE("PRC 32K(Big) 97 x 151 mm"), 970, 1510);
    WXADDPAPER(wxPaperSize::Penv_1,              96,                         wxTRANSLATE("PRC Envelope #1 102 x 165 mm"), 1020, 1650);
    WXADDPAPER(wxPaperSize::Penv_2,              97,                         wxTRANSLATE("PRC Envelope #2 102 x 176 mm"), 1020, 1760);
    WXADDPAPER(wxPaperSize::Penv_3,              98,                         wxTRANSLATE("PRC Envelope #3 125 x 176 mm"), 1250, 1760);
    WXADDPAPER(wxPaperSize::Penv_4,              99,                         wxTRANSLATE("PRC Envelope #4 110 x 208 mm"), 1100, 2080);
    WXADDPAPER(wxPaperSize::Penv_5,              100,                        wxTRANSLATE("PRC Envelope #5 110 x 220 mm"), 1100, 2200);
    WXADDPAPER(wxPaperSize::Penv_6,              101,                        wxTRANSLATE("PRC Envelope #6 120 x 230 mm"), 1200, 2300);
    WXADDPAPER(wxPaperSize::Penv_7,              102,                        wxTRANSLATE("PRC Envelope #7 160 x 230 mm"), 1600, 2300);
    WXADDPAPER(wxPaperSize::Penv_8,              103,                        wxTRANSLATE("PRC Envelope #8 120 x 309 mm"), 1200, 3090);
    WXADDPAPER(wxPaperSize::Penv_9,              104,                        wxTRANSLATE("PRC Envelope #9 229 x 324 mm"), 2290, 3240);
    WXADDPAPER(wxPaperSize::Penv_10,             105,                        wxTRANSLATE("PRC Envelope #10 324 x 458 mm"), 3240, 4580);
    WXADDPAPER(wxPaperSize::P16k_rotated,        106,                        wxTRANSLATE("PRC 16K Rotated"), 2150, 1460);
    WXADDPAPER(wxPaperSize::P32k_rotated,        107,                        wxTRANSLATE("PRC 32K Rotated"), 1510, 970);
    WXADDPAPER(wxPaperSize::P32kbig_rotated,     108,                        wxTRANSLATE("PRC 32K(Big) Rotated"), 1510, 970);
    WXADDPAPER(wxPaperSize::Penv_1_rotated,      109,                        wxTRANSLATE("PRC Envelope #1 Rotated 165 x 102 mm"), 1650, 1020);
    WXADDPAPER(wxPaperSize::Penv_2_rotated,      110,                        wxTRANSLATE("PRC Envelope #2 Rotated 176 x 102 mm"), 1760, 1020);
    WXADDPAPER(wxPaperSize::Penv_3_rotated,      111,                        wxTRANSLATE("PRC Envelope #3 Rotated 176 x 125 mm"), 1760, 1250);
    WXADDPAPER(wxPaperSize::Penv_4_rotated,      112,                        wxTRANSLATE("PRC Envelope #4 Rotated 208 x 110 mm"), 2080, 1100);
    WXADDPAPER(wxPaperSize::Penv_5_rotated,      113,                        wxTRANSLATE("PRC Envelope #5 Rotated 220 x 110 mm"), 2200, 1100);
    WXADDPAPER(wxPaperSize::Penv_6_rotated,      114,                        wxTRANSLATE("PRC Envelope #6 Rotated 230 x 120 mm"), 2300, 1200);
    WXADDPAPER(wxPaperSize::Penv_7_rotated,      115,                        wxTRANSLATE("PRC Envelope #7 Rotated 230 x 160 mm"), 2300, 1600);
    WXADDPAPER(wxPaperSize::Penv_8_rotated,      116,                        wxTRANSLATE("PRC Envelope #8 Rotated 309 x 120 mm"), 3090, 1200);
    WXADDPAPER(wxPaperSize::Penv_9_rotated,      117,                        wxTRANSLATE("PRC Envelope #9 Rotated 324 x 229 mm"), 3240, 2290);
    WXADDPAPER(wxPaperSize::Penv_10_rotated,     118,                        wxTRANSLATE("PRC Envelope #10 Rotated 458 x 324 mm"), 4580, 3240);

    // notice that the values 135 and 136 for Windows paper size ids of A0 and
    // A1 formats are not documented anywhere but seem to work for at least
    // some printers so we use them until we find a better way (see #11083)
    WXADDPAPER(wxPaperSize::A0,                  136,                        wxTRANSLATE("A0 sheet, 841 x 1189 mm"), 8410, 11888);
    WXADDPAPER(wxPaperSize::A1,                  135,                        wxTRANSLATE("A1 sheet, 594 x 841 mm"), 5940, 8410);
}

void wxPrintPaperDatabase::ClearDatabase()
{
    delete m_list;
    WX_CLEAR_HASH_MAP(wxStringToPrintPaperTypeHashMap, *m_map);
    delete m_map;
}

void wxPrintPaperDatabase::AddPaperType(wxPaperSize paperId, const wxString& name, int w, int h)
{
    wxPrintPaperType* tmp = new wxPrintPaperType(paperId, 0, name, w, h);
    (*m_map)[name] = tmp;
    m_list->push_back(tmp);
}

void wxPrintPaperDatabase::AddPaperType(wxPaperSize paperId, int platformId, const wxString& name, int w, int h)
{
    wxPrintPaperType* tmp = new wxPrintPaperType(paperId, platformId, name, w, h);
    (*m_map)[name] = tmp;
    m_list->push_back(tmp);
}

wxPrintPaperType *wxPrintPaperDatabase::FindPaperType(const wxString& name) const
{
    wxStringToPrintPaperTypeHashMap::iterator it = m_map->find(name);
    if (it != m_map->end())
        return it->second;
    else
        return nullptr;
}

wxPrintPaperType *wxPrintPaperDatabase::FindPaperType(wxPaperSize id) const
{
    using iterator = wxStringToPrintPaperTypeHashMap::iterator;

    for (iterator it = m_map->begin(), en = m_map->end(); it != en; ++it)
    {
        wxPrintPaperType* paperType = it->second;
        if (paperType->GetId() == id)
            return paperType;
    }

    return nullptr;
}

wxPrintPaperType *wxPrintPaperDatabase::FindPaperTypeByPlatformId(int id) const
{
    using iterator = wxStringToPrintPaperTypeHashMap::iterator;

    for (iterator it = m_map->begin(), en = m_map->end(); it != en; ++it)
    {
        wxPrintPaperType* paperType = it->second;
        if (paperType->GetPlatformId() == id)
            return paperType;
    }

    return nullptr;
}

wxPrintPaperType *wxPrintPaperDatabase::FindPaperType(const wxSize& sz) const
{
    // Take the item ordering into account so that the more common types
    // are likely to be taken into account first. This fixes problems with,
    // for example, Letter reverting to A4 in the page setup dialog because
    // it was wrongly translated to Note.
    for ( size_t i = 0; i < GetCount(); i++ )
    {
        wxPrintPaperType* const paperType = Item(i);
        const wxSize paperSize = paperType->GetSize() ;
        if ( std::abs(paperSize.x - sz.x) < 10 && std::abs(paperSize.y - sz.y) < 10 )
            return paperType;
    }

    return nullptr;
}

// Convert name to size id
wxPaperSize wxPrintPaperDatabase::ConvertNameToId(const wxString& name) const
{
    wxPrintPaperType* type = FindPaperType(name);
    if (type)
        return type->GetId();
    else
        return wxPaperSize::None;
}

// Convert size id to name
wxString wxPrintPaperDatabase::ConvertIdToName(wxPaperSize paperId) const
{
    wxPrintPaperType* type = FindPaperType(paperId);
    if (type)
        return type->GetName();
    else
        return wxEmptyString;
}

// Get the paper size
wxSize wxPrintPaperDatabase::GetSize(wxPaperSize paperId) const
{
    wxPrintPaperType* type = FindPaperType(paperId);
    if (type)
        return type->GetSize();
    else
        return {0, 0};
}

// Get the paper size
wxPaperSize wxPrintPaperDatabase::GetSize(const wxSize& size) const
{
    wxPrintPaperType* type = FindPaperType(size);
    if (type)
        return type->GetId();
    else
        return wxPaperSize::None;
}

// QUICK and DIRTY
size_t wxPrintPaperDatabase::GetCount() const
{
    return m_list->GetCount();
}

wxPrintPaperType* wxPrintPaperDatabase::Item(size_t index) const
{
    return m_list->Item(index)->GetData();
}

// A module to allow initialization/cleanup of print paper
// things without calling these functions from app.cpp.

class WXDLLEXPORT wxPrintPaperModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxPrintPaperModule);
public:
    bool OnInit() override;
    void OnExit() override;
};

wxIMPLEMENT_DYNAMIC_CLASS(wxPrintPaperModule, wxModule);

/*
 * Initialization/cleanup module
 */

bool wxPrintPaperModule::OnInit()
{
    wxThePrintPaperDatabase = new wxPrintPaperDatabase;
    wxThePrintPaperDatabase->CreateDatabase();

    return true;
}

void wxPrintPaperModule::OnExit()
{
    wxDELETE(wxThePrintPaperDatabase);
}

#endif // wxUSE_PRINTING_ARCHITECTURE
