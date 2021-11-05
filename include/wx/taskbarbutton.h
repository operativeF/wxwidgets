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

#include "wx/geometry/rect.h"
#include "wx/icon.h"

#include <string>
#include <vector>

class wxTaskBarButton;
class wxTaskBarJumpListCategory;
class wxTaskBarJumpList;
class wxTaskBarJumpListImpl;

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

class wxThumbBarButton : public wxObject
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

    std::string m_tooltip;
    
    wxIcon m_icon;

    wxTaskBarButton *m_taskBarButtonParent{nullptr};

    int m_id{0};

    bool m_enable{true};
    bool m_dismissOnClick{false};
    bool m_hasBackground{true};
    bool m_shown{true};
    bool m_interactive{true};

    wxDECLARE_DYNAMIC_CLASS(wxThumbBarButton);
};

class wxTaskBarButton
{
public:
    wxTaskBarButton& operator=(wxTaskBarButton&&) = delete;

    // Factory function, may return NULL if task bar buttons are not supported
    // by the current system.
    static std::unique_ptr<wxTaskBarButton> Create(wxWindow* parent);

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

class wxTaskBarJumpListItem
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

    wxTaskBarJumpListItem& operator=(wxTaskBarJumpListItem&&) = delete;

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
    std::string m_title;
    std::string m_filePath;
    std::string m_arguments;
    std::string m_tooltip;
    std::string m_iconPath;

    wxTaskBarJumpListCategory *m_parentCategory;

    int      m_iconIndex{0};
    wxTaskBarJumpListItemType m_type;
};

using wxTaskBarJumpListItems = std::vector<wxTaskBarJumpListItem *>;

class wxTaskBarJumpListCategory
{
public:
    wxTaskBarJumpListCategory(wxTaskBarJumpList *parent = nullptr,
                              const std::string& title = {});
    virtual ~wxTaskBarJumpListCategory();

    wxTaskBarJumpListCategory& operator=(wxTaskBarJumpListCategory&&) = delete;

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

class wxTaskBarJumpList
{
public:
    wxTaskBarJumpList(const std::string& appID = {});
    virtual ~wxTaskBarJumpList();

    wxTaskBarJumpList& operator=(wxTaskBarJumpList&&) = delete;

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
