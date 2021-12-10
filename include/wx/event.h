/////////////////////////////////////////////////////////////////////////////
// Name:        wx/event.h
// Purpose:     Event classes
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_EVENT_H_
#define _WX_EVENT_H_

#include "wx/cpp.h"
#include "wx/object.h"
#include "wx/clntdata.h"

#if wxUSE_GUI
    #include "wx/cursor.h"
    #include "wx/mousestate.h"
#endif

#include "wx/thread.h"
#include "wx/tracker.h"
#include "wx/any.h"
#include "wx/typeinfo.h"

import Utils.Geometry;

import <bit>;
import <cstdint>;
import <string>;
import <type_traits>;
import <vector>;


// This is now always defined, but keep it for backwards compatibility.
#define wxHAS_CALL_AFTER

/*  this window should always process UI update events */
inline constexpr unsigned int wxWS_EX_PROCESS_UI_UPDATES      = 0x00000020;

// ----------------------------------------------------------------------------
// forward declarations
// ----------------------------------------------------------------------------

class wxList;
class wxEvent;
class wxEventFilter;

#if wxUSE_GUI
    class wxDC;
    class wxMenu;
    class wxWindow;
    class wxWindowBase;
#endif // wxUSE_GUI

// We operate with pointer to members of wxEvtHandler (such functions are used
// as event handlers in the event tables or as arguments to Connect()) but by
// default MSVC uses a restricted (but more efficient) representation of
// pointers to members which can't deal with multiple base classes. To avoid
// mysterious (as the compiler is not good enough to detect this and give a
// sensible error message) errors in the user code as soon as it defines
// classes inheriting from both wxEvtHandler (possibly indirectly, e.g. via
// wxWindow) and something else (including our own wxTrackable but not limited
// to it), we use the special MSVC keyword telling the compiler to use a more
// general pointer to member representation for the classes inheriting from
// wxEvtHandler.
#ifdef __VISUALC__
    #define wxMSVC_FWD_MULTIPLE_BASES __multiple_inheritance
#else
    #define wxMSVC_FWD_MULTIPLE_BASES
#endif

class wxMSVC_FWD_MULTIPLE_BASES wxEvtHandler;
class wxEventConnectionRef;

// ----------------------------------------------------------------------------
// Event types
// ----------------------------------------------------------------------------

using wxEventType = int;

inline constexpr wxEventType wxEVT_ANY = -1;

// This macro exists for compatibility only (even though it was never public,
// it still appears in some code using wxWidgets), see public
// wxEVENT_HANDLER_CAST instead.
#define wxStaticCastEvent(type, val) static_cast<type>(val)

#define wxDECLARE_EVENT_TABLE_ENTRY(type, winid, idLast, fn, obj) \
    wxEventTableEntry(type, winid, idLast, wxNewEventTableFunctor(type, fn), obj)

#define wxDECLARE_EVENT_TABLE_TERMINATOR() \
    wxEventTableEntry(wxEVT_NULL, 0, 0, NULL, NULL)

// generate a new unique event type
extern wxEventType wxNewEventType();

// events are represented by an instance of wxEventTypeTag and the
// corresponding type must be specified for type-safety checks

// define a new custom event type, can be used alone or after event
// declaration in the header using one of the macros below
#define wxDEFINE_EVENT( name, type ) \
    const wxEventTypeTag< type > name( wxNewEventType() )

// the general version allowing exporting the event type from DLL, used by
// wxWidgets itself
#define wxDECLARE_EXPORTED_EVENT( expdecl, name, type ) \
    extern const expdecl wxEventTypeTag< type > name

// this is the version which will normally be used in the user code
#define wxDECLARE_EVENT( name, type ) \
    wxDECLARE_EXPORTED_EVENT( wxEMPTY_PARAMETER_VALUE, name, type )


// these macros are only used internally for backwards compatibility and
// allow to define an alias for an existing event type (this is used by
// wxEVT_SPIN_XXX)
#define wxDEFINE_EVENT_ALIAS( name, type, value ) \
    const wxEventTypeTag< type > name( value )

#define wxDECLARE_EXPORTED_EVENT_ALIAS( expdecl, name, type ) \
    extern const expdecl wxEventTypeTag< type > name

// The type-erased method signature used for event handling.
typedef void (wxEvtHandler::*wxEventFunction)(wxEvent&);

template <typename T>
inline wxEventFunction wxEventFunctionCast(void (wxEvtHandler::*func)(T&))
{
    // There is no going around the cast here: we do rely calling the event
    // handler method, which takes a reference to an object of a class derived
    // from wxEvent, as if it took wxEvent itself. On all platforms supported
    // by wxWidgets, this cast is harmless, but it's not a valid cast in C++
    // and gcc 8 started giving warnings about this (with -Wextra), so suppress
    // them locally to avoid generating hundreds of them when compiling any
    // code using event table macros.

    // FIXME: UB
    return reinterpret_cast<wxEventFunction>(func);
}

// In good old pre-C++17 times we could just static_cast the event handler,
// defined in some class deriving from wxEvtHandler, to the "functype" which is
// a type of wxEvtHandler method. But with C++17 this doesn't work when the
// handler is a noexcept function, so we need to cast it to a noexcept function
// pointer first.
namespace wxPrivate
{

// Cast to noexcept function type first if necessary.
template <typename E, typename C>
constexpr auto DoCast(void (C::*pmf)(E&))
{
    return static_cast<void (wxEvtHandler::*)(E&)>(pmf);
}

template <typename E, typename C>
constexpr auto DoCast(void (C::*pmf)(E&) noexcept)
{
    return static_cast<void (wxEvtHandler::*)(E&)>(
            static_cast<void (wxEvtHandler::*)(E&) noexcept>(pmf)
        );
}

// Helper used to recover the type of the handler argument from the function
// type. This is required in order to explicitly pass this type to DoCast<> as
// the compiler would be unable to deduce it for overloaded functions.

// Generic template version, doing nothing.
template <typename F>
struct EventArgOf;

// Specialization sufficient to cover all event handler function types.
template <typename C, typename E>
struct EventArgOf<void (C::*)(E&)>
{
    using type = E;
};


} // namespace wxPrivate

#define wxEventHandlerNoexceptCast(functype, pmf) \
    wxPrivate::DoCast<wxPrivate::EventArgOf<functype>::type>(pmf)


// Try to cast the given event handler to the correct handler type:
#define wxEVENT_HANDLER_CAST( functype, func ) \
    wxEventFunctionCast(wxEventHandlerNoexceptCast(functype, &func))


// The tag is a type associated to the event type (which is an integer itself,
// in spite of its name) value. It exists in order to be used as a template
// parameter and provide a mapping between the event type values and their
// corresponding wxEvent-derived classes.
template <typename T>
class wxEventTypeTag
{
public:
    // The class of wxEvent-derived class carried by the events of this type.
    using EventClass = T;

    wxEventTypeTag(wxEventType type) : m_type(type) { }

    // Return a wxEventType reference for the initialization of the static
    // event tables. See wxEventTableEntry::m_eventType for a more thorough
    // explanation.
    operator const wxEventType&() const { return m_type; }

private:
    wxEventType m_type;
};

// We had some trouble with using wxEventFunction
// in the past so we had introduced wxObjectEventFunction which
// used to be a typedef for a member of wxObject and not wxEvtHandler to work
// around this but as eVC is not really supported any longer we now only keep
// this for backwards compatibility and, despite its name, this is a typedef
// for wxEvtHandler member now -- but if we have the same problem with another
// compiler we can restore its old definition for it.
using wxObjectEventFunction = wxEventFunction;

// The event functor which is stored in the static and dynamic event tables:
class wxEventFunctor
{
public:
    virtual ~wxEventFunctor() = default;

    // Invoke the actual event handler:
    virtual void operator()(wxEvtHandler *, wxEvent&) = 0;

    // this function tests whether this functor is matched, for the purpose of
    // finding it in an event table in Unbind(), by the given functor:
    virtual bool IsMatching(const wxEventFunctor& functor) const = 0;

    // If the functor holds an wxEvtHandler, then get access to it and track
    // its lifetime with wxEventConnectionRef:
    virtual wxEvtHandler *GetEvtHandler() const
        { return nullptr; }

    // This is only used to maintain backward compatibility in
    // wxAppConsoleBase::CallEventHandler and ensures that an overwritten
    // wxAppConsoleBase::HandleEvent is still called for functors which hold an
    // wxEventFunction:
    virtual wxEventFunction GetEvtMethod() const
        { return nullptr; }

private:
    WX_DECLARE_ABSTRACT_TYPEINFO(wxEventFunctor)
};

// A plain method functor for the untyped legacy event types:
class wxObjectEventFunctor : public wxEventFunctor
{
public:
    wxObjectEventFunctor(wxObjectEventFunction method, wxEvtHandler *handler)
        : m_handler( handler ), m_method( method )
    {}

    void operator()(wxEvtHandler *handler, wxEvent& event) override;

    bool IsMatching(const wxEventFunctor& functor) const override
    {
        if ( typeid(functor) == typeid(*this) )
        {
            const wxObjectEventFunctor &other =
                static_cast< const wxObjectEventFunctor & >( functor );

            return ( m_method == other.m_method || !other.m_method ) &&
                   ( m_handler == other.m_handler || !other.m_handler );
        }
        else
            return false;
    }

    wxEvtHandler *GetEvtHandler() const override
        { return m_handler; }

    wxEventFunction GetEvtMethod() const override
        { return m_method; }

private:
    wxEvtHandler *m_handler{nullptr};
    wxEventFunction m_method{nullptr};

    // Provide a dummy default ctor for type info purposes
    wxObjectEventFunctor()  = default;

    WX_DECLARE_TYPEINFO_INLINE(wxObjectEventFunctor)
};

// Create a functor for the legacy events: used by Connect()
inline wxObjectEventFunctor *
wxNewEventFunctor([[maybe_unused]] const wxEventType& evtType,
                  wxObjectEventFunction method,
                  wxEvtHandler *handler)
{
    return new wxObjectEventFunctor(method, handler);
}

// This version is used by wxDECLARE_EVENT_TABLE_ENTRY()
inline wxObjectEventFunctor *
wxNewEventTableFunctor([[maybe_unused]] const wxEventType& evtType,
                       wxObjectEventFunction method)
{
    return new wxObjectEventFunctor(method, nullptr);
}

inline wxObjectEventFunctor
wxMakeEventFunctor([[maybe_unused]] const wxEventType& evtType,
                        wxObjectEventFunction method,
                        wxEvtHandler *handler)
{
    return wxObjectEventFunctor(method, handler);
}

namespace wxPrivate
{

// helper template defining nested "type" typedef as the event class
// corresponding to the given event type
template <typename T> struct EventClassOf;

// the typed events provide the information about the class of the events they
// carry themselves:
template <typename T>
struct EventClassOf< wxEventTypeTag<T> >
{
    using type = typename wxEventTypeTag<T>::EventClass;
};

// for the old untyped events we don't have information about the exact event
// class carried by them
template <>
struct EventClassOf<wxEventType>
{
    using type = wxEvent;
};


// helper class defining operations different for method functors using an
// object of wxEvtHandler-derived class as handler and the others
template <typename T, typename A, bool> struct HandlerImpl;

// specialization for handlers deriving from wxEvtHandler
template <typename T, typename A>
struct HandlerImpl<T, A, true>
{
    static bool IsEvtHandler()
        { return true; }
    static T *ConvertFromEvtHandler(wxEvtHandler *p)
        { return dynamic_cast<T *>(p); }
    static wxEvtHandler *ConvertToEvtHandler(T *p)
        { return p; }
    static wxEventFunction ConvertToEvtMethod(void (T::*f)(A&))
        { return wxEventFunctionCast(
                    static_cast<void (wxEvtHandler::*)(A&)>(f)); }
};

// specialization for handlers not deriving from wxEvtHandler
template <typename T, typename A>
struct HandlerImpl<T, A, false>
{
    static bool IsEvtHandler()
        { return false; }
    static T *ConvertFromEvtHandler(wxEvtHandler *)
        { return nullptr; }
    static wxEvtHandler *ConvertToEvtHandler(T *)
        { return nullptr; }
    static wxEventFunction ConvertToEvtMethod(void (T::*)(A&))
        { return nullptr; }
};

} // namespace wxPrivate

// functor forwarding the event to a method of the given object
//
// notice that the object class may be different from the class in which the
// method is defined but it must be convertible to this class
//
// also, the type of the handler parameter doesn't need to be exactly the same
// as EventTag::EventClass but it must be its base class -- this is explicitly
// allowed to handle different events in the same handler taking wxEvent&, for
// example
template
  <typename EventTag, typename Class, typename EventArg, typename EventHandler>
class wxEventFunctorMethod
    : public wxEventFunctor,
      private wxPrivate::HandlerImpl
              <
                Class,
                EventArg,
                std::is_base_of_v<Class, wxEvtHandler> != false
              >
{
private:
    static void CheckHandlerArgument(EventArg *) { }

public:
    // the event class associated with the given event tag
    using EventClass = typename wxPrivate::EventClassOf<EventTag>::type;


    wxEventFunctorMethod(void (Class::*method)(EventArg&), EventHandler *handler)
        : m_handler( handler ), m_method( method )
    {
        wxASSERT_MSG( handler || this->IsEvtHandler(),
                      "handlers defined in non-wxEvtHandler-derived classes "
                      "must be connected with a valid sink object" );

        // if you get an error here it means that the signature of the handler
        // you're trying to use is not compatible with (i.e. is not the same as
        // or a base class of) the real event class used for this event type
        CheckHandlerArgument(static_cast<EventClass *>(nullptr));
    }

    void operator()(wxEvtHandler *handler, wxEvent& event) override
    {
        Class * realHandler = m_handler;
        if ( !realHandler )
        {
            realHandler = this->ConvertFromEvtHandler(handler);

            // this is not supposed to happen but check for it nevertheless
            wxCHECK_RET( realHandler, "invalid event handler" );
        }

        // the real (run-time) type of event is EventClass and we checked in
        // the ctor that EventClass can be converted to EventArg, so this cast
        // is always valid
        (realHandler->*m_method)(static_cast<EventArg&>(event));
    }

    bool IsMatching(const wxEventFunctor& functor) const override
    {
        if ( typeid(functor) != typeid(*this) )
            return false;

        using ThisFunctor = wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler>;

        // the cast is valid because wxTypeId()s matched above
        const ThisFunctor& other = static_cast<const ThisFunctor &>(functor);

        return (m_method == other.m_method || other.m_method == nullptr) &&
               (m_handler == other.m_handler || other.m_handler == nullptr);
    }

    wxEvtHandler *GetEvtHandler() const override
        { return this->ConvertToEvtHandler(m_handler); }

    wxEventFunction GetEvtMethod() const override
        { return this->ConvertToEvtMethod(m_method); }

private:
    EventHandler *m_handler;
    void (Class::*m_method)(EventArg&);

    // Provide a dummy default ctor for type info purposes
    wxEventFunctorMethod() { }

    using thisClass = wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler>;
    WX_DECLARE_TYPEINFO_INLINE(thisClass)
};


// functor forwarding the event to function (function, static method)
template <typename EventTag, typename EventArg>
class wxEventFunctorFunction : public wxEventFunctor
{
private:
    static void CheckHandlerArgument(EventArg *) { }

public:
    // the event class associated with the given event tag
    using EventClass = typename wxPrivate::EventClassOf<EventTag>::type;

    wxEventFunctorFunction( void ( *handler )( EventArg & ))
        : m_handler( handler )
    {
        // if you get an error here it means that the signature of the handler
        // you're trying to use is not compatible with (i.e. is not the same as
        // or a base class of) the real event class used for this event type
        CheckHandlerArgument(static_cast<EventClass *>(NULL));
    }

    void operator()([[maybe_unused]] wxEvtHandler *handler, wxEvent& event) override
    {
        // If you get an error here like "must use .* or ->* to call
        // pointer-to-member function" then you probably tried to call
        // Bind/Unbind with a method pointer but without a handler pointer or
        // NULL as a handler e.g.:
        // Unbind( wxEVT_XXX, &EventHandler::method );
        // or
        // Unbind( wxEVT_XXX, &EventHandler::method, NULL )
        m_handler(static_cast<EventArg&>(event));
    }

    bool IsMatching(const wxEventFunctor &functor) const override
    {
        if ( typeid(functor) != typeid(*this) )
            return false;

        typedef wxEventFunctorFunction<EventTag, EventArg> ThisFunctor;

        const ThisFunctor& other = static_cast<const ThisFunctor&>( functor );

        return m_handler == other.m_handler;
    }

private:
    void (*m_handler)(EventArg&);

    // Provide a dummy default ctor for type info purposes
    wxEventFunctorFunction() { }

    using thisClass = wxEventFunctorFunction<EventTag, EventArg>;
    WX_DECLARE_TYPEINFO_INLINE(thisClass)
};


template <typename EventTag, typename Functor>
class wxEventFunctorFunctor : public wxEventFunctor
{
public:
    using EventArg = typename EventTag::EventClass;

    wxEventFunctorFunctor(const Functor& handler)
        : m_handler(handler), m_handlerAddr(&handler)
        { }

    void operator()([[maybe_unused]] wxEvtHandler *handler, wxEvent& event) override
    {
        // If you get an error here like "must use '.*' or '->*' to call
        // pointer-to-member function" then you probably tried to call
        // Bind/Unbind with a method pointer but without a handler pointer or
        // NULL as a handler e.g.:
        // Unbind( wxEVT_XXX, &EventHandler::method );
        // or
        // Unbind( wxEVT_XXX, &EventHandler::method, NULL )
        m_handler(static_cast<EventArg&>(event));
    }

    bool IsMatching(const wxEventFunctor &functor) const override
    {
        if ( typeid(functor) != typeid(*this) )
            return false;

        typedef wxEventFunctorFunctor<EventTag, Functor> FunctorThis;

        const FunctorThis& other = static_cast<const FunctorThis&>(functor);

        // The only reliable/portable way to compare two functors is by
        // identity:
        return m_handlerAddr == other.m_handlerAddr;
    }

private:
    // Store a copy of the functor to prevent using/calling an already
    // destroyed instance:
    Functor m_handler;

    // Use the address of the original functor for comparison in IsMatching:
    const void *m_handlerAddr;

    // Provide a dummy default ctor for type info purposes
    wxEventFunctorFunctor() { }

    using thisClass = wxEventFunctorFunctor<EventTag, Functor>;
    WX_DECLARE_TYPEINFO_INLINE(thisClass)
};

// Create functors for the templatized events, either allocated on the heap for
// wxNewXXX() variants (this is needed in wxEvtHandler::Bind<>() to store them
// in dynamic event table) or just by returning them as temporary objects (this
// is enough for Unbind<>() and we avoid unnecessary heap allocation like this).


// Create functors wrapping functions:
template <typename EventTag, typename EventArg>
inline wxEventFunctorFunction<EventTag, EventArg> *
wxNewEventFunctor(const EventTag&, void (*func)(EventArg &))
{
    return new wxEventFunctorFunction<EventTag, EventArg>(func);
}

template <typename EventTag, typename EventArg>
inline wxEventFunctorFunction<EventTag, EventArg>
wxMakeEventFunctor(const EventTag&, void (*func)(EventArg &))
{
    return wxEventFunctorFunction<EventTag, EventArg>(func);
}

// Create functors wrapping other functors:
template <typename EventTag, typename Functor>
inline wxEventFunctorFunctor<EventTag, Functor> *
wxNewEventFunctor(const EventTag&, const Functor &func)
{
    return new wxEventFunctorFunctor<EventTag, Functor>(func);
}

template <typename EventTag, typename Functor>
inline wxEventFunctorFunctor<EventTag, Functor>
wxMakeEventFunctor(const EventTag&, const Functor &func)
{
    return wxEventFunctorFunctor<EventTag, Functor>(func);
}

// Create functors wrapping methods:
template
  <typename EventTag, typename Class, typename EventArg, typename EventHandler>
inline wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler> *
wxNewEventFunctor(const EventTag&,
                  void (Class::*method)(EventArg&),
                  EventHandler *handler)
{
    return new wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler>(
                method, handler);
}

template
    <typename EventTag, typename Class, typename EventArg, typename EventHandler>
inline wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler>
wxMakeEventFunctor(const EventTag&,
                   void (Class::*method)(EventArg&),
                   EventHandler *handler)
{
    return wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler>(
                method, handler);
}

// Create an event functor for the event table via wxDECLARE_EVENT_TABLE_ENTRY:
// in this case we don't have the handler (as it's always the same as the
// object which generated the event) so we must use Class as its type
template <typename EventTag, typename Class, typename EventArg>
inline wxEventFunctorMethod<EventTag, Class, EventArg, Class> *
wxNewEventTableFunctor(const EventTag&, void (Class::*method)(EventArg&))
{
    return new wxEventFunctorMethod<EventTag, Class, EventArg, Class>(
                    method, nullptr);
}


// many, but not all, standard event types

    // some generic events
inline constexpr wxEventType wxEVT_FIRST = 10000;
inline constexpr wxEventType wxEVT_USER_FIRST = wxEVT_FIRST + 2000;

extern const wxEventType wxEVT_NULL;

    // Need events declared to do this
class wxIdleEvent;
class wxThreadEvent;
class wxAsyncMethodCallEvent;
class wxCommandEvent;
class wxMouseEvent;
class wxFocusEvent;
class wxChildFocusEvent;
class wxKeyEvent;
class wxNavigationKeyEvent;
class wxSetCursorEvent;
class wxScrollEvent;
class wxSpinEvent;
class wxScrollWinEvent;
class wxSizeEvent;
class wxMoveEvent;
class wxCloseEvent;
class wxActivateEvent;
class wxWindowCreateEvent;
class wxWindowDestroyEvent;
class wxShowEvent;
class wxIconizeEvent;
class wxMaximizeEvent;
class wxFullScreenEvent;
class wxMouseCaptureChangedEvent;
class wxMouseCaptureLostEvent;
class wxPaintEvent;
class wxEraseEvent;
class wxNcPaintEvent;
class wxMenuEvent;
class wxContextMenuEvent;
class wxSysColourChangedEvent;
class wxDisplayChangedEvent;
class wxDPIChangedEvent;
class wxQueryNewPaletteEvent;
class wxPaletteChangedEvent;
class wxJoystickEvent;
class wxDropFilesEvent;
class wxInitDialogEvent;
class wxUpdateUIEvent;
class wxClipboardTextEvent;
class wxHelpEvent;
class wxGestureEvent;
class wxPanGestureEvent;
class wxZoomGestureEvent;
class wxRotateGestureEvent;
class wxTwoFingerTapEvent;
class wxLongPressEvent;
class wxPressAndTapEvent;


    // Command events
wxDECLARE_EVENT(wxEVT_BUTTON, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_CHECKBOX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_CHOICE, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_LISTBOX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_LISTBOX_DCLICK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_CHECKLISTBOX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MENU, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SLIDER, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_RADIOBOX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_RADIOBUTTON, wxCommandEvent);

// wxEVT_SCROLLBAR is deprecated, use wxEVT_SCROLL... events
wxDECLARE_EVENT(wxEVT_VLBOX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COMBOBOX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_TOOL_RCLICKED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_TOOL_DROPDOWN, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_TOOL_ENTER, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COMBOBOX_DROPDOWN, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COMBOBOX_CLOSEUP, wxCommandEvent);

    // Thread and asynchronous method call events
wxDECLARE_EVENT(wxEVT_THREAD, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_ASYNC_METHOD_CALL, wxAsyncMethodCallEvent);

    // Mouse event types
wxDECLARE_EVENT(wxEVT_LEFT_DOWN, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_LEFT_UP, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_MIDDLE_DOWN, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_MIDDLE_UP, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_RIGHT_DOWN, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_RIGHT_UP, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_MOTION, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_ENTER_WINDOW, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_LEAVE_WINDOW, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_LEFT_DCLICK, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_MIDDLE_DCLICK, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_RIGHT_DCLICK, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_SET_FOCUS, wxFocusEvent);
wxDECLARE_EVENT(wxEVT_KILL_FOCUS, wxFocusEvent);
wxDECLARE_EVENT(wxEVT_CHILD_FOCUS, wxChildFocusEvent);
wxDECLARE_EVENT(wxEVT_MOUSEWHEEL, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_AUX1_DOWN, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_AUX1_UP, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_AUX1_DCLICK, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_AUX2_DOWN, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_AUX2_UP, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_AUX2_DCLICK, wxMouseEvent);
wxDECLARE_EVENT(wxEVT_MAGNIFY, wxMouseEvent);

    // Character input event type
wxDECLARE_EVENT(wxEVT_CHAR, wxKeyEvent);
wxDECLARE_EVENT(wxEVT_CHAR_HOOK, wxKeyEvent);
wxDECLARE_EVENT(wxEVT_NAVIGATION_KEY, wxNavigationKeyEvent);
wxDECLARE_EVENT(wxEVT_KEY_DOWN, wxKeyEvent);
wxDECLARE_EVENT(wxEVT_KEY_UP, wxKeyEvent);
#if wxUSE_HOTKEY
wxDECLARE_EVENT(wxEVT_HOTKEY, wxKeyEvent);
#endif
// This is a private event used by wxMSW code only and subject to change or
// disappear in the future. Don't use.
wxDECLARE_EVENT(wxEVT_AFTER_CHAR, wxKeyEvent);

    // Set cursor event
wxDECLARE_EVENT(wxEVT_SET_CURSOR, wxSetCursorEvent);

    // wxScrollBar and wxSlider event identifiers
wxDECLARE_EVENT(wxEVT_SCROLL_TOP, wxScrollEvent);
wxDECLARE_EVENT(wxEVT_SCROLL_BOTTOM, wxScrollEvent);
wxDECLARE_EVENT(wxEVT_SCROLL_LINEUP, wxScrollEvent);
wxDECLARE_EVENT(wxEVT_SCROLL_LINEDOWN, wxScrollEvent);
wxDECLARE_EVENT(wxEVT_SCROLL_PAGEUP, wxScrollEvent);
wxDECLARE_EVENT(wxEVT_SCROLL_PAGEDOWN, wxScrollEvent);
wxDECLARE_EVENT(wxEVT_SCROLL_THUMBTRACK, wxScrollEvent);
wxDECLARE_EVENT(wxEVT_SCROLL_THUMBRELEASE, wxScrollEvent);
wxDECLARE_EVENT(wxEVT_SCROLL_CHANGED, wxScrollEvent);

// Due to a bug in older wx versions, wxSpinEvents were being sent with type of
// wxEVT_SCROLL_LINEUP, wxEVT_SCROLL_LINEDOWN and wxEVT_SCROLL_THUMBTRACK. But
// with the type-safe events in place, these event types are associated with
// wxScrollEvent. To allow handling of spin events, new event types have been
// defined in spinbutt.h/spinnbuttcmn.cpp. To maintain backward compatibility
// the spin event types are being initialized with the scroll event types.

#if wxUSE_SPINBTN

wxDECLARE_EXPORTED_EVENT_ALIAS(, wxEVT_SPIN_UP,   wxSpinEvent );
wxDECLARE_EXPORTED_EVENT_ALIAS(, wxEVT_SPIN_DOWN, wxSpinEvent );
wxDECLARE_EXPORTED_EVENT_ALIAS(, wxEVT_SPIN,      wxSpinEvent );

#endif

    // Scroll events from wxWindow
wxDECLARE_EVENT(wxEVT_SCROLLWIN_TOP, wxScrollWinEvent);
wxDECLARE_EVENT(wxEVT_SCROLLWIN_BOTTOM, wxScrollWinEvent);
wxDECLARE_EVENT(wxEVT_SCROLLWIN_LINEUP, wxScrollWinEvent);
wxDECLARE_EVENT(wxEVT_SCROLLWIN_LINEDOWN, wxScrollWinEvent);
wxDECLARE_EVENT(wxEVT_SCROLLWIN_PAGEUP, wxScrollWinEvent);
wxDECLARE_EVENT(wxEVT_SCROLLWIN_PAGEDOWN, wxScrollWinEvent);
wxDECLARE_EVENT(wxEVT_SCROLLWIN_THUMBTRACK, wxScrollWinEvent);
wxDECLARE_EVENT(wxEVT_SCROLLWIN_THUMBRELEASE, wxScrollWinEvent);

    // Gesture events
wxDECLARE_EVENT(wxEVT_GESTURE_PAN, wxPanGestureEvent);
wxDECLARE_EVENT(wxEVT_GESTURE_ZOOM, wxZoomGestureEvent);
wxDECLARE_EVENT(wxEVT_GESTURE_ROTATE, wxRotateGestureEvent);
wxDECLARE_EVENT(wxEVT_TWO_FINGER_TAP, wxTwoFingerTapEvent);
wxDECLARE_EVENT(wxEVT_LONG_PRESS, wxLongPressEvent);
wxDECLARE_EVENT(wxEVT_PRESS_AND_TAP, wxPressAndTapEvent);

    // System events
wxDECLARE_EVENT(wxEVT_SIZE, wxSizeEvent);
wxDECLARE_EVENT(wxEVT_MOVE, wxMoveEvent);
wxDECLARE_EVENT(wxEVT_CLOSE_WINDOW, wxCloseEvent);
wxDECLARE_EVENT(wxEVT_END_SESSION, wxCloseEvent);
wxDECLARE_EVENT(wxEVT_QUERY_END_SESSION, wxCloseEvent);
wxDECLARE_EVENT(wxEVT_ACTIVATE_APP, wxActivateEvent);
wxDECLARE_EVENT(wxEVT_ACTIVATE, wxActivateEvent);
wxDECLARE_EVENT(wxEVT_CREATE, wxWindowCreateEvent);
wxDECLARE_EVENT(wxEVT_DESTROY, wxWindowDestroyEvent);
wxDECLARE_EVENT(wxEVT_SHOW, wxShowEvent);
wxDECLARE_EVENT(wxEVT_ICONIZE, wxIconizeEvent);
wxDECLARE_EVENT(wxEVT_MAXIMIZE, wxMaximizeEvent);
wxDECLARE_EVENT(wxEVT_FULLSCREEN, wxFullScreenEvent);
wxDECLARE_EVENT(wxEVT_MOUSE_CAPTURE_CHANGED, wxMouseCaptureChangedEvent);
wxDECLARE_EVENT(wxEVT_MOUSE_CAPTURE_LOST, wxMouseCaptureLostEvent);
wxDECLARE_EVENT(wxEVT_PAINT, wxPaintEvent);
wxDECLARE_EVENT(wxEVT_ERASE_BACKGROUND, wxEraseEvent);
wxDECLARE_EVENT(wxEVT_NC_PAINT, wxNcPaintEvent);
wxDECLARE_EVENT(wxEVT_MENU_OPEN, wxMenuEvent);
wxDECLARE_EVENT(wxEVT_MENU_CLOSE, wxMenuEvent);
wxDECLARE_EVENT(wxEVT_MENU_HIGHLIGHT, wxMenuEvent);
wxDECLARE_EVENT(wxEVT_CONTEXT_MENU, wxContextMenuEvent);
wxDECLARE_EVENT(wxEVT_SYS_COLOUR_CHANGED, wxSysColourChangedEvent);
wxDECLARE_EVENT(wxEVT_DISPLAY_CHANGED, wxDisplayChangedEvent);
wxDECLARE_EVENT(wxEVT_DPI_CHANGED, wxDPIChangedEvent);
wxDECLARE_EVENT(wxEVT_QUERY_NEW_PALETTE, wxQueryNewPaletteEvent);
wxDECLARE_EVENT(wxEVT_PALETTE_CHANGED, wxPaletteChangedEvent);
wxDECLARE_EVENT(wxEVT_JOY_BUTTON_DOWN, wxJoystickEvent);
wxDECLARE_EVENT(wxEVT_JOY_BUTTON_UP, wxJoystickEvent);
wxDECLARE_EVENT(wxEVT_JOY_MOVE, wxJoystickEvent);
wxDECLARE_EVENT(wxEVT_JOY_ZMOVE, wxJoystickEvent);
wxDECLARE_EVENT(wxEVT_DROP_FILES, wxDropFilesEvent);
wxDECLARE_EVENT(wxEVT_INIT_DIALOG, wxInitDialogEvent);
wxDECLARE_EVENT(wxEVT_IDLE, wxIdleEvent);
wxDECLARE_EVENT(wxEVT_UPDATE_UI, wxUpdateUIEvent);
wxDECLARE_EVENT(wxEVT_SIZING, wxSizeEvent);
wxDECLARE_EVENT(wxEVT_MOVING, wxMoveEvent);
wxDECLARE_EVENT(wxEVT_MOVE_START, wxMoveEvent);
wxDECLARE_EVENT(wxEVT_MOVE_END, wxMoveEvent);
wxDECLARE_EVENT(wxEVT_HIBERNATE, wxActivateEvent);

    // Clipboard events
wxDECLARE_EVENT(wxEVT_TEXT_COPY, wxClipboardTextEvent);
wxDECLARE_EVENT(wxEVT_TEXT_CUT, wxClipboardTextEvent);
wxDECLARE_EVENT(wxEVT_TEXT_PASTE, wxClipboardTextEvent);

    // Generic command events
    // Note: a click is a higher-level event than button down/up
wxDECLARE_EVENT(wxEVT_COMMAND_LEFT_CLICK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_LEFT_DCLICK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_RIGHT_CLICK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_RIGHT_DCLICK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_SET_FOCUS, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_KILL_FOCUS, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_ENTER, wxCommandEvent);

    // Help events
wxDECLARE_EVENT(wxEVT_HELP, wxHelpEvent);
wxDECLARE_EVENT(wxEVT_DETAILED_HELP, wxHelpEvent);

// these 2 events are the same
#define wxEVT_TOOL wxEVT_MENU

// ----------------------------------------------------------------------------
// Compatibility
// ----------------------------------------------------------------------------

// this event is also used by wxComboBox and wxSpinCtrl which don't include
// wx/textctrl.h in all ports [yet], so declare it here as well
//
// still, any new code using it should include wx/textctrl.h explicitly
wxDECLARE_EVENT(wxEVT_TEXT, wxCommandEvent);


// ----------------------------------------------------------------------------
// wxEvent(-derived) classes
// ----------------------------------------------------------------------------

// the predefined constants for the number of times we propagate event
// upwards window child-parent chain
enum wxEventPropagation
{
    // don't propagate it at all
    wxEVENT_PROPAGATE_NONE = 0,

    // propagate it until it is processed
    wxEVENT_PROPAGATE_MAX = INT_MAX
};

// The different categories for a wxEvent; see wxEvent::GetEventCategory.
// NOTE: they are used as OR-combinable flags by wxEventLoopBase::YieldFor
enum wxEventCategory
{
    // this is the category for those events which are generated to update
    // the appearance of the GUI but which (usually) do not comport data
    // processing, i.e. which do not provide input or output data
    // (e.g. size events, scroll events, etc).
    // They are events NOT directly generated by the user's input devices.
    wxEVT_CATEGORY_UI = 1,

    // this category groups those events which are generated directly from the
    // user through input devices like mouse and keyboard and usually result in
    // data to be processed from the application.
    // (e.g. mouse clicks, key presses, etc)
    wxEVT_CATEGORY_USER_INPUT = 2,

    // this category is for wxSocketEvent
    wxEVT_CATEGORY_SOCKET = 4,

    // this category is for wxTimerEvent
    wxEVT_CATEGORY_TIMER = 8,

    // this category is for any event used to send notifications from the
    // secondary threads to the main one or in general for notifications among
    // different threads (which may or may not be user-generated)
    wxEVT_CATEGORY_THREAD = 16,


    // implementation only

    // used in the implementations of wxEventLoopBase::YieldFor
    wxEVT_CATEGORY_UNKNOWN = 32,

    // a special category used as an argument to wxEventLoopBase::YieldFor to indicate that
    // AppYield() should leave all wxEvents on the queue while emptying the native event queue
    // (native events will be processed but the wxEvents they generate will be queued)
    wxEVT_CATEGORY_CLIPBOARD = 64,


    // shortcut masks

    // this category groups those events which are emitted in response to
    // events of the native toolkit and which typically are not-"delayable".
    wxEVT_CATEGORY_NATIVE_EVENTS = wxEVT_CATEGORY_UI|wxEVT_CATEGORY_USER_INPUT,

    // used in wxEventLoopBase::YieldFor to specify all event categories should be processed:
    wxEVT_CATEGORY_ALL =
        wxEVT_CATEGORY_UI|wxEVT_CATEGORY_USER_INPUT|wxEVT_CATEGORY_SOCKET| \
        wxEVT_CATEGORY_TIMER|wxEVT_CATEGORY_THREAD|wxEVT_CATEGORY_UNKNOWN| \
        wxEVT_CATEGORY_CLIPBOARD
};

/*
 * wxWidgets events, covering all interesting things that might happen
 * (button clicking, resizing, setting text in widgets, etc.).
 *
 * For each completely new event type, derive a new event class.
 * An event CLASS represents a C++ class defining a range of similar event TYPES;
 * examples are canvas events, panel item command events.
 * An event TYPE is a unique identifier for a particular system event,
 * such as a button press or a listbox deselection.
 *
 */

class wxEvent : public wxObject
{
public:
    wxEvent(int winid = 0, wxEventType commandType = wxEVT_NULL );

    void SetEventType(wxEventType typ) { m_eventType = typ; }
    wxEventType GetEventType() const { return m_eventType; }

    wxObject *GetEventObject() const { return m_eventObject; }
    void SetEventObject(wxObject *obj) { m_eventObject = obj; }

    long GetTimestamp() const { return m_timeStamp; }
    void SetTimestamp(long ts = 0) { m_timeStamp = ts; }

    int GetId() const { return m_id; }
    void SetId(int Id) { m_id = Id; }

    // Returns the user data optionally associated with the event handler when
    // using Connect() or Bind().
    wxObject *GetEventUserData() const { return m_callbackUserData; }

    // Can instruct event processor that we wish to ignore this event
    // (treat as if the event table entry had not been found): this must be done
    // to allow the event processing by the base classes (calling event.Skip()
    // is the analog of calling the base class version of a virtual function)
    void Skip(bool skip = true) { m_skipped = skip; }
    bool GetSkipped() const { return m_skipped; }

    // This function is used to create a copy of the event polymorphically and
    // all derived classes must implement it because otherwise wxPostEvent()
    // for them wouldn't work (it needs to do a copy of the event)
    virtual wxEvent *Clone() const = 0;

    // this function is used to selectively process events in wxEventLoopBase::YieldFor
    // NOTE: by default it returns wxEVT_CATEGORY_UI just because the major
    //       part of wxWidgets events belong to that category.
    virtual wxEventCategory GetEventCategory() const
        { return wxEVT_CATEGORY_UI; }

    // Implementation only: this test is explicitly anti OO and this function
    // exists only for optimization purposes.
    bool IsCommandEvent() const { return m_isCommandEvent; }

    // Determine if this event should be propagating to the parent window.
    bool ShouldPropagate() const
        { return m_propagationLevel != wxEVENT_PROPAGATE_NONE; }

    // Stop an event from propagating to its parent window, returns the old
    // propagation level value
    int StopPropagation()
    {
        const int propagationLevel = m_propagationLevel;
        m_propagationLevel = wxEVENT_PROPAGATE_NONE;
        return propagationLevel;
    }

    // Resume the event propagation by restoring the propagation level
    // (returned by StopPropagation())
    void ResumePropagation(int propagationLevel)
    {
        m_propagationLevel = propagationLevel;
    }

    // This method is for internal use only and allows to get the object that
    // is propagating this event upwards the window hierarchy, if any.
    wxEvtHandler* GetPropagatedFrom() const { return m_propagatedFrom; }

    // This is for internal use only and is only called by
    // wxEvtHandler::ProcessEvent() to check whether it's the first time this
    // event is being processed
    bool WasProcessed()
    {
        if ( m_wasProcessed )
            return true;

        m_wasProcessed = true;

        return false;
    }

    // This is for internal use only and is used for setting, testing and
    // resetting of m_willBeProcessedAgain flag.
    void SetWillBeProcessedAgain()
    {
        m_willBeProcessedAgain = true;
    }

    bool WillBeProcessedAgain()
    {
        if ( m_willBeProcessedAgain )
        {
            m_willBeProcessedAgain = false;
            return true;
        }

        return false;
    }

    // This is also used only internally by ProcessEvent() to check if it
    // should process the event normally or only restrict the search for the
    // event handler to this object itself.
    bool ShouldProcessOnlyIn(wxEvtHandler *h) const
    {
        return h == m_handlerToProcessOnlyIn;
    }

    // Called to indicate that the result of ShouldProcessOnlyIn() wasn't taken
    // into account. The existence of this function may seem counterintuitive
    // but unfortunately it's needed by wxScrollHelperEvtHandler, see comments
    // there. Don't even think of using this in your own code, this is a gross
    // hack and is only needed because of wx complicated history and should
    // never be used anywhere else.
    void DidntHonourProcessOnlyIn()
    {
        m_handlerToProcessOnlyIn = nullptr;
    }

protected:
    wxObject*         m_eventObject{nullptr};

public:
    // m_callbackUserData is for internal usage only
    wxObject*         m_callbackUserData{nullptr};

private:
    // If this handler
    wxEvtHandler *m_handlerToProcessOnlyIn{nullptr};

protected:
    // The object that the event is being propagated from, initially NULL and
    // only set by wxPropagateOnce.
    wxEvtHandler*     m_propagatedFrom{nullptr};

    wxEventType       m_eventType;
    long              m_timeStamp{0};
    int               m_id;
    
    // the propagation level: while it is positive, we propagate the event to
    // the parent window (if any)
    int               m_propagationLevel{wxEVENT_PROPAGATE_NONE};

    bool              m_skipped{false};
    bool              m_isCommandEvent{false};

    // initially false but becomes true as soon as WasProcessed() is called for
    // the first time, as this is done only by ProcessEvent() it explains the
    // variable name: it becomes true after ProcessEvent() was called at least
    // once for this event
    bool m_wasProcessed{false};

    // This one is initially false too, but can be set to true to indicate that
    // the event will be passed to another handler if it's not processed in
    // this one.
    bool m_willBeProcessedAgain{false};

protected:
    wxEvent(const wxEvent&);            // for implementing Clone()
    wxEvent& operator=(const wxEvent&); // for derived classes operator=()

private:
    // It needs to access our m_propagationLevel and m_propagatedFrom fields.
    friend class wxPropagateOnce;

    // and this one needs to access our m_handlerToProcessOnlyIn
    friend class wxEventProcessInHandlerOnly;


    wxDECLARE_ABSTRACT_CLASS(wxEvent);
};

/*
 * Helper class to temporarily change an event not to propagate.
 */
class wxPropagationDisabler
{
public:
    wxPropagationDisabler(wxEvent& event) : m_event(event)
    {
        m_propagationLevelOld = m_event.StopPropagation();
    }

    ~wxPropagationDisabler()
    {
        m_event.ResumePropagation(m_propagationLevelOld);
    }

private:
    wxEvent& m_event;
    int m_propagationLevelOld;
};

/*
 * Helper used to indicate that an event is propagated upwards the window
 * hierarchy by the given window.
 */
class wxPropagateOnce
{
public:
    // The handler argument should normally be non-NULL to allow the parent
    // event handler to know that it's being used to process an event coming
    // from the child, it's only NULL by default for backwards compatibility.
    wxPropagateOnce(wxEvent& event, wxEvtHandler* handler = nullptr)
        : m_event(event),
          m_propagatedFromOld(event.m_propagatedFrom)
    {
        wxASSERT_MSG( m_event.m_propagationLevel > 0,
                      "shouldn't be used unless ShouldPropagate()!" );

        m_event.m_propagationLevel--;
        m_event.m_propagatedFrom = handler;
    }

    ~wxPropagateOnce()
    {
        m_event.m_propagatedFrom = m_propagatedFromOld;
        m_event.m_propagationLevel++;
    }

private:
    wxEvent& m_event;
    wxEvtHandler* const m_propagatedFromOld;
};

// A helper object used to temporarily make wxEvent::ShouldProcessOnlyIn()
// return true for the handler passed to its ctor.
class wxEventProcessInHandlerOnly
{
public:
    wxEventProcessInHandlerOnly(wxEvent& event, wxEvtHandler *handler)
        : m_event(event),
          m_handlerToProcessOnlyInOld(event.m_handlerToProcessOnlyIn)
    {
        m_event.m_handlerToProcessOnlyIn = handler;
    }

    ~wxEventProcessInHandlerOnly()
    {
        m_event.m_handlerToProcessOnlyIn = m_handlerToProcessOnlyInOld;
    }

private:
    wxEvent& m_event;
    wxEvtHandler * const m_handlerToProcessOnlyInOld;
};


class wxEventBasicPayloadMixin
{
public:
    wxEventBasicPayloadMixin() = default;

    wxEventBasicPayloadMixin& operator=(const wxEventBasicPayloadMixin&) = delete;

    void SetString(const std::string& s) { m_cmdString = s; }
    const std::string& GetString() const { return m_cmdString; }

    void SetInt(unsigned int i) { m_commandInt = i; }
    int GetInt() const { return m_commandInt; }

    void SetExtraLong(long extraLong) { m_extraLong = extraLong; }
    long GetExtraLong() const { return m_extraLong; }

protected:
    // Note: these variables have "cmd" or "command" in their name for backward compatibility:
    //       they used to be part of wxCommandEvent, not this mixin.
    std::string          m_cmdString;     // String event argument
    unsigned int         m_commandInt{0};
    long                 m_extraLong{0};     // Additional information (e.g. select/deselect)
};

class wxEventAnyPayloadMixin : public wxEventBasicPayloadMixin
{
public:
    wxEventAnyPayloadMixin()  = default;

    wxEventBasicPayloadMixin& operator=(const wxEventBasicPayloadMixin&) = delete;

#if wxUSE_ANY
    template<typename T>
    void SetPayload(const T& payload)
    {
        m_payload = payload;
    }

    template<typename T>
    T GetPayload() const
    {
        return m_payload.As<T>();
    }

protected:
    wxAny m_payload;
#endif // wxUSE_ANY
};


// Idle event
/*
 wxEVT_IDLE
 */

// Whether to always send idle events to windows, or
// to only send update events to those with the
// wxWS_EX_PROCESS_IDLE style.

enum wxIdleMode
{
        // Send idle events to all windows
    wxIDLE_PROCESS_ALL,

        // Send idle events to windows that have
        // the wxWS_EX_PROCESS_IDLE flag specified
    wxIDLE_PROCESS_SPECIFIED
};

class wxIdleEvent : public wxEvent
{
public:
    wxIdleEvent()
        : wxEvent(0, wxEVT_IDLE)
          
        { }
    wxIdleEvent(const wxIdleEvent& event) = default;

	wxIdleEvent& operator=(const wxIdleEvent&) = delete;

    void RequestMore(bool needMore = true) { m_requestMore = needMore; }
    bool MoreRequested() const { return m_requestMore; }

    wxEvent *Clone() const override { return new wxIdleEvent(*this); }

    // Specify how wxWidgets will send idle events: to
    // all windows, or only to those which specify that they
    // will process the events.
    static void SetMode(wxIdleMode mode) { sm_idleMode = mode; }

    // Returns the idle event mode
    static wxIdleMode GetMode() { return sm_idleMode; }

protected:
    bool m_requestMore{false};
    inline static wxIdleMode sm_idleMode{wxIDLE_PROCESS_ALL};

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};


// Thread event

class wxThreadEvent : public wxEvent,
                                       public wxEventAnyPayloadMixin
{
public:
    wxThreadEvent(wxEventType eventType = wxEVT_THREAD, int id = wxID_ANY)
        : wxEvent(id, eventType)
        { }

    wxThreadEvent(const wxThreadEvent& event)
        : wxEvent(event),
          wxEventAnyPayloadMixin(event)
    {
        SetString(GetString());
    }

	wxThreadEvent& operator=(const wxThreadEvent&) = delete;

    wxEvent *Clone() const override
    {
        return new wxThreadEvent(*this);
    }

    // this is important to avoid that calling wxEventLoopBase::YieldFor thread events
    // gets processed when this is unwanted:
    wxEventCategory GetEventCategory() const override
        { return wxEVT_CATEGORY_THREAD; }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};


// Asynchronous method call events: these event are processed by wxEvtHandler
// itself and result in a call to its Execute() method which simply calls the
// specified method. The difference with a simple method call is that this is
// done asynchronously, i.e. at some later time, instead of immediately when
// the event object is constructed.

// This is a base class used to process all method calls.
class wxAsyncMethodCallEvent : public wxEvent
{
public:
    wxAsyncMethodCallEvent(wxObject* object)
        : wxEvent(wxID_ANY, wxEVT_ASYNC_METHOD_CALL)
    {
        SetEventObject(object);
    }

    wxAsyncMethodCallEvent(const wxAsyncMethodCallEvent& other) = default;

    virtual void Execute() = 0;
};

// This is a version for calling methods without parameters.
template <typename T>
class wxAsyncMethodCallEvent0 : public wxAsyncMethodCallEvent
{
public:
    using ObjectType = T;
    typedef void (ObjectType::*MethodType)();

    wxAsyncMethodCallEvent0(ObjectType* object,
                            MethodType method)
        : wxAsyncMethodCallEvent(object),
          m_object(object),
          m_method(method)
    {
    }

    wxAsyncMethodCallEvent0(const wxAsyncMethodCallEvent0& other)
        : wxAsyncMethodCallEvent(other),
          m_object(other.m_object),
          m_method(other.m_method)
    {
    }

    wxEvent *Clone() const override
    {
        return new wxAsyncMethodCallEvent0(*this);
    }

    void Execute() override
    {
        (m_object->*m_method)();
    }

private:
    ObjectType* const m_object;
    const MethodType m_method;
};

// This is a version for calling methods with a single parameter.
template <typename T, typename T1>
class wxAsyncMethodCallEvent1 : public wxAsyncMethodCallEvent
{
public:
    using ObjectType = T;
    typedef void (ObjectType::*MethodType)(T1 x1);
    using ParamType1 = std::remove_reference_t<T1>;

    wxAsyncMethodCallEvent1(ObjectType* object,
                            MethodType method,
                            const ParamType1& x1)
        : wxAsyncMethodCallEvent(object),
          m_object(object),
          m_method(method),
          m_param1(x1)
    {
    }

    wxAsyncMethodCallEvent1(const wxAsyncMethodCallEvent1& other)
        : wxAsyncMethodCallEvent(other),
          m_object(other.m_object),
          m_method(other.m_method),
          m_param1(other.m_param1)
    {
    }

    wxEvent *Clone() const override
    {
        return new wxAsyncMethodCallEvent1(*this);
    }

    void Execute() override
    {
        (m_object->*m_method)(m_param1);
    }

private:
    ObjectType* const m_object;
    const MethodType m_method;
    const ParamType1 m_param1;
};

// This is a version for calling methods with two parameters.
template <typename T, typename T1, typename T2>
class wxAsyncMethodCallEvent2 : public wxAsyncMethodCallEvent
{
public:
    using ObjectType = T;
    typedef void (ObjectType::*MethodType)(T1 x1, T2 x2);
    using ParamType1 = std::remove_reference_t<T1>;
    using ParamType2 = std::remove_reference_t<T1>;

    wxAsyncMethodCallEvent2(ObjectType* object,
                            MethodType method,
                            const ParamType1& x1,
                            const ParamType2& x2)
        : wxAsyncMethodCallEvent(object),
          m_object(object),
          m_method(method),
          m_param1(x1),
          m_param2(x2)
    {
    }

    wxAsyncMethodCallEvent2(const wxAsyncMethodCallEvent2& other)
        : wxAsyncMethodCallEvent(other),
          m_object(other.m_object),
          m_method(other.m_method),
          m_param1(other.m_param1),
          m_param2(other.m_param2)
    {
    }

    wxEvent *Clone() const override
    {
        return new wxAsyncMethodCallEvent2(*this);
    }

    void Execute() override
    {
        (m_object->*m_method)(m_param1, m_param2);
    }

private:
    ObjectType* const m_object;
    const MethodType m_method;
    const ParamType1 m_param1;
    const ParamType2 m_param2;
};

// This is a version for calling any functors
template <typename T>
class wxAsyncMethodCallEventFunctor : public wxAsyncMethodCallEvent
{
public:
    using FunctorType = T;

    wxAsyncMethodCallEventFunctor(wxObject *object, const FunctorType& fn)
        : wxAsyncMethodCallEvent(object),
          m_fn(fn)
    {
    }

    wxAsyncMethodCallEventFunctor(const wxAsyncMethodCallEventFunctor& other)
        : wxAsyncMethodCallEvent(other),
          m_fn(other.m_fn)
    {
    }

    wxEvent *Clone() const override
    {
        return new wxAsyncMethodCallEventFunctor(*this);
    }

    void Execute() override
    {
        m_fn();
    }

private:
    FunctorType m_fn;
};

#if wxUSE_GUI


// Item or menu event class
/*
 wxEVT_BUTTON
 wxEVT_CHECKBOX
 wxEVT_CHOICE
 wxEVT_LISTBOX
 wxEVT_LISTBOX_DCLICK
 wxEVT_TEXT
 wxEVT_TEXT_ENTER
 wxEVT_MENU
 wxEVT_SLIDER
 wxEVT_RADIOBOX
 wxEVT_RADIOBUTTON
 wxEVT_SCROLLBAR
 wxEVT_VLBOX
 wxEVT_COMBOBOX
 wxEVT_TOGGLEBUTTON
*/

class wxCommandEvent : public wxEvent,
                                        public wxEventBasicPayloadMixin
{
public:
    wxCommandEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxEvent(winid, commandType)
    {
        m_clientData = nullptr;
        m_clientObject = nullptr;
        m_isCommandEvent = true;

        // the command events are propagated upwards by default
        m_propagationLevel = wxEVENT_PROPAGATE_MAX;
    }

    wxCommandEvent(const wxCommandEvent& event)
        : wxEvent(event),
          wxEventBasicPayloadMixin(event),
          m_clientData(event.m_clientData),
          m_clientObject(event.m_clientObject)
    {
        // Because GetString() can retrieve the string text only on demand, we
        // need to copy it explicitly.
        if ( m_cmdString.empty() )
            m_cmdString = event.GetString();
    }

	wxCommandEvent& operator=(const wxCommandEvent&) = delete;

    // Set/Get client data from controls
    void SetClientData(void* clientData) { m_clientData = clientData; }
    void *GetClientData() const { return m_clientData; }

    // Set/Get client object from controls
    void SetClientObject(wxClientData* clientObject) { m_clientObject = clientObject; }
    wxClientData *GetClientObject() const { return m_clientObject; }

    // Note: this shadows wxEventBasicPayloadMixin::GetString(), because it does some
    // GUI-specific hacks
    std::string GetString() const;

    // Get listbox selection if single-choice
    int GetSelection() const { return m_commandInt; }

    // Get checkbox value
    bool IsChecked() const { return m_commandInt != 0; }

    // true if the listbox event was a selection.
    bool IsSelection() const { return (m_extraLong != 0); }

    wxEvent *Clone() const override { return new wxCommandEvent(*this); }
    wxEventCategory GetEventCategory() const override { return wxEVT_CATEGORY_USER_INPUT; }

protected:
    void*             m_clientData;    // Arbitrary client data
    wxClientData*     m_clientObject;  // Arbitrary client object

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// this class adds a possibility to react (from the user) code to a control
// notification: allow or veto the operation being reported.
class wxNotifyEvent  : public wxCommandEvent
{
public:
    wxNotifyEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxCommandEvent(commandType, winid)
        { m_bAllow = true; }

    wxNotifyEvent(const wxNotifyEvent& event)
        : wxCommandEvent(event)
        { m_bAllow = event.m_bAllow; }

	wxNotifyEvent& operator=(const wxNotifyEvent&) = delete;

    // veto the operation (usually it's allowed by default)
    void Veto() { m_bAllow = false; }

    // allow the operation if it was disabled by default
    void Allow() { m_bAllow = true; }

    // for implementation code only: is the operation allowed?
    bool IsAllowed() const { return m_bAllow; }

    wxEvent *Clone() const override { return new wxNotifyEvent(*this); }

private:
    bool m_bAllow;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};


// Scroll event class, derived form wxCommandEvent. wxScrollEvents are
// sent by wxSlider and wxScrollBar.
/*
 wxEVT_SCROLL_TOP
 wxEVT_SCROLL_BOTTOM
 wxEVT_SCROLL_LINEUP
 wxEVT_SCROLL_LINEDOWN
 wxEVT_SCROLL_PAGEUP
 wxEVT_SCROLL_PAGEDOWN
 wxEVT_SCROLL_THUMBTRACK
 wxEVT_SCROLL_THUMBRELEASE
 wxEVT_SCROLL_CHANGED
*/

class wxScrollEvent : public wxCommandEvent
{
public:
    wxScrollEvent(wxEventType commandType = wxEVT_NULL,
                  int winid = 0, int pos = 0, int orient = 0);

	wxScrollEvent& operator=(const wxScrollEvent&) = delete;

    int GetOrientation() const { return (int) m_extraLong; }
    int GetPosition() const { return m_commandInt; }
    void SetOrientation(int orient) { m_extraLong = (long) orient; }
    void SetPosition(int pos) { m_commandInt = pos; }

    wxEvent *Clone() const override { return new wxScrollEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// ScrollWin event class, derived fom wxEvent. wxScrollWinEvents
// are sent by wxWindow.
/*
 wxEVT_SCROLLWIN_TOP
 wxEVT_SCROLLWIN_BOTTOM
 wxEVT_SCROLLWIN_LINEUP
 wxEVT_SCROLLWIN_LINEDOWN
 wxEVT_SCROLLWIN_PAGEUP
 wxEVT_SCROLLWIN_PAGEDOWN
 wxEVT_SCROLLWIN_THUMBTRACK
 wxEVT_SCROLLWIN_THUMBRELEASE
*/

class wxScrollWinEvent : public wxEvent
{
public:
    wxScrollWinEvent(wxEventType commandType = wxEVT_NULL,
                     int pos = 0, int orient = 0);
    wxScrollWinEvent(const wxScrollWinEvent& event) : wxEvent(event)
        {    m_commandInt = event.m_commandInt;
            m_extraLong = event.m_extraLong;    }

	wxScrollWinEvent& operator=(const wxScrollWinEvent&) = delete;

    int GetOrientation() const { return (int) m_extraLong; }
    int GetPosition() const { return m_commandInt; }
    void SetOrientation(int orient) { m_extraLong = (long) orient; }
    void SetPosition(int pos) { m_commandInt = pos; }

    wxEvent *Clone() const override { return new wxScrollWinEvent(*this); }

protected:
    int               m_commandInt;
    long              m_extraLong;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};



// Mouse event class

/*
 wxEVT_LEFT_DOWN
 wxEVT_LEFT_UP
 wxEVT_MIDDLE_DOWN
 wxEVT_MIDDLE_UP
 wxEVT_RIGHT_DOWN
 wxEVT_RIGHT_UP
 wxEVT_MOTION
 wxEVT_ENTER_WINDOW
 wxEVT_LEAVE_WINDOW
 wxEVT_LEFT_DCLICK
 wxEVT_MIDDLE_DCLICK
 wxEVT_RIGHT_DCLICK
*/

enum class wxMouseWheelAxis
{
    Vertical,
    Horizontal
};

class wxMouseEvent : public wxEvent,
                                      public wxMouseState
{
public:
    wxMouseEvent(wxEventType mouseType = wxEVT_NULL)
    {
        m_eventType = mouseType;
    }

    wxMouseEvent(const wxMouseEvent& event)
        : wxEvent(event),
          wxMouseState(event)
    {
        Assign(event);
    }

    // Was it a button event? (*doesn't* mean: is any button *down*?)
    bool IsButton() const { return Button(wxMOUSE_BTN_ANY); }

    // Was it a down event from this (or any) button?
    bool ButtonDown(int but = wxMOUSE_BTN_ANY) const;

    // Was it a dclick event from this (or any) button?
    bool ButtonDClick(int but = wxMOUSE_BTN_ANY) const;

    // Was it a up event from this (or any) button?
    bool ButtonUp(int but = wxMOUSE_BTN_ANY) const;

    // Was this event generated by the given button?
    bool Button(int but) const;

    // Get the button which is changing state (wxMOUSE_BTN_NONE if none)
    int GetButton() const;

    // Find which event was just generated
    bool LeftDown() const { return (m_eventType == wxEVT_LEFT_DOWN); }
    bool MiddleDown() const { return (m_eventType == wxEVT_MIDDLE_DOWN); }
    bool RightDown() const { return (m_eventType == wxEVT_RIGHT_DOWN); }
    bool Aux1Down() const { return (m_eventType == wxEVT_AUX1_DOWN); }
    bool Aux2Down() const { return (m_eventType == wxEVT_AUX2_DOWN); }

    bool LeftUp() const { return (m_eventType == wxEVT_LEFT_UP); }
    bool MiddleUp() const { return (m_eventType == wxEVT_MIDDLE_UP); }
    bool RightUp() const { return (m_eventType == wxEVT_RIGHT_UP); }
    bool Aux1Up() const { return (m_eventType == wxEVT_AUX1_UP); }
    bool Aux2Up() const { return (m_eventType == wxEVT_AUX2_UP); }

    bool LeftDClick() const { return (m_eventType == wxEVT_LEFT_DCLICK); }
    bool MiddleDClick() const { return (m_eventType == wxEVT_MIDDLE_DCLICK); }
    bool RightDClick() const { return (m_eventType == wxEVT_RIGHT_DCLICK); }
    bool Aux1DClick() const { return (m_eventType == wxEVT_AUX1_DCLICK); }
    bool Aux2DClick() const { return (m_eventType == wxEVT_AUX2_DCLICK); }

    bool Magnify() const { return (m_eventType == wxEVT_MAGNIFY); }

    // True if a button is down and the mouse is moving
    bool Dragging() const
    {
        return (m_eventType == wxEVT_MOTION) && ButtonIsDown(wxMOUSE_BTN_ANY);
    }

    // True if the mouse is moving, and no button is down
    bool Moving() const
    {
        return (m_eventType == wxEVT_MOTION) && !ButtonIsDown(wxMOUSE_BTN_ANY);
    }

    // True if the mouse is just entering the window
    bool Entering() const { return (m_eventType == wxEVT_ENTER_WINDOW); }

    // True if the mouse is just leaving the window
    bool Leaving() const { return (m_eventType == wxEVT_LEAVE_WINDOW); }

    // Returns the number of mouse clicks associated with this event.
    int GetClickCount() const { return m_clickCount; }

    // Find the logical position of the event given the DC
    wxPoint GetLogicalPosition(const wxDC& dc) const;

    // Get wheel rotation, positive or negative indicates direction of
    // rotation.  Current devices all send an event when rotation is equal to
    // +/-WheelDelta, but this allows for finer resolution devices to be
    // created in the future.  Because of this you shouldn't assume that one
    // event is equal to 1 line or whatever, but you should be able to either
    // do partial line scrolling or wait until +/-WheelDelta rotation values
    // have been accumulated before scrolling.
    int GetWheelRotation() const { return m_wheelRotation; }

    // Get wheel delta, normally 120.  This is the threshold for action to be
    // taken, and one such action (for example, scrolling one increment)
    // should occur for each delta.
    int GetWheelDelta() const { return m_wheelDelta; }

    // On Mac, has the user selected "Natural" scrolling in their System
    // Preferences? Currently false on all other OS's.
    bool IsWheelInverted() const { return m_wheelInverted; }

    // Gets the axis the wheel operation concerns; wxMouseWheelAxis::Vertical
    // (most common case) or wxMouseWheelAxis::Horizontal (for horizontal scrolling
    // using e.g. a trackpad).
    wxMouseWheelAxis GetWheelAxis() const { return m_wheelAxis; }

    // Returns the configured number of lines (or whatever) to be scrolled per
    // wheel action. Defaults to three.
    int GetLinesPerAction() const { return m_linesPerAction; }

    // Returns the configured number of columns (or whatever) to be scrolled per
    // wheel action. Defaults to three.
    int GetColumnsPerAction() const { return m_columnsPerAction; }

    // Is the system set to do page scrolling?
    bool IsPageScroll() const { return ((unsigned int)m_linesPerAction == UINT_MAX); }

    float GetMagnification() const { return m_magnification; }
    wxEvent *Clone() const override { return new wxMouseEvent(*this); }
    wxEventCategory GetEventCategory() const override { return wxEVT_CATEGORY_USER_INPUT; }

    wxMouseEvent& operator=(const wxMouseEvent& event)
    {
        if (&event != this)
            Assign(event);
        return *this;
    }

public:
    int           m_clickCount{-1};

    int           m_wheelRotation{0};
    int           m_wheelDelta{0};
    int           m_linesPerAction{0};
    int           m_columnsPerAction{0};

    float         m_magnification{0.0F};
    wxMouseWheelAxis m_wheelAxis{wxMouseWheelAxis::Vertical};

    bool          m_wheelInverted{false};

protected:
    void Assign(const wxMouseEvent& evt);

private:
    wxDECLARE_DYNAMIC_CLASS(wxMouseEvent);
};

// Cursor set event

/*
   wxEVT_SET_CURSOR
 */

class wxSetCursorEvent : public wxEvent
{
public:
    wxSetCursorEvent(wxCoord x = 0, wxCoord y = 0)
        : wxEvent(0, wxEVT_SET_CURSOR),
          m_x(x), m_y(y) 
        { }

    wxSetCursorEvent(const wxSetCursorEvent& event) = default;

	wxSetCursorEvent& operator=(const wxSetCursorEvent&) = delete;

    wxCoord GetX() const { return m_x; }
    wxCoord GetY() const { return m_y; }

    void SetCursor(const wxCursor& cursor) { m_cursor = cursor; }
    const wxCursor& GetCursor() const { return m_cursor; }
    bool HasCursor() const { return m_cursor.IsOk(); }

    wxEvent *Clone() const override { return new wxSetCursorEvent(*this); }

private:
    wxCursor m_cursor;

    wxCoord  m_x, m_y;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

 // Gesture Event

inline constexpr unsigned int wxTwoFingerTimeInterval = 200;

class wxGestureEvent : public wxEvent
{
public:
    wxGestureEvent(wxWindowID winid = 0, wxEventType type = wxEVT_NULL)
        : wxEvent(winid, type)
    {
        m_isStart = false;
        m_isEnd = false;
    }

    wxGestureEvent(const wxGestureEvent& event) : wxEvent(event)
        , m_pos(event.m_pos)
    {
        m_isStart = event.m_isStart;
        m_isEnd = event.m_isEnd;
    }

	wxGestureEvent& operator=(const wxGestureEvent&) = delete;

    const wxPoint& GetPosition() const { return m_pos; }
    void SetPosition(const wxPoint& pos) { m_pos = pos; }
    bool IsGestureStart() const { return m_isStart; }
    void SetGestureStart(bool isStart = true) { m_isStart = isStart; }
    bool IsGestureEnd() const { return m_isEnd; }
    void SetGestureEnd(bool isEnd = true) { m_isEnd = isEnd; }

    wxEvent *Clone() const override { return new wxGestureEvent(*this); }

protected:
    wxPoint m_pos;
    bool m_isStart, m_isEnd;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();

};

 // Pan Gesture Event

 /*
  wxEVT_GESTURE_PAN
  */

class wxPanGestureEvent : public wxGestureEvent
{
public:
    wxPanGestureEvent(wxWindowID winid = 0)
        : wxGestureEvent(winid, wxEVT_GESTURE_PAN)
    {
    }

    wxPanGestureEvent(const wxPanGestureEvent& event) = default;

	wxPanGestureEvent& operator=(const wxPanGestureEvent&) = delete;

    wxPoint GetDelta() const { return m_delta; }
    void SetDelta(const wxPoint& delta) { m_delta = delta; }

    wxEvent *Clone() const override { return new wxPanGestureEvent(*this); }

private:
    wxPoint m_delta;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

 // Zoom Gesture Event

 /*
  wxEVT_GESTURE_ZOOM
  */

class wxZoomGestureEvent : public wxGestureEvent
{
public:
    wxZoomGestureEvent(wxWindowID winid = 0)
        : wxGestureEvent(winid, wxEVT_GESTURE_ZOOM)
        { m_zoomFactor = 1.0; }

    wxZoomGestureEvent(const wxZoomGestureEvent& event) : wxGestureEvent(event)
    {
        m_zoomFactor = event.m_zoomFactor;
    }

	wxZoomGestureEvent& operator=(const wxZoomGestureEvent&) = delete;

    double GetZoomFactor() const { return m_zoomFactor; }
    void SetZoomFactor(double zoomFactor) { m_zoomFactor = zoomFactor; }

    wxEvent *Clone() const override { return new wxZoomGestureEvent(*this); }

private:
    double m_zoomFactor;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

 // Rotate Gesture Event

 /*
  wxEVT_GESTURE_ROTATE
  */

class wxRotateGestureEvent : public wxGestureEvent
{
public:
    wxRotateGestureEvent(wxWindowID winid = 0)
        : wxGestureEvent(winid, wxEVT_GESTURE_ROTATE)
        { m_rotationAngle = 0.0; }

    wxRotateGestureEvent(const wxRotateGestureEvent& event) : wxGestureEvent(event)
    {
        m_rotationAngle = event.m_rotationAngle;
    }

	wxRotateGestureEvent& operator=(const wxRotateGestureEvent&) = delete;

    double GetRotationAngle() const { return m_rotationAngle; }
    void SetRotationAngle(double rotationAngle) { m_rotationAngle = rotationAngle; }

    wxEvent *Clone() const override { return new wxRotateGestureEvent(*this); }

private:
    double m_rotationAngle;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

 // Two Finger Tap Gesture Event

 /*
  wxEVT_TWO_FINGER_TAP
  */

class wxTwoFingerTapEvent : public wxGestureEvent
{
public:
    wxTwoFingerTapEvent(wxWindowID winid = 0)
        : wxGestureEvent(winid, wxEVT_TWO_FINGER_TAP)
        { }

    wxTwoFingerTapEvent(const wxTwoFingerTapEvent& event) = default;

	wxTwoFingerTapEvent& operator=(const wxTwoFingerTapEvent&) = delete;

    wxEvent *Clone() const override { return new wxTwoFingerTapEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

 // Long Press Gesture Event

 /*
  wxEVT_LONG_PRESS
  */

class wxLongPressEvent : public wxGestureEvent
{
public:
    wxLongPressEvent(wxWindowID winid = 0)
        : wxGestureEvent(winid, wxEVT_LONG_PRESS)
        { }

    wxLongPressEvent(const wxLongPressEvent& event) = default;

	wxLongPressEvent& operator=(const wxLongPressEvent&) = delete;

    wxEvent *Clone() const override { return new wxLongPressEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

 // Press And Tap Gesture Event

 /*
  wxEVT_PRESS_AND_TAP
  */

class wxPressAndTapEvent : public wxGestureEvent
{
public:
    wxPressAndTapEvent(wxWindowID winid = 0)
        : wxGestureEvent(winid, wxEVT_PRESS_AND_TAP)
        { }

    wxPressAndTapEvent(const wxPressAndTapEvent& event) = default;

	wxPressAndTapEvent& operator=(const wxPressAndTapEvent&) = delete;

    wxEvent *Clone() const override { return new wxPressAndTapEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Keyboard input event class

/*
 wxEVT_CHAR
 wxEVT_CHAR_HOOK
 wxEVT_KEY_DOWN
 wxEVT_KEY_UP
 wxEVT_HOTKEY
 */

// key categories: the bit flags for IsKeyInCategory() function
//
// the enum values used may change in future version of wx
// use the named constants only, or bitwise combinations thereof
enum wxKeyCategoryFlags
{
    // arrow keys, on and off numeric keypads
    WXK_CATEGORY_ARROW  = 1,

    // page up and page down keys, on and off numeric keypads
    WXK_CATEGORY_PAGING = 2,

    // home and end keys, on and off numeric keypads
    WXK_CATEGORY_JUMP   = 4,

    // tab key, on and off numeric keypads
    WXK_CATEGORY_TAB    = 8,

    // backspace and delete keys, on and off numeric keypads
    WXK_CATEGORY_CUT    = 16,

    // all keys usually used for navigation
    WXK_CATEGORY_NAVIGATION = WXK_CATEGORY_ARROW |
                              WXK_CATEGORY_PAGING |
                              WXK_CATEGORY_JUMP
};

class wxKeyEvent : public wxEvent,
                                    public wxKeyboardState
{
public:
    wxKeyEvent(wxEventType keyType = wxEVT_NULL);

    // Normal copy ctor and a ctor creating a new event for the same key as the
    // given one but a different event type (this is used in implementation
    // code only, do not use outside of the library).
    wxKeyEvent(const wxKeyEvent& evt);
    wxKeyEvent(wxEventType eventType, const wxKeyEvent& evt);

    // get the key code: an ASCII7 char or an element of wxKeyCode enum
    int GetKeyCode() const { return (int)m_keyCode; }

    // returns true iff this event's key code is of a certain type
    bool IsKeyInCategory(int category) const;

    // get the Unicode character corresponding to this key
    wxChar GetUnicodeKey() const { return m_uniChar; }

    // get the raw key code (platform-dependent)
    std::uint32_t GetRawKeyCode() const { return m_rawCode; }

    // get the raw key flags (platform-dependent)
    std::uint32_t GetRawKeyFlags() const { return m_rawFlags; }

    // Find the position of the event
    wxPoint GetPosition() const
        { return wxPoint(m_x, m_y); }

    // Get X position
    wxCoord GetX() const;

    // Get Y position
    wxCoord GetY() const;

    // Can be called from wxEVT_CHAR_HOOK handler to allow generation of normal
    // key events even though the event had been handled (by default they would
    // not be generated in this case).
    void DoAllowNextEvent() { m_allowNext = true; }

    // Return the value of the "allow next" flag, for internal use only.
    bool IsNextEventAllowed() const { return m_allowNext; }


    wxEvent *Clone() const override { return new wxKeyEvent(*this); }
    wxEventCategory GetEventCategory() const override { return wxEVT_CATEGORY_USER_INPUT; }

    // we do need to copy wxKeyEvent sometimes (in wxTreeCtrl code, for
    // example)
    wxKeyEvent& operator=(const wxKeyEvent& evt);

public:
    // these fields contain the platform-specific information about
    // key that was pressed
    std::uint32_t      m_rawCode;
    std::uint32_t      m_rawFlags;

    // Do not use these fields directly, they are initialized on demand, so
    // call GetX() and GetY() or GetPosition() instead.
    wxCoord       m_x, m_y;

    long          m_keyCode;

    // This contains the full Unicode character
    // in a character events in Unicode mode
    wxChar        m_uniChar{WXK_NONE};

private:
    // Set the event to propagate if necessary, i.e. if it's of wxEVT_CHAR_HOOK
    // type. This is used by all ctors.
    void InitPropagation()
    {
        if ( m_eventType == wxEVT_CHAR_HOOK )
            m_propagationLevel = wxEVENT_PROPAGATE_MAX;

        m_allowNext = false;
    }

    // Copy only the event data present in this class, this is used by
    // AssignKeyData() and copy ctor.
    void DoAssignMembers(const wxKeyEvent& evt)
    {
        m_x = evt.m_x;
        m_y = evt.m_y;
        m_hasPosition = evt.m_hasPosition;

        m_keyCode = evt.m_keyCode;

        m_rawCode = evt.m_rawCode;
        m_rawFlags = evt.m_rawFlags;
        m_uniChar = evt.m_uniChar;
    }

    // Initialize m_x and m_y using the current mouse cursor position if
    // necessary.
    void InitPositionIfNecessary() const;

    // If this flag is true, the normal key events should still be generated
    // even if wxEVT_CHAR_HOOK had been handled. By default it is false as
    // handling wxEVT_CHAR_HOOK suppresses all the subsequent events.
    bool m_allowNext;

    // If true, m_x and m_y were already initialized. If false, try to get them
    // when they're requested.
    bool m_hasPosition;

    wxDECLARE_DYNAMIC_CLASS(wxKeyEvent);
};

// Size event class
/*
 wxEVT_SIZE
 */

class wxSizeEvent : public wxEvent
{
public:
    wxSizeEvent() : wxEvent(0, wxEVT_SIZE)
        { }
    wxSizeEvent(const wxSize& sz, int winid = 0)
        : wxEvent(winid, wxEVT_SIZE),
          m_size(sz)
        { }
    wxSizeEvent(const wxSizeEvent& event) = default;
    wxSizeEvent(const wxRect& rect, int id = 0)
        : m_size(rect.GetSize()), m_rect(rect)
        { m_eventType = wxEVT_SIZING; m_id = id; }

	wxSizeEvent& operator=(const wxSizeEvent&) = delete;

    wxSize GetSize() const { return m_size; }
    void SetSize(wxSize size) { m_size = size; }
    wxRect GetRect() const { return m_rect; }
    void SetRect(const wxRect& rect) { m_rect = rect; }

    wxEvent *Clone() const override { return new wxSizeEvent(*this); }

private:
    wxRect m_rect; // Used for wxEVT_SIZING

    // For internal usage only. Will be converted to protected members.
    wxSize m_size;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Move event class

/*
 wxEVT_MOVE
 */

class wxMoveEvent : public wxEvent
{
public:
    wxMoveEvent()
        : wxEvent(0, wxEVT_MOVE)
        { }
    wxMoveEvent(const wxPoint& pos, int winid = 0)
        : wxEvent(winid, wxEVT_MOVE),
          m_pos(pos)
        { }
    wxMoveEvent(const wxMoveEvent& event)
        : wxEvent(event),
          m_pos(event.m_pos)
    { }
    wxMoveEvent(const wxRect& rect, int id = 0)
        : m_pos(rect.GetPosition()), m_rect(rect)
        { m_eventType = wxEVT_MOVING; m_id = id; }

	wxMoveEvent& operator=(const wxMoveEvent&) = delete;

    wxPoint GetPosition() const { return m_pos; }
    void SetPosition(const wxPoint& pos) { m_pos = pos; }
    wxRect GetRect() const { return m_rect; }
    void SetRect(const wxRect& rect) { m_rect = rect; }

    wxEvent *Clone() const override { return new wxMoveEvent(*this); }

protected:
    wxRect m_rect;

    wxPoint m_pos;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Paint event class
/*
 wxEVT_PAINT
 wxEVT_NC_PAINT
 */

class wxPaintEvent : public wxEvent
{
    // This ctor is only intended to be used by wxWidgets itself, so it's
    // intentionally declared as private when not building the library itself.
#ifdef WXBUILDING
public:
#endif // WXBUILDING
    explicit wxPaintEvent(wxWindowBase* window = nullptr);
	
    wxPaintEvent& operator=(const wxPaintEvent&) = delete;

    wxEvent *Clone() const override { return new wxPaintEvent(*this); }

	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

class wxNcPaintEvent : public wxEvent
{
    // This ctor is only intended to be used by wxWidgets itself, so it's
    // intentionally declared as private when not building the library itself.
#ifdef WXBUILDING
public:
#endif // WXBUILDING
    explicit wxNcPaintEvent(wxWindowBase* window = nullptr);

	wxNcPaintEvent& operator=(const wxNcPaintEvent&) = delete;

    wxEvent *Clone() const override { return new wxNcPaintEvent(*this); }

	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Erase background event class
/*
 wxEVT_ERASE_BACKGROUND
 */

class wxEraseEvent : public wxEvent
{
public:
    wxEraseEvent(int Id = 0, wxDC *dc = nullptr)
        : wxEvent(Id, wxEVT_ERASE_BACKGROUND),
          m_dc(dc)
        { }

    wxEraseEvent(const wxEraseEvent& event) = default;

	wxEraseEvent& operator=(const wxEraseEvent&) = delete;

    wxDC *GetDC() const { return m_dc; }

    wxEvent *Clone() const override { return new wxEraseEvent(*this); }

protected:
    wxDC *m_dc;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Focus event class
/*
 wxEVT_SET_FOCUS
 wxEVT_KILL_FOCUS
 */

class wxFocusEvent : public wxEvent
{
public:
    wxFocusEvent(wxEventType type = wxEVT_NULL, int winid = 0)
        : wxEvent(winid, type)
        { m_win = nullptr; }

    wxFocusEvent(const wxFocusEvent& event)
        : wxEvent(event)
        { m_win = event.m_win; }

	wxFocusEvent& operator=(const wxFocusEvent&) = delete;

    // The window associated with this event is the window which had focus
    // before for SET event and the window which will have focus for the KILL
    // one. NB: it may be NULL in both cases!
    wxWindow *GetWindow() const { return m_win; }
    void SetWindow(wxWindow *win) { m_win = win; }

    wxEvent *Clone() const override { return new wxFocusEvent(*this); }

private:
    wxWindow *m_win;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// wxChildFocusEvent notifies the parent that a child has got the focus: unlike
// wxFocusEvent it is propagated upwards the window chain
class wxChildFocusEvent : public wxCommandEvent
{
public:
    wxChildFocusEvent(wxWindow *win = nullptr);

	wxChildFocusEvent& operator=(const wxChildFocusEvent&) = delete;

    wxWindow *GetWindow() const { return (wxWindow *)GetEventObject(); }

    wxEvent *Clone() const override { return new wxChildFocusEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Activate event class
/*
 wxEVT_ACTIVATE
 wxEVT_ACTIVATE_APP
 wxEVT_HIBERNATE
 */

class wxActivateEvent : public wxEvent
{
public:
    // Type of activation. For now we can only detect if it was by mouse or by
    // some other method and even this is only available under wxMSW.
    enum class Reason
    {
        Mouse,
        Unknown
    };

    wxActivateEvent(wxEventType type = wxEVT_NULL, bool active = true,
                    int Id = 0, Reason activationReason = Reason::Unknown)
        : wxEvent(Id, type),
        m_activationReason(activationReason)
    {
        m_active = active;
    }
    wxActivateEvent(const wxActivateEvent& event)
        : wxEvent(event)
    {
        m_active = event.m_active;
        m_activationReason = event.m_activationReason;
    }

	wxActivateEvent& operator=(const wxActivateEvent&) = delete;

    bool GetActive() const { return m_active; }
    Reason GetActivationReason() const { return m_activationReason;}

    wxEvent *Clone() const override { return new wxActivateEvent(*this); }

private:
    Reason m_activationReason;

    bool m_active;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// InitDialog event class
/*
 wxEVT_INIT_DIALOG
 */

class wxInitDialogEvent : public wxEvent
{
public:
    wxInitDialogEvent(int Id = 0)
        : wxEvent(Id, wxEVT_INIT_DIALOG)
        { }

	wxInitDialogEvent& operator=(const wxInitDialogEvent&) = delete;

    wxEvent *Clone() const override { return new wxInitDialogEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Miscellaneous menu event class
/*
 wxEVT_MENU_OPEN,
 wxEVT_MENU_CLOSE,
 wxEVT_MENU_HIGHLIGHT,
*/

class wxMenuEvent : public wxEvent
{
public:
    wxMenuEvent(wxEventType type = wxEVT_NULL, int winid = 0, wxMenu* menu = nullptr)
        : wxEvent(winid, type)
        { m_menuId = winid; m_menu = menu; }
    wxMenuEvent(const wxMenuEvent& event)
        : wxEvent(event)
    { m_menuId = event.m_menuId; m_menu = event.m_menu; }

	wxMenuEvent& operator=(const wxMenuEvent&) = delete;

    // only for wxEVT_MENU_HIGHLIGHT
    int GetMenuId() const { return m_menuId; }

    // only for wxEVT_MENU_OPEN/CLOSE
    bool IsPopup() const { return m_menuId == wxID_ANY; }

    // only for wxEVT_MENU_OPEN/CLOSE
    wxMenu* GetMenu() const { return m_menu; }

    wxEvent *Clone() const override { return new wxMenuEvent(*this); }

private:
    wxMenu* m_menu;

    int     m_menuId;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Window close or session close event class
/*
 wxEVT_CLOSE_WINDOW,
 wxEVT_END_SESSION,
 wxEVT_QUERY_END_SESSION
 */

class wxCloseEvent : public wxEvent
{
public:
    wxCloseEvent(wxEventType type = wxEVT_NULL, int winid = 0)
        : wxEvent(winid, type)
          {}

    wxCloseEvent(const wxCloseEvent& event) = default;

	wxCloseEvent& operator=(const wxCloseEvent&) = delete;

    void SetLoggingOff(bool logOff) { m_loggingOff = logOff; }
    bool GetLoggingOff() const
    {
        // m_loggingOff flag is only used by wxEVT_[QUERY_]END_SESSION, it
        // doesn't make sense for wxEVT_CLOSE_WINDOW
        wxASSERT_MSG( m_eventType != wxEVT_CLOSE_WINDOW,
                      "this flag is for end session events only" );

        return m_loggingOff;
    }

    void Veto(bool veto = true)
    {
        // GetVeto() will return false anyhow...
        wxCHECK_RET( m_canVeto,
                     "call to Veto() ignored (can't veto this event)" );

        m_veto = veto;
    }
    void SetCanVeto(bool canVeto) { m_canVeto = canVeto; }
    bool CanVeto() const { return m_canVeto; }
    bool GetVeto() const { return m_canVeto && m_veto; }

    wxEvent *Clone() const override { return new wxCloseEvent(*this); }

protected:
    bool m_loggingOff{true},
         m_veto{false},
         m_canVeto{true};

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

/*
 wxEVT_SHOW
 */

class wxShowEvent : public wxEvent
{
public:
    wxShowEvent(int winid = 0, bool show = false)
        : wxEvent(winid, wxEVT_SHOW)
        { m_show = show; }
    wxShowEvent(const wxShowEvent& event)
        : wxEvent(event)
    { m_show = event.m_show; }

	wxShowEvent& operator=(const wxShowEvent&) = delete;

    void SetShow(bool show) { m_show = show; }

    // return true if the window was shown, false if hidden
    bool IsShown() const { return m_show; }

    wxEvent *Clone() const override { return new wxShowEvent(*this); }

protected:
    bool m_show;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

/*
 wxEVT_ICONIZE
 */

class wxIconizeEvent : public wxEvent
{
public:
    wxIconizeEvent(int winid = 0, bool iconized = true)
        : wxEvent(winid, wxEVT_ICONIZE)
        { m_iconized = iconized; }
    wxIconizeEvent(const wxIconizeEvent& event)
        : wxEvent(event)
    { m_iconized = event.m_iconized; }

	wxIconizeEvent& operator=(const wxIconizeEvent&) = delete;

    // return true if the frame was iconized, false if restored
    bool IsIconized() const { return m_iconized; }

    wxEvent *Clone() const override { return new wxIconizeEvent(*this); }

protected:
    bool m_iconized;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};
/*
 wxEVT_MAXIMIZE
 */

class wxMaximizeEvent : public wxEvent
{
public:
    wxMaximizeEvent(int winid = 0)
        : wxEvent(winid, wxEVT_MAXIMIZE)
        { }

	wxMaximizeEvent& operator=(const wxMaximizeEvent&) = delete;

    wxEvent *Clone() const override { return new wxMaximizeEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

/*
 wxEVT_FULLSCREEN
 */
class wxFullScreenEvent : public wxEvent
{
public:
    wxFullScreenEvent(int winid = 0, bool fullscreen = true)
        : wxEvent(winid, wxEVT_FULLSCREEN)
        { m_fullscreen = fullscreen; }
    wxFullScreenEvent(const wxFullScreenEvent& event)
        : wxEvent(event)
        { m_fullscreen = event.m_fullscreen; }

	wxFullScreenEvent& operator=(const wxFullScreenEvent&) = delete;

    bool IsFullScreen() const { return m_fullscreen; }

    wxEvent *Clone() const override { return new wxFullScreenEvent(*this); }

protected:
    bool m_fullscreen;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Joystick event class
/*
 wxEVT_JOY_BUTTON_DOWN,
 wxEVT_JOY_BUTTON_UP,
 wxEVT_JOY_MOVE,
 wxEVT_JOY_ZMOVE
*/

// Which joystick? Same as Windows ids so no conversion necessary.
enum
{
    wxJOYSTICK1,
    wxJOYSTICK2
};

// Which button is down?
enum
{
    wxJOY_BUTTON1    = 1,
    wxJOY_BUTTON2    = 2,
    wxJOY_BUTTON3    = 4,
    wxJOY_BUTTON4    = 8,
    wxJOY_BUTTON_ANY = wxJOY_BUTTON1 | wxJOY_BUTTON2 | wxJOY_BUTTON3 | wxJOY_BUTTON4
};

class wxJoystickEvent : public wxEvent
{
protected:
    wxPoint      m_pos;
    unsigned int m_buttonChange;   // Which button changed?

    int          m_zPosition{0};
    unsigned int m_buttonState;    // Which buttons are down?
    int          m_joyStick;       // Which joystick?

public:
    wxJoystickEvent(wxEventType type = wxEVT_NULL,
                    unsigned int state = 0,
                    int joystick = wxJOYSTICK1,
                    unsigned int change = 0)
        : wxEvent(0, type),
          
          
          m_buttonChange(change),
          m_buttonState(state),
          m_joyStick(joystick)
    {
    }

    wxJoystickEvent(const wxJoystickEvent& event) = default;

	wxJoystickEvent& operator=(const wxJoystickEvent&) = delete;

    wxPoint GetPosition() const { return m_pos; }
    int GetZPosition() const { return m_zPosition; }
    int GetButtonState() const { return m_buttonState; }
    unsigned int GetButtonChange() const { return m_buttonChange; }
    int GetButtonOrdinal() const { return std::countr_zero(m_buttonChange); }
    int GetJoystick() const { return m_joyStick; }

    void SetJoystick(int stick) { m_joyStick = stick; }
    void SetButtonState(int state) { m_buttonState = state; }
    void SetButtonChange(unsigned int change) { m_buttonChange = change; }
    void SetPosition(const wxPoint& pos) { m_pos = pos; }
    void SetZPosition(int zPos) { m_zPosition = zPos; }

    // Was it a button event? (*doesn't* mean: is any button *down*?)
    bool IsButton() const { return ((GetEventType() == wxEVT_JOY_BUTTON_DOWN) ||
            (GetEventType() == wxEVT_JOY_BUTTON_UP)); }

    // Was it a move event?
    bool IsMove() const { return (GetEventType() == wxEVT_JOY_MOVE); }

    // Was it a zmove event?
    bool IsZMove() const { return (GetEventType() == wxEVT_JOY_ZMOVE); }

    // Was it a down event from button 1, 2, 3, 4 or any?
    bool ButtonDown(unsigned int but = wxJOY_BUTTON_ANY) const
    { return ((GetEventType() == wxEVT_JOY_BUTTON_DOWN) &&
            ((but == wxJOY_BUTTON_ANY) || (but == m_buttonChange))); }

    // Was it a up event from button 1, 2, 3 or any?
    bool ButtonUp(unsigned int but = wxJOY_BUTTON_ANY) const
    { return ((GetEventType() == wxEVT_JOY_BUTTON_UP) &&
            ((but == wxJOY_BUTTON_ANY) || (but == m_buttonChange))); }

    // Was the given button 1,2,3,4 or any in Down state?
    bool ButtonIsDown(unsigned int but =  wxJOY_BUTTON_ANY) const
    { return (((but == wxJOY_BUTTON_ANY) && (m_buttonState != 0)) ||
            ((m_buttonState & but) == but)); }

    wxEvent *Clone() const override { return new wxJoystickEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Drop files event class
/*
 wxEVT_DROP_FILES
 */

class wxDropFilesEvent : public wxEvent
{
public:
    wxDropFilesEvent(wxEventType type = wxEVT_NULL,
                     int noFiles = 0,
                     std::vector<std::string> files = {})
        : wxEvent(0, type),
          m_noFiles(noFiles),
          m_files(files)
        { }

	wxDropFilesEvent& operator=(const wxDropFilesEvent&) = delete;

    wxPoint GetPosition() const { return m_pos; }
    int GetNumberOfFiles() const { return m_noFiles; }
    const std::vector<std::string>& GetFiles() const { return m_files; }

    wxEvent *Clone() const override { return new wxDropFilesEvent(*this); }

    std::vector<std::string> m_files;
    wxPoint   m_pos;
    int       m_noFiles;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Update UI event
/*
 wxEVT_UPDATE_UI
 */

// Whether to always send update events to windows, or
// to only send update events to those with the
// wxWS_EX_PROCESS_UI_UPDATES style.

enum class wxUpdateUIMode
{
        // Send UI update events to all windows
    All,

        // Send UI update events to windows that have
        // the wxWS_EX_PROCESS_UI_UPDATES flag specified
    Specified
};

class wxUpdateUIEvent : public wxCommandEvent
{
public:
    wxUpdateUIEvent(wxWindowID commandId = 0)
        : wxCommandEvent(wxEVT_UPDATE_UI, commandId)
    {
    }

    wxUpdateUIEvent(const wxUpdateUIEvent& event) = default;

	wxUpdateUIEvent& operator=(const wxUpdateUIEvent&) = delete;

    bool GetChecked() const { return m_checked; }
    bool GetEnabled() const { return m_enabled; }
    bool GetShown() const { return m_shown; }
    std::string GetText() const { return m_text; }
    bool GetSetText() const { return m_setText; }
    bool GetSetChecked() const { return m_setChecked; }
    bool GetSetEnabled() const { return m_setEnabled; }
    bool GetSetShown() const { return m_setShown; }

    void Check(bool check) { m_checked = check; m_setChecked = true; }
    void Enable(bool enable) { m_enabled = enable; m_setEnabled = true; }
    void Show(bool show) { m_shown = show; m_setShown = true; }
    void SetText(const std::string& text) { m_text = text; m_setText = true; }

    // A flag saying if the item can be checked. True by default.
    bool IsCheckable() const { return m_isCheckable; }
    void DisallowCheck() { m_isCheckable = false; }

    // Sets the interval between updates in milliseconds.
    // Set to -1 to disable updates, or to 0 to update as frequently as possible.
    static void SetUpdateInterval(long updateInterval) { sm_updateInterval = updateInterval; }

    // Returns the current interval between updates in milliseconds
    static long GetUpdateInterval() { return sm_updateInterval; }

    // Can we update this window?
    static bool CanUpdate(wxWindowBase *win);

    // Reset the update time to provide a delay until the next
    // time we should update
    static void ResetUpdateTime();

    // Specify how wxWidgets will send update events: to
    // all windows, or only to those which specify that they
    // will process the events.
    static void SetMode(wxUpdateUIMode mode) { sm_updateMode = mode; }

    // Returns the UI update mode
    static wxUpdateUIMode GetMode() { return sm_updateMode; }

    wxEvent *Clone() const override { return new wxUpdateUIEvent(*this); }

protected:
    std::string   m_text;

    bool          m_checked{false};
    bool          m_enabled{false};
    bool          m_shown{false};
    bool          m_setEnabled{false};
    bool          m_setShown{false};
    bool          m_setText{false};
    bool          m_setChecked{false};
    bool          m_isCheckable{true};

#if wxUSE_LONGLONG
    inline static wxLongLong       sm_lastUpdate{0};
#endif
    inline static long             sm_updateInterval{0};
    inline static wxUpdateUIMode   sm_updateMode{wxUpdateUIMode::All};

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

/*
 wxEVT_SYS_COLOUR_CHANGED
 */

// TODO: shouldn't all events record the window ID?
class wxSysColourChangedEvent : public wxEvent
{
public:
    wxSysColourChangedEvent()
        : wxEvent(0, wxEVT_SYS_COLOUR_CHANGED)
        { }

	wxSysColourChangedEvent& operator=(const wxSysColourChangedEvent&) = delete;

    wxEvent *Clone() const override { return new wxSysColourChangedEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

/*
 wxEVT_MOUSE_CAPTURE_CHANGED
 The window losing the capture receives this message
 (even if it released the capture itself).
 */

class wxMouseCaptureChangedEvent : public wxEvent
{
public:
    wxMouseCaptureChangedEvent(wxWindowID winid = 0, wxWindow* gainedCapture = nullptr)
        : wxEvent(winid, wxEVT_MOUSE_CAPTURE_CHANGED),
          m_gainedCapture(gainedCapture)
        { }

    wxMouseCaptureChangedEvent(const wxMouseCaptureChangedEvent& event) = default;

	wxMouseCaptureChangedEvent& operator=(const wxMouseCaptureChangedEvent&) = delete;

    wxEvent *Clone() const override { return new wxMouseCaptureChangedEvent(*this); }

    wxWindow* GetCapturedWindow() const { return m_gainedCapture; }

private:
    wxWindow* m_gainedCapture;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

/*
 wxEVT_MOUSE_CAPTURE_LOST
 The window losing the capture receives this message, unless it released it
 it itself or unless wxWindow::CaptureMouse was called on another window
 (and so capture will be restored when the new capturer releases it).
 */

class wxMouseCaptureLostEvent : public wxEvent
{
public:
    wxMouseCaptureLostEvent(wxWindowID winid = 0)
        : wxEvent(winid, wxEVT_MOUSE_CAPTURE_LOST)
    {}

    wxMouseCaptureLostEvent(const wxMouseCaptureLostEvent& event) = default;

	wxMouseCaptureLostEvent& operator=(const wxMouseCaptureLostEvent&) = delete;

    wxEvent *Clone() const override { return new wxMouseCaptureLostEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

/*
 wxEVT_DISPLAY_CHANGED
 */
class wxDisplayChangedEvent : public wxEvent
{
public:
    wxDisplayChangedEvent()
        : wxEvent(0, wxEVT_DISPLAY_CHANGED)
        { }

	wxDisplayChangedEvent& operator=(const wxDisplayChangedEvent&) = delete;

    wxEvent *Clone() const override { return new wxDisplayChangedEvent(*this); }

	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();

};

/*
 wxEVT_DPI_CHANGED
 */
class wxDPIChangedEvent : public wxEvent
{
public:
    explicit
    wxDPIChangedEvent(const wxSize& oldDPI = wxDefaultSize,
                      const wxSize& newDPI = wxDefaultSize)
        : wxEvent(0, wxEVT_DPI_CHANGED),
          m_oldDPI(oldDPI),
          m_newDPI(newDPI)
        { }

	wxDPIChangedEvent& operator=(const wxDPIChangedEvent&) = delete;

    wxSize GetOldDPI() const { return m_oldDPI; }
    wxSize GetNewDPI() const { return m_newDPI; }

    wxEvent *Clone() const override { return new wxDPIChangedEvent(*this); }

private:
    wxSize m_oldDPI;
    wxSize m_newDPI;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

/*
 wxEVT_PALETTE_CHANGED
 */

class wxPaletteChangedEvent : public wxEvent
{
public:
    wxPaletteChangedEvent(wxWindowID winid = 0)
        : wxEvent(winid, wxEVT_PALETTE_CHANGED)
          
        { }

    wxPaletteChangedEvent(const wxPaletteChangedEvent& event) = default;

	wxPaletteChangedEvent& operator=(const wxPaletteChangedEvent&) = delete;

    void SetChangedWindow(wxWindow* win) { m_changedWindow = win; }
    wxWindow* GetChangedWindow() const { return m_changedWindow; }

    wxEvent *Clone() const override { return new wxPaletteChangedEvent(*this); }

protected:
    wxWindow*     m_changedWindow{nullptr};

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

/*
 wxEVT_QUERY_NEW_PALETTE
 Indicates the window is getting keyboard focus and should re-do its palette.
 */

class wxQueryNewPaletteEvent : public wxEvent
{
public:
    wxQueryNewPaletteEvent(wxWindowID winid = 0)
        : wxEvent(winid, wxEVT_QUERY_NEW_PALETTE)
          
        { }
    wxQueryNewPaletteEvent(const wxQueryNewPaletteEvent& event) = default;

	wxQueryNewPaletteEvent& operator=(const wxQueryNewPaletteEvent&) = delete;

    // App sets this if it changes the palette.
    void SetPaletteRealized(bool realized) { m_paletteRealized = realized; }
    bool GetPaletteRealized() const { return m_paletteRealized; }

    wxEvent *Clone() const override { return new wxQueryNewPaletteEvent(*this); }

protected:
    bool m_paletteRealized{false};

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

/*
 Event generated by dialog navigation keys
 wxEVT_NAVIGATION_KEY
 */
// NB: don't derive from command event to avoid being propagated to the parent
class wxNavigationKeyEvent : public wxEvent
{
public:
    wxNavigationKeyEvent()
        : wxEvent(0, wxEVT_NAVIGATION_KEY),
          m_flags(IsForward | FromTab)  
    {
        m_propagationLevel = wxEVENT_PROPAGATE_NONE;
    }

    wxNavigationKeyEvent(const wxNavigationKeyEvent& event) = default;

	wxNavigationKeyEvent& operator=(const wxNavigationKeyEvent&) = delete;

    // direction: forward (true) or backward (false)
    bool GetDirection() const
        { return (m_flags & IsForward) != 0; }
    void SetDirection(bool bForward)
        { if ( bForward ) m_flags |= IsForward; else m_flags &= ~IsForward; }

    // it may be a window change event (MDI, notebook pages...) or a control
    // change event
    bool IsWindowChange() const
        { return (m_flags & WinChange) != 0; }
    void SetWindowChange(bool bIs)
        { if ( bIs ) m_flags |= WinChange; else m_flags &= ~WinChange; }

    // Set to true under MSW if the event was generated using the tab key.
    // This is required for proper navogation over radio buttons
    bool IsFromTab() const
        { return (m_flags & FromTab) != 0; }
    void SetFromTab(bool bIs)
        { if ( bIs ) m_flags |= FromTab; else m_flags &= ~FromTab; }

    // the child which has the focus currently (may be NULL - use
    // wxWindow::FindFocus then)
    wxWindow* GetCurrentFocus() const { return m_focus; }
    void SetCurrentFocus(wxWindow *win) { m_focus = win; }

    // Set flags
    void SetFlags(unsigned int flags) { m_flags = flags; }

    wxEvent *Clone() const override { return new wxNavigationKeyEvent(*this); }

    enum wxNavigationKeyEventFlags
    {
        IsBackward = 0x0000,
        IsForward = 0x0001,
        WinChange = 0x0002,
        FromTab = 0x0004
    };

    wxWindow *m_focus{nullptr};

    unsigned int m_flags;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// Window creation/destruction events: the first is sent as soon as window is
// created (i.e. the underlying GUI object exists), but when the C++ object is
// fully initialized (so virtual functions may be called). The second,
// wxEVT_DESTROY, is sent right before the window is destroyed - again, it's
// still safe to call virtual functions at this moment
/*
 wxEVT_CREATE
 wxEVT_DESTROY
 */

class wxWindowCreateEvent : public wxCommandEvent
{
public:
    wxWindowCreateEvent(wxWindow *win = nullptr);

	wxWindowCreateEvent& operator=(const wxWindowCreateEvent&) = delete;

    wxWindow *GetWindow() const { return (wxWindow *)GetEventObject(); }

    wxEvent *Clone() const override { return new wxWindowCreateEvent(*this); }

	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

class wxWindowDestroyEvent : public wxCommandEvent
{
public:
    wxWindowDestroyEvent(wxWindow *win = nullptr);

	wxWindowDestroyEvent& operator=(const wxWindowDestroyEvent&) = delete;

    wxWindow *GetWindow() const { return (wxWindow *)GetEventObject(); }

    wxEvent *Clone() const override { return new wxWindowDestroyEvent(*this); }

	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// A help event is sent when the user clicks on a window in context-help mode.
/*
 wxEVT_HELP
 wxEVT_DETAILED_HELP
*/

class wxHelpEvent : public wxCommandEvent
{
public:
    // how was this help event generated?
    enum class Origin
    {
        Unknown,    // unrecognized event source
        Keyboard,   // event generated from F1 key press
        HelpButton  // event from [?] button on the title bar (Windows)
    };

    wxHelpEvent(wxEventType type = wxEVT_NULL,
                wxWindowID winid = 0,
                const wxPoint& pt = wxDefaultPosition,
                Origin origin = Origin::Unknown)
        : wxCommandEvent(type, winid),
          m_pos(pt),
          m_origin(GuessOrigin(origin))
    { }

    wxHelpEvent(const wxHelpEvent& event) = default;

	wxHelpEvent& operator=(const wxHelpEvent&) = delete;

    // Position of event (in screen coordinates)
    const wxPoint& GetPosition() const { return m_pos; }
    void SetPosition(const wxPoint& pos) { m_pos = pos; }

    // Optional link to further help
    const std::string& GetLink() const { return m_link; }
    void SetLink(const std::string& link) { m_link = link; }

    // Optional target to display help in. E.g. a window specification
    const std::string& GetTarget() const { return m_target; }
    void SetTarget(const std::string& target) { m_target = target; }

    wxEvent *Clone() const override { return new wxHelpEvent(*this); }

    // optional indication of the event source
    Origin GetOrigin() const { return m_origin; }
    void SetOrigin(Origin origin) { m_origin = origin; }

protected:
    std::string  m_target;
    std::string  m_link;

    wxPoint   m_pos;
    Origin    m_origin;

    // we can try to guess the event origin ourselves, even if none is
    // specified in the ctor
    static Origin GuessOrigin(Origin origin);

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// A Clipboard Text event is sent when a window intercepts text copy/cut/paste
// message, i.e. the user has cut/copied/pasted data from/into a text control
// via ctrl-C/X/V, ctrl/shift-del/insert, a popup menu command, etc.
// NOTE : under windows these events are *NOT* generated automatically
// for a Rich Edit text control.
/*
wxEVT_TEXT_COPY
wxEVT_TEXT_CUT
wxEVT_TEXT_PASTE
*/

class wxClipboardTextEvent : public wxCommandEvent
{
public:
    wxClipboardTextEvent(wxEventType type = wxEVT_NULL,
                     wxWindowID winid = 0)
        : wxCommandEvent(type, winid)
    { }

    wxClipboardTextEvent(const wxClipboardTextEvent& event) = default;

	wxClipboardTextEvent& operator=(const wxClipboardTextEvent&) = delete;

    wxEvent *Clone() const override { return new wxClipboardTextEvent(*this); }

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// A Context event is sent when the user right clicks on a window or
// presses Shift-F10
// NOTE : Under windows this is a repackaged WM_CONTETXMENU message
//        Under other systems it may have to be generated from a right click event
/*
 wxEVT_CONTEXT_MENU
*/

class wxContextMenuEvent : public wxCommandEvent
{
public:
    wxContextMenuEvent(wxEventType type = wxEVT_NULL,
                       wxWindowID winid = 0,
                       const wxPoint& pt = wxDefaultPosition)
        : wxCommandEvent(type, winid),
          m_pos(pt)
    { }

    wxContextMenuEvent(const wxContextMenuEvent& event) = default;

	wxContextMenuEvent& operator=(const wxContextMenuEvent&) = delete;

    // Position of event (in screen coordinates)
    const wxPoint& GetPosition() const { return m_pos; }
    void SetPosition(const wxPoint& pos) { m_pos = pos; }

    wxEvent *Clone() const override { return new wxContextMenuEvent(*this); }

protected:
    wxPoint   m_pos;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};


/* TODO
 wxEVT_SETTING_CHANGED, // WM_WININICHANGE
// wxEVT_FONT_CHANGED,  // WM_FONTCHANGE: roll into wxEVT_SETTING_CHANGED, but remember to propagate
                        // wxEVT_FONT_CHANGED to all other windows (maybe).
 wxEVT_DRAW_ITEM, // Leave these three as virtual functions in wxControl?? Platform-specific.
 wxEVT_MEASURE_ITEM,
 wxEVT_COMPARE_ITEM
*/

#endif // wxUSE_GUI


// ============================================================================
// event handler and related classes
// ============================================================================


// struct containing the members common to static and dynamic event tables
// entries
struct wxEventTableEntryBase
{
    wxEventTableEntryBase(int winid, int idLast,
                          wxEventFunctor* fn, wxObject *data)
        : m_id(winid),
          m_lastId(idLast),
          m_fn(fn),
          m_callbackUserData(data)
    {
        wxASSERT_MSG( idLast == wxID_ANY || winid <= idLast,
                      "invalid IDs range: lower bound > upper bound" );
    }

    wxEventTableEntryBase( const wxEventTableEntryBase &entry )
        : m_id( entry.m_id ),
          m_lastId( entry.m_lastId ),
          m_fn( entry.m_fn ),
          m_callbackUserData( entry.m_callbackUserData )
    {
        // This is a 'hack' to ensure that only one instance tries to delete
        // the functor pointer. It is safe as long as the only place where the
        // copy constructor is being called is when the static event tables are
        // being initialized (a temporary instance is created and then this
        // constructor is called).

        const_cast<wxEventTableEntryBase&>( entry ).m_fn = nullptr;
    }

    ~wxEventTableEntryBase()
    {
        delete m_fn;
    }

    wxEventTableEntryBase& operator=(const wxEventTableEntryBase&) = delete;

    // function/method/functor to call
    wxEventFunctor* m_fn;

    // arbitrary user data associated with the callback
    wxObject* m_callbackUserData;

    // the range of ids for this entry: if m_lastId == wxID_ANY, the range
    // consists only of m_id, otherwise it is m_id..m_lastId inclusive
    int m_id,
        m_lastId;
};

// an entry from a static event table
struct wxEventTableEntry : public wxEventTableEntryBase
{
    wxEventTableEntry(const int& evType, int winid, int idLast,
                      wxEventFunctor* fn, wxObject *data)
        : wxEventTableEntryBase(winid, idLast, fn, data),
        m_eventType(evType)
    { }

    // the reference to event type: this allows us to not care about the
    // (undefined) order in which the event table entries and the event types
    // are initialized: initially the value of this reference might be
    // invalid, but by the time it is used for the first time, all global
    // objects will have been initialized (including the event type constants)
    // and so it will have the correct value when it is needed
    const int& m_eventType;
};

// an entry used in dynamic event table managed by wxEvtHandler::Connect()
struct wxDynamicEventTableEntry : public wxEventTableEntryBase
{
    wxDynamicEventTableEntry(int evType, int winid, int idLast,
                             wxEventFunctor* fn, wxObject *data)
        : wxEventTableEntryBase(winid, idLast, fn, data),
          m_eventType(evType)
    { }

    wxDynamicEventTableEntry& operator=(const wxDynamicEventTableEntry&) = delete;

    // not a reference here as we can't keep a reference to a temporary int
    // created to wrap the constant value typically passed to Connect() - nor
    // do we need it
    int m_eventType;
};

// ----------------------------------------------------------------------------
// wxEventTable: an array of event entries terminated with {0, 0, 0, 0, 0}
// ----------------------------------------------------------------------------

struct wxEventTable
{
    const wxEventTable *baseTable;    // base event table (next in chain)
    const wxEventTableEntry *entries; // bottom of entry array
};

// ----------------------------------------------------------------------------
// wxEventHashTable: a helper of wxEvtHandler to speed up wxEventTable lookups.
// ----------------------------------------------------------------------------

using wxEventTableEntryPointerArray = std::vector<const wxEventTableEntry*>;

class wxEventHashTable
{
private:
    // Internal data structs
    struct EventTypeTable
    {
        wxEventType                   eventType;
        wxEventTableEntryPointerArray eventEntryTable;
    };
    
    using EventTypeTablePointer = EventTypeTable *;

public:
    // Constructor, needs the event table it needs to hash later on.
    // Note: hashing of the event table is not done in the constructor as it
    //       can be that the event table is not yet full initialize, the hash
    //       will gets initialized when handling the first event look-up request.
    wxEventHashTable(const wxEventTable &table);
    ~wxEventHashTable();

   wxEventHashTable& operator=(wxEventHashTable&&) = delete;

    // Handle the given event, in other words search the event table hash
    // and call self->ProcessEvent() if a match was found.
    bool HandleEvent(wxEvent& event, wxEvtHandler *self);

    // Clear table
    void Clear();

#if wxUSE_MEMORY_TRACING
    // Clear all tables: only used to work around problems in memory tracing
    // code
    static void ClearAll();
#endif // wxUSE_MEMORY_TRACING

protected:
    // Init the hash table with the entries of the static event table.
    void InitHashTable();
    // Helper function of InitHashTable() to insert 1 entry into the hash table.
    void AddEntry(const wxEventTableEntry &entry);
    // Allocate and init with null pointers the base hash table.
    void AllocEventTypeTable(size_t size);
    // Grow the hash table in size and transfer all items currently
    // in the table to the correct location in the new table.
    void GrowEventTypeTable();

protected:
    EventTypeTablePointer* m_eventTypeTable;
    wxEventHashTable*      m_previous;
    wxEventHashTable*      m_next;

    const wxEventTable&    m_table;

    size_t                 m_size;

    bool                   m_rebuildHash;

    inline static wxEventHashTable* sm_first{nullptr};
};

// ----------------------------------------------------------------------------
// wxEvtHandler: the base class for all objects handling wxWidgets events
// ----------------------------------------------------------------------------

class wxEvtHandler : public wxObject
                                    , public wxTrackable
{
public:
    wxEvtHandler();
    ~wxEvtHandler();

    wxEvtHandler& operator=(wxEvtHandler&&) = delete;

    // Event handler chain
    // -------------------

    wxEvtHandler *GetNextHandler() const { return m_nextHandler; }
    wxEvtHandler *GetPreviousHandler() const { return m_previousHandler; }
    virtual void SetNextHandler(wxEvtHandler *handler) { m_nextHandler = handler; }
    virtual void SetPreviousHandler(wxEvtHandler *handler) { m_previousHandler = handler; }

    void SetEvtHandlerEnabled(bool enabled) { m_enabled = enabled; }
    bool GetEvtHandlerEnabled() const { return m_enabled; }

    void Unlink();
    bool IsUnlinked() const;


    // Global event filters
    // --------------------

    // Add an event filter whose FilterEvent() method will be called for each
    // and every event processed by wxWidgets. The filters are called in LIFO
    // order and wxApp is registered as an event filter by default. The pointer
    // must remain valid until it's removed with RemoveFilter() and is not
    // deleted by wxEvtHandler.
    static void AddFilter(wxEventFilter* filter);

    // Remove a filter previously installed with AddFilter().
    static void RemoveFilter(wxEventFilter* filter);


    // Event queuing and processing
    // ----------------------------

    // Process an event right now: this can only be called from the main
    // thread, use QueueEvent() for scheduling the events for
    // processing from other threads.
    [[maybe_unused]] virtual bool ProcessEvent(wxEvent& event);

    // Process an event by calling ProcessEvent and handling any exceptions
    // thrown by event handlers. It's mostly useful when processing wx events
    // when called from C code (e.g. in GTK+ callback) when the exception
    // wouldn't correctly propagate to wxEventLoop.
    bool SafelyProcessEvent(wxEvent& event);
        // NOTE: uses ProcessEvent()

    // This method tries to process the event in this event handler, including
    // any preprocessing done by TryBefore() and all the handlers chained to
    // it, but excluding the post-processing done in TryAfter().
    //
    // It is meant to be called from ProcessEvent() only and is not virtual,
    // additional event handlers can be hooked into the normal event processing
    // logic using TryBefore() and TryAfter() hooks.
    //
    // You can also call it yourself to forward an event to another handler but
    // without propagating it upwards if it's unhandled (this is usually
    // unwanted when forwarding as the original handler would already do it if
    // needed normally).
    bool ProcessEventLocally(wxEvent& event);

    // Schedule the given event to be processed later. It takes ownership of
    // the event pointer, i.e. it will be deleted later. This is safe to call
    // from multiple threads although you still need to ensure that std::string
    // fields of the event object are deep copies and not use the same string
    // buffer as other std::string objects in this thread.
    virtual void QueueEvent(wxEvent *event);

    // Add an event to be processed later: notice that this function is not
    // safe to call from threads other than main, use QueueEvent()
    virtual void AddPendingEvent(const wxEvent& event)
    {
        // notice that the thread-safety problem comes from the fact that
        // Clone() doesn't make deep copies of std::string fields of wxEvent
        // object and so the same std::string could be used from both threads when
        // the event object is destroyed in this one -- QueueEvent() avoids
        // this problem as the event pointer is not used any more in this
        // thread at all after it is called.
        QueueEvent(event.Clone());
    }

    void ProcessPendingEvents();
        // NOTE: uses ProcessEvent()

    void DeletePendingEvents();

#if wxUSE_THREADS
    bool ProcessThreadEvent(const wxEvent& event);
        // NOTE: uses AddPendingEvent(); call only from secondary threads
#endif

#if wxUSE_EXCEPTIONS
    // This is a private function which handles any exceptions arising during
    // the execution of user-defined code called in the event loop context by
    // forwarding them to wxApp::OnExceptionInMainLoop() and, if it rethrows,
    // to wxApp::OnUnhandledException(). In any case this function ensures that
    // no exceptions ever escape from it and so is useful to call at module
    // boundary.
    //
    // It must be only called when handling an active exception.
    static void WXConsumeException();
#endif // wxUSE_EXCEPTIONS

    // Asynchronous method calls: these methods schedule the given method
    // pointer for a later call (during the next idle event loop iteration).
    //
    // Notice that the method is called on this object itself, so the object
    // CallAfter() is called on must have the correct dynamic type.
    //
    // These method can be used from another thread.

    template <typename T>
    void CallAfter(void (T::*method)())
    {
        QueueEvent(
            new wxAsyncMethodCallEvent0<T>(static_cast<T*>(this), method)
        );
    }

    // Notice that we use P1 and not T1 for the parameter to allow passing
    // parameters that are convertible to the type taken by the method
    // instead of being exactly the same, to be closer to the usual method call
    // semantics.
    template <typename T, typename T1, typename P1>
    void CallAfter(void (T::*method)(T1 x1), P1 x1)
    {
        QueueEvent(
            new wxAsyncMethodCallEvent1<T, T1>(
                static_cast<T*>(this), method, x1)
        );
    }

    template <typename T, typename T1, typename T2, typename P1, typename P2>
    void CallAfter(void (T::*method)(T1 x1, T2 x2), P1 x1, P2 x2)
    {
        QueueEvent(
            new wxAsyncMethodCallEvent2<T, T1, T2>(
                static_cast<T*>(this), method, x1, x2)
        );
    }

    template <typename T>
    void CallAfter(const T& fn)
    {
        QueueEvent(new wxAsyncMethodCallEventFunctor<T>(this, fn));
    }


    // Connecting and disconnecting
    // ----------------------------

    // These functions are used for old, untyped, event handlers and don't
    // check that the type of the function passed to them actually matches the
    // type of the event. They also only allow connecting events to methods of
    // wxEvtHandler-derived classes.
    //
    // The template Connect() methods below are safer and allow connecting
    // events to arbitrary functions or functors -- but require compiler
    // support for templates.

    // Dynamic association of a member function handler with the event handler,
    // winid and event type
    void Connect(int winid,
                 int lastId,
                 wxEventType eventType,
                 wxObjectEventFunction func,
                 wxObject *userData = nullptr,
                 wxEvtHandler *eventSink = nullptr)
    {
        DoBind(winid, lastId, eventType,
                  wxNewEventFunctor(eventType, func, eventSink),
                  userData);
    }

    // Convenience function: take just one id
    void Connect(int winid,
                 wxEventType eventType,
                 wxObjectEventFunction func,
                 wxObject *userData = nullptr,
                 wxEvtHandler *eventSink = nullptr)
        { Connect(winid, wxID_ANY, eventType, func, userData, eventSink); }

    // Even more convenient: without id (same as using id of wxID_ANY)
    void Connect(wxEventType eventType,
                 wxObjectEventFunction func,
                 wxObject *userData = nullptr,
                 wxEvtHandler *eventSink = nullptr)
        { Connect(wxID_ANY, wxID_ANY, eventType, func, userData, eventSink); }

    bool Disconnect(int winid,
                    int lastId,
                    wxEventType eventType,
                    wxObjectEventFunction func = nullptr,
                    wxObject *userData = nullptr,
                    wxEvtHandler *eventSink = nullptr)
    {
        return DoUnbind(winid, lastId, eventType,
                            wxMakeEventFunctor(eventType, func, eventSink),
                            userData );
    }

    bool Disconnect(int winid = wxID_ANY,
                    wxEventType eventType = wxEVT_NULL,
                    wxObjectEventFunction func = nullptr,
                    wxObject *userData = nullptr,
                    wxEvtHandler *eventSink = nullptr)
        { return Disconnect(winid, wxID_ANY, eventType, func, userData, eventSink); }

    bool Disconnect(wxEventType eventType,
                    wxObjectEventFunction func,
                    wxObject *userData = nullptr,
                    wxEvtHandler *eventSink = nullptr)
        { return Disconnect(wxID_ANY, eventType, func, userData, eventSink); }

    // Bind functions to an event:
    template <typename EventTag, typename EventArg>
    void Bind(const EventTag& eventType,
              void (*function)(EventArg &),
              int winid = wxID_ANY,
              int lastId = wxID_ANY,
              wxObject *userData = nullptr)
    {
        DoBind(winid, lastId, eventType,
                  wxNewEventFunctor(eventType, function),
                  userData);
    }


    template <typename EventTag, typename EventArg>
    bool Unbind(const EventTag& eventType,
                void (*function)(EventArg &),
                int winid = wxID_ANY,
                int lastId = wxID_ANY,
                wxObject *userData = nullptr)
    {
        return DoUnbind(winid, lastId, eventType,
                            wxMakeEventFunctor(eventType, function),
                            userData);
    }

    // Bind functors to an event:
    template <typename EventTag, typename Functor>
    void Bind(const EventTag& eventType,
              const Functor &functor,
              int winid = wxID_ANY,
              int lastId = wxID_ANY,
              wxObject *userData = nullptr)
    {
        DoBind(winid, lastId, eventType,
                  wxNewEventFunctor(eventType, functor),
                  userData);
    }


    template <typename EventTag, typename Functor>
    bool Unbind(const EventTag& eventType,
                const Functor &functor,
                int winid = wxID_ANY,
                int lastId = wxID_ANY,
                wxObject *userData = nullptr)
    {
        return DoUnbind(winid, lastId, eventType,
                            wxMakeEventFunctor(eventType, functor),
                            userData);
    }


    // Bind a method of a class (called on the specified handler which must
    // be convertible to this class) object to an event:

    template <typename EventTag, typename Class, typename EventArg, typename EventHandler>
    void Bind(const EventTag &eventType,
              void (Class::*method)(EventArg &),
              EventHandler *handler,
              int winid = wxID_ANY,
              int lastId = wxID_ANY,
              wxObject *userData = nullptr)
    {
        DoBind(winid, lastId, eventType,
                  wxNewEventFunctor(eventType, method, handler),
                  userData);
    }

    template <typename EventTag, typename Class, typename EventArg, typename EventHandler>
    bool Unbind(const EventTag &eventType,
                void (Class::*method)(EventArg&),
                EventHandler *handler,
                int winid = wxID_ANY,
                int lastId = wxID_ANY,
                wxObject *userData = nullptr )
    {
        return DoUnbind(winid, lastId, eventType,
                            wxMakeEventFunctor(eventType, method, handler),
                            userData);
    }

    // User data can be associated with each wxEvtHandler
    void SetClientObject( wxClientData *data ) { DoSetClientObject(data); }
    wxClientData *GetClientObject() const { return DoGetClientObject(); }

    void SetClientData( void *data ) { DoSetClientData(data); }
    void *GetClientData() const { return DoGetClientData(); }


    // implementation from now on
    // --------------------------

    // check if the given event table entry matches this event by id (the check
    // for the event type should be done by caller) and call the handler if it
    // does
    //
    // return true if the event was processed, false otherwise (no match or the
    // handler decided to skip the event)
    static bool ProcessEventIfMatchesId(const wxEventTableEntryBase& tableEntry,
                                        wxEvtHandler *handler,
                                        wxEvent& event);

    // Allow iterating over all connected dynamic event handlers: you must pass
    // the same "cookie" to GetFirst() and GetNext() and call them until null
    // is returned.
    //
    // These functions are for internal use only.
    wxDynamicEventTableEntry* GetFirstDynamicEntry(size_t& cookie) const;
    wxDynamicEventTableEntry* GetNextDynamicEntry(size_t& cookie) const;

    virtual bool SearchEventTable(wxEventTable& table, wxEvent& event);
    bool SearchDynamicEventTable( wxEvent& event );

    // Avoid problems at exit by cleaning up static hash table gracefully
    void ClearEventHashTable() { GetEventHashTable().Clear(); }
    void OnSinkDestroyed( wxEvtHandler *sink );


private:
    void DoBind(int winid,
                   int lastId,
                   wxEventType eventType,
                   wxEventFunctor *func,
                   wxObject* userData = nullptr);

    bool DoUnbind(int winid,
                      int lastId,
                      wxEventType eventType,
                      const wxEventFunctor& func,
                      wxObject *userData = nullptr);

    static const wxEventTableEntry sm_eventTableEntries[];

protected:
    // hooks for wxWindow used by ProcessEvent()
    // -----------------------------------------

    // this one is called before trying our own event table to allow plugging
    // in the event handlers overriding the default logic, this is used by e.g.
    // validators.
    virtual bool TryBefore(wxEvent& event);

    // This one is not a hook but just a helper which looks up the handler in
    // this object itself.
    //
    // It is called from ProcessEventLocally() and normally shouldn't be called
    // directly as doing it would ignore any chained event handlers
    bool TryHereOnly(wxEvent& event);

    // Another helper which simply calls pre-processing hook and then tries to
    // handle the event at this handler level.
    bool TryBeforeAndHere(wxEvent& event)
    {
        return TryBefore(event) || TryHereOnly(event);
    }

    // this one is called after failing to find the event handle in our own
    // table to give a chance to the other windows to process it
    //
    // base class implementation passes the event to wxTheApp
    virtual bool TryAfter(wxEvent& event);

    // Overriding this method allows filtering the event handlers dynamically
    // connected to this object. If this method returns false, the handler is
    // not connected at all. If it returns true, it is connected using the
    // possibly modified fields of the given entry.
    virtual bool OnDynamicBind([[maybe_unused]] wxDynamicEventTableEntry& entry)
    {
        return true;
    }


    static const wxEventTable sm_eventTable;
    virtual const wxEventTable *GetEventTable() const;

    static wxEventHashTable   sm_eventHashTable;
    virtual wxEventHashTable& GetEventHashTable() const;

#if wxUSE_THREADS
    // critical section protecting m_pendingEvents
    wxCriticalSection m_pendingEventsLock;
#endif // wxUSE_THREADS

    wxEvtHandler*       m_nextHandler{nullptr};
    wxEvtHandler*       m_previousHandler{nullptr};

    using DynamicEvents = std::vector<wxDynamicEventTableEntry *>;
    DynamicEvents* m_dynamicEvents{nullptr};

    wxList*             m_pendingEvents{nullptr};

    // The user data: either an object which will be deleted by the container
    // when it's deleted or some raw pointer which we do nothing with - only
    // one type of data can be used with the given window (i.e. you cannot set
    // the void data and then associate the container with wxClientData or vice
    // versa)
    union
    {
        wxClientData *m_clientObject;
        void         *m_clientData;
    };

    // what kind of data do we have?
    wxClientDataType m_clientDataType{wxClientDataType::None};

    // Is event handler enabled?
    bool                m_enabled{true};

    // client data accessors
    virtual void DoSetClientObject( wxClientData *data );
    virtual wxClientData *DoGetClientObject() const;

    virtual void DoSetClientData( void *data );
    virtual void *DoGetClientData() const;

    // Search tracker objects for event connection with this sink
    wxEventConnectionRef *FindRefInTrackerList(wxEvtHandler *handler);

private:
    // pass the event to wxTheApp instance, called from TryAfter()
    bool DoTryApp(wxEvent& event);

    // try to process events in all handlers chained to this one
    bool DoTryChain(wxEvent& event);

    // Head of the event filter linked list.
    static wxEventFilter* ms_filterList;
};

using wxEvtHandlerArray = std::vector<wxEvtHandler*>;


// Define an inline method of wxObjectEventFunctor which couldn't be defined
// before wxEvtHandler declaration: at least Sun CC refuses to compile function
// calls through pointer to member for forward-declared classes (see #12452).
inline void wxObjectEventFunctor::operator()(wxEvtHandler *handler, wxEvent& event)
{
    wxEvtHandler * const realHandler = m_handler ? m_handler : handler;

    (realHandler->*m_method)(event);
}

// ----------------------------------------------------------------------------
// wxEventConnectionRef represents all connections between two event handlers
// and enables automatic disconnect when an event handler sink goes out of
// scope. Each connection/disconnect increases/decreases ref count, and
// when it reaches zero the node goes out of scope.
// ----------------------------------------------------------------------------

class wxEventConnectionRef : public wxTrackerNode
{
public:
    wxEventConnectionRef()  = default;
    wxEventConnectionRef(wxEvtHandler *src, wxEvtHandler *sink)
        : m_src(src), m_sink(sink), m_refCount(1)
    {
        m_sink->AddNode(this);
    }

    wxEventConnectionRef& operator=(const wxEventConnectionRef&) = delete;

    // The sink is being destroyed
    void OnObjectDestroy( ) override
    {
        if ( m_src )
            m_src->OnSinkDestroyed( m_sink );
        delete this;
    }

    wxEventConnectionRef *ToEventConnection() override { return this; }

    void IncRef() { m_refCount++; }
    void DecRef()
    {
        if ( !--m_refCount )
        {
            // The sink holds the only external pointer to this object
            if ( m_sink )
                m_sink->RemoveNode(this);
            delete this;
        }
    }

private:
    wxEvtHandler *m_src{nullptr},
                 *m_sink{nullptr};
    int m_refCount{0};

    friend class wxEvtHandler;
};

// Post a message to the given event handler which will be processed during the
// next event loop iteration.
//
// Notice that this one is not thread-safe, use wxQueueEvent()
inline void wxPostEvent(wxEvtHandler *dest, const wxEvent& event)
{
    wxCHECK_RET( dest, "need an object to post event to" );

    dest->AddPendingEvent(event);
}

// Wrapper around wxEvtHandler::QueueEvent(): adds an event for later
// processing, unlike wxPostEvent it is safe to use from different thread even
// for events with std::string members
inline void wxQueueEvent(wxEvtHandler *dest, wxEvent *event)
{
    wxCHECK_RET( dest, "need an object to queue event for" );

    dest->QueueEvent(event);
}

typedef void (wxEvtHandler::*wxEventFunction)(wxEvent&);
typedef void (wxEvtHandler::*wxIdleEventFunction)(wxIdleEvent&);
typedef void (wxEvtHandler::*wxThreadEventFunction)(wxThreadEvent&);

#define wxEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxEventFunction, func)
#define wxIdleEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxIdleEventFunction, func)
#define wxThreadEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxThreadEventFunction, func)

#if wxUSE_GUI

// ----------------------------------------------------------------------------
// wxEventBlocker: helper class to temporarily disable event handling for a window
// ----------------------------------------------------------------------------

class wxEventBlocker : public wxEvtHandler
{
public:
    wxEventBlocker(wxWindow *win, wxEventType type = wxEVT_ANY);
    ~wxEventBlocker();

   wxEventBlocker& operator=(wxEventBlocker&&) = delete;

    void Block(wxEventType type)
    {
        m_eventsToBlock.push_back(static_cast<int>(type));
    }

    bool ProcessEvent(wxEvent& event) override;

protected:
    std::vector<int> m_eventsToBlock;
    wxWindow *m_window;
};

typedef void (wxEvtHandler::*wxCommandEventFunction)(wxCommandEvent&);
typedef void (wxEvtHandler::*wxScrollEventFunction)(wxScrollEvent&);
typedef void (wxEvtHandler::*wxScrollWinEventFunction)(wxScrollWinEvent&);
typedef void (wxEvtHandler::*wxSizeEventFunction)(wxSizeEvent&);
typedef void (wxEvtHandler::*wxMoveEventFunction)(wxMoveEvent&);
typedef void (wxEvtHandler::*wxPaintEventFunction)(wxPaintEvent&);
typedef void (wxEvtHandler::*wxNcPaintEventFunction)(wxNcPaintEvent&);
typedef void (wxEvtHandler::*wxEraseEventFunction)(wxEraseEvent&);
typedef void (wxEvtHandler::*wxMouseEventFunction)(wxMouseEvent&);
typedef void (wxEvtHandler::*wxCharEventFunction)(wxKeyEvent&);
typedef void (wxEvtHandler::*wxFocusEventFunction)(wxFocusEvent&);
typedef void (wxEvtHandler::*wxChildFocusEventFunction)(wxChildFocusEvent&);
typedef void (wxEvtHandler::*wxActivateEventFunction)(wxActivateEvent&);
typedef void (wxEvtHandler::*wxMenuEventFunction)(wxMenuEvent&);
typedef void (wxEvtHandler::*wxJoystickEventFunction)(wxJoystickEvent&);
typedef void (wxEvtHandler::*wxDropFilesEventFunction)(wxDropFilesEvent&);
typedef void (wxEvtHandler::*wxInitDialogEventFunction)(wxInitDialogEvent&);
typedef void (wxEvtHandler::*wxSysColourChangedEventFunction)(wxSysColourChangedEvent&);
typedef void (wxEvtHandler::*wxDisplayChangedEventFunction)(wxDisplayChangedEvent&);
typedef void (wxEvtHandler::*wxDPIChangedEventFunction)(wxDPIChangedEvent&);
typedef void (wxEvtHandler::*wxUpdateUIEventFunction)(wxUpdateUIEvent&);
typedef void (wxEvtHandler::*wxCloseEventFunction)(wxCloseEvent&);
typedef void (wxEvtHandler::*wxShowEventFunction)(wxShowEvent&);
typedef void (wxEvtHandler::*wxIconizeEventFunction)(wxIconizeEvent&);
typedef void (wxEvtHandler::*wxMaximizeEventFunction)(wxMaximizeEvent&);
typedef void (wxEvtHandler::*wxNavigationKeyEventFunction)(wxNavigationKeyEvent&);
typedef void (wxEvtHandler::*wxPaletteChangedEventFunction)(wxPaletteChangedEvent&);
typedef void (wxEvtHandler::*wxQueryNewPaletteEventFunction)(wxQueryNewPaletteEvent&);
typedef void (wxEvtHandler::*wxWindowCreateEventFunction)(wxWindowCreateEvent&);
typedef void (wxEvtHandler::*wxWindowDestroyEventFunction)(wxWindowDestroyEvent&);
typedef void (wxEvtHandler::*wxSetCursorEventFunction)(wxSetCursorEvent&);
typedef void (wxEvtHandler::*wxNotifyEventFunction)(wxNotifyEvent&);
typedef void (wxEvtHandler::*wxHelpEventFunction)(wxHelpEvent&);
typedef void (wxEvtHandler::*wxContextMenuEventFunction)(wxContextMenuEvent&);
typedef void (wxEvtHandler::*wxMouseCaptureChangedEventFunction)(wxMouseCaptureChangedEvent&);
typedef void (wxEvtHandler::*wxMouseCaptureLostEventFunction)(wxMouseCaptureLostEvent&);
typedef void (wxEvtHandler::*wxClipboardTextEventFunction)(wxClipboardTextEvent&);
typedef void (wxEvtHandler::*wxPanGestureEventFunction)(wxPanGestureEvent&);
typedef void (wxEvtHandler::*wxZoomGestureEventFunction)(wxZoomGestureEvent&);
typedef void (wxEvtHandler::*wxRotateGestureEventFunction)(wxRotateGestureEvent&);
typedef void (wxEvtHandler::*wxTwoFingerTapEventFunction)(wxTwoFingerTapEvent&);
typedef void (wxEvtHandler::*wxLongPressEventFunction)(wxLongPressEvent&);
typedef void (wxEvtHandler::*wxPressAndTapEventFunction)(wxPressAndTapEvent&);

#define wxCommandEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxCommandEventFunction, func)
#define wxScrollEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxScrollEventFunction, func)
#define wxScrollWinEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxScrollWinEventFunction, func)
#define wxSizeEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxSizeEventFunction, func)
#define wxMoveEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMoveEventFunction, func)
#define wxPaintEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxPaintEventFunction, func)
#define wxNcPaintEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxNcPaintEventFunction, func)
#define wxEraseEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxEraseEventFunction, func)
#define wxMouseEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMouseEventFunction, func)
#define wxCharEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxCharEventFunction, func)
#define wxKeyEventHandler(func) wxCharEventHandler(func)
#define wxFocusEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxFocusEventFunction, func)
#define wxChildFocusEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxChildFocusEventFunction, func)
#define wxActivateEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxActivateEventFunction, func)
#define wxMenuEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMenuEventFunction, func)
#define wxJoystickEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxJoystickEventFunction, func)
#define wxDropFilesEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxDropFilesEventFunction, func)
#define wxInitDialogEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxInitDialogEventFunction, func)
#define wxSysColourChangedEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxSysColourChangedEventFunction, func)
#define wxDisplayChangedEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxDisplayChangedEventFunction, func)
#define wxDPIChangedEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxDPIChangedEventFunction, func)
#define wxUpdateUIEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxUpdateUIEventFunction, func)
#define wxCloseEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxCloseEventFunction, func)
#define wxShowEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxShowEventFunction, func)
#define wxIconizeEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxIconizeEventFunction, func)
#define wxMaximizeEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMaximizeEventFunction, func)
#define wxNavigationKeyEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxNavigationKeyEventFunction, func)
#define wxPaletteChangedEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxPaletteChangedEventFunction, func)
#define wxQueryNewPaletteEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxQueryNewPaletteEventFunction, func)
#define wxWindowCreateEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxWindowCreateEventFunction, func)
#define wxWindowDestroyEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxWindowDestroyEventFunction, func)
#define wxSetCursorEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxSetCursorEventFunction, func)
#define wxNotifyEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxNotifyEventFunction, func)
#define wxHelpEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxHelpEventFunction, func)
#define wxContextMenuEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxContextMenuEventFunction, func)
#define wxMouseCaptureChangedEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMouseCaptureChangedEventFunction, func)
#define wxMouseCaptureLostEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMouseCaptureLostEventFunction, func)
#define wxClipboardTextEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxClipboardTextEventFunction, func)
#define wxPanGestureEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxPanGestureEventFunction, func)
#define wxZoomGestureEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxZoomGestureEventFunction, func)
#define wxRotateGestureEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxRotateGestureEventFunction, func)
#define wxTwoFingerTapEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxTwoFingerTapEventFunction, func)
#define wxLongPressEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxLongPressEventFunction, func)
#define wxPressAndTapEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxPressAndTapEventFunction, func)

#endif // wxUSE_GUI

// N.B. In GNU-WIN32, you *have* to take the address of a member function
// (use &) or the compiler crashes...

#define wxDECLARE_EVENT_TABLE()                                         \
    private:                                                            \
        static const wxEventTableEntry sm_eventTableEntries[];          \
    protected:                                                          \
        const wxEventTable* GetEventTable() const override;     \
        wxEventHashTable& GetEventHashTable() const override;   \
        static const wxEventTable        sm_eventTable;                 \
        static wxEventHashTable          sm_eventHashTable

#define wxBEGIN_EVENT_TABLE(theClass, baseClass) \
    const wxEventTable theClass::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass::sm_eventTableEntries[0] }; \
    const wxEventTable *theClass::GetEventTable() const \
        { return &theClass::sm_eventTable; } \
    wxEventHashTable theClass::sm_eventHashTable(theClass::sm_eventTable); \
    wxEventHashTable &theClass::GetEventHashTable() const \
        { return theClass::sm_eventHashTable; } \
    const wxEventTableEntry theClass::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE1(theClass, baseClass, T1) \
    template<typename T1> \
    const wxEventTable theClass<T1>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1>::sm_eventTableEntries[0] }; \
    template<typename T1> \
    const wxEventTable *theClass<T1>::GetEventTable() const \
        { return &theClass<T1>::sm_eventTable; } \
    template<typename T1> \
    wxEventHashTable theClass<T1>::sm_eventHashTable(theClass<T1>::sm_eventTable); \
    template<typename T1> \
    wxEventHashTable &theClass<T1>::GetEventHashTable() const \
        { return theClass<T1>::sm_eventHashTable; } \
    template<typename T1> \
    const wxEventTableEntry theClass<T1>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE2(theClass, baseClass, T1, T2) \
    template<typename T1, typename T2> \
    const wxEventTable theClass<T1, T2>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2> \
    const wxEventTable *theClass<T1, T2>::GetEventTable() const \
        { return &theClass<T1, T2>::sm_eventTable; } \
    template<typename T1, typename T2> \
    wxEventHashTable theClass<T1, T2>::sm_eventHashTable(theClass<T1, T2>::sm_eventTable); \
    template<typename T1, typename T2> \
    wxEventHashTable &theClass<T1, T2>::GetEventHashTable() const \
        { return theClass<T1, T2>::sm_eventHashTable; } \
    template<typename T1, typename T2> \
    const wxEventTableEntry theClass<T1, T2>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE3(theClass, baseClass, T1, T2, T3) \
    template<typename T1, typename T2, typename T3> \
    const wxEventTable theClass<T1, T2, T3>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2, T3>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3> \
    const wxEventTable *theClass<T1, T2, T3>::GetEventTable() const \
        { return &theClass<T1, T2, T3>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3> \
    wxEventHashTable theClass<T1, T2, T3>::sm_eventHashTable(theClass<T1, T2, T3>::sm_eventTable); \
    template<typename T1, typename T2, typename T3> \
    wxEventHashTable &theClass<T1, T2, T3>::GetEventHashTable() const \
        { return theClass<T1, T2, T3>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3> \
    const wxEventTableEntry theClass<T1, T2, T3>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE4(theClass, baseClass, T1, T2, T3, T4) \
    template<typename T1, typename T2, typename T3, typename T4> \
    const wxEventTable theClass<T1, T2, T3, T4>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2, T3, T4>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3, typename T4> \
    const wxEventTable *theClass<T1, T2, T3, T4>::GetEventTable() const \
        { return &theClass<T1, T2, T3, T4>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3, typename T4> \
    wxEventHashTable theClass<T1, T2, T3, T4>::sm_eventHashTable(theClass<T1, T2, T3, T4>::sm_eventTable); \
    template<typename T1, typename T2, typename T3, typename T4> \
    wxEventHashTable &theClass<T1, T2, T3, T4>::GetEventHashTable() const \
        { return theClass<T1, T2, T3, T4>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3, typename T4> \
    const wxEventTableEntry theClass<T1, T2, T3, T4>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE5(theClass, baseClass, T1, T2, T3, T4, T5) \
    template<typename T1, typename T2, typename T3, typename T4, typename T5> \
    const wxEventTable theClass<T1, T2, T3, T4, T5>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2, T3, T4, T5>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3, typename T4, typename T5> \
    const wxEventTable *theClass<T1, T2, T3, T4, T5>::GetEventTable() const \
        { return &theClass<T1, T2, T3, T4, T5>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5> \
    wxEventHashTable theClass<T1, T2, T3, T4, T5>::sm_eventHashTable(theClass<T1, T2, T3, T4, T5>::sm_eventTable); \
    template<typename T1, typename T2, typename T3, typename T4, typename T5> \
    wxEventHashTable &theClass<T1, T2, T3, T4, T5>::GetEventHashTable() const \
        { return theClass<T1, T2, T3, T4, T5>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5> \
    const wxEventTableEntry theClass<T1, T2, T3, T4, T5>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE7(theClass, baseClass, T1, T2, T3, T4, T5, T6, T7) \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> \
    const wxEventTable theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> \
    const wxEventTable *theClass<T1, T2, T3, T4, T5, T6, T7>::GetEventTable() const \
        { return &theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> \
    wxEventHashTable theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventHashTable(theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventTable); \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> \
    wxEventHashTable &theClass<T1, T2, T3, T4, T5, T6, T7>::GetEventHashTable() const \
        { return theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> \
    const wxEventTableEntry theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE8(theClass, baseClass, T1, T2, T3, T4, T5, T6, T7, T8) \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> \
    const wxEventTable theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> \
    const wxEventTable *theClass<T1, T2, T3, T4, T5, T6, T7, T8>::GetEventTable() const \
        { return &theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> \
    wxEventHashTable theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventHashTable(theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventTable); \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> \
    wxEventHashTable &theClass<T1, T2, T3, T4, T5, T6, T7, T8>::GetEventHashTable() const \
        { return theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> \
    const wxEventTableEntry theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventTableEntries[] = { \

#define wxEND_EVENT_TABLE() \
    wxDECLARE_EVENT_TABLE_TERMINATOR() };

/*
 * Event table macros
 */

// helpers for writing shorter code below: declare an event macro taking 2, 1
// or none ids (the missing ids default to wxID_ANY)
//
// macro arguments:
//  - evt one of wxEVT_XXX constants
//  - id1, id2 ids of the first/last id
//  - fn the function (should be cast to the right type)
#define wx__DECLARE_EVT2(evt, id1, id2, fn) \
    wxDECLARE_EVENT_TABLE_ENTRY(evt, id1, id2, fn, NULL),
#define wx__DECLARE_EVT1(evt, id, fn) \
    wx__DECLARE_EVT2(evt, id, wxID_ANY, fn)
#define wx__DECLARE_EVT0(evt, fn) \
    wx__DECLARE_EVT1(evt, wxID_ANY, fn)


// Generic events
#define EVT_CUSTOM(event, winid, func) \
    wx__DECLARE_EVT1(event, winid, wxEventHandler(func))
#define EVT_CUSTOM_RANGE(event, id1, id2, func) \
    wx__DECLARE_EVT2(event, id1, id2, wxEventHandler(func))

// EVT_COMMAND
#define EVT_COMMAND(winid, event, func) \
    wx__DECLARE_EVT1(event, winid, wxCommandEventHandler(func))

#define EVT_COMMAND_RANGE(id1, id2, event, func) \
    wx__DECLARE_EVT2(event, id1, id2, wxCommandEventHandler(func))

#define EVT_NOTIFY(event, winid, func) \
    wx__DECLARE_EVT1(event, winid, wxNotifyEventHandler(func))

#define EVT_NOTIFY_RANGE(event, id1, id2, func) \
    wx__DECLARE_EVT2(event, id1, id2, wxNotifyEventHandler(func))

// Miscellaneous
#define EVT_SIZE(func)  wx__DECLARE_EVT0(wxEVT_SIZE, wxSizeEventHandler(func))
#define EVT_SIZING(func)  wx__DECLARE_EVT0(wxEVT_SIZING, wxSizeEventHandler(func))
#define EVT_MOVE(func)  wx__DECLARE_EVT0(wxEVT_MOVE, wxMoveEventHandler(func))
#define EVT_MOVING(func)  wx__DECLARE_EVT0(wxEVT_MOVING, wxMoveEventHandler(func))
#define EVT_MOVE_START(func)  wx__DECLARE_EVT0(wxEVT_MOVE_START, wxMoveEventHandler(func))
#define EVT_MOVE_END(func)  wx__DECLARE_EVT0(wxEVT_MOVE_END, wxMoveEventHandler(func))
#define EVT_CLOSE(func)  wx__DECLARE_EVT0(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(func))
#define EVT_END_SESSION(func)  wx__DECLARE_EVT0(wxEVT_END_SESSION, wxCloseEventHandler(func))
#define EVT_QUERY_END_SESSION(func)  wx__DECLARE_EVT0(wxEVT_QUERY_END_SESSION, wxCloseEventHandler(func))
#define EVT_PAINT(func)  wx__DECLARE_EVT0(wxEVT_PAINT, wxPaintEventHandler(func))
#define EVT_NC_PAINT(func)  wx__DECLARE_EVT0(wxEVT_NC_PAINT, wxNcPaintEventHandler(func))
#define EVT_ERASE_BACKGROUND(func)  wx__DECLARE_EVT0(wxEVT_ERASE_BACKGROUND, wxEraseEventHandler(func))
#define EVT_CHAR(func)  wx__DECLARE_EVT0(wxEVT_CHAR, wxCharEventHandler(func))
#define EVT_KEY_DOWN(func)  wx__DECLARE_EVT0(wxEVT_KEY_DOWN, wxKeyEventHandler(func))
#define EVT_KEY_UP(func)  wx__DECLARE_EVT0(wxEVT_KEY_UP, wxKeyEventHandler(func))
#if wxUSE_HOTKEY
#define EVT_HOTKEY(winid, func)  wx__DECLARE_EVT1(wxEVT_HOTKEY, winid, wxCharEventHandler(func))
#endif
#define EVT_CHAR_HOOK(func)  wx__DECLARE_EVT0(wxEVT_CHAR_HOOK, wxCharEventHandler(func))
#define EVT_MENU_OPEN(func)  wx__DECLARE_EVT0(wxEVT_MENU_OPEN, wxMenuEventHandler(func))
#define EVT_MENU_CLOSE(func)  wx__DECLARE_EVT0(wxEVT_MENU_CLOSE, wxMenuEventHandler(func))
#define EVT_MENU_HIGHLIGHT(winid, func)  wx__DECLARE_EVT1(wxEVT_MENU_HIGHLIGHT, winid, wxMenuEventHandler(func))
#define EVT_MENU_HIGHLIGHT_ALL(func)  wx__DECLARE_EVT0(wxEVT_MENU_HIGHLIGHT, wxMenuEventHandler(func))
#define EVT_SET_FOCUS(func)  wx__DECLARE_EVT0(wxEVT_SET_FOCUS, wxFocusEventHandler(func))
#define EVT_KILL_FOCUS(func)  wx__DECLARE_EVT0(wxEVT_KILL_FOCUS, wxFocusEventHandler(func))
#define EVT_CHILD_FOCUS(func)  wx__DECLARE_EVT0(wxEVT_CHILD_FOCUS, wxChildFocusEventHandler(func))
#define EVT_ACTIVATE(func)  wx__DECLARE_EVT0(wxEVT_ACTIVATE, wxActivateEventHandler(func))
#define EVT_ACTIVATE_APP(func)  wx__DECLARE_EVT0(wxEVT_ACTIVATE_APP, wxActivateEventHandler(func))
#define EVT_HIBERNATE(func)  wx__DECLARE_EVT0(wxEVT_HIBERNATE, wxActivateEventHandler(func))
#define EVT_END_SESSION(func)  wx__DECLARE_EVT0(wxEVT_END_SESSION, wxCloseEventHandler(func))
#define EVT_QUERY_END_SESSION(func)  wx__DECLARE_EVT0(wxEVT_QUERY_END_SESSION, wxCloseEventHandler(func))
#define EVT_DROP_FILES(func)  wx__DECLARE_EVT0(wxEVT_DROP_FILES, wxDropFilesEventHandler(func))
#define EVT_INIT_DIALOG(func)  wx__DECLARE_EVT0(wxEVT_INIT_DIALOG, wxInitDialogEventHandler(func))
#define EVT_SYS_COLOUR_CHANGED(func) wx__DECLARE_EVT0(wxEVT_SYS_COLOUR_CHANGED, wxSysColourChangedEventHandler(func))
#define EVT_DISPLAY_CHANGED(func)  wx__DECLARE_EVT0(wxEVT_DISPLAY_CHANGED, wxDisplayChangedEventHandler(func))
#define EVT_DPI_CHANGED(func)  wx__DECLARE_EVT0(wxEVT_DPI_CHANGED, wxDPIChangedEventHandler(func))
#define EVT_SHOW(func) wx__DECLARE_EVT0(wxEVT_SHOW, wxShowEventHandler(func))
#define EVT_MAXIMIZE(func) wx__DECLARE_EVT0(wxEVT_MAXIMIZE, wxMaximizeEventHandler(func))
#define EVT_ICONIZE(func) wx__DECLARE_EVT0(wxEVT_ICONIZE, wxIconizeEventHandler(func))
#define EVT_NAVIGATION_KEY(func) wx__DECLARE_EVT0(wxEVT_NAVIGATION_KEY, wxNavigationKeyEventHandler(func))
#define EVT_PALETTE_CHANGED(func) wx__DECLARE_EVT0(wxEVT_PALETTE_CHANGED, wxPaletteChangedEventHandler(func))
#define EVT_QUERY_NEW_PALETTE(func) wx__DECLARE_EVT0(wxEVT_QUERY_NEW_PALETTE, wxQueryNewPaletteEventHandler(func))
#define EVT_WINDOW_CREATE(func) wx__DECLARE_EVT0(wxEVT_CREATE, wxWindowCreateEventHandler(func))
#define EVT_WINDOW_DESTROY(func) wx__DECLARE_EVT0(wxEVT_DESTROY, wxWindowDestroyEventHandler(func))
#define EVT_SET_CURSOR(func) wx__DECLARE_EVT0(wxEVT_SET_CURSOR, wxSetCursorEventHandler(func))
#define EVT_MOUSE_CAPTURE_CHANGED(func) wx__DECLARE_EVT0(wxEVT_MOUSE_CAPTURE_CHANGED, wxMouseCaptureChangedEventHandler(func))
#define EVT_MOUSE_CAPTURE_LOST(func) wx__DECLARE_EVT0(wxEVT_MOUSE_CAPTURE_LOST, wxMouseCaptureLostEventHandler(func))

// Mouse events
#define EVT_LEFT_DOWN(func) wx__DECLARE_EVT0(wxEVT_LEFT_DOWN, wxMouseEventHandler(func))
#define EVT_LEFT_UP(func) wx__DECLARE_EVT0(wxEVT_LEFT_UP, wxMouseEventHandler(func))
#define EVT_MIDDLE_DOWN(func) wx__DECLARE_EVT0(wxEVT_MIDDLE_DOWN, wxMouseEventHandler(func))
#define EVT_MIDDLE_UP(func) wx__DECLARE_EVT0(wxEVT_MIDDLE_UP, wxMouseEventHandler(func))
#define EVT_RIGHT_DOWN(func) wx__DECLARE_EVT0(wxEVT_RIGHT_DOWN, wxMouseEventHandler(func))
#define EVT_RIGHT_UP(func) wx__DECLARE_EVT0(wxEVT_RIGHT_UP, wxMouseEventHandler(func))
#define EVT_MOTION(func) wx__DECLARE_EVT0(wxEVT_MOTION, wxMouseEventHandler(func))
#define EVT_LEFT_DCLICK(func) wx__DECLARE_EVT0(wxEVT_LEFT_DCLICK, wxMouseEventHandler(func))
#define EVT_MIDDLE_DCLICK(func) wx__DECLARE_EVT0(wxEVT_MIDDLE_DCLICK, wxMouseEventHandler(func))
#define EVT_RIGHT_DCLICK(func) wx__DECLARE_EVT0(wxEVT_RIGHT_DCLICK, wxMouseEventHandler(func))
#define EVT_LEAVE_WINDOW(func) wx__DECLARE_EVT0(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(func))
#define EVT_ENTER_WINDOW(func) wx__DECLARE_EVT0(wxEVT_ENTER_WINDOW, wxMouseEventHandler(func))
#define EVT_MOUSEWHEEL(func) wx__DECLARE_EVT0(wxEVT_MOUSEWHEEL, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX1_DOWN(func) wx__DECLARE_EVT0(wxEVT_AUX1_DOWN, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX1_UP(func) wx__DECLARE_EVT0(wxEVT_AUX1_UP, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX1_DCLICK(func) wx__DECLARE_EVT0(wxEVT_AUX1_DCLICK, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX2_DOWN(func) wx__DECLARE_EVT0(wxEVT_AUX2_DOWN, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX2_UP(func) wx__DECLARE_EVT0(wxEVT_AUX2_UP, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX2_DCLICK(func) wx__DECLARE_EVT0(wxEVT_AUX2_DCLICK, wxMouseEventHandler(func))
#define EVT_MAGNIFY(func) wx__DECLARE_EVT0(wxEVT_MAGNIFY, wxMouseEventHandler(func))

// All mouse events
#define EVT_MOUSE_EVENTS(func) \
    EVT_LEFT_DOWN(func) \
    EVT_LEFT_UP(func) \
    EVT_LEFT_DCLICK(func) \
    EVT_MIDDLE_DOWN(func) \
    EVT_MIDDLE_UP(func) \
    EVT_MIDDLE_DCLICK(func) \
    EVT_RIGHT_DOWN(func) \
    EVT_RIGHT_UP(func) \
    EVT_RIGHT_DCLICK(func) \
    EVT_MOUSE_AUX1_DOWN(func) \
    EVT_MOUSE_AUX1_UP(func) \
    EVT_MOUSE_AUX1_DCLICK(func) \
    EVT_MOUSE_AUX2_DOWN(func) \
    EVT_MOUSE_AUX2_UP(func) \
    EVT_MOUSE_AUX2_DCLICK(func) \
    EVT_MOTION(func) \
    EVT_LEAVE_WINDOW(func) \
    EVT_ENTER_WINDOW(func) \
    EVT_MOUSEWHEEL(func) \
    EVT_MAGNIFY(func)

// Scrolling from wxWindow (sent to wxScrolledWindow)
#define EVT_SCROLLWIN_TOP(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_TOP, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_BOTTOM(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_BOTTOM, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_LINEUP(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_LINEUP, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_LINEDOWN(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_LINEDOWN, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_PAGEUP(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_PAGEUP, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_PAGEDOWN(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_PAGEDOWN, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_THUMBTRACK(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_THUMBTRACK, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_THUMBRELEASE(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_THUMBRELEASE, wxScrollWinEventHandler(func))

#define EVT_SCROLLWIN(func) \
    EVT_SCROLLWIN_TOP(func) \
    EVT_SCROLLWIN_BOTTOM(func) \
    EVT_SCROLLWIN_LINEUP(func) \
    EVT_SCROLLWIN_LINEDOWN(func) \
    EVT_SCROLLWIN_PAGEUP(func) \
    EVT_SCROLLWIN_PAGEDOWN(func) \
    EVT_SCROLLWIN_THUMBTRACK(func) \
    EVT_SCROLLWIN_THUMBRELEASE(func)

// Scrolling from wxSlider and wxScrollBar
#define EVT_SCROLL_TOP(func) wx__DECLARE_EVT0(wxEVT_SCROLL_TOP, wxScrollEventHandler(func))
#define EVT_SCROLL_BOTTOM(func) wx__DECLARE_EVT0(wxEVT_SCROLL_BOTTOM, wxScrollEventHandler(func))
#define EVT_SCROLL_LINEUP(func) wx__DECLARE_EVT0(wxEVT_SCROLL_LINEUP, wxScrollEventHandler(func))
#define EVT_SCROLL_LINEDOWN(func) wx__DECLARE_EVT0(wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(func))
#define EVT_SCROLL_PAGEUP(func) wx__DECLARE_EVT0(wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(func))
#define EVT_SCROLL_PAGEDOWN(func) wx__DECLARE_EVT0(wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(func))
#define EVT_SCROLL_THUMBTRACK(func) wx__DECLARE_EVT0(wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(func))
#define EVT_SCROLL_THUMBRELEASE(func) wx__DECLARE_EVT0(wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(func))
#define EVT_SCROLL_CHANGED(func) wx__DECLARE_EVT0(wxEVT_SCROLL_CHANGED, wxScrollEventHandler(func))

#define EVT_SCROLL(func) \
    EVT_SCROLL_TOP(func) \
    EVT_SCROLL_BOTTOM(func) \
    EVT_SCROLL_LINEUP(func) \
    EVT_SCROLL_LINEDOWN(func) \
    EVT_SCROLL_PAGEUP(func) \
    EVT_SCROLL_PAGEDOWN(func) \
    EVT_SCROLL_THUMBTRACK(func) \
    EVT_SCROLL_THUMBRELEASE(func) \
    EVT_SCROLL_CHANGED(func)

// Scrolling from wxSlider and wxScrollBar, with an id
#define EVT_COMMAND_SCROLL_TOP(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_TOP, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_BOTTOM(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_BOTTOM, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_LINEUP(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_LINEUP, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_LINEDOWN(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_LINEDOWN, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_PAGEUP(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_PAGEUP, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_PAGEDOWN(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_PAGEDOWN, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_THUMBTRACK(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_THUMBTRACK, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_THUMBRELEASE(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_THUMBRELEASE, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_CHANGED(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_CHANGED, winid, wxScrollEventHandler(func))

#define EVT_COMMAND_SCROLL(winid, func) \
    EVT_COMMAND_SCROLL_TOP(winid, func) \
    EVT_COMMAND_SCROLL_BOTTOM(winid, func) \
    EVT_COMMAND_SCROLL_LINEUP(winid, func) \
    EVT_COMMAND_SCROLL_LINEDOWN(winid, func) \
    EVT_COMMAND_SCROLL_PAGEUP(winid, func) \
    EVT_COMMAND_SCROLL_PAGEDOWN(winid, func) \
    EVT_COMMAND_SCROLL_THUMBTRACK(winid, func) \
    EVT_COMMAND_SCROLL_THUMBRELEASE(winid, func) \
    EVT_COMMAND_SCROLL_CHANGED(winid, func)

// Gesture events
#define EVT_GESTURE_PAN(winid, func) wx__DECLARE_EVT1(wxEVT_GESTURE_PAN, winid, wxPanGestureEventHandler(func))
#define EVT_GESTURE_ZOOM(winid, func) wx__DECLARE_EVT1(wxEVT_GESTURE_ZOOM, winid, wxZoomGestureEventHandler(func))
#define EVT_GESTURE_ROTATE(winid, func) wx__DECLARE_EVT1(wxEVT_GESTURE_ROTATE, winid, wxRotateGestureEventHandler(func))
#define EVT_TWO_FINGER_TAP(winid, func) wx__DECLARE_EVT1(wxEVT_TWO_FINGER_TAP, winid, wxTwoFingerTapEventHandler(func))
#define EVT_LONG_PRESS(winid, func) wx__DECLARE_EVT1(wxEVT_LONG_PRESS, winid, wxLongPressEventHandler(func))
#define EVT_PRESS_AND_TAP(winid, func) wx__DECLARE_EVT1(wxEVT_PRESS_AND_TAP, winid, wxPressAndTapEvent(func))

// Convenience macros for commonly-used commands
#define EVT_CHECKBOX(winid, func) wx__DECLARE_EVT1(wxEVT_CHECKBOX, winid, wxCommandEventHandler(func))
#define EVT_CHOICE(winid, func) wx__DECLARE_EVT1(wxEVT_CHOICE, winid, wxCommandEventHandler(func))
#define EVT_LISTBOX(winid, func) wx__DECLARE_EVT1(wxEVT_LISTBOX, winid, wxCommandEventHandler(func))
#define EVT_LISTBOX_DCLICK(winid, func) wx__DECLARE_EVT1(wxEVT_LISTBOX_DCLICK, winid, wxCommandEventHandler(func))
#define EVT_MENU(winid, func) wx__DECLARE_EVT1(wxEVT_MENU, winid, wxCommandEventHandler(func))
#define EVT_MENU_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_MENU, id1, id2, wxCommandEventHandler(func))
#define EVT_BUTTON(winid, func) wx__DECLARE_EVT1(wxEVT_BUTTON, winid, wxCommandEventHandler(func))
#define EVT_SLIDER(winid, func) wx__DECLARE_EVT1(wxEVT_SLIDER, winid, wxCommandEventHandler(func))
#define EVT_RADIOBOX(winid, func) wx__DECLARE_EVT1(wxEVT_RADIOBOX, winid, wxCommandEventHandler(func))
#define EVT_RADIOBUTTON(winid, func) wx__DECLARE_EVT1(wxEVT_RADIOBUTTON, winid, wxCommandEventHandler(func))
// EVT_SCROLLBAR is now obsolete since we use EVT_COMMAND_SCROLL... events
#define EVT_SCROLLBAR(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLLBAR, winid, wxCommandEventHandler(func))
#define EVT_VLBOX(winid, func) wx__DECLARE_EVT1(wxEVT_VLBOX, winid, wxCommandEventHandler(func))
#define EVT_COMBOBOX(winid, func) wx__DECLARE_EVT1(wxEVT_COMBOBOX, winid, wxCommandEventHandler(func))
#define EVT_TOOL(winid, func) wx__DECLARE_EVT1(wxEVT_TOOL, winid, wxCommandEventHandler(func))
#define EVT_TOOL_DROPDOWN(winid, func) wx__DECLARE_EVT1(wxEVT_TOOL_DROPDOWN, winid, wxCommandEventHandler(func))
#define EVT_TOOL_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_TOOL, id1, id2, wxCommandEventHandler(func))
#define EVT_TOOL_RCLICKED(winid, func) wx__DECLARE_EVT1(wxEVT_TOOL_RCLICKED, winid, wxCommandEventHandler(func))
#define EVT_TOOL_RCLICKED_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_TOOL_RCLICKED, id1, id2, wxCommandEventHandler(func))
#define EVT_TOOL_ENTER(winid, func) wx__DECLARE_EVT1(wxEVT_TOOL_ENTER, winid, wxCommandEventHandler(func))
#define EVT_CHECKLISTBOX(winid, func) wx__DECLARE_EVT1(wxEVT_CHECKLISTBOX, winid, wxCommandEventHandler(func))
#define EVT_COMBOBOX_DROPDOWN(winid, func) wx__DECLARE_EVT1(wxEVT_COMBOBOX_DROPDOWN, winid, wxCommandEventHandler(func))
#define EVT_COMBOBOX_CLOSEUP(winid, func) wx__DECLARE_EVT1(wxEVT_COMBOBOX_CLOSEUP, winid, wxCommandEventHandler(func))

// Generic command events
#define EVT_COMMAND_LEFT_CLICK(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_LEFT_CLICK, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_LEFT_DCLICK(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_LEFT_DCLICK, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_RIGHT_CLICK(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_RIGHT_CLICK, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_RIGHT_DCLICK(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_RIGHT_DCLICK, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_SET_FOCUS(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_SET_FOCUS, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_KILL_FOCUS(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_KILL_FOCUS, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_ENTER(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_ENTER, winid, wxCommandEventHandler(func))

// Joystick events

#define EVT_JOY_BUTTON_DOWN(func) wx__DECLARE_EVT0(wxEVT_JOY_BUTTON_DOWN, wxJoystickEventHandler(func))
#define EVT_JOY_BUTTON_UP(func) wx__DECLARE_EVT0(wxEVT_JOY_BUTTON_UP, wxJoystickEventHandler(func))
#define EVT_JOY_MOVE(func) wx__DECLARE_EVT0(wxEVT_JOY_MOVE, wxJoystickEventHandler(func))
#define EVT_JOY_ZMOVE(func) wx__DECLARE_EVT0(wxEVT_JOY_ZMOVE, wxJoystickEventHandler(func))

// All joystick events
#define EVT_JOYSTICK_EVENTS(func) \
    EVT_JOY_BUTTON_DOWN(func) \
    EVT_JOY_BUTTON_UP(func) \
    EVT_JOY_MOVE(func) \
    EVT_JOY_ZMOVE(func)

// Idle event
#define EVT_IDLE(func) wx__DECLARE_EVT0(wxEVT_IDLE, wxIdleEventHandler(func))

// Update UI event
#define EVT_UPDATE_UI(winid, func) wx__DECLARE_EVT1(wxEVT_UPDATE_UI, winid, wxUpdateUIEventHandler(func))
#define EVT_UPDATE_UI_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_UPDATE_UI, id1, id2, wxUpdateUIEventHandler(func))

// Help events
#define EVT_HELP(winid, func) wx__DECLARE_EVT1(wxEVT_HELP, winid, wxHelpEventHandler(func))
#define EVT_HELP_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_HELP, id1, id2, wxHelpEventHandler(func))
#define EVT_DETAILED_HELP(winid, func) wx__DECLARE_EVT1(wxEVT_DETAILED_HELP, winid, wxHelpEventHandler(func))
#define EVT_DETAILED_HELP_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_DETAILED_HELP, id1, id2, wxHelpEventHandler(func))

// Context Menu Events
#define EVT_CONTEXT_MENU(func) wx__DECLARE_EVT0(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(func))
#define EVT_COMMAND_CONTEXT_MENU(winid, func) wx__DECLARE_EVT1(wxEVT_CONTEXT_MENU, winid, wxContextMenuEventHandler(func))

// Clipboard text Events
#define EVT_TEXT_CUT(winid, func) wx__DECLARE_EVT1(wxEVT_TEXT_CUT, winid, wxClipboardTextEventHandler(func))
#define EVT_TEXT_COPY(winid, func) wx__DECLARE_EVT1(wxEVT_TEXT_COPY, winid, wxClipboardTextEventHandler(func))
#define EVT_TEXT_PASTE(winid, func) wx__DECLARE_EVT1(wxEVT_TEXT_PASTE, winid, wxClipboardTextEventHandler(func))

// Thread events
#define EVT_THREAD(id, func)  wx__DECLARE_EVT1(wxEVT_THREAD, id, wxThreadEventHandler(func))

// ----------------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------------

#if wxUSE_GUI

// Find a window with the focus, that is also a descendant of the given window.
// This is used to determine the window to initially send commands to.
wxWindow* wxFindFocusDescendant(wxWindow* ancestor);

#endif // wxUSE_GUI

#endif // _WX_EVENT_H_
