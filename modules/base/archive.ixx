/////////////////////////////////////////////////////////////////////////////
// Name:        wx/archive.h
// Purpose:     Streams for archive formats
// Author:      Mike Wetherell
// Copyright:   (c) 2004 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/datetime.h"
#include "wx/filefn.h"

export module WX.Cmn.ArchStream;

import WX.Cmn.Stream;
import WX.File.Filename;

#if defined WX_TEST_ARCHIVE_ITERATOR
import <iterator>;
#endif

import <string>;
import <utility>;

#if wxUSE_STREAMS && wxUSE_ARCHIVE_STREAMS

export
{

class wxArchiveNotifier
{
public:
    virtual ~wxArchiveNotifier() = default;

    virtual void OnEntryUpdated(class wxArchiveEntry& entry) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// wxArchiveEntry
//
// Holds an entry's meta data, such as filename and timestamp.

class wxArchiveEntry
{
public:
    virtual ~wxArchiveEntry() = default;

    virtual wxDateTime   GetDateTime() const = 0;
    virtual wxFileOffset GetSize() const = 0;
    virtual wxFileOffset GetOffset() const = 0;
    virtual bool         IsDir() const = 0;
    virtual bool         IsReadOnly() const = 0;
    virtual std::string     GetInternalName() const = 0;
    virtual wxPathFormat GetInternalFormat() const = 0;
    virtual std::string     GetName(wxPathFormat format = wxPATH_NATIVE) const = 0;

    virtual void SetDateTime(const wxDateTime& dt) = 0;
    virtual void SetSize(wxFileOffset size) = 0;
    virtual void SetIsDir(bool isDir = true) = 0;
    virtual void SetIsReadOnly(bool isReadOnly = true) = 0;
    virtual void SetName(const std::string& name,
                         wxPathFormat format = wxPATH_NATIVE) = 0;

    wxArchiveEntry *Clone() const { return DoClone(); }

    void SetNotifier(wxArchiveNotifier& notifier);
    virtual void UnsetNotifier() { m_notifier = nullptr; }

protected:
    wxArchiveEntry() = default;
    wxArchiveEntry(const wxArchiveEntry& e) = default;

    virtual void SetOffset(wxFileOffset offset) = 0;
    virtual wxArchiveEntry* DoClone() const = 0;

    wxArchiveNotifier *GetNotifier() const { return m_notifier; }
    wxArchiveEntry& operator=(const wxArchiveEntry& entry);

private:
    wxArchiveNotifier *m_notifier{nullptr};
};


/////////////////////////////////////////////////////////////////////////////
// wxArchiveInputStream
//
// GetNextEntry() returns an wxArchiveEntry object containing the meta-data
// for the next entry in the archive (and gives away ownership). Reading from
// the wxArchiveInputStream then returns the entry's data. Eof() becomes true
// after an attempt has been made to read past the end of the entry's data.
//
// When there are no more entries, GetNextEntry() returns NULL and sets Eof().

class wxArchiveInputStream : public wxFilterInputStream
{
public:
    using entry_type = wxArchiveEntry;

    virtual bool OpenEntry(wxArchiveEntry& entry) = 0;
    virtual bool CloseEntry() = 0;

    wxArchiveEntry *GetNextEntry()  { return DoGetNextEntry(); }

    char Peek() override  { return wxInputStream::Peek(); }

protected:
    wxArchiveInputStream(wxInputStream& stream, wxMBConv& conv);
    wxArchiveInputStream(wxInputStream *stream, wxMBConv& conv);

    virtual wxArchiveEntry *DoGetNextEntry() = 0;

    wxMBConv& GetConv() const       { return m_conv; }

private:
    wxMBConv& m_conv;
};


/////////////////////////////////////////////////////////////////////////////
// wxArchiveOutputStream
//
// PutNextEntry is used to create a new entry in the output archive, then
// the entry's data is written to the wxArchiveOutputStream.
//
// Only one entry can be open for output at a time; another call to
// PutNextEntry closes the current entry and begins the next.
//
// The overload 'bool PutNextEntry(wxArchiveEntry *entry)' takes ownership
// of the entry object.

class wxArchiveOutputStream : public wxFilterOutputStream
{
public:
    virtual bool PutNextEntry(wxArchiveEntry *entry) = 0;

    virtual bool PutNextEntry(const std::string& name,
                              const wxDateTime& dt = wxDateTime::Now(),
                              wxFileOffset size = wxInvalidOffset) = 0;

    virtual bool PutNextDirEntry(const std::string& name,
                                 const wxDateTime& dt = wxDateTime::Now()) = 0;

    virtual bool CopyEntry(wxArchiveEntry *entry,
                           wxArchiveInputStream& stream) = 0;

    virtual bool CopyArchiveMetaData(wxArchiveInputStream& stream) = 0;

    virtual bool CloseEntry() = 0;

protected:
    wxArchiveOutputStream(wxOutputStream& stream, wxMBConv& conv);
    wxArchiveOutputStream(wxOutputStream *stream, wxMBConv& conv);

    wxMBConv& GetConv() const { return m_conv; }

private:
    wxMBConv& m_conv;
};


/////////////////////////////////////////////////////////////////////////////
// wxArchiveIterator
//
// An input iterator that can be used to transfer an archive's catalog to
// a container.

#if defined WX_TEST_ARCHIVE_ITERATOR
import <iterator>;
import <utility>;

template <class X, class Y> inline
void _wxSetArchiveIteratorValue(
    X& val, Y entry, [[maybe_unused]] void *d)
{
    val = X(entry);
}
template <class X, class Y, class Z> inline
void _wxSetArchiveIteratorValue(
    std::pair<X, Y>& val, Z entry, [[maybe_unused]] Z d)
{
    val = std::make_pair(X(entry->GetInternalName()), Y(entry));
}

template <class Arc, class T = typename Arc::entry_type*>
class wxArchiveIterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T *;
    using reference = T &;

    wxArchiveIterator() : m_rep(NULL) { }

    wxArchiveIterator(Arc& arc) {
        typename Arc::entry_type* entry = arc.GetNextEntry();
        m_rep = entry ? new Rep(arc, entry) : NULL;
    }

    wxArchiveIterator(const wxArchiveIterator& it) : m_rep(it.m_rep) {
        if (m_rep)
            m_rep->AddRef();
    }

    ~wxArchiveIterator() {
        if (m_rep)
            m_rep->UnRef();
    }

    const T& operator *() const {
        return m_rep->GetValue();
    }

    const T* operator ->() const {
        return &**this;
    }

    wxArchiveIterator& operator =(const wxArchiveIterator& it) {
        if (it.m_rep)
            it.m_rep.AddRef();
        if (m_rep)
            this->m_rep.UnRef();
        m_rep = it.m_rep;
        return *this;
    }

    wxArchiveIterator& operator ++() {
        m_rep = m_rep->Next();
        return *this;
    }

    wxArchiveIterator operator ++(int) {
        wxArchiveIterator it(*this);
        ++(*this);
        return it;
    }

    bool operator ==(const wxArchiveIterator& j) const {
        return m_rep == j.m_rep;
    }

    bool operator !=(const wxArchiveIterator& j) const {
        return !(*this == j);
    }

private:
    class Rep {
        Arc& m_arc;
        typename Arc::entry_type* m_entry;
        T m_value;
        int m_ref;

    public:
        Rep(Arc& arc, typename Arc::entry_type* entry)
            : m_arc(arc), m_entry(entry), m_value(), m_ref(1) { }
        ~Rep()
            { delete m_entry; }

        void AddRef() {
            m_ref++;
        }

        void UnRef() {
            if (--m_ref == 0)
                delete this;
        }

        Rep *Next() {
            typename Arc::entry_type* entry = m_arc.GetNextEntry();
            if (!entry) {
                UnRef();
                return NULL;
            }
            if (m_ref > 1) {
                m_ref--;
                return new Rep(m_arc, entry);
            }
            delete m_entry;
            m_entry = entry;
            m_value = T();
            return this;
        }

        const T& GetValue() {
            if (m_entry) {
                _wxSetArchiveIteratorValue(m_value, m_entry, m_entry);
                m_entry = NULL;
            }
            return m_value;
        }
    } *m_rep;
};

using wxArchiveIter = wxArchiveIterator<wxArchiveInputStream>;
using wxArchivePairIter = wxArchiveIterator<wxArchiveInputStream, std::pair<std::string, wxArchiveEntry *>>;

#endif // defined WX_TEST_ARCHIVE_ITERATOR


/////////////////////////////////////////////////////////////////////////////
// wxArchiveClassFactory
//
// A wxArchiveClassFactory instance for a particular archive type allows
// the creation of the other classes that may be needed.

void wxUseArchiveClasses();

class wxArchiveClassFactory : public wxFilterClassFactoryBase
{
public:
    using entry_type = wxArchiveEntry;
    using instream_type = wxArchiveInputStream;
    using outstream_type = wxArchiveOutputStream;
    using notifier_type = wxArchiveNotifier;
#if defined WX_TEST_ARCHIVE_ITERATOR
    using iter_type = wxArchiveIter;
    using pairiter_type = wxArchivePairIter;
#endif

    wxArchiveEntry *NewEntry() const
        { return DoNewEntry(); }
    wxArchiveInputStream *NewStream(wxInputStream& stream) const
        { return DoNewStream(stream); }
    wxArchiveOutputStream *NewStream(wxOutputStream& stream) const
        { return DoNewStream(stream); }
    wxArchiveInputStream *NewStream(wxInputStream *stream) const
        { return DoNewStream(stream); }
    wxArchiveOutputStream *NewStream(wxOutputStream *stream) const
        { return DoNewStream(stream); }

    virtual std::string GetInternalName(
        const std::string& name,
        wxPathFormat format = wxPATH_NATIVE) const = 0;

    // FIXME-UTF8: remove these from this file, they are used for ANSI
    //             build only
    void SetConv(wxMBConv& conv) { m_pConv = &conv; }
    wxMBConv& GetConv() const
        { if (m_pConv) return *m_pConv; else return wxConvLocal; }

    static const wxArchiveClassFactory *Find(const std::string& protocol,
                                             wxStreamProtocolType type
                                             = wxSTREAM_PROTOCOL);

    static const wxArchiveClassFactory *GetFirst();
    const wxArchiveClassFactory *GetNext() const { return m_next; }

    void PushFront() { Remove(); m_next = sm_first; sm_first = this; }
    void Remove();

protected:
    // old compilers don't support covarient returns, so 'Do' methods are
    // used to simulate them
    virtual wxArchiveEntry        *DoNewEntry() const = 0;
    virtual wxArchiveInputStream  *DoNewStream(wxInputStream& stream) const = 0;
    virtual wxArchiveOutputStream *DoNewStream(wxOutputStream& stream) const = 0;
    virtual wxArchiveInputStream  *DoNewStream(wxInputStream *stream) const = 0;
    virtual wxArchiveOutputStream *DoNewStream(wxOutputStream *stream) const = 0;

    wxArchiveClassFactory() :  m_next(this) { }
    wxArchiveClassFactory& operator=([[maybe_unused]] const wxArchiveClassFactory& f)
        { return *this; }

private:
    wxMBConv *m_pConv{nullptr};
    inline static wxArchiveClassFactory *sm_first{nullptr};
    wxArchiveClassFactory *m_next;
};

} // export

#endif // wxUSE_STREAMS && wxUSE_ARCHIVE_STREAMS

