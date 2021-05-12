///////////////////////////////////////////////////////////////////////////////
// Name:        wx/stack.h
// Purpose:     STL stack clone
// Author:      Lindsay Mathieson, Vadim Zeitlin
// Created:     30.07.2001
// Copyright:   (c) 2001 Lindsay Mathieson <lindsay@mathieson.org> (WX_DECLARE_STACK)
//                  2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_STACK_H_
#define _WX_STACK_H_

#include <vector>

#include <stack>
#define wxStack std::stack

// Deprecated macro-based class for compatibility only, don't use any more.
#define WX_DECLARE_STACK(obj, cls) \
class cls : public std::vector<obj> \
{\
public:\
    void push(const obj& o)\
    {\
        push_back(o); \
    };\
\
    void pop()\
    {\
        pop_back(); \
    };\
\
    obj& top()\
    {\
        return at(size() - 1);\
    };\
    const obj& top() const\
    {\
        return at(size() - 1); \
    };\
}

#endif // _WX_STACK_H_

