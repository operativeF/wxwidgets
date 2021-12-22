/////////////////////////////////////////////////////////////////////////////
// Name:        wx/zipstrm.h
// Purpose:     Streams for Zip files
// Author:      Mike Wetherell
// Copyright:   (c) Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/archive.h"
#include "wx/filename.h"

export module WX.Cmn.ZipStream;

import <cstdint>;

#if wxUSE_ZIPSTREAM

export
{

// some methods from wxZipInputStream and wxZipOutputStream stream do not get
// exported/imported when compiled with Mingw versions before 3.4.2. So they
// are imported/exported individually as a workaround
#if (defined(__GNUWIN32__) || defined(__MINGW32__)) \
    && (!defined __GNUC__ \
       || !defined __GNUC_MINOR__ \
       || !defined __GNUC_PATCHLEVEL__ \
       || __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ < 30402)
#define WXZIPFIX
#else
#define WXZIPFIX
#endif

/////////////////////////////////////////////////////////////////////////////
// constants

// Compression Method, only 0 (store) and 8 (deflate) are supported here
//
enum wxZipMethod
{
    wxZIP_METHOD_STORE,
    wxZIP_METHOD_SHRINK,
    wxZIP_METHOD_REDUCE1,
    wxZIP_METHOD_REDUCE2,
    wxZIP_METHOD_REDUCE3,
    wxZIP_METHOD_REDUCE4,
    wxZIP_METHOD_IMPLODE,
    wxZIP_METHOD_TOKENIZE,
    wxZIP_METHOD_DEFLATE,
    wxZIP_METHOD_DEFLATE64,
    wxZIP_METHOD_BZIP2 = 12,
    wxZIP_METHOD_DEFAULT = 0xffff
};

// Originating File-System.
//
// These are Pkware's values. Note that Info-zip disagree on some of them,
// most notably NTFS.
//
enum wxZipSystem
{
    wxZIP_SYSTEM_MSDOS,
    wxZIP_SYSTEM_AMIGA,
    wxZIP_SYSTEM_OPENVMS,
    wxZIP_SYSTEM_UNIX,
    wxZIP_SYSTEM_VM_CMS,
    wxZIP_SYSTEM_ATARI_ST,
    wxZIP_SYSTEM_OS2_HPFS,
    wxZIP_SYSTEM_MACINTOSH,
    wxZIP_SYSTEM_Z_SYSTEM,
    wxZIP_SYSTEM_CPM,
    wxZIP_SYSTEM_WINDOWS_NTFS,
    wxZIP_SYSTEM_MVS,
    wxZIP_SYSTEM_VSE,
    wxZIP_SYSTEM_ACORN_RISC,
    wxZIP_SYSTEM_VFAT,
    wxZIP_SYSTEM_ALTERNATE_MVS,
    wxZIP_SYSTEM_BEOS,
    wxZIP_SYSTEM_TANDEM,
    wxZIP_SYSTEM_OS_400
};

// Dos/Win file attributes
//
enum wxZipAttributes
{
    wxZIP_A_RDONLY = 0x01,
    wxZIP_A_HIDDEN = 0x02,
    wxZIP_A_SYSTEM = 0x04,
    wxZIP_A_SUBDIR = 0x10,
    wxZIP_A_ARCH   = 0x20,

    wxZIP_A_MASK   = 0x37
};

// Values for the flags field in the zip headers
//
enum wxZipFlags
{
    wxZIP_ENCRYPTED         = 0x0001,
    wxZIP_DEFLATE_NORMAL    = 0x0000,   // normal compression
    wxZIP_DEFLATE_EXTRA     = 0x0002,   // extra compression
    wxZIP_DEFLATE_FAST      = 0x0004,   // fast compression
    wxZIP_DEFLATE_SUPERFAST = 0x0006,   // superfast compression
    wxZIP_DEFLATE_MASK      = 0x0006,
    wxZIP_SUMS_FOLLOW       = 0x0008,   // crc and sizes come after the data
    wxZIP_ENHANCED          = 0x0010,
    wxZIP_PATCH             = 0x0020,
    wxZIP_STRONG_ENC        = 0x0040,
    wxZIP_LANG_ENC_UTF8     = 0x0800,   // filename and comment are UTF8
    wxZIP_UNUSED            = 0x0F80,
    wxZIP_RESERVED          = 0xF000
};

enum wxZipArchiveFormat
{
    /// Default zip format
    wxZIP_FORMAT_DEFAULT,
    /// ZIP64 format
    wxZIP_FORMAT_ZIP64
};

// value for the 'version needed to extract' field (20 means 2.0)
enum {
    VERSION_NEEDED_TO_EXTRACT = 20,
    Z64_VERSION_NEEDED_TO_EXTRACT = 45 // File uses ZIP64 format extensions
};

// Forward decls
//
class wxZipEntry;
class wxZipInputStream;


/////////////////////////////////////////////////////////////////////////////
// wxZipNotifier

class wxZipNotifier
{
public:
    virtual ~wxZipNotifier() = default;

    virtual void OnEntryUpdated(wxZipEntry& entry) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// Zip Entry - holds the meta data for a file in the zip

class wxDataOutputStream;

class wxZipEntry : public wxArchiveEntry
{
public:
    wxZipEntry(const wxString& name = {},
               const wxDateTime& dt = wxDateTime::Now(),
               wxFileOffset size = wxInvalidOffset);
    ~wxZipEntry();

    wxZipEntry(const wxZipEntry& entry);
    wxZipEntry& operator=(const wxZipEntry& entry);

    // Get accessors
    wxDateTime   GetDateTime() const override            { return m_DateTime; }
    wxFileOffset GetSize() const override                { return m_Size; }
    wxFileOffset GetOffset() const override              { return m_Offset; }
    wxString     GetInternalName() const override        { return m_Name; }
    int          GetMethod() const              { return m_Method; }
    int          GetFlags() const               { return m_Flags; }
    std::uint32_t     GetCrc() const                 { return m_Crc; }
    wxFileOffset GetCompressedSize() const      { return m_CompressedSize; }
    int          GetSystemMadeBy() const        { return m_SystemMadeBy; }
    wxString     GetComment() const             { return m_Comment; }
    std::uint32_t     GetExternalAttributes() const  { return m_ExternalAttributes; }
    wxPathFormat GetInternalFormat() const override      { return wxPATH_UNIX; }
    int          GetMode() const;
    const char  *GetLocalExtra() const;
    size_t       GetLocalExtraLen() const;
    const char  *GetExtra() const;
    size_t       GetExtraLen() const;
    wxString     GetName(wxPathFormat format = wxPATH_NATIVE) const override;

    // is accessors
    inline bool IsDir() const override;
    inline bool IsText() const;
    inline bool IsReadOnly() const override;
    inline bool IsMadeByUnix() const;

    // set accessors
    void SetDateTime(const wxDateTime& dt) override      { m_DateTime = dt; }
    void SetSize(wxFileOffset size) override             { m_Size = size; }
    void SetMethod(int method)                  { m_Method = (std::uint16_t)method; }
    void SetComment(const wxString& comment)    { m_Comment = comment; }
    void SetExternalAttributes(std::uint32_t attr )  { m_ExternalAttributes = attr; }
    void SetSystemMadeBy(int system);
    void SetMode(int mode);
    void SetExtra(const char *extra, size_t len);
    void SetLocalExtra(const char *extra, size_t len);

    inline void SetName(const wxString& name,
                        wxPathFormat format = wxPATH_NATIVE) override;

    static wxString GetInternalName(const wxString& name,
                                    wxPathFormat format = wxPATH_NATIVE,
                                    bool *pIsDir = nullptr);

    // set is accessors
    void SetIsDir(bool isDir = true) override;
    inline void SetIsReadOnly(bool isReadOnly = true) override;
    inline void SetIsText(bool isText = true);

    wxZipEntry *Clone() const                   { return ZipClone(); }

    void SetNotifier(wxZipNotifier& notifier);
    void UnsetNotifier() override;

protected:
    // Internal attributes
    static constexpr auto TEXT_ATTR = 1;

    // protected Get accessors
    int GetVersionNeeded() const                { return m_VersionNeeded; }
    wxFileOffset GetKey() const                 { return m_Key; }
    int GetVersionMadeBy() const                { return m_VersionMadeBy; }
    int GetDiskStart() const                    { return m_DiskStart; }
    int GetInternalAttributes() const           { return m_InternalAttributes; }

    void SetVersionNeeded(int version)          { m_VersionNeeded = (std::uint16_t)version; }
    void SetOffset(wxFileOffset offset) override         { m_Offset = offset; }
    void SetFlags(int flags)                    { m_Flags = (std::uint16_t)flags; }
    void SetVersionMadeBy(int version)          { m_VersionMadeBy = (std::uint8_t)version; }
    void SetCrc(std::uint32_t crc)                   { m_Crc = crc; }
    void SetCompressedSize(wxFileOffset size)   { m_CompressedSize = size; }
    void SetKey(wxFileOffset offset)            { m_Key = offset; }
    void SetDiskStart(int start)                { m_DiskStart = (std::uint16_t)start; }
    void SetInternalAttributes(int attr)        { m_InternalAttributes = (std::uint16_t)attr; }

    virtual wxZipEntry *ZipClone() const        { return new wxZipEntry(*this); }

    void Notify();

private:
    wxArchiveEntry* DoClone() const override             { return ZipClone(); }

    size_t ReadLocal(wxInputStream& stream, wxMBConv& conv);
    size_t WriteLocal(wxOutputStream& stream, wxMBConv& conv, wxZipArchiveFormat zipFormat);

    size_t ReadCentral(wxInputStream& stream, wxMBConv& conv);
    size_t WriteCentral(wxOutputStream& stream, wxMBConv& conv) const;

    size_t ReadDescriptor(wxInputStream& stream);
    size_t WriteDescriptor(wxOutputStream& stream, std::uint32_t crc,
                           wxFileOffset compressedSize, wxFileOffset size);

    void WriteLocalFileSizes(wxDataOutputStream& ds) const;
    void WriteLocalZip64ExtraInfo(wxOutputStream& stream) const;

    bool LoadExtraInfo(const char* extraData, std::uint16_t extraLen, bool localInfo);

    std::uint16_t GetInternalFlags(bool checkForUTF8) const;

    std::uint8_t      m_SystemMadeBy{wxZIP_SYSTEM_MSDOS};       // one of enum wxZipSystem
    std::uint8_t      m_VersionMadeBy;      // major * 10 + minor

    std::uint16_t     m_VersionNeeded{VERSION_NEEDED_TO_EXTRACT};      // ver needed to extract (20 i.e. v2.0)
    std::uint16_t     m_Flags{0};
    std::uint16_t     m_Method{wxZIP_METHOD_DEFAULT};             // compression method (one of wxZipMethod)
    wxDateTime   m_DateTime;
    std::uint32_t     m_Crc{0};
    wxFileOffset m_CompressedSize;
    wxFileOffset m_Size;
    wxString     m_Name;               // in internal format
    wxFileOffset m_Key;                // the original offset for copied entries
    wxFileOffset m_Offset;             // file offset of the entry

    wxString     m_Comment;
    std::uint16_t     m_DiskStart{0};          // for multidisk archives, not unsupported
    std::uint16_t     m_InternalAttributes{0}; // bit 0 set for text files
    std::uint32_t     m_ExternalAttributes{0}; // system specific depends on SystemMadeBy
    std::uint16_t     m_z64infoOffset{0};      // Offset of ZIP64 local extra data for file sizes

    class wxZipMemory *m_Extra{nullptr};
    class wxZipMemory *m_LocalExtra{nullptr};

    wxZipNotifier *m_zipnotifier{nullptr};
    class wxZipWeakLinks *m_backlink{nullptr};

    friend class wxZipInputStream;
    friend class wxZipOutputStream;

    wxDECLARE_DYNAMIC_CLASS(wxZipEntry);
};


/////////////////////////////////////////////////////////////////////////////
// wxZipOutputStream

WX_DECLARE_LIST_WITH_DECL(wxZipEntry, wxZipEntryList_, class);

class wxZipOutputStream : public wxArchiveOutputStream
{
public:
    wxZipOutputStream(wxOutputStream& stream,
                      int level = -1,
                      wxMBConv& conv = wxConvUTF8);
    wxZipOutputStream(wxOutputStream *stream,
                      int level = -1,
                      wxMBConv& conv = wxConvUTF8);
    WXZIPFIX ~wxZipOutputStream();

    wxZipOutputStream& operator=(wxZipOutputStream&&) = delete;

    bool PutNextEntry(wxZipEntry *entry)        { return DoCreate(entry); }

    bool WXZIPFIX PutNextEntry(const wxString& name,
                               const wxDateTime& dt = wxDateTime::Now(),
                               wxFileOffset size = wxInvalidOffset) override;

    bool WXZIPFIX PutNextDirEntry(const wxString& name,
                                  const wxDateTime& dt = wxDateTime::Now()) override;

    bool WXZIPFIX CopyEntry(wxZipEntry *entry, wxZipInputStream& inputStream);
    bool WXZIPFIX CopyArchiveMetaData(wxZipInputStream& inputStream);

    void WXZIPFIX Sync() override;
    bool WXZIPFIX CloseEntry() override;
    bool WXZIPFIX Close() override;

    void SetComment(const wxString& comment)    { m_Comment = comment; }

    int  GetLevel() const                       { return m_level; }
    void WXZIPFIX SetLevel(int level);

    void SetFormat(wxZipArchiveFormat format)   { m_format = format; }
    wxZipArchiveFormat GetFormat() const        { return m_format; }

protected:
    size_t WXZIPFIX OnSysWrite(const void *buffer, size_t size) override;
    wxFileOffset OnSysTell() const override      { return m_entrySize; }

    // this protected interface isn't yet finalised
    struct Buffer { const char *m_data; size_t m_size; };
    virtual wxOutputStream* WXZIPFIX OpenCompressor(wxOutputStream& stream,
                                                    wxZipEntry& entry,
                                                    const Buffer bufs[]);
    virtual bool WXZIPFIX CloseCompressor(wxOutputStream *comp);

    bool IsParentSeekable() const
        { return m_offsetAdjustment != wxInvalidOffset; }

private:
    bool WXZIPFIX PutNextEntry(wxArchiveEntry *entry) override;
    bool WXZIPFIX CopyEntry(wxArchiveEntry *entry, wxArchiveInputStream& stream) override;
    bool WXZIPFIX CopyArchiveMetaData(wxArchiveInputStream& stream) override;

    bool IsOpened() const { return m_comp || m_pending; }

    bool DoCreate(wxZipEntry *entry, bool raw = false);
    void CreatePendingEntry(const void *buffer, size_t size);
    void CreatePendingEntry();

    class wxStoredOutputStream *m_store;
    class wxZlibOutputStream2 *m_deflate{nullptr};
    class wxZipStreamLink *m_backlink{nullptr};
    wxZipEntryList_ m_entries;
    char *m_initialData;
    size_t m_initialSize{0};
    wxZipEntry *m_pending{nullptr};
    bool m_raw{false};
    wxFileOffset m_headerOffset{0};
    size_t m_headerSize{0};
    wxFileOffset m_entrySize{0};
    std::uint32_t m_crcAccumulator;
    wxOutputStream *m_comp{nullptr};
    int m_level;
    wxFileOffset m_offsetAdjustment{wxInvalidOffset};
    wxString m_Comment;
    bool m_endrecWritten{false};
    wxZipArchiveFormat m_format{wxZIP_FORMAT_DEFAULT};
};


/////////////////////////////////////////////////////////////////////////////
// wxZipInputStream

class wxZipInputStream : public wxArchiveInputStream
{
public:
    using entry_type = wxZipEntry;

    wxZipInputStream(wxInputStream& stream, wxMBConv& conv = wxConvLocal);
    wxZipInputStream(wxInputStream *stream, wxMBConv& conv = wxConvLocal);

    WXZIPFIX ~wxZipInputStream();

    wxZipInputStream& operator=(wxZipInputStream&&) = delete;

    bool OpenEntry(wxZipEntry& entry)   { return DoOpen(&entry); }
    bool WXZIPFIX CloseEntry() override;

    wxZipEntry *GetNextEntry();

    wxString WXZIPFIX GetComment();
    int WXZIPFIX GetTotalEntries();

    wxFileOffset GetLength() const override { return m_entry.GetSize(); }

protected:
    size_t WXZIPFIX OnSysRead(void *buffer, size_t size) override;
    wxFileOffset OnSysTell() const override { return m_decomp ? m_decomp->TellI() : 0; }

    // this protected interface isn't yet finalised
    virtual wxInputStream* WXZIPFIX OpenDecompressor(wxInputStream& stream);
    virtual bool WXZIPFIX CloseDecompressor(wxInputStream *decomp);

private:
    
    void Init(const wxString& file);

    wxArchiveEntry *DoGetNextEntry() override    { return GetNextEntry(); }

    bool WXZIPFIX OpenEntry(wxArchiveEntry& entry) override;

    wxStreamError ReadLocal(bool readEndRec = false);
    wxStreamError ReadCentral();

    std::uint32_t ReadSignature();
    bool FindEndRecord();
    bool LoadEndRecord();

    bool AtHeader() const       { return m_headerSize == 0; }
    bool AfterHeader() const    { return m_headerSize > 0 && !m_decomp; }
    bool IsOpened() const       { return m_decomp != nullptr; }

    wxZipStreamLink *MakeLink(wxZipOutputStream *out);

    bool DoOpen(wxZipEntry *entry = nullptr, bool raw = false);
    bool OpenDecompressor(bool raw = false);

    class wxStoredInputStream *m_store;
    class wxZlibInputStream2 *m_inflate;
    class wxRawInputStream *m_rawin;
    wxZipEntry m_entry;
    bool m_raw;
    size_t m_headerSize;
    std::uint32_t m_crcAccumulator;
    wxInputStream *m_decomp;
    bool m_parentSeekable;
    class wxZipWeakLinks *m_weaklinks;
    class wxZipStreamLink *m_streamlink;
    wxFileOffset m_offsetAdjustment;
    wxFileOffset m_position;
    std::uint32_t m_signature;
    size_t m_TotalEntries;
    wxString m_Comment;

    friend bool wxZipOutputStream::CopyEntry(
                    wxZipEntry *entry, wxZipInputStream& inputStream);
    friend bool wxZipOutputStream::CopyArchiveMetaData(
                    wxZipInputStream& inputStream);
};


/////////////////////////////////////////////////////////////////////////////
// Iterators

#if defined WX_TEST_ARCHIVE_ITERATOR
using wxZipIter = wxArchiveIterator<wxZipInputStream>;
using wxZipPairIter = wxArchiveIterator<wxZipInputStream, std::pair<wxString, wxZipEntry *>>;
#endif


/////////////////////////////////////////////////////////////////////////////
// wxZipClassFactory

class wxZipClassFactory : public wxArchiveClassFactory
{
public:
    using entry_type = wxZipEntry;
    using instream_type = wxZipInputStream;
    using outstream_type = wxZipOutputStream;
    using notifier_type = wxZipNotifier;
#if defined WX_TEST_ARCHIVE_ITERATOR
    using iter_type = wxZipIter;
    using pairiter_type = wxZipPairIter;
#endif

    wxZipClassFactory();

    wxZipEntry *NewEntry() const
        { return new wxZipEntry; }
    wxZipInputStream *NewStream(wxInputStream& stream) const
        { return new wxZipInputStream(stream, GetConv()); }
    wxZipOutputStream *NewStream(wxOutputStream& stream) const
        { return new wxZipOutputStream(stream, -1, GetConv()); }
    wxZipInputStream *NewStream(wxInputStream *stream) const
        { return new wxZipInputStream(stream, GetConv()); }
    wxZipOutputStream *NewStream(wxOutputStream *stream) const
        { return new wxZipOutputStream(stream, -1, GetConv()); }

    wxString GetInternalName(const wxString& name,
                             wxPathFormat format = wxPATH_NATIVE) const override
        { return wxZipEntry::GetInternalName(name, format); }

    const wxChar * const *GetProtocols(wxStreamProtocolType type
                                       = wxSTREAM_PROTOCOL) const override;

protected:
    wxArchiveEntry *DoNewEntry() const override
        { return NewEntry(); }
    wxArchiveInputStream *DoNewStream(wxInputStream& stream) const override
        { return NewStream(stream); }
    wxArchiveOutputStream *DoNewStream(wxOutputStream& stream) const override
        { return NewStream(stream); }
    wxArchiveInputStream *DoNewStream(wxInputStream *stream) const override
        { return NewStream(stream); }
    wxArchiveOutputStream *DoNewStream(wxOutputStream *stream) const override
        { return NewStream(stream); }
};


/////////////////////////////////////////////////////////////////////////////
// wxZipEntry inlines

inline bool wxZipEntry::IsText() const
{
    return (m_InternalAttributes & TEXT_ATTR) != 0;
}

inline bool wxZipEntry::IsDir() const
{
    return (m_ExternalAttributes & wxZIP_A_SUBDIR) != 0;
}

inline bool wxZipEntry::IsReadOnly() const
{
    return (m_ExternalAttributes & wxZIP_A_RDONLY) != 0;
}

inline bool wxZipEntry::IsMadeByUnix() const
{
    switch ( m_SystemMadeBy )
    {
        case wxZIP_SYSTEM_MSDOS:
            // note: some unix zippers put madeby = dos
            return (m_ExternalAttributes & ~0xFFFF) != 0;

        case wxZIP_SYSTEM_OPENVMS:
        case wxZIP_SYSTEM_UNIX:
        case wxZIP_SYSTEM_ATARI_ST:
        case wxZIP_SYSTEM_ACORN_RISC:
        case wxZIP_SYSTEM_BEOS:
        case wxZIP_SYSTEM_TANDEM:
            return true;

        case wxZIP_SYSTEM_AMIGA:
        case wxZIP_SYSTEM_VM_CMS:
        case wxZIP_SYSTEM_OS2_HPFS:
        case wxZIP_SYSTEM_MACINTOSH:
        case wxZIP_SYSTEM_Z_SYSTEM:
        case wxZIP_SYSTEM_CPM:
        case wxZIP_SYSTEM_WINDOWS_NTFS:
        case wxZIP_SYSTEM_MVS:
        case wxZIP_SYSTEM_VSE:
        case wxZIP_SYSTEM_VFAT:
        case wxZIP_SYSTEM_ALTERNATE_MVS:
        case wxZIP_SYSTEM_OS_400:
            return false;
    }

    // Unknown system, assume not Unix.
    return false;
}

inline void wxZipEntry::SetIsText(bool isText)
{
    if (isText)
        m_InternalAttributes |= TEXT_ATTR;
    else
        m_InternalAttributes &= ~TEXT_ATTR;
}

inline void wxZipEntry::SetIsReadOnly(bool isReadOnly)
{
    if (isReadOnly)
        SetMode(GetMode() & ~0222);
    else
        SetMode(GetMode() | 0200);
}

inline void wxZipEntry::SetName(const wxString& name,
                                wxPathFormat format /*=wxPATH_NATIVE*/)
{
    bool isDir;
    m_Name = GetInternalName(name, format, &isDir);
    SetIsDir(isDir);
}

} // export

#endif // wxUSE_ZIPSTREAM
