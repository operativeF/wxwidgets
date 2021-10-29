#############################################################################
# Name:        build/cmake/options.cmake
# Purpose:     User selectable build options
# Author:      Tobias Taschner
# Created:     2016-09-24
# Copyright:   (c) 2016 wxWidgets development team
# Licence:     wxWindows licence
#############################################################################

# Add an AddGlobalOption that will also declare a suitable macro at global scope
function(AddGlobalOption OptionName Description DefaultValue)
    option(${OptionName} Description DefaultValue)
    add_compile_definitions(${OptionName}=${DefaultValue})
endfunction()

# Global build options
option(wxBUILD_SHARED "Build wx libraries as shared libs" ${BUILD_SHARED_LIBS})

AddGlobalOption(wxBUILD_MONOLITHIC "build wxWidgets as single library" 0)
AddGlobalOption(wxBUILD_SAMPLES "Build only important samples (SOME) or ALL" 0)
AddGlobalOption(wxBUILD_TESTS "Build console tests (CONSOLE_ONLY) or ALL" 0)
AddGlobalOption(wxBUILD_DEMOS "Build demos" 0)
AddGlobalOption(wxBUILD_BENCHMARKS "Build benchmarks" 0)
AddGlobalOption(wxBUILD_PRECOMP "Use precompiled headers" 0)
AddGlobalOption(wxBUILD_INSTALL "Create install/uninstall target for wxWidgets" 0)
AddGlobalOption(wxUSE_GUI "Build with GUI" 1)
# Allow user specified setup.h folder
set(wxBUILD_CUSTOM_SETUP_HEADER_PATH "" CACHE PATH "Include path containing custom wx/setup.h")
mark_as_advanced(wxBUILD_CUSTOM_SETUP_HEADER_PATH)

if(WIN32)
    set(wxUSE_DPI_AWARE_MANIFEST "per-monitor" CACHE STRING "DPI Awareness; default is per-monitor")
    set_property(CACHE wxUSE_DPI_AWARE_MANIFEST PROPERTY STRINGS "none" "system" "per-monitor")
endif()

set(wxBUILD_DEBUG_LEVEL "Default" CACHE STRING "Debugging level")
set_property(CACHE wxBUILD_DEBUG_LEVEL PROPERTY STRINGS Default 0 1 2)
mark_as_advanced(wxBUILD_DEBUG_LEVEL)

if(NOT APPLE)
    AddGlobalOption(wxBUILD_USE_STATIC_RUNTIME "Link using the static runtime library" 0)
    mark_as_advanced(wxBUILD_USE_STATIC_RUNTIME)
endif()

if(MSVC)
    AddGlobalOption(wxBUILD_MSVC_MULTIPROC "Enable multi-processor compilation for MSVC" 1)
    mark_as_advanced(wxBUILD_MSVC_MULTIPROC)
endif()

if(WIN32)
    set(wxBUILD_VENDOR "custom" CACHE STRING "Short string identifying your company (used in DLL name)")
endif()
set(wxBUILD_FLAVOUR "" CACHE STRING "Specify a name to identify the build")
mark_as_advanced(wxBUILD_FLAVOUR)

AddGlobalOption(wxBUILD_OPTIMISE "use speed-optimised C/C++ compiler flags for release build" 0)
mark_as_advanced(wxBUILD_OPTIMISE)
if(MSVC)
    set(wxBUILD_STRIPPED_RELEASE_DEFAULT 0)
else()
    set(wxBUILD_STRIPPED_RELEASE_DEFAULT 1)
endif()
AddGlobalOption(wxBUILD_STRIPPED_RELEASE "remove debug symbols in release build" ${wxBUILD_STRIPPED_RELEASE_DEFAULT})
mark_as_advanced(wxBUILD_STRIPPED_RELEASE)
AddGlobalOption(wxBUILD_PIC "Enable position independent code (PIC)." 1)
mark_as_advanced(wxBUILD_PIC)
AddGlobalOption(wxUSE_NO_RTTI "disable RTTI support" 0)

# STL options
AddGlobalOption(wxUSE_STL "use standard C++ classes for everything" 1) # TBR
AddGlobalOption(wxUSE_STD_CONTAINERS "use standard C++ container classes" 1) # TBR
set(wxTHIRD_PARTY_LIBRARIES ${wxTHIRD_PARTY_LIBRARIES} wxUSE_STL "use C++ STL classes")

# TBR
AddGlobalOption(wxUSE_UNICODE "compile with Unicode support (NOT RECOMMENDED to be turned 0)" 1)
if(NOT WIN32)
    AddGlobalOption(wxUSE_UNICODE_UTF8 "use UTF-8 representation for strings (Unix only)" 0)
    wx_dependent_option(wxUSE_UTF8_LOCALE_ONLY "only support UTF-8 locales in UTF-8 build (Unix only)" 1 "wxUSE_UNICODE_UTF8" 0)
endif()

AddGlobalOption(wxUSE_COMPILER_TLS "enable use of compiler TLS support" 0)

if(NOT WIN32)
    AddGlobalOption(wxUSE_VISIBILITY "use of ELF symbols visibility")
endif()

AddGlobalOption(wxUSE_UNSAFE_WXSTRING_CONV "provide unsafe implicit conversions in wxString to const char* or std::string" 1)
AddGlobalOption(wxUSE_REPRODUCIBLE_BUILD "enable reproducable build" 0)

# ---------------------------------------------------------------------------
# external libraries
# ---------------------------------------------------------------------------
#TBR wx_option
# Add a option and mark is as advanced if it starts with wxUSE_
# wx_option(<name> <desc> [default] [STRINGS strings])
# The default is 1 if not third parameter is specified
function(wx_option name desc)
    cmake_parse_arguments(OPTION "" "" "STRINGS" ${ARGN})
    if(ARGC EQUAL 2)
        set(default 1)
    else()
        set(default ${OPTION_UNPARSED_ARGUMENTS})
    endif()

    if(OPTION_STRINGS)
        set(cache_type STRING)
    else()
        set(cache_type BOOL)
    endif()

    set(${name} "${default}" CACHE ${cache_type} "${desc}")
    string(SUBSTRING ${name} 0 6 prefix)
    if(prefix STREQUAL "wxUSE_")
        mark_as_advanced(${name})
    endif()
    if(OPTION_STRINGS)
        set_property(CACHE ${name} PROPERTY STRINGS ${OPTION_STRINGS})
        # Check valid value
        set(value_is_valid FALSE)
        set(avail_values)
        foreach(opt ${OPTION_STRINGS})
            if(${name} STREQUAL opt)
                set(value_is_valid TRUE)
                break()
            endif()
            wx_string_append(avail_values " ${opt}")
        endforeach()
        if(NOT value_is_valid)
            message(FATAL_ERROR "Invalid value \"${${name}}\" for option ${name}. Valid values are: ${avail_values}")
        endif()
    endif()
endfunction()

# TBR
# wx_install(...)
# Forward to install call if wxBUILD_INSTALL is enabled
macro(wx_install)
    if(wxBUILD_INSTALL)
        install(${ARGN})
    endif()
endmacro()

# Add third party library
function(wx_add_thirdparty_library var_name lib_name help_str)
    cmake_parse_arguments(THIRDPARTY "" "DEFAULT;DEFAULT_APPLE;DEFAULT_WIN32" "" ${ARGN})

    if(THIRDPARTY_DEFAULT)
        set(thirdparty_lib_default ${THIRDPARTY_DEFAULT})
    elseif(THIRDPARTY_DEFAULT_APPLE AND APPLE)
        set(thirdparty_lib_default ${THIRDPARTY_DEFAULT_APPLE})
    elseif(THIRDPARTY_DEFAULT_WIN32 AND WIN32)
        set(thirdparty_lib_default ${THIRDPARTY_DEFAULT_WIN32})
    elseif(UNIX AND NOT APPLE)
        # Try sys libraries for MSYS and CYGWIN
        set(thirdparty_lib_default sys)
    elseif(WIN32 OR APPLE)
        # On Windows or apple platforms prefer using the builtin libraries
        set(thirdparty_lib_default builtin)
    else()
        set(thirdparty_lib_default sys)
    endif()

    wx_option(${var_name} ${help_str} ${thirdparty_lib_default}
        STRINGS builtin sys 0)

    if(${var_name} STREQUAL "sys")
        # If the sys library can not be found use builtin
        find_package(${lib_name})
        string(TOUPPER ${lib_name} lib_name_upper)
        if(NOT ${${lib_name_upper}_FOUND})
            wx_option_force_value(${var_name} builtin)
        endif()
    endif()

    if(${var_name} STREQUAL "builtin" AND NOT wxBUILD_SHARED)
        # Only install if we build as static libraries
        wx_install(TARGETS ${target_name}
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            )
    endif()

    set(wxTHIRD_PARTY_LIBRARIES ${wxTHIRD_PARTY_LIBRARIES} ${var_name} "${help_str}" PARENT_SCOPE)
endfunction()

# FIXME: Use extern library or use vcpkg
#wx_add_thirdparty_library(wxUSE_REGEX REGEX "enable support for wxRegEx class" DEFAULT builtin)
wx_add_thirdparty_library(wxUSE_ZLIB ZLIB "use zlib for LZW compression" DEFAULT_APPLE sys)
wx_add_thirdparty_library(wxUSE_EXPAT EXPAT "use expat for XML parsing" DEFAULT_APPLE sys)
wx_add_thirdparty_library(wxUSE_LIBJPEG JPEG "use libjpeg (JPEG file format)")
wx_add_thirdparty_library(wxUSE_LIBPNG PNG "use libpng (PNG image format)")
wx_add_thirdparty_library(wxUSE_LIBTIFF TIFF "use libtiff (TIFF file format)")

AddGlobalOption(wxUSE_LIBLZMA "use LZMA compression" 0)
set(wxTHIRD_PARTY_LIBRARIES ${wxTHIRD_PARTY_LIBRARIES} wxUSE_LIBLZMA "use liblzma for LZMA compression")

AddGlobalOption(wxUSE_OPENGL "use OpenGL (or Mesa)" 1)

if(UNIX)
    AddGlobalOption(wxUSE_LIBSDL "use SDL for audio on Unix")
    AddGlobalOption(wxUSE_LIBICONV "use libiconv (character conversion)")
    AddGlobalOption(wxUSE_LIBNOTIFY "use libnotify for notifications")
    AddGlobalOption(wxUSE_XTEST "use XTest extension")
    AddGlobalOption(wxUSE_LIBMSPACK "use libmspack (CHM help files loading)")
    AddGlobalOption(wxUSE_LIBGNOMEVFS "use GNOME VFS for associating MIME types")
    AddGlobalOption(wxUSE_GLCANVAS_EGL "use EGL backend for wxGLCanvas")

    set(wxTHIRD_PARTY_LIBRARIES ${wxTHIRD_PARTY_LIBRARIES} wxUSE_LIBSDL "use SDL for audio on Unix")
    set(wxTHIRD_PARTY_LIBRARIES ${wxTHIRD_PARTY_LIBRARIES} wxUSE_LIBMSPACK "use libmspack (CHM help files loading)")
endif()

# ---------------------------------------------------------------------------
# optional non GUI features
# ---------------------------------------------------------------------------
AddGlobalOption(wxUSE_INTL "use internationalization system" 1)
AddGlobalOption(wxUSE_XLOCALE "use x-locale support (requires wxLocale)" 0)
AddGlobalOption(wxUSE_CONFIG "use wxConfig (and derived) classes" 1)

AddGlobalOption(wxUSE_SOCKETS "use socket/network classes" 0)
AddGlobalOption(wxUSE_IPV6 "enable IPv6 support in wxSocket" 0)
if(WIN32)
    AddGlobalOption(wxUSE_OLE "use OLE classes" 1)
endif()
AddGlobalOption(wxUSE_DATAOBJ "use data object classes" 1)

AddGlobalOption(wxUSE_IPC "use interprocess communication (wxSocket etc.)" 0)

AddGlobalOption(wxUSE_CONSOLE_EVENTLOOP "use event loop in console programs too" 1)

# please keep the settings below in alphabetical order
AddGlobalOption(wxUSE_ANY "Use wxAny class" 1)
AddGlobalOption(wxUSE_ARCHIVE_STREAMS "use wxArchive streams" 1)
AddGlobalOption(wxUSE_BASE64 "use base64 encoding/decoding functions" 1)
AddGlobalOption(wxUSE_STACKWALKER "use wxStackWalker class for getting backtraces" 0)
AddGlobalOption(wxUSE_ON_FATAL_EXCEPTION "catch signals in wxApp::OnFatalException" 0)
AddGlobalOption(wxUSE_CMDLINE_PARSER "use wxCmdLineParser class" 1)
AddGlobalOption(wxUSE_DATETIME "use wxDateTime class" 1)
AddGlobalOption(wxUSE_DEBUGREPORT "use wxDebugReport class" 1)

AddGlobalOption(wxUSE_DYNLIB_CLASS "use wxLibrary class for DLL loading" 1)
AddGlobalOption(wxUSE_DYNAMIC_LOADER "use (new) wxDynamicLibrary class" 1)
AddGlobalOption(wxUSE_EXCEPTIONS "build exception-safe library" 1)
AddGlobalOption(wxUSE_EXTENDED_RTTI "use extended RTTI (XTI)" 0)
AddGlobalOption(wxUSE_FFILE "use wxFFile class" 1)
AddGlobalOption(wxUSE_FILE "use wxFile class" 1)
AddGlobalOption(wxUSE_FILE_HISTORY "use wxFileHistory class" 1)
AddGlobalOption(wxUSE_FILESYSTEM "use virtual file systems classes" 1)
AddGlobalOption(wxUSE_FONTENUM "use wxFontEnumerator class" 1)
AddGlobalOption(wxUSE_FONTMAP "use font encodings conversion classes" 1)
AddGlobalOption(wxUSE_FS_ARCHIVE "use virtual archive filesystems" 1)
AddGlobalOption(wxUSE_FS_INET "use virtual HTTP/FTP filesystems" 1)
AddGlobalOption(wxUSE_FS_ZIP "now replaced by fs_archive" 1)
AddGlobalOption(wxUSE_FSVOLUME "use wxFSVolume class" 1)
AddGlobalOption(wxUSE_FSWATCHER "use wxFileSystemWatcher class" 1)
AddGlobalOption(wxUSE_GEOMETRY "use geometry class" 1)
AddGlobalOption(wxUSE_LOG "use logging system" 1)
AddGlobalOption(wxUSE_LONGLONG "use wxLongLong class" 1)
AddGlobalOption(wxUSE_MIMETYPE "use wxMimeTypesManager" 1)
AddGlobalOption(wxUSE_PRINTF_POS_PARAMS "use wxVsnprintf() which supports positional parameters" 1)
AddGlobalOption(wxUSE_SECRETSTORE "use wxSecretStore class" 1)
AddGlobalOption(wxUSE_SNGLINST_CHECKER "use wxSingleInstanceChecker class" 1)
AddGlobalOption(wxUSE_SOUND "use wxSound class" 1)
AddGlobalOption(wxUSE_STDPATHS "use wxStandardPaths class" 1)
AddGlobalOption(wxUSE_STOPWATCH "use wxStopWatch class" 1)
AddGlobalOption(wxUSE_STREAMS "use wxStream etc classes" 1)
AddGlobalOption(wxUSE_SYSTEM_OPTIONS "use wxSystemOptions" 1)
AddGlobalOption(wxUSE_TARSTREAM "use wxTar streams" 0) # FIXME: Windows
AddGlobalOption(wxUSE_TEXTBUFFER "use wxTextBuffer class" 1)
AddGlobalOption(wxUSE_TEXTFILE "use wxTextFile class" 1)
AddGlobalOption(wxUSE_TIMER "use wxTimer class" 1)
AddGlobalOption(wxUSE_VARIANT "use wxVariant class" 1)

# WebRequest options
AddGlobalOption(wxUSE_WEBREQUEST "use wxWebRequest class" 1)
if(WIN32)
    AddGlobalOption(wxUSE_WEBREQUEST_WINHTTP "use wxWebRequest WinHTTP backend" 1)
endif()
if(APPLE)
    AddGlobalOption(wxUSE_WEBREQUEST_URLSESSION "use wxWebRequest URLSession backend" 1)
endif()
if(APPLE OR WIN32)
    set(wxUSE_WEBREQUEST_CURL_DEFAULT 0)
else()
    set(wxUSE_WEBREQUEST_CURL_DEFAULT 1)
endif()

option(wxUSE_WEBREQUEST_CURL "use wxWebRequest libcurl backend" ${wxUSE_WEBREQUEST_CURL_DEFAULT})

AddGlobalOption(wxUSE_ZIPSTREAM "use wxZip streams" 0)

# URL-related classes
AddGlobalOption(wxUSE_URL "use wxURL class" 1)
AddGlobalOption(wxUSE_PROTOCOL "use wxProtocol class" 1)
AddGlobalOption(wxUSE_PROTOCOL_HTTP "HTTP support in wxProtocol" 1)
AddGlobalOption(wxUSE_PROTOCOL_FTP "FTP support in wxProtocol" 1)
AddGlobalOption(wxUSE_PROTOCOL_FILE "FILE support in wxProtocol" 1)

AddGlobalOption(wxUSE_THREADS "use threads" 1)

if(WIN32)
    if(MINGW)
        #TODO: version check, as newer versions have no problem enabling this
        set(wxUSE_DBGHELP_DEFAULT 0)
    else()
        set(wxUSE_DBGHELP_DEFAULT 1)
    endif()
    option(wxUSE_DBGHELP "use dbghelp.dll API" ${wxUSE_DBGHELP_DEFAULT})
    AddGlobalOption(wxUSE_INICONF "use wxIniConfig" 1)
    AddGlobalOption(wxUSE_WINSOCK2 "include <winsock2.h> rather than <winsock.h>" 0)
    AddGlobalOption(wxUSE_REGKEY "use wxRegKey class" 1)
endif()

if(wxUSE_GUI)

# ---------------------------------------------------------------------------
# optional "big" GUI features
# ---------------------------------------------------------------------------

AddGlobalOption(wxUSE_DOC_VIEW_ARCHITECTURE "use document view architecture" 1)
AddGlobalOption(wxUSE_HELP "use help subsystem" 1)
AddGlobalOption(wxUSE_MS_HTML_HELP "use MS HTML Help (win32)" 1)
AddGlobalOption(wxUSE_HTML "use wxHTML sub-library" 1)
AddGlobalOption(wxUSE_WXHTML_HELP "use wxHTML-based help" 1)
AddGlobalOption(wxUSE_XRC "use XRC resources sub-library" 1)
AddGlobalOption(wxUSE_XML "use the xml library (overruled by wxUSE_XRC)" 1)
AddGlobalOption(wxUSE_AUI "use AUI docking library" 1)
AddGlobalOption(wxUSE_PROPGRID "use wxPropertyGrid library" 1)
AddGlobalOption(wxUSE_RIBBON "use wxRibbon library" 1)
AddGlobalOption(wxUSE_STC "use wxStyledTextCtrl library" 1)
AddGlobalOption(wxUSE_CONSTRAINTS "use layout-constraints system" 1)
AddGlobalOption(wxUSE_LOGGUI "use standard GUI logger" 1)
AddGlobalOption(wxUSE_LOGWINDOW "use wxLogWindow" 1)
AddGlobalOption(wxUSE_LOG_DIALOG "use wxLogDialog" 1)
AddGlobalOption(wxUSE_MDI "use multiple document interface architecture" 1)
AddGlobalOption(wxUSE_MDI_ARCHITECTURE "use docview architecture with MDI" 1)
AddGlobalOption(wxUSE_MEDIACTRL "use wxMediaCtrl class" 1)
AddGlobalOption(wxUSE_RICHTEXT "use wxRichTextCtrl" 1)
AddGlobalOption(wxUSE_POSTSCRIPT "use wxPostscriptDC device context (default for gtk+)" 1)
AddGlobalOption(wxUSE_AFM_FOR_POSTSCRIPT "in wxPostScriptDC class use AFM (adobe font metrics) file for character widths" 1)
AddGlobalOption(wxUSE_PRINTING_ARCHITECTURE "use printing architecture" 1)
AddGlobalOption(wxUSE_SVG "use wxSVGFileDC device context" 1)
AddGlobalOption(wxUSE_WEBVIEW "use wxWebView library" 1)

# wxDC is implemented in terms of wxGraphicsContext in wxOSX so the latter
# can't be disabled, don't even provide an AddGlobalOption to do it
if(APPLE)
    set(wxUSE_GRAPHICS_CONTEXT 1)
else()
    AddGlobalOption(wxUSE_GRAPHICS_CONTEXT "use graphics context 2D drawing API")
    if(WIN32)
        AddGlobalOption(wxUSE_GRAPHICS_DIRECT2D "enable Direct2D graphics context")
    endif()
endif()

if(WXGTK)
    set(wxUSE_CAIRO_DEFAULT 1)
else()
    set(wxUSE_CAIRO_DEFAULT 0)
endif()
AddGlobalOption(wxUSE_CAIRO "enable Cairo graphics context" ${wxUSE_CAIRO_DEFAULT})

# ---------------------------------------------------------------------------
# IPC &c
# ---------------------------------------------------------------------------

AddGlobalOption(wxUSE_CLIPBOARD "use wxClipboard class" 1)
AddGlobalOption(wxUSE_DRAG_AND_DROP "use Drag'n'Drop classes" 1)

# ---------------------------------------------------------------------------
# optional GUI controls (in alphabetical order except the first one)
# ---------------------------------------------------------------------------

# don't set DEFAULT_wxUSE_XXX below if the option is not specified
# AddGlobalOption(wxUSE_CONTROLS "disable compilation of all standard controls") # TBR

# features affecting multiple controls
AddGlobalOption(wxUSE_MARKUP "support wxControl::SetLabelMarkup" 1)

# please keep the settings below in alphabetical order
AddGlobalOption(wxUSE_ACCEL "use accelerators" 1)
AddGlobalOption(wxUSE_ACTIVITYINDICATOR "use wxActivityIndicator class" 1)
AddGlobalOption(wxUSE_ADDREMOVECTRL "use wxAddRemoveCtrl" 1)
AddGlobalOption(wxUSE_ANIMATIONCTRL "use wxAnimationCtrl class" 1)
AddGlobalOption(wxUSE_BANNERWINDOW "use wxBannerWindow class" 1)
AddGlobalOption(wxUSE_ARTPROVIDER_STD "use standard XPM icons in wxArtProvider" 1)
AddGlobalOption(wxUSE_ARTPROVIDER_TANGO "use Tango icons in wxArtProvider" 1)
AddGlobalOption(wxUSE_BMPBUTTON "use wxBitmapButton class" 1)
AddGlobalOption(wxUSE_BITMAPCOMBOBOX "use wxBitmapComboBox class" 1)
AddGlobalOption(wxUSE_BUTTON "use wxButton class" 1)
AddGlobalOption(wxUSE_CALENDARCTRL "use wxCalendarCtrl class" 1)
AddGlobalOption(wxUSE_CARET "use wxCaret class" 1)
AddGlobalOption(wxUSE_CHECKBOX "use wxCheckBox class" 1)
AddGlobalOption(wxUSE_CHECKLISTBOX "use wxCheckListBox (listbox with checkboxes) class" 1)
AddGlobalOption(wxUSE_CHOICE "use wxChoice class" 1)
AddGlobalOption(wxUSE_CHOICEBOOK "use wxChoicebook class" 1)
AddGlobalOption(wxUSE_COLLPANE "use wxCollapsiblePane class" 1)
AddGlobalOption(wxUSE_COLOURPICKERCTRL "use wxColourPickerCtrl class" 1)
AddGlobalOption(wxUSE_COMBOBOX "use wxComboBox class" 1)
AddGlobalOption(wxUSE_COMBOCTRL "use wxComboCtrl class" 1)
AddGlobalOption(wxUSE_COMMANDLINKBUTTON "use wxCommmandLinkButton class" 1)
AddGlobalOption(wxUSE_DATAVIEWCTRL "use wxDataViewCtrl class" 1)
AddGlobalOption(wxUSE_NATIVE_DATAVIEWCTRL "use the native wxDataViewCtrl if available" 1)
AddGlobalOption(wxUSE_DATEPICKCTRL "use wxDatePickerCtrl class" 1)
AddGlobalOption(wxUSE_DETECT_SM "use code to detect X11 session manager" 0)
AddGlobalOption(wxUSE_DIRPICKERCTRL "use wxDirPickerCtrl class" 1)
AddGlobalOption(wxUSE_DISPLAY "use wxDisplay class" 1)
AddGlobalOption(wxUSE_EDITABLELISTBOX "use wxEditableListBox class" 1)
AddGlobalOption(wxUSE_FILECTRL "use wxFileCtrl class" 1)
AddGlobalOption(wxUSE_FILEPICKERCTRL "use wxFilePickerCtrl class" 1)
AddGlobalOption(wxUSE_FONTPICKERCTRL "use wxFontPickerCtrl class" 1)
AddGlobalOption(wxUSE_GAUGE "use wxGauge class" 1)
AddGlobalOption(wxUSE_GRID "use wxGrid class" 1)
AddGlobalOption(wxUSE_HEADERCTRL "use wxHeaderCtrl class" 1)
AddGlobalOption(wxUSE_HYPERLINKCTRL "use wxHyperlinkCtrl class" 1)
AddGlobalOption(wxUSE_IMAGLIST "use wxImageList class" 1)
AddGlobalOption(wxUSE_INFOBAR "use wxInfoBar class" 1)
AddGlobalOption(wxUSE_LISTBOOK "use wxListbook class" 1)
AddGlobalOption(wxUSE_LISTBOX "use wxListBox class" 1)
AddGlobalOption(wxUSE_LISTCTRL "use wxListCtrl class" 1)
AddGlobalOption(wxUSE_NOTEBOOK "use wxNotebook class" 1)
AddGlobalOption(wxUSE_NOTIFICATION_MESSAGE "use wxNotificationMessage class" 1)
AddGlobalOption(wxUSE_ODCOMBOBOX "use wxOwnerDrawnComboBox class" 1)
AddGlobalOption(wxUSE_POPUPWIN "use wxPopUpWindow class" 1)
AddGlobalOption(wxUSE_PREFERENCES_EDITOR "use wxPreferencesEditor class" 1)
AddGlobalOption(wxUSE_RADIOBOX "use wxRadioBox class" 1)
AddGlobalOption(wxUSE_RADIOBTN "use wxRadioButton class" 1)
AddGlobalOption(wxUSE_RICHMSGDLG "use wxRichMessageDialog class" 1)
AddGlobalOption(wxUSE_RICHTOOLTIP "use wxRichToolTip class" 1)
AddGlobalOption(wxUSE_REARRANGECTRL "use wxRearrangeList/Ctrl/Dialog" 1)
AddGlobalOption(wxUSE_SASH "use wxSashWindow class" 1)
AddGlobalOption(wxUSE_SCROLLBAR "use wxScrollBar class and scrollable windows" 1)
AddGlobalOption(wxUSE_SEARCHCTRL "use wxSearchCtrl class" 1)
AddGlobalOption(wxUSE_SLIDER "use wxSlider class" 1)
AddGlobalOption(wxUSE_SPINBTN "use wxSpinButton class" 1)
AddGlobalOption(wxUSE_SPINCTRL "use wxSpinCtrl class" 1)
AddGlobalOption(wxUSE_SPLITTER "use wxSplitterWindow class" 1)
AddGlobalOption(wxUSE_STATBMP "use wxStaticBitmap class" 1)
AddGlobalOption(wxUSE_STATBOX "use wxStaticBox class" 1)
AddGlobalOption(wxUSE_STATLINE "use wxStaticLine class" 1)
AddGlobalOption(wxUSE_STATTEXT "use wxStaticText class" 1)
AddGlobalOption(wxUSE_STATUSBAR "use wxStatusBar class" 1)
AddGlobalOption(wxUSE_TASKBARBUTTON "use wxTaskBarButton class" 1)
AddGlobalOption(wxUSE_TASKBARICON "use wxTaskBarIcon class" 1)
AddGlobalOption(wxUSE_TOOLBAR_NATIVE "use native wxToolBar class" 1)
AddGlobalOption(wxUSE_TEXTCTRL "use wxTextCtrl class" 1)
if(wxUSE_TEXTCTRL)
    # we don't have special switches to disable wxUSE_RICHEDIT[2], it doesn't
    # seem useful to allow disabling them
    set(wxUSE_RICHEDIT 1)
    set(wxUSE_RICHEDIT2 1)
endif()
AddGlobalOption(wxUSE_TIMEPICKCTRL "use wxTimePickerCtrl class" 1)
AddGlobalOption(wxUSE_TIPWINDOW "use wxTipWindow class" 1)
AddGlobalOption(wxUSE_TOGGLEBTN "use wxToggleButton class" 1)
AddGlobalOption(wxUSE_TOOLBAR "use wxToolBar class" 1)
AddGlobalOption(wxUSE_TOOLBOOK "use wxToolbook class" 1)
AddGlobalOption(wxUSE_TREEBOOK "use wxTreebook class" 1)
AddGlobalOption(wxUSE_TREECTRL "use wxTreeCtrl class" 1)
AddGlobalOption(wxUSE_TREELISTCTRL "use wxTreeListCtrl class" 1)

# ---------------------------------------------------------------------------
# common dialogs
# ---------------------------------------------------------------------------

AddGlobalOption(wxUSE_COMMON_DIALOGS "use all common dialogs" 1)
AddGlobalOption(wxUSE_ABOUTDLG "use wxAboutBox" 1)
AddGlobalOption(wxUSE_CHOICEDLG "use wxChoiceDialog" 1)
AddGlobalOption(wxUSE_COLOURDLG "use wxColourDialog" 1)
AddGlobalOption(wxUSE_CREDENTIALDLG "use wxCredentialEntryDialog" 1)
AddGlobalOption(wxUSE_FILEDLG "use wxFileDialog" 1)
AddGlobalOption(wxUSE_FINDREPLDLG "use wxFindReplaceDialog" 1)
AddGlobalOption(wxUSE_FONTDLG "use wxFontDialog" 1)
AddGlobalOption(wxUSE_DIRDLG "use wxDirDialog" 1)
AddGlobalOption(wxUSE_MSGDLG "use wxMessageDialog" 1)
AddGlobalOption(wxUSE_NUMBERDLG "use wxNumberEntryDialog" 1)
AddGlobalOption(wxUSE_SPLASH "use wxSplashScreen" 1)
AddGlobalOption(wxUSE_TEXTDLG "use wxTextDialog" 1)
AddGlobalOption(wxUSE_STARTUP_TIPS "use startup tips" 1)
AddGlobalOption(wxUSE_PROGRESSDLG "use wxProgressDialog" 1)
AddGlobalOption(wxUSE_WIZARDDLG "use wxWizard" 1)

# ---------------------------------------------------------------------------
# misc GUI options
# ---------------------------------------------------------------------------

AddGlobalOption(wxUSE_MENUS "use wxMenu and wxMenuItem classes" 1)
AddGlobalOption(wxUSE_MENUBAR "use wxMenuBar class" 1)
AddGlobalOption(wxUSE_MINIFRAME "use wxMiniFrame class" 1)
AddGlobalOption(wxUSE_TOOLTIPS "use wxToolTip class" 1)
AddGlobalOption(wxUSE_SPLINES "use spline drawing code" 1)
AddGlobalOption(wxUSE_MOUSEWHEEL "use mousewheel" 1)
AddGlobalOption(wxUSE_VALIDATORS "use wxValidator and derived classes" 1)
AddGlobalOption(wxUSE_BUSYINFO "use wxBusyInfo" 1)
AddGlobalOption(wxUSE_HOTKEY "use wxWindow::RegisterHotKey()" 1)
AddGlobalOption(wxUSE_JOYSTICK "use wxJoystick" 1)
AddGlobalOption(wxUSE_METAFILE "use wxMetaFile" 1)
AddGlobalOption(wxUSE_DRAGIMAGE "use wxDragImage" 1)
AddGlobalOption(wxUSE_UIACTIONSIMULATOR "use wxUIActionSimulator (experimental)" 1)
AddGlobalOption(wxUSE_DC_TRANSFORM_MATRIX "use wxDC::SetTransformMatrix and related" 1)
AddGlobalOption(wxUSE_WEBVIEW_WEBKIT "use wxWebView WebKit backend" 1)
if(WIN32 OR APPLE)
    set(wxUSE_PRIVATE_FONTS_DEFAULT 1)
else()
    set(wxUSE_PRIVATE_FONTS_DEFAULT 0)
endif()
AddGlobalOption(wxUSE_PRIVATE_FONTS "use fonts not installed on the system" ${wxUSE_PRIVATE_FONTS_DEFAULT})

# ---------------------------------------------------------------------------
# support for image formats that do not rely on external library
# ---------------------------------------------------------------------------

AddGlobalOption(wxUSE_PALETTE "use wxPalette class" 1)
AddGlobalOption(wxUSE_IMAGE "use wxImage class" 1)
AddGlobalOption(wxUSE_GIF "use gif images (GIF file format)" 1)
AddGlobalOption(wxUSE_PCX "use pcx images (PCX file format)" 1)
AddGlobalOption(wxUSE_TGA "use tga images (TGA file format)" 1)
AddGlobalOption(wxUSE_IFF "use iff images (IFF file format)" 1)
AddGlobalOption(wxUSE_PNM "use pnm images (PNM file format)" 1)
AddGlobalOption(wxUSE_XPM "use xpm images (XPM file format)" 1)
AddGlobalOption(wxUSE_ICO_CUR "use Windows ICO and CUR formats" 1)

# ---------------------------------------------------------------------------
# wxMSW-only options
# ---------------------------------------------------------------------------

if(WIN32)
    if(MSVC_VERSION GREATER 1600 AND NOT CMAKE_VS_PLATFORM_TOOLSET MATCHES "_xp$")
        set(wxUSE_WINRT_DEFAULT 1)
    else()
        set(wxUSE_WINRT_DEFAULT 0)
    endif()
    if(MSVC_VERSION GREATER 1800 AND NOT CMAKE_VS_PLATFORM_TOOLSET MATCHES "_xp$" AND
        EXISTS "${wxSOURCE_DIR}/3rdparty/webview2")
        set(wxUSE_WEBVIEW_EDGE_DEFAULT 1)
    else()
        set(wxUSE_WEBVIEW_EDGE_DEFAULT 0)
    endif()

    AddGlobalOption(wxUSE_ACCESSIBILITY "enable accessibility support" 1)
    AddGlobalOption(wxUSE_ACTIVEX " enable wxActiveXContainer class (Win32 only)" 1)
    AddGlobalOption(wxUSE_CRASHREPORT "enable wxCrashReport::Generate() to create mini dumps (Win32 only)" 1)
    AddGlobalOption(wxUSE_DC_CACHEING "cache temporary wxDC objects (Win32 only)" 1)
    AddGlobalOption(wxUSE_NATIVE_PROGRESSDLG "use native progress dialog implementation" 1)
    AddGlobalOption(wxUSE_NATIVE_STATUSBAR "use native statusbar implementation)" 1)
    AddGlobalOption(wxUSE_OWNER_DRAWN "use owner drawn controls (Win32)" 1)
    AddGlobalOption(wxUSE_POSTSCRIPT_ARCHITECTURE_IN_MSW "use PS printing in wxMSW (Win32 only)" 1)
    AddGlobalOption(wxUSE_TASKBARICON_BALLOONS "enable wxTaskBarIcon::ShowBalloon() method (Win32 only)" 1)
    AddGlobalOption(wxUSE_UXTHEME "enable support for Windows XP themed look (Win32 only)" 1)
    AddGlobalOption(wxUSE_WEBVIEW_EDGE "use wxWebView Edge (Chromium) backend (Windows 7+ only)" ${wxUSE_WEBVIEW_EDGE_DEFAULT})
    AddGlobalOption(wxUSE_WEBVIEW_IE "use wxWebView IE backend (Win32 only)" 0) #TBR
    AddGlobalOption(wxUSE_WINRT "enable WinRT support" ${wxUSE_WINRT_DEFAULT})
    AddGlobalOption(wxUSE_WXDIB "use wxDIB class (Win32 only)" 1)
endif()

# this one is not really MSW-specific but it exists mainly to be turned 0
# under MSW, it should be 0 by default on the other platforms
if(WIN32)
    set(wxDEFAULT_wxUSE_AUTOID_MANAGEMENT 1)
else()
    set(wxDEFAULT_wxUSE_AUTOID_MANAGEMENT 0)
endif()

AddGlobalOption(wxUSE_AUTOID_MANAGEMENT "use automatic ids management" ${wxDEFAULT_wxUSE_AUTOID_MANAGEMENT})

endif() # wxUSE_GUI
