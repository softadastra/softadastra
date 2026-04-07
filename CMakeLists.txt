cmake_minimum_required(VERSION 3.20)

project(softadastra
    VERSION 0.1.0
    DESCRIPTION "Softadastra umbrella build"
    LANGUAGES CXX
)

include(GNUInstallDirs)

# Build context
set(SOFTADASTRA_UMBRELLA_BUILD ON)

# ========================
# Options
# ========================
option(SOFTADASTRA_BUILD_EXAMPLES "Build umbrella examples" OFF)
option(SOFTADASTRA_BUILD_APP "Build app module" OFF)
option(SOFTADASTRA_BUILD_CLI "Build cli module" OFF)

# ========================
# Global settings
# ========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ========================
# Helper
# ========================
function(softadastra_add_module module_name)
    set(module_dir "${CMAKE_CURRENT_SOURCE_DIR}/modules/${module_name}")

    if(EXISTS "${module_dir}/CMakeLists.txt")
        message(STATUS "[softadastra] adding module: ${module_name}")
        add_subdirectory("${module_dir}")
    else()
        message(STATUS "[softadastra] skipping module '${module_name}' (missing CMakeLists.txt)")
    endif()
endfunction()

# ========================
# Core modules
# ========================
softadastra_add_module(core)
softadastra_add_module(fs)
softadastra_add_module(wal)
softadastra_add_module(store)
softadastra_add_module(sync)
softadastra_add_module(transport)
softadastra_add_module(discovery)
softadastra_add_module(metadata)

# ========================
# Optional modules
# ========================
if(SOFTADASTRA_BUILD_APP)
    softadastra_add_module(app)
else()
    message(STATUS "[softadastra] app module disabled")
endif()

if(SOFTADASTRA_BUILD_CLI)
    softadastra_add_module(cli)
else()
    message(STATUS "[softadastra] cli module disabled")
endif()

# ========================
# Umbrella examples
# ========================
if(SOFTADASTRA_BUILD_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
        add_subdirectory(examples)
    else()
        message(STATUS "[softadastra] examples/ missing CMakeLists.txt -> skipping")
    endif()
endif()

# ========================
# Summary
# ========================
message(STATUS "------------------------------------------------------")
message(STATUS "Softadastra umbrella configured (${PROJECT_VERSION})")
message(STATUS "Build examples: ${SOFTADASTRA_BUILD_EXAMPLES}")
message(STATUS "Build app:      ${SOFTADASTRA_BUILD_APP}")
message(STATUS "Build cli:      ${SOFTADASTRA_BUILD_CLI}")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "------------------------------------------------------")
