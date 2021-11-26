/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/artprov.cpp
// Purpose:     wxArtProvider class
// Author:      Vaclav Slavik
// Modified by:
// Created:     18/03/2002
// Copyright:   (c) Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/artprov.h"

#include "wx/dcmemory.h"
#include "wx/list.h"
#include "wx/log.h"
#include "wx/hashmap.h"
#include "wx/module.h"

#include <fmt/core.h>

#include "wx/listimpl.cpp"
WX_DECLARE_LIST(wxArtProvider, wxArtProvidersList);
WX_DEFINE_LIST(wxArtProvidersList)

import WX.Image;

// ----------------------------------------------------------------------------
// Cache class - stores already requested bitmaps
// ----------------------------------------------------------------------------

WX_DECLARE_EXPORTED_STRING_HASH_MAP(wxBitmap, wxArtProviderBitmapsHash);
WX_DECLARE_EXPORTED_STRING_HASH_MAP(wxIconBundle, wxArtProviderIconBundlesHash);

class wxArtProviderCache
{
public:
    bool GetBitmap(const std::string& full_id, wxBitmap* bmp);
    void PutBitmap(const std::string& full_id, const wxBitmap& bmp)
        { m_bitmapsHash[full_id] = bmp; }

    bool GetIconBundle(const std::string& full_id, wxIconBundle* bmp);
    void PutIconBundle(const std::string& full_id, const wxIconBundle& iconbundle)
        { m_iconBundlesHash[full_id] = iconbundle; }

    void Clear();

    static std::string ConstructHashID(const wxArtID& id,
                                    const wxArtClient& client,
                                    const wxSize& size);

    static std::string ConstructHashID(const wxArtID& id,
                                    const wxArtClient& client);

private:
    wxArtProviderBitmapsHash m_bitmapsHash;         // cache of wxBitmaps
    wxArtProviderIconBundlesHash m_iconBundlesHash; // cache of wxIconBundles
};

bool wxArtProviderCache::GetBitmap(const std::string& full_id, wxBitmap* bmp)
{
    wxArtProviderBitmapsHash::iterator entry = m_bitmapsHash.find(full_id);
    if ( entry == m_bitmapsHash.end() )
    {
        return false;
    }
    else
    {
        *bmp = entry->second;
        return true;
    }
}

bool wxArtProviderCache::GetIconBundle(const std::string& full_id, wxIconBundle* bmp)
{
    wxArtProviderIconBundlesHash::iterator entry = m_iconBundlesHash.find(full_id);
    if ( entry == m_iconBundlesHash.end() )
    {
        return false;
    }
    else
    {
        *bmp = entry->second;
        return true;
    }
}

void wxArtProviderCache::Clear()
{
    m_bitmapsHash.clear();
    m_iconBundlesHash.clear();
}

std::string wxArtProviderCache::ConstructHashID(const wxArtID& id,
                                                const wxArtClient& client)
{
    return fmt::format("{}-{}", id, client);
}


std::string wxArtProviderCache::ConstructHashID(const wxArtID& id,
                                                const wxArtClient& client,
                                                const wxSize& size)
{
    return fmt::format("{}-{}-{}", ConstructHashID(id, client), size.x, size.y);
}

// ============================================================================
// wxArtProvider class
// ============================================================================

// ----------------------------------------------------------------------------
// wxArtProvider ctors/dtor
// ----------------------------------------------------------------------------

wxArtProvider::~wxArtProvider()
{
    Remove(this);
}

// ----------------------------------------------------------------------------
// wxArtProvider operations on provider stack
// ----------------------------------------------------------------------------

void wxArtProvider::CommonAddingProvider()
{
    if ( !sm_providers )
    {
        sm_providers = new wxArtProvidersList;
        sm_cache = new wxArtProviderCache;
    }

    sm_cache->Clear();
}

void wxArtProvider::Push(wxArtProvider *provider)
{
    CommonAddingProvider();
    sm_providers->Insert(provider);
}

void wxArtProvider::PushBack(wxArtProvider *provider)
{
    CommonAddingProvider();
    sm_providers->Append(provider);
}

bool wxArtProvider::Pop()
{
    wxCHECK_MSG( sm_providers, false, "no wxArtProvider exists" );
    wxCHECK_MSG( !sm_providers->empty(), false, "wxArtProviders stack is empty" );

    delete sm_providers->GetFirst()->GetData();
    sm_cache->Clear();
    return true;
}

bool wxArtProvider::Remove(wxArtProvider *provider)
{
    wxCHECK_MSG( sm_providers, false, "no wxArtProvider exists" );

    if ( sm_providers->DeleteObject(provider) )
    {
        sm_cache->Clear();
        return true;
    }

    return false;
}

bool wxArtProvider::Delete(wxArtProvider *provider)
{
    // provider will remove itself from the stack in its dtor
    delete provider;

    return true;
}

void wxArtProvider::CleanUpProviders()
{
    if ( sm_providers )
    {
        while ( !sm_providers->empty() )
            delete *sm_providers->begin();

        wxDELETE(sm_providers);
        wxDELETE(sm_cache);
    }
}

// ----------------------------------------------------------------------------
// wxArtProvider: retrieving bitmaps/icons
// ----------------------------------------------------------------------------

void wxArtProvider::RescaleBitmap(wxBitmap& bmp, const wxSize& sizeNeeded)
{
    wxCHECK_RET( sizeNeeded.IsFullySpecified(), "New size must be given" );

#if wxUSE_IMAGE
    wxImage img = bmp.ConvertToImage();
    img.Rescale(sizeNeeded.x, sizeNeeded.y);
    bmp = wxBitmap(img);
#else // !wxUSE_IMAGE
    // Fallback method of scaling the bitmap
    wxBitmap newBmp(sizeNeeded, bmp.GetDepth());
#if defined(__WXMSW__) || defined(__WXOSX__)
    // wxBitmap::UseAlpha() is used only on wxMSW and wxOSX.
    newBmp.UseAlpha(bmp.HasAlpha());
#endif // __WXMSW__ || __WXOSX__
    {
        wxMemoryDC dc(newBmp);
        double scX = (double)sizeNeeded.GetWidth() / bmp.GetWidth();
        double scY = (double)sizeNeeded.GetHeight() / bmp.GetHeight();
        dc.SetUserScale({scX, scY});
        dc.DrawBitmap(bmp, 0, 0);
    }
    bmp = newBmp;
#endif // wxUSE_IMAGE/!wxUSE_IMAGE
}

wxBitmap wxArtProvider::GetBitmap(const wxArtID& id,
                                  const wxArtClient& client,
                                  const wxSize& size)
{
    // safety-check against writing client,id,size instead of id,client,size:
    wxASSERT_MSG( client.back() == 'C', "invalid 'client' parameter" );

    wxCHECK_MSG( sm_providers, wxNullBitmap, "no wxArtProvider exists" );

    std::string hashId = wxArtProviderCache::ConstructHashID(id, client, size);

    wxBitmap bmp;
    if ( !sm_cache->GetBitmap(hashId, &bmp) )
    {
        for (wxArtProvidersList::compatibility_iterator node = sm_providers->GetFirst();
             node; node = node->GetNext())
        {
            bmp = node->GetData()->CreateBitmap(id, client, size);
            if ( bmp.IsOk() )
                break;
        }

        wxSize sizeNeeded = size;
        if ( !bmp.IsOk() )
        {
            // no bitmap created -- as a fallback, try if we can find desired
            // icon in a bundle
            wxIconBundle iconBundle = DoGetIconBundle(id, client);
            if ( iconBundle.IsOk() )
            {
                if ( sizeNeeded == wxDefaultSize )
                    sizeNeeded = GetNativeSizeHint(client);

                wxIcon icon(iconBundle.GetIcon(sizeNeeded));
                if ( icon.IsOk() )
                {
                    // this icon may be not of the correct size, it will be
                    // rescaled below in such case
                    bmp.CopyFromIcon(icon);
                }
            }
        }

        // if we didn't get the correct size, resize the bitmap
        if ( bmp.IsOk() && sizeNeeded != wxDefaultSize )
        {
            if ( bmp.GetSize() != sizeNeeded )
            {
                RescaleBitmap(bmp, sizeNeeded);
            }
        }

        sm_cache->PutBitmap(hashId, bmp);
    }

    return bmp;
}

wxIconBundle wxArtProvider::GetIconBundle(const wxArtID& id, const wxArtClient& client)
{
    wxIconBundle iconbundle(DoGetIconBundle(id, client));

    if ( iconbundle.IsOk() )
    {
        return iconbundle;
    }
    else
    {
        // fall back to single-icon bundle
        return {GetIcon(id, client)};
    }
}

wxIconBundle wxArtProvider::DoGetIconBundle(const wxArtID& id, const wxArtClient& client)
{
    // safety-check against writing client,id,size instead of id,client,size:
    wxASSERT_MSG( client.back() == 'C', "invalid 'client' parameter" );

    wxCHECK_MSG( sm_providers, wxNullIconBundle, "no wxArtProvider exists" );

    std::string hashId = wxArtProviderCache::ConstructHashID(id, client);

    wxIconBundle iconbundle;
    if ( !sm_cache->GetIconBundle(hashId, &iconbundle) )
    {
        for (wxArtProvidersList::compatibility_iterator node = sm_providers->GetFirst();
             node; node = node->GetNext())
        {
            iconbundle = node->GetData()->CreateIconBundle(id, client);
            if ( iconbundle.IsOk() )
                break;
        }

        sm_cache->PutIconBundle(hashId, iconbundle);
    }

    return iconbundle;
}

wxIcon wxArtProvider::GetIcon(const wxArtID& id,
                                         const wxArtClient& client,
                                         const wxSize& size)
{
    wxBitmap bmp = GetBitmap(id, client, size);

    if ( !bmp.IsOk() )
        return wxNullIcon;

    wxIcon icon;
    icon.CopyFromBitmap(bmp);
    return icon;
}

/* static */
wxArtID wxArtProvider::GetMessageBoxIconId(unsigned int flags)
{
    switch ( flags & wxICON_MASK )
    {
        default:
            wxFAIL_MSG("incorrect message box icon flags");
            [[fallthrough]];

        case wxICON_ERROR:
            return wxART_ERROR;

        case wxICON_INFORMATION:
            return wxART_INFORMATION;

        case wxICON_WARNING:
            return wxART_WARNING;

        case wxICON_QUESTION:
            return wxART_QUESTION;
    }
}

wxSize wxArtProvider::GetSizeHint(const wxArtClient& client,
                                         bool platform_dependent)
{
    if (!platform_dependent)
    {
        wxArtProvidersList::compatibility_iterator node = sm_providers->GetFirst();
        if (node)
            return node->GetData()->DoGetSizeHint(client);
    }

    return GetNativeSizeHint(client);
}

#ifndef wxHAS_NATIVE_ART_PROVIDER_IMPL
/*static*/
wxSize wxArtProvider::GetNativeSizeHint([[maybe_unused]] const wxArtClient& client)
{
    // rather than returning some arbitrary value that doesn't make much
    // sense (as 2.8 used to do), tell the caller that we don't have a clue:
    return wxDefaultSize;
}

/*static*/
void wxArtProvider::InitNativeProvider()
{
}
#endif // !wxHAS_NATIVE_ART_PROVIDER_IMPL


/* static */
bool wxArtProvider::HasNativeProvider()
{
#ifdef __WXGTK20__
    return true;
#else
    return false;
#endif
}

// ============================================================================
// wxArtProviderModule
// ============================================================================

class wxArtProviderModule: public wxModule
{
public:
    bool OnInit() override
    {
        // The order here is such that the native provider will be used first
        // and the standard one last as all these default providers add
        // themselves to the bottom of the stack.
        wxArtProvider::InitNativeProvider();
#if wxUSE_ARTPROVIDER_TANGO
        wxArtProvider::InitTangoProvider();
#endif // wxUSE_ARTPROVIDER_TANGO
#if wxUSE_ARTPROVIDER_STD
        wxArtProvider::InitStdProvider();
#endif // wxUSE_ARTPROVIDER_STD
        return true;
    }
    void OnExit() override
    {
        wxArtProvider::CleanUpProviders();
    }

    wxDECLARE_DYNAMIC_CLASS(wxArtProviderModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxArtProviderModule, wxModule);
