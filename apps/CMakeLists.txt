cmake_minimum_required(VERSION 3.20)

# ========================
# Apps options
# ========================
if(NOT DEFINED SOFTADASTRA_BUILD_APPS)
    option(SOFTADASTRA_BUILD_APPS "Build Softadastra applications" ON)
endif()

if(NOT DEFINED SOFTADASTRA_BUILD_CLI_APP)
    option(SOFTADASTRA_BUILD_CLI_APP "Build Softadastra CLI application" ON)
endif()

if(NOT DEFINED SOFTADASTRA_BUILD_NODE_APP)
    option(SOFTADASTRA_BUILD_NODE_APP "Build Softadastra node application" OFF)
endif()

if(NOT SOFTADASTRA_BUILD_APPS)
    message(STATUS "[apps] SOFTADASTRA_BUILD_APPS=OFF -> skipping apps")
    return()
endif()

# ========================
# CLI app
# ========================
if(SOFTADASTRA_BUILD_CLI_APP)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cli/CMakeLists.txt")
        add_subdirectory(cli)
    else()
        message(STATUS "[apps] cli/CMakeLists.txt missing -> skipping cli app")
    endif()
endif()

# ========================
# Node app
# ========================
if(SOFTADASTRA_BUILD_NODE_APP)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/node/CMakeLists.txt")
        add_subdirectory(node)
    else()
        message(STATUS "[apps] node/CMakeLists.txt missing -> skipping node app")
    endif()
endif()

# ========================
# Summary
# ========================
message(STATUS "------------------------------------------------------")
message(STATUS "softadastra apps configured")
message(STATUS "Build apps:      ${SOFTADASTRA_BUILD_APPS}")
message(STATUS "Build CLI app:   ${SOFTADASTRA_BUILD_CLI_APP}")
message(STATUS "Build node app:  ${SOFTADASTRA_BUILD_NODE_APP}")
message(STATUS "------------------------------------------------------")
