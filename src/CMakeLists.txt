# Targets to build
# gl
# media
# qa
# webview
# xrc

add_subdirectory(algorithm)

if(wxUSE_AUI)
add_subdirectory(aui)
endif()

add_subdirectory(base)
add_subdirectory(core)
#add_subdirectory(html)
add_subdirectory(msw)
#add_subdirectory(net)
add_subdirectory(print)
# add_subdirectory(propgrid)
# add_subdirectory(ribbon)
#add_subdirectory(richtext)
add_subdirectory(StdLibCore)
add_subdirectory(type)
#add_subdirectory(webview)
add_subdirectory(xml)
add_subdirectory(xrc)
