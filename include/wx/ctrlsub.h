/////////////////////////////////////////////////////////////////////////////
// Name:        wx/ctrlsub.h (read: "wxConTRoL with SUBitems")
// Purpose:     wxControlWithItems interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     22.10.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CTRLSUB_H_BASE_
#define _WX_CTRLSUB_H_BASE_

#include "wx/defs.h"

#if wxUSE_CONTROLS

#include "wx/control.h"      // base class
#include "wx/stringutils.h"

#include <cstdint>
#include <string>
#include <vector>

#include <gsl/gsl>


// ----------------------------------------------------------------------------
// wxItemContainer defines an interface which is implemented by all controls
// which have string subitems each of which may be selected.
//
// It is decomposed in wxItemContainerImmutable which omits all methods
// adding/removing items and is used by wxRadioBox and wxItemContainer itself.
//
// Examples: wxListBox, wxCheckListBox, wxChoice and wxComboBox (which
// implements an extended interface deriving from this one)
// ----------------------------------------------------------------------------

class wxItemContainerImmutable
{
public:
    virtual ~wxItemContainerImmutable() = default;

    // accessing strings
    // -----------------

    virtual size_t GetCount() const = 0;
    bool IsEmpty() const noexcept { return GetCount() == 0; }

    virtual std::string GetString(unsigned int n) const = 0;
    std::vector<std::string> GetStrings() const;
    virtual void SetString(unsigned int n, const std::string& s) = 0;

    // finding string natively is either case sensitive or insensitive
    // but never both so fall back to this base version for not
    // supported search type
    // FIXME: Remove this and split into two functions.
    virtual int FindString(std::string_view s, bool bCase = false) const
    {
        const std::size_t count = GetCount();

        if(!bCase)
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                if (wx::utils::IsSameAsNoCase(s, GetString(i)))
                    return gsl::narrow_cast<int>(i);
            }
        }
        else
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                if (wx::utils::IsSameAsCase(s, GetString(i)))
                    return gsl::narrow_cast<int>(i);
            }
        }

        return wxNOT_FOUND;
    }


    // selection
    // ---------

    virtual void SetSelection(int n) = 0;
    virtual int GetSelection() const = 0;

    // set selection to the specified string, return false if not found
    bool SetStringSelection(const std::string& s);

    // return the selected string or empty string if none
    virtual std::string GetStringSelection() const;

    // this is the same as SetSelection( for single-selection controls but
    // reads better for multi-selection ones
    void Select(int n) { SetSelection(n); }


protected:
    // check that the index is valid
    bool IsValid(unsigned int n) const { return n < GetCount(); }
    bool IsValidInsert(unsigned int n) const { return n <= GetCount(); }
};

// ----------------------------------------------------------------------------
// wxItemContainer extends wxItemContainerImmutable interface with methods
// for adding/removing items.
//
// Classes deriving from this one must override DoInsertItems() to implement
// adding items to the control. This can often be implemented more efficiently
// than simply looping over the elements and inserting them but if this is not
// the case, the generic DoInsertItemsInLoop can be used in implementation, but
// in this case DoInsertItem() needs to be overridden.
// ----------------------------------------------------------------------------

class wxItemContainer : public wxItemContainerImmutable
{
private:
    // AppendItems() and InsertItems() helpers just call DoAppend/InsertItems()
    // after doing some checks
    //
    // NB: they're defined here so that they're inlined when used in public part
    int AppendItems(const std::vector<std::string>& items,
                    void **clientData,
                    wxClientDataType type)
    {
        if ( items.empty() )
            return wxNOT_FOUND;

        return DoAppendItems(items, clientData, type);
    }

    int AppendItems(const std::vector<std::string>& items)
    {
        return AppendItems(items, nullptr, wxClientDataType::None);
    }

    int AppendItems(const std::vector<std::string>& items, void **clientData)
    {
        wxASSERT_MSG( GetClientDataType() != wxClientDataType::Object,
                      "can't mix different types of client data" );

        return AppendItems(items, clientData, wxClientDataType::Void);
    }

    int AppendItems(const std::vector<std::string>& items,
                    wxClientData **clientData)
    {
        wxASSERT_MSG( GetClientDataType() != wxClientDataType::Void,
                      "can't mix different types of client data" );

        return AppendItems(items, reinterpret_cast<void **>(clientData),
                           wxClientDataType::Object);
    }

    int InsertItems(const std::vector<std::string>& items,
                    unsigned int pos,
                    void **clientData,
                    wxClientDataType type)
    {
        wxASSERT_MSG( !IsSorted(), "can't insert items in sorted control" );

        wxCHECK_MSG( pos <= GetCount(), wxNOT_FOUND,
                     "position out of range" );

        // not all derived classes handle empty arrays correctly in
        // DoInsertItems() and besides it really doesn't make much sense to do
        // this (for append it could correspond to creating an initially empty
        // control but why would anybody need to insert 0 items?)
        wxCHECK_MSG( !items.empty(), wxNOT_FOUND,
                     "need something to insert" );

        return DoInsertItems(items, pos, clientData, type);
    }

    int InsertItems(const std::vector<std::string>& items, unsigned int pos)
    {
        return InsertItems(items, pos, nullptr, wxClientDataType::None);
    }

    int InsertItems(const std::vector<std::string>& items,
                     unsigned int pos,
                     void **clientData)
    {
        wxASSERT_MSG( GetClientDataType() != wxClientDataType::Object,
                      "can't mix different types of client data" );

        return InsertItems(items, pos, clientData, wxClientDataType::Void);
    }

    int InsertItems(const std::vector<std::string>& items,
                     unsigned int pos,
                     wxClientData **clientData)
    {
        wxASSERT_MSG( GetClientDataType() != wxClientDataType::Void,
                      "can't mix different types of client data" );

        return InsertItems(items, pos,
                           reinterpret_cast<void **>(clientData),
                           wxClientDataType::Object);
    }

public:
    // adding items
    // ------------

    // append single item, return its position in the control (which can be
    // different from the last one if the control is sorted)
    // FIXME: Stupid.
    int Append(const std::string& item)
        { return AppendItems(std::vector<std::string>{item}); }
    int Append(const std::string& item, void *clientData)
        { return AppendItems(std::vector<std::string>{item}, &clientData); }
    int Append(const std::string& item, wxClientData *clientData)
        { return AppendItems(std::vector<std::string>{item}, &clientData); }

    // append several items at once to the control, return the position of the
    // last item appended
    int Append(const std::vector<std::string>& items)
        { return AppendItems(items); }
    int Append(const std::vector<std::string>& items, void **clientData)
        { return AppendItems(items, clientData); }
    int Append(const std::vector<std::string>& items, wxClientData **clientData)
        { return AppendItems(items, clientData); }

    // only for RTTI needs (separate name)
    // void AppendString(const std::string& item)
    //     { Append(item); }


    // inserting items: not for sorted controls!
    // -----------------------------------------

    // insert single item at the given position, return its effective position
    int Insert(const std::string& item, unsigned int pos)
        { return InsertItems(std::vector<std::string>{item}, pos); }
    int Insert(const std::string& item, unsigned int pos, void *clientData)
        { return InsertItems(std::vector<std::string>{item}, pos, &clientData); }
    int Insert(const std::string& item, unsigned int pos, wxClientData *clientData)
        { return InsertItems(std::vector<std::string>{item}, pos, &clientData); }

    // insert several items at once into the control, return the index of the
    // last item inserted
    int Insert(const std::vector<std::string>& items, unsigned int pos)
        { return InsertItems(items, pos); }
    int Insert(const std::vector<std::string>& items, unsigned int pos, void **clientData)
        { return InsertItems(items, pos, clientData); }
    int Insert(const std::vector<std::string>& items,
               unsigned int pos,
               wxClientData **clientData)
        { return InsertItems(items, pos, clientData); }

    // replacing items
    // ---------------

    void Set(const std::vector<std::string>& items)
        { Clear(); Append(items); }
    void Set(const std::vector<std::string>& items, void **clientData)
        { Clear(); Append(items, clientData); }
    void Set(const std::vector<std::string>& items, wxClientData **clientData)
        { Clear(); Append(items, clientData); }

    // deleting items
    // --------------

    virtual void Clear();
    void Delete(unsigned int pos);


    // various accessors
    // -----------------

    // The control may maintain its items in a sorted order in which case
    // items are automatically inserted at the right position when they are
    // inserted or appended. Derived classes have to override this method if
    // they implement sorting, typically by returning HasFlag(wxXX_SORT)
    virtual bool IsSorted() const { return false; }


    // client data stuff
    // -----------------

    void SetClientData(unsigned int n, void* clientData);
    void* GetClientData(unsigned int n) const;

    // SetClientObject() takes ownership of the pointer, GetClientObject()
    // returns it but keeps the ownership while DetachClientObject() expects
    // the caller to delete the pointer and also resets the internally stored
    // one to NULL for this item
    void SetClientObject(unsigned int n, wxClientData* clientData);
    wxClientData* GetClientObject(unsigned int n) const;
    wxClientData* DetachClientObject(unsigned int n);

    // return the type of client data stored in this control: usually it just
    // returns m_clientDataItemsType but must be overridden in the controls
    // which delegate their client data storage to another one (e.g. wxChoice
    // in wxUniv which stores data in wxListBox which it uses anyhow); don't
    // forget to override SetClientDataType() if you override this one
    //
    // NB: for this to work no code should ever access m_clientDataItemsType
    //     directly but only via this function!
    virtual wxClientDataType GetClientDataType() const
        { return m_clientDataItemsType; }

    bool HasClientData() const
        { return GetClientDataType() != wxClientDataType::None; }
    bool HasClientObjectData() const
        { return GetClientDataType() == wxClientDataType::Object; }
    bool HasClientUntypedData() const
        { return GetClientDataType() == wxClientDataType::Void; }

protected:
    // there is usually no need to override this method but you can do it if it
    // is more convenient to only do "real" insertions in DoInsertItems() and
    // to implement items appending here (in which case DoInsertItems() should
    // call this method if pos == GetCount() as it can still be called in this
    // case if public Insert() is called with such position)
    virtual int DoAppendItems(const std::vector<std::string>& items,
                              void **clientData,
                              wxClientDataType type)
    {
        return DoInsertItems(items, GetCount(), clientData, type);
    }

    // this method must be implemented to insert the items into the control at
    // position pos which can be GetCount() meaning that the items should be
    // appended; for the sorted controls the position can be ignored
    //
    // the derived classes typically use AssignNewItemClientData() to
    // associate the data with the items as they're being inserted
    //
    // the method should return the index of the position the last item was
    // inserted into or wxNOT_FOUND if an error occurred
    virtual int DoInsertItems(const std::vector<std::string> & items,
                              unsigned int pos,
                              void **clientData,
                              wxClientDataType type) = 0;

    // before the client data is set for the first time for the control which
    // hadn't had it before, DoInitItemClientData() is called which gives the
    // derived class the possibility to initialize its client data storage only
    // when client data is really used
    virtual void DoInitItemClientData() { }
    virtual void DoSetItemClientData(unsigned int n, void *clientData) = 0;
    virtual void *DoGetItemClientData(unsigned int n) const = 0;

    virtual void DoClear() = 0;
    virtual void DoDeleteOneItem(unsigned int pos) = 0;


    // methods useful for the derived classes which don't have any better way
    // of adding multiple items to the control than doing it one by one: such
    // classes should call DoInsertItemsInLoop() from their DoInsert() and
    // override DoInsertOneItem() to perform the real insertion
    virtual int DoInsertOneItem(const std::string& item, unsigned int pos);

    // helper for DoInsertItems(): n is the index into clientData, pos is the
    // position of the item in the control
    void AssignNewItemClientData(unsigned int pos,
                                 void **clientData,
                                 unsigned int n,
                                 wxClientDataType type);

    // free the client object associated with the item at given position and
    // set it to NULL (must only be called if HasClientObjectData())
    void ResetItemClientObject(unsigned int n);

    // set the type of the client data stored in this control: override this if
    // you override GetClientDataType()
    virtual void SetClientDataType(wxClientDataType clientDataItemsType)
    {
        m_clientDataItemsType = clientDataItemsType;
    }

private:
    // the type of the client data for the items
    wxClientDataType m_clientDataItemsType{wxClientDataType::None};
};

// Inheriting directly from a wxWindow-derived class and wxItemContainer
// unfortunately introduces an ambiguity for all GetClientXXX() methods as they
// are inherited twice: the "global" versions from wxWindow and the per-item
// versions taking the index from wxItemContainer.
//
// So we need to explicitly resolve them and this helper template class is
// provided to do it. To use it, simply inherit from wxWindowWithItems<Window,
// Container> instead of Window and Container interface directly.
template <class W, class C>
class wxWindowWithItems : public W, public C
{
public:
    using BaseWindowClass = W;
    using BaseContainerInterface = C;

    void SetClientData(void *data)
        { BaseWindowClass::SetClientData(data); }
    void *GetClientData() const
        { return BaseWindowClass::GetClientData(); }
    void SetClientObject(wxClientData *data)
        { BaseWindowClass::SetClientObject(data); }
    wxClientData *GetClientObject() const
        { return BaseWindowClass::GetClientObject(); }

    void SetClientData(unsigned int n, void* clientData)
        { wxItemContainer::SetClientData(n, clientData); }
    void* GetClientData(unsigned int n) const
        { return wxItemContainer::GetClientData(n); }
    void SetClientObject(unsigned int n, wxClientData* clientData)
        { wxItemContainer::SetClientObject(n, clientData); }
    wxClientData* GetClientObject(unsigned int n) const
        { return wxItemContainer::GetClientObject(n); }
};

class wxControlWithItemsBase :
    public wxWindowWithItems<wxControl, wxItemContainer>
{
public:
    wxControlWithItemsBase& operator=(wxControlWithItemsBase&&) = delete;
    
    // usually the controls like list/combo boxes have their own background
    // colour
    bool ShouldInheritColours() const override { return false; }


    // Implementation only from now on.

    // Generate an event of the given type for the selection change.
    void SendSelectionChangedEvent(wxEventType eventType);

protected:
    // fill in the client object or data field of the event as appropriate
    //
    // calls InitCommandEvent() and, if n != wxNOT_FOUND, also sets the per
    // item client data
    void InitCommandEventWithItems(wxCommandEvent& event, int n);
};

// define the platform-specific wxControlWithItems class
#if defined(__WXMSW__)
    #include "wx/msw/ctrlsub.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/ctrlsub.h"
#elif defined(__WXQT__)
    #include "wx/qt/ctrlsub.h"
#else
    class wxControlWithItems : public wxControlWithItemsBase
    {
    public:
        wxControlWithItems& operator=(wxControlWithItems&&) = delete;
        
    private:
        wxDECLARE_ABSTRACT_CLASS(wxControlWithItems);
    };
#endif

#endif // wxUSE_CONTROLS

#endif // _WX_CTRLSUB_H_BASE_
