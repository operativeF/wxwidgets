///////////////////////////////////////////////////////////////////////////////
// Name:        wx/withimages.h
// Purpose:     Declaration of a simple wxWithImages class.
// Author:      Vadim Zeitlin
// Created:     2011-08-17
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/icon.h"
#include "wx/imaglist.h"
#include "wx/gdicmn.h"

export module WX.Core.WithImages;

// ----------------------------------------------------------------------------
// wxWithImages: mix-in class providing access to wxImageList.
// ----------------------------------------------------------------------------

export
{

class wxWithImages
{
public:
    static constexpr auto NO_IMAGE{-1};

    virtual ~wxWithImages()
    {
        FreeIfNeeded();
    }

    wxWithImages& operator=(wxWithImages&&) = delete;

    // Sets the image list to use, it is *not* deleted by the control.
    virtual void SetImageList(wxImageList* imageList)
    {
        FreeIfNeeded();
        m_imageList = imageList;
    }

    // As SetImageList() but we will delete the image list ourselves.
    void AssignImageList(wxImageList* imageList)
    {
        SetImageList(imageList);
        m_ownsImageList = true;
    }

    // Get pointer (may be NULL) to the associated image list.
    wxImageList* GetImageList() const { return m_imageList; }

protected:
    // Return true if we have a valid image list.
    bool HasImageList() const { return m_imageList != nullptr; }

    // Return the image with the given index from the image list.
    //
    // If there is no image list or if index == NO_IMAGE, silently returns
    // wxNullIcon.
    wxIcon GetImage(int iconIndex) const
    {
        return m_imageList && iconIndex != NO_IMAGE
                    ? m_imageList->GetIcon(iconIndex)
                    : wxNullIcon;
    }

private:
    // Free the image list if necessary, i.e. if we own it.
    void FreeIfNeeded()
    {
        if ( m_ownsImageList )
        {
            delete m_imageList;
            m_imageList = nullptr;

            // We don't own it any more.
            m_ownsImageList = false;
        }
    }


    // The associated image list or nullptr.
    wxImageList* m_imageList{nullptr};

    // False by default, if true then we delete m_imageList.
    bool m_ownsImageList{false};
};

} // export
