/////////////////////////////////////////////////////////////////////////////
// Name:        wx/image.h
// Purpose:     wxImage class
// Author:      Robert Roebling
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/dynarray.h"
#include "wx/gdicmn.h"
#include "wx/object.h"
#include "wx/hashmap.h"

#include "wx/stream.h"

#include <gsl/gsl>

export module WX.Image.Base;

import Utils.Geometry;
import WX.WinDef;

import <string>;
import <vector>;

#if wxUSE_IMAGE

export
{

inline constexpr char wxIMAGE_OPTION_QUALITY[] =         "quality";
inline constexpr char wxIMAGE_OPTION_FILENAME[] =        "FileName";
inline constexpr char wxIMAGE_OPTION_RESOLUTION[] =      "Resolution";
inline constexpr char wxIMAGE_OPTION_RESOLUTIONX[] =     "ResolutionX";
inline constexpr char wxIMAGE_OPTION_RESOLUTIONY[] =     "ResolutionY";
inline constexpr char wxIMAGE_OPTION_RESOLUTIONUNIT[] =  "ResolutionUnit";
inline constexpr char wxIMAGE_OPTION_MAX_WIDTH[] =       "MaxWidth";
inline constexpr char wxIMAGE_OPTION_MAX_HEIGHT[] =      "MaxHeight";
inline constexpr char wxIMAGE_OPTION_ORIGINAL_WIDTH[] =  "OriginalWidth";
inline constexpr char wxIMAGE_OPTION_ORIGINAL_HEIGHT[] = "OriginalHeight";

// These two options are filled in upon reading CUR file and can (should) be
// specified when saving a CUR file - they define the hotspot of the cursor:
inline constexpr char wxIMAGE_OPTION_CUR_HOTSPOT_X[]  = "HotSpotX";
inline constexpr char wxIMAGE_OPTION_CUR_HOTSPOT_Y[]  = "HotSpotY";


// constants used with wxIMAGE_OPTION_RESOLUTIONUNIT
//
// NB: don't change these values, they correspond to libjpeg constants
enum class wxImageResolution
{
    // Resolution not specified
    None,

    // Resolution specified in inches
    Inches,

    // Resolution specified in centimeters
    Centimeters
};

// Constants for wxImage::Scale() for determining the level of quality
enum class wxImageResizeQuality
{
    // different image resizing algorithms used by Scale() and Rescale()
    Nearest = 0,
    Bilinear = 1,
    Bicubic = 2,
    BoxAverage = 3,

    // default quality is low (but fast)
    Normal = Nearest,

    // highest (but best) quality
    High = 4
};

// Constants for wxImage::Paste() for specifying alpha blending option.
enum wxImageAlphaBlendMode
{
    // Overwrite the original alpha values with the ones being pasted.
    wxIMAGE_ALPHA_BLEND_OVER = 0,

    // Compose the original alpha values with the ones being pasted.
    wxIMAGE_ALPHA_BLEND_COMPOSE = 1
};

enum
{
    wxPNG_TYPE_COLOUR = 0,
    wxPNG_TYPE_GREY = 2,
    wxPNG_TYPE_GREY_RED = 3,
    wxPNG_TYPE_PALETTE = 4
};

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

// alpha channel values: fully transparent, default threshold separating
// transparent pixels from opaque for a few functions dealing with alpha and
// fully opaque

inline constexpr unsigned char wxIMAGE_ALPHA_TRANSPARENT = 0;
inline constexpr unsigned char wxIMAGE_ALPHA_THRESHOLD = 0x80;
inline constexpr unsigned char wxIMAGE_ALPHA_OPAQUE = 0xff;

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class wxImage;
class wxPalette;

//-----------------------------------------------------------------------------
// wxVariant support
//-----------------------------------------------------------------------------

#if wxUSE_VARIANT
#include "wx/variant.h"
DECLARE_VARIANT_OBJECT(wxImage)
#endif

//-----------------------------------------------------------------------------
// wxImageHandler
//-----------------------------------------------------------------------------

class wxImageHandler: public wxObject
{
public:
    wxImageHandler() = default;

#if wxUSE_STREAMS
    // NOTE: LoadFile and SaveFile are not pure virtuals to allow derived classes
    //       to implement only one of the two
    virtual bool LoadFile( [[maybe_unused]] wxImage *image, [[maybe_unused]] wxInputStream& stream,
                           [[maybe_unused]] bool verbose=true, [[maybe_unused]] int index=-1 )
        { return false; }
    virtual bool SaveFile( [[maybe_unused]] wxImage *image, [[maybe_unused]] wxOutputStream& stream,
                           [[maybe_unused]] bool verbose=true )
        { return false; }

    int GetImageCount( wxInputStream& stream );
        // save the stream position, call DoGetImageCount() and restore the position

    bool CanRead( wxInputStream& stream ) { return CallDoCanRead(stream); }
    bool CanRead( const std::string& name );
#endif // wxUSE_STREAMS

    void SetName(const std::string& name) { m_name = name; }
    void SetExtension(const std::string& ext) { m_extension = ext; }
    void SetAltExtensions(const std::vector<std::string>& exts) { m_altExtensions = exts; }
    void SetType(wxBitmapType type) { m_type = type; }
    void SetMimeType(const std::string& type) { m_mime = type; }
    const std::string& GetName() const { return m_name; }
    const std::string& GetExtension() const { return m_extension; }
    const std::vector<std::string>& GetAltExtensions() const { return m_altExtensions; }
    wxBitmapType GetType() const { return m_type; }
    const std::string& GetMimeType() const { return m_mime; }

protected:
#if wxUSE_STREAMS
    // NOTE: this function is allowed to change the current stream position
    //       since GetImageCount() will take care of restoring it later
    virtual int DoGetImageCount( [[maybe_unused]] wxInputStream& stream )
        { return 1; }       // default return value is 1 image

    // NOTE: this function is allowed to change the current stream position
    //       since CallDoCanRead() will take care of restoring it later
    virtual bool DoCanRead( wxInputStream& stream ) = 0;

    // save the stream position, call DoCanRead() and restore the position
    bool CallDoCanRead(wxInputStream& stream);
#endif // wxUSE_STREAMS

    // helper for the derived classes SaveFile() implementations: returns the
    // values of x- and y-resolution options specified as the image options if
    // any
    static wxImageResolution
    GetResolutionFromOptions(const wxImage& image, int *x, int *y);

    std::vector<std::string> m_altExtensions;

    std::string  m_name{};
    std::string  m_extension{};
    std::string  m_mime;
    
    wxBitmapType m_type{wxBitmapType::Invalid};

private:
    wxDECLARE_CLASS(wxImageHandler);
};

//-----------------------------------------------------------------------------
// wxImageHistogram
//-----------------------------------------------------------------------------

struct wxImageHistogramEntry
{
    unsigned long index{0};
    unsigned long value{0};
};

WX_DECLARE_HASH_MAP(unsigned long, wxImageHistogramEntry,
                             wxIntegerHash, wxIntegerEqual,
                             wxImageHistogramBase);

class wxImageHistogram : public wxImageHistogramBase
{
public:
    wxImageHistogram() : wxImageHistogramBase(256) { }

    // get the key in the histogram for the given RGB values
    static unsigned long MakeKey(unsigned char r,
                                 unsigned char g,
                                 unsigned char b)
    {
        return (r << 16) | (g << 8) | b;
    }

    // find first colour that is not used in the image and has higher
    // RGB values than RGB(startR, startG, startB)
    //
    // returns true and puts this colour in r, g, b (each of which may be NULL)
    // on success or returns false if there are no more free colours
    bool FindFirstUnusedColour(unsigned char *r,
                               unsigned char *g,
                               unsigned char *b,
                               unsigned char r2 = 1,
                               unsigned char g2 = 0,
                               unsigned char b2 = 0 ) const
    {
        unsigned long key = MakeKey(r2, g2, b2);

        while ( find(key) != end() )
        {
            // color already used
            r2++;
            if ( r2 >= 255 )
            {
                r2 = 0;
                g2++;
                if ( g2 >= 255 )
                {
                    g2 = 0;
                    b2++;
                    if ( b2 >= 255 )
                    {
                        return false;
                    }
                }
            }

            key = MakeKey(r2, g2, b2);
        }

        if ( r )
            *r = r2;
        if ( g )
            *g = g2;
        if ( b )
            *b = b2;

        return true;
    }
};

//-----------------------------------------------------------------------------
// wxImage
//-----------------------------------------------------------------------------

    // red, green and blue are 8 bit unsigned integers in the range of 0..255
    // We use the identifier RGBValue instead of RGB, since RGB is #defined
struct RGBValue
{
    constexpr RGBValue(unsigned char r, unsigned char g, unsigned char b)
        : red(r), green(g), blue(b) {}

    unsigned char red{ 0 };
    unsigned char green{ 0 };
    unsigned char blue{ 0 };
};

class wxImage: public wxObject
{
public:
    // hue, saturation and value are doubles in the range 0.0..1.0
    struct HSVValue
    {
        HSVValue() = default;
        HSVValue(double h, double s, double v)
            : hue(h), saturation(s), value(v) {}

        double hue{0.0};
        double saturation{0.0};
        double value{0.0};
    };

    wxImage() = default;
    wxImage( int width, int height, bool clear = true )
        { Create( width, height, clear ); }
    wxImage( int width, int height, unsigned char* data, bool static_data = false )
        { Create( width, height, data, static_data ); }
    wxImage( int width, int height, unsigned char* data, unsigned char* alpha, bool static_data = false )
        { Create( width, height, data, alpha, static_data ); }

    // ctor variants using wxSize:
    wxImage( const wxSize& sz, bool clear = true )
        { Create( sz, clear ); }
    wxImage( const wxSize& sz, unsigned char* data, bool static_data = false )
        { Create( sz, data, static_data ); }
    wxImage( const wxSize& sz, unsigned char* data, unsigned char* alpha, bool static_data = false )
        { Create( sz, data, alpha, static_data ); }

    wxImage( const std::string& name, wxBitmapType type = wxBitmapType::Any, int index = -1 )
        { LoadFile( name, type, index ); }
    wxImage( const std::string& name, const std::string& mimetype, int index = -1 )
        { LoadFile( name, mimetype, index ); }
    explicit wxImage( const char* const* xpmData )
        { Create(xpmData); }

#if wxUSE_STREAMS
    wxImage( wxInputStream& stream, wxBitmapType type = wxBitmapType::Any, int index = -1 )
        { LoadFile( stream, type, index ); }
    wxImage( wxInputStream& stream, const std::string& mimetype, int index = -1 )
        { LoadFile( stream, mimetype, index ); }
#endif // wxUSE_STREAMS

    [[maybe_unused]] bool Create( const char* const* xpmData );

    [[maybe_unused]] bool Create( int width, int height, bool clear = true );
    [[maybe_unused]] bool Create( int width, int height, unsigned char* data, bool static_data = false );
    [[maybe_unused]] bool Create( int width, int height, unsigned char* data, unsigned char* alpha, bool static_data = false );

    // Create() variants using wxSize:
    [[maybe_unused]] bool Create( const wxSize& sz, bool clear = true )
        { return Create(sz.x, sz.y, clear); }
    [[maybe_unused]] bool Create( const wxSize& sz, unsigned char* data, bool static_data = false )
        { return Create(sz.x, sz.y, data, static_data); }
    [[maybe_unused]] bool Create( const wxSize& sz, unsigned char* data, unsigned char* alpha, bool static_data = false )
        { return Create(sz.x, sz.y, data, alpha, static_data); }

    void Destroy();

    // initialize the image data with zeroes
    void Clear(unsigned char value = 0);

    // creates an identical copy of the image (the = operator
    // just raises the ref count)
    wxImage Copy() const;

    // return the new image with size width*height
    wxImage GetSubImage( const wxRect& rect) const;

    // Paste the image or part of this image into an image of the given size at the pos
    //  any newly exposed areas will be filled with the rgb colour
    //  by default if r = g = b = -1 then fill with this image's mask colour or find and
    //  set a suitable mask colour
    wxImage Size( const wxSize& size, const wxPoint& pos,
                  int r = -1, int g = -1, int b = -1 ) const;

    // Copy the data of the given image to the specified position of this one
    // taking care of the out of bounds problems. Mask is respected, but alpha
    // is simply replaced by default, use wxIMAGE_ALPHA_BLEND_COMPOSE to
    // combine it with the original image alpha values if needed.
    void Paste(const wxImage& image, int x, int y,
               wxImageAlphaBlendMode alphaBlend = wxIMAGE_ALPHA_BLEND_OVER);

    // return the new image with size width*height
    wxImage Scale( int width, int height,
                   wxImageResizeQuality quality = wxImageResizeQuality::Normal ) const;

    // box averager and bicubic filters for up/down sampling
    wxImage ResampleNearest(int width, int height) const;
    wxImage ResampleBox(int width, int height) const;
    wxImage ResampleBilinear(int width, int height) const;
    wxImage ResampleBicubic(int width, int height) const;

    // blur the image according to the specified pixel radius
    wxImage Blur(int radius) const;
    wxImage BlurHorizontal(int radius) const;
    wxImage BlurVertical(int radius) const;

    wxImage ShrinkBy( int xFactor , int yFactor ) const ;

    // rescales the image in place
    wxImage& Rescale( int width, int height,
                      wxImageResizeQuality quality = wxImageResizeQuality::Normal )
        { return *this = Scale(width, height, quality); }

    // resizes the image in place
    wxImage& Resize( const wxSize& size, const wxPoint& pos,
                     int r = -1, int g = -1, int b = -1 ) { return *this = Size(size, pos, r, g, b); }

    // Rotates the image about the given point, 'angle' radians.
    // Returns the rotated image, leaving this image intact.
    wxImage Rotate(double angle, const wxPoint & centre_of_rotation,
                   bool interpolating = true, wxPoint * offset_after_rotation = nullptr) const;

    wxImage Rotate90( bool clockwise = true ) const;
    wxImage Rotate180() const;
    wxImage Mirror( bool horizontally = true ) const;

    // replace one colour with another
    void Replace( unsigned char r1, unsigned char g1, unsigned char b1,
                  unsigned char r2, unsigned char g2, unsigned char b2 );

    // Convert to greyscale image. Uses the luminance component (Y) of the image.
    // The luma value (YUV) is calculated using (R * weight_r) + (G * weight_g) + (B * weight_b), defaults to ITU-T BT.601
    wxImage ConvertToGreyscale(double weight_r, double weight_g, double weight_b) const;
    wxImage ConvertToGreyscale() const;

    // convert to monochrome image (<r,g,b> will be replaced by white,
    // everything else by black)
    wxImage ConvertToMono( unsigned char r, unsigned char g, unsigned char b ) const;

    // Convert to disabled (dimmed) image.
    wxImage ConvertToDisabled(unsigned char brightness = 255) const;

    // Convert the image based on the given lightness.
    wxImage ChangeLightness(int alpha) const;

    // these routines are slow but safe
    void SetRGB( int x, int y, unsigned char r, unsigned char g, unsigned char b );
    void SetRGB( const wxRect& rect, unsigned char r, unsigned char g, unsigned char b );
    unsigned char GetRed( int x, int y ) const;
    unsigned char GetGreen( int x, int y ) const;
    unsigned char GetBlue( int x, int y ) const;

    void SetAlpha(int x, int y, unsigned char alpha);
    unsigned char GetAlpha(int x, int y) const;

    // find first colour that is not used in the image and has higher
    // RGB values than <startR,startG,startB>
    bool FindFirstUnusedColour( unsigned char *r, unsigned char *g, unsigned char *b,
                                unsigned char startR = 1, unsigned char startG = 0,
                                unsigned char startB = 0 ) const;
    // Set image's mask to the area of 'mask' that has <r,g,b> colour
    bool SetMaskFromImage(const wxImage & mask,
                          unsigned char mr, unsigned char mg, unsigned char mb);

    // converts image's alpha channel to mask (choosing mask colour
    // automatically or using the specified colour for the mask), if it has
    // any, does nothing otherwise:
    bool ConvertAlphaToMask(unsigned char threshold = wxIMAGE_ALPHA_THRESHOLD);
    bool ConvertAlphaToMask(unsigned char mr, unsigned char mg, unsigned char mb,
                            unsigned char threshold = wxIMAGE_ALPHA_THRESHOLD);


    // This method converts an image where the original alpha
    // information is only available as a shades of a colour
    // (actually shades of grey) typically when you draw anti-
    // aliased text into a bitmap. The DC drawinf routines
    // draw grey values on the black background although they
    // actually mean to draw white with different alpha values.
    // This method reverses it, assuming a black (!) background
    // and white text (actually only the red channel is read).
    // The method will then fill up the whole image with the
    // colour given.
    bool ConvertColourToAlpha( unsigned char r, unsigned char g, unsigned char b );

    // Methods for controlling LoadFile() behaviour. Currently they allow to
    // specify whether the function should log warnings if there are any
    // problems with the image file not completely preventing it from being
    // loaded. By default the warnings are logged, but this can be disabled
    // either globally or for a particular image object.
    enum
    {
        Load_Verbose = 1
    };

    static void SetDefaultLoadFlags(unsigned int flags);
    static unsigned int GetDefaultLoadFlags();

    void SetLoadFlags(unsigned int flags);
    unsigned int GetLoadFlags() const;

    static bool CanRead( const std::string& name );
    static int GetImageCount( const std::string& name, wxBitmapType type = wxBitmapType::Any );
    virtual bool LoadFile( const std::string& name, wxBitmapType type = wxBitmapType::Any, int index = -1 );
    virtual bool LoadFile( const std::string& name, const std::string& mimetype, int index = -1 );

#if wxUSE_STREAMS
    static bool CanRead( wxInputStream& stream );
    static int GetImageCount( wxInputStream& stream, wxBitmapType type = wxBitmapType::Any );
    virtual bool LoadFile( wxInputStream& stream, wxBitmapType type = wxBitmapType::Any, int index = -1 );
    virtual bool LoadFile( wxInputStream& stream, const std::string& mimetype, int index = -1 );
#endif

    virtual bool SaveFile( const std::string& name ) const;
    virtual bool SaveFile( const std::string& name, wxBitmapType type ) const;
    virtual bool SaveFile( const std::string& name, const std::string& mimetype ) const;

#if wxUSE_STREAMS
    virtual bool SaveFile( wxOutputStream& stream, wxBitmapType type ) const;
    virtual bool SaveFile( wxOutputStream& stream, const std::string& mimetype ) const;
#endif

    bool IsOk() const;
    int GetWidth() const;
    int GetHeight() const;

    wxSize GetSize() const
        { return wxSize(GetWidth(), GetHeight()); }

    // Gets the type of image found by LoadFile or specified with SaveFile
    wxBitmapType GetType() const;

    // Set the image type, this is normally only called if the image is being
    // created from data in the given format but not using LoadFile() (e.g.
    // wxGIFDecoder uses this)
    void SetType(wxBitmapType type);

    // these functions provide fastest access to wxImage data but should be
    // used carefully as no checks are done
    unsigned char *GetData() const;
    void SetData( unsigned char *data, bool static_data=false );
    void SetData( unsigned char *data, int new_width, int new_height, bool static_data=false );

    unsigned char *GetAlpha() const;    // may return NULL!
    bool HasAlpha() const { return GetAlpha() != nullptr; }
    void SetAlpha(unsigned char *alpha = nullptr, bool static_data=false);
    void InitAlpha();
    void ClearAlpha();

    // return true if this pixel is masked or has alpha less than specified
    // threshold
    bool IsTransparent(int x, int y,
                       unsigned char threshold = wxIMAGE_ALPHA_THRESHOLD) const;

    // Mask functions
    void SetMaskColour( unsigned char r, unsigned char g, unsigned char b );
    // Get the current mask colour or find a suitable colour
    // returns true if using current mask colour
    bool GetOrFindMaskColour( unsigned char *r, unsigned char *g, unsigned char *b ) const;
    unsigned char GetMaskRed() const;
    unsigned char GetMaskGreen() const;
    unsigned char GetMaskBlue() const;
    void SetMask( bool mask = true );
    bool HasMask() const;

#if wxUSE_PALETTE
    // Palette functions
    bool HasPalette() const;
    const wxPalette& GetPalette() const;
    void SetPalette(const wxPalette& palette);
#endif // wxUSE_PALETTE

    // Option functions (arbitrary name/value mapping)
    void SetOption(const std::string& name, const std::string& value);
    void SetOption(const std::string& name, int value);
    std::string GetOption(const std::string& name) const;
    int GetOptionInt(const std::string& name) const;
    bool HasOption(const std::string& name) const;

    unsigned long CountColours( unsigned long stopafter = (unsigned long) -1 ) const;

    // Computes the histogram of the image and fills a hash table, indexed
    // with integer keys built as 0xRRGGBB, containing wxImageHistogramEntry
    // objects. Each of them contains an 'index' (useful to build a palette
    // with the image colours) and a 'value', which is the number of pixels
    // in the image with that colour.
    // Returned value: # of entries in the histogram
    unsigned long ComputeHistogram( wxImageHistogram &h ) const;

    // Rotates the hue of each pixel in the image by angle, which is a double in
    // the range [-1.0..+1.0], where -1.0 corresponds to -360 degrees and +1.0
    // corresponds to +360 degrees.
    void RotateHue(double angle);

    // Changes the saturation of each pixel in the image. factor is a double in
    // the range [-1.0..+1.0], where -1.0 corresponds to -100 percent and +1.0
    // corresponds to +100 percent.
    void ChangeSaturation(double factor);

    // Changes the brightness (value) of each pixel in the image. factor is a
    // double in the range [-1.0..+1.0], where -1.0 corresponds to -100 percent
    // and +1.0 corresponds to +100 percent.
    void ChangeBrightness(double factor);

    // Changes the hue, the saturation and the brightness (value) of each pixel
    // in the image. angleH is a double in the range [-1.0..+1.0], where -1.0
    // corresponds to -360 degrees and +1.0 corresponds to +360 degrees, factorS
    // is a double in the range [-1.0..+1.0], where -1.0 corresponds to -100
    // percent and +1.0 corresponds to +100 percent and factorV is a double in
    // the range [-1.0..+1.0], where -1.0 corresponds to -100 percent and +1.0
    // corresponds to +100 percent.
    void ChangeHSV(double angleH, double factorS, double factorV);

    static wxList& GetHandlers() { return sm_handlers; }
    static void AddHandler( wxImageHandler *handler );
    static void InsertHandler( wxImageHandler *handler );
    static bool RemoveHandler( const std::string& name );
    static wxImageHandler *FindHandler( const std::string& name );
    static wxImageHandler *FindHandler( const std::string& extension, wxBitmapType imageType );
    static wxImageHandler *FindHandler( wxBitmapType imageType );

    static wxImageHandler *FindHandlerMime( const std::string& mimetype );

    static std::string GetImageExtWildcard();

    static void CleanUpHandlers();
    static void InitStandardHandlers();

    static HSVValue RGBtoHSV(const RGBValue& rgb);
    static RGBValue HSVtoRGB(const HSVValue& hsv);

protected:
    inline static wxList   sm_handlers;

    // return the index of the point with the given coordinates or -1 if the
    // image is invalid of the coordinates are out of range
    //
    // note that index must be multiplied by 3 when using it with RGB array
    long XYToIndex(int x, int y) const;

    wxObjectRefData* CreateRefData() const override;
    wxObjectRefData* CloneRefData(const wxObjectRefData* data) const override;

    // Helper function used internally by wxImage class only.
    template <typename T>
    void ApplyToAllPixels(void (*filter)(wxImage *, unsigned char *, T), T value);

private:
    friend class wxImageHandler;

    // Possible values for MakeEmptyClone() flags.
    enum
    {
        // Create an image with the same orientation as this one. This is the
        // default and only exists for symmetry with SwapOrientation.
        Clone_SameOrientation = 0,

        // Create an image with the same height as this image width and the
        // same width as this image height.
        Clone_SwapOrientation = 1
    };

    // Returns a new blank image with the same dimensions (or with width and
    // height swapped if Clone_SwapOrientation flag is given), alpha, and mask
    // as this image itself. This is used by several functions creating
    // modified versions of this image.
    wxImage MakeEmptyClone(int flags = Clone_SameOrientation) const;

#if wxUSE_STREAMS
    // read the image from the specified stream updating image type if
    // successful
    bool DoLoad(wxImageHandler& handler, wxInputStream& stream, int index);

    // write the image to the specified stream and also update the image type
    // if successful
    bool DoSave(wxImageHandler& handler, wxOutputStream& stream) const;
#endif // wxUSE_STREAMS
};

WX_DECLARE_OBJARRAY(wxImage, wxImageArray);

inline wxImage wxNullImage;

} // export

#endif // wxUSE_IMAGE
