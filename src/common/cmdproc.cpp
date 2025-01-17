///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/cmdproc.cpp
// Purpose:     wxCommand and wxCommandProcessor classes
// Author:      Julian Smart (extracted from docview.h by VZ)
// Modified by:
// Created:     05.11.00
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/intl.h"
#include "wx/string.h"
#include "wx/accel.h"
#include "wx/cmdproc.h"
#include "wx/menu.h"

// ----------------------------------------------------------------------------
// wxCommand
// ----------------------------------------------------------------------------

wxCommand::wxCommand(bool canUndoIt, const wxString& name)
    : m_commandName(name),
      m_canUndo(canUndoIt)
{
}

// ----------------------------------------------------------------------------
// Command processor
// ----------------------------------------------------------------------------

wxCommandProcessor::wxCommandProcessor(int maxCommands)
    : m_maxNoCommands(maxCommands)
    #if wxUSE_ACCEL
    , m_undoAccelerator('\t' + wxAcceleratorEntry(wxACCEL_CTRL, 'Z', 0, nullptr).ToString())
    , m_redoAccelerator('\t' + wxAcceleratorEntry(wxACCEL_CTRL, 'Y', 0, nullptr).ToString())
#endif // wxUSE_ACCEL
{
}

wxCommandProcessor::~wxCommandProcessor()
{
    ClearCommands();
}

bool wxCommandProcessor::DoCommand(wxCommand& cmd)
{
    return cmd.Do();
}

bool wxCommandProcessor::UndoCommand(wxCommand& cmd)
{
    return cmd.Undo();
}

// Pass a command to the processor. The processor calls Do();
// if successful, is appended to the command history unless
// storeIt is false.
bool wxCommandProcessor::Submit(wxCommand *command, bool storeIt)
{
    wxCHECK_MSG( command, false, "no command in wxCommandProcessor::Submit" );

    if ( !DoCommand(*command) )
    {
        // the user code expects the command to be deleted anyhow
        delete command;

        return false;
    }

    if ( storeIt )
        Store(command);
    else
        delete command;

    return true;
}

void wxCommandProcessor::Store(wxCommand *command)
{
    wxCHECK_RET( command, "no command in wxCommandProcessor::Store" );

    // Correct a bug: we must chop off the current 'branch'
    // so that we're at the end of the command list.
    if (!m_currentCommand)
        ClearCommands();
    else
    {
        wxList::compatibility_iterator node = m_currentCommand->GetNext();
        while (node)
        {
            wxList::compatibility_iterator next = node->GetNext();

            // Make sure m_lastSavedCommand won't point to freed memory
            if ( m_lastSavedCommand && m_lastSavedCommand == node )
                m_lastSavedCommand = wxList::compatibility_iterator();

            delete (wxCommand *)node->GetData();
            m_commands.Erase(node);

            node = next;
        }
    }

    if ( (int)m_commands.GetCount() == m_maxNoCommands )
    {
        wxList::compatibility_iterator firstNode = m_commands.GetFirst();

        // Make sure m_lastSavedCommand won't point to freed memory
        if ( m_lastSavedCommand && m_lastSavedCommand == firstNode )
            m_lastSavedCommand = wxList::compatibility_iterator();

        wxCommand *firstCommand = (wxCommand *)firstNode->GetData();
        delete firstCommand;
        m_commands.Erase(firstNode);
    }

    m_commands.Append(command);
    m_currentCommand = m_commands.GetLast();
    SetMenuStrings();
}

bool wxCommandProcessor::Undo()
{
    wxCommand *command = GetCurrentCommand();
    if ( command && command->CanUndo() )
    {
        if ( UndoCommand(*command) )
        {
            m_currentCommand = m_currentCommand->GetPrevious();
            SetMenuStrings();
            return true;
        }
    }

    return false;
}

bool wxCommandProcessor::Redo()
{
    wxCommand *redoCommand = nullptr;
    wxList::compatibility_iterator redoNode;

    if ( m_currentCommand )
    {
        // is there anything to redo?
        if ( m_currentCommand->GetNext() )
        {
            redoCommand = (wxCommand *)m_currentCommand->GetNext()->GetData();
            redoNode = m_currentCommand->GetNext();
        }
    }
    else // no current command, redo the first one
    {
        if (m_commands.GetCount() > 0)
        {
            redoCommand = (wxCommand *)m_commands.GetFirst()->GetData();
            redoNode = m_commands.GetFirst();
        }
    }

    if (redoCommand)
    {
        if (DoCommand(*redoCommand))
        {
            m_currentCommand = redoNode;
            SetMenuStrings();
            return true;
        }
    }
    return false;
}

bool wxCommandProcessor::CanUndo() const
{
    wxCommand *command = GetCurrentCommand();

    return command && command->CanUndo();
}

bool wxCommandProcessor::CanRedo() const
{
    if (m_currentCommand && !m_currentCommand->GetNext())
        return false;

    if (m_currentCommand && m_currentCommand->GetNext())
        return true;

    if (!m_currentCommand && (m_commands.GetCount() > 0))
        return true;

    return false;
}

void wxCommandProcessor::Initialize()
{
    m_currentCommand = m_commands.GetLast();
    SetMenuStrings();
}

void wxCommandProcessor::SetMenuStrings()
{
#if wxUSE_MENUS
    if (m_commandEditMenu)
    {
        wxString undoLabel = GetUndoMenuLabel();
        wxString redoLabel = GetRedoMenuLabel();

        m_commandEditMenu->SetLabel(wxID_UNDO, undoLabel);
        m_commandEditMenu->Enable(wxID_UNDO, CanUndo());

        m_commandEditMenu->SetLabel(wxID_REDO, redoLabel);
        m_commandEditMenu->Enable(wxID_REDO, CanRedo());
    }
#endif // wxUSE_MENUS
}

// Gets the current Undo menu label.
wxString wxCommandProcessor::GetUndoMenuLabel() const
{
    wxString buf;
    if (m_currentCommand)
    {
        const wxCommand *command = (wxCommand *)m_currentCommand->GetData();
        wxString commandName(command->GetName());
        if (commandName.empty()) commandName = _("Unnamed command");
        const bool canUndo = command->CanUndo();
        if (canUndo)
            buf = wxString(_("&Undo ")) + commandName + m_undoAccelerator;
        else
            buf = wxString(_("Can't &Undo ")) + commandName + m_undoAccelerator;
    }
    else
    {
        buf = _("&Undo") + m_undoAccelerator;
    }

    return buf;
}

// Gets the current Undo menu label.
wxString wxCommandProcessor::GetRedoMenuLabel() const
{
    wxString buf;
    if (m_currentCommand)
    {
        // We can redo, if we're not at the end of the history.
        if (m_currentCommand->GetNext())
        {
            wxCommand *redoCommand = (wxCommand *)m_currentCommand->GetNext()->GetData();
            wxString redoCommandName(redoCommand->GetName());
            if (redoCommandName.empty()) redoCommandName = _("Unnamed command");
            buf = wxString(_("&Redo ")) + redoCommandName + m_redoAccelerator;
        }
        else
        {
            buf = _("&Redo") + m_redoAccelerator;
        }
    }
    else
    {
        if (m_commands.GetCount() == 0)
        {
            buf = _("&Redo") + m_redoAccelerator;
        }
        else
        {
            // currentCommand is NULL but there are commands: this means that
            // we've undone to the start of the list, but can redo the first.
            wxCommand *redoCommand = (wxCommand *)m_commands.GetFirst()->GetData();
            wxString redoCommandName(redoCommand->GetName());
            if (redoCommandName.empty()) redoCommandName = _("Unnamed command");
            buf = wxString(_("&Redo ")) + redoCommandName + m_redoAccelerator;
        }
    }
    return buf;
}

void wxCommandProcessor::ClearCommands()
{
    wxList::compatibility_iterator node = m_commands.GetFirst();
    while (node)
    {
        wxCommand *command = (wxCommand *)node->GetData();
        delete command;
        m_commands.Erase(node);
        node = m_commands.GetFirst();
    }

    m_currentCommand = wxList::compatibility_iterator();
    m_lastSavedCommand = wxList::compatibility_iterator();
}

bool wxCommandProcessor::IsDirty() const
{
    if ( !m_lastSavedCommand )
    {
        // We have never been saved, so we are dirty if and only if we have any
        // commands at all.
        //
        // NB: The ugly "!!" test is needed to avoid warnings both from MSVC in
        //     non-STL build and g++ in STL build.
        return !!m_currentCommand;
    }

    if ( !m_currentCommand )
    {
        // This only happens if all commands were undone after saving the
        // document: we're dirty then.
        return true;
    }

    // Finally if both iterators are valid, we may just compare them.
    return m_currentCommand != m_lastSavedCommand;
}

