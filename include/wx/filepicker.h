/////////////////////////////////////////////////////////////////////////////
// Name:        wx/filepicker.h
// Purpose:     wxFilePickerCtrl, wxDirPickerCtrl base header
// Author:      Francesco Montorsi
// Modified by:
// Created:     14/4/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FILEDIRPICKER_H_BASE_
#define _WX_FILEDIRPICKER_H_BASE_

#if wxUSE_FILEPICKERCTRL || wxUSE_DIRPICKERCTRL

#include "wx/pickerbase.h"
#include "wx/filename.h"

#include <memory>
import <string>;

class wxDialog;
class wxFileDirPickerEvent;

inline constexpr char wxFilePickerCtrlNameStr[] = "filepicker";
inline constexpr char wxFilePickerWidgetNameStr[] = "filepickerwidget";
inline constexpr char wxDirPickerCtrlNameStr[] = "dirpicker";
inline constexpr char wxDirPickerWidgetNameStr[] = "dirpickerwidget";
inline constexpr char wxFilePickerWidgetLabel[] = wxTRANSLATE("Browse");
inline constexpr char wxDirPickerWidgetLabel[] = wxTRANSLATE("Browse");

// ----------------------------------------------------------------------------
// wxFileDirPickerEvent: used by wxFilePickerCtrl and wxDirPickerCtrl only
// ----------------------------------------------------------------------------

class wxFileDirPickerEvent : public wxCommandEvent
{
public:
    wxFileDirPickerEvent() = default;
    wxFileDirPickerEvent(wxEventType type, wxObject *generator, int id, const std::string &path)
        : wxCommandEvent(type, id),
          m_path(path)
    {
        SetEventObject(generator);
    }

	wxFileDirPickerEvent& operator=(const wxFileDirPickerEvent&) = delete;

    std::string GetPath() const { return m_path; }
    void SetPath(const std::string &p) { m_path = p; }

    // default copy ctor, assignment operator and dtor are ok
    // FIXME: Are they really?
    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxFileDirPickerEvent>(*this); }

private:
    std::string m_path;
};

wxDECLARE_EVENT( wxEVT_FILEPICKER_CHANGED, wxFileDirPickerEvent );
wxDECLARE_EVENT( wxEVT_DIRPICKER_CHANGED, wxFileDirPickerEvent );

// ----------------------------------------------------------------------------
// event types and macros
// ----------------------------------------------------------------------------

typedef void (wxEvtHandler::*wxFileDirPickerEventFunction)(wxFileDirPickerEvent&);

#define wxFileDirPickerEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxFileDirPickerEventFunction, func)

#define EVT_FILEPICKER_CHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FILEPICKER_CHANGED, id, wxFileDirPickerEventHandler(fn))
#define EVT_DIRPICKER_CHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_DIRPICKER_CHANGED, id, wxFileDirPickerEventHandler(fn))

// ----------------------------------------------------------------------------
// wxFileDirPickerWidgetBase: a generic abstract interface which must be
//                           implemented by controls used by wxFileDirPickerCtrlBase
// ----------------------------------------------------------------------------

class wxFileDirPickerWidgetBase
{
public:
    virtual ~wxFileDirPickerWidgetBase() = default;

    // Path here is the name of the selected file or directory.
    std::string GetPath() const { return m_path; }
    virtual void SetPath(const std::string &str) { m_path=str; }

    // Set the directory to open the file browse dialog at initially.
    virtual void SetInitialDirectory(const std::string& dir) = 0;

    // returns the picker widget cast to wxControl
    virtual wxControl *AsControl() = 0;

protected:
    virtual void UpdateDialogPath(wxDialog *) = 0;
    virtual void UpdatePathFromDialog(wxDialog *) = 0;

    std::string m_path;
};

// Styles which must be supported by all controls implementing wxFileDirPickerWidgetBase
// NB: these styles must be defined to carefully-chosen values to
//     avoid conflicts with wxButton's styles

inline constexpr unsigned int wxFLP_OPEN                    = 0x0400;
inline constexpr unsigned int wxFLP_SAVE                    = 0x0800;
inline constexpr unsigned int wxFLP_OVERWRITE_PROMPT        = 0x1000;
inline constexpr unsigned int wxFLP_FILE_MUST_EXIST         = 0x2000;
inline constexpr unsigned int wxFLP_CHANGE_DIR              = 0x4000;
inline constexpr unsigned int wxFLP_SMALL                   = wxPB_SMALL;

// NOTE: wxMULTIPLE is not supported !


inline constexpr unsigned int wxDIRP_DIR_MUST_EXIST         = 0x0008;
inline constexpr unsigned int wxDIRP_CHANGE_DIR             = 0x0010;
inline constexpr unsigned int wxDIRP_SMALL                  = wxPB_SMALL;


// map platform-dependent controls which implement the wxFileDirPickerWidgetBase
// under the name "wxFilePickerWidget" and "wxDirPickerWidget".
// NOTE: wxFileDirPickerCtrlBase will allocate a wx{File|Dir}PickerWidget and this
//       requires that all classes being mapped as wx{File|Dir}PickerWidget have the
//       same prototype for the constructor...
// since GTK >= 2.6, there is GtkFileButton
#if defined(__WXGTK20__) && !defined(__WXUNIVERSAL__)
    #include "wx/gtk/filepicker.h"
    using wxFilePickerWidget = wxFileButton;
    using wxDirPickerWidget  = wxDirButton;
#else
    #include "wx/generic/filepickerg.h"
    using wxFilePickerWidget = wxGenericFileButton;
    using wxDirPickerWidget  = wxGenericDirButton;
#endif



// ----------------------------------------------------------------------------
// wxFileDirPickerCtrlBase
// ----------------------------------------------------------------------------

class wxFileDirPickerCtrlBase : public wxPickerBase
{
protected:
    // NB: no default values since this function will never be used
    //     directly by the user and derived classes wouldn't use them
    bool CreateBase(wxWindow *parent,
                    wxWindowID id,
                    const std::string& path,
                    std::string_view message,
                    const std::string& wildcard,
                    const wxPoint& pos,
                    const wxSize& size,
                    unsigned int style,
                    const wxValidator& validator,
                    std::string_view name);

public:         // public API

    std::string GetPath() const;
    void SetPath(const std::string &str);

    // Set the directory to open the file browse dialog at initially.
    void SetInitialDirectory(const std::string& dir)
    {
        m_pickerIface->SetInitialDirectory(dir);
    }

public:        // internal functions

    void UpdatePickerFromTextCtrl() override;
    void UpdateTextCtrlFromPicker() override;

    // event handler for our picker
    void OnFileDirChange(wxFileDirPickerEvent &);

    // TRUE if any textctrl change should update the current working directory
    virtual bool IsCwdToUpdate() const = 0;

    // Returns the event type sent by this picker
    virtual wxEventType GetEventType() const = 0;

    virtual void DoConnect( wxControl *sender, wxFileDirPickerCtrlBase *eventSink ) = 0;

    // Returns the filtered value currently placed in the text control (if present).
    virtual std::string GetTextCtrlValue() const = 0;

protected:
    // creates the picker control
    virtual
    std::unique_ptr<wxFileDirPickerWidgetBase> CreatePicker(wxWindow *parent,
                                            const std::string& path,
                                            std::string_view message,
                                            const std::string& wildcard) = 0;

protected:

    // m_picker object as wxFileDirPickerWidgetBase interface
    std::unique_ptr<wxFileDirPickerWidgetBase> m_pickerIface;
};

#endif  // wxUSE_FILEPICKERCTRL || wxUSE_DIRPICKERCTRL


#if wxUSE_FILEPICKERCTRL

// ----------------------------------------------------------------------------
// wxFilePickerCtrl: platform-independent class which embeds the
// platform-dependent wxFilePickerWidget and, if wxFLP_USE_TEXTCTRL style is
// used, a textctrl next to it.
// ----------------------------------------------------------------------------

inline constexpr unsigned int wxFLP_USE_TEXTCTRL            = wxPB_USE_TEXTCTRL;

#ifdef __WXGTK__
    // GTK apps usually don't have a textctrl next to the picker
    inline constexpr unsigned int wxFLP_DEFAULT_STYLE       = wxFLP_OPEN|wxFLP_FILE_MUST_EXIST;
#else
    inline constexpr unsigned int wxFLP_DEFAULT_STYLE       = wxFLP_USE_TEXTCTRL|wxFLP_OPEN|wxFLP_FILE_MUST_EXIST;
#endif

class wxFilePickerCtrl : public wxFileDirPickerCtrlBase
{
public:
    wxFilePickerCtrl() = default;

    wxFilePickerCtrl(wxWindow *parent,
                     wxWindowID id,
                     const std::string& path = {},
                     std::string_view message = wxFileSelectorPromptStr,
                     const std::string& wildcard = wxFileSelectorDefaultWildcardStr,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxFLP_DEFAULT_STYLE,
                     const wxValidator& validator = {},
                     const std::string& name = wxFilePickerCtrlNameStr)
    {
        Create(parent, id, path, message, wildcard, pos, size, style,
               validator, name);
    }

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& path = {},
                std::string_view message = wxFileSelectorPromptStr,
                const std::string& wildcard = wxFileSelectorDefaultWildcardStr,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxFLP_DEFAULT_STYLE,
                const wxValidator& validator = {},
                std::string_view name = wxFilePickerCtrlNameStr);

    void SetFileName(const wxFileName &filename)
        { SetPath(filename.GetFullPath()); }

    wxFileName GetFileName() const
        { return wxFileName(GetPath()); }

public:     // overrides

    // return the text control value in canonical form
    std::string GetTextCtrlValue() const override;

    bool IsCwdToUpdate() const override
        { return HasFlag(wxFLP_CHANGE_DIR); }

    wxEventType GetEventType() const override
        { return wxEVT_FILEPICKER_CHANGED; }

    void DoConnect( wxControl *sender, wxFileDirPickerCtrlBase *eventSink ) override
    {
        sender->Bind(wxEVT_FILEPICKER_CHANGED,
            &wxFileDirPickerCtrlBase::OnFileDirChange, eventSink );
    }


protected:
    
    std::unique_ptr<wxFileDirPickerWidgetBase> CreatePicker(wxWindow *parent,
                                            const std::string& path,
                                            std::string_view message,
                                            const std::string& wildcard) override
    {
        return std::make_unique<wxFilePickerWidget>(parent, wxID_ANY,
                                      wxGetTranslation(wxFilePickerWidgetLabel).ToStdString(),
                                      path, message, wildcard,
                                      wxDefaultPosition, wxDefaultSize,
                                      GetPickerStyle(wxGetWindowStyle()));
    }

    // extracts the style for our picker from wxFileDirPickerCtrlBase's style
    unsigned int GetPickerStyle(unsigned int style) const override
    {
        return style & (wxFLP_OPEN |
                        wxFLP_SAVE |
                        wxFLP_OVERWRITE_PROMPT |
                        wxFLP_FILE_MUST_EXIST |
                        wxFLP_CHANGE_DIR |
                        wxFLP_USE_TEXTCTRL |
                        wxFLP_SMALL);
    }
};

#endif      // wxUSE_FILEPICKERCTRL


#if wxUSE_DIRPICKERCTRL

// ----------------------------------------------------------------------------
// wxDirPickerCtrl: platform-independent class which embeds the
// platform-dependent wxDirPickerWidget and eventually a textctrl
// (see wxDIRP_USE_TEXTCTRL) next to it.
// ----------------------------------------------------------------------------

inline constexpr unsigned int wxDIRP_USE_TEXTCTRL            = wxPB_USE_TEXTCTRL;

#ifdef __WXGTK__
    // GTK apps usually don't have a textctrl next to the picker
    inline constexpr unsigned int wxDIRP_DEFAULT_STYLE       = wxDIRP_DIR_MUST_EXIST;
#else
    inline constexpr unsigned int wxDIRP_DEFAULT_STYLE       = wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST;
#endif

class wxDirPickerCtrl : public wxFileDirPickerCtrlBase
{
public:
    wxDirPickerCtrl() = default;

    wxDirPickerCtrl(wxWindow *parent, wxWindowID id,
                    const std::string& path = {},
                    std::string_view message = wxDirSelectorPromptStr,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    unsigned int style = wxDIRP_DEFAULT_STYLE,
                    const wxValidator& validator = {},
                    std::string_view name = wxDirPickerCtrlNameStr)
    {
        Create(parent, id, path, message, pos, size, style, validator, name);
    }

    [[maybe_unused]] bool Create(wxWindow *parent, wxWindowID id,
                const std::string& path = {},
                std::string_view message = wxDirSelectorPromptStr,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDIRP_DEFAULT_STYLE,
                const wxValidator& validator = {},
                std::string_view name = wxDirPickerCtrlNameStr);

    void SetDirName(const wxFileName &dirname)
        { SetPath(dirname.GetPath()); }

    wxFileName GetDirName() const
        { return wxFileName::DirName(GetPath()); }

public:     // overrides

    std::string GetTextCtrlValue() const override;

    bool IsCwdToUpdate() const override
        { return HasFlag(wxDIRP_CHANGE_DIR); }

    wxEventType GetEventType() const override
        { return wxEVT_DIRPICKER_CHANGED; }

    void DoConnect( wxControl *sender, wxFileDirPickerCtrlBase *eventSink ) override
    {
        sender->Bind( wxEVT_DIRPICKER_CHANGED,
            &wxFileDirPickerCtrlBase::OnFileDirChange, eventSink );
    }


protected:
    
    std::unique_ptr<wxFileDirPickerWidgetBase> CreatePicker(wxWindow *parent,
                                            const std::string& path,
                                            std::string_view message,
                                            [[maybe_unused]] const std::string& wildcard) override
    {
        return std::make_unique<wxDirPickerWidget>(parent, wxID_ANY,
                                     wxGetTranslation(wxDirPickerWidgetLabel).ToStdString(),
                                     path, message,
                                     wxDefaultPosition, wxDefaultSize,
                                     GetPickerStyle(wxGetWindowStyle()));
    }

    // extracts the style for our picker from wxFileDirPickerCtrlBase's style
    unsigned int GetPickerStyle(unsigned int style) const override
    {
        return style & (wxDIRP_DIR_MUST_EXIST |
                        wxDIRP_CHANGE_DIR |
                        wxDIRP_USE_TEXTCTRL |
                        wxDIRP_SMALL);
    }
};

#endif      // wxUSE_DIRPICKERCTRL

#endif // _WX_FILEDIRPICKER_H_BASE_

