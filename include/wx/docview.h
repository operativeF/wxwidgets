/////////////////////////////////////////////////////////////////////////////
// Name:        wx/docview.h
// Purpose:     Doc/View classes
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DOCH__
#define _WX_DOCH__

#if wxUSE_DOC_VIEW_ARCHITECTURE

#include "wx/list.h"
#include "wx/dlist.h"
#include "wx/cmdproc.h"


#if wxUSE_PRINTING_ARCHITECTURE
    #include "wx/print.h"
#endif

#include <memory>

import <filesystem>;
import <iosfwd>;
import <string>;
import <vector>;

class wxFileHistory;
class wxFrame;
class wxWindow;
class wxDocument;
class wxView;
class wxDocTemplate;
class wxDocManager;
class wxPrintInfo;
class wxConfigBase;

class wxDocChildFrameAnyBase;

namespace fs = std::filesystem;

// Flags for wxDocManager (can be combined).
enum
{
    wxDOC_NEW    = 1,
    wxDOC_SILENT = 2
};

// Document template flags
enum
{
    wxTEMPLATE_VISIBLE = 1,
    wxTEMPLATE_INVISIBLE = 2,
    wxDEFAULT_TEMPLATE_FLAGS = wxTEMPLATE_VISIBLE
};

// TODO: Should this be adjustable?
inline constexpr unsigned int wxMAX_FILE_HISTORY = 9;

using wxDocVector = std::vector<wxDocument *>;
using wxViewVector = std::vector<wxView *>;
using wxDocTemplateVector = std::vector<wxDocTemplate *>;

class wxDocument : public wxEvtHandler
{
public:
    wxDocument(wxDocument *parent = nullptr);
    ~wxDocument();

    wxDocument& operator=(wxDocument&&) = delete;

    void SetFilename(const fs::path& filename, bool notifyViews = false);
    fs::path GetFilename() const { return m_documentFile.filename(); }

    void SetTitle(const std::string& title) { m_documentTitle = title; }
    std::string GetTitle() const { return m_documentTitle; }

    void SetDocumentName(std::string name) { m_documentTypeName = name; }
    std::string GetDocumentName() const { return m_documentTypeName; }

    // access the flag indicating whether this document had been already saved,
    // SetDocumentSaved() is only used internally, don't call it
    bool GetDocumentSaved() const { return m_savedYet; }
    void SetDocumentSaved(bool saved = true) { m_savedYet = saved; }

    // activate the first view of the document if any
    void Activate();

    // return true if the document hasn't been modified since the last time it
    // was saved (implying that it returns false if it was never saved, even if
    // the document is not modified)
    bool AlreadySaved() const { return !IsModified() && GetDocumentSaved(); }

    virtual bool Close();
    virtual bool Save();
    virtual bool SaveAs();
    virtual bool Revert();

    virtual std::ostream& SaveObject(std::ostream& stream);
    virtual std::istream& LoadObject(std::istream& stream);

    // Called by wxWidgets
    virtual bool OnSaveDocument(const fs::path& filename);
    virtual bool OnOpenDocument(const fs::path& filename);
    virtual bool OnNewDocument();
    virtual bool OnCloseDocument();

    // Prompts for saving if about to close a modified document. Returns true
    // if ok to close the document (may have saved in the meantime, or set
    // modified to false)
    virtual bool OnSaveModified();

    // if you override, remember to call the default
    // implementation (wxDocument::OnChangeFilename)
    virtual void OnChangeFilename(bool notifyViews);

    // Called by framework if created automatically by the default document
    // manager: gives document a chance to initialise and (usually) create a
    // view
    virtual bool OnCreate(const fs::path& path, unsigned int flags);

    // By default, creates a base wxCommandProcessor.
    virtual std::unique_ptr<wxCommandProcessor> OnCreateCommandProcessor();
    virtual wxCommandProcessor* GetCommandProcessor() const
        { return m_commandProcessor.get(); }
    virtual void SetCommandProcessor(std::unique_ptr<wxCommandProcessor> proc)
        { m_commandProcessor = std::move(proc); }

    // Called after a view is added or removed. The default implementation
    // deletes the document if this is there are no more views.
    virtual void OnChangedViewList();

    // Called from OnCloseDocument(), does nothing by default but may be
    // overridden. Return value is ignored.
    virtual bool DeleteContents();

    virtual bool Draw(wxDC&);
    virtual bool IsModified() const { return m_documentModified; }
    virtual void Modify(bool mod);

    virtual bool AddView(wxView *view);
    virtual bool RemoveView(wxView *view);

    wxViewVector GetViewsVector() const;

    wxList& GetViews() { return m_documentViews; }
    const wxList& GetViews() const { return m_documentViews; }

    wxView *GetFirstView() const;

    virtual void UpdateAllViews(wxView *sender = nullptr, wxObject *hint = nullptr);
    virtual void NotifyClosing();

    // Remove all views (because we're closing the document)
    virtual bool DeleteAllViews();

    // Other stuff
    virtual wxDocManager *GetDocumentManager() const;
    virtual wxDocTemplate *GetDocumentTemplate() const
        { return m_documentTemplate; }
    virtual void SetDocumentTemplate(wxDocTemplate *temp)
        { m_documentTemplate = temp; }

    // Get the document name to be shown to the user: the title if there is
    // any, otherwise the filename if the document was saved and, finally,
    // "unnamed" otherwise
    virtual std::string GetUserReadableName() const;

    // Returns a window that can be used as a parent for document-related
    // dialogs. Override if necessary.
    virtual wxWindow *GetDocumentWindow() const;

    // Returns true if this document is a child document corresponding to a
    // part of the parent document and not a disk file as usual.
    bool IsChildDocument() const { return m_documentParent != nullptr; }

protected:
    wxList                m_documentViews;
    fs::path              m_documentFile;
    std::string           m_documentTitle;
    std::string           m_documentTypeName;
    wxDocTemplate*        m_documentTemplate{nullptr};
    bool                  m_documentModified{false};

    // if the document parent is non-NULL, it's a pseudo-document corresponding
    // to a part of the parent document which can't be saved or loaded
    // independently of its parent and is always closed when its parent is
    wxDocument*           m_documentParent;

    std::unique_ptr<wxCommandProcessor>   m_commandProcessor;
    bool                  m_savedYet{false};

    // Called by OnSaveDocument and OnOpenDocument to implement standard
    // Save/Load behaviour. Re-implement in derived class for custom
    // behaviour.
    virtual bool DoSaveDocument(const fs::path& file);
    virtual bool DoOpenDocument(const fs::path& file);

    // the default implementation of GetUserReadableName()
    std::string DoGetUserReadableName() const;

private:
    // list of all documents whose m_documentParent is this one
    using DocsList = wxDList<wxDocument>;
    DocsList m_childDocuments;

    wxDECLARE_ABSTRACT_CLASS(wxDocument);
};

class wxView: public wxEvtHandler
{
public:
    ~wxView();

    wxView& operator=(wxView&&) = delete;

    wxDocument *GetDocument() const { return m_viewDocument; }
    virtual void SetDocument(wxDocument *doc);

    std::string GetViewName() const { return m_viewTypeName; }
    void SetViewName(std::string name) { m_viewTypeName = name; }

    wxWindow *GetFrame() const { return m_viewFrame ; }
    void SetFrame(wxWindow *frame) { m_viewFrame = frame; }

    virtual void OnActivateView(bool activate,
                                wxView *activeView,
                                wxView *deactiveView);
    virtual void OnDraw(wxDC *dc) = 0;
    virtual void OnPrint(wxDC *dc, wxObject *info);
    virtual void OnUpdate(wxView *sender, wxObject *hint = nullptr);
    virtual void OnClosingDocument() {}
    virtual void OnChangeFilename();

    // Called by framework if created automatically by the default document
    // manager class: gives view a chance to initialise
    virtual bool OnCreate([[maybe_unused]] wxDocument *doc, [[maybe_unused]] unsigned int flags)
        { return true; }

    // Checks if the view is the last one for the document; if so, asks user
    // to confirm save data (if modified). If ok, deletes itself and returns
    // true.
    virtual bool Close(bool deleteWindow = true);

    // Override to do cleanup/veto close
    virtual bool OnClose(bool deleteWindow);

    // A view's window can call this to notify the view it is (in)active.
    // The function then notifies the document manager.
    virtual void Activate(bool activate);

    wxDocManager *GetDocumentManager() const
        { return m_viewDocument->GetDocumentManager(); }

#if wxUSE_PRINTING_ARCHITECTURE
    virtual std::unique_ptr<wxPrintout> OnCreatePrintout();
#endif

    // implementation only
    // -------------------

    // set the associated frame, it is used to reset its view when we're
    // destroyed
    void SetDocChildFrame(wxDocChildFrameAnyBase *docChildFrame);

    // get the associated frame, may be NULL during destruction
    wxDocChildFrameAnyBase* GetDocChildFrame() const { return m_docChildFrame; }

protected:
    // hook the document into event handlers chain here
    bool TryBefore(wxEvent& event) override;

    wxDocument*       m_viewDocument{nullptr};
    std::string          m_viewTypeName;
    wxWindow*         m_viewFrame{nullptr};

    wxDocChildFrameAnyBase *m_docChildFrame{nullptr};

private:
    wxDECLARE_ABSTRACT_CLASS(wxView);
};

// Represents user interface (and other) properties of documents and views
class wxDocTemplate: public wxObject
{

friend class wxDocManager;

public:
    // Associate document and view types. They're for identifying what view is
    // associated with what template/document type
    wxDocTemplate(wxDocManager *manager,
                  const std::string& descr,
                  const std::string& filter,
                  const std::string& dir,
                  const std::string& ext,
                  const std::string& docTypeName,
                  const std::string& viewTypeName,
                  wxClassInfo *docClassInfo = nullptr,
                  wxClassInfo *viewClassInfo = nullptr,
                  unsigned int flags = wxDEFAULT_TEMPLATE_FLAGS);

    ~wxDocTemplate();

    wxDocTemplate& operator=(wxDocTemplate&&) = delete;

    // By default, these two member functions dynamically creates document and
    // view using dynamic instance construction. Override these if you need a
    // different method of construction.
    virtual wxDocument *CreateDocument(const fs::path& path, unsigned int flags = 0);
    virtual wxView *CreateView(wxDocument *doc, unsigned int flags = 0);

    // Helper method for CreateDocument; also allows you to do your own document
    // creation
    virtual bool InitDocument(wxDocument* doc,
                              const fs::path& path,
                              unsigned int flags = 0);

    std::string GetDefaultExtension() const { return m_defaultExt; }
    std::string GetDescription() const { return m_description; }
    fs::path GetDirectory() const { return m_directory; }
    wxDocManager *GetDocumentManager() const { return m_documentManager; }
    void SetDocumentManager(wxDocManager *manager)
        { m_documentManager = manager; }
    std::string GetFileFilter() const { return m_fileFilter; }
    unsigned int GetFlags() const { return m_flags; }
    virtual std::string GetViewName() const { return m_viewTypeName; }
    virtual std::string GetDocumentName() const { return m_docTypeName; }

    void SetFileFilter(const std::string& filter) { m_fileFilter = filter; }
    void SetDirectory(const fs::path& dir) { m_directory = dir; }
    void SetDescription(const std::string& descr) { m_description = descr; }
    void SetDefaultExtension(const std::string& ext) { m_defaultExt = ext; }
    void SetFlags(unsigned int flags) { m_flags = flags; }

    bool IsVisible() const { return (m_flags & wxTEMPLATE_VISIBLE) != 0; }

    wxClassInfo* GetDocClassInfo() const { return m_docClassInfo; }
    wxClassInfo* GetViewClassInfo() const { return m_viewClassInfo; }

    virtual bool FileMatchesTemplate(const fs::path& path);

protected:
    fs::path             m_directory;

    std::string          m_fileFilter;
    std::string          m_description;
    std::string          m_defaultExt;
    std::string          m_docTypeName;
    std::string          m_viewTypeName;

    wxDocManager*     m_documentManager;

    // For dynamic creation of appropriate instances.
    wxClassInfo*      m_docClassInfo;
    wxClassInfo*      m_viewClassInfo;

    unsigned int         m_flags;

    // Called by CreateDocument and CreateView to create the actual
    // document/view object.
    //
    // By default uses the ClassInfo provided to the constructor. Override
    // these functions to provide a different method of creation.
    virtual wxDocument *DoCreateDocument();
    virtual wxView *DoCreateView();

private:
    wxDECLARE_CLASS(wxDocTemplate);
};

// One object of this class may be created in an application, to manage all
// the templates and documents.
class wxDocManager: public wxEvtHandler
{
public:
    // NB: flags are unused, don't pass wxDOC_XXX to this ctor
    wxDocManager(unsigned int flags = 0, bool initialize = true);
    ~wxDocManager();

    wxDocManager& operator=(wxDocManager&&) = delete;

    virtual bool Initialize();

    // Handlers for common user commands
    void OnFileClose(wxCommandEvent& event);
    void OnFileCloseAll(wxCommandEvent& event);
    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileRevert(wxCommandEvent& event);
    void OnFileSave(wxCommandEvent& event);
    void OnFileSaveAs(wxCommandEvent& event);
    void OnMRUFile(wxCommandEvent& event);
#if wxUSE_PRINTING_ARCHITECTURE
    void OnPrint(wxCommandEvent& event);
    void OnPreview(wxCommandEvent& event);
    void OnPageSetup(wxCommandEvent& event);
#endif // wxUSE_PRINTING_ARCHITECTURE
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);

    // Handlers for UI update commands
    void OnUpdateFileOpen(wxUpdateUIEvent& event);
    void OnUpdateDisableIfNoDoc(wxUpdateUIEvent& event);
    void OnUpdateFileRevert(wxUpdateUIEvent& event);
    void OnUpdateFileNew(wxUpdateUIEvent& event);
    void OnUpdateFileSave(wxUpdateUIEvent& event);
    void OnUpdateFileSaveAs(wxUpdateUIEvent& event);
    void OnUpdateUndo(wxUpdateUIEvent& event);
    void OnUpdateRedo(wxUpdateUIEvent& event);

    // called when file format detection didn't work, can be overridden to do
    // something in this case
    virtual void OnOpenFileFailure() { }

    virtual wxDocument *CreateDocument(const fs::path& path, unsigned int flags = 0);

    // wrapper around CreateDocument() with a more clear name
    wxDocument *CreateNewDocument()
        { return CreateDocument("", wxDOC_NEW); }

    virtual wxView *CreateView(wxDocument *doc, unsigned int flags = 0);
    virtual void DeleteTemplate(wxDocTemplate *temp, unsigned int flags = 0);
    virtual bool FlushDoc(wxDocument *doc);
    virtual wxDocTemplate *MatchTemplate(const fs::path& path);
    virtual wxDocTemplate *SelectDocumentPath(wxDocTemplate **templates,
            int noTemplates, fs::path& path, unsigned int flags, bool save = false);
    virtual wxDocTemplate *SelectDocumentType(wxDocTemplate **templates,
            int noTemplates, bool sort = false);
    virtual wxDocTemplate *SelectViewType(wxDocTemplate **templates,
            int noTemplates, bool sort = false);
    virtual wxDocTemplate *FindTemplateForPath(const fs::path& path);

    void AssociateTemplate(wxDocTemplate *temp);
    void DisassociateTemplate(wxDocTemplate *temp);

    // Find template from document class info, may return NULL.
    wxDocTemplate* FindTemplate(const wxClassInfo* documentClassInfo);

    // Find document from file name, may return NULL.
    wxDocument* FindDocumentByPath(const fs::path& path) const;

    wxDocument *GetCurrentDocument() const;

    void SetMaxDocsOpen(int n) { m_maxDocsOpen = n; }
    int GetMaxDocsOpen() const { return m_maxDocsOpen; }

    // Add and remove a document from the manager's list
    void AddDocument(wxDocument *doc);
    void RemoveDocument(wxDocument *doc);

    // closes all currently open documents
    bool CloseDocuments(bool force = true);

    // closes the specified document
    bool CloseDocument(wxDocument* doc, bool force = false);

    // Clear remaining documents and templates
    bool Clear(bool force = true);

    // Views or windows should inform the document manager
    // when a view is going in or out of focus
    virtual void ActivateView(wxView *view, bool activate = true);
    virtual wxView *GetCurrentView() const { return m_currentView; }

    // This method tries to find an active view harder than GetCurrentView():
    // if the latter is NULL, it also checks if we don't have just a single
    // view and returns it then.
    wxView *GetAnyUsableView() const;


    wxDocVector GetDocumentsVector() const;
    wxDocTemplateVector GetTemplatesVector() const;

    wxList& GetDocuments() { return m_docs; }
    wxList& GetTemplates() { return m_templates; }

    // Return the default name for a new document (by default returns strings
    // in the form "unnamed <counter>" but can be overridden)
    virtual std::string MakeNewDocumentName();

    // Make a frame title (override this to do something different)
    virtual std::string MakeFrameTitle(wxDocument* doc);

    virtual wxFileHistory *OnCreateFileHistory();
    virtual wxFileHistory *GetFileHistory() const { return m_fileHistory; }

    // File history management
    virtual void AddFileToHistory(const fs::path& file);
    virtual void RemoveFileFromHistory(size_t i);
    virtual size_t GetHistoryFilesCount() const;
    virtual fs::path GetHistoryFile(size_t i) const;
    virtual void FileHistoryUseMenu(wxMenu *menu);
    virtual void FileHistoryRemoveMenu(wxMenu *menu);
#if wxUSE_CONFIG
    virtual void FileHistoryLoad(const wxConfigBase& config);
    virtual void FileHistorySave(wxConfigBase& config);
#endif // wxUSE_CONFIG

    virtual void FileHistoryAddFilesToMenu();
    virtual void FileHistoryAddFilesToMenu(wxMenu* menu);

    fs::path GetLastDirectory() const;
    void SetLastDirectory(const std::string& dir) { m_lastDirectory = dir; }

    // Get the current document manager
    static wxDocManager* GetDocumentManager() { return sm_docManager; }

#if wxUSE_PRINTING_ARCHITECTURE
    wxPageSetupDialogData& GetPageSetupDialogData()
        { return m_pageSetupDialogData; }
    const wxPageSetupDialogData& GetPageSetupDialogData() const
        { return m_pageSetupDialogData; }
#endif // wxUSE_PRINTING_ARCHITECTURE

protected:
    // Called when a file selected from the MRU list doesn't exist any more.
    // The default behaviour is to remove the file from the MRU and notify the
    // user about it but this method can be overridden to customize it.
    virtual void OnMRUFileNotExist(unsigned n, const fs::path& filename);

    // Open the MRU file with the given index in our associated file history.
    void DoOpenMRUFile(unsigned n);
#if wxUSE_PRINTING_ARCHITECTURE
    virtual wxPreviewFrame* CreatePreviewFrame(wxPrintPreviewBase* preview,
                                               wxWindow *parent,
                                               const std::string& title);
#endif // wxUSE_PRINTING_ARCHITECTURE

    // hook the currently active view into event handlers chain here
    bool TryBefore(wxEvent& event) override;

    // return the command processor for the current document, if any
    wxCommandProcessor *GetCurrentCommandProcessor() const;

    int               m_defaultDocumentNameCounter{1};
    int               m_maxDocsOpen{INT_MAX};
    wxList            m_docs;
    wxList            m_templates;
    wxView*           m_currentView{nullptr};
    wxFileHistory*    m_fileHistory{nullptr};
    fs::path          m_lastDirectory;
    inline static wxDocManager* sm_docManager{nullptr};

#if wxUSE_PRINTING_ARCHITECTURE
    wxPageSetupDialogData m_pageSetupDialogData;
#endif // wxUSE_PRINTING_ARCHITECTURE

    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// Base class for child frames -- this is what wxView renders itself into
//
// Notice that this is a mix-in class so it doesn't derive from wxWindow, only
// wxDocChildFrameAny does
// ----------------------------------------------------------------------------

class wxDocChildFrameAnyBase
{
public:
    // default ctor, use Create() after it
    wxDocChildFrameAnyBase()
    {
        m_childDocument = nullptr;
        m_childView = nullptr;
        m_win = nullptr;
        m_lastEvent = nullptr;
    }

    // full ctor equivalent to using the default one and Create()
    wxDocChildFrameAnyBase(wxDocument *doc, wxView *view, wxWindow *win)
    {
        Create(doc, view, win);
    }

    // method which must be called for an object created using the default ctor
    //
    // note that it returns bool just for consistency with Create() methods in
    // other classes, we never return false from here
    [[maybe_unused]] bool Create(wxDocument *doc, wxView *view, wxWindow *win)
    {
        m_childDocument = doc;
        m_childView = view;
        m_win = win;

        if ( view )
            view->SetDocChildFrame(this);

        return true;
    }

    // dtor doesn't need to be virtual, an object should never be destroyed via
    // a pointer to this class
    ~wxDocChildFrameAnyBase()
    {
        // prevent the view from deleting us if we're being deleted directly
        // (and not via Close() + Destroy())
        if ( m_childView )
            m_childView->SetDocChildFrame(nullptr);
    }

    wxDocChildFrameAnyBase& operator=(wxDocChildFrameAnyBase&&) = delete;

    wxDocument *GetDocument() const { return m_childDocument; }
    wxView *GetView() const { return m_childView; }
    void SetDocument(wxDocument *doc) { m_childDocument = doc; }
    void SetView(wxView *view) { m_childView = view; }

    wxWindow *GetWindow() const { return m_win; }

    // implementation only

    // Check if this event had been just processed in this frame.
    bool HasAlreadyProcessed(wxEvent& event) const
    {
        return m_lastEvent == &event;
    }

protected:
    // we're not a wxEvtHandler but we provide this wxEvtHandler-like function
    // which is called from TryBefore() of the derived classes to give our view
    // a chance to process the message before the frame event handlers are used
    bool TryProcessEvent(wxEvent& event);

    // called from EVT_CLOSE handler in the frame: check if we can close and do
    // cleanup if so; veto the event otherwise
    bool CloseView(wxCloseEvent& event);


    wxDocument*       m_childDocument;
    wxView*           m_childView;

    // the associated window: having it here is not terribly elegant but it
    // allows us to avoid having any virtual functions in this class
    wxWindow* m_win;

private:
    // Pointer to the last processed event used to avoid sending the same event
    // twice to wxDocManager, from here and from wxDocParentFrameAnyBase.
    wxEvent* m_lastEvent;
};

// ----------------------------------------------------------------------------
// Template implementing child frame concept using the given wxFrame-like class
//
// This is used to define wxDocChildFrame and wxDocMDIChildFrame: ChildFrame is
// a wxFrame or wxMDIChildFrame (although in theory it could be any wxWindow-
// derived class as long as it provided a ctor with the same signature as
// wxFrame and OnActivate() method) and ParentFrame is either wxFrame or
// wxMDIParentFrame.
// ----------------------------------------------------------------------------

// Note that we intentionally do not use for this class as it
// has only inline methods.

template <class ChildFrame, class ParentFrame>
class wxDocChildFrameAny : public ChildFrame,
                           public wxDocChildFrameAnyBase
{
public:
    using BaseClass = ChildFrame;

    // default ctor, use Create after it
    wxDocChildFrameAny() = default;

    // ctor for a frame showing the given view of the specified document
    wxDocChildFrameAny(wxDocument *doc,
                       wxView *view,
                       ParentFrame *parent,
                       wxWindowID id,
                       const std::string& title,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       unsigned int style = wxDEFAULT_FRAME_STYLE,
                       std::string_view name = wxFrameNameStr)
    {
        Create(doc, view, parent, id, title, pos, size, style, name);
    }

    wxDocChildFrameAny& operator=(wxDocChildFrameAny<ChildFrame, ParentFrame>&&) = delete;

    [[maybe_unused]] bool Create(wxDocument *doc,
                wxView *view,
                ParentFrame *parent,
                wxWindowID id,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE,
                std::string_view name = wxFrameNameStr)
    {
        if ( !wxDocChildFrameAnyBase::Create(doc, view, this) )
            return false;

        if ( !BaseClass::Create(parent, id, title, pos, size, style, name) )
            return false;

        this->Bind(wxEVT_ACTIVATE, &wxDocChildFrameAny::OnActivate, this);
        this->Bind(wxEVT_CLOSE_WINDOW, &wxDocChildFrameAny::OnCloseWindow, this);

        return true;
    }

protected:
    // hook the child view into event handlers chain here
    bool TryBefore(wxEvent& event) override
    {
        return TryProcessEvent(event) || BaseClass::TryBefore(event);
    }

private:
    void OnActivate(wxActivateEvent& event)
    {
        BaseClass::OnActivate(event);

        if ( m_childView )
            m_childView->Activate(event.GetActive());
    }

    void OnCloseWindow(wxCloseEvent& event)
    {
        if ( CloseView(event) )
            this->Destroy();
        //else: vetoed
    }
};

// ----------------------------------------------------------------------------
// A default child frame: we need to define it as a class just for wxRTTI,
// otherwise we could simply typedef it
// ----------------------------------------------------------------------------

using wxDocChildFrameBase = wxDocChildFrameAny<wxFrame, wxFrame>;

class wxDocChildFrame : public wxDocChildFrameBase
{
public:
    wxDocChildFrame() = default;

    wxDocChildFrame(wxDocument *doc,
                    wxView *view,
                    wxFrame *parent,
                    wxWindowID id,
                    const std::string& title,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    unsigned int style = wxDEFAULT_FRAME_STYLE,
                    std::string_view name = wxFrameNameStr)
        : wxDocChildFrameBase(doc, view,
                              parent, id, title, pos, size, style, name)
    {
    }

    wxDocChildFrame& operator=(wxDocChildFrame&&) = delete;

    [[maybe_unused]] bool Create(wxDocument *doc,
                wxView *view,
                wxFrame *parent,
                wxWindowID id,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE,
                std::string_view name = wxFrameNameStr)
    {
        return wxDocChildFrameBase::Create
               (
                    doc, view,
                    parent, id, title, pos, size, style, name
               );
    }

private:
    wxDECLARE_CLASS(wxDocChildFrame);
};

// ----------------------------------------------------------------------------
// wxDocParentFrame and related classes.
//
// As with wxDocChildFrame we define a template base class used by both normal
// and MDI versions
// ----------------------------------------------------------------------------

// Base class containing type-independent code of wxDocParentFrameAny
//
// Similarly to wxDocChildFrameAnyBase, this class is a mix-in and doesn't
// derive from wxWindow.
class wxDocParentFrameAnyBase
{
public:
    wxDocParentFrameAnyBase(wxWindow* frame)
        : m_frame(frame)
    {
        m_docManager = nullptr;
    }

    wxDocParentFrameAnyBase& operator=(wxDocParentFrameAnyBase&&) = delete;

    wxDocManager *GetDocumentManager() const { return m_docManager; }

protected:
    // This is similar to wxDocChildFrameAnyBase method with the same name:
    // while we're not an event handler ourselves and so can't override
    // TryBefore(), we provide a helper that the derived template class can use
    // from its TryBefore() implementation.
    bool TryProcessEvent(wxEvent& event);

    wxWindow* const m_frame;
    wxDocManager *m_docManager;
};

// This is similar to wxDocChildFrameAny and is used to provide common
// implementation for both wxDocParentFrame and wxDocMDIParentFrame
template <class BaseFrame>
class wxDocParentFrameAny : public BaseFrame,
                            public wxDocParentFrameAnyBase
{
public:
    wxDocParentFrameAny() : wxDocParentFrameAnyBase(this) { }
    wxDocParentFrameAny(wxDocManager *manager,
                        wxFrame *frame,
                        wxWindowID id,
                        const std::string& title,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        unsigned int style = wxDEFAULT_FRAME_STYLE,
                        std::string_view name = wxFrameNameStr)
        : wxDocParentFrameAnyBase(this)
    {
        Create(manager, frame, id, title, pos, size, style, name);
    }

    wxDocParentFrameAny& operator=(wxDocParentFrameAny&&) = delete;

    [[maybe_unused]] bool Create(wxDocManager *manager,
                wxFrame *frame,
                wxWindowID id,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE,
                std::string_view name = wxFrameNameStr)
    {
        m_docManager = manager;

        if ( !BaseFrame::Create(frame, id, title, pos, size, style, name) )
            return false;

        this->Bind(wxEVT_MENU, &wxDocParentFrameAny::OnExit, this, wxID_EXIT);
        this->Bind(wxEVT_CLOSE_WINDOW, &wxDocParentFrameAny::OnCloseWindow, this);

        return true;
    }

protected:
    // hook the document manager into event handling chain here
    bool TryBefore(wxEvent& event) override
    {
        // It is important to send the event to the base class first as
        // wxMDIParentFrame overrides its TryBefore() to send the menu events
        // to the currently active child frame and the child must get them
        // before our own TryProcessEvent() is executed, not afterwards.
        return BaseFrame::TryBefore(event) || TryProcessEvent(event);
    }

private:
    void OnExit([[maybe_unused]] wxCommandEvent& event)
    {
        this->Close();
    }

    void OnCloseWindow(wxCloseEvent& event)
    {
        if ( m_docManager && !m_docManager->Clear(!event.CanVeto()) )
        {
            // The user decided not to close finally, abort.
            event.Veto();
        }
        else
        {
            // Just skip the event, base class handler will destroy the window.
            event.Skip();
        }
    }
};

using wxDocParentFrameBase = wxDocParentFrameAny<wxFrame>;

class wxDocParentFrame : public wxDocParentFrameBase
{
public:
    wxDocParentFrame()  = default;

    wxDocParentFrame(wxDocManager *manager,
                     wxFrame *parent,
                     wxWindowID id,
                     const std::string& title,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxDEFAULT_FRAME_STYLE,
                     std::string_view name = wxFrameNameStr)
        : wxDocParentFrameBase(manager,
                               parent, id, title, pos, size, style, name)
    {
    }

    wxDocParentFrame& operator=(wxDocParentFrame&&) = delete;

    [[maybe_unused]] bool Create(wxDocManager *manager,
                wxFrame *parent,
                wxWindowID id,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE,
                std::string_view name = wxFrameNameStr)
    {
        return wxDocParentFrameBase::Create(manager,
                                            parent, id, title,
                                            pos, size, style, name);
    }

private:
    wxDECLARE_CLASS(wxDocParentFrame);
};

// ----------------------------------------------------------------------------
// Provide simple default printing facilities
// ----------------------------------------------------------------------------

#if wxUSE_PRINTING_ARCHITECTURE
class wxDocPrintout : public wxPrintout
{
public:
    wxDocPrintout(wxView *view = nullptr, const std::string& title = {});

    wxDocPrintout& operator=(wxDocPrintout&&) = delete;

    // implement wxPrintout methods
    bool OnPrintPage(int page) override;
    bool HasPage(int page) override;
    bool OnBeginDocument(int startPage, int endPage) override;
    void GetPageInfo(int *minPage, int *maxPage,
                             int *selPageFrom, int *selPageTo) override;

    virtual wxView *GetView() { return m_printoutView; }

protected:
    wxView*       m_printoutView;

private:
    wxDECLARE_DYNAMIC_CLASS(wxDocPrintout);
};
#endif // wxUSE_PRINTING_ARCHITECTURE

// For compatibility with existing file formats:
// converts from/to a stream to/from a temporary file.
bool
wxTransferFileToStream(const std::string& filename, std::ostream& stream);
bool
wxTransferStreamToFile(std::istream& stream, const std::string& filename);
#else
bool
wxTransferFileToStream(const std::string& filename, wxOutputStream& stream);
bool
wxTransferStreamToFile(wxInputStream& stream, const std::string& filename);

inline wxViewVector wxDocument::GetViewsVector() const
{
    return m_documentViews.AsVector<wxView*>();
}

inline wxDocVector wxDocManager::GetDocumentsVector() const
{
    return m_docs.AsVector<wxDocument*>();
}

inline wxDocTemplateVector wxDocManager::GetTemplatesVector() const
{
    return m_templates.AsVector<wxDocTemplate*>();
}

#endif // wxUSE_DOC_VIEW_ARCHITECTURE

#endif // _WX_DOCH__
