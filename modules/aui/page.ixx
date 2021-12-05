module;

#include "wx/bitmap.h"

#include "wx/arrimpl.cpp"

export module WX.AUI.Page;

import Utils.Geometry;

import <string>;

export
{

class wxWindow;

struct wxAuiNotebookPage
{
    std::string caption;           // caption displayed on the tab
    std::string tooltip;              // tooltip displayed when hovering over tab title
    wxBitmap bitmap;               // tab's bitmap
    wxRect rect;                   // tab's hit rectangle
    wxWindow* window{nullptr};     // page's associated window
    bool active{false};            // true if the page is currently active
    bool hover{false};             // true if mouse hovering over tab
};

using wxAuiNotebookPageArray = std::vector<wxAuiNotebookPage>;

}
