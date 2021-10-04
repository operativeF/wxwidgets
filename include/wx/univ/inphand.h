///////////////////////////////////////////////////////////////////////////////
// Name:        wx/univ/inphand.h
// Purpose:     wxInputHandler class maps the keyboard and mouse events to the
//              actions which then are performed by the control
// Author:      Vadim Zeitlin
// Modified by:
// Created:     18.08.00
// Copyright:   (c) 2000 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIV_INPHAND_H_
#define _WX_UNIV_INPHAND_H_

#include "wx/univ/inpcons.h"         // for wxControlAction(s)

// ----------------------------------------------------------------------------
// types of the standard input handlers which can be passed to
// wxTheme::GetInputHandler()
// ----------------------------------------------------------------------------

constexpr wxChar wxINP_HANDLER_DEFAULT[]           = wxT("");
constexpr wxChar wxINP_HANDLER_BUTTON[]            = wxT("button");
constexpr wxChar wxINP_HANDLER_CHECKBOX[]          = wxT("checkbox");
constexpr wxChar wxINP_HANDLER_CHECKLISTBOX[]      = wxT("checklistbox");
constexpr wxChar wxINP_HANDLER_COMBOBOX[]          = wxT("combobox");
constexpr wxChar wxINP_HANDLER_LISTBOX[]           = wxT("listbox");
constexpr wxChar wxINP_HANDLER_NOTEBOOK[]          = wxT("notebook");
constexpr wxChar wxINP_HANDLER_RADIOBTN[]          = wxT("radiobtn");
constexpr wxChar wxINP_HANDLER_SCROLLBAR[]         = wxT("scrollbar");
constexpr wxChar wxINP_HANDLER_SLIDER[]            = wxT("slider");
constexpr wxChar wxINP_HANDLER_SPINBTN[]           = wxT("spinbtn");
constexpr wxChar wxINP_HANDLER_STATUSBAR[]         = wxT("statusbar");
constexpr wxChar wxINP_HANDLER_TEXTCTRL[]          = wxT("textctrl");
constexpr wxChar wxINP_HANDLER_TOOLBAR[]           = wxT("toolbar");
constexpr wxChar wxINP_HANDLER_TOPLEVEL[]          = wxT("toplevel");

// ----------------------------------------------------------------------------
// wxInputHandler: maps the events to the actions
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxInputHandler : public wxObject
{
public:
    // map a keyboard event to one or more actions (pressed == true if the key
    // was pressed, false if released), returns true if something was done
    virtual bool HandleKey(wxInputConsumer *consumer,
                           const wxKeyEvent& event,
                           bool pressed) = 0;

    // map a mouse (click) event to one or more actions
    virtual bool HandleMouse(wxInputConsumer *consumer,
                             const wxMouseEvent& event) = 0;

    // handle mouse movement (or enter/leave) event: it is separated from
    // HandleMouse() for convenience as many controls don't care about mouse
    // movements at all
    virtual bool HandleMouseMove(wxInputConsumer *consumer,
                                 const wxMouseEvent& event);

    // do something with focus set/kill event: this is different from
    // HandleMouseMove() as the mouse maybe over the control without it having
    // focus
    //
    // return true to refresh the control, false otherwise
    virtual bool HandleFocus(wxInputConsumer *consumer, const wxFocusEvent& event);

    // react to the app getting/losing activation
    //
    // return true to refresh the control, false otherwise
    virtual bool HandleActivation(wxInputConsumer *consumer, bool activated);

    ~wxInputHandler();
};

// ----------------------------------------------------------------------------
// wxStdInputHandler is just a base class for all other "standard" handlers
// and also provides the way to chain input handlers together
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxStdInputHandler : public wxInputHandler
{
public:
    wxStdInputHandler(wxInputHandler *handler) : m_handler(handler) { }

    virtual bool HandleKey(wxInputConsumer *consumer,
                           const wxKeyEvent& event,
                           bool pressed) override
    {
        return m_handler ? m_handler->HandleKey(consumer, event, pressed)
                         : false;
    }

    virtual bool HandleMouse(wxInputConsumer *consumer,
                             const wxMouseEvent& event) override
    {
        return m_handler ? m_handler->HandleMouse(consumer, event) : false;
    }

    bool HandleMouseMove(wxInputConsumer *consumer, const wxMouseEvent& event) override
    {
        return m_handler ? m_handler->HandleMouseMove(consumer, event) : false;
    }

    bool HandleFocus(wxInputConsumer *consumer, const wxFocusEvent& event) override
    {
        return m_handler ? m_handler->HandleFocus(consumer, event) : false;
    }

private:
    wxInputHandler *m_handler;
};

#endif // _WX_UNIV_INPHAND_H_
