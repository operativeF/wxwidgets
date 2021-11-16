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
    option(${OptionName} ${Description} ${DefaultValue})
    if(${DefaultValue})
        add_compile_definitions(${OptionName}=1)
    else() # Option is disabled
        add_compile_definitions(${OptionName}=0)
    endif()
endfunction()

# Global build options
option(wxBUILD_SHARED "Build wx libraries as shared libs" ${BUILD_SHARED_LIBS})

AddGlobalOption(wxBUILD_MONOLITHIC "build wxWidgets as single library" OFF)
AddGlobalOption(wxBUILD_SAMPLES "Build only important samples (SOME) or ALL" OFF)
AddGlobalOption(wxBUILD_TESTS "Build console tests (CONSOLE_ONLY) or ALL" OFF)
AddGlobalOption(wxBUILD_DEMOS "Build demos" OFF)
AddGlobalOption(wxBUILD_BENCHMARKS "Build benchmarks" OFF)
AddGlobalOption(wxBUILD_PRECOMP "Use precompiled headers" OFF)
AddGlobalOption(wxBUILD_INSTALL "Create install/uninstall target for wxWidgets" OFF)
AddGlobalOption(wxUSE_GUI "Build with GUI" ON)
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
    AddGlobalOption(wxBUILD_USE_STATIC_RUNTIME "Link using the static runtime library" ON)
    mark_as_advanced(wxBUILD_USE_STATIC_RUNTIME)
endif()

if(WIN32)
    set(wxBUILD_VENDOR "custom" CACHE STRING "Short string identifying your company (used in DLL name)")
endif()
set(wxBUILD_FLAVOUR "" CACHE STRING "Specify a name to identify the build")
mark_as_advanced(wxBUILD_FLAVOUR)

AddGlobalOption(wxBUILD_OPTIMISE "use speed-optimised C/C++ compiler flags for release build" ON)
mark_as_advanced(wxBUILD_OPTIMISE)

if(MSVC)
    set(wxBUILD_STRIPPED_RELEASE_DEFAULT OFF)
else()
    set(wxBUILD_STRIPPED_RELEASE_DEFAULT ON)
endif()

AddGlobalOption(wxBUILD_STRIPPED_RELEASE "remove debug symbols in release build" ${wxBUILD_STRIPPED_RELEASE_DEFAULT})
mark_as_advanced(wxBUILD_STRIPPED_RELEASE)
AddGlobalOption(wxBUILD_PIC "Enable position independent code (PIC)." ON)
mark_as_advanced(wxBUILD_PIC)
AddGlobalOption(wxUSE_NO_RTTI "disable RTTI support" OFF)

# STL options
AddGlobalOption(wxUSE_STL "use standard C++ classes for everything" ON) # TBR
AddGlobalOption(wxUSE_STD_CONTAINERS "use standard C++ container classes" ON) # TBR
set(wxTHIRD_PARTY_LIBRARIES ${wxTHIRD_PARTY_LIBRARIES} wxUSE_STL "use C++ STL classes")

# TBR
AddGlobalOption(wxUSE_UNICODE "compile with Unicode support (NOT RECOMMENDED to be turned OFF)" ON)
if(NOT WIN32)
    AddGlobalOption(wxUSE_UNICODE_UTF8 "use UTF-8 representation for strings (Unix only)" OFF)
    wx_dependent_option(wxUSE_UTF8_LOCALE_ONLY "only support UTF-8 locales in UTF-8 build (Unix only)" 1 "wxUSE_UNICODE_UTF8" OFF)
endif()

AddGlobalOption(wxUSE_COMPILER_TLS "enable use of compiler TLS support" ON)

if(NOT WIN32)
    AddGlobalOption(wxUSE_VISIBILITY "use of ELF symbols visibility")
endif()

AddGlobalOption(wxUSE_UNSAFE_WXSTRING_CONV "provide unsafe implicit conversions in wxString to const char* or std::string" ON)
AddGlobalOption(wxUSE_REPRODUCIBLE_BUILD "enable reproducable build" OFF)

# ---------------------------------------------------------------------------
# external libraries
# ---------------------------------------------------------------------------

# FIXME: Use extern library or use vcpkg
#wx_add_thirdparty_library(wxUSE_REGEX REGEX "enable support for wxRegEx class" DEFAULT builtin)
#wx_add_thirdparty_library(wxUSE_ZLIB ZLIB "use zlib for LZW compression" DEFAULT_APPLE sys)
#wx_add_thirdparty_library(wxUSE_EXPAT EXPAT "use expat for XML parsing" DEFAULT_APPLE sys)

AddGlobalOption(wxUSE_LIBPNG "Use libpng (PNG image format)" ON)
AddGlobalOption(wxUSE_LIBTIFF "Use libtiff (TIFF image format)" ON)
AddGlobalOption(wxUSE_LIBJPEG "Use libjpeg (JPEG file format)" ON)

AddGlobalOption(wxUSE_LIBLZMA "use LZMA compression" OFF)
set(wxTHIRD_PARTY_LIBRARIES ${wxTHIRD_PARTY_LIBRARIES} wxUSE_LIBLZMA "use liblzma for LZMA compression")

AddGlobalOption(wxUSE_OPENGL "use OpenGL (or Mesa)" ON)

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
AddGlobalOption(wxUSE_INTL "use internationalization system" ON)
AddGlobalOption(wxUSE_XLOCALE "use x-locale support (requires wxLocale)" ON)
AddGlobalOption(wxUSE_CONFIG "use wxConfig (and derived) classes" ON)

AddGlobalOption(wxUSE_FILECONFIG "Use file configuration classes." ON)

AddGlobalOption(wxUSE_REGISTRY "Use Windows Registry for configurations." OFF)

AddGlobalOption(wxUSE_SOCKETS "use socket/network classes" OFF)
AddGlobalOption(wxUSE_IPV6 "enable IPv6 support in wxSocket" ON)
if(WIN32)
    AddGlobalOption(wxUSE_OLE "use OLE classes" ON)
endif()
AddGlobalOption(wxUSE_DATAOBJ "use data object classes" ON)

AddGlobalOption(wxUSE_IPC "use interprocess communication (wxSocket etc.)" OFF) # FIXME: Requires Registry

AddGlobalOption(wxUSE_CONSOLE_EVENTLOOP "use event loop in console programs too" ON)

# please keep the settings below in alphabetical order
AddGlobalOption(wxUSE_ANY "Use wxAny class" ON)
AddGlobalOption(wxUSE_ARCHIVE_STREAMS "use wxArchive streams" ON)
AddGlobalOption(wxUSE_BASE64 "use base64 encoding/decoding functions" ON)
AddGlobalOption(wxUSE_STACKWALKER "use wxStackWalker class for getting backtraces" OFF)
AddGlobalOption(wxUSE_ON_FATAL_EXCEPTION "catch signals in wxApp::OnFatalException" ON)
AddGlobalOption(wxUSE_CMDLINE_PARSER "use wxCmdLineParser class" ON)
AddGlobalOption(wxUSE_DATETIME "use wxDateTime class" ON)
AddGlobalOption(wxUSE_DEBUGREPORT "use wxDebugReport class" ON)

AddGlobalOption(wxUSE_DYNLIB_CLASS "use wxLibrary class for DLL loading" ON)
AddGlobalOption(wxUSE_DYNAMIC_LOADER "use (new) wxDynamicLibrary class" ON)
AddGlobalOption(wxUSE_EXCEPTIONS "build exception-safe library" ON)
AddGlobalOption(wxUSE_EXTENDED_RTTI "use extended RTTI (XTI)" OFF)
AddGlobalOption(wxUSE_FFILE "use wxFFile class" ON)
AddGlobalOption(wxUSE_FILE "use wxFile class" ON)
AddGlobalOption(wxUSE_FILE_HISTORY "use wxFileHistory class" ON)
AddGlobalOption(wxUSE_FILESYSTEM "use virtual file systems classes" ON)
AddGlobalOption(wxUSE_FONTENUM "use wxFontEnumerator class" ON)
AddGlobalOption(wxUSE_FONTMAP "use font encodings conversion classes" ON)
AddGlobalOption(wxUSE_FS_ARCHIVE "use virtual archive filesystems" ON)
AddGlobalOption(wxUSE_FS_INET "use virtual HTTP/FTP filesystems" ON)
AddGlobalOption(wxUSE_FS_ZIP "now replaced by fs_archive" ON)
AddGlobalOption(wxUSE_FSVOLUME "use wxFSVolume class" ON)
AddGlobalOption(wxUSE_FSWATCHER "use wxFileSystemWatcher class" ON)
AddGlobalOption(wxUSE_GEOMETRY "use geometry class" ON)
AddGlobalOption(wxUSE_LOG "use logging system" ON)
AddGlobalOption(wxUSE_LONGLONG "use wxLongLong class" ON)
AddGlobalOption(wxUSE_MIMETYPE "use wxMimeTypesManager" ON) # FIXME: Relies on Registry.
AddGlobalOption(wxUSE_PRINTF_POS_PARAMS "use wxVsnprintf() which supports positional parameters" ON)
AddGlobalOption(wxUSE_SECRETSTORE "use wxSecretStore class" ON)
AddGlobalOption(wxUSE_SNGLINST_CHECKER "use wxSingleInstanceChecker class" ON)
AddGlobalOption(wxUSE_SOUND "use wxSound class" ON)
AddGlobalOption(wxUSE_STDPATHS "use wxStandardPaths class" ON)
AddGlobalOption(wxUSE_STOPWATCH "use wxStopWatch class" ON)
AddGlobalOption(wxUSE_STREAMS "use wxStream etc classes" ON)
AddGlobalOption(wxUSE_SYSTEM_OPTIONS "use wxSystemOptions" ON)
AddGlobalOption(wxUSE_TARSTREAM "use wxTar streams" OFF) # FIXME: Windows
AddGlobalOption(wxUSE_TEXTBUFFER "use wxTextBuffer class" ON)
AddGlobalOption(wxUSE_TEXTFILE "use wxTextFile class" ON)
AddGlobalOption(wxUSE_TIMER "use wxTimer class" ON)
AddGlobalOption(wxUSE_VARIANT "use wxVariant class" OFF)

# WebRequest options
AddGlobalOption(wxUSE_WEBREQUEST "use wxWebRequest class" ON)
if(WIN32)
    AddGlobalOption(wxUSE_WEBREQUEST_WINHTTP "use wxWebRequest WinHTTP backend" ON)
endif()
if(APPLE)
    AddGlobalOption(wxUSE_WEBREQUEST_URLSESSION "use wxWebRequest URLSession backend" ON)
endif()
if(APPLE OR WIN32)
    set(wxUSE_WEBREQUEST_CURL_DEFAULT OFF)
else()
    set(wxUSE_WEBREQUEST_CURL_DEFAULT ON)
endif()

option(wxUSE_WEBREQUEST_CURL "use wxWebRequest libcurl backend" ${wxUSE_WEBREQUEST_CURL_DEFAULT})

AddGlobalOption(wxUSE_ZIPSTREAM "use wxZip streams" OFF)

# URL-related classes
AddGlobalOption(wxUSE_URL "use wxURL class" OFF)
AddGlobalOption(wxUSE_PROTOCOL "use wxProtocol class" OFF)
AddGlobalOption(wxUSE_PROTOCOL_HTTP "HTTP support in wxProtocol" OFF)
AddGlobalOption(wxUSE_PROTOCOL_FTP "FTP support in wxProtocol" OFF)
AddGlobalOption(wxUSE_PROTOCOL_FILE "FILE support in wxProtocol" OFF)

AddGlobalOption(wxUSE_THREADS "use threads" ON)

if(WIN32)
    if(MINGW)
        #TODO: version check, as newer versions have no problem enabling this
        set(wxUSE_DBGHELP_DEFAULT OFF)
    else()
        set(wxUSE_DBGHELP_DEFAULT ON)
    endif()
    option(wxUSE_DBGHELP "use dbghelp.dll API" ${wxUSE_DBGHELP_DEFAULT})
    AddGlobalOption(wxUSE_INICONF "use wxIniConfig" OFF)
    AddGlobalOption(wxUSE_WINSOCK2 "include <winsock2.h> rather than <winsock.h>" OFF)
    AddGlobalOption(wxUSE_REGKEY "use wxRegKey class" ON)
endif()

if(wxUSE_GUI)

# ---------------------------------------------------------------------------
# optional "big" GUI features
# ---------------------------------------------------------------------------

AddGlobalOption(wxUSE_DOC_VIEW_ARCHITECTURE "use document view architecture" ON)
AddGlobalOption(wxUSE_HELP "use help subsystem" ON)
AddGlobalOption(wxUSE_MS_HTML_HELP "use MS HTML Help (win32)" ON)
AddGlobalOption(wxUSE_HTML "use wxHTML sub-library" ON)
AddGlobalOption(wxUSE_WXHTML_HELP "use wxHTML-based help" ON)
AddGlobalOption(wxUSE_XRC "use XRC resources sub-library" OFF)
AddGlobalOption(wxUSE_XML "use the xml library (overruled by wxUSE_XRC)" ON)
AddGlobalOption(wxUSE_AUI "use AUI docking library" ON)
AddGlobalOption(wxUSE_PROPGRID "use wxPropertyGrid library" ON)
AddGlobalOption(wxUSE_RIBBON "use wxRibbon library" ON)
AddGlobalOption(wxUSE_STC "use wxStyledTextCtrl library" ON)
AddGlobalOption(wxUSE_CONSTRAINTS "use layout-constraints system" ON)
AddGlobalOption(wxUSE_LOGGUI "use standard GUI logger" ON)
AddGlobalOption(wxUSE_LOGWINDOW "use wxLogWindow" ON)
AddGlobalOption(wxUSE_LOG_DIALOG "use wxLogDialog" ON)
AddGlobalOption(wxUSE_MDI "use multiple document interface architecture" ON)
AddGlobalOption(wxUSE_MDI_ARCHITECTURE "use docview architecture with MDI" ON)
AddGlobalOption(wxUSE_MEDIACTRL "use wxMediaCtrl class" ON)
AddGlobalOption(wxUSE_RICHTEXT "use wxRichTextCtrl" ON)
AddGlobalOption(wxUSE_POSTSCRIPT "use wxPostscriptDC device context (default for gtk+)" OFF)
AddGlobalOption(wxUSE_AFM_FOR_POSTSCRIPT "in wxPostScriptDC class use AFM (adobe font metrics) file for character widths" ON)
AddGlobalOption(wxUSE_PRINTING_ARCHITECTURE "use printing architecture" ON)
AddGlobalOption(wxUSE_SVG "use wxSVGFileDC device context" ON)
AddGlobalOption(wxUSE_WEBVIEW "use wxWebView library" ON)

# wxDC is implemented in terms of wxGraphicsContext in wxOSX so the latter
# can't be disabled, don't even provide an AddGlobalOption to do it
if(APPLE)
    set(wxUSE_GRAPHICS_CONTEXT ON)
else()
    AddGlobalOption(wxUSE_GRAPHICS_CONTEXT "use graphics context 2D drawing API" ON)

    if(WIN32)
        AddGlobalOption(wxUSE_GRAPHICS_DIRECT2D "enable Direct2D graphics context" ON)
    endif()
endif()

if(WXGTK)
    set(wxUSE_CAIRO_DEFAULT ON)
else()
    set(wxUSE_CAIRO_DEFAULT OFF)
endif()
AddGlobalOption(wxUSE_CAIRO "enable Cairo graphics context" ${wxUSE_CAIRO_DEFAULT})

# ---------------------------------------------------------------------------
# IPC &c
# ---------------------------------------------------------------------------

AddGlobalOption(wxUSE_CLIPBOARD "use wxClipboard class" ON)
AddGlobalOption(wxUSE_DRAG_AND_DROP "use Drag'n'Drop classes" ON)

# ---------------------------------------------------------------------------
# optional GUI controls (in alphabetical order except the first one)
# ---------------------------------------------------------------------------

# don't set DEFAULT_wxUSE_XXX below if the option is not specified
AddGlobalOption(wxUSE_CONTROLS "disable compilation of all standard controls" ON) # TBR

# features affecting multiple controls
AddGlobalOption(wxUSE_MARKUP "support wxControl::SetLabelMarkup" ON)

# please keep the settings below in alphabetical order
AddGlobalOption(wxUSE_ACCEL "use accelerators" ON)
AddGlobalOption(wxUSE_ACTIVITYINDICATOR "use wxActivityIndicator class" ON)
AddGlobalOption(wxUSE_ADDREMOVECTRL "use wxAddRemoveCtrl" ON)
AddGlobalOption(wxUSE_ANIMATIONCTRL "use wxAnimationCtrl class" ON)
AddGlobalOption(wxUSE_BANNERWINDOW "use wxBannerWindow class" ON)
AddGlobalOption(wxUSE_BOOKCTRL "Use book control class" ON)
AddGlobalOption(wxUSE_ARTPROVIDER_STD "use standard XPM icons in wxArtProvider" ON)
AddGlobalOption(wxUSE_ARTPROVIDER_TANGO "use Tango icons in wxArtProvider" ON)
AddGlobalOption(wxUSE_BMPBUTTON "use wxBitmapButton class" ON)
AddGlobalOption(wxUSE_BITMAPCOMBOBOX "use wxBitmapComboBox class" ON)
AddGlobalOption(wxUSE_BUTTON "use wxButton class" ON)
AddGlobalOption(wxUSE_CALENDARCTRL "use wxCalendarCtrl class" ON)
AddGlobalOption(wxUSE_CARET "use wxCaret class" ON)
AddGlobalOption(wxUSE_CHECKBOX "use wxCheckBox class" ON)
AddGlobalOption(wxUSE_CHECKLISTBOX "use wxCheckListBox (listbox with checkboxes) class" ON)
AddGlobalOption(wxUSE_CHOICE "use wxChoice class" ON)
AddGlobalOption(wxUSE_CHOICEBOOK "use wxChoicebook class" ON)
AddGlobalOption(wxUSE_COLLPANE "use wxCollapsiblePane class" ON)
AddGlobalOption(wxUSE_COLOURPICKERCTRL "use wxColourPickerCtrl class" ON)
AddGlobalOption(wxUSE_COMBOBOX "use wxComboBox class" ON)
AddGlobalOption(wxUSE_COMBOCTRL "use wxComboCtrl class" ON)
AddGlobalOption(wxUSE_COMMANDLINKBUTTON "use wxCommmandLinkButton class" ON)
AddGlobalOption(wxUSE_DATAVIEWCTRL "use wxDataViewCtrl class" OFF)
AddGlobalOption(wxUSE_NATIVE_DATAVIEWCTRL "use the native wxDataViewCtrl if available" OFF)
AddGlobalOption(wxUSE_DATEPICKCTRL "use wxDatePickerCtrl class" ON)
AddGlobalOption(wxUSE_DETECT_SM "use code to detect X11 session manager" OFF)
AddGlobalOption(wxUSE_DIRPICKERCTRL "use wxDirPickerCtrl class" ON)
AddGlobalOption(wxUSE_DISPLAY "use wxDisplay class" ON)
AddGlobalOption(wxUSE_EDITABLELISTBOX "use wxEditableListBox class" ON)
AddGlobalOption(wxUSE_FILECTRL "use wxFileCtrl class" ON)
AddGlobalOption(wxUSE_FILEPICKERCTRL "use wxFilePickerCtrl class" ON)
AddGlobalOption(wxUSE_FONTPICKERCTRL "use wxFontPickerCtrl class" ON)
AddGlobalOption(wxUSE_GAUGE "use wxGauge class" ON)
AddGlobalOption(wxUSE_GRID "use wxGrid class" ON)
AddGlobalOption(wxUSE_HEADERCTRL "use wxHeaderCtrl class" ON)
AddGlobalOption(wxUSE_HYPERLINKCTRL "use wxHyperlinkCtrl class" ON)
AddGlobalOption(wxUSE_IMAGLIST "use wxImageList class" ON)
AddGlobalOption(wxUSE_INFOBAR "use wxInfoBar class" ON)
AddGlobalOption(wxUSE_LISTBOOK "use wxListbook class" ON)
AddGlobalOption(wxUSE_LISTBOX "use wxListBox class" ON)
AddGlobalOption(wxUSE_LISTCTRL "use wxListCtrl class" ON)
AddGlobalOption(wxUSE_NOTEBOOK "use wxNotebook class" ON)
AddGlobalOption(wxUSE_NOTIFICATION_MESSAGE "use wxNotificationMessage class" ON)
AddGlobalOption(wxUSE_ODCOMBOBOX "use wxOwnerDrawnComboBox class" ON)
AddGlobalOption(wxUSE_POPUPWIN "use wxPopUpWindow class" ON)
AddGlobalOption(wxUSE_PREFERENCES_EDITOR "use wxPreferencesEditor class" ON)
AddGlobalOption(wxUSE_RADIOBOX "use wxRadioBox class" ON)
AddGlobalOption(wxUSE_RADIOBTN "use wxRadioButton class" ON)
AddGlobalOption(wxUSE_RICHMSGDLG "use wxRichMessageDialog class" ON)
AddGlobalOption(wxUSE_RICHTOOLTIP "use wxRichToolTip class" ON)
AddGlobalOption(wxUSE_REARRANGECTRL "use wxRearrangeList/Ctrl/Dialog" ON)
AddGlobalOption(wxUSE_SASH "use wxSashWindow class" ON)
AddGlobalOption(wxUSE_SCROLLBAR "use wxScrollBar class and scrollable windows" ON)
AddGlobalOption(wxUSE_SEARCHCTRL "use wxSearchCtrl class" ON)
AddGlobalOption(wxUSE_SLIDER "use wxSlider class" ON)
AddGlobalOption(wxUSE_SPINBTN "use wxSpinButton class" ON)
AddGlobalOption(wxUSE_SPINCTRL "use wxSpinCtrl class" ON)
AddGlobalOption(wxUSE_SPLITTER "use wxSplitterWindow class" ON)
AddGlobalOption(wxUSE_STATBMP "use wxStaticBitmap class" ON)
AddGlobalOption(wxUSE_STATBOX "use wxStaticBox class" ON)
AddGlobalOption(wxUSE_STATLINE "use wxStaticLine class" ON)
AddGlobalOption(wxUSE_STATTEXT "use wxStaticText class" ON)
AddGlobalOption(wxUSE_STATUSBAR "use wxStatusBar class" ON)
AddGlobalOption(wxUSE_TASKBARBUTTON "use wxTaskBarButton class" ON)
AddGlobalOption(wxUSE_TASKBARICON "use wxTaskBarIcon class" ON)
AddGlobalOption(wxUSE_TOOLBAR_NATIVE "use native wxToolBar class" ON)
AddGlobalOption(wxUSE_TEXTCTRL "use wxTextCtrl class" ON)
if(wxUSE_TEXTCTRL)
    # we don't have special switches to disable wxUSE_RICHEDIT[2], it doesn't
    # seem useful to allow disabling them
    set(wxUSE_RICHEDIT 1)
    set(wxUSE_RICHEDIT2 1)
endif()

AddGlobalOption(wxUSE_TIMEPICKCTRL "use wxTimePickerCtrl class" ON)
AddGlobalOption(wxUSE_TIPWINDOW "use wxTipWindow class" ON)
AddGlobalOption(wxUSE_TOGGLEBTN "use wxToggleButton class" ON)
AddGlobalOption(wxUSE_TOOLBAR "use wxToolBar class" ON)
AddGlobalOption(wxUSE_TOOLBOOK "use wxToolbook class" ON)
AddGlobalOption(wxUSE_TREEBOOK "use wxTreebook class" ON)
AddGlobalOption(wxUSE_TREECTRL "use wxTreeCtrl class" ON)

# FIXME: Intrinisically linked to dataview (inheritance)
AddGlobalOption(wxUSE_TREELISTCTRL "use wxTreeListCtrl class" OFF)

if(wxUSE_BUTTON AND wxUSE_TOGGLEBTN)
    add_compile_definitions(wxHAS_ANY_BUTTON)
endif()

# ---------------------------------------------------------------------------
# common dialogs
# ---------------------------------------------------------------------------

AddGlobalOption(wxUSE_COMMON_DIALOGS "use all common dialogs" ON)
AddGlobalOption(wxUSE_ABOUTDLG "use wxAboutBox" ON)
AddGlobalOption(wxUSE_CHOICEDLG "use wxChoiceDialog" ON)
AddGlobalOption(wxUSE_COLOURDLG "use wxColourDialog" ON)
AddGlobalOption(wxUSE_CREDENTIALDLG "use wxCredentialEntryDialog" ON)
AddGlobalOption(wxUSE_FILEDLG "use wxFileDialog" ON)
AddGlobalOption(wxUSE_FINDREPLDLG "use wxFindReplaceDialog" ON)
AddGlobalOption(wxUSE_FONTDLG "use wxFontDialog" ON)
AddGlobalOption(wxUSE_DIRDLG "use wxDirDialog" ON)
AddGlobalOption(wxUSE_MSGDLG "use wxMessageDialog" ON)
AddGlobalOption(wxUSE_NUMBERDLG "use wxNumberEntryDialog" ON)
AddGlobalOption(wxUSE_SPLASH "use wxSplashScreen" ON)
AddGlobalOption(wxUSE_TEXTDLG "use wxTextDialog" ON)
AddGlobalOption(wxUSE_STARTUP_TIPS "use startup tips" ON)
AddGlobalOption(wxUSE_PROGRESSDLG "use wxProgressDialog" ON)
AddGlobalOption(wxUSE_WIZARDDLG "use wxWizard" ON)

# ---------------------------------------------------------------------------
# misc GUI options
# ---------------------------------------------------------------------------

AddGlobalOption(wxUSE_MENUS "use wxMenu and wxMenuItem classes" ON)
AddGlobalOption(wxUSE_MENUBAR "use wxMenuBar class" ON)
AddGlobalOption(wxUSE_MINIFRAME "use wxMiniFrame class" ON)
AddGlobalOption(wxUSE_TOOLTIPS "use wxToolTip class" ON)
AddGlobalOption(wxUSE_SPLINES "use spline drawing code" ON)
AddGlobalOption(wxUSE_MOUSEWHEEL "use mousewheel" ON)
AddGlobalOption(wxUSE_VALIDATORS "use wxValidator and derived classes" ON)
AddGlobalOption(wxUSE_BUSYINFO "use wxBusyInfo" ON)
AddGlobalOption(wxUSE_HOTKEY "use wxWindow::RegisterHotKey()" ON)
AddGlobalOption(wxUSE_JOYSTICK "use wxJoystick" OFF) # FIXME: Requires use of Registry.
AddGlobalOption(wxUSE_METAFILE "use wxMetaFile" OFF)
AddGlobalOption(wxUSE_DRAGIMAGE "use wxDragImage" ON)
AddGlobalOption(wxUSE_UIACTIONSIMULATOR "use wxUIActionSimulator (experimental)" ON)
AddGlobalOption(wxUSE_DC_TRANSFORM_MATRIX "use wxDC::SetTransformMatrix and related" ON)
AddGlobalOption(wxUSE_WEBVIEW_WEBKIT "use wxWebView WebKit backend" ON)
if(WIN32 OR APPLE)
    set(wxUSE_PRIVATE_FONTS_DEFAULT ON)
else()
    set(wxUSE_PRIVATE_FONTS_DEFAULT OFF)
endif()
AddGlobalOption(wxUSE_PRIVATE_FONTS "use fonts not installed on the system" ${wxUSE_PRIVATE_FONTS_DEFAULT})

# ---------------------------------------------------------------------------
# support for image formats that do not rely on external library
# ---------------------------------------------------------------------------

AddGlobalOption(wxUSE_PALETTE "use wxPalette class" ON)
AddGlobalOption(wxUSE_IMAGE "use wxImage class" ON)
AddGlobalOption(wxUSE_GIF "use gif images (GIF file format)" ON)
AddGlobalOption(wxUSE_PCX "use pcx images (PCX file format)" ON)
AddGlobalOption(wxUSE_TGA "use tga images (TGA file format)" ON)
AddGlobalOption(wxUSE_IFF "use iff images (IFF file format)" ON)
AddGlobalOption(wxUSE_PNM "use pnm images (PNM file format)" ON)
AddGlobalOption(wxUSE_XPM "use xpm images (XPM file format)" ON)
AddGlobalOption(wxUSE_ICO_CUR "use Windows ICO and CUR formats" ON)

# ---------------------------------------------------------------------------
# wxMSW-only options
# ---------------------------------------------------------------------------

if(WIN32)
    if(MSVC_VERSION GREATER 1600 AND NOT CMAKE_VS_PLATFORM_TOOLSET MATCHES "_xp$")
        set(wxUSE_WINRT_DEFAULT ON)
    else()
        set(wxUSE_WINRT_DEFAULT OFF)
    endif()
    if(MSVC_VERSION GREATER 1800 AND NOT CMAKE_VS_PLATFORM_TOOLSET MATCHES "_xp$" AND
        EXISTS "${wxSOURCE_DIR}/3rdparty/webview2")
        set(wxUSE_WEBVIEW_EDGE_DEFAULT ON)
    else()
        set(wxUSE_WEBVIEW_EDGE_DEFAULT OFF)
    endif()

    AddGlobalOption(wxUSE_ACCESSIBILITY "enable accessibility support" ON)
    # FIXME: Intrinsically connected to wxVariant 
    AddGlobalOption(wxUSE_ACTIVEX " enable wxActiveXContainer class (Win32 only)" OFF)
    AddGlobalOption(wxUSE_CRASHREPORT "enable wxCrashReport::Generate() to create mini dumps (Win32 only)" ON)
    AddGlobalOption(wxUSE_DC_CACHEING "cache temporary wxDC objects (Win32 only)" ON)
    AddGlobalOption(wxUSE_NATIVE_PROGRESSDLG "use native progress dialog implementation" ON)
    AddGlobalOption(wxUSE_NATIVE_STATUSBAR "use native statusbar implementation)" ON)
    AddGlobalOption(wxUSE_OWNER_DRAWN "use owner drawn controls (Win32)" ON)
    AddGlobalOption(wxUSE_POSTSCRIPT_ARCHITECTURE_IN_MSW "use PS printing in wxMSW (Win32 only)" ON)
    AddGlobalOption(wxUSE_TASKBARICON_BALLOONS "enable wxTaskBarIcon::ShowBalloon() method (Win32 only)" ON)
    AddGlobalOption(wxUSE_UXTHEME "enable support for Windows XP themed look (Win32 only)" ON)
    AddGlobalOption(wxUSE_WEBVIEW_EDGE "use wxWebView Edge (Chromium) backend (Windows 7+ only)" ON)
    AddGlobalOption(wxUSE_WINRT "enable WinRT support" ${wxUSE_WINRT_DEFAULT})
    AddGlobalOption(wxUSE_WXDIB "use wxDIB class (Win32 only)" ON)
endif()

# this one is not really MSW-specific but it exists mainly to be turned 0
# under MSW, it should be 0 by default on the other platforms
if(WIN32)
    set(wxDEFAULT_wxUSE_AUTOID_MANAGEMENT ON)
else()
    set(wxDEFAULT_wxUSE_AUTOID_MANAGEMENT OFF)
endif()

AddGlobalOption(wxUSE_AUTOID_MANAGEMENT "use automatic ids management" ${wxDEFAULT_wxUSE_AUTOID_MANAGEMENT})

endif() # wxUSE_GUI
