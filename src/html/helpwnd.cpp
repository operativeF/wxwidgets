/////////////////////////////////////////////////////////////////////////////
// Name:        src/html/helpwnd.cpp
// Purpose:     wxHtmlHelpWindow
// Notes:       Based on htmlhelp.cpp, implementing a monolithic
//              HTML Help controller class,  by Vaclav Slavik
// Author:      Harm van der Heijden and Vaclav Slavik
// Copyright:   (c) Harm van der Heijden and Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_WXHTML_HELP

#include "wx/object.h"
#include "wx/intl.h"
#include "wx/log.h"

#if wxUSE_STREAMS
import WX.Cmn.Stream;
#endif

#include "wx/bmpbuttn.h"
#include "wx/statbox.h"
#include "wx/radiobox.h"
#include "wx/menu.h"
#include "wx/msgdlg.h"
#include "wx/textctrl.h"
#include "wx/toolbar.h"
#include "wx/choicdlg.h"
#include "wx/filedlg.h"

#include "wx/html/helpfrm.h"
#include "wx/html/helpdlg.h"
#include "wx/html/helpctrl.h"
#include "wx/notebook.h"
#include "wx/imaglist.h"
#include "wx/treectrl.h"
#include "wx/html/htmlwin.h"
#include "wx/busyinfo.h"
#include "wx/progdlg.h"
#include "wx/fontenum.h"
#include "wx/artprov.h"
#include "wx/spinctrl.h"
#include "wx/wupdlock.h"

import WX.Cfg.Flags;
import WX.Core.Sizer;
import WX.Cmn.WFStream;

import WX.Utils.Settings;
import Utils.Strings;

// what is considered "small index"?
constexpr int INDEX_IS_SMALL = 1000;

// minimum width for content tree and index
// (we cannot let minimum size be determined from content, else long titles
//  make the help frame unusable)
constexpr wxCoord CONTENT_TREE_INDEX_MIN_WIDTH = 150;

/* Motif defines this as a macro */
#ifdef Below
#undef Below
#endif

//--------------------------------------------------------------------------
// wxHtmlHelpTreeItemData (private)
//--------------------------------------------------------------------------

class wxHtmlHelpTreeItemData : public wxTreeItemData
{
    public:
        explicit wxHtmlHelpTreeItemData(int id)  
            { m_Id = id;}

        int m_Id;
};


//--------------------------------------------------------------------------
// wxHtmlHelpHashData (private)
//--------------------------------------------------------------------------

class wxHtmlHelpHashData : public wxObject
{
public:
    wxHtmlHelpHashData(int index, wxTreeItemId id)
        : m_Index(index),
            m_Id(id)  
    {}
    
    int m_Index;
    wxTreeItemId m_Id;
};


//--------------------------------------------------------------------------
// wxHtmlHelpHtmlWindow (private)
//--------------------------------------------------------------------------


class wxHtmlHelpHtmlWindow : public wxHtmlWindow
{
public:
    wxHtmlHelpHtmlWindow(wxHtmlHelpWindow *win, wxWindow *parent, wxWindowID id = wxID_ANY,
                         const wxPoint& pos = wxDefaultPosition, const wxSize& sz = wxDefaultSize, unsigned int style = wxHW_DEFAULT_STYLE)
        : wxHtmlWindow(parent, id, pos, sz, style), m_Window(win)
    {
        SetStandardFonts();
    }

    wxHtmlHelpHtmlWindow(const wxHtmlHelpHtmlWindow&) = delete;
	wxHtmlHelpHtmlWindow& operator=(const wxHtmlHelpHtmlWindow&) = delete;

    bool LoadPage(const std::string& location) override
    {
        if ( !wxHtmlWindow::LoadPage(location) )
            return false;

        m_Window->NotifyPageChanged();
        return true;
    }

    // Returns full location with anchor (helper)
    static wxString GetOpenedPageWithAnchor(wxHtmlWindow *win)
    {
        if(!win)
            return {};

        wxString an = win->GetOpenedAnchor();
        wxString pg = win->GetOpenedPage();
        if(!an.empty())
        {
            pg << "#" << an;
        }
        return pg;
    }

private:
    wxHtmlHelpWindow *m_Window;
};


//---------------------------------------------------------------------------
// wxHtmlHelpWindow::m_mergedIndex
//---------------------------------------------------------------------------

using wxHtmlHelpDataItemPtrArray = std::vector<const wxHtmlHelpDataItem*>;

struct wxHtmlHelpMergedIndexItem
{
    wxHtmlHelpMergedIndexItem *parent;
    wxString                   name;
    wxHtmlHelpDataItemPtrArray items;
};

WX_DECLARE_OBJARRAY(wxHtmlHelpMergedIndexItem, wxHtmlHelpMergedIndex);
WX_DEFINE_OBJARRAY(wxHtmlHelpMergedIndex)

void wxHtmlHelpWindow::UpdateMergedIndex()
{
    delete m_mergedIndex;
    m_mergedIndex = new wxHtmlHelpMergedIndex;
    wxHtmlHelpMergedIndex& merged = *m_mergedIndex;

    const wxHtmlHelpDataItems& items = m_Data->GetIndexArray();
    size_t len = items.size();

    wxHtmlHelpMergedIndexItem *history[128] = {nullptr};

    for (size_t i = 0; i < len; i++)
    {
        const wxHtmlHelpDataItem& item = items[i];
        wxASSERT_MSG( item.level < 128, "nested index entries too deep" );

        if (history[item.level] &&
            history[item.level]->items[0]->name == item.name)
        {
            // same index entry as previous one, update list of items
            history[item.level]->items.Add(&item);
        }
        else
        {
            // new index entry
            wxHtmlHelpMergedIndexItem *mi = new wxHtmlHelpMergedIndexItem();
            mi->name = item.GetIndentedName();
            mi->items.Add(&item);
            mi->parent = (item.level == 0) ? nullptr : history[item.level - 1];
            history[item.level] = mi;
            merged.Add(mi);
        }
    }
}

//---------------------------------------------------------------------------
// wxHtmlHelpWindow
//---------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxHtmlHelpWindow, wxWindow);

wxBEGIN_EVENT_TABLE(wxHtmlHelpWindow, wxWindow)
    EVT_TOOL_RANGE(wxID_HTML_PANEL, wxID_HTML_OPTIONS, wxHtmlHelpWindow::OnToolbar)
    EVT_BUTTON(wxID_HTML_BOOKMARKSREMOVE, wxHtmlHelpWindow::OnToolbar)
    EVT_BUTTON(wxID_HTML_BOOKMARKSADD, wxHtmlHelpWindow::OnToolbar)
    EVT_TREE_SEL_CHANGED(wxID_HTML_TREECTRL, wxHtmlHelpWindow::OnContentsSel)
    EVT_LISTBOX(wxID_HTML_INDEXLIST, wxHtmlHelpWindow::OnIndexSel)
    EVT_LISTBOX(wxID_HTML_SEARCHLIST, wxHtmlHelpWindow::OnSearchSel)
    EVT_BUTTON(wxID_HTML_SEARCHBUTTON, wxHtmlHelpWindow::OnSearch)
    EVT_TEXT_ENTER(wxID_HTML_SEARCHTEXT, wxHtmlHelpWindow::OnSearch)
    EVT_BUTTON(wxID_HTML_INDEXBUTTON, wxHtmlHelpWindow::OnIndexFind)
    EVT_TEXT_ENTER(wxID_HTML_INDEXTEXT, wxHtmlHelpWindow::OnIndexFind)
    EVT_BUTTON(wxID_HTML_INDEXBUTTONALL, wxHtmlHelpWindow::OnIndexAll)
    EVT_COMBOBOX(wxID_HTML_BOOKMARKSLIST, wxHtmlHelpWindow::OnBookmarksSel)
    EVT_SIZE(wxHtmlHelpWindow::OnSize)
wxEND_EVENT_TABLE()

wxHtmlHelpWindow::wxHtmlHelpWindow(wxWindow* parent, wxWindowID id,
                                    const wxPoint& pos,
                                    const wxSize& size,
                                    unsigned int style, unsigned int helpStyle, wxHtmlHelpData* data)
{
    Init(data);
    Create(parent, id, pos, size, style, helpStyle);
}

void wxHtmlHelpWindow::Init(wxHtmlHelpData* data)
{
    if (data)
    {
        m_Data = data;
        m_DataCreated = false;
    }
    else
    {
        m_Data = new wxHtmlHelpData();
        m_DataCreated = true;
    }

    m_ContentsPage = 0;
    m_IndexPage = 0;
    m_SearchPage = 0;

    m_ContentsBox = nullptr;
    m_IndexList = nullptr;
    m_IndexButton = nullptr;
    m_IndexButtonAll = nullptr;
    m_IndexText = nullptr;
    m_SearchList = nullptr;
    m_SearchButton = nullptr;
    m_SearchText = nullptr;
    m_SearchChoice = nullptr;
    m_IndexCountInfo = nullptr;
    m_Splitter = nullptr;
    m_NavigPan = nullptr;
    m_NavigNotebook = nullptr;
    m_HtmlWin = nullptr;
    m_Bookmarks = nullptr;
    m_SearchCaseSensitive = nullptr;
    m_SearchWholeWords = nullptr;

    m_mergedIndex = nullptr;

#if wxUSE_CONFIG
    m_Config = nullptr;
    m_ConfigRoot.clear();
#endif // wxUSE_CONFIG

    m_Cfg.x = m_Cfg.y = wxDefaultCoord;
    m_Cfg.w = 700;
    m_Cfg.h = 480;
    m_Cfg.sashpos = 240;
    m_Cfg.navig_on = true;

    m_NormalFonts = m_FixedFonts = nullptr;
    m_NormalFace.clear();
    m_FixedFace.clear();
#ifdef __WXMSW__
    m_FontSize = 10;
#else
    m_FontSize = 14;
#endif

#if wxUSE_PRINTING_ARCHITECTURE
    m_Printer = nullptr;
#endif

    m_PagesHash = nullptr;
    m_UpdateContents = true;
    m_toolBar = nullptr;
    m_helpController = nullptr;
}

// Create: builds the GUI components.
// with the style flag it's possible to toggle the toolbar, contents, index and search
// controls.
// m_HtmlWin will *always* be created, but it's important to realize that
// m_ContentsBox, m_IndexList, m_SearchList, m_SearchButton, m_SearchText and
// m_SearchButton may be NULL.
// moreover, if no contents, index or searchpage is needed, m_Splitter and
// m_NavigPan will be NULL too (with m_HtmlWin directly connected to the frame)

bool wxHtmlHelpWindow::Create(wxWindow* parent, wxWindowID id,
                             const wxPoint& pos, const wxSize& size,
                             unsigned int style, unsigned int helpStyle)
{
    m_hfStyle = helpStyle;

#if wxUSE_CONFIG
    // Do the config in two steps. We read the HtmlWindow customization after we
    // create the window.
    if (m_Config)
        ReadCustomization(m_Config, m_ConfigRoot);
#endif // wxUSE_CONFIG

    wxWindow::Create(parent, id, pos, size, style, "wxHtmlHelp");

    SetHelpText(_("Displays help as you browse the books on the left."));

    m_Cfg.x = GetPosition().x;
    m_Cfg.y = GetPosition().y;

    int notebook_page = 0;

    // The sizer for the whole top-level window.
    wxSizer *topWindowSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topWindowSizer);
    SetAutoLayout(true);

#if wxUSE_TOOLBAR
    // toolbar?
    if (helpStyle & (wxHF_TOOLBAR | wxHF_FLAT_TOOLBAR))
    {
        wxToolBar *toolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                           wxNO_BORDER | wxTB_HORIZONTAL |
                                           wxTB_DOCKABLE | wxTB_NODIVIDER |
                                           (helpStyle & wxHF_FLAT_TOOLBAR ? wxTB_FLAT : 0));
        toolBar->SetMargins( 2, 2 );
        toolBar->SetToolBitmapSize( wxSize(22,22) );
        AddToolbarButtons(toolBar, helpStyle);
        toolBar->Realize();
        topWindowSizer->Add(toolBar, 0, wxEXPAND);
        m_toolBar = toolBar;
    }
#endif //wxUSE_TOOLBAR

    wxSizer *navigSizer = nullptr;

#ifdef __WXMSW__
    wxBorder htmlWindowBorder = wxBORDER_THEME;
#else
    wxBorder htmlWindowBorder = wxBORDER_SUNKEN;
#endif

    if (helpStyle & (wxHF_CONTENTS | wxHF_INDEX | wxHF_SEARCH))
    {
        // traditional help controller; splitter window with html page on the
        // right and a notebook containing various pages on the left
        long splitterStyle = wxSP_3D;
        // Drawing moving sash can cause problems on wxMac
#ifdef __WXMAC__
        splitterStyle = 0; // 3D style looks poor on Mac
        splitterStyle |= wxSP_LIVE_UPDATE;
#endif
        m_Splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, splitterStyle);

        topWindowSizer->Add(m_Splitter, 1, wxEXPAND);

        m_HtmlWin = new wxHtmlHelpHtmlWindow(this, m_Splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_DEFAULT_STYLE|htmlWindowBorder);
        m_NavigPan = new wxPanel(m_Splitter, wxID_ANY);
        m_NavigNotebook = new wxNotebook(m_NavigPan, wxID_HTML_NOTEBOOK,
                                         wxDefaultPosition, wxDefaultSize);
#ifdef __WXMAC__
        m_NavigNotebook->SetWindowVariant(wxWindowVariant::Small);
#endif

        navigSizer = new wxBoxSizer(wxVERTICAL);
        navigSizer->Add(m_NavigNotebook, 1, wxEXPAND);

        m_NavigPan->SetSizer(navigSizer);
    }
    else
    {
        // only html window, no notebook with index,contents etc
        m_HtmlWin = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_DEFAULT_STYLE|htmlWindowBorder);
        topWindowSizer->Add(m_HtmlWin, 1, wxEXPAND);
    }

#if wxUSE_CONFIG
    if ( m_Config )
        m_HtmlWin->ReadCustomization(m_Config, m_ConfigRoot);
#endif // wxUSE_CONFIG

    // contents tree panel?
    if ( helpStyle & wxHF_CONTENTS )
    {
        wxWindow *dummy = new wxPanel(m_NavigNotebook, wxID_HTML_INDEXPAGE);
        wxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

        topsizer->Add(0, 10);

        dummy->SetSizer(topsizer);

        if ( helpStyle & wxHF_BOOKMARKS )
        {
            long comboStyle = wxCB_READONLY;
#ifndef __WXMAC__
            // Not supported on OSX/Cocoa presently
            comboStyle |= wxCB_SORT;

#endif
            m_Bookmarks = new wxComboBox(dummy, wxID_HTML_BOOKMARKSLIST,
                                         "",
                                         wxDefaultPosition, wxDefaultSize,
                                         {}, comboStyle);
            m_Bookmarks->Append(_("(bookmarks)"));
            for (unsigned i = 0; i < m_BookmarksNames.GetCount(); i++)
                m_Bookmarks->Append(m_BookmarksNames[i]);
            m_Bookmarks->SetSelection(0);

            wxBitmapButton *bmpbt1, *bmpbt2;
            bmpbt1 = new wxBitmapButton(dummy, wxID_HTML_BOOKMARKSADD,
                                 wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK,
                                                          wxART_BUTTON));
            bmpbt2 = new wxBitmapButton(dummy, wxID_HTML_BOOKMARKSREMOVE,
                                 wxArtProvider::GetBitmap(wxART_DEL_BOOKMARK,
                                                          wxART_BUTTON));
#if wxUSE_TOOLTIPS
            bmpbt1->SetToolTip(_("Add current page to bookmarks"));
            bmpbt2->SetToolTip(_("Remove current page from bookmarks"));
#endif // wxUSE_TOOLTIPS

            wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

            sizer->Add(m_Bookmarks, 1, wxALIGN_CENTRE_VERTICAL | wxRIGHT, 5);
            sizer->Add(bmpbt1, 0, wxALIGN_CENTRE_VERTICAL | wxRIGHT, 2);
            sizer->Add(bmpbt2, 0, wxALIGN_CENTRE_VERTICAL, 0);

            topsizer->Add(sizer, 0, wxEXPAND | wxDirection::wxLEFT | wxDirection::wxBOTTOM | wxDirection::wxRIGHT, 10);
        }

        m_ContentsBox = new wxTreeCtrl(dummy, wxID_HTML_TREECTRL,
                                       wxDefaultPosition, wxDefaultSize,
#if defined(__WXGTK20__) || defined(__WXMAC__)
                                       wxSUNKEN_BORDER |
                                       wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT |
                                       wxTR_NO_LINES
#else
                                       wxSUNKEN_BORDER |
                                       wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT |
                                       wxTR_LINES_AT_ROOT
#endif
                                       );

        wxImageList *ContentsImageList = new wxImageList(16, 16);
        ContentsImageList->Add(wxArtProvider::GetIcon(wxART_HELP_BOOK,
                                                      wxART_HELP_BROWSER,
                                                      wxSize(16, 16)));
        ContentsImageList->Add(wxArtProvider::GetIcon(wxART_HELP_FOLDER,
                                                      wxART_HELP_BROWSER,
                                                      wxSize(16, 16)));
        ContentsImageList->Add(wxArtProvider::GetIcon(wxART_HELP_PAGE,
                                                      wxART_HELP_BROWSER,
                                                      wxSize(16, 16)));

        m_ContentsBox->AssignImageList(ContentsImageList);

        topsizer->Add(m_ContentsBox, 1,
                      wxEXPAND | wxDirection::wxLEFT | wxDirection::wxBOTTOM | wxDirection::wxRIGHT,
                      2);

        m_NavigNotebook->AddPage(dummy, _("Contents"));
        m_ContentsPage = notebook_page++;
    }

    // index listbox panel?
    if ( helpStyle & wxHF_INDEX )
    {
        wxWindow *dummy = new wxPanel(m_NavigNotebook, wxID_HTML_INDEXPAGE);
        wxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

        dummy->SetSizer(topsizer);

        m_IndexText = new wxTextCtrl(dummy, wxID_HTML_INDEXTEXT, "",
                                     wxDefaultPosition, wxDefaultSize,
                                     wxTE_PROCESS_ENTER);
        m_IndexButton = new wxButton(dummy, wxID_HTML_INDEXBUTTON, _("Find"));
        m_IndexButtonAll = new wxButton(dummy, wxID_HTML_INDEXBUTTONALL,
                                        _("Show all"));
        m_IndexCountInfo = new wxStaticText(dummy, wxID_HTML_COUNTINFO,
                                            "", wxDefaultPosition,
                                            wxDefaultSize,
                                            wxALIGN_RIGHT | wxST_NO_AUTORESIZE);
        m_IndexList = new wxListBox(dummy, wxID_HTML_INDEXLIST,
                                    wxDefaultPosition, wxDefaultSize,
                                    {}, wxLB_SINGLE);

#if wxUSE_TOOLTIPS
        m_IndexButton->SetToolTip(_("Display all index items that contain given substring. Search is case insensitive."));
        m_IndexButtonAll->SetToolTip(_("Show all items in index"));
#endif //wxUSE_TOOLTIPS

        topsizer->Add(m_IndexText, 0, wxEXPAND | wxDirection::wxALL, 10);
        wxSizer *btsizer = new wxBoxSizer(wxHORIZONTAL);
        btsizer->Add(m_IndexButton, 0, wxDirection::wxRIGHT, 2);
        btsizer->Add(m_IndexButtonAll);
        topsizer->Add(btsizer, 0,
                      wxALIGN_RIGHT | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxBOTTOM, 10);
        topsizer->Add(m_IndexCountInfo, 0, wxEXPAND | wxDirection::wxLEFT | wxDirection::wxRIGHT, 2);
        topsizer->Add(m_IndexList, 1, wxEXPAND | wxDirection::wxALL, 2);

        m_NavigNotebook->AddPage(dummy, _("Index"));
        m_IndexPage = notebook_page++;
    }

    // search list panel?
    if ( helpStyle & wxHF_SEARCH )
    {
        wxWindow *dummy = new wxPanel(m_NavigNotebook, wxID_HTML_INDEXPAGE);
        wxSizer *sizer = new wxBoxSizer(wxVERTICAL);

        dummy->SetSizer(sizer);

        m_SearchText = new wxTextCtrl(dummy, wxID_HTML_SEARCHTEXT,
                                      "",
                                      wxDefaultPosition, wxDefaultSize,
                                      wxTE_PROCESS_ENTER);
        m_SearchChoice = new wxChoice(dummy, wxID_HTML_SEARCHCHOICE,
                                      wxDefaultPosition, wxSize(125,wxDefaultCoord));
        m_SearchCaseSensitive = new wxCheckBox(dummy, wxID_ANY, _("Case sensitive"));
        m_SearchWholeWords = new wxCheckBox(dummy, wxID_ANY, _("Whole words only"));
        m_SearchButton = new wxButton(dummy, wxID_HTML_SEARCHBUTTON, _("Search"));
#if wxUSE_TOOLTIPS
        m_SearchButton->SetToolTip(_("Search contents of help book(s) for all occurrences of the text you typed above"));
#endif //wxUSE_TOOLTIPS
        m_SearchList = new wxListBox(dummy, wxID_HTML_SEARCHLIST,
                                     wxDefaultPosition, wxDefaultSize,
                                     {}, wxLB_SINGLE);

        sizer->Add(m_SearchText, 0, wxEXPAND | wxDirection::wxALL, 10);
        sizer->Add(m_SearchChoice, 0, wxEXPAND | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxBOTTOM, 10);
        sizer->Add(m_SearchCaseSensitive, 0, wxDirection::wxLEFT | wxDirection::wxRIGHT, 10);
        sizer->Add(m_SearchWholeWords, 0, wxDirection::wxLEFT | wxDirection::wxRIGHT, 10);
        sizer->Add(m_SearchButton, 0, wxDirection::wxALL | wxALIGN_RIGHT, 8);
        sizer->Add(m_SearchList, 1, wxDirection::wxALL | wxEXPAND, 2);

        m_NavigNotebook->AddPage(dummy, _("Search"));
        m_SearchPage = notebook_page;
    }

    m_HtmlWin->Show();

    RefreshLists();

    if ( navigSizer )
    {
        navigSizer->SetSizeHints(m_NavigPan);
        m_NavigPan->Layout();
    }

    // showtime
    if ( m_NavigPan && m_Splitter )
    {
        // The panel will have its own min size which the splitter
        // should respect
        //if (m_NavigPan)
        //    m_Splitter->SetMinimumPaneSize(m_NavigPan->GetBestSize().x);
        //else
        m_Splitter->SetMinimumPaneSize(20);

        if ( m_Cfg.navig_on )
        {
            m_NavigPan->Show();
            m_Splitter->SplitVertically(m_NavigPan, m_HtmlWin, m_Cfg.sashpos);
        }
        else
        {
            m_NavigPan->Show(false);
            m_Splitter->Initialize(m_HtmlWin);
        }
    }

    // Reduce flicker by updating the splitter pane sizes before the
    // frame is shown
    wxSizeEvent sizeEvent(GetSize(), GetId());
    GetEventHandler()->ProcessEvent(sizeEvent);

    if (m_Splitter)
        m_Splitter->UpdateSize();

    return true;
}

wxHtmlHelpWindow::~wxHtmlHelpWindow()
{
    if ( m_helpController )
        m_helpController->SetHelpWindow(nullptr);

    delete m_mergedIndex;

    // PopEventHandler(); // wxhtmlhelpcontroller (not any more!)
    if (m_DataCreated)
        delete m_Data;
    delete m_NormalFonts;
    delete m_FixedFonts;
    if (m_PagesHash)
    {
        WX_CLEAR_HASH_TABLE(*m_PagesHash);
        delete m_PagesHash;
    }
#if wxUSE_PRINTING_ARCHITECTURE
    delete m_Printer;
#endif
}

void wxHtmlHelpWindow::SetController(wxHtmlHelpController* controller)
{
    if (m_DataCreated)
        delete m_Data;
    m_helpController = controller;
    m_Data = controller->GetHelpData();
    m_DataCreated = false;
}

#if wxUSE_TOOLBAR
void wxHtmlHelpWindow::AddToolbarButtons(wxToolBar *toolBar, unsigned int style)
{
    wxBitmap wpanelBitmap =
        wxArtProvider::GetBitmap(wxART_HELP_SIDE_PANEL, wxART_TOOLBAR);
    wxBitmap wbackBitmap =
        wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_TOOLBAR);
    wxBitmap wforwardBitmap =
        wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR);
    wxBitmap wupnodeBitmap =
        wxArtProvider::GetBitmap(wxART_GO_TO_PARENT, wxART_TOOLBAR);
    wxBitmap wupBitmap =
        wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR);
    wxBitmap wdownBitmap =
        wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_TOOLBAR);
    wxBitmap wopenBitmap =
        wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR);
    wxBitmap wprintBitmap =
        wxArtProvider::GetBitmap(wxART_PRINT, wxART_TOOLBAR);
    wxBitmap woptionsBitmap =
        wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_TOOLBAR);

    wxASSERT_MSG( (wpanelBitmap.IsOk() && wbackBitmap.IsOk() &&
                   wforwardBitmap.IsOk() && wupnodeBitmap.IsOk() &&
                   wupBitmap.IsOk() && wdownBitmap.IsOk() &&
                   wopenBitmap.IsOk() && wprintBitmap.IsOk() &&
                   woptionsBitmap.IsOk()),
                  "One or more HTML help frame toolbar bitmap could not be loaded.") ;


    toolBar->AddTool(wxID_HTML_PANEL, "", wpanelBitmap, "Show/hide navigation panel");
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_HTML_BACK, "", wbackBitmap, "Go back");
    toolBar->AddTool(wxID_HTML_FORWARD, "", wforwardBitmap, "Go forward");
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_HTML_UPNODE, "", wupnodeBitmap, "Go one level up in document hierarchy");
    toolBar->AddTool(wxID_HTML_UP, "", wupBitmap, "Previous page");
    toolBar->AddTool(wxID_HTML_DOWN, "", wdownBitmap, "Next page");

    if ((style & wxHF_PRINT) || (style & wxHF_OPEN_FILES))
        toolBar->AddSeparator();

    if (style & wxHF_OPEN_FILES)
        toolBar->AddTool(wxID_HTML_OPENFILE, "", wopenBitmap, "Open HTML document");

#if wxUSE_PRINTING_ARCHITECTURE
    if (style & wxHF_PRINT)
        toolBar->AddTool(wxID_HTML_PRINT, "", wprintBitmap, "Print this page");
#endif

    toolBar->AddSeparator();
    toolBar->AddTool(wxID_HTML_OPTIONS, "", woptionsBitmap, "Display options dialog");

    // Allow application to add custom buttons
    wxHtmlHelpFrame* parentFrame = dynamic_cast<wxHtmlHelpFrame*>(GetParent());
    wxHtmlHelpDialog* parentDialog = dynamic_cast<wxHtmlHelpDialog*>(GetParent());
    if (parentFrame)
        parentFrame->AddToolbarButtons(toolBar, style);
    if (parentDialog)
        parentDialog->AddToolbarButtons(toolBar, style);
}
#endif //wxUSE_TOOLBAR


bool wxHtmlHelpWindow::Display(const wxString& x)
{
    wxString url = m_Data->FindPageByName(x);
    if (!url.empty())
    {
        m_HtmlWin->LoadPage(url);
        return true;
    }

    return false;
}

bool wxHtmlHelpWindow::Display(int id)
{
    wxString url = m_Data->FindPageById(id);
    if (!url.empty())
    {
        m_HtmlWin->LoadPage(url);
        return true;
    }

    return false;
}

bool wxHtmlHelpWindow::DisplayContents()
{
    if (! m_ContentsBox)
        return false;

    if (!m_Splitter->IsSplit())
    {
        m_NavigPan->Show();
        m_HtmlWin->Show();
        m_Splitter->SplitVertically(m_NavigPan, m_HtmlWin, m_Cfg.sashpos);
        m_Cfg.navig_on = true;
    }

    m_NavigNotebook->SetSelection(m_ContentsPage);

    if (m_Data->GetBookRecArray().GetCount() > 0)
    {
        wxHtmlBookRecord& book = m_Data->GetBookRecArray()[0];
        if (!book.GetStart().empty())
            m_HtmlWin->LoadPage(book.GetFullPath(book.GetStart()));
    }

    return true;
}

bool wxHtmlHelpWindow::DisplayIndex()
{
    if (! m_IndexList)
        return false;

    if (!m_Splitter->IsSplit())
    {
        m_NavigPan->Show();
        m_HtmlWin->Show();
        m_Splitter->SplitVertically(m_NavigPan, m_HtmlWin, m_Cfg.sashpos);
    }

    m_NavigNotebook->SetSelection(m_IndexPage);

    if (m_Data->GetBookRecArray().GetCount() > 0)
    {
        wxHtmlBookRecord& book = m_Data->GetBookRecArray()[0];
        if (!book.GetStart().empty())
            m_HtmlWin->LoadPage(book.GetFullPath(book.GetStart()));
    }

    return true;
}

void wxHtmlHelpWindow::DisplayIndexItem(const wxHtmlHelpMergedIndexItem *it)
{
    if (it->items.size() == 1)
    {
        if (!it->items[0]->page.empty())
        {
            m_HtmlWin->LoadPage(it->items[0]->GetFullPath());
        }
    }
    else
    {
        wxBusyCursor busy_cursor;

        // more pages associated with this index item -- let the user choose
        // which one she/he wants from a list:
        std::vector<std::string> arr;
        size_t len = it->items.size();
        for (size_t i = 0; i < len; i++)
        {
            std::string page = it->items[i]->page;
            // try to find page's title in contents:
            const wxHtmlHelpDataItems& contents = m_Data->GetContentsArray();
            size_t clen = contents.size();
            for (size_t j = 0; j < clen; j++)
            {
                if (contents[j].page == page)
                {
                    page = contents[j].name;
                    break;
                }
            }
            arr.push_back(page);
        }

        wxSingleChoiceDialog dlg(this,
                                 "Please choose the page to display:",
                                 "Help Topics",
                                 arr,
                                 (void**)nullptr, // No client data
                                 wxCHOICEDLG_STYLE & ~wxCENTRE);
        if (dlg.ShowModal() == wxID_OK)
        {
            m_HtmlWin->LoadPage(it->items[dlg.GetSelection()]->GetFullPath());
        }
    }
}

bool wxHtmlHelpWindow::KeywordSearch(const wxString& keyword,
                                    wxHelpSearchMode mode)
{
    wxCHECK_MSG( !keyword.empty(), false, "must have a non empty keyword" );

    if (mode == wxHelpSearchMode::All)
    {
        if ( !(m_SearchList &&
               m_SearchButton && m_SearchText && m_SearchChoice) )
            return false;
    }
    else if (mode == wxHelpSearchMode::Index)
    {
        if ( !(m_IndexList &&
               m_IndexButton && m_IndexButtonAll && m_IndexText) )
            return false;
    }

    int foundcnt = 0;
    wxString foundstr;
    wxString book;

    if (!m_Splitter->IsSplit())
    {
        m_NavigPan->Show();
        m_HtmlWin->Show();
        m_Splitter->SplitVertically(m_NavigPan, m_HtmlWin, m_Cfg.sashpos);
    }

    if (mode == wxHelpSearchMode::All)
    {
        m_NavigNotebook->SetSelection(m_SearchPage);
        m_SearchList->Clear();
        m_SearchText->SetValue(keyword);
        m_SearchButton->Disable();

        if (m_SearchChoice->GetSelection() != 0)
            book = m_SearchChoice->GetStringSelection();

        wxHtmlSearchStatus status(m_Data, keyword,
                                  m_SearchCaseSensitive->GetValue(),
                                  m_SearchWholeWords->GetValue(),
                                  book);

#if wxUSE_PROGRESSDLG
        wxProgressDialog progress(_("Searching..."),
                                  _("No matching page found yet"),
                                  status.GetMaxIndex(), this,
                                  wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);
#endif

        while (status.IsActive())
        {
            int curi;
            curi = status.GetCurIndex();
            if (curi % 32 == 0
#if wxUSE_PROGRESSDLG
                && !progress.Update(curi)
#endif
               )
                break;
            if (status.Search())
            {
                foundstr.Printf(_("Found %i matches"), ++foundcnt);
#if wxUSE_PROGRESSDLG
                progress.Update(status.GetCurIndex(), foundstr);
#endif
                m_SearchList->Append(status.GetName(), const_cast<wxHtmlHelpDataItem*>(status.GetCurItem()));
            }
        }

        m_SearchButton->Enable();
        m_SearchText->SetSelection(0, keyword.length());
        m_SearchText->SetFocus();
    }
    else if (mode == wxHelpSearchMode::Index)
    {
        m_NavigNotebook->SetSelection(m_IndexPage);
        m_IndexList->Clear();
        m_IndexButton->Disable();
        m_IndexButtonAll->Disable();
        m_IndexText->SetValue(keyword);

        DoIndexFind();
        m_IndexButton->Enable();
        m_IndexButtonAll->Enable();
        foundcnt = m_IndexList->GetCount();
    }

    if (foundcnt)
    {
        switch ( mode )
        {
            default:
                wxFAIL_MSG( "unknown help search mode" );
                [[fallthrough]];

            case wxHelpSearchMode::All:
            {
                wxHtmlHelpDataItem *it =
                    (wxHtmlHelpDataItem*) m_SearchList->GetClientData(0);
                if (it)
                {
                    m_HtmlWin->LoadPage(it->GetFullPath());
                }
                break;
            }

            case wxHelpSearchMode::Index:
            {
                wxHtmlHelpMergedIndexItem* it =
                    (wxHtmlHelpMergedIndexItem*) m_IndexList->GetClientData(0);
                if (it)
                    DisplayIndexItem(it);
                break;
            }
        }

    }

    return foundcnt > 0;
}

void wxHtmlHelpWindow::CreateContents()
{
    if (! m_ContentsBox)
        return ;

    if (m_PagesHash)
    {
        WX_CLEAR_HASH_TABLE(*m_PagesHash);
        delete m_PagesHash;
    }

    const wxHtmlHelpDataItems& contents = m_Data->GetContentsArray();

    size_t cnt = contents.size();

    m_PagesHash = new wxHashTable(wxKEY_STRING, 2 * cnt);

    static constexpr int MAX_ROOTS = 64;

    wxTreeItemId roots[MAX_ROOTS];
    // VS: this array holds information about whether we've set item icon at
    //     given level. This is necessary because m_Data has a flat structure
    //     and there's no way of recognizing if some item has subitems or not.
    //     We set the icon later: when we find an item with level=n, we know
    //     that the last item with level=n-1 was afolder with subitems, so we
    //     set its icon accordingly
    bool imaged[MAX_ROOTS];
    m_ContentsBox->DeleteAllItems();

    roots[0] = m_ContentsBox->AddRoot(_("(Help)"));
    imaged[0] = true;

    for (size_t i = 0; i < cnt; i++)
    {
        wxHtmlHelpDataItem *it = &contents[i];
        // Handle books:
        if (it->level == 0)
        {
            if (m_hfStyle & wxHF_MERGE_BOOKS)
                // VS: we don't want book nodes, books' content should
                //    appear under tree's root. This line will create a "fake"
                //    record about book node so that the rest of this look
                //    will believe there really _is_ a book node and will
                //    behave correctly.
                roots[1] = roots[0];
            else
            {
                roots[1] = m_ContentsBox->AppendItem(roots[0],
                                         it->name, IMG_Book, -1,
                                         new wxHtmlHelpTreeItemData(i));
                m_ContentsBox->SetItemBold(roots[1], true);
            }
            imaged[1] = true;
        }
        // ...and their contents:
        else
        {
            roots[it->level + 1] = m_ContentsBox->AppendItem(
                                     roots[it->level], it->name, IMG_Page,
                                     -1, new wxHtmlHelpTreeItemData(i));
            imaged[it->level + 1] = false;
        }

        m_PagesHash->Put(it->GetFullPath(),
                         new wxHtmlHelpHashData(i, roots[it->level + 1]));

        // Set the icon for the node one level up in the hierarchy,
        // unless already done (see comment above imaged[] declaration)
        if (!imaged[it->level])
        {
            int image = IMG_Folder;
            if (m_hfStyle & wxHF_ICONS_BOOK)
                image = IMG_Book;
            else if (m_hfStyle & wxHF_ICONS_BOOK_CHAPTER)
                image = (it->level == 1) ? IMG_Book : IMG_Folder;
            m_ContentsBox->SetItemImage(roots[it->level], image);
            m_ContentsBox->SetItemImage(roots[it->level], image,
                                        wxTreeItemIcon_Selected);
            imaged[it->level] = true;
        }
    }

    m_ContentsBox->SetMinSize(wxSize(CONTENT_TREE_INDEX_MIN_WIDTH,
                                     m_ContentsBox->GetMinHeight()));
}

void wxHtmlHelpWindow::CreateIndex()
{
    if (! m_IndexList)
        return ;

    m_IndexList->Clear();

    unsigned long cnt = m_mergedIndex->size();

    wxString cnttext;
    if (cnt > INDEX_IS_SMALL)
        cnttext.Printf(_("%d of %lu"), 0, cnt);
    else
        cnttext.Printf(_("%lu of %lu"), cnt, cnt);
    m_IndexCountInfo->SetLabel(cnttext);
    if (cnt > INDEX_IS_SMALL)
        return;

    for (size_t i = 0; i < cnt; i++)
        m_IndexList->Append((*m_mergedIndex)[i].name,
                            (char*)(&(*m_mergedIndex)[i]));

    m_IndexList->SetMinSize(wxSize(CONTENT_TREE_INDEX_MIN_WIDTH,
                                   m_IndexList->GetMinHeight()));
}

void wxHtmlHelpWindow::CreateSearch()
{
    if (! (m_SearchList && m_SearchChoice))
        return ;
    m_SearchList->Clear();
    m_SearchChoice->Clear();
    m_SearchChoice->Append(_("Search in all books"));
    const wxHtmlBookRecArray& bookrec = m_Data->GetBookRecArray();
    int i, cnt = bookrec.GetCount();
    for (i = 0; i < cnt; i++)
        m_SearchChoice->Append(bookrec[i].GetTitle());
    m_SearchChoice->SetSelection(0);
}

void wxHtmlHelpWindow::RefreshLists()
{
    // Update m_mergedIndex:
    UpdateMergedIndex();
    // Update the controls
    CreateContents();
    CreateIndex();
    CreateSearch();
}

#if wxUSE_CONFIG
void wxHtmlHelpWindow::ReadCustomization(wxConfigBase *cfg, const wxString& path)
{
    wxString oldpath;
    wxString tmp;

    if (!path.empty())
    {
        oldpath = cfg->GetPath();
        cfg->SetPath("/" + path);
    }

    m_Cfg.navig_on = cfg->Read("hcNavigPanel", m_Cfg.navig_on) != 0;
    m_Cfg.sashpos = cfg->Read("hcSashPos", m_Cfg.sashpos);
    m_Cfg.x = cfg->Read("hcX", m_Cfg.x);
    m_Cfg.y = cfg->Read("hcY", m_Cfg.y);
    m_Cfg.w = cfg->Read("hcW", m_Cfg.w);
    m_Cfg.h = cfg->Read("hcH", m_Cfg.h);

    m_FixedFace = cfg->Read("hcFixedFace", m_FixedFace);
    m_NormalFace = cfg->Read("hcNormalFace", m_NormalFace);
    m_FontSize = cfg->Read("hcBaseFontSize", m_FontSize);

    {
        int cnt;
        wxString val, s;

        cnt = cfg->Read("hcBookmarksCnt", 0L);
        if (cnt != 0)
        {
            int i;
            m_BookmarksNames.Clear();
            m_BookmarksPages.Clear();
            if (m_Bookmarks)
            {
                m_Bookmarks->Clear();
                m_Bookmarks->Append(_("(bookmarks)"));
            }

            for (i = 0; i < cnt; i++)
            {
                val.Printf("hcBookmark_%i", i);
                s = cfg->Read(val);
                m_BookmarksNames.push_back(s);
                if (m_Bookmarks) m_Bookmarks->Append(s);
                val.Printf("hcBookmark_%i_url", i);
                s = cfg->Read(val);
                m_BookmarksPages.push_back(s);
            }
        }
    }

    if (m_HtmlWin)
        m_HtmlWin->ReadCustomization(cfg);

    if (!path.empty())
        cfg->SetPath(oldpath);
}

void wxHtmlHelpWindow::WriteCustomization(wxConfigBase *cfg, const wxString& path)
{
    wxString oldpath;
    wxString tmp;

    if (!path.empty())
    {
        oldpath = cfg->GetPath();
        cfg->SetPath("/" + path);
    }

    cfg->Write("hcNavigPanel", m_Cfg.navig_on);
    cfg->Write("hcSashPos", (long)m_Cfg.sashpos);

    //  Don't write if iconized as this would make the window
    //  disappear next time it is shown!
    cfg->Write("hcX", (long)m_Cfg.x);
    cfg->Write("hcY", (long)m_Cfg.y);
    cfg->Write("hcW", (long)m_Cfg.w);
    cfg->Write("hcH", (long)m_Cfg.h);

    cfg->Write("hcFixedFace", m_FixedFace);
    cfg->Write("hcNormalFace", m_NormalFace);
    cfg->Write("hcBaseFontSize", (long)m_FontSize);

    if (m_Bookmarks)
    {
        int i;
        int cnt = m_BookmarksNames.GetCount();
        wxString val;

        cfg->Write("hcBookmarksCnt", (long)cnt);
        for (i = 0; i < cnt; i++)
        {
            val.Printf("hcBookmark_%i", i);
            cfg->Write(val, m_BookmarksNames[i]);
            val.Printf("hcBookmark_%i_url", i);
            cfg->Write(val, m_BookmarksPages[i]);
        }
    }

    if (m_HtmlWin)
        m_HtmlWin->WriteCustomization(cfg);

    if (!path.empty())
        cfg->SetPath(oldpath);
}
#endif // wxUSE_CONFIG

static void SetFontsToHtmlWin(wxHtmlWindow *win, const wxString& scalf, const wxString& fixf, int size)
{
    int f_sizes[7];
    f_sizes[0] = int(size * 0.6);
    f_sizes[1] = int(size * 0.8);
    f_sizes[2] = size;
    f_sizes[3] = int(size * 1.2);
    f_sizes[4] = int(size * 1.4);
    f_sizes[5] = int(size * 1.6);
    f_sizes[6] = int(size * 1.8);

    win->SetFonts(scalf, fixf, f_sizes);
}

class wxHtmlHelpWindowOptionsDialog : public wxDialog
{
public:
    wxComboBox *NormalFont, *FixedFont;
    wxSpinCtrl *FontSize;
    wxHtmlWindow *TestWin;

    explicit wxHtmlHelpWindowOptionsDialog(wxWindow *parent)
        : wxDialog(parent, wxID_ANY, wxString(_("Help Browser Options")))
    {
        wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
        wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 3, 2, 5);

        sizer->Add(new wxStaticText(this, wxID_ANY, _("Normal font:")));
        sizer->Add(new wxStaticText(this, wxID_ANY, _("Fixed font:")));
        sizer->Add(new wxStaticText(this, wxID_ANY, _("Font size:")));

        sizer->Add(NormalFont = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition,
                      wxSize(200, wxDefaultCoord),
                      {}, wxCB_DROPDOWN | wxCB_READONLY));

        sizer->Add(FixedFont = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition,
                      wxSize(200, wxDefaultCoord),
                      {}, wxCB_DROPDOWN | wxCB_READONLY));

        sizer->Add(FontSize = new wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition,
                      wxDefaultSize, wxSP_ARROW_KEYS, 2, 100, 2, "wxSpinCtrl"));

        topsizer->Add(sizer, 0, wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP, 10);

        topsizer->Add(new wxStaticText(this, wxID_ANY, _("Preview:")),
                        0, wxDirection::wxLEFT | wxDirection::wxTOP, 10);

        topsizer->AddSpacer(5);

        topsizer->Add(TestWin = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(20, 150),
                                                 wxHW_SCROLLBAR_AUTO|wxBORDER_THEME),
                        1, wxEXPAND | wxDirection::wxLEFT | wxDirection::wxRIGHT, 10);

        wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
        wxButton *ok;
        sizer2->Add(ok = new wxButton(this, wxID_OK), 0, wxALL, 10);
        ok->SetDefault();
        sizer2->Add(new wxButton(this, wxID_CANCEL), 0, wxALL, 10);
        topsizer->Add(sizer2, 0, wxALIGN_RIGHT);

        SetSizer(topsizer);
        topsizer->Fit(this);
        Centre(wxBOTH);
    }

    wxHtmlHelpWindowOptionsDialog(const wxHtmlHelpWindowOptionsDialog&) = delete;
	wxHtmlHelpWindowOptionsDialog& operator=(const wxHtmlHelpWindowOptionsDialog&) = delete;

    void UpdateTestWin()
    {
        wxBusyCursor bcur;
        SetFontsToHtmlWin(TestWin,
                          NormalFont->GetStringSelection(),
                          FixedFont->GetStringSelection(),
                          FontSize->GetValue());

        wxString content(_("font size"));

        content = "<font size=-2>" + content + " -2</font><br>"
                  "<font size=-1>" + content + " -1</font><br>"
                  "<font size=+0>" + content + " +0</font><br>"
                  "<font size=+1>" + content + " +1</font><br>"
                  "<font size=+2>" + content + " +2</font><br>"
                  "<font size=+3>" + content + " +3</font><br>"
                  "<font size=+4>" + content + " +4</font><br>";

        content = wxString( "<html><body><table><tr><td>" ) +
                  _("Normal face<br>and <u>underlined</u>. ") +
                  _("<i>Italic face.</i> ") +
                  _("<b>Bold face.</b> ") +
                  _("<b><i>Bold italic face.</i></b><br>") +
                  content +
                  wxString( "</td><td><tt>" ) +
                  _("Fixed size face.<br> <b>bold</b> <i>italic</i> ") +
                  _("<b><i>bold italic <u>underlined</u></i></b><br>") +
                  content +
                  "</tt></td></tr></table></body></html>";

        TestWin->SetPage( content );
    }

    void OnUpdate([[maybe_unused]] wxCommandEvent& event)
    {
        UpdateTestWin();
    }
    void OnUpdateSpin([[maybe_unused]] wxSpinEvent& event)
    {
        UpdateTestWin();
    }

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxHtmlHelpWindowOptionsDialog, wxDialog)
    EVT_COMBOBOX(wxID_ANY, wxHtmlHelpWindowOptionsDialog::OnUpdate)
    EVT_SPINCTRL(wxID_ANY, wxHtmlHelpWindowOptionsDialog::OnUpdateSpin)
wxEND_EVENT_TABLE()

void wxHtmlHelpWindow::OptionsDialog()
{
    wxHtmlHelpWindowOptionsDialog dlg(this);

    if (m_NormalFonts == nullptr)
    {
        m_NormalFonts = new std::vector<std::string>(wxFontEnumerator::GetFacenames());
        std::sort(m_NormalFonts->begin(), m_NormalFonts->end()); // ascending sort
    }
    if (m_FixedFonts == nullptr)
    {
        m_FixedFonts = new std::vector<std::string>(
                    wxFontEnumerator::GetFacenames(wxFONTENCODING_SYSTEM,
                    true /*enum fixed width only*/));
        std::sort(m_FixedFonts->begin(), m_FixedFonts->end()); // ascending sort
    }

    // VS: We want to show the font that is actually used by wxHtmlWindow.
    //     If customization dialog wasn't used yet, facenames are empty and
    //     wxHtmlWindow uses default fonts -- let's find out what they
    //     are so that we can pass them to the dialog:
    if (m_NormalFace.empty())
    {
        wxFont fnt(m_FontSize,
                   wxFontFamily::Swiss, wxFontStyle::Normal, wxFONTWEIGHT_NORMAL);
        m_NormalFace = fnt.GetFaceName();
    }
    if (m_FixedFace.empty())
    {
        wxFont fnt(m_FontSize,
                   wxFontFamily::Modern, wxFontStyle::Normal, wxFONTWEIGHT_NORMAL);
        m_FixedFace = fnt.GetFaceName();
    }

    // Lock updates to the choice controls before inserting potentially many
    // items into them until the end of this block.
    {
        wxWindowUpdateLocker lockNormalFont(dlg.NormalFont);
        wxWindowUpdateLocker lockFixedFont(dlg.FixedFont);

        for (const auto& normalFont : *m_NormalFonts)
            dlg.NormalFont->Append(normalFont);

        for (const auto& fixedFont : *m_FixedFonts)
            dlg.FixedFont->Append(fixedFont);

        if (!m_NormalFace.empty())
            dlg.NormalFont->SetStringSelection(m_NormalFace);
        else
            dlg.NormalFont->SetSelection(0);
        if (!m_FixedFace.empty())
            dlg.FixedFont->SetStringSelection(m_FixedFace);
        else
            dlg.FixedFont->SetSelection(0);
    }

    dlg.FontSize->SetValue(m_FontSize);
    dlg.UpdateTestWin();

    if (dlg.ShowModal() == wxID_OK)
    {
        m_NormalFace = dlg.NormalFont->GetStringSelection();
        m_FixedFace = dlg.FixedFont->GetStringSelection();
        m_FontSize = dlg.FontSize->GetValue();
        SetFontsToHtmlWin(m_HtmlWin, m_NormalFace, m_FixedFace, m_FontSize);
    }
}

void wxHtmlHelpWindow::NotifyPageChanged()
{
    if (m_UpdateContents && m_PagesHash)
    {
        wxString page = wxHtmlHelpHtmlWindow::GetOpenedPageWithAnchor(m_HtmlWin);
        wxHtmlHelpHashData *ha = nullptr;
        if (!page.empty())
            ha = (wxHtmlHelpHashData*) m_PagesHash->Get(page);

        if (ha)
        {
            bool olduc = m_UpdateContents;
            m_UpdateContents = false;
            m_ContentsBox->SelectItem(ha->m_Id);
            m_ContentsBox->EnsureVisible(ha->m_Id);
            m_UpdateContents = olduc;
        }
    }
}

/*
EVENT HANDLING :
*/


void wxHtmlHelpWindow::OnToolbar(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case wxID_HTML_BACK :
            m_HtmlWin->HistoryBack();
            break;

        case wxID_HTML_FORWARD :
            m_HtmlWin->HistoryForward();
            break;

        case wxID_HTML_UP :
            if (m_PagesHash)
            {
                wxString page = wxHtmlHelpHtmlWindow::GetOpenedPageWithAnchor(m_HtmlWin);
                wxHtmlHelpHashData *ha = nullptr;
                if (!page.empty())
                    ha = (wxHtmlHelpHashData*) m_PagesHash->Get(page);
                if (ha && ha->m_Index > 0)
                {
                    const wxHtmlHelpDataItem& it = m_Data->GetContentsArray()[ha->m_Index - 1];
                    if (!it.page.empty())
                    {
                        m_HtmlWin->LoadPage(it.GetFullPath());
                    }
                }
            }
            break;

        case wxID_HTML_UPNODE :
            if (m_PagesHash)
            {
                wxString page = wxHtmlHelpHtmlWindow::GetOpenedPageWithAnchor(m_HtmlWin);
                wxHtmlHelpHashData *ha = nullptr;
                if (!page.empty())
                    ha = (wxHtmlHelpHashData*) m_PagesHash->Get(page);
                if (ha && ha->m_Index > 0)
                {
                    int level =
                        m_Data->GetContentsArray()[ha->m_Index].level - 1;
                    int ind = ha->m_Index - 1;

                    const wxHtmlHelpDataItem *it =
                        &m_Data->GetContentsArray()[ind];
                    while (ind >= 0 && it->level != level)
                    {
                        ind--;
                        it = &m_Data->GetContentsArray()[ind];
                    }
                    if (ind >= 0)
                    {
                        if (!it->page.empty())
                            m_HtmlWin->LoadPage(it->GetFullPath());
                    }
                }
            }
            break;

        case wxID_HTML_DOWN :
            if (m_PagesHash)
            {
                wxString page = wxHtmlHelpHtmlWindow::GetOpenedPageWithAnchor(m_HtmlWin);
                wxHtmlHelpHashData *ha = nullptr;
                if (!page.empty())
                    ha = (wxHtmlHelpHashData*) m_PagesHash->Get(page);

                const wxHtmlHelpDataItems& contents = m_Data->GetContentsArray();
                if (ha && ha->m_Index < (int)contents.size() - 1)
                {
                    size_t idx = ha->m_Index + 1;

                    while (contents[idx].GetFullPath() == page) idx++;

                    if (!contents[idx].page.empty())
                        m_HtmlWin->LoadPage(contents[idx].GetFullPath());
                }
            }
            break;

        case wxID_HTML_PANEL :
            {
                if (! (m_Splitter && m_NavigPan))
                    return ;
                if (m_Splitter->IsSplit())
                {
                    m_Cfg.sashpos = m_Splitter->GetSashPosition();
                    m_Splitter->Unsplit(m_NavigPan);
                    m_Cfg.navig_on = false;
                }
                else
                {
                    m_NavigPan->Show();
                    m_HtmlWin->Show();
                    m_Splitter->SplitVertically(m_NavigPan, m_HtmlWin, m_Cfg.sashpos);
                    m_Cfg.navig_on = true;
                }
            }
            break;

        case wxID_HTML_OPTIONS :
            OptionsDialog();
            break;

        case wxID_HTML_BOOKMARKSADD :
            {
                wxString item;
                wxString url;

                item = m_HtmlWin->GetOpenedPageTitle();
                url = m_HtmlWin->GetOpenedPage();
                if (item.empty())
                    item = url.AfterLast(wxT('/'));
                if (m_BookmarksPages.Index(url) == wxNOT_FOUND)
                {
                    m_Bookmarks->Append(item);
                    m_BookmarksNames.push_back(item);
                    m_BookmarksPages.push_back(url);
                }
            }
            break;

        case wxID_HTML_BOOKMARKSREMOVE :
            {
                wxString item;
                int pos;

                item = m_Bookmarks->GetStringSelection();
                pos = m_BookmarksNames.Index(item);
                if (pos != wxNOT_FOUND)
                {
                    m_BookmarksNames.RemoveAt(pos);
                    m_BookmarksPages.RemoveAt(pos);
                    pos = m_Bookmarks->GetSelection();
                    wxASSERT_MSG( pos != wxNOT_FOUND , "Unknown bookmark position" ) ;
                    m_Bookmarks->Delete((unsigned int)pos);
                }
            }
            break;

#if wxUSE_PRINTING_ARCHITECTURE
        case wxID_HTML_PRINT :
            {
                if (m_Printer == nullptr)
                    m_Printer = new wxHtmlEasyPrinting(_("Help Printing"), this);
                if (m_HtmlWin->GetOpenedPage().empty())
                {
                    wxLogWarning(_("Cannot print empty page."));
                }
                else
                {
                    m_Printer->PrintFile(m_HtmlWin->GetOpenedPage());
                }
            }
            break;
#endif

        case wxID_HTML_OPENFILE :
            {
                wxString filemask = wxString(
                    _("HTML files (*.html;*.htm)|*.html;*.htm|")) +
                    _("Help books (*.htb)|*.htb|Help books (*.zip)|*.zip|") +
                    _("HTML Help Project (*.hhp)|*.hhp|") +
#if wxUSE_LIBMSPACK
                    _("Compressed HTML Help file (*.chm)|*.chm|") +
#endif
                    wxALL_FILES;
                wxString s = wxFileSelector(_("Open HTML document"),
                                            {},
                                            {},
                                            {},
                                            filemask,
                                            wxFD_OPEN | wxFD_FILE_MUST_EXIST,
                                            this);
                if (!s.empty())
                {
                    wxString ext = s.Right(4).Lower();
                    if (ext == ".zip" || ext == ".htb" ||
#if wxUSE_LIBMSPACK
                        ext == ".chm" ||
#endif
                        ext == ".hhp")
                    {
                        wxBusyCursor bcur;
                        m_Data->AddBook(s);
                        RefreshLists();
                    }
                    else
                        m_HtmlWin->LoadPage(s);
                }
            }
            break;
    }
}

void wxHtmlHelpWindow::OnContentsSel(wxTreeEvent& event)
{
    wxHtmlHelpTreeItemData *pg;

    pg = (wxHtmlHelpTreeItemData*) m_ContentsBox->GetItemData(event.GetItem());

    if (pg && m_UpdateContents)
    {
        const wxHtmlHelpDataItems& contents = m_Data->GetContentsArray();
        m_UpdateContents = false;
        if (!contents[pg->m_Id].page.empty())
            m_HtmlWin->LoadPage(contents[pg->m_Id].GetFullPath());
        m_UpdateContents = true;
    }
}

void wxHtmlHelpWindow::OnIndexSel([[maybe_unused]] wxCommandEvent& event)
{
    wxHtmlHelpMergedIndexItem *it = (wxHtmlHelpMergedIndexItem*)
        m_IndexList->GetClientData(m_IndexList->GetSelection());
    if (it)
        DisplayIndexItem(it);
}

void wxHtmlHelpWindow::OnIndexFind([[maybe_unused]] wxCommandEvent& event)
{
    DoIndexFind();
}

void wxHtmlHelpWindow::DoIndexFind()
{
    wxString sr = m_IndexText->GetLineText(0);
    sr.MakeLower();
    if (sr.empty())
    {
        DoIndexAll();
    }
    else
    {
        wxBusyCursor bcur;

        m_IndexList->Clear();
        const wxHtmlHelpMergedIndex& index = *m_mergedIndex;
        const unsigned cnt = index.size();

        int displ = 0;
        for (unsigned i = 0; i < cnt; i++)
        {
            if (index[i].name.Lower().find(sr) != wxString::npos)
            {
                int pos = m_IndexList->Append(index[i].name,
                                              (char*)(&index[i]));

                if (displ++ == 0)
                {
                    // don't automatically show topic selector if this
                    // item points to multiple pages:
                    if (index[i].items.size() == 1)
                    {
                        m_IndexList->SetSelection(0);
                        DisplayIndexItem(&index[i]);
                    }
                }

                // if this is nested item of the index, show its parent(s)
                // as well, otherwise it would not be clear what entry is
                // shown:
                wxHtmlHelpMergedIndexItem *parent = index[i].parent;
                while (parent)
                {
                    if (pos == 0 ||
                        (index.Index(*(wxHtmlHelpMergedIndexItem*)m_IndexList->GetClientData(pos-1))) < index.Index(*parent))
                    {
                        m_IndexList->Insert(parent->name,
                                            pos, (char*)parent);
                        parent = parent->parent;
                    }
                    else break;
                }

                // finally, it the item we just added is itself a parent for
                // other items, show them as well, because they are refinements
                // of the displayed index entry (i.e. it is implicitly contained
                // in them: "foo" with parent "bar" reads as "bar, foo"):
                int level = index[i].items[0]->level;
                i++;
                while (i < cnt && index[i].items[0]->level > level)
                {
                    m_IndexList->Append(index[i].name, (char*)(&index[i]));
                    i++;
                }
                i--;
            }
        }

        wxString cnttext;
        cnttext.Printf(_("%i of %u"), displ, cnt);
        m_IndexCountInfo->SetLabel(cnttext);

        m_IndexText->SetSelection(0, sr.length());
        m_IndexText->SetFocus();
    }
}

void wxHtmlHelpWindow::OnIndexAll([[maybe_unused]] wxCommandEvent& event)
{
    DoIndexAll();
}

void wxHtmlHelpWindow::DoIndexAll()
{
    wxBusyCursor bcur;

    m_IndexList->Clear();
    const wxHtmlHelpMergedIndex& index = *m_mergedIndex;
    const unsigned cnt = index.size();
    bool first = true;

    for (unsigned i = 0; i < cnt; i++)
    {
        m_IndexList->Append(index[i].name, (char*)(&index[i]));
        if (first)
        {
            // don't automatically show topic selector if this
            // item points to multiple pages:
            if (index[i].items.size() == 1)
            {
                DisplayIndexItem(&index[i]);
            }
            first = false;
        }
    }

    wxString cnttext;
    cnttext.Printf(_("%u of %u"), cnt, cnt);
    m_IndexCountInfo->SetLabel(cnttext);
}

void wxHtmlHelpWindow::OnSearchSel([[maybe_unused]] wxCommandEvent& event)
{
    wxHtmlHelpDataItem *it = (wxHtmlHelpDataItem*) m_SearchList->GetClientData(m_SearchList->GetSelection());
    if (it)
    {
        if (!it->page.empty())
            m_HtmlWin->LoadPage(it->GetFullPath());
    }
}

void wxHtmlHelpWindow::OnSearch([[maybe_unused]] wxCommandEvent& event)
{
    wxString sr = m_SearchText->GetLineText(0);

    if (!sr.empty())
        KeywordSearch(sr, wxHelpSearchMode::All);
}

void wxHtmlHelpWindow::OnBookmarksSel([[maybe_unused]] wxCommandEvent& event)
{
    wxString str = m_Bookmarks->GetStringSelection();
    int idx = m_BookmarksNames.Index(str);
    if (!str.empty() && str != _("(bookmarks)") && idx != wxNOT_FOUND)
    {
       m_HtmlWin->LoadPage(m_BookmarksPages[(size_t)idx]);
    }
}

void wxHtmlHelpWindow::OnSize([[maybe_unused]] wxSizeEvent& event)
{
    Layout();
}

#endif // wxUSE_WXHTML_HELP
