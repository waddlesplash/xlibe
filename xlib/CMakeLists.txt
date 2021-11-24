aux_source_directory(. SOURCES)
add_library(X11 SHARED ${SOURCES})
target_link_libraries(X11 be translation)
