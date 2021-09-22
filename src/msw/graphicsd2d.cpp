/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/graphicsd2d.cpp
// Purpose:     Implementation of Direct2D Render Context
// Author:      Pana Alexandru <astronothing@gmail.com>
// Created:     2014-05-20
// Copyright:   (c) 2014 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#if wxUSE_GRAPHICS_DIRECT2D

// Minimum supported client: Windows 8 and Platform Update for Windows 7
#define wxD2D_DEVICE_CONTEXT_SUPPORTED 0

// We load these functions at runtime from the d2d1.dll.
// However, since they are also used inside the d2d1.h header we must provide
// implementations matching the exact declarations. These defines ensures we
// are not violating the ODR rule.
#define D2D1CreateFactory wxD2D1CreateFactory
#define D2D1MakeRotateMatrix wxD2D1MakeRotateMatrix
#define D2D1MakeSkewMatrix wxD2D1MakeSkewMatrix
#define D2D1IsMatrixInvertible wxD2D1IsMatrixInvertible
#define D2D1InvertMatrix wxD2D1InvertMatrix
#if wxD2D_DEVICE_CONTEXT_SUPPORTED
#define D3D11CreateDevice wxD3D11CreateDevice
#endif

// There are clashes between the names of the member fields and parameters
// in the standard d2d1helper.h header resulting in C4458 with VC14,
// so disable this warning for this file as there is no other way to
// avoid it.

#include "wx/msw/private/graphicsd2d.h"

#ifdef __MINGW64_TOOLCHAIN__
#ifndef DWRITE_E_NOFONT
#define DWRITE_E_NOFONT _HRESULT_TYPEDEF_(0x88985002L)
#endif
#endif

#if wxD2D_DEVICE_CONTEXT_SUPPORTED
#include <d3d11.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#endif

#ifndef WX_PRECOMP
    #include "wx/msw/private.h"
    #include "wx/msw/wrap/utils.h"
    
    #include <boost/nowide/convert.hpp>
    #include <fmt/core.h>
    #include <gsl/gsl>

    #include <algorithm>
    #include <limits>
    #include <memory>
#endif // !WX_PRECOMP

#include "wx/dc.h"
#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/image.h"
#include "wx/module.h"
#include "wx/window.h"

#include "wx/graphics.h"
#include "wx/dynlib.h"
#include "wx/msw/ole/comimpl.h"
#include "wx/msw/private/comptr.h"
#include "wx/private/graphics.h"

#include <stack>

// This must be the last header included to only affect the DEFINE_GUID()
// occurrences below but not any GUIDs declared in the standard files included
// above.
#include <initguid.h>

// Generic error message for a failed direct2d operation
#define wxFAILED_HRESULT_MSG(result) \
    fmt::format("Direct2D failed with HRESULT {:x}", (result))

// Checks a HRESULT value for success, otherwise displays an error message and
// returns from the enclosing function.
#define wxCHECK_HRESULT_RET(result) \
    wxCHECK_RET(SUCCEEDED(result), wxFAILED_HRESULT_MSG(result))

#define wxCHECK2_HRESULT_RET(result, returnValue)                             \
    wxCHECK2_MSG(SUCCEEDED(result), return returnValue,                       \
        wxFAILED_HRESULT_MSG(result))

// Variation of wxCHECK_HRESULT_RET for functions which must return a pointer
#define wxCHECK_HRESULT_RET_PTR(result) wxCHECK2_HRESULT_RET(result, NULL)

// Checks the precondition of wxManagedResourceHolder::AcquireResource, namely
// that it is bound to a manager.
#define wxCHECK_RESOURCE_HOLDER_PRE()                                         \
    {                                                                         \
    if (IsResourceAcquired()) return;                                         \
    wxCHECK_RET(IsBound(),                                                    \
        "Cannot acquire a native resource without being bound to a manager"); \
    }

// Checks the postcondition of wxManagedResourceHolder::AcquireResource, namely
// that it was successful in acquiring the native resource.
#define wxCHECK_RESOURCE_HOLDER_POST() \
    wxCHECK_RET(m_nativeResource != NULL, "Could not acquire native resource");


// Helper class used to check for direct2d availability at runtime and to
// dynamically load the required symbols from d2d1.dll and dwrite.dll
class wxDirect2D
{
public:

    enum wxD2DVersion
    {
        wxD2D_VERSION_1_0,
        wxD2D_VERSION_1_1,
        wxD2D_VERSION_1_2,
        wxD2D_VERSION_1_3,
        wxD2D_VERSION_NONE
    };

    static bool Initialize()
    {
        if (!m_initialized)
        {
            m_hasDirect2DSupport = LoadLibraries();
            m_initialized = true;
        }

        return m_hasDirect2DSupport;
    }

    static bool HasDirect2DSupport()
    {
        Initialize();

        return m_hasDirect2DSupport;
    }

    static wxD2DVersion GetDirect2DVersion()
    {
        return m_D2DRuntimeVersion;
    }

private:
    static bool LoadLibraries()
    {
        if ( !m_dllDirect2d.Load(wxT("d2d1.dll"), wxDL_VERBATIM | wxDL_QUIET) )
            return false;

        if ( !m_dllDirectWrite.Load(wxT("dwrite.dll"), wxDL_VERBATIM | wxDL_QUIET) )
            return false;

#if wxD2D_DEVICE_CONTEXT_SUPPORTED
        if (!m_dllDirect3d.Load(wxT("d3d11.dll"), wxDL_VERBATIM | wxDL_QUIET))
            return false;
#endif

        #define wxLOAD_FUNC(dll, name)                    \
        name = (name##_t)dll.RawGetSymbol(#name);         \
            if ( !name )                                  \
            return false;

        wxLOAD_FUNC(m_dllDirect2d, D2D1CreateFactory);
        wxLOAD_FUNC(m_dllDirect2d, D2D1MakeRotateMatrix);
        wxLOAD_FUNC(m_dllDirect2d, D2D1MakeSkewMatrix);
        wxLOAD_FUNC(m_dllDirect2d, D2D1IsMatrixInvertible);
        wxLOAD_FUNC(m_dllDirect2d, D2D1InvertMatrix);
        wxLOAD_FUNC(m_dllDirectWrite, DWriteCreateFactory);

#if wxD2D_DEVICE_CONTEXT_SUPPORTED
        wxLOAD_FUNC(m_dllDirect3d, D3D11CreateDevice);
        m_D2DRuntimeVersion = wxD2D_VERSION_1_1;
#else
        m_D2DRuntimeVersion = wxD2D_VERSION_1_0;
#endif

        return true;
    }

public:
    using D2D1CreateFactory_t = HRESULT (WINAPI*)(D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS*, void**);
    static D2D1CreateFactory_t D2D1CreateFactory;

    using D2D1MakeRotateMatrix_t = void (WINAPI*)(FLOAT, D2D1_POINT_2F, D2D1_MATRIX_3X2_F*);
    static D2D1MakeRotateMatrix_t D2D1MakeRotateMatrix;

    using D2D1MakeSkewMatrix_t = void (WINAPI*)(FLOAT, FLOAT, D2D1_POINT_2F, D2D1_MATRIX_3X2_F*);
    static D2D1MakeSkewMatrix_t D2D1MakeSkewMatrix;

    using D2D1IsMatrixInvertible_t = BOOL (WINAPI*)(const D2D1_MATRIX_3X2_F*);
    static D2D1IsMatrixInvertible_t D2D1IsMatrixInvertible;

    using D2D1InvertMatrix_t = BOOL (WINAPI*)(D2D1_MATRIX_3X2_F*);
    static D2D1InvertMatrix_t D2D1InvertMatrix;

    using DWriteCreateFactory_t = HRESULT (WINAPI*)(DWRITE_FACTORY_TYPE, REFIID, IUnknown**);
    static DWriteCreateFactory_t DWriteCreateFactory;

#if wxD2D_DEVICE_CONTEXT_SUPPORTED
    using D3D11CreateDevice_t = HRESULT (WINAPI*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, CONST D3D_FEATURE_LEVEL*, UINT, UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
    static D3D11CreateDevice_t D3D11CreateDevice;
#endif

private:
    inline static bool m_initialized{false};
    inline static bool m_hasDirect2DSupport{false};
    inline static wxD2DVersion m_D2DRuntimeVersion{wxD2D_VERSION_NONE};

    inline static wxDynamicLibrary m_dllDirect2d{};
    inline static wxDynamicLibrary m_dllDirectWrite;
#if wxD2D_DEVICE_CONTEXT_SUPPORTED
    inline static wxDynamicLibrary m_dllDirect3d{};
#endif
};

// define the (not yet imported) functions
wxDirect2D::D2D1CreateFactory_t wxDirect2D::D2D1CreateFactory = nullptr;
wxDirect2D::D2D1MakeRotateMatrix_t wxDirect2D::D2D1MakeRotateMatrix = nullptr;
wxDirect2D::D2D1MakeSkewMatrix_t wxDirect2D::D2D1MakeSkewMatrix = nullptr;
wxDirect2D::D2D1IsMatrixInvertible_t wxDirect2D::D2D1IsMatrixInvertible = nullptr;
wxDirect2D::D2D1InvertMatrix_t wxDirect2D::D2D1InvertMatrix = nullptr;
wxDirect2D::DWriteCreateFactory_t wxDirect2D::DWriteCreateFactory = nullptr;

#if wxD2D_DEVICE_CONTEXT_SUPPORTED
wxDirect2D::D3D11CreateDevice_t wxDirect2D::D3D11CreateDevice = NULL;
#endif

// define the interface GUIDs
DEFINE_GUID(wxIID_IWICImagingFactory,
            0xec5ec8a9, 0xc395, 0x4314, 0x9c, 0x77, 0x54, 0xd7, 0xa9, 0x35, 0xff, 0x70);

DEFINE_GUID(wxIID_ID2D1Factory,
            0x06152247, 0x6f50, 0x465a, 0x92, 0x45, 0x11, 0x8b, 0xfd, 0x3b, 0x60, 0x07);

DEFINE_GUID(wxIID_IDWriteFactory,
            0xb859ee5a, 0xd838, 0x4b5b, 0xa2, 0xe8, 0x1a, 0xdc, 0x7d, 0x93, 0xdb, 0x48);

DEFINE_GUID(wxIID_IWICBitmapSource,
            0x00000120, 0xa8f2, 0x4877, 0xba, 0x0a, 0xfd, 0x2b, 0x66, 0x45, 0xfb, 0x94);

DEFINE_GUID(GUID_WICPixelFormat32bppPBGRA,
            0x6fddc324, 0x4e03, 0x4bfe, 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x10);

DEFINE_GUID(GUID_WICPixelFormat32bppBGR,
            0x6fddc324, 0x4e03, 0x4bfe, 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0e);

#if wxD2D_DEVICE_CONTEXT_SUPPORTED
DEFINE_GUID(IID_IDXGIDevice,
            0x54ec77fa, 0x1377, 0x44e6, 0x8c, 0x32, 0x88, 0xfd, 0x5f, 0x44, 0xc8, 0x4c);
#endif

#ifndef CLSID_WICImagingFactory
DEFINE_GUID(CLSID_WICImagingFactory,
            0xcacaf262, 0x9370, 0x4615, 0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0xa);
#endif

#if wxUSE_PRIVATE_FONTS
DEFINE_GUID(wxIID_IDWriteFontFileEnumerator,
            0x72755049, 0x5ff7, 0x435d, 0x83, 0x48, 0x4b, 0xe9, 0x7c, 0xfa, 0x6c, 0x7c);

DEFINE_GUID(wxIID_IDWriteFontCollectionLoader,
            0xcca920e4, 0x52f0, 0x492b, 0xbf, 0xa8, 0x29, 0xc7, 0x2e, 0xe0, 0xa4, 0x68);
#endif // wxUSE_PRIVATE_FONTS

// Implementation of the Direct2D functions
HRESULT WINAPI wxD2D1CreateFactory(
    D2D1_FACTORY_TYPE factoryType,
    REFIID riid,
    CONST D2D1_FACTORY_OPTIONS *pFactoryOptions,
    void **ppIFactory)
{
    if (!wxDirect2D::Initialize())
        return S_FALSE;

    return wxDirect2D::D2D1CreateFactory(
        factoryType,
        riid,
        pFactoryOptions,
        ppIFactory);
}

void WINAPI wxD2D1MakeRotateMatrix(
    FLOAT angle,
    D2D1_POINT_2F center,
    D2D1_MATRIX_3X2_F *matrix)
{
    if (!wxDirect2D::Initialize())
        return;

    wxDirect2D::D2D1MakeRotateMatrix(angle, center, matrix);
}

void WINAPI wxD2D1MakeSkewMatrix(
    FLOAT angleX,
    FLOAT angleY,
    D2D1_POINT_2F center,
    D2D1_MATRIX_3X2_F *matrix)
{
    if (!wxDirect2D::Initialize())
        return;

    wxDirect2D::D2D1MakeSkewMatrix(angleX, angleY, center, matrix);
}

BOOL WINAPI wxD2D1IsMatrixInvertible(
    const D2D1_MATRIX_3X2_F *matrix)
{
    if (!wxDirect2D::Initialize())
        return FALSE;

    return wxDirect2D::D2D1IsMatrixInvertible(matrix);
}

BOOL WINAPI wxD2D1InvertMatrix(
    D2D1_MATRIX_3X2_F *matrix)
{
    if (!wxDirect2D::Initialize())
        return FALSE;

    return wxDirect2D::D2D1InvertMatrix(matrix);
}

#if wxD2D_DEVICE_CONTEXT_SUPPORTED
HRESULT WINAPI wxD3D11CreateDevice(
    IDXGIAdapter* pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    CONST D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    ID3D11Device** ppDevice,
    D3D_FEATURE_LEVEL* pFeatureLevel,
    ID3D11DeviceContext** ppImmediateContext)
{
    if (!wxDirect2D::Initialize())
        return S_FALSE;

    return wxDirect2D::D3D11CreateDevice(
        pAdapter,
        DriverType,
        Software,
        Flags,
        pFeatureLevels,
        FeatureLevels,
        SDKVersion,
        ppDevice,
        pFeatureLevel,
        ppImmediateContext);
}
#endif

#if wxUSE_PRIVATE_FONTS

// This function is defined in src/msw/font.cpp.
extern const std::vector<std::string>& wxGetPrivateFontFileNames();

namespace
{
wxCOMPtr<IDWriteFontCollection> gs_pPrivateFontCollection;

using wxDirect2DFontKey = unsigned int;

class wxDirect2DFontFileEnumerator : public IDWriteFontFileEnumerator
{
public:
    wxDirect2DFontFileEnumerator(IDWriteFactory* pFactory, const std::vector<std::string>& fontCollection)
        : m_factory(pFactory)
        , m_filePaths(fontCollection)
        , m_nextIndex(0)
    {
    }

    virtual ~wxDirect2DFontFileEnumerator() = default;

    wxDirect2DFontFileEnumerator(const wxDirect2DFontFileEnumerator&) = delete;
	wxDirect2DFontFileEnumerator& operator=(const wxDirect2DFontFileEnumerator&) = delete;

    // IDWriteFontFileEnumerator methods
    wxSTDMETHODIMP MoveNext(BOOL* pHasCurrentFile) override
    {
        HRESULT hr = S_OK;

        *pHasCurrentFile = FALSE;
        m_currentFile.reset();

        if ( m_nextIndex < m_filePaths.size() )
        {
            hr = m_factory->CreateFontFileReference(boost::nowide::widen(m_filePaths[m_nextIndex]).c_str(), nullptr, &m_currentFile);
            if ( SUCCEEDED(hr) )
            {
                *pHasCurrentFile = TRUE;
                ++m_nextIndex;
            }
        }

        return hr;
    }

    wxSTDMETHODIMP GetCurrentFontFile(IDWriteFontFile** ppFontFile) override
    {
        if ( m_currentFile )
        {
            m_currentFile.get()->AddRef();
        }
        *ppFontFile = m_currentFile;

        return m_currentFile ? S_OK : E_FAIL;
    }

    // IUnknown methods
    DECLARE_IUNKNOWN_METHODS;

private:
    wxCOMPtr<IDWriteFactory> m_factory;
    wxCOMPtr<IDWriteFontFile> m_currentFile;
    std::vector<std::string> m_filePaths;
    size_t m_nextIndex;
};

BEGIN_IID_TABLE(wxDirect2DFontFileEnumerator)
ADD_IID(Unknown)
ADD_RAW_IID(wxIID_IDWriteFontFileEnumerator)
END_IID_TABLE;

IMPLEMENT_IUNKNOWN_METHODS(wxDirect2DFontFileEnumerator)

class wxDirect2DFontCollectionLoader : public IDWriteFontCollectionLoader
{
public:
    wxDirect2DFontCollectionLoader() = default;
    virtual ~wxDirect2DFontCollectionLoader() = default;

    wxDirect2DFontCollectionLoader(const wxDirect2DFontCollectionLoader&) = delete;
	wxDirect2DFontCollectionLoader& operator=(const wxDirect2DFontCollectionLoader&) = delete;

    // IDWriteFontCollectionLoader methods
    wxSTDMETHODIMP CreateEnumeratorFromKey(IDWriteFactory* pFactory,
                                        void const* pCollectionKey, UINT32 collectionKeySize,
                                        IDWriteFontFileEnumerator** pFontFileEnumerator) override
    {
        if ( !pFontFileEnumerator )
            return E_INVALIDARG;

        *pFontFileEnumerator = nullptr;

        if ( collectionKeySize != sizeof(wxDirect2DFontKey) )
            return E_INVALIDARG;

        wxDirect2DFontKey key = *static_cast<wxDirect2DFontKey const*>(pCollectionKey);
        if ( key != ms_key )
            return E_INVALIDARG;

        if ( ms_fontList.empty() )
            return E_INVALIDARG;

        wxDirect2DFontFileEnumerator* pEnumerator = new wxDirect2DFontFileEnumerator(pFactory, ms_fontList);
        if ( !pEnumerator )
            return E_OUTOFMEMORY;

        pEnumerator->AddRef();
        *pFontFileEnumerator = pEnumerator;

        return S_OK;
    }

    // Singleton loader instance
    static IDWriteFontCollectionLoader* GetLoader()
    {
        static wxCOMPtr<wxDirect2DFontCollectionLoader> instance(new wxDirect2DFontCollectionLoader());

        return instance;
    }

    static bool IsInitialized()
    {
        return ms_isInitialized;
    }

    static wxDirect2DFontKey SetFontList(const std::vector<std::string>& list)
    {
        ms_fontList = list;
        // Every time font collection is changed, generate unique key
        return ++ms_key;
    }

    static const std::vector<std::string>& GetFontList()
    {
        return ms_fontList;
    }

    // IUnknown methods
    DECLARE_IUNKNOWN_METHODS;

private:
    inline static std::vector<std::string> ms_fontList;
    inline static wxDirect2DFontKey ms_key{0};
    inline static bool ms_isInitialized{true};
};

BEGIN_IID_TABLE(wxDirect2DFontCollectionLoader)
ADD_IID(Unknown)
ADD_RAW_IID(wxIID_IDWriteFontCollectionLoader)
END_IID_TABLE;

IMPLEMENT_IUNKNOWN_METHODS(wxDirect2DFontCollectionLoader)

} // anonymous namespace

#endif // wxUSE_PRIVATE_FONTS

static IWICImagingFactory* gs_WICImagingFactory = nullptr;

IWICImagingFactory* wxWICImagingFactory()
{
    if (gs_WICImagingFactory == nullptr) {
        HRESULT hr = ::CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            wxIID_IWICImagingFactory,
            (LPVOID*)&gs_WICImagingFactory);
        wxCHECK_HRESULT_RET_PTR(hr);
    }
    return gs_WICImagingFactory;
}

static ID2D1Factory* gs_ID2D1Factory = nullptr;

ID2D1Factory* wxD2D1Factory()
{
    if (!wxDirect2D::Initialize())
        return nullptr;

    if (gs_ID2D1Factory == nullptr)
    {
        D2D1_FACTORY_OPTIONS factoryOptions = {D2D1_DEBUG_LEVEL_NONE};

        // According to
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ee794287(v=vs.85).aspx
        // the Direct2D Debug Layer is only available starting with Windows 8
        // and Visual Studio 2012.
#if defined(__WXDEBUG__) && defined(__VISUALC__) && wxCHECK_VISUALC_VERSION(11)
        if ( wxGetWinVersion() >= wxWinVersion_8 )
        {
            factoryOptions.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
        }
#endif  //__WXDEBUG__

        HRESULT hr = wxDirect2D::D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            wxIID_ID2D1Factory,
            &factoryOptions,
            reinterpret_cast<void**>(&gs_ID2D1Factory)
            );
        wxCHECK_HRESULT_RET_PTR(hr);
    }
    return gs_ID2D1Factory;
}

static IDWriteFactory* gs_IDWriteFactory = nullptr;

IDWriteFactory* wxDWriteFactory()
{
    if (!wxDirect2D::Initialize())
        return nullptr;

    if (gs_IDWriteFactory == nullptr)
    {
        wxDirect2D::DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            wxIID_IDWriteFactory,
            reinterpret_cast<IUnknown**>(&gs_IDWriteFactory)
            );
#if wxUSE_PRIVATE_FONTS
        // Register our custom font loader
        HRESULT hr = gs_IDWriteFactory->RegisterFontCollectionLoader(wxDirect2DFontCollectionLoader::GetLoader());
        if ( FAILED(hr) )
        {
            wxLogError(_("Could not register custom DirectWrite font loader."));
        }
#endif // wxUSE_PRIVATE_FONTS
    }
    return gs_IDWriteFactory;
}

extern WXDLLIMPEXP_DATA_CORE(wxGraphicsPen) wxNullGraphicsPen;
extern WXDLLIMPEXP_DATA_CORE(wxGraphicsBrush) wxNullGraphicsBrush;

// We use the notion of a context supplier because the context
// needed to create Direct2D resources (namely the RenderTarget)
// is itself device-dependent and might change during the lifetime
// of the resources which were created from it.
template <typename C>
struct wxContextSupplier
{
    using ContextType = C;

    virtual C GetContext() = 0;
};

using wxD2DContextSupplier = wxContextSupplier<ID2D1RenderTarget*>;

// A resource holder manages a generic resource by acquiring
// and releasing it on demand.
struct wxResourceHolder
{
    // Acquires the managed resource if necessary (not already acquired)
    virtual void AcquireResource() = 0;

    // Releases the managed resource
    virtual void ReleaseResource() = 0;

    // Checks if the resources was previously acquired
    virtual bool IsResourceAcquired() = 0;

    // Returns the managed resource or NULL if the resources
    // was not previously acquired
    virtual void* GetResource() = 0;

    virtual ~wxResourceHolder() = default;
};

class wxD2DResourceManager;

struct wxD2DManagedObject
{
    virtual void Bind(wxD2DResourceManager* manager) = 0;
    virtual void UnBind() = 0;
    virtual bool IsBound() = 0;
    virtual wxD2DResourceManager* GetManager() = 0;

    virtual ~wxD2DManagedObject() = default;
};

struct wxManagedResourceHolder : public wxResourceHolder, public wxD2DManagedObject
{
};

// A Direct2D resource manager handles the device-dependent
// resource holders attached to it by requesting them to
// release their resources when the API invalidates.
// NOTE: We're using a list because we expect to have multiple
// insertions but very rarely a traversal (if ever).
WX_DECLARE_LIST(wxManagedResourceHolder, wxManagedResourceListType);
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(wxManagedResourceListType);

class wxD2DResourceManager: public wxD2DContextSupplier
{
public:
    void RegisterResourceHolder(wxManagedResourceHolder* resourceHolder)
    {
        m_resources.push_back(resourceHolder);
    }

    void UnregisterResourceHolder(wxManagedResourceHolder* resourceHolder)
    {
        m_resources.remove(resourceHolder);
    }

    void ReleaseResources()
    {
        for (auto it = m_resources.begin(); it != m_resources.end(); ++it)
        {
            (*it)->ReleaseResource();
        }

        // Check that all resources were released
        for (auto it = m_resources.begin(); it != m_resources.end(); ++it)
        {
            wxCHECK_RET(!(*it)->IsResourceAcquired(), "One or more device-dependent resources failed to release");
        }
    }

    virtual ~wxD2DResourceManager()
    {
        while (!m_resources.empty())
        {
            m_resources.front()->ReleaseResource();
            m_resources.front()->UnBind();
        }
    }

private:
    wxManagedResourceListType m_resources;
};

// A Direct2D resource holder manages device dependent resources
// by storing any information necessary for acquiring the resource
// and releasing the resource when the API invalidates it.
template<typename T>
class wxD2DResourceHolder: public wxManagedResourceHolder
{
public:
    wxD2DResourceHolder() = default;

    ~wxD2DResourceHolder() override
    {
        UnBind();
        ReleaseResource();
    }

    bool IsResourceAcquired() override
    {
        return m_nativeResource != nullptr;
    }

    void* GetResource() override
    {
        return GetD2DResource();
    }

    wxCOMPtr<T>& GetD2DResource()
    {
        if (!IsResourceAcquired())
        {
            AcquireResource();
        }

        return m_nativeResource;
    }

    void AcquireResource() override
    {
        wxCHECK_RESOURCE_HOLDER_PRE();

        DoAcquireResource();

        wxCHECK_RESOURCE_HOLDER_POST();
    }

    void ReleaseResource() override
    {
        m_nativeResource.reset();
    }

    wxD2DContextSupplier::ContextType GetContext()
    {
        return m_resourceManager->GetContext();
    }

    void Bind(wxD2DResourceManager* manager) override
    {
        if (IsBound())
            return;

        m_resourceManager = manager;
        m_resourceManager->RegisterResourceHolder(this);
    }

    void UnBind() override
    {
        if (!IsBound())
            return;

        m_resourceManager->UnregisterResourceHolder(this);
        m_resourceManager = nullptr;
    }

    bool IsBound() override
    {
        return m_resourceManager != nullptr;
    }

    wxD2DResourceManager* GetManager() override
    {
        return m_resourceManager;
    }

protected:
    virtual void DoAcquireResource() = 0;

private:
    wxD2DResourceManager* m_resourceManager{nullptr};

protected:
    wxCOMPtr<T> m_nativeResource;
};

// Used as super class for graphics data objects
// to forward the bindings to their internal resource holder.
struct wxD2DManagedGraphicsData : public wxD2DManagedObject
{
    void Bind(wxD2DResourceManager* manager) override
    {
        GetManagedObject()->Bind(manager);
    }

    void UnBind() override
    {
        GetManagedObject()->UnBind();
    }

    bool IsBound() override
    {
        return GetManagedObject()->IsBound();
    }

    wxD2DResourceManager* GetManager() override
    {
        return GetManagedObject()->GetManager();
    }

    virtual wxD2DManagedObject* GetManagedObject() = 0;

    ~wxD2DManagedGraphicsData() override = default;
};

D2D1_CAP_STYLE wxD2DConvertPenCap(wxPenCap cap)
{
    switch (cap)
    {
    case wxCAP_ROUND:
        return D2D1_CAP_STYLE_ROUND;
    case wxCAP_PROJECTING:
        return D2D1_CAP_STYLE_SQUARE;
    case wxCAP_BUTT:
        return D2D1_CAP_STYLE_FLAT;
    case wxCAP_INVALID:
        return D2D1_CAP_STYLE_FLAT;
    }

    wxFAIL_MSG("unknown pen cap");
    return D2D1_CAP_STYLE_FLAT;
}

D2D1_LINE_JOIN wxD2DConvertPenJoin(wxPenJoin join)
{
    switch (join)
    {
    case wxJOIN_BEVEL:
        return D2D1_LINE_JOIN_BEVEL;
    case wxJOIN_MITER:
        return D2D1_LINE_JOIN_MITER;
    case wxJOIN_ROUND:
        return D2D1_LINE_JOIN_ROUND;
    case wxJOIN_INVALID:
        return D2D1_LINE_JOIN_MITER;
    }

    wxFAIL_MSG("unknown pen join");
    return D2D1_LINE_JOIN_MITER;
}

D2D1_DASH_STYLE wxD2DConvertPenStyle(wxPenStyle dashStyle)
{
    switch (dashStyle)
    {
    case wxPenStyle::Solid:
        return D2D1_DASH_STYLE_SOLID;
    case wxPenStyle::Dot:
        return D2D1_DASH_STYLE_DOT;
    case wxPenStyle::LongDash:
        return D2D1_DASH_STYLE_DASH;
    case wxPenStyle::ShortDash:
        return D2D1_DASH_STYLE_DASH;
    case wxPenStyle::DotDash:
        return D2D1_DASH_STYLE_DASH_DOT;
    case wxPenStyle::UserDash:
        return D2D1_DASH_STYLE_CUSTOM;

    // NB: These styles cannot be converted to a D2D1_DASH_STYLE
    // and must be handled separately.
    case wxPenStyle::Transparent:
        [[fallthrough]];
    case wxPenStyle::Invalid:
        [[fallthrough]];
    case wxPenStyle::StippleMaskOpaque:
        [[fallthrough]];
    case wxPenStyle::StippleMask:
        [[fallthrough]];
    case wxPenStyle::Stipple:
        [[fallthrough]];
    case wxPenStyle::BDiagonalHatch:
        [[fallthrough]];
    case wxPenStyle::CrossDiagHatch:
        [[fallthrough]];
    case wxPenStyle::FDiagonalHatch:
        [[fallthrough]];
    case wxPenStyle::CrossHatch:
        [[fallthrough]];
    case wxPenStyle::HorizontalHatch:
        [[fallthrough]];
    case wxPenStyle::VerticalHatch:
        return D2D1_DASH_STYLE_SOLID;
    }

    wxFAIL_MSG("unknown pen style");
    return D2D1_DASH_STYLE_SOLID;
}

D2D1_COLOR_F wxD2DConvertColour(wxColour colour)
{
    return D2D1::ColorF(
        colour.Red() / 255.0f,
        colour.Green() / 255.0f,
        colour.Blue() / 255.0f,
        colour.Alpha() / 255.0f);
}

#if wxD2D_DEVICE_CONTEXT_SUPPORTED
bool wxD2DCompositionModeSupported(wxCompositionMode compositionMode)
{
    return (compositionMode != wxCOMPOSITION_CLEAR) || (compositionMode != wxCOMPOSITION_INVALID);
}

D2D1_COMPOSITE_MODE wxD2DConvertCompositionMode(wxCompositionMode compositionMode)
{
    switch (compositionMode)
    {
    case wxCOMPOSITION_SOURCE:
        return D2D1_COMPOSITE_MODE_SOURCE_COPY;
    case wxCOMPOSITION_OVER:
        return D2D1_COMPOSITE_MODE_SOURCE_OVER;
    case wxCOMPOSITION_IN:
        return D2D1_COMPOSITE_MODE_SOURCE_IN;
    case wxCOMPOSITION_OUT:
        return D2D1_COMPOSITE_MODE_SOURCE_OUT;
    case wxCOMPOSITION_ATOP:
        return D2D1_COMPOSITE_MODE_SOURCE_ATOP;
    case wxCOMPOSITION_DEST_OVER:
        return D2D1_COMPOSITE_MODE_DESTINATION_OVER;
    case wxCOMPOSITION_DEST_IN:
        return D2D1_COMPOSITE_MODE_DESTINATION_IN;
    case wxCOMPOSITION_DEST_OUT:
        return D2D1_COMPOSITE_MODE_DESTINATION_OUT;
    case wxCOMPOSITION_DEST_ATOP:
        return D2D1_COMPOSITE_MODE_DESTINATION_ATOP;
    case wxCOMPOSITION_XOR:
        return D2D1_COMPOSITE_MODE_XOR;
    case wxCOMPOSITION_ADD:
        return D2D1_COMPOSITE_MODE_PLUS;

    // unsupported composition modes
    case wxCOMPOSITION_DEST:
        [[fallthrough]];
    case wxCOMPOSITION_CLEAR:
        [[fallthrough]];
    case wxCOMPOSITION_INVALID:
        [[fallthrough]];
    default: // FIXME: Unknown should never happen. Fix with enum class.
        wxFAIL_MSG("unknown / unsupported composition mode");
        return D2D1_COMPOSITE_MODE_SOURCE_COPY;
    }
}

#endif // wxD2D_DEVICE_CONTEXT_SUPPORTED

// Direct2D 1.1 introduces a new enum for specifying the interpolation quality
// which is only used with the ID2D1DeviceContext::DrawImage method.
#if wxD2D_DEVICE_CONTEXT_SUPPORTED
D2D1_INTERPOLATION_MODE wxD2DConvertInterpolationMode(wxInterpolationQuality interpolationQuality)
{
    switch (interpolationQuality)
    {
    case wxInterpolationQuality::Default:
        [[fallthrough]];
    case wxInterpolationQuality::None:
        [[fallthrough]];
    case wxInterpolationQuality::Fast:
        return D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
    case wxInterpolationQuality::Good:
        return D2D1_INTERPOLATION_MODE_LINEAR;
    case wxInterpolationQuality::Best:
        return D2D1_INTERPOLATION_MODE_CUBIC;
    }

    wxFAIL_MSG("unknown interpolation quality");
    return D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
}
#endif // wxD2D_DEVICE_CONTEXT_SUPPORTED

D2D1_BITMAP_INTERPOLATION_MODE wxD2DConvertBitmapInterpolationMode(wxInterpolationQuality interpolationQuality)
{
    switch (interpolationQuality)
    {
    case wxInterpolationQuality::Default:
        [[fallthrough]];
    case wxInterpolationQuality::None:
        [[fallthrough]];
    case wxInterpolationQuality::Fast:
        [[fallthrough]];
    case wxInterpolationQuality::Good:
        return D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
    case wxInterpolationQuality::Best:
        return D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
    }

    wxFAIL_MSG("unknown interpolation quality");
    return D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
}

D2D1_RECT_F wxD2DConvertRect(const wxRect& rect)
{
    return D2D1::RectF(gsl::narrow_cast<float>(rect.GetLeft()),
                       gsl::narrow_cast<float>(rect.GetTop()),
                       gsl::narrow_cast<float>(rect.GetRight()),
                       gsl::narrow_cast<float>(rect.GetBottom()));
}

wxCOMPtr<ID2D1Geometry> wxD2DConvertRegionToGeometry(ID2D1Factory* direct2dFactory, const wxRegion& region)
{
    // TODO: Use lambdas.
    // Build the array of geometries
    HRESULT hr;
    std::vector<ID2D1Geometry*> geometries;
    int rectCount;
    if ( region.IsEmpty() )
    {
        // Empty region is skipped by iterator
        // so we have to create it in a special way.
        rectCount = 1;
        geometries.resize(rectCount);

        hr = direct2dFactory->CreateRectangleGeometry(
                        D2D1::RectF(0.0F, 0.0F, 0.0F, 0.0F),
                        (ID2D1RectangleGeometry**)(&geometries[0]));
        wxFAILED_HRESULT_MSG(hr);
    }
    else
    {
        // Count the number of rectangles which compose the region
        wxRegionIterator regionIterator(region);
        std::size_t rectCount{0};

        // FIXME: Isn't it better just to get m_numRect instead of iterating?
        while(regionIterator++)
            rectCount++;

        geometries.resize(rectCount);
        regionIterator.Reset(region);

        int i = 0;
        while(regionIterator)
        {
            wxRect rect = regionIterator.GetRect();
            rect.SetWidth(rect.GetWidth() + 1);
            rect.SetHeight(rect.GetHeight() + 1);

            hr = direct2dFactory->CreateRectangleGeometry(
                wxD2DConvertRect(rect),
                (ID2D1RectangleGeometry**)(&geometries[i]));
            wxFAILED_HRESULT_MSG(hr);

            i++;
            ++regionIterator;
        }
    }

    // Create a geometry group to hold all the rectangles
    wxCOMPtr<ID2D1GeometryGroup> resultGeometry;
    hr = direct2dFactory->CreateGeometryGroup(
        D2D1_FILL_MODE_WINDING,
        &geometries[0],
        geometries.size(),
        &resultGeometry);
    wxFAILED_HRESULT_MSG(hr);

    // Cleanup temporaries
    std::for_each(geometries.begin(), geometries.end(), [](auto&& geometry){ geometry->Release(); });

    return wxCOMPtr<ID2D1Geometry>(resultGeometry);
}

class wxD2DOffsetHelper
{
public:
    explicit wxD2DOffsetHelper(wxGraphicsContext* g)
        : m_context(g)
    {
        if (m_context->ShouldOffset())
        {
            const wxGraphicsMatrix matrix(m_context->GetTransform());
            double x = m_context->GetContentScaleFactor(), y = x;
            matrix.TransformDistance(&x, &y);
            m_offset = 0.5 / std::min(std::fabs(x), std::fabs(y));
            m_context->Translate(m_offset, m_offset);
        }
    }

    ~wxD2DOffsetHelper()
    {
        if (m_offset > 0.0)
        {
            m_context->Translate(-m_offset, -m_offset);
        }
    }

private:
    wxGraphicsContext* m_context;
    double m_offset{0.0};
};

bool operator==(const D2D1::Matrix3x2F& lhs, const D2D1::Matrix3x2F& rhs)
{
    return
        lhs._11 == rhs._11 && lhs._12 == rhs._12 &&
        lhs._21 == rhs._21 && lhs._22 == rhs._22 &&
        lhs._31 == rhs._31 && lhs._32 == rhs._32;
}

//-----------------------------------------------------------------------------
// wxD2DMatrixData declaration
//-----------------------------------------------------------------------------

class wxD2DMatrixData : public wxGraphicsMatrixData
{
public:
    explicit wxD2DMatrixData(wxGraphicsRenderer* renderer);
    wxD2DMatrixData(wxGraphicsRenderer* renderer, const D2D1::Matrix3x2F& matrix);

    wxGraphicsObjectRefData* Clone() const override;

    void Concat(const wxGraphicsMatrixData* t) override;

    void Set(double a = 1.0, double b = 0.0, double c = 0.0, double d = 1.0,
        double tx = 0.0, double ty = 0.0) override;

    void Get(double* a = nullptr, double* b = nullptr,  double* c = nullptr,
        double* d = nullptr, double* tx = nullptr, double* ty = nullptr) const override;

    void Invert() override;

    bool IsEqual(const wxGraphicsMatrixData* t) const override;

    bool IsIdentity() const override;

    void Translate(double dx, double dy) override;

    void Scale(double xScale, double yScale) override;

    void Rotate(double angle) override;

    void TransformPoint(double* x, double* y) const override;

    void TransformDistance(double* dx, double* dy) const override;

    void* GetNativeMatrix() const override;

    D2D1::Matrix3x2F GetMatrix3x2F() const;

private:
    D2D1::Matrix3x2F m_matrix;
};

//-----------------------------------------------------------------------------
// wxD2DMatrixData implementation
//-----------------------------------------------------------------------------

wxD2DMatrixData::wxD2DMatrixData(wxGraphicsRenderer* renderer) : wxGraphicsMatrixData(renderer)
{
    m_matrix = D2D1::Matrix3x2F::Identity();
}

wxD2DMatrixData::wxD2DMatrixData(wxGraphicsRenderer* renderer, const D2D1::Matrix3x2F& matrix) :
    wxGraphicsMatrixData(renderer), m_matrix(matrix)
{
}

wxGraphicsObjectRefData* wxD2DMatrixData::Clone() const
{
    return new wxD2DMatrixData(GetRenderer(), m_matrix);
}

void wxD2DMatrixData::Concat(const wxGraphicsMatrixData* t)
{
    // Elements of resulting matrix are modified in-place in SetProduct()
    // so multiplied matrices cannot be the instances of the resulting matrix.
    // Note that parameter matrix (t) is the multiplicand.
    const D2D1::Matrix3x2F m1(dynamic_cast<const wxD2DMatrixData*>(t)->m_matrix);
    const D2D1::Matrix3x2F m2(m_matrix);
    m_matrix.SetProduct(m1, m2);
}

void wxD2DMatrixData::Set(double a, double b, double c, double d, double tx, double ty)
{
    m_matrix._11 = gsl::narrow_cast<float>(a);
    m_matrix._12 = gsl::narrow_cast<float>(b);
    m_matrix._21 = gsl::narrow_cast<float>(c);
    m_matrix._22 = gsl::narrow_cast<float>(d);
    m_matrix._31 = gsl::narrow_cast<float>(tx);
    m_matrix._32 = gsl::narrow_cast<float>(ty);
}

void wxD2DMatrixData::Get(double* a, double* b,  double* c, double* d, double* tx, double* ty) const
{
    *a =  static_cast<double>(m_matrix._11);
    *b =  static_cast<double>(m_matrix._12);
    *c =  static_cast<double>(m_matrix._21);
    *d =  static_cast<double>(m_matrix._22);
    *tx = static_cast<double>(m_matrix._31);
    *ty = static_cast<double>(m_matrix._32);
}

void wxD2DMatrixData::Invert()
{
    m_matrix.Invert();
}

bool wxD2DMatrixData::IsEqual(const wxGraphicsMatrixData* t) const
{
    return m_matrix == dynamic_cast<const wxD2DMatrixData*>(t)->m_matrix;
}

bool wxD2DMatrixData::IsIdentity() const
{
    return m_matrix.IsIdentity();
}

void wxD2DMatrixData::Translate(double dx, double dy)
{
    m_matrix = D2D1::Matrix3x2F::Translation(gsl::narrow_cast<float>(dx), gsl::narrow_cast<float>(dy)) * m_matrix;
}

void wxD2DMatrixData::Scale(double xScale, double yScale)
{
    m_matrix = D2D1::Matrix3x2F::Scale(gsl::narrow_cast<float>(xScale), gsl::narrow_cast<float>(yScale)) * m_matrix;
}

void wxD2DMatrixData::Rotate(double angle)
{
    m_matrix = D2D1::Matrix3x2F::Rotation(gsl::narrow_cast<float>(wxRadToDeg(angle))) * m_matrix;
}

void wxD2DMatrixData::TransformPoint(double* x, double* y) const
{
    D2D1_POINT_2F result = m_matrix.TransformPoint(D2D1::Point2F(gsl::narrow_cast<float>(*x),
                                                                 gsl::narrow_cast<float>(*y)));
    *x = static_cast<double>(result.x);
    *y = static_cast<double>(result.y);
}

void wxD2DMatrixData::TransformDistance(double* dx, double* dy) const
{
    D2D1::Matrix3x2F noTranslationMatrix = m_matrix;
    noTranslationMatrix._31 = 0;
    noTranslationMatrix._32 = 0;
    D2D1_POINT_2F result = noTranslationMatrix.TransformPoint(D2D1::Point2F(gsl::narrow_cast<float>(*dx),
                                                                            gsl::narrow_cast<float>(*dy)));
    *dx = static_cast<double>(result.x);
    *dy = static_cast<double>(result.y);
}

void* wxD2DMatrixData::GetNativeMatrix() const
{
    return const_cast<void*>(static_cast<const void*>(&m_matrix));
}

D2D1::Matrix3x2F wxD2DMatrixData::GetMatrix3x2F() const
{
    return m_matrix;
}

const wxD2DMatrixData* wxGetD2DMatrixData(const wxGraphicsMatrix& matrix)
{
    return dynamic_cast<const wxD2DMatrixData*>(matrix.GetMatrixData());
}

//-----------------------------------------------------------------------------
// wxD2DPathData declaration
//-----------------------------------------------------------------------------

bool operator==(const D2D1_POINT_2F& lhs, const D2D1_POINT_2F& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

class wxD2DPathData : public wxGraphicsPathData
{
public :

    // ID2D1PathGeometry objects are device-independent resources created
    // from a ID2D1Factory. This means we can safely create the resource outside
    // (the wxD2DRenderer handles this) and store it here since it never gets
    // thrown away by the GPU.
    wxD2DPathData(wxGraphicsRenderer* renderer, ID2D1Factory* d2dFactory);

    ~wxD2DPathData() override;

    void SetFillMode(D2D1_FILL_MODE fillMode)
    {
        m_fillMode = fillMode;
    }

    D2D1_FILL_MODE GetFillMode() const
    {
        return m_fillMode;
    }

    ID2D1PathGeometry* GetPathGeometry();

    // This closes the geometry sink, ensuring all the figures are stored inside
    // the ID2D1PathGeometry. Calling this method is required before any draw operation
    // involving a path.
    void Flush() noexcept;

    wxGraphicsObjectRefData* Clone() const override;

    // begins a new subpath at (x,y)
    void MoveToPoint(double x, double y) override;

    // adds a straight line from the current point to (x,y)
    void AddLineToPoint(double x, double y) override;

    // adds a cubic Bezier curve from the current point, using two control points and an end point
    void AddCurveToPoint(double cx1, double cy1, double cx2, double cy2, double x, double y) override;

    // adds an arc of a circle centering at (x,y) with radius (r) from startAngle to endAngle
    void AddArc(double x, double y, double r, double startAngle, double endAngle, bool clockwise) override;

    // gets the last point of the current path, (0,0) if not yet set
    void GetCurrentPoint(double* x, double* y) const override;

    // adds another path
    void AddPath(const wxGraphicsPathData* path) override;

    // closes the current sub-path
    void CloseSubpath() override;

    // returns the native path
    void* GetNativePath() const override;

    // give the native path returned by GetNativePath() back (there might be some deallocations necessary)
    void UnGetNativePath(void* WXUNUSED(p)) const override {}

    // transforms each point of this path by the matrix
    void Transform(const wxGraphicsMatrixData* matrix) override;

    // gets the bounding box enclosing all points (possibly including control points)
    void GetBox(double* x, double* y, double* w, double *h) const override;

    bool Contains(double x, double y, wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven) const override;

    // appends an ellipsis as a new closed subpath fitting the passed rectangle
    void AddCircle(double x, double y, double r) override;

    // appends an ellipse
    void AddEllipse(double x, double y, double w, double h) override;

private:
    void EnsureGeometryOpen();

    void EnsureSinkOpen();

    void EnsureFigureOpen(const D2D1_POINT_2F& pos);

    void EndFigure(D2D1_FIGURE_END figureEnd);

    ID2D1Geometry* GetFullGeometry(D2D1_FILL_MODE fillMode) const;

    bool IsEmpty() const;
    bool IsStateSafeForFlush() const;

    struct GeometryStateData
    {
        bool m_isCurrentPointSet;
        D2D1_POINT_2F m_currentPoint;
        bool m_isFigureOpen;
        D2D1_POINT_2F m_figureStart;
        bool m_isFigureLogStartSet;
        D2D1_POINT_2F m_figureLogStart;
    };
    void SaveGeometryState(GeometryStateData& data) const;
    void RestoreGeometryState(const GeometryStateData& data);

    wxCOMPtr<ID2D1PathGeometry> m_pathGeometry;

    wxCOMPtr<ID2D1GeometrySink> m_geometrySink;

    wxCOMPtr<ID2D1Factory> m_direct2dfactory;

    mutable wxCOMPtr<ID2D1GeometryGroup> m_combinedGeometry;
    std::vector<ID2D1Geometry*> m_pTransformedGeometries;

    bool m_currentPointSet;
    D2D1_POINT_2F m_currentPoint;

    bool m_figureOpened;
    D2D1_POINT_2F m_figureStart;
    bool m_figureLogStartSet;
    D2D1_POINT_2F m_figureLogStart;

    bool m_geometryWritable;

    D2D1_FILL_MODE m_fillMode;
};

//-----------------------------------------------------------------------------
// wxD2DPathData implementation
//-----------------------------------------------------------------------------

wxD2DPathData::wxD2DPathData(wxGraphicsRenderer* renderer, ID2D1Factory* d2dFactory) :
    wxGraphicsPathData(renderer),
    m_direct2dfactory(d2dFactory),
    m_currentPointSet(false),
    m_currentPoint(D2D1::Point2F(0.0f, 0.0f)),
    m_figureOpened(false),
    m_figureStart(D2D1::Point2F(0.0f, 0.0f)),
    m_figureLogStartSet(false),
    m_figureLogStart(D2D1::Point2F(0.0f, 0.0f)),
    m_geometryWritable(true),
    m_fillMode(D2D1_FILL_MODE_ALTERNATE)
{
    m_direct2dfactory->CreatePathGeometry(&m_pathGeometry);
    // To properly initialize path geometry there is also
    // necessary to open at least once its geometry sink.
    m_pathGeometry->Open(&m_geometrySink);
}

wxD2DPathData::~wxD2DPathData()
{
    Flush();

    std::for_each(m_pTransformedGeometries.begin(),
                  m_pTransformedGeometries.end(),
                  [](auto& transformedGeometry){ transformedGeometry->Release(); });
}

ID2D1PathGeometry* wxD2DPathData::GetPathGeometry()
{
    return m_pathGeometry;
}

wxD2DPathData::wxGraphicsObjectRefData* wxD2DPathData::Clone() const
{
    wxD2DPathData* newPathData = new wxD2DPathData(GetRenderer(), m_direct2dfactory);

    newPathData->EnsureGeometryOpen();

    // Only geometry with closed sink can be
    // transferred to another geometry object with
    // ID2D1PathGeometry::Stream() so we have to check
    // if actual transfer succeeded.

    // Transfer geometry to the new geometry sink.
    HRESULT hr = m_pathGeometry->Stream(newPathData->m_geometrySink);
    wxASSERT_MSG( SUCCEEDED(hr), wxS("Current geometry is in invalid state") );
    if ( FAILED(hr) )
    {
        delete newPathData;
        return nullptr;
    }

    // Copy the collection of transformed geometries.
    ID2D1TransformedGeometry* pTransformedGeometry;
    for ( size_t i = 0; i < m_pTransformedGeometries.size(); i++ )
    {
        pTransformedGeometry = nullptr;
        hr = m_direct2dfactory->CreateTransformedGeometry(
                    m_pTransformedGeometries[i],
                    D2D1::Matrix3x2F::Identity(), &pTransformedGeometry);
        wxASSERT_MSG( SUCCEEDED(hr), wxFAILED_HRESULT_MSG(hr) );
        newPathData->m_pTransformedGeometries.push_back(pTransformedGeometry);
    }

    // Copy positional data.
    GeometryStateData curState;
    SaveGeometryState(curState);
    newPathData->RestoreGeometryState(curState);

    return newPathData;
}

void wxD2DPathData::Flush() noexcept
{
    if (m_geometrySink != nullptr)
    {
        if ( m_figureOpened )
        {
            m_geometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
            m_figureOpened = false;
        }

        if( m_geometryWritable )
        {
            HRESULT hr = m_geometrySink->Close();
            wxCHECK_HRESULT_RET(hr);
            m_geometryWritable = false;
        }
    }
}

void wxD2DPathData::EnsureGeometryOpen()
{
    if (!m_geometryWritable)
    {
        wxCOMPtr<ID2D1PathGeometry> newPathGeometry;
        HRESULT hr;
        hr = m_direct2dfactory->CreatePathGeometry(&newPathGeometry);
        wxCHECK_HRESULT_RET(hr);

        m_geometrySink.reset();
        hr = newPathGeometry->Open(&m_geometrySink);
        wxCHECK_HRESULT_RET(hr);

        if (m_pathGeometry != nullptr)
        {
            hr = m_pathGeometry->Stream(m_geometrySink);
            wxCHECK_HRESULT_RET(hr);
        }

        m_pathGeometry = newPathGeometry;
        m_geometryWritable = true;
    }
}

void wxD2DPathData::EnsureSinkOpen()
{
    EnsureGeometryOpen();

    if (m_geometrySink == nullptr)
    {
        HRESULT hr = m_pathGeometry->Open(&m_geometrySink);
        wxCHECK_HRESULT_RET(hr);
    }
}

void wxD2DPathData::EnsureFigureOpen(const D2D1_POINT_2F& pos)
{
    EnsureSinkOpen();

    if (!m_figureOpened)
    {
        m_figureStart = pos;
        m_geometrySink->BeginFigure(m_figureStart, D2D1_FIGURE_BEGIN_FILLED);
        m_figureOpened = true;
        m_currentPoint = m_figureStart;
    }
}

void wxD2DPathData::EndFigure(D2D1_FIGURE_END figureEnd)
{
    if (m_figureOpened)
    {
        // Ensure that sub-path being closed contains at least one point.
        if( figureEnd == D2D1_FIGURE_END_CLOSED )
            m_geometrySink->AddLine(m_currentPoint);

        if( figureEnd == D2D1_FIGURE_END_OPEN ||
            !m_figureLogStartSet ||
            m_figureLogStart == m_figureStart )
        {
            // If figure will remain open or if its logical startpoint
            // is not used or if it is the same as the actual
            // startpoint then we can end the figure in a standard way.
            m_geometrySink->EndFigure(figureEnd);
        }
        else
        {
            // If we want to end and close the figure for which
            // logical startpoint is not the same as actual startpoint
            // we have to fill the gap between the actual and logical
            // endpoints on our own.
            m_geometrySink->AddLine(m_figureLogStart);
            m_geometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
            m_figureStart = m_figureLogStart;
        }
        m_figureOpened = false;
        m_figureLogStartSet = false;

        // If the figure is closed then current point
        // should be moved to the beginning of the figure.
        if( figureEnd == D2D1_FIGURE_END_CLOSED )
            m_currentPoint = m_figureStart;
    }
}

ID2D1Geometry* wxD2DPathData::GetFullGeometry(D2D1_FILL_MODE fillMode) const
{
    // Our final path geometry is represented by geometry group
    // which contains all transformed geometries plus current geometry.

    // We have to store pointers to all transformed geometries
    // as well as pointer to the current geometry in the auxiliary array.
    const size_t numGeometries = m_pTransformedGeometries.size();

    std::vector<ID2D1Geometry*> pGeometries(numGeometries + 1);
    
    for( size_t i = 0; i < numGeometries; i++ )
        pGeometries[i] = m_pTransformedGeometries[i];

    pGeometries[numGeometries] = m_pathGeometry;

    // And use this array as a source to create geometry group.
    m_combinedGeometry.reset();
    HRESULT hr = m_direct2dfactory->CreateGeometryGroup(fillMode,
                                  pGeometries.data(), numGeometries + 1, &m_combinedGeometry);
    wxFAILED_HRESULT_MSG(hr);

    return m_combinedGeometry;
}

bool wxD2DPathData::IsEmpty() const
{
    return !m_currentPointSet && !m_figureOpened &&
            m_pTransformedGeometries.size() == 0;
}

bool wxD2DPathData::IsStateSafeForFlush() const
{
    // Only geometry with not yet started figure
    // or with started but empty figure can be fully
    // restored to its initial state after invoking Flush().
    if( !m_figureOpened )
        return true;

    D2D1_POINT_2F actFigureStart = m_figureLogStartSet ?
                        m_figureLogStart : m_figureStart;
    return m_currentPoint == actFigureStart;
}

void wxD2DPathData::SaveGeometryState(GeometryStateData& data) const
{
    data.m_isFigureOpen = m_figureOpened;
    data.m_isFigureLogStartSet = m_figureLogStartSet;
    data.m_isCurrentPointSet = m_currentPointSet;
    data.m_currentPoint = m_currentPoint;
    data.m_figureStart = m_figureStart;
    data.m_figureLogStart = m_figureLogStart;
}

void wxD2DPathData::RestoreGeometryState(const GeometryStateData& data)
{
    if( data.m_isFigureOpen )
    {
        // If the figure has to be re-started at the startpoint
        // which is not the current point then we have to start it
        // physically at the current point but with storing also its
        // logical startpoint to use it later on to close the figure,
        // if necessary.
        // Ending and closing the figure using this proxy startpoint
        // is only a simulation of regular closure and figure can behave
        // in a slightly different way than figure closed with physical
        // startpoint so this action should be avoided if only possible.
        D2D1_POINT_2F actFigureStart = data.m_isFigureLogStartSet ?
                         data.m_figureLogStart : data.m_figureStart;
        if ( !(data.m_currentPoint == actFigureStart) )
        {
            m_figureLogStart = actFigureStart;
            m_figureLogStartSet = true;
            EnsureFigureOpen(data.m_currentPoint);
        }
        else
        {
            EnsureFigureOpen(actFigureStart);
        }
    }
    else
    {
        m_figureOpened = false;
    }

    m_currentPointSet = data.m_isCurrentPointSet;
    m_currentPoint = data.m_isCurrentPointSet ?
                data.m_currentPoint : D2D1::Point2F(0.0F, 0.0F);
}

void wxD2DPathData::MoveToPoint(double x, double y)
{
    // Close current sub-path (leaving the figure as is).
    EndFigure(D2D1_FIGURE_END_OPEN);
    // Store new current point
    m_currentPoint = D2D1::Point2F(gsl::narrow_cast<float>(x), gsl::narrow_cast<float>(y));
    m_currentPointSet = true;
}

// adds a straight line from the current point to (x,y)
void wxD2DPathData::AddLineToPoint(double x, double y)
{
    // If current point is not yet set then
    // this function should behave as MoveToPoint.
    if( !m_currentPointSet )
    {
        MoveToPoint(x, y);
        return;
    }

    const auto xf = gsl::narrow_cast<float>(x);
    const auto yf = gsl::narrow_cast<float>(y);

    EnsureFigureOpen(m_currentPoint);
    m_geometrySink->AddLine(D2D1::Point2F(xf, yf));

    m_currentPoint = D2D1::Point2F(xf, yf);
}

// adds a cubic Bezier curve from the current point, using two control points and an end point
void wxD2DPathData::AddCurveToPoint(double cx1, double cy1, double cx2, double cy2, double x, double y)
{
    // If no current point is set then this function should behave
    // as if preceded by a call to MoveToPoint(cx1, cy1).
    if( !m_currentPointSet )
        MoveToPoint(cx1, cy1);

    EnsureFigureOpen(m_currentPoint);

    D2D1_BEZIER_SEGMENT bezierSegment = {
        { gsl::narrow_cast<float>(cx1), gsl::narrow_cast<float>(cy1) },
        { gsl::narrow_cast<float>(cx2), gsl::narrow_cast<float>(cy2) },
        { gsl::narrow_cast<float>(x),   gsl::narrow_cast<float>(y) } };
    m_geometrySink->AddBezier(bezierSegment);

    m_currentPoint = D2D1::Point2F(gsl::narrow_cast<float>(x), gsl::narrow_cast<float>(y));
}

// adds an arc of a circle centering at (x,y) with radius (r) from startAngle to endAngle
void wxD2DPathData::AddArc(double x, double y, double r, double startAngle, double endAngle, bool clockwise)
{
    double angle;

    // For the sake of compatibility normalize angles the same way
    // as it is done in Cairo.
    if ( clockwise )
    {
        // If endAngle < startAngle it needs to be progressively
        // increased by 2*M_PI until endAngle > startAngle.
        if ( endAngle < startAngle )
        {
            while ( endAngle <= startAngle )
            {
                endAngle += 2.0 * std::numbers::pi;
            }
        }

        angle = endAngle - startAngle;
    }
    else
    {
        // If endAngle > startAngle it needs to be progressively
        // decreased by 2*M_PI until endAngle < startAngle.
        if ( endAngle > startAngle )
        {
            while ( endAngle >= startAngle )
            {
                endAngle -= 2.0 * std::numbers::pi;
            }
        }

        angle = startAngle - endAngle;
    }

    const auto start = wxPoint2DDouble(std::cos(startAngle) * r, std::sin(startAngle) * r);
    const auto end = wxPoint2DDouble(std::cos(endAngle) * r, std::sin(endAngle) * r);

    // To ensure compatibility with Cairo an initial
    // line segment to the beginning of the arc needs
    // to be added to the path.
    if ( m_figureOpened )
    {
        AddLineToPoint(start.m_x + x, start.m_y + y);
    }
    else if ( m_currentPointSet )
    {
        EnsureFigureOpen(m_currentPoint);
        AddLineToPoint(start.m_x + x, start.m_y + y);
    }
    else
    {
        MoveToPoint(start.m_x + x, start.m_y + y);
        EnsureFigureOpen(m_currentPoint);
    }

    const D2D1_SWEEP_DIRECTION sweepDirection = clockwise ?
       D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
    const D2D1_SIZE_F size = D2D1::SizeF(gsl::narrow_cast<FLOAT>(r), gsl::narrow_cast<FLOAT>(r));

    if ( angle >= 2.0 * std::numbers::pi)
    {
        // In addition to arc we need to draw full circle(s).
        // Remarks:
        // 1. Parity of the number of the circles has to be
        // preserved because this matters when path would be
        // filled with wxPolygonFillMode::OddEven flag set (using
        // D2D1_FILL_MODE_ALTERNATE mode) when number of the
        // edges is counted.
        // 2. ID2D1GeometrySink::AddArc() doesn't work
        // with 360-degree arcs so we need to construct
        // the circle from two halves.
        D2D1_ARC_SEGMENT circleSegment1 =
        {
            D2D1::Point2(gsl::narrow_cast<float>(x - start.m_x), gsl::narrow_cast<float>(y - start.m_y)),  // end point
            size,                     // size
            0.0f,                     // rotation
            sweepDirection,           // sweep direction
            D2D1_ARC_SIZE_SMALL       // arc size
        };
        D2D1_ARC_SEGMENT circleSegment2 =
        {
            D2D1::Point2(gsl::narrow_cast<float>(x + start.m_x), gsl::narrow_cast<float>(y + start.m_y)),  // end point
            size,                     // size
            0.0f,                     // rotation
            sweepDirection,           // sweep direction
            D2D1_ARC_SIZE_SMALL       // arc size
        };

        int numCircles = (int)(angle / (2.0 * std::numbers::pi));
        numCircles = (numCircles - 1) % 2 + 1;
        for( int i = 0; i < numCircles; i++ )
        {
            m_geometrySink->AddArc(circleSegment1);
            m_geometrySink->AddArc(circleSegment2);
        }

        // Reduce the angle to [0..2*M_PI) range.
        angle = std::fmod(angle, 2.0 * std::numbers::pi);
    }

    const D2D1_ARC_SIZE arcSize = angle > std::numbers::pi ?
       D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL;
    const D2D1_POINT_2F endPoint =
       D2D1::Point2(gsl::narrow_cast<float>(end.m_x + x), gsl::narrow_cast<float>(end.m_y + y));

    D2D1_ARC_SEGMENT arcSegment =
    {
        endPoint,                     // end point
        size,                         // size
        0.0f,                         // rotation
        sweepDirection,               // sweep direction
        arcSize                       // arc size
    };

    m_geometrySink->AddArc(arcSegment);

    m_currentPoint = endPoint;
}

// appends an ellipsis as a new closed subpath fitting the passed rectangle
void wxD2DPathData::AddCircle(double x, double y, double r)
{
    AddEllipse(x - r, y - r, r * 2, r * 2);
}

// appends an ellipse
void wxD2DPathData::AddEllipse(double x, double y, double w, double h)
{
    if ( w <= 0.0 || h <= 0.0 )
      return;

    // Calculate radii
    const double rx = w / 2.0;
    const double ry = h / 2.0;

    MoveToPoint(x + w, y + ry);
    // Open new subpath
    EnsureFigureOpen(m_currentPoint);

    D2D1_ARC_SEGMENT arcSegmentLower =
    {
        D2D1::Point2((FLOAT)(x), (FLOAT)(y + ry)),     // end point
        D2D1::SizeF((FLOAT)(rx), (FLOAT)(ry)),         // size
        0.0f,
        D2D1_SWEEP_DIRECTION_CLOCKWISE,
        D2D1_ARC_SIZE_SMALL
    };
    m_geometrySink->AddArc(arcSegmentLower);

    D2D1_ARC_SEGMENT arcSegmentUpper =
    {
        D2D1::Point2((FLOAT)(x + w), (FLOAT)(y + ry)), // end point
        D2D1::SizeF((FLOAT)(rx), (FLOAT)(ry)),         // size
        0.0f,
        D2D1_SWEEP_DIRECTION_CLOCKWISE,
        D2D1_ARC_SIZE_SMALL
    };
    m_geometrySink->AddArc(arcSegmentUpper);

    CloseSubpath();
}

// gets the last point of the current path, (0,0) if not yet set
void wxD2DPathData::GetCurrentPoint(double* x, double* y) const
{
    if (x != nullptr) *x = m_currentPoint.x;
    if (y != nullptr) *y = m_currentPoint.y;
}

// adds another path
void wxD2DPathData::AddPath(const wxGraphicsPathData* path)
{
    wxD2DPathData* pathSrc =
         const_cast<wxD2DPathData*>(dynamic_cast<const wxD2DPathData*>(path));

    // Nothing to do if geometry of appended path is not initialized.
    if ( pathSrc->m_pathGeometry == nullptr || pathSrc->m_geometrySink == nullptr )
        return;

    // Because only closed geometry (with closed sink)
    // can be transferred to another geometry object with
    // ID2D1PathGeometry::Stream() so we have to close it
    // before any operation and to re-open afterwards.
    // Unfortunately, to close the sink it is also necessary
    // to end the figure it contains, if it was open.
    // After re-opening the geometry we should also re-start
    // the figure (if it was open) and restore its state but
    // it seems there is no straightforward way to do so
    // if the figure is not empty.
    //
    // So, only if appended path has a sub-path closed or
    // has an empty sub-path open there is possible to restore
    // its state after appending it to the current path and only
    // in this case the operation doesn't introduce side effects.

    // Nothing to do if appended path is empty.
    if ( pathSrc->IsEmpty() )
        return;

    // Save positional and auxiliary data
    // of the appended path and its geometry.
    GeometryStateData curStateSrc;
    pathSrc->SaveGeometryState(curStateSrc);

    // Close appended geometry.
    pathSrc->Flush();

    // Close current geometry (leaving the figure as is).
    Flush();

    ID2D1TransformedGeometry* pTransformedGeometry = nullptr;
    // Add current geometry to the collection transformed geometries.
    HRESULT hr = m_direct2dfactory->CreateTransformedGeometry(m_pathGeometry,
                        D2D1::Matrix3x2F::Identity(), &pTransformedGeometry);
    wxCHECK_HRESULT_RET(hr);
    m_pTransformedGeometries.push_back(pTransformedGeometry);

    // Add to the collection transformed geometries from the appended path.
    for ( size_t i = 0; i < pathSrc->m_pTransformedGeometries.size(); i++ )
    {
        pTransformedGeometry = nullptr;
        hr = m_direct2dfactory->CreateTransformedGeometry(
                    pathSrc->m_pTransformedGeometries[i],
                    D2D1::Matrix3x2F::Identity(), &pTransformedGeometry);
        wxCHECK_HRESULT_RET(hr);
        m_pTransformedGeometries.push_back(pTransformedGeometry);
    }

    // Clear and reopen current geometry.
    m_pathGeometry.reset();
    EnsureGeometryOpen();

    // Transfer appended geometry to the current geometry sink.
    hr = pathSrc->m_pathGeometry->Stream(m_geometrySink);
    wxCHECK_HRESULT_RET(hr);

    // Apply to the current path positional data from the appended path.
    // This operation fully sets geometry to the required state
    // only if it represents geometry without started figure
    // or with started but empty figure.
    RestoreGeometryState(curStateSrc);

    // Reopen appended geometry.
    pathSrc->EnsureGeometryOpen();
    // Restore its positional data.
    // This operation fully restores geometry to the required state
    // only if it represents geometry without started figure
    // or with started but empty figure.
    pathSrc->RestoreGeometryState(curStateSrc);
}

// closes the current sub-path
void wxD2DPathData::CloseSubpath()
{
    // If we have a sub-path open by call to MoveToPoint(),
    // which doesn't open a new figure by itself,
    // we have to open a new figure now to get a required 1-point path.
    if ( !m_figureOpened && m_currentPointSet )
    {
        EnsureFigureOpen(m_currentPoint);
    }
    // Close sub-path and close the figure.
    if ( m_figureOpened )
    {
        EndFigure(D2D1_FIGURE_END_CLOSED);
        MoveToPoint(m_figureStart.x, m_figureStart.y);
    }
}

void* wxD2DPathData::GetNativePath() const
{
    return GetFullGeometry(GetFillMode());
}

void wxD2DPathData::Transform(const wxGraphicsMatrixData* matrix)
{
    // Unfortunately, it looks there is no straightforward way to apply
    // transformation to the current underlying path geometry
    // (ID2D1PathGeometry object) "in-place" (ie. transform it and use
    // for further graphics operations, including next transformations too).
    // Some simple methods offered by D2D are not useful for these purposes:
    // 1. ID2D1Factory::CreateTransformedGeometry() converts ID2D1PathGeometry
    // object to ID2D1TransformedGeometry object but ID2D1TransformedGeometry
    // inherits from ID2D1Geometry (not from ID2D1PathGeometry)
    // and hence cannot be used for further path operations.
    // 2. ID2D1Geometry::CombineWithGeometry() which could be used to get final
    // path geometry by combining empty geometry with transformed geometry
    // doesn't offer any combine mode which would produce a "sum" of geometries
    // (D2D1_COMBINE_MODE_UNION produces kind of outline). Moreover the result
    // is stored in ID2D1SimplifiedGeometrySink not in ID2DGeometrySink.

    // So, it seems that ability to transform the wxGraphicsPath
    // (several times) and still use it after this operation(s)
    // can be achieved (only?) by using a geometry group object
    // (ID2D1GeometryGroup) this way:
    // 1. After applying transformation to the current path geometry with
    // ID2D1Factory::CreateTransformedGeometry() the result is stored
    // in the collection of transformed geometries (an auxiliary array)
    // and after that a new (empty) geometry is open (in the same state
    // as just closed one) and this geometry is used as a current one
    // for further graphics operations.
    // 2. Since above steps are done at every transformation so our effective
    // geometry will be a superposition of all previously transformed
    // geometries stored in the collection (array) and the current
    // operational geometry.
    // 3. If there is necessary to use this combined effective geometry
    // in any operation then ID2D1GeometryGroup created with
    // ID2D1Factory::CreateGeometryGroup() from the collection
    // of stored geometries will act as a proxy geometry.

    const auto* m = static_cast<D2D1::Matrix3x2F*>(matrix->GetNativeMatrix());

    // Save current positional data.
    GeometryStateData curState;
    SaveGeometryState(curState);
    // We need to close the geometry what requires also to end a figure
    // (if started). This ended figure should be re-started in its initial
    // state when all path processing is done but due to the Direct2D
    // constraints this can be fully done only if open figure was empty.
    // So, Transform() can be safely called if path doesn't contain the open
    // sub-path or if open sub-path is empty.

    // Close current geometry.
    Flush();

    HRESULT hr;
    ID2D1TransformedGeometry* pTransformedGeometry;
    // Apply given transformation to all previously stored geometries too.
    for( size_t i = 0; i < m_pTransformedGeometries.size(); i++ )
    {
        pTransformedGeometry = nullptr;
        hr = m_direct2dfactory->CreateTransformedGeometry(m_pTransformedGeometries[i], m, &pTransformedGeometry);
        wxCHECK_HRESULT_RET(hr);

        m_pTransformedGeometries[i]->Release();
        m_pTransformedGeometries[i] = pTransformedGeometry;
    }

    // Transform current geometry and add the result
    // to the collection of transformed geometries.
    pTransformedGeometry = nullptr;
    hr = m_direct2dfactory->CreateTransformedGeometry(m_pathGeometry, m, &pTransformedGeometry);
    wxCHECK_HRESULT_RET(hr);
    m_pTransformedGeometries.push_back(pTransformedGeometry);

    // Clear and reopen current geometry.
    m_pathGeometry.reset();
    EnsureGeometryOpen();
    // Restore the figure with transformed positional data.
    // This operation fully restores geometry to the required state
    // only if IsStateSafeForFlush() returns true.
    curState.m_currentPoint = m->TransformPoint(curState.m_currentPoint);
    curState.m_figureLogStart = m->TransformPoint(curState.m_figureLogStart);
    curState.m_figureStart = m->TransformPoint(curState.m_figureStart);
    RestoreGeometryState(curState);
}

void wxD2DPathData::GetBox(double* x, double* y, double* w, double *h) const
{
    D2D1_RECT_F bounds;
    ID2D1Geometry *curGeometry = GetFullGeometry(GetFillMode());
    HRESULT hr = curGeometry->GetBounds(D2D1::Matrix3x2F::Identity(), &bounds);
    wxCHECK_HRESULT_RET(hr);
    // Check if bounds are empty
    if ( bounds.left > bounds.right )
    {
        bounds.left = bounds.top = bounds.right = bounds.bottom = 0.0F;
    }
    if (x) *x = bounds.left;
    if (y) *y = bounds.top;
    if (w) *w = bounds.right - bounds.left;
    if (h) *h = bounds.bottom - bounds.top;
}

bool wxD2DPathData::Contains(double x, double y, wxPolygonFillMode fillStyle) const
{
    const D2D1_FILL_MODE fillMode = (fillStyle == wxPolygonFillMode::OddEven) ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING;
    ID2D1Geometry *curGeometry = GetFullGeometry(fillMode);

    BOOL result;
    curGeometry->FillContainsPoint(D2D1::Point2F(gsl::narrow_cast<float>(x), gsl::narrow_cast<float>(y)), D2D1::Matrix3x2F::Identity(), &result);
    return result != FALSE;
}

wxD2DPathData* wxGetD2DPathData(const wxGraphicsPath& path)
{
    return dynamic_cast<wxD2DPathData*>(path.GetGraphicsData());
}

// This utility class is used to read a color value with the format
// PBGRA from a byte stream and to write a color back to the stream.
// It's used in conjunction with the IWICBitmapSource or IWICBitmap
// pixel data to easily read and write color values.
struct wxPBGRAColor
{
    explicit wxPBGRAColor(BYTE* stream) :
        b(*stream), g(*(stream + 1)), r(*(stream + 2)), a(*(stream + 3))
    {}

    explicit wxPBGRAColor(const wxColor& color) :
        b(color.Blue()), g(color.Green()), r(color.Red()), a(color.Alpha())
    {}

    bool IsBlack() const { return r == 0 && g == 0 && b == 0; }

    void Write(BYTE* stream) const
    {
        *(stream + 0) = b;
        *(stream + 1) = g;
        *(stream + 2) = r;
        *(stream + 3) = a;
    }

    BYTE b, g, r, a;
};

namespace
{
wxCOMPtr<IWICBitmapSource> wxCreateWICBitmap(const WXHBITMAP sourceBitmap, bool hasAlpha, bool forceAlpha)
{
    HRESULT hr;

    wxCOMPtr<IWICBitmap> wicBitmap;
    hr = wxWICImagingFactory()->CreateBitmapFromHBITMAP(sourceBitmap, nullptr, hasAlpha ? WICBitmapUsePremultipliedAlpha : WICBitmapIgnoreAlpha, &wicBitmap);
    wxCHECK2_HRESULT_RET(hr, wxCOMPtr<IWICBitmapSource>(nullptr));

    wxCOMPtr<IWICFormatConverter> converter;
    hr = wxWICImagingFactory()->CreateFormatConverter(&converter);
    wxCHECK2_HRESULT_RET(hr, wxCOMPtr<IWICBitmapSource>(nullptr));

    WICPixelFormatGUID pixelFormat = hasAlpha || forceAlpha ? GUID_WICPixelFormat32bppPBGRA : GUID_WICPixelFormat32bppBGR;

    hr = converter->Initialize(
        wicBitmap,
        pixelFormat,
        WICBitmapDitherTypeNone, nullptr, 0.f,
        WICBitmapPaletteTypeMedianCut);
    wxCHECK2_HRESULT_RET(hr, wxCOMPtr<IWICBitmapSource>(nullptr));

    return wxCOMPtr<IWICBitmapSource>(converter);
}

inline wxCOMPtr<IWICBitmapSource> wxCreateWICBitmap(const wxBitmap& sourceBitmap, bool forceAlpha)
{
    return wxCreateWICBitmap(sourceBitmap.GetHBITMAP(), sourceBitmap.HasAlpha(), forceAlpha);
}

#if wxUSE_IMAGE
void CreateWICBitmapFromImage(const wxImage& img, bool forceAlpha, IWICBitmap** ppBmp)
{
    const int width = img.GetWidth();
    const int height = img.GetHeight();
    // Create a compatible WIC Bitmap
    WICPixelFormatGUID fmt = img.HasAlpha() || img.HasMask() || forceAlpha ? GUID_WICPixelFormat32bppPBGRA : GUID_WICPixelFormat32bppBGR;
    HRESULT hr = wxWICImagingFactory()->CreateBitmap(width, height, fmt, WICBitmapCacheOnLoad, ppBmp);
    wxCHECK_HRESULT_RET(hr);

    // Copy contents of source image to the WIC bitmap.
    WICRect rcLock = { 0, 0, width, height };
    wxCOMPtr<IWICBitmapLock> pLock;
    hr = (*ppBmp)->Lock(&rcLock, WICBitmapLockWrite, &pLock);
    wxCHECK_HRESULT_RET(hr);

    UINT rowStride = 0;
    hr = pLock->GetStride(&rowStride);
    wxCHECK_HRESULT_RET(hr);

    UINT bufferSize = 0;
    BYTE* pBuffer = nullptr;
    hr = pLock->GetDataPointer(&bufferSize, &pBuffer);
    wxCHECK_HRESULT_RET(hr);

    const unsigned char* imgRGB = img.GetData();    // source RGB buffer
    const unsigned char* imgAlpha = img.GetAlpha(); // source alpha buffer
    BYTE* pBmpBuffer = pBuffer;
    for ( int y = 0; y < height; y++ )
    {
        BYTE* pPixByte = pBmpBuffer;
        for ( int x = 0; x < width; x++ )
        {
            unsigned char r = *imgRGB++;
            unsigned char g = *imgRGB++;
            unsigned char b = *imgRGB++;
            if ( imgAlpha )
            {
                unsigned char a = *imgAlpha++;
                // Premultiply RGB values
                *pPixByte++ = (b * a + 127) / 255;
                *pPixByte++ = (g * a + 127) / 255;
                *pPixByte++ = (r * a + 127) / 255;
                *pPixByte++ = a;
            }
            else
            {
                *pPixByte++ = b;
                *pPixByte++ = g;
                *pPixByte++ = r;
                *pPixByte++ = 255;
            }
        }

        pBmpBuffer += rowStride;
    }

    // If there is a mask, set the alpha bytes in the target buffer to
    // fully transparent or retain original value
    if ( img.HasMask() )
    {
        const unsigned char mr = img.GetMaskRed();
        const unsigned char mg = img.GetMaskGreen();
        const unsigned char mb = img.GetMaskBlue();

        imgRGB = img.GetData();
        pBmpBuffer = pBuffer;
        for ( int y = 0; y < height; y++ )
        {
            BYTE* pPixByte = pBmpBuffer;
            for ( int x = 0; x < width; x++ )
            {
                if ( imgRGB[0] == mr && imgRGB[1] == mg && imgRGB[2] == mb )
                    pPixByte[0] = pPixByte[1] = pPixByte[2] = pPixByte[3] = 0;

                imgRGB += 3;
                pPixByte += 4;
            }

            pBmpBuffer += rowStride;
        }
    }
}

void CreateImageFromWICBitmap(IWICBitmap* pBmp, wxImage* pImg)
{
    UINT width, height;
    HRESULT hr = pBmp->GetSize(&width, &height);
    wxCHECK_HRESULT_RET(hr);

    WICRect rcLock = { 0, 0, (INT)width, (INT)height };
    wxCOMPtr<IWICBitmapLock> pLock;
    hr = pBmp->Lock(&rcLock, WICBitmapLockRead, &pLock);
    wxCHECK_HRESULT_RET(hr);

    UINT rowStride = 0;
    hr = pLock->GetStride(&rowStride);
    wxCHECK_HRESULT_RET(hr);

    UINT bufferSize = 0;
    BYTE* pBmpBuffer = nullptr;
    hr = pLock->GetDataPointer(&bufferSize, &pBmpBuffer);
    wxCHECK_HRESULT_RET(hr);

    WICPixelFormatGUID pixelFormat;
    hr = pLock->GetPixelFormat(&pixelFormat);
    wxCHECK_HRESULT_RET(hr);
    wxASSERT_MSG(pixelFormat == GUID_WICPixelFormat32bppPBGRA || pixelFormat == GUID_WICPixelFormat32bppBGR,
                 "Unsupported pixel format");

    // Only premultiplied ARGB bitmaps are supported.
    const bool hasAlpha = pixelFormat == GUID_WICPixelFormat32bppPBGRA;

    if ( pImg->IsOk() )
    {
        if ( pImg->GetWidth() != (int)width || pImg->GetHeight() != (int)height )
        {
            pImg->Resize(wxSize(width, height), wxPoint(0, 0));
        }
    }
    else
    {
        pImg->Create(width, height);
    }
    if ( hasAlpha && !pImg->HasAlpha() )
    {
        pImg->SetAlpha();
    }
    pImg->SetMask(false);

    unsigned char* destRGB = pImg->GetData();
    unsigned char* destAlpha = pImg->GetAlpha();
    for ( UINT y = 0; y < height; y++ )
    {
        BYTE* pPixByte = pBmpBuffer;
        for ( UINT x = 0; x < width; x++ )
        {
            wxPBGRAColor color = wxPBGRAColor(pPixByte);
            unsigned char a = hasAlpha ? color.a : wxIMAGE_ALPHA_OPAQUE;
            // Undo premultiplication for ARGB bitmap
            *destRGB++ = (a > 0 && a < 255)?(color.r * 255) / a : color.r;
            *destRGB++ = (a > 0 && a < 255)?(color.g * 255) / a : color.g;
            *destRGB++ = (a > 0 && a < 255)?(color.b * 255) / a : color.b;
            if ( destAlpha )
                *destAlpha++ = a;

            pPixByte += 4;
        }

        pBmpBuffer += rowStride;
    }
}
#endif // wxUSE_IMAGE
};

// WIC Bitmap Source for creating hatch patterned bitmaps
class wxHatchBitmapSource : public IWICBitmapSource
{
public:
    wxHatchBitmapSource(wxBrushStyle brushStyle, const wxColor& color) :
        m_brushStyle(brushStyle), m_color(color), m_refCount(0l)
    {
    }

    virtual ~wxHatchBitmapSource() = default;

    HRESULT STDMETHODCALLTYPE GetSize(__RPC__out UINT *width, __RPC__out UINT *height) override
    {
        if (width != nullptr) *width = 8;
        if (height != nullptr) *height = 8;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetPixelFormat(__RPC__out WICPixelFormatGUID *pixelFormat) override
    {
        if (pixelFormat != nullptr) *pixelFormat = GUID_WICPixelFormat32bppPBGRA;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetResolution(__RPC__out double *dpiX, __RPC__out double *dpiY) override
    {
        if (dpiX != nullptr) *dpiX = 96.0;
        if (dpiY != nullptr) *dpiY = 96.0;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CopyPalette(__RPC__in_opt IWICPalette*  WXUNUSED(palette)) override
    {
        return WINCODEC_ERR_PALETTEUNAVAILABLE;
    }

    HRESULT STDMETHODCALLTYPE CopyPixels(
        const WICRect* WXUNUSED(prc),
        UINT WXUNUSED(stride),
        UINT WXUNUSED(bufferSize),
        BYTE *buffer) override
    {
        // patterns are encoded in a bit map of size 8 x 8
        static constexpr unsigned char BDIAGONAL_PATTERN[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
        static constexpr unsigned char FDIAGONAL_PATTERN[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
        static constexpr unsigned char CROSSDIAG_PATTERN[8] = { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81 };
        static constexpr unsigned char CROSS_PATTERN[8] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF };
        static constexpr unsigned char HORIZONTAL_PATTERN[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF };
        static constexpr unsigned char VERTICAL_PATTERN[8] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };

        switch (m_brushStyle)
        {
            case wxBrushStyle::BDiagonalHatch:
                CopyPattern(buffer, BDIAGONAL_PATTERN);
                break;
            case wxBrushStyle::CrossDiagHatch:
                CopyPattern(buffer, CROSSDIAG_PATTERN);
                break;
            case wxBrushStyle::FDiagonalHatch:
                CopyPattern(buffer, FDIAGONAL_PATTERN);
                break;
            case wxBrushStyle::CrossHatch:
                CopyPattern(buffer, CROSS_PATTERN);
                break;
            case wxBrushStyle::HorizontalHatch:
                CopyPattern(buffer, HORIZONTAL_PATTERN);
                break;
            case wxBrushStyle::VerticalHatch:
                CopyPattern(buffer, VERTICAL_PATTERN);
                break;
            default:
                break;
        }

        return S_OK;
    }

    // Implementations adapted from: "Implementing IUnknown in C++"
    // http://msdn.microsoft.com/en-us/library/office/cc839627%28v=office.15%29.aspx

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID referenceId, void** object) override
    {
        if (!object)
        {
            return E_INVALIDARG;
        }

        *object = nullptr;

        if (referenceId == IID_IUnknown || referenceId == wxIID_IWICBitmapSource)
        {
            *object = (LPVOID)this;
            AddRef();
            return NOERROR;
        }

        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        InterlockedIncrement(&m_refCount);
        return m_refCount;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        wxCHECK_MSG(m_refCount > 0, 0, "Unbalanced number of calls to Release");

        ULONG refCount = InterlockedDecrement(&m_refCount);
        if (m_refCount == 0)
        {
            delete this;
        }
        return refCount;
    }

private:

    // Copies an 8x8 bit pattern to a PBGRA byte buffer
    void CopyPattern(BYTE* buffer, const unsigned char* pattern) const
    {
        static const wxPBGRAColor transparent(wxTransparentColour);

        int k = 0;

        for (int i = 0; i < 8; ++i)
        {
            for (int j = 7; j >= 0; --j)
            {
                const bool isColorBit = (pattern[i] & (1 << j)) > 0;
                (isColorBit ? m_color : transparent).Write(buffer + k);
                k += 4;
            }
        }
    }

    // The hatch style produced by this bitmap source
    const wxBrushStyle m_brushStyle;

    // The colour of the hatch
    const wxPBGRAColor m_color;

    // Internally used to implement IUnknown's reference counting
    ULONG m_refCount;
};

// RAII class hosting a WIC bitmap lock used for writing
// pixel data to a WICBitmap
class wxBitmapPixelWriteLock
{
public:
    explicit wxBitmapPixelWriteLock(IWICBitmap* bitmap)
    {
        // Retrieve the size of the bitmap
        UINT w, h;
        bitmap->GetSize(&w, &h);
        WICRect lockSize = {0, 0, (INT)w, (INT)h};

        // Obtain a bitmap lock for exclusive write
        bitmap->Lock(&lockSize, WICBitmapLockWrite, &m_pixelLock);
    }

    IWICBitmapLock* GetLock() { return m_pixelLock; }

private:
    wxCOMPtr<IWICBitmapLock> m_pixelLock;
};

class wxD2DBitmapResourceHolder : public wxD2DResourceHolder<ID2D1Bitmap>
{
public:
    explicit wxD2DBitmapResourceHolder(const wxBitmap& sourceBitmap)
    {
        HRESULT hr;
        if ( sourceBitmap.GetMask() )
        {
            int w = sourceBitmap.GetWidth();
            int h = sourceBitmap.GetHeight();

            wxCOMPtr<IWICBitmapSource> colorBitmap = wxCreateWICBitmap(sourceBitmap, true);
            wxCOMPtr<IWICBitmapSource> maskBitmap = wxCreateWICBitmap(sourceBitmap.GetMask()->GetBitmap(), false);

            hr = wxWICImagingFactory()->CreateBitmap(w, h, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &m_srcBitmap);
            wxCHECK_HRESULT_RET(hr);

            std::vector<BYTE> colorBuffer(4 * w * h);
            std::vector<BYTE> maskBuffer(4 * w * h);

            BYTE* resultBuffer;

            hr = colorBitmap->CopyPixels(nullptr, w * 4, 4 * w * h, colorBuffer.data());
            wxCHECK_HRESULT_RET(hr);
            hr = maskBitmap->CopyPixels(nullptr, w * 4, 4 * w * h, maskBuffer.data());
            wxCHECK_HRESULT_RET(hr);

            {
                wxBitmapPixelWriteLock lock(m_srcBitmap);

                UINT bufferSize = 0;
                hr = lock.GetLock()->GetDataPointer(&bufferSize, &resultBuffer);

                static const wxPBGRAColor transparentColor(wxTransparentColour);

                // Create the result bitmap
                for ( int i = 0; i < w * h * 4; i += 4 )
                {
                    wxPBGRAColor color(colorBuffer.data() + i);
                    wxPBGRAColor mask(maskBuffer.data() + i);

                    if ( mask.IsBlack() )
                    {
                        transparentColor.Write(resultBuffer + i);
                    }
                    else
                    {
                        color.Write(resultBuffer + i);
                    }
                }
            }
        }
        else
        {
            wxCOMPtr<IWICBitmapSource> srcBmp = wxCreateWICBitmap(sourceBitmap, false);
            hr = wxWICImagingFactory()->CreateBitmapFromSource(srcBmp, WICBitmapNoCache, &m_srcBitmap);
            wxCHECK_HRESULT_RET(hr);
        }
    }

    explicit wxD2DBitmapResourceHolder(IWICBitmap* pSrcBmp) :
        m_srcBitmap(pSrcBmp)
    {
    }

    wxSize GetSize() const
    {
        UINT w, h;
        HRESULT hr = m_srcBitmap->GetSize(&w, &h);
        wxCHECK2_HRESULT_RET(hr, wxSize());

        return {gsl::narrow_cast<int>(w), gsl::narrow_cast<int>(h)};
    }

#if wxUSE_IMAGE
    explicit wxD2DBitmapResourceHolder(const wxImage& img)
    {
        CreateWICBitmapFromImage(img, false, &m_srcBitmap);
    }

    wxImage ConvertToImage() const
    {
        wxImage img;
        CreateImageFromWICBitmap(m_srcBitmap, &img);

        return img;
    }
#endif // wxUSE_IMAGE

    wxD2DBitmapResourceHolder* GetSubBitmap(double x, double y, double w, double h) const
    {
        wxCOMPtr<IWICBitmapClipper> clipper;
        HRESULT hr = wxWICImagingFactory()->CreateBitmapClipper(&clipper);
        wxCHECK2_HRESULT_RET(hr, nullptr);

        WICRect r = { (INT)x, (INT)y, (INT)w, (INT)h };
        hr = clipper->Initialize(m_srcBitmap, &r);
        wxCHECK2_HRESULT_RET(hr, nullptr);

        wxCOMPtr<IWICBitmap> subBmp;
        hr = wxWICImagingFactory()->CreateBitmapFromSource(clipper, WICBitmapNoCache, &subBmp);
        wxCHECK2_HRESULT_RET(hr, nullptr);

        return new wxD2DBitmapResourceHolder(subBmp);
    }

protected:
    void DoAcquireResource() override
    {
        HRESULT hr = GetContext()->CreateBitmapFromWicBitmap(m_srcBitmap, nullptr, &m_nativeResource);
        wxCHECK_HRESULT_RET(hr);
    }

private:
    wxCOMPtr<IWICBitmap> m_srcBitmap;
};

//-----------------------------------------------------------------------------
// wxD2DBitmapData declaration
//-----------------------------------------------------------------------------

class wxD2DBitmapData : public wxGraphicsBitmapData, public wxD2DManagedGraphicsData
{
public:
    using NativeType = wxD2DBitmapResourceHolder;

    wxD2DBitmapData(wxGraphicsRenderer* renderer, const wxBitmap& bitmap) :
        wxGraphicsBitmapData(renderer)
    {
        m_bitmapHolder = new NativeType(bitmap);
    }

    wxD2DBitmapData(wxGraphicsRenderer* renderer, const wxImage& image) :
        wxGraphicsBitmapData(renderer)
    {
        m_bitmapHolder = new NativeType(image);
    }

    wxD2DBitmapData(wxGraphicsRenderer* renderer, NativeType* pseudoNativeBitmap) :
        wxGraphicsBitmapData(renderer), m_bitmapHolder(pseudoNativeBitmap) {}

    ~wxD2DBitmapData() override;

    // returns the native representation
    void* GetNativeBitmap() const override;

    wxCOMPtr<ID2D1Bitmap> GetD2DBitmap();

    wxD2DManagedObject* GetManagedObject() override
    {
        return m_bitmapHolder;
    }

private:
    NativeType* m_bitmapHolder;
};

//-----------------------------------------------------------------------------
// wxD2DBitmapData implementation
//-----------------------------------------------------------------------------

wxD2DBitmapData::~wxD2DBitmapData()
{
    delete m_bitmapHolder;
}

void* wxD2DBitmapData::GetNativeBitmap() const
{
    return static_cast<void*>(m_bitmapHolder);
}

wxCOMPtr<ID2D1Bitmap> wxD2DBitmapData::GetD2DBitmap()
{
    return m_bitmapHolder->GetD2DResource();
}

wxD2DBitmapData* wxGetD2DBitmapData(const wxGraphicsBitmap& bitmap)
{
    return dynamic_cast<wxD2DBitmapData*>(bitmap.GetRefData());
}

// Helper class used to create and safely release a ID2D1GradientStopCollection from wxGraphicsGradientStops
class wxD2DGradientStopsHelper : public wxD2DResourceHolder<ID2D1GradientStopCollection>
{
public:
    explicit wxD2DGradientStopsHelper(const wxGraphicsGradientStops& gradientStops)
    {
        const int stopCount = gradientStops.GetCount();
        m_gradientStops.reserve(stopCount);
        for ( int i = 0; i < stopCount; ++i )
        {
            D2D1_GRADIENT_STOP stop
            {
                .position = gradientStops.Item(i).GetPosition(),
                .color = wxD2DConvertColour(gradientStops.Item(i).GetColour())
            };

            m_gradientStops.emplace_back(stop);
        }
    }

protected:
    void DoAcquireResource() override
    {
        wxCHECK_RET(!m_gradientStops.empty(), "No gradient stops provided");

        HRESULT hr = GetContext()->CreateGradientStopCollection(&m_gradientStops[0],
            m_gradientStops.size(), D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &m_nativeResource);
        wxCHECK_HRESULT_RET(hr);
    }

private:
    std::vector<D2D1_GRADIENT_STOP> m_gradientStops;
};

template <typename B>
class wxD2DBrushResourceHolder : public wxD2DResourceHolder<B>
{
public:
    explicit wxD2DBrushResourceHolder(const wxBrush& brush) : m_sourceBrush(brush) {}

protected:
    const wxBrush m_sourceBrush;
};

class wxD2DSolidBrushResourceHolder : public wxD2DBrushResourceHolder<ID2D1SolidColorBrush>
{
public:
    explicit wxD2DSolidBrushResourceHolder(const wxBrush& brush) : wxD2DBrushResourceHolder(brush) {}

protected:
    void DoAcquireResource() override
    {
        wxColour colour = m_sourceBrush.GetColour();
        HRESULT hr = GetContext()->CreateSolidColorBrush(wxD2DConvertColour(colour), &m_nativeResource);
        wxCHECK_HRESULT_RET(hr);
    }
};

class wxD2DBitmapBrushResourceHolder : public wxD2DBrushResourceHolder<ID2D1BitmapBrush>
{
public:
    explicit wxD2DBitmapBrushResourceHolder(const wxBrush& brush) : wxD2DBrushResourceHolder(brush) {}

protected:
    void DoAcquireResource() override
    {
        // TODO: cache this bitmap
        wxD2DBitmapResourceHolder bitmap(*(m_sourceBrush.GetStipple()));
        bitmap.Bind(GetManager());

        HRESULT result = GetContext()->CreateBitmapBrush(
            bitmap.GetD2DResource(),
            D2D1::BitmapBrushProperties(
            D2D1_EXTEND_MODE_WRAP,
            D2D1_EXTEND_MODE_WRAP,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR),
            &m_nativeResource);

        wxCHECK_HRESULT_RET(result);
    }
};

class wxD2DHatchBrushResourceHolder : public wxD2DBrushResourceHolder<ID2D1BitmapBrush>
{
public:
    explicit wxD2DHatchBrushResourceHolder(const wxBrush& brush) : wxD2DBrushResourceHolder(brush) {}

protected:
    void DoAcquireResource() override
    {
        wxCOMPtr<wxHatchBitmapSource> hatchBitmapSource{new wxHatchBitmapSource(m_sourceBrush.GetStyle(), m_sourceBrush.GetColour())};

        wxCOMPtr<ID2D1Bitmap> bitmap;

        HRESULT hr = GetContext()->CreateBitmapFromWicBitmap(hatchBitmapSource, &bitmap);
        wxCHECK_HRESULT_RET(hr);

        hr = GetContext()->CreateBitmapBrush(
            bitmap,
            D2D1::BitmapBrushProperties(
            D2D1_EXTEND_MODE_WRAP,
            D2D1_EXTEND_MODE_WRAP,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR),
            &m_nativeResource);
        wxCHECK_HRESULT_RET(hr);
    }
};

class wxD2DLinearGradientBrushResourceHolder : public wxD2DResourceHolder<ID2D1LinearGradientBrush>
{
public:
    struct LinearGradientInfo {
        const double x1;
        const double y1;
        const double x2;
        const double y2;
        const wxGraphicsGradientStops stops;
        const wxGraphicsMatrix matrix;
        LinearGradientInfo(double& x1_, double& y1_,
                           double& x2_, double& y2_,
                           const wxGraphicsGradientStops& stops_,
                           const wxGraphicsMatrix& matrix_)
            : x1(x1_), y1(y1_), x2(x2_), y2(y2_), stops(stops_), matrix(matrix_) {}
    };

    wxD2DLinearGradientBrushResourceHolder(double& x1, double& y1,
                                           double& x2, double& y2,
                                           const wxGraphicsGradientStops& stops,
                                           const wxGraphicsMatrix& matrix)
        : m_linearGradientInfo(x1, y1, x2, y2, stops, matrix) {}

protected:
    void DoAcquireResource() override
    {
        wxD2DGradientStopsHelper helper(m_linearGradientInfo.stops);
        helper.Bind(GetManager());

        HRESULT hr = GetContext()->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(gsl::narrow_cast<float>(m_linearGradientInfo.x1), gsl::narrow_cast<float>(m_linearGradientInfo.y1)),
                D2D1::Point2F(gsl::narrow_cast<float>(m_linearGradientInfo.x2), gsl::narrow_cast<float>(m_linearGradientInfo.y2))),
            helper.GetD2DResource(),
            &m_nativeResource);
        wxCHECK_HRESULT_RET(hr);

        if (! m_linearGradientInfo.matrix.IsNull())
        {
            D2D1::Matrix3x2F matrix = wxGetD2DMatrixData(m_linearGradientInfo.matrix)->GetMatrix3x2F();
            matrix.Invert();
            m_nativeResource->SetTransform(matrix);
        }
    }
private:
    const LinearGradientInfo m_linearGradientInfo;
};

class wxD2DRadialGradientBrushResourceHolder : public wxD2DResourceHolder<ID2D1RadialGradientBrush>
{
public:
    struct RadialGradientInfo {
        const double x1;
        const double y1;
        const double x2;
        const double y2;
        const double radius;
        const wxGraphicsGradientStops stops;
        const wxGraphicsMatrix matrix;

        RadialGradientInfo(double x1_, double y1_,
                           double x2_, double y2_,
                           double r,
                           const wxGraphicsGradientStops& stops_,
                           const wxGraphicsMatrix& matrix_)
            : x1(x1_), y1(y1_), x2(x2_), y2(y2_), radius(r), stops(stops_), matrix(matrix_) {}
    };

    wxD2DRadialGradientBrushResourceHolder(double& x1, double& y1,
                                           double& x2, double& y2,
                                           double& r,
                                           const wxGraphicsGradientStops& stops,
                                           const wxGraphicsMatrix& matrix)
        : m_radialGradientInfo(x1, y1, x2, y2, r, stops, matrix) {}

protected:
    void DoAcquireResource() override
    {
        wxD2DGradientStopsHelper helper(m_radialGradientInfo.stops);
        helper.Bind(GetManager());

        const auto xo = gsl::narrow_cast<float>(m_radialGradientInfo.x1 - m_radialGradientInfo.x2);
        const auto yo = gsl::narrow_cast<float>(m_radialGradientInfo.y1 - m_radialGradientInfo.y2);

        HRESULT hr = GetContext()->CreateRadialGradientBrush(
            D2D1::RadialGradientBrushProperties(
                D2D1::Point2F(gsl::narrow_cast<float>(m_radialGradientInfo.x1), gsl::narrow_cast<float>(m_radialGradientInfo.y1)),
                D2D1::Point2F(xo, yo),
                gsl::narrow_cast<float>(m_radialGradientInfo.radius),
                gsl::narrow_cast<float>(m_radialGradientInfo.radius)),
            helper.GetD2DResource(),
            &m_nativeResource);
        wxCHECK_HRESULT_RET(hr);

        if (! m_radialGradientInfo.matrix.IsNull())
        {
            D2D1::Matrix3x2F matrix = wxGetD2DMatrixData(m_radialGradientInfo.matrix)->GetMatrix3x2F();
            matrix.Invert();
            m_nativeResource->SetTransform(matrix);
        }
    }

private:
    const RadialGradientInfo m_radialGradientInfo;
};

//-----------------------------------------------------------------------------
// wxD2DBrushData declaration
//-----------------------------------------------------------------------------

class wxD2DBrushData : public wxGraphicsObjectRefData, public wxD2DManagedGraphicsData
{
public:
    wxD2DBrushData(wxGraphicsRenderer* renderer, const wxBrush brush);

    explicit wxD2DBrushData(wxGraphicsRenderer* renderer);

    void CreateLinearGradientBrush(double x1, double y1,
                                   double x2, double y2,
                                   const wxGraphicsGradientStops& stops,
                                   const wxGraphicsMatrix& matrix = wxNullGraphicsMatrix);

    void CreateRadialGradientBrush(double startX, double startY,
                                   double endX, double endY,
                                   double radius,
                                   const wxGraphicsGradientStops& stops,
                                   const wxGraphicsMatrix& matrix = wxNullGraphicsMatrix);

    ID2D1Brush* GetBrush() const
    {
        return (ID2D1Brush*)(m_brushResourceHolder->GetResource());
    }

    wxD2DManagedObject* GetManagedObject() override
    {
        return m_brushResourceHolder.get();
    }

private:
    std::shared_ptr<wxManagedResourceHolder> m_brushResourceHolder;
};

//-----------------------------------------------------------------------------
// wxD2DBrushData implementation
//-----------------------------------------------------------------------------

wxD2DBrushData::wxD2DBrushData(wxGraphicsRenderer* renderer, const wxBrush brush)
    : wxGraphicsObjectRefData(renderer), m_brushResourceHolder(nullptr)
{
    if (brush.GetStyle() == wxBrushStyle::Solid)
    {
        m_brushResourceHolder = std::make_shared<wxD2DSolidBrushResourceHolder>(brush);
    }
    else if (brush.IsHatch())
    {
        m_brushResourceHolder = std::make_shared<wxD2DHatchBrushResourceHolder>(brush);
    }
    else
    {
        m_brushResourceHolder = std::make_shared<wxD2DBitmapBrushResourceHolder>(brush);
    }
}

wxD2DBrushData::wxD2DBrushData(wxGraphicsRenderer* renderer)
    : wxGraphicsObjectRefData(renderer), m_brushResourceHolder(nullptr)
{
}

void wxD2DBrushData::CreateLinearGradientBrush(
    double x1, double y1,
    double x2, double y2,
    const wxGraphicsGradientStops& stops,
    const wxGraphicsMatrix& matrix)
{
    m_brushResourceHolder = std::make_shared<wxD2DLinearGradientBrushResourceHolder>(
        x1, y1, x2, y2, stops, matrix);
}

void wxD2DBrushData::CreateRadialGradientBrush(
    double startX, double startY,
    double endX, double endY,
    double radius,
    const wxGraphicsGradientStops& stops,
    const wxGraphicsMatrix& matrix)
{
    m_brushResourceHolder = std::make_shared<wxD2DRadialGradientBrushResourceHolder>(
        startX, startY, endX, endY, radius, stops, matrix);
}

wxD2DBrushData* wxGetD2DBrushData(const wxGraphicsBrush& brush)
{
    return dynamic_cast<wxD2DBrushData*>(brush.GetGraphicsData());
}

bool wxIsHatchPenStyle(wxPenStyle penStyle)
{
    return penStyle >= wxPenStyle::FirstHatch && penStyle <= wxPenStyle::LastHatch;
}

wxBrushStyle wxConvertPenStyleToBrushStyle(wxPenStyle penStyle)
{
    switch(penStyle)
    {
    case wxPenStyle::BDiagonalHatch:
        return wxBrushStyle::BDiagonalHatch;
    case wxPenStyle::CrossDiagHatch:
        return wxBrushStyle::CrossDiagHatch;
    case wxPenStyle::FDiagonalHatch:
        return wxBrushStyle::FDiagonalHatch;
    case wxPenStyle::CrossHatch:
        return wxBrushStyle::CrossHatch;
    case wxPenStyle::HorizontalHatch:
        return wxBrushStyle::HorizontalHatch;
    case wxPenStyle::VerticalHatch:
        return wxBrushStyle::VerticalHatch;
    default:
        return wxBrushStyle::Solid;
    }
}

//-----------------------------------------------------------------------------
// wxD2DPenData declaration
//-----------------------------------------------------------------------------

class wxD2DPenData : public wxGraphicsObjectRefData, public wxD2DManagedGraphicsData
{
public:
    wxD2DPenData(wxGraphicsRenderer* renderer,
                 ID2D1Factory* direct2dFactory,
                 const wxGraphicsPenInfo& info);

    void CreateStrokeStyle(ID2D1Factory* const direct2dfactory);

    ID2D1Brush* GetBrush();

    FLOAT GetWidth();
    bool IsZeroWidth() const;
    void SetWidth(const wxGraphicsContext* context);

    ID2D1StrokeStyle* GetStrokeStyle();

    wxD2DManagedObject* GetManagedObject() override
    {
        return m_stippleBrush->GetManagedObject();
    }

private:
    // We store the original pen description for later when we need to recreate
    // the device-dependent resources.
    const wxGraphicsPenInfo m_penInfo;

    // A stroke style is a device-independent resource.
    // Describes the caps, miter limit, line join, and dash information.
    wxCOMPtr<ID2D1StrokeStyle> m_strokeStyle;

    // Drawing outlines with Direct2D requires a brush for the color or stipple.
    std::shared_ptr<wxD2DBrushData> m_stippleBrush;

    // The width of the stroke
    float m_width;
};

//-----------------------------------------------------------------------------
// wxD2DPenData implementation
//-----------------------------------------------------------------------------

wxD2DPenData::wxD2DPenData(
    wxGraphicsRenderer* renderer,
    ID2D1Factory* direct2dFactory,
    const wxGraphicsPenInfo& info)
    : wxGraphicsObjectRefData(renderer),
      m_penInfo(info),
      m_width(gsl::narrow_cast<float>(info.GetWidth()))
{
    CreateStrokeStyle(direct2dFactory);

    wxBrush strokeBrush;

    if (m_penInfo.GetStyle() == wxPenStyle::Stipple)
    {
        strokeBrush.SetStipple(m_penInfo.GetStipple());
        strokeBrush.SetStyle(wxBrushStyle::Stipple);
    }
    else if(wxIsHatchPenStyle(m_penInfo.GetStyle()))
    {
        strokeBrush.SetStyle(wxConvertPenStyleToBrushStyle(m_penInfo.GetStyle()));
        strokeBrush.SetColour(m_penInfo.GetColour());
    }
    else
    {
        strokeBrush.SetColour(m_penInfo.GetColour());
        strokeBrush.SetStyle(wxBrushStyle::Solid);
    }

    switch ( m_penInfo.GetGradientType() )
    {
    case wxGradientType::None:
        m_stippleBrush = std::make_shared<wxD2DBrushData>(renderer, strokeBrush);
        break;

    case wxGradientType::Linear:
        m_stippleBrush = std::make_shared<wxD2DBrushData>(renderer);
        m_stippleBrush->CreateLinearGradientBrush(
                                m_penInfo.GetX1(), m_penInfo.GetY1(),
                                m_penInfo.GetX2(), m_penInfo.GetY2(),
                                m_penInfo.GetStops(),
                                m_penInfo.GetMatrix());
        break;

    case wxGradientType::Radial:
        m_stippleBrush = std::make_shared<wxD2DBrushData>(renderer);
        m_stippleBrush->CreateRadialGradientBrush(
                                m_penInfo.GetStartX(), m_penInfo.GetStartY(),
                                m_penInfo.GetEndX(), m_penInfo.GetEndY(),
                                m_penInfo.GetRadius(),
                                m_penInfo.GetStops(),
                                m_penInfo.GetMatrix());
        break;
    }
}


void wxD2DPenData::CreateStrokeStyle(ID2D1Factory* const direct2dfactory)
{
    const D2D1_CAP_STYLE capStyle = wxD2DConvertPenCap(m_penInfo.GetCap());
    const D2D1_LINE_JOIN lineJoin = wxD2DConvertPenJoin(m_penInfo.GetJoin());
    const D2D1_DASH_STYLE dashStyle = wxD2DConvertPenStyle(m_penInfo.GetStyle());

    int dashCount{m_penInfo.GetDashCount()};
    auto dashes = std::make_unique<FLOAT[]>(dashCount);

    if (dashStyle == D2D1_DASH_STYLE_CUSTOM)
    {
        for (int i = 0; i < dashCount; ++i)
        {
            dashes[i] = m_penInfo.GetDash()[i];
        }

    }

    direct2dfactory->CreateStrokeStyle(
        D2D1::StrokeStyleProperties(capStyle, capStyle, capStyle, lineJoin, 0, dashStyle, 0.0f),
        dashes.get(), dashCount,
        &m_strokeStyle);
}

void wxD2DPenData::SetWidth(const wxGraphicsContext* context)
{
    if (m_penInfo.GetWidth() <= 0)
    {
        const wxGraphicsMatrix matrix(context->GetTransform());
        double x = context->GetContentScaleFactor(), y = x;
        matrix.TransformDistance(&x, &y);
        m_width = gsl::narrow_cast<float>(1.0 / std::min(std::fabs(x), std::fabs(y)));
    }
}

ID2D1Brush* wxD2DPenData::GetBrush()
{
    return m_stippleBrush->GetBrush();
}

FLOAT wxD2DPenData::GetWidth()
{
    return m_width;
}

bool wxD2DPenData::IsZeroWidth() const
{
    return m_penInfo.GetWidth() <= 0.0;
}

ID2D1StrokeStyle* wxD2DPenData::GetStrokeStyle()
{
    return m_strokeStyle;
}

wxD2DPenData* wxGetD2DPenData(const wxGraphicsPen& pen)
{
    return dynamic_cast<wxD2DPenData*>(pen.GetGraphicsData());
}

class wxD2DFontData : public wxGraphicsObjectRefData
{
public:
    wxD2DFontData(wxGraphicsRenderer* renderer, const wxFont& font, const wxRealPoint& dpi, const wxColor& color);

    wxCOMPtr<IDWriteTextLayout> CreateTextLayout(std::string_view text) const;

    wxD2DBrushData& GetBrushData() { return m_brushData; }

    wxCOMPtr<IDWriteTextFormat> GetTextFormat() const { return m_textFormat; }

    wxCOMPtr<IDWriteFont> GetFont() { return m_font; }

private:
    // The native, device-independent font object
    wxCOMPtr<IDWriteFont> m_font;

    // The native, device-independent font object
    wxCOMPtr<IDWriteTextFormat> m_textFormat;

    // We use a color brush to render the font
    wxD2DBrushData m_brushData;

    bool m_underlined;

    bool m_strikethrough;
};

wxD2DFontData::wxD2DFontData(wxGraphicsRenderer* renderer, const wxFont& font, const wxRealPoint& dpi, const wxColor& color) :
    wxGraphicsObjectRefData(renderer), m_brushData(renderer, wxBrush(color)),
    m_underlined(font.GetUnderlined()), m_strikethrough(font.GetStrikethrough())
{
    wxCOMPtr<IDWriteGdiInterop> gdiInterop;
    HRESULT hr = wxDWriteFactory()->GetGdiInterop(&gdiInterop);
    wxCHECK_HRESULT_RET(hr);

    LOGFONTW logfont;
    int n = GetObjectW(font.GetHFONT(), sizeof(logfont), &logfont);
    wxCHECK_RET( n > 0, wxS("Failed to obtain font info") );

    // Ensure the LOGFONT object contains the correct font face name
    if (logfont.lfFaceName[0] == L'\0')
    {
        // The length of the font name must not exceed LF_FACESIZE TCHARs,
        // including the terminating NULL.
        std::wstring name = font.GetFaceName().substr(0, WXSIZEOF(logfont.lfFaceName)-1);
        for(std::size_t i{0}; auto&& ch : name)
        {
            logfont.lfFaceName[i] = ch;
            ++i;
        }

        logfont.lfFaceName[name.length()] = L'\0';
    }

    wxCOMPtr<IDWriteFontFamily> fontFamily;
    wxCOMPtr<IDWriteFontCollection> fontCollection; // NULL if font is taken from the system collection

    hr = gdiInterop->CreateFontFromLOGFONT(&logfont, &m_font);
    if ( hr == DWRITE_E_NOFONT )
    {
        // It was attempted to create DirectWrite font from non-TrueType GDI font
        // or from private GDI font.
#if wxUSE_PRIVATE_FONTS
        // Make private fonts available to D2D.
        const std::vector<std::string>& privateFonts = wxGetPrivateFontFileNames();
        if ( privateFonts.empty() )
        {
            wxLogApiError(wxString::Format("IDWriteGdiInterop::CreateFontFromLOGFONT() for '%s'", logfont.lfFaceName), hr);
            return;
        }
        // Update font collection if the list of private fonts has changed.
        if ( privateFonts != wxDirect2DFontCollectionLoader::GetFontList() )
        {
            wxDirect2DFontKey collectionKey = wxDirect2DFontCollectionLoader::SetFontList(privateFonts);

            gs_pPrivateFontCollection.reset();
            hr = wxDWriteFactory()->CreateCustomFontCollection(
                                        wxDirect2DFontCollectionLoader::GetLoader(),
                                        &collectionKey, sizeof(collectionKey),
                                        &gs_pPrivateFontCollection);
            wxCHECK_HRESULT_RET(hr);
        }
        wxCHECK_RET(gs_pPrivateFontCollection != nullptr, "No custom font collection created");

        UINT32 fontIdx{std::numeric_limits<std::uint32_t>::max()};
        BOOL fontFound{FALSE};
        hr = gs_pPrivateFontCollection->FindFamilyName(logfont.lfFaceName, &fontIdx, &fontFound);
        wxCHECK_HRESULT_RET(hr);

        if ( !fontFound )
        {
            wxFAIL_MSG(wxString::Format("Couldn't find custom font family '%s'", logfont.lfFaceName));
            return;
        }

        hr = gs_pPrivateFontCollection->GetFontFamily(fontIdx, &fontFamily);
        wxCHECK_HRESULT_RET(hr);

        // Even though DWRITE_FONT_WEIGHT is an enum, it's values are within the same range
        // as font width values in LOGFONT (0-1000) so we can cast LONG to this enum.
        const DWRITE_FONT_WEIGHT fWeight = static_cast<DWRITE_FONT_WEIGHT>(logfont.lfWeight);

        DWRITE_FONT_STYLE fStyle;
        if ( logfont.lfItalic == TRUE )
            fStyle = DWRITE_FONT_STYLE_ITALIC;
        else
            fStyle = DWRITE_FONT_STYLE_NORMAL;

        DWRITE_FONT_STRETCH fStretch = DWRITE_FONT_STRETCH_NORMAL;

        hr = fontFamily->GetFirstMatchingFont(fWeight, fStretch, fStyle, &m_font);
        wxCHECK_RET(SUCCEEDED(hr),
            wxString::Format("Failed to find custom font '%s' (HRESULT = %x)", logfont.lfFaceName, hr));

        fontCollection = gs_pPrivateFontCollection;
#else
        return;
#endif // wxUSE_PRIVATE_FONTS
    }
    else
    {
        wxCHECK_RET(SUCCEEDED(hr),
            wxString::Format("Failed to create font '%s' (HRESULT = %x)", logfont.lfFaceName, hr));

        hr = m_font->GetFontFamily(&fontFamily);
        wxCHECK_HRESULT_RET(hr);
    }

    wxCOMPtr<IDWriteLocalizedStrings> familyNames;
    hr = fontFamily->GetFamilyNames(&familyNames);
    wxCHECK_HRESULT_RET(hr);

    UINT32 length;
    hr = familyNames->GetStringLength(0, &length);
    wxCHECK_HRESULT_RET(hr);

    auto name = std::make_unique<wchar_t[]>(length + 1);
    hr = familyNames->GetString(0, name.get(), length + 1);
    wxCHECK_HRESULT_RET(hr);

    FLOAT fontSize = !dpi.y
        ? FLOAT(font.GetPixelSize().y)
        : FLOAT(font.GetFractionalPointSize() * dpi.y / 72);

    hr = wxDWriteFactory()->CreateTextFormat(
        name.get(),
        fontCollection,
        m_font->GetWeight(),
        m_font->GetStyle(),
        m_font->GetStretch(),
        fontSize,
        L"en-us",
        &m_textFormat);

    wxCHECK_HRESULT_RET(hr);
}

wxCOMPtr<IDWriteTextLayout> wxD2DFontData::CreateTextLayout(std::string_view text) const
{
    static constexpr FLOAT MAX_WIDTH  = std::numeric_limits<float>::max();
    static constexpr FLOAT MAX_HEIGHT = std::numeric_limits<float>::max();

    HRESULT hr;

    wxCOMPtr<IDWriteTextLayout> textLayout;

    hr = wxDWriteFactory()->CreateTextLayout(
        boost::nowide::widen(text).c_str(),
        text.length(),
        m_textFormat,
        MAX_WIDTH,
        MAX_HEIGHT,
        &textLayout);
    wxCHECK2_HRESULT_RET(hr, wxCOMPtr<IDWriteTextLayout>(nullptr));

    DWRITE_TEXT_RANGE textRange = { 0, (UINT32) text.length() };

    if (m_underlined)
    {
        textLayout->SetUnderline(true, textRange);
    }

    if (m_strikethrough)
    {
        textLayout->SetStrikethrough(true, textRange);
    }

    return textLayout;
}

wxD2DFontData* wxGetD2DFontData(const wxGraphicsFont& font)
{
    return dynamic_cast<wxD2DFontData*>(font.GetGraphicsData());
}

// A render target resource holder exposes methods relevant
// for native render targets such as resize
class wxD2DRenderTargetResourceHolder : public wxD2DResourceHolder<ID2D1RenderTarget>
{
public:
    // This method is called when an external event signals the underlying DC
    // is resized (e.g. the resizing of a window). Some implementations can leave
    // this method empty, while others must adjust the render target size to match
    // the underlying DC.
    virtual void Resize()
    {
    }

    // We use this method instead of the one provided by the native render target
    // because Direct2D 1.0 render targets do not accept a composition mode
    // parameter, while the device context in Direct2D 1.1 does. This way, we make
    // best use of the capabilities of each render target.
    //
    // The default implementation works for all render targets, but the D2D 1.0
    // render target holders shouldn't need to override it, since none of the
    // 1.0 render targets offer a better version of this method.
    virtual void DrawBitmap(ID2D1Bitmap* bitmap,
        const D2D1_RECT_F& srcRect, const D2D1_RECT_F& destRect,
        wxInterpolationQuality interpolationQuality,
        wxCompositionMode WXUNUSED(compositionMode))
    {
        m_nativeResource->DrawBitmap(
            bitmap,
            destRect,
            1.0f,
            wxD2DConvertBitmapInterpolationMode(interpolationQuality),
            srcRect);
    }

    // We use this method instead of the one provided by the native render target
    // because some contexts might require writing to a buffer (e.g. an image
    // context), and some render targets might require additional operations to
    // be executed (e.g. the device context must present the swap chain)
    virtual HRESULT Flush()
    {
        return m_nativeResource->Flush();
    }

    // Composition is not supported at in D2D 1.0, and we only allow for:
    // wxCOMPOSITION_DEST - which is essentially a no-op and is handled
    //                      externally by preventing any draw calls.
    // wxCOMPOSITION_OVER - which copies the source over the destination using
    //                      alpha blending. This is the default way D2D 1.0
    //                      draws images.
    virtual bool SetCompositionMode(wxCompositionMode compositionMode)
    {
        return compositionMode == wxCOMPOSITION_DEST || compositionMode == wxCOMPOSITION_OVER;
    }
};

#if wxUSE_IMAGE
class wxD2DImageRenderTargetResourceHolder : public wxD2DRenderTargetResourceHolder
{
public:
    wxD2DImageRenderTargetResourceHolder(wxImage* image, ID2D1Factory* factory) :
        m_resultImage(image), m_factory(factory)
    {
    }

    HRESULT Flush() override
    {
        HRESULT hr = m_nativeResource->Flush();
        FlushRenderTargetToImage();
        return hr;
    }

    ~wxD2DImageRenderTargetResourceHolder() override
    {
        FlushRenderTargetToImage();
    }

protected:
    void DoAcquireResource() override
    {
        CreateWICBitmapFromImage(*m_resultImage, true, &m_wicBitmap);

        // Create the render target
        HRESULT hr = m_factory->CreateWicBitmapRenderTarget(
            m_wicBitmap,
            D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_SOFTWARE,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
            &m_nativeResource);
        wxCHECK_HRESULT_RET(hr);
    }

private:
    void FlushRenderTargetToImage()
    {
        CreateImageFromWICBitmap(m_wicBitmap, m_resultImage);
   }

    wxImage* m_resultImage;
    wxCOMPtr<IWICBitmap> m_wicBitmap;

    ID2D1Factory* m_factory;
};
#endif // wxUSE_IMAGE

class wxD2DHwndRenderTargetResourceHolder : public wxD2DRenderTargetResourceHolder
{
public:
    using ImplementationType = ID2D1HwndRenderTarget*;

    wxD2DHwndRenderTargetResourceHolder(HWND hwnd, ID2D1Factory* factory) :
        m_hwnd(hwnd), m_factory(factory)
    {
    }

    void Resize() override
    {
        const RECT clientRect = wxGetClientRect(m_hwnd);

        const D2D1_SIZE_U hwndSize = D2D1::SizeU(
            clientRect.right - clientRect.left,
            clientRect.bottom - clientRect.top);

        const D2D1_SIZE_U renderTargetSize = GetRenderTarget()->GetPixelSize();

        if (hwndSize.width != renderTargetSize.width || hwndSize.height != renderTargetSize.height)
        {
            GetRenderTarget()->Resize(hwndSize);
        }
    }

protected:
    void DoAcquireResource() override
    {
        wxCOMPtr<ID2D1HwndRenderTarget> renderTarget;

        const RECT clientRect = wxGetClientRect(m_hwnd);

        const D2D1_SIZE_U size = D2D1::SizeU(
            clientRect.right - clientRect.left,
            clientRect.bottom - clientRect.top);

        HRESULT result = m_factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &renderTarget);

        if (FAILED(result))
        {
            wxFAIL_MSG("Could not create Direct2D render target");
        }

        renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        m_nativeResource = renderTarget;
    }

private:
    // Converts the underlying resource pointer of type
    // ID2D1RenderTarget* to the actual implementation type
    ImplementationType GetRenderTarget()
    {
        return dynamic_cast<ImplementationType>(GetD2DResource().get());
    }

    HWND m_hwnd;
    ID2D1Factory* m_factory;
};

#if wxD2D_DEVICE_CONTEXT_SUPPORTED
class wxD2DDeviceContextResourceHolder : public wxD2DRenderTargetResourceHolder
{
public:
    wxD2DDeviceContextResourceHolder(ID2D1Factory* factory, HWND hwnd) :
        m_hwnd(hwnd)
    {
        HRESULT hr = factory->QueryInterface(IID_ID2D1Factory1, reinterpret_cast<void**>(&m_factory));
        wxCHECK_HRESULT_RET(hr);
    }

    void DrawBitmap(ID2D1Bitmap* bitmap,
        const D2D1_RECT_F& srcRect, const D2D1_RECT_F& destRect,
        wxInterpolationQuality interpolationQuality,
        wxCompositionMode compositionMode) override
    {
        D2D1_POINT_2F offset = D2D1::Point2(destRect.left, destRect.top);
        m_context->DrawImage(bitmap,
            offset,
            srcRect,
            wxD2DConvertInterpolationMode(interpolationQuality),
            wxD2DConvertCompositionMode(compositionMode));
    }

    HRESULT Flush() override
    {
        HRESULT hr = m_nativeResource->Flush();
        DXGI_PRESENT_PARAMETERS params = { 0 };
        m_swapChain->Present1(1, 0, &params);
        return hr;
    }

protected:

    // Adapted from http://msdn.microsoft.com/en-us/library/windows/desktop/hh780339%28v=vs.85%29.aspx
    void DoAcquireResource() override
    {
        // This flag adds support for surfaces with a different color channel ordering than the API default.
        // You need it for compatibility with Direct2D.
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        // This array defines the set of DirectX hardware feature levels this app  supports.
        // The ordering is important and you should  preserve it.
        // Don't forget to declare your app's minimum required feature level in its
        // description.  All apps are assumed to support 9.1 unless otherwise stated.
        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_3,
            D3D_FEATURE_LEVEL_11_2,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        // Create the DX11 API device object, and get a corresponding context.
        wxCOMPtr<ID3D11Device> device;
        wxCOMPtr<ID3D11DeviceContext> context;

        HRESULT hr = D3D11CreateDevice(
            NULL,                    // specify null to use the default adapter
            D3D_DRIVER_TYPE_HARDWARE,
            0,
            creationFlags,              // optionally set debug and Direct2D compatibility flags
            featureLevels,              // list of feature levels this app can support
            ARRAYSIZE(featureLevels),   // number of possible feature levels
            D3D11_SDK_VERSION,
            &device,                    // returns the Direct3D device created
            &m_featureLevel,            // returns feature level of device created
            &context);                  // returns the device immediate context
        wxCHECK_HRESULT_RET(hr);

        // Obtain the underlying DXGI device of the Direct3D11 device.
        hr = device->QueryInterface(IID_IDXGIDevice, (void**)&m_dxgiDevice);
        wxCHECK_HRESULT_RET(hr);

        // Obtain the Direct2D device for 2-D rendering.
        hr = m_factory->CreateDevice(m_dxgiDevice, &m_device);
        wxCHECK_HRESULT_RET(hr);

        // Get Direct2D device's corresponding device context object.
        hr = m_device->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_context);
        wxCHECK_HRESULT_RET(hr);

        m_nativeResource = m_context;

        AttachSurface();
    }

private:
    void AttachSurface()
    {
        // Allocate a descriptor.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
        swapChainDesc.Width = 0;
        swapChainDesc.Height = 0;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.Flags = 0;

        // Identify the physical adapter (GPU or card) this device is runs on.
        wxCOMPtr<IDXGIAdapter> dxgiAdapter;
        HRESULT hr = m_dxgiDevice->GetAdapter(&dxgiAdapter);
        wxCHECK_HRESULT_RET(hr);

        // Get the factory object that created the DXGI device.
        wxCOMPtr<IDXGIFactory2> dxgiFactory;
        hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
        wxCHECK_HRESULT_RET(hr);

        // Get the final swap chain for this window from the DXGI factory.
        hr = dxgiFactory->CreateSwapChainForHwnd(
            m_dxgiDevice,
            m_hwnd,
            &swapChainDesc,
            NULL,    // allow on all displays
            NULL,
            &m_swapChain);
        wxCHECK_HRESULT_RET(hr);

        // Ensure that DXGI doesn't queue more than one frame at a time.
        hr = m_dxgiDevice->SetMaximumFrameLatency(1);
        wxCHECK_HRESULT_RET(hr);

        // Get the backbuffer for this window which is be the final 3D render target.
        wxCOMPtr<ID3D11Texture2D> backBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        wxCHECK_HRESULT_RET(hr);

        FLOAT dpiX{static_cast<FLOAT>(::GetDpiForWindow(m_hwnd))};
        FLOAT dpiY{dpiX};

        // Now we set up the Direct2D render target bitmap linked to the swapchain.
        // Whenever we render to this bitmap, it is directly rendered to the
        // swap chain associated with the window.
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
            dpiX, dpiY);

        // Direct2D needs the dxgi version of the backbuffer surface pointer.
        wxCOMPtr<IDXGISurface> dxgiBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
        wxCHECK_HRESULT_RET(hr);

        // Get a D2D surface from the DXGI back buffer to use as the D2D render target.
        hr = m_context->CreateBitmapFromDxgiSurface(
            dxgiBackBuffer.get(),
            &bitmapProperties,
            &m_targetBitmap);
        wxCHECK_HRESULT_RET(hr);

        // Now we can set the Direct2D render target.
        m_context->SetTarget(m_targetBitmap);
    }

    ~wxD2DDeviceContextResourceHolder()
    {
        DXGI_PRESENT_PARAMETERS params{0};
        m_swapChain->Present1(1, 0, &params);
    }

private:
    wxCOMPtr<ID2D1Factory1> m_factory;

    HWND m_hwnd;

    D3D_FEATURE_LEVEL m_featureLevel;
    wxCOMPtr<IDXGIDevice1> m_dxgiDevice;
    wxCOMPtr<ID2D1Device> m_device;
    wxCOMPtr<ID2D1DeviceContext> m_context;
    wxCOMPtr<ID2D1Bitmap1> m_targetBitmap;
    wxCOMPtr<IDXGISwapChain1> m_swapChain;
};
#endif

class wxD2DDCRenderTargetResourceHolder : public wxD2DRenderTargetResourceHolder
{
public:
    wxD2DDCRenderTargetResourceHolder(ID2D1Factory* factory, HDC hdc, D2D1_ALPHA_MODE alphaMode) :
        m_factory(factory), m_hdc(hdc), m_alphaMode(alphaMode)
    {
    }

protected:
    void DoAcquireResource() override
    {
        wxCOMPtr<ID2D1DCRenderTarget> renderTarget;
        D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, m_alphaMode));

        HRESULT hr = m_factory->CreateDCRenderTarget(
            &renderTargetProperties,
            &renderTarget);
        wxCHECK_HRESULT_RET(hr);

        // We want draw on the entire device area.
        // GetClipBox() retrieves logical size of DC
        // what is what we need to pass to BindDC.
        RECT r;
        int status = ::GetClipBox(m_hdc, &r);
        wxCHECK_RET( status != ERROR, wxS("Error retrieving DC dimensions") );

        hr = renderTarget->BindDC(m_hdc, &r);
        wxCHECK_HRESULT_RET(hr);
        renderTarget->SetTransform(
                       D2D1::Matrix3x2F::Translation(gsl::narrow_cast<float>(-r.left), gsl::narrow_cast<float>(-r.top)));

        m_nativeResource = renderTarget;
    }

private:
    ID2D1Factory* m_factory;
    HDC m_hdc;
    D2D1_ALPHA_MODE m_alphaMode;
};

// The null context has no state of its own and does nothing.
// It is only used as a base class for the lightweight
// measuring context. The measuring context cannot inherit from
// the default implementation wxD2DContext, because some methods
// from wxD2DContext require the presence of a "context"
// (render target) in order to acquire various device-dependent
// resources. Without a proper context, those methods would fail.
// The methods implemented in the null context are fundamentally no-ops.
class wxNullContext : public wxGraphicsContext
{
public:
    explicit wxNullContext(wxGraphicsRenderer* renderer) : wxGraphicsContext(renderer) {}
    std::pair<double, double> GetTextExtent(std::string_view, double*, double*) const override { return {}; }
    std::vector<double> GetPartialTextExtents(std::string_view) const override { return {}; }
    void Clip(const wxRegion&) override {}
    void Clip(double, double, double, double) override {}
    void ResetClip() override {}
    void GetClipBox(double*, double*, double*, double*) override {}
    void* GetNativeContext() override { return nullptr; }
    bool SetAntialiasMode(wxAntialiasMode) override { return false; }
    bool SetInterpolationQuality(wxInterpolationQuality) override { return false; }
    bool SetCompositionMode(wxCompositionMode) override { return false; }
    void BeginLayer(double) override {}
    void EndLayer() override {}
    void Translate(double, double) override {}
    void Scale(double, double) override {}
    void Rotate(double) override {}
    void ConcatTransform(const wxGraphicsMatrix&) override {}
    void SetTransform(const wxGraphicsMatrix&) override {}
    wxGraphicsMatrix GetTransform() const override { return wxNullGraphicsMatrix; }
    void StrokePath(const wxGraphicsPath&) override {}
    void FillPath(const wxGraphicsPath&, wxPolygonFillMode) override {}
    void DrawBitmap(const wxGraphicsBitmap&, double, double, double, double) override {}
    void DrawBitmap(const wxBitmap&, double, double, double, double) override {}
    void DrawIcon(const wxIcon&, double, double, double, double) override {}
    void PushState() override {}
    void PopState() override {}
    void Flush() override {}

protected:
    void DoDrawText(std::string_view, double, double) override {}
};

class wxD2DMeasuringContext : public wxNullContext
{
public:
    explicit wxD2DMeasuringContext(wxGraphicsRenderer* renderer) : wxNullContext(renderer) {}

    std::pair<double, double> GetTextExtent(std::string_view str, double* descent, double* externalLeading) const override
    {
        return GetTextExtent(wxGetD2DFontData(m_font), str, descent, externalLeading);
    }

    std::vector<double> GetPartialTextExtents(std::string_view text) const override
    {
        return GetPartialTextExtents(wxGetD2DFontData(m_font), text);
    }

    static std::vector<double> GetPartialTextExtents(wxD2DFontData* fontData, std::string_view text)
    {
        std::vector<double> widths;

        for (unsigned int i = 0; i < text.length(); ++i)
        {
            auto width = GetTextExtent(fontData, text.substr(0, i), nullptr, nullptr).first;
            widths.push_back(width);
        }

        return widths;
    }

    static std::pair<double, double> GetTextExtent(wxD2DFontData* fontData, std::string_view str, double* descent, double* externalLeading)
    {
        wxCOMPtr<IDWriteTextLayout> textLayout = fontData->CreateTextLayout(str);
        wxCOMPtr<IDWriteFont> font = fontData->GetFont();

        DWRITE_TEXT_METRICS textMetrics;
        textLayout->GetMetrics(&textMetrics);

        DWRITE_FONT_METRICS fontMetrics;
        font->GetMetrics(&fontMetrics);

        FLOAT ratio = fontData->GetTextFormat()->GetFontSize() / (FLOAT)fontMetrics.designUnitsPerEm;

        if (descent != nullptr) *descent = fontMetrics.descent * ratio;
        if (externalLeading != nullptr) *externalLeading = std::max(0.0f, (fontMetrics.ascent + fontMetrics.descent) * ratio - textMetrics.height);

        return { textMetrics.widthIncludingTrailingWhitespace, textMetrics.height };
    }
};

//-----------------------------------------------------------------------------
// wxD2DContext declaration
//-----------------------------------------------------------------------------

class wxD2DContext : public wxGraphicsContext, wxD2DResourceManager
{
public:
    // Create the context for the given HWND, which may be associated (if it's
    // non-null) with the given wxWindow.
    wxD2DContext(wxGraphicsRenderer* renderer,
                 ID2D1Factory* direct2dFactory,
                 HWND hwnd,
                 wxWindow* window = nullptr);

    // Create the context for the given HDC which may be associated (if it's
    // non-null) with the given wxDC.
    wxD2DContext(wxGraphicsRenderer* renderer,
                 ID2D1Factory* direct2dFactory,
                 HDC hdc,
                 const wxDC* dc = nullptr,
                 D2D1_ALPHA_MODE alphaMode = D2D1_ALPHA_MODE_IGNORE);

#if wxUSE_IMAGE
    wxD2DContext(wxGraphicsRenderer* renderer, ID2D1Factory* direct2dFactory, wxImage& image);
#endif // wxUSE_IMAGE

    wxD2DContext(wxGraphicsRenderer* renderer, ID2D1Factory* direct2dFactory, void* nativeContext);

    ~wxD2DContext() override;

    wxD2DContext(const wxD2DContext&) = delete;
	wxD2DContext& operator=(const wxD2DContext&) = delete;

    void Clip(const wxRegion& region) override;

    void Clip(double x, double y, double w, double h) override;

    void ResetClip() override;

    void GetClipBox(double* x, double* y, double* w, double* h) override;

    // The native context used by wxD2DContext is a Direct2D render target.
    void* GetNativeContext() override;

    bool SetAntialiasMode(wxAntialiasMode antialias) override;

    bool SetInterpolationQuality(wxInterpolationQuality interpolation) override;

    bool SetCompositionMode(wxCompositionMode op) override;

    void BeginLayer(double opacity) override;

    void EndLayer() override;

    void Translate(double dx, double dy) override;

    void Scale(double xScale, double yScale) override;

    void Rotate(double angle) override;

    void ConcatTransform(const wxGraphicsMatrix& matrix) override;

    void SetTransform(const wxGraphicsMatrix& matrix) override;

    wxGraphicsMatrix GetTransform() const override;

    void StrokePath(const wxGraphicsPath& p) override;

    void FillPath(const wxGraphicsPath& p , wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven) override;

    void DrawRectangle(double x, double y, double w, double h) override;

    void DrawRoundedRectangle(double x, double y, double w, double h, double radius) override;

    void DrawEllipse(double x, double y, double w, double h) override;

    void DrawBitmap(const wxGraphicsBitmap& bmp, double x, double y, double w, double h) override;

    void DrawBitmap(const wxBitmap& bmp, double x, double y, double w, double h) override;

    void DrawIcon(const wxIcon& icon, double x, double y, double w, double h) override;

    void PushState() override;

    void PopState() override;

    std::pair<double, double> GetTextExtent(
        std::string_view str,
        double* descent,
        double* externalLeading) const override;

    std::vector<double> GetPartialTextExtents(std::string_view text) const override;

    bool ShouldOffset() const override;

    void SetPen(const wxGraphicsPen& pen) override;

    void Flush() override;

    void GetDPI(double* dpiX, double* dpiY) const override;

    wxD2DContextSupplier::ContextType GetContext() override
    {
        return GetRenderTarget();
    }

private:
    void DoDrawText(std::string_view str, double x, double y) override;

    void EnsureInitialized();

    HRESULT CreateRenderTarget();

    void AdjustRenderTargetSize();

    void ReleaseDeviceDependentResources();

    ID2D1RenderTarget* GetRenderTarget() const;

    void SetClipLayer(ID2D1Geometry* clipGeometry);

    enum LayerType
    {
        CLIP_LAYER,
        OTHER_LAYER
    };

    struct LayerData
    {
        LayerType type{CLIP_LAYER};
        D2D1_LAYER_PARAMETERS params{};
        wxCOMPtr<ID2D1Layer> layer;
        wxCOMPtr<ID2D1Geometry> geometry;
        D2D1_MATRIX_3X2_F transformMatrix{};
    };

    struct StateData
    {
        // A ID2D1DrawingStateBlock represents the drawing state of a render target:
        // the anti aliasing mode, transform, tags, and text-rendering options.
        // The context owns these pointers and is responsible for releasing them.
        wxCOMPtr<ID2D1DrawingStateBlock> drawingState;
        // We need to store also current layers.
        std::stack<LayerData> layers;
    };

    ID2D1Factory* m_direct2dFactory;

    std::shared_ptr<wxD2DRenderTargetResourceHolder> m_renderTargetHolder;
    std::stack<StateData> m_stateStack;
    std::stack<LayerData> m_layers;

    ID2D1RenderTarget* m_cachedRenderTarget{nullptr};
    D2D1::Matrix3x2F m_initTransform;

    // Clipping box
    bool m_isClipBoxValid{false};

    double m_clipX1{0.0};
    double m_clipY1{0.0};
    double m_clipX2{0.0};
    double m_clipY2{0.0};
};

//-----------------------------------------------------------------------------
// wxD2DContext implementation
//-----------------------------------------------------------------------------

wxD2DContext::wxD2DContext(wxGraphicsRenderer* renderer,
                           ID2D1Factory* direct2dFactory,
                           HWND hwnd,
                           wxWindow* window) :
    wxGraphicsContext(renderer, window), m_direct2dFactory(direct2dFactory),
#if wxD2D_DEVICE_CONTEXT_SUPPORTED
    m_renderTargetHolder(new wxD2DDeviceContextResourceHolder(direct2dFactory, hwnd))
#else
    m_renderTargetHolder(new wxD2DHwndRenderTargetResourceHolder(hwnd, direct2dFactory))
#endif
{
    RECT r = wxGetWindowRect(hwnd);
    m_width = r.right - r.left;
    m_height = r.bottom - r.top;
    
    m_renderTargetHolder->Bind(this);
    EnsureInitialized();
}

wxD2DContext::wxD2DContext(wxGraphicsRenderer* renderer,
                           ID2D1Factory* direct2dFactory,
                           HDC hdc,
                           const wxDC* dc,
                           D2D1_ALPHA_MODE alphaMode) :
    wxGraphicsContext(renderer, dc->GetWindow()), m_direct2dFactory(direct2dFactory),
    m_renderTargetHolder(new wxD2DDCRenderTargetResourceHolder(direct2dFactory, hdc, alphaMode))
{
    if ( dc )
    {
        const wxSize dcSize = dc->GetSize();
        m_width = dcSize.x;
        m_height = dcSize.y;
    }

    m_renderTargetHolder->Bind(this);
    EnsureInitialized();

}

#if wxUSE_IMAGE
wxD2DContext::wxD2DContext(wxGraphicsRenderer* renderer, ID2D1Factory* direct2dFactory, wxImage& image) :
    wxGraphicsContext(renderer), m_direct2dFactory(direct2dFactory),
    m_renderTargetHolder(new wxD2DImageRenderTargetResourceHolder(&image, direct2dFactory))
{
    m_width = image.GetWidth();
    m_height = image.GetHeight();
    
    m_renderTargetHolder->Bind(this);
    EnsureInitialized();

}
#endif // wxUSE_IMAGE

wxD2DContext::wxD2DContext(wxGraphicsRenderer* renderer, ID2D1Factory* direct2dFactory, void* nativeContext) :
    wxGraphicsContext(renderer), m_direct2dFactory(direct2dFactory)
{
    m_renderTargetHolder = *((std::shared_ptr<wxD2DRenderTargetResourceHolder>*)nativeContext);
    
    m_renderTargetHolder->Bind(this);
    EnsureInitialized();
}



wxD2DContext::~wxD2DContext()
{
    // Remove all layers from the stack of layers.
    while ( !m_layers.empty() )
    {
        LayerData ld = m_layers.top();
        m_layers.pop();

        GetRenderTarget()->PopLayer();
        ld.layer.reset();
        ld.geometry.reset();
    }

    HRESULT result = GetRenderTarget()->EndDraw();
    wxCHECK_HRESULT_RET(result);

    ReleaseResources();
}

ID2D1RenderTarget* wxD2DContext::GetRenderTarget() const
{
    return m_cachedRenderTarget;
}

void wxD2DContext::Clip(const wxRegion& region)
{
    wxCOMPtr<ID2D1Geometry> clipGeometry = wxD2DConvertRegionToGeometry(m_direct2dFactory, region);

    SetClipLayer(clipGeometry);
}

void wxD2DContext::Clip(double x, double y, double w, double h)
{
    wxCOMPtr<ID2D1RectangleGeometry> clipGeometry;
    HRESULT hr = m_direct2dFactory->CreateRectangleGeometry(
                        D2D1::RectF(gsl::narrow_cast<float>(x),
                                    gsl::narrow_cast<float>(y),
                                    gsl::narrow_cast<float>(x + w),
                                    gsl::narrow_cast<float>(y + h)), &clipGeometry);
    wxCHECK_HRESULT_RET(hr);

    SetClipLayer(clipGeometry);
}

void wxD2DContext::SetClipLayer(ID2D1Geometry* clipGeometry)
{
    EnsureInitialized();

    wxCOMPtr<ID2D1Layer> clipLayer;
    HRESULT hr = GetRenderTarget()->CreateLayer(&clipLayer);
    wxCHECK_HRESULT_RET(hr);

    LayerData ld;
    ld.type = CLIP_LAYER;
    ld.params = D2D1::LayerParameters(D2D1::InfiniteRect(), clipGeometry, GetRenderTarget()->GetAntialiasMode());
    ld.layer = clipLayer;
    ld.geometry = clipGeometry;
    // We need to store CTM to be able to re-apply
    // the layer at the original position later on.
    GetRenderTarget()->GetTransform(&ld.transformMatrix);

    GetRenderTarget()->PushLayer(ld.params, clipLayer);
    // Store layer parameters.
    m_layers.push(ld);

    m_isClipBoxValid = false;
}

void wxD2DContext::ResetClip()
{
    std::stack<LayerData> layersToRestore;
    // Remove all clipping layers from the stack of layers.
    while ( !m_layers.empty() )
    {
        LayerData ld = m_layers.top();
        m_layers.pop();

        if ( ld.type == CLIP_LAYER )
        {
            GetRenderTarget()->PopLayer();
            ld.layer.reset();
            ld.geometry.reset();
            continue;
        }

        GetRenderTarget()->PopLayer();
        // Save non-clipping layer
        layersToRestore.push(ld);
    }

    HRESULT hr = GetRenderTarget()->Flush();
    wxCHECK_HRESULT_RET(hr);

    // Re-apply all remaining non-clipping layers.
    // First, save current transformation matrix.
    D2D1_MATRIX_3X2_F currTransform;
    GetRenderTarget()->GetTransform(&currTransform);
    while ( !layersToRestore.empty() )
    {
        LayerData ld = layersToRestore.top();
        layersToRestore.pop();

        // Restore layer at original position.
        GetRenderTarget()->SetTransform(&ld.transformMatrix);
        GetRenderTarget()->PushLayer(ld.params, ld.layer);
        // Store layer parameters.
        m_layers.push(ld);
    }
    // Restore current transformation matrix.
    GetRenderTarget()->SetTransform(&currTransform);

    m_isClipBoxValid = false;
}

void wxD2DContext::GetClipBox(double* x, double* y, double* w, double* h)
{
    if ( !m_isClipBoxValid )
    {
        // To obtain actual clipping box we have to start with rectangle
        // covering the entire render target and interesect with this rectangle
        // all clipping layers. Bounding box of the final geometry
        // (being intersection of all clipping layers) is a clipping box.

        wxCOMPtr<ID2D1RectangleGeometry> rectGeometry;
        HRESULT hr = m_direct2dFactory->CreateRectangleGeometry(
                        D2D1::RectF(0.0F, 0.0F, (FLOAT)m_width, (FLOAT)m_height),
                        &rectGeometry);
        wxCHECK_HRESULT_RET(hr);

        wxCOMPtr<ID2D1Geometry> clipGeometry(rectGeometry);

        std::stack<LayerData> layers(m_layers);

        while( !layers.empty() )
        {
            LayerData ld = layers.top();
            layers.pop();

            if ( ld.type == CLIP_LAYER )
            {
                // If current geometry is empty (null region)
                // or there is no intersection between geometries
                // then final result is "null" rectangle geometry.
                FLOAT area;
                hr = ld.geometry->ComputeArea(ld.transformMatrix, &area);
                wxCHECK_HRESULT_RET(hr);
                D2D1_GEOMETRY_RELATION geomRel;
                hr = clipGeometry->CompareWithGeometry(ld.geometry, ld.transformMatrix, &geomRel);
                wxCHECK_HRESULT_RET(hr);
                if ( area <= std::numeric_limits<float>::min() || geomRel == D2D1_GEOMETRY_RELATION_DISJOINT )
                {
                    wxCOMPtr<ID2D1RectangleGeometry> nullGeometry;
                    hr = m_direct2dFactory->CreateRectangleGeometry(
                                D2D1::RectF(0.0F, 0.0F, 0.0F, 0.0F), &nullGeometry);
                    wxCHECK_HRESULT_RET(hr);

                    clipGeometry.reset();
                    clipGeometry = nullGeometry;
                    break;
                }

                wxCOMPtr<ID2D1PathGeometry> pathGeometryClip;
                hr = m_direct2dFactory->CreatePathGeometry(&pathGeometryClip);
                wxCHECK_HRESULT_RET(hr);
                wxCOMPtr<ID2D1GeometrySink> pGeometrySink;
                hr = pathGeometryClip->Open(&pGeometrySink);
                wxCHECK_HRESULT_RET(hr);

                hr = clipGeometry->CombineWithGeometry(ld.geometry, D2D1_COMBINE_MODE_INTERSECT,
                                                       ld.transformMatrix, pGeometrySink);
                wxCHECK_HRESULT_RET(hr);
                hr = pGeometrySink->Close();
                wxCHECK_HRESULT_RET(hr);
                pGeometrySink.reset();

                clipGeometry = pathGeometryClip;
                pathGeometryClip.reset();
            }
        }

        // Final clipping geometry is given in device coordinates
        // so we need to transform its bounds to logical coordinates.
        D2D1::Matrix3x2F currTransform;
        GetRenderTarget()->GetTransform(&currTransform);
        currTransform.Invert();

        D2D1_RECT_F bounds;
        // First check if clip region is empty.
        FLOAT clipArea;
        hr = clipGeometry->ComputeArea(currTransform, &clipArea);
        wxCHECK_HRESULT_RET(hr);

        if ( clipArea <= std::numeric_limits<float>::min() )
        {
            bounds.left = bounds.top = bounds.right = bounds.bottom = 0.0F;
        }
        else
        {
            // If it is not empty then get it bounds.
            hr = clipGeometry->GetBounds(currTransform, &bounds);
            wxCHECK_HRESULT_RET(hr);
        }

        m_clipX1 = bounds.left;
        m_clipY1 = bounds.top;
        m_clipX2 = bounds.right;
        m_clipY2 = bounds.bottom;
        m_isClipBoxValid = true;
    }

    if ( x )
        *x = m_clipX1;
    if ( y )
        *y = m_clipY1;
    if ( w )
        *w = m_clipX2 - m_clipX1;
    if ( h )
        *h = m_clipY2 - m_clipY1;
}

void* wxD2DContext::GetNativeContext()
{
    return &m_renderTargetHolder;
}

void wxD2DContext::StrokePath(const wxGraphicsPath& p)
{
    if (m_composition == wxCOMPOSITION_DEST)
        return;

    wxD2DOffsetHelper helper(this);

    EnsureInitialized();
    AdjustRenderTargetSize();

    wxD2DPathData* pathData = wxGetD2DPathData(p);
    pathData->Flush();

    if (!m_pen.IsNull())
    {
        wxD2DPenData* penData = wxGetD2DPenData(m_pen);
        penData->SetWidth(this);
        penData->Bind(this);
        ID2D1Brush* nativeBrush = penData->GetBrush();
        GetRenderTarget()->DrawGeometry((ID2D1Geometry*)pathData->GetNativePath(), nativeBrush, penData->GetWidth(), penData->GetStrokeStyle());
    }
}

void wxD2DContext::FillPath(const wxGraphicsPath& p , wxPolygonFillMode fillStyle)
{
    if (m_composition == wxCOMPOSITION_DEST)
        return;

    EnsureInitialized();
    AdjustRenderTargetSize();

    wxD2DPathData* pathData = wxGetD2DPathData(p);
    pathData->SetFillMode(fillStyle == wxPolygonFillMode::OddEven ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING);
    pathData->Flush();

    if (!m_brush.IsNull())
    {
        wxD2DBrushData* brushData = wxGetD2DBrushData(m_brush);
        brushData->Bind(this);
        GetRenderTarget()->FillGeometry((ID2D1Geometry*)pathData->GetNativePath(), brushData->GetBrush());
    }
}

bool wxD2DContext::SetAntialiasMode(wxAntialiasMode antialias)
{
    if (m_antialias == antialias)
    {
        return true;
    }

    D2D1_ANTIALIAS_MODE antialiasMode;
    D2D1_TEXT_ANTIALIAS_MODE textAntialiasMode;

    switch ( antialias )
    {
    case wxAntialiasMode::Default:
        antialiasMode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
        textAntialiasMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
        break;

    case wxAntialiasMode::None:
        antialiasMode = D2D1_ANTIALIAS_MODE_ALIASED;
        textAntialiasMode = D2D1_TEXT_ANTIALIAS_MODE_ALIASED;
        break;

    default:
        wxFAIL_MSG("Unknown antialias mode");
        return false;
    }

    GetRenderTarget()->SetAntialiasMode(antialiasMode);
    GetRenderTarget()->SetTextAntialiasMode(textAntialiasMode);

    m_antialias = antialias;
    return true;
}

bool wxD2DContext::SetInterpolationQuality(wxInterpolationQuality interpolation)
{
    // Since different versions of Direct2D have different enumerations for
    // interpolation quality, we deffer the conversion to the method which
    // does the actual drawing.

    m_interpolation = interpolation;
    return true;
}

bool wxD2DContext::SetCompositionMode(wxCompositionMode compositionMode)
{
    if (m_composition == compositionMode)
        return true;

    if (m_renderTargetHolder->SetCompositionMode(compositionMode))
    {
        // the composition mode is supported by the render target
        m_composition = compositionMode;
        return true;
    }

    return false;
}

void wxD2DContext::BeginLayer(double opacity)
{
    EnsureInitialized();

    wxCOMPtr<ID2D1Layer> layer;
    HRESULT hr = GetRenderTarget()->CreateLayer(&layer);
    wxCHECK_HRESULT_RET(hr);

    LayerData ld;
    ld.type = OTHER_LAYER;
    ld.params = D2D1::LayerParameters(D2D1::InfiniteRect(),
                        nullptr,
                        D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
                        D2D1::IdentityMatrix(), gsl::narrow_cast<float>(opacity));
    ld.layer = layer;
    // We need to store CTM to be able to re-apply
    // the layer at the original position later on.
    GetRenderTarget()->GetTransform(&ld.transformMatrix);

    GetRenderTarget()->PushLayer(ld.params, layer);

    // Store layer parameters.
    m_layers.push(ld);
}

void wxD2DContext::EndLayer()
{
    std::stack<LayerData> layersToRestore;
    // Temporarily remove all clipping layers
    // above the first standard layer
    // and next permanently remove this layer.
    while ( !m_layers.empty() )
    {
        LayerData ld = m_layers.top();
        m_layers.pop();

        if ( ld.type == CLIP_LAYER )
        {
            GetRenderTarget()->PopLayer();
            layersToRestore.push(ld);
            continue;
        }

        // We found a non-clipping layer to remove.
        GetRenderTarget()->PopLayer();
        ld.layer.reset();
        break;
    }

    if ( m_layers.empty() )
    {
        HRESULT hr = GetRenderTarget()->Flush();
        wxCHECK_HRESULT_RET(hr);
    }

    // Re-apply all stored clipping layers.
    // First, save current transformation matrix.
    D2D1_MATRIX_3X2_F currTransform;
    GetRenderTarget()->GetTransform(&currTransform);
    while ( !layersToRestore.empty() )
    {
        LayerData ld = layersToRestore.top();
        layersToRestore.pop();

        if ( ld.type == CLIP_LAYER )
        {
            // Restore layer at original position.
            GetRenderTarget()->SetTransform(&ld.transformMatrix);
            GetRenderTarget()->PushLayer(ld.params, ld.layer);
        }
        else
        {
            wxFAIL_MSG( wxS("Invalid layer type") );
        }
        // Store layer parameters.
        m_layers.push(ld);
    }
    // Restore current transformation matrix.
    GetRenderTarget()->SetTransform(&currTransform);
}

void wxD2DContext::Translate(double dx, double dy)
{
    wxGraphicsMatrix translationMatrix = CreateMatrix();
    translationMatrix.Translate(dx, dy);
    ConcatTransform(translationMatrix);
}

void wxD2DContext::Scale(double xScale, double yScale)
{
    wxGraphicsMatrix scaleMatrix = CreateMatrix();
    scaleMatrix.Scale(xScale, yScale);
    ConcatTransform(scaleMatrix);
}

void wxD2DContext::Rotate(double angle)
{
    wxGraphicsMatrix rotationMatrix = CreateMatrix();
    rotationMatrix.Rotate(angle);
    ConcatTransform(rotationMatrix);
}

void wxD2DContext::ConcatTransform(const wxGraphicsMatrix& matrix)
{
    D2D1::Matrix3x2F localMatrix = wxGetD2DMatrixData(GetTransform())->GetMatrix3x2F();
    D2D1::Matrix3x2F concatMatrix = wxGetD2DMatrixData(matrix)->GetMatrix3x2F();

    D2D1::Matrix3x2F resultMatrix;
    resultMatrix.SetProduct(concatMatrix, localMatrix);

    wxGraphicsMatrix resultTransform;
    resultTransform.SetRefData(new wxD2DMatrixData(GetRenderer(), resultMatrix));

    SetTransform(resultTransform);
}

void wxD2DContext::SetTransform(const wxGraphicsMatrix& matrix)
{
    EnsureInitialized();

    D2D1::Matrix3x2F m;
    m.SetProduct(wxGetD2DMatrixData(matrix)->GetMatrix3x2F(), m_initTransform);
    GetRenderTarget()->SetTransform(&m);

    m_isClipBoxValid = false;
}

wxGraphicsMatrix wxD2DContext::GetTransform() const
{
    D2D1::Matrix3x2F transformMatrix;

    if (GetRenderTarget() != nullptr)
    {
        GetRenderTarget()->GetTransform(&transformMatrix);

        if ( m_initTransform.IsInvertible() )
        {
            D2D1::Matrix3x2F invMatrix = m_initTransform;
            invMatrix.Invert();

            D2D1::Matrix3x2F m;
            m.SetProduct(transformMatrix, invMatrix);
            transformMatrix = m;
        }
    }
    else
    {
        transformMatrix = D2D1::Matrix3x2F::Identity();
    }

    wxD2DMatrixData* matrixData = new wxD2DMatrixData(GetRenderer(), transformMatrix);

    wxGraphicsMatrix matrix;
    matrix.SetRefData(matrixData);

    return matrix;
}

void wxD2DContext::DrawBitmap(const wxGraphicsBitmap& bmp, double x, double y, double w, double h)
{
    if (m_composition == wxCOMPOSITION_DEST)
        return;

    wxD2DBitmapData* bitmapData = wxGetD2DBitmapData(bmp);
    bitmapData->Bind(this);
    const wxSize bmpSize = static_cast<wxD2DBitmapData::NativeType*>(bitmapData->GetNativeBitmap())->GetSize();

    m_renderTargetHolder->DrawBitmap(
        bitmapData->GetD2DBitmap(),
        D2D1::RectF(0.0F, 0.0F, gsl::narrow_cast<float>(bmpSize.x), gsl::narrow_cast<float>(bmpSize.y)),
        D2D1::RectF(gsl::narrow_cast<float>(x),
                    gsl::narrow_cast<float>(y),
                    gsl::narrow_cast<float>(x + w),
                    gsl::narrow_cast<float>(y + h)),
        GetInterpolationQuality(),
        GetCompositionMode());
}

void wxD2DContext::DrawBitmap(const wxBitmap& bmp, double x, double y, double w, double h)
{
    wxGraphicsBitmap graphicsBitmap = CreateBitmap(bmp);
    DrawBitmap(graphicsBitmap, x, y, w, h);
}

void wxD2DContext::DrawIcon(const wxIcon& icon, double x, double y, double w, double h)
{
    DrawBitmap(wxBitmap(icon), x, y, w, h);
}

void wxD2DContext::PushState()
{
    EnsureInitialized();

    StateData state;
    m_direct2dFactory->CreateDrawingStateBlock(&state.drawingState);
    GetRenderTarget()->SaveDrawingState(state.drawingState);
    state.layers = m_layers;

    m_stateStack.push(state);
}

void wxD2DContext::PopState()
{
    wxCHECK_RET(!m_stateStack.empty(), wxS("No state to pop"));

    // Remove all layers from the stack of layers.
    while ( !m_layers.empty() )
    {
        LayerData ld = m_layers.top();
        m_layers.pop();

        GetRenderTarget()->PopLayer();
        ld.layer.reset();
        ld.geometry.reset();
    }

    // Retrieve state data.
    StateData state = m_stateStack.top();
    m_stateStack.pop();

    // Restore all saved layers.
    std::stack<LayerData> layersToRestore;
    // We have to restore layers on the stack from "bottom" to "top",
    // so we have to create a "reverted" stack.
    while ( !state.layers.empty() )
    {
        LayerData ld = state.layers.top();
        state.layers.pop();

        layersToRestore.push(ld);
    }
    // And next set layers from the top of "reverted" stack.
    while ( !layersToRestore.empty() )
    {
        LayerData ld = layersToRestore.top();
        layersToRestore.pop();

        // Restore layer at original position.
        GetRenderTarget()->SetTransform(&ld.transformMatrix);
        GetRenderTarget()->PushLayer(ld.params, ld.layer);

        // Store layer parameters.
        m_layers.push(ld);
    }

    // Restore drawing state.
    GetRenderTarget()->RestoreDrawingState(state.drawingState);

    m_isClipBoxValid = false;
}

std::pair<double, double> wxD2DContext::GetTextExtent(
    std::string_view str,
    double* descent,
    double* externalLeading) const
{
    // FIXME: Does not return wxSize.
    //wxCHECK_RET(!m_font.IsNull(),
    //    wxS("wxD2DContext::GetTextExtent - no valid font set"));

    return wxD2DMeasuringContext::GetTextExtent(
        wxGetD2DFontData(m_font), str, descent, externalLeading);
}

std::vector<double> wxD2DContext::GetPartialTextExtents(std::string_view text) const
{
    //wxCHECK_RET(!m_font.IsNull(),
    //    wxS("wxD2DContext::GetPartialTextExtents - no valid font set"));

    return wxD2DMeasuringContext::GetPartialTextExtents(wxGetD2DFontData(m_font), text);
}

bool wxD2DContext::ShouldOffset() const
{
    if (!m_enableOffset || m_pen.IsNull())
        return false;

    wxD2DPenData* const penData = wxGetD2DPenData(m_pen);

    // always offset for 1-pixel width
    if (penData->IsZeroWidth())
        return true;

    // no offset if overall scale is not odd integer
    const wxGraphicsMatrix matrix(GetTransform());
    double x = GetContentScaleFactor();
    double y = x;
    matrix.TransformDistance(&x, &y);
    // FIXME: Double equality
    if (!(std::fmod(std::min(std::fabs(x), std::fabs(y)), 2.0) == 1.0))
        return false;

    // offset if pen width is odd integer
    // FIXME: Double equality
    return std::fmod(static_cast<double>(penData->GetWidth()), 2.0) == 1.0;
}

void wxD2DContext::DoDrawText(std::string_view str, double x, double y)
{
    wxCHECK_RET(!m_font.IsNull(),
        wxS("wxD2DContext::wxDrawText - no valid font set"));

    if (m_composition == wxCOMPOSITION_DEST)
        return;

    wxD2DFontData* fontData = wxGetD2DFontData(m_font);
    fontData->GetBrushData().Bind(this);

    wxCOMPtr<IDWriteTextLayout> textLayout = fontData->CreateTextLayout(str);

    // Render the text
    GetRenderTarget()->DrawTextLayout(
        D2D1::Point2F(gsl::narrow_cast<float>(x), gsl::narrow_cast<float>(y)),
        textLayout,
        fontData->GetBrushData().GetBrush());
}

void wxD2DContext::EnsureInitialized()
{
    if (!m_renderTargetHolder->IsResourceAcquired())
    {
        m_cachedRenderTarget = m_renderTargetHolder->GetD2DResource();
        GetRenderTarget()->GetTransform(&m_initTransform);
        GetRenderTarget()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        GetRenderTarget()->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);
        GetRenderTarget()->BeginDraw();
    }
    else
    {
        m_cachedRenderTarget = m_renderTargetHolder->GetD2DResource();
    }
}

void wxD2DContext::SetPen(const wxGraphicsPen& pen)
{
    wxGraphicsContext::SetPen(pen);

    if (!m_pen.IsNull())
    {
        EnsureInitialized();

        wxD2DPenData* penData = wxGetD2DPenData(pen);
        penData->Bind(this);
    }
}

void wxD2DContext::AdjustRenderTargetSize()
{
    m_renderTargetHolder->Resize();

    // Currently GetSize() can only be called when using MSVC because gcc
    // doesn't handle returning aggregates by value as done by D2D libraries,
    // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64384. Not updating the
    // size is not great, but it's better than crashing.
#ifdef __VISUALC__
    D2D1_SIZE_F renderTargetSize = m_renderTargetHolder->GetD2DResource()->GetSize();
    m_width = renderTargetSize.width;
    m_height =  renderTargetSize.height;
#endif // __VISUALC__
}

void wxD2DContext::ReleaseDeviceDependentResources()
{
    ReleaseResources();
}

void wxD2DContext::DrawRectangle(double x, double y, double w, double h)
{
    if (m_composition == wxCOMPOSITION_DEST)
        return;

    wxD2DOffsetHelper helper(this);

    EnsureInitialized();
    AdjustRenderTargetSize();

    D2D1_RECT_F rect = { (FLOAT)x, (FLOAT)y, (FLOAT)(x + w), (FLOAT)(y + h) };


    if (!m_brush.IsNull())
    {
        wxD2DBrushData* brushData = wxGetD2DBrushData(m_brush);
        brushData->Bind(this);
        GetRenderTarget()->FillRectangle(rect, brushData->GetBrush());
    }

    if (!m_pen.IsNull())
    {
        wxD2DPenData* penData = wxGetD2DPenData(m_pen);
        penData->SetWidth(this);
        penData->Bind(this);
        GetRenderTarget()->DrawRectangle(rect, penData->GetBrush(), penData->GetWidth(), penData->GetStrokeStyle());
    }
}

void wxD2DContext::DrawRoundedRectangle(double x, double y, double w, double h, double radius)
{
    if (m_composition == wxCOMPOSITION_DEST)
        return;

    wxD2DOffsetHelper helper(this);

    EnsureInitialized();
    AdjustRenderTargetSize();

    D2D1_RECT_F rect = { (FLOAT)x, (FLOAT)y, (FLOAT)(x + w), (FLOAT)(y + h) };

    D2D1_ROUNDED_RECT roundedRect = { rect, (FLOAT)radius, (FLOAT)radius };

    if (!m_brush.IsNull())
    {
        wxD2DBrushData* brushData = wxGetD2DBrushData(m_brush);
        brushData->Bind(this);
        GetRenderTarget()->FillRoundedRectangle(roundedRect, brushData->GetBrush());
    }

    if (!m_pen.IsNull())
    {
        wxD2DPenData* penData = wxGetD2DPenData(m_pen);
        penData->SetWidth(this);
        penData->Bind(this);
        GetRenderTarget()->DrawRoundedRectangle(roundedRect, penData->GetBrush(), penData->GetWidth(), penData->GetStrokeStyle());
    }
}

void wxD2DContext::DrawEllipse(double x, double y, double w, double h)
{
    if (m_composition == wxCOMPOSITION_DEST)
        return;

    wxD2DOffsetHelper helper(this);

    EnsureInitialized();
    AdjustRenderTargetSize();

    D2D1_ELLIPSE ellipse = {
        { (FLOAT)(x + w / 2), (FLOAT)(y + h / 2) }, // center point
          (FLOAT)(w / 2),                      // radius x
          (FLOAT)(h / 2)                       // radius y
    };

    if (!m_brush.IsNull())
    {
        wxD2DBrushData* brushData = wxGetD2DBrushData(m_brush);
        brushData->Bind(this);
        GetRenderTarget()->FillEllipse(ellipse, brushData->GetBrush());
    }

    if (!m_pen.IsNull())
    {
        wxD2DPenData* penData = wxGetD2DPenData(m_pen);
        penData->SetWidth(this);
        penData->Bind(this);
        GetRenderTarget()->DrawEllipse(ellipse, penData->GetBrush(), penData->GetWidth(), penData->GetStrokeStyle());
    }
}

void wxD2DContext::Flush()
{
    std::stack<LayerData> layersToRestore;
    // Temporarily remove all layers from the stack of layers.
    while ( !m_layers.empty() )
    {
        LayerData ld = m_layers.top();
        m_layers.pop();

        GetRenderTarget()->PopLayer();

        // Save layer data.
        layersToRestore.push(ld);
    }

    HRESULT hr = m_renderTargetHolder->Flush();

    if ( hr == (HRESULT)D2DERR_RECREATE_TARGET )
    {
        ReleaseDeviceDependentResources();
    }
    else
    {
        wxCHECK_HRESULT_RET(hr);
    }

    // Re-apply all layers.
    // First, save current transformation matrix.
    D2D1_MATRIX_3X2_F currTransform;
    GetRenderTarget()->GetTransform(&currTransform);
    while ( !layersToRestore.empty() )
    {
        LayerData ld = layersToRestore.top();
        layersToRestore.pop();

        // Restore layer at original position.
        GetRenderTarget()->SetTransform(&ld.transformMatrix);
        GetRenderTarget()->PushLayer(ld.params, ld.layer);

        // Store layer parameters.
        m_layers.push(ld);
    }
    // Restore current transformation matrix.
    GetRenderTarget()->SetTransform(&currTransform);
}

void wxD2DContext::GetDPI(double* dpiX, double* dpiY) const
{
    if ( GetWindow() )
    {
        const wxSize dpi = GetWindow()->GetDPI();

        if ( dpiX )
            *dpiX = dpi.x;
        if ( dpiY )
            *dpiY = dpi.y;
    }
    else
    {
        FLOAT x, y;
        GetRenderTarget()->GetDpi(&x, &y);

        if ( dpiX )
            *dpiX = x;
        if ( dpiY )
            *dpiY = y;
    }
}

//-----------------------------------------------------------------------------
// wxD2DRenderer declaration
//-----------------------------------------------------------------------------

class wxD2DRenderer : public wxGraphicsRenderer
{
public :
    wxD2DRenderer();

    ~wxD2DRenderer() override;

	wxD2DRenderer(const wxD2DRenderer&) = delete;
	wxD2DRenderer& operator=(const wxD2DRenderer&) = delete;

    std::unique_ptr<wxGraphicsContext> CreateContext(const wxWindowDC& dc) override;

    std::unique_ptr<wxGraphicsContext> CreateContext(const wxMemoryDC& dc) override;

#if wxUSE_PRINTING_ARCHITECTURE
    std::unique_ptr<wxGraphicsContext> CreateContext(const wxPrinterDC& dc) override;
#endif

#if wxUSE_ENH_METAFILE
    std::unique_ptr<wxGraphicsContext> CreateContext(const wxEnhMetaFileDC& dc) override;
#endif

    std::unique_ptr<wxGraphicsContext> CreateContextFromNativeContext(void* context) override;

    std::unique_ptr<wxGraphicsContext> CreateContextFromNativeWindow(void* window) override;

    std::unique_ptr<wxGraphicsContext> CreateContextFromNativeHDC(WXHDC dc) override;

    std::unique_ptr<wxGraphicsContext> CreateContext(wxWindow* window) override;

#if wxUSE_IMAGE
    std::unique_ptr<wxGraphicsContext> CreateContextFromImage(wxImage& image) override;
#endif // wxUSE_IMAGE

    std::unique_ptr<wxGraphicsContext> CreateMeasuringContext() override;

    wxGraphicsPath CreatePath() override;

    wxGraphicsMatrix CreateMatrix(
        double a = 1.0, double b = 0.0, double c = 0.0, double d = 1.0,
        double tx = 0.0, double ty = 0.0) override;

    wxGraphicsPen CreatePen(const wxGraphicsPenInfo& info) override;

    wxGraphicsBrush CreateBrush(const wxBrush& brush) override;

    wxGraphicsBrush CreateLinearGradientBrush(
        double x1, double y1,
        double x2, double y2,
        const wxGraphicsGradientStops& stops,
        const wxGraphicsMatrix& matrix = wxNullGraphicsMatrix) override;

    wxGraphicsBrush CreateRadialGradientBrush(
        double startX, double startY,
        double endX, double endY,
        double radius,
        const wxGraphicsGradientStops& stops,
        const wxGraphicsMatrix& matrix = wxNullGraphicsMatrix) override;

    // create a native bitmap representation
    wxGraphicsBitmap CreateBitmap(const wxBitmap& bitmap) override;

#if wxUSE_IMAGE
    wxGraphicsBitmap CreateBitmapFromImage(const wxImage& image) override;
    wxImage CreateImageFromBitmap(const wxGraphicsBitmap& bmp) override;
#endif

    wxGraphicsFont wxCreateFont(const wxFont& font, const wxColour& col) override;

    wxGraphicsFont wxCreateFont(double sizeInPixels,
                              const std::string& facename,
                              int flags = wxFONTFLAG_DEFAULT,
                              const wxColour& col = *wxBLACK) override;

    wxGraphicsFont CreateFontAtDPI(const wxFont& font,
                                           const wxRealPoint& dpi,
                                           const wxColour& col) override;

    // create a graphics bitmap from a native bitmap
    wxGraphicsBitmap CreateBitmapFromNativeBitmap(void* bitmap) override;

    // create a sub-image from a native image representation
    wxGraphicsBitmap CreateSubBitmap(const wxGraphicsBitmap& bitmap, double x, double y, double w, double h) override;

    std::string GetName() const override;
    void GetVersion(int* major, int* minor, int* micro) const override;

    ID2D1Factory* GetD2DFactory();

private:
    wxCOMPtr<ID2D1Factory> m_direct2dFactory;

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

//-----------------------------------------------------------------------------
// wxD2DRenderer implementation
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxD2DRenderer,wxGraphicsRenderer);

static wxD2DRenderer *gs_D2DRenderer = nullptr;

wxGraphicsRenderer* wxGraphicsRenderer::GetDirect2DRenderer()
{
    if (!wxDirect2D::Initialize())
        return nullptr;

    if (!gs_D2DRenderer)
    {
        gs_D2DRenderer = new wxD2DRenderer();
    }

    return gs_D2DRenderer;
}

wxD2DRenderer::wxD2DRenderer()
    : m_direct2dFactory(wxD2D1Factory())
{
    if ( m_direct2dFactory.get() == nullptr )
    {
        wxFAIL_MSG("Could not create Direct2D Factory.");
    }
}

wxD2DRenderer::~wxD2DRenderer()
{
    m_direct2dFactory.reset();
}

std::unique_ptr<wxGraphicsContext> wxD2DRenderer::CreateContext(const wxWindowDC& dc)
{
    return std::make_unique<wxD2DContext>(this, m_direct2dFactory, dc.GetHDC(), &dc);
}

std::unique_ptr<wxGraphicsContext> wxD2DRenderer::CreateContext(const wxMemoryDC& dc)
{
    wxBitmap bmp = dc.GetSelectedBitmap();
    wxASSERT_MSG( bmp.IsOk(), wxS("Should select a bitmap before creating wxGraphicsContext") );

    return std::make_unique<wxD2DContext>(this, m_direct2dFactory, dc.GetHDC(), &dc,
                            bmp.HasAlpha() ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE);
}

#if wxUSE_PRINTING_ARCHITECTURE
std::unique_ptr<wxGraphicsContext> wxD2DRenderer::CreateContext(const wxPrinterDC& WXUNUSED(dc))
{
    wxFAIL_MSG("not implemented");
    return nullptr;
}
#endif

#if wxUSE_ENH_METAFILE
std::unique_ptr<wxGraphicsContext> wxD2DRenderer::CreateContext(const wxEnhMetaFileDC& WXUNUSED(dc))
{
    wxFAIL_MSG("not implemented");
    return nullptr;
}
#endif

std::unique_ptr<wxGraphicsContext> wxD2DRenderer::CreateContextFromNativeContext(void* nativeContext)
{
    return std::make_unique<wxD2DContext>(this, m_direct2dFactory, nativeContext);
}

std::unique_ptr<wxGraphicsContext> wxD2DRenderer::CreateContextFromNativeWindow(void* window)
{
    return std::make_unique<wxD2DContext>(this, m_direct2dFactory, (HWND)window);
}

std::unique_ptr<wxGraphicsContext> wxD2DRenderer::CreateContextFromNativeHDC(WXHDC dc)
{
    return std::make_unique<wxD2DContext>(this, m_direct2dFactory, (HDC)dc);
}

std::unique_ptr<wxGraphicsContext> wxD2DRenderer::CreateContext(wxWindow* window)
{
    return std::make_unique<wxD2DContext>(this, m_direct2dFactory, (HWND)window->GetHWND(), window);
}

#if wxUSE_IMAGE
std::unique_ptr<wxGraphicsContext> wxD2DRenderer::CreateContextFromImage(wxImage& image)
{
    return std::make_unique<wxD2DContext>(this, m_direct2dFactory, image);
}
#endif // wxUSE_IMAGE

std::unique_ptr<wxGraphicsContext> wxD2DRenderer::CreateMeasuringContext()
{
    return std::make_unique<wxD2DMeasuringContext>(this);
}

wxGraphicsPath wxD2DRenderer::CreatePath()
{
    wxGraphicsPath p;
    p.SetRefData(new wxD2DPathData(this, m_direct2dFactory));

    return p;
}

wxGraphicsMatrix wxD2DRenderer::CreateMatrix(
    double a, double b, double c, double d,
    double tx, double ty)
{
    wxD2DMatrixData* matrixData = new wxD2DMatrixData(this);
    matrixData->Set(a, b, c, d, tx, ty);

    wxGraphicsMatrix matrix;
    matrix.SetRefData(matrixData);

    return matrix;
}

wxGraphicsPen wxD2DRenderer::CreatePen(const wxGraphicsPenInfo& info)
{
    if ( info.GetStyle() == wxPenStyle::Transparent )
    {
        return wxNullGraphicsPen;
    }
    else
    {
        wxGraphicsPen p;
        wxD2DPenData* penData = new wxD2DPenData(this, m_direct2dFactory, info);
        p.SetRefData(penData);
        return p;
    }
}

wxGraphicsBrush wxD2DRenderer::CreateBrush(const wxBrush& brush)
{
    if ( !brush.IsOk() || brush.GetStyle() == wxBrushStyle::Transparent )
    {
        return wxNullGraphicsBrush;
    }
    else
    {
        wxGraphicsBrush b;
        b.SetRefData(new wxD2DBrushData(this, brush));
        return b;
    }
}

wxGraphicsBrush wxD2DRenderer::CreateLinearGradientBrush(
    double x1, double y1,
    double x2, double y2,
    const wxGraphicsGradientStops& stops,
    const wxGraphicsMatrix& matrix)
{
    wxD2DBrushData* brushData = new wxD2DBrushData(this);
    brushData->CreateLinearGradientBrush(x1, y1, x2, y2, stops, matrix);

    wxGraphicsBrush brush;
    brush.SetRefData(brushData);

    return brush;
}

wxGraphicsBrush wxD2DRenderer::CreateRadialGradientBrush(
    double startX, double startY,
    double endX, double endY,
    double radius,
    const wxGraphicsGradientStops& stops,
    const wxGraphicsMatrix& matrix)
{
    wxD2DBrushData* brushData = new wxD2DBrushData(this);
    brushData->CreateRadialGradientBrush(startX, startY, endX, endY, radius, stops, matrix);

    wxGraphicsBrush brush;
    brush.SetRefData(brushData);

    return brush;
}

// create a native bitmap representation
wxGraphicsBitmap wxD2DRenderer::CreateBitmap(const wxBitmap& bitmap)
{
    wxD2DBitmapData* bitmapData = new wxD2DBitmapData(this, bitmap);

    wxGraphicsBitmap graphicsBitmap;
    graphicsBitmap.SetRefData(bitmapData);

    return graphicsBitmap;
}

// create a graphics bitmap from a native bitmap
wxGraphicsBitmap wxD2DRenderer::CreateBitmapFromNativeBitmap(void* bitmap)
{
    wxD2DBitmapData* bitmapData = new wxD2DBitmapData(this, static_cast<wxD2DBitmapResourceHolder*>(bitmap));

    wxGraphicsBitmap graphicsBitmap;
    graphicsBitmap.SetRefData(bitmapData);

    return graphicsBitmap;
}

#if wxUSE_IMAGE
wxGraphicsBitmap wxD2DRenderer::CreateBitmapFromImage(const wxImage& image)
{
    wxD2DBitmapData* bitmapData = new wxD2DBitmapData(this, image);

    wxGraphicsBitmap graphicsBitmap;
    graphicsBitmap.SetRefData(bitmapData);

    return graphicsBitmap;
}

wxImage wxD2DRenderer::CreateImageFromBitmap(const wxGraphicsBitmap& bmp)
{
    return static_cast<wxD2DBitmapData::NativeType*>(bmp.GetNativeBitmap())
        ->ConvertToImage();
}
#endif

wxGraphicsFont wxD2DRenderer::wxCreateFont(const wxFont& font, const wxColour& col)
{
    return CreateFontAtDPI(font, wxRealPoint(), col);
}

wxGraphicsFont wxD2DRenderer::wxCreateFont(double sizeInPixels,
                                         const std::string& facename,
                                         int flags,
                                         const wxColour& col)
{
    using msw::utils::unique_dcwnd;
    // Use the same DPI as wxFont will use in SetPixelSize, so these cancel
    // each other out and we are left with the actual pixel size.
    unique_dcwnd hdc{::GetDC(nullptr)};
    wxRealPoint dpi(::GetDeviceCaps(hdc.get(), LOGPIXELSX),
                    ::GetDeviceCaps(hdc.get(), LOGPIXELSY));

    return CreateFontAtDPI(
        wxFontInfo(wxSize(sizeInPixels, sizeInPixels)).AllFlags(flags).FaceName(facename),
        dpi, col);
}

wxGraphicsFont wxD2DRenderer::CreateFontAtDPI(const wxFont& font,
                                              const wxRealPoint& dpi,
                                              const wxColour& col)
{
    auto fontData = std::make_unique<wxD2DFontData>(this, font, dpi, col);
    if ( !fontData->GetFont() )
    {
        // Apparently a non-TrueType font is given and hence
        // corresponding DirectWrite font couldn't be created.
        return wxNullGraphicsFont;
    }

    wxGraphicsFont graphicsFont;
    graphicsFont.SetRefData(fontData.release());

    return graphicsFont;
}

// create a sub-image from a native image representation
wxGraphicsBitmap wxD2DRenderer::CreateSubBitmap(const wxGraphicsBitmap& bitmap, double x, double y, double w, double h)
{
    using NativeBitmap = wxD2DBitmapData::NativeType*;

    NativeBitmap natBmp = static_cast<NativeBitmap>(bitmap.GetNativeBitmap())->GetSubBitmap(x, y, w, h);
    wxGraphicsBitmap bmpRes;
    bmpRes.SetRefData(new wxD2DBitmapData(this, natBmp));
    return bmpRes;
}

std::string wxD2DRenderer::GetName() const
{
    return "direct2d";
}

void wxD2DRenderer::GetVersion(int* major, int* minor, int* micro) const
{
    if (wxDirect2D::HasDirect2DSupport())
    {
        if (major)
            *major = 1;

        if (minor)
        {
            switch(wxDirect2D::GetDirect2DVersion())
            {
            case wxDirect2D::wxD2D_VERSION_1_0:
                *minor = 0;
                break;
            case wxDirect2D::wxD2D_VERSION_1_1:
                *minor = 1;
                break;
            case wxDirect2D::wxD2D_VERSION_1_2:
                *minor = 2;
                break;
            case wxDirect2D::wxD2D_VERSION_1_3:
                *minor = 3;
                break;
            case wxDirect2D::wxD2D_VERSION_NONE:
                // This is not supposed to happen, but we handle this value in
                // the switch to ensure that we'll get warnings if any new
                // values, not handled here, are added to the enum later.
                *minor = -1;
                break;
            }
        }

        if (micro)
            *micro = 0;
    }
}

ID2D1Factory* wxD2DRenderer::GetD2DFactory()
{
    return m_direct2dFactory;
}

ID2D1Factory* wxGetD2DFactory(wxGraphicsRenderer* renderer)
{
    return dynamic_cast<wxD2DRenderer*>(renderer)->GetD2DFactory();
}

// ----------------------------------------------------------------------------
// Module ensuring all global/singleton objects are destroyed on shutdown.
// ----------------------------------------------------------------------------

class wxDirect2DModule : public wxModule
{
public:
    wxDirect2DModule()
    {
        // Using Direct2D requires OLE and, importantly, we must ensure our
        // OnExit() runs before it is uninitialized.
        AddDependency("wxOleInitModule");
    }

    bool OnInit() override
    {
        return true;
    }

    void OnExit() override
    {
        if ( gs_WICImagingFactory )
        {
            gs_WICImagingFactory->Release();
            gs_WICImagingFactory = nullptr;
        }

        if ( gs_IDWriteFactory )
        {
#if wxUSE_PRIVATE_FONTS
            if ( wxDirect2DFontCollectionLoader::IsInitialized() )
            {
                gs_pPrivateFontCollection.reset();
                gs_IDWriteFactory->UnregisterFontCollectionLoader(wxDirect2DFontCollectionLoader::GetLoader());
            }
#endif // wxUSE_PRIVATE_FONTS
            gs_IDWriteFactory->Release();
            gs_IDWriteFactory = nullptr;
        }

        if ( gs_D2DRenderer )
        {
            delete gs_D2DRenderer;
            gs_D2DRenderer = nullptr;
        }

        if ( gs_ID2D1Factory )
        {
            gs_ID2D1Factory->Release();
            gs_ID2D1Factory = nullptr;
        }
    }

private:
    wxDECLARE_DYNAMIC_CLASS(wxDirect2DModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxDirect2DModule, wxModule);


#endif // wxUSE_GRAPHICS_DIRECT2D
