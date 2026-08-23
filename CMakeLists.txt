cmake_minimum_required(VERSION 3.20)

project(softadastra LANGUAGES CXX)

# --------------------------------------------------------------------
# Language
# --------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------------------------------------------
# Options
# --------------------------------------------------------------------

option(
  SOFTADASTRA_BUILD_TESTS
  "Build Softadastra tests"
  ON
)

# --------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------

find_package(Vix CONFIG REQUIRED)

# --------------------------------------------------------------------
# Internal implementation
#
# This is not a public Softadastra library or SDK.
# It only exists to share implementation between the executable
# and the tests.
# --------------------------------------------------------------------

add_library(softadastra_internal STATIC
  src/cli/Cli.cpp

  src/host/Host.cpp
  src/host/HostService.cpp
  src/host/HostState.cpp

  src/software/SoftwareManager.cpp

  src/connectivity/ConnectivityManager.cpp

  src/control/ControlClient.cpp
  src/control/ControlServer.cpp
)

target_include_directories(softadastra_internal
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(softadastra_internal
  PUBLIC
    vix::vix
)

# --------------------------------------------------------------------
# Warnings
# --------------------------------------------------------------------

if(MSVC)
  target_compile_options(softadastra_internal PRIVATE
    /W4
    /permissive-
  )
else()
  target_compile_options(softadastra_internal PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

# --------------------------------------------------------------------
# Executable
#
# main.cpp is implemented at the final application-entry step.
# EXCLUDE_FROM_ALL keeps the current staged build valid while main.cpp
# is intentionally still empty.
#
# Remove EXCLUDE_FROM_ALL when src/main.cpp is implemented.
# --------------------------------------------------------------------

add_executable(softadastra EXCLUDE_FROM_ALL
  src/main.cpp
)

target_link_libraries(softadastra
  PRIVATE
    softadastra_internal
)

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------

if(SOFTADASTRA_BUILD_TESTS)
  include(CTest)
  enable_testing()

  include(FetchContent)

  find_package(GTest CONFIG QUIET)

  if(NOT GTest_FOUND)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.14.0
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)
  endif()

  include(GoogleTest)

  function(softadastra_add_test target source)
    add_executable(${target}
      ${source}
    )

    target_link_libraries(${target}
      PRIVATE
        softadastra_internal
        GTest::gtest_main
    )

    if(MSVC)
      target_compile_options(${target} PRIVATE
        /W4
        /permissive-
      )
    else()
      target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
      )
    endif()

    gtest_discover_tests(${target})
  endfunction()

  # Host
  softadastra_add_test(
    softadastra_host_test
    tests/host/host_test.cpp
  )

  softadastra_add_test(
    softadastra_host_service_test
    tests/host/host_service_test.cpp
  )

  softadastra_add_test(
    softadastra_host_state_test
    tests/host/host_state_test.cpp
  )

  # Software
  softadastra_add_test(
    softadastra_software_id_test
    tests/software/software_id_test.cpp
  )

  softadastra_add_test(
    softadastra_software_entry_test
    tests/software/software_entry_test.cpp
  )

  softadastra_add_test(
    softadastra_software_state_test
    tests/software/software_state_test.cpp
  )

  softadastra_add_test(
    softadastra_software_manager_test
    tests/software/software_manager_test.cpp
  )

  # Connectivity
  softadastra_add_test(
    softadastra_connectivity_manager_test
    tests/connectivity/connectivity_manager_test.cpp
  )

  # Control
  softadastra_add_test(
    softadastra_control_client_test
    tests/control/control_client_test.cpp
  )

  softadastra_add_test(
    softadastra_control_server_test
    tests/control/control_server_test.cpp
  )

  # Platform
  softadastra_add_test(
    softadastra_platform_test
    tests/platform/platform_test.cpp
  )

  softadastra_add_test(
    softadastra_process_test
    tests/platform/process_test.cpp
  )

  softadastra_add_test(
    softadastra_service_test
    tests/platform/service_test.cpp
  )

  softadastra_add_test(
    softadastra_network_test
    tests/platform/network_test.cpp
  )
endif()
