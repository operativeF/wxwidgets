/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/gdicmn.cpp
// Purpose:     Common GDI classes
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/display.h"
#include "wx/bitmap.h"
#include "wx/brush.h"
#include "wx/colour.h"
#include "wx/cursor.h"
#include "wx/font.h"
#include "wx/gdicmn.h"
#include "wx/gdiobj.h"
#include "wx/icon.h"
#include "wx/iconbndl.h"
#include "wx/log.h"
#include "wx/palette.h"
#include "wx/pen.h"

import WX.Utils.Settings;
import Utils.Geometry;

import <cmath>;

wxIMPLEMENT_ABSTRACT_CLASS(wxGDIObject, wxObject);


wxBrushList* wxTheBrushList;
wxFontList*  wxTheFontList;
wxPenList*   wxThePenList;

wxColourDatabase* wxTheColourDatabase;

wxBitmap  wxNullBitmap;
wxBrush   wxNullBrush;
wxColour  wxNullColour;
wxCursor  wxNullCursor;
wxFont    wxNullFont;
wxIcon    wxNullIcon;
wxPen     wxNullPen;
#if wxUSE_PALETTE
wxPalette wxNullPalette;
#endif
wxIconBundle wxNullIconBundle;

#include "wx/listimpl.cpp"
WX_DEFINE_LIST(wxPointList)


// ============================================================================
// wxColourDatabase
// ============================================================================

// ----------------------------------------------------------------------------
// wxColourDatabase dtor
// ----------------------------------------------------------------------------

wxColourDatabase::~wxColourDatabase()
{
    if ( m_map )
    {
        WX_CLEAR_HASH_MAP(wxStringToColourHashMap, *m_map);
    }
}

// Colour database stuff
void wxColourDatabase::Initialize()
{
    if ( m_map )
    {
        // already initialized
        return;
    }

    m_map = std::make_unique<wxStringToColourHashMap>();

    struct wxColourDesc
    {
        const char* name;
        unsigned char r,g,b;
    };

    // FIXME: Convert to narrow string.
    static constexpr wxColourDesc wxColourTable[] =
    {
        {"AQUAMARINE",112, 219, 147},
        {"BLACK",0, 0, 0},
        {"BLUE", 0, 0, 255},
        {"BLUE VIOLET", 159, 95, 159},
        {"BROWN", 165, 42, 42},
        {"CADET BLUE", 95, 159, 159},
        {"CORAL", 255, 127, 0},
        {"CORNFLOWER BLUE", 66, 66, 111},
        {"CYAN", 0, 255, 255},
        {"DARK GREY", 47, 47, 47},   // ?

        {"DARK GREEN", 47, 79, 47},
        {"DARK OLIVE GREEN", 79, 79, 47},
        {"DARK ORCHID", 153, 50, 204},
        {"DARK SLATE BLUE", 107, 35, 142},
        {"DARK SLATE GREY", 47, 79, 79},
        {"DARK TURQUOISE", 112, 147, 219},
        {"DIM GREY", 84, 84, 84},
        {"FIREBRICK", 142, 35, 35},
        {"FOREST GREEN", 35, 142, 35},
        {"GOLD", 204, 127, 50},
        {"GOLDENROD", 219, 219, 112},
        {"GREY", 128, 128, 128},
        {"GREEN", 0, 255, 0},
        {"GREEN YELLOW", 147, 219, 112},
        {"INDIAN RED", 79, 47, 47},
        {"KHAKI", 159, 159, 95},
        {"LIGHT BLUE", 191, 216, 216},
        {"LIGHT GREY", 192, 192, 192},
        {"LIGHT STEEL BLUE", 143, 143, 188},
        {"LIME GREEN", 50, 204, 50},
        {"LIGHT MAGENTA", 255, 119, 255},
        {"MAGENTA", 255, 0, 255},
        {"MAROON", 142, 35, 107},
        {"MEDIUM AQUAMARINE", 50, 204, 153},
        {"MEDIUM GREY", 100, 100, 100},
        {"MEDIUM BLUE", 50, 50, 204},
        {"MEDIUM FOREST GREEN", 107, 142, 35},
        {"MEDIUM GOLDENROD", 234, 234, 173},
        {"MEDIUM ORCHID", 147, 112, 219},
        {"MEDIUM SEA GREEN", 66, 111, 66},
        {"MEDIUM SLATE BLUE", 127, 0, 255},
        {"MEDIUM SPRING GREEN", 127, 255, 0},
        {"MEDIUM TURQUOISE", 112, 219, 219},
        {"MEDIUM VIOLET RED", 219, 112, 147},
        {"MIDNIGHT BLUE", 47, 47, 79},
        {"NAVY", 35, 35, 142},
        {"ORANGE", 204, 50, 50},
        {"ORANGE RED", 255, 0, 127},
        {"ORCHID", 219, 112, 219},
        {"PALE GREEN", 143, 188, 143},
        {"PINK", 255, 192, 203},
        {"PLUM", 234, 173, 234},
        {"PURPLE", 176, 0, 255},
        {"RED", 255, 0, 0},
        {"SALMON", 111, 66, 66},
        {"SEA GREEN", 35, 142, 107},
        {"SIENNA", 142, 107, 35},
        {"SKY BLUE", 50, 153, 204},
        {"SLATE BLUE", 0, 127, 255},
        {"SPRING GREEN", 0, 255, 127},
        {"STEEL BLUE", 35, 107, 142},
        {"TAN", 219, 147, 112},
        {"THISTLE", 216, 191, 216},
        {"TURQUOISE", 173, 234, 234},
        {"VIOLET", 79, 47, 79},
        {"VIOLET RED", 204, 50, 153},
        {"WHEAT", 216, 216, 191},
        {"WHITE", 255, 255, 255},
        {"YELLOW", 255, 255, 0},
        {"YELLOW GREEN", 153, 204, 50}
    };

    for ( const auto& cc : wxColourTable )
    {
        (*m_map)[cc.name] = new wxColour(cc.r, cc.g, cc.b);
    }
}

// ----------------------------------------------------------------------------
// wxColourDatabase operations
// ----------------------------------------------------------------------------

void wxColourDatabase::AddColour(const std::string& name, const wxColour& colour)
{
    Initialize();

    // canonicalize the colour names before using them as keys: they should be
    // in upper case
    std::string colName = wx::utils::ToUpperCopy(name);

    // ... and we also allow both grey/gray
    std::string colNameAlt{colName};

    if ( !wx::utils::ReplaceAll(colNameAlt, "GRAY", "GREY"))
    {
        // but in this case it is not necessary so avoid extra search below
        colNameAlt.clear();
    }

    wxStringToColourHashMap::iterator it = m_map->find(colName);
    if ( it == m_map->end() && !colNameAlt.empty() )
        it = m_map->find(colNameAlt);
    if ( it != m_map->end() )
    {
        *(it->second) = colour;
    }
    else // new colour
    {
        (*m_map)[colName] = new wxColour(colour);
    }
}

wxColour wxColourDatabase::Find(const std::string& colour) const
{
    wxColourDatabase * const self = const_cast<wxColourDatabase *>(this);
    self->Initialize();

    // make the comparaison case insensitive and also match both grey and gray
    std::string colName = wx::utils::ToUpperCopy(colour);

    std::string colNameAlt{colName};

    if ( wx::utils::ReplaceAll(colNameAlt, "GRAY", "GREY") )
        colNameAlt.clear();

    wxStringToColourHashMap::iterator it = m_map->find(colName);
    if ( it == m_map->end() && !colNameAlt.empty() )
        it = m_map->find(colNameAlt);
    if ( it != m_map->end() )
        return *(it->second);

    // we did not find any result in existing colours:
    // we won't use wxString -> wxColour conversion because the
    // wxColour::Set(const wxString &) function which does that conversion
    // internally uses this function (wxColourDatabase::Find) and we want
    // to avoid infinite recursion !
    return wxNullColour;
}

std::string wxColourDatabase::FindName(const wxColour& colour) const
{
    wxColourDatabase * const self = const_cast<wxColourDatabase *>(this);
    self->Initialize();

    using iterator = wxStringToColourHashMap::iterator;

    for ( iterator it = m_map->begin(), en = m_map->end(); it != en; ++it )
    {
        if ( *(it->second) == colour )
            return it->first;
    }

    return {};
}

// ============================================================================
// stock objects
// ============================================================================

static wxStockGDI gs_wxStockGDI_instance;
wxStockGDI* wxStockGDI::ms_instance = &gs_wxStockGDI_instance;
wxObject* wxStockGDI::ms_stockObject[ITEMCOUNT];

void wxStockGDI::DeleteAll()
{
    for (unsigned i = 0; i < ITEMCOUNT; i++)
    {
        wxDELETE(ms_stockObject[i]);
    }
}

const wxBrush* wxStockGDI::GetBrush(Item item)
{
    wxBrush* brush = dynamic_cast<wxBrush*>(ms_stockObject[item]);
    if (brush == nullptr)
    {
        switch (item)
        {
        case BRUSH_BLACK:
            brush = new wxBrush(*GetColour(COLOUR_BLACK), wxBrushStyle::Solid);
            break;
        case BRUSH_BLUE:
            brush = new wxBrush(*GetColour(COLOUR_BLUE), wxBrushStyle::Solid);
            break;
        case BRUSH_CYAN:
            brush = new wxBrush(*GetColour(COLOUR_CYAN), wxBrushStyle::Solid);
            break;
        case BRUSH_GREEN:
            brush = new wxBrush(*GetColour(COLOUR_GREEN), wxBrushStyle::Solid);
            break;
        case BRUSH_YELLOW:
            brush = new wxBrush(*GetColour(COLOUR_YELLOW), wxBrushStyle::Solid);
            break;
        case BRUSH_GREY:
            brush = new wxBrush(wxColour("GREY"), wxBrushStyle::Solid);
            break;
        case BRUSH_LIGHTGREY:
            brush = new wxBrush(*GetColour(COLOUR_LIGHTGREY), wxBrushStyle::Solid);
            break;
        case BRUSH_MEDIUMGREY:
            brush = new wxBrush(wxColour("MEDIUM GREY"), wxBrushStyle::Solid);
            break;
        case BRUSH_RED:
            brush = new wxBrush(*GetColour(COLOUR_RED), wxBrushStyle::Solid);
            break;
        case BRUSH_TRANSPARENT:
            brush = new wxBrush(*GetColour(COLOUR_BLACK), wxBrushStyle::Transparent);
            break;
        case BRUSH_WHITE:
            brush = new wxBrush(*GetColour(COLOUR_WHITE), wxBrushStyle::Solid);
            break;
        default:
            wxFAIL;
        }
        ms_stockObject[item] = brush;
    }
    return brush;
}

const wxColour* wxStockGDI::GetColour(Item item)
{
    wxColour* colour = dynamic_cast<wxColour*>(ms_stockObject[item]);
    if (colour == nullptr)
    {
        switch (item)
        {
        case COLOUR_BLACK:
            colour = new wxColour(0, 0, 0);
            break;
        case COLOUR_BLUE:
            colour = new wxColour(0, 0, 255);
            break;
        case COLOUR_CYAN:
            colour = new wxColour(0, 255, 255);
            break;
        case COLOUR_GREEN:
            colour = new wxColour(0, 255, 0);
            break;
        case COLOUR_YELLOW:
            colour = new wxColour(255, 255, 0);
            break;
        case COLOUR_LIGHTGREY:
            colour = new wxColour(192, 192, 192);
            break;
        case COLOUR_RED:
            colour = new wxColour(255, 0, 0);
            break;
        case COLOUR_WHITE:
            colour = new wxColour(255, 255, 255);
            break;
        default:
            wxFAIL;
        }
        ms_stockObject[item] = colour;
    }
    return colour;
}

const wxCursor* wxStockGDI::GetCursor(Item item)
{
    wxCursor* cursor = dynamic_cast<wxCursor*>(ms_stockObject[item]);
    if (cursor == nullptr)
    {
        switch (item)
        {
        case CURSOR_CROSS:
            cursor = new wxCursor(wxCURSOR_CROSS);
            break;
        case CURSOR_HOURGLASS:
            cursor = new wxCursor(wxCURSOR_WAIT);
            break;
        case CURSOR_STANDARD:
            cursor = new wxCursor(wxCURSOR_ARROW);
            break;
        default:
            wxFAIL;
        }
        ms_stockObject[item] = cursor;
    }
    return cursor;
}

const wxFont* wxStockGDI::GetFont(Item item)
{
    wxFont* font = dynamic_cast<wxFont*>(ms_stockObject[item]);
    if (font == nullptr)
    {
        switch (item)
        {
        case FONT_ITALIC:
            font = new wxFont(GetFont(FONT_NORMAL)->GetPointSize(),
                              wxFontFamily::Roman, wxFontStyle::Italic, wxFONTWEIGHT_NORMAL);
            break;
        case FONT_NORMAL:
            font = new wxFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
            break;
        case FONT_SMALL:
            font = new wxFont(GetFont(FONT_NORMAL)->GetPointSize()
                    // Using the font 2 points smaller than the normal one
                    // results in font so small as to be unreadable under MSW.
                    // We might want to actually use -1 under the other
                    // platforms too but for now be conservative and keep -2
                    // there for compatibility with the old behaviour as the
                    // small font seems to be readable enough there as it is.
#ifdef __WXMSW__
                    - 1,
#else
                    - 2,
#endif
                    wxFontFamily::Swiss, wxFontStyle::Normal, wxFONTWEIGHT_NORMAL);
            break;
        case FONT_SWISS:
            font = new wxFont(GetFont(FONT_NORMAL)->GetPointSize(),
                              wxFontFamily::Swiss, wxFontStyle::Normal, wxFONTWEIGHT_NORMAL);
            break;
        default:
            wxFAIL;
        }
        ms_stockObject[item] = font;
    }
    return font;
}

const wxPen* wxStockGDI::GetPen(Item item)
{
    wxPen* pen = dynamic_cast<wxPen*>(ms_stockObject[item]);
    if (pen == nullptr)
    {
        switch (item)
        {
        case PEN_BLACK:
            pen = new wxPen(*GetColour(COLOUR_BLACK), 1, wxPenStyle::Solid);
            break;
        case PEN_BLACKDASHED:
            pen = new wxPen(*GetColour(COLOUR_BLACK), 1, wxPenStyle::ShortDash);
            break;
        case PEN_BLUE:
            pen = new wxPen(*GetColour(COLOUR_BLUE), 1, wxPenStyle::Solid);
            break;
        case PEN_CYAN:
            pen = new wxPen(*GetColour(COLOUR_CYAN), 1, wxPenStyle::Solid);
            break;
        case PEN_GREEN:
            pen = new wxPen(*GetColour(COLOUR_GREEN), 1, wxPenStyle::Solid);
            break;
        case PEN_YELLOW:
            pen = new wxPen(*GetColour(COLOUR_YELLOW), 1, wxPenStyle::Solid);
            break;
        case PEN_GREY:
            pen = new wxPen(wxColour("GREY"), 1, wxPenStyle::Solid);
            break;
        case PEN_LIGHTGREY:
            pen = new wxPen(*GetColour(COLOUR_LIGHTGREY), 1, wxPenStyle::Solid);
            break;
        case PEN_MEDIUMGREY:
            pen = new wxPen(wxColour("MEDIUM GREY"), 1, wxPenStyle::Solid);
            break;
        case PEN_RED:
            pen = new wxPen(*GetColour(COLOUR_RED), 1, wxPenStyle::Solid);
            break;
        case PEN_TRANSPARENT:
            pen = new wxPen(*GetColour(COLOUR_BLACK), 1, wxPenStyle::Transparent);
            break;
        case PEN_WHITE:
            pen = new wxPen(*GetColour(COLOUR_WHITE), 1, wxPenStyle::Solid);
            break;
        default:
            wxFAIL;
        }
        ms_stockObject[item] = pen;
    }
    return pen;
}

void wxInitializeStockLists()
{
    wxTheColourDatabase = new wxColourDatabase;

    wxTheBrushList = new wxBrushList;
    wxThePenList = new wxPenList;
    wxTheFontList = new wxFontList;
}

void wxDeleteStockLists()
{
    wxDELETE(wxTheBrushList);
    wxDELETE(wxThePenList);
    wxDELETE(wxTheFontList);

    // wxTheColourDatabase is cleaned up by wxAppBase::CleanUp()
}

// ============================================================================
// wxTheXXXList stuff (semi-obsolete)
// ============================================================================

wxGDIObjListBase::~wxGDIObjListBase()
{
    for (wxList::compatibility_iterator node = list.GetFirst(); node; node = node->GetNext())
    {
        delete static_cast<wxObject*>(node->GetData());
    }
}

wxPen *wxPenList::FindOrCreatePen (const wxColour& colour, int width, wxPenStyle style)
{
    for ( wxList::compatibility_iterator node = list.GetFirst();
          node;
          node = node->GetNext() )
    {
        wxPen * const pen = (wxPen *) node->GetData();
        if ( pen->GetWidth () == width &&
                pen->GetStyle () == style &&
                    pen->GetColour() == colour )
            return pen;
    }

    wxPen* pen = nullptr;
    wxPen penTmp(colour, width, style);
    if (penTmp.IsOk())
    {
        pen = new wxPen(penTmp);
        list.Append(pen);
    }

    return pen;
}

wxBrush *wxBrushList::FindOrCreateBrush (const wxColour& colour, wxBrushStyle style)
{
    for ( wxList::compatibility_iterator node = list.GetFirst();
          node;
          node = node->GetNext() )
    {
        wxBrush * const brush = (wxBrush *) node->GetData ();
        if ( brush->GetStyle() == style && brush->GetColour() == colour )
            return brush;
    }

    wxBrush* brush = nullptr;
    wxBrush brushTmp(colour, style);
    if (brushTmp.IsOk())
    {
        brush = new wxBrush(brushTmp);
        list.Append(brush);
    }

    return brush;
}

wxFont *wxFontList::FindOrCreateFont(int pointSize,
                                     wxFontFamily family,
                                     wxFontStyle style,
                                     wxFontWeight weight,
                                     bool underline,
                                     const std::string& facename,
                                     wxFontEncoding encoding)
{
    // In all ports but wxOSX, the effective family of a font created using
    // wxFontFamily::Default is wxFontFamily::Swiss so this is what we need to
    // use for comparison.
    //
    // In wxOSX the original wxFontFamily::Default seems to be kept and it uses
    // a different font than wxFontFamily::Swiss anyhow so we just preserve it.
#ifndef __WXOSX__
    if ( family == wxFontFamily::Default )
        family = wxFontFamily::Swiss;
#endif // !__WXOSX__

    // In wxMSW, creating a font with wxFontStyle::Slant creates the same font
    // as wxFontStyle::Italic and its GetStyle() returns the latter, so we must
    // account for it here. Notice that wxOSX also uses the same native font
    // for these styles, but wxFont::GetStyle() in it still returns different
    // values depending on how the font was created, so there is inconsistency
    // between ports here which it would be nice to fix in one way or another
    // (wxGTK supports both as separate styles, so it doesn't suffer from it).
 #ifdef __WXMSW__
    if ( style == wxFontStyle::Slant )
        style = wxFontStyle::Italic;
 #endif // __WXMSW__

    wxFont *font;
    wxList::compatibility_iterator node;
    for (node = list.GetFirst(); node; node = node->GetNext())
    {
        font = (wxFont *)node->GetData();
        if (
             font->GetPointSize () == pointSize &&
             font->GetStyle () == style &&
             font->GetWeight () == weight &&
             font->GetUnderlined () == underline )
        {
            // empty facename matches anything at all: this is bad because
            // depending on which fonts are already created, we might get back
            // a different font if we create it with empty facename, but it is
            // still better than never matching anything in the cache at all
            // in this case
            bool same;
            const std::string fontFaceName = font->GetFaceName();

            if (facename.empty() || fontFaceName.empty())
                same = font->GetFamily() == family;
            else
                same = fontFaceName == facename;

            if ( same && (encoding != wxFONTENCODING_DEFAULT) )
            {
                // have to match the encoding too
                same = font->GetEncoding() == encoding;
            }

            if ( same )
            {
                return font;
            }
        }
    }

    // font not found, create the new one
    font = nullptr;
    wxFont fontTmp(pointSize, family, style, weight, underline, facename, encoding);
    if (fontTmp.IsOk())
    {
        font = new wxFont(fontTmp);
        list.Append(font);
    }

    return font;
}

void wxClientDisplayRect(int *x, int *y, int *width, int *height)
{
    const wxRect rect = wxGetClientDisplayRect();
    if ( x )
        *x = rect.x;
    if ( y )
        *y = rect.y;
    if ( width )
        *width = rect.width;
    if ( height )
        *height = rect.height;
}

wxRect wxGetClientDisplayRect()
{
    return wxDisplay().GetClientArea();
}

void wxDisplaySizeMM(int *width, int *height)
{
    const wxSize size = wxGetDisplaySizeMM();
    if ( width )
        *width = size.x;
    if ( height )
        *height = size.y;
}

wxSize wxGetDisplaySizeMM()
{
    const wxSize ppi = wxGetDisplayPPI();
    if ( !ppi.x || !ppi.y )
        return {0, 0};

    const auto pixels = wxDisplay().GetGeometry().GetSize();
    return {std::lround(pixels.x * inches2mm / ppi.x),
            std::lround(pixels.y * inches2mm / ppi.y)};
}

wxSize wxGetDisplayPPI()
{
    return wxDisplay().GetPPI();
}

wxResourceCache::~wxResourceCache ()
{
    wxList::compatibility_iterator node = GetFirst ();
    while (node) {
        wxObject *item = (wxObject *)node->GetData();
        delete item;

        node = node->GetNext ();
    }
}
