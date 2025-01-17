///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/ole/dropsrc.cpp
// Purpose:     implementation of wxIDropSource and wxDropSource
// Author:      Vadim Zeitlin
// Modified by:
// Created:     10.05.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_OLE && wxUSE_DRAG_AND_DROP

#include "wx/msw/private.h"

#include "wx/window.h"
#include "wx/log.h"

#include "wx/dnd.h"

#include "wx/msw/ole/oleutils.h"

import WX.WinDef;

// ----------------------------------------------------------------------------
// wxIDropSource implementation of IDropSource interface
// ----------------------------------------------------------------------------

class wxIDropSource : public IDropSource
{
public:
  explicit wxIDropSource(wxDropSource *pDropSource);

  wxIDropSource(const wxIDropSource&) = delete;
  wxIDropSource& operator=(const wxIDropSource&) = delete;

  // IDropSource
  STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override;
  STDMETHODIMP GiveFeedback(DWORD dwEffect) override;

    DECLARE_IUNKNOWN_METHODS;

private:
  DWORD         m_grfInitKeyState;  // button which started the d&d operation
  wxDropSource *m_pDropSource;      // pointer to C++ class we belong to
};

// ============================================================================
// Implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxIDropSource implementation
// ----------------------------------------------------------------------------
BEGIN_IID_TABLE(wxIDropSource)
  ADD_IID(Unknown)
  ADD_IID(DropSource)
END_IID_TABLE;

IMPLEMENT_IUNKNOWN_METHODS(wxIDropSource)

wxIDropSource::wxIDropSource(wxDropSource *pDropSource)
{
  wxASSERT( pDropSource != nullptr );

  m_pDropSource = pDropSource;
  m_grfInitKeyState = 0;
}

// Name    : wxIDropSource::QueryContinueDrag
// Purpose : decide if drag operation must be continued or not
// Returns : HRESULT: S_OK              if we should continue
//                    DRAGDROP_S_DROP   to drop right now
//                    DRAGDROP_S_CANCEL to cancel everything
// Params  : [in] BOOL  fEscapePressed  <Esc> pressed since last call?
//           [in] DWORD grfKeyState     mask containing state of kbd keys
// Notes   : as there is no reasonably simple portable way to implement this
//           function, we currently don't give the possibility to override the
//           default behaviour implemented here
STDMETHODIMP wxIDropSource::QueryContinueDrag(BOOL fEscapePressed,
                                              DWORD grfKeyState)
{
  if ( fEscapePressed )
    return DRAGDROP_S_CANCEL;

  // initialize ourself with the drag begin button
  if ( m_grfInitKeyState == 0 ) {
    m_grfInitKeyState = grfKeyState & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON);
  }

  if ( !(grfKeyState & m_grfInitKeyState) ) {
    // button which started d&d released, go!
    return DRAGDROP_S_DROP;
  }

  return S_OK;
}

// Name    : wxIDropSource::GiveFeedback
// Purpose : give UI feedback according to current state of operation
// Returns : STDMETHODIMP
// Params  : [in] DWORD dwEffect - what would happen if we dropped now
// Notes   : default implementation is ok in more than 99% of cases
STDMETHODIMP wxIDropSource::GiveFeedback(DWORD dwEffect)
{
  wxDragResult effect;
  if ( dwEffect & DROPEFFECT_COPY )
    effect = wxDragResult::Copy;
  else if ( dwEffect & DROPEFFECT_MOVE )
    effect = wxDragResult::Move;
  else
    effect = wxDragResult::None;

  if ( m_pDropSource->GiveFeedback(effect) )
    return S_OK;

  return DRAGDROP_S_USEDEFAULTCURSORS;
}

// ----------------------------------------------------------------------------
// wxDropSource implementation
// ----------------------------------------------------------------------------

// ctors

wxDropSource::wxDropSource([[maybe_unused]] wxWindow* win,
                           const wxCursor &cursorCopy,
                           const wxCursor &cursorMove,
                           const wxCursor &cursorStop)
            : wxDropSourceBase(cursorCopy, cursorMove, cursorStop)
{
    m_pIDropSource = new wxIDropSource(this);
    m_pIDropSource->AddRef();
}

wxDropSource::wxDropSource(wxDataObject& data,
                           [[maybe_unused]] wxWindow* win,
                           const wxCursor &cursorCopy,
                           const wxCursor &cursorMove,
                           const wxCursor &cursorStop)
            : wxDropSourceBase(cursorCopy, cursorMove, cursorStop)
{
    m_pIDropSource = new wxIDropSource(this);
    m_pIDropSource->AddRef();
    SetData(data);
}

wxDropSource::~wxDropSource()
{
    m_pIDropSource->Release();
}

// Name    : DoDragDrop
// Purpose : start drag and drop operation
// Returns : wxDragResult - the code of performed operation
// Params  : [in] int flags: specifies if moving is allowed (or only copying)
// Notes   : you must call SetData() before if you had used def ctor
wxDragResult wxDropSource::DoDragDrop(unsigned int flags)
{
  wxCHECK_MSG( m_data != nullptr, wxDragResult::None, "No data in wxDropSource!" );

  DWORD dwEffect;
  HRESULT hr = ::DoDragDrop(m_data->GetInterface(),
                            m_pIDropSource,
                            (flags & wxDrag_AllowMove)
                                ? DROPEFFECT_COPY | DROPEFFECT_MOVE
                                : DROPEFFECT_COPY,
                            &dwEffect);

  if ( hr == DRAGDROP_S_CANCEL ) {
    return wxDragResult::Cancel;
  }
  else if ( hr == DRAGDROP_S_DROP ) {
    if ( dwEffect & DROPEFFECT_COPY ) {
      return wxDragResult::Copy;
    }
    else if ( dwEffect & DROPEFFECT_MOVE ) {
      // consistency check: normally, we shouldn't get "move" at all
      // here if we don't allow it, but in practice it does happen quite often
      return (flags & wxDrag_AllowMove) ? wxDragResult::Move : wxDragResult::Copy;
    }
    else {
      // not copy or move
      return wxDragResult::None;
    }
  }
  else {
    if ( FAILED(hr) ) {
      wxLogApiError("DoDragDrop", hr);
      wxLogError("Drag & drop operation failed.");
    }
    else {
      wxLogDebug("Unexpected success return code %08lx from DoDragDrop.",
                 hr);
    }

    return wxDragResult::Error;
  }
}

// Name    : wxDropSource::GiveFeedback
// Purpose : visually inform the user about d&d operation state
// Returns : bool: true if we do all ourselves or false for default feedback
// Params  : [in] DragResult effect - what would happen if we dropped now
// Notes   : here we just leave this stuff for default implementation
bool wxDropSource::GiveFeedback(wxDragResult effect)
{
    const wxCursor& cursor = GetCursor(effect);
    if ( cursor.IsOk() )
    {
        ::SetCursor((WXHCURSOR)cursor.GetHCURSOR());

        return true;
    }
    else
    {
        return false;
    }
}

#endif  // wxUSE_OLE && wxUSE_DRAG_AND_DROP
