///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/stackwalk.h
// Purpose:     wxStackWalker for MSW
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2005-01-08
// Copyright:   (c) 2005 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_STACKWALK_H_
#define _WX_MSW_STACKWALK_H_

import <vector>;

// these structs are declared in windows headers
struct _CONTEXT;
struct _EXCEPTION_POINTERS;

// and these in dbghelp.h
struct _SYMBOL_INFO;
struct _SYMBOL_INFOW;

#define wxSYMBOL_INFO _SYMBOL_INFOW

// ----------------------------------------------------------------------------
// wxStackFrame
// ----------------------------------------------------------------------------

class wxStackFrame : public wxStackFrameBase
{
private:
    wxStackFrame *ConstCast() const
        { return const_cast<wxStackFrame *>(this); }

    size_t DoGetParamCount() const { return m_paramTypes.size(); }

public:
    wxStackFrame(size_t level, void *address, size_t addrFrame)
        : wxStackFrameBase(level, address),
          m_addrFrame(addrFrame)
    {
    }

    size_t GetParamCount() const override
    {
        ConstCast()->OnGetParam();
        return DoGetParamCount();
    }

    bool
    GetParam(size_t n, wxString *type, wxString *name, wxString *value) const override;

    // callback used by OnGetParam(), don't call directly
    void OnParam(wxSYMBOL_INFO *pSymInfo);

protected:
    void OnGetName() override;
    void OnGetLocation() override;

    void OnGetParam();


    // helper for debug API: it wants to have addresses as DWORDs
    size_t GetSymAddr() const
    {
        return reinterpret_cast<size_t>(m_address);
    }

private:
    bool m_hasName{false};
    bool m_hasLocation{false};

    size_t m_addrFrame;

    std::vector<wxString> m_paramTypes;
    std::vector<wxString> m_paramNames;
    std::vector<wxString> m_paramValues;
};

// ----------------------------------------------------------------------------
// wxStackWalker
// ----------------------------------------------------------------------------

class wxStackWalker : public wxStackWalkerBase
{
public:
    // we don't use ctor argument, it is for compatibility with Unix version
    // only
    wxStackWalker([[maybe_unused]] const char* argv0) {}

    void Walk(size_t skip = 1, size_t maxDepth = wxSTACKWALKER_MAX_DEPTH) override;
#if wxUSE_ON_FATAL_EXCEPTION
    void WalkFromException(size_t maxDepth = wxSTACKWALKER_MAX_DEPTH) override;
#endif // wxUSE_ON_FATAL_EXCEPTION


    // enumerate stack frames from the given context
    void WalkFrom(const _CONTEXT *ctx, size_t skip = 1, size_t maxDepth = wxSTACKWALKER_MAX_DEPTH);
    void WalkFrom(const _EXCEPTION_POINTERS *ep, size_t skip = 1, size_t maxDepth = wxSTACKWALKER_MAX_DEPTH);
};

#endif // _WX_MSW_STACKWALK_H_

