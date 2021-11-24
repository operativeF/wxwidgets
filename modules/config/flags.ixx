// A useful, if temporary, way of getting all the enums out of defs.h 
// In the future, this will be broken up such that the flags will be moved
// to a more appropriate place or made more specific to a certain class.
export module WX.Cfg.Flags;

export
{

/*  ---------------------------------------------------------------------------- */
/*  Geometric flags */
/*  ---------------------------------------------------------------------------- */

/*  centering into frame rather than screen (obsolete) */
inline constexpr auto wxCENTER_FRAME          = 0x0000;
/*  centre on screen rather than parent */
inline constexpr auto wxCENTRE_ON_SCREEN      = 0x0002;
inline constexpr auto wxCENTER_ON_SCREEN      = wxCENTRE_ON_SCREEN;

/*  ---------------------------------------------------------------------------- */
/*  Window style flags */
/*  ---------------------------------------------------------------------------- */

/*
 * Values are chosen so they can be |'ed in a bit list.
 * Some styles are used across more than one group,
 * so the values mustn't clash with others in the group.
 * Otherwise, numbers can be reused across groups.
 */

/*
    Summary of the bits used by various styles.

    High word, containing styles which can be used with many windows:

    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  \_ wxFULL_REPAINT_ON_RESIZE
      |  |  |  |  |  |  |  |  |  |  |  |  |  |  \____ wxPOPUP_WINDOW
      |  |  |  |  |  |  |  |  |  |  |  |  |  \_______ wxWANTS_CHARS
      |  |  |  |  |  |  |  |  |  |  |  |  \__________ wxTAB_TRAVERSAL
      |  |  |  |  |  |  |  |  |  |  |  \_____________ wxTRANSPARENT_WINDOW
      |  |  |  |  |  |  |  |  |  |  \________________ wxBORDER_NONE
      |  |  |  |  |  |  |  |  |  \___________________ wxCLIP_CHILDREN
      |  |  |  |  |  |  |  |  \______________________ wxALWAYS_SHOW_SB
      |  |  |  |  |  |  |  \_________________________ wxBORDER_STATIC
      |  |  |  |  |  |  \____________________________ wxBORDER_SIMPLE
      |  |  |  |  |  \_______________________________ wxBORDER_RAISED
      |  |  |  |  \__________________________________ wxBORDER_SUNKEN
      |  |  |  \_____________________________________ wxBORDER_{DOUBLE,THEME}
      |  |  \________________________________________ wxCAPTION/wxCLIP_SIBLINGS
      |  \___________________________________________ wxHSCROLL
      \______________________________________________ wxVSCROLL


    Low word style bits is class-specific meaning that the same bit can have
    different meanings for different controls (e.g. 0x10 is wxCB_READONLY
    meaning that the control can't be modified for wxComboBox but wxLB_SORT
    meaning that the control should be kept sorted for wxListBox, while
    wxLB_SORT has a different value -- and this is just fine).
 */

/*
 * Window (Frame/dialog/subwindow/panel item) style flags
 */

inline constexpr auto wxVSCROLL               = 0x80000000;
inline constexpr auto wxHSCROLL               = 0x40000000;
inline constexpr auto wxCAPTION               = 0x20000000;

/*  wxALWAYS_SHOW_SB: instead of hiding the scrollbar when it is not needed, */
/*  disable it - but still show (see also wxLB_ALWAYS_SB style) */
/*  */
/*  NB: as this style is only supported by wxUniversal and wxMSW so far */
inline constexpr auto wxALWAYS_SHOW_SB        = 0x00800000;

/*  Clip children when painting, which reduces flicker in e.g. frames and */
/*  splitter windows, but can't be used in a panel where a static box must be */
/*  'transparent' (panel paints the background for it) */
inline constexpr auto wxCLIP_CHILDREN         = 0x00400000;

/*  Note we're reusing the wxCAPTION style because we won't need captions */
/*  for subwindows/controls */
inline constexpr auto wxCLIP_SIBLINGS         = 0x20000000;

inline constexpr auto wxTRANSPARENT_WINDOW    = 0x00100000;

/*  Add this style to a panel to get tab traversal working outside of dialogs */
/*  (on by default for wxPanel, wxDialog, wxScrolledWindow) */
inline constexpr auto wxTAB_TRAVERSAL         = 0x00080000;

/*  Add this style if the control wants to get all keyboard messages (under */
/*  Windows, it won't normally get the dialog navigation key events) */
inline constexpr auto wxWANTS_CHARS           = 0x00040000;

/*  Make window retained (Motif only, see src/generic/scrolwing.cpp)
 *  This is non-zero only under wxMotif, to avoid a clash with wxPOPUP_WINDOW
 *  on other platforms
 */

#ifdef __WXMOTIF__
inline constexpr auto wxRETAINED              = 0x00020000;
#else
inline constexpr auto wxRETAINED              = 0x00000000;
#endif
inline constexpr auto wxBACKINGSTORE          = wxRETAINED;

/*  set this flag to create a special popup window: it will be always shown on */
/*  top of other windows, will capture the mouse and will be dismissed when the */
/*  mouse is clicked outside of it or if it loses focus in any other way */
inline constexpr auto wxPOPUP_WINDOW          = 0x00020000;

/*  force a full repaint when the window is resized (instead of repainting just */
/*  the invalidated area) */
inline constexpr auto wxFULL_REPAINT_ON_RESIZE = 0x00010000;

/*  obsolete: now this is the default behaviour */
/*  */
/*  don't invalidate the whole window (resulting in a PAINT event) when the */
/*  window is resized (currently, makes sense for wxMSW only) */
inline constexpr auto wxNO_FULL_REPAINT_ON_RESIZE = 0;

/*
 * Extra window style flags (use wxWS_EX prefix to make it clear that they
 * should be passed to wxWindow::SetExtraStyle(), not SetWindowStyle())
 */

/* This flag is obsolete as recursive validation is now the default (and only
 * possible) behaviour. Simply don't use it any more in the new code. */
inline constexpr auto wxWS_EX_VALIDATE_RECURSIVELY    = 0x00000000; /* used to be 1 */

/*  wxCommandEvents and the objects of the derived classes are forwarded to the */
/*  parent window and so on recursively by default. Using this flag for the */
/*  given window allows to block this propagation at this window, i.e. prevent */
/*  the events from being propagated further upwards. The dialogs have this */
/*  flag on by default. */
inline constexpr auto wxWS_EX_BLOCK_EVENTS            = 0x00000002;

/*  don't use this window as an implicit parent for the other windows: this must */
/*  be used with transient windows as otherwise there is the risk of creating a */
/*  dialog/frame with this window as a parent which would lead to a crash if the */
/*  parent is destroyed before the child */
inline constexpr auto wxWS_EX_TRANSIENT               = 0x00000004;

/*  don't paint the window background, we'll assume it will */
/*  be done by a theming engine. This is not yet used but could */
/*  possibly be made to work in the future, at least on Windows */
inline constexpr auto wxWS_EX_THEMED_BACKGROUND       = 0x00000008;

/*  this window should always process idle events */
inline constexpr auto wxWS_EX_PROCESS_IDLE            = 0x00000010;

/*  Use this style to add a context-sensitive help to the window (currently for */
/*  Win32 only and it doesn't work if wxMINIMIZE_BOX or wxMAXIMIZE_BOX are used) */
inline constexpr auto wxWS_EX_CONTEXTHELP             = 0x00000080;

/* synonyms for wxWS_EX_CONTEXTHELP for compatibility */
inline constexpr auto wxFRAME_EX_CONTEXTHELP          = wxWS_EX_CONTEXTHELP;
inline constexpr auto wxDIALOG_EX_CONTEXTHELP         = wxWS_EX_CONTEXTHELP;

/*  Create a window which is attachable to another top level window */
inline constexpr auto wxFRAME_DRAWER          = 0x0020;

/*
 * MDI parent frame style flags
 * Can overlap with some of the above.
 */

inline constexpr auto wxFRAME_NO_WINDOW_MENU  = 0x0100;

/*
 * wxMenuBar style flags
 */
/*  use native docking */
inline constexpr auto wxMB_DOCKABLE       = 0x0001;

/*
 * wxMenu style flags
 */
inline constexpr auto wxMENU_TEAROFF      = 0x0001;

/*
 * Apply to all panel items
 */
inline constexpr auto wxCOLOURED          = 0x0800;
inline constexpr auto wxFIXED_LENGTH      = 0x0400;

/*  wxLB_OWNERDRAW is Windows-only */
inline constexpr auto wxLB_NEEDED_SB      = 0x0000;
inline constexpr auto wxLB_OWNERDRAW      = 0x0100;
inline constexpr auto wxLB_ALWAYS_SB      = 0x0200;
inline constexpr auto wxLB_NO_SB          = 0x0400;
inline constexpr auto wxLB_HSCROLL        = wxHSCROLL;
/*  always show an entire number of rows */
inline constexpr auto wxLB_INT_HEIGHT     = 0x0800;

/*
 * wxComboBox style flags
 */
inline constexpr auto wxCB_SIMPLE         = 0x0004;
inline constexpr auto wxCB_SORT           = 0x0008;
inline constexpr auto wxCB_READONLY       = 0x0010;
inline constexpr auto wxCB_DROPDOWN       = 0x0020;

// Border flags: the values are chosen for backwards compatibility
enum wxBorder
{
    // This is different from wxBORDER_NONE as by default the controls do have
    // border
    wxBORDER_DEFAULT = 0,

    wxBORDER_NONE   = 0x00200000,
    wxBORDER_STATIC = 0x01000000,
    wxBORDER_SIMPLE = 0x02000000,
    wxBORDER_RAISED = 0x04000000,
    wxBORDER_SUNKEN = 0x08000000,
    wxBORDER_DOUBLE = 0x10000000, /* deprecated */
    wxBORDER_THEME  = wxBORDER_DOUBLE,

    // A mask to extract border style from the combination of flags
    wxBORDER_MASK   = 0x1f200000
};

/*  New styles (border styles are now in their own enum) */
inline constexpr auto wxDOUBLE_BORDER         = wxBORDER_DOUBLE;
inline constexpr auto wxSUNKEN_BORDER         = wxBORDER_SUNKEN;
inline constexpr auto wxRAISED_BORDER         = wxBORDER_RAISED;
inline constexpr auto wxBORDER                = wxBORDER_SIMPLE;
inline constexpr auto wxSIMPLE_BORDER         = wxBORDER_SIMPLE;
inline constexpr auto wxSTATIC_BORDER         = wxBORDER_STATIC;
inline constexpr auto wxNO_BORDER             = wxBORDER_NONE;

enum wxOrientation
{
    /* don't change the values of these elements, they are used elsewhere */
    wxHORIZONTAL              = 0x0004,
    wxVERTICAL                = 0x0008,

    wxBOTH                    = wxVERTICAL | wxHORIZONTAL,

    /*  a mask to extract orientation from the combination of flags */
    wxORIENTATION_MASK        = wxBOTH
};

enum wxDirection
{
    wxLEFT                    = 0x0010,
    wxRIGHT                   = 0x0020,
    wxUP                      = 0x0040,
    wxDOWN                    = 0x0080,

    wxTOP                     = wxUP,
    wxBOTTOM                  = wxDOWN,

    wxNORTH                   = wxUP,
    wxSOUTH                   = wxDOWN,
    wxWEST                    = wxLEFT,
    wxEAST                    = wxRIGHT,

    wxALL                     = (wxUP | wxDOWN | wxRIGHT | wxLEFT),

    /*  a mask to extract direction from the combination of flags */
    wxDIRECTION_MASK           = wxALL
};

enum wxAlignment
{
    /*
        0 is a valid wxAlignment value (both wxALIGN_LEFT and wxALIGN_TOP
        use it) so define a symbolic name for an invalid alignment value
        which can be assumed to be different from anything else
     */
    wxALIGN_INVALID           = -1,

    wxALIGN_NOT               = 0x0000,
    wxALIGN_CENTER_HORIZONTAL = 0x0100,
    wxALIGN_CENTRE_HORIZONTAL = wxALIGN_CENTER_HORIZONTAL,
    wxALIGN_LEFT              = wxALIGN_NOT,
    wxALIGN_TOP               = wxALIGN_NOT,
    wxALIGN_RIGHT             = 0x0200,
    wxALIGN_BOTTOM            = 0x0400,
    wxALIGN_CENTER_VERTICAL   = 0x0800,
    wxALIGN_CENTRE_VERTICAL   = wxALIGN_CENTER_VERTICAL,

    wxALIGN_CENTER            = (wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL),
    wxALIGN_CENTRE            = wxALIGN_CENTER,

    /*  a mask to extract alignment from the combination of flags */
    wxALIGN_MASK              = 0x0f00
};

enum wxGeometryCentre
{
    wxCENTRE                  = 0x0001,
    wxCENTER                  = wxCENTRE
};


/*
 * wxRadioBox style flags
 * These styles are not used in any port.
 */
inline constexpr auto wxRA_LEFTTORIGHT    = 0x0001;
inline constexpr auto wxRA_TOPTOBOTTOM    = 0x0002;

/*  New, more intuitive names to specify majorDim argument */
inline constexpr auto wxRA_SPECIFY_COLS   = wxHORIZONTAL;
inline constexpr auto wxRA_SPECIFY_ROWS   = wxVERTICAL;

/*  Old names for compatibility */
inline constexpr auto wxRA_HORIZONTAL     = wxHORIZONTAL;
inline constexpr auto wxRA_VERTICAL       = wxVERTICAL;

/*
 * wxScrollBar flags
 */
inline constexpr auto wxSB_HORIZONTAL      = wxHORIZONTAL;
inline constexpr auto wxSB_VERTICAL        = wxVERTICAL;

/* This makes it easier to specify a 'normal' border for a control */
inline constexpr auto wxDEFAULT_CONTROL_BORDER = wxBORDER_SUNKEN;


/* A mask which can be used to filter (out) all wxWindow-specific styles.
 */
inline constexpr auto wxWINDOW_STYLE_MASK =
    (wxVSCROLL|wxHSCROLL|wxBORDER_MASK|wxALWAYS_SHOW_SB|wxCLIP_CHILDREN|
     wxCLIP_SIBLINGS|wxTRANSPARENT_WINDOW|wxTAB_TRAVERSAL|wxWANTS_CHARS|
     wxRETAINED|wxPOPUP_WINDOW|wxFULL_REPAINT_ON_RESIZE);

// In C++20 operations on the elements of different enums are deprecated and
// many compilers (clang 10+, gcc 11+, MSVS 2019) warn about combining them,
// as a lot of existing code using them does, so we provide explicit operators
// for doing this, that do the same thing as would happen without them, but
// without the warnings.

#define wxALLOW_COMBINING_ENUMS_IMPL(en1, en2)                            \
    inline int operator|(en1 v1, en2 v2)                                  \
        { return static_cast<int>(v1) | static_cast<int>(v2); }           \
    inline int operator+(en1 v1, en2 v2)                                  \
        { return static_cast<int>(v1) + static_cast<int>(v2); }

#define wxALLOW_COMBINING_ENUMS(en1, en2)                                 \
    wxALLOW_COMBINING_ENUMS_IMPL(en1, en2)                                \
    wxALLOW_COMBINING_ENUMS_IMPL(en2, en1)

/*
    Elements of these enums can be combined with each other when using
    wxSizer::Add() overload not using wxSizerFlags.
 */

wxALLOW_COMBINING_ENUMS(wxAlignment, wxDirection)
wxALLOW_COMBINING_ENUMS(wxAlignment, wxGeometryCentre)
wxALLOW_COMBINING_ENUMS(wxDirection, wxGeometryCentre)

/*
 * extended dialog specifiers. these values are stored in a different
 * flag and thus do not overlap with other style flags. note that these
 * values do not correspond to the return values of the dialogs (for
 * those values, look at the wxID_XXX defines).
 */

/*  wxCENTRE already defined as  0x00000001 */
inline constexpr auto wxYES                  = 0x00000002;
inline constexpr auto wxOK                   = 0x00000004;
inline constexpr auto wxNO                   = 0x00000008;
inline constexpr auto wxYES_NO               = (wxYES | wxNO);
inline constexpr auto wxCANCEL               = 0x00000010;
inline constexpr auto wxAPPLY                = 0x00000020;
inline constexpr auto wxCLOSE                = 0x00000040;

inline constexpr auto wxOK_DEFAULT            = 0x00000000;  /* has no effect (default) */
inline constexpr auto wxYES_DEFAULT           = 0x00000000;  /* has no effect (default) */
inline constexpr auto wxNO_DEFAULT            = 0x00000080;  /* only valid with wxYES_NO */
inline constexpr auto wxCANCEL_DEFAULT        = 0x80000000;  /* only valid with wxCANCEL */

inline constexpr auto wxICON_WARNING          = 0x00000100;
inline constexpr auto wxICON_ERROR            = 0x00000200;
inline constexpr auto wxICON_QUESTION         = 0x00000400;
inline constexpr auto wxICON_INFORMATION      = 0x00000800;
inline constexpr auto wxICON_EXCLAMATION      = wxICON_WARNING;
inline constexpr auto wxICON_HAND             = wxICON_ERROR;
inline constexpr auto wxICON_STOP             = wxICON_ERROR;
inline constexpr auto wxICON_ASTERISK         = wxICON_INFORMATION;

inline constexpr auto wxHELP                  = 0x00001000;
inline constexpr auto wxFORWARD               = 0x00002000;
inline constexpr auto wxBACKWARD              = 0x00004000;
inline constexpr auto wxRESET                 = 0x00008000;
inline constexpr auto wxMORE                  = 0x00010000;
inline constexpr auto wxSETUP                 = 0x00020000;
inline constexpr auto wxICON_NONE             = 0x00040000;
inline constexpr auto wxICON_AUTH_NEEDED      = 0x00080000;

inline constexpr auto wxICON_MASK = (wxICON_EXCLAMATION|wxICON_HAND|wxICON_QUESTION|wxICON_INFORMATION|wxICON_NONE|wxICON_AUTH_NEEDED);

} // export
