/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dialog.h
// Purpose:     wxDialogBase class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     29.06.99
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DIALOG_H_BASE_
#define _WX_DIALOG_H_BASE_

#include "wx/toplevel.h"
#include "wx/containr.h"

#include <memory>

class WXDLLIMPEXP_FWD_CORE wxSizer;
class WXDLLIMPEXP_FWD_CORE wxStdDialogButtonSizer;
class WXDLLIMPEXP_FWD_CORE wxBoxSizer;
class WXDLLIMPEXP_FWD_CORE wxDialogLayoutAdapter;
class WXDLLIMPEXP_FWD_CORE wxDialog;
class WXDLLIMPEXP_FWD_CORE wxButton;
class WXDLLIMPEXP_FWD_CORE wxScrolledWindow;
class wxTextSizerWrapper;

// Also see the bit summary table in wx/toplevel.h.

inline constexpr int wxDIALOG_NO_PARENT = 0x00000020;  // Don't make owned by apps top window

inline constexpr int wxDEFAULT_DIALOG_STYLE = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;

// Layout adaptation levels, for SetLayoutAdaptationLevel

// Don't do any layout adaptation
inline constexpr int wxDIALOG_ADAPTATION_NONE = 0;

// Only look for wxStdDialogButtonSizer for non-scrolling part
inline constexpr int wxDIALOG_ADAPTATION_STANDARD_SIZER = 1;

// Also look for any suitable sizer for non-scrolling part
inline constexpr int wxDIALOG_ADAPTATION_ANY_SIZER = 2;

// Also look for 'loose' standard buttons for non-scrolling part
inline constexpr int wxDIALOG_ADAPTATION_LOOSE_BUTTONS = 3;

// Layout adaptation mode, for SetLayoutAdaptationMode
enum class wxDialogLayoutAdaptationMode
{
    Default,   // use global adaptation enabled status
    Enabled,   // enable this dialog overriding global status
    Disabled   // disable this dialog overriding global status
};

enum class wxDialogModality
{
    None,
    WindowModal,
    AppModal
};

inline constexpr char wxDialogNameStr[] = "dialog";

class WXDLLIMPEXP_CORE wxDialogBase : public wxNavigationEnabled<wxTopLevelWindow>
{
public:
    wxDialogBase();
    ~wxDialogBase() = default;

    wxDialogBase(const wxDialogBase&) = delete;
    wxDialogBase& operator=(const wxDialogBase&) = delete;
    wxDialogBase(wxDialogBase&&) = default;
    wxDialogBase& operator=(wxDialogBase&&) = default;

    // define public wxDialog methods to be implemented by the derived classes
    virtual int ShowModal() = 0;
    virtual void EndModal(int retCode) = 0;
    virtual bool IsModal() const = 0;
    // show the dialog frame-modally (needs a parent), using app-modal
    // dialogs on platforms that don't support it
    virtual void ShowWindowModal () ;
    virtual void SendWindowModalDialogEvent ( wxEventType type );

    template<typename Functor>
    void ShowWindowModalThenDo(const Functor& onEndModal);

    // Modal dialogs have a return code - usually the id of the last
    // pressed button
    void SetReturnCode(int returnCode) { m_returnCode = returnCode; }
    int GetReturnCode() const { return m_returnCode; }

    // Set the identifier for the affirmative button: this button will close
    // the dialog after validating data and calling TransferDataFromWindow()
    void SetAffirmativeId(int affirmativeId);
    int GetAffirmativeId() const { return m_affirmativeId; }

    // Set identifier for Esc key translation: the button with this id will
    // close the dialog without doing anything else; special value wxID_NONE
    // means to not handle Esc at all while wxID_ANY means to map Esc to
    // wxID_CANCEL if present and GetAffirmativeId() otherwise
    void SetEscapeId(int escapeId);
    int GetEscapeId() const { return m_escapeId; }

    // Find the parent to use for modal dialog: try to use the specified parent
    // but fall back to the current active window or main application window as
    // last resort if it is unsuitable.
    //
    // As this function is often called from the ctor, the window style may be
    // not set yet and hence must be passed explicitly to it so that we could
    // check whether it contains wxDIALOG_NO_PARENT bit.
    //
    // This function always returns a valid top level window or NULL.
    wxWindow *GetParentForModalDialog(wxWindow *parent, long style) const;

    // This overload can only be used for already initialized windows, i.e. not
    // from the ctor. It uses the current window parent and style.
    wxWindow *GetParentForModalDialog() const
    {
        return GetParentForModalDialog(GetParent(), GetWindowStyle());
    }

#if wxUSE_STATTEXT // && wxUSE_TEXTCTRL
    // splits text up at newlines and places the lines into a vertical
    // wxBoxSizer, with the given maximum width, lines will not be wrapped
    // for negative values of widthMax
    wxSizer *CreateTextSizer(const std::string& message, int widthMax = -1);

    // same as above but uses a customized wxTextSizerWrapper to create
    // non-standard controls for the lines
    wxSizer *CreateTextSizer(const std::string& message,
                             wxTextSizerWrapper& wrapper,
                             int widthMax = -1);
#endif // wxUSE_STATTEXT // && wxUSE_TEXTCTRL

    // returns a horizontal wxBoxSizer containing the given buttons
    //
    // notice that the returned sizer can be NULL if no buttons are put in the
    // sizer (this mostly happens under smart phones and other atypical
    // platforms which have hardware buttons replacing OK/Cancel and such)
    wxSizer *CreateButtonSizer(long flags);

    // returns a sizer containing the given one and a static line separating it
    // from the preceding elements if it's appropriate for the current platform
    wxSizer *CreateSeparatedSizer(wxSizer *sizer);

    // returns the sizer containing CreateButtonSizer() below a separating
    // static line for the platforms which use static lines for items
    // separation (i.e. not Mac)
    //
    // this is just a combination of CreateButtonSizer() and
    // CreateSeparatedSizer()
    wxSizer *CreateSeparatedButtonSizer(long flags);

#if wxUSE_BUTTON
    wxStdDialogButtonSizer *CreateStdDialogButtonSizer( long flags );
#endif // wxUSE_BUTTON

    // Do layout adaptation
    virtual bool DoLayoutAdaptation();

    // Can we do layout adaptation?
    virtual bool CanDoLayoutAdaptation();

    // Returns a content window if there is one. This can be used by the layout adapter, for
    // example to make the pages of a book control into scrolling windows
    virtual wxWindow* GetContentWindow() const { return nullptr; }

    // Add an id to the list of main button identifiers that should be in the button sizer
    void AddMainButtonId(wxWindowID id) { m_mainButtonIds.push_back((int) id); }
    std::vector<int>& GetMainButtonIds() { return m_mainButtonIds; }

    // Is this id in the main button id array?
    bool IsMainButtonId(wxWindowID id) const
    {
        return std::find(m_mainButtonIds.begin(), m_mainButtonIds.end(),
            ((int) id)) != std::end(m_mainButtonIds);
    }

    // Level of adaptation, from none (Level 0) to full (Level 3). To disable adaptation,
    // set level 0, for example in your dialog constructor. You might
    // do this if you know that you are displaying on a large screen and you don't want the
    // dialog changed.
    void SetLayoutAdaptationLevel(int level) { m_layoutAdaptationLevel = level; }
    int GetLayoutAdaptationLevel() const { return m_layoutAdaptationLevel; }

    /// Override global adaptation enabled/disabled status
    void SetLayoutAdaptationMode(wxDialogLayoutAdaptationMode mode) { m_layoutAdaptationMode = mode; }
    wxDialogLayoutAdaptationMode GetLayoutAdaptationMode() const { return m_layoutAdaptationMode; }

    // Returns true if the adaptation has been done
    void SetLayoutAdaptationDone(bool adaptationDone) { m_layoutAdaptationDone = adaptationDone; }
    bool GetLayoutAdaptationDone() const { return m_layoutAdaptationDone; }

    // Set layout adapter class, returning old adapter
    static wxDialogLayoutAdapter* SetLayoutAdapter(wxDialogLayoutAdapter* adapter);
    static wxDialogLayoutAdapter* GetLayoutAdapter() { return sm_layoutAdapter; }

    // Global switch for layout adaptation
    static bool IsLayoutAdaptationEnabled() { return sm_layoutAdaptation; }
    static void EnableLayoutAdaptation(bool enable) { sm_layoutAdaptation = enable; }

    // modality kind
    virtual wxDialogModality GetModality() const;
protected:
    // emulate click of a button with the given id if it's present in the dialog
    //
    // return true if button was "clicked" or false if we don't have it
    bool EmulateButtonClickIfPresent(int id);

    // this function is used by OnCharHook() to decide whether the given key
    // should close the dialog
    //
    // for most platforms the default implementation (which just checks for
    // Esc) is sufficient, but Mac port also adds Cmd-. here and other ports
    // could do something different if needed
    virtual bool IsEscapeKey(const wxKeyEvent& event);

    // end either modal or modeless dialog, for the modal dialog rc is used as
    // the dialog return code
    void EndDialog(int rc);

    // call Validate() and TransferDataFromWindow() and close dialog with
    // wxID_OK return code
    void AcceptAndClose();

    // The return code from modal dialog
    int m_returnCode{0};

    // The identifier for the affirmative button (usually wxID_OK)
    int m_affirmativeId{wxID_OK};

    // The identifier for cancel button (usually wxID_CANCEL)
    int m_escapeId{wxID_ANY};

    // Flags whether layout adaptation has been done for this dialog
    bool                                m_layoutAdaptationDone{false};

    // Extra button identifiers to be taken as 'main' button identifiers
    // to be placed in the non-scrolling area
    std::vector<int>                          m_mainButtonIds;

    // Adaptation level
    int                                 m_layoutAdaptationLevel{3};

    // Local override for global adaptation enabled status
    wxDialogLayoutAdaptationMode        m_layoutAdaptationMode{wxDialogLayoutAdaptationMode::Default};

    // Global layout adapter
    inline static wxDialogLayoutAdapter*       sm_layoutAdapter{nullptr};

    // Global adaptation switch
    inline static bool                         sm_layoutAdaptation{false};

private:
    // helper of GetParentForModalDialog(): returns the passed in window if it
    // can be used as our parent or NULL if it can't
    wxWindow *CheckIfCanBeUsedAsParent(wxWindow *parent) const;

    // Helper of OnCharHook() and OnCloseWindow(): find the appropriate button
    // for closing the dialog and send a click event for it.
    //
    // Return true if we found a button to close the dialog and "clicked" it or
    // false otherwise.
    bool SendCloseButtonClickEvent();

    // handle Esc key presses
    void OnCharHook(wxKeyEvent& event);

    // handle closing the dialog window
    void OnCloseWindow(wxCloseEvent& event);

    // handle the standard buttons
    void OnButton(wxCommandEvent& event);

    // update the background colour
    void OnSysColourChanged(wxSysColourChangedEvent& event);

    wxDECLARE_EVENT_TABLE();
};

/*!
 * Base class for layout adapters - code that, for example, turns a dialog into a
 * scrolling dialog if there isn't enough screen space. You can derive further
 * adapter classes to do any other kind of adaptation, such as applying a watermark, or adding
 * a help mechanism.
 */

class WXDLLIMPEXP_CORE wxDialogLayoutAdapter: public wxObject
{
    wxDECLARE_CLASS(wxDialogLayoutAdapter);
public:
    wxDialogLayoutAdapter() = default;

    // Override this function to indicate that adaptation should be done
    virtual bool CanDoLayoutAdaptation(wxDialog* dialog) = 0;

    // Override this function to do the adaptation
    virtual bool DoLayoutAdaptation(wxDialog* dialog) = 0;
};

/*!
 * Standard adapter. Does scrolling adaptation for paged and regular dialogs.
 *
 */

class WXDLLIMPEXP_CORE wxStandardDialogLayoutAdapter: public wxDialogLayoutAdapter
{
    wxDECLARE_CLASS(wxStandardDialogLayoutAdapter);
public:
    wxStandardDialogLayoutAdapter() = default;

// Overrides

    // Indicate that adaptation should be done
    bool CanDoLayoutAdaptation(wxDialog* dialog) override;

    // Do layout adaptation
    bool DoLayoutAdaptation(wxDialog* dialog) override;

// Implementation

    // Create the scrolled window
    virtual wxScrolledWindow* CreateScrolledWindow(wxWindow* parent);

#if wxUSE_BUTTON
    // Find a standard or horizontal box sizer
    virtual wxSizer* FindButtonSizer(bool stdButtonSizer, wxDialog* dialog, wxSizer* sizer, int& retBorder, int accumlatedBorder = 0);

    // Check if this sizer contains standard buttons, and so can be repositioned in the dialog
    virtual bool IsOrdinaryButtonSizer(wxDialog* dialog, wxBoxSizer* sizer);

    // Check if this is a standard button
    virtual bool IsStandardButton(wxDialog* dialog, wxButton* button);

    // Find 'loose' main buttons in the existing layout and add them to the standard dialog sizer
    virtual bool FindLooseButtons(wxDialog* dialog, wxStdDialogButtonSizer* buttonSizer, wxSizer* sizer, int& count);
#endif // wxUSE_BUTTON

    // Reparent the controls to the scrolled window, except those in buttonSizer
    virtual void ReparentControls(wxWindow* parent, wxWindow* reparentTo, wxSizer* buttonSizer = nullptr);
    static void DoReparentControls(wxWindow* parent, wxWindow* reparentTo, wxSizer* buttonSizer = nullptr);

    // A function to fit the dialog around its contents, and then adjust for screen size.
    // If scrolled windows are passed, scrolling is enabled in the required orientation(s).
    virtual bool FitWithScrolling(wxDialog* dialog, wxScrolledWindow* scrolledWindow);
    virtual bool FitWithScrolling(wxDialog* dialog, wxWindowList& windows);
    static bool DoFitWithScrolling(wxDialog* dialog, wxScrolledWindow* scrolledWindow);
    static bool DoFitWithScrolling(wxDialog* dialog, wxWindowList& windows);

    // Find whether scrolling will be necessary for the dialog, returning wxVERTICAL, wxHORIZONTAL or both
    virtual int MustScroll(wxDialog* dialog, wxSize& windowSize, wxSize& displaySize);
    static int DoMustScroll(wxDialog* dialog, wxSize& windowSize, wxSize& displaySize);
};

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/dialog.h"
#else
    #if defined(__WXMSW__)
        #include "wx/msw/dialog.h"
    #elif defined(__WXMOTIF__)
        #include "wx/motif/dialog.h"
    #elif defined(__WXGTK20__)
        #include "wx/gtk/dialog.h"
    #elif defined(__WXGTK__)
        #include "wx/gtk1/dialog.h"
    #elif defined(__WXMAC__)
        #include "wx/osx/dialog.h"
    #elif defined(__WXQT__)
        #include "wx/qt/dialog.h"
    #endif
#endif

class WXDLLIMPEXP_CORE wxWindowModalDialogEvent  : public wxCommandEvent
{
public:
    wxWindowModalDialogEvent (wxEventType commandType = wxEVT_NULL, int id = 0)
        : wxCommandEvent(commandType, id) { }

	wxWindowModalDialogEvent& operator=(const wxWindowModalDialogEvent&) = delete;

    wxDialog *GetDialog() const
        { return wxStaticCast(GetEventObject(), wxDialog); }

    int GetReturnCode() const
        { return GetDialog()->GetReturnCode(); }

    wxEvent *Clone() const override { return new wxWindowModalDialogEvent (*this); }

public:
	wxClassInfo *GetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_WINDOW_MODAL_DIALOG_CLOSED , wxWindowModalDialogEvent );

typedef void (wxEvtHandler::*wxWindowModalDialogEventFunction)(wxWindowModalDialogEvent &);

#define wxWindowModalDialogEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxWindowModalDialogEventFunction, func)

#define EVT_WINDOW_MODAL_DIALOG_CLOSED(winid, func) \
    wx__DECLARE_EVT1(wxEVT_WINDOW_MODAL_DIALOG_CLOSED, winid, wxWindowModalDialogEventHandler(func))

template<typename Functor>
class wxWindowModalDialogEventFunctor
{
public:
    wxWindowModalDialogEventFunctor(const Functor& f)
        : m_f(new Functor(f))
    {}

    void operator()(wxWindowModalDialogEvent& event)
    {
        if ( m_f )
        {
            // We only want to call this handler once. Also, by deleting
            // the functor here, its data (such as wxWindowPtr pointing to
            // the dialog) are freed immediately after exiting this operator().
            std::shared_ptr<Functor> functor(m_f);
            m_f.reset();

            (*functor)(event.GetReturnCode());
        }
        else // was already called once
        {
            event.Skip();
        }
    }

private:
    std::shared_ptr<Functor> m_f;
};

template<typename Functor>
void wxDialogBase::ShowWindowModalThenDo(const Functor& onEndModal)
{
    Bind(wxEVT_WINDOW_MODAL_DIALOG_CLOSED,
         wxWindowModalDialogEventFunctor<Functor>(onEndModal));
    ShowWindowModal();
}

#endif
    // _WX_DIALOG_H_BASE_
