/////////////////////////////////////////////////////////////////////////////
// Name:        wx/ipcbase.h
// Purpose:     Base classes for IPC
// Author:      Julian Smart
// Modified by:
// Created:     4/1/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/string.h"

export module WX.Cmn.IpcBase;

export
{

enum wxIPCFormat
{
  wxIPC_INVALID =          0,
  wxIPC_TEXT =             1,  /* CF_TEXT */
  wxIPC_BITMAP =           2,  /* CF_BITMAP */
  wxIPC_METAFILE =         3,  /* CF_METAFILEPICT */
  wxIPC_SYLK =             4,
  wxIPC_DIF =              5,
  wxIPC_TIFF =             6,
  wxIPC_OEMTEXT =          7,  /* CF_OEMTEXT */
  wxIPC_DIB =              8,  /* CF_DIB */
  wxIPC_PALETTE =          9,
  wxIPC_PENDATA =          10,
  wxIPC_RIFF =             11,
  wxIPC_WAVE =             12,
  wxIPC_UTF16TEXT =        13, /* CF_UNICODE */
  wxIPC_ENHMETAFILE =      14,
  wxIPC_FILENAME =         15, /* CF_HDROP */
  wxIPC_LOCALE =           16,
  wxIPC_UTF8TEXT =         17,
  wxIPC_UTF32TEXT =        18,
#if SIZEOF_WCHAR_T == 2
  wxIPC_UNICODETEXT = wxIPC_UTF16TEXT,
#elif SIZEOF_WCHAR_T == 4
  wxIPC_UNICODETEXT = wxIPC_UTF32TEXT,
#else
#  error "Unknown wchar_t size"
#endif
  wxIPC_PRIVATE =          20
};

class wxServerBase;
class wxClientBase;

class wxConnectionBase
{
public:
  wxConnectionBase(void *buffer, size_t size); // use external buffer
  wxConnectionBase() = default; // use internal, adaptive buffer
  wxConnectionBase(const wxConnectionBase& copy);
  virtual ~wxConnectionBase();

  wxConnectionBase& operator=(const wxConnectionBase&) = delete;

  void SetConnected( bool c ) { m_connected = c; }
  bool GetConnected() const { return m_connected; }

  // Calls that CLIENT can make
  bool Execute(const void *data, size_t size, wxIPCFormat fmt = wxIPC_PRIVATE)
      { return DoExecute(data, size, fmt); }
  bool Execute(const char *s, size_t size = wxNO_LEN)
      { return DoExecute(s, size == wxNO_LEN ? strlen(s) + 1
                                             : size, wxIPC_TEXT); }
  bool Execute(const wchar_t *ws, size_t size = wxNO_LEN)
      { return DoExecute(ws, size == wxNO_LEN ? (wcslen(ws) + 1)*sizeof(wchar_t)
                                              : size, wxIPC_UNICODETEXT); }
  bool Execute(const wxString& s)
  {
      const wxScopedCharBuffer buf = s.utf8_str();
      return DoExecute(buf, strlen(buf) + 1, wxIPC_UTF8TEXT);
  }
  bool Execute(const wxCStrData& cs)
      { return Execute(cs.AsString()); }

  virtual const void *Request(const wxString& item,
                              size_t *size = nullptr,
                              wxIPCFormat format = wxIPC_TEXT) = 0;

  bool Poke(const wxString& item, const void *data, size_t size,
            wxIPCFormat fmt = wxIPC_PRIVATE)
      { return DoPoke(item, data, size, fmt); }
  bool Poke(const wxString& item, const char *s, size_t size = wxNO_LEN)
      { return DoPoke(item, s, size == wxNO_LEN ? strlen(s) + 1
                                                : size, wxIPC_TEXT); }
  bool Poke(const wxString& item, const wchar_t *ws, size_t size = wxNO_LEN)
      { return DoPoke(item, ws,
                      size == wxNO_LEN ? (wcslen(ws) + 1)*sizeof(wchar_t)
                                       : size, wxIPC_UNICODETEXT); }
  bool Poke(const wxString& item, const wxString& s)
  {
      const wxScopedCharBuffer buf = s.utf8_str();
      return DoPoke(item, buf,  strlen(buf) + 1, wxIPC_UTF8TEXT);
  }
  bool Poke(const wxString& item, const wxCStrData& cs)
      { return Poke(item, cs.AsString()); }

  virtual bool StartAdvise(const wxString& item) = 0;
  virtual bool StopAdvise(const wxString& item) = 0;

  // Calls that SERVER can make
  bool Advise(const wxString& item, const void *data, size_t size,
              wxIPCFormat fmt = wxIPC_PRIVATE)
      { return DoAdvise(item, data, size, fmt); }
  bool Advise(const wxString& item, const char *s, size_t size = wxNO_LEN)
      { return DoAdvise(item, s, size == wxNO_LEN ? strlen(s) + 1
                                                  : size, wxIPC_TEXT); }
  bool Advise(const wxString& item, const wchar_t *ws, size_t size = wxNO_LEN)
      { return DoAdvise(item, ws,
                        size == wxNO_LEN ? (wcslen(ws) + 1)*sizeof(wchar_t)
                                         : size, wxIPC_UNICODETEXT); }
  bool Advise(const wxString& item, const wxString& s)
  {
      const wxScopedCharBuffer buf = s.utf8_str();
      return DoAdvise(item, buf,  strlen(buf) + 1, wxIPC_UTF8TEXT);
  }
  bool Advise(const wxString& item, const wxCStrData& cs)
      { return Advise(item, cs.AsString()); }

  // Calls that both can make
  virtual bool Disconnect() = 0;


  // Callbacks to SERVER - override at will
  virtual bool OnExec([[maybe_unused]] const wxString& topic,
                      [[maybe_unused]] const wxString& data)
  {
      wxFAIL_MSG( "This method shouldn't be called, if it is, it probably "
                  "means that you didn't update your old code overriding "
                  "OnExecute() to use the new parameter types (\"const void *\" "
                  "instead of \"wxChar *\" and \"size_t\" instead of \"int\"), "
                  "you must do it or your code wouldn't be executed at all!" );
      return false;
  }

  // deprecated function kept for backwards compatibility: usually you will
  // want to override OnExec() above instead which receives its data in a more
  // convenient format
  virtual bool OnExecute(const wxString& topic,
                         const void *data,
                         size_t size,
                         wxIPCFormat format)
      { return OnExec(topic, GetTextFromData(data, size, format)); }

  virtual const void *OnRequest([[maybe_unused]] const wxString& topic,
                                [[maybe_unused]] const wxString& item,
                                size_t *size,
                                [[maybe_unused]] wxIPCFormat format)
      { *size = 0; return nullptr; }

  virtual bool OnPoke([[maybe_unused]] const wxString& topic,
                      [[maybe_unused]] const wxString& item,
                      [[maybe_unused]] const void *data,
                      [[maybe_unused]] size_t size,
                      [[maybe_unused]] wxIPCFormat format)
      { return false; }

  virtual bool OnStartAdvise([[maybe_unused]] const wxString& topic,
                             [[maybe_unused]] const wxString& item)
      { return false; }

  virtual bool OnStopAdvise([[maybe_unused]] const wxString& topic,
                            [[maybe_unused]] const wxString& item)
      { return false; }

  // Callbacks to CLIENT - override at will
  virtual bool OnAdvise([[maybe_unused]] const wxString& topic,
                        [[maybe_unused]] const wxString& item,
                        [[maybe_unused]] const void *data,
                        [[maybe_unused]] size_t size,
                        [[maybe_unused]] wxIPCFormat format)
      { return false; }

  // Callbacks to BOTH
  virtual bool OnDisconnect() { delete this; return true; }


  // return true if this is one of the formats used for textual data
  // transmission
  static bool IsTextFormat(wxIPCFormat format)
  {
      return format == wxIPC_TEXT ||
             format == wxIPC_UTF8TEXT ||
             format == wxIPC_UTF16TEXT ||
             format == wxIPC_UTF32TEXT;
  }

  // converts from the data and format into a wxString automatically
  //
  // this function accepts data in all of wxIPC_TEXT, wxIPC_UNICODETEXT, and
  // wxIPC_UTF8TEXT formats but asserts if the format is anything else (i.e.
  // such that IsTextFormat(format) doesn't return true)
  //
  // notice that the size parameter here contains the total size of the data,
  // including the terminating '\0' or L'\0'
  static
  wxString GetTextFromData(const void *data, size_t size, wxIPCFormat format);


  // return a buffer at least this size, reallocating buffer if needed
  // returns NULL if using an inadequate user buffer which can't be resized
  void *GetBufferAtLeast(size_t bytes);

protected:
  virtual bool DoExecute(const void *data, size_t size, wxIPCFormat format) = 0;
  virtual bool DoPoke(const wxString& item, const void *data, size_t size,
                      wxIPCFormat format) = 0;
  virtual bool DoAdvise(const wxString& item, const void *data, size_t size,
                        wxIPCFormat format) = 0;


private:
  char         *m_buffer{nullptr};
  size_t        m_buffersize{0};
  bool          m_deletebufferwhendone{true};

protected:
  bool          m_connected{true};
};


class wxServerBase
{
public:
  virtual ~wxServerBase() = default;

  // Returns false on error (e.g. port number is already in use)
  virtual bool Create(const wxString& serverName) = 0;

  // Callbacks to SERVER - override at will
  virtual wxConnectionBase *OnAcceptConnection(const wxString& topic) = 0;
};

class wxClientBase
{
public:
  virtual ~wxClientBase() = default;

  virtual bool ValidHost(const wxString& host) = 0;

  // Call this to make a connection. Returns NULL if cannot.
  virtual wxConnectionBase *MakeConnection(const wxString& host,
                                           const wxString& server,
                                           const wxString& topic) = 0;

  // Callbacks to CLIENT - override at will
  virtual wxConnectionBase *OnMakeConnection() = 0;
};

} // export
