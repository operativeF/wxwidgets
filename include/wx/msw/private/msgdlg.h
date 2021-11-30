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

#include "wx/string.h"

#include <memory>

import WX.WinDef;

import <string>;

// Provides methods for creating a task dialog.
namespace wxMSWMessageDialog
{

    class wxMSWTaskDialogConfig
    {
    public:
        static constexpr auto MAX_BUTTONS = 4;

        wxMSWTaskDialogConfig()
            : buttons(new TASKDIALOG_BUTTON[MAX_BUTTONS])
              
        {}

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
        unsigned int style{0};

        bool useCustomLabels{false};

        // Will create a task dialog with it's parameters for it's creation
        // stored in the provided WXTASKDIALOGCONFIG parameter.
        // NOTE: The wxMSWTaskDialogConfig object needs to remain accessible
        // during the subsequent call to TaskDialogIndirect().
        void MSWCommonTaskDialogInit(WXTASKDIALOGCONFIG &tdc);

        // Used by MSWCommonTaskDialogInit() to add a regular button or a
        // button with a custom label if used.
        void AddTaskDialogButton(WXTASKDIALOGCONFIG &tdc,
                                 int btnCustomId,
                                 int btnCommonId,
                                 const std::string& customLabel);
    }; // class wxMSWTaskDialogConfig

    using TaskDialogIndirect_t = WXHRESULT (__stdcall*)(const WXTASKDIALOGCONFIG *,
                                                        int *, int *, WXBOOL *);

    // Return the pointer to TaskDialogIndirect(). This should only be called
    // if HasNativeTaskDialog() returned true and is normally guaranteed to
    // succeed in this case.
    TaskDialogIndirect_t GetTaskDialogIndirectFunc();

    // Check if the task dialog is available: this simply checks the OS version
    // as we know that it's only present in Vista and later.
    bool HasNativeTaskDialog();

    // Translates standard MSW button IDs like IDCANCEL into an equivalent
    // wx constant such as wxCANCEL.
    int MSWTranslateReturnCode(int msAns);
}; // namespace wxMSWMessageDialog

#endif // _WX_MSW_PRIVATE_MSGDLG_H_
