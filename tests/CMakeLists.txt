cmake_minimum_required(VERSION 3.20)

# Si on arrive ici sans BUILD_TESTING, on sort.
if (NOT BUILD_TESTING)
  return()
endif()

message(STATUS "Configuring drive-core tests")

include(FetchContent)

# ===== Fetch Catch2 v3 =====
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.7.0 # tu peux ajuster si besoin
)

FetchContent_MakeAvailable(catch2)

# ===== Tests target =====
add_executable(drive-core-tests
  AuthServiceTests.cpp
  FileServiceTests.cpp
  # SyncServiceTests.cpp
  # etc. ajoute les fichiers au fur et à mesure
)

target_include_directories(drive-core-tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(drive-core-tests
  PRIVATE
    softadastra_drive_core
    Catch2::Catch2WithMain
)

# Intégration avec CTest
include(Catch)
catch_discover_tests(drive-core-tests)
