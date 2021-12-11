/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/notebook.h
// Purpose:     MSW/GTK compatible notebook (a.k.a. property sheet)
// Author:      Robert Roebling
// Modified by: Vadim Zeitlin for Windows version
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _NOTEBOOK_H
#define _NOTEBOOK_H

#if wxUSE_NOTEBOOK

#include "wx/control.h"

import WX.WinDef;
import WX.Win.UniqueHnd;
import Utils.Geometry;

import <string>;

using msw::utils::unique_brush;

// you can set USE_NOTEBOOK_ANTIFLICKER to 0 for desktop Windows versions too
// to disable code whih results in flicker-less notebook redrawing at the
// expense of some extra GDI resource consumption
#define USE_NOTEBOOK_ANTIFLICKER    1

class wxNotebook : public wxNotebookBase
{
public:
  wxNotebook() = default;

    // the same arguments as for wxControl (@@@ any special styles?)
  wxNotebook(wxWindow *parent,
             wxWindowID id,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             unsigned int style = 0,
             std::string_view name = wxNotebookNameStr);
    // Create() function
  [[maybe_unused]] bool Create(wxWindow *parent,
              wxWindowID id,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              unsigned int style = 0,
              std::string_view name = wxNotebookNameStr);

  wxNotebook& operator=(wxNotebook&&) = delete;

  // accessors
  // ---------
    // get number of pages in the dialog
  size_t GetPageCount() const override;

    // set the currently selected page, return the index of the previously
    // selected one (or wxNOT_FOUND on error)
    // NB: this function will _not_ generate wxEVT_NOTEBOOK_PAGE_xxx events
  int SetSelection(size_t nPage) override;

    // changes selected page without sending events
  int ChangeSelection(size_t nPage) override;

    // set/get the title of a page
  bool SetPageText(size_t nPage, const std::string& strText) override;
  std::string GetPageText(size_t nPage) const override;

  // image list stuff: each page may have an image associated with it. All
  // the images belong to an image list, so you have to
  // 1) create an image list
  // 2) associate it with the notebook
  // 3) set for each page it's image
    // associate image list with a control
  void SetImageList(wxImageList* imageList) override;

    // sets/returns item's image index in the current image list
  int  GetPageImage(size_t nPage) const override;
  bool SetPageImage(size_t nPage, int nImage) override;

    // currently it's always 1 because wxGTK doesn't support multi-row
    // tab controls
  int GetRowCount() const override;

  // control the appearance of the notebook pages
    // set the size (the same for all pages)
  void SetPageSize(const wxSize& size) override;
    // set the padding between tabs (in pixels)
  void SetPadding(const wxSize& padding) override;

  // operations
  // ----------
    // remove all pages
  bool DeleteAllPages() override;

    // inserts a new page to the notebook (it will be deleted ny the notebook,
    // don't delete it yourself). If bSelect, this page becomes active.
  bool InsertPage(size_t nPage,
                  wxNotebookPage *pPage,
                  const std::string& strText,
                  bool bSelect = false,
                  int imageId = NO_IMAGE) override;

    // Windows-only at present. Also, you must use the wxNB_FIXEDWIDTH
    // style.
  void SetTabSize(const wxSize& sz) override;

    // hit test
  int HitTest(const wxPoint& pt, unsigned int* flags = nullptr) const override;

    // calculate the size of the notebook from the size of its page
  wxSize CalcSizeFromPage(const wxSize& sizePage) const override;

  // callbacks
  // ---------
  void OnSize(wxSizeEvent& event);
  void OnNavigationKey(wxNavigationKeyEvent& event);

  // base class virtuals
  // -------------------

  bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) override;
  bool MSWOnScroll(int orientation, WXWORD nSBCode,
                           WXWORD pos, WXHWND control) override;

  // Attempts to get colour for UX theme page background
  wxColour GetThemeBackgroundColour() const override;

  // implementation only
  // -------------------

#if wxUSE_UXTHEME
  bool SetBackgroundColour(const wxColour& colour) override
  {
      if ( !wxNotebookBase::SetBackgroundColour(colour) )
          return false;

      UpdateBgBrush();

      return true;
  }

  // draw child background
  bool MSWPrintChild(WXHDC hDC, wxWindow *win) override;

  bool MSWHasInheritableBackground() const override { return true; }
#endif // wxUSE_UXTHEME

  // translate wxWin styles to the Windows ones
  WXDWORD MSWGetStyle(unsigned int flags, WXDWORD *exstyle = nullptr) const override;

protected:
  // hides the currently shown page and shows the given one (if not -1) and
  // updates m_selection accordingly
  void UpdateSelection(int selNew);

  // remove one page from the notebook, without deleting
  wxNotebookPage *DoRemovePage(size_t nPage) override;

  // get the page rectangle for the current notebook size
  //
  // returns empty rectangle if an error occurs, do test for it
  wxRect GetPageSize() const;

  // set the size of the given page to fit in the notebook
  void AdjustPageSize(wxNotebookPage *page);

#if wxUSE_UXTHEME
  void MSWAdjustBrushOrg(int *xOrg, int* yOrg) const override
  {
      *xOrg -= m_bgBrushAdj.x;
      *yOrg -= m_bgBrushAdj.y;
  }

  // return the themed brush for painting our children
  WXHBRUSH MSWGetCustomBgBrush() override { return m_hbrBackground.get(); }

  // gets the bitmap of notebook background and returns a brush from it and
  // sets m_bgBrushAdj
  WXHBRUSH QueryBgBitmap();

  // creates the brush to be used for drawing the tab control background
  void UpdateBgBrush();
#endif // wxUSE_UXTHEME

  // these function are used for reducing flicker on notebook resize
  void OnEraseBackground(wxEraseEvent& event);
  void OnPaint(wxPaintEvent& event);

#if wxUSE_UXTHEME
  // background brush used to paint the tab control
  unique_brush m_hbrBackground;

  // offset for MSWAdjustBrushOrg()
  wxPoint m_bgBrushAdj;
#endif // wxUSE_UXTHEME

#if USE_NOTEBOOK_ANTIFLICKER
    // true if we have already subclassed our updown control
    bool m_hasSubclassedUpdown{false};
#endif // USE_NOTEBOOK_ANTIFLICKER

public:
  wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_NOTEBOOK

#endif // _NOTEBOOK_H
