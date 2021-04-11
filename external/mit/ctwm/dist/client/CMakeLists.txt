include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Build the libctwm_client library
add_library(ctwmc SHARED libctwm.c)

# There's a demolib that links against it
add_executable(demolib demolib.c)
target_link_libraries(demolib ctwmc ${X11_LIBRARIES})

# There's also a 'gtw' that doesn't, but is sorta client-like...
add_executable(gtw gtw.c)
target_link_libraries(gtw ${X11_LIBRARIES})

# A program to [attempt to] forward keystrokes from layered root-ish
# windows.
add_executable(forward forward.c)
target_link_libraries(forward ${X11_LIBRARIES})


# Meta-target
add_custom_target(client
	DEPENDS demolib gtw
)

# Mention it
message(STATUS "Building libctwmc and demolib.")
