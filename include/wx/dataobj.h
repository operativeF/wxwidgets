///////////////////////////////////////////////////////////////////////////////
// Name:        wx/dataobj.h
// Purpose:     common data object classes
// Author:      Vadim Zeitlin, Robert Roebling
// Modified by:
// Created:     26.05.99
// Copyright:   (c) wxWidgets Team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DATAOBJ_H_BASE_
#define _WX_DATAOBJ_H_BASE_

#if wxUSE_DATAOBJ

#include "wx/gdicmn.h"
#include "wx/string.h"
#include "wx/bitmap.h"
#include "wx/list.h"

import WX.Image;

import <vector>;

// ============================================================================
/*
   Generic data transfer related classes. The class hierarchy is as follows:

                                - wxDataObject-
                               /               \
                              /                 \
            wxDataObjectSimple                  wxDataObjectComposite
           /           |      \
          /            |       \
   wxTextDataObject    |     wxBitmapDataObject
                       |
               wxCustomDataObject
                       |
                       |
               wxImageDataObject
*/
// ============================================================================

// ----------------------------------------------------------------------------
// wxDataFormat class is declared in platform-specific headers: it represents
// a format for data which may be either one of the standard ones (text,
// bitmap, ...) or a custom one which is then identified by a unique string.
// ----------------------------------------------------------------------------

/* the class interface looks like this (pseudo code):

class wxDataFormat
{
public:
    typedef <integral type> NativeFormat;

    wxDataFormat(NativeFormat format = wxDF_INVALID);
    wxDataFormat(const wxString& format);

    wxDataFormat& operator=(NativeFormat format);
    wxDataFormat& operator=(const wxDataFormat& format);

    bool operator==(NativeFormat format) const;
    bool operator!=(NativeFormat format) const;

    void SetType(NativeFormat format);
    NativeFormat GetType() const;

    wxString GetId() const;
    void SetId(const wxString& format);
};

*/

/*  the values of the format constants should be the same as corresponding */
/*  CF_XXX constants in Windows API */
enum wxDataFormatId
{
    wxDF_INVALID =          0,
    wxDF_TEXT =             1,  /* CF_TEXT */
    wxDF_BITMAP =           2,  /* CF_BITMAP */
    wxDF_METAFILE =         3,  /* CF_METAFILEPICT */
    wxDF_SYLK =             4,
    wxDF_DIF =              5,
    wxDF_TIFF =             6,
    wxDF_OEMTEXT =          7,  /* CF_OEMTEXT */
    wxDF_DIB =              8,  /* CF_DIB */
    wxDF_PALETTE =          9,
    wxDF_PENDATA =          10,
    wxDF_RIFF =             11,
    wxDF_WAVE =             12,
    wxDF_UNICODETEXT =      13,
    wxDF_ENHMETAFILE =      14,
    wxDF_FILENAME =         15, /* CF_HDROP */
    wxDF_LOCALE =           16,
    wxDF_PRIVATE =          20,
    wxDF_HTML =             30, /* Note: does not correspond to CF_ constant */
    wxDF_PNG =              31, /* Note: does not correspond to CF_ constant */
    wxDF_MAX
};

#if defined(__WXMSW__)
    #include "wx/msw/ole/dataform.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/dataform.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/dataform.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/dataform.h"
#elif defined(__WXX11__)
    #include "wx/x11/dataform.h"
#elif defined(__WXMAC__)
    #include "wx/osx/dataform.h"
#elif defined(__WXQT__)
    #include "wx/qt/dataform.h"
#endif

// the value for default argument to some functions (corresponds to
// wxDF_INVALID)
extern const wxDataFormat& wxFormatInvalid;

// ----------------------------------------------------------------------------
// wxDataObject represents a piece of data which knows which formats it
// supports and knows how to render itself in each of them - GetDataHere(),
// and how to restore data from the buffer (SetData()).
//
// Although this class may be used directly (i.e. custom classes may be
// derived from it), in many cases it might be simpler to use either
// wxDataObjectSimple or wxDataObjectComposite classes.
//
// A data object may be "read only", i.e. support only GetData() functions or
// "read-write", i.e. support both GetData() and SetData() (in principle, it
// might be "write only" too, but this is rare). Moreover, it doesn't have to
// support the same formats in Get() and Set() directions: for example, a data
// object containing JPEG image might accept BMPs in GetData() because JPEG
// image may be easily transformed into BMP but not in SetData(). Accordingly,
// all methods dealing with formats take an additional "direction" argument
// which is either SET or GET and which tells the function if the format needs
// to be supported by SetData() or GetDataHere().
// ----------------------------------------------------------------------------

class wxDataObjectBase
{
public:
    enum Direction
    {
        Get  = 0x01,    // format is supported by GetDataHere()
        Set  = 0x02,    // format is supported by SetData()
        Both = 0x03     // format is supported by both (unused currently)
    };

    virtual ~wxDataObjectBase() = default;

    // get the best suited format for rendering our data
    virtual wxDataFormat GetPreferredFormat(Direction dir = Get) const = 0;

    // get the number of formats we support
    virtual size_t GetFormatCount(Direction dir = Get) const = 0;

    // return all formats in the provided array (of size GetFormatCount())
    virtual void GetAllFormats(wxDataFormat *formats,
                               Direction dir = Get) const = 0;

    // get the (total) size of data for the given format
    virtual size_t GetDataSize(const wxDataFormat& format) const = 0;

    // copy raw data (in the specified format) to the provided buffer, return
    // true if data copied successfully, false otherwise
    virtual bool GetDataHere(const wxDataFormat& format, void *buf) const = 0;

    // get data from the buffer of specified length (in the given format),
    // return true if the data was read successfully, false otherwise
    virtual bool SetData([[maybe_unused]] const wxDataFormat& format,
                         [[maybe_unused]] size_t len, const [[maybe_unused]] void * buf)
    {
        return false;
    }

    // returns true if this format is supported
    bool IsSupported(const wxDataFormat& format, Direction dir = Get) const;
};

// ----------------------------------------------------------------------------
// include the platform-specific declarations of wxDataObject
// ----------------------------------------------------------------------------

#if defined(__WXMSW__)
    #include "wx/msw/ole/dataobj.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/dataobj.h"
#elif defined(__WXX11__)
    #include "wx/x11/dataobj.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/dataobj.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/dataobj.h"
#elif defined(__WXMAC__)
    #include "wx/osx/dataobj.h"
#elif defined(__WXQT__)
    #include "wx/qt/dataobj.h"
#endif

// ----------------------------------------------------------------------------
// wxDataObjectSimple is a wxDataObject which only supports one format (in
// both Get and Set directions, but you may return false from GetDataHere() or
// SetData() if one of them is not supported). This is the simplest possible
// wxDataObject implementation.
//
// This is still an "abstract base class" (although it doesn't have any pure
// virtual functions), to use it you should derive from it and implement
// GetDataSize(), GetDataHere() and SetData() functions because the base class
// versions don't do anything - they just return "not implemented".
//
// This class should be used when you provide data in only one format (no
// conversion to/from other formats), either a standard or a custom one.
// Otherwise, you should use wxDataObjectComposite or wxDataObject directly.
// ----------------------------------------------------------------------------

class wxDataObjectSimple : public wxDataObject
{
public:
    // ctor takes the format we support, but it can also be set later with
    // SetFormat()
    wxDataObjectSimple(const wxDataFormat& format = wxFormatInvalid)
        : m_format(format)
        {
        }

    wxDataObjectSimple& operator=(wxDataObjectSimple&&) = delete;

    // get/set the format we support
    const wxDataFormat& GetFormat() const { return m_format; }
    void SetFormat(const wxDataFormat& format) { m_format = format; }

    // functions to override in derived class (the base class versions
    // just return "not implemented")
    // -----------------------------------------------------------------------

    // get the size of our data
    virtual size_t GetDataSize() const
        { return 0; }

    // copy our data to the buffer
    virtual bool GetDataHere([[maybe_unused]] void *buf) const
        { return false; }

    // copy data from buffer to our data
    virtual bool SetData([[maybe_unused]] size_t len, [[maybe_unused]] const void *buf)
        { return false; }

    
    // ----------------------------------
    wxDataFormat GetPreferredFormat([[maybe_unused]] wxDataObjectBase::Direction dir = Get) const override
        { return m_format; }
    size_t GetFormatCount([[maybe_unused]] wxDataObjectBase::Direction dir = Get) const override
        { return 1; }
    void GetAllFormats(wxDataFormat *formats,
                               [[maybe_unused]] wxDataObjectBase::Direction dir = Get) const override
        { *formats = m_format; }
    size_t GetDataSize([[maybe_unused]] const wxDataFormat& format) const override
        { return GetDataSize(); }
    bool GetDataHere([[maybe_unused]] const wxDataFormat& format,
                             void *buf) const override
        { return GetDataHere(buf); }
    bool SetData([[maybe_unused]] const wxDataFormat& format,
                         size_t len, const void *buf) override
        { return SetData(len, buf); }

private:
    // the one and only format we support
    wxDataFormat m_format;
};

// ----------------------------------------------------------------------------
// wxDataObjectComposite is the simplest way to implement wxDataObject
// supporting multiple formats. It contains several wxDataObjectSimple and
// supports all formats supported by any of them.
//
// This class shouldn't be (normally) derived from, but may be used directly.
// If you need more flexibility than what it provides, you should probably use
// wxDataObject directly.
// ----------------------------------------------------------------------------

WX_DECLARE_LIST(wxDataObjectSimple, wxSimpleDataObjectList);

class wxDataObjectComposite : public wxDataObject
{
public:
    ~wxDataObjectComposite();

    wxDataObjectComposite& operator=(wxDataObjectComposite&&) = delete;

    // add data object (it will be deleted by wxDataObjectComposite, hence it
    // must be allocated on the heap) whose format will become the preferred
    // one if preferred == true
    void Add(wxDataObjectSimple *dataObject, bool preferred = false);

    // Report the format passed to the SetData method.  This should be the
    // format of the data object within the composite that received data from
    // the clipboard or the DnD operation.  You can use this method to find
    // out what kind of data object was received.
    wxDataFormat GetReceivedFormat() const;

    // Returns the pointer to the object which supports this format or NULL.
    // The returned pointer is owned by wxDataObjectComposite and must
    // therefore not be destroyed by the caller.
    wxDataObjectSimple *wxGetObject(const wxDataFormat& format,
                                  wxDataObjectBase::Direction dir = Get) const;

    
    // ----------------------------------
    wxDataFormat GetPreferredFormat(wxDataObjectBase::Direction dir = Get) const override;
    size_t GetFormatCount(wxDataObjectBase::Direction dir = Get) const override;
    void GetAllFormats(wxDataFormat *formats, wxDataObjectBase::Direction dir = Get) const override;
    size_t GetDataSize(const wxDataFormat& format) const override;
    bool GetDataHere(const wxDataFormat& format, void *buf) const override;
    bool SetData(const wxDataFormat& format, size_t len, const void *buf) override;
#if defined(__WXMSW__)
    const void* GetSizeFromBuffer( const void* buffer, size_t* size,
                                           const wxDataFormat& format ) override;
    void* SetSizeInBuffer( void* buffer, size_t size,
                                   const wxDataFormat& format ) override;
    size_t GetBufferOffset( const wxDataFormat& format ) override;
#endif

private:
    // the list of all (simple) data objects whose formats we support
    wxSimpleDataObjectList m_dataObjects;

    // the index of the preferred one (0 initially, so by default the first
    // one is the preferred)
    size_t m_preferred{0};

    wxDataFormat m_receivedFormat{wxFormatInvalid};
};

// ============================================================================
// Standard implementations of wxDataObjectSimple which can be used directly
// (i.e. without having to derive from them) for standard data type transfers.
//
// Note that although all of them can work with provided data, you can also
// override their virtual GetXXX() functions to only provide data on demand.
// ============================================================================

// ----------------------------------------------------------------------------
// wxTextDataObject contains text data
// ----------------------------------------------------------------------------

#if defined(__WXGTK20__) || defined(__WXX11__) || defined(__WXQT__)
    #define wxNEEDS_UTF8_FOR_TEXT_DATAOBJ
#elif defined(__WXMAC__)
    #define wxNEEDS_UTF16_FOR_TEXT_DATAOBJ
#endif

class wxHTMLDataObject : public wxDataObjectSimple
{
public:
    // ctor: you can specify the text here or in SetText(), or override
    // GetText()
    wxHTMLDataObject(const wxString& html = {})
        : wxDataObjectSimple(wxDF_HTML),
          m_html(html)
        {
        }

    // functions which you may override if you want to provide text on
    // demand only - otherwise, the trivial default versions will be used
    virtual size_t GetLength() const { return m_html.Len() + 1; }
    virtual wxString GetHTML() const { return m_html; }
    virtual void SetHTML(const wxString& html) { m_html = html; }

    size_t GetDataSize() const override;
    bool GetDataHere(void *buf) const override;
    bool SetData(size_t len, const void *buf) override;

    // Must provide overloads to avoid hiding them (and warnings about it)
    size_t GetDataSize(const wxDataFormat&) const override
    {
        return GetDataSize();
    }
    bool GetDataHere(const wxDataFormat&, void *buf) const override
    {
        return GetDataHere(buf);
    }
    bool SetData(const wxDataFormat&, size_t len, const void *buf) override
    {
        return SetData(len, buf);
    }

private:
    wxString m_html;
};

class wxTextDataObject : public wxDataObjectSimple
{
public:
    // ctor: you can specify the text here or in SetText(), or override
    // GetText()
    wxTextDataObject(const std::string& text = {})
        : wxDataObjectSimple(wxDF_UNICODETEXT),
          m_text(text)
        {
        }

    wxTextDataObject& operator=(wxTextDataObject&&) = delete;

    // functions which you may override if you want to provide text on
    // demand only - otherwise, the trivial default versions will be used
    virtual std::size_t GetTextLength() const { return m_text.length() + 1; }
    virtual std::string GetText() const { return m_text; }
    virtual void SetText(const std::string& text) { m_text = text; }

    
    // ----------------------------------

    // some platforms have 2 and not 1 format for text data
#if defined(wxNEEDS_UTF8_FOR_TEXT_DATAOBJ) || defined(wxNEEDS_UTF16_FOR_TEXT_DATAOBJ)
    size_t GetFormatCount([[maybe_unused]] Direction dir = Get) const override { return 2; }
    virtual void GetAllFormats(wxDataFormat *formats,
                               [[maybe_unused]] wxDataObjectBase::Direction dir = Get) const override;

    size_t GetDataSize() const override { return GetDataSize(GetPreferredFormat()); }
    bool GetDataHere(void *buf) const override { return GetDataHere(GetPreferredFormat(), buf); }
    bool SetData(size_t len, const void *buf) override { return SetData(GetPreferredFormat(), len, buf); }

    size_t GetDataSize(const wxDataFormat& format) const override;
    bool GetDataHere(const wxDataFormat& format, void *pBuf) const override;
    bool SetData(const wxDataFormat& format, size_t nLen, const void* pBuf) override;
#else // !wxNEEDS_UTF{8,16}_FOR_TEXT_DATAOBJ
    size_t GetDataSize() const override;
    bool GetDataHere(void *buf) const override;
    bool SetData(size_t len, const void *buf) override;
    // Must provide overloads to avoid hiding them (and warnings about it)
    size_t GetDataSize(const wxDataFormat&) const override
    {
        return GetDataSize();
    }
    bool GetDataHere(const wxDataFormat&, void *buf) const override
    {
        return GetDataHere(buf);
    }
    bool SetData(const wxDataFormat&, size_t len, const void *buf) override
    {
        return SetData(len, buf);
    }
#endif // different wxTextDataObject implementations

private:
#if defined(__WXQT__)
    // Overridden to set text directly instead of extracting byte array
    void QtSetDataSingleFormat(const class QMimeData &mimeData, const wxDataFormat &format) override;
#endif

    std::string m_text;
};

// ----------------------------------------------------------------------------
// wxBitmapDataObject contains a bitmap
// ----------------------------------------------------------------------------

class wxBitmapDataObjectBase : public wxDataObjectSimple
{
public:
    // ctor: you can specify the bitmap here or in SetBitmap(), or override
    // GetBitmap()
    wxBitmapDataObjectBase(const wxBitmap& bitmap = wxNullBitmap)
        : wxDataObjectSimple(wxDF_BITMAP), m_bitmap(bitmap)
        {
        }

    wxBitmapDataObjectBase& operator=(wxBitmapDataObjectBase&&) = delete;

    // functions which you may override if you want to provide data on
    // demand only - otherwise, the trivial default versions will be used
    virtual wxBitmap GetBitmap() const { return m_bitmap; }
    virtual void SetBitmap(const wxBitmap& bitmap) { m_bitmap = bitmap; }

protected:
    wxBitmap m_bitmap;
};

// ----------------------------------------------------------------------------
// wxFileDataObject contains a list of filenames
//
// NB: notice that this is a "write only" object, it can only be filled with
//     data from drag and drop operation.
// ----------------------------------------------------------------------------

class wxFileDataObjectBase : public wxDataObjectSimple
{
public:
    // ctor: use AddFile() later to fill the array
    wxFileDataObjectBase() : wxDataObjectSimple(wxDF_FILENAME) { }

    wxFileDataObjectBase& operator=(wxFileDataObjectBase&&) = delete;

    // get a reference to our array
    const std::vector<std::string>& GetFilenames() const { return m_filenames; }

protected:
    std::vector<std::string> m_filenames;
};

// ----------------------------------------------------------------------------
// wxCustomDataObject contains arbitrary untyped user data.
//
// It is understood that this data can be copied bitwise.
// ----------------------------------------------------------------------------

class wxCustomDataObject : public wxDataObjectSimple
{
public:
    // if you don't specify the format in the ctor, you can still use
    // SetFormat() later
    wxCustomDataObject(const wxDataFormat& format = wxFormatInvalid);

    // the dtor calls Free()
    ~wxCustomDataObject();

    wxCustomDataObject& operator=(wxCustomDataObject&&) = delete;

    // you can call SetData() to set m_data: it will make a copy of the data
    // you pass - or you can use TakeData() which won't copy anything, but
    // will take ownership of data (i.e. will call Free() on it later)
    void TakeData(size_t size, void *data);

    // this function is called to allocate "size" bytes of memory from
    // SetData(). The default version uses operator new[].
    virtual void *Alloc(size_t size);

    // this function is called when the data is freed, you may override it to
    // anything you want (or may be nothing at all). The default version calls
    // operator delete[] on m_data
    virtual void Free();

    // get data: you may override these functions if you wish to provide data
    // only when it's requested
    virtual size_t GetSize() const { return m_size; }
    virtual void *GetData() const { return m_data; }

    
    // ----------------------------------
    size_t GetDataSize() const override;
    bool GetDataHere(void *buf) const override;
    bool SetData(size_t size, const void *buf) override;
    // Must provide overloads to avoid hiding them (and warnings about it)
    size_t GetDataSize(const wxDataFormat&) const override
    {
        return GetDataSize();
    }
    bool GetDataHere(const wxDataFormat&, void *buf) const override
    {
        return GetDataHere(buf);
    }
    bool SetData(const wxDataFormat&, size_t len, const void *buf) override
    {
        return SetData(len, buf);
    }

private:
    size_t m_size{0};
    void  *m_data{nullptr};
};

// ----------------------------------------------------------------------------
// wxImageDataObject - data object for wxImage
// ----------------------------------------------------------------------------

class wxImageDataObject : public wxCustomDataObject
{
public:
    explicit wxImageDataObject(const wxImage& image = wxNullImage);

    wxImageDataObject& operator=(wxImageDataObject&&) = delete;

    void SetImage(const wxImage& image);
    wxImage GetImage() const;
};

// ----------------------------------------------------------------------------
// include platform-specific declarations of wxXXXBase classes
// ----------------------------------------------------------------------------

#if defined(__WXMSW__)
    #include "wx/msw/ole/dataobj2.h"
    // wxURLDataObject defined in msw/ole/dataobj2.h
#elif defined(__WXGTK20__)
    #include "wx/gtk/dataobj2.h"
    // wxURLDataObject defined in gtk/dataobj2.h

#else
    #if defined(__WXGTK__)
        #include "wx/gtk1/dataobj2.h"
    #elif defined(__WXX11__)
        #include "wx/x11/dataobj2.h"
    #elif defined(__WXMOTIF__)
        #include "wx/motif/dataobj2.h"
    #elif defined(__WXMAC__)
        #include "wx/osx/dataobj2.h"
    #elif defined(__WXQT__)
        #include "wx/qt/dataobj2.h"
    #endif

    // wxURLDataObject is simply wxTextDataObject with a different name
    class wxURLDataObject : public wxTextDataObject
    {
    public:
        wxURLDataObject(const wxString& url = {})
            : wxTextDataObject(url)
        {
        }

        wxString GetURL() const { return GetText(); }
        void SetURL(const wxString& url) { SetText(url); }
    };
#endif

#endif // wxUSE_DATAOBJ

#endif // _WX_DATAOBJ_H_BASE_
