/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/minifram.h
// Purpose:     wxMiniFrame class
// Author:      Mariano Reingart
// Copyright:   (c) 2014 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MINIFRAM_H_
#define _WX_MINIFRAM_H_

#include "wx/frame.h"

class wxMiniFrame : public wxFrame
{
public:
  wxMiniFrame() { }

	wxMiniFrame(const wxMiniFrame&) = delete;
	wxMiniFrame& operator=(const wxMiniFrame&) = delete;

  bool Create(wxWindow *parent,
              wxWindowID id,
              const wxString& title,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              unsigned int style = wxCAPTION | wxCLIP_CHILDREN | wxRESIZE_BORDER,
              const wxString& name = wxASCII_STR(wxFrameNameStr))
  {
      return wxFrame::Create(parent, id, title, pos, size,
                             style | wxFRAME_TOOL_WINDOW | wxFRAME_NO_TASKBAR,
                             name);
  }

  wxMiniFrame(wxWindow *parent,
              wxWindowID id,
              const wxString& title,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              unsigned int style = wxCAPTION | wxCLIP_CHILDREN | wxRESIZE_BORDER,
              const wxString& name = wxASCII_STR(wxFrameNameStr))
  {
      Create(parent, id, title, pos, size, style, name);
  }

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif
    // _WX_MINIFRAM_H_
