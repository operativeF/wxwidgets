///////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/textmeasure.h
// Purpose:     declaration of wxTextMeasure class
// Author:      Manuel Martin
// Created:     2012-10-05
// Copyright:   (c) 1997-2012 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRIVATE_TEXTMEASURE_H_
#define _WX_PRIVATE_TEXTMEASURE_H_

class wxDC;
class wxFont;
class wxWindow;

// ----------------------------------------------------------------------------
// wxTextMeasure: class used to measure text extent.
// ----------------------------------------------------------------------------

class wxTextMeasureBase
{
public:
    // The first ctor argument must be non-NULL, i.e. each object of this class
    // is associated with either a valid wxDC or a valid wxWindow. The font can
    // be NULL to use the current DC/window font or can be specified explicitly.
    wxTextMeasureBase(const wxDC *dc, const wxFont *theFont);
    wxTextMeasureBase(const wxWindow *win, const wxFont *theFont);

    // Even though this class is not supposed to be used polymorphically, give
    // it a virtual dtor to avoid compiler warnings.
    virtual ~wxTextMeasureBase() = default;

    wxTextMeasureBase(const wxTextMeasureBase&) = delete;
	wxTextMeasureBase& operator=(const wxTextMeasureBase&) = delete;

    // Return the extent of a single line string.
    wxSize GetTextExtent(std::string_view string,
                         wxCoord *descent = nullptr,
                         wxCoord *externalLeading = nullptr);

    // The same for a multiline (with '\n') string.
    void GetMultiLineTextExtent(std::string_view text,
                                wxCoord *width,
                                wxCoord *height,
                                wxCoord *heightOneLine = nullptr);

    // Find the dimensions of the largest string.
    wxSize GetLargestStringExtent(const std::vector<std::string>& strings);

    // Fill the array with the widths for each "0..N" substrings for N from 1
    // to text.length().
    //
    // The scaleX argument is the horizontal scale used by wxDC and is only
    // used in the generic implementation.
    std::vector<int> GetPartialTextExtents(std::string_view text, double scaleX);


    // This is another method which is only used by MeasuringGuard.
    bool IsUsingDCImpl() const { return m_useDCImpl; }

protected:
    // RAII wrapper for the two methods above.
    class MeasuringGuard
    {
    public:
        MeasuringGuard(wxTextMeasureBase& tm) : m_tm(tm)
        {
            // BeginMeasuring() should only be called if we have a native DC,
            // so don't call it if we delegate to a DC of unknown type.
            if ( !m_tm.IsUsingDCImpl() )
                m_tm.BeginMeasuring();
        }

        ~MeasuringGuard()
        {
            if ( !m_tm.IsUsingDCImpl() )
                m_tm.EndMeasuring();
        }

    private:
        wxTextMeasureBase& m_tm;
    };


    // These functions are called by our public methods before and after each
    // call to DoGetTextExtent(). Derived classes may override them to prepare
    // for -- possibly several -- subsequent calls to DoGetTextExtent().
    //
    // As these calls must be always paired, they're never called directly but
    // only by our friend MeasuringGuard class.
    virtual void BeginMeasuring() { }
    virtual void EndMeasuring() { }


    // The main function of this class, to be implemented in platform-specific
    // way used by all our public methods.
    //
    // The width and height pointers here are never NULL and the input string
    // is not empty.
    virtual wxSize DoGetTextExtent(std::string_view string,
                                   wxCoord *descent = nullptr,
                                   wxCoord *externalLeading = nullptr) = 0;

    // The real implementation of GetPartialTextExtents().
    //
    // On input, widths array contains text.length() zero elements and the text
    // is guaranteed to be non-empty.
    virtual std::vector<int> DoGetPartialTextExtents(std::string_view text, double scaleX) = 0;

    // Call either DoGetTextExtent() or wxDC::GetTextExtent() depending on the
    // value of m_useDCImpl.
    //
    // This must be always used instead of calling DoGetTextExtent() directly!
    wxSize CallGetTextExtent(std::string_view string,
                             wxCoord *descent = nullptr,
                             wxCoord *externalLeading = nullptr);

    // Get line height: used when the line is empty because CallGetTextExtent()
    // would just return (0, 0) in this case.
    int GetEmptyLineHeight();

    // Return a valid font: if one was given to us in the ctor, use this one,
    // otherwise use the current font of the associated wxDC or wxWindow.
    wxFont GetFont() const;


    // Exactly one of m_dc and m_win is non-NULL for any given object of this
    // class.
    const wxDC* const m_dc;
    const wxWindow* const m_win;

    // This one can be NULL or not.
    const wxFont* const m_font;
    
    // If this is true, simply forward to wxDC::GetTextExtent() from our
    // CallGetTextExtent() instead of calling our own DoGetTextExtent().
    //
    // We need this because our DoGetTextExtent() typically only works with
    // native DCs, i.e. those having an WXHDC under Windows or using Pango under
    // GTK+. However wxTextMeasure object can be constructed for any wxDC, not
    // necessarily a native one and in this case we must call back into the DC
    // implementation of text measuring itself.
    bool m_useDCImpl;
};

// Include the platform dependent class declaration, if any.
#if defined(__WXGTK20__)
    #include "wx/gtk/private/textmeasure.h"
#elif defined(__WXMSW__)
    #include "wx/msw/private/textmeasure.h"
#else // no platform-specific implementation of wxTextMeasure yet
    #include "wx/generic/private/textmeasure.h"

    #define wxUSE_GENERIC_TEXTMEASURE 1
#endif

#ifndef wxUSE_GENERIC_TEXTMEASURE
    #define wxUSE_GENERIC_TEXTMEASURE 0
#endif

#endif // _WX_PRIVATE_TEXTMEASURE_H_
