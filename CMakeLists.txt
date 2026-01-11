cmake_minimum_required(VERSION 3.20)

project(softadastra VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SA_BUILD_TESTS    "Build unit tests" ON)
option(SA_BUILD_EXAMPLES "Build examples"   ON)

option(SA_ENABLE_CORE    "Build softadastra core module" ON)
option(SA_ENABLE_SYNC    "Build softadastra sync module" ON)
option(SA_ENABLE_NET     "Build softadastra net module"  ON)
option(SA_ENABLE_EDGE    "Build softadastra edge module" OFF)

option(SA_ENABLE_ADAPTER_DESKTOP "Build desktop adapter" OFF)
option(SA_ENABLE_ADAPTER_WEB     "Build web adapter"     OFF)
option(SA_ENABLE_ADAPTER_BACKEND "Build backend adapter" OFF)

# Umbrella target
add_library(softadastra INTERFACE)
add_library(softadastra::softadastra ALIAS softadastra)

target_include_directories(softadastra INTERFACE
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Core (required by almost everything)
if (SA_ENABLE_CORE)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/core/CMakeLists.txt")
    add_subdirectory(modules/core core_build)
  else()
    message(FATAL_ERROR "Missing modules/core. Run: git submodule update --init --recursive")
  endif()
else()
  message(FATAL_ERROR "SA_ENABLE_CORE=OFF is not supported. core is required.")
endif()

# Sync
if (SA_ENABLE_SYNC)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/sync/CMakeLists.txt")
    add_subdirectory(modules/sync sync_build)
  else()
    message(FATAL_ERROR "Missing modules/sync. Run: git submodule update --init --recursive")
  endif()
endif()

# Net
if (SA_ENABLE_NET)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/net/CMakeLists.txt")
    add_subdirectory(modules/net net_build)
  else()
    message(FATAL_ERROR "Missing modules/net. Run: git submodule update --init --recursive")
  endif()
endif()

# Edge (optional)
if (SA_ENABLE_EDGE)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/edge/CMakeLists.txt")
    add_subdirectory(modules/edge edge_build)
  else()
    message(FATAL_ERROR "Missing modules/edge. Run: git submodule update --init --recursive")
  endif()
endif()

# Adapters
if (SA_ENABLE_ADAPTER_DESKTOP)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/adapters/desktop/CMakeLists.txt")
    add_subdirectory(modules/adapters/desktop adapter_desktop_build)
  else()
    message(FATAL_ERROR "Missing modules/adapters/desktop.")
  endif()
endif()

if (SA_ENABLE_ADAPTER_WEB)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/adapters/web/CMakeLists.txt")
    add_subdirectory(modules/adapters/web adapter_web_build)
  else()
    message(FATAL_ERROR "Missing modules/adapters/web.")
  endif()
endif()

if (SA_ENABLE_ADAPTER_BACKEND)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/adapters/backend/CMakeLists.txt")
    add_subdirectory(modules/adapters/backend adapter_backend_build)
  else()
    message(FATAL_ERROR "Missing modules/adapters/backend.")
  endif()
endif()

# Link umbrella to module targets if they exist
# Convention: each module exports softadastra::<name>
target_link_libraries(softadastra INTERFACE softadastra::core)

if (TARGET softadastra::sync)
  target_link_libraries(softadastra INTERFACE softadastra::sync)
endif()

if (TARGET softadastra::net)
  target_link_libraries(softadastra INTERFACE softadastra::net)
endif()

if (TARGET softadastra::edge)
  target_link_libraries(softadastra INTERFACE softadastra::edge)
endif()

if (TARGET softadastra::adapter_desktop)
  target_link_libraries(softadastra INTERFACE softadastra::adapter_desktop)
endif()

if (TARGET softadastra::adapter_web)
  target_link_libraries(softadastra INTERFACE softadastra::adapter_web)
endif()

if (TARGET softadastra::adapter_backend)
  target_link_libraries(softadastra INTERFACE softadastra::adapter_backend)
endif()

message(STATUS "------------------------------------------------------")
message(STATUS "Softadastra umbrella configured")
message(STATUS "Version           : ${PROJECT_VERSION}")
message(STATUS "Core              : ${SA_ENABLE_CORE}")
message(STATUS "Sync              : ${SA_ENABLE_SYNC}")
message(STATUS "Net               : ${SA_ENABLE_NET}")
message(STATUS "Edge              : ${SA_ENABLE_EDGE}")
message(STATUS "Adapter desktop   : ${SA_ENABLE_ADAPTER_DESKTOP}")
message(STATUS "Adapter web       : ${SA_ENABLE_ADAPTER_WEB}")
message(STATUS "Adapter backend   : ${SA_ENABLE_ADAPTER_BACKEND}")
message(STATUS "Examples          : ${SA_BUILD_EXAMPLES}")
message(STATUS "Tests             : ${SA_BUILD_TESTS}")
message(STATUS "------------------------------------------------------")
