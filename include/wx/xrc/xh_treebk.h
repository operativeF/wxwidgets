/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_treebk.h
// Purpose:     XML resource handler for wxTreebook
// Author:      Evgeniy Tarassov
// Created:     2005/09/28
// Copyright:   (c) 2005 TT-Solutions <vadim@tt-solutions.com>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_TREEBK_H_
#define _WX_XH_TREEBK_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_TREEBOOK

class  wxTreebook;

WX_DEFINE_ARRAY_SIZE_T(size_t, wxArrayTbkPageIndexes, class);

// ---------------------------------------------------------------------
// wxTreebookXmlHandler class
// ---------------------------------------------------------------------
// Resource xml structure have to be almost the "same" as for wxNotebook
// except the additional (size_t)depth parameter for treebookpage nodes
// which indicates the depth of the page in the tree.
// There is only one logical constraint on this parameter :
// it cannot be greater than the previous page depth plus one
class wxTreebookXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxTreebookXmlHandler);

public:
    wxTreebookXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    wxTreebook *m_tbk{nullptr};
    wxArrayTbkPageIndexes m_treeContext;
    bool m_isInside{false};
};


// Example:
// -------
// Label
// \--First
// |  \--Second
// \--Third
//
//<resource>
//  ...
//  <object class="wxTreebook">
//    <object class="treebookpage">
//      <object class="wxWindow" />
//      <label>My first page</label>
//      <depth>0</depth>
//    </object>
//    <object class="treebookpage">
//      <object class="wxWindow" />
//      <label>First</label>
//      <depth>1</depth>
//    </object>
//    <object class="treebookpage">
//      <object class="wxWindow" />
//      <label>Second</label>
//      <depth>2</depth>
//    </object>
//    <object class="treebookpage">
//      <object class="wxWindow" />
//      <label>Third</label>
//      <depth>1</depth>
//    </object>
//  </object>
//  ...
//</resource>

#endif // wxUSE_XRC && wxUSE_TREEBOOK

#endif // _WX_XH_TREEBK_H_
