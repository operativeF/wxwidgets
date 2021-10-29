/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gdicmn.h
// Purpose:     Common GDI classes, types and declarations
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GDICMNH__
#define _WX_GDICMNH__

// ---------------------------------------------------------------------------
// headers
// ---------------------------------------------------------------------------

#include "wx/defs.h"

#include "wx/list.h"
#include "wx/geometry/point.h"
#include "wx/geometry/rect.h"
#include "wx/geometry/size.h"
#include "wx/string.h"
#include "wx/fontenc.h"
#include "wx/hashmap.h"
#include "wx/math.h"
#include "wx/bitflags.h"

#include <cstdint>

// ---------------------------------------------------------------------------
// forward declarations
// ---------------------------------------------------------------------------

class  WXDLLIMPEXP_FWD_CORE wxBitmap;
class  WXDLLIMPEXP_FWD_CORE wxBrush;
class  WXDLLIMPEXP_FWD_CORE wxColour;
class  WXDLLIMPEXP_FWD_CORE wxCursor;
class  WXDLLIMPEXP_FWD_CORE wxFont;
class  WXDLLIMPEXP_FWD_CORE wxIcon;
class  WXDLLIMPEXP_FWD_CORE wxPalette;
class  WXDLLIMPEXP_FWD_CORE wxPen;
class  WXDLLIMPEXP_FWD_CORE wxRegion;
class  WXDLLIMPEXP_FWD_CORE wxIconBundle;

// ---------------------------------------------------------------------------
// constants
// ---------------------------------------------------------------------------

// Bitmap flags
enum class wxBitmapType
{
    Invalid,          // should be == 0 for compatibility!
    BMP,
    BMP_Resource,
    resource = BMP_Resource,
    ICO,
    ICO_Resource,
    CUR,
    CUR_Resource,
    XBM,
    XBM_Data,
    XPM,
    XPM_Data,
    TIFF,
    TIF = TIFF,
    TIFF_Resource,
    TIF_Resource = TIFF_Resource,
    GIF,
    GIF_Resource,
    PNG,
    PNG_Resource,
    JPEG,
    JPEG_Resource,
    PNM,
    PNM_Resource,
    PCX,
    PCX_Resource,
    PICT,
    PICT_Resource,
    ICON,
    ICON_Resource,
    ANI,
    IFF,
    TGA,
    MACCURSOR,
    MACCURSOR_Resource,

    Max,
    Any = 50
};

//  Polygon filling mode
enum class wxPolygonFillMode
{
    OddEven,
    WindingRule
};

// Standard cursors
enum wxStockCursor
{
    wxCURSOR_NONE,          // should be 0
    wxCURSOR_ARROW,
    wxCURSOR_RIGHT_ARROW,
    wxCURSOR_BULLSEYE,
    wxCURSOR_CHAR,
    wxCURSOR_CROSS,
    wxCURSOR_HAND,
    wxCURSOR_IBEAM,
    wxCURSOR_LEFT_BUTTON,
    wxCURSOR_MAGNIFIER,
    wxCURSOR_MIDDLE_BUTTON,
    wxCURSOR_NO_ENTRY,
    wxCURSOR_PAINT_BRUSH,
    wxCURSOR_PENCIL,
    wxCURSOR_POINT_LEFT,
    wxCURSOR_POINT_RIGHT,
    wxCURSOR_QUESTION_ARROW,
    wxCURSOR_RIGHT_BUTTON,
    wxCURSOR_SIZENESW,
    wxCURSOR_SIZENS,
    wxCURSOR_SIZENWSE,
    wxCURSOR_SIZEWE,
    wxCURSOR_SIZING,
    wxCURSOR_SPRAYCAN,
    wxCURSOR_WAIT,
    wxCURSOR_WATCH,
    wxCURSOR_BLANK,
#ifdef __WXGTK__
    wxCURSOR_DEFAULT, // standard X11 cursor
#endif
#ifdef __WXMAC__
    wxCURSOR_COPY_ARROW , // MacOS Theme Plus arrow
#endif
#ifdef __X__
    // Not yet implemented for Windows
    wxCURSOR_CROSS_REVERSE,
    wxCURSOR_DOUBLE_ARROW,
    wxCURSOR_BASED_ARROW_UP,
    wxCURSOR_BASED_ARROW_DOWN,
#endif // X11
    wxCURSOR_ARROWWAIT,
#ifdef __WXMAC__
    wxCURSOR_OPEN_HAND,
    wxCURSOR_CLOSED_HAND,
#endif

    wxCURSOR_MAX
};

#ifndef __WXGTK__
    constexpr unsigned int wxCURSOR_DEFAULT = wxCURSOR_ARROW;
#endif

#ifndef __WXMAC__
    // TODO CS supply openhand and closedhand cursors
    constexpr unsigned int wxCURSOR_OPEN_HAND      = wxCURSOR_HAND;
    constexpr unsigned int wxCURSOR_CLOSED_HAND    = wxCURSOR_HAND;
#endif

// ----------------------------------------------------------------------------
// Ellipsize() constants
// ----------------------------------------------------------------------------

enum class wxEllipsizeFlags
{
    None,
    ProcessMnemonics,
    ExpandTabs,
    Default,
    _max_size
};

using EllipsizeFlags = InclBitfield<wxEllipsizeFlags>;

// NB: Don't change the order of these values, they're the same as in
//     PangoEllipsizeMode enum.
enum class wxEllipsizeMode
{
    None,
    Start,
    Middle,
    End
};

// ---------------------------------------------------------------------------
// macros
// ---------------------------------------------------------------------------

#if defined(WX_WINDOWS) && wxUSE_WXDIB
    #define wxHAS_IMAGES_IN_RESOURCES
#endif

/* Useful macro for creating icons portably, for example:

    wxIcon *icon = new wxICON(sample);

  expands into:

    wxIcon *icon = new wxIcon("sample");      // On Windows
    wxIcon *icon = new wxIcon(sample_xpm);    // On wxGTK/Linux
 */

#ifdef wxHAS_IMAGES_IN_RESOURCES
    // Load from a resource
    #define wxICON(X) wxIcon(wxT(#X))
#elif defined(__WXDFB__)
    // Initialize from an included XPM
    #define wxICON(X) wxIcon( X##_xpm )
#elif defined(__WXGTK__)
    // Initialize from an included XPM
    #define wxICON(X) wxIcon( X##_xpm )
#elif defined(__WXMAC__)
    // Initialize from an included XPM
    #define wxICON(X) wxIcon( X##_xpm )
#elif defined(__WXMOTIF__)
    // Initialize from an included XPM
    #define wxICON(X) wxIcon( X##_xpm )
#elif defined(__WXX11__)
    // Initialize from an included XPM
    #define wxICON(X) wxIcon( X##_xpm )
#elif defined(__WXQT__)
    // Initialize from an included XPM
    #define wxICON(X) wxIcon( X##_xpm )
#else
    // This will usually mean something on any platform
    #define wxICON(X) wxIcon(wxT(#X))
#endif // platform

/* Another macro: this one is for portable creation of bitmaps. We assume that
   under Unix bitmaps live in XPMs and under Windows they're in resources.
 */

#if defined(WX_WINDOWS) && wxUSE_WXDIB
    #define wxBITMAP(name) wxBitmap(wxT(#name), wxBitmapType::BMP_Resource)
#elif defined(__WXGTK__)   || \
      defined(__WXMOTIF__) || \
      defined(__WXX11__)   || \
      defined(__WXMAC__)   || \
      defined(__WXDFB__)
    // Initialize from an included XPM
    #define wxBITMAP(name) wxBitmap(name##_xpm)
#else // other platforms
    #define wxBITMAP(name) wxBitmap(name##_xpm, wxBitmapType::XPM)
#endif // platform

// Macro for creating wxBitmap from in-memory PNG data.
//
// It reads PNG data from name_png static byte arrays that can be created using
// e.g. misc/scripts/png2c.py.
//
// This macro exists mostly as a helper for wxBITMAP_PNG() below but also
// because it's slightly more convenient to use than NewFromPNGData() directly.
#define wxBITMAP_PNG_FROM_DATA(name) \
    wxBitmap::NewFromPNGData(name##_png, WXSIZEOF(name##_png))

// Similar to wxBITMAP but used for the bitmaps in PNG format.
//
// Under Windows they should be embedded into the resource file using RT_RCDATA
// resource type and under OS X the PNG file with the specified name must be
// available in the resource subdirectory of the bundle. Elsewhere, this is
// exactly the same thing as wxBITMAP_PNG_FROM_DATA() described above.
#if (defined(WX_WINDOWS) && wxUSE_WXDIB) || defined(__WXOSX__)
    #define wxBITMAP_PNG(name) wxBitmap(wxS(#name), wxBITMAP_TYPE_PNG_RESOURCE)
#else
    #define wxBITMAP_PNG(name) wxBITMAP_PNG_FROM_DATA(name)
#endif

// ---------------------------------------------------------------------------
// Management of pens, brushes and fonts
// ---------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxGDIObjListBase {
public:
    ~wxGDIObjListBase();

protected:
    wxList list;
};

WX_DECLARE_STRING_HASH_MAP(wxColour*, wxStringToColourHashMap);

class WXDLLIMPEXP_CORE wxColourDatabase
{
public:
    ~wxColourDatabase();

    // find colour by name or name for the given colour
    wxColour Find(const wxString& name) const;
    wxString FindName(const wxColour& colour) const;

    // add a new colour to the database
    void AddColour(const wxString& name, const wxColour& colour);

private:
    // load the database with the built in colour values when called for the
    // first time, do nothing after this
    void Initialize();

    wxStringToColourHashMap *m_map{nullptr};
};

class WXDLLIMPEXP_CORE wxResourceCache: public wxList
{
public:
    ~wxResourceCache();
};

// ---------------------------------------------------------------------------
// global variables
// ---------------------------------------------------------------------------


/* Stock objects

  wxStockGDI creates the stock GDI objects on demand.  Pointers to the
  created objects are stored in the ms_stockObject array, which is indexed
  by the Item enum values.  Platorm-specific fonts can be created by
  implementing a derived class with an override for the GetFont function.
  wxStockGDI operates as a singleton, accessed through the ms_instance
  pointer.  By default this pointer is set to an instance of wxStockGDI.
  A derived class must arrange to set this pointer to an instance of itself.
*/
class WXDLLIMPEXP_CORE wxStockGDI
{
public:
    enum Item {
        BRUSH_BLACK,
        BRUSH_BLUE,
        BRUSH_CYAN,
        BRUSH_GREEN,
        BRUSH_YELLOW,
        BRUSH_GREY,
        BRUSH_LIGHTGREY,
        BRUSH_MEDIUMGREY,
        BRUSH_RED,
        BRUSH_TRANSPARENT,
        BRUSH_WHITE,
        COLOUR_BLACK,
        COLOUR_BLUE,
        COLOUR_CYAN,
        COLOUR_GREEN,
        COLOUR_YELLOW,
        COLOUR_LIGHTGREY,
        COLOUR_RED,
        COLOUR_WHITE,
        CURSOR_CROSS,
        CURSOR_HOURGLASS,
        CURSOR_STANDARD,
        FONT_ITALIC,
        FONT_NORMAL,
        FONT_SMALL,
        FONT_SWISS,
        PEN_BLACK,
        PEN_BLACKDASHED,
        PEN_BLUE,
        PEN_CYAN,
        PEN_GREEN,
        PEN_YELLOW,
        PEN_GREY,
        PEN_LIGHTGREY,
        PEN_MEDIUMGREY,
        PEN_RED,
        PEN_TRANSPARENT,
        PEN_WHITE,
        ITEMCOUNT
    };

    virtual ~wxStockGDI() = default;

    wxStockGDI& operator=(wxStockGDI&&) = delete;

    static void DeleteAll();

    static wxStockGDI& instance() { return *ms_instance; }

    static const wxBrush* GetBrush(Item item);
    static const wxColour* GetColour(Item item);
    static const wxCursor* GetCursor(Item item);
    // Can be overridden by platform-specific derived classes
    virtual const wxFont* GetFont(Item item);
    static const wxPen* GetPen(Item item);

protected:
    static wxStockGDI* ms_instance;

    static wxObject* ms_stockObject[ITEMCOUNT];
};

#define wxITALIC_FONT  wxStockGDI::instance().GetFont(wxStockGDI::FONT_ITALIC)
#define wxNORMAL_FONT  wxStockGDI::instance().GetFont(wxStockGDI::FONT_NORMAL)
#define wxSMALL_FONT   wxStockGDI::instance().GetFont(wxStockGDI::FONT_SMALL)
#define wxSWISS_FONT   wxStockGDI::instance().GetFont(wxStockGDI::FONT_SWISS)

#define wxBLACK_DASHED_PEN  wxStockGDI::GetPen(wxStockGDI::PEN_BLACKDASHED)
#define wxBLACK_PEN         wxStockGDI::GetPen(wxStockGDI::PEN_BLACK)
#define wxBLUE_PEN          wxStockGDI::GetPen(wxStockGDI::PEN_BLUE)
#define wxCYAN_PEN          wxStockGDI::GetPen(wxStockGDI::PEN_CYAN)
#define wxGREEN_PEN         wxStockGDI::GetPen(wxStockGDI::PEN_GREEN)
#define wxYELLOW_PEN        wxStockGDI::GetPen(wxStockGDI::PEN_YELLOW)
#define wxGREY_PEN          wxStockGDI::GetPen(wxStockGDI::PEN_GREY)
#define wxLIGHT_GREY_PEN    wxStockGDI::GetPen(wxStockGDI::PEN_LIGHTGREY)
#define wxMEDIUM_GREY_PEN   wxStockGDI::GetPen(wxStockGDI::PEN_MEDIUMGREY)
#define wxRED_PEN           wxStockGDI::GetPen(wxStockGDI::PEN_RED)
#define wxTRANSPARENT_PEN   wxStockGDI::GetPen(wxStockGDI::PEN_TRANSPARENT)
#define wxWHITE_PEN         wxStockGDI::GetPen(wxStockGDI::PEN_WHITE)

#define wxBLACK_BRUSH        wxStockGDI::GetBrush(wxStockGDI::BRUSH_BLACK)
#define wxBLUE_BRUSH         wxStockGDI::GetBrush(wxStockGDI::BRUSH_BLUE)
#define wxCYAN_BRUSH         wxStockGDI::GetBrush(wxStockGDI::BRUSH_CYAN)
#define wxGREEN_BRUSH        wxStockGDI::GetBrush(wxStockGDI::BRUSH_GREEN)
#define wxYELLOW_BRUSH       wxStockGDI::GetBrush(wxStockGDI::BRUSH_YELLOW)
#define wxGREY_BRUSH         wxStockGDI::GetBrush(wxStockGDI::BRUSH_GREY)
#define wxLIGHT_GREY_BRUSH   wxStockGDI::GetBrush(wxStockGDI::BRUSH_LIGHTGREY)
#define wxMEDIUM_GREY_BRUSH  wxStockGDI::GetBrush(wxStockGDI::BRUSH_MEDIUMGREY)
#define wxRED_BRUSH          wxStockGDI::GetBrush(wxStockGDI::BRUSH_RED)
#define wxTRANSPARENT_BRUSH  wxStockGDI::GetBrush(wxStockGDI::BRUSH_TRANSPARENT)
#define wxWHITE_BRUSH        wxStockGDI::GetBrush(wxStockGDI::BRUSH_WHITE)

#define wxBLACK       wxStockGDI::GetColour(wxStockGDI::COLOUR_BLACK)
#define wxBLUE        wxStockGDI::GetColour(wxStockGDI::COLOUR_BLUE)
#define wxCYAN        wxStockGDI::GetColour(wxStockGDI::COLOUR_CYAN)
#define wxGREEN       wxStockGDI::GetColour(wxStockGDI::COLOUR_GREEN)
#define wxYELLOW      wxStockGDI::GetColour(wxStockGDI::COLOUR_YELLOW)
#define wxLIGHT_GREY  wxStockGDI::GetColour(wxStockGDI::COLOUR_LIGHTGREY)
#define wxRED         wxStockGDI::GetColour(wxStockGDI::COLOUR_RED)
#define wxWHITE       wxStockGDI::GetColour(wxStockGDI::COLOUR_WHITE)

#define wxCROSS_CURSOR      wxStockGDI::GetCursor(wxStockGDI::CURSOR_CROSS)
#define wxHOURGLASS_CURSOR  wxStockGDI::GetCursor(wxStockGDI::CURSOR_HOURGLASS)
#define wxSTANDARD_CURSOR   wxStockGDI::GetCursor(wxStockGDI::CURSOR_STANDARD)

// 'Null' objects
extern WXDLLIMPEXP_DATA_CORE(wxBitmap)     wxNullBitmap;
extern WXDLLIMPEXP_DATA_CORE(wxIcon)       wxNullIcon;
extern WXDLLIMPEXP_DATA_CORE(wxCursor)     wxNullCursor;
extern WXDLLIMPEXP_DATA_CORE(wxPen)        wxNullPen;
extern WXDLLIMPEXP_DATA_CORE(wxBrush)      wxNullBrush;
extern WXDLLIMPEXP_DATA_CORE(wxPalette)    wxNullPalette;
extern WXDLLIMPEXP_DATA_CORE(wxFont)       wxNullFont;
extern WXDLLIMPEXP_DATA_CORE(wxColour)     wxNullColour;
extern WXDLLIMPEXP_DATA_CORE(wxIconBundle) wxNullIconBundle;

extern WXDLLIMPEXP_DATA_CORE(wxColourDatabase*)  wxTheColourDatabase;

constexpr char wxPanelNameStr[] = "panel";

constexpr wxSize wxDefaultSize{wxDefaultCoord, wxDefaultCoord};
constexpr wxPoint wxDefaultPosition{wxDefaultCoord, wxDefaultCoord};

// ---------------------------------------------------------------------------
// global functions
// ---------------------------------------------------------------------------

// resource management
extern void WXDLLIMPEXP_CORE wxInitializeStockLists();
extern void WXDLLIMPEXP_CORE wxDeleteStockLists();

// Note: all the display-related functions here exist for compatibility only,
// please use wxDisplay class in the new code

// is the display colour (or monochrome)?
extern bool WXDLLIMPEXP_CORE wxColourDisplay();

// Returns depth of screen
extern int WXDLLIMPEXP_CORE wxDisplayDepth();
#define wxGetDisplayDepth wxDisplayDepth

// get the display size
extern void WXDLLIMPEXP_CORE wxDisplaySize(int *width, int *height);
extern wxSize WXDLLIMPEXP_CORE wxGetDisplaySize();
extern void WXDLLIMPEXP_CORE wxDisplaySizeMM(int *width, int *height);
extern wxSize WXDLLIMPEXP_CORE wxGetDisplaySizeMM();
extern wxSize WXDLLIMPEXP_CORE wxGetDisplayPPI();

// Get position and size of the display workarea
extern void WXDLLIMPEXP_CORE wxClientDisplayRect(int *x, int *y, int *width, int *height);
extern wxRect WXDLLIMPEXP_CORE wxGetClientDisplayRect();

// set global cursor
extern void WXDLLIMPEXP_CORE wxSetCursor(const wxCursor& cursor);

#endif
    // _WX_GDICMNH__
