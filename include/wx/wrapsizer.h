/////////////////////////////////////////////////////////////////////////////
// Name:        wx/wrapsizer.h
// Purpose:     provide wrapping sizer for layout (wxWrapSizer)
// Author:      Arne Steinarson
// Created:     2008-05-08
// Copyright:   (c) Arne Steinarson
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WRAPSIZER_H_
#define _WX_WRAPSIZER_H_

import WX.Core.Sizer;
import WX.Cfg.Flags;

// flags for wxWrapSizer
enum
{
    wxEXTEND_LAST_ON_EACH_LINE = 1,
    // don't leave spacers in the beginning of a new row
    wxREMOVE_LEADING_SPACES = 2,

    wxWRAPSIZER_DEFAULT_FLAGS = wxEXTEND_LAST_ON_EACH_LINE |
                                wxREMOVE_LEADING_SPACES
};

// ----------------------------------------------------------------------------
// A box sizer that can wrap items on several lines when sum of widths exceed
// available line width.
// ----------------------------------------------------------------------------

class wxWrapSizer : public wxBoxSizer
{
public:
    wxWrapSizer(unsigned int orient = wxHORIZONTAL, unsigned int flags = wxWRAPSIZER_DEFAULT_FLAGS);
    ~wxWrapSizer();

    wxWrapSizer& operator=(wxWrapSizer&&) = delete;

    wxSize CalcMin() override;
    void RepositionChildren(wxSize minSize) override;

    bool InformFirstDirection(int direction,
                              int size,
                              int availableOtherDir) override;

protected:
    // This method is called to decide if an item represents empty space or
    // not. We do this to avoid having space-only items first or last on a
    // wrapped line (left alignment).
    //
    // By default only spacers are considered to be empty items but a derived
    // class may override this item if some other kind of sizer elements should
    // be also considered empty for some reason.
    virtual bool IsSpaceItem(wxSizerItem *item) const
    {
        return item->IsSpacer();
    }

    // helpers of CalcMin()
    void CalcMinFromMinor(int totMinor);
    void CalcMinFromMajor(int totMajor);
    void CalcMinUsingCurrentLayout();
    void CalcMinFittingSize(const wxSize& szBoundary);
    void CalcMaxSingleItemSize();

    // temporarily change the proportion of the last item of the N-th row to
    // extend to the end of line if the appropriate flag is set
    void AdjustLastRowItemProp(size_t n, wxSizerItem *itemLast);

    // remove all the items from m_rows
    void ClearRows();

    // return the N-th row sizer from m_rows creating it if necessary
    wxSizer *GetRowSizer(size_t n);

    // should be called after completion of each row
    void FinishRow(size_t n, int rowMajor, int rowMinor, wxSizerItem *itemLast);


    const unsigned int m_flags;       // Flags specified in the ctor

    int m_dirInform{0};         // Direction for size information
    int m_availSize{-1};         // Size available in m_dirInform direction
    int m_availableOtherDir{0}; // Size available in the other direction
    bool m_lastUsed{true};         // Indicates whether value from InformFirst... has
                             //  been used yet

    // The sizes below are computed by RepositionChildren(), i.e. they don't have
    // valid values during the initial call to CalcMin() and they are only
    // valid for the current layout (i.e. the current number of rows)
    int m_minSizeMinor{0};      // Min size in minor direction
    int m_maxSizeMajor{0};      // Size of longest row
    int m_minItemMajor{INT_MAX};      // Size of smallest item in major direction

    // the minimal size needed for this sizer as calculated by the last call to
    // our CalcMin()
    wxSize m_calculatedMinSize;

    wxBoxSizer m_rows;       // Sizer containing multiple rows of our items
};

#endif // _WX_WRAPSIZER_H_
