/////////////////////////////////////////////////////////////////////////////
// Name:        wx/zstream.h
// Purpose:     Memory stream classes
// Author:      Guilhem Lavaux
// Modified by: Mike Wetherell
// Created:     11/07/98
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

export module WX.Cmn.ZStream;

import WX.Cmn.Stream;

import WX.Utils.VersionInfo;

#if wxUSE_ZLIB && wxUSE_STREAMS

export
{

// Compression level
enum wxZlibCompressionLevels {
    wxZ_DEFAULT_COMPRESSION = -1,
    wxZ_NO_COMPRESSION = 0,
    wxZ_BEST_SPEED = 1,
    wxZ_BEST_COMPRESSION = 9
};

// Flags
enum wxZLibFlags {
    wxZLIB_NO_HEADER = 0,    // raw deflate stream, no header or checksum
    wxZLIB_ZLIB = 1,         // zlib header and checksum
    wxZLIB_GZIP = 2,         // gzip header and checksum, requires zlib 1.2.1+
    wxZLIB_AUTO = 3          // autodetect header zlib or gzip
};

class wxZlibInputStream: public wxFilterInputStream {
 public:
 // FIXME: Redundant construction via ptr / ref?
  wxZlibInputStream(wxInputStream& stream, int flags = wxZLIB_AUTO);
  wxZlibInputStream(wxInputStream *stream, int flags = wxZLIB_AUTO);
  ~wxZlibInputStream();

  wxZlibInputStream& operator=(wxZlibInputStream&&) = delete;

  char Peek() override { return wxInputStream::Peek(); }
  wxFileOffset GetLength() const override { return wxInputStream::GetLength(); }

  static bool CanHandleGZip();

  bool SetDictionary(const char *data, size_t datalen);
  bool SetDictionary(const wxMemoryBuffer &buf);

 protected:
  size_t OnSysRead(void *buffer, size_t size) override;
  wxFileOffset OnSysTell() const override { return m_pos; }

 private:
  void Init(int flags);

 protected:
  size_t m_z_size;
  unsigned char *m_z_buffer;
  struct z_stream_s *m_inflate;
  wxFileOffset m_pos;
};

class wxZlibOutputStream: public wxFilterOutputStream {
 public:
  wxZlibOutputStream(wxOutputStream& stream, int level = -1, int flags = wxZLIB_ZLIB);
  wxZlibOutputStream(wxOutputStream *stream, int level = -1, int flags = wxZLIB_ZLIB);
  ~wxZlibOutputStream() { Close(); }

  wxZlibOutputStream& operator=(wxZlibOutputStream&&) = delete;

  void Sync() override { DoFlush(false); }
  bool Close() override;
  wxFileOffset GetLength() const override { return m_pos; }

  static bool CanHandleGZip();

  bool SetDictionary(const char *data, size_t datalen);
  bool SetDictionary(const wxMemoryBuffer &buf);

 protected:
  size_t OnSysWrite(const void *buffer, size_t size) override;
  wxFileOffset OnSysTell() const override { return m_pos; }

  virtual void DoFlush(bool final);

 private:
  void Init(int level, int flags);

 protected:
  size_t m_z_size;
  unsigned char *m_z_buffer;
  struct z_stream_s *m_deflate;
  wxFileOffset m_pos;
};

class wxZlibClassFactory: public wxFilterClassFactory
{
public:
    wxZlibClassFactory();

    wxFilterInputStream *NewStream(wxInputStream& stream) const override
        { return new wxZlibInputStream(stream); }
    wxFilterOutputStream *NewStream(wxOutputStream& stream) const override
        { return new wxZlibOutputStream(stream, -1); }
    wxFilterInputStream *NewStream(wxInputStream *stream) const override
        { return new wxZlibInputStream(stream); }
    wxFilterOutputStream *NewStream(wxOutputStream *stream) const override
        { return new wxZlibOutputStream(stream, -1); }

    const wxChar * const *GetProtocols(wxStreamProtocolType type
                                       = wxSTREAM_PROTOCOL) const override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxZlibClassFactory);
};

class wxGzipClassFactory: public wxFilterClassFactory
{
public:
    wxGzipClassFactory();

    wxFilterInputStream *NewStream(wxInputStream& stream) const override
        { return new wxZlibInputStream(stream); }
    wxFilterOutputStream *NewStream(wxOutputStream& stream) const override
        { return new wxZlibOutputStream(stream, -1); }
    wxFilterInputStream *NewStream(wxInputStream *stream) const override
        { return new wxZlibInputStream(stream); }
    wxFilterOutputStream *NewStream(wxOutputStream *stream) const override
        { return new wxZlibOutputStream(stream, -1); }

    const wxChar * const *GetProtocols(wxStreamProtocolType type
                                       = wxSTREAM_PROTOCOL) const override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxGzipClassFactory);
};

wxVersionInfo wxGetZlibVersionInfo();

} // export

#endif
  // wxUSE_ZLIB && wxUSE_STREAMS
