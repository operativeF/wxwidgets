/////////////////////////////////////////////////////////////////////////////
// Name:        wx/html/m_templ.h
// Purpose:     Modules template file
// Author:      Vaclav Slavik
// Copyright:   (c) Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

/*

DESCRIPTION:
This is set of macros for easier writing of tag handlers. How to use it?
See mod_fonts.cpp for example...

Attention! This is quite strange C++ bastard. Before using it,
I STRONGLY recommend reading and understanding these macros!!

*/


#ifndef _WX_M_TEMPL_H_
#define _WX_M_TEMPL_H_

#if wxUSE_HTML

#include "wx/defs.h"

#include "wx/html/winpars.h"

#define TAG_HANDLER_BEGIN(name,tags)                                      \
    class wxHTML_Handler_##name : public wxHtmlWinTagHandler              \
    {                                                                     \
        public:                                                           \
            wxString GetSupportedTags() override {return wxT(tags);}



#define TAG_HANDLER_VARS                                                  \
        private:

#define TAG_HANDLER_CONSTR(name)                                                \
        public:                                                           \
        wxHTML_Handler_##name () : wxHtmlWinTagHandler()


#define TAG_HANDLER_PROC(varib)                                           \
        public:                                                           \
            bool HandleTag(const wxHtmlTag& varib) override



#define TAG_HANDLER_END(name)                                             \
    };




#define TAGS_MODULE_BEGIN(name)                                           \
    class wxHTML_Module##name : public wxHtmlTagsModule                   \
    {                                                                     \
        wxDECLARE_DYNAMIC_CLASS(wxHTML_Module##name );                    \
        public:                                                           \
            void FillHandlersTable(wxHtmlWinParser *parser) override    \
                {




#define TAGS_MODULE_ADD(handler)                                          \
                    parser->AddTagHandler(new wxHTML_Handler_##handler);




#define TAGS_MODULE_END(name)                                             \
                }                                                         \
    };                                                                    \
    wxIMPLEMENT_DYNAMIC_CLASS(wxHTML_Module##name , wxHtmlTagsModule)



#endif
#endif
