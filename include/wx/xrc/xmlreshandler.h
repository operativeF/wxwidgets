/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xmlreshandler.cpp
// Purpose:     XML resource handler
// Author:      Steven Lamerton
// Created:     2011/01/26
// Copyright:   (c) 2011 Steven Lamerton
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XRC_XMLRESHANDLER_H_
#define _WX_XRC_XMLRESHANDLER_H_

#if wxUSE_XRC

#include "wx/string.h"
#include "wx/artprov.h"
#include "wx/colour.h"
#include "wx/filesys.h"
#include "wx/font.h"
#include "wx/imaglist.h"

import WX.Cfg.Flags;

class wxAnimation;
class wxAnimationCtrlBase;

class wxXmlNode;
class wxXmlResource;

class wxXmlResourceHandler;
class wxWindow;


// Helper macro used by the classes derived from wxXmlResourceHandler but also
// by wxXmlResourceHandler implementation itself.
#define XRC_ADD_STYLE(style) AddStyle(wxT(#style), style)

// Flags for GetNodeText().
enum
{
    wxXRC_TEXT_NO_TRANSLATE = 1,
    wxXRC_TEXT_NO_ESCAPE    = 2
};

// Abstract base class for the implementation object used by
// wxXmlResourceHandlerImpl. The real implementation is in
// wxXmlResourceHandlerImpl class in the "xrc" library while this class is in
// the "core" itself -- but it is so small that it doesn't matter.

class wxXmlResourceHandlerImplBase : public wxObject
{
public:
    // Constructor.
    wxXmlResourceHandlerImplBase(wxXmlResourceHandler *handler)
        : m_handler(handler)
    {}

    virtual wxObject *CreateResource(wxXmlNode *node, wxObject *parent,
                                     wxObject *instance) = 0;
    virtual bool IsOfClass(wxXmlNode *node, const wxString& classname) const = 0;
    virtual bool IsObjectNode(const wxXmlNode *node) const = 0;
    virtual wxString GetNodeContent(const wxXmlNode *node) = 0;
    virtual wxXmlNode *GetNodeParent(const wxXmlNode *node) const = 0;
    virtual wxXmlNode *GetNodeNext(const wxXmlNode *node) const = 0;
    virtual wxXmlNode *GetNodeChildren(const wxXmlNode *node) const = 0;
    virtual bool HasParam(const wxString& param) = 0;
    virtual wxXmlNode *GetParamNode(const wxString& param) = 0;
    virtual wxString GetParamValue(const wxString& param) = 0;
    virtual wxString GetParamValue(const wxXmlNode* node) = 0;
    virtual int GetStyle(const wxString& param = "style", int defaults = 0) = 0;
    virtual std::string GetNodeText(const wxXmlNode *node, int flags = 0) = 0;
    virtual int GetID() = 0;
    virtual std::string GetName() = 0;
    virtual bool GetBool(const wxString& param, bool defaultv = false) = 0;
    virtual long GetLong(const wxString& param, long defaultv = 0) = 0;
    virtual float GetFloat(const wxString& param, float defaultv = 0) = 0;
    virtual wxColour GetColour(const wxString& param,
                               const wxColour& defaultv = wxNullColour) = 0;
    virtual wxSize GetSize(const wxString& param = "size",
                           wxWindow *windowToUse = nullptr) = 0;
    virtual wxPoint GetPosition(const wxString& param = "pos") = 0;
    virtual wxCoord GetDimension(const wxString& param, wxCoord defaultv = 0,
                                 wxWindow *windowToUse = nullptr) = 0;
    virtual wxSize GetPairInts(const wxString& param) = 0;
    virtual wxDirection GetDirection(const wxString& param, wxDirection dir = wxLEFT) = 0;
    virtual wxBitmap GetBitmap(const wxString& param = "bitmap",
                               const wxArtClient& defaultArtClient = wxART_OTHER,
                               wxSize size = wxDefaultSize) = 0;
    virtual wxBitmap GetBitmap(const wxXmlNode* node,
                               const wxArtClient& defaultArtClient = wxART_OTHER,
                               wxSize size = wxDefaultSize) = 0;
    virtual wxIcon GetIcon(const wxString& param = "icon",
                           const wxArtClient& defaultArtClient = wxART_OTHER,
                           wxSize size = wxDefaultSize) = 0;
    virtual wxIcon GetIcon(const wxXmlNode* node,
                           const wxArtClient& defaultArtClient = wxART_OTHER,
                           wxSize size = wxDefaultSize) = 0;
    virtual wxIconBundle GetIconBundle(const wxString& param,
                                       const wxArtClient& defaultArtClient = wxART_OTHER) = 0;
    virtual wxImageList *GetImageList(const wxString& param = "imagelist") = 0;

#if wxUSE_ANIMATIONCTRL
    virtual wxAnimation* GetAnimation(const wxString& param = "animation",
                                      wxAnimationCtrlBase* ctrl = nullptr) = 0;
#endif

    virtual wxFont GetFont(const wxString& param = "font", wxWindow* parent = nullptr) = 0;
    virtual bool GetBoolAttr(const wxString& attr, bool defaultv) = 0;
    virtual wxString GetFilePath(const wxXmlNode* node) = 0;
    virtual void SetupWindow(wxWindow *wnd) = 0;
    virtual void CreateChildren(wxObject *parent, bool this_hnd_only = false) = 0;
    virtual void CreateChildrenPrivately(wxObject *parent,
                                         wxXmlNode *rootnode = nullptr) = 0;
    virtual wxObject *CreateResFromNode(wxXmlNode *node, wxObject *parent,
                                        wxObject *instance = nullptr) = 0;

#if wxUSE_FILESYSTEM
    virtual wxFileSystem& GetCurFileSystem() = 0;
#endif
    virtual void ReportError(wxXmlNode *context, const wxString& message) = 0;
    virtual void ReportError(const wxString& message) = 0;
    virtual void ReportParamError(const wxString& param, const wxString& message) = 0;

    wxXmlResourceHandler* GetHandler() { return m_handler; }

protected:
    wxXmlResourceHandler *m_handler;
};

// Base class for all XRC handlers.
//
// Notice that this class is defined in the core library itself and so can be
// used as the base class by classes in any GUI library. However to actually be
// usable, it needs to be registered with wxXmlResource which implies linking
// the application with the xrc library.
//
// Also note that all the methods forwarding to GetImpl() are documented only
// in wxXmlResourceHandlerImpl in wx/xrc/xmlres.h to avoid duplication.

class wxXmlResourceHandler : public wxObject
{
public:
    // Constructor creates an unusable object, before anything can be done with
    // it, SetImpl() needs to be called as done by wxXmlResource::AddHandler().
    wxXmlResourceHandler()
    {
        m_node = nullptr;
        m_parent =
        m_instance = nullptr;
        m_parentAsWindow = nullptr;
        m_resource = nullptr;

        m_impl = nullptr;
    }

    // This should be called exactly once.
    void SetImpl(wxXmlResourceHandlerImplBase* impl)
    {
        wxASSERT_MSG( !m_impl, "Should be called exactly once" );

        m_impl = impl;
    }


    // Destructor.
    ~wxXmlResourceHandler()
    {
        delete m_impl;
    }

    wxObject *CreateResource(wxXmlNode *node, wxObject *parent,
                             wxObject *instance)
    {
        return GetImpl()->CreateResource(node, parent, instance);
    }

    // This one is called from CreateResource after variables
    // were filled.
    virtual wxObject *DoCreateResource() = 0;

    // Returns true if it understands this node and can create
    // a resource from it, false otherwise.
    virtual bool CanHandle(wxXmlNode *node) = 0;


    void SetParentResource(wxXmlResource *res)
    {
        m_resource = res;
    }


    // These methods are not forwarded to wxXmlResourceHandlerImpl because they
    // are called from the derived classes ctors and so before SetImpl() can be
    // called.

    // Add a style flag (e.g. wxMB_DOCKABLE) to the list of flags
    // understood by this handler.
    void AddStyle(const wxString& name, int value);

    // Add styles common to all wxWindow-derived classes.
    void AddWindowStyles();

protected:
    // Everything else is simply forwarded to wxXmlResourceHandlerImpl.
    void ReportError(wxXmlNode *context, const wxString& message)
    {
        GetImpl()->ReportError(context, message);
    }
    void ReportError(const wxString& message)
    {
        GetImpl()->ReportError(message);
    }
    void ReportParamError(const wxString& param, const wxString& message)
    {
        GetImpl()->ReportParamError(param, message);
    }

    bool IsOfClass(wxXmlNode *node, const wxString& classname) const
    {
        return GetImpl()->IsOfClass(node, classname);
    }

    bool IsObjectNode(const wxXmlNode *node) const
    {
        return GetImpl()->IsObjectNode(node);
    }
    wxString GetNodeContent(const wxXmlNode *node)
    {
        return GetImpl()->GetNodeContent(node);
    }

    wxXmlNode *GetNodeParent(const wxXmlNode *node) const
    {
        return GetImpl()->GetNodeParent(node);
    }
    wxXmlNode *GetNodeNext(const wxXmlNode *node) const
    {
        return GetImpl()->GetNodeNext(node);
    }
    wxXmlNode *GetNodeChildren(const wxXmlNode *node) const
    {
        return GetImpl()->GetNodeChildren(node);
    }

    bool HasParam(const wxString& param)
    {
        return GetImpl()->HasParam(param);
    }

    wxXmlNode *GetParamNode(const wxString& param)
    {
        return GetImpl()->GetParamNode(param);
    }
    wxString GetParamValue(const wxString& param)
    {
        return GetImpl()->GetParamValue(param);
    }
    wxString GetParamValue(const wxXmlNode* node)
    {
        return GetImpl()->GetParamValue(node);
    }
    int GetStyle(const wxString& param = "style", int defaults = 0)
    {
        return GetImpl()->GetStyle(param, defaults);
    }
    std::string GetNodeText(const wxXmlNode *node, int flags = 0)
    {
        return GetImpl()->GetNodeText(node, flags);
    }
    wxString GetText(const wxString& param, bool translate = true)
    {
        return GetImpl()->GetNodeText(GetImpl()->GetParamNode(param),
                                      translate ? 0 : wxXRC_TEXT_NO_TRANSLATE);
    }
    int GetID() const
    {
        return GetImpl()->GetID();
    }
    std::string GetName()
    {
        return GetImpl()->GetName();
    }
    bool GetBool(const wxString& param, bool defaultv = false)
    {
        return GetImpl()->GetBool(param, defaultv);
    }
    long GetLong(const wxString& param, long defaultv = 0)
    {
        return GetImpl()->GetLong(param, defaultv);
    }
    float GetFloat(const wxString& param, float defaultv = 0)
    {
        return GetImpl()->GetFloat(param, defaultv);
    }
    wxColour GetColour(const wxString& param,
                       const wxColour& defaultv = wxNullColour)
    {
        return GetImpl()->GetColour(param, defaultv);
    }
    wxSize GetSize(const wxString& param = "size",
                   wxWindow *windowToUse = nullptr)
    {
        return GetImpl()->GetSize(param, windowToUse);
    }
    wxPoint GetPosition(const wxString& param = "pos")
    {
        return GetImpl()->GetPosition(param);
    }
    wxCoord GetDimension(const wxString& param, wxCoord defaultv = 0,
                         wxWindow *windowToUse = nullptr)
    {
        return GetImpl()->GetDimension(param, defaultv, windowToUse);
    }
    wxSize GetPairInts(const wxString& param)
    {
        return GetImpl()->GetPairInts(param);
    }
    wxDirection GetDirection(const wxString& param, wxDirection dir = wxLEFT)
    {
        return GetImpl()->GetDirection(param, dir);
    }
    wxBitmap GetBitmap(const wxString& param = "bitmap",
                       const wxArtClient& defaultArtClient = wxART_OTHER,
                       wxSize size = wxDefaultSize)
    {
        return GetImpl()->GetBitmap(param, defaultArtClient, size);
    }
    wxBitmap GetBitmap(const wxXmlNode* node,
                       const wxArtClient& defaultArtClient = wxART_OTHER,
                       wxSize size = wxDefaultSize)
    {
        return GetImpl()->GetBitmap(node, defaultArtClient, size);
    }
    wxIcon GetIcon(const wxString& param = "icon",
                   const wxArtClient& defaultArtClient = wxART_OTHER,
                   wxSize size = wxDefaultSize)
    {
        return GetImpl()->GetIcon(param, defaultArtClient, size);
    }
    wxIcon GetIcon(const wxXmlNode* node,
                   const wxArtClient& defaultArtClient = wxART_OTHER,
                   wxSize size = wxDefaultSize)
    {
        return GetImpl()->GetIcon(node, defaultArtClient, size);
    }
    wxIconBundle GetIconBundle(const wxString& param,
                               const wxArtClient& defaultArtClient = wxART_OTHER)
    {
        return GetImpl()->GetIconBundle(param, defaultArtClient);
    }
    wxImageList *GetImageList(const wxString& param = "imagelist")
    {
        return GetImpl()->GetImageList(param);
    }

#if wxUSE_ANIMATIONCTRL
    wxAnimation* GetAnimation(const wxString& param = "animation",
                              wxAnimationCtrlBase* ctrl = nullptr)
    {
        return GetImpl()->GetAnimation(param, ctrl);
    }
#endif

    wxFont GetFont(const wxString& param = "font",
                   wxWindow* parent = nullptr)
    {
        return GetImpl()->GetFont(param, parent);
    }
    bool GetBoolAttr(const wxString& attr, bool defaultv)
    {
        return GetImpl()->GetBoolAttr(attr, defaultv);
    }
    wxString GetFilePath(const wxXmlNode* node)
    {
        return GetImpl()->GetFilePath(node);
    }
    void SetupWindow(wxWindow *wnd)
    {
        GetImpl()->SetupWindow(wnd);
    }
    void CreateChildren(wxObject *parent, bool this_hnd_only = false)
    {
        GetImpl()->CreateChildren(parent, this_hnd_only);
    }
    void CreateChildrenPrivately(wxObject *parent, wxXmlNode *rootnode = nullptr)
    {
        GetImpl()->CreateChildrenPrivately(parent, rootnode);
    }
    wxObject *CreateResFromNode(wxXmlNode *node,
                                wxObject *parent, wxObject *instance = nullptr)
    {
        return GetImpl()->CreateResFromNode(node, parent, instance);
    }

#if wxUSE_FILESYSTEM
    wxFileSystem& GetCurFileSystem()
    {
        return GetImpl()->GetCurFileSystem();
    }
#endif

    // Variables (filled by CreateResource)
    wxXmlNode *m_node;
    wxString m_class;
    wxObject *m_parent, *m_instance;
    wxWindow *m_parentAsWindow;
    wxXmlResource *m_resource;

    // provide method access to those member variables
    wxXmlResource* GetResource() const        { return m_resource; }
    wxXmlNode* GetNode() const                { return m_node; }
    wxString GetClass() const                 { return m_class; }
    wxObject* GetParent() const               { return m_parent; }
    wxObject* GetInstance() const             { return m_instance; }
    wxWindow* GetParentAsWindow() const       { return m_parentAsWindow; }


    std::vector<wxString> m_styleNames;
    std::vector<int> m_styleValues;

    friend class wxXmlResourceHandlerImpl;

private:
    // This is supposed to never return NULL because SetImpl() should have been
    // called.
    wxXmlResourceHandlerImplBase* GetImpl() const;

    wxXmlResourceHandlerImplBase *m_impl;

    wxDECLARE_ABSTRACT_CLASS(wxXmlResourceHandler);
};

#endif // wxUSE_XRC

#endif // _WX_XRC_XMLRESHANDLER_H_
