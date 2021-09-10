/////////////////////////////////////////////////////////////////////////////
// Name:        include/taskbarbutton.h
// Purpose:     Defines wxTaskBarButton class for manipulating buttons on the
//              windows taskbar.
// Author:      Chaobin Zhang <zhchbin@gmail.com>
// Created:     2014-04-30
// Copyright:   (c) 2014 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TASKBARBUTTON_H_
#define _WX_TASKBARBUTTON_H_

#include "wx/defs.h"

#if wxUSE_TASKBARBUTTON

#include "wx/icon.h"

#include <string>
#include <vector>

class WXDLLIMPEXP_FWD_CORE wxTaskBarButton;
class WXDLLIMPEXP_FWD_CORE wxTaskBarJumpListCategory;
class WXDLLIMPEXP_FWD_CORE wxTaskBarJumpList;
class WXDLLIMPEXP_FWD_CORE wxTaskBarJumpListImpl;

// ----------------------------------------------------------------------------
// wxTaskBarButton: define wxTaskBarButton interface.
// ----------------------------------------------------------------------------

/**
    State of the task bar button.
*/
enum class wxTaskBarButtonState
{
    NoProgress    = 0,
    Indeterminate = 1,
    Normal        = 2,
    Error         = 4,
    Paused        = 8
};

class WXDLLIMPEXP_CORE wxThumbBarButton : public wxObject
{
public:
    wxThumbBarButton() = default;

    wxThumbBarButton(int id,
                     const wxIcon& icon,
                     const std::string& tooltip = {},
                     bool enable = true,
                     bool dismissOnClick = false,
                     bool hasBackground = true,
                     bool shown = true,
                     bool interactive = true);

    [[maybe_unused]] bool Create(int id,
                const wxIcon& icon,
                const std::string& tooltip = {},
                bool enable = true,
                bool dismissOnClick = false,
                bool hasBackground = true,
                bool shown = true,
                bool interactive = true);
    int GetID() const { return m_id; }
    const wxIcon& GetIcon() const { return m_icon; }
    const std::string& GetTooltip() const { return m_tooltip; }

    bool IsEnable() const { return m_enable; }
    void Enable(bool enable = true);
    void Disable() { Enable(false); }

    bool IsDismissOnClick() const { return m_dismissOnClick; }
    void EnableDismissOnClick(bool enable = true);
    void DisableDimissOnClick() { EnableDismissOnClick(false); }

    bool HasBackground() const { return m_hasBackground; }
    void SetHasBackground(bool has = true);

    bool IsShown() const { return m_shown; }
    void Show(bool shown = true);
    void Hide() { Show(false); }

    bool IsInteractive() const { return m_interactive; }
    void SetInteractive(bool interactive = true);

    void SetParent(wxTaskBarButton *parent) { m_taskBarButtonParent = parent; }
    wxTaskBarButton* GetParent() const { return m_taskBarButtonParent; }

private:
    bool UpdateParentTaskBarButton();

    int m_id{0};
    wxIcon m_icon;
    std::string m_tooltip;
    bool m_enable{true};
    bool m_dismissOnClick{false};
    bool m_hasBackground{true};
    bool m_shown{true};
    bool m_interactive{true};
    wxTaskBarButton *m_taskBarButtonParent{nullptr};

    wxDECLARE_DYNAMIC_CLASS(wxThumbBarButton);
};

class WXDLLIMPEXP_CORE wxTaskBarButton
{
public:

   wxTaskBarButton(const wxTaskBarButton&) = delete;
   wxTaskBarButton& operator=(const wxTaskBarButton&) = delete;
   wxTaskBarButton(wxTaskBarButton&&) = default;
   wxTaskBarButton& operator=(wxTaskBarButton&&) = default;

    // Factory function, may return NULL if task bar buttons are not supported
    // by the current system.
    static wxTaskBarButton* New(wxWindow* parent);

    virtual ~wxTaskBarButton() = default;

    // Operations:
    virtual void SetProgressRange(int range) = 0;
    virtual void SetProgressValue(int value) = 0;
    virtual void PulseProgress() = 0;
    virtual void Show(bool show = true) = 0;
    virtual void Hide() = 0;
    virtual void SetThumbnailTooltip(const std::string& tooltip) = 0;
    virtual void SetProgressState(wxTaskBarButtonState state) = 0;
    virtual void SetOverlayIcon(const wxIcon& icon,
                                const std::string& description = {}) = 0;
    virtual void SetThumbnailClip(const wxRect& rect) = 0;
    virtual void SetThumbnailContents(const wxWindow *child) = 0;
    virtual bool InsertThumbBarButton(size_t pos, wxThumbBarButton *button) = 0;
    virtual bool AppendThumbBarButton(wxThumbBarButton *button) = 0;
    virtual bool AppendSeparatorInThumbBar() = 0;
    virtual wxThumbBarButton* RemoveThumbBarButton(wxThumbBarButton *button) = 0;
    virtual wxThumbBarButton* RemoveThumbBarButton(int id) = 0;
    virtual void Realize() = 0;

protected:
    wxTaskBarButton() = default;
};

enum class wxTaskBarJumpListItemType
{
    Separator,
    Task,
    Destination
};

class WXDLLIMPEXP_CORE wxTaskBarJumpListItem
{
public:
    wxTaskBarJumpListItem(wxTaskBarJumpListCategory *parentCategory = nullptr,
        wxTaskBarJumpListItemType type = wxTaskBarJumpListItemType::Separator,
        const std::string& title = {},
        const std::string& filePath = {},
        const std::string& arguments = {},
        const std::string& tooltip = {},
        const std::string& iconPath = {},
        int iconIndex = 0);

   wxTaskBarJumpListItem(const wxTaskBarJumpListItem&) = delete;
   wxTaskBarJumpListItem& operator=(const wxTaskBarJumpListItem&) = delete;
   wxTaskBarJumpListItem(wxTaskBarJumpListItem&&) = default;
   wxTaskBarJumpListItem& operator=(wxTaskBarJumpListItem&&) = default;

    wxTaskBarJumpListItemType GetType() const;
    void SetType(wxTaskBarJumpListItemType type);
    const std::string& GetTitle() const;
    void SetTitle(const std::string& title);
    const std::string& GetFilePath() const;
    void SetFilePath(const std::string& filePath);
    const std::string& GetArguments() const;
    void SetArguments(const std::string& arguments);
    const std::string& GetTooltip() const;
    void SetTooltip(const std::string& tooltip);
    const std::string& GetIconPath() const;
    void SetIconPath(const std::string& iconPath);
    int GetIconIndex() const;
    void SetIconIndex(int iconIndex);
    wxTaskBarJumpListCategory* GetCategory() const;
    void SetCategory(wxTaskBarJumpListCategory *category);

private:
    wxTaskBarJumpListCategory *m_parentCategory;
    wxTaskBarJumpListItemType m_type;
    std::string m_title;
    std::string m_filePath;
    std::string m_arguments;
    std::string m_tooltip;
    std::string m_iconPath;
    int      m_iconIndex{0};
};

using wxTaskBarJumpListItems = std::vector<wxTaskBarJumpListItem *>;

class WXDLLIMPEXP_CORE wxTaskBarJumpListCategory
{
public:
    wxTaskBarJumpListCategory(wxTaskBarJumpList *parent = nullptr,
                              const std::string& title = {});
    virtual ~wxTaskBarJumpListCategory();

   wxTaskBarJumpListCategory(const wxTaskBarJumpListCategory&) = delete;
   wxTaskBarJumpListCategory& operator=(const wxTaskBarJumpListCategory&) = delete;
   wxTaskBarJumpListCategory(wxTaskBarJumpListCategory&&) = default;
   wxTaskBarJumpListCategory& operator=(wxTaskBarJumpListCategory&&) = default;

    wxTaskBarJumpListItem* Append(wxTaskBarJumpListItem *item);
    void Delete(wxTaskBarJumpListItem *item);
    wxTaskBarJumpListItem* Remove(wxTaskBarJumpListItem *item);
    wxTaskBarJumpListItem* FindItemByPosition(size_t pos) const;
    wxTaskBarJumpListItem* Insert(size_t pos, wxTaskBarJumpListItem *item);
    wxTaskBarJumpListItem* Prepend(wxTaskBarJumpListItem *item);
    void SetTitle(const std::string& title);
    const std::string& GetTitle() const;
    const wxTaskBarJumpListItems& GetItems() const;

private:
    friend class wxTaskBarJumpListItem;

    void Update();

    wxTaskBarJumpList *m_parent;
    wxTaskBarJumpListItems m_items;
    std::string m_title;
};

using wxTaskBarJumpListCategories = std::vector<wxTaskBarJumpListCategory *>;

class WXDLLIMPEXP_CORE wxTaskBarJumpList
{
public:
    wxTaskBarJumpList(const std::string& appID = {});
    virtual ~wxTaskBarJumpList();

   wxTaskBarJumpList(const wxTaskBarJumpList&) = delete;
   wxTaskBarJumpList& operator=(const wxTaskBarJumpList&) = delete;
   wxTaskBarJumpList(wxTaskBarJumpList&&) = default;
   wxTaskBarJumpList& operator=(wxTaskBarJumpList&&) = default;

    void ShowRecentCategory(bool shown = true);
    void HideRecentCategory();
    void ShowFrequentCategory(bool shown = true);
    void HideFrequentCategory();

    wxTaskBarJumpListCategory& GetTasks() const;
    const wxTaskBarJumpListCategory& GetFrequentCategory() const;
    const wxTaskBarJumpListCategory& GetRecentCategory() const;
    const wxTaskBarJumpListCategories& GetCustomCategories() const;

    void AddCustomCategory(wxTaskBarJumpListCategory* category);
    wxTaskBarJumpListCategory* RemoveCustomCategory(const std::string& title);
    void DeleteCustomCategory(const std::string& title);

private:
    friend class wxTaskBarJumpListCategory;

    void Update();
    wxTaskBarJumpListImpl *m_jumpListImpl;
};

#endif // wxUSE_TASKBARBUTTON

#endif  // _WX_TASKBARBUTTON_H_
