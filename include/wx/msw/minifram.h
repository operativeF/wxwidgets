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

import <string>;

class wxMiniFrame : public wxFrame
{
public:
  wxMiniFrame() = default;
  wxMiniFrame& operator=(wxMiniFrame&&) = delete;

  wxMiniFrame(wxWindow *parent,
              wxWindowID id,
              const std::string& title,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              unsigned int style = wxCAPTION | wxCLIP_CHILDREN | wxRESIZE_BORDER,
              std::string_view name = wxFrameNameStr)
  {
      Create(parent, id, title, pos, size, style, name);
  }
  
  [[maybe_unused]] bool Create(wxWindow *parent,
              wxWindowID id,
              const std::string& title,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              unsigned int style = wxCAPTION | wxCLIP_CHILDREN | wxRESIZE_BORDER,
              std::string_view name = wxFrameNameStr)
  {
      return wxFrame::Create(parent, id, title, pos, size,
                             style |
                             wxFRAME_TOOL_WINDOW |
                             (parent ? wxFRAME_FLOAT_ON_PARENT : 0), name);
  }
};

#endif
    // _WX_MINIFRAM_H_
