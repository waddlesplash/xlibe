add_executable(xhello xhello.c)
target_link_libraries(xhello X11)

add_executable(kt-clock kt-clock.c)
target_link_libraries(kt-clock X11)

add_executable(kt-draw kt-draw.c)
target_link_libraries(kt-draw X11)

add_executable(listfonts listfonts.c)
target_link_libraries(listfonts X11)

add_executable(mouse-events mouse-events.c)
target_link_libraries(mouse-events X11)
