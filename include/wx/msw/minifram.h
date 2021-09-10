/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/minifram.h
// Purpose:     wxMiniFrame class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MINIFRAM_H_
#define _WX_MINIFRAM_H_

#include "wx/frame.h"

#include <string>

class WXDLLIMPEXP_CORE wxMiniFrame : public wxFrame
{
public:
  wxMiniFrame() = default;

  wxMiniFrame(const wxMiniFrame&) = delete;
  wxMiniFrame& operator=(const wxMiniFrame&) = delete;
  wxMiniFrame(wxMiniFrame&&) = default;
  wxMiniFrame& operator=(wxMiniFrame&&) = default;

  ~wxMiniFrame() = default;

  [[maybe_unused]] bool Create(wxWindow *parent,
              wxWindowID id,
              const std::string& title,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              long style = wxCAPTION | wxCLIP_CHILDREN | wxRESIZE_BORDER,
              const std::string& name = wxFrameNameStr)
  {
      return wxFrame::Create(parent, id, title, pos, size,
                             style |
                             wxFRAME_TOOL_WINDOW |
                             (parent ? wxFRAME_FLOAT_ON_PARENT : 0), name);
  }

  wxMiniFrame(wxWindow *parent,
              wxWindowID id,
              const std::string& title,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              long style = wxCAPTION | wxCLIP_CHILDREN | wxRESIZE_BORDER,
              const std::string& name = wxFrameNameStr)
  {
      Create(parent, id, title, pos, size, style, name);
  }

public:
	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif
    // _WX_MINIFRAM_H_
