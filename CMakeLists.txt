cmake_minimum_required(VERSION 3.8)
project(Xlibe)

include(GNUInstallDirs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

include_directories(include)
add_subdirectory(xlib)
add_subdirectory(xext)
add_subdirectory(test)

#####
## installation rules
#####
install(DIRECTORY include/X11 DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS X11 Xext DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(contents "
Name: X11
Description: X Library (Haiku compatibility)
Version: 1.6.12
Requires: xproto
Cflags:
Libs: -lX11 -Wl,--no-undefined
")
file(WRITE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/x11.pc ${contents})
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/x11.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

set(contents "
Name: Xext
Description: Misc X Extension Library (Haiku compatility)
Version: 1.3.3
Requires: xextproto
Requires.private: x11
Cflags:
Libs: -lXext
")
file(WRITE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/xext.pc ${contents})
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/xext.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
