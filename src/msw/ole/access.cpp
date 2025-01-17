///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/ole/access.cpp
// Purpose:     implementation of wxIAccessible and wxAccessible
// Author:      Julian Smart
// Modified by:
// Created:     2003-02-12
// Copyright:   (c) 2003 Julian Smart
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_OLE && wxUSE_ACCESSIBILITY

#include "wx/access.h"

#include "wx/app.h"
#include "wx/window.h"
#include "wx/log.h"

#include "wx/msw/ole/oleutils.h"

#ifndef OBJID_CLIENT
#define OBJID_CLIENT 0xFFFFFFFC
#endif

// Convert to Windows role
int wxConvertToWindowsRole(wxAccSystemRole wxrole);

// Convert to Windows state
constexpr LONG wxConvertToWindowsState(unsigned int wxstate)
{
    LONG state = 0;
    if (wxstate & wxACC_STATE_SYSTEM_ALERT_HIGH)
        state |= STATE_SYSTEM_ALERT_HIGH;

    if (wxstate & wxACC_STATE_SYSTEM_ALERT_MEDIUM)
        state |= STATE_SYSTEM_ALERT_MEDIUM;

    if (wxstate & wxACC_STATE_SYSTEM_ALERT_LOW)
        state |= STATE_SYSTEM_ALERT_LOW;

    if (wxstate & wxACC_STATE_SYSTEM_ANIMATED)
        state |= STATE_SYSTEM_ANIMATED;

    if (wxstate & wxACC_STATE_SYSTEM_BUSY)
        state |= STATE_SYSTEM_BUSY;

    if (wxstate & wxACC_STATE_SYSTEM_CHECKED)
        state |= STATE_SYSTEM_CHECKED;

    if (wxstate & wxACC_STATE_SYSTEM_COLLAPSED)
        state |= STATE_SYSTEM_COLLAPSED;

    if (wxstate & wxACC_STATE_SYSTEM_DEFAULT)
        state |= STATE_SYSTEM_DEFAULT;

    if (wxstate & wxACC_STATE_SYSTEM_EXPANDED)
        state |= STATE_SYSTEM_EXPANDED;

    if (wxstate & wxACC_STATE_SYSTEM_EXTSELECTABLE)
        state |= STATE_SYSTEM_EXTSELECTABLE;

    if (wxstate & wxACC_STATE_SYSTEM_FLOATING)
        state |= STATE_SYSTEM_FLOATING;

    if (wxstate & wxACC_STATE_SYSTEM_FOCUSABLE)
        state |= STATE_SYSTEM_FOCUSABLE;

    if (wxstate & wxACC_STATE_SYSTEM_FOCUSED)
        state |= STATE_SYSTEM_FOCUSED;

    if (wxstate & wxACC_STATE_SYSTEM_HOTTRACKED)
        state |= STATE_SYSTEM_HOTTRACKED;

    if (wxstate & wxACC_STATE_SYSTEM_INVISIBLE)
        state |= STATE_SYSTEM_INVISIBLE;

    if (wxstate & wxACC_STATE_SYSTEM_MARQUEED)
        state |= STATE_SYSTEM_MARQUEED;

    if (wxstate & wxACC_STATE_SYSTEM_MIXED)
        state |= STATE_SYSTEM_MIXED;

    if (wxstate & wxACC_STATE_SYSTEM_MULTISELECTABLE)
        state |= STATE_SYSTEM_MULTISELECTABLE;

    if (wxstate & wxACC_STATE_SYSTEM_OFFSCREEN)
        state |= STATE_SYSTEM_OFFSCREEN;

    if (wxstate & wxACC_STATE_SYSTEM_PRESSED)
        state |= STATE_SYSTEM_PRESSED;

    //    if (wxstate & wxACC_STATE_SYSTEM_PROTECTED)
    //        state |= STATE_SYSTEM_PROTECTED;

    if (wxstate & wxACC_STATE_SYSTEM_READONLY)
        state |= STATE_SYSTEM_READONLY;

    if (wxstate & wxACC_STATE_SYSTEM_SELECTABLE)
        state |= STATE_SYSTEM_SELECTABLE;

    if (wxstate & wxACC_STATE_SYSTEM_SELECTED)
        state |= STATE_SYSTEM_SELECTED;

    if (wxstate & wxACC_STATE_SYSTEM_SELFVOICING)
        state |= STATE_SYSTEM_SELFVOICING;

    if (wxstate & wxACC_STATE_SYSTEM_UNAVAILABLE)
        state |= STATE_SYSTEM_UNAVAILABLE;

    return state;
}

// Convert to Windows selection flag
int wxConvertToWindowsSelFlag(wxAccSelectionFlags sel);

// Convert from Windows selection flag
wxAccSelectionFlags wxConvertFromWindowsSelFlag(unsigned int sel);

#if wxUSE_VARIANT
// ----------------------------------------------------------------------------
// wxIEnumVARIANT interface implementation
// ----------------------------------------------------------------------------

class wxIEnumVARIANT : public IEnumVARIANT
{
public:
    explicit wxIEnumVARIANT(const wxVariant& variant);

    wxIEnumVARIANT(const wxIEnumVARIANT&) = delete;
	wxIEnumVARIANT& operator=(const wxIEnumVARIANT&) = delete;

    DECLARE_IUNKNOWN_METHODS;

    // IEnumVARIANT
    STDMETHODIMP Next(ULONG celt, VARIANT *rgelt, ULONG *pceltFetched) override;
    STDMETHODIMP Skip(ULONG celt) override;
    STDMETHODIMP Reset() override;
    STDMETHODIMP Clone(IEnumVARIANT **ppenum) override;

private:
    wxVariant m_variant;  // List of further variants
    int       m_nCurrent; // Current enum position
};

// ----------------------------------------------------------------------------
// wxIEnumVARIANT
// ----------------------------------------------------------------------------

BEGIN_IID_TABLE(wxIEnumVARIANT)
    ADD_IID(Unknown)
    ADD_IID(EnumVARIANT)
END_IID_TABLE;

IMPLEMENT_IUNKNOWN_METHODS(wxIEnumVARIANT)

// wxVariant contains a list of further variants.
wxIEnumVARIANT::wxIEnumVARIANT(const wxVariant& variant)
{
    m_variant = variant;
}

STDMETHODIMP wxIEnumVARIANT::Next(ULONG      celt,
                                    VARIANT *rgelt,
                                    ULONG     *pceltFetched)
{
    wxLogTrace(wxTRACE_OleCalls, "wxIEnumVARIANT::Next");

    if ( celt > 1 ) {
        // we only return 1 element at a time - mainly because I'm too lazy to
        // implement something which you're never asked for anyhow
        return S_FALSE;
    }

    if ( !m_variant.IsType("list") )
        return S_FALSE;

    if ( m_nCurrent < (int) m_variant.GetList().GetCount() ) {
        if (!wxConvertVariantToOle(m_variant[m_nCurrent++], rgelt[0]))
        {
            return S_FALSE;
        }

        // TODO: should we AddRef if this is an object?

        * pceltFetched = 1;
        return S_OK;
    }
    else {
        // bad index
        return S_FALSE;
    }
}

STDMETHODIMP wxIEnumVARIANT::Skip(ULONG celt)
{
    wxLogTrace(wxTRACE_OleCalls, "wxIEnumVARIANT::Skip");

    if ( !m_variant.IsType("list") )
        return S_FALSE;

    m_nCurrent += celt;
    if ( m_nCurrent < (int) m_variant.GetList().GetCount() )
        return S_OK;

    // no, can't skip this many elements
    m_nCurrent -= celt;

    return S_FALSE;
}

STDMETHODIMP wxIEnumVARIANT::Reset()
{
    wxLogTrace(wxTRACE_OleCalls, "wxIEnumVARIANT::Reset");

    m_nCurrent = 0;

    return S_OK;
}

STDMETHODIMP wxIEnumVARIANT::Clone(IEnumVARIANT **ppenum)
{
    wxLogTrace(wxTRACE_OleCalls, "wxIEnumVARIANT::Clone");

    wxIEnumVARIANT *pNew = new wxIEnumVARIANT(m_variant);
    pNew->AddRef();
    *ppenum = pNew;

    return S_OK;
}

#endif // wxUSE_VARIANT

// ----------------------------------------------------------------------------
// wxIAccessible implementation of IAccessible interface
// ----------------------------------------------------------------------------

class wxIAccessible : public IAccessible
{
public:
    explicit wxIAccessible(wxAccessible *pAccessible);

    wxIAccessible(const wxIAccessible&) = delete;
	wxIAccessible& operator=(const wxIAccessible&) = delete;

    // Called to indicate object should prepare to be deleted.
    void Quiesce();

    DECLARE_IUNKNOWN_METHODS;

// IAccessible

// Navigation and Hierarchy

        // Retrieves the child element or child object at a given point on the screen.
        // All visual objects support this method; sound objects do not support it.

    STDMETHODIMP accHitTest(LONG xLeft, LONG yLeft, VARIANT* pVarID) override;

        // Retrieves the specified object's current screen location. All visual objects must
        // support this method; sound objects do not support it.

    STDMETHODIMP accLocation ( LONG* pxLeft, LONG* pyTop, LONG* pcxWidth, LONG* pcyHeight, VARIANT varID) override;

        // Traverses to another user interface element within a container and retrieves the object.
        // All visual objects must support this method.

    STDMETHODIMP accNavigate ( LONG navDir, VARIANT varStart, VARIANT* pVarEnd) override;

        // Retrieves the address of an IDispatch interface for the specified child.
        // All objects must support this property.

    STDMETHODIMP get_accChild ( VARIANT varChildID, IDispatch** ppDispChild) override;

        // Retrieves the number of children that belong to this object.
        // All objects must support this property.

    STDMETHODIMP get_accChildCount ( LONG* pCountChildren) override;

        // Retrieves the IDispatch interface of the object's parent.
        // All objects support this property.

    STDMETHODIMP get_accParent ( IDispatch** ppDispParent) override;

// Descriptive Properties and Methods

        // Performs the object's default action. Not all objects have a default
        // action.

    STDMETHODIMP accDoDefaultAction(VARIANT varID) override;

        // Retrieves a string that describes the object's default action.
        // Not all objects have a default action.

    STDMETHODIMP get_accDefaultAction ( VARIANT varID, BSTR* pszDefaultAction) override;

        // Retrieves a string that describes the visual appearance of the specified object.
        // Not all objects have a description.

    STDMETHODIMP get_accDescription ( VARIANT varID, BSTR* pszDescription) override;

        // Retrieves an object's Help property string.
        // Not all objects support this property.

    STDMETHODIMP get_accHelp ( VARIANT varID, BSTR* pszHelp) override;

        // Retrieves the full path of the WinHelp file associated with the specified
        // object and the identifier of the appropriate topic within that file.
        // Not all objects support this property.

    STDMETHODIMP get_accHelpTopic ( BSTR* pszHelpFile, VARIANT varChild, LONG* pidTopic) override;

        // Retrieves the specified object's shortcut key or access key, also known as
        // the mnemonic. All objects that have a shortcut key or access key support
        // this property.

    STDMETHODIMP get_accKeyboardShortcut ( VARIANT varID, BSTR* pszKeyboardShortcut) override;

        // Retrieves the name of the specified object.
        // All objects support this property.

    STDMETHODIMP get_accName ( VARIANT varID, BSTR* pszName) override;

        // Retrieves information that describes the role of the specified object.
        // All objects support this property.

    STDMETHODIMP get_accRole ( VARIANT varID, VARIANT* pVarRole) override;

        // Retrieves the current state of the specified object.
        // All objects support this property.

    STDMETHODIMP get_accState ( VARIANT varID, VARIANT* pVarState) override;

        // Retrieves the value of the specified object.
        // Not all objects have a value.

    STDMETHODIMP get_accValue ( VARIANT varID, BSTR* pszValue) override;

// Selection and Focus

        // Modifies the selection or moves the keyboard focus of the
        // specified object. All objects that select or receive the
        // keyboard focus must support this method.

    STDMETHODIMP accSelect ( LONG flagsSelect, VARIANT varID ) override;

        // Retrieves the object that has the keyboard focus. All objects
        // that receive the keyboard focus must support this property.

    STDMETHODIMP get_accFocus ( VARIANT* pVarID) override;

        // Retrieves the selected children of this object. All objects
        // selected must support this property.

    STDMETHODIMP get_accSelection ( VARIANT * pVarChildren) override;

// Obsolete

    STDMETHODIMP put_accName([[maybe_unused]] VARIANT varChild, [[maybe_unused]] BSTR szName) override { return E_FAIL; }
    STDMETHODIMP put_accValue([[maybe_unused]] VARIANT varChild, [[maybe_unused]] BSTR szName) override { return E_FAIL; }

// IDispatch

        // Get type info

    STDMETHODIMP GetTypeInfo(unsigned int typeInfo, LCID lcid, ITypeInfo** ppTypeInfo) override;

        // Get type info count

    STDMETHODIMP GetTypeInfoCount(unsigned int* typeInfoCount) override;

        // Get ids of names

    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** names, unsigned int cNames,
        LCID lcid, DISPID* dispId) override;

        // Invoke

    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
                        WXWORD wFlags, DISPPARAMS *pDispParams,
                        VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
                        unsigned int *puArgErr ) override;

// Helpers

    // Gets the standard IAccessible interface for the given child or object.
    // Call Release if this is non-NULL.
    IAccessible* GetChildStdAccessible(int id);

    // Gets the IAccessible interface for the given child or object.
    // Call Release if this is non-NULL.
    IAccessible* GetChildAccessible(int id);

private:
    wxAccessible *m_pAccessible;      // pointer to C++ class we belong to
    bool m_bQuiescing;                // Object is to be deleted
};

// ============================================================================
// Implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxIAccessible implementation
// ----------------------------------------------------------------------------
BEGIN_IID_TABLE(wxIAccessible)
  ADD_IID(Unknown)
  ADD_IID(Accessible)
  ADD_IID(Dispatch)
END_IID_TABLE;

IMPLEMENT_IUNKNOWN_METHODS(wxIAccessible)

wxIAccessible::wxIAccessible(wxAccessible *pAccessible)
{
    wxASSERT( pAccessible != nullptr );

    m_pAccessible = pAccessible;
    m_bQuiescing = false;
}

// Called to indicate object should prepare to be deleted.

void wxIAccessible::Quiesce()
{
    m_bQuiescing = true;
    m_pAccessible = nullptr;
}

// Retrieves the child element or child object at a given point on the screen.
// All visual objects support this method; sound objects do not support it.

STDMETHODIMP wxIAccessible::accHitTest(LONG xLeft, LONG yLeft, VARIANT* pVarID)
{
    wxLogTrace("access", "accHitTest");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    wxAccessible* childObject = nullptr;
    int childId = 0;
    VariantInit(pVarID);

    wxAccStatus status = m_pAccessible->HitTest(wxPoint(xLeft, yLeft), & childId, & childObject);

    if (status == wxAccStatus::Fail)
        return E_FAIL;
    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;
    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Use standard interface instead.
        IAccessible* stdInterface = m_pAccessible->GetIAccessibleStd();
        if (!stdInterface)
            return E_NOTIMPL;
        else
            return stdInterface->accHitTest(xLeft, yLeft, pVarID);
    }

    if (childObject)
    {
        if (childObject == m_pAccessible)
        {
            pVarID->vt = VT_I4;
            pVarID->lVal = CHILDID_SELF;
            return S_OK;
        }
        else
        {
            wxIAccessible* childIA = childObject->GetIAccessible();
            if (!childIA)
                return E_NOTIMPL;

            if (childIA->QueryInterface(IID_IDispatch, (LPVOID*) & pVarID->pdispVal) != S_OK)
                return E_FAIL;

            pVarID->vt = VT_DISPATCH;
            return S_OK;
        }
    }
    else if (childId > 0)
    {
        pVarID->vt = VT_I4;
        pVarID->lVal = childId;
        return S_OK;
    }
    else
    {
        pVarID->vt = VT_EMPTY;
        return S_FALSE;
    }

    #if 0
    // all cases above already cause some return action so below line
    // is unreachable and cause unnecessary warning
    return E_NOTIMPL;
    #endif
}

// Retrieves the specified object's current screen location. All visual objects must
// support this method; sound objects do not support it.

STDMETHODIMP wxIAccessible::accLocation ( LONG* pxLeft, LONG* pyTop, LONG* pcxWidth, LONG* pcyHeight, VARIANT varID)
{
    wxLogTrace("access", "accLocation");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    wxRect rect;

    wxAccStatus status = m_pAccessible->GetLocation(rect, varID.lVal);
    if (status == wxAccStatus::Fail)
        return E_FAIL;
    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;
    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varID);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varID);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varID);
    }
    else
    {
        *pxLeft = rect.x;
        *pyTop = rect.y;
        *pcxWidth = rect.width;
        *pcyHeight = rect.height;
        return S_OK;
    }

    return E_NOTIMPL;
}

// Traverses to another user interface element within a container and retrieves the object.
// All visual objects must support this method.

STDMETHODIMP wxIAccessible::accNavigate ( LONG navDir, VARIANT varStart, VARIANT* pVarEnd)
{
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;
    // FIXME: Log trace doesn't play nice with std::string.
    //wxLogTrace("access", fmt::format("accNavigate for {}", m_pAccessible->GetWindow()->wxGetClassInfo()->wxGetClassName()));

    if ( varStart.vt != VT_I4 || varStart.lVal < 0 )
    {
        wxLogTrace("access", "Invalid arg for accNavigate");
        return E_INVALIDARG;
    }

    wxAccessible* elementObject = nullptr;
    int elementId = 0;
    VariantInit(pVarEnd);
    wxNavDir navDirWX = wxNavDir::FirstChild;

    wxString navStr;

    switch (navDir)
    {
    case NAVDIR_DOWN:
        navDirWX = wxNavDir::Down;
        navStr = "wxNavDir::Down";
        break;

    case NAVDIR_FIRSTCHILD:
        navDirWX = wxNavDir::FirstChild;
        navStr = "wxNavDir::FirstChild";
        break;

    case NAVDIR_LASTCHILD:
        navDirWX = wxNavDir::LastChild;
        navStr = "wxNavDir::LastChild";
        break;

    case NAVDIR_LEFT:
        navDirWX = wxNavDir::Left;
        navStr = "wxNavDir::Left";
        break;

    case NAVDIR_NEXT:
        navDirWX = wxNavDir::Next;
        navStr = "wxNavDir::Next";
        break;

    case NAVDIR_PREVIOUS:
        navDirWX = wxNavDir::Previous;
        navStr = "wxNavDir::Previous";
        break;

    case NAVDIR_RIGHT:
        navDirWX = wxNavDir::Right;
        navStr = "wxNavDir::Right";
        break;

    case NAVDIR_UP:
        navDirWX = wxNavDir::Up;
        navStr = "wxNavDir::Up";
        break;
    default:
        {
            wxLogTrace("access", "Unknown NAVDIR symbol");
            break;
        }
    }
    wxLogTrace("access", navStr);

    wxAccStatus status = m_pAccessible->Navigate(navDirWX, varStart.lVal, & elementId,
        & elementObject);

    if (status == wxAccStatus::Fail)
    {
        wxLogTrace("access", "wxAccessible::Navigate failed");
        return E_FAIL;
    }

    if (status == wxAccStatus::InvalidArg)
    {
        wxLogTrace("access", "Invalid argument passed to wxAccessible::Navigate");
        return E_INVALIDARG;
    }

    if (status == wxAccStatus::False)
    {
        wxLogTrace("access", "wxAccessible::Navigate found no object in this direction");
        return S_FALSE;
    }

    if (status == wxAccStatus::NotImplemented)
    {
        wxLogTrace("access", "Navigate not implemented");

        // Try to use child object directly.
        if (varStart.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varStart.lVal);
            if (childAccessible)
            {
                varStart.lVal = 0;
                HRESULT hResult = childAccessible->accNavigate(navDir, varStart, pVarEnd);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->accNavigate(navDir, varStart, pVarEnd);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->accNavigate(navDir, varStart, pVarEnd);
    }
    else
    {
        if (elementObject)
        {
            wxLogTrace("access", "Getting wxIAccessible and calling QueryInterface for Navigate");
            wxIAccessible* objectIA = elementObject->GetIAccessible();
            if (!objectIA)
            {
                wxLogTrace("access", "No wxIAccessible");
                return E_FAIL;
            }

            HRESULT hResult = objectIA->QueryInterface(IID_IDispatch, (LPVOID*) & pVarEnd->pdispVal);
            if (hResult != S_OK)
            {
                wxLogTrace("access", "QueryInterface failed");
                return E_FAIL;
            }

            wxLogTrace("access", "Called QueryInterface for Navigate");
            pVarEnd->vt = VT_DISPATCH;
            return S_OK;
        }
        else if (elementId > 0)
        {
            wxLogTrace("access", "Returning element id from Navigate");
            pVarEnd->vt = VT_I4;
            pVarEnd->lVal = elementId;
            return S_OK;
        }
        else
        {
            wxLogTrace("access", "No object in accNavigate");
            pVarEnd->vt = VT_EMPTY;
            return S_FALSE;
        }
    }

    wxLogTrace("access", "Failing Navigate");
    return E_NOTIMPL;
}

// Retrieves the address of an IDispatch interface for the specified child.
// All objects must support this property.

STDMETHODIMP wxIAccessible::get_accChild ( VARIANT varChildID, IDispatch** ppDispChild)
{
    wxLogTrace("access", "get_accChild");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varChildID.vt != VT_I4 || varChildID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for get_accChild");
        return E_INVALIDARG;
    }

    if (varChildID.lVal == CHILDID_SELF)
    {
        *ppDispChild = this;
        AddRef();
        return S_OK;
    }

    wxAccessible* child = nullptr;

    wxAccStatus status = m_pAccessible->GetChild(varChildID.lVal, & child);
    if (status == wxAccStatus::Fail)
    {
        wxLogTrace("access", "GetChild failed");
        return E_FAIL;
    }
    if (status == wxAccStatus::InvalidArg)
    {
        wxLogTrace("access", "Invalid argument passed to GetChild");
        return E_INVALIDARG;
    }

    if (status == wxAccStatus::NotImplemented)
    {
        // Use standard interface instead.
        IAccessible* stdInterface = m_pAccessible->GetIAccessibleStd();
        if (!stdInterface)
            return E_NOTIMPL;
        else
        {
            wxLogTrace("access", "Using standard interface for get_accChild");
            return stdInterface->get_accChild (varChildID, ppDispChild);
        }
    }
    else
    {
        if (child)
        {
            wxIAccessible* objectIA = child->GetIAccessible();
            if (!objectIA)
                return E_NOTIMPL;

            if (objectIA->QueryInterface(IID_IDispatch, (LPVOID*) ppDispChild) != S_OK)
            {
                wxLogTrace("access", "QueryInterface failed in get_accChild");
                return E_FAIL;
            }

            return S_OK;
        }
        else
        {
            wxLogTrace("access", "Not an accessible object");
            return S_FALSE; // Indicates it's not an accessible object
        }
    }

    #if 0
    // all cases above already cause some return action so below line
    // is unreachable and cause unnecessary warning
    return E_NOTIMPL;
    #endif
}

// Retrieves the number of children that belong to this object.
// All objects must support this property.

STDMETHODIMP wxIAccessible::get_accChildCount ( LONG* pCountChildren)
{
    wxLogTrace("access", "get_accChildCount");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    int childCount = 0;
    wxAccStatus status = m_pAccessible->GetChildCount(& childCount);
    if (status == wxAccStatus::Fail)
        return E_FAIL;

    if (status == wxAccStatus::NotImplemented)
    {
        // Use standard interface instead.
        IAccessible* stdInterface = m_pAccessible->GetIAccessibleStd();
        if (!stdInterface)
            return E_NOTIMPL;
        else
        {
            wxLogTrace("access", "Using standard interface for get_accChildCount");
            HRESULT res = stdInterface->get_accChildCount (pCountChildren);
            wxString str;
            str.Printf("Number of children was %d", (int) (*pCountChildren));
            wxLogTrace("access", str);
            return res;
        }
    }
    else
    {
        * pCountChildren = (LONG) childCount;
        return S_OK;
    }

    #if 0
    // all cases above already cause some return action so below line
    // is unreachable and cause unnecessary warning
    return E_NOTIMPL;
    #endif
}

// Retrieves the IDispatch interface of the object's parent.
// All objects support this property.

STDMETHODIMP wxIAccessible::get_accParent ( IDispatch** ppDispParent)
{
    wxLogTrace("access", "get_accParent");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    wxAccessible* parent = nullptr;
    wxAccStatus status = m_pAccessible->GetParent(& parent);

    if (status == wxAccStatus::Fail)
        return E_FAIL;

    // It doesn't seem acceptable to return S_FALSE with a NULL
    // ppDispParent, so if we have no wxWidgets parent, we leave
    // it to the standard interface.
    if (status == wxAccStatus::NotImplemented || !parent)
    {
        wxLogTrace("access", "Using standard interface to get the parent.");
        // Use standard interface instead.
        IAccessible* stdInterface = m_pAccessible->GetIAccessibleStd();
        if (!stdInterface)
            return E_NOTIMPL;
        else
            return stdInterface->get_accParent (ppDispParent);
    }
    else
    {
        if (parent)
        {
            wxIAccessible* objectIA = parent->GetIAccessible();
            if (!objectIA)
                return E_FAIL;

            wxLogTrace("access", "About to call QueryInterface");
            if (objectIA->QueryInterface(IID_IDispatch, (LPVOID*) ppDispParent) != S_OK)
            {
                wxLogTrace("access", "Failed QueryInterface");
                return E_FAIL;
            }

            wxLogTrace("access", "Returning S_OK for get_accParent");
            return S_OK;
        }
        else
        {
            // This doesn't seem to be allowed, despite the documentation,
            // so we handle it higher up by using the standard interface.
            wxLogTrace("access", "Returning NULL parent because there was none");
            *ppDispParent = nullptr;
            return S_FALSE;
        }
    }

    #if 0
    // all cases above already cause some return action so below line
    // is unreachable and cause unnecessary warning
    return E_NOTIMPL;
    #endif
}

// Performs the object's default action. Not all objects have a default
// action.

STDMETHODIMP wxIAccessible::accDoDefaultAction(VARIANT varID)
{
    wxLogTrace("access", "accDoDefaultAction");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varID.vt != VT_I4 || varID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for accDoDefaultAction");
        return E_INVALIDARG;
    }

    const wxAccStatus status = m_pAccessible->DoDefaultAction(varID.lVal);
    if (status == wxAccStatus::Ok)
        return S_OK;

    if (status == wxAccStatus::Fail)
        return E_FAIL;

    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->accDoDefaultAction(varID);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->accDoDefaultAction(varID);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->accDoDefaultAction(varID);
    }
    return E_FAIL;
}

// Retrieves a string that describes the object's default action.
// Not all objects have a default action.

STDMETHODIMP wxIAccessible::get_accDefaultAction ( VARIANT varID, BSTR* pszDefaultAction)
{
    wxLogTrace("access", "get_accDefaultAction");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varID.vt != VT_I4 || varID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for get_accDefaultAction");
        return E_INVALIDARG;
    }

    std::string defaultAction;
    wxAccStatus status = m_pAccessible->GetDefaultAction(varID.lVal, & defaultAction);
    if (status == wxAccStatus::Fail)
        return E_FAIL;

    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;

    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->get_accDefaultAction(varID, pszDefaultAction);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->get_accDefaultAction(varID, pszDefaultAction);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->get_accDefaultAction(varID, pszDefaultAction);
    }
    else
    {
        if (defaultAction.empty())
        {
            * pszDefaultAction = nullptr;
            return S_FALSE;
        }
        else
        {
            * pszDefaultAction = wxBasicString(defaultAction).Detach();
            return S_OK;
        }
    }
    return E_FAIL;
}

// Retrieves a string that describes the visual appearance of the specified object.
// Not all objects have a description.

STDMETHODIMP wxIAccessible::get_accDescription ( VARIANT varID, BSTR* pszDescription)
{
    wxLogTrace("access", "get_accDescription");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varID.vt != VT_I4 || varID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for get_accDescription");
        return E_INVALIDARG;
    }

    std::string description;
    wxAccStatus status = m_pAccessible->GetDescription(varID.lVal, & description);
    if (status == wxAccStatus::Fail)
        return E_FAIL;
    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;
    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->get_accDescription(varID, pszDescription);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->get_accDescription(varID, pszDescription);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->get_accDescription(varID, pszDescription);
    }
    else
    {
        if (description.empty())
        {
            * pszDescription = nullptr;
            return S_FALSE;
        }
        else
        {
            * pszDescription = wxBasicString(description).Detach();
            return S_OK;
        }
    }
    return E_NOTIMPL;
}

// Retrieves an object's Help property string.
// Not all objects support this property.

STDMETHODIMP wxIAccessible::get_accHelp ( VARIANT varID, BSTR* pszHelp)
{
    wxLogTrace("access", "get_accHelp");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varID.vt != VT_I4 || varID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for get_accHelp");
        return E_INVALIDARG;
    }

    std::string helpString;
    wxAccStatus status = m_pAccessible->GetHelpText(varID.lVal, & helpString);
    if (status == wxAccStatus::Fail)
        return E_FAIL;
    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;
    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->get_accHelp(varID, pszHelp);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->get_accHelp(varID, pszHelp);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->get_accHelp (varID, pszHelp);
    }
    else
    {
        if (helpString.empty())
        {
            * pszHelp = nullptr;
            return S_FALSE;
        }
        else
        {
            * pszHelp = wxBasicString(helpString).Detach();
            return S_OK;
        }
    }
    return E_NOTIMPL;
}

// Retrieves the full path of the WinHelp file associated with the specified
// object and the identifier of the appropriate topic within that file.
// Not all objects support this property.
// NOTE: not supported by wxWidgets at this time. Use
// GetHelpText instead.

STDMETHODIMP wxIAccessible::get_accHelpTopic ( BSTR* pszHelpFile, VARIANT varChild, LONG* pidTopic)
{
    wxLogTrace("access", "get_accHelpTopic");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varChild.vt != VT_I4 || varChild.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for get_accHelpTopic");
        return E_INVALIDARG;
    }

    wxAccStatus status = wxAccStatus::NotImplemented;
    if (status == wxAccStatus::Fail)
        return E_FAIL;
    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varChild.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varChild.lVal);
            if (childAccessible)
            {
                varChild.lVal = 0;
                HRESULT hResult = childAccessible->get_accHelpTopic(pszHelpFile, varChild, pidTopic);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->get_accHelpTopic(pszHelpFile, varChild, pidTopic);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->get_accHelpTopic (pszHelpFile, varChild, pidTopic);
    }
    return E_NOTIMPL;
}

// Retrieves the specified object's shortcut key or access key, also known as
// the mnemonic. All objects that have a shortcut key or access key support
// this property.

STDMETHODIMP wxIAccessible::get_accKeyboardShortcut ( VARIANT varID, BSTR* pszKeyboardShortcut)
{
    wxLogTrace("access", "get_accKeyboardShortcut");
    *pszKeyboardShortcut = nullptr;

    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varID.vt != VT_I4 || varID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for get_accKeyboardShortcut");
        return E_INVALIDARG;
    }

    std::string keyboardShortcut;
    wxAccStatus status = m_pAccessible->GetKeyboardShortcut(varID.lVal, & keyboardShortcut);
    if (status == wxAccStatus::Fail)
        return E_FAIL;
    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;
    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->get_accKeyboardShortcut(varID, pszKeyboardShortcut);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->get_accKeyboardShortcut(varID, pszKeyboardShortcut);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->get_accKeyboardShortcut (varID, pszKeyboardShortcut);
    }
    else
    {
        if (keyboardShortcut.empty())
        {
            * pszKeyboardShortcut = nullptr;
            return S_FALSE;
        }
        else
        {
            * pszKeyboardShortcut = wxBasicString(keyboardShortcut).Detach();
            return S_OK;
        }
    }
    return E_NOTIMPL;
}

// Retrieves the name of the specified object.
// All objects support this property.

STDMETHODIMP wxIAccessible::get_accName ( VARIANT varID, BSTR* pszName)
{
    wxLogTrace("access", "get_accName");
    *pszName = nullptr;

    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varID.vt != VT_I4 || varID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for get_accName");
        return E_INVALIDARG;
    }

    std::string name;

    wxAccStatus status = m_pAccessible->GetName(varID.lVal, & name);

    if (status == wxAccStatus::Fail)
        return E_FAIL;

    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->get_accName(varID, pszName);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->get_accName(varID, pszName);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->get_accName (varID, pszName);
    }
    else
    {
        if ( name.empty() )
        {
            *pszName = nullptr;
            return S_FALSE;
        }
        else
        {
            *pszName = wxBasicString(name).Detach();
        }
        return S_OK;
    }
    return E_NOTIMPL;
}

// Retrieves information that describes the role of the specified object.
// All objects support this property.

STDMETHODIMP wxIAccessible::get_accRole ( VARIANT varID, VARIANT* pVarRole)
{
    wxLogTrace("access", "get_accRole");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varID.vt != VT_I4 || varID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for get_accRole");
        return E_INVALIDARG;
    }

    VariantInit(pVarRole);

    wxAccSystemRole role = wxAccSystemRole::None;

    wxAccStatus status = m_pAccessible->GetRole(varID.lVal, & role);

    if (status == wxAccStatus::Fail)
        return E_FAIL;

    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->get_accRole(varID, pVarRole);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->get_accRole(varID, pVarRole);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->get_accRole (varID, pVarRole);
    }
    else
    {
        if (role == wxAccSystemRole::None)
        {
            pVarRole->vt = VT_EMPTY;
            return S_OK;
        }

        pVarRole->lVal = wxConvertToWindowsRole(role);
        pVarRole->vt = VT_I4;

        return S_OK;
    }
    return E_NOTIMPL;
}

// Retrieves the current state of the specified object.
// All objects support this property.

STDMETHODIMP wxIAccessible::get_accState ( VARIANT varID, VARIANT* pVarState)
{
    wxLogTrace("access", "get_accState");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varID.vt != VT_I4 || varID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for get_accState");
        return E_INVALIDARG;
    }

    unsigned int wxstate{};

    wxAccStatus status = m_pAccessible->GetState(varID.lVal, &wxstate);
    if (status == wxAccStatus::Fail)
        return E_FAIL;
    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->get_accState(varID, pVarState);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->get_accState(varID, pVarState);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->get_accState (varID, pVarState);
    }
    else
    {
        LONG state = wxConvertToWindowsState(wxstate);
        pVarState->lVal = state;
        pVarState->vt = VT_I4;
        return S_OK;
    }
    return E_NOTIMPL;
}

// Retrieves the value of the specified object.
// Not all objects have a value.

STDMETHODIMP wxIAccessible::get_accValue ( VARIANT varID, BSTR* pszValue)
{
    wxLogTrace("access", "get_accValue");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varID.vt != VT_I4 || varID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for get_accValue");
        return E_INVALIDARG;
    }

    std::string strValue;

    wxAccStatus status = m_pAccessible->GetValue(varID.lVal, & strValue);

    if (status == wxAccStatus::Fail)
        return E_FAIL;

    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;

    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->get_accValue(varID, pszValue);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->get_accValue(varID, pszValue);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->get_accValue (varID, pszValue);
    }
    else
    {
        if ( strValue.empty() )
        {
            *pszValue = nullptr;
            return S_FALSE;
        }
        else
        {
            * pszValue = wxBasicString(strValue).Detach();
            return S_OK;
        }
    }
    return E_NOTIMPL;
}

// Modifies the selection or moves the keyboard focus of the
// specified object. All objects that select or receive the
// keyboard focus must support this method.

STDMETHODIMP wxIAccessible::accSelect ( LONG flagsSelect, VARIANT varID )
{
    wxLogTrace("access", "get_accSelect");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    if (varID.vt != VT_I4 || varID.lVal < 0)
    {
        wxLogTrace("access", "Invalid arg for accSelect");
        return E_INVALIDARG;
    }

    wxAccSelectionFlags wxsel = wxConvertFromWindowsSelFlag(flagsSelect);

    wxAccStatus status = m_pAccessible->Select(varID.lVal, wxsel);
    if (status == wxAccStatus::Fail)
        return E_FAIL;
    if (status == wxAccStatus::InvalidArg)
        return E_INVALIDARG;
    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Try to use child object directly.
        if (varID.lVal > 0)
        {
            IAccessible* childAccessible = GetChildAccessible(varID.lVal);
            if (childAccessible)
            {
                varID.lVal = 0;
                HRESULT hResult = childAccessible->accSelect(flagsSelect, varID);
                childAccessible->Release();
                return hResult;
            }
            else if (m_pAccessible->GetIAccessibleStd())
                return m_pAccessible->GetIAccessibleStd()->accSelect(flagsSelect, varID);
        }
        else if (m_pAccessible->GetIAccessibleStd())
            return m_pAccessible->GetIAccessibleStd()->accSelect(flagsSelect, varID);
    }
    else
        return S_OK;

    return E_NOTIMPL;
}

// Retrieves the object that has the keyboard focus. All objects
// that receive the keyboard focus must support this property.

STDMETHODIMP wxIAccessible::get_accFocus ( VARIANT* pVarID)
{
    wxLogTrace("access", "get_accFocus");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    wxAccessible* childObject = nullptr;
    int childId = 0;
    VariantInit(pVarID);

    wxAccStatus status = m_pAccessible->GetFocus(& childId, & childObject);
    if (status == wxAccStatus::Fail)
        return E_FAIL;
    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Use standard interface instead.
        IAccessible* stdInterface = m_pAccessible->GetIAccessibleStd();
        if (!stdInterface)
            return E_NOTIMPL;
        else
            return stdInterface->get_accFocus (pVarID);
    }
    if (childObject)
    {
        if (childObject == m_pAccessible)
        {
            pVarID->vt = VT_I4;
            pVarID->lVal = CHILDID_SELF;
            return S_OK;
        }
        else
        {
            wxIAccessible* childIA = childObject->GetIAccessible();
            if (!childIA)
                return E_NOTIMPL;

            if (childIA->QueryInterface(IID_IDispatch, (LPVOID*) & pVarID->pdispVal) != S_OK)
                return E_FAIL;

            pVarID->vt = VT_DISPATCH;
            return S_OK;
        }
    }
    else if (childId > 0)
    {
        pVarID->vt = VT_I4;
        pVarID->lVal = childId;
        return S_OK;
    }
    else
    {
        pVarID->vt = VT_EMPTY;
        return S_FALSE;
    }

    #if 0
    // all cases above already cause some return action so below line
    // is unreachable and cause unnecessary warning
    return E_NOTIMPL;
    #endif
}

// Retrieves the selected children of this object. All objects
// selected must support this property.

STDMETHODIMP wxIAccessible::get_accSelection ( VARIANT * pVarChildren)
{
#if wxUSE_VARIANT
    wxLogTrace("access", "get_accSelection");
    wxASSERT( ( m_pAccessible != nullptr ) || ( m_bQuiescing == true ) );
    if (!m_pAccessible)
        return E_FAIL;

    VariantInit(pVarChildren);

    wxVariant selections;
    wxAccStatus status = m_pAccessible->GetSelections(& selections);
    if (status == wxAccStatus::Fail)
        return E_FAIL;
    if (status == wxAccStatus::NotSupported)
        return DISP_E_MEMBERNOTFOUND;

    if (status == wxAccStatus::NotImplemented)
    {
        // Use standard interface instead.
        IAccessible* stdInterface = m_pAccessible->GetIAccessibleStd();
        if (!stdInterface)
            return E_NOTIMPL;
        else
            return stdInterface->get_accSelection (pVarChildren);
    }
    else
    {
        if ( selections.IsNull() )
        {
            pVarChildren->vt = VT_EMPTY;

            return S_OK;
        }
        else if ( selections.IsType("long") )
        {
            pVarChildren->vt = VT_I4;
            pVarChildren->lVal = selections.GetLong();

            return S_OK;
        }
        else if ( selections.IsType("void*") )
        {
            wxAccessible* childObject = (wxAccessible*) selections.GetVoidPtr();
            wxIAccessible* childIA = childObject->GetIAccessible();
            if (!childIA)
                return E_NOTIMPL;

            if (childIA->QueryInterface(IID_IDispatch, (LPVOID*) & pVarChildren->pdispVal) != S_OK)
                return E_FAIL;

            pVarChildren->vt = VT_DISPATCH;

            return S_OK;
        }
        else if ( selections.IsType("list") )
        {
            wxASSERT_MSG( selections.GetCount() > 1,
                          "Multiple child objects should be selected" );
            // TODO: should we AddRef for every "void*" member??

            wxIEnumVARIANT* enumVariant = new wxIEnumVARIANT(selections);
            enumVariant->AddRef();

            pVarChildren->vt = VT_UNKNOWN;
            pVarChildren->punkVal = enumVariant;

            return S_OK;
        }
    }
#else
    wxUnusedVar(pVarChildren);
#endif // wxUSE_VARIANT

    return E_NOTIMPL;
}

// Get type info

STDMETHODIMP wxIAccessible::GetTypeInfo([[maybe_unused]] unsigned int typeInfo, [[maybe_unused]] LCID lcid, ITypeInfo** ppTypeInfo)
{
    *ppTypeInfo = nullptr;
    return E_NOTIMPL;
}

// Get type info count

STDMETHODIMP wxIAccessible::GetTypeInfoCount(unsigned int* typeInfoCount)
{
    *typeInfoCount = 0;
    return E_NOTIMPL;
}

// Get ids of names

STDMETHODIMP wxIAccessible::GetIDsOfNames([[maybe_unused]] REFIID riid, [[maybe_unused]] OLECHAR** names, [[maybe_unused]] unsigned int cNames,
        [[maybe_unused]] LCID lcid, [[maybe_unused]] DISPID* dispId)
{
    return E_NOTIMPL;
}

// Invoke

STDMETHODIMP wxIAccessible::Invoke([[maybe_unused]] DISPID dispIdMember, [[maybe_unused]] REFIID riid, [[maybe_unused]] LCID lcid,
                        [[maybe_unused]] WXWORD wFlags, [[maybe_unused]] DISPPARAMS *pDispParams,
                        [[maybe_unused]] VARIANT *pVarResult, [[maybe_unused]] EXCEPINFO *pExcepInfo,
                        [[maybe_unused]] unsigned int *puArgErr )
{
    return E_NOTIMPL;
}

// Gets the standard IAccessible interface for the given child or object.
// Call Release if this is non-NULL.
IAccessible* wxIAccessible::GetChildStdAccessible(int id)
{
    if (id == 0)
    {
        IAccessible* obj = m_pAccessible->GetIAccessibleStd();

        obj->AddRef();
        return obj;
    }
    else
    {
        VARIANT var;
        VariantInit(& var);
        var.vt = VT_I4;
        var.lVal = id;
        IDispatch* pDispatch = nullptr;
        if (S_OK == get_accChild ( var, & pDispatch))
        {
            IAccessible* childAccessible = nullptr;
            if (pDispatch->QueryInterface(IID_IAccessible, (LPVOID*) & childAccessible) == S_OK)
            {
                pDispatch->Release();
                wxIAccessible* c = (wxIAccessible*) childAccessible;
                IAccessible* stdChildAccessible = c->m_pAccessible->GetIAccessibleStd();
                stdChildAccessible->AddRef();
                childAccessible->Release();
                return stdChildAccessible;
            }
            else
            {
                pDispatch->Release();
            }
        }
    }

#if 0
    {
        // Loop until we find the right id
        LONG nChildren = 0;
        this->get_accChildCount(& nChildren);

        for (int i = 0; i < nChildren; i++)
        {
            LONG obtained = 0;
            VARIANT var;
            VariantInit(& var);
            var.vt = VT_I4;
            if (S_OK == AccessibleChildren(this, i, 1, & var, &obtained))
            {
                if (var.lVal == id)
                {
                    VariantInit(& var);
                    var.vt = VT_DISPATCH;
                    if (S_OK == AccessibleChildren(this, i, 1, & var, &obtained))
                    {
                        IAccessible* childAccessible = NULL;
                        if (var.pdispVal->QueryInterface(IID_IAccessible, (LPVOID*) & childAccessible) == S_OK)
                        {
                            var.pdispVal->Release();
                            return childAccessible;
                        }
                        else
                        {
                            var.pdispVal->Release();
                        }
                    }
                }
                break;
            }
        }
    }
#endif
    return nullptr;
}

// Gets the IAccessible interface for the given child or object.
// Call Release if this is non-NULL.
IAccessible* wxIAccessible::GetChildAccessible(int id)
{
    if (id == 0)
    {
        IAccessible* obj = this;

        obj->AddRef();
        return obj;
    }
    else
    {
        VARIANT var;
        VariantInit(& var);
        var.vt = VT_I4;
        var.lVal = id;
        IDispatch* pDispatch = nullptr;
        if (S_OK == get_accChild ( var, & pDispatch))
        {
            IAccessible* childAccessible = nullptr;
            if (pDispatch->QueryInterface(IID_IAccessible, (LPVOID*) & childAccessible) == S_OK)
            {
                pDispatch->Release();
                return childAccessible;
            }
            else
            {
                pDispatch->Release();
            }
        }
    }
    return nullptr;
}

// ----------------------------------------------------------------------------
// wxAccessible implementation
// ----------------------------------------------------------------------------

// ctors

wxAccessible::wxAccessible(wxWindow* win)
            : wxAccessibleBase(win),
              m_pIAccessible(new wxIAccessible(this))
{
    m_pIAccessible->AddRef();
}

wxAccessible::~wxAccessible()
{
    m_pIAccessible->Quiesce();
    m_pIAccessible->Release();
    if (m_pIAccessibleStd)
        m_pIAccessibleStd->Release();
}

// Gets or creates a standard interface for this object.
IAccessible *wxAccessible::GetIAccessibleStd()
{
    if (m_pIAccessibleStd)
        return m_pIAccessibleStd;

    if (GetWindow())
    {
        HRESULT retCode = ::CreateStdAccessibleObject((WXHWND) GetWindow()->GetHWND(),
                OBJID_CLIENT, IID_IAccessible, (void**) & m_pIAccessibleStd);
        if (retCode == S_OK)
            return m_pIAccessibleStd;
        else
        {
            m_pIAccessibleStd = nullptr;
            return nullptr;
        }
    }
    return nullptr;
}

namespace
{

struct SendNotification
{
    SendNotification(DWORD eventType_, WXHWND hwnd_, LONG idObject_, LONG idChild_)
        : eventType(eventType_), hwnd(hwnd_), idObject(idObject_), idChild(idChild_)
    {}

    void operator()()
    {
        ::NotifyWinEvent(eventType, hwnd, idObject, idChild);
    }

    WXHWND hwnd;
    DWORD eventType;
    LONG idObject, idChild;
};

} // anonymous namespace

// Sends an event when something changes in an accessible object.
void wxAccessible::NotifyEvent(int eventType, wxWindow* window, wxAccObject objectType,
                        int objectId)
{
    // send the notification in idle time to be sure it is sent after the change
    // was fully done in wx code
    const WXHWND hwnd = (WXHWND)window->GetHWND();
    SendNotification delayed((DWORD)eventType, hwnd, (LONG)objectType, (LONG)objectId);
    wxTheApp->CallAfter(delayed);
}

// Utilities

// Convert to Windows role
int wxConvertToWindowsRole(wxAccSystemRole wxrole)
{
    switch (wxrole)
    {
    case wxAccSystemRole::None:
        return 0;
    case wxAccSystemRole::Alert:
        return ROLE_SYSTEM_ALERT;
    case wxAccSystemRole::Animation:
        return ROLE_SYSTEM_ANIMATION;
    case wxAccSystemRole::Application:
        return ROLE_SYSTEM_APPLICATION;
    case wxAccSystemRole::Border:
        return ROLE_SYSTEM_BORDER;
    case wxAccSystemRole::ButtonDropDown:
        return ROLE_SYSTEM_BUTTONDROPDOWN;
    case wxAccSystemRole::ButtonDropDownGRID:
        return ROLE_SYSTEM_BUTTONDROPDOWNGRID;
    case wxAccSystemRole::ButtonMenu:
        return ROLE_SYSTEM_BUTTONMENU;
    case wxAccSystemRole::Caret:
        return ROLE_SYSTEM_CARET;
    case wxAccSystemRole::Cell:
        return ROLE_SYSTEM_CELL;
    case wxAccSystemRole::Character:
        return ROLE_SYSTEM_CHARACTER;
    case wxAccSystemRole::Chart:
        return ROLE_SYSTEM_CHART;
    case wxAccSystemRole::CheckButton:
        return ROLE_SYSTEM_CHECKBUTTON;
    case wxAccSystemRole::Client:
        return ROLE_SYSTEM_CLIENT;
    case wxAccSystemRole::Clock:
        return ROLE_SYSTEM_CLOCK;
    case wxAccSystemRole::Column:
        return ROLE_SYSTEM_COLUMN;
    case wxAccSystemRole::ColumnHeader:
        return ROLE_SYSTEM_COLUMNHEADER;
    case wxAccSystemRole::ComboBox:
        return ROLE_SYSTEM_COMBOBOX;
    case wxAccSystemRole::Cursor:
        return ROLE_SYSTEM_CURSOR;
    case wxAccSystemRole::Diagram:
        return ROLE_SYSTEM_DIAGRAM;
    case wxAccSystemRole::Dial:
        return ROLE_SYSTEM_DIAL;
    case wxAccSystemRole::Dialog:
        return ROLE_SYSTEM_DIALOG;
    case wxAccSystemRole::Document:
        return ROLE_SYSTEM_DOCUMENT;
    case wxAccSystemRole::Droplist:
        return ROLE_SYSTEM_DROPLIST;
    case wxAccSystemRole::Equation:
        return ROLE_SYSTEM_EQUATION;
    case wxAccSystemRole::Graphic:
        return ROLE_SYSTEM_GRAPHIC;
    case wxAccSystemRole::Grip:
        return ROLE_SYSTEM_GRIP;
    case wxAccSystemRole::Grouping:
        return ROLE_SYSTEM_GROUPING;
    case wxAccSystemRole::HelpBalloon:
        return ROLE_SYSTEM_HELPBALLOON;
    case wxAccSystemRole::HotkeyField:
        return ROLE_SYSTEM_HOTKEYFIELD;
    case wxAccSystemRole::Indicator:
        return ROLE_SYSTEM_INDICATOR;
    case wxAccSystemRole::Link:
        return ROLE_SYSTEM_LINK;
    case wxAccSystemRole::List:
        return ROLE_SYSTEM_LIST;
    case wxAccSystemRole::ListItem:
        return ROLE_SYSTEM_LISTITEM;
    case wxAccSystemRole::Menubar:
        return ROLE_SYSTEM_MENUBAR;
    case wxAccSystemRole::MenuItem:
        return ROLE_SYSTEM_MENUITEM;
    case wxAccSystemRole::MenuPopup:
        return ROLE_SYSTEM_MENUPOPUP;
    case wxAccSystemRole::Outline:
        return ROLE_SYSTEM_OUTLINE;
    case wxAccSystemRole::OutlineItem:
        return ROLE_SYSTEM_OUTLINEITEM;
    case wxAccSystemRole::PageTab:
        return ROLE_SYSTEM_PAGETAB;
    case wxAccSystemRole::PageTabList:
        return ROLE_SYSTEM_PAGETABLIST;
    case wxAccSystemRole::Pane:
        return ROLE_SYSTEM_PANE;
    case wxAccSystemRole::ProgressBar:
        return ROLE_SYSTEM_PROGRESSBAR;
    case wxAccSystemRole::PropertyPage:
        return ROLE_SYSTEM_PROPERTYPAGE;
    case wxAccSystemRole::PushButton:
        return ROLE_SYSTEM_PUSHBUTTON;
    case wxAccSystemRole::RadioButton:
        return ROLE_SYSTEM_RADIOBUTTON;
    case wxAccSystemRole::Row:
        return ROLE_SYSTEM_ROW;
    case wxAccSystemRole::RowHeader:
        return ROLE_SYSTEM_ROWHEADER;
    case wxAccSystemRole::Scrollbar:
        return ROLE_SYSTEM_SCROLLBAR;
    case wxAccSystemRole::Separator:
        return ROLE_SYSTEM_SEPARATOR;
    case wxAccSystemRole::Slider:
        return ROLE_SYSTEM_SLIDER;
    case wxAccSystemRole::Sound:
        return ROLE_SYSTEM_SOUND;
    case wxAccSystemRole::SpinButton:
        return ROLE_SYSTEM_SPINBUTTON;
    case wxAccSystemRole::StaticText:
        return ROLE_SYSTEM_STATICTEXT;
    case wxAccSystemRole::StatusBar:
        return ROLE_SYSTEM_STATUSBAR;
    case wxAccSystemRole::Table:
        return ROLE_SYSTEM_TABLE;
    case wxAccSystemRole::Text:
        return ROLE_SYSTEM_TEXT;
    case wxAccSystemRole::Titlebar:
        return ROLE_SYSTEM_TITLEBAR;
    case wxAccSystemRole::Toolbar:
        return ROLE_SYSTEM_TOOLBAR;
    case wxAccSystemRole::Tooltip:
        return ROLE_SYSTEM_TOOLTIP;
    case wxAccSystemRole::Whitespace:
        return ROLE_SYSTEM_WHITESPACE;
    case wxAccSystemRole::Window:
        return ROLE_SYSTEM_WINDOW;
    }
    return 0;
}

// Convert to Windows selection flag
int wxConvertToWindowsSelFlag(wxAccSelectionFlags wxsel)
{
    int sel = 0;

    if (wxsel & wxACC_SEL_TAKEFOCUS)
        sel |= SELFLAG_TAKEFOCUS;
    if (wxsel & wxACC_SEL_TAKESELECTION)
        sel |= SELFLAG_TAKESELECTION;
    if (wxsel & wxACC_SEL_EXTENDSELECTION)
        sel |= SELFLAG_EXTENDSELECTION;
    if (wxsel & wxACC_SEL_ADDSELECTION)
        sel |= SELFLAG_ADDSELECTION;
    if (wxsel & wxACC_SEL_REMOVESELECTION)
        sel |= SELFLAG_REMOVESELECTION;
    return sel;
}

// Convert from Windows selection flag
wxAccSelectionFlags wxConvertFromWindowsSelFlag(unsigned int sel)
{
    int wxsel = 0;

    if (sel & SELFLAG_TAKEFOCUS)
        wxsel |= wxACC_SEL_TAKEFOCUS;
    if (sel & SELFLAG_TAKESELECTION)
        wxsel |= wxACC_SEL_TAKESELECTION;
    if (sel & SELFLAG_EXTENDSELECTION)
        wxsel |= wxACC_SEL_EXTENDSELECTION;
    if (sel & SELFLAG_ADDSELECTION)
        wxsel |= wxACC_SEL_ADDSELECTION;
    if (sel & SELFLAG_REMOVESELECTION)
        wxsel |= wxACC_SEL_REMOVESELECTION;
    return (wxAccSelectionFlags) wxsel;
}


#endif  // wxUSE_OLE && wxUSE_ACCESSIBILITY
