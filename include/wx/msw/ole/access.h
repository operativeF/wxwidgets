///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/ole/access.h
// Purpose:     declaration of the wxAccessible class
// Author:      Julian Smart
// Modified by:
// Created:     2003-02-12
// Copyright:   (c) 2003 Julian Smart
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef   _WX_ACCESS_H_
#define   _WX_ACCESS_H_

#if wxUSE_ACCESSIBILITY

// ----------------------------------------------------------------------------
// forward declarations
// ----------------------------------------------------------------------------

struct IAccessible;
class wxIAccessible;
class wxWindow;

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// wxAccessible implements accessibility behaviour.
// ----------------------------------------------------------------------------

class wxAccessible : public wxAccessibleBase
{
public:
    wxAccessible(wxWindow *win = nullptr);
    ~wxAccessible();

	wxAccessible& operator=(wxAccessible&&) = delete;

    // Returns the wxIAccessible pointer
    wxIAccessible* GetIAccessible() { return m_pIAccessible; }

    // Returns the IAccessible standard interface pointer
    IAccessible* GetIAccessibleStd();

// Operations

    // Sends an event when something changes in an accessible object.
    static void NotifyEvent(int eventType, wxWindow* window, wxAccObject objectType,
                            int objectId);

private:
    wxIAccessible * m_pIAccessible;  // the pointer to COM interface
    IAccessible*    m_pIAccessibleStd{nullptr};  // the pointer to the standard COM interface,
                                        // for default processing
};

#endif  //wxUSE_ACCESSIBILITY

#endif  //_WX_ACCESS_H_

