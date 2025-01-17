///////////////////////////////////////////////////////////////////////////////
// Name:        wx/modalhook.h
// Purpose:     Allows to hook into showing modal dialogs.
// Author:      Vadim Zeitlin
// Created:     2013-05-19
// Copyright:   (c) 2013 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MODALHOOK_H_
#define _WX_MODALHOOK_H_

import <vector>;

class wxDialog;

// ----------------------------------------------------------------------------
// Class allowing to be notified about any modal dialog calls.
// ----------------------------------------------------------------------------

// To be notified about entering and exiting modal dialogs and possibly to
// replace them with something else (e.g. just return a predefined value for
// testing), define an object of this class, override its Enter() and
// possibly Exit() methods and call Register() on it.
class wxModalDialogHook
{
public:
    // Dtor unregisters the hook if it had been registered.
    virtual ~wxModalDialogHook() { DoUnregister(); }

    wxModalDialogHook& operator=(wxModalDialogHook&&) = delete;

    // Register this hook as being active, i.e. its Enter() and Exit() methods
    // will be called.
    //
    // Notice that the order of registration matters: the last hook registered
    // is called first, and if its Enter() returns something != wxID_NONE, the
    // subsequent hooks are skipped.
    void Register();

    // Unregister this hook. Notice that is done automatically from the dtor.
    void Unregister();

    // Called from wxWidgets code before showing any modal dialogs and calls
    // Enter() for every registered hook.
    static int CallEnter(wxDialog* dialog);

    // Called from wxWidgets code after dismissing the dialog and calls Exit()
    // for every registered hook.
    static void CallExit(wxDialog* dialog);

protected:
    // Called by wxWidgets before showing any modal dialogs, override this to
    // be notified about this and return anything but wxID_NONE to skip showing
    // the modal dialog entirely and just return the specified result.
    virtual int Enter(wxDialog* dialog) = 0;

    // Called by wxWidgets after dismissing the modal dialog. Notice that it
    // won't be called if Enter() hadn't been.
    virtual void Exit([[maybe_unused]] wxDialog* dialog) { }

private:
    // Unregister the given hook, return true if it was done or false if the
    // hook wasn't found.
    bool DoUnregister();

    // All the hooks in reverse registration order (i.e. in call order).
    using Hooks = std::vector<wxModalDialogHook *>;
    static Hooks ms_hooks;
};

// Helper object used by WX_MODAL_DIALOG_HOOK below to ensure that CallExit()
// is called on scope exit.
class wxModalDialogHookExitGuard
{
public:
    explicit wxModalDialogHookExitGuard(wxDialog* dialog)
        : m_dialog(dialog)
    {
    }

    ~wxModalDialogHookExitGuard()
    {
        wxModalDialogHook::CallExit(m_dialog);
    }

    wxModalDialogHookExitGuard& operator=(wxModalDialogHookExitGuard&&) = delete;
    
private:
    wxDialog* const m_dialog;
};

// This macro needs to be used at the top of every implementation of
// ShowModal() in order for wxModalDialogHook to work.
#define WX_HOOK_MODAL_DIALOG()                                                \
    const int modalDialogHookRC = wxModalDialogHook::CallEnter(this);         \
    if ( modalDialogHookRC != wxID_NONE )                                     \
        return modalDialogHookRC;                                             \
    wxModalDialogHookExitGuard modalDialogHookExit(this)

#endif // _WX_MODALHOOK_H_
