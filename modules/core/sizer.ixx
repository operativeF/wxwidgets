/////////////////////////////////////////////////////////////////////////////
// Name:        wx/sizer.h
// Purpose:     provide wxSizer class for layout
// Author:      Robert Roebling and Robin Dunn
// Modified by: Ron Lee, Vadim Zeitlin (wxSizerFlags)
// Created:
// Copyright:   (c) Robin Dunn, Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/object.h"
#include "wx/window.h"

export module WX.Core.Sizer;

import WX.Cfg.Flags;

import Utils.Geometry;

import <string>;
import <vector>;

//---------------------------------------------------------------------------
// classes
//---------------------------------------------------------------------------

export
{

#if (!defined(__WXGTK20__) || !defined(__WXMAC__))
    #define wxNEEDS_BORDER_IN_PX
#endif

// ----------------------------------------------------------------------------
// wxSizerFlags: flags used for an item in the sizer
// ----------------------------------------------------------------------------

enum wxStretch
{
    wxSTRETCH_NOT             = 0x0000,
    wxSHRINK                  = 0x1000,
    wxGROW                    = 0x2000,
    wxEXPAND                  = wxGROW,
    wxSHAPED                  = 0x4000,
    wxTILE                    = 0xc000, /* wxSHAPED | wxFIXED_MINSIZE */

    /*  a mask to extract stretch from the combination of flags */
    wxSTRETCH_MASK            = 0x7000 /* sans wxTILE */
};

/* misc. flags for wxSizer items */
enum wxSizerFlagBits
{
    wxFIXED_MINSIZE                = 0x8000,
    wxRESERVE_SPACE_EVEN_IF_HIDDEN = 0x0002,

    /*  a mask to extract wxSizerFlagBits from combination of flags */
    wxSIZER_FLAG_BITS_MASK         = 0x8002
};

class wxSizerFlags
{
public:
    // construct the flags object initialized with the given proportion (0 by
    // default)
    wxSizerFlags(int proportion = 0) : m_proportion(proportion)
    {
        m_flags = 0;
        m_borderInPixels = 0;
    }

    // setters for all sizer flags, they all return the object itself so that
    // calls to them can be chained

    wxSizerFlags& Proportion(int proportion)
    {
        m_proportion = proportion;
        return *this;
    }

    wxSizerFlags& Expand()
    {
        m_flags |= wxEXPAND;
        return *this;
    }

    // notice that Align() replaces the current alignment flags, use specific
    // methods below such as Top(), Left() &c if you want to set just the
    // vertical or horizontal alignment
    wxSizerFlags& Align(int alignment) // combination of wxAlignment values
    {
        m_flags &= ~wxALIGN_MASK;
        m_flags |= alignment;

        return *this;
    }

    // this is just a shortcut for Align()
    wxSizerFlags& Centre() { return Align(wxALIGN_CENTRE); }
    wxSizerFlags& Center() { return Centre(); }

    // but all the remaining methods turn on the corresponding alignment flag
    // without affecting the existing ones
    wxSizerFlags& CentreVertical()
    {
        m_flags = (m_flags & ~wxALIGN_BOTTOM) | wxALIGN_CENTRE_VERTICAL;
        return *this;
    }

    wxSizerFlags& CenterVertical() { return CentreVertical(); }

    wxSizerFlags& CentreHorizontal()
    {
        m_flags = (m_flags & ~wxALIGN_RIGHT) | wxALIGN_CENTRE_HORIZONTAL;
        return *this;
    }

    wxSizerFlags& CenterHorizontal() { return CentreHorizontal(); }

    wxSizerFlags& Top()
    {
        m_flags &= ~(wxALIGN_BOTTOM | wxALIGN_CENTRE_VERTICAL);
        return *this;
    }

    wxSizerFlags& Left()
    {
        m_flags &= ~(wxALIGN_RIGHT | wxALIGN_CENTRE_HORIZONTAL);
        return *this;
    }

    wxSizerFlags& Right()
    {
        m_flags = (m_flags & ~wxALIGN_CENTRE_HORIZONTAL) | wxALIGN_RIGHT;
        return *this;
    }

    wxSizerFlags& Bottom()
    {
        m_flags = (m_flags & ~wxALIGN_CENTRE_VERTICAL) | wxALIGN_BOTTOM;
        return *this;
    }


    // default border size used by Border() below
    static int GetDefaultBorder()
    {
        return std::lround(GetDefaultBorderFractional());
    }

    static float GetDefaultBorderFractional()
    {
    #ifdef __WXGTK20__
        // GNOME HIG says to use 6px as the base unit:
        // http://library.gnome.org/devel/hig-book/stable/design-window.html.en
        return 6;
    #elif defined(__WXMAC__)
        // Not sure if this is really the correct size for the border.
        return 5;
    #else
        // For the other platforms, we need to scale raw pixel values using the
        // current DPI, do it once (and cache the result) in another function.
        return DoGetDefaultBorderInPx();
    #endif
    }


    wxSizerFlags& Border(unsigned int direction, int borderInPixels)
    {
        wxCHECK_MSG( !(direction & ~wxALL), *this,
                     "direction must be a combination of wxDirection "
                     "enum values." );

        m_flags &= ~wxALL;
        m_flags |= direction;

        m_borderInPixels = borderInPixels;

        return *this;
    }

    wxSizerFlags& Border(unsigned int direction = wxALL)
    {
        return Border(direction, std::lround(GetDefaultBorderFractional()));
    }

    wxSizerFlags& DoubleBorder(unsigned int direction = wxALL)
    {
        return Border(direction, std::lround(2 * GetDefaultBorderFractional()));
    }

    wxSizerFlags& TripleBorder(unsigned int direction = wxALL)
    {
        return Border(direction, std::lround(3 * GetDefaultBorderFractional()));
    }

    wxSizerFlags& HorzBorder()
    {
        return Border(wxDirection::wxLEFT | wxDirection::wxRIGHT, std::lround(GetDefaultBorderFractional()));
    }

    wxSizerFlags& DoubleHorzBorder()
    {
        return Border(wxDirection::wxLEFT | wxDirection::wxRIGHT, std::lround(2 * GetDefaultBorderFractional()));
    }

    // setters for the others flags
    wxSizerFlags& Shaped()
    {
        m_flags |= wxSHAPED;

        return *this;
    }

    wxSizerFlags& FixedMinSize()
    {
        m_flags |= wxFIXED_MINSIZE;

        return *this;
    }

    // makes the item ignore window's visibility status
    wxSizerFlags& ReserveSpaceEvenIfHidden()
    {
        m_flags |= wxRESERVE_SPACE_EVEN_IF_HIDDEN;
        return *this;
    }

    int GetProportion() const { return m_proportion; }
    int GetFlags() const { return m_flags; }
    int GetBorderInPixels() const { return m_borderInPixels; }

private:
#ifdef wxNEEDS_BORDER_IN_PX
    static float DoGetDefaultBorderInPx();
#endif // wxNEEDS_BORDER_IN_PX

    int m_proportion;
    unsigned int m_flags;
    int m_borderInPixels;
};


// ----------------------------------------------------------------------------
// wxSizerSpacer: used by wxSizerItem to represent a spacer
// ----------------------------------------------------------------------------

class wxSizerSpacer
{
public:
    wxSizerSpacer(wxSize size) : m_size(size), m_isShown(true) { }

    void SetSize(wxSize size) { m_size = size; }
    wxSize GetSize() const { return m_size; }

    void Show(bool show) { m_isShown = show; }
    bool IsShown() const { return m_isShown; }

private:
    // the size, in pixel
    wxSize m_size;

    // is the spacer currently shown?
    bool m_isShown;
};

// ----------------------------------------------------------------------------
// wxSizerItem
// ----------------------------------------------------------------------------

class wxSizerItem : public wxObject
{
public:
    // window
    wxSizerItem( wxWindow *window,
                 int proportion=0,
                 int flag=0,
                 int border=0,
                 wxObject* userData=nullptr );

    // window with flags
    wxSizerItem(wxWindow *window, const wxSizerFlags& flags)
    {
        Init(flags);

        DoSetWindow(window);
    }

    // subsizer
    wxSizerItem( wxSizer *sizer,
                 int proportion=0,
                 int flag=0,
                 int border=0,
                 wxObject* userData=nullptr );

    // sizer with flags
    wxSizerItem(wxSizer *sizer, const wxSizerFlags& flags)
    {
        Init(flags);

        DoSetSizer(sizer);
    }

    // spacer
    wxSizerItem( int width,
                 int height,
                 int proportion=0,
                 int flag=0,
                 int border=0,
                 wxObject* userData=nullptr);

    // spacer with flags
    wxSizerItem(int width, int height, const wxSizerFlags& flags)
    {
        Init(flags);

        DoSetSpacer(wxSize(width, height));
    }

    wxSizerItem() = default;
    ~wxSizerItem();

    wxSizerItem& operator=(wxSizerItem&&) = delete;

    virtual void DeleteWindows();

    // Enable deleting the SizerItem without destroying the contained sizer.
    void DetachSizer() { m_sizer = nullptr; }

    // Enable deleting the SizerItem without resetting the sizer in the
    // contained window.
    void DetachWindow() { m_window = nullptr; m_kind = ItemKind::None; }

    virtual wxSize GetSize() const;
    virtual wxSize CalcMin();
    virtual void SetDimension( const wxPoint& pos, wxSize size );

    wxSize GetMinSize() const
        { return m_minSize; }
    wxSize GetMinSizeWithBorder() const;

    wxSize GetMaxSize() const
        { return IsWindow() ? m_window->GetMaxSize() : wxDefaultSize; }
    wxSize GetMaxSizeWithBorder() const;

    void SetMinSize(wxSize size)
    {
        if ( IsWindow() )
            m_window->SetMinSize(size);
        m_minSize = size;
    }

    // if either of dimensions is zero, ratio is assumed to be 1
    // to avoid "divide by zero" errors
    void SetRatio(int width, int height)
        { m_ratio = (width && height) ? ((float) width / (float) height) : 1; }
    void SetRatio(wxSize size)
        { SetRatio(size.x, size.y); }
    void SetRatio(float ratio)
        { m_ratio = ratio; }
    float GetRatio() const
        { return m_ratio; }

    virtual wxRect GetRect() { return m_rect; }

    // set a sizer item id (different from a window id, all sizer items,
    // including spacers, can have an associated id)
    void SetId(int id) { m_id = id; }
    int GetId() const { return m_id; }

    bool IsWindow() const { return m_kind == ItemKind::Window; }
    bool IsSizer() const { return m_kind == ItemKind::Sizer; }
    bool IsSpacer() const { return m_kind == ItemKind::Spacer; }

    void SetProportion( int proportion )
        { m_proportion = proportion; }
    int GetProportion() const
        { return m_proportion; }
    void SetFlag( unsigned int flag )
        { m_flag = flag; }
    unsigned int GetFlag() const
        { return m_flag; }
    void SetBorder( int border )
        { m_border = border; }
    int GetBorder() const
        { return m_border; }

    wxWindow *GetWindow() const
        { return m_kind == ItemKind::Window ? m_window : nullptr; }
    wxSizer *GetSizer() const
        { return m_kind == ItemKind::Sizer ? m_sizer : nullptr; }
    wxSize GetSpacer() const;

    // This function behaves obviously for the windows and spacers but for the
    // sizers it returns true if any sizer element is shown and only returns
    // false if all of them are hidden. Also, it always returns true if
    // wxRESERVE_SPACE_EVEN_IF_HIDDEN flag was used.
    bool IsShown() const;

    void Show(bool show);

    void SetUserData(wxObject* userData)
        { delete m_userData; m_userData = userData; }
    wxObject* GetUserData() const
        { return m_userData; }
    wxPoint GetPosition() const
        { return m_pos; }

    // Called once the first component of an item has been decided. This is
    // used in algorithms that depend on knowing the size in one direction
    // before the min size in the other direction can be known.
    // Returns true if it made use of the information (and min size was changed).
    bool InformFirstDirection( int direction, int size, int availableOtherDir=-1 );

    // these functions delete the current contents of the item if it's a sizer
    // or a spacer but not if it is a window
    void AssignWindow(wxWindow *window)
    {
        Free();
        DoSetWindow(window);
    }

    void AssignSizer(wxSizer *sizer)
    {
        Free();
        DoSetSizer(sizer);
    }

    void AssignSpacer(wxSize size)
    {
        Free();
        DoSetSpacer(size);
    }

    void AssignSpacer(int w, int h) { AssignSpacer(wxSize(w, h)); }

protected:
    // common part of ctors taking wxSizerFlags
    void Init(const wxSizerFlags& flags);

    // free current contents
    void Free();

    // common parts of Set/AssignXXX()
    void DoSetWindow(wxWindow *window);
    void DoSetSizer(wxSizer *sizer);
    void DoSetSpacer(wxSize size);

    // Add the border specified for this item to the given size
    // if it's != wxDefaultSize, just return wxDefaultSize otherwise.
    wxSize AddBorderToSize(wxSize size) const;

    // discriminated union: depending on m_kind one of the fields is valid
    enum class ItemKind
    {
        None,
        Window,
        Sizer,
        Spacer,
        Max
    };

    // on screen rectangle of this item (not including borders)
    wxRect       m_rect;

    wxPoint      m_pos;
    wxSize       m_minSize;

    wxObject    *m_userData{nullptr};

    union
    {
        wxWindow      *m_window;
        wxSizer       *m_sizer;
        wxSizerSpacer *m_spacer;
    };

    int          m_proportion{0};
    int          m_border{0};
    unsigned int m_flag{};
    int          m_id{wxID_NONE};

    // Aspect ratio can always be calculated from m_size,
    // but this would cause precision loss when the window
    // is shrunk.  It is safer to preserve the initial value.
    float        m_ratio;
    
    ItemKind m_kind{ItemKind::None};
};

WX_DECLARE_LIST( wxSizerItem, wxSizerItemList );


//---------------------------------------------------------------------------
// wxSizer
//---------------------------------------------------------------------------

class wxSizer: public wxObject, public wxClientDataContainer
{
public:
    ~wxSizer();

    // methods for adding elements to the sizer: there are Add/Insert/Prepend
    // overloads for each of window/sizer/spacer/wxSizerItem
    wxSizerItem* Add(wxWindow *window,
                     int proportion = 0,
                     int flag = 0,
                     int border = 0,
                     wxObject* userData = nullptr);
    wxSizerItem* Add(wxSizer *sizer,
                     int proportion = 0,
                     int flag = 0,
                     int border = 0,
                     wxObject* userData = nullptr);
    wxSizerItem* Add(int width,
                     int height,
                     int proportion = 0,
                     int flag = 0,
                     int border = 0,
                     wxObject* userData = nullptr);
    wxSizerItem* Add( wxWindow *window, const wxSizerFlags& flags);
    wxSizerItem* Add( wxSizer *sizer, const wxSizerFlags& flags);
    wxSizerItem* Add( int width, int height, const wxSizerFlags& flags);
    wxSizerItem* Add( wxSizerItem *item);

    virtual wxSizerItem *AddSpacer(int size);
    wxSizerItem* AddStretchSpacer(int prop = 1);

    wxSizerItem* Insert(size_t index,
                        wxWindow *window,
                        int proportion = 0,
                        int flag = 0,
                        int border = 0,
                        wxObject* userData = nullptr);
    wxSizerItem* Insert(size_t index,
                        wxSizer *sizer,
                        int proportion = 0,
                        int flag = 0,
                        int border = 0,
                        wxObject* userData = nullptr);
    wxSizerItem* Insert(size_t index,
                        int width,
                        int height,
                        int proportion = 0,
                        int flag = 0,
                        int border = 0,
                        wxObject* userData = nullptr);
    wxSizerItem* Insert(size_t index,
                        wxWindow *window,
                        const wxSizerFlags& flags);
    wxSizerItem* Insert(size_t index,
                        wxSizer *sizer,
                        const wxSizerFlags& flags);
    wxSizerItem* Insert(size_t index,
                        int width,
                        int height,
                        const wxSizerFlags& flags);

    // NB: do _not_ override this function in the derived classes, this one is
    //     virtual for compatibility reasons only to allow old code overriding
    //     it to continue to work, override DoInsert() instead in the new code
    virtual wxSizerItem* Insert(size_t index, wxSizerItem *item);

    wxSizerItem* InsertSpacer(size_t index, int size);
    wxSizerItem* InsertStretchSpacer(size_t index, int prop = 1);

    wxSizerItem* Prepend(wxWindow *window,
                         int proportion = 0,
                         int flag = 0,
                         int border = 0,
                         wxObject* userData = nullptr);
    wxSizerItem* Prepend(wxSizer *sizer,
                         int proportion = 0,
                         int flag = 0,
                         int border = 0,
                         wxObject* userData = nullptr);
    wxSizerItem* Prepend(int width,
                         int height,
                         int proportion = 0,
                         int flag = 0,
                         int border = 0,
                         wxObject* userData = nullptr);
    wxSizerItem* Prepend(wxWindow *window, const wxSizerFlags& flags);
    wxSizerItem* Prepend(wxSizer *sizer, const wxSizerFlags& flags);
    wxSizerItem* Prepend(int width, int height, const wxSizerFlags& flags);
    wxSizerItem* Prepend(wxSizerItem *item);

    wxSizerItem* PrependSpacer(int size);
    wxSizerItem* PrependStretchSpacer(int prop = 1);

    // set (or possibly unset if window is NULL) or get the window this sizer
    // is used in
    void SetContainingWindow(wxWindow *window);
    wxWindow *GetContainingWindow() const { return m_containingWindow; }

    virtual bool Remove( wxSizer *sizer );
    virtual bool Remove( int index );

    virtual bool Detach( wxWindow *window );
    virtual bool Detach( wxSizer *sizer );
    virtual bool Detach( int index );

    virtual bool Replace( wxWindow *oldwin, wxWindow *newwin, bool recursive = false );
    virtual bool Replace( wxSizer *oldsz, wxSizer *newsz, bool recursive = false );
    virtual bool Replace( size_t index, wxSizerItem *newitem );

    virtual void Clear( bool delete_windows = false );
    virtual void DeleteWindows();

    // Inform sizer about the first direction that has been decided (by parent item)
    // Returns true if it made use of the information (and recalculated min size)
    //
    // Note that while this method doesn't do anything by default, it should
    // almost always be overridden in the derived classes and should have been
    // pure virtual if not for backwards compatibility constraints.
    virtual bool InformFirstDirection( [[maybe_unused]] int direction, [[maybe_unused]] int size, [[maybe_unused]] int availableOtherDir )
        { return false; }

    void SetMinSize( wxSize size )
        { DoSetMinSize( size ); }

    // Searches recursively
    bool SetItemMinSize( wxWindow *window, wxSize minSize )
        { return DoSetItemMinSize( window, minSize ); }

    // Searches recursively
    bool SetItemMinSize( wxSizer *sizer, wxSize minSize )
        { return DoSetItemMinSize( sizer, minSize ); }

    bool SetItemMinSize( size_t index, wxSize minSize )
        { return DoSetItemMinSize( index, minSize ); }

    wxSize GetSize() const
        { return m_size; }
    wxPoint GetPosition() const
        { return m_position; }

    // Calculate the minimal size or return m_minSize if bigger.
    wxSize GetMinSize();

    // These virtual functions are used by the layout algorithm: first
    // CalcMin() is called to calculate the minimal size of the sizer and
    // prepare for laying it out and then RepositionChildren() is called with
    // this size to really update all the sizer items.
    virtual wxSize CalcMin() = 0;

    // This method should be overridden but isn't pure virtual for backwards
    // compatibility.
    virtual void RepositionChildren([[maybe_unused]] wxSize minSize)
    {
        RecalcSizes();
    }

    // This is a deprecated version of RepositionChildren() which doesn't take
    // the minimal size parameter which is not needed for very simple sizers
    // but typically is for anything more complicated, so prefer to override
    // RepositionChildren() in new code.
    //
    // If RepositionChildren() is not overridden, this method must be
    // overridden, calling the base class version results in an assertion
    // failure.
    virtual void RecalcSizes();

    virtual void Layout();

    wxSize ComputeFittingClientSize(wxWindow *window);
    wxSize ComputeFittingWindowSize(wxWindow *window);

    wxSize Fit( wxWindow *window );
    void FitInside( wxWindow *window );
    void SetSizeHints( wxWindow *window );

    wxSizerItemList& GetChildren()
        { return m_children; }
    const wxSizerItemList& GetChildren() const
        { return m_children; }

    void SetDimension(const wxPoint& pos, wxSize size)
    {
        m_position = pos;
        m_size = size;
        Layout();

        // This call is required for wxWrapSizer to be able to calculate its
        // minimal size correctly.
        InformFirstDirection(wxHORIZONTAL, size.x, size.y);
    }
    void SetDimension(int x, int y, int width, int height)
        { SetDimension(wxPoint(x, y), wxSize(width, height)); }

    size_t GetItemCount() const { return m_children.GetCount(); }
    bool IsEmpty() const { return m_children.IsEmpty(); }

    wxSizerItem* GetItem( wxWindow *window, bool recursive = false );
    wxSizerItem* GetItem( wxSizer *sizer, bool recursive = false );
    wxSizerItem* GetItem( size_t index );
    wxSizerItem* GetItemById( int id, bool recursive = false );

    // Manage whether individual scene items are considered
    // in the layout calculations or not.
    bool Show( wxWindow *window, bool show = true, bool recursive = false );
    bool Show( wxSizer *sizer, bool show = true, bool recursive = false );
    bool Show( size_t index, bool show = true );

    bool Hide( wxSizer *sizer, bool recursive = false )
        { return Show( sizer, false, recursive ); }
    bool Hide( wxWindow *window, bool recursive = false )
        { return Show( window, false, recursive ); }
    bool Hide( size_t index )
        { return Show( index, false ); }

    bool IsShown( wxWindow *window ) const;
    bool IsShown( wxSizer *sizer ) const;
    bool IsShown( size_t index ) const;

    // Recursively call wxWindow::Show () on all sizer items.
    virtual void ShowItems (bool show);

    void Show(bool show) { ShowItems(show); }

    // This is the ShowItems() counterpart and returns true if any of the sizer
    // items are shown.
    virtual bool AreAnyItemsShown() const;

protected:
    wxSize              m_size;
    wxSize              m_minSize;
    wxPoint             m_position;
    wxSizerItemList     m_children;

    // the window this sizer is used in, can be NULL
    wxWindow *m_containingWindow{nullptr};

    wxSize GetMaxClientSize( wxWindow *window ) const;
    wxSize GetMinClientSize( wxWindow *window );
    wxSize VirtualFitSize( wxWindow *window );

    virtual void DoSetMinSize( wxSize minsize );
    virtual bool DoSetItemMinSize( wxWindow *window, wxSize minSize );
    virtual bool DoSetItemMinSize( wxSizer *sizer, wxSize minSize );
    virtual bool DoSetItemMinSize( size_t index, wxSize minSize );

    // insert a new item into m_children at given index and return the item
    // itself
    virtual wxSizerItem* DoInsert(size_t index, wxSizerItem *item);
};

//---------------------------------------------------------------------------
// wxGridSizer
//---------------------------------------------------------------------------

class wxGridSizer: public wxSizer
{
public:
    // ctors specifying the number of columns only: number of rows will be
    // deduced automatically depending on the number of sizer elements
    wxGridSizer( int cols, int vgap, int hgap );
    wxGridSizer( int cols, wxSize gap = {0, 0} );

    // ctors specifying the number of rows and columns
    wxGridSizer( int rows, int cols, int vgap, int hgap );
    wxGridSizer( int rows, int cols, wxSize gap );

    void RepositionChildren(wxSize minSize) override;
    wxSize CalcMin() override;

    void SetCols( int cols )
    {
        wxASSERT_MSG( cols >= 0, "Number of columns must be non-negative");
        m_cols = cols;
    }

    void SetRows( int rows )
    {
        wxASSERT_MSG( rows >= 0, "Number of rows must be non-negative");
        m_rows = rows;
    }

    void SetVGap( int gap )     { m_vgap = gap; }
    void SetHGap( int gap )     { m_hgap = gap; }
    int GetCols() const         { return m_cols; }
    int GetRows() const         { return m_rows; }
    int GetVGap() const         { return m_vgap; }
    int GetHGap() const         { return m_hgap; }

    int GetEffectiveColsCount() const   { return m_cols ? m_cols : CalcCols(); }
    int GetEffectiveRowsCount() const   { return m_rows ? m_rows : CalcRows(); }

    // return the number of total items and the number of columns and rows
    // (for internal use only)
    int CalcRowsCols(int& rows, int& cols) const;

protected:
    // the number of rows/columns in the sizer, if 0 then it is determined
    // dynamically depending on the total number of items
    int    m_rows;
    int    m_cols;

    // gaps between rows and columns
    int    m_vgap;
    int    m_hgap;

    wxSizerItem *DoInsert(size_t index, wxSizerItem *item) override;

    void SetItemBounds(wxSizerItem *item, wxRect boundary);

    // returns the number of columns/rows needed for the current total number
    // of children (and the fixed number of rows/columns)
    int CalcCols() const
    {
        wxCHECK_MSG
        (
            m_rows, 0,
            "Can't calculate number of cols if number of rows is not specified"
        );

        return int(m_children.GetCount() + m_rows - 1) / m_rows;
    }

    int CalcRows() const
    {
        wxCHECK_MSG
        (
            m_cols, 0,
            "Can't calculate number of cols if number of rows is not specified"
        );

        return int(m_children.GetCount() + m_cols - 1) / m_cols;
    }
};

//---------------------------------------------------------------------------
// wxFlexGridSizer
//---------------------------------------------------------------------------

// values which define the behaviour for resizing wxFlexGridSizer cells in the
// "non-flexible" direction
enum class wxFlexSizerGrowMode
{
    // don't resize the cells in non-flexible direction at all
    None,

    // uniformly resize only the specified ones (default)
    Specified,

    // uniformly resize all cells
    All
};

class wxFlexGridSizer: public wxGridSizer
{
public:
    // ctors specifying the number of columns only: number of rows will be
    // deduced automatically depending on the number of sizer elements
    wxFlexGridSizer( int cols, int vgap, int hgap );
    wxFlexGridSizer( int cols, wxSize gap = {0, 0} );

    wxFlexGridSizer( int rows, int cols, int vgap, int hgap );
    wxFlexGridSizer( int rows, int cols, wxSize gap );

    wxFlexGridSizer& operator=(wxFlexGridSizer&&) = delete;

    // set the rows/columns which will grow (the others will remain of the
    // constant initial size)
    void AddGrowableRow( size_t idx, int proportion = 0 );
    void RemoveGrowableRow( size_t idx );
    void AddGrowableCol( size_t idx, int proportion = 0 );
    void RemoveGrowableCol( size_t idx );

    bool IsRowGrowable( size_t idx );
    bool IsColGrowable( size_t idx );

    // the sizer cells may grow in both directions, not grow at all or only
    // grow in one direction but not the other

    // the direction may be wxVERTICAL, wxHORIZONTAL or wxBOTH (default)
    void SetFlexibleDirection(int direction) { m_flexDirection = direction; }
    int GetFlexibleDirection() const { return m_flexDirection; }

    // note that the grow mode only applies to the direction which is not
    // flexible
    void SetNonFlexibleGrowMode(wxFlexSizerGrowMode mode) { m_growMode = mode; }
    wxFlexSizerGrowMode GetNonFlexibleGrowMode() const { return m_growMode; }

    // Read-only access to the row heights and col widths arrays
    const std::vector<int>& GetRowHeights() const { return m_rowHeights; }
    const std::vector<int>& GetColWidths() const  { return m_colWidths; }

    // implementation
    void RepositionChildren(wxSize minSize) override;
    wxSize CalcMin() override;

protected:
    void AdjustForFlexDirection();
    void AdjustForGrowables(wxSize sz, wxSize minSize);
    wxSize FindWidthsAndHeights(int nrows, int ncols);

    // the heights/widths of all rows/columns
    std::vector<int>  m_rowHeights,
                m_colWidths;

    // indices of the growable columns and rows
    std::vector<int>  m_growableRows,
                m_growableCols;

    // proportion values of the corresponding growable rows and columns
    std::vector<int>  m_growableRowsProportions,
                m_growableColsProportions;

    // parameters describing whether the growable cells should be resized in
    // both directions or only one
    int m_flexDirection{wxBOTH};
    wxFlexSizerGrowMode m_growMode{wxFlexSizerGrowMode::Specified};
};

//---------------------------------------------------------------------------
// wxBoxSizer
//---------------------------------------------------------------------------

class wxBoxSizer: public wxSizer
{
public:
    wxBoxSizer(int orient)
    {
        m_orient = orient;
        m_totalProportion = 0;

        wxASSERT_MSG( m_orient == wxHORIZONTAL || m_orient == wxVERTICAL,
                      "invalid value for wxBoxSizer orientation" );
    }

    wxSizerItem *AddSpacer(int size) override;

    int GetOrientation() const { return m_orient; }

    bool IsVertical() const { return m_orient == wxVERTICAL; }

    void SetOrientation(int orient) { m_orient = orient; }

    // implementation of our resizing logic
    wxSize CalcMin() override;
    void RepositionChildren(wxSize minSize) override;

    bool InformFirstDirection(int direction,
                                      int size,
                                      int availableOtherDir) override;

protected:
    // Only overridden to perform extra debugging checks.
    wxSizerItem *DoInsert(size_t index, wxSizerItem *item) override;

    // helpers for our code: this returns the component of the given wxSize in
    // the direction of the sizer and in the other direction, respectively
    int GetSizeInMajorDir(wxSize sz) const
    {
        return m_orient == wxHORIZONTAL ? sz.x : sz.y;
    }

    int& SizeInMajorDir(wxSize& sz)
    {
        return m_orient == wxHORIZONTAL ? sz.x : sz.y;
    }

    int& PosInMajorDir(wxPoint& pt)
    {
        return m_orient == wxHORIZONTAL ? pt.x : pt.y;
    }

    int GetSizeInMinorDir(wxSize sz) const
    {
        return m_orient == wxHORIZONTAL ? sz.y : sz.x;
    }

    int& SizeInMinorDir(wxSize& sz)
    {
        return m_orient == wxHORIZONTAL ? sz.y : sz.x;
    }

    int& PosInMinorDir(wxPoint& pt)
    {
        return m_orient == wxHORIZONTAL ? pt.y : pt.x;
    }

    // another helper: creates wxSize from major and minor components
    wxSize SizeFromMajorMinor(int major, int minor) const
    {
        if ( m_orient == wxHORIZONTAL )
        {
            return wxSize(major, minor);
        }
        else // wxVERTICAL
        {
            return wxSize(minor, major);
        }
    }


    // either wxHORIZONTAL or wxVERTICAL
    int m_orient;

    // the sum of proportion of all of our elements
    int m_totalProportion;
};

//---------------------------------------------------------------------------
// wxStaticBoxSizer
//---------------------------------------------------------------------------

#if wxUSE_STATBOX

class wxStaticBox;

class wxStaticBoxSizer: public wxBoxSizer
{
public:
    wxStaticBoxSizer(wxStaticBox *box, int orient);
    wxStaticBoxSizer(int orient, wxWindow *win, const std::string& label = {});
    ~wxStaticBoxSizer();

    wxStaticBoxSizer& operator=(wxStaticBoxSizer&&) = delete;

    wxSize CalcMin() override;
    void RepositionChildren(wxSize minSize) override;

    wxStaticBox *GetStaticBox() const
        { return m_staticBox; }

    // override to hide/show the static box as well
    void ShowItems (bool show) override;
    bool AreAnyItemsShown() const override;

    bool Detach( wxWindow *window ) override;
    bool Detach( wxSizer *sizer ) override { return wxBoxSizer::Detach(sizer); }
    bool Detach( int index ) override { return wxBoxSizer::Detach(index); }

protected:
    wxStaticBox   *m_staticBox;
};

#endif // wxUSE_STATBOX

//---------------------------------------------------------------------------
// wxStdDialogButtonSizer
//---------------------------------------------------------------------------

#if wxUSE_BUTTON

class wxStdDialogButtonSizer: public wxBoxSizer
{
public:
    // Constructor just creates a new wxBoxSizer, not much else.
    // Box sizer orientation is automatically determined here:
    // vertical for PDAs, horizontal for everything else?
    wxStdDialogButtonSizer();

    wxStdDialogButtonSizer& operator=(wxStdDialogButtonSizer&&) = delete;

    // Checks button ID against system IDs and sets one of the pointers below
    // to this button. Does not do any sizer-related things here.
    void AddButton(wxButton *button);

    // Use these if no standard ID can/should be used
    void SetAffirmativeButton( wxButton *button );
    void SetNegativeButton( wxButton *button );
    void SetCancelButton( wxButton *button );

    // All platform-specific code here, checks which buttons exist and add
    // them to the sizer accordingly.
    // Note - one potential hack on Mac we could use here,
    // if m_buttonAffirmative is wxID_SAVE then ensure wxID_SAVE
    // is set to _("Save") and m_buttonNegative is set to _("Don't Save")
    // I wouldn't add any other hacks like that into here,
    // but this one I can see being useful.
    void Realize();

    wxButton *GetAffirmativeButton() const { return m_buttonAffirmative; }
    wxButton *GetApplyButton() const { return m_buttonApply; }
    wxButton *GetNegativeButton() const { return m_buttonNegative; }
    wxButton *GetCancelButton() const { return m_buttonCancel; }
    wxButton *GetHelpButton() const { return m_buttonHelp; }

protected:
    wxButton *m_buttonAffirmative;  // wxID_OK, wxID_YES, wxID_SAVE go here
    wxButton *m_buttonApply;        // wxID_APPLY
    wxButton *m_buttonNegative;     // wxID_NO
    wxButton *m_buttonCancel;       // wxID_CANCEL, wxID_CLOSE
    wxButton *m_buttonHelp;         // wxID_HELP, wxID_CONTEXT_HELP
};

#endif // wxUSE_BUTTON

} // export

// ----------------------------------------------------------------------------
// inline functions implementation
// ----------------------------------------------------------------------------

inline wxSizerItem*
wxSizer::Insert(size_t index, wxSizerItem *item)
{
    return DoInsert(index, item);
}


inline wxSizerItem*
wxSizer::Add( wxSizerItem *item )
{
    return Insert( m_children.GetCount(), item );
}

inline wxSizerItem*
wxSizer::Add( wxWindow *window, int proportion, int flag, int border, wxObject* userData )
{
    return Add( new wxSizerItem( window, proportion, flag, border, userData ) );
}

inline wxSizerItem*
wxSizer::Add( wxSizer *sizer, int proportion, int flag, int border, wxObject* userData )
{
    return Add( new wxSizerItem( sizer, proportion, flag, border, userData ) );
}

inline wxSizerItem*
wxSizer::Add( int width, int height, int proportion, int flag, int border, wxObject* userData )
{
    return Add( new wxSizerItem( width, height, proportion, flag, border, userData ) );
}

inline wxSizerItem*
wxSizer::Add( wxWindow *window, const wxSizerFlags& flags )
{
    return Add( new wxSizerItem(window, flags) );
}

inline wxSizerItem*
wxSizer::Add( wxSizer *sizer, const wxSizerFlags& flags )
{
    return Add( new wxSizerItem(sizer, flags) );
}

inline wxSizerItem*
wxSizer::Add( int width, int height, const wxSizerFlags& flags )
{
    return Add( new wxSizerItem(width, height, flags) );
}

inline wxSizerItem*
wxSizer::AddSpacer(int size)
{
    return Add(size, size);
}

inline wxSizerItem*
wxSizer::AddStretchSpacer(int prop)
{
    return Add(0, 0, prop);
}

inline wxSizerItem*
wxSizer::Prepend( wxSizerItem *item )
{
    return Insert( 0, item );
}

inline wxSizerItem*
wxSizer::Prepend( wxWindow *window, int proportion, int flag, int border, wxObject* userData )
{
    return Prepend( new wxSizerItem( window, proportion, flag, border, userData ) );
}

inline wxSizerItem*
wxSizer::Prepend( wxSizer *sizer, int proportion, int flag, int border, wxObject* userData )
{
    return Prepend( new wxSizerItem( sizer, proportion, flag, border, userData ) );
}

inline wxSizerItem*
wxSizer::Prepend( int width, int height, int proportion, int flag, int border, wxObject* userData )
{
    return Prepend( new wxSizerItem( width, height, proportion, flag, border, userData ) );
}

inline wxSizerItem*
wxSizer::PrependSpacer(int size)
{
    return Prepend(size, size);
}

inline wxSizerItem*
wxSizer::PrependStretchSpacer(int prop)
{
    return Prepend(0, 0, prop);
}

inline wxSizerItem*
wxSizer::Prepend( wxWindow *window, const wxSizerFlags& flags )
{
    return Prepend( new wxSizerItem(window, flags) );
}

inline wxSizerItem*
wxSizer::Prepend( wxSizer *sizer, const wxSizerFlags& flags )
{
    return Prepend( new wxSizerItem(sizer, flags) );
}

inline wxSizerItem*
wxSizer::Prepend( int width, int height, const wxSizerFlags& flags )
{
    return Prepend( new wxSizerItem(width, height, flags) );
}

inline wxSizerItem*
wxSizer::Insert( size_t index,
                 wxWindow *window,
                 int proportion,
                 int flag,
                 int border,
                 wxObject* userData )
{
    return Insert( index, new wxSizerItem( window, proportion, flag, border, userData ) );
}

inline wxSizerItem*
wxSizer::Insert( size_t index,
                 wxSizer *sizer,
                 int proportion,
                 int flag,
                 int border,
                 wxObject* userData )
{
    return Insert( index, new wxSizerItem( sizer, proportion, flag, border, userData ) );
}

inline wxSizerItem*
wxSizer::Insert( size_t index,
                 int width,
                 int height,
                 int proportion,
                 int flag,
                 int border,
                 wxObject* userData )
{
    return Insert( index, new wxSizerItem( width, height, proportion, flag, border, userData ) );
}

inline wxSizerItem*
wxSizer::Insert( size_t index, wxWindow *window, const wxSizerFlags& flags )
{
    return Insert( index, new wxSizerItem(window, flags) );
}

inline wxSizerItem*
wxSizer::Insert( size_t index, wxSizer *sizer, const wxSizerFlags& flags )
{
    return Insert( index, new wxSizerItem(sizer, flags) );
}

inline wxSizerItem*
wxSizer::Insert( size_t index, int width, int height, const wxSizerFlags& flags )
{
    return Insert( index, new wxSizerItem(width, height, flags) );
}

inline wxSizerItem*
wxSizer::InsertSpacer(size_t index, int size)
{
    return Insert(index, size, size);
}

inline wxSizerItem*
wxSizer::InsertStretchSpacer(size_t index, int prop)
{
    return Insert(index, 0, 0, prop);
}
