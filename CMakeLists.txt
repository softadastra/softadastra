cmake_minimum_required(VERSION 3.20)
project(drive-core LANGUAGES CXX)

# ===== C++ standard =====
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(VIX_ENABLE_SANITIZERS "Enable ASan/UBSan (dev only)" OFF)

# ===== Prefixes (adjust if your install lives elsewhere) =====
list(APPEND CMAKE_PREFIX_PATH 
  "/usr/local"
  "/usr/local/lib/cmake"
  "/usr/local/lib/cmake/vix"  # new lowercase package dir
  "/usr/local/lib/cmake/Vix"  # legacy package dir (exports fmt::fmt)
)

# ❗ Disable ORM completely in generated apps (core-only)
set(CMAKE_DISABLE_FIND_PACKAGE_VixOrm ON)

# ===== System deps first =====
find_package(Threads REQUIRED)
find_package(OpenSSL QUIET)
find_package(Boost REQUIRED COMPONENTS system thread filesystem)

# Optional: create a small shim if Boost::filesystem isn't exported
if (NOT TARGET Boost::filesystem)
  if (Boost_FOUND AND Boost_FILESYSTEM_LIBRARY)
    add_library(Boost::filesystem UNKNOWN IMPORTED)
    set_target_properties(Boost::filesystem PROPERTIES
      IMPORTED_LOCATION "${Boost_FILESYSTEM_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
    message(STATUS "Shim: Using Boost::filesystem from ${Boost_FILESYSTEM_LIBRARY}")
  else()
    message(FATAL_ERROR "Boost::filesystem not found and no library path provided.")
  endif()
endif()

# ===== fmt shim (must be BEFORE find_package(Vix/vix)) =====
# Some legacy Vix packages export vix::utils with fmt::fmt in interface.
# To avoid configure errors, ensure fmt::fmt exists (alias header-only or dummy).
find_package(fmt QUIET)
if (TARGET fmt::fmt-header-only AND NOT TARGET fmt::fmt)
  add_library(fmt::fmt ALIAS fmt::fmt-header-only)
endif()
if (NOT TARGET fmt::fmt)
  add_library(fmt::fmt INTERFACE IMPORTED)
  target_include_directories(fmt::fmt INTERFACE "/usr/include" "/usr/local/include")
endif()

# ===== Vix (core-only) — prefer new lowercase package, keep legacy fallback =====
set(vix_FOUND FALSE)
find_package(vix QUIET CONFIG)        # new
if (vix_FOUND)
  message(STATUS "Found vix (lowercase) package config")
else()
  find_package(Vix QUIET CONFIG)       # legacy
  if (Vix_FOUND)
    message(STATUS "Found Vix (legacy) package config")
    set(vix_FOUND TRUE)
  endif()
endif()

if (NOT vix_FOUND)
  message(FATAL_ERROR "Could not find Vix/vix package config. Set CMAKE_PREFIX_PATH to your install prefix.")
endif()

# Decide main target name (new vs legacy namespace)
if (TARGET vix::vix)
  set(VIX_MAIN_TARGET vix::vix)
elseif (TARGET Vix::vix)
  set(VIX_MAIN_TARGET Vix::vix)
else()
  message(FATAL_ERROR "Neither vix::vix nor Vix::vix target exists. Check your Vix/vix installation.")
endif()

# ===== Library target (core backend logic) =====
add_library(softadastra_drive_core STATIC
  src/app/App.cpp
  # src/config/Config.cpp
  # src/http/AuthController.cpp
  # src/http/FileController.cpp
  # src/http/FolderController.cpp
  # src/http/SyncController.cpp
  # src/service/AuthService.cpp
  # src/service/FileService.cpp
  # src/service/FolderService.cpp
  # src/service/SyncService.cpp
  # src/service/AiClient.cpp
  # src/repository/UserRepository.cpp
  # src/repository/ProjectRepository.cpp
  # src/repository/FileRepository.cpp
  # src/repository/SyncRepository.cpp
  # src/ws/SyncWebSocketServer.cpp
  # src/utils/IdGenerator.cpp
  # src/utils/Hash.cpp
)

target_include_directories(softadastra_drive_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(softadastra_drive_core
  PRIVATE
    ${VIX_MAIN_TARGET}
    Boost::system Boost::thread Boost::filesystem
    Threads::Threads
)

if (OpenSSL_FOUND)
  target_link_libraries(softadastra_drive_core PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

if (UNIX AND NOT APPLE)
  target_link_libraries(softadastra_drive_core PRIVATE dl)
endif()

# Warnings
if (MSVC)
  target_compile_options(softadastra_drive_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(softadastra_drive_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Sanitizers (optional, dev only)
if (VIX_ENABLE_SANITIZERS AND NOT MSVC)
  target_compile_options(softadastra_drive_core PRIVATE -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined)
  target_link_options(softadastra_drive_core PRIVATE -fsanitize=address,undefined)
endif()

# ===== Executable target (thin runner) =====
add_executable(drive-core
  src/main.cpp
)

target_link_libraries(drive-core
  PRIVATE
    softadastra_drive_core
)

set_target_properties(drive-core PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  INSTALL_RPATH_USE_LINK_PATH ON
)

# ===== Testing option =====
option(BUILD_TESTING "Build tests" ON)

if (BUILD_TESTING)
  include(CTest)
  add_subdirectory(tests)
endif()

# ===== Run helper =====
add_custom_target(run
  COMMAND $<TARGET_FILE:drive-core>
  DEPENDS drive-core
  USES_TERMINAL
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
