cmake_minimum_required(VERSION 3.20)

project(softadastra
    VERSION 0.1.0
    DESCRIPTION "Softadastra umbrella build"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

# Build context
set(SOFTADASTRA_UMBRELLA_BUILD ON)

# ========================
# Options
# ========================
option(SOFTADASTRA_BUILD_EXAMPLES "Build umbrella examples" OFF)
option(SOFTADASTRA_BUILD_TESTS "Build umbrella tests" OFF)
option(SOFTADASTRA_BUILD_APP "Build app module" OFF)
option(SOFTADASTRA_BUILD_CLI "Build cli module" ON)
option(SOFTADASTRA_INSTALL "Install Softadastra umbrella package" ON)
option(SOFTADASTRA_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# ========================
# Propagate global options to submodules
# ========================
set(SOFTADASTRA_BUILD_TESTS
    ${SOFTADASTRA_BUILD_TESTS}
    CACHE BOOL "Build umbrella tests" FORCE
)

set(SOFTADASTRA_WARNINGS_AS_ERRORS
    ${SOFTADASTRA_WARNINGS_AS_ERRORS}
    CACHE BOOL "Treat warnings as errors" FORCE
)

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
# Umbrella package config
# ========================
if(SOFTADASTRA_INSTALL)
    if(SOFTADASTRA_BUILD_CLI)
        set(SOFTADASTRA_UMBRELLA_INCLUDE_CLI
            "include(\"\${_softadastra_cmake_dir}/softadastra_cli/softadastra_cliConfig.cmake\")"
        )
    else()
        set(SOFTADASTRA_UMBRELLA_INCLUDE_CLI "")
    endif()

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/softadastraConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/softadastraConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/softadastraConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/softadastra"
        PATH_VARS CMAKE_INSTALL_LIBDIR
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/softadastraConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/softadastraConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/softadastra"
    )

    install(
        FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
            "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/softadastra"
        OPTIONAL
    )
endif()

# ========================
# Summary
# ========================
message(STATUS "------------------------------------------------------")
message(STATUS "Softadastra umbrella configured (${PROJECT_VERSION})")
message(STATUS "Build examples: ${SOFTADASTRA_BUILD_EXAMPLES}")
message(STATUS "Build tests:    ${SOFTADASTRA_BUILD_TESTS}")
message(STATUS "Build app:      ${SOFTADASTRA_BUILD_APP}")
message(STATUS "Build cli:      ${SOFTADASTRA_BUILD_CLI}")
message(STATUS "Install:        ${SOFTADASTRA_INSTALL}")
message(STATUS "Warnings=error: ${SOFTADASTRA_WARNINGS_AS_ERRORS}")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "------------------------------------------------------")
