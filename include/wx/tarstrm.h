/////////////////////////////////////////////////////////////////////////////
// Name:        wx/tarstrm.h
// Purpose:     Streams for Tar files
// Author:      Mike Wetherell
// Copyright:   (c) 2004 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WXTARSTREAM_H__
#define _WX_WXTARSTREAM_H__

#include "wx/archive.h"
#include "wx/hashmap.h"

import WX.File.Flags;

import <cstdint>;
import <utility>;


/////////////////////////////////////////////////////////////////////////////
// Constants

// TypeFlag values
enum wxTarType
{
    wxTAR_REGTYPE   = '0',      // regular file
    wxTAR_LNKTYPE   = '1',      // hard link
    wxTAR_SYMTYPE   = '2',      // symbolic link
    wxTAR_CHRTYPE   = '3',      // character special
    wxTAR_BLKTYPE   = '4',      // block special
    wxTAR_DIRTYPE   = '5',      // directory
    wxTAR_FIFOTYPE  = '6',      // named pipe
    wxTAR_CONTTYPE  = '7'       // contiguous file
};

// Archive Formats (use wxTAR_PAX, it's backward compatible)
enum wxTarFormat
{
    wxTAR_USTAR,                // POSIX.1-1990 tar format
    wxTAR_PAX                   // POSIX.1-2001 tar format
};


/////////////////////////////////////////////////////////////////////////////
// wxTarNotifier

class wxTarNotifier
{
public:
    virtual ~wxTarNotifier() = default;

    virtual void OnEntryUpdated(class wxTarEntry& entry) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// Tar Entry - hold the meta data for a file in the tar

class wxTarEntry : public wxArchiveEntry
{
public:
    wxTarEntry(const wxString& name = {},
               const wxDateTime& dt = wxDateTime::Now(),
               wxFileOffset size = wxInvalidOffset);

    // Get accessors
    wxString     GetName(wxPathFormat format = wxPATH_NATIVE) const override;
    wxString     GetInternalName() const override        { return m_Name; }
    wxPathFormat GetInternalFormat() const override      { return wxPATH_UNIX; }
    int          GetMode() const;
    int          GetUserId() const              { return m_UserId; }
    int          GetGroupId() const             { return m_GroupId; }
    wxFileOffset GetSize() const override                { return m_Size; }
    wxFileOffset GetOffset() const override              { return m_Offset; }
    wxDateTime   GetDateTime() const override            { return m_ModifyTime; }
    wxDateTime   GetAccessTime() const          { return m_AccessTime; }
    wxDateTime   GetCreateTime() const          { return m_CreateTime; }
    int          GetTypeFlag() const            { return m_TypeFlag; }
    wxString     GetLinkName() const            { return m_LinkName; }
    wxString     GetUserName() const            { return m_UserName; }
    wxString     GetGroupName() const           { return m_GroupName; }
    int          GetDevMajor() const            { return m_DevMajor; }
    int          GetDevMinor() const            { return m_DevMinor; }

    // is accessors
    bool IsDir() const override;
    bool IsReadOnly() const override                     { return !(m_Mode & 0222); }

    // set accessors
    void SetName(const wxString& name, wxPathFormat format = wxPATH_NATIVE) override;
    void SetUserId(int id)                      { m_UserId = id; }
    void SetGroupId(int id)                     { m_GroupId = id; }
    void SetMode(int mode);
    void SetSize(wxFileOffset size) override             { m_Size = size; }
    void SetDateTime(const wxDateTime& dt) override      { m_ModifyTime = dt; }
    void SetAccessTime(const wxDateTime& dt)    { m_AccessTime = dt; }
    void SetCreateTime(const wxDateTime& dt)    { m_CreateTime = dt; }
    void SetTypeFlag(int type)                  { m_TypeFlag = type; }
    void SetLinkName(const wxString& link)      { m_LinkName = link; }
    void SetUserName(const wxString& user)      { m_UserName = user; }
    void SetGroupName(const wxString& group)    { m_GroupName = group; }
    void SetDevMajor(int dev)                   { m_DevMajor = dev; }
    void SetDevMinor(int dev)                   { m_DevMinor = dev; }

    // set is accessors
    void SetIsDir(bool isDir = true) override;
    void SetIsReadOnly(bool isReadOnly = true) override;

    static wxString GetInternalName(const wxString& name,
                                    wxPathFormat format = wxPATH_NATIVE,
                                    bool *pIsDir = nullptr);

    wxTarEntry *Clone() const { return new wxTarEntry(*this); }

    void SetNotifier([[maybe_unused]] wxTarNotifier& notifier) { }

private:
    void SetOffset(wxFileOffset offset) override         { m_Offset = offset; }

    wxArchiveEntry* DoClone() const override     { return Clone(); }

    wxString     m_Name;
    int          m_Mode{0644};
    bool         m_IsModeSet{false};
    int          m_UserId;
    int          m_GroupId;
    wxFileOffset m_Size;
    wxFileOffset m_Offset;
    wxDateTime   m_ModifyTime;
    wxDateTime   m_AccessTime;
    wxDateTime   m_CreateTime;
    int          m_TypeFlag{wxTAR_REGTYPE};
    wxString     m_LinkName;
    wxString     m_UserName;
    wxString     m_GroupName;
    int          m_DevMajor;
    int          m_DevMinor;

    friend class wxTarInputStream;

    wxDECLARE_DYNAMIC_CLASS(wxTarEntry);
};


/////////////////////////////////////////////////////////////////////////////
// wxTarInputStream

WX_DECLARE_STRING_HASH_MAP(wxString, wxTarHeaderRecords);

class wxTarInputStream : public wxArchiveInputStream
{
public:
    using entry_type = wxTarEntry;

    wxTarInputStream(wxInputStream& stream, wxMBConv& conv = wxConvLocal);
    wxTarInputStream(wxInputStream *stream, wxMBConv& conv = wxConvLocal);
    ~wxTarInputStream();

    wxTarInputStream& operator=(wxTarInputStream&&) = delete;

    bool OpenEntry(wxTarEntry& entry);
    bool CloseEntry() override;

    wxTarEntry *GetNextEntry();

    wxFileOffset GetLength() const override      { return m_size; }
    bool IsSeekable() const override { return m_parent_i_stream->IsSeekable(); }

protected:
    size_t OnSysRead(void *buffer, size_t size) override;
    wxFileOffset OnSysTell() const override      { return m_pos; }
    wxFileOffset OnSysSeek(wxFileOffset seek, wxSeekMode mode) override;

private:
    wxArchiveEntry *DoGetNextEntry() override    { return GetNextEntry(); }
    bool OpenEntry(wxArchiveEntry& entry) override;
    bool IsOpened() const               { return m_pos != wxInvalidOffset; }

    wxStreamError ReadHeaders();
    bool ReadExtendedHeader(wxTarHeaderRecords*& recs);

    wxString GetExtendedHeader(const wxString& key) const;
    wxString GetHeaderPath() const;
    wxFileOffset GetHeaderNumber(int id) const;
    wxString GetHeaderString(int id) const;
    wxDateTime GetHeaderDate(const wxString& key) const;

    wxFileOffset m_pos;     // position within the current entry
    wxFileOffset m_offset;  // offset to the start of the entry's data
    wxFileOffset m_size;    // size of the current entry's data

    int m_sumType;
    int m_tarType;
    class wxTarHeaderBlock *m_hdr;
    wxTarHeaderRecords *m_HeaderRecs;
    wxTarHeaderRecords *m_GlobalHeaderRecs;
};


/////////////////////////////////////////////////////////////////////////////
// wxTarOutputStream

class wxTarOutputStream : public wxArchiveOutputStream
{
public:
    wxTarOutputStream(wxOutputStream& stream,
                      wxTarFormat format = wxTAR_PAX,
                      wxMBConv& conv = wxConvLocal);
    wxTarOutputStream(wxOutputStream *stream,
                      wxTarFormat format = wxTAR_PAX,
                      wxMBConv& conv = wxConvLocal);
    ~wxTarOutputStream();

    wxTarOutputStream& operator=(wxTarOutputStream&&) = delete;

    bool PutNextEntry(wxTarEntry *entry);

    bool PutNextEntry(const wxString& name,
                      const wxDateTime& dt = wxDateTime::Now(),
                      wxFileOffset size = wxInvalidOffset) override;

    bool PutNextDirEntry(const wxString& name,
                         const wxDateTime& dt = wxDateTime::Now()) override;

    bool CopyEntry(wxTarEntry *entry, wxTarInputStream& inputStream);
    bool CopyArchiveMetaData([[maybe_unused]] wxTarInputStream& s) { return true; }

    void Sync() override;
    bool CloseEntry() override;
    bool Close() override;

    bool IsSeekable() const override { return m_parent_o_stream->IsSeekable(); }

    void SetBlockingFactor(int factor)  { m_BlockingFactor = factor; }
    int GetBlockingFactor() const       { return m_BlockingFactor; }

protected:
    size_t OnSysWrite(const void *buffer, size_t size) override;
    wxFileOffset OnSysTell() const override      { return m_pos; }
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;

private:
    void Init(wxTarFormat format);

    bool PutNextEntry(wxArchiveEntry *entry) override;
    bool CopyEntry(wxArchiveEntry *entry, wxArchiveInputStream& stream) override;
    bool CopyArchiveMetaData([[maybe_unused]] wxArchiveInputStream& s) override { return true; }
    bool IsOpened() const               { return m_pos != wxInvalidOffset; }

    bool WriteHeaders(wxTarEntry& entry);
    bool ModifyHeader();
    wxString PaxHeaderPath(const wxString& format, const wxString& path);

    void SetExtendedHeader(const wxString& key, const wxString& value);
    void SetHeaderPath(const wxString& name);
    bool SetHeaderNumber(int id, wxFileOffset n);
    void SetHeaderString(int id, const wxString& str);
    void SetHeaderDate(const wxString& key, const wxDateTime& datetime);

    wxFileOffset m_pos;     // position within the current entry
    wxFileOffset m_maxpos;  // max pos written
    wxFileOffset m_size;    // expected entry size

    wxFileOffset m_headpos; // offset within the file to the entry's header
    wxFileOffset m_datapos; // offset within the file to the entry's data

    wxFileOffset m_tarstart;// offset within the file to the tar
    wxFileOffset m_tarsize; // size of tar so far

    bool m_pax;
    int m_BlockingFactor;
    std::uint32_t m_chksum;
    bool m_large;
    class wxTarHeaderBlock *m_hdr;
    class wxTarHeaderBlock *m_hdr2;
    char *m_extendedHdr;
    size_t m_extendedSize;
    wxString m_badfit;
    bool m_endrecWritten;
};


/////////////////////////////////////////////////////////////////////////////
// Iterators

#if defined WX_TEST_ARCHIVE_ITERATOR
using wxTarIter = wxArchiveIterator<wxTarInputStream>;
using wxTarPairIter = wxArchiveIterator<wxTarInputStream, std::pair<wxString, wxTarEntry *>>;
#endif


/////////////////////////////////////////////////////////////////////////////
// wxTarClassFactory

class wxTarClassFactory : public wxArchiveClassFactory
{
public:
    using entry_type = wxTarEntry;
    using instream_type = wxTarInputStream;
    using outstream_type = wxTarOutputStream;
    using notifier_type = wxTarNotifier;
#if defined WX_TEST_ARCHIVE_ITERATOR
    using iter_type = wxTarIter;
    using pairiter_type = wxTarPairIter;
#endif

    wxTarClassFactory();

    wxTarEntry *NewEntry() const
        { return new wxTarEntry; }
    wxTarInputStream *NewStream(wxInputStream& stream) const
        { return new wxTarInputStream(stream, GetConv()); }
    wxTarOutputStream *NewStream(wxOutputStream& stream) const
        { return new wxTarOutputStream(stream, wxTAR_PAX, GetConv()); }
    wxTarInputStream *NewStream(wxInputStream *stream) const
        { return new wxTarInputStream(stream, GetConv()); }
    wxTarOutputStream *NewStream(wxOutputStream *stream) const
        { return new wxTarOutputStream(stream, wxTAR_PAX, GetConv()); }

    wxString GetInternalName(const wxString& name,
                             wxPathFormat format = wxPATH_NATIVE) const override
        { return wxTarEntry::GetInternalName(name, format); }

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

#endif // _WX_WXTARSTREAM_H__
