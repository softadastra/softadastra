cmake_minimum_required(VERSION 3.20)

# ========================
# Softadastra node application
# ========================
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(softadastra_app_node
        VERSION 0.1.0
        DESCRIPTION "Softadastra node daemon application"
        LANGUAGES CXX
    )
endif()

# ========================
# Sources
# ========================
file(GLOB_RECURSE SOFTADASTRA_APP_NODE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# ========================
# Executable
# ========================
add_executable(softadastra-node
    ${SOFTADASTRA_APP_NODE_SOURCES}
)

target_compile_features(softadastra-node PRIVATE cxx_std_20)

# ========================
# Dependencies
# ========================
target_link_libraries(softadastra-node
    PRIVATE
        softadastra::core
        softadastra::fs
        softadastra::wal
        softadastra::store
        softadastra::sync
        softadastra::transport
        softadastra::discovery
        softadastra::metadata
)

# ========================
# Include directories
# ========================
target_include_directories(softadastra-node
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ========================
# Compiler warnings
# ========================
if(MSVC)
    target_compile_options(softadastra-node PRIVATE /W4 /permissive-)
    if(SOFTADASTRA_WARNINGS_AS_ERRORS)
        target_compile_options(softadastra-node PRIVATE /WX)
    endif()
else()
    target_compile_options(softadastra-node PRIVATE -Wall -Wextra -Wpedantic)
    if(SOFTADASTRA_WARNINGS_AS_ERRORS)
        target_compile_options(softadastra-node PRIVATE -Werror)
    endif()
endif()

# ========================
# Target properties
# ========================
set_target_properties(softadastra-node PROPERTIES
    OUTPUT_NAME softadastra-node
    VERSION 0.1.0
)

# ========================
# Install
# ========================
include(GNUInstallDirs)

install(
    TARGETS softadastra-node
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ========================
# Summary
# ========================
message(STATUS "------------------------------------------------------")
message(STATUS "softadastra node application configured (0.1.0)")
message(STATUS "Executable:      softadastra-node")
message(STATUS "Source dir:      ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Warnings=error:  ${SOFTADASTRA_WARNINGS_AS_ERRORS}")
message(STATUS "Build type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix:  ${CMAKE_INSTALL_PREFIX}")
message(STATUS "------------------------------------------------------")
