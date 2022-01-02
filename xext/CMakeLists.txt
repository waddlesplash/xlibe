aux_source_directory(. SOURCES)
add_library(Xext SHARED ${SOURCES})
target_link_libraries(Xext X11)
set_target_properties(Xext PROPERTIES SOVERSION 6)
