///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/ole/dropsrc.h
// Purpose:     declaration of the wxDropSource class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     06.03.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef   _WX_OLEDROPSRC_H
#define   _WX_OLEDROPSRC_H

#if wxUSE_DRAG_AND_DROP

// ----------------------------------------------------------------------------
// forward declarations
// ----------------------------------------------------------------------------

class wxIDropSource;
class wxDataObject;
class wxWindow;

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

// this macro may be used instead for wxDropSource ctor arguments: it will use
// the cursor 'name' from the resources under MSW, but will expand to
// something else under GTK. If you don't use it, you will have to use #ifdef
// in the application code.
#define wxDROP_ICON(name)   wxCursor(wxT(#name))

// ----------------------------------------------------------------------------
// wxDropSource is used to start the drag-&-drop operation on associated
// wxDataObject object. It's responsible for giving UI feedback while dragging.
// ----------------------------------------------------------------------------

class wxDropSource : public wxDropSourceBase
{
public:
    // ctors: if you use default ctor you must call SetData() later!
    //
    // NB: the "wxWindow *win" parameter is unused and is here only for wxGTK
    //     compatibility, as well as both icon parameters
    wxDropSource(wxWindow *win = nullptr,
                 const wxCursor &cursorCopy = wxNullCursor,
                 const wxCursor &cursorMove = wxNullCursor,
                 const wxCursor &cursorStop = wxNullCursor);
    wxDropSource(wxDataObject& data,
                 wxWindow *win = nullptr,
                 const wxCursor &cursorCopy = wxNullCursor,
                 const wxCursor &cursorMove = wxNullCursor,
                 const wxCursor &cursorStop = wxNullCursor);

    ~wxDropSource();

	wxDropSource& operator=(wxDropSource&&) = delete;

    // do it (call this in response to a mouse button press, for example)
    // params: if bAllowMove is false, data can be only copied
    wxDragResult DoDragDrop(unsigned int flags = wxDrag_CopyOnly) override;

    // overridable: you may give some custom UI feedback during d&d operation
    // in this function (it's called on each mouse move, so it shouldn't be
    // too slow). Just return false if you want default feedback.
    bool GiveFeedback(wxDragResult effect) override;

private:
    wxIDropSource *m_pIDropSource;  // the pointer to COM interface
};

#endif  //wxUSE_DRAG_AND_DROP

#endif  //_WX_OLEDROPSRC_H
