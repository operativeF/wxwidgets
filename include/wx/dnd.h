///////////////////////////////////////////////////////////////////////////////
// Name:        wx/dnd.h
// Purpose:     Drag and drop classes declarations
// Author:      Vadim Zeitlin, Robert Roebling
// Modified by:
// Created:     26.05.99
// Copyright:   (c) wxWidgets Team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DND_H_BASE_
#define _WX_DND_H_BASE_

#include "wx/defs.h"

#if wxUSE_DRAG_AND_DROP

#include "wx/dataobj.h"
#include "wx/cursor.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// flags for wxDropSource::DoDragDrop()
//
// NB: wxDrag_CopyOnly must be 0 (== FALSE) and wxDrag_AllowMove must be 1
//     (== TRUE) for compatibility with the old DoDragDrop(bool) method!
enum
{
    wxDrag_CopyOnly    = 0, // allow only copying
    wxDrag_AllowMove   = 1, // allow moving (copying is always allowed)
    wxDrag_DefaultMove = 3  // the default operation is move, not copy
};

// result of wxDropSource::DoDragDrop() call
enum class wxDragResult
{
    Error,    // error prevented the d&d operation from completing
    None,     // drag target didn't accept the data
    Copy,     // the data was successfully copied
    Move,     // the data was successfully moved (MSW only)
    Link,     // operation is a drag-link
    Cancel    // the operation was cancelled by user (not an error)
};

// return true if res indicates that something was done during a dnd operation,
// i.e. is neither error nor none nor cancel
WXDLLIMPEXP_CORE bool wxIsDragResultOk(wxDragResult res);

// ----------------------------------------------------------------------------
// wxDropSource is the object you need to create (and call DoDragDrop on it)
// to initiate a drag-and-drop operation
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxDropSourceBase
{
public:
    wxDropSourceBase(const wxCursor &cursorCopy = wxNullCursor,
                     const wxCursor &cursorMove = wxNullCursor,
                     const wxCursor &cursorStop = wxNullCursor)
        : m_cursorCopy(cursorCopy),
          m_cursorMove(cursorMove),
          m_cursorStop(cursorStop)
        { m_data = nullptr; }
    virtual ~wxDropSourceBase() = default;

   wxDropSourceBase(const wxDropSourceBase&) = delete;
   wxDropSourceBase& operator=(const wxDropSourceBase&) = delete;
   wxDropSourceBase(wxDropSourceBase&&) = default;
   wxDropSourceBase& operator=(wxDropSourceBase&&) = default;

    // set the data which is transferred by drag and drop
    void SetData(wxDataObject& data)
      { m_data = &data; }

    wxDataObject *GetDataObject()
      { return m_data; }

    // set the icon corresponding to given drag result
    void SetCursor(wxDragResult res, const wxCursor& cursor)
    {
        if ( res == wxDragResult::Copy )
            m_cursorCopy = cursor;
        else if ( res == wxDragResult::Move )
            m_cursorMove = cursor;
        else
            m_cursorStop = cursor;
    }

    // start drag action, see enum wxDragResult for return value description
    //
    // if flags contains wxDrag_AllowMove, moving (and only copying) data is
    // allowed, if it contains wxDrag_DefaultMove (which includes the previous
    // flag), it is even the default operation
    virtual wxDragResult DoDragDrop(int flags = wxDrag_CopyOnly) = 0;

    // override to give feedback depending on the current operation result
    // "effect" and return true if you did something, false to let the library
    // give the default feedback
    virtual bool GiveFeedback(wxDragResult WXUNUSED(effect)) { return false; }

protected:
    const wxCursor& GetCursor(wxDragResult res) const
    {
        if ( res == wxDragResult::Copy )
            return m_cursorCopy;
        else if ( res == wxDragResult::Move )
            return m_cursorMove;
        else
            return m_cursorStop;
    }

    // the data we're dragging
    wxDataObject *m_data;

    // the cursors to use for feedback
    wxCursor m_cursorCopy,
             m_cursorMove,
             m_cursorStop;
};

// ----------------------------------------------------------------------------
// wxDropTarget should be associated with a window if it wants to be able to
// receive data via drag and drop.
//
// To use this class, you should derive from wxDropTarget and implement
// OnData() pure method. You may also wish to override OnDrop() if you
// want to accept the data only inside some region of the window (this may
// avoid having to copy the data to this application which happens only when
// OnData() is called)
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxDropTargetBase
{
public:
    // ctor takes a pointer to heap-allocated wxDataObject which will be owned
    // by wxDropTarget and deleted by it automatically. If you don't give it
    // here, you can use SetDataObject() later.
    wxDropTargetBase(wxDataObject *dataObject = nullptr)
        { m_dataObject = dataObject; m_defaultAction = wxDragResult::None; }
    // dtor deletes our data object
    virtual ~wxDropTargetBase()
        { delete m_dataObject; }

   wxDropTargetBase(const wxDropTargetBase&) = delete;
   wxDropTargetBase& operator=(const wxDropTargetBase&) = delete;
   wxDropTargetBase(wxDropTargetBase&&) = default;
   wxDropTargetBase& operator=(wxDropTargetBase&&) = default;

    // get/set the associated wxDataObject
    wxDataObject *GetDataObject() const
        { return m_dataObject; }
    void SetDataObject(wxDataObject *dataObject)
        { delete m_dataObject;
    m_dataObject = dataObject; }

    // these functions are called when data is moved over position (x, y) and
    // may return either wxDragResult::Copy, wxDragResult::Move or wxDragResult::None depending on
    // what would happen if the data were dropped here.
    //
    // the last parameter is what would happen by default and is determined by
    // the platform-specific logic (for example, under Windows it's wxDragResult::Copy
    // if Ctrl key is pressed and wxDragResult::Move otherwise) except that it will
    // always be wxDragResult::None if the carried data is in an unsupported format.

    // called when the mouse enters the window (only once until OnLeave())
    virtual wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def)
        { return OnDragOver(x, y, def); }

    // called when the mouse moves in the window - shouldn't take long to
    // execute or otherwise mouse movement would be too slow
    virtual wxDragResult OnDragOver(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                                    wxDragResult def)
        { return def; }

    // called when mouse leaves the window: might be used to remove the
    // feedback which was given in OnEnter()
    virtual void OnLeave() { }

    // this function is called when data is dropped at position (x, y) - if it
    // returns true, OnData() will be called immediately afterwards which will
    // allow to retrieve the data dropped.
    virtual bool OnDrop(wxCoord x, wxCoord y) = 0;

    // called after OnDrop() returns TRUE: you will usually just call
    // GetData() from here and, probably, also refresh something to update the
    // new data and, finally, return the code indicating how did the operation
    // complete (returning default value in case of success and wxDragResult::Error on
    // failure is usually ok)
    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) = 0;

    // may be called *only* from inside OnData() and will fill m_dataObject
    // with the data from the drop source if it returns true
    virtual bool GetData() = 0;

    // sets the default action for drag and drop:
    // use wxDragResult::Move or wxDragResult::Copy to set default action to move or copy
    // and use wxDragResult::None (default) to set default action specified by
    // initialization of dragging (see wxDropSourceBase::DoDragDrop())
    void SetDefaultAction(wxDragResult action)
        { m_defaultAction = action; }

    // returns default action for drag and drop or
    // wxDragResult::None if this not specified
    wxDragResult GetDefaultAction()
        { return m_defaultAction; }

protected:
    wxDataObject *m_dataObject;
    wxDragResult m_defaultAction;
};

// ----------------------------------------------------------------------------
// include platform dependent class declarations
// ----------------------------------------------------------------------------

#if defined(__WXMSW__)
    #include "wx/msw/ole/dropsrc.h"
    #include "wx/msw/ole/droptgt.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/dnd.h"
#elif defined(__WXX11__)
    #include "wx/x11/dnd.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/dnd.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/dnd.h"
#elif defined(__WXMAC__)
    #include "wx/osx/dnd.h"
#elif defined(__WXQT__)
    #include "wx/qt/dnd.h"
#endif

// ----------------------------------------------------------------------------
// standard wxDropTarget implementations (implemented in common/dobjcmn.cpp)
// ----------------------------------------------------------------------------

// A simple wxDropTarget derived class for text data: you only need to
// override OnDropText() to get something working
class WXDLLIMPEXP_CORE wxTextDropTarget : public wxDropTarget
{
public:
    wxTextDropTarget();

   wxTextDropTarget(const wxTextDropTarget&) = delete;
   wxTextDropTarget& operator=(const wxTextDropTarget&) = delete;
   wxTextDropTarget(wxTextDropTarget&&) = delete;
   wxTextDropTarget& operator=(wxTextDropTarget&&) = delete;

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text) = 0;

    wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) override;
};

// A drop target which accepts files (dragged from File Manager or Explorer)
class WXDLLIMPEXP_CORE wxFileDropTarget : public wxDropTarget
{
public:
    wxFileDropTarget();

    wxFileDropTarget(const wxFileDropTarget&) = delete;
    wxFileDropTarget& operator=(const wxFileDropTarget&) = delete;
    wxFileDropTarget(wxFileDropTarget&&) = delete;
    wxFileDropTarget& operator=(wxFileDropTarget&&) = delete;

    // parameters are the number of files and the array of file names
    virtual bool OnDropFiles(wxCoord x, wxCoord y,
                             const std::vector<wxString>& filenames) = 0;

    wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) override;
};

#endif // wxUSE_DRAG_AND_DROP

#endif // _WX_DND_H_BASE_
