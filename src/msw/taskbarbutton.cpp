/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/taskbarbutton.cpp
// Purpose:     Implements wxTaskBarButtonImpl class for manipulating buttons on
//              the Windows taskbar.
// Author:      Chaobin Zhang <zhchbin@gmail.com>
// Created:     2014-06-01
// Copyright:   (c) 2014 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"

#include "wx/icon.h"
#include "wx/toplevel.h"

#if wxUSE_TASKBARBUTTON

#ifdef _MSC_VER
    #pragma comment( lib, "shlwapi" )
#endif

#include "wx/msw/taskbarbutton.h"

#include "wx/msw/private/comptr.h"
#include "wx/msw/private/cotaskmemptr.h"

#if wxUSE_DYNLIB_CLASS
    #include "wx/dynlib.h"
#endif // wxUSE_DYNLIB_CLASS

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

import WX.WinDef;

// ----------------------------------------------------------------------------
// Redefine the interfaces: ITaskbarList3, IObjectCollection,
// ICustomDestinationList, IShellLink, IShellItem, IApplicationDocumentLists
// etc.
// ----------------------------------------------------------------------------

WINOLEAPI PropVariantClear(PROPVARIANT* pvar);

#ifndef PropVariantInit
#define PropVariantInit(pvar) memset ( (pvar), 0, sizeof(PROPVARIANT) )
#endif

#ifndef INFOTIPSIZE
constexpr int INFOTIPSIZE = 1024;
#endif

namespace {

// The maximum number of thumbnail toolbar buttons allowed on windows is 7.
constexpr int MAX_BUTTON_COUNT = 7;

DEFINE_GUID(wxCLSID_TaskbarList,
    0x56fdf344, 0xfd6d, 0x11d0, 0x95, 0x8a, 0x0, 0x60, 0x97, 0xc9, 0xa0, 0x90);
DEFINE_GUID(wxCLSID_DestinationList,
    0x77f10cf0, 0x3db5, 0x4966, 0xb5, 0x20, 0xb7, 0xc5, 0x4f, 0xd3,0x5e, 0xd6);
DEFINE_GUID(wxCLSID_EnumerableObjectCollection,
    0x2d3468c1, 0x36a7, 0x43b6, 0xac, 0x24, 0xd3, 0xf0, 0x2f, 0xd9, 0x60, 0x7a);
DEFINE_GUID(wxCLSID_ShellLink,
    0x00021401, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(wxIID_ICustomDestinationList,
    0x6332debf, 0x87b5, 0x4670, 0x90, 0xc0, 0x5e, 0x57, 0xb4, 0x08, 0xa4, 0x9e);
DEFINE_GUID(wxIID_ITaskbarList3,
    0xea1afb91, 0x9e28, 0x4b86, 0x90, 0xe9, 0x9e, 0x9f, 0x8a, 0x5e, 0xef, 0xaf);
DEFINE_GUID(wxIID_IPropertyStore,
    0x886d8eeb, 0x8cf2, 0x4446, 0x8d, 0x02, 0xcd, 0xba, 0x1d, 0xbd, 0xcf, 0x99);
DEFINE_GUID(wxIID_IObjectArray,
    0x92ca9dcd, 0x5622, 0x4bba, 0xa8, 0x05, 0x5e, 0x9f, 0x54, 0x1b, 0xd8, 0xc9);
DEFINE_GUID(wxIID_IObjectCollection,
    0x5632b1a4, 0xe38a, 0x400a, 0x92, 0x8a, 0xd4, 0xcd, 0x63, 0x23, 0x02, 0x95);
DEFINE_GUID(wxIID_IApplicationDocumentLists,
    0x3c594f9f, 0x9f30, 0x47a1, 0x97, 0x9a, 0xc9, 0xe8, 0x3d, 0x3d, 0x0a, 0x06);
DEFINE_GUID(wxCLSID_ApplicationDocumentLists,
    0x86bec222, 0x30f2, 0x47e0, 0x9f, 0x25, 0x60, 0xd1, 0x1c, 0xd7, 0x5c, 0x28);
DEFINE_GUID(wxIID_IUnknown,
    0x00000000, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(wxIID_IShellItem,
    0x43826d1e, 0xe718, 0x42ee, 0xbc, 0x55, 0xa1, 0xe2, 0x61, 0xc3, 0x7b, 0xfe);

using HIMAGELIST = IUnknown*;

#ifndef PROPERTYKEY_DEFINED
typedef struct _tagpropertykey
{
    GUID fmtid;
    WXDWORD pid;
} PROPERTYKEY;
#endif // !PROPERTYKEY_DEFINED

#define REFPROPERTYKEY const PROPERTYKEY &

#define DEFINE_PROPERTYKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) \
    const PROPERTYKEY name  = \
    { { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }, pid }

DEFINE_PROPERTYKEY(PKEY_Title,
    0xf29f85e0, 0x4ff9, 0x1068, 0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9, 2);
DEFINE_PROPERTYKEY(PKEY_AppUserModel_IsDestListSeparator,
    0x9f4c2855, 0x9f79, 0x4b39, 0xa8, 0xd0, 0xe1, 0xd4, 0x2d, 0xe1, 0xd5, 0xf3, 6);
DEFINE_PROPERTYKEY(PKEY_Link_Arguments,
    0x436f2667, 0x14e2, 0x4feb, 0xb3, 0x0a, 0x14, 0x6c, 0x53, 0xb5, 0xb6, 0x74, 100);

#define IShellLink      wxIShellLinkW

DEFINE_GUID(wxIID_IShellLink,
    0x000214F9, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

} // anonymous namespace

struct wxITaskbarList : public IUnknown
{
    virtual HRESULT wxSTDCALL HrInit() = 0;
    virtual HRESULT wxSTDCALL AddTab(WXHWND) = 0;
    virtual HRESULT wxSTDCALL DeleteTab(WXHWND) = 0;
    virtual HRESULT wxSTDCALL ActivateTab(WXHWND) = 0;
    virtual HRESULT wxSTDCALL SetActiveAlt(WXHWND) = 0;
};

struct wxITaskbarList2 : public wxITaskbarList
{
    virtual HRESULT wxSTDCALL MarkFullscreenWindow(WXHWND, BOOL) = 0;
};

struct wxIShellLinkA : public IUnknown
{
    virtual HRESULT wxSTDCALL GetPath(LPSTR, int, WIN32_FIND_DATAA*, WXDWORD) = 0;
    virtual HRESULT wxSTDCALL GetIDList(LPITEMIDLIST *ppidl) = 0;
    virtual HRESULT wxSTDCALL SetIDList(LPCITEMIDLIST pidl) = 0;
    virtual HRESULT wxSTDCALL GetDescription(LPSTR, int) = 0;
    virtual HRESULT wxSTDCALL SetDescription(LPCSTR) = 0;
    virtual HRESULT wxSTDCALL GetWorkingDirectory(LPSTR, int) = 0;
    virtual HRESULT wxSTDCALL SetWorkingDirectory(LPCSTR) = 0;
    virtual HRESULT wxSTDCALL GetArguments(LPSTR, int) = 0;
    virtual HRESULT wxSTDCALL SetArguments(LPCSTR) = 0;
    virtual HRESULT wxSTDCALL GetHotkey(WXWORD*) = 0;
    virtual HRESULT wxSTDCALL SetHotkey(WXWORD) = 0;
    virtual HRESULT wxSTDCALL GetShowCmd(int*) = 0;
    virtual HRESULT wxSTDCALL SetShowCmd(int) = 0;
    virtual HRESULT wxSTDCALL GetIconLocation(LPSTR, int, int*) = 0;
    virtual HRESULT wxSTDCALL SetIconLocation(LPCSTR, int) = 0;
    virtual HRESULT wxSTDCALL SetRelativePath(LPCSTR, WXDWORD) = 0;
    virtual HRESULT wxSTDCALL Resolve(WXHWND, WXDWORD) = 0;
    virtual HRESULT wxSTDCALL SetPath(LPCSTR) = 0;
};

struct wxIShellLinkW : public IUnknown
{
    virtual HRESULT wxSTDCALL GetPath(LPWSTR, int, WIN32_FIND_DATAW*, WXDWORD) = 0;
    virtual HRESULT wxSTDCALL GetIDList(LPITEMIDLIST *ppidl) = 0;
    virtual HRESULT wxSTDCALL SetIDList(LPCITEMIDLIST pidl) = 0;
    virtual HRESULT wxSTDCALL GetDescription(LPWSTR, int) = 0;
    virtual HRESULT wxSTDCALL SetDescription(LPCWSTR) = 0;
    virtual HRESULT wxSTDCALL GetWorkingDirectory(LPWSTR, int) = 0;
    virtual HRESULT wxSTDCALL SetWorkingDirectory(LPCWSTR) = 0;
    virtual HRESULT wxSTDCALL GetArguments(LPWSTR, int) = 0;
    virtual HRESULT wxSTDCALL SetArguments(LPCWSTR) = 0;
    virtual HRESULT wxSTDCALL GetHotkey(WXWORD*) = 0;
    virtual HRESULT wxSTDCALL SetHotkey(WXWORD) = 0;
    virtual HRESULT wxSTDCALL GetShowCmd(int*) = 0;
    virtual HRESULT wxSTDCALL SetShowCmd(int) = 0;
    virtual HRESULT wxSTDCALL GetIconLocation(LPWSTR, int, int*) = 0;
    virtual HRESULT wxSTDCALL SetIconLocation(LPCWSTR, int) = 0;
    virtual HRESULT wxSTDCALL SetRelativePath(LPCWSTR, WXDWORD) = 0;
    virtual HRESULT wxSTDCALL Resolve(WXHWND, WXDWORD) = 0;
    virtual HRESULT wxSTDCALL SetPath(LPCWSTR) = 0;
};

namespace
{

inline HRESULT InitPropVariantFromBoolean(BOOL fVal, PROPVARIANT *ppropvar)
{
    ppropvar->vt = VT_BOOL;
    ppropvar->boolVal = fVal ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

inline HRESULT InitPropVariantFromString(PCWSTR psz, PROPVARIANT *ppropvar)
{
    HRESULT hr = E_FAIL;
    ppropvar->vt = VT_LPWSTR;

#if wxUSE_DYNLIB_CLASS
    typedef HRESULT (WINAPI *SHStrDupW_t)(LPCWSTR, LPWSTR*);
    static SHStrDupW_t s_pfnSHStrDupW = nullptr;
    if ( !s_pfnSHStrDupW )
    {
        wxDynamicLibrary dll("shlwapi.dll");
        if ( dll.IsLoaded() )
        {
            s_pfnSHStrDupW = (SHStrDupW_t)dll.GetSymbol("SHStrDupW");
        }
    }

    if ( s_pfnSHStrDupW )
    {
        hr = s_pfnSHStrDupW(psz, &ppropvar->pwszVal);
    }
#elif defined (_MSC_VER)
    hr = SHStrDupW(psz, &ppropvar->pwszVal);
#else
    wxUnusedVar(psz);
#endif

    if ( FAILED(hr) )
    {
        PropVariantInit(ppropvar);
    }
    return hr;
}

THUMBBUTTONFLAGS GetNativeThumbButtonFlags(const wxThumbBarButton& button)
{
    WXUINT flags = 0;
    flags |= (button.IsEnable() ? THBF_ENABLED : THBF_DISABLED);
    if ( button.IsDismissOnClick() )
        flags |= THBF_DISMISSONCLICK;
    if ( !button.HasBackground() )
        flags |= THBF_NOBACKGROUND;
    if ( !button.IsShown() )
        flags |= THBF_HIDDEN;
    if ( !button.IsInteractive() )
        flags |= THBF_NONINTERACTIVE;
    return static_cast<THUMBBUTTONFLAGS>(flags);
}

bool AddShellLink(IObjectCollection *collection,
                  const wxTaskBarJumpListItem& item)
{
    wxCOMPtr<IShellLink> shellLink;
    wxCOMPtr<IPropertyStore> propertyStore;

    HRESULT hr = CoCreateInstance
                 (
                     wxCLSID_ShellLink,
                     nullptr,
                     CLSCTX_INPROC_SERVER,
                     wxIID_IShellLink,
                     reinterpret_cast<void**> (&(shellLink))
                 );
    if ( FAILED(hr) )
    {
        wxLogApiError("CoCreateInstance(wxCLSID_ShellLink)", hr);
        return false;
    }

    if ( item.GetType() == wxTaskBarJumpListItemType::Task ||
         item.GetType() == wxTaskBarJumpListItemType::Destination )
    {
        if ( !item.GetFilePath().empty() )
            shellLink->SetPath(boost::nowide::widen(item.GetFilePath()).c_str());
        if ( !item.GetArguments().empty() )
            shellLink->SetArguments(boost::nowide::widen(item.GetArguments()).c_str());
        if ( !item.GetIconPath().empty() )
        {
            shellLink->SetIconLocation(boost::nowide::widen(item.GetIconPath()).c_str(),
                                        item.GetIconIndex());
        }
        if ( !item.GetTooltip().empty() )
            shellLink->SetDescription(boost::nowide::widen(item.GetTooltip()).c_str());
    }

    hr = shellLink->QueryInterface(wxIID_IPropertyStore,
                                   reinterpret_cast<void**>(&(propertyStore)));
    if ( FAILED(hr) )
    {
        wxLogApiError("IShellLink(QueryInterface)", hr);
        return false;
    }

    PROPVARIANT pv;
    if ( item.GetType() == wxTaskBarJumpListItemType::Task ||
         item.GetType() == wxTaskBarJumpListItemType::Destination )
    {
        hr = InitPropVariantFromString(boost::nowide::widen(item.GetTitle()).c_str(), &pv);
        if ( SUCCEEDED(hr) )
        {
            hr = propertyStore->SetValue(PKEY_Title, pv);
        }
    }
    else if ( item.GetType() == wxTaskBarJumpListItemType::Separator )
    {
        hr = InitPropVariantFromBoolean(TRUE, &pv);
        if ( SUCCEEDED(hr) )
        {
            hr = propertyStore->SetValue(PKEY_AppUserModel_IsDestListSeparator,
                                         pv);
        }
    }

    // Save the changes we made to the property store.
    propertyStore->Commit();
    PropVariantClear(&pv);

    // Add this IShellLink object to the given collection.
    hr = collection->AddObject(shellLink);

    return SUCCEEDED(hr);
}

wxTaskBarJumpListItem* GetItemFromIShellLink(IShellLink* link)
{
    if ( !link )
        return nullptr;

    wxTaskBarJumpListItem* item =
        new wxTaskBarJumpListItem(nullptr, wxTaskBarJumpListItemType::Destination);

    wxCOMPtr<IPropertyStore> linkProps;
    HRESULT hr = link->QueryInterface
                 (
                     wxIID_IPropertyStore,
                     reinterpret_cast<void **>(&linkProps)
                 );
    if ( FAILED(hr) )
    {
        wxLogApiError("IShellLink::QueryInterface", hr);
        return nullptr;
    }

    PROPVARIANT var;
    linkProps->GetValue(PKEY_Link_Arguments, &var);

    std::wstring wArgs{var.pwszVal};
    item->SetArguments(boost::nowide::narrow(wArgs));
    PropVariantClear(&var);

    static constexpr int bufferSize = 2048;
    wchar_t buffer[bufferSize];

    link->GetDescription(buffer, INFOTIPSIZE);
    item->SetTooltip(boost::nowide::narrow(buffer));

    int dummyIndex;
    link->GetIconLocation(buffer, bufferSize - 1, &dummyIndex);
    item->SetIconPath(boost::nowide::narrow(buffer));

    link->GetPath(buffer, bufferSize - 1, nullptr, 0x1);
    item->SetFilePath(boost::nowide::narrow(buffer));

    return item;
}

wxTaskBarJumpListItem* GetItemFromIShellItem(IShellItem *shellItem)
{
    if ( !shellItem )
        return nullptr;

    wxTaskBarJumpListItem *item =
        new wxTaskBarJumpListItem(nullptr, wxTaskBarJumpListItemType::Destination);

    wxCoTaskMemPtr<wchar_t> name;
    shellItem->GetDisplayName(SIGDN_FILESYSPATH, &name);
    std::wstring tmpName{*name};
    item->SetFilePath(boost::nowide::narrow(tmpName));
    return item;
}

IObjectCollection* CreateObjectCollection()
{
    IObjectCollection* collection;

    HRESULT hr;
    hr = CoCreateInstance
         (
             wxCLSID_EnumerableObjectCollection,
             nullptr,
             CLSCTX_INPROC,
             wxIID_IObjectCollection,
             reinterpret_cast<void**>(&(collection))
         );
    if ( FAILED(hr) )
    {
        wxLogApiError("CoCreateInstance(wxCLSID_EnumerableObjectCollection)",
                      hr);
        return nullptr;
    }

    return collection;
}

} // namespace

class wxITaskbarList3 : public wxITaskbarList2
{
public:
    virtual HRESULT wxSTDCALL SetProgressValue(WXHWND, ULONGLONG, ULONGLONG) = 0;
    virtual HRESULT wxSTDCALL SetProgressState(WXHWND, TBPFLAG) = 0;
    virtual HRESULT wxSTDCALL RegisterTab(WXHWND, WXHWND) = 0;
    virtual HRESULT wxSTDCALL UnregisterTab(WXHWND) = 0;
    virtual HRESULT wxSTDCALL SetTabOrder(WXHWND, WXHWND) = 0;
    virtual HRESULT wxSTDCALL SetTabActive(WXHWND, WXHWND, WXDWORD) = 0;
    virtual HRESULT wxSTDCALL ThumbBarAddButtons(WXHWND, WXUINT, LPTHUMBBUTTON) = 0;
    virtual
        HRESULT wxSTDCALL ThumbBarUpdateButtons(WXHWND, WXUINT, LPTHUMBBUTTON) = 0;
    virtual HRESULT wxSTDCALL ThumbBarSetImageList(WXHWND, ::HIMAGELIST) = 0;
    virtual HRESULT wxSTDCALL SetOverlayIcon(WXHWND, WXHICON, LPCWSTR) = 0;
    virtual HRESULT wxSTDCALL SetThumbnailTooltip(WXHWND, LPCWSTR pszTip) = 0;
    virtual HRESULT wxSTDCALL SetThumbnailClip(WXHWND, RECT *) = 0;
};

// ----------------------------------------------------------------------------
// wxTaskBarJumpListImpl: definition of class for internal taskbar jump list
// implementation.
// ----------------------------------------------------------------------------
class wxTaskBarJumpListImpl
{
public:
    explicit wxTaskBarJumpListImpl(wxTaskBarJumpList *jumpList = nullptr,
                          const std::string& appID = "");
    virtual ~wxTaskBarJumpListImpl();
    void ShowRecentCategory(bool shown = true);
    void HideRecentCategory();
    void ShowFrequentCategory(bool shown = true);
    void HideFrequentCategory();

    wxTaskBarJumpListCategory& GetTasks();
    const wxTaskBarJumpListCategory& GetFrequentCategory();
    const wxTaskBarJumpListCategory& GetRecentCategory();
    const wxTaskBarJumpListCategories& GetCustomCategories() const;

    void AddCustomCategory(wxTaskBarJumpListCategory* category);
    wxTaskBarJumpListCategory* RemoveCustomCategory(const std::string& title);
    void DeleteCustomCategory(const std::string& title);
    void Update();

private:
    bool BeginUpdate();
    bool CommitUpdate();
    void AddTasksToDestinationList();
    void AddCustomCategoriesToDestinationList();
    void LoadKnownCategory(const std::string& title);

    // Application User Model ID.
    std::string m_appID;

    wxTaskBarJumpListCategories m_customCategories;

    wxTaskBarJumpList *m_jumpList;

    wxCOMPtr<ICustomDestinationList>    m_destinationList;
    wxCOMPtr<IObjectArray>              m_objectArray;

    std::unique_ptr<wxTaskBarJumpListCategory> m_tasks;
    std::unique_ptr<wxTaskBarJumpListCategory> m_frequent;
    std::unique_ptr<wxTaskBarJumpListCategory> m_recent;

    bool m_recent_visible;
    bool m_frequent_visible;
};

// ----------------------------------------------------------------------------
// wxThumbBarButton Implementation.
// ----------------------------------------------------------------------------

wxThumbBarButton::wxThumbBarButton(int id,
                                   const wxIcon& icon,
                                   const std::string& tooltip,
                                   bool enable,
                                   bool dismissOnClick,
                                   bool hasBackground,
                                   bool shown,
                                   bool interactive)
    : m_id(id),
      m_icon(icon),
      m_tooltip(tooltip),
      m_enable(enable),
      m_dismissOnClick(dismissOnClick),
      m_hasBackground(hasBackground),
      m_shown(shown),
      m_interactive(interactive)
{
}

bool wxThumbBarButton::Create(int id,
                              const wxIcon& icon,
                              const std::string& tooltip,
                              bool enable,
                              bool dismissOnClick,
                              bool hasBackground,
                              bool shown,
                              bool interactive)
{
    m_id = id;
    m_icon = icon;
    m_tooltip = tooltip;
    m_enable = enable;
    m_dismissOnClick = dismissOnClick;
    m_hasBackground = hasBackground;
    m_shown = shown;
    m_interactive = interactive;
    return true;
}

void wxThumbBarButton::Enable(bool enable)
{
    if ( m_enable != enable )
    {
        m_enable = enable;
        UpdateParentTaskBarButton();
    }
}

void wxThumbBarButton::SetHasBackground(bool has)
{
    if ( m_hasBackground != has )
    {
        m_hasBackground = has;
        UpdateParentTaskBarButton();
    }
}

void wxThumbBarButton::EnableDismissOnClick(bool enable)
{
    if ( m_dismissOnClick != enable )
    {
        m_dismissOnClick = enable;
        UpdateParentTaskBarButton();
    }
}

void wxThumbBarButton::Show(bool shown)
{
    if ( m_shown != shown )
    {
        m_shown = shown;
        UpdateParentTaskBarButton();
    }
}

void wxThumbBarButton::SetInteractive(bool interactive)
{
    if ( m_interactive != interactive )
    {
        m_interactive = interactive;
        UpdateParentTaskBarButton();
    }
}

bool wxThumbBarButton::UpdateParentTaskBarButton()
{
    if ( !m_taskBarButtonParent )
        return false;

    return dynamic_cast<wxTaskBarButtonImpl*>(m_taskBarButtonParent)->InitOrUpdateThumbBarButtons();
}

// ----------------------------------------------------------------------------
// wxTaskBarButtonImpl Implementation.
// ----------------------------------------------------------------------------

/* static */
std::unique_ptr<wxTaskBarButton> wxTaskBarButton::Create(wxWindow* parent)
{
    wxITaskbarList3* taskbarList = nullptr;

    HRESULT hr = ::CoCreateInstance
                 (
                    wxCLSID_TaskbarList,
                    nullptr,
                    CLSCTX_INPROC_SERVER,
                    wxIID_ITaskbarList3,
                    reinterpret_cast<void **>(&taskbarList)
                 );
    if ( FAILED(hr) )
    {
        // Don't log this error, it may be normal when running under XP.
        return nullptr;
    }

    hr = taskbarList->HrInit();
    if ( FAILED(hr) )
    {
        // This is however unexpected.
        wxLogApiError("ITaskbarList3::Init", hr);

        taskbarList->Release();
        return nullptr;
    }

    return std::make_unique<wxTaskBarButtonImpl>(taskbarList, parent);
}

wxTaskBarButtonImpl::wxTaskBarButtonImpl(wxITaskbarList3* taskbarList,
                                         wxWindow* parent)
    : m_parent(parent),
      m_taskbarList(taskbarList),
      m_progressRange(0),
      m_progressValue(0),
      m_progressState(wxTaskBarButtonState::NoProgress),
      m_hasInitThumbnailToolbar(false)
{
}

wxTaskBarButtonImpl::~wxTaskBarButtonImpl()
{
    if ( m_taskbarList )
      m_taskbarList->Release();

    for ( wxThumbBarButtons::iterator iter = m_thumbBarButtons.begin();
          iter != m_thumbBarButtons.end();
          ++iter)
    {
        delete (*iter);
    }
    m_thumbBarButtons.clear();
}

void wxTaskBarButtonImpl::Realize()
{
    // (Re-)apply all settings: this is needed if settings were made before the
    // create message was sent, taskbar icon is hidden and shown again or
    // explorer is restarted
    SetProgressRange(m_progressRange);
    SetProgressState(m_progressState);
    if ( m_progressValue > 0 )
        SetProgressValue(m_progressValue);
    SetThumbnailTooltip(m_thumbnailTooltip);
    SetOverlayIcon(m_overlayIcon, m_overlayIconDescription);
    if ( !m_thumbnailClipRect.IsEmpty() )
        SetThumbnailClip(m_thumbnailClipRect);
    m_hasInitThumbnailToolbar = false;
    InitOrUpdateThumbBarButtons();
}

void wxTaskBarButtonImpl::SetProgressRange(int range)
{
    m_progressRange = range;
    if ( m_progressRange == 0 )
        SetProgressState(wxTaskBarButtonState::NoProgress);
}

void wxTaskBarButtonImpl::SetProgressValue(int value)
{
    m_progressValue = value;
    m_taskbarList->SetProgressValue(m_parent->GetHWND(), value, m_progressRange);
}

void wxTaskBarButtonImpl::PulseProgress()
{
    SetProgressState(wxTaskBarButtonState::Indeterminate);
}

void wxTaskBarButtonImpl::Show(bool show)
{
    if ( show )
        m_taskbarList->AddTab(m_parent->GetHWND());
    else
        m_taskbarList->DeleteTab(m_parent->GetHWND());
}

void wxTaskBarButtonImpl::Hide()
{
    Show(false);
}

void wxTaskBarButtonImpl::SetThumbnailTooltip(const std::string& tooltip)
{
    m_thumbnailTooltip = tooltip;
    m_taskbarList->SetThumbnailTooltip(m_parent->GetHWND(), boost::nowide::widen(tooltip).c_str());
}

void wxTaskBarButtonImpl::SetProgressState(wxTaskBarButtonState state)
{
    m_progressState = state;
    m_taskbarList->SetProgressState(m_parent->GetHWND(), static_cast<TBPFLAG>(state));
}

void wxTaskBarButtonImpl::SetOverlayIcon(const wxIcon& icon,
                                         const std::string& description)
{
    m_overlayIcon = icon;
    m_overlayIconDescription = description;
    m_taskbarList->SetOverlayIcon(m_parent->GetHWND(),
                                  GetHiconOf(icon),
                                  boost::nowide::widen(description).c_str());
}

void wxTaskBarButtonImpl::SetThumbnailClip(const wxRect& rect)
{
    m_thumbnailClipRect = rect;
    RECT rc;
    wxCopyRectToRECT(rect, rc);
    m_taskbarList->SetThumbnailClip(m_parent->GetHWND(), rect.IsEmpty() ? nullptr : &rc);
}

void wxTaskBarButtonImpl::SetThumbnailContents(const wxWindow *child)
{
    SetThumbnailClip(child->GetRect());
}

bool wxTaskBarButtonImpl::AppendThumbBarButton(wxThumbBarButton *button)
{
    wxASSERT_MSG( m_thumbBarButtons.size() < MAX_BUTTON_COUNT,
                  "Number of ThumbBarButtons and separators is limited to 7" );

    button->SetParent(this);
    m_thumbBarButtons.push_back(button);
    return InitOrUpdateThumbBarButtons();
}

bool wxTaskBarButtonImpl::AppendSeparatorInThumbBar()
{
    wxASSERT_MSG( m_thumbBarButtons.size() < MAX_BUTTON_COUNT,
                  "Number of ThumbBarButtons and separators is limited to 7" );

    // Append a disable ThumbBarButton without background can simulate the
    // behavior of appending a separator.
    wxThumbBarButton *separator = new wxThumbBarButton(wxID_ANY,
                                                       wxNullIcon,
                                                       "",
                                                       false,
                                                       false,
                                                       false);
    m_thumbBarButtons.push_back(separator);
    return InitOrUpdateThumbBarButtons();
}

bool wxTaskBarButtonImpl::InsertThumbBarButton(size_t pos,
                                               wxThumbBarButton *button)
{
    wxASSERT_MSG( m_thumbBarButtons.size() < MAX_BUTTON_COUNT,
                  "Number of ThumbBarButtons and separators is limited to 7" );
    wxASSERT_MSG( pos <= m_thumbBarButtons.size(),
                  "Invalid index when inserting the button" );

    button->SetParent(this);
    m_thumbBarButtons.insert(m_thumbBarButtons.begin() + pos, button);
    return InitOrUpdateThumbBarButtons();
}

wxThumbBarButton* wxTaskBarButtonImpl::RemoveThumbBarButton(
    wxThumbBarButton *button)
{
    return RemoveThumbBarButton(button->GetID());
}

wxThumbBarButton* wxTaskBarButtonImpl::RemoveThumbBarButton(int id)
{
    for ( wxThumbBarButtons::iterator iter = m_thumbBarButtons.begin();
          iter != m_thumbBarButtons.end();
          ++iter )
    {
        wxThumbBarButton* button = *iter;
        if ( id == button->GetID() )
        {
            m_thumbBarButtons.erase(iter);
            button->SetParent(nullptr);
            InitOrUpdateThumbBarButtons();
            return button;
        }
    }

    return nullptr;
}

bool wxTaskBarButtonImpl::InitOrUpdateThumbBarButtons()
{
    THUMBBUTTON buttons[MAX_BUTTON_COUNT];
    HRESULT hr;

    for ( size_t i = 0; i < MAX_BUTTON_COUNT; ++i )
    {
        memset(&buttons[i], 0, sizeof buttons[i]);
        buttons[i].iId = i;
        buttons[i].dwFlags = THBF_HIDDEN;
        buttons[i].dwMask = static_cast<THUMBBUTTONMASK>(THB_FLAGS);
    }

    for ( size_t i = 0; i < m_thumbBarButtons.size(); ++i )
    {
        buttons[i].hIcon = GetHiconOf(m_thumbBarButtons[i]->GetIcon());
        buttons[i].dwFlags = GetNativeThumbButtonFlags(*m_thumbBarButtons[i]);
        buttons[i].dwMask = static_cast<THUMBBUTTONMASK>(THB_ICON | THB_FLAGS);
        std::string tooltip = m_thumbBarButtons[i]->GetTooltip();
        if ( tooltip.empty() )
            continue;

        // Truncate the tooltip if its length longer than szTip(THUMBBUTTON)
        // allowed length (260).
        tooltip.resize(260);
        wxStrlcpy(buttons[i].szTip, boost::nowide::widen(tooltip).c_str(), tooltip.length());
        buttons[i].dwMask =
            static_cast<THUMBBUTTONMASK>(buttons[i].dwMask | THB_TOOLTIP);
    }

    if ( !m_hasInitThumbnailToolbar )
    {
        hr = m_taskbarList->ThumbBarAddButtons(m_parent->GetHWND(),
                                               MAX_BUTTON_COUNT,
                                               buttons);
        if ( FAILED(hr) )
        {
            wxLogApiError("ITaskbarList3::ThumbBarAddButtons", hr);
        }
        m_hasInitThumbnailToolbar = true;
    }
    else
    {
        hr = m_taskbarList->ThumbBarUpdateButtons(m_parent->GetHWND(),
                                                  MAX_BUTTON_COUNT,
                                                  buttons);
        if ( FAILED(hr) )
        {
            wxLogApiError("ITaskbarList3::ThumbBarUpdateButtons", hr);
        }
    }

    return SUCCEEDED(hr);
}

wxThumbBarButton* wxTaskBarButtonImpl::GetThumbBarButtonByIndex(size_t index)
{
    if ( index >= m_thumbBarButtons.size() )
        return nullptr;

    return m_thumbBarButtons[index];
}

// ----------------------------------------------------------------------------
// wxTaskBarJumpListItem Implementation.
// ----------------------------------------------------------------------------
wxTaskBarJumpListItem::wxTaskBarJumpListItem(wxTaskBarJumpListCategory *parent,
                                             wxTaskBarJumpListItemType type,
                                             const std::string& title,
                                             const std::string& filePath,
                                             const std::string& arguments,
                                             const std::string& tooltip,
                                             const std::string& iconPath,
                                             int iconIndex)
    : m_parentCategory(parent),
      m_type(type),
      m_title(title),
      m_filePath(filePath),
      m_arguments(arguments),
      m_tooltip(tooltip),
      m_iconPath(iconPath),
      m_iconIndex(iconIndex)
{
}

wxTaskBarJumpListItemType wxTaskBarJumpListItem::GetType() const
{
    return m_type;
}

void wxTaskBarJumpListItem::SetType(wxTaskBarJumpListItemType type)
{
    m_type = type;
    if ( m_parentCategory )
        m_parentCategory->Update();
}

const std::string& wxTaskBarJumpListItem::GetTitle() const
{
    return m_title;
}

void wxTaskBarJumpListItem::SetTitle(const std::string& title)
{
    m_title = title;
    if ( m_parentCategory )
        m_parentCategory->Update();
}

const std::string& wxTaskBarJumpListItem::GetFilePath() const
{
    return m_filePath;
}

void wxTaskBarJumpListItem::SetFilePath(const std::string& filePath)
{
    m_filePath = filePath;
    if ( m_parentCategory )
        m_parentCategory->Update();
}

const std::string& wxTaskBarJumpListItem::GetArguments() const
{
    return m_arguments;
}

void wxTaskBarJumpListItem::SetArguments(const std::string& arguments)
{
    m_arguments = arguments;
    if ( m_parentCategory )
        m_parentCategory->Update();
}

const std::string& wxTaskBarJumpListItem::GetTooltip() const
{
    return m_tooltip;
}

void wxTaskBarJumpListItem::SetTooltip(const std::string& tooltip)
{
    m_tooltip = tooltip;
    if ( m_parentCategory )
        m_parentCategory->Update();
}

const std::string& wxTaskBarJumpListItem::GetIconPath() const
{
    return m_iconPath;
}

void wxTaskBarJumpListItem::SetIconPath(const std::string& iconPath)
{
    m_iconPath = iconPath;
    if ( m_parentCategory )
        m_parentCategory->Update();
}

int wxTaskBarJumpListItem::GetIconIndex() const
{
    return m_iconIndex;
}

void wxTaskBarJumpListItem::SetIconIndex(int iconIndex)
{
    m_iconIndex = iconIndex;
    if ( m_parentCategory )
        m_parentCategory->Update();
}

wxTaskBarJumpListCategory* wxTaskBarJumpListItem::GetCategory() const
{
    return m_parentCategory;
}

void wxTaskBarJumpListItem::SetCategory(wxTaskBarJumpListCategory *category)
{
    m_parentCategory = category;
}

// ----------------------------------------------------------------------------
// wxTaskBarJumpListCategory Implementation.
// ----------------------------------------------------------------------------
wxTaskBarJumpListCategory::wxTaskBarJumpListCategory(wxTaskBarJumpList *parent,
                                                     const std::string& title)
    : m_parent(parent),
      m_title(title)
{
}

wxTaskBarJumpListCategory::~wxTaskBarJumpListCategory()
{
    for ( wxTaskBarJumpListItems::iterator it = m_items.begin();
          it != m_items.end();
          ++it )
    {
        delete *it;
    }
}

wxTaskBarJumpListItem*
wxTaskBarJumpListCategory::Append(wxTaskBarJumpListItem *item)
{
    m_items.push_back(item);
    item->SetCategory(this);
    Update();

    return item;
}

void wxTaskBarJumpListCategory::Delete(wxTaskBarJumpListItem *item)
{
    item = Remove(item);
    item->SetCategory(nullptr);
    Update();

    if ( item )
        delete item;
}

wxTaskBarJumpListItem*
wxTaskBarJumpListCategory::Remove(wxTaskBarJumpListItem *item)
{
    for (wxTaskBarJumpListItems::iterator it = m_items.begin();
         it != m_items.end();
         ++it)
    {
        if ( *it == item )
        {
            m_items.erase(it);
            item->SetCategory(nullptr);
            Update();
            return item;
        }
    }

    return nullptr;
}

wxTaskBarJumpListItem*
wxTaskBarJumpListCategory::FindItemByPosition(size_t pos) const
{
    wxASSERT_MSG( pos < m_items.size(), "invalid pos." );
    return m_items[pos];
}

wxTaskBarJumpListItem*
wxTaskBarJumpListCategory::Insert(size_t pos, wxTaskBarJumpListItem *item)
{
    wxASSERT_MSG( pos <= m_items.size(), "invalid pos." );
    m_items.insert(m_items.begin() + pos, item);
    item->SetCategory(this);
    Update();

    return item;
}

wxTaskBarJumpListItem*
wxTaskBarJumpListCategory::Prepend(wxTaskBarJumpListItem *item)
{
    return Insert(0, item);
}

void wxTaskBarJumpListCategory::SetTitle(const std::string& title)
{
    m_title = title;
    Update();
}

const std::string& wxTaskBarJumpListCategory::GetTitle() const
{
    return m_title;
}

const wxTaskBarJumpListItems& wxTaskBarJumpListCategory::GetItems() const
{
    return m_items;
}

void wxTaskBarJumpListCategory::Update()
{
    if ( m_parent )
        m_parent->Update();
}

// ----------------------------------------------------------------------------
// wxTaskBarJumpList Implementation.
// ----------------------------------------------------------------------------
wxTaskBarJumpList::wxTaskBarJumpList(const std::string& appID)
    : m_jumpListImpl(new wxTaskBarJumpListImpl(this, appID))
{
}

wxTaskBarJumpList::~wxTaskBarJumpList()
{
    delete m_jumpListImpl;
}

wxTaskBarJumpListCategory& wxTaskBarJumpList::GetTasks() const
{
    return m_jumpListImpl->GetTasks();
}

void wxTaskBarJumpList::ShowRecentCategory(bool shown)
{
    m_jumpListImpl->ShowRecentCategory(shown);
}

void wxTaskBarJumpList::HideRecentCategory()
{
    m_jumpListImpl->HideRecentCategory();
}

void wxTaskBarJumpList::ShowFrequentCategory(bool shown)
{
    m_jumpListImpl->ShowFrequentCategory(shown);
}

void wxTaskBarJumpList::HideFrequentCategory()
{
    m_jumpListImpl->HideFrequentCategory();
}

const wxTaskBarJumpListCategory& wxTaskBarJumpList::GetFrequentCategory() const
{
    return m_jumpListImpl->GetFrequentCategory();
}

const wxTaskBarJumpListCategory& wxTaskBarJumpList::GetRecentCategory() const
{
    return m_jumpListImpl->GetRecentCategory();
}

const wxTaskBarJumpListCategories&
wxTaskBarJumpList::GetCustomCategories() const
{
    return m_jumpListImpl->GetCustomCategories();
}

void wxTaskBarJumpList::AddCustomCategory(wxTaskBarJumpListCategory* category)
{
    m_jumpListImpl->AddCustomCategory(category);
}

wxTaskBarJumpListCategory* wxTaskBarJumpList::RemoveCustomCategory(
    const std::string& title)
{
    return m_jumpListImpl->RemoveCustomCategory(title);
}

void wxTaskBarJumpList::DeleteCustomCategory(const std::string& title)
{
    m_jumpListImpl->DeleteCustomCategory(title);
}

void wxTaskBarJumpList::Update()
{
    m_jumpListImpl->Update();
}

// ----------------------------------------------------------------------------
// wxTaskBarJumpListImpl Implementation.
// ----------------------------------------------------------------------------
wxTaskBarJumpListImpl::wxTaskBarJumpListImpl(wxTaskBarJumpList *jumpList,
                                             const std::string& appID)
    : m_jumpList(jumpList),
      m_destinationList(nullptr),
      m_appID(appID)
{
    HRESULT hr = CoCreateInstance
                 (
                    wxCLSID_DestinationList,
                    nullptr,
                    CLSCTX_INPROC_SERVER,
                    wxIID_ICustomDestinationList,
                    reinterpret_cast<void**> (&(m_destinationList))
                );
    if ( FAILED(hr) )
    {
        wxLogApiError("CoCreateInstance(wxCLSID_DestinationList)", hr);
        return;
    }
}

wxTaskBarJumpListImpl::~wxTaskBarJumpListImpl()
{
    for ( wxTaskBarJumpListCategories::iterator it = m_customCategories.begin();
          it != m_customCategories.end();
          ++it )
    {
        delete *it;
    }
}

void wxTaskBarJumpListImpl::Update()
{
    if ( !BeginUpdate() )
        return;

    AddTasksToDestinationList();
    AddCustomCategoriesToDestinationList();
    if ( m_recent_visible )
        m_destinationList->AppendKnownCategory(KDC_RECENT);
    if ( m_frequent_visible )
        m_destinationList->AppendKnownCategory(KDC_FREQUENT);
    CommitUpdate();
}

wxTaskBarJumpListCategory& wxTaskBarJumpListImpl::GetTasks()
{
    if ( m_tasks.get() == nullptr )
        m_tasks.reset(new wxTaskBarJumpListCategory(m_jumpList, "Tasks"));

    return *(m_tasks.get());
}

void wxTaskBarJumpListImpl::ShowRecentCategory(bool shown)
{
    m_recent_visible = shown;
}

void wxTaskBarJumpListImpl::HideRecentCategory()
{
    ShowRecentCategory(false);
}

void wxTaskBarJumpListImpl::ShowFrequentCategory(bool shown)
{
    m_frequent_visible = shown;
}

void wxTaskBarJumpListImpl::HideFrequentCategory()
{
    ShowFrequentCategory(false);
}

const wxTaskBarJumpListCategory& wxTaskBarJumpListImpl::GetFrequentCategory()
{
    std::string title = "Frequent";
    if ( m_frequent.get() == nullptr )
        m_frequent.reset(new wxTaskBarJumpListCategory(m_jumpList, title));
    LoadKnownCategory(title);

    return *m_frequent.get();
}

const wxTaskBarJumpListCategory& wxTaskBarJumpListImpl::GetRecentCategory()
{
    std::string title = "Recent";
    if ( m_recent.get() == nullptr )
        m_recent.reset(new wxTaskBarJumpListCategory(m_jumpList, title));
    LoadKnownCategory(title);

    return *m_recent.get();
}

const wxTaskBarJumpListCategories&
wxTaskBarJumpListImpl::GetCustomCategories() const
{
    return m_customCategories;
}

void
wxTaskBarJumpListImpl::AddCustomCategory(wxTaskBarJumpListCategory *category)
{
    wxASSERT_MSG( category != nullptr, "Invalid category." );
    m_customCategories.push_back(category);
}

wxTaskBarJumpListCategory*
wxTaskBarJumpListImpl::RemoveCustomCategory(const std::string& title)
{
    for ( wxTaskBarJumpListCategories::iterator it = m_customCategories.begin();
          it != m_customCategories.end();
          ++it )
    {
        wxTaskBarJumpListCategory* tbJlCat = *it;
        if ( tbJlCat->GetTitle() == title )
        {
            m_customCategories.erase(it);
            return tbJlCat;
        }
    }

    return nullptr;
}

void wxTaskBarJumpListImpl::DeleteCustomCategory(const std::string& title)
{
    wxTaskBarJumpListCategory* category = RemoveCustomCategory(title);
    if ( category )
        delete category;
}

bool wxTaskBarJumpListImpl::BeginUpdate()
{
    if ( m_destinationList == nullptr )
        return false;

    unsigned int max_count = 0;
    m_objectArray = nullptr;
    HRESULT hr = m_destinationList->BeginList(&max_count,
        wxIID_IObjectArray, reinterpret_cast<void**>(&m_objectArray));
    if ( !m_appID.empty() )
        m_destinationList->SetAppID(boost::nowide::widen(m_appID).c_str());

    return SUCCEEDED(hr);
}

bool wxTaskBarJumpListImpl::CommitUpdate()
{
    return SUCCEEDED(m_destinationList->CommitList());
}

void wxTaskBarJumpListImpl::AddTasksToDestinationList()
{
    if ( !m_tasks.get() )
        return;

    wxCOMPtr<IObjectCollection> collection(CreateObjectCollection());
    if ( !collection )
        return;

    const wxTaskBarJumpListItems& tasks = m_tasks->GetItems();
    for ( wxTaskBarJumpListItems::const_iterator it = tasks.begin();
          it != tasks.end();
          ++it )
    {
        wxASSERT_MSG( ((*it)->GetType() == wxTaskBarJumpListItemType::Task ||
                       (*it)->GetType() == wxTaskBarJumpListItemType::Separator),
                      "Invalid task Item." );
        AddShellLink(collection, *(*it));
    }
    m_destinationList->AddUserTasks(collection);
}

void wxTaskBarJumpListImpl::AddCustomCategoriesToDestinationList()
{
    for ( wxTaskBarJumpListCategories::iterator it = m_customCategories.begin();
          it != m_customCategories.end();
          ++it )
    {
        wxCOMPtr<IObjectCollection> collection(CreateObjectCollection());
        if ( !collection )
            continue;

        const wxTaskBarJumpListItems& tasks = (*it)->GetItems();
        for ( wxTaskBarJumpListItems::const_iterator iter = tasks.begin();
              iter != tasks.end();
              ++iter )
        {
            wxASSERT_MSG(
                (*iter)->GetType() == wxTaskBarJumpListItemType::Destination,
                "Invalid category item." );
            AddShellLink(collection, *(*iter));
        }
        m_destinationList->AppendCategory(boost::nowide::widen((*it)->GetTitle()).c_str(),
                                          collection);
    }
}

void wxTaskBarJumpListImpl::LoadKnownCategory(const std::string& title)
{
    wxCOMPtr<IApplicationDocumentLists> docList;
    HRESULT hr = CoCreateInstance
                 (
                    wxCLSID_ApplicationDocumentLists,
                    nullptr,
                    CLSCTX_INPROC_SERVER,
                    wxIID_IApplicationDocumentLists,
                    reinterpret_cast<void **>(&docList)
                 );
    if ( FAILED(hr) )
    {
        wxLogApiError("CoCreateInstance(wxCLSID_ApplicationDocumentLists)", hr);
        return;
    }
    if ( !m_appID.empty() )
        docList->SetAppID(boost::nowide::widen(m_appID).c_str());

    wxCOMPtr<IObjectArray> array;
    wxASSERT_MSG( title == "Recent" || title == "Frequent", "Invalid title." );
    hr = docList->GetList
                 (
                     title == "Recent" ? ADLT_RECENT : ADLT_FREQUENT,
                     0,
                     wxIID_IObjectArray,
                     reinterpret_cast<void **>(&array)
                 );
    if ( FAILED(hr) )
    {
        wxLogApiError("IApplicationDocumentLists::GetList", hr);
        return;
    }

    WXUINT count = 0;
    array->GetCount(&count);
    for (WXUINT i = 0; i < count; ++i)
    {
        wxCOMPtr<IUnknown> collectionItem;
        hr = array->GetAt(i, wxIID_IUnknown,
                          reinterpret_cast<void **>(&collectionItem));
        if ( FAILED(hr) )
        {
            wxLogApiError("IObjectArray::GetAt", hr);
            continue;
        }

        wxCOMPtr<IShellLink> shellLink;
        wxCOMPtr<IShellItem> shellItem;
        wxTaskBarJumpListItem* item = nullptr;

        if ( SUCCEEDED(collectionItem->QueryInterface(
                 wxIID_IShellLink, reinterpret_cast<void**>(&shellLink))) )
        {
            item = GetItemFromIShellLink(shellLink);
        }
        else if ( SUCCEEDED(collectionItem->QueryInterface(
                      wxIID_IShellItem, reinterpret_cast<void**>(&shellItem))) )
        {
            item = GetItemFromIShellItem(shellItem);
        }
        else
        {
            wxLogError("Can not query interfaces: IShellLink or IShellItem.");
        }

        if ( item )
        {
            if ( title == "Frequent" )
                m_frequent->Append(item);
            else
                m_recent->Append(item);
        }
    }
}

#endif // wxUSE_TASKBARBUTTON
