///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/msgdlg.h
// Purpose:     helper functions used with native message dialog
// Author:      Rickard Westerlund
// Created:     2010-07-12
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PRIVATE_MSGDLG_H_
#define _WX_MSW_PRIVATE_MSGDLG_H_

#include "wx/msw/wrapcctl.h"

#include <memory>

// Macro to help identify if task dialogs are available: we rely on
// TD_WARNING_ICON being defined in the headers for this as this symbol is used
// by the task dialogs only. Also notice that task dialogs are available for
// Unicode applications only.
#if defined(TD_WARNING_ICON)
    #define wxHAS_MSW_TASKDIALOG
#endif

// Provides methods for creating a task dialog.
namespace wxMSWMessageDialog
{

#ifdef wxHAS_MSW_TASKDIALOG
    class wxMSWTaskDialogConfig
    {
    public:
        enum { MAX_BUTTONS = 4  };

        wxMSWTaskDialogConfig()
            : buttons(new TASKDIALOG_BUTTON[MAX_BUTTONS])
              
            { }

        // initializes the object from a message dialog.
        wxMSWTaskDialogConfig(const wxMessageDialogBase& dlg);

        wxString btnYesLabel;
        wxString btnNoLabel;
        wxString btnOKLabel;
        wxString btnCancelLabel;
        wxString btnHelpLabel;

        std::string caption;
        std::string message;
        std::string extendedMessage;

        std::unique_ptr<TASKDIALOG_BUTTON[]> buttons;
        wxWindow *parent{nullptr};

        long iconId{0};
        long style{0};

        bool useCustomLabels{false};

        // Will create a task dialog with it's parameters for it's creation
        // stored in the provided TASKDIALOGCONFIG parameter.
        // NOTE: The wxMSWTaskDialogConfig object needs to remain accessible
        // during the subsequent call to TaskDialogIndirect().
        void MSWCommonTaskDialogInit(TASKDIALOGCONFIG &tdc);

        // Used by MSWCommonTaskDialogInit() to add a regular button or a
        // button with a custom label if used.
        void AddTaskDialogButton(TASKDIALOGCONFIG &tdc,
                                 int btnCustomId,
                                 int btnCommonId,
                                 const std::string& customLabel);
    }; // class wxMSWTaskDialogConfig


    typedef HRESULT (WINAPI *TaskDialogIndirect_t)(const TASKDIALOGCONFIG *,
                                                   int *, int *, BOOL *);

    // Return the pointer to TaskDialogIndirect(). This should only be called
    // if HasNativeTaskDialog() returned true and is normally guaranteed to
    // succeed in this case.
    TaskDialogIndirect_t GetTaskDialogIndirectFunc();
#endif // wxHAS_MSW_TASKDIALOG


    // Check if the task dialog is available: this simply checks the OS version
    // as we know that it's only present in Vista and later.
    bool HasNativeTaskDialog();

    // Translates standard MSW button IDs like IDCANCEL into an equivalent
    // wx constant such as wxCANCEL.
    int MSWTranslateReturnCode(int msAns);
}; // namespace wxMSWMessageDialog

#endif // _WX_MSW_PRIVATE_MSGDLG_H_
